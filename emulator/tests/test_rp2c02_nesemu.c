#include "bridge.h"
#include "rp2c02_ext.h"
#include "rp2c02_nesemu.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    rp2c02_ext_t state;
    rp2c02_ext_reset(&state);

    static const uint8_t palette[5] = {
        0x01u, 0x12u, 0x23u, 0x34u, 0x0fu,
    };
    for (unsigned i = 0; i < 5u; ++i) {
        rp2c02_ext_write_palette(&state, i, palette[i]);
    }

    static uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W];
    static uint8_t out[PPU_ACTIVE_H][PPU_ACTIVE_W];

    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y) {
        for (unsigned x = 0; x < PPU_ACTIVE_W; ++x) {
            ext[y][x] = (uint8_t)((x + y) % 5u);
        }
    }

    assert(rp2c02_nesemu_render(ext, &state, out) == 0);

    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y) {
        for (unsigned x = 0; x < PPU_ACTIVE_W; ++x) {
            assert(out[y][x] == palette[ext[y][x]]);
        }
    }

    assert(rp2c02_nesemu_revision() != NULL);
    puts("rp2c02_nesemu: PASS");
    return 0;
}
