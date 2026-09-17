#ifndef GBCRT_BRIDGE_H
#define GBCRT_BRIDGE_H

#include <stdint.h>

#define GB_W 160
#define GB_H 144
#define PPU_ACTIVE_W 256
#define PPU_ACTIVE_H 240
#define BRIDGE_IMAGE_W 234
#define BRIDGE_BORDER_W 11

/* EXT indices 0..3 are reserved for Game Boy shades 0..3. */
#define BRIDGE_SHADE_COUNT 4u

/*
 * The border must not reuse shade 0: Game Boy shade 0 is the lightest shade,
 * while the V1 border is fixed black.  EXT is four bits wide, so index 4 is
 * reserved as a dedicated border color entry in RP2C02 palette RAM.
 */
#define BRIDGE_BORDER_EXT_INDEX 4u

/* Source pixels are 2-bit Game Boy shade indices stored as bytes 0..3 here. */
void bridge_scale_frame(const uint8_t src[GB_H][GB_W],
                        uint8_t dst[PPU_ACTIVE_H][PPU_ACTIVE_W],
                        uint8_t border_index);

unsigned bridge_source_x(unsigned out_x);
unsigned bridge_source_y(unsigned out_y);

#endif
