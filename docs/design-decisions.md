# Design Decision Record

This file is the current architectural decision register for the project. It distinguishes settled design intent from open choices and explicit non-goals.

## Status vocabulary

- **SET** — current design basis.
- **OPEN** — intentionally not frozen yet.
- **OPTIONAL** — supported/reserved, but not required for base operation.
- **DEFERRED** — possible later work outside version 1.
- **REJECTED FOR V1** — intentionally excluded from the first implementation.

## Architecture

### SET — Game Boy DMG / SGB compatibility is a core target

The project is a **Game Boy DMG / SGB** video adapter.

Common path:

```text
Game Boy DMG / SGB source
    -> deterministic 2-bit capture
    -> 160x144 source framebuffer
    -> fixed aspect-preserving scaler
    -> border/output composition
    -> EXT0..EXT3
    -> NES/Nintendo video PPU (RP2C02)
    -> composite NTSC
    -> CRT television
```

Source-specific differences belong at the source/clock interface, not in the scaler or output architecture.

### SET — RP2C02-compatible NES PPU is the final video generator

The PPU supplies raster timing, palette/color selection and composite NTSC. The project does not emulate a complete NES.

### SET — V0.1 schematic is partitioned into independent subsystems

The first connection-level schematic freezes only the central interconnect:

```text
Game Boy DMG / SGB taps -> Pico 2 / RP2350 -> RP2C02
```

Clock generation, final power implementation and downstream video-output circuitry are separate sheets/subsystems.

The central sheet uses off-sheet interfaces:

```text
PPU_CLK_IN      -> RP2C02 clock input (~21.4772727 MHz NTSC target)
GB_CLK_IN       -> Game Boy/SGB synchronized clock-injection point (~4.2203555 MHz current target)
PPU_VIDEO_RAW   <- RP2C02 pin 21 VOUT
PPU_5V          -> RP2C02 supply
PICO_VSYS       -> Pico 2 power input
GND             -> mandatory common reference
```

See [`../hardware/schematic-v0.1.md`](../hardware/schematic-v0.1.md) and [`../hardware/netlist-v0.1.csv`](../hardware/netlist-v0.1.csv).

### SET — V0.1 video boundary ends at raw RP2C02 VOUT

The central schematic does not define a finished 75-ohm television output. `PPU_VIDEO_RAW` is the handoff node to a later video-output sheet.

A minimal composite buffer may be documented later. Existing NES S-Video/HDMI/other downstream video modifications may be reusable independently, but they are outside the baseline capture/scaler/EXT architecture and are not assumed compatible until validated.

## Source interface

### SET — DMG reference capture signals

Baseline source signals are:

```text
LD0 LD1 CP CPL ST S GND
```

Exact sampling edge, voltage and loading remain bench-validated quantities.

### OPTIONAL — P14/P15 for SGB-lite

`P14/P15` are passive optional inputs for direct SGB palette-command listening. Their absence must not affect normal video or manual palettes.

### SET — SGB external clock replacement is an established design principle

For SGB hardware, the host-derived clock path may be interrupted/isolated and an external clock injected into the SGB-side node. This is treated as established project implementation knowledge; each exact board revision still needs its physical cut/injection point and electrical conditioning documented.

## Source image and buffering

### SET — Common 160x144x2-bit representation

The framebuffer stores the original four Game Boy shade indices, not RGB values.

```text
160 x 144 x 2 bits = 5,760 bytes/frame
2 frames = 11,520 bytes = 11.25 KiB
```

### SET — Two complete FRONT/BACK source buffers

BACK receives the incoming frame. FRONT is displayed. Roles change only at a complete-frame boundary. The displayed buffer is never modified in place.

### SET — Framebuffers are not a frame-rate converter

The common-clock architecture eliminates continuous source/output drift; the two buffers exist for ownership and tear-free presentation only.

## Scaling and geometry

### SET — Preserve Game Boy picture proportions

V1 uses:

```text
PPU raster:          256 x 240
left border:          11 samples
Game Boy image:      234 x 240
right border:         11 samples
```

