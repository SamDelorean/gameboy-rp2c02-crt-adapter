#ifndef GBCRT_COMPARISON_RENDER_H
#define GBCRT_COMPARISON_RENDER_H

#include "gb_source.h"
#include "rp2c02_ext.h"

#include <stdint.h>

#define COMPARISON_W 1280
#define COMPARISON_H 720

void comparison_render(rgb8_t *canvas,
                       const gb_source_frame_t *frame,
                       const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
                       const rp2c02_ext_t *ppu);

#endif
