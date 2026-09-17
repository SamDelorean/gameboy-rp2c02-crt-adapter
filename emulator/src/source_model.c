#include "source_model.h"

#include <string.h>

const char *gbcrt_source_model_name(gbcrt_source_model_t model)
{
    return model == GBCRT_SOURCE_MODEL_SGB ? "SGB" : "DMG";
}

int gbcrt_source_model_parse(const char *text, gbcrt_source_model_t *model)
{
    if (!text || !model) return -1;
    if (strcmp(text, "dmg") == 0) {
        *model = GBCRT_SOURCE_MODEL_DMG;
        return 0;
    }
    if (strcmp(text, "sgb") == 0) {
        *model = GBCRT_SOURCE_MODEL_SGB;
        return 0;
    }
    return -1;
}
