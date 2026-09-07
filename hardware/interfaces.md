# Hardware Interfaces — Draft

This document defines the physical/logical signal boundary between the Game Boy DMG / SGB source, the digital controller, the RP2C02-compatible PPU, the clock subsystem, and the user interface.

The controller-selection pin budget is maintained in [`../docs/controller-selection.md`](../docs/controller-selection.md). This file is the corresponding **interconnection table**: what connects to what, in which direction, and why.

## Controller interconnection matrix

### Signals that terminate at the controller

| Signal/group | Origin | Destination | MCU direction | GPIO count | Timing class | Status | Expected function | Electrical note |
|---|---|---|---:|---:|---|---|---|---|
| `LD0`, `LD1` | Game Boy DMG / compatible SGB video source | controller | input | 2 | pixel-rate / deterministic | required | 2-bit Game Boy shade value | DMG logic level must be adapted/validated before direct MCU connection |
| `CP` | Game Boy source | controller | input | 1 | pixel-rate / deterministic | required | source pixel sampling/qualification clock | edge and voltage must be measured |
| `CPL` | Game Boy source | controller | input | 1 | line timing | required until capture validation says otherwise | line/latch timing reference | source-specific electrical validation required |
| `ST` | Game Boy source | controller | input | 1 | line timing | required until capture validation says otherwise | line-start/timing reconstruction | source-specific electrical validation required |
| `S` | Game Boy source | controller | input | 1 | frame timing | required until capture validation says otherwise | frame boundary/reference | source-specific electrical validation required |
| `P14`, `P15` | SGB-capable source | controller | input | 2 | low-rate protocol edges | optional / reserved | passive SGB-lite palette-command listener | listener must not load/disturb JOYP/SGB signaling |
| `EXT0..EXT3` | controller | RP2C02 | output | 4 | PPU-pixel-rate / deterministic | required | external PPU palette index / border value | PPU input thresholds and required 3.3↔5 V adaptation must be validated |
| `D0..D7` | controller ↔ RP2C02 | RP2C02 ↔ controller | bidirectional-capable preferred | 8 | low-rate register bus | required interface function | PPU register/palette data; status read if supported | 5 V PPU bus; final direct/buffered topology open |
| `A0..A2` | controller | RP2C02 | output | 3 | low-rate register bus | required | PPU register select | level compatibility to be validated |
| `R/W` | controller | RP2C02 | output | 1 | low-rate register bus | required | PPU bus direction/control | level compatibility to be validated |
| `/CS` | controller | RP2C02 | output | 1 | low-rate register bus | required | qualify PPU register access | level compatibility to be validated |
| `/RESET` | controller | RP2C02 | output | 1 | startup/control | required | deterministic PPU reset/init | may be direct or buffered depending final logic family |
| `/INT` | RP2C02 | controller | input | 1 | frame-rate timing | required | VBlank/safe presentation boundary | 5 V-domain output must be safely received by MCU |
| `SDA`, `SCL` | controller ↔ programmable clock generator | Si5351A or equivalent ↔ controller | SDA bidirectional, SCL output | 2 | low-rate control | required for proposal 1 | configure common clock generator | pull-up voltage and I2C domain must be defined in schematic |
| `PALETTE_BUTTON` | momentary switch | controller | input | 1 | human-rate | required | AUTO/SGB/manual palette cycle | defined pull-up/pull-down; firmware debounce |
| diagnostic markers | controller | test points | output | 2-4 desirable | debug | desirable | capture/VBlank/output timing observation | route to labeled pads/header |
| debug UART | controller ↔ host tool | header/USB-serial | TX/RX | 2 desirable | debug | optional | bring-up logging and diagnostics | voltage-domain compatible header only |

### Important non-MCU signal paths

These signals are part of the adapter but **do not consume controller GPIO** in the baseline architecture.

| Signal | Origin | Destination | Function | Notes |
|---|---|---|---|---|
| `PPU_CLK` | common clock generator | RP2C02 clock input | ~21.4772727 MHz PPU master reference | buffer/conditioning as required |
| `GB_SYNC_CLK` | common clock generator | Game Boy DMG / SGB source clock injection point | synchronized Game Boy-domain reference, working target ~4.2203555 MHz | original source must be isolated before injection |
| `COMPOSITE_OUT` | RP2C02 | video output connector / CRT | analog NTSC composite video | controller is not in analog output path |
| `GND` | common reference | all blocks | electrical reference | grounding/layout to be defined in schematic |
| supply rails | power subsystem | controller / PPU / clock / buffers | operating power | exact 3.3 V / 5 V partition remains to be frozen |

## Direct-GPIO budget implied by the interconnection table

With all RP2C02 host-bus signals connected directly to the controller:

```text
Game Boy capture inputs       6
EXT0..EXT3 outputs            4
PPU host interface           15
Si5351A control               2
palette button                1
-------------------------------
required                     28 GPIO

optional P14/P15             +2
-------------------------------
planned DMG / SGB total      30 GPIO
```

