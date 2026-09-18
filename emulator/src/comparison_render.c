#include "comparison_render.h"
#include "rp2c02_timing.h"
#ifdef GBCRT_ENABLE_NESEMU_PPU
#include "rp2c02_nesemu.h"
#endif
#include "ui_font.h"

#include <stddef.h>
#include <stdio.h>

static void fill(rgb8_t *canvas, rgb8_t c)
{
    for (size_t i = 0; i < (size_t)COMPARISON_W * COMPARISON_H; ++i) {
        canvas[i] = c;
    }
}

static void rect(rgb8_t *canvas,
                 unsigned x0,
                 unsigned y0,
                 unsigned w,
                 unsigned h,
                 rgb8_t c)
{
    for (unsigned y = y0; y < y0 + h && y < COMPARISON_H; ++y) {
        for (unsigned x = x0; x < x0 + w && x < COMPARISON_W; ++x) {
            canvas[(size_t)y * COMPARISON_W + x] = c;
        }
    }
}

static void frame_rect(rgb8_t *canvas,
                       unsigned x,
                       unsigned y,
                       unsigned w,
                       unsigned h,
                       unsigned thickness,
                       rgb8_t c)
{
    rect(canvas, x, y, w, thickness, c);
    rect(canvas, x, y + h - thickness, w, thickness, c);
    rect(canvas, x, y, thickness, h, c);
    rect(canvas, x + w - thickness, y, thickness, h, c);
}

static rgb8_t rgb_from_rgba(uint32_t rgba)
{
    return (rgb8_t){
        (uint8_t)(rgba & 0xffu),
        (uint8_t)((rgba >> 8) & 0xffu),
        (uint8_t)((rgba >> 16) & 0xffu),
    };
}

int comparison_palette_button_contains(unsigned x, unsigned y)
{
    return x >= COMPARISON_PALETTE_BUTTON_X &&
           x < COMPARISON_PALETTE_BUTTON_X + COMPARISON_PALETTE_BUTTON_W &&
           y >= COMPARISON_PALETTE_BUTTON_Y &&
           y < COMPARISON_PALETTE_BUTTON_Y + COMPARISON_PALETTE_BUTTON_H;
}

int comparison_palette_editor_shade_at(unsigned x, unsigned y, unsigned *shade)
{
    if (y < COMPARISON_EDITOR_SHADE_Y ||
        y >= COMPARISON_EDITOR_SHADE_Y + COMPARISON_EDITOR_SHADE_H) {
        return 0;
    }

    for (unsigned i = 0; i < 4u; ++i) {
        const unsigned x0 = COMPARISON_EDITOR_SHADE_X +
            i * (COMPARISON_EDITOR_SHADE_W + COMPARISON_EDITOR_SHADE_GAP);
        if (x >= x0 && x < x0 + COMPARISON_EDITOR_SHADE_W) {
            if (shade) *shade = i;
            return 1;
        }
    }
    return 0;
}

int comparison_palette_editor_code_at(unsigned x, unsigned y, uint8_t *code)
{
    const unsigned grid_w = 16u * COMPARISON_EDITOR_CELL_W;
    const unsigned grid_h = 4u * COMPARISON_EDITOR_CELL_H;
    if (x < COMPARISON_EDITOR_GRID_X || x >= COMPARISON_EDITOR_GRID_X + grid_w ||
        y < COMPARISON_EDITOR_GRID_Y || y >= COMPARISON_EDITOR_GRID_Y + grid_h) {
        return 0;
    }

    const unsigned column = (x - COMPARISON_EDITOR_GRID_X) / COMPARISON_EDITOR_CELL_W;
    const unsigned row = (y - COMPARISON_EDITOR_GRID_Y) / COMPARISON_EDITOR_CELL_H;
    if (code) *code = (uint8_t)((row << 4) | column);
    return 1;
}

