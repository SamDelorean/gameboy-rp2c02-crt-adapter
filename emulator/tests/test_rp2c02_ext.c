#include "rp2c02_ext.h"

#include <assert.h>
#include <stdio.h>

static int rgb_equal(rgb8_t a, rgb8_t b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

int main(void)
{
    rp2c02_ext_t ppu;
    rp2c02_ext_reset(&ppu);

    for (unsigned i = 0; i < 32; ++i) {
        assert(ppu.palette_ram[i] == 0u);
    }

    /* Palette RAM stores only the six physical color-code bits. */
    rp2c02_ext_write_palette(&ppu, 0, 0x8fu);
    assert(rp2c02_ext_palette_code(&ppu, 0) == 0x0fu);

    rp2c02_ext_write_palette(&ppu, 1, 0x09u);
    rp2c02_ext_write_palette(&ppu, 2, 0x19u);
    rp2c02_ext_write_palette(&ppu, 3, 0x29u);
    assert(rp2c02_ext_palette_code(&ppu, 1) == 0x09u);
    assert(rp2c02_ext_palette_code(&ppu, 2) == 0x19u);
    assert(rp2c02_ext_palette_code(&ppu, 3) == 0x29u);

    /* Same hue, different PPU value bits must remain visibly distinct. */
    const rgb8_t dark_green = rp2c02_demo_rgb(0x09u);
    const rgb8_t mid_green = rp2c02_demo_rgb(0x19u);
    const rgb8_t light_green = rp2c02_demo_rgb(0x29u);
    assert(!rgb_equal(dark_green, mid_green));
    assert(!rgb_equal(mid_green, light_green));
    assert(!rgb_equal(dark_green, light_green));

    /* Canonical project black remains black. */
    const rgb8_t black = rp2c02_demo_rgb(0x0fu);
    assert(black.r == 0u && black.g == 0u && black.b == 0u);

    /* Preview lookup itself is six-bit just like the PPU color value. */
    assert(rgb_equal(rp2c02_demo_rgb(0x29u), rp2c02_demo_rgb(0x69u)));

    puts("RP2C02 EXT palette OK: full six-bit color code preserved");
    return 0;
}
