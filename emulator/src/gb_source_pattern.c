#include "gb_source.h"
#include "pattern_source.h"

#include <stdlib.h>

typedef struct {
    uint64_t frame_number;
} pattern_ctx_t;

static uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint32_t)r |
           ((uint32_t)g << 8) |
           ((uint32_t)b << 16) |
           0xff000000u;
}

static uint32_t shade_rgba(uint8_t shade)
{
    static const uint8_t dmg[4][3] = {
        {198, 220, 140},
        {132, 165, 99},
        {57, 97, 57},
        {8, 24, 16},
    };
    const uint8_t *c = dmg[shade & 3u];
    return pack_rgba(c[0], c[1], c[2]);
}

static int pattern_next_frame(gb_source_t *source, gb_source_frame_t *frame)
{
    pattern_ctx_t *ctx = source->ctx;
    pattern_source_make(frame->shade);

    for (unsigned y = 0; y < GB_H; ++y) {
        for (unsigned x = 0; x < GB_W; ++x) {
            frame->reference_rgba[y][x] = shade_rgba(frame->shade[y][x]);
        }
    }

    frame->frame_number = ctx->frame_number++;
    return 0;
}

static int pattern_set_key(gb_source_t *source, gb_source_key_t key, int pressed)
{
    (void)source;
    (void)key;
    (void)pressed;
    return 0;
}

static void pattern_destroy(gb_source_t *source)
{
    free(source->ctx);
}

static const gb_source_ops_t pattern_ops = {
    .name = "pattern",
    .next_frame = pattern_next_frame,
    .set_key = pattern_set_key,
    .destroy = pattern_destroy,
};

int gb_source_pattern_create(gb_source_t *source)
{
    if (!source) return -1;
    pattern_ctx_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return -1;
    source->ops = &pattern_ops;
    source->ctx = ctx;
    return 0;
}
