# Hardware Interfaces — Draft

## DMG interface

Required video/timing signals:

```text
LD0
LD1
CP
CPL
ST
S
GND
```

Optional SGB-related signals:

```text
P14
P15
```

The project also requires a method to provide the modified DMG clock target.

Working target:

```text
DMG_SYNC_CLK ≈ 4.2203555 MHz
```

The exact injection point and buffering are installation-specific and must be documented once the target motherboard revision is selected.

The external synchronized clock must not simply be driven against an active original oscillator. The prototype procedure must define how the stock source is isolated, disabled, removed, or otherwise prevented from contention.

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
      PPU_CLK            DMG_SYNC_CLK
 ~21.4772727 MHz        ~4.2203555 MHz
          |                   |
    buffer/isolation     buffer/isolation
      as required          as required
          |                   |
       RP2C02              DMG CPU/SoC
```

The architectural requirement is the **common reference**. The Si5351A is the preferred first implementation candidate, not yet a frozen production component.

### Candidate output conditioning

A small buffer/level-interface device such as a **74AHCT125-class part** is a current candidate for one or both clock branches where TTL-compatible drive, isolation or edge conditioning is useful.

The final part and topology remain open until bench measurements establish:

- DMG clock-input voltage requirements,
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
DMG_CLK_AT_IC
GND
```

This allows the generated clocks to be compared before and after any buffering/level-shifting stage.

## User interface

```text
PALETTE_BUTTON
```

No menu display or multi-button UI is planned for version 1.

## Electrical validation before PCB freeze

For every signal group, record:

- source voltage domain,
- sink voltage domain,
- input thresholds,
- drive strength,
- expected edge rate,
- pull-up/pull-down behavior,
- loading constraints,
- whether direct connection, resistor isolation, buffer, or level shifter is required.

For the clock paths also record:

- actual frequency at the driven IC pin,
- duty cycle,
- rise/fall time,
- overshoot/undershoot,
- phase/frequency relationship between DMG and PPU outputs,
- startup state before the Si5351A is configured.
