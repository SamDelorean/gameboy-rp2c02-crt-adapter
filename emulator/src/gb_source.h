#ifndef GBCRT_GB_SOURCE_H
#define GBCRT_GB_SOURCE_H

#include "bridge.h"
#include "source_model.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    /* Final Game Boy four-shade image. Values are 0..3. */
    uint8_t shade[GB_H][GB_W];

    /* Reference image from the selected source core, packed 0xAABBGGRR. */
    uint32_t reference_rgba[GB_H][GB_W];

    uint64_t frame_number;

    /*
     * Optional already-decoded SGB palette supplied by the source core.
     * The main virtual bench does not emulate P14/P15, JOYP transport,
     * Arduino/RP2350 firmware, PIO or DMA. SameBoy owns SGB protocol decoding;
     * project code only maps these RGB555 colors to RP2C02 palette codes.
     *
     * sgb_palette_command is 0xff when the source provides effective palette
     * state without exposing a transport-level command ID.
     */
    bool sgb_palette_valid;
    uint16_t sgb_palette_rgb555[4];
    uint8_t sgb_palette_command;
    uint64_t sgb_palette_sequence;
} gb_source_frame_t;

typedef enum {
    GB_SOURCE_KEY_RIGHT = 0,
    GB_SOURCE_KEY_LEFT,
    GB_SOURCE_KEY_UP,
    GB_SOURCE_KEY_DOWN,
    GB_SOURCE_KEY_A,
    GB_SOURCE_KEY_B,
    GB_SOURCE_KEY_SELECT,
    GB_SOURCE_KEY_START,
    GB_SOURCE_KEY_COUNT
} gb_source_key_t;

typedef struct gb_source gb_source_t;

typedef struct {
    const char *name;
    int (*next_frame)(gb_source_t *source, gb_source_frame_t *frame);
    int (*set_key)(gb_source_t *source, gb_source_key_t key, int pressed);
    void (*destroy)(gb_source_t *source);
} gb_source_ops_t;

struct gb_source {
    const gb_source_ops_t *ops;
    void *ctx;
};

int gb_source_next_frame(gb_source_t *source, gb_source_frame_t *frame);
/* Returns 0 for accepted/no-op input, -1 for invalid source/key. */
int gb_source_set_key(gb_source_t *source, gb_source_key_t key, int pressed);
void gb_source_destroy(gb_source_t *source);

/* Dependency-free source used by regression tests and offline previews. */
int gb_source_pattern_create(gb_source_t *source);

#ifdef GBCRT_ENABLE_SAMEBOY
/*
 * SameBoy-backed source. ROM and boot ROM are user-supplied files and are
 * never stored in this repository.
 *
 * DMG: use SameBoy's normal framebuffer, recovering the four known DMG shades.
 * SGB: use SameBoy's HLE SGB state directly: raw 160x144 four-shade image plus
 * already-decoded effective SGB palette. The project then implements only the
 * abstract alternate-video bridge to the RP2C02 model.
 */
int gb_source_sameboy_create_model(gb_source_t *source,
                                   const char *rom_path,
                                   const char *boot_rom_path,
                                   gbcrt_source_model_t model);

/* Compatibility wrapper: the historical default remains DMG. */
int gb_source_sameboy_create(gb_source_t *source,
                             const char *rom_path,
                             const char *boot_rom_path);
#endif

#endif