The working NTSC PPU pixel-aspect model is approximately 8:7, making 234x240 a close physical match to the Game Boy logical 160:144 image geometry.

### SET — Vertical 144 -> 240 by 5/3 repetition

```text
L0 L1 L2 -> L0 L0 L1 L2 L2
```

### SET — Horizontal 160 -> 234 by deterministic repetition

Every source pixel is emitted once and exactly 74 are duplicated per line.

### SET — No scaled intermediate framebuffer

Neither 234x240 nor 256x240 is stored as a full output frame.

### REJECTED FOR V1 — Stretch/zoom/crop modes

V1 has one aspect-preserving presentation geometry.

## Border generator

### SET — Border generator is separate from scaler

```text
11 border + 234 image + 11 border = 256 samples
```

### SET — V1 border is fixed black

### DEFERRED — Simple future border colors/effects

Future low-complexity border behavior may be added without changing framebuffer or scaler.

## Timing and clocks

### SET — Shared timing reference

Working targets:

```text
RP2C02 master:          ~21.4772727 MHz
Game Boy source target: ~4.2203555 MHz
f_GB / f_PPU_master = 798 / 4061
```

Design objective:

```text
1 complete Game Boy source frame = 1 complete simplified RP2C02 frame
```

### OPEN — Exact common clock generator

Si5351A remains proposal 1. The common-reference architecture is SET; the exact clock IC is not frozen until frequency accuracy, jitter, duty cycle and startup are validated.

The V0.1 central schematic does not include this generator; it receives `PPU_CLK_IN` and the Game Boy/SGB receives `GB_CLK_IN` as off-sheet clock nets.

### SET — `/INT` / VBlank is the safe software boundary

Use PPU `/INT` for buffer presentation changes, palette writes, UI state and diagnostics after PPU initialization.

## Controller

### SET — RP2350 is the V1 controller family

**Raspberry Pi Pico 2 is the preferred prototype/module implementation.**

Selection criterion: minimum additional electronics while preserving deterministic I/O and easy construction.

Reasons:

- ample SRAM;
- PIO + DMA;
- current RP2350 digital GPIO are 5 V tolerant while powered, reducing/removing DMG input level-shifting hardware;
- Pico 2 exposes 26 GPIO and the optimized interface uses 23 including optional P14/P15;
- USB/SWD are available without consuming the remaining GPIO margin.

Use 5 V source inputs only on the RP2350/Pico 2 fault-tolerant digital GPIO group; ADC-capable GPIO26..28 are reserved for 3.3 V-only diagnostics/future functions.

### SET — Hardware-assisted deterministic pixel I/O

Capture and EXT output use PIO/DMA or equivalent hardware assistance; pixel-rate interrupt bit-banging is not the architecture.

## Minimal PPU CPU/register interface

### SET — V1 PPU host interface is write-only

V1 does not require CPU-side PPU reads. Startup waits through the documented warm-up interval and later uses `/INT` as the VBlank reference.

Required registers:

```text
$2000 PPUCTRL
$2001 PPUMASK
$2006 PPUADDR
$2007 PPUDATA
```

### SET — PPU `R/W` fixed LOW

Tie `R/W` to GND. `/CS` remains MCU-controlled.

### SET — PPU `/RESET` uses passive pull-up, not an MCU GPIO

Keep reset inactive with a pull-up and expose a test/reset pad that can force it low during bench work.

### SET — PPU A1 and A2 are tied together

For the four V1 registers, A1 and A2 always match. One MCU signal drives both:

```text
PAIR A0 = 00 -> $2000
PAIR A0 = 01 -> $2001
PAIR A0 = 10 -> $2006
PAIR A0 = 11 -> $2007
```

### SET — EXT0..EXT3 share MCU GPIO with PPU D0..D3

The same four RP2350 outputs drive both PPU EXT0..3 and CPU D0..3. Because V1 is write-only and `/CS` is inactive during normal pixel output, this removes four GPIO without external mux logic.

Bench validation must confirm loading/contention behavior on real PPU/clones.

### SET — Direct minimized interface preferred over shift registers

