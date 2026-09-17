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
    assert(nearly_equal(gbcrt_gb_clock_hz(GBCRT_CLOCK_STOCK,
                                          GBCRT_SOURCE_MODEL_DMG),
                        4194304.0, 0.001));
    assert(nearly_equal(gbcrt_gb_frame_hz(GBCRT_CLOCK_STOCK,
                                          GBCRT_SOURCE_MODEL_DMG),
                        59.7275005696, 0.000001));

    assert(nearly_equal(gbcrt_gb_clock_hz(GBCRT_CLOCK_STOCK,
                                          GBCRT_SOURCE_MODEL_SGB),
                        4295454.4, 0.001));
    assert(nearly_equal(gbcrt_gb_frame_hz(GBCRT_CLOCK_STOCK,
                                          GBCRT_SOURCE_MODEL_SGB),
                        61.1678970153, 0.000001));

    assert(nearly_equal(gbcrt_ppu_frame_hz(), 60.0984775561, 0.000001));
    assert(nearly_equal(gbcrt_gb_clock_hz(GBCRT_CLOCK_SYNC,
                                          GBCRT_SOURCE_MODEL_DMG),
                        4220355.4879, 0.001));
    assert(nearly_equal(gbcrt_gb_clock_hz(GBCRT_CLOCK_SYNC,
                                          GBCRT_SOURCE_MODEL_SGB),
                        4220355.4879, 0.001));
    assert(nearly_equal(gbcrt_source_frames_per_ppu_frame(
                            GBCRT_CLOCK_SYNC, GBCRT_SOURCE_MODEL_DMG),
                        1.0, 0.000000001));
    assert(nearly_equal(gbcrt_source_frames_per_ppu_frame(
                            GBCRT_CLOCK_SYNC, GBCRT_SOURCE_MODEL_SGB),
                        1.0, 0.000000001));

    assert(nearly_equal(gbcrt_stock_frame_slip_seconds(GBCRT_SOURCE_MODEL_DMG),
                        2.695585, 0.00001));
    assert(nearly_equal(gbcrt_stock_frame_slip_seconds(GBCRT_SOURCE_MODEL_SGB),
                        0.935087, 0.00001));

    gbcrt_clock_scheduler_t sync;
    gbcrt_clock_scheduler_init(&sync,
                               GBCRT_CLOCK_SYNC,
                               GBCRT_SOURCE_MODEL_DMG);
    for (unsigned i = 0; i < 1000; ++i) {
        assert(gbcrt_clock_scheduler_step(&sync) == 1u);
    }
    assert(sync.output_frames == 1000u);
    assert(sync.source_frames == 1000u);
    assert(sync.repeated_output_frames == 0u);
    assert(sync.skipped_source_frames == 0u);

    gbcrt_clock_scheduler_t dmg_stock;
    gbcrt_clock_scheduler_init(&dmg_stock,
                               GBCRT_CLOCK_STOCK,
                               GBCRT_SOURCE_MODEL_DMG);
    unsigned dmg_advanced = 0;
    for (unsigned i = 0; i < 1000; ++i) {
        dmg_advanced += gbcrt_clock_scheduler_step(&dmg_stock);
    }
    assert(dmg_advanced == 993u);
    assert(dmg_stock.source_frames == 993u);
    assert(dmg_stock.repeated_output_frames == 7u);
    assert(dmg_stock.skipped_source_frames == 0u);

    gbcrt_clock_scheduler_t sgb_stock;
    gbcrt_clock_scheduler_init(&sgb_stock,
                               GBCRT_CLOCK_STOCK,
                               GBCRT_SOURCE_MODEL_SGB);
    unsigned sgb_advanced = 0;
    unsigned two_frame_steps = 0;
    for (unsigned i = 0; i < 1000; ++i) {
        const unsigned advances = gbcrt_clock_scheduler_step(&sgb_stock);
        sgb_advanced += advances;
        if (advances == 2u) two_frame_steps++;
        else assert(advances == 1u);
    }
    assert(sgb_advanced == 1017u);
    assert(sgb_stock.source_frames == 1017u);
    assert(sgb_stock.repeated_output_frames == 0u);
    assert(sgb_stock.skipped_source_frames == 17u);
    assert(two_frame_steps == 17u);

    gbcrt_clock_scheduler_set_mode(&dmg_stock, GBCRT_CLOCK_SYNC);
    for (unsigned i = 0; i < 10; ++i) {
        assert(gbcrt_clock_scheduler_step(&dmg_stock) == 1u);
    }

    puts("clock modes OK: DMG stock repeats; SGB stock skips; sync is 1:1");
    return 0;
}
