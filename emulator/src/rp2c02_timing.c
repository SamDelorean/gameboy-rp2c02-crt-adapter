#include "rp2c02_timing.h"

void rp2c02_timing_reset(rp2c02_timing_t *timing)
{
    timing->dot = 0;
    timing->scanline = 0;
    timing->frame = 0;
    timing->vblank = false;
}

unsigned rp2c02_timing_step(rp2c02_timing_t *timing)
{
    unsigned events = RP2C02_TIMING_NONE;

    if (timing->scanline < RP2C02_VISIBLE_SCANLINES &&
        timing->dot < RP2C02_VISIBLE_DOTS) {
        events |= RP2C02_TIMING_VISIBLE_DOT;
    }

    if (timing->scanline == RP2C02_VBLANK_START_SCANLINE &&
        timing->dot == 1u) {
        timing->vblank = true;
        events |= RP2C02_TIMING_VBLANK_START;
    }

    if (timing->scanline == RP2C02_PRE_RENDER_SCANLINE &&
        timing->dot == 1u) {
        timing->vblank = false;
        events |= RP2C02_TIMING_VBLANK_END;
    }

    timing->dot++;
    if (timing->dot == RP2C02_DOTS_PER_SCANLINE) {
        timing->dot = 0;
        timing->scanline++;

        if (timing->scanline == RP2C02_SCANLINES_PER_FRAME) {
            timing->scanline = 0;
            timing->frame++;
            events |= RP2C02_TIMING_FRAME_WRAP;
        }
    }

    return events;
}

bool rp2c02_timing_nmi_asserted(const rp2c02_timing_t *timing,
                                 bool nmi_enabled)
{
    return timing && nmi_enabled && timing->vblank;
}
