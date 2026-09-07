# Interconnect Schematic V0.1 — Game Boy DMG / SGB → Pico 2 → RP2C02

This is the first **connection-level schematic basis** for the project. It intentionally covers only the Game Boy source interface, the RP2350/Pico 2 digital bridge, and the RP2C02-compatible PPU.

It does **not** yet include the clock-generator circuit or the final analog/video-output circuit. Those are separate sheets/subsystems.

## Sheet boundaries

Off-sheet inputs / outputs:

```text
PPU_CLK_IN       -> external clock-generator sheet -> RP2C02 pin 18
                   target for NTSC RP2C02: ~21.4772727 MHz

GB_CLK_IN        -> external clock-generator sheet -> Game Boy DMG / SGB clock-injection point
                   current synchronized target: ~4.2203555 MHz

PPU_VIDEO_RAW    <- RP2C02 pin 21 VOUT
                   raw/unbuffered composite source; downstream video stage not defined here

PPU_5V           -> power subsystem -> RP2C02 +5 V
PICO_VSYS        -> power subsystem / USB / carrier implementation -> Pico 2 VSYS
GND              -> common reference for Game Boy, Pico 2 and PPU
```

The project deliberately leaves `PPU_VIDEO_RAW` as a handoff node. A stock-style composite amplifier, S-Video-related modification, HDMI-oriented modification, or another downstream video implementation may be evaluated independently. Such downstream circuitry is outside the V0.1 interconnect sheet and is not implied to be automatically compatible until validated.

## Functional schematic

```text
                       +---------------- CLOCK SUBSYSTEM (separate) ----------------+
                       |                                                             |
                       |       PPU_CLK_IN ~21.4772727 MHz      GB_CLK_IN ~4.2203555 MHz
                       |                 |                              |
                       +-----------------|------------------------------|-------------+
                                         |                              |
                                         v                              v
                                  RP2C02 pin 18                Game Boy clock injection

 GAME BOY DMG / SGB                   RASPBERRY PI PICO 2                     RP2C02
 source-side taps                        (RP2350)                         NTSC composite PPU

 LD0 -------------------------------> GP0  (hdr pin 1)
 LD1 -------------------------------> GP1  (hdr pin 2)
 CP  -------------------------------> GP2  (hdr pin 4)
 CPL -------------------------------> GP3  (hdr pin 5)
 ST  -------------------------------> GP4  (hdr pin 6)
 S   -------------------------------> GP5  (hdr pin 7)

 P14 -- optional --------------------> GP18 (hdr pin 24)
 P15 -- optional --------------------> GP19 (hdr pin 25)

                                      GP6  (hdr pin 9)  ---- EXT0_D0 ----+---- pin 14 EXT0
                                                                         +---- pin  2 CPU D0
                                      GP7  (hdr pin 10) ---- EXT1_D1 ----+---- pin 15 EXT1
                                                                         +---- pin  3 CPU D1
                                      GP8  (hdr pin 11) ---- EXT2_D2 ----+---- pin 16 EXT2
                                                                         +---- pin  4 CPU D2
                                      GP9  (hdr pin 12) ---- EXT3_D3 ----+---- pin 17 EXT3
                                                                         +---- pin  5 CPU D3

                                      GP10 (hdr pin 14) ---- PPU_D4 ---------- pin  6 CPU D4
                                      GP11 (hdr pin 15) ---- PPU_D5 ---------- pin  7 CPU D5
                                      GP12 (hdr pin 16) ---- PPU_D6 ---------- pin  8 CPU D6
                                      GP13 (hdr pin 17) ---- PPU_D7 ---------- pin  9 CPU D7

                                      GP14 (hdr pin 19) ---- PPU_A12_PAIR ---- pin 10 CPU A2
                                                                           +-- pin 11 CPU A1
                                      GP15 (hdr pin 20) ---- PPU_A0 ---------- pin 12 CPU A0

                                      GP16 (hdr pin 21) ---- PPU_nCS --------- pin 13 /CS
                                                            |
                                                            +-- R1 10 kΩ -- +3V3

                                      GP17 (hdr pin 22) <--- PPU_nINT --------- pin 19 /INT
                                                            |
                                                            +-- R2 10 kΩ -- +3V3

                                      GP22 (hdr pin 29) ---- PALETTE_BUTTON
                                                            |
                                                           SW1
                                                            |
                                                           GND
                                      (RP2350 internal pull-up enabled)

                              GND --------------------------- pin  1 R/W

 PPU_5V ---------------------------------------------------- pin 40 +5V
                                                            |
                                                           C1 100 nF
                                                            |
 GND ------------------------------------------------------- pin 20 GND

 PPU_5V ------------------------------ R3 10 kΩ ------------ pin 22 /RST
                                                               |
                                                           TP_PPU_RST
                                                  (bench pad may pull LOW)

 PPU_CLK_IN ------------------------------------------------ pin 18 CLK

                                                            pin 21 VOUT ----> PPU_VIDEO_RAW

 RP2C02 external VRAM-side pins unused in V0.1:
 pin 39 ALE; pins 38..31 AD0..AD7; pins 30..25 A8..A13; pin 24 /RD; pin 23 /WR
 -> NC on this sheet. Do not tie them together or to a rail merely because they are unused.
```

