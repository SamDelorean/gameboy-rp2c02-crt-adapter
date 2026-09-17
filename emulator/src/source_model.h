#ifndef GBCRT_SOURCE_MODEL_H
#define GBCRT_SOURCE_MODEL_H

typedef enum {
    GBCRT_SOURCE_MODEL_DMG = 0,
    GBCRT_SOURCE_MODEL_SGB = 1,
} gbcrt_source_model_t;

const char *gbcrt_source_model_name(gbcrt_source_model_t model);
int gbcrt_source_model_parse(const char *text, gbcrt_source_model_t *model);

#endif
