#include "gb_source.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    gb_source_t source = {0};
    gb_source_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    assert(gb_source_pattern_create(&source) == 0);
    assert(source.ops != NULL);
    assert(source.ops->name != NULL);

    /* Pattern source accepts joypad events as deliberate no-ops. */
    for (int key = 0; key < GB_SOURCE_KEY_COUNT; ++key) {
        assert(gb_source_set_key(&source, (gb_source_key_t)key, 1) == 0);
        assert(gb_source_set_key(&source, (gb_source_key_t)key, 0) == 0);
    }
    assert(gb_source_set_key(&source, (gb_source_key_t)-1, 1) == -1);
    assert(gb_source_set_key(&source, GB_SOURCE_KEY_COUNT, 1) == -1);

    assert(gb_source_next_frame(&source, &frame) == 0);
    assert(frame.frame_number == 0u);

    for (unsigned y = 0; y < GB_H; ++y) {
        for (unsigned x = 0; x < GB_W; ++x) {
            assert(frame.shade[y][x] <= 3u);
            assert((frame.reference_rgba[y][x] >> 24) == 0xffu);
        }
    }

    assert(gb_source_next_frame(&source, &frame) == 0);
    assert(frame.frame_number == 1u);

    gb_source_destroy(&source);
    assert(source.ops == NULL);
    assert(source.ctx == NULL);

    puts("Game Boy source abstraction and joypad interface OK");
    return 0;
}
