#ifndef GBCRT_ADAPTER_PALETTE_H
#define GBCRT_ADAPTER_PALETTE_H

#include "rp2c02_ext.h"

#include <stdint.h>

typedef struct {
    const char *name;
    uint8_t shade_code[4];
} adapter_palette_preset_t;

/* Candidate manual catalog currently contains 16 entries; AUTO/SGB is separate. */
unsigned adapter_palette_count(void);
const adapter_palette_preset_t *adapter_palette_get(unsigned index);

/*
 * Apply one virtual-bench preset to RP2C02 palette RAM. Game Boy shades 0..3
 * use EXT indices 0..3. The dedicated V1 border EXT index is always loaded
 * with canonical RP2C02 black ($0F).
 */
void adapter_palette_apply(rp2c02_ext_t *ppu, unsigned index);

/* Apply an arbitrary four-shade mapping, used by the virtual-bench editor. */
void adapter_palette_apply_codes(rp2c02_ext_t *ppu,
                                 const uint8_t shade_code[4]);

/*
 * Provisional virtual-bench conversion for SGB RGB555 colors. It compares the
 * expanded RGB value against the current 64-entry RP2C02 monitor LUT and
 * deliberately excludes $0D. This is not the final perceptual/measured
 * hardware quantizer.
 */
uint8_t adapter_palette_quantize_rgb555(uint16_t rgb555);
void adapter_palette_apply_sgb_rgb555(rp2c02_ext_t *ppu,
                                      const uint16_t rgb555[4]);

#endif
