#include "rp2c02_nesemu.h"

#include "NESEmu/Ppu.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

constexpr unsigned kDotsPerLine = 341u;
constexpr unsigned kLinesPerFrame = 262u;
constexpr unsigned kFirstVisibleDot = 2u;
constexpr unsigned kLastVisibleDot = 257u;
constexpr const char *kPinnedRevision =
    "4966aa09259ef965d4b6bd2635a1dfe57a8569cb";

struct PpuBus {
    uint16_t address = 0;
    uint8_t data = 0;

    uint16_t getAddressBus() const { return address; }
    void setAddressBus(uint16_t value) { address = value; }

    uint8_t getDataBus() const { return data; }
    void setDataBus(uint8_t value) { data = value; }

    /* External VRAM is intentionally absent for the project's rendering-
       disabled EXT path. Keep deterministic zero/open memory if the donor core
       ever touches it while a diagnostic register combination is active. */
    void performRead() { data = 0; }
    void performWrite() {}
};

struct InterruptSink {
    void interrupt(bool high) { (void)high; }
};

struct GraphicSink {
    uint8_t (*out)[PPU_ACTIVE_W];

    explicit GraphicSink(uint8_t (*buffer)[PPU_ACTIVE_W]) : out(buffer) {}

    void notifyVBlankStarted() {}

    void plotPixel(unsigned x,
                   unsigned y,
                   uint8_t color,
                   bool emphasize_red,
                   bool emphasize_green,
                   bool emphasize_blue)
    {
        (void)emphasize_red;
        (void)emphasize_green;
        (void)emphasize_blue;
        if (x < PPU_ACTIVE_W && y < PPU_ACTIVE_H) {
            out[y][x] = color & 0x3fu;
        }
    }
};

struct CpuIoBus {
    uint16_t address = 0;
    uint8_t data = 0;

    uint16_t getAddressBus() const { return address; }
    uint8_t getDataBus() const { return data; }
    void setDataBus(uint8_t value) { data = value; }
};

using DonorPpu = NESEmu::Ppu::Chip<NESEmu::Ppu::Model::Ricoh2C02,
                                    PpuBus,
                                    InterruptSink,
                                    GraphicSink>;

struct Harness {
    PpuBus bus;
    InterruptSink interrupt;
    GraphicSink graphics;
    DonorPpu ppu;
    unsigned dot = 0;
    unsigned scanline = 0;

    explicit Harness(uint8_t (*out)[PPU_ACTIVE_W])
        : graphics(out), ppu(bus, interrupt, graphics)
    {
        ppu.powerUp();

        /* NESEmu applies the reset request at the end of a PPU clock. Dot 0
           produces no visible pixel, so this safely establishes the documented
           post-reset register state before any project data is presented. */
        tick();
    }

    void advance_position()
    {
        ++dot;
        if (dot == kDotsPerLine) {
            dot = 0;
            ++scanline;
            if (scanline == kLinesPerFrame) scanline = 0;
        }
    }

    void tick()
    {
        ppu.clock();
        advance_position();
    }

    void write_register(uint8_t reg, uint8_t value)
    {
        CpuIoBus io;
        io.address = reg & 7u;
        io.data = value;
        ppu.writePerformed(io);
        tick();
    }

    void set_ppu_address(uint16_t address)
    {
        write_register(RP2C02_REG_ADDR, (uint8_t)((address >> 8) & 0x3fu));
        write_register(RP2C02_REG_ADDR, (uint8_t)(address & 0xffu));

        /* The donor core models the delayed t -> v copy after $2006. */
        tick();
        tick();
    }

    void load_state(const rp2c02_ext_t &state)
    {
        /* Load palette RAM with rendering disabled and increment-by-one. */
        write_register(RP2C02_REG_CTRL, 0u);
        write_register(RP2C02_REG_MASK, 0u);
        for (unsigned i = 0; i < 32u; ++i) {
            /*
             * $3F10/$14/$18/$1C mirror $3F00/$04/$08/$0C on a 2C02.
             * Writing all 32 addresses linearly would therefore overwrite
             * four already-loaded base entries. Address each unique slot
             * explicitly and skip only those four mirrors.
             */
            if ((i & 0x13u) == 0x10u) continue;
            set_ppu_address((uint16_t)(0x3f00u + i));
            write_register(RP2C02_REG_DATA, state.palette_ram[i] & 0x3fu);
        }

        /* Restore the project-visible register state after setup. */
        write_register(RP2C02_REG_CTRL, state.ctrl);
        write_register(RP2C02_REG_MASK, state.mask);
        set_ppu_address(state.vram_address & 0x3fffu);
    }

    void sync_to_frame_start()
    {
        while (dot != 0u || scanline != 0u) tick();
    }

    void render(const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W])
    {
        const unsigned clocks = kDotsPerLine * kLinesPerFrame;
        for (unsigned i = 0; i < clocks; ++i) {
            if (scanline < PPU_ACTIVE_H &&
                dot >= kFirstVisibleDot && dot <= kLastVisibleDot) {
                const unsigned x = dot - kFirstVisibleDot;
                ppu.exts(ext[scanline][x] & 0x0fu);
            }
            else {
                ppu.exts(0u);
            }
            tick();
        }
    }
};

} // namespace

extern "C" int rp2c02_nesemu_render(
    const uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W],
    const rp2c02_ext_t *state,
    uint8_t out_code[PPU_ACTIVE_H][PPU_ACTIVE_W])
{
    if (!ext || !state || !out_code) return -1;

    std::memset(out_code, 0, PPU_ACTIVE_H * PPU_ACTIVE_W * sizeof(uint8_t));

    Harness harness(out_code);
    harness.load_state(*state);
    harness.sync_to_frame_start();
    harness.render(ext);
    return 0;
}

extern "C" const char *rp2c02_nesemu_revision(void)
{
    return kPinnedRevision;
}
