#ifndef GBCRT_COMPARISON_RENDER_H
#define GBCRT_COMPARISON_RENDER_H

#include "clock_mode.h"
#include "gb_source.h"
#include "rp2c02_ext.h"

#include <stdint.h>

#define COMPARISON_W 1280
#define COMPARISON_H 720

typedef struct {
    gbcrt_clock_mode_t clock_mode;
    int menu_open;
    int menu_selection;
    int paused;
    const char *source_name;
} comparison_view_state_t;

void comparison_render(rgb8_t *canvas,
                       const gb_source_frame_t *frame,
                       const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
                       const rp2c02_ext_t *ppu,
                       const comparison_view_state_t *state);

#endif
