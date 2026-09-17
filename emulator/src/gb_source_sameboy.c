#include "gb_source.h"
#include "sameboy_bootroms.h"

#ifdef GBCRT_ENABLE_SAMEBOY

#include "Core/gb.h"
#include "Core/sgb.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    GB_gameboy_t *gb;
    char *boot_rom_override;
    int boot_rom_load_failed;
    uint32_t screen[GB_W * GB_H];
    gbcrt_source_model_t model;
    uint16_t last_sgb_palette[4];
    bool last_sgb_palette_valid;
    uint64_t sgb_palette_sequence;
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

static char *copy_string(const char *text)
{
    if (!text) return NULL;
    const size_t size = strlen(text) + 1u;
    char *copy = malloc(size);
    if (copy) memcpy(copy, text, size);
    return copy;
}

static void sameboy_load_boot_rom(GB_gameboy_t *gb, GB_boot_rom_t type)
{
    sameboy_ctx_t *ctx = GB_get_user_data(gb);
    if (!ctx) return;

    ctx->boot_rom_load_failed = 0;

    if (ctx->boot_rom_override) {
        if (GB_load_boot_rom(gb, ctx->boot_rom_override) != 0) {
            fprintf(stderr, "failed to load SameBoy boot ROM override: %s\n",
                    ctx->boot_rom_override);
            ctx->boot_rom_load_failed = 1;
        }
        return;
    }

    switch (type) {
    case GB_BOOT_ROM_DMG:
        GB_load_boot_rom_from_buffer(gb,
                                     gbcrt_sameboy_dmg_boot,
                                     sizeof(gbcrt_sameboy_dmg_boot));
        return;
    case GB_BOOT_ROM_SGB:
        GB_load_boot_rom_from_buffer(gb,
                                     gbcrt_sameboy_sgb_boot,
                                     sizeof(gbcrt_sameboy_sgb_boot));
        return;
    default:
        fprintf(stderr,
                "no built-in SameBoy boot ROM is available for requested type %d\n",
                (int)type);
        ctx->boot_rom_load_failed = 1;
        return;
    }
}

static int reject_cgb_only_rom(GB_gameboy_t *gb)
{
    size_t rom_size = 0u;
    uint16_t bank = 0u;
    const uint8_t *rom = GB_get_direct_access(gb,
                                               GB_DIRECT_ACCESS_ROM,
                                               &rom_size,
                                               &bank);
    (void)bank;
    if (!rom || rom_size <= 0x143u) return 0;

    if (rom[0x143u] == 0xc0u) {
        fprintf(stderr,
                "ROM header marks this cartridge as CGB-only; "
                "the current PoC supports DMG/SGB sources only\n");
        return -1;
    }
    return 0;
}

static int update_sgb_palette_metadata(sameboy_ctx_t *ctx,
                                       gb_source_frame_t *frame)
{
    if (!ctx || !ctx->gb || !ctx->gb->sgb) return -1;

    uint16_t current[4];
    for (unsigned i = 0; i < 4; ++i) {
        current[i] = ctx->gb->sgb->effective_palettes[i] & 0x7fffu;
    }

    if (!ctx->last_sgb_palette_valid ||
        memcmp(current, ctx->last_sgb_palette, sizeof(current)) != 0) {
        memcpy(ctx->last_sgb_palette, current, sizeof(current));
        ctx->last_sgb_palette_valid = true;
        ctx->sgb_palette_sequence++;
    }

    frame->sgb_palette_valid = true;
    memcpy(frame->sgb_palette_rgb555, current, sizeof(current));
    frame->sgb_palette_command = 0xffu; /* HLE result; command transport is abstracted. */
    frame->sgb_palette_sequence = ctx->sgb_palette_sequence;
    return 0;
}

static int sameboy_next_frame(gb_source_t *source, gb_source_frame_t *frame)
{
    sameboy_ctx_t *ctx = source->ctx;
    if (!ctx || !ctx->gb) return -1;

    (void)GB_run_frame(ctx->gb);

    if (ctx->model == GBCRT_SOURCE_MODEL_SGB) {
        if (!ctx->gb->sgb) {
            fprintf(stderr, "SameBoy SGB HLE state is unavailable\n");
            return -1;
        }

        /*
         * SameBoy already decoded the SGB protocol.  The virtual bench consumes
         * its raw four-shade Game Boy screen buffer plus its effective SGB
         * palette.  No JOYP/P14/P15 electrical transport, Arduino/RP2350, PIO,
         * DMA or firmware behavior is emulated here.
         */
        for (unsigned y = 0; y < GB_H; ++y) {
            for (unsigned x = 0; x < GB_W; ++x) {
                const size_t i = (size_t)y * GB_W + x;
                frame->shade[y][x] = ctx->gb->sgb->screen_buffer[i] & 3u;
                frame->reference_rgba[y][x] = ctx->screen[i];
            }
        }

        if (update_sgb_palette_metadata(ctx, frame) != 0) return -1;
    }
    else {
        frame->sgb_palette_valid = false;
        frame->sgb_palette_command = 0xffu;
        frame->sgb_palette_sequence = 0u;
        memset(frame->sgb_palette_rgb555, 0, sizeof(frame->sgb_palette_rgb555));

        for (unsigned y = 0; y < GB_H; ++y) {
            for (unsigned x = 0; x < GB_W; ++x) {
                const uint32_t rgba = ctx->screen[(size_t)y * GB_W + x];
                frame->reference_rgba[y][x] = rgba;
                frame->shade[y][x] = shade_from_rgba(rgba);
            }
        }
    }

    frame->frame_number = ctx->frame_number++;
    return 0;
}

