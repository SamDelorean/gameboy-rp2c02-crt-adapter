#ifndef GBCRT_RP2C02_EXT_H
#define GBCRT_RP2C02_EXT_H

#include <stdbool.h>
#include <stdint.h>

#define RP2C02_REG_CTRL 0u
#define RP2C02_REG_MASK 1u
#define RP2C02_REG_ADDR 6u
#define RP2C02_REG_DATA 7u

#define RP2C02_CTRL_INCREMENT_32 0x04u
#define RP2C02_CTRL_EXT_OUTPUT   0x40u
#define RP2C02_MASK_GREYSCALE    0x01u
#define RP2C02_MASK_RENDER_BG    0x08u
#define RP2C02_MASK_RENDER_SPR   0x10u

typedef struct {
    uint8_t palette_ram[32];
    uint8_t ctrl;
    uint8_t mask;
    uint16_t vram_address;
    bool ppuaddr_high_next;
} rp2c02_ext_t;

typedef struct {
    uint8_t r, g, b;
} rgb8_t;

void rp2c02_ext_reset(rp2c02_ext_t *ppu);

/* Direct logical palette helper. Indices $10/$14/$18/$1C mirror the
 * corresponding background entries just like RP2C02 palette RAM. */
void rp2c02_ext_write_palette(rp2c02_ext_t *ppu,
                              unsigned index,
                              uint8_t nes_color_code);
uint8_t rp2c02_ext_palette_code(const rp2c02_ext_t *ppu,
                                uint8_t ext_nibble);

/* Minimal write-only CPU/register interface used by the hardware project.
 * Only $2000, $2001, $2006 and $2007 are modeled. External VRAM is intentionally
 * absent; $2007 writes matter here only when v points into internal palette RAM. */
void rp2c02_ext_cpu_write(rp2c02_ext_t *ppu, uint8_t reg, uint8_t value);

/* Map any address in $3F00-$3FFF to the physical 32-byte palette storage,
 * including $3F10/$14/$18/$1C mirrors. Caller must pass a palette address. */
unsigned rp2c02_ext_palette_index_for_address(uint16_t address);

bool rp2c02_ext_rendering_enabled(const rp2c02_ext_t *ppu);
bool rp2c02_ext_palette_override_active(const rp2c02_ext_t *ppu);

/* Composite color-code selection for the project's rendering-disabled EXT-input
 * mode. If v points into palette RAM, the addressed palette entry overrides EXT.
 * PPUCTRL bit 6 set is modeled defensively as EXT input forced to zero. */
uint8_t rp2c02_ext_rendering_disabled_code(const rp2c02_ext_t *ppu,
                                            uint8_t ext_nibble);

rgb8_t rp2c02_demo_rgb(uint8_t nes_color_code);

#endif
