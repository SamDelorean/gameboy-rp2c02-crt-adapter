#include "gb_source.h"

#include <string.h>

int gb_source_next_frame(gb_source_t *source, gb_source_frame_t *frame)
{
    if (!source || !source->ops || !source->ops->next_frame || !frame) return -1;
    return source->ops->next_frame(source, frame);
}

int gb_source_set_key(gb_source_t *source, gb_source_key_t key, int pressed)
{
    if (!source || !source->ops || key < 0 || key >= GB_SOURCE_KEY_COUNT) return -1;
    if (!source->ops->set_key) return 0;
    return source->ops->set_key(source, key, pressed ? 1 : 0);
}

void gb_source_destroy(gb_source_t *source)
{
    if (!source) return;
    if (source->ops && source->ops->destroy) source->ops->destroy(source);
    memset(source, 0, sizeof(*source));
}
