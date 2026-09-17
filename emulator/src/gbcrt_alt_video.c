#include "gbcrt_alt_video.h"

#include "adapter_palette.h"

#include <string.h>

void gbcrt_alt_video_init(gbcrt_alt_video_t *state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    rp2c02_ext_reset(&state->ppu);
    state->palette_mode = 0u;
}

unsigned gbcrt_alt_video_palette_mode(const gbcrt_alt_video_t *state)
{
    return state ? state->palette_mode : 0u;
}

void gbcrt_alt_video_set_palette_mode(gbcrt_alt_video_t *state, unsigned mode)
{
    if (!state) return;
    const unsigned count = adapter_palette_count();
    const unsigned total_modes = count + 1u; /* AUTO/SGB + manual presets */
    state->palette_mode = total_modes ? mode % total_modes : 0u;
}

void gbcrt_alt_video_next_palette(gbcrt_alt_video_t *state)
{
    if (!state) return;
    gbcrt_alt_video_set_palette_mode(state, state->palette_mode + 1u);
}

static void apply_selected_palette(gbcrt_alt_video_t *state,
                                   const uint16_t sgb_rgb555[4],
                                   bool sgb_palette_valid)
{
    if (state->palette_mode == 0u) {
        if (sgb_palette_valid && sgb_rgb555) {
            adapter_palette_apply_sgb_rgb555(&state->ppu, sgb_rgb555);
        }
        else {
            adapter_palette_apply(&state->ppu, 0u);
        }
        return;
    }

    adapter_palette_apply(&state->ppu, state->palette_mode - 1u);
}

void gbcrt_alt_video_render(gbcrt_alt_video_t *state,
                            const uint8_t shade[GB_H][GB_W],
                            const uint16_t sgb_rgb555[4],
                            bool sgb_palette_valid,
                            rgb8_t out_rgb[PPU_ACTIVE_H][PPU_ACTIVE_W])
{
    if (!state || !shade || !out_rgb) return;

    apply_selected_palette(state, sgb_rgb555, sgb_palette_valid);
    bridge_scale_frame(shade, state->ext, BRIDGE_BORDER_EXT_INDEX);

    for (unsigned y = 0; y < PPU_ACTIVE_H; ++y) {
        for (unsigned x = 0; x < PPU_ACTIVE_W; ++x) {
            const uint8_t code =
                rp2c02_ext_rendering_disabled_code(&state->ppu,
                                                    state->ext[y][x]);
            out_rgb[y][x] = rp2c02_demo_rgb(code);
        }
    }
}
