#include "ui_font.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>

static void glyph_rows(char c, uint8_t r[7])
{
    for (unsigned i = 0; i < 7; ++i) r[i] = 0;
    c = (char)toupper((unsigned char)c);

    switch (c) {
    case 'A': { uint8_t v[7]={14,17,17,31,17,17,17}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'B': { uint8_t v[7]={30,17,17,30,17,17,30}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'C': { uint8_t v[7]={14,17,16,16,16,17,14}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'D': { uint8_t v[7]={30,17,17,17,17,17,30}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'E': { uint8_t v[7]={31,16,16,30,16,16,31}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'F': { uint8_t v[7]={31,16,16,30,16,16,16}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'G': { uint8_t v[7]={14,17,16,23,17,17,15}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'H': { uint8_t v[7]={17,17,17,31,17,17,17}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'I': { uint8_t v[7]={31,4,4,4,4,4,31}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'J': { uint8_t v[7]={7,2,2,2,18,18,12}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'K': { uint8_t v[7]={17,18,20,24,20,18,17}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'L': { uint8_t v[7]={16,16,16,16,16,16,31}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'M': { uint8_t v[7]={17,27,21,21,17,17,17}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'N': { uint8_t v[7]={17,25,21,19,17,17,17}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'O': { uint8_t v[7]={14,17,17,17,17,17,14}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'P': { uint8_t v[7]={30,17,17,30,16,16,16}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'Q': { uint8_t v[7]={14,17,17,17,21,18,13}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'R': { uint8_t v[7]={30,17,17,30,20,18,17}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'S': { uint8_t v[7]={15,16,16,14,1,1,30}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'T': { uint8_t v[7]={31,4,4,4,4,4,4}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'U': { uint8_t v[7]={17,17,17,17,17,17,14}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'V': { uint8_t v[7]={17,17,17,17,17,10,4}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'W': { uint8_t v[7]={17,17,17,21,21,21,10}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'X': { uint8_t v[7]={17,17,10,4,10,17,17}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'Y': { uint8_t v[7]={17,17,10,4,4,4,4}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case 'Z': { uint8_t v[7]={31,1,2,4,8,16,31}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '0': { uint8_t v[7]={14,17,19,21,25,17,14}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '1': { uint8_t v[7]={4,12,4,4,4,4,14}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '2': { uint8_t v[7]={14,17,1,2,4,8,31}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '3': { uint8_t v[7]={30,1,1,14,1,1,30}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '4': { uint8_t v[7]={2,6,10,18,31,2,2}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '5': { uint8_t v[7]={31,16,16,30,1,1,30}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '6': { uint8_t v[7]={14,16,16,30,17,17,14}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '7': { uint8_t v[7]={31,1,2,4,8,8,8}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '8': { uint8_t v[7]={14,17,17,14,17,17,14}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '9': { uint8_t v[7]={14,17,17,15,1,1,14}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case ':': { uint8_t v[7]={0,4,4,0,4,4,0}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '.': r[6]=4; break;
    case '-': r[3]=14; break;
    case '+': r[2]=4; r[3]=14; r[4]=4; break;
    case '/': { uint8_t v[7]={1,2,2,4,8,8,16}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    case '=': r[2]=14; r[4]=14; break;
    case '>': { uint8_t v[7]={16,8,4,2,4,8,16}; for(int i=0;i<7;i++)r[i]=v[i]; } break;
    default: break;
    }
}

static void put_block(rgb8_t *canvas,
                      unsigned canvas_w,
                      unsigned canvas_h,
                      unsigned x,
                      unsigned y,
                      unsigned scale,
                      rgb8_t color)
{
    for (unsigned yy = 0; yy < scale; ++yy) {
        if (y + yy >= canvas_h) continue;
        for (unsigned xx = 0; xx < scale; ++xx) {
            if (x + xx >= canvas_w) continue;
            canvas[(size_t)(y + yy) * canvas_w + x + xx] = color;
        }
    }
}

void ui_font_draw_text(rgb8_t *canvas,
                       unsigned canvas_w,
                       unsigned canvas_h,
                       unsigned x,
                       unsigned y,
                       unsigned scale,
                       const char *text,
                       rgb8_t color)
{
    if (!canvas || !text || scale == 0) return;

    unsigned cursor = x;
    for (const char *p = text; *p; ++p) {
        if (*p == ' ') {
            cursor += 4u * scale;
            continue;
        }

        uint8_t rows[7];
        glyph_rows(*p, rows);
        for (unsigned gy = 0; gy < 7; ++gy) {
            for (unsigned gx = 0; gx < 5; ++gx) {
                if (rows[gy] & (1u << (4u - gx))) {
                    put_block(canvas,
                              canvas_w,
                              canvas_h,
                              cursor + gx * scale,
                              y + gy * scale,
                              scale,
                              color);
                }
            }
        }
        cursor += 6u * scale;
    }
}
