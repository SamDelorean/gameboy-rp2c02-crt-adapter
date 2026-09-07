# Hardware Interfaces — Draft

This document defines the physical/logical signal boundary between the **Game Boy DMG / SGB** source, the RP2350/Pico 2 controller, the RP2C02-compatible PPU, the common clock subsystem, and the user interface.

The controller choice and GPIO budget are maintained in [`../docs/controller-selection.md`](../docs/controller-selection.md).

## Baseline controller

Version 1 now targets **RP2350**, with **Raspberry Pi Pico 2** as the preferred prototype/module implementation.

The choice is driven by minimum external electronics:

- PIO + DMA for deterministic capture/output;
- enough SRAM for the two 160x144x2-bit buffers;
- current RP2350 digital GPIO are 5 V tolerant while powered, allowing direct reception of 5 V Game Boy signals on the fault-tolerant GPIO group;
- 26 GPIO exposed on Pico 2, sufficient after the no-IC pin-saving scheme below.

Do not route 5 V source inputs to Pico 2 GPIO26..28 because those are ADC-capable pins and are not part of the same 5 V-tolerant digital group.

## Optimized interconnection matrix

### Game Boy DMG / SGB source to controller

| Signal | Source | Destination | MCU direction | Timing | V1 use | Electrical plan |
|---|---|---|---|---|---|---|
| `LD0` | DMG/SGB video source | RP2350 | input | pixel-rate | shade bit 0 | direct to 5 V-tolerant digital GPIO after source measurement |
| `LD1` | DMG/SGB video source | RP2350 | input | pixel-rate | shade bit 1 | same |
| `CP` | DMG/SGB video source | RP2350 | input | pixel-rate | pixel sampling/qualification | same |
| `CPL` | DMG/SGB video source | RP2350 | input | line timing | capture timing | retain until bench proves redundant |
| `ST` | DMG/SGB video source | RP2350 | input | line timing | line start | retain until bench proves redundant |
| `S` | DMG/SGB video source | RP2350 | input | frame timing | frame reference | retain |
| `P14` | SGB-capable source | RP2350 | input | protocol | optional SGB-lite | passive input only; do not add loading pull unless measured necessary |
| `P15` | SGB-capable source | RP2350 | input | protocol | optional SGB-lite | same |

The downstream image path is common once a valid 160x144x2-bit frame has been reconstructed.

## RP2350 to RP2C02 external-pixel interface

The four EXT lines remain direct, timing-critical outputs:

```text
EXT0
EXT1
EXT2
EXT3
```

They should occupy four contiguous RP2350 GPIO for one PIO output group.

### Shared EXT / CPU-data nets

Version 1 reuses those same four MCU pins for the low nibble of the PPU CPU data bus:

```text
RP2350 GPIO -> RP2C02 EXT0 and CPU D0
RP2350 GPIO -> RP2C02 EXT1 and CPU D1
RP2350 GPIO -> RP2C02 EXT2 and CPU D2
RP2350 GPIO -> RP2C02 EXT3 and CPU D3
```

This saves four MCU GPIO without adding a multiplexer.

The topology depends on the V1 write-only PPU interface:

- PPU `R/W` is fixed LOW;
- CPU `D0..D7` are never intentionally driven by the PPU toward the MCU;
- `/CS` is inactive during normal EXT pixel output;
- low-rate register writes occur during a safe interval, preferably VBlank.

Bench validation must confirm that the selected RP2C02/clone presents no unexpected contention or loading.

## RP2C02 write-only CPU/register interface

### Data

```text
D0..D3  shared with EXT0..EXT3
D4..D7  four dedicated RP2350 outputs
```

### Address reduction

V1 needs only:

```text
$2000 PPUCTRL   000
$2001 PPUMASK   001
$2006 PPUADDR   110
$2007 PPUDATA   111
```

For these addresses, `A1` and `A2` are always equal. Therefore they are tied together physically and controlled by one MCU signal:

```text
PPU_A12_PAIR -> RP2C02 A1 + A2
PPU_A0       -> RP2C02 A0
```

The resulting two-bit register selection is:

```text
PAIR A0 = 00 -> $2000
PAIR A0 = 01 -> $2001
PAIR A0 = 10 -> $2006
PAIR A0 = 11 -> $2007
```

### Bus control

| PPU pin | V1 connection | Reason |
|---|---|---|
| `R/W` | tied LOW / GND | V1 is write-only |
| `/CS` | one RP2350 output plus safe pull-up | qualifies every register write; remains high during normal output |
| `/RESET` | pull-up to +5 V plus test/reset pad | no dedicated MCU pin; PPU powers up with reset inactive |
| `/INT` | direct RP2350 input, pulled up to 3.3 V | open-drain VBlank reference |

The firmware waits through the PPU warm-up interval before relying on writes and then enables `/INT`/NMI through PPUCTRL. `PPUSTATUS` reads are not required by the V1 baseline.

## Passive/default-state plan

The following passive states deliberately replace MCU GPIO or extra logic.

