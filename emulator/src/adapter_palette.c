#include "adapter_palette.h"
#include "bridge.h"

#include <limits.h>
#include <stddef.h>

/*
 * Candidate V1 catalog used by the virtual bench. The first four presets are
 * LCD-inspired anchors; the remaining entries are coherent RP2C02 hue ramps.
 * Final hardware freeze still requires visual/bench review. Within every
 * preset, Game Boy shade 0 is lightest and shade 3 is darkest.
 */
static const adapter_palette_preset_t presets[] = {
    /* LCD-inspired anchors. */
    {"DMG LCD",       {0x38u, 0x28u, 0x18u, 0x08u}},
    {"POCKET LCD",    {0x38u, 0x10u, 0x2Du, 0x08u}},
    {"LIGHT TEAL",    {0x3Bu, 0x2Cu, 0x1Cu, 0x0Cu}},
    {"GRAYSCALE",     {0x30u, 0x10u, 0x00u, 0x0Fu}},

    /* Coherent RP2C02 hue ramps for general four-shade use. */
    {"LIME",          {0x39u, 0x29u, 0x19u, 0x09u}},
    {"GREEN",         {0x3Au, 0x2Au, 0x1Au, 0x0Au}},
    {"MINT",          {0x3Bu, 0x2Bu, 0x1Bu, 0x0Bu}},
    {"CYAN",          {0x3Cu, 0x2Cu, 0x1Cu, 0x0Cu}},
    {"SKY BLUE",      {0x31u, 0x21u, 0x11u, 0x01u}},
    {"BLUE",          {0x32u, 0x22u, 0x12u, 0x02u}},
    {"VIOLET",        {0x33u, 0x23u, 0x13u, 0x03u}},
    {"LILAC",         {0x34u, 0x24u, 0x14u, 0x04u}},
    {"ROSE",          {0x35u, 0x25u, 0x15u, 0x05u}},
    {"RED",           {0x36u, 0x26u, 0x16u, 0x06u}},
    {"AMBER",         {0x37u, 0x27u, 0x17u, 0x07u}},
    {"HIGH CONTRAST", {0x30u, 0x10u, 0x2Du, 0x0Fu}},
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

void adapter_palette_apply_codes(rp2c02_ext_t *ppu,
                                 const uint8_t shade_code[4])
{
    if (!ppu || !shade_code) return;

    for (unsigned shade = 0; shade < BRIDGE_SHADE_COUNT; ++shade) {
        rp2c02_ext_write_palette(ppu, shade, shade_code[shade]);
    }

    /* V1 side borders remain fixed canonical black regardless of palette. */
    rp2c02_ext_write_palette(ppu, BRIDGE_BORDER_EXT_INDEX, 0x0Fu);
}

void adapter_palette_apply(rp2c02_ext_t *ppu, unsigned index)
{
    const adapter_palette_preset_t *preset = adapter_palette_get(index);
    if (!ppu || !preset) return;
    adapter_palette_apply_codes(ppu, preset->shade_code);
}

static unsigned expand5(unsigned value)
{
    value &= 0x1fu;
    return (value << 3) | (value >> 2);
}

uint8_t adapter_palette_quantize_rgb555(uint16_t rgb555)
{
    const int r = (int)expand5(rgb555);
    const int g = (int)expand5(rgb555 >> 5);
    const int b = (int)expand5(rgb555 >> 10);

    unsigned best_error = UINT_MAX;
    uint8_t best_code = 0x0fu;

    for (unsigned code = 0; code < 64u; ++code) {
        /* $0D is deliberately avoided for the NTSC hardware path. */
        if (code == 0x0du) continue;

        const rgb8_t candidate = rp2c02_demo_rgb((uint8_t)code);
        const int dr = r - (int)candidate.r;
        const int dg = g - (int)candidate.g;
        const int db = b - (int)candidate.b;
        const unsigned error = (unsigned)(dr * dr + dg * dg + db * db);

        if (error < best_error) {
            best_error = error;
            best_code = (uint8_t)code;
        }
    }

    return best_code;
}

void adapter_palette_apply_sgb_rgb555(rp2c02_ext_t *ppu,
                                      const uint16_t rgb555[4])
{
    if (!ppu || !rgb555) return;

    uint8_t shade_code[BRIDGE_SHADE_COUNT];
    for (unsigned shade = 0; shade < BRIDGE_SHADE_COUNT; ++shade) {
        shade_code[shade] = adapter_palette_quantize_rgb555(rgb555[shade]);
    }
    adapter_palette_apply_codes(ppu, shade_code);
}