static int sameboy_set_key(gb_source_t *source,
                           gb_source_key_t key,
                           int pressed)
{
    sameboy_ctx_t *ctx = source->ctx;
    if (!ctx || !ctx->gb || key < 0 || key >= GB_SOURCE_KEY_COUNT) return -1;

    static const GB_key_t map[GB_SOURCE_KEY_COUNT] = {
        [GB_SOURCE_KEY_RIGHT]  = GB_KEY_RIGHT,
        [GB_SOURCE_KEY_LEFT]   = GB_KEY_LEFT,
        [GB_SOURCE_KEY_UP]     = GB_KEY_UP,
        [GB_SOURCE_KEY_DOWN]   = GB_KEY_DOWN,
        [GB_SOURCE_KEY_A]      = GB_KEY_A,
        [GB_SOURCE_KEY_B]      = GB_KEY_B,
        [GB_SOURCE_KEY_SELECT] = GB_KEY_SELECT,
        [GB_SOURCE_KEY_START]  = GB_KEY_START,
    };

    GB_set_key_state(ctx->gb, map[key], pressed != 0);
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
    free(ctx->boot_rom_override);
    free(ctx);
}

static const gb_source_ops_t sameboy_dmg_ops = {
    .name = "SameBoy DMG",
    .next_frame = sameboy_next_frame,
    .set_key = sameboy_set_key,
    .destroy = sameboy_destroy,
};

static const gb_source_ops_t sameboy_sgb_ops = {
    .name = "SameBoy SGB",
    .next_frame = sameboy_next_frame,
    .set_key = sameboy_set_key,
    .destroy = sameboy_destroy,
};

int gb_source_sameboy_create_model(gb_source_t *source,
                                   const char *rom_path,
                                   const char *boot_rom_path,
                                   gbcrt_source_model_t model)
{
    if (!source || !rom_path) return -1;
    if (model != GBCRT_SOURCE_MODEL_DMG && model != GBCRT_SOURCE_MODEL_SGB) return -1;

    sameboy_ctx_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return -1;

    ctx->model = model;

    /*
     * SGB uses SameBoy's HLE SFC side deliberately.  SameBoy therefore owns
     * command decoding and palette state; this project only adapts the already
     * decoded image/palette to the RP2C02 path.
     */
    const GB_model_t sameboy_model =
        model == GBCRT_SOURCE_MODEL_SGB ? GB_MODEL_SGB_NTSC : GB_MODEL_DMG_B;

    ctx->gb = GB_init(GB_alloc(), sameboy_model);
    if (!ctx->gb) {
        free(ctx);
        return -1;
    }

    if (boot_rom_path) {
        ctx->boot_rom_override = copy_string(boot_rom_path);
        if (!ctx->boot_rom_override) {
            GB_free(ctx->gb);
            GB_dealloc(ctx->gb);
            free(ctx);
            return -1;
        }
    }

    /*
     * Follow SameBoy's documented embedding API: frontends register a boot-ROM
     * callback, and GB_reset() requests the correct image for the selected
     * model.  The built-in images below are compiled from the same pinned
     * SameBoy source revision used by this project. --boot remains an explicit
     * override for validation/debugging.
     */
    GB_set_user_data(ctx->gb, ctx);
    GB_set_boot_rom_load_callback(ctx->gb, sameboy_load_boot_rom);
    if (ctx->boot_rom_load_failed) {
        sameboy_destroy(&(gb_source_t){.ctx = ctx});
        return -1;
    }

    GB_set_border_mode(ctx->gb, GB_BORDER_NEVER);
    GB_set_pixels_output(ctx->gb, ctx->screen);
    GB_set_rgb_encode_callback(ctx->gb, encode_rgb);
    GB_set_palette(ctx->gb, &GB_PALETTE_DMG);
    GB_set_emulate_joypad_bouncing(ctx->gb, false);

    if (GB_load_rom(ctx->gb, rom_path) != 0 ||
        reject_cgb_only_rom(ctx->gb) != 0) {
        sameboy_destroy(&(gb_source_t){.ctx = ctx});
        return -1;
    }

    ctx->boot_rom_load_failed = 0;
    GB_reset(ctx->gb);
    if (ctx->boot_rom_load_failed) {
        sameboy_destroy(&(gb_source_t){.ctx = ctx});
        return -1;
    }

    source->ops = model == GBCRT_SOURCE_MODEL_SGB ? &sameboy_sgb_ops : &sameboy_dmg_ops;
    source->ctx = ctx;
    return 0;
}

int gb_source_sameboy_create(gb_source_t *source,
                             const char *rom_path,
                             const char *boot_rom_path)
{
    return gb_source_sameboy_create_model(source,
                                          rom_path,
                                          boot_rom_path,
                                          GBCRT_SOURCE_MODEL_DMG);
}

#endif
