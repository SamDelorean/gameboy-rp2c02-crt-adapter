#include "adapter_palette.h"
#include "bridge.h"

#include <stddef.h>

/*
 * Virtual-bench presets only.  Their count/names do not freeze the final V1
 * hardware preset set.  Within each preset, Game Boy shade 0 is lightest and
 * shade 3 is darkest.
 */
static const adapter_palette_preset_t presets[] = {
    {"DMG GREEN", {0x2Au, 0x1Au, 0x0Au, 0x0Fu}},
    {"GRAYSCALE", {0x30u, 0x20u, 0x10u, 0x0Fu}},
    {"AMBER",     {0x37u, 0x27u, 0x17u, 0x0Fu}},
    {"COOL BLUE", {0x31u, 0x21u, 0x11u, 0x0Fu}},
    {"LILAC",     {0x34u, 0x24u, 0x14u, 0x0Fu}},
};

unsigned adapter_palette_count(void)
{
    return (unsigned)(sizeof(presets) / sizeof(presets[0]));
}

const adapter_palette_preset_t *adapter_palette_get(unsigned index)
{
    const unsigned count = adapter_palette_count();
    if (count == 0u) return NULL;
    return &presets[index % count];
}

void adapter_palette_apply(rp2c02_ext_t *ppu, unsigned index)
{
    const adapter_palette_preset_t *preset = adapter_palette_get(index);
    if (!ppu || !preset) return;

    for (unsigned shade = 0; shade < BRIDGE_SHADE_COUNT; ++shade) {
        rp2c02_ext_write_palette(ppu, shade, preset->shade_code[shade]);
    }

    /* V1 side borders remain fixed canonical black regardless of preset. */
    rp2c02_ext_write_palette(ppu, BRIDGE_BORDER_EXT_INDEX, 0x0Fu);
}
