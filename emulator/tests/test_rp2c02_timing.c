#include "rp2c02_timing.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    rp2c02_timing_t timing;
    rp2c02_timing_reset(&timing);

    unsigned visible = 0;
    unsigned vblank_start = 0;
    unsigned vblank_end = 0;
    unsigned frame_wrap = 0;

    for (unsigned i = 0; i < RP2C02_FRAME_DOTS; ++i) {
        const unsigned events = rp2c02_timing_step(&timing);
        if (events & RP2C02_TIMING_VISIBLE_DOT) visible++;
        if (events & RP2C02_TIMING_VBLANK_START) vblank_start++;
        if (events & RP2C02_TIMING_VBLANK_END) vblank_end++;
        if (events & RP2C02_TIMING_FRAME_WRAP) frame_wrap++;
    }

    assert(visible == RP2C02_VISIBLE_DOTS * RP2C02_VISIBLE_SCANLINES);
    assert(vblank_start == 1u);
    assert(vblank_end == 1u);
    assert(frame_wrap == 1u);

    assert(timing.dot == 0u);
    assert(timing.scanline == 0u);
    assert(timing.frame == 1u);
    assert(timing.vblank == false);

    puts("RP2C02 timing OK: 341x262, 89342 dots/frame, rendering-disabled timing");
    return 0;
}