comparison_palette_editor_action_t comparison_palette_editor_action_at(unsigned x, unsigned y)
{
    if (y < COMPARISON_EDITOR_BUTTON_Y ||
        y >= COMPARISON_EDITOR_BUTTON_Y + COMPARISON_EDITOR_BUTTON_H) {
        return COMPARISON_EDITOR_ACTION_NONE;
    }

    for (unsigned i = 0; i < 5u; ++i) {
        const unsigned x0 = COMPARISON_EDITOR_SHADE_X +
            i * (COMPARISON_EDITOR_BUTTON_W + COMPARISON_EDITOR_BUTTON_GAP);
        if (x >= x0 && x < x0 + COMPARISON_EDITOR_BUTTON_W) {
            return (comparison_palette_editor_action_t)
                (COMPARISON_EDITOR_ACTION_REVERSE + i);
        }
    }
    return COMPARISON_EDITOR_ACTION_NONE;
}

static void draw_menu(rgb8_t *canvas, const comparison_view_state_t *state)
{
    const rgb8_t bar = {30, 30, 34};
    const rgb8_t text = {232, 232, 232};
    const rgb8_t idle = {70, 70, 76};
    const rgb8_t active = {166, 166, 178};
    const rgb8_t accent = {245, 245, 245};

    rect(canvas, 0, 0, COMPARISON_W, 48, bar);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      18, 16, 2, "CLOCK", text);

    rect(canvas, 105, 8, 108, 32,
         state && state->clock_mode == GBCRT_CLOCK_STOCK ? active : idle);
    rect(canvas, 222, 8, 96, 32,
         !state || state->clock_mode == GBCRT_CLOCK_SYNC ? active : idle);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      120, 16, 2, "STOCK", accent);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      239, 16, 2, "SYNC", accent);

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      350, 18, 1,
                      "M MENU   C CLOCK   E EDIT PALETTE   SPACE PAUSE   Q QUIT",
                      text);

    /*
     * Virtual equivalent of the single physical palette pushbutton. It does
     * not model an Arduino/RP2350 or debounce/timing logic; clicking it merely
     * requests the next palette mode, exactly like pressing P.
     */
    rect(canvas,
         COMPARISON_PALETTE_BUTTON_X,
         COMPARISON_PALETTE_BUTTON_Y,
         COMPARISON_PALETTE_BUTTON_W,
         COMPARISON_PALETTE_BUTTON_H,
         active);
    frame_rect(canvas,
               COMPARISON_PALETTE_BUTTON_X,
               COMPARISON_PALETTE_BUTTON_Y,
               COMPARISON_PALETTE_BUTTON_W,
               COMPARISON_PALETTE_BUTTON_H,
               2,
               accent);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      COMPARISON_PALETTE_BUTTON_X + 16,
                      COMPARISON_PALETTE_BUTTON_Y + 9,
                      1,
                      "NEXT PALETTE [P]",
                      accent);

    if (state && state->paused) {
        rect(canvas, 1115, 8, 145, 32, (rgb8_t){90, 60, 60});
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          1141, 16, 2, "PAUSED", accent);
    }

    if (!state || !state->menu_open) return;

    rect(canvas, 16, 48, 340, 118, (rgb8_t){22, 22, 26});
    frame_rect(canvas, 16, 48, 340, 118, 2, (rgb8_t){200, 200, 210});
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      32, 60, 2, "CLOCK MODE", text);

    const int selected = state->menu_selection ? 1 : 0;
    rect(canvas, 30, 88, 310, 28,
         selected == 0 ? active : (rgb8_t){45, 45, 50});
    rect(canvas, 30, 124, 310, 28,
         selected == 1 ? active : (rgb8_t){45, 45, 50});

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      42, 96, 1, "STOCK GB   4.194304 MHZ", accent);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      42, 132, 1, "SYNC       4.220355 MHZ", accent);
}

