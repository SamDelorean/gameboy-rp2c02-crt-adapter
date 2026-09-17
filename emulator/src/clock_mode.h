#ifndef GBCRT_CLOCK_MODE_H
#define GBCRT_CLOCK_MODE_H

#include <stdbool.h>

typedef enum {
    GBCRT_CLOCK_STOCK = 0,
    GBCRT_CLOCK_SYNC = 1,
} gbcrt_clock_mode_t;

typedef struct {
    gbcrt_clock_mode_t mode;
    double source_phase;
    unsigned long long output_frames;
    unsigned long long source_frames;
    unsigned long long repeated_output_frames;
} gbcrt_clock_scheduler_t;

const char *gbcrt_clock_mode_name(gbcrt_clock_mode_t mode);
double gbcrt_gb_clock_hz(gbcrt_clock_mode_t mode);
double gbcrt_gb_frame_hz(gbcrt_clock_mode_t mode);
double gbcrt_ppu_frame_hz(void);
double gbcrt_source_frames_per_ppu_frame(gbcrt_clock_mode_t mode);
double gbcrt_stock_frame_slip_seconds(void);

void gbcrt_clock_scheduler_init(gbcrt_clock_scheduler_t *scheduler,
                                gbcrt_clock_mode_t mode);
void gbcrt_clock_scheduler_set_mode(gbcrt_clock_scheduler_t *scheduler,
                                    gbcrt_clock_mode_t mode);
/*
 * Call once per RP2C02 output frame. Returns true when a new Game Boy source
 * frame should be generated. In stock-clock mode this naturally repeats an
 * occasional source frame; in synchronized mode it advances 1:1.
 */
bool gbcrt_clock_scheduler_step(gbcrt_clock_scheduler_t *scheduler);

#endif
