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

/* Palette editor occupies the left comparison panel so RP2C02 preview remains visible. */
#define COMPARISON_EDITOR_X 48
#define COMPARISON_EDITOR_Y 78
#define COMPARISON_EDITOR_W 554
#define COMPARISON_EDITOR_H 592
#define COMPARISON_EDITOR_SHADE_X 72
#define COMPARISON_EDITOR_SHADE_Y 190
#define COMPARISON_EDITOR_SHADE_W 118
#define COMPARISON_EDITOR_SHADE_H 70
#define COMPARISON_EDITOR_SHADE_GAP 10
#define COMPARISON_EDITOR_GRID_X 74
#define COMPARISON_EDITOR_GRID_Y 318
#define COMPARISON_EDITOR_CELL_W 31
#define COMPARISON_EDITOR_CELL_H 36
#define COMPARISON_EDITOR_BUTTON_Y 505
#define COMPARISON_EDITOR_BUTTON_W 92
#define COMPARISON_EDITOR_BUTTON_H 34
#define COMPARISON_EDITOR_BUTTON_GAP 10

typedef enum {
    COMPARISON_EDITOR_ACTION_NONE = 0,
    COMPARISON_EDITOR_ACTION_REVERSE,
    COMPARISON_EDITOR_ACTION_RESET,
    COMPARISON_EDITOR_ACTION_PREV,
    COMPARISON_EDITOR_ACTION_NEXT,
    COMPARISON_EDITOR_ACTION_DONE,
} comparison_palette_editor_action_t;

typedef struct {
    gbcrt_clock_mode_t clock_mode;
    gbcrt_source_model_t source_model;
    int menu_open;
    int menu_selection;
    int paused;
    int palette_editor_open;
    unsigned palette_editor_selected_shade;
    uint8_t palette_editor_codes[4];
    const char *source_name;
    const char *palette_name;
} comparison_view_state_t;

int comparison_palette_button_contains(unsigned x, unsigned y);
int comparison_palette_editor_shade_at(unsigned x, unsigned y, unsigned *shade);
int comparison_palette_editor_code_at(unsigned x, unsigned y, uint8_t *code);
comparison_palette_editor_action_t comparison_palette_editor_action_at(unsigned x, unsigned y);

void comparison_render(rgb8_t *canvas,
                       const gb_source_frame_t *frame,
                       const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
                       const rp2c02_ext_t *ppu,
                       const comparison_view_state_t *state);

#endif
