#include "gb_source.h"

#include <string.h>

int gb_source_next_frame(gb_source_t *source, gb_source_frame_t *frame)
{
    if (!source || !source->ops || !source->ops->next_frame || !frame) return -1;
    return source->ops->next_frame(source, frame);
}

void gb_source_destroy(gb_source_t *source)
{
    if (!source) return;
    if (source->ops && source->ops->destroy) source->ops->destroy(source);
    memset(source, 0, sizeof(*source));
}
