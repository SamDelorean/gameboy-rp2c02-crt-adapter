#ifndef GBCRT_ALT_VIDEO_H
#define GBCRT_ALT_VIDEO_H

#include "bridge.h"
#include "rp2c02_ext.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Logical alternate-video state shared by the standalone regression bench and
 * the future SameBoy SDL extension.  This is not an RP2350/Arduino emulator.
 */
typedef struct {
    rp2c02_ext_t ppu;
    unsigned palette_mode; /* 0 = AUTO/SGB, 1..N = manual presets */
    uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W];
} gbcrt_alt_video_t;

void gbcrt_alt_video_init(gbcrt_alt_video_t *state);

unsigned gbcrt_alt_video_palette_mode(const gbcrt_alt_video_t *state);
void gbcrt_alt_video_set_palette_mode(gbcrt_alt_video_t *state, unsigned mode);
void gbcrt_alt_video_next_palette(gbcrt_alt_video_t *state);

/*
 * Render one logical Game Boy frame to the RP2C02 visible 256x240 region.
 *
 * Inputs:
 *   shade[y][x]          Game Boy four-shade image, values 0..3.
 *   sgb_rgb555[4]        optional already-decoded SGB palette from SameBoy.
 *   sgb_palette_valid    true when sgb_rgb555 is valid.
 *
 * Output:
 *   out_rgb[y][x]        desktop RGB visualization of RP2C02 color selection.
 *
 * AUTO/SGB uses sgb_rgb555 when available and otherwise falls back to manual
 * preset 0. Manual modes ignore SGB palette state.
 */
void gbcrt_alt_video_render(gbcrt_alt_video_t *state,
                            const uint8_t shade[GB_H][GB_W],
                            const uint16_t sgb_rgb555[4],
                            bool sgb_palette_valid,
                            rgb8_t out_rgb[PPU_ACTIVE_H][PPU_ACTIVE_W]);

#endif
