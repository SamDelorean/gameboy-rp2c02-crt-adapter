#ifndef GBCRT_RP2C02_FRAME_H
#define GBCRT_RP2C02_FRAME_H

#include "bridge.h"
#include "rp2c02_ext.h"

#include <stdint.h>

typedef struct {
    uint32_t total_dots;
    uint32_t visible_dots;
    uint32_t vblank_starts;
    uint32_t vblank_ends;
    uint32_t frame_wraps;
} rp2c02_frame_stats_t;

/*
 * Render one complete simplified NTSC RP2C02 frame by stepping the reduced
 * 341x262 timing model. Normal NES background/sprite rendering is assumed
 * disabled. During the visible 256x240 region, EXT0..EXT3 select palette RAM
 * entries and therefore the monitor RGB color used for this virtual bench.
 *
 * out_rgb contains only the active 256x240 picture. Blanking/sync waveform
 * generation remains a later model layer.
 */
void rp2c02_render_ext_frame(
    const rp2c02_ext_t *ppu,
    const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
    rgb8_t out_rgb[PPU_ACTIVE_H][PPU_ACTIVE_W],
    rp2c02_frame_stats_t *stats);

#endif
