# Design Decision Record

This file is the current architectural decision register for the project. It distinguishes settled design intent from open choices and explicit non-goals.

## Status vocabulary

- **SET** — current design basis.
- **OPEN** — intentionally not frozen yet.
- **OPTIONAL** — supported/reserved, but not required for base operation.
- **DEFERRED** — possible later work outside version 1.
- **REJECTED FOR V1** — intentionally excluded from the first implementation.

## Architecture

### SET — Game Boy DMG / SGB compatibility is a core project target

The project is planned as a **Game Boy DMG / SGB** video adapter.

DMG is the first reference platform for bench validation, but SGB/SGB-CPU-compatible source hardware is part of the architecture plan rather than a future unrelated extension.

The intended common path is:

```text
Game Boy DMG / SGB video source
    -> deterministic capture
    -> 160 x 144 x 2-bit source framebuffer
    -> fixed aspect-preserving scaler
    -> border/output composition
    -> EXT0..EXT3
    -> NES/Nintendo video PPU (RP2C02)
    -> composite NTSC
    -> CRT television
```

Source-specific electrical differences should be confined to the input/clock-interface layer wherever possible.

See [`source-compatibility.md`](source-compatibility.md).

### SET — NES/Nintendo video PPU as final video generator

The project uses an NTSC **RP2C02-compatible PPU** from the Nintendo Entertainment System / NES as the final video stage:

- raster/timing generator,
- palette/color stage,
- composite NTSC source.

The project does not build or emulate a complete NES.

## Game Boy source hardware

### SET — Direct Game Boy video-side capture

Required source signals on the DMG reference platform are presently:

- `LD0`
- `LD1`
- `CP`
- `CPL`
- `ST`
- `S`
- GND/reference

For SGB/SGB-CPU-compatible source hardware, equivalent accessible signals must be identified and electrically validated before that exact configuration is declared supported.

Exact voltage levels, sample edge, active-pixel window and loading remain subject to bench validation.

### OPTIONAL — P14/P15 for SGB-lite

`P14` and `P15` are optional passive inputs for SGB palette-command listening.

They are not required for the common DMG / SGB video path, framebuffer, scaler, composite output or manual palette selection.

### SET — Prefer damaged-display DMG donor units for experimentation

Early DMG prototypes should preferentially use a Game Boy whose LCD is no longer reasonably repairable, provided the logic board and LCD timing/data signals remain healthy.

Restorable consoles should not be sacrificed merely for development convenience.

## Source image and buffering

### SET — Common 160 x 144, 2-bit source representation

The framebuffer stores the original four Game Boy shade indices, not RGB values.

```text
160 x 144 x 2 bits = 5,760 bytes/frame
```

This representation is common to DMG and validated SGB-compatible sources once capture is complete.

### SET — Two complete source framebuffers

Version 1 uses FRONT/BACK ping-pong buffers:

```text
2 x 5,760 = 11,520 bytes = 11.25 KiB
```

- BACK receives the incoming frame.
- FRONT is read by the display/scaler path.
- Roles change only at a defined complete-frame boundary.
- The displayed buffer is never modified in place.

### SET — Framebuffers are not a frame-rate converter

The two source buffers exist for clean ownership, complete-frame presentation and tear-free handoff.

They are not intended to absorb continuous drift between unrelated video clocks. Source/output cadence is synchronized by the common clock architecture.

## Scaling and picture geometry

### SET — Preserve Game Boy picture proportions

The earlier proposal to stretch the Game Boy image across the full `256 x 240` active PPU raster is superseded.

Version 1 fills the available vertical height while preserving the apparent Game Boy image proportions on the NTSC CRT.

Baseline output:

```text
PPU active raster: 256 x 240
left border:         11 samples
Game Boy picture:   234 x 240
right border:        11 samples
```

The working RP2C02 NTSC pixel-aspect model is approximately `8:7`. With that non-square geometry, `234 x 240` closely preserves the Game Boy logical `160/144 = 10/9` picture ratio while allowing equal side borders.

See [`scaling.md`](scaling.md).

### SET — Vertical scaler: 144 -> 240 by 5/3 repetition

```text
L0 L1 L2 -> L0 L0 L1 L2 L2
repeat:       2  1  2
```

The pattern repeats 48 times per frame.

### SET — Horizontal scaler: 160 -> 234 by deterministic integer repetition

Every source pixel is emitted at least once and exactly 74 source pixels are duplicated:

```text
160 + 74 = 234
```

A centered integer error accumulator, fixed table or equivalent deterministic state machine may be used.

### SET — No scaled intermediate framebuffer required

Neither a `234 x 240` nor a `256 x 240` framebuffer is required. Scaling and border generation occur while reading FRONT.

### REJECTED FOR V1 — Multiple user-selectable scaling modes

Version 1 has one presentation geometry: aspect-preserving, vertically filled, pillarboxed output.

Stretch/zoom/crop modes are outside the initial scope.

## Border / output composition

### SET — Border generator is logically separate from the scaler

Each visible line is composed as:

```text
11 border + 234 scaled image + 11 border = 256 samples
```

The border samples are not stored in the source framebuffer and are not generated by the scaling algorithm itself.

### SET — Version 1 border value is fixed black

Black is the only version-1 behavior.

### DEFERRED — Simple future border colors/effects

A later revision may allow a different fixed color, palette-related color or another low-complexity effect inside the border generator.

Such behavior must not alter the source framebuffer or aspect-correct scaler.

## Timing and clocks

### SET — Shared timing reference

The Game Boy source and RP2C02 derive their clocks from one reference so the frame domains do not accumulate relative drift.

