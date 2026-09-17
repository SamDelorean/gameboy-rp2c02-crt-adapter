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

    puts("adapter palettes OK: shade indices independent, border remains black");
    return 0;
}