Diagnostic pins and UART would require additional margin.

This count is intentionally conservative: `CPL`, `ST`, and `S` remain part of the baseline source interface until real capture tests prove that any can be removed without reducing robustness or source compatibility.

## Recommended partition by timing criticality

### Keep direct to the controller

These paths are timing-critical enough that they should remain direct rather than being placed behind a slow GPIO expander:

```text
LD0
LD1
CP
CPL/ST/S as required by final capture state machine
EXT0
EXT1
EXT2
EXT3
/INT  (preferred direct)
```

For an RP2040-class implementation, pin assignment must be designed around PIO requirements rather than chosen arbitrarily after the schematic is complete.

Preferred placement principles:

- `LD0` and `LD1` should form an efficient two-bit PIO input group;
- `EXT0..EXT3` should preferably occupy four contiguous GPIOs;
- the clock/timing inputs should be assigned after the exact capture PIO state machine is known;
- optional `P14/P15` should remain available without consuming the last debug-capable pins.

### Candidate for serialization/latching

The RP2C02 host/register bus is low-rate:

```text
D0..D7
A0..A2
R/W
/CS
/RESET
```

It is used for:

- PPU initialization,
- register writes,
- palette-RAM writes,
- occasional control/status operations.

It is **not** used at pixel rate.

Therefore this group is the preferred place to reduce MCU pin count if RP2040/Pico remains the selected controller.

One candidate architecture is a small serial-to-parallel latched output stage:

```text
controller
   |
   | SER
   | SHIFT_CLK
   | LATCH_CLK
   v
+---------------------+
| low-rate PPU latch  |
+---------------------+
 |  D0..D7
 |  A0..A2
 |  R/W
 |  /CS
 +--/RESET

RP2C02 /INT ------------------> controller direct
```

The exact IC family is not frozen. A 5 V TTL-compatible latch/shift implementation may also serve as part of the level interface, but this must be selected from actual input/output thresholds rather than assumed.

A pure output-only serialization scheme must also answer whether V1 needs `PPUSTATUS` reads. If register reads are required, an input path/bus transceiver or another low-rate host-bus architecture must be added.

This optimization belongs in the PPU control interface only. It must not add latency or indeterminism to Game Boy capture or `EXT0..EXT3` output.

## Game Boy DMG / SGB source interface

The adapter is planned for **Game Boy DMG / SGB** source compatibility.

The common image path is based on the Game Boy LCD/video interface. The presently expected video/timing signals are:

```text
LD0
LD1
CP
CPL
ST
S
GND
```

These signals are required for the baseline capture path on a DMG. For an SGB/SGB-CPU-compatible source, the project must identify and validate the equivalent accessible signals before declaring that exact configuration supported.

The downstream framebuffer/scaler/output architecture should remain identical once a valid 160x144x2-bit source frame has been reconstructed.

### Optional SGB-lite signals

```text
P14
P15
```

`P14` and `P15` remain explicitly **optional** in the connector/pinout proposal. They are not needed for normal video capture, scaling, PPU output or manual palette selection.

Their purpose is to preserve a minimal hardware path for the passive SGB-lite listener. When connected to an SGB-capable CPU/configuration, the controller may observe command traffic and recover supported direct SGB palette commands. When absent or left unconnected, the adapter must operate normally.

For version 1 the adapter treats P14/P15 as passive inputs only; no active SGB response or controller-ID emulation is required.

### Source compatibility table to maintain

Each validated source configuration should eventually have a record containing at least:

| Source | Video signal access | Voltage levels | Clock access | P14/P15 | Status |
|---|---|---|---|---|---|
| Game Boy DMG | to be measured/frozen | to be measured | external synchronized clock planned | optional | reference platform |
| Super Game Boy / SGB-CPU configuration | to be identified per hardware | to be measured | external replacement principle established; exact board point to document | optional SGB-lite | planned compatibility |

The repository must not infer electrical equivalence merely from functional similarity.

## Game Boy synchronized clock input

The project requires a method to provide the modified Game Boy source clock target.

Working target:

```text
GB_SYNC_CLK ≈ 4.2203555 MHz
```

For DMG this replaces the previously named `DMG_SYNC_CLK`; the more general name reflects the DMG / SGB compatibility target.

The exact injection point and buffering are installation-specific and must be documented for every validated source motherboard/configuration.

The external synchronized clock must not simply be driven against an active original oscillator. The prototype procedure must define how the stock source is isolated, disabled, removed or otherwise prevented from contention.

For SGB, the project treats interruption of the host-derived clock path plus external injection as an established modification principle; see [`../docs/sgb-clock-injection.md`](../docs/sgb-clock-injection.md).

## RP2C02-compatible PPU interface

External pixel index:

```text
EXT0
EXT1
EXT2
EXT3
```

Control/register interface:

```text
D0..D7
A0..A2
R/W
/CS
/RESET
/INT
```

Clock:

