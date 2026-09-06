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

The project also requires a method to provide the modified DMG clock target. The exact injection point and buffering are installation-specific and must be documented once the target motherboard revision is selected.

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
PPU_CLK
```

Video:

```text
COMPOSITE_OUT
```

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
