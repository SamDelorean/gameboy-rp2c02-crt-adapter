#ifndef GBCRT_RP2C02_TIMING_H
#define GBCRT_RP2C02_TIMING_H

#include <stdbool.h>
#include <stdint.h>

#define RP2C02_DOTS_PER_SCANLINE 341u
#define RP2C02_SCANLINES_PER_FRAME 262u
#define RP2C02_VISIBLE_DOTS 256u
#define RP2C02_VISIBLE_SCANLINES 240u
#define RP2C02_VBLANK_START_SCANLINE 241u
#define RP2C02_PRE_RENDER_SCANLINE 261u
#define RP2C02_FRAME_DOTS (RP2C02_DOTS_PER_SCANLINE * RP2C02_SCANLINES_PER_FRAME)

typedef enum {
    RP2C02_TIMING_NONE         = 0,
    RP2C02_TIMING_VISIBLE_DOT  = 1u << 0,
    RP2C02_TIMING_VBLANK_START = 1u << 1,
    RP2C02_TIMING_VBLANK_END   = 1u << 2,
    RP2C02_TIMING_FRAME_WRAP   = 1u << 3,
} rp2c02_timing_event_t;

typedef struct {
    uint16_t dot;
    uint16_t scanline;
    uint64_t frame;
    bool vblank;
} rp2c02_timing_t;

void rp2c02_timing_reset(rp2c02_timing_t *timing);

/*
 * Process the current PPU dot, return event flags, then advance exactly one
 * PPU dot. The model intentionally assumes rendering-disabled timing, so the
 * NTSC odd-frame skipped dot is not applied.
 */
unsigned rp2c02_timing_step(rp2c02_timing_t *timing);

/* In the project's write-only host model PPUSTATUS is never read, so the
 * VBlank flag remains set from scanline 241 dot 1 until pre-render dot 1.
 * /INT is open-drain active-low on hardware; this helper returns the logical
 * asserted state, not the electrical pin level. */
bool rp2c02_timing_nmi_asserted(const rp2c02_timing_t *timing,
                                 bool nmi_enabled);

#endif