The former serialized/latch PPU-host proposal is not the V1 baseline because the RP2350/Pico 2 direct topology fits without those extra ICs.

### SET — RP2C02 external VRAM-side bus is unconnected in the V0.1 central sheet

The V0.1 architecture uses the PPU internal palette and keeps normal background/sprite rendering disabled. Therefore the external PPU memory-side pins (`ALE`, `AD0..AD7`, `A8..A13`, `/RD`, `/WR`) are marked NC on the first schematic rather than adding CHR/nametable memory hardware.

This remains a bench-validation item before PCB freeze; unused PPU outputs/bidirectional pins must not be tied together or to arbitrary rails.

## GPIO budget

```text
Game Boy capture                    6
shared EXT0..3 / PPU D0..3         4
PPU D4..7                           4
PPU A1+A2 pair + A0                 2
PPU /CS                             1
PPU /INT                            1
clock-generator I2C                 2
palette button                      1
-------------------------------------
mandatory DMG total                21
optional P14/P15                   +2
-------------------------------------
DMG / SGB planned total           23
```

Pico 2 therefore retains 3 GPIO for 3.3 V diagnostics/future use. GP20/GP21 remain reserved for the separate clock-generator sheet if I2C control is required.

## Passive/default-state policy

### SET — Use passive states where they safely remove control GPIO

- `R/W`: hard LOW.
- `/RESET`: pull HIGH.
- `/CS`: pull HIGH so the PPU stays deselected while MCU boots.
- `/INT`: pull to 3.3 V because the PPU output is open-drain.
- `PALETTE_BUTTON`: RP2350 internal pull-up, button to GND unless later EMC testing requires an external resistor.
- I2C SDA/SCL: normal external pull-ups to 3.3 V on the later clock sheet if used.

Do not add arbitrary pulls to source pixel/timing lines, P14/P15 or EXT lines without a measured reason.

## PPU and clone compatibility

### SET — Original Ricoh RP2C02 is not mandatory

Discrete NTSC-compatible clones may be used if they pass project-specific EXT, palette, reset, timing and composite-output tests.

### OPEN — Clone compatibility matrix

UA6528-class devices remain initial historical candidates.

## Palette behavior

### SET — Global four-color palette

Shade indices remain independent of palette.

### SET — One-button mode cycle

```text
AUTO/SGB -> manual preset 1 -> ... -> manual preset N -> AUTO/SGB
```

### SET — Manual user selection overrides incoming SGB palette traffic

While manual mode is active, SGB commands may be decoded/cached but may not alter the visible palette.

### OPEN — Exact number of manual presets

Current target: 8 or 16 curated palettes.

## Super Game Boy compatibility

### SET — Common SGB video-path compatibility is a core goal

SGB-compatible source hardware should use the same capture/buffer/scaler/output architecture once equivalent source signals are accessed.

### OPTIONAL — Passive SGB-lite commands

Initial direct commands:

```text
PAL01 PAL23 PAL03 PAL12
```

### REJECTED FOR V1 — Full SGB emulation

No active JOYP response, MLT_REQ feedback, regional attribute colorization, tile transfers or graphical SGB borders.

## NES rendering features

### REJECTED FOR V1 — Normal NES tile/sprite rendering

No NES CPU, CHR graphics, nametables, OAM or sprites are required. The PPU is a raster/palette/composite stage.

## Software philosophy

### SET — Keep source, framebuffer, scaler, border and palette separable

### SET — Prefer small deterministic state machines

### SET — Diagnostics remain maintained project assets

## Explicit project boundary

### SET — Direct CRT/yoke-deflection is outside this repository

### DEFERRED / SEPARATE — Alternative downstream NES video modifications

Alternative PPU video-output modifications may be documented by reference, but HDMI, S-Video, RGB-oriented or other downstream conversion hardware is not part of the central V1 schematic. The baseline handoff is `PPU_VIDEO_RAW`.

## Change discipline

When a SET decision changes:

1. update this register;
2. update the affected technical document(s);
3. record the reason in the project log/issue;
4. distinguish measurement from hypothesis;
5. do not leave obsolete implementation rules presented as current design.
