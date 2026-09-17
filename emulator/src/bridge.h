#ifndef GBCRT_BRIDGE_H
#define GBCRT_BRIDGE_H

#include <stdint.h>

#define GB_W 160
#define GB_H 144
#define PPU_ACTIVE_W 256
#define PPU_ACTIVE_H 240
#define BRIDGE_IMAGE_W 234
#define BRIDGE_BORDER_W 11

/* Source pixels are 2-bit Game Boy shade indices stored as bytes 0..3 here. */
void bridge_scale_frame(const uint8_t src[GB_H][GB_W],
                        uint8_t dst[PPU_ACTIVE_H][PPU_ACTIVE_W],
                        uint8_t border_index);

unsigned bridge_source_x(unsigned out_x);
unsigned bridge_source_y(unsigned out_y);

#endif
