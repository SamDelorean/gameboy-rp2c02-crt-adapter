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

## Phase 1 — Digital controller — COMPLETE AT DESIGN LEVEL

**V1 decision: RP2350, with Raspberry Pi Pico 2 as the preferred prototype/module implementation.**

The selected baseline satisfies:

- sufficient SRAM for the two 160x144x2-bit source framebuffers;
- enough GPIO for the optimized direct DMG/SGB interface;
- PIO + DMA for deterministic capture/output;
- 5 V-tolerant digital inputs on the appropriate RP2350 GPIO, reducing blanket level-shifting hardware;
- simple Arduino-Pico/USB development workflow.

The earlier RP2040 / Raspberry Pi Pico candidate is retained only as historical comparison and is not an open design choice.

Bench validation of pin loading, voltage thresholds and timing is still required before PCB freeze.

## Phase 2 — Freeze the common clock source — OPEN

Working targets:

- RP2C02 master: ~21.4772727 MHz,
- modified Game Boy source clock: ~4.2203555 MHz.

Both clocks should be derived from one reference to eliminate long-term relative drift.

The common-reference architecture is SET. Si5351A remains proposal 1; the exact generator IC and output conditioning remain open pending frequency/jitter/edge-quality validation.

The final clock-interface design must document the physical injection/isolation point for the actual DMG and each SGB source hardware revision used for validation. SGB external clock replacement itself is already treated as an established project design principle.

## Phase 3 — Prove the PPU EXT concept on the bench — PENDING BENCH

Before integrating Game Boy hardware:

- reset and minimally initialize the PPU,
- write known palette values,
- drive static `EXT0..EXT3` patterns,
- verify composite output,
- verify `/INT` / VBlank behavior,
- document oscilloscope captures and CRT/capture images.

The V0.2 firmware already provides the low-rate PPU initialization/palette path and static EXT test state needed for this bring-up.

This test will become the basis of the clone-PPU compatibility matrix.

## Phase 4 — Capture the Game Boy LCD stream — THEORETICAL V0.2 IMPLEMENTED / BENCH PENDING

Firmware V0.2 now contains a PIO + DMA capture engine based on documented DMG LCD behavior. It reconstructs the 160x144x2-bit source frame into BACK while preserving FRONT/BACK ownership.

Bench work must validate the actual electrical/timing behavior of:

- `LD0`
- `LD1`
- `CP`
- `CPL`
- `ST`
- `S`

Confirm polarity, sampling phase, active-pixel behavior, line boundary, frame boundary, fine-scroll/suppressed-clock cases and electrical levels on real DMG hardware first.

Then verify that an available SGB/SGB-CPU-compatible source exposes an equivalent usable capture path, documenting any pinout, loading, level or timing differences rather than assuming identity.

See `firmware/capture-engine.md`.

## Phase 5 — Ping-pong buffering — IMPLEMENTED IN V0.2 / VALIDATION PENDING

The firmware allocates complete 160x144x2-bit FRONT/BACK frames:

- 5,760 bytes per frame,
- 11,520 bytes total for two buffers.

A BACK buffer awaiting PPU VBlank presentation is never overwritten; a newer raw capture is dropped instead.

The framebuffer format and capture/output pipeline remain source-agnostic so the same code path can serve DMG and validated SGB-compatible sources.

## Phase 6 — Aspect-correct fixed scaling — ALGORITHM/TABLES IMPLEMENTED / EXT ENGINE PENDING

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

Firmware V0.2 generates and self-checks the repetition tables. The remaining work is to consume FRONT through the deterministic EXT PIO/DMA output engine.

## Phase 7 — Add curated palette presets and border generator — PARTIAL

- one momentary button,
- initially 8 or 16 useful global palettes,
- VBlank-safe updates,
- border generator integrated with the output path,
- fixed black 11-dot side borders for the first release.

The button state machine and provisional bring-up palettes exist in firmware V0.2. Final curated palette values and the deterministic EXT/border output engine remain pending.

The border block remains logically independent so later simple color/effect experiments do not alter scaling or framebuffer logic.

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
- verify synchronized clock operation and exact physical injection point,
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
- RP2350/Pico 2 controller/programming,
- RP2C02 control bus,
- `EXT0..EXT3`,
- composite output,
- optional `P14/P15` SGB header/test points,
- palette button,
- debug/test points.

The V0.1 central interconnect is already documented; this phase closes the remaining clock, power and output sheets and converts the design to production-ready KiCad sources.

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
- Arduino-Pico programming instructions,
- DMG installation guide,
- SGB/SGB-CPU installation or signal-access notes for validated configurations,
- validation procedure,
- compatibility matrix,
- finalized licensing notices.
