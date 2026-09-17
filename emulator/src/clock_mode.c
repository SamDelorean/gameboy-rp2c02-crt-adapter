#include "clock_mode.h"

#include <math.h>

#define DMG_CLOCK_STOCK_HZ 4194304.0
#define SGB_NTSC_CLOCK_STOCK_HZ (21477272.0 / 5.0)
#define GB_CLOCKS_PER_FRAME 70224.0
#define RP2C02_MASTER_HZ 21477272.727272727
#define RP2C02_MASTER_DIV_TO_DOT 4.0
#define RP2C02_DOTS_PER_FRAME (341.0 * 262.0)

const char *gbcrt_clock_mode_name(gbcrt_clock_mode_t mode)
{
    return mode == GBCRT_CLOCK_STOCK ? "STOCK" : "SYNC";
}

double gbcrt_ppu_frame_hz(void)
{
    const double dot_hz = RP2C02_MASTER_HZ / RP2C02_MASTER_DIV_TO_DOT;
    return dot_hz / RP2C02_DOTS_PER_FRAME;
}

double gbcrt_gb_clock_hz(gbcrt_clock_mode_t mode,
                         gbcrt_source_model_t source_model)
{
    if (mode == GBCRT_CLOCK_SYNC) {
        return gbcrt_ppu_frame_hz() * GB_CLOCKS_PER_FRAME;
    }

    return source_model == GBCRT_SOURCE_MODEL_SGB ?
        SGB_NTSC_CLOCK_STOCK_HZ : DMG_CLOCK_STOCK_HZ;
}

double gbcrt_gb_frame_hz(gbcrt_clock_mode_t mode,
                         gbcrt_source_model_t source_model)
{
    return gbcrt_gb_clock_hz(mode, source_model) / GB_CLOCKS_PER_FRAME;
}

double gbcrt_source_frames_per_ppu_frame(gbcrt_clock_mode_t mode,
                                         gbcrt_source_model_t source_model)
{
    return gbcrt_gb_frame_hz(mode, source_model) / gbcrt_ppu_frame_hz();
}

double gbcrt_stock_frame_slip_seconds(gbcrt_source_model_t source_model)
{
    const double delta = fabs(gbcrt_gb_frame_hz(GBCRT_CLOCK_STOCK, source_model) -
                              gbcrt_ppu_frame_hz());
    return delta > 0.0 ? 1.0 / delta : 0.0;
}

void gbcrt_clock_scheduler_init(gbcrt_clock_scheduler_t *scheduler,
                                gbcrt_clock_mode_t mode,
                                gbcrt_source_model_t source_model)
{
    scheduler->mode = mode;
    scheduler->source_model = source_model;
    scheduler->source_phase = 0.0;
    scheduler->output_frames = 0;
    scheduler->source_frames = 0;
    scheduler->repeated_output_frames = 0;
    scheduler->skipped_source_frames = 0;
}

void gbcrt_clock_scheduler_set_mode(gbcrt_clock_scheduler_t *scheduler,
                                    gbcrt_clock_mode_t mode)
{
    scheduler->mode = mode;
    scheduler->source_phase = 0.0;
}

unsigned gbcrt_clock_scheduler_step(gbcrt_clock_scheduler_t *scheduler)
{
    scheduler->output_frames++;
    scheduler->source_phase +=
        gbcrt_source_frames_per_ppu_frame(scheduler->mode,
                                          scheduler->source_model);

    const unsigned advances = (unsigned)floor(scheduler->source_phase + 1e-12);
    scheduler->source_phase -= advances;
    scheduler->source_frames += advances;

    if (advances == 0u) {
        scheduler->repeated_output_frames++;
    }
    else if (advances > 1u) {
        scheduler->skipped_source_frames += advances - 1u;
    }

    return advances;
}
