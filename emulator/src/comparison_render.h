#ifndef GBCRT_COMPARISON_RENDER_H
#define GBCRT_COMPARISON_RENDER_H

#include "clock_mode.h"
#include "gb_source.h"
#include "rp2c02_ext.h"

#include <stdint.h>

#define COMPARISON_W 1280
#define COMPARISON_H 720

/* Logical-canvas hitbox for the one-button palette control. */
#define COMPARISON_PALETTE_BUTTON_X 900
#define COMPARISON_PALETTE_BUTTON_Y 8
#define COMPARISON_PALETTE_BUTTON_W 190
#define COMPARISON_PALETTE_BUTTON_H 32

typedef struct {
    gbcrt_clock_mode_t clock_mode;
    gbcrt_source_model_t source_model;
    int menu_open;
    int menu_selection;
    int paused;
    const char *source_name;
    const char *palette_name;
} comparison_view_state_t;

int comparison_palette_button_contains(unsigned x, unsigned y);

void comparison_render(rgb8_t *canvas,
                       const gb_source_frame_t *frame,
                       const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
                       const rp2c02_ext_t *ppu,
                       const comparison_view_state_t *state);

#endif
