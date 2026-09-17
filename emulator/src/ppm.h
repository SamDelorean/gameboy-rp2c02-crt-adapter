#ifndef GBCRT_PPM_H
#define GBCRT_PPM_H

#include "rp2c02_ext.h"

int ppm_write_rgb(const char *path, const rgb8_t *pixels, unsigned width, unsigned height);

#endif
