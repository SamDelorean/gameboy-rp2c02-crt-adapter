#include "bridge.h"
#include "pattern_source.h"
#include "ppm.h"
#include "rp2c02_ext.h"

#include <stdio.h>
#include <stdlib.h>

#define CANVAS_W 1280
#define CANVAS_H 720

static rgb8_t shade_rgb(uint8_t shade)
{
    static const rgb8_t dmg[4] = {
        {198, 220, 140}, {132, 165, 99}, {57, 97, 57}, {8, 24, 16}
    };
    return dmg[shade & 3u];
}

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

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "comparison.ppm";

    uint8_t gb[GB_H][GB_W];
    uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W];
    pattern_source_make(gb);
    bridge_scale_frame(gb, ext, 0u);

    rp2c02_ext_t ppu;
    rp2c02_ext_reset(&ppu);
    rp2c02_ext_write_palette(&ppu, 0, 0x0f);
    rp2c02_ext_write_palette(&ppu, 1, 0x09);
    rp2c02_ext_write_palette(&ppu, 2, 0x19);
    rp2c02_ext_write_palette(&ppu, 3, 0x29);

    rgb8_t *canvas = calloc((size_t)CANVAS_W * CANVAS_H, sizeof(*canvas));
    if (!canvas) return 2;
    fill(canvas, (rgb8_t){238, 238, 238});

    rect(canvas, 60, 80, 520, 560, (rgb8_t){24, 24, 24});
    rect(canvas, 700, 80, 520, 560, (rgb8_t){24, 24, 24});

    /* Left: Game Boy reference stand-in, 160x144 at 3x integer scale. */
    const unsigned lx = 80, ly = 144, lscale = 3;
    for (unsigned y = 0; y < GB_H; ++y)
        for (unsigned x = 0; x < GB_W; ++x) {
            rgb8_t c = shade_rgb(gb[y][x]);
            rect(canvas, lx + x * lscale, ly + y * lscale, lscale, lscale, c);
        }

    /* Right: adapter path, 256x240 at 2x integer scale. */
    const unsigned rx = 704, ry = 120, rscale = 2;
    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y)
        for (unsigned x = 0; x < PPU_ACTIVE_W; ++x) {
            rgb8_t c = rp2c02_demo_rgb(rp2c02_ext_palette_code(&ppu, ext[y][x]));
            rect(canvas, rx + x * rscale, ry + y * rscale, rscale, rscale, c);
        }

    const int rc = ppm_write_rgb(out, canvas, CANVAS_W, CANVAS_H);
    free(canvas);
    if (rc != 0) return 3;

    printf("wrote %s (%dx%d comparison canvas)\n", out, CANVAS_W, CANVAS_H);
    printf("left: Game Boy reference 160x144; right: RP2C02 EXT path 256x240\n");
    return 0;
}
