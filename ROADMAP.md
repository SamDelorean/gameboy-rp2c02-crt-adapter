# Roadmap

## Cross-cutting compatibility target — Game Boy DMG / SGB

The project is planned from the beginning as a **Game Boy DMG / SGB** video adapter rather than a DMG-only design with SGB support added later.

The common baseline is the Game Boy-compatible LCD/video source path. Wherever the required LCD data/timing signals and clock access are available, the same capture, buffering, scaling, palette and RP2C02 output architecture should be reusable.

SGB-specific palette signaling is an additional optional layer:

- reserve/passively observe `P14/P15` where available;
- support lightweight SGB palette recovery without making it mandatory for basic video;
- keep manual palette control fully functional;
- do not require full SGB emulation for version 1.

Every major hardware, firmware and validation phase below should therefore consider both DMG and SGB-capable source configurations where practical.

## Phase 1 — Freeze the digital controller

Select the final MCU/FPGA only after demonstrating:

- deterministic Game Boy LCD capture,
- deterministic RP2C02 EXT output,
- sufficient GPIO for the base DMG path plus optional `P14/P15`,
- at least 11.25 KiB of framebuffer RAM,
- practical low-cost prototyping,
- no pixel-rate interrupt bit-banging.

**Current leading candidate:** RP2040 / Raspberry Pi Pico.

## Phase 2 — Freeze the common clock source

Working targets:

- RP2C02 master: ~21.4772727 MHz,
- modified Game Boy source clock: ~4.2203555 MHz.

Both clocks should be derived from one reference to eliminate long-term relative drift.

The final clock-interface design must document the injection/isolation method for the actual DMG and any SGB source hardware used for validation.

## Phase 3 — Prove the PPU EXT concept on the bench

Before integrating Game Boy hardware:

- reset and minimally initialize the PPU,
- write known palette values,
- drive static `EXT0..EXT3` patterns,
- verify composite output,
- verify `/INT` / VBlank behavior,
- document oscilloscope captures and CRT/capture images.

This test will become the basis of the clone-PPU compatibility matrix.

## Phase 4 — Capture the Game Boy LCD stream

Validate the actual electrical/timing behavior of:

- `LD0`
- `LD1`
- `CP`
- `CPL`
- `ST`
- `S`

Confirm polarity, sample edge, active-pixel window, line boundary and frame boundary on real DMG hardware first.

Then verify that an available SGB/SGB-CPU-compatible source exposes an equivalent usable capture path, documenting any pinout, loading, level or timing differences rather than assuming identity.

## Phase 5 — Implement ping-pong buffering

Store complete 160x144x2-bit frames:

- 5,760 bytes per frame,
- 11,520 bytes total for two buffers.

Prioritize robust frame presentation over minimum memory use.

The framebuffer format and capture/output pipeline must remain source-agnostic so the same code path can serve DMG and SGB-compatible sources.

## Phase 6 — Implement aspect-correct fixed scaling

Baseline presentation:

```text
PPU raster: 256 x 240
11 border + 234-dot Game Boy image + 11 border
```

Scaling rules:

- vertical `144 -> 240` using exact `5/3` repetition,
- horizontal `160 -> 234` using deterministic integer nearest-neighbor repetition,
- 74 source-pixel duplications per line,
- no scaled framebuffer,
- no interpolation,
- no tearing.

The scaler operates only on the common 160x144 Game Boy shade image and therefore does not depend on whether the source is DMG or SGB-capable hardware.

## Phase 7 — Add curated palette presets and border generator

- one momentary button,
- initially 8 or 16 useful global palettes,
- VBlank-safe updates,
- border generator integrated with the output path,
- fixed black 11-dot side borders for the first release.

The border block should remain logically independent so later simple color/effect experiments do not alter scaling or framebuffer logic.

## Phase 8 — Validate SGB compatibility and add SGB-lite listening

Treat SGB as an explicit compatibility target, not merely a late optional feature.

Validate the common video path first, then passively monitor optional `P14/P15` and initially support direct palette commands:

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

Tasks:

- document the SGB source hardware/configuration used,
- verify the Game Boy video capture path,
- verify synchronized clock operation or document required differences,
- capture raw `P14/P15` traffic,
- decode supported packets,
- convert RGB555 colors to a suitable RP2C02 palette,
- verify one-button manual override,
- document games/configurations that do and do not emit usable passive SGB palette traffic.

Full SGB emulation, spatial attributes and graphical SGB borders remain outside version 1.

## Phase 9 — Freeze the integrated schematic

Publish and review:

- power/decoupling,
- Game Boy DMG / SGB source input conditioning,
- common clock generation,
- controller/programming,
- RP2C02 control bus,
- `EXT0..EXT3`,
- composite output,
- optional `P14/P15` SGB header/test points,
- palette button,
- debug/test points.

The schematic and connector tables must label which signals are required for the common DMG/SGB video path and which are optional SGB-lite signals.

## Phase 10 — Prototype and validate

Document reproducibly:

- clock measurements,
- DMG LCD captures,
- SGB-compatible source captures where hardware is available,
- aspect-correct scaling geometry,
- test patterns,
- palette/manual override behavior,
- passive SGB palette behavior,
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
- DMG installation guide,
- SGB/SGB-CPU installation or signal-access notes for validated configurations,
- validation procedure,
- compatibility matrix,
- finalized licensing notices.
