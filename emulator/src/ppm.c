#include "ppm.h"
#include <stdio.h>

int ppm_write_rgb(const char *path, const rgb8_t *pixels, unsigned width, unsigned height)
{
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fprintf(f, "P6\n%u %u\n255\n", width, height);
    const size_t n = (size_t)width * height;
    for (size_t i = 0; i < n; ++i) {
        fputc(pixels[i].r, f);
        fputc(pixels[i].g, f);
        fputc(pixels[i].b, f);
    }
    return fclose(f) == 0 ? 0 : -1;
}
