# Controller Selection

## Decision

**SET for the version-1 baseline: RP2350, with Raspberry Pi Pico 2 as the preferred prototype/module implementation.**

The deciding criterion is not maximum CPU performance. The project prefers the controller/topology that achieves the required deterministic video path with the **least additional electronics** while remaining easy to build and debug.

RP2350/Pico 2 is preferred over RP2040/Pico because it retains PIO + DMA, provides ample SRAM, and current RP2350 digital GPIO are 5 V tolerant when IOVDD is powered. This can eliminate the separate level-shifting stage that RP2040 would require for the 5 V Game Boy LCD signals.

Use current RP2350 silicon and do not rely on obsolete early-silicon high-impedance behavior. On Pico 2, the ADC-capable GPIO26..28 are not to be used for 5 V source inputs; the Game Boy/SGB 5 V inputs should use the fault-tolerant digital GPIO group.

See [`../hardware/interfaces.md`](../hardware/interfaces.md) for the electrical/interconnection contract.

## Hard requirements

The selected controller must provide:

- at least 11.25 KiB for two complete 160x144x2-bit source framebuffers;
- deterministic Game Boy DMG / SGB source capture;
- deterministic four-bit `EXT0..EXT3` output;
- enough GPIO for the optimized direct interface including optional `P14/P15`;
- hardware-assisted I/O (PIO/DMA or equivalent), not pixel-rate interrupt bit-banging;
- practical clock-generator control;
- simple module-level prototyping and USB/SWD development access.

RP2350/Pico 2 exceeds the memory/peripheral requirements: the relevant advantage here is **5 V-tolerant digital input plus PIO/DMA and sufficient exposed GPIO after optimization**.

## Design rule for saving pins

Before adding a latch, bus expander, level shifter, or other glue IC, use the following priority:

1. remove signals that can be fixed safely by wiring or pull resistors;
2. merge address/control signals whose required states are identical;
3. reuse MCU GPIO where two PPU pins can safely share one driven net;
4. preserve direct dedicated paths for timing-critical video signals;
5. add external logic only if bench measurements show the direct minimized topology is insufficient.

## PPU host interface: write-only V1

Version 1 does **not** require PPU CPU-bus reads.

The firmware can avoid `PPUSTATUS` reads by:

- allowing the documented PPU warm-up interval after power/reset before writing the registers that are initially inhibited;
- using `/INT` after enabling NMI as the safe VBlank/frame reference.

This makes the CPU-side PPU interface write-only and enables several no-IC pin reductions.

The required PPU CPU-interface registers are:

| Register | A2 A1 A0 | V1 use |
|---|---|---|
| `$2000` PPUCTRL | `000` | EXT input mode / NMI control |
| `$2001` PPUMASK | `001` | rendering state if explicitly written |
| `$2006` PPUADDR | `110` | palette address setup |
| `$2007` PPUDATA | `111` | palette data write |

No V1 requirement currently needs `$2002`..`$2005`.

## Pin-saving decisions

### SET — `R/W` fixed LOW

The PPU CPU interface is write-only in V1, so PPU `R/W` is tied to GND and consumes no MCU GPIO.

`/CS` remains MCU-controlled and is the access qualifier/strobe.

### SET — PPU `/RESET` normally pulled HIGH, no MCU GPIO

The PPU can power up with `/RESET` held inactive; Famicom hardware is a precedent for a permanently high PPU reset input.

For the project schematic, prefer a pull-up plus labeled test/reset pad rather than dedicating an MCU GPIO. This gives a defined state and preserves the ability to force reset during bench work.

Startup firmware must wait through the PPU warm-up interval before relying on register writes.

### SET — Tie PPU A1 and A2 together

For the four registers used by V1, A1 and A2 always have identical values:

```text
$2000: A2 A1 = 00
$2001: A2 A1 = 00
$2006: A2 A1 = 11
$2007: A2 A1 = 11
```

Therefore PPU CPU address pins `A1` and `A2` are physically tied together and driven by one MCU signal, `PPU_A12_PAIR`.

`A0` remains separate.

This selects exactly the four useful states:

```text
PAIR A0 = 00 -> $2000
PAIR A0 = 01 -> $2001
PAIR A0 = 10 -> $2006
PAIR A0 = 11 -> $2007
```

No external logic is required.

### SET — Share MCU `EXT0..EXT3` with PPU CPU `D0..D3`

Each of the four timing-critical EXT output GPIOs also connects to the corresponding low-nibble PPU CPU data input:

```text
MCU EXT0_DATA0 -> PPU EXT0 + CPU D0
MCU EXT1_DATA1 -> PPU EXT1 + CPU D1
MCU EXT2_DATA2 -> PPU EXT2 + CPU D2
MCU EXT3_DATA3 -> PPU EXT3 + CPU D3
```

This is possible because:

- V1 keeps PPU `R/W` LOW, so the CPU data pins are never intentionally used as PPU outputs;
- `/CS` remains inactive during normal pixel output;
- register writes occur at low rate and preferably in VBlank;
- the momentary EXT value present during a register write is irrelevant to the visible image when performed in the safe interval.

