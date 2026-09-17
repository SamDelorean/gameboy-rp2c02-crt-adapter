#include "rp2c02_frame.h"
#include "rp2c02_timing.h"

#include <string.h>

void rp2c02_render_ext_frame(
    const rp2c02_ext_t *ppu,
    const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
    rgb8_t out_rgb[PPU_ACTIVE_H][PPU_ACTIVE_W],
    rp2c02_frame_stats_t *stats)
{
    rp2c02_timing_t timing;
    rp2c02_timing_reset(&timing);

    rp2c02_frame_stats_t local_stats;
    memset(&local_stats, 0, sizeof(local_stats));

    for (unsigned i = 0; i < RP2C02_FRAME_DOTS; ++i) {
        const unsigned dot = timing.dot;
        const unsigned scanline = timing.scanline;
        const unsigned events = rp2c02_timing_step(&timing);

        local_stats.total_dots++;

        if (events & RP2C02_TIMING_VISIBLE_DOT) {
            const uint8_t code = rp2c02_ext_palette_code(ppu, ext[scanline][dot]);
            out_rgb[scanline][dot] = rp2c02_demo_rgb(code);
            local_stats.visible_dots++;
        }
        if (events & RP2C02_TIMING_VBLANK_START) local_stats.vblank_starts++;
        if (events & RP2C02_TIMING_VBLANK_END) local_stats.vblank_ends++;
        if (events & RP2C02_TIMING_FRAME_WRAP) local_stats.frame_wraps++;
    }

    if (stats) *stats = local_stats;
}
