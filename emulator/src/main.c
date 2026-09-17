#include "bridge.h"
#include "comparison_render.h"
#include "gb_source.h"
#include "ppm.h"
#include "rp2c02_ext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *argv0)
{
    fprintf(stderr,
            "usage: %s [--out comparison.ppm] [--frames N]"
#ifdef GBCRT_ENABLE_SAMEBOY
            " [--rom game.gb --boot dmg_boot.bin]"
#endif
            "\n",
            argv0);
}

static int create_source(gb_source_t *source,
                         const char *rom,
                         const char *boot)
{
    if (rom) {
#ifdef GBCRT_ENABLE_SAMEBOY
        if (!boot) {
            fprintf(stderr, "--rom currently requires --boot for the SameBoy DMG source\n");
            return -1;
        }
        if (gb_source_sameboy_create(source, rom, boot) != 0) {
            fprintf(stderr, "failed to initialize SameBoy source\n");
            return -1;
        }
        return 0;
#else
        (void)boot;
        fprintf(stderr,
                "this build has no SameBoy support; rebuild with "
                "-DGBCRT_ENABLE_SAMEBOY=ON -DSAMEBOY_ROOT=/path/to/SameBoy\n");
        return -1;
#endif
    }

    return gb_source_pattern_create(source);
}

int main(int argc, char **argv)
{
    const char *out = "comparison.ppm";
    const char *rom = NULL;
    const char *boot = NULL;
    unsigned frames = 1;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            out = argv[++i];
        }
        else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            frames = (unsigned)strtoul(argv[++i], NULL, 10);
            if (frames == 0) frames = 1;
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
    if (create_source(&source, rom, boot) != 0) return 2;

    gb_source_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    for (unsigned n = 0; n < frames; ++n) {
        if (gb_source_next_frame(&source, &frame) != 0) {
            fprintf(stderr, "source frame generation failed\n");
            gb_source_destroy(&source);
            return 3;
        }
    }

    uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W];
    bridge_scale_frame(frame.shade, ext, 0u);

    rp2c02_ext_t ppu;
    rp2c02_ext_reset(&ppu);
    rp2c02_ext_write_palette(&ppu, 0, 0x0f);
    rp2c02_ext_write_palette(&ppu, 1, 0x09);
    rp2c02_ext_write_palette(&ppu, 2, 0x19);
    rp2c02_ext_write_palette(&ppu, 3, 0x29);

    rgb8_t *canvas = calloc((size_t)COMPARISON_W * COMPARISON_H, sizeof(*canvas));
    if (!canvas) {
        gb_source_destroy(&source);
        return 4;
    }

    comparison_render(canvas, &frame, ext, &ppu);

    const int rc = ppm_write_rgb(out, canvas, COMPARISON_W, COMPARISON_H);
    free(canvas);

    const char *source_name = source.ops && source.ops->name ? source.ops->name : "unknown";
    gb_source_destroy(&source);

    if (rc != 0) return 5;

    printf("wrote %s (%dx%d comparison canvas)\n", out, COMPARISON_W, COMPARISON_H);
    printf("source: %s, frame: %llu\n", source_name,
           (unsigned long long)frame.frame_number);
    printf("left: source reference 160x144; right: RP2C02 EXT path 256x240\n");
    return 0;
}
