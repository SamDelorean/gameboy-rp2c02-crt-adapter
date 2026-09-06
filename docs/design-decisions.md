# Design Decision Record

This file consolidates the current architectural decisions of the project so contributors can distinguish **settled design intent**, **open choices**, and **explicit non-goals** without reconstructing the conversation history.

It is a living document. A decision may be revised later, but changes should be deliberate and documented.

## Status vocabulary

- **SET** — current design basis; implementation should follow it unless superseded by a documented decision.
- **OPEN** — intentionally not frozen yet.
- **OPTIONAL** — supported or reserved, but not required for basic operation.
- **DEFERRED** — useful future direction, outside version 1.
- **REJECTED FOR V1** — explicitly excluded from the first implementation.

## Architecture

### SET — RP2C02-class PPU as final video generator

The project uses an NTSC RP2C02-compatible PPU as:

- raster/timing generator,
- palette/color stage,
- composite NTSC output stage.

The project does not build or emulate a complete NES.

### SET — Digital bridge between DMG LCD bus and PPU EXT inputs

Base signal flow:

```text
DMG LCD signals
    -> capture
    -> source framebuffer
    -> fixed scaler
    -> EXT0..EXT3
    -> RP2C02-compatible PPU
    -> composite NTSC
    -> CRT
```

## Game Boy source hardware

### SET — Direct LCD-side capture

Required DMG-side signals are presently:

- `LD0`
- `LD1`
- `CP`
- `CPL`
- `ST`
- `S`
- GND/reference

Electrical levels, exact sample edge and active-pixel window must still be verified before freezing the production schematic.

### SET — Prefer damaged-display donor units for experimentation

Early prototypes should preferentially use a Game Boy DMG whose LCD is no longer reasonably repairable, provided the logic board and LCD timing/data signals remain healthy.

The project should avoid sacrificing complete, restorable consoles merely for development convenience.

## Source image and buffering

### SET — 160 x 144, 2-bit source representation

The framebuffer stores the original four DMG shade indices rather than RGB values.

### SET — Two complete source framebuffers

One source frame:

```text
160 x 144 x 2 bits = 5,760 bytes
```

Two buffers:

```text
11,520 bytes = 11.25 KiB
```

The design prioritizes robustness and clear buffer ownership over minimum RAM use.

### SET — Ping-pong ownership

- BACK receives the incoming frame.
- FRONT is displayed/scaled.
- FRONT and BACK exchange roles only at a defined complete-frame boundary.
- The output path must never read from a buffer while the capture path modifies it.

## Scaling

### SET — Full active PPU picture target: 256 x 240

Version 1 scales the full 160 x 144 DMG image to 256 x 240.

### SET — Nearest-neighbor repetition only

No bilinear/bicubic filtering, floating-point resampling or arbitrary-ratio scaler is required.

### SET — Horizontal 8/5 scaler

Five source pixels become eight output pixels using the center-phased repetition pattern:

```text
A B C D E -> A A B C C D E E
repeat:       2 1 2 1 2
```

The pattern repeats 32 times per line.

### SET — Vertical 5/3 scaler

Three source lines become five output lines:

```text
L0 L1 L2 -> L0 L0 L1 L2 L2
repeat:       2  1  2
```

The pattern repeats 48 times per frame.

### SET — No required 256 x 240 intermediate framebuffer

The baseline design scales while reading FRONT. A pre-scaled output framebuffer is not required unless later measurements show a compelling implementation advantage.

## Timing and clocks

### SET — Shared timing reference

The Game Boy and the RP2C02 should derive their operating clocks from a common reference to prevent accumulated frame drift.

Working targets currently documented:

```text
RP2C02 master: ~21.4772727 MHz
modified DMG:  ~4.2203555 MHz
```

These remain working engineering values until validated on the final hardware implementation.

### SET — One source frame per PPU frame

The architecture is designed so one completed DMG source frame corresponds to one PPU output frame.

### SET — Use PPU VBlank as safe software boundary

`/INT` / VBlank is the preferred event for:

- buffer presentation changes,
- palette writes,
- low-rate UI housekeeping,
- diagnostics.

## Controller

### OPEN — Final controller selection

Current leading candidate: **RP2040 / Raspberry Pi Pico**.

Reasons:

- sufficient SRAM,
- PIO,
- DMA,
- low cost,
- easy prototype assembly.