static void draw_palette_editor(rgb8_t *canvas, const comparison_view_state_t *state)
{
    if (!state || !state->palette_editor_open) return;

    const rgb8_t panel = {22, 22, 26};
    const rgb8_t edge = {205, 205, 210};
    const rgb8_t text = {238, 238, 240};
    const rgb8_t secondary = {180, 180, 188};
    const rgb8_t card = {48, 48, 54};
    const rgb8_t active = {245, 245, 245};
    const rgb8_t warning = {210, 82, 82};

    rect(canvas,
         COMPARISON_EDITOR_X,
         COMPARISON_EDITOR_Y,
         COMPARISON_EDITOR_W,
         COMPARISON_EDITOR_H,
         panel);
    frame_rect(canvas,
               COMPARISON_EDITOR_X,
               COMPARISON_EDITOR_Y,
               COMPARISON_EDITOR_W,
               COMPARISON_EDITOR_H,
               3,
               edge);

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      70, 98, 2, "PALETTE EDITOR", text);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      70, 136, 1,
                      "SELECT A GAME BOY SHADE, THEN CHOOSE AN RP2C02 COLOR CODE",
                      secondary);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      70, 160, 1,
                      "LIGHTEST   SHADE 0 > SHADE 1 > SHADE 2 > SHADE 3   DARKEST",
                      active);

    for (unsigned shade = 0; shade < 4u; ++shade) {
        const unsigned x = COMPARISON_EDITOR_SHADE_X +
            shade * (COMPARISON_EDITOR_SHADE_W + COMPARISON_EDITOR_SHADE_GAP);
        const uint8_t code = state->palette_editor_codes[shade] & 0x3fu;
        char label[24];
        snprintf(label, sizeof(label), "SHADE %u", shade);

        rect(canvas, x, COMPARISON_EDITOR_SHADE_Y,
             COMPARISON_EDITOR_SHADE_W, COMPARISON_EDITOR_SHADE_H, card);
        frame_rect(canvas, x, COMPARISON_EDITOR_SHADE_Y,
                   COMPARISON_EDITOR_SHADE_W, COMPARISON_EDITOR_SHADE_H,
                   shade == state->palette_editor_selected_shade ? 3u : 1u,
                   shade == state->palette_editor_selected_shade ? active : secondary);
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          x + 28, COMPARISON_EDITOR_SHADE_Y + 8, 1, label, text);
        rect(canvas, x + 8, COMPARISON_EDITOR_SHADE_Y + 26,
             COMPARISON_EDITOR_SHADE_W - 16, 25, rp2c02_demo_rgb(code));
        snprintf(label, sizeof(label), "0X%02X", code);
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          x + 44, COMPARISON_EDITOR_SHADE_Y + 56, 1, label, text);
    }

    char selected[64];
    snprintf(selected, sizeof(selected),
             "RP2C02 COLOR TABLE 0X00-0X3F   EDITING SHADE %u",
             state->palette_editor_selected_shade & 3u);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      74, 292, 1, selected, text);

    const uint8_t selected_code =
        state->palette_editor_codes[state->palette_editor_selected_shade & 3u] & 0x3fu;
    for (unsigned row = 0; row < 4u; ++row) {
        for (unsigned column = 0; column < 16u; ++column) {
            const uint8_t code = (uint8_t)((row << 4) | column);
            const unsigned x = COMPARISON_EDITOR_GRID_X +
                column * COMPARISON_EDITOR_CELL_W;
            const unsigned y = COMPARISON_EDITOR_GRID_Y +
                row * COMPARISON_EDITOR_CELL_H;
            char label[4];
            snprintf(label, sizeof(label), "%02X", code);

            rect(canvas, x, y,
                 COMPARISON_EDITOR_CELL_W - 2u,
                 COMPARISON_EDITOR_CELL_H - 2u,
                 card);
            rect(canvas, x + 2, y + 2,
                 COMPARISON_EDITOR_CELL_W - 6u, 18u,
                 rp2c02_demo_rgb(code));
            if (code == selected_code) {
                frame_rect(canvas, x, y,
                           COMPARISON_EDITOR_CELL_W - 2u,
                           COMPARISON_EDITOR_CELL_H - 2u,
                           2u, active);
            }
            if (code == 0x0du) {
                frame_rect(canvas, x, y,
                           COMPARISON_EDITOR_CELL_W - 2u,
                           COMPARISON_EDITOR_CELL_H - 2u,
                           2u, warning);
            }
            ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                              x + 8, y + 24, 1, label, text);
        }
    }

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      74, 470, 1,
                      "0D MARKED: AVOID FOR FINAL NTSC HARDWARE PRESETS",
                      warning);

    static const char *labels[5] = {"REVERSE", "RESET", "PREV", "NEXT", "DONE"};
    for (unsigned i = 0; i < 5u; ++i) {
        const unsigned x = COMPARISON_EDITOR_SHADE_X +
            i * (COMPARISON_EDITOR_BUTTON_W + COMPARISON_EDITOR_BUTTON_GAP);
        rect(canvas, x, COMPARISON_EDITOR_BUTTON_Y,
             COMPARISON_EDITOR_BUTTON_W, COMPARISON_EDITOR_BUTTON_H, card);
        frame_rect(canvas, x, COMPARISON_EDITOR_BUTTON_Y,
                   COMPARISON_EDITOR_BUTTON_W, COMPARISON_EDITOR_BUTTON_H,
                   2u, secondary);
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          x + 12, COMPARISON_EDITOR_BUTTON_Y + 13,
                          1, labels[i], text);
    }

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      74, 558, 1,
                      "E OR ESC: DONE   R: REVERSE   1-4: SELECT SHADE",
                      secondary);
    snprintf(selected, sizeof(selected),
             "CURRENT: S0=%02X  S1=%02X  S2=%02X  S3=%02X",
             state->palette_editor_codes[0],
             state->palette_editor_codes[1],
             state->palette_editor_codes[2],
             state->palette_editor_codes[3]);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      74, 586, 1, selected, text);
}

