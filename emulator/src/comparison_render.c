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
                      "M MENU   C CLOCK   SPACE PAUSE   Q QUIT",
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

static void draw_rp2c02_frame(rgb8_t *canvas,
                              unsigned rx,
                              unsigned ry,
                              unsigned scale,
                              const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
                              const rp2c02_ext_t *ppu)
{
#ifdef GBCRT_ENABLE_NESEMU_PPU
    uint8_t code[PPU_ACTIVE_H][PPU_ACTIVE_W];
    if (rp2c02_nesemu_render(ext, ppu, code) == 0) {
        for (unsigned y = 0; y < PPU_ACTIVE_H; ++y) {
            for (unsigned x = 0; x < PPU_ACTIVE_W; ++x) {
                rect(canvas,
                     rx + x * scale,
                     ry + y * scale,
                     scale,
                     scale,
                     rp2c02_demo_rgb(code[y][x]));
            }
        }
        return;
    }
#endif

    rp2c02_timing_t timing;
    rp2c02_timing_reset(&timing);

    /*
     * Dependency-free fallback: walk one complete 341x262 rendering-disabled
     * RP2C02 frame. When the donor PPU is enabled, the preview above is sourced
     * from johnmph/NESEmu instead and this reduced path remains as a regression
     * oracle and no-dependency build option.
     *
     * Color selection goes through the rendering-disabled hardware rule, not
     * directly through palette RAM: EXT selects the backdrop palette entry
     * unless the PPU's v address is still inside $3F00-$3FFF, in which case
     * the addressed palette entry overrides EXT.
     */
    for (unsigned i = 0; i < RP2C02_FRAME_DOTS; ++i) {
        const unsigned x = timing.dot;
        const unsigned y = timing.scanline;
        const unsigned events = rp2c02_timing_step(&timing);

        if (events & RP2C02_TIMING_VISIBLE_DOT) {
            const uint8_t code =
                rp2c02_ext_rendering_disabled_code(ppu, ext[y][x]);
            rect(canvas,
                 rx + x * scale,
                 ry + y * scale,
                 scale,
                 scale,
                 rp2c02_demo_rgb(code));
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

    /* Formal panel regions. The image wells are deliberately framed separately
       from the surrounding panel so geometry differences remain visually clear. */
    rect(canvas, 40, 70, 570, 600, panel);
    rect(canvas, 670, 70, 570, 600, panel);
    frame_rect(canvas, 40, 70, 570, 600, 4, panel_edge);
    frame_rect(canvas, 670, 70, 570, 600, 4, panel_edge);

    rect(canvas, 48, 78, 554, 42, panel_header);
    rect(canvas, 678, 78, 554, 42, panel_header);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      68, 92, 2, "GAME BOY REFERENCE", text);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      700, 92, 2, "RP2C02 EXT PATH", text);

    /* Left source region: 160x144 at exact 3x integer scale = 480x432. */
    const unsigned lx = 85;
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

    /* Right adapter region: visible 256x240 portion of a 341x262 PPU frame. */
    const unsigned rx = 699;
    const unsigned ry = 132;
    const unsigned rscale = 2;
    rect(canvas, rx - 6, ry - 6, 524, 492, (rgb8_t){8, 8, 8});
    frame_rect(canvas, rx - 6, ry - 6, 524, 492, 2, video_edge);
    draw_rp2c02_frame(canvas, rx, ry, rscale, ext, ppu);

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      79, 612, 1, "SOURCE REGION 160X144", text);
    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      696, 630, 1, "PPU REGION 256X240  11+234+11", text);

    if (state && state->source_name) {
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          79, 635, 1, state->source_name, secondary);
    }

    ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                      79, 652, 1,
                      "ARROWS MOVE  Z A  X B  BKSP SELECT  ENTER START",
                      secondary);

    if (state && state->palette_name) {
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          696, 650, 1, "PALETTE", secondary);
        ui_font_draw_text(canvas, COMPARISON_W, COMPARISON_H,
                          760, 650, 1, state->palette_name, text);
    }
}
