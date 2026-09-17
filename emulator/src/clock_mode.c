#include "clock_mode.h"

#include <math.h>

#define GB_CLOCK_STOCK_HZ 4194304.0
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

double gbcrt_gb_clock_hz(gbcrt_clock_mode_t mode)
{
    if (mode == GBCRT_CLOCK_STOCK) {
        return GB_CLOCK_STOCK_HZ;
    }
    return gbcrt_ppu_frame_hz() * GB_CLOCKS_PER_FRAME;
}

double gbcrt_gb_frame_hz(gbcrt_clock_mode_t mode)
{
    return gbcrt_gb_clock_hz(mode) / GB_CLOCKS_PER_FRAME;
}

double gbcrt_source_frames_per_ppu_frame(gbcrt_clock_mode_t mode)
{
    return gbcrt_gb_frame_hz(mode) / gbcrt_ppu_frame_hz();
}

double gbcrt_stock_frame_slip_seconds(void)
{
    const double delta = fabs(gbcrt_gb_frame_hz(GBCRT_CLOCK_STOCK) -
                              gbcrt_ppu_frame_hz());
    return delta > 0.0 ? 1.0 / delta : 0.0;
}

void gbcrt_clock_scheduler_init(gbcrt_clock_scheduler_t *scheduler,
                                gbcrt_clock_mode_t mode)
{
    scheduler->mode = mode;
    scheduler->source_phase = 0.0;
    scheduler->output_frames = 0;
    scheduler->source_frames = 0;
    scheduler->repeated_output_frames = 0;
}

void gbcrt_clock_scheduler_set_mode(gbcrt_clock_scheduler_t *scheduler,
                                    gbcrt_clock_mode_t mode)
{
    scheduler->mode = mode;
    scheduler->source_phase = 0.0;
}

bool gbcrt_clock_scheduler_step(gbcrt_clock_scheduler_t *scheduler)
{
    scheduler->output_frames++;
    scheduler->source_phase += gbcrt_source_frames_per_ppu_frame(scheduler->mode);

    if (scheduler->source_phase >= 1.0) {
        scheduler->source_phase -= 1.0;
        scheduler->source_frames++;
        return true;
    }

    scheduler->repeated_output_frames++;
    return false;
}
