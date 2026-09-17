#ifndef GBCRT_UI_FONT_H
#define GBCRT_UI_FONT_H

#include "rp2c02_ext.h"

void ui_font_draw_text(rgb8_t *canvas,
                       unsigned canvas_w,
                       unsigned canvas_h,
                       unsigned x,
                       unsigned y,
                       unsigned scale,
                       const char *text,
                       rgb8_t color);

#endif
