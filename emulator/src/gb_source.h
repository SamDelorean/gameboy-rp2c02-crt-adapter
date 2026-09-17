#ifndef GBCRT_GB_SOURCE_H
#define GBCRT_GB_SOURCE_H

#include "bridge.h"

#include <stdint.h>

typedef struct {
    /* Final Game Boy four-shade image. Values are 0..3. */
    uint8_t shade[GB_H][GB_W];

    /* Reference image from the selected source core, packed 0xAABBGGRR. */
    uint32_t reference_rgba[GB_H][GB_W];

    uint64_t frame_number;
} gb_source_frame_t;

typedef struct gb_source gb_source_t;

typedef struct {
    const char *name;
    int (*next_frame)(gb_source_t *source, gb_source_frame_t *frame);
    void (*destroy)(gb_source_t *source);
} gb_source_ops_t;

struct gb_source {
    const gb_source_ops_t *ops;
    void *ctx;
};

int gb_source_next_frame(gb_source_t *source, gb_source_frame_t *frame);
void gb_source_destroy(gb_source_t *source);

/* Dependency-free source used by V0.1 and regression tests. */
int gb_source_pattern_create(gb_source_t *source);

#ifdef GBCRT_ENABLE_SAMEBOY
/*
 * SameBoy-backed source. ROM and boot ROM are user-supplied files and are
 * never stored in this repository.
 */
int gb_source_sameboy_create(gb_source_t *source,
                             const char *rom_path,
                             const char *boot_rom_path);
#endif

#endif