#define CRT_PPU_ACTIVE_W 256u
#define CRT_PPU_ACTIVE_H 240u
#define CRT_CLEAN_APERTURE_W 280u
#define CRT_SIDE_PAD 12u
#define CRT_PREVIEW_W 576u
#define CRT_PREVIEW_H 432u

static void render_rp2c02_codes(
    const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
    const rp2c02_ext_t *ppu,
    uint8_t code[PPU_ACTIVE_H][PPU_ACTIVE_W])
{
#ifdef GBCRT_ENABLE_NESEMU_PPU
    if (rp2c02_nesemu_render(ext, ppu, code) == 0) {
        return;
    }
#endif

    rp2c02_timing_t timing;
    rp2c02_timing_reset(&timing);

    for (unsigned i = 0; i < RP2C02_FRAME_DOTS; ++i) {
        const unsigned x = timing.dot;
        const unsigned y = timing.scanline;
        const unsigned events = rp2c02_timing_step(&timing);

        if (events & RP2C02_TIMING_VISIBLE_DOT) {
            code[y][x] =
                rp2c02_ext_rendering_disabled_code(ppu, ext[y][x]);
        }
    }
}

static void draw_rp2c02_crt_frame(
    rgb8_t *canvas,
    unsigned rx,
    unsigned ry,
    const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
    const rp2c02_ext_t *ppu)
{
    uint8_t code[PPU_ACTIVE_H][PPU_ACTIVE_W];
    render_rp2c02_codes(ext, ppu, code);

    /*
     * Physical-preview presentation, not a change to PPU geometry:
     *
     * NESdev's NTSC clean-aperture model pads the 256 active PPU samples with
     * 12 backdrop/border samples on each side, yielding 280x240.  Presenting
     * that clean aperture as 4:3 gives 320x240 square-pixel equivalent.
     *
     * Here both comparison displays share a 432-pixel height:
     *   Game Boy reference: 480x432 (160:144)
     *   CRT preview:        576x432 (4:3)
     *
     * The internal adapter raster remains exactly 256x240 and still contains
     * its own 11 + 234 + 11 composition.  Only the desktop preview changes.
     */
    const rgb8_t crt_side_border = {0, 0, 0};

    for (unsigned dy = 0; dy < CRT_PREVIEW_H; ++dy) {
        const unsigned sy = (dy * CRT_PPU_ACTIVE_H) / CRT_PREVIEW_H;

        for (unsigned dx = 0; dx < CRT_PREVIEW_W; ++dx) {
            const unsigned clean_x =
                (dx * CRT_CLEAN_APERTURE_W) / CRT_PREVIEW_W;

            rgb8_t c = crt_side_border;
            if (clean_x >= CRT_SIDE_PAD &&
                clean_x < CRT_SIDE_PAD + CRT_PPU_ACTIVE_W) {
                const unsigned sx = clean_x - CRT_SIDE_PAD;
                c = rp2c02_demo_rgb(code[sy][sx]);
            }

            canvas[(size_t)(ry + dy) * COMPARISON_W + (rx + dx)] = c;
        }
    }
}