Alternatives retained until timing validation closes the choice:

- RP2350/Pico 2,
- suitable ESP32-class devices,
- practical FPGA solutions.

### SET — Deterministic hardware-assisted pixel I/O

Regardless of controller, the pixel stream should use PIO/DMA or an equivalent deterministic peripheral mechanism.

Pixel-rate interrupt-driven GPIO bit-banging is not the intended architecture.

## PPU and clone compatibility

### SET — Original Ricoh chip not mandatory

The project is intentionally compatible in concept with discrete NTSC PPU clones, salvaged or new-old-stock devices, provided they pass the project-specific validation procedure.

### SET — Compatibility is measured, not assumed

A clone must be checked for the functions this project actually uses:

- `EXT0..EXT3` as external inputs,
- palette RAM behavior,
- reset/init behavior,
- `/INT` / VBlank,
- NTSC timing,
- usable composite output,
- material palette/analog differences.

General ability to run NES games is insufficient proof.

### OPEN — Clone compatibility matrix

UA6528-class devices are historical candidates to validate. Other discrete clones may be added as real measurements become available.

Integrated NOAC/COB devices are not treated as pin-compatible substitutes unless their required PPU functions are actually accessible.

## Palette behavior

### SET — Global four-color palette

Version 1 maps the four DMG shade values to one global four-color PPU palette for the whole screen.

### SET — One-button user interface

A single pushbutton cycles through a curated set of complete palette presets.

### OPEN — Exact number of presets

Current practical target: **8 or 16** curated palettes.

Candidate families include:

- neutral grayscale,
- classic DMG green,
- amber,
- sepia,
- blue/cool palettes,
- selected SGB/GBC-inspired mappings.

### SET — Do not expose every mathematical palette combination

Useful, readable presets are preferred over exhaustive combinatorial palette selection.

### SET — Palette changes during a safe PPU interval

Palette writes are deferred to VBlank/safe timing rather than performed asynchronously during visible output.

## Overscan / border

### SET — Version 1 border/overscan is fixed black

This is a passive behavior, not another user menu option.

### DEFERRED — Alternative simple border behavior

Possible future firmware may derive border color from the active palette or repeat edge pixels/lines, but this is intentionally outside the initial implementation.

## Optional Super Game Boy support

### OPTIONAL — Passive SGB-lite listener

Reserve optional `P14` and `P15` inputs.

Initial direct palette commands of interest:

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

The received RGB555 colors may be converted to suitable RP2C02 palette colors.

### SET — Base operation does not depend on SGB

If P14/P15 are not connected or no valid SGB palette traffic is observed, normal manual palette operation continues unchanged.

### REJECTED FOR V1 — Full SGB emulation

Version 1 does not implement:

- active JOYP response,
- `MLT_REQ` feedback,
- regional attribute colorization,
- tile transfers,
- graphical SGB borders,
- complete SGB presence emulation.

### DEFERRED — Reserve additional joypad lines where cheap

PCB/test-point planning may reserve P10-P15 where doing so costs little, preserving the possibility of later active SGB experiments without making them a version-1 dependency.

## NES rendering features

### REJECTED FOR V1 — Normal background/sprite rendering

Version 1 does not need CHR graphics, nametables, OAM, sprites or a NES CPU.

Keeping normal rendering disabled preserves the simple timing model and avoids turning the project into a partial NES implementation.

## Software philosophy

### SET — Keep shade storage, scaling and palette independent

The framebuffer contains only DMG shade indices. Scaling duplicates indices. Palette selection is applied later by the PPU/output stage.

### SET — Prefer small deterministic state machines

The project favors transparent periodic logic over generalized graphics abstractions.

### SET — Diagnostics are production project assets

Bench modes such as color bars, checkerboards, line patterns, frame counters and timing markers should remain maintained because they form the reproducible validation procedure.

## Explicit project boundary

### SET — Direct CRT/yoke-deflection approach is not this repository's architecture

Earlier direct-deflection concepts are outside this branch. This repository documents the RP2C02-based design unless a future independent branch explicitly states otherwise.

## Change discipline

When a SET decision changes:

1. update this file;
2. update the affected technical document(s);
3. record the reason in the project log/issue;
4. distinguish new measurement from hypothesis;
5. avoid silently changing implementation behavior without updating the design basis.
