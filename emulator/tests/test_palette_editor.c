#include "comparison_render.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    for (unsigned shade = 0; shade < 4u; ++shade) {
        const unsigned x = COMPARISON_EDITOR_SHADE_X +
            shade * (COMPARISON_EDITOR_SHADE_W + COMPARISON_EDITOR_SHADE_GAP) + 4u;
        const unsigned y = COMPARISON_EDITOR_SHADE_Y + 4u;
        unsigned got = 99u;
        assert(comparison_palette_editor_shade_at(x, y, &got));
        assert(got == shade);
    }

    uint8_t code = 0xffu;
    assert(comparison_palette_editor_code_at(COMPARISON_EDITOR_GRID_X + 2u,
                                             COMPARISON_EDITOR_GRID_Y + 2u,
                                             &code));
    assert(code == 0x00u);

    assert(comparison_palette_editor_code_at(COMPARISON_EDITOR_GRID_X + 15u * COMPARISON_EDITOR_CELL_W + 2u,
                                             COMPARISON_EDITOR_GRID_Y + 3u * COMPARISON_EDITOR_CELL_H + 2u,
                                             &code));
    assert(code == 0x3fu);

    for (unsigned action = COMPARISON_EDITOR_ACTION_REVERSE;
         action <= COMPARISON_EDITOR_ACTION_DONE;
         ++action) {
        const unsigned index = action - COMPARISON_EDITOR_ACTION_REVERSE;
        const unsigned x = COMPARISON_EDITOR_SHADE_X +
            index * (COMPARISON_EDITOR_BUTTON_W + COMPARISON_EDITOR_BUTTON_GAP) + 4u;
        const unsigned y = COMPARISON_EDITOR_BUTTON_Y + 4u;
        assert(comparison_palette_editor_action_at(x, y) ==
               (comparison_palette_editor_action_t)action);
    }

    assert(!comparison_palette_editor_shade_at(0u, 0u, NULL));
    assert(!comparison_palette_editor_code_at(0u, 0u, NULL));
    assert(comparison_palette_editor_action_at(0u, 0u) == COMPARISON_EDITOR_ACTION_NONE);

    puts("palette_editor: PASS");
    return 0;
}
