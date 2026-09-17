#ifndef GBCRT_RP2C02_EXT_H
#define GBCRT_RP2C02_EXT_H

#include <stdint.h>

typedef struct {
    uint8_t palette_ram[32];
} rp2c02_ext_t;

typedef struct {
    uint8_t r, g, b;
} rgb8_t;

void rp2c02_ext_reset(rp2c02_ext_t *ppu);
void rp2c02_ext_write_palette(rp2c02_ext_t *ppu, unsigned index, uint8_t nes_color_code);
uint8_t rp2c02_ext_palette_code(const rp2c02_ext_t *ppu, uint8_t ext_nibble);
rgb8_t rp2c02_demo_rgb(uint8_t nes_color_code);

#endif
