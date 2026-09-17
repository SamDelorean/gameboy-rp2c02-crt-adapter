#include "adapter_palette.h"
#include "bridge.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    const unsigned count = adapter_palette_count();
    assert(count >= 2u);

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
        assert(preset->shade_code[0] != preset->shade_code[3]);
    }

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
