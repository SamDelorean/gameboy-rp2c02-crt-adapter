#ifndef GBCRT_CLOCK_MODE_H
#define GBCRT_CLOCK_MODE_H

#include "source_model.h"

#include <stdbool.h>

typedef enum {
    GBCRT_CLOCK_STOCK = 0,
    GBCRT_CLOCK_SYNC = 1,
} gbcrt_clock_mode_t;

typedef struct {
    gbcrt_clock_mode_t mode;
    gbcrt_source_model_t source_model;
    double source_phase;
    unsigned long long output_frames;
    unsigned long long source_frames;
    unsigned long long repeated_output_frames;
    unsigned long long skipped_source_frames;
} gbcrt_clock_scheduler_t;

const char *gbcrt_clock_mode_name(gbcrt_clock_mode_t mode);
double gbcrt_gb_clock_hz(gbcrt_clock_mode_t mode,
                         gbcrt_source_model_t source_model);
double gbcrt_gb_frame_hz(gbcrt_clock_mode_t mode,
                         gbcrt_source_model_t source_model);
double gbcrt_ppu_frame_hz(void);
double gbcrt_source_frames_per_ppu_frame(gbcrt_clock_mode_t mode,
                                         gbcrt_source_model_t source_model);
double gbcrt_stock_frame_slip_seconds(gbcrt_source_model_t source_model);

void gbcrt_clock_scheduler_init(gbcrt_clock_scheduler_t *scheduler,
                                gbcrt_clock_mode_t mode,
                                gbcrt_source_model_t source_model);
void gbcrt_clock_scheduler_set_mode(gbcrt_clock_scheduler_t *scheduler,
                                    gbcrt_clock_mode_t mode);

/*
 * Call once per RP2C02 output frame. The return value is the number of source
 * frames that should be consumed before presenting this output frame:
 *
 *   0 -> repeat the previous Game Boy frame in STOCK mode
 *   1 -> normal one-to-one advance
 *
 * SYNC always returns 1. SGB does not have a separate SNES-derived timing
 * model here: the project assumes its original clock path has already been
 * isolated and the source is driven by either STOCK Game Boy clock or the
 * project SYNC clock, exactly like DMG at the system boundary.
 */
unsigned gbcrt_clock_scheduler_step(gbcrt_clock_scheduler_t *scheduler);

#endif