| Net | Default hardware state | Purpose |
|---|---|---|
| PPU `R/W` | hard LOW | permanent write direction |
| PPU `/RESET` | pull-up to +5 V | defined inactive reset; test pad can force LOW |
| PPU `/CS` | pull-up to validated HIGH rail | keeps host interface deselected while RP2350 boots/resets |
| PPU `/INT` | pull-up to 3.3 V | required for open-drain output and MCU-safe input level |
| `PALETTE_BUTTON` | RP2350 internal pull-up, button to GND | removes external button resistor unless EMC testing says otherwise |
| I2C `SDA/SCL` | external pull-ups to 3.3 V | normal reliable I2C bus requirement |

Do **not** add arbitrary pulls to `LD0`, `LD1`, `CP`, `CPL`, `ST`, `S`, `P14`, `P15`, or `EXT0..EXT3` merely to define idle states. These are active interfaces and extra loading must be justified by measurement.

## Controller GPIO budget

```text
Game Boy capture                    6
EXT0..3 / PPU D0..3 shared         4
PPU D4..D7                          4
PPU A1+A2 pair + A0                 2
PPU /CS                             1
PPU /INT                            1
clock-generator I2C                 2
palette button                      1
-------------------------------------
required DMG total                 21

optional P14/P15                   +2
-------------------------------------
planned DMG / SGB total            23
```

Pico 2 exposes 26 multifunction GPIO, leaving three GPIO for diagnostics/future 3.3 V-only functions.

## First Pico 2 pin map

This is a routing target, not yet PCB-frozen:

```text
GPIO0   LD0
GPIO1   LD1
GPIO2   CP
GPIO3   CPL
GPIO4   ST
GPIO5   S

GPIO6   EXT0 / PPU D0
GPIO7   EXT1 / PPU D1
GPIO8   EXT2 / PPU D2
GPIO9   EXT3 / PPU D3

GPIO10  PPU D4
GPIO11  PPU D5
GPIO12  PPU D6
GPIO13  PPU D7
GPIO14  PPU A1+A2 pair
GPIO15  PPU A0
GPIO16  PPU /CS
GPIO17  PPU /INT
GPIO18  P14 optional
GPIO19  P15 optional
GPIO20  I2C SDA
GPIO21  I2C SCL
GPIO22  palette button

GPIO26..28  3.3 V-only diagnostics / future use
```

The allocation deliberately makes the two source pixel bits and the four EXT outputs contiguous for PIO.

## Important non-MCU signal paths

| Signal | Origin | Destination | Function |
|---|---|---|---|
| `PPU_CLK` | common clock generator | RP2C02 | ~21.4772727 MHz master clock |
| `GB_SYNC_CLK` | common clock generator | DMG / SGB clock injection | working target ~4.2203555 MHz |
| `COMPOSITE_OUT` | RP2C02 | CRT/video connector | NTSC composite video |
| `GND` | common reference | all blocks | electrical reference |

Neither video output nor the two generated clocks consume MCU GPIO.

## Common clock subsystem — proposal 1

The architectural requirement remains one common reference. The first implementation proposal remains Si5351A or equivalent:

```text
common reference
      |
   Si5351A
      |
  +---+---+
  |       |
PPU_CLK  GB_SYNC_CLK
  |       |
RP2C02  Game Boy DMG / SGB
```

The clock-generator IC itself is still OPEN pending frequency/jitter validation. The controller selection does not depend on Si5351A specifically.

For SGB, interruption of the SNES-derived clock path and external clock injection are treated as an established modification principle; see [`../docs/sgb-clock-injection.md`](../docs/sgb-clock-injection.md).

## Border/output composition

The controller emits each visible line as:

```text
11 border + 234 scaled Game Boy image + 11 border = 256 samples
```

The border generator is separate from the scaler. V1 uses safe black; future simple colors/effects can be added without changing the framebuffer or scaler.

## User interface

One momentary `PALETTE_BUTTON` cycles:

```text
AUTO/SGB -> manual preset 1 -> ... -> manual preset N -> AUTO/SGB
```

If an SGB-derived palette is visible, one press exits AUTO/SGB and selects the first manual preset. Manual selection has priority over later P14/P15 traffic.

The button is not timing-critical and should use firmware debounce.

## Electrical validation before PCB freeze

Measure and record:

1. actual DMG/SGB signal amplitudes and thresholds;
2. RP2C02/clone acceptance of 3.3 V on `EXT`, CPU data, address and `/CS` inputs;
3. loading/contension on shared `EXT0..3` / `D0..3` nets;
4. `/CS` pull-up rail and reset-state behavior;
5. `/INT` open-drain operation with 3.3 V pull-up;
6. PPU `/RESET` behavior with passive pull-up;
7. I2C pull-up values and domains;
8. clock waveform, jitter, duty cycle and startup behavior;
9. PIO/DMA operation with the proposed pin grouping.

If a buffer or level translator is later required, it should be added only for the measured interface that needs it, not as a blanket assumption across the board.
