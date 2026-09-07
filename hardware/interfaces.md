# Hardware Interfaces — Draft

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
| Super Game Boy / SGB-CPU configuration | to be identified per hardware | to be measured | to be identified/validated | optional SGB-lite | planned compatibility |

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

For SGB source hardware, this same architectural goal remains, but the exact clock-access method must be validated separately rather than copied blindly from the DMG installation.

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
