#ifndef GBCRT_RP2C02_NESEMU_H
#define GBCRT_RP2C02_NESEMU_H

#include "bridge.h"
#include "rp2c02_ext.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Render one complete RP2C02 visible region with johnmph/NESEmu's Ricoh2C02
 * core. The project supplies only the EXT nibble stream and the small PPU
 * register/palette state already maintained by rp2c02_ext_t.
 *
 * out_code receives the PPU's native six-bit color codes (0x00..0x3f).
 * Conversion to desktop RGB remains a presentation concern.
 */
int rp2c02_nesemu_render(
    const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
    const rp2c02_ext_t *state,
    uint8_t out_code[PPU_ACTIVE_H][PPU_ACTIVE_W]);

const char *rp2c02_nesemu_revision(void);

#ifdef __cplusplus
}
#endif

#endif
