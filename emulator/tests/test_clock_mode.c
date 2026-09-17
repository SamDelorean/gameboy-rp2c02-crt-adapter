#include "clock_mode.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static int nearly_equal(double a, double b, double tol)
{
    return fabs(a - b) <= tol;
}

int main(void)
{
    assert(nearly_equal(gbcrt_gb_clock_hz(GBCRT_CLOCK_STOCK), 4194304.0, 0.001));
    assert(nearly_equal(gbcrt_gb_frame_hz(GBCRT_CLOCK_STOCK),
                        59.7275005696, 0.000001));
    assert(nearly_equal(gbcrt_ppu_frame_hz(), 60.0984775561, 0.000001));
    assert(nearly_equal(gbcrt_gb_clock_hz(GBCRT_CLOCK_SYNC),
                        4220355.4879, 0.001));
    assert(nearly_equal(gbcrt_source_frames_per_ppu_frame(GBCRT_CLOCK_SYNC),
                        1.0, 0.000000001));
    assert(nearly_equal(gbcrt_stock_frame_slip_seconds(),
                        2.695585, 0.00001));

    gbcrt_clock_scheduler_t sync;
    gbcrt_clock_scheduler_init(&sync, GBCRT_CLOCK_SYNC);
    for (unsigned i = 0; i < 1000; ++i) {
        assert(gbcrt_clock_scheduler_step(&sync));
    }
    assert(sync.output_frames == 1000u);
    assert(sync.source_frames == 1000u);
    assert(sync.repeated_output_frames == 0u);

    gbcrt_clock_scheduler_t stock;
    gbcrt_clock_scheduler_init(&stock, GBCRT_CLOCK_STOCK);
    unsigned advances = 0;
    unsigned repeats = 0;
    for (unsigned i = 0; i < 1000; ++i) {
        if (gbcrt_clock_scheduler_step(&stock)) advances++;
        else repeats++;
    }

    assert(advances == stock.source_frames);
    assert(repeats == stock.repeated_output_frames);
    assert(advances + repeats == 1000u);
    assert(repeats >= 6u && repeats <= 7u);

    gbcrt_clock_scheduler_set_mode(&stock, GBCRT_CLOCK_SYNC);
    for (unsigned i = 0; i < 10; ++i) {
        assert(gbcrt_clock_scheduler_step(&stock));
    }

    puts("clock modes OK: stock drift is visible; synchronized mode is 1:1");
    return 0;
}
