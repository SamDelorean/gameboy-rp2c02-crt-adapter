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
    unsigned nmi_asserted_samples = 0;

    assert(!rp2c02_timing_nmi_asserted(&timing, false));
    assert(!rp2c02_timing_nmi_asserted(&timing, true));

    for (unsigned i = 0; i < RP2C02_FRAME_DOTS; ++i) {
        const unsigned dot_before = timing.dot;
        const unsigned scanline_before = timing.scanline;
        const unsigned events = rp2c02_timing_step(&timing);

        if (events & RP2C02_TIMING_VISIBLE_DOT) visible++;

        if (events & RP2C02_TIMING_VBLANK_START) {
            vblank_start++;
            assert(scanline_before == RP2C02_VBLANK_START_SCANLINE);
            assert(dot_before == 1u);
            assert(timing.vblank);
            assert(rp2c02_timing_nmi_asserted(&timing, true));
            assert(!rp2c02_timing_nmi_asserted(&timing, false));
        }

        if (events & RP2C02_TIMING_VBLANK_END) {
            vblank_end++;
            assert(scanline_before == RP2C02_PRE_RENDER_SCANLINE);
            assert(dot_before == 1u);
            assert(!timing.vblank);
            assert(!rp2c02_timing_nmi_asserted(&timing, true));
        }

        if (events & RP2C02_TIMING_FRAME_WRAP) frame_wrap++;
        if (rp2c02_timing_nmi_asserted(&timing, true)) nmi_asserted_samples++;
    }

    assert(visible == RP2C02_VISIBLE_DOTS * RP2C02_VISIBLE_SCANLINES);
    assert(vblank_start == 1u);
    assert(vblank_end == 1u);
    assert(frame_wrap == 1u);
    assert(nmi_asserted_samples > 0u);

    assert(timing.dot == 0u);
    assert(timing.scanline == 0u);
    assert(timing.frame == 1u);
    assert(timing.vblank == false);
    assert(!rp2c02_timing_nmi_asserted(&timing, true));

    puts("RP2C02 timing OK: 341x262, exact VBlank boundaries, and NMI assertion window");
    return 0;
}
