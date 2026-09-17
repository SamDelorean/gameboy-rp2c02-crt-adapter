#include "adapter_palette.h"
#include "bridge.h"
#include "clock_mode.h"
#include "comparison_render.h"
#include "gb_source.h"
#include "ppm.h"
#include "rp2c02_ext.h"
#include "source_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *argv0)
{
    fprintf(stderr,
            "usage: %s [--out comparison.ppm] [--frames N] [--clock stock|sync]"
            " [--source-model dmg|sgb]"
#ifdef GBCRT_ENABLE_SAMEBOY
            " [--rom game.gb --boot boot.bin]"
#endif
            "\n",
            argv0);
}

static int parse_clock_mode(const char *text, gbcrt_clock_mode_t *mode)
{
    if (strcmp(text, "stock") == 0) {
        *mode = GBCRT_CLOCK_STOCK;
        return 0;
    }
    if (strcmp(text, "sync") == 0) {
        *mode = GBCRT_CLOCK_SYNC;
        return 0;
    }
    return -1;
}

static int create_source(gb_source_t *source,
                         const char *rom,
                         const char *boot,
                         gbcrt_source_model_t source_model)
{
    if (rom) {
#ifdef GBCRT_ENABLE_SAMEBOY
        if (!boot) {
            fprintf(stderr, "--rom currently requires --boot for the SameBoy source\n");
            return -1;
        }
        if (gb_source_sameboy_create_model(source, rom, boot, source_model) != 0) {
            fprintf(stderr, "failed to initialize SameBoy %s source\n",
                    gbcrt_source_model_name(source_model));
            return -1;
        }
        return 0;
#else
        (void)boot;
        (void)source_model;
        fprintf(stderr,
                "this build has no SameBoy support; rebuild with "
                "-DGBCRT_ENABLE_SAMEBOY=ON -DSAMEBOY_ROOT=/path/to/SameBoy\n");
        return -1;
#endif
    }

    return gb_source_pattern_create(source);
}

static int advance_source(gb_source_t *source,
                          gb_source_frame_t *frame,
                          unsigned count)
{
    for (unsigned i = 0; i < count; ++i) {
        if (gb_source_next_frame(source, frame) != 0) return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    const char *out = "comparison.ppm";
    const char *rom = NULL;
    const char *boot = NULL;
    unsigned output_frames = 1;
    gbcrt_clock_mode_t clock_mode = GBCRT_CLOCK_SYNC;
    gbcrt_source_model_t source_model = GBCRT_SOURCE_MODEL_DMG;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            out = argv[++i];
        }
        else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            output_frames = (unsigned)strtoul(argv[++i], NULL, 10);
            if (output_frames == 0) output_frames = 1;
        }
        else if (strcmp(argv[i], "--clock") == 0 && i + 1 < argc) {
            if (parse_clock_mode(argv[++i], &clock_mode) != 0) {
                usage(argv[0]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "--source-model") == 0 && i + 1 < argc) {
            if (gbcrt_source_model_parse(argv[++i], &source_model) != 0) {
                usage(argv[0]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "--rom") == 0 && i + 1 < argc) {
            rom = argv[++i];
        }
        else if (strcmp(argv[i], "--boot") == 0 && i + 1 < argc) {
            boot = argv[++i];
        }
        else {
            usage(argv[0]);
            return 1;
        }
    }

    gb_source_t source = {0};
    if (create_source(&source, rom, boot, source_model) != 0) return 2;

    gb_source_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    if (gb_source_next_frame(&source, &frame) != 0) {
        fprintf(stderr, "initial source frame generation failed\n");
        gb_source_destroy(&source);
        return 3;
    }

    gbcrt_clock_scheduler_t scheduler;
    gbcrt_clock_scheduler_init(&scheduler, clock_mode, source_model);
    scheduler.output_frames = 1;
    scheduler.source_frames = 1;

    for (unsigned n = 1; n < output_frames; ++n) {
        const unsigned advances = gbcrt_clock_scheduler_step(&scheduler);
        if (advance_source(&source, &frame, advances) != 0) {
            fprintf(stderr, "source frame generation failed\n");
            gb_source_destroy(&source);
            return 3;
        }
    }

    uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W];
    bridge_scale_frame(frame.shade, ext, BRIDGE_BORDER_EXT_INDEX);

    rp2c02_ext_t ppu;
    rp2c02_ext_reset(&ppu);
    const unsigned palette_index = 0u;
    adapter_palette_apply(&ppu, palette_index);
    const adapter_palette_preset_t *palette = adapter_palette_get(palette_index);

    rgb8_t *canvas = calloc((size_t)COMPARISON_W * COMPARISON_H, sizeof(*canvas));
    if (!canvas) {
        gb_source_destroy(&source);
        return 4;
    }

    const char *source_name = source.ops && source.ops->name ? source.ops->name : "unknown";
    comparison_view_state_t view = {
        .clock_mode = clock_mode,
        .source_model = source_model,
        .menu_open = 0,
        .menu_selection = (int)clock_mode,
        .paused = 0,
        .source_name = source_name,
        .palette_name = palette ? palette->name : "unknown",
    };
    comparison_render(canvas, &frame, ext, &ppu, &view);

    const int rc = ppm_write_rgb(out, canvas, COMPARISON_W, COMPARISON_H);
    free(canvas);
    gb_source_destroy(&source);

    if (rc != 0) return 5;

    printf("wrote %s (%dx%d comparison canvas)\n", out, COMPARISON_W, COMPARISON_H);
    printf("source: %s, model: %s, GB frame: %llu, output frame: %llu, repeats: %llu, skipped source: %llu\n",
           source_name,
           gbcrt_source_model_name(source_model),
           (unsigned long long)frame.frame_number,
           scheduler.output_frames,
           scheduler.repeated_output_frames,
           scheduler.skipped_source_frames);
    printf("clock mode: %s, GB %.6f Hz, GB frame %.6f Hz, PPU frame %.6f Hz\n",
           gbcrt_clock_mode_name(clock_mode),
           gbcrt_gb_clock_hz(clock_mode, source_model),
           gbcrt_gb_frame_hz(clock_mode, source_model),
           gbcrt_ppu_frame_hz());
    printf("palette: %s; border EXT index: %u -> $0F black\n",
           palette ? palette->name : "unknown",
           BRIDGE_BORDER_EXT_INDEX);
    printf("left: source reference 160x144; right: RP2C02 EXT path 256x240\n");
    return 0;
}
