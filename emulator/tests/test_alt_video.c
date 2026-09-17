#include "adapter_palette.h"
#include "gbcrt_alt_video.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int rgb_equal(rgb8_t a, rgb8_t b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

int main(void)
{
    gbcrt_alt_video_t alt;
    gbcrt_alt_video_init(&alt);

    uint8_t shade[GB_H][GB_W];
    for (unsigned y = 0; y < GB_H; ++y) {
        for (unsigned x = 0; x < GB_W; ++x) {
            shade[y][x] = (uint8_t)((x / 20u) & 3u);
        }
    }

    rgb8_t out0[PPU_ACTIVE_H][PPU_ACTIVE_W];
    rgb8_t out1[PPU_ACTIVE_H][PPU_ACTIVE_W];

    gbcrt_alt_video_render(&alt, shade, NULL, false, out0);

    assert(gbcrt_alt_video_palette_mode(&alt) == 0u);

    /* Side border is always canonical black. */
    const rgb8_t black = rp2c02_demo_rgb(0x0fu);
    assert(rgb_equal(out0[100][0], black));
    assert(rgb_equal(out0[100][10], black));
    assert(rgb_equal(out0[100][245], black));
    assert(rgb_equal(out0[100][255], black));

    /* Manual preset 1 is intentionally the same DMG-green fallback as AUTO.
     * Select preset 2 so this regression checks an actually different mapping. */
    gbcrt_alt_video_set_palette_mode(&alt, 2u);
    assert(gbcrt_alt_video_palette_mode(&alt) == 2u);
    gbcrt_alt_video_render(&alt, shade, NULL, false, out1);

    int changed = 0;
    for (unsigned y = 0; y < PPU_ACTIVE_H && !changed; ++y) {
        for (unsigned x = BRIDGE_BORDER_W;
             x < BRIDGE_BORDER_W + BRIDGE_IMAGE_W;
             ++x) {
            if (!rgb_equal(out0[y][x], out1[y][x])) {
                changed = 1;
                break;
            }
        }
    }
    assert(changed);

    /* Cycling from the current mode through the remaining modes returns to AUTO/SGB. */
    const unsigned total_modes = adapter_palette_count() + 1u;
    for (unsigned i = 0; i < total_modes - 2u; ++i) {
        gbcrt_alt_video_next_palette(&alt);
    }
    assert(gbcrt_alt_video_palette_mode(&alt) == 0u);

    /* AUTO/SGB must consume already-decoded SGB RGB555 when supplied. */
    const uint16_t sgb_palette[4] = {
        0x7fffu, /* white */
        0x03ffu,
        0x001fu,
        0x0000u, /* black */
    };
    gbcrt_alt_video_render(&alt, shade, sgb_palette, true, out1);

    changed = 0;
    for (unsigned y = 0; y < PPU_ACTIVE_H && !changed; ++y) {
        for (unsigned x = BRIDGE_BORDER_W;
             x < BRIDGE_BORDER_W + BRIDGE_IMAGE_W;
             ++x) {
            if (!rgb_equal(out0[y][x], out1[y][x])) {
                changed = 1;
                break;
            }
        }
    }
    assert(changed);

    puts("alt_video: PASS");
    return 0;
}
