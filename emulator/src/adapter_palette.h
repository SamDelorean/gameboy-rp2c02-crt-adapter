#ifndef GBCRT_ADAPTER_PALETTE_H
#define GBCRT_ADAPTER_PALETTE_H

#include "rp2c02_ext.h"

#include <stdint.h>

typedef struct {
    const char *name;
    uint8_t shade_code[4];
} adapter_palette_preset_t;

unsigned adapter_palette_count(void);
const adapter_palette_preset_t *adapter_palette_get(unsigned index);

/*
 * Apply one virtual-bench preset to RP2C02 palette RAM.  Game Boy shades
 * 0..3 use EXT indices 0..3.  The dedicated V1 border EXT index is always
 * loaded with canonical RP2C02 black ($0F).
 */
void adapter_palette_apply(rp2c02_ext_t *ppu, unsigned index);

#endif