## Exact connection table

### Game Boy DMG / SGB to Pico 2

| Source signal | Pico 2 GPIO | Pico 2 header pin | Direction at Pico | V0.1 role |
|---|---:|---:|---|---|
| `LD0` | GP0 | 1 | input | source shade bit |
| `LD1` | GP1 | 2 | input | source shade bit |
| `CP` | GP2 | 4 | input | pixel sampling/qualification |
| `CPL` | GP3 | 5 | input | LCD timing |
| `ST` | GP4 | 6 | input | line timing |
| `S` | GP5 | 7 | input | frame timing |
| `P14` | GP18 | 24 | input | optional SGB-lite listener |
| `P15` | GP19 | 25 | input | optional SGB-lite listener |
| `GND` | any Pico GND | — | reference | mandatory common ground |

No added pull-up or pull-down is specified on the active Game Boy source signals in V0.1.

### Pico 2 to RP2C02 CPU/EXT interface

| Pico 2 net | GPIO / hdr pin | RP2C02 pin(s) | PPU function | Notes |
|---|---|---|---|---|
| `EXT0_D0` | GP6 / 9 | 14 + 2 | EXT0 + CPU D0 | shared net |
| `EXT1_D1` | GP7 / 10 | 15 + 3 | EXT1 + CPU D1 | shared net |
| `EXT2_D2` | GP8 / 11 | 16 + 4 | EXT2 + CPU D2 | shared net |
| `EXT3_D3` | GP9 / 12 | 17 + 5 | EXT3 + CPU D3 | shared net |
| `PPU_D4` | GP10 / 14 | 6 | CPU D4 | write-only host bus |
| `PPU_D5` | GP11 / 15 | 7 | CPU D5 | write-only host bus |
| `PPU_D6` | GP12 / 16 | 8 | CPU D6 | write-only host bus |
| `PPU_D7` | GP13 / 17 | 9 | CPU D7 | write-only host bus |
| `PPU_A12_PAIR` | GP14 / 19 | 10 + 11 | CPU A2 + A1 | A1 and A2 intentionally tied |
| `PPU_A0` | GP15 / 20 | 12 | CPU A0 | register select |
| `PPU_nCS` | GP16 / 21 | 13 | /CS | R1 pull-up to 3.3 V |
| `PPU_nINT` | GP17 / 22 | 19 | /INT | open-drain PPU output; R2 pull-up to 3.3 V |

### RP2C02 fixed/passive connections