Working targets based on the DMG timing model:

```text
RP2C02 master: ~21.4772727 MHz
Game Boy source target: ~4.2203555 MHz
```

Working relationship:

```text
f_GB / f_PPU_master = 798 / 4061
```

These remain engineering target values until validated on final hardware.

For SGB source hardware, the same system-level objective applies, but exact clock access/injection must be validated for the actual configuration rather than assumed identical to DMG.

### SET — Synchronize the source instead of building asynchronous frame-rate conversion

The Game Boy source clock is adapted so that the design objective is:

```text
1 complete Game Boy source frame
        =
1 complete simplified RP2C02 frame
```

This avoids generalized asynchronous frame-rate conversion, periodic frame drop/duplication, deep timing-absorption buffers and a synchronization framebuffer at output resolution.

### OPEN — Clock hardware proposal 1: Si5351A

The first implementation proposal is one Si5351A from a common crystal/reference:

```text
common reference
      |
   Si5351A
      |
  +---+---+
  |       |
CLK0     CLK1
  |       |
  v       v
PPU   Game Boy source
~21.4772727 MHz   ~4.2203555 MHz target
```

The common-reference architecture is SET. The exact Si5351A implementation remains OPEN until bench validation.

A `74AHCT125`-class buffer/interface is a current candidate where isolation or drive is required, but the final component is not frozen.

The original source clock must be isolated/disabled before an external synchronized clock is applied.

### SET — PPU VBlank as safe software boundary

`/INT` / VBlank is the preferred interval for:

- FRONT/BACK presentation changes,
- palette writes,
- low-rate user-interface updates,
- diagnostics.

## Controller

### OPEN — Final controller selection

Current leading candidate: **RP2040 / Raspberry Pi Pico** because of SRAM, PIO, DMA, low cost and easy prototyping.

The final device must support deterministic capture/output plus optional P14/P15 inputs without requiring a separate SGB firmware architecture.

### SET — Hardware-assisted deterministic pixel I/O

Pixel capture/output should use PIO/DMA or an equivalent deterministic peripheral mechanism. Pixel-rate GPIO bit-banging from ordinary interrupts is not the intended architecture.

## PPU and clone compatibility

### SET — Original Ricoh chip not mandatory

Discrete NTSC-compatible PPU clones may be used if they pass project-specific tests.

### SET — Compatibility is measured, not assumed

A candidate must be checked for:

- `EXT0..EXT3` external-input operation,
- palette RAM behavior,
- reset/register behavior,
- `/INT` / VBlank,
- NTSC timing,
- usable composite output,
- significant palette/analog differences.

Running NES games is not sufficient evidence for this project's unusual EXT-input use case.

### OPEN — Clone compatibility matrix

UA6528-class devices are initial historical candidates. Other discrete clones may be added as measured data becomes available.

## Palette behavior

### SET — Global four-color palette

The four Game Boy shades map to one global four-color PPU palette.

The framebuffer and scaler remain palette-independent.

### SET — One-button user interface

The single button cycles:

```text
AUTO/SGB -> manual preset 1 -> manual preset 2 -> ... -> manual preset N -> AUTO/SGB
```

### OPEN — Exact number of manual presets

Current target: 8 or 16 curated palettes.

### SET — User selection overrides SGB palette traffic

If an SGB-derived palette is visible, one button press leaves AUTO/SGB and selects the first manual preset.

While a manual preset is active, P14/P15 traffic may still be decoded/cached but cannot replace the visible palette.

### SET — Palette writes occur during a safe PPU interval

Visible palette changes are applied during VBlank/safe timing.

## Super Game Boy compatibility

### SET — Common SGB video compatibility is planned independently of SGB-lite

The project intends the common capture/buffer/scaler/output path to operate with SGB/SGB-CPU-compatible source hardware where the equivalent Game Boy video and clock signals are accessible.

Each exact configuration must be bench validated and documented.

### OPTIONAL — Passive P14/P15 SGB-lite listener

Initial direct commands of interest:

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

Received RGB555 colors may be converted to suitable RP2C02 colors.

Failure to receive these commands must not affect normal video or manual palettes.

### REJECTED FOR V1 — Full SGB emulation

Version 1 does not implement active JOYP response, `MLT_REQ` feedback, regional attribute colorization, tile transfers or graphical SGB borders.

## NES rendering features

### REJECTED FOR V1 — Normal NES background/sprite rendering

Version 1 does not need CHR graphics, nametables, OAM, sprites or a NES CPU. The PPU is used as a video/raster/palette/composite stage, not as part of a partial NES implementation.

## Software philosophy

### SET — Keep source interface, shade storage, scaling and palette separable

- source driver: handles DMG/SGB-specific electrical/timing details;
- framebuffer: original 2-bit Game Boy shade indices;
- scaler: deterministic repetition of those indices;
- border generator: fills non-image samples;
- palette: applied later by the PPU/output stage.

### SET — Prefer small deterministic state machines

Transparent integer state machines are preferred over generalized graphics abstractions, interpolation or floating-point video processing.

### SET — Diagnostics are project assets

Color bars, checkerboards, line patterns, frame counters, VBlank indicators, border geometry markers and timing markers should remain maintained as reproducible validation tools.

## Explicit project boundary

### SET — Direct CRT/yoke-deflection is outside this repository

This repository documents the RP2C02-based Game Boy-to-CRT design.

## Change discipline

When a SET decision changes:

1. update this register;
2. update the affected technical document(s);
3. record the reason in the project log/issue;
4. distinguish measurement from hypothesis;
5. do not leave obsolete implementation rules presented as current design.
