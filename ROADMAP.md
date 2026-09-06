# Roadmap

## Phase 1 — Freeze the digital controller

Select the final MCU/FPGA only after demonstrating:

- deterministic DMG LCD capture,
- deterministic RP2C02 EXT output,
- sufficient GPIO,
- at least 11.25 KiB of framebuffer RAM,
- practical low-cost prototyping,
- no pixel-rate interrupt bit-banging.

**Current leading candidate:** RP2040 / Raspberry Pi Pico.

## Phase 2 — Freeze the common clock source

Working targets:

- RP2C02 master: ~21.4772727 MHz,
- modified DMG: ~4.2203555 MHz.

Both clocks should be derived from one reference to eliminate long-term relative drift.

## Phase 3 — Prove the PPU EXT concept on the bench

Before integrating a Game Boy:

- reset and minimally initialize the PPU,
- write known palette values,
- drive static `EXT0..EXT3` patterns,
- verify composite output,
- verify `/INT` / VBlank behavior,
- document oscilloscope captures and CRT/capture images.

This test will become the basis of the clone-PPU compatibility matrix.

## Phase 4 — Capture the DMG LCD stream

Validate the actual electrical/timing behavior of:

- `LD0`
- `LD1`
- `CP`
- `CPL`
- `ST`
- `S`

Confirm polarity, sample edge, active-pixel window, line boundary, and frame boundary on real hardware.

## Phase 5 — Implement ping-pong buffering

Store complete 160x144x2-bit frames:

- 5,760 bytes per frame,
- 11,520 bytes total for two buffers.

Prioritize robust frame presentation over minimum memory use.

## Phase 6 — Implement fixed scaling

Map 160x144 to 256x240:

- horizontal ratio `8/5`,
- vertical ratio `5/3`,
- nearest-neighbor,
- deterministic full-raster coverage,
- no tearing.

## Phase 7 — Add curated palette presets

- one momentary button,
- initially 8 or 16 useful global palettes,
- VBlank-safe updates,
- fixed black overscan for the first release.

## Phase 8 — Add optional SGB-lite listening

Passively monitor `P14/P15` and initially support only direct palette commands:

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

Convert RGB555 source colors to a suitable RP2C02 palette.

## Phase 9 — Freeze the integrated schematic

Publish and review:

- power/decoupling,
- DMG input conditioning,
- common clock generation,
- controller/programming,
- RP2C02 control bus,
- `EXT0..EXT3`,
- composite output,
- optional SGB header,
- palette button,
- debug/test points.

## Phase 10 — Prototype and validate

Document reproducibly:

- clock measurements,
- LCD captures,
- test patterns,
- composite waveform,
- CRT photographs,
- clone-PPU results,
- known limitations.

## Phase 11 — Reproducible hardware release

Release:

- KiCad project sources,
- BOM,
- fabrication outputs,
- firmware source and binaries,
- programming instructions,
- installation guide,
- validation procedure,
- finalized licensing notices.
