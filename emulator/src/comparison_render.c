#include "comparison_render.h"

#include <stddef.h>

static void fill(rgb8_t *canvas, rgb8_t c)
{
    for (size_t i = 0; i < (size_t)COMPARISON_W * COMPARISON_H; ++i) {
        canvas[i] = c;
    }
}

static void rect(rgb8_t *canvas,
                 unsigned x0,
                 unsigned y0,
                 unsigned w,
                 unsigned h,
                 rgb8_t c)
{
    for (unsigned y = y0; y < y0 + h && y < COMPARISON_H; ++y) {
        for (unsigned x = x0; x < x0 + w && x < COMPARISON_W; ++x) {
            canvas[(size_t)y * COMPARISON_W + x] = c;
        }
    }
}

static rgb8_t rgb_from_rgba(uint32_t rgba)
{
    return (rgb8_t){
        (uint8_t)(rgba & 0xffu),
        (uint8_t)((rgba >> 8) & 0xffu),
        (uint8_t)((rgba >> 16) & 0xffu),
    };
}

void comparison_render(rgb8_t *canvas,
                       const gb_source_frame_t *frame,
                       const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
                       const rp2c02_ext_t *ppu)
{
    fill(canvas, (rgb8_t){238, 238, 238});

    /* Two deliberately obvious dark frames around the comparison images. */
    rect(canvas, 60, 80, 520, 560, (rgb8_t){24, 24, 24});
    rect(canvas, 700, 80, 520, 560, (rgb8_t){24, 24, 24});

    /* Left: source-core reference, 160x144 at exact 3x integer scale. */
    const unsigned lx = 80;
    const unsigned ly = 144;
    const unsigned lscale = 3;
    for (unsigned y = 0; y < GB_H; ++y) {
        for (unsigned x = 0; x < GB_W; ++x) {
            rect(canvas,
                 lx + x * lscale,
                 ly + y * lscale,
                 lscale,
                 lscale,
                 rgb_from_rgba(frame->reference_rgba[y][x]));
        }
    }

    /* Right: adapter output, 256x240 at exact 2x integer scale. */
    const unsigned rx = 704;
    const unsigned ry = 120;
    const unsigned rscale = 2;
    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y) {
        for (unsigned x = 0; x < PPU_ACTIVE_W; ++x) {
            const uint8_t code = rp2c02_ext_palette_code(ppu, ext[y][x]);
            rect(canvas,
                 rx + x * rscale,
                 ry + y * rscale,
                 rscale,
                 rscale,
                 rp2c02_demo_rgb(code));
        }
    }
}