void comparison_render(rgb8_t *canvas,
                       const gb_source_frame_t *frame,
                       const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
                       const rp2c02_ext_t *ppu,
                       const comparison_view_state_t *state)
{
    const rgb8_t bg = {222, 222, 226};
    const rgb8_t panel = {38, 38, 42};
    const rgb8_t panel_header = {58, 58, 64};
    const rgb8_t panel_edge = {12, 12, 14};
    const rgb8_t video_edge = {205, 205, 210};
    const rgb8_t text = {238, 238, 240};
    const rgb8_t secondary = {180, 180, 188};

    fill(canvas, bg);
    draw_menu(canvas, state);

    /*
     * Comparison panels deliberately use equal displayed heights. The Game Boy
     * keeps its logical 160:144 shape; the CRT panel uses a 4:3 NTSC clean
     * aperture. Only width differs.
     */
    rect(canvas, 30, 70, 570, 600, panel);
    rect(canvas, 640, 70, 610, 600, panel);
    frame_rect(canvas, 30, 70, 570, 600, 4, panel_edge);
    frame_rect(canvas, 640, 70, 610, 600, 4, panel_edge);

    rect(canvas, 38, 78, 554, 42, panel_header);
    rect(canvas, 648, 78, 594, 42, panel_header);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      58, 92, 2, "GAME BOY REFERENCE", text);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      670, 92, 2, "RP2C02 CRT PREVIEW", text);

    /* Left source region: 160x144 at exact 3x integer scale = 480x432. */
    const unsigned lx = 75;
    const unsigned ly = 150;
    const unsigned lscale = 3;
    rect(canvas, lx - 6, ly - 6, 492, 444, (rgb8_t){8, 8, 8});
    frame_rect(canvas, lx - 6, ly - 6, 492, 444, 2, video_edge);
    for (unsigned y = 0; y < GB_H; ++y) {
        for (unsigned x = 0; x < GB_W; ++x) {
            rect(canvas,
                 lx + x * lscale,
                 ly + y * lscale,
                 lscale,
                 lscale,
                 rgb_from_rgba(frame->reference_rgba[y][x]));
        }
    }

    /*
     * Right physical-preview region: 256x240 PPU active samples are embedded
     * in a 280x240 NTSC clean aperture and displayed as 576x432 (4:3).
     */
    const unsigned rx = 657;
    const unsigned ry = 150;
    rect(canvas, rx - 6, ry - 6,
         CRT_PREVIEW_W + 12u, CRT_PREVIEW_H + 12u, (rgb8_t){8, 8, 8});
    frame_rect(canvas, rx - 6, ry - 6,
               CRT_PREVIEW_W + 12u, CRT_PREVIEW_H + 12u, 2, video_edge);
    draw_rp2c02_crt_frame(canvas, rx, ry, ext, ppu);

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      69, 612, 1, "SOURCE 160X144  DISPLAY 480X432", text);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      654, 612, 1, "CRT 4:3  DISPLAY 576X432", text);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      654, 630, 1, "PPU 256X240  NTSC APERTURE 280X240", secondary);

    if (state && state->source_name) {
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          69, 635, 1, state->source_name, secondary);
    }

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      69, 652, 1,
                      "ARROWS MOVE  Z A  X B  BKSP SELECT  ENTER START",
                      secondary);

    if (state && state->palette_name) {
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          654, 650, 1, "PALETTE", secondary);
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          718, 650, 1, state->palette_name, text);
    }

    draw_palette_editor(canvas, state);
}
