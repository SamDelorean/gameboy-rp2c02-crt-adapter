# Optional SGB-lite Palette Listener

## Goal

Recover a useful subset of Super Game Boy palette information without turning the project into a full SGB emulator.

## Optional physical inputs

- `P14`
- `P15`

These are **optional taps**. A normal DMG installation must operate fully without them.

They are included in the proposed connector/interface only to preserve a low-cost path for SGB-lite functionality. Leaving them unconnected must not disable capture, scaling, manual palette selection, or normal composite output.

For SGB-CPU or other compatible installations, P14/P15 may be observed passively to recover SGB command traffic.

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
cached SGB four-color palette
      |
AUTO/SGB palette mode
```

## User override is mandatory

SGB-derived colorization must **never lock the user into the received palette**.

The single palette pushbutton always remains authoritative.

Recommended mode model:

```text
0  AUTO/SGB
1  manual preset 1
2  manual preset 2
3  manual preset 3
...
N  manual preset N
   -> wrap to AUTO/SGB
```

Behavior:

1. In **AUTO/SGB**, a valid supported SGB palette command may replace the active global palette.
2. If no valid SGB palette is available, AUTO/SGB uses a defined fallback palette.
3. A button press while an SGB-derived palette is visible immediately exits AUTO/SGB and selects the first manual preset.
4. In a manual mode, later SGB commands may be decoded and cached, but they must **not override the user's selected manual palette**.
5. Cycling the button back to AUTO/SGB re-enables SGB control. The latest valid cached SGB palette may be applied immediately; if none is available, the fallback palette remains.

This gives SGB-aware software an automatic palette path without sacrificing the basic one-button manual interface.

## Important limitation

A passive listener cannot force every SGB-enhanced game to transmit SGB commands on every Game Boy configuration.

Some software may first attempt to detect SGB-specific behavior before using its SGB features. Therefore the optional listener is best understood as opportunistic compatibility rather than universal automatic SGB support.

## SGB-CPU installations

The optional interface is intentionally retained for unusual Game Boy builds using an SGB CPU or other configurations where SGB signaling may be naturally present.

P14/P15 are inputs to the adapter only in the passive SGB-lite implementation. The first version does not drive or emulate them.

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

If P14/P15 are absent, if no compatible SGB command is observed, or if the user selects a manual mode, the manually selected palette remains fully functional.
