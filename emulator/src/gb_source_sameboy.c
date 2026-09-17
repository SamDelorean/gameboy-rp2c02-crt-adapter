#include "gb_source.h"

#ifdef GBCRT_ENABLE_SAMEBOY

#include "Core/gb.h"

#include <limits.h>
#include <stdlib.h>

typedef struct {
    GB_gameboy_t *gb;
    uint32_t screen[GB_W * GB_H];
    uint64_t frame_number;
} sameboy_ctx_t;

/* Our comparison frontend uses a deliberately explicit 0xAABBGGRR layout. */
static uint32_t encode_rgb(GB_gameboy_t *gb, uint8_t r, uint8_t g, uint8_t b)
{
    (void)gb;
    return (uint32_t)r |
           ((uint32_t)g << 8) |
           ((uint32_t)b << 16) |
           0xff000000u;
}

static uint8_t shade_from_rgba(uint32_t rgba)
{
    const int r = (int)(rgba & 0xffu);
    const int g = (int)((rgba >> 8) & 0xffu);
    const int b = (int)((rgba >> 16) & 0xffu);

    unsigned best_shade = 0;
    unsigned best_error = UINT_MAX;

    /* SameBoy maps DMG shade 0..3 to GB_PALETTE_DMG colors 3..0. */
    for (unsigned shade = 0; shade < 4; ++shade) {
        const unsigned palette_index = 3u - shade;
        const int dr = r - (int)GB_PALETTE_DMG.colors[palette_index].r;
        const int dg = g - (int)GB_PALETTE_DMG.colors[palette_index].g;
        const int db = b - (int)GB_PALETTE_DMG.colors[palette_index].b;
        const unsigned error = (unsigned)(dr * dr + dg * dg + db * db);
        if (error < best_error) {
            best_error = error;
            best_shade = shade;
        }
    }

    return (uint8_t)best_shade;
}

static int sameboy_next_frame(gb_source_t *source, gb_source_frame_t *frame)
{
    sameboy_ctx_t *ctx = source->ctx;
    if (!ctx || !ctx->gb) return -1;

    (void)GB_run_frame(ctx->gb);

    for (unsigned y = 0; y < GB_H; ++y) {
        for (unsigned x = 0; x < GB_W; ++x) {
            const uint32_t rgba = ctx->screen[(size_t)y * GB_W + x];
            frame->reference_rgba[y][x] = rgba;
            frame->shade[y][x] = shade_from_rgba(rgba);
        }
    }

    frame->frame_number = ctx->frame_number++;
    return 0;
}

static void sameboy_destroy(gb_source_t *source)
{
    sameboy_ctx_t *ctx = source->ctx;
    if (!ctx) return;
    if (ctx->gb) {
        GB_free(ctx->gb);
        GB_dealloc(ctx->gb);
    }
    free(ctx);
}

static const gb_source_ops_t sameboy_ops = {
    .name = "SameBoy",
    .next_frame = sameboy_next_frame,
    .destroy = sameboy_destroy,
};

int gb_source_sameboy_create(gb_source_t *source,
                             const char *rom_path,
                             const char *boot_rom_path)
{
    if (!source || !rom_path || !boot_rom_path) return -1;

    sameboy_ctx_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return -1;

    ctx->gb = GB_init(GB_alloc(), GB_MODEL_DMG_B);
    if (!ctx->gb) {
        free(ctx);
        return -1;
    }

    GB_set_border_mode(ctx->gb, GB_BORDER_NEVER);
    GB_set_pixels_output(ctx->gb, ctx->screen);
    GB_set_rgb_encode_callback(ctx->gb, encode_rgb);
    GB_set_palette(ctx->gb, &GB_PALETTE_DMG);

    if (GB_load_rom(ctx->gb, rom_path) != 0 ||
        GB_load_boot_rom(ctx->gb, boot_rom_path) != 0) {
        GB_free(ctx->gb);
        GB_dealloc(ctx->gb);
        free(ctx);
        return -1;
    }

    GB_reset(ctx->gb);

    source->ops = &sameboy_ops;
    source->ctx = ctx;
    return 0;
}

#endif
