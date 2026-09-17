#include "rp2c02_ext.h"

#include <string.h>

static bool is_palette_address(uint16_t address)
{
    const uint16_t a = address & 0x3fffu;
    return a >= 0x3f00u;
}

unsigned rp2c02_ext_palette_index_for_address(uint16_t address)
{
    unsigned index = address & 0x1fu;

    /* $3F10/$14/$18/$1C are mirrors of $3F00/$04/$08/$0C. */
    if ((index & 0x13u) == 0x10u) {
        index -= 0x10u;
    }

    return index;
}

void rp2c02_ext_reset(rp2c02_ext_t *ppu)
{
    memset(ppu, 0, sizeof(*ppu));
    ppu->ppuaddr_high_next = true;
}

void rp2c02_ext_write_palette(rp2c02_ext_t *ppu,
                              unsigned index,
                              uint8_t nes_color_code)
{
    if (index < 32u) {
        unsigned physical = index;
        if ((physical & 0x13u) == 0x10u) {
            physical -= 0x10u;
        }
        ppu->palette_ram[physical] = nes_color_code & 0x3fu;
    }
}

uint8_t rp2c02_ext_palette_code(const rp2c02_ext_t *ppu,
                                uint8_t ext_nibble)
{
    return ppu->palette_ram[ext_nibble & 0x0fu] & 0x3fu;
}

void rp2c02_ext_cpu_write(rp2c02_ext_t *ppu, uint8_t reg, uint8_t value)
{
    if (!ppu) return;

    switch (reg & 7u) {
    case RP2C02_REG_CTRL:
        ppu->ctrl = value;
        break;

    case RP2C02_REG_MASK:
        ppu->mask = value;
        break;

    case RP2C02_REG_ADDR:
        if (ppu->ppuaddr_high_next) {
            ppu->vram_address =
                (uint16_t)((ppu->vram_address & 0x00ffu) |
                           ((uint16_t)(value & 0x3fu) << 8));
            ppu->ppuaddr_high_next = false;
        }
        else {
            ppu->vram_address =
                (uint16_t)((ppu->vram_address & 0x3f00u) | value);
            ppu->ppuaddr_high_next = true;
        }
        break;

    case RP2C02_REG_DATA: {
        const uint16_t address = ppu->vram_address & 0x3fffu;
        if (is_palette_address(address)) {
            const unsigned index = rp2c02_ext_palette_index_for_address(address);
            ppu->palette_ram[index] = value & 0x3fu;
        }

        const uint16_t increment =
            (ppu->ctrl & RP2C02_CTRL_INCREMENT_32) ? 32u : 1u;
        ppu->vram_address =
            (uint16_t)((ppu->vram_address + increment) & 0x3fffu);
        break;
    }

    default:
        /* Other PPU registers are deliberately outside the reduced model. */
        break;
    }
}

bool rp2c02_ext_rendering_enabled(const rp2c02_ext_t *ppu)
{
    return ppu &&
           (ppu->mask & (RP2C02_MASK_RENDER_BG | RP2C02_MASK_RENDER_SPR));
}

bool rp2c02_ext_palette_override_active(const rp2c02_ext_t *ppu)
{
    if (!ppu || rp2c02_ext_rendering_enabled(ppu)) return false;
    return is_palette_address(ppu->vram_address);
}

uint8_t rp2c02_ext_rendering_disabled_code(const rp2c02_ext_t *ppu,
                                            uint8_t ext_nibble)
{
    if (!ppu) return 0u;

    unsigned palette_index;
    if (rp2c02_ext_palette_override_active(ppu)) {
        palette_index =
            rp2c02_ext_palette_index_for_address(ppu->vram_address);
    }
    else if (ppu->ctrl & RP2C02_CTRL_EXT_OUTPUT) {
        /* In EXT output/slave mode the internal EXT input is forced to zero. */
        palette_index = 0u;
    }
    else {
        palette_index = ext_nibble & 0x0fu;
    }

    uint8_t code = ppu->palette_ram[palette_index] & 0x3fu;
    if (ppu->mask & RP2C02_MASK_GREYSCALE) {
        code &= 0x30u;
    }
    return code;
}

rgb8_t rp2c02_demo_rgb(uint8_t code)
{
    /*
     * Provisional monitor approximation for the 64 six-bit RP2C02 color
     * values. It is intentionally a display LUT, not an NTSC waveform model.
     *
     * Important: bits 4..5 are the PPU value/luma field and must not be
     * discarded. The earlier preview indexed only code & 0x0f, making e.g.
     * $09, $19 and $29 appear identical even though the 2C02 treats them as
     * different value levels of the same hue.
     *
     * The table follows a conventional decoded 2C02 monitor approximation.
     * Real CRT color depends on the composite decoder and will be modeled
     * separately if the project later needs waveform-level validation.
     */
    static const rgb8_t demo[64] = {
        /* $00-$0F */
        { 84, 84, 84}, {  0, 30,116}, {  8, 16,144}, { 48,  0,136},
        { 68,  0,100}, { 92,  0, 48}, { 84,  4,  0}, { 60, 24,  0},
        { 32, 42,  0}, {  8, 58,  0}, {  0, 64,  0}, {  0, 60,  0},
        {  0, 50, 60}, {  0,  0,  0}, {  0,  0,  0}, {  0,  0,  0},

        /* $10-$1F */
        {152,150,152}, {  8, 76,196}, { 48, 50,236}, { 92, 30,228},
        {136, 20,176}, {160, 20,100}, {152, 34, 32}, {120, 60,  0},
        { 84, 90,  0}, { 40,114,  0}, {  8,124,  0}, {  0,118, 40},
        {  0,102,120}, {  0,  0,  0}, {  0,  0,  0}, {  0,  0,  0},

        /* $20-$2F */
        {236,238,236}, { 76,154,236}, {120,124,236}, {176, 98,236},
        {228, 84,236}, {236, 88,180}, {236,106,100}, {212,136, 32},
        {160,170,  0}, {116,196,  0}, { 76,208, 32}, { 56,204,108},
        { 56,180,204}, { 60, 60, 60}, {  0,  0,  0}, {  0,  0,  0},

        /* $30-$3F */
        {236,238,236}, {168,204,236}, {188,188,236}, {212,178,236},
        {236,174,236}, {236,174,212}, {236,180,176}, {228,196,144},
        {204,210,120}, {180,222,120}, {168,226,144}, {152,226,180},
        {160,214,228}, {160,162,160}, {  0,  0,  0}, {  0,  0,  0},
    };

    return demo[code & 0x3fu];
}
