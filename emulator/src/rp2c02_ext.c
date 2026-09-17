#include "rp2c02_ext.h"
#include <string.h>

void rp2c02_ext_reset(rp2c02_ext_t *ppu)
{
    memset(ppu, 0, sizeof(*ppu));
}

void rp2c02_ext_write_palette(rp2c02_ext_t *ppu, unsigned index, uint8_t nes_color_code)
{
    if (index < 32u) {
        ppu->palette_ram[index] = nes_color_code & 0x3fu;
    }
}

uint8_t rp2c02_ext_palette_code(const rp2c02_ext_t *ppu, uint8_t ext_nibble)
{
    return ppu->palette_ram[ext_nibble & 0x0fu] & 0x3fu;
}

rgb8_t rp2c02_demo_rgb(uint8_t code)
{
    /* Provisional monitor LUT only; not the final analog NTSC color model. */
    static const rgb8_t demo[16] = {
        {84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136},
        {68, 0, 100}, {92, 0, 48}, {84, 4, 0}, {60, 24, 0},
        {32, 42, 0}, {8, 58, 0}, {0, 64, 0}, {0, 60, 0},
        {0, 50, 60}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
    };
    return demo[code & 0x0fu];
}
