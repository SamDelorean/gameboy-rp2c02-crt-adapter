#include "rp2c02_ext.h"

#include <assert.h>
#include <stdio.h>

static int rgb_equal(rgb8_t a, rgb8_t b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

static void set_ppu_address(rp2c02_ext_t *ppu, uint16_t address)
{
    rp2c02_ext_cpu_write(ppu, RP2C02_REG_ADDR, (uint8_t)(address >> 8));
    rp2c02_ext_cpu_write(ppu, RP2C02_REG_ADDR, (uint8_t)address);
}

int main(void)
{
    rp2c02_ext_t ppu;
    rp2c02_ext_reset(&ppu);

    for (unsigned i = 0; i < 32; ++i) {
        assert(ppu.palette_ram[i] == 0u);
    }
    assert(ppu.ctrl == 0u);
    assert(ppu.mask == 0u);
    assert(ppu.vram_address == 0u);
    assert(ppu.vram_temp_address == 0u);
    assert(ppu.ppuaddr_high_next);
    assert(!rp2c02_ext_rendering_enabled(&ppu));
    assert(!rp2c02_ext_palette_override_active(&ppu));

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

    /* Physical palette mirrors: sprite entry-0 addresses mirror background
       entry-0 addresses, while $04/$08/$0C remain independently addressable. */
    rp2c02_ext_write_palette(&ppu, 4u, 0x21u);
    rp2c02_ext_write_palette(&ppu, 0x14u, 0x16u);
    assert(ppu.palette_ram[4] == 0x16u);
    assert(rp2c02_ext_palette_index_for_address(0x3f10u) == 0u);
    assert(rp2c02_ext_palette_index_for_address(0x3f14u) == 4u);
    assert(rp2c02_ext_palette_index_for_address(0x3f18u) == 8u);
    assert(rp2c02_ext_palette_index_for_address(0x3f1cu) == 12u);
    assert(rp2c02_ext_palette_index_for_address(0x3f24u) == 4u);

    /* The first PPUADDR write changes temporary t only. v, and therefore EXT
       output selection, changes only after the second write copies t -> v. */
    set_ppu_address(&ppu, 0x0000u);
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_ADDR, 0x3fu);
    assert(ppu.vram_address == 0x0000u);
    assert((ppu.vram_temp_address & 0x3f00u) == 0x3f00u);
    assert(!ppu.ppuaddr_high_next);
    assert(!rp2c02_ext_palette_override_active(&ppu));
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_ADDR, 0x05u);
    assert(ppu.vram_address == 0x3f05u);
    assert(ppu.ppuaddr_high_next);
    assert(rp2c02_ext_palette_override_active(&ppu));

    /* Exercise the exact write-only register subset used by the project. */
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_DATA, 0x2au);
    assert(ppu.palette_ram[5] == 0x2au);
    assert(ppu.vram_address == 0x3f06u);

    set_ppu_address(&ppu, 0x3f10u);
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_DATA, 0x31u);
    assert(ppu.palette_ram[0] == 0x31u);

    /* With rendering disabled and v outside palette RAM, EXT selects the
       background palette entry directly. */
    rp2c02_ext_write_palette(&ppu, 2u, 0x22u);
    set_ppu_address(&ppu, 0x0000u);
    assert(!rp2c02_ext_palette_override_active(&ppu));
    assert(rp2c02_ext_rendering_disabled_code(&ppu, 2u) == 0x22u);

    /* Leaving v in palette RAM overrides EXT, reproducing the real PPU trap
       that the firmware avoids by restoring PPUADDR to $0000. */
    set_ppu_address(&ppu, 0x3f05u);
    assert(rp2c02_ext_palette_override_active(&ppu));
    assert(rp2c02_ext_rendering_disabled_code(&ppu, 2u) == 0x2au);

    set_ppu_address(&ppu, 0x0000u);
    assert(!rp2c02_ext_palette_override_active(&ppu));
    assert(rp2c02_ext_rendering_disabled_code(&ppu, 2u) == 0x22u);

    /* EXT output/slave mode forces the internal EXT input to zero. The project
       keeps this bit clear on hardware, but the reduced model makes that rule
       explicit so an accidental configuration is detectable. */
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_CTRL, RP2C02_CTRL_EXT_OUTPUT);
    assert(rp2c02_ext_rendering_disabled_code(&ppu, 2u) == 0x31u);
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_CTRL, 0u);

    /* Greyscale is an index mask ($30), not a change to stored palette RAM. */
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_MASK, RP2C02_MASK_GREYSCALE);
    assert(rp2c02_ext_rendering_disabled_code(&ppu, 2u) == 0x20u);
    assert(ppu.palette_ram[2] == 0x22u);
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_MASK, 0u);

    /* PPUCTRL bit 2 selects the documented +32 PPUDATA increment. */
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_CTRL, RP2C02_CTRL_INCREMENT_32);
    set_ppu_address(&ppu, 0x3f01u);
    rp2c02_ext_cpu_write(&ppu, RP2C02_REG_DATA, 0x12u);
    assert(ppu.vram_address == 0x3f21u);

    puts("RP2C02 EXT model OK: six-bit colors, mirrors, host writes, PPUADDR latch, and backdrop override");
    return 0;
}