| RP2C02 pin | Signal | V0.1 connection | Reason |
|---:|---|---|---|
| 1 | `R/W` | GND | V1 PPU host interface is write-only |
| 18 | `CLK` | `PPU_CLK_IN` off-sheet flag | clock generator is a separate sheet |
| 19 | `/INT` | GP17 + 10 kΩ pull-up to 3.3 V | VBlank reference; open-drain output |
| 20 | GND | common GND | reference |
| 21 | `VOUT` | `PPU_VIDEO_RAW` off-sheet flag | raw/unbuffered composite source |
| 22 | `/RST` | 10 kΩ pull-up to `PPU_5V`; test pad | no dedicated MCU GPIO |
| 23 | `/WR` | NC | external PPU memory bus unused in V0.1 |
| 24 | `/RD` | NC | external PPU memory bus unused in V0.1 |
| 25..30 | A13..A8 | NC | external PPU memory bus unused in V0.1 |
| 31..38 | AD7..AD0 | NC | external PPU memory bus unused in V0.1 |
| 39 | ALE | NC | external PPU memory bus unused in V0.1 |
| 40 | +5 V | `PPU_5V` | PPU supply |

C1 = 100 nF ceramic directly between RP2C02 pins 40 and 20, physically close to the package. Bulk supply capacitance belongs to the later power sheet/carrier implementation.

## Register-address reduction used by the schematic

`A1` and `A2` are tied because V1 only needs these four CPU-interface registers:

```text
PPU_A12_PAIR  PPU_A0    selected register
      0          0      $2000 PPUCTRL
      0          1      $2001 PPUMASK
      1          0      $2006 PPUADDR
      1          1      $2007 PPUDATA
```

`$2002..$2005` are not part of the V1 host-interface requirement.

## Shared EXT / data-bus nets

GP6..GP9 intentionally drive both `EXT0..EXT3` and PPU CPU data bits `D0..D3`.

Normal picture output:

```text
/CS = HIGH
GP6..GP9 -> EXT0..EXT3 pixel indices
PPU CPU D0..D3 are not participating in a host access
```

Register/palette write:

```text
R/W = LOW permanently
set GP6..GP13 = data byte
set PPU_A12_PAIR + PPU_A0
pulse /CS LOW
return /CS HIGH
resume EXT pixel stream
```

Host writes should be scheduled in VBlank/safe output intervals. Bench validation must verify that the doubled pin load on GP6..GP9 is acceptable with the selected RP2C02 or clone.

## Pico 2 pins deliberately not used on this sheet

```text
GP20 / header 26    reserved for separate clock-generator sheet if control is required
GP21 / header 27    reserved for separate clock-generator sheet if control is required
GP26 / header 31    3.3 V-only diagnostic/future
GP27 / header 32    3.3 V-only diagnostic/future
GP28 / header 34    3.3 V-only diagnostic/future
```

The clock subsystem may later use GP20/GP21 for I2C, but that is intentionally not part of this V0.1 interconnect sheet.

## Electrical assumptions that remain to be bench-validated

The drawing is a **design-basis schematic**, not yet a release-to-fabrication schematic. Before freezing a PCB, verify:

1. 3.3 V Pico 2 outputs are reliably recognized by the selected RP2C02/clone on `EXT`, CPU data, address and `/CS` inputs.
2. Game Boy/SGB source signals remain within RP2350 fault-tolerant digital-input limits under the actual power-up/power-down sequence.
3. GP6..GP9 can drive the parallel EXT + CPU-D pin load with acceptable edge quality.
4. `/INT` behaves correctly with a 3.3 V external pull-up.
5. `/RESET` high and the firmware warm-up delay are sufficient for reliable initialization across selected PPU revisions/clones.
6. No external VRAM-side connection is required for the rendering-disabled / internal-palette-only V1 mode.

If a measurement fails, add only the smallest buffer/level-interface stage required by that measured problem.

## Video-output scope boundary

`PPU_VIDEO_RAW` is intentionally the final node of this sheet.

The RP2C02 raw output normally requires downstream buffering/drive for a 75-ohm composite load. The project may later document a minimal composite stage, while other existing NES video modifications may be reusable as independent downstream work. HDMI, S-Video, RGB-oriented or other video-output modifications are not part of the baseline V1 schematic and should remain separate from the Game Boy capture/scaler/EXT architecture.