This removes four additional MCU pins without a mux or latch.

Bench validation must still confirm there is no unexpected loading or contention on the selected RP2C02/clone.

## Optimized controller I/O contract

| Function | Signals at MCU | Direction | GPIO | Timing | Notes |
|---|---|---:|---:|---|---|
| Game Boy pixels | `LD0`, `LD1` | in | 2 | pixel-rate | 5 V source; use RP2350 FT GPIO |
| Game Boy pixel clock | `CP` | in | 1 | pixel-rate | 5 V source; FT GPIO |
| Game Boy timing | `CPL`, `ST`, `S` | in | 3 | deterministic | retain until bench proves one redundant |
| SGB-lite | `P14`, `P15` | in | 2 | protocol timing | optional; use FT GPIO if 5 V |
| EXT + PPU D0..D3 shared | `EXT0_DATA0..EXT3_DATA3` | out | 4 | pixel-rate | contiguous PIO group preferred |
| PPU D4..D7 | `PPU_D4..D7` | out | 4 | low-rate | write-only bus |
| PPU address | `PPU_A12_PAIR`, `PPU_A0` | out | 2 | low-rate | A1 and A2 physically tied |
| PPU access | `/CS` | out | 1 | low-rate | external pull-up for safe MCU reset state |
| PPU VBlank | `/INT` | in | 1 | frame-rate | open-drain; pull up to MCU-safe rail |
| Clock generator | `SDA`, `SCL` | I/O,out | 2 | low-rate | Si5351A proposal |
| Palette button | `PALETTE_BUTTON` | in | 1 | human-rate | internal RP2350 pull-up can be used |

### Final GPIO budget

```text
Game Boy capture                    6
shared EXT0..3 / PPU D0..3         4
PPU D4..7                           4
PPU address pair + A0               2
PPU /CS                             1
PPU /INT                            1
clock-generator I2C                 2
palette button                      1
-------------------------------------
mandatory DMG total                21 GPIO

optional P14/P15                   +2
-------------------------------------
planned DMG / SGB total            23 GPIO
```

A Pico 2 exposes 26 multifunction GPIO, leaving **3 GPIO of margin** even with SGB-lite reserved.

Those remaining pins should be used primarily for scope/logic-analyzer markers or other 3.3 V-only diagnostics. USB CDC and SWD should be preferred over consuming two more GPIO for a permanent UART.

## Proposed Pico 2 functional placement

This is the first pin-placement plan; exact PIO program constraints may adjust individual assignments before schematic freeze.

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

GPIO26..28  reserved for 3.3 V diagnostics / future non-5-V functions
```

Benefits of this arrangement:

- all six DMG capture signals are contiguous;
- `LD0/LD1` form a contiguous two-bit input field;
- `EXT0..EXT3` form a contiguous four-bit PIO output field;
- all expected 5 V source inputs stay off the ADC GPIO26..28;
- SGB-lite remains present without consuming diagnostic margin.

## Pull-up / pull-down policy

### External passive parts that are useful

| Signal | Passive state | Reason |
|---|---|---|
| PPU `/RESET` | pull-up to +5 V | defined inactive reset without MCU GPIO; test pad may pull low |
| PPU `/CS` | pull-up to a validated logic-high rail, preferably MCU-safe 3.3 V if PPU threshold allows | keeps PPU deselected while MCU pins are in reset/default state |
| PPU `/INT` | pull-up to 3.3 V | `/INT` is open-drain and becomes directly MCU-safe |
| I2C `SDA/SCL` | normal I2C pull-ups to 3.3 V | required for reliable clock-generator bus |

### Use MCU internal pull where appropriate

`PALETTE_BUTTON` should use an RP2350 internal pull-up with the switch to GND unless EMC/noise testing later justifies an external resistor.

### Do not add pulls casually

Do not add default pulls to:

- `LD0`, `LD1`, `CP`, `CPL`, `ST`, `S`;
- `P14`, `P15`;
- `EXT0..EXT3`.

These are actively driven interfaces; unnecessary pulls add load and can interfere with timing or SGB/joypad signaling.

## Why the former shift-register proposal is no longer preferred

The earlier 17/19-GPIO serialized PPU-host-bus idea remains technically possible, but it adds at least one glue-logic stage and complicates reads/bring-up.

With RP2350/Pico 2 and the direct pin-sharing decisions above, the project fits in 23 GPIO with no PPU bus expander or latch. Therefore external serialization is **not the V1 baseline**.

## Remaining electrical checks before schematic freeze

The architecture is SET, but bench validation must still confirm:

1. 3.3 V RP2350 outputs are accepted reliably by RP2C02/selected clone CPU and EXT inputs;
2. the shared `EXT0..3` / `D0..3` nets do not create loading or contention;
3. `/CS` high level and pull-up rail are valid for both PPU and MCU;
4. `/INT` open-drain behavior with a 3.3 V pull-up;
5. actual DMG/SGB source levels on all eight possible source inputs;
6. PIO/DMA timing with the proposed contiguous GPIO placement.

If one of these measurements requires a buffer, add only the smallest interface stage required by that measured problem.
