# Optional SGB-lite Palette Listener

## Goal

Recover a useful subset of Super Game Boy palette information without turning the project into a full SGB emulator.

## Optional physical inputs

- `P14`
- `P15`

These are optional taps. A normal DMG installation must operate fully without them.

## Initial supported direct palette commands

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

## Processing model

```text
P14/P15 traffic
      |
SGB packet decoder
      |
direct palette command
      |
RGB555 colors
      |
quantization to suitable RP2C02 colors
      |
global four-color palette
```

## Important limitation

A passive listener cannot force every SGB-enhanced game to transmit SGB commands on every Game Boy configuration.

Some software may first attempt to detect SGB-specific behavior before using its SGB features. Therefore the optional listener is best understood as opportunistic compatibility rather than universal automatic SGB support.

## SGB-CPU installations

The optional interface is intentionally retained for unusual Game Boy builds using an SGB CPU or other configurations where SGB signaling may be naturally present.

## Deliberate non-goals for version 1

Do not initially implement:

- active controller-ID emulation,
- `MLT_REQ` response behavior,
- `ATTR_BLK`,
- `ATTR_LIN`,
- `ATTR_DIV`,
- `ATTR_CHR`,
- `PAL_TRN`,
- `ATTR_TRN`,
- `CHR_TRN`,
- `PCT_TRN`,
- SGB graphical borders.

If no compatible SGB command is observed, the selected manual palette remains in use.
