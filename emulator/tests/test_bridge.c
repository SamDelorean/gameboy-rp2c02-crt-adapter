#include "bridge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    unsigned hx[GB_W] = {0};
    unsigned vy[GB_H] = {0};

    for (unsigned x = 0; x < BRIDGE_IMAGE_W; ++x) {
        unsigned sx = bridge_source_x(x);
        assert(sx < GB_W);
        hx[sx]++;
    }
    unsigned once = 0, twice = 0;
    for (unsigned i = 0; i < GB_W; ++i) {
        assert(hx[i] == 1u || hx[i] == 2u);
        if (hx[i] == 1u) once++;
        else twice++;
    }
    assert(once == 86u);
    assert(twice == 74u);

    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y) {
        unsigned sy = bridge_source_y(y);
        assert(sy < GB_H);
        vy[sy]++;
    }
    for (unsigned i = 0; i < GB_H; ++i) {
        assert(vy[i] == 1u || vy[i] == 2u);
    }
    for (unsigned g = 0; g < GB_H / 3u; ++g) {
        assert(vy[g * 3u + 0u] == 2u);
        assert(vy[g * 3u + 1u] == 1u);
        assert(vy[g * 3u + 2u] == 2u);
    }

    uint8_t src[GB_H][GB_W];
    uint8_t dst[PPU_ACTIVE_H][PPU_ACTIVE_W];
    memset(src, 3, sizeof(src));
    bridge_scale_frame(src, dst, 0);
    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y) {
        for (unsigned x = 0; x < BRIDGE_BORDER_W; ++x) assert(dst[y][x] == 0u);
        for (unsigned x = BRIDGE_BORDER_W; x < BRIDGE_BORDER_W + BRIDGE_IMAGE_W; ++x) assert(dst[y][x] == 3u);
        for (unsigned x = BRIDGE_BORDER_W + BRIDGE_IMAGE_W; x < PPU_ACTIVE_W; ++x) assert(dst[y][x] == 0u);
    }

    puts("bridge geometry OK: 160x144 -> 234x240 + 11/11 border");
    return 0;
}
