#include "bridge.h"

unsigned bridge_source_x(unsigned out_x)
{
    /* Canonical center-sampled mapping: floor((x + 0.5) * 160 / 234). */
    return ((2u * out_x + 1u) * GB_W) / (2u * BRIDGE_IMAGE_W);
}

unsigned bridge_source_y(unsigned out_y)
{
    /* 144 -> 240. This yields the exact repeating 2,1,2 pattern per 3 source lines. */
    return ((2u * out_y + 1u) * GB_H) / (2u * PPU_ACTIVE_H);
}

void bridge_scale_frame(const uint8_t src[GB_H][GB_W],
                        uint8_t dst[PPU_ACTIVE_H][PPU_ACTIVE_W],
                        uint8_t border_index)
{
    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y) {
        const unsigned sy = bridge_source_y(y);

        for (unsigned x = 0; x < BRIDGE_BORDER_W; ++x) {
            dst[y][x] = border_index;
        }

        for (unsigned x = 0; x < BRIDGE_IMAGE_W; ++x) {
            dst[y][BRIDGE_BORDER_W + x] = src[sy][bridge_source_x(x)] & 0x03u;
        }

        for (unsigned x = BRIDGE_BORDER_W + BRIDGE_IMAGE_W; x < PPU_ACTIVE_W; ++x) {
            dst[y][x] = border_index;
        }
    }
}