```text
PPU_CLK ≈ 21.4772727 MHz
```

Video:

```text
COMPOSITE_OUT
```

## Shared clock-generation block — proposal 1

The first concrete clock-generation proposal uses one **Si5351A** programmable clock generator and one common crystal/reference source.

```text
          crystal / common reference
                    |
                 Si5351A
                    |
          +---------+---------+
          |                   |
        CLK0                CLK1
          |                   |
          v                   v
      PPU_CLK             GB_SYNC_CLK
 ~21.4772727 MHz        ~4.2203555 MHz
          |                   |
    buffer/isolation     buffer/isolation
      as required          as required
          |                   |
       RP2C02        Game Boy DMG / SGB source
```

The architectural requirement is the **common reference**. The Si5351A is the preferred first implementation candidate, not yet a frozen production component.

## Why the Game Boy source clock is modified

This block is not only a convenient way to generate two clocks. It is used to simplify the whole adapter architecture.

A stock Game Boy timing domain and an NTSC RP2C02 do not naturally complete frames at exactly the same rate. If both remained independent, the bridge would need additional logic and memory to absorb long-term source/output drift.

Instead, the Game Boy source is intentionally adapted so that its frame period matches the simplified RP2C02 NTSC frame period closely enough for the system to operate as one frequency-related video chain.

Design objective:

```text
1 Game Boy source frame
        =
1 RP2C02 output frame
```

This allows the bridge to avoid a generalized asynchronous frame-rate-conversion subsystem.

The two 160 x 144 x 2-bit framebuffers used by the digital controller remain part of the design, but only for clean ping-pong capture/display ownership and tear-free frame handoff. They are not intended to act as a timing reservoir between two free-running video standards.

Accordingly, the shared-clock approach eliminates the need for hardware whose main purpose would otherwise be:

- storing multiple source/output frames to absorb drift,
- inserting/dropping/repeating frames,
- elastically changing readout timing,
- managing a larger asynchronous clock-domain crossing,
- maintaining a full output-resolution synchronization framebuffer.

This is a deliberate hardware trade: a small modification to the Game Boy source clock is accepted in exchange for fewer parts, less RAM, simpler firmware and deterministic latency.

For SGB source hardware, the same system-level goal applies. The external-clock replacement principle is considered established; the exact board-level cut/injection point and electrical conditioning remain installation-specific documentation.

### Candidate output conditioning

A small buffer/level-interface device such as a **74AHCT125-class part** is a current candidate for one or both clock branches where TTL-compatible drive, isolation or edge conditioning is useful.

The final part and topology remain open until bench measurements establish:

- Game Boy source clock-input voltage requirements,
- PPU/clone clock-input requirements,
- Si5351A output amplitude,
- acceptable duty cycle,
- edge-rate/ringing behavior,
- required source impedance/series damping,
- loading and fanout.

### Required clock test points

The prototype PCB should expose, at minimum:

```text
CLK_REF
CLK_SI5351_CLK0
CLK_SI5351_CLK1
PPU_CLK_AT_IC
GB_CLK_AT_SOURCE
GND
```

This allows the generated clocks to be compared before and after any buffering/level-shifting stage.

## Border/output composition interface

The output path conceptually contains a separate border generator around the 234-dot scaled image:

```text
11 border + 234 image + 11 border = 256 dots
```

Version 1 drives both border regions as safe black. Keeping this block logically separate allows later simple border colors/effects without changing the Game Boy capture or scaler interfaces.

## User interface

Version 1 uses one momentary pushbutton:

```text
PALETTE_BUTTON
```

No menu display or multi-button UI is planned.

The button cycles the palette-control mode through:

```text
AUTO/SGB
manual preset 1
manual preset 2
...
manual preset N
-> AUTO/SGB
```

The button is always authoritative. If an SGB-derived palette is currently being displayed, the next press must immediately leave AUTO/SGB and select the first manual preset. While a manual preset is selected, P14/P15 traffic may still be observed/cached by firmware but must not alter the visible palette.

This requirement ensures that adding the optional SGB pins does not reduce the usefulness or predictability of the simple one-switch interface.

Hardware requirements for the button remain intentionally minimal:

- one digital input,
- defined pull-up or pull-down,
- firmware debounce,
- no timing-critical function on the switch path.

## Electrical validation before PCB freeze

For every signal group and every supported Game Boy source configuration, record:

- source voltage domain,
- sink voltage domain,
- input thresholds,
- drive strength,
- expected edge rate,
- pull-up/pull-down behavior,
- loading constraints,
- whether direct connection, resistor isolation, buffer or level shifter is required.

For optional P14/P15 also verify that the listener input does not materially load or disturb normal joypad/SGB signaling.

For the clock paths also record:

- actual frequency at the driven IC/source pin,
- duty cycle,
- rise/fall time,
- overshoot/undershoot,
- phase/frequency relationship between Game Boy and PPU outputs,
- startup state before the Si5351A is configured.
