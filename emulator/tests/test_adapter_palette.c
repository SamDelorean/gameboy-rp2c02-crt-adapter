#include "adapter_palette.h"
#include "bridge.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned demo_luma(uint8_t code)
{
    const rgb8_t rgb = rp2c02_demo_rgb(code);
    /* Integer Rec.709-style weighting is sufficient for monotonicity tests. */
    return 2126u * rgb.r + 7152u * rgb.g + 722u * rgb.b;
}

static int same_codes(const adapter_palette_preset_t *a,
                      const adapter_palette_preset_t *b)
{
    return memcmp(a->shade_code, b->shade_code, sizeof(a->shade_code)) == 0;
}

int main(void)
{
    const unsigned count = adapter_palette_count();
    /* Candidate V1 catalog: 16 manual presets plus AUTO/SGB outside this table. */
    assert(count == 16u);

    for (unsigned i = 0; i < count; ++i) {
        const adapter_palette_preset_t *preset = adapter_palette_get(i);
        assert(preset != NULL);
        assert(preset->name != NULL);

        rp2c02_ext_t ppu;
        rp2c02_ext_reset(&ppu);
        adapter_palette_apply(&ppu, i);

        for (unsigned shade = 0; shade < BRIDGE_SHADE_COUNT; ++shade) {
            assert(rp2c02_ext_palette_code(&ppu, (uint8_t)shade) ==
                   (preset->shade_code[shade] & 0x3fu));
        }

        assert(rp2c02_ext_palette_code(&ppu, BRIDGE_BORDER_EXT_INDEX) == 0x0fu);

        for (unsigned shade = 0; shade < BRIDGE_SHADE_COUNT; ++shade) {
            assert(preset->shade_code[shade] < 64u);
            assert(preset->shade_code[shade] != 0x0du);
            if (shade + 1u < BRIDGE_SHADE_COUNT) {
                assert(preset->shade_code[shade] != preset->shade_code[shade + 1u]);
                assert(demo_luma(preset->shade_code[shade]) >
                       demo_luma(preset->shade_code[shade + 1u]));
            }
        }

        for (unsigned j = 0; j < i; ++j) {
            const adapter_palette_preset_t *previous = adapter_palette_get(j);
            assert(strcmp(preset->name, previous->name) != 0);
            assert(!same_codes(preset, previous));
        }
    }

    /* LCD-inspired anchors are intentional within this candidate set. */
    const uint8_t dmg_expected[4] = {0x38u, 0x28u, 0x18u, 0x08u};
    const uint8_t pocket_expected[4] = {0x38u, 0x10u, 0x2du, 0x08u};
    const uint8_t light_expected[4] = {0x3bu, 0x2cu, 0x1cu, 0x0cu};
    const uint8_t gray_expected[4] = {0x30u, 0x10u, 0x00u, 0x0fu};
    assert(memcmp(adapter_palette_get(0)->shade_code, dmg_expected, 4u) == 0);
    assert(memcmp(adapter_palette_get(1)->shade_code, pocket_expected, 4u) == 0);
    assert(memcmp(adapter_palette_get(2)->shade_code, light_expected, 4u) == 0);
    assert(memcmp(adapter_palette_get(3)->shade_code, gray_expected, 4u) == 0);

    /* Preset lookup/cycling is deliberately modular. */
    assert(adapter_palette_get(count) == adapter_palette_get(0));

    /* The editor path accepts arbitrary RP2C02 codes but keeps V1 border black. */
    const uint8_t custom[4] = {0x3cu, 0x2bu, 0x17u, 0x0fu};
    rp2c02_ext_t custom_ppu;
    rp2c02_ext_reset(&custom_ppu);
    adapter_palette_apply_codes(&custom_ppu, custom);
    for (unsigned shade = 0; shade < 4u; ++shade) {
        assert(rp2c02_ext_palette_code(&custom_ppu, (uint8_t)shade) == custom[shade]);
    }
    assert(rp2c02_ext_palette_code(&custom_ppu, BRIDGE_BORDER_EXT_INDEX) == 0x0fu);

    /* SGB automatic conversion is separate from the curated manual presets. */
    const uint16_t sgb[4] = {
        0x7fffu, /* white */
        0x03ffu, /* yellow-ish */
        0x001fu, /* red */
        0x0000u, /* black */
    };

    uint8_t expected[4];
    for (unsigned i = 0; i < 4; ++i) {
        expected[i] = adapter_palette_quantize_rgb555(sgb[i]);
        assert(expected[i] < 64u);
        assert(expected[i] != 0x0du);
    }
    assert(expected[0] != expected[3]);

    rp2c02_ext_t sgb_ppu;
    rp2c02_ext_reset(&sgb_ppu);
    adapter_palette_apply_sgb_rgb555(&sgb_ppu, sgb);
    for (unsigned shade = 0; shade < 4; ++shade) {
        assert(rp2c02_ext_palette_code(&sgb_ppu, (uint8_t)shade) == expected[shade]);
    }
    assert(rp2c02_ext_palette_code(&sgb_ppu, BRIDGE_BORDER_EXT_INDEX) == 0x0fu);

    puts("adapter palettes OK: manual presets + provisional SGB RGB555 quantization");
    return 0;
}
