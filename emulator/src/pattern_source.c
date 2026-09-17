#include "pattern_source.h"

void pattern_source_make(uint8_t frame[GB_H][GB_W])
{
    for (unsigned y = 0; y < GB_H; ++y) {
        for (unsigned x = 0; x < GB_W; ++x) {
            uint8_t shade = (uint8_t)((x * 4u) / GB_W);
            if ((x % 20u) == 0u || (y % 18u) == 0u) {
                shade = (uint8_t)((shade + 1u) & 3u);
            }
            frame[y][x] = shade;
        }
    }
}
