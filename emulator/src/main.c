#include "bridge.h"
#include "gb_source.h"
#include "ppm.h"
#include "rp2c02_ext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CANVAS_W 1280
#define CANVAS_H 720

static void fill(rgb8_t *canvas, rgb8_t c)
{
    for (size_t i = 0; i < (size_t)CANVAS_W * CANVAS_H; ++i) canvas[i] = c;
}

static void rect(rgb8_t *canvas, unsigned x0, unsigned y0, unsigned w, unsigned h, rgb8_t c)
{
    for (unsigned y = y0; y < y0 + h && y < CANVAS_H; ++y)
        for (unsigned x = x0; x < x0 + w && x < CANVAS_W; ++x)
            canvas[(size_t)y * CANVAS_W + x] = c;
}

static rgb8_t rgb_from_rgba(uint32_t rgba)
{
    return (rgb8_t){
        (uint8_t)(rgba & 0xffu),
        (uint8_t)((rgba >> 8) & 0xffu),
        (uint8_t)((rgba >> 16) & 0xffu),
    };
}

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

    if (rom) {
#ifdef GBCRT_ENABLE_SAMEBOY
        if (!boot) {
            fprintf(stderr, "--rom currently requires --boot for the SameBoy DMG source\n");
            return 1;
        }
        if (gb_source_sameboy_create(&source, rom, boot) != 0) {
            fprintf(stderr, "failed to initialize SameBoy source\n");
            return 2;
        }
#else
        fprintf(stderr,
                "this build has no SameBoy support; rebuild with "
                "-DGBCRT_ENABLE_SAMEBOY=ON -DSAMEBOY_ROOT=/path/to/SameBoy\n");
        return 2;
#endif
    }
    else {
        if (gb_source_pattern_create(&source) != 0) return 2;
    }

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

    rgb8_t *canvas = calloc((size_t)CANVAS_W * CANVAS_H, sizeof(*canvas));
    if (!canvas) {
        gb_source_destroy(&source);
        return 4;
    }
    fill(canvas, (rgb8_t){238, 238, 238});

    rect(canvas, 60, 80, 520, 560, (rgb8_t){24, 24, 24});
    rect(canvas, 700, 80, 520, 560, (rgb8_t){24, 24, 24});

    /* Left: normal reference frame supplied by the selected Game Boy source. */
    const unsigned lx = 80, ly = 144, lscale = 3;
    for (unsigned y = 0; y < GB_H; ++y)
        for (unsigned x = 0; x < GB_W; ++x) {
            rgb8_t c = rgb_from_rgba(frame.reference_rgba[y][x]);
            rect(canvas, lx + x * lscale, ly + y * lscale, lscale, lscale, c);
        }

    /* Right: project's bridge + reduced RP2C02 EXT/palette path. */
    const unsigned rx = 704, ry = 120, rscale = 2;
    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y)
        for (unsigned x = 0; x < PPU_ACTIVE_W; ++x) {
            rgb8_t c = rp2c02_demo_rgb(rp2c02_ext_palette_code(&ppu, ext[y][x]));
            rect(canvas, rx + x * rscale, ry + y * rscale, rscale, rscale, c);
        }

    const int rc = ppm_write_rgb(out, canvas, CANVAS_W, CANVAS_H);
    free(canvas);

    const char *source_name = source.ops && source.ops->name ? source.ops->name : "unknown";
    gb_source_destroy(&source);

    if (rc != 0) return 5;

    printf("wrote %s (%dx%d comparison canvas)\n", out, CANVAS_W, CANVAS_H);
    printf("source: %s, frame: %llu\n", source_name,
           (unsigned long long)frame.frame_number);
    printf("left: source reference 160x144; right: RP2C02 EXT path 256x240\n");
    return 0;
}
