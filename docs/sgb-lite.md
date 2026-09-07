# Super Game Boy Compatibility and Optional SGB-lite Palette Listener

## Compatibility goal

The project is intended for **Game Boy DMG / SGB** use.

That compatibility has two distinct layers which must not be confused:

1. **common video-path compatibility** — use the same Game Boy capture, framebuffer, aspect-correct scaler, palette and RP2C02 output architecture with an SGB/SGB-CPU-compatible video source when the required signals are accessible;
2. **optional SGB-lite palette recovery** — passively observe SGB command traffic on `P14/P15` and recover a limited subset of palette information.

The first goal is part of the core compatibility plan. The second is an optional enhancement.

## Common SGB video path

For an SGB/SGB-CPU source configuration, the project should validate and document:

- access to the equivalent 160x144 Game Boy pixel/timing stream;
- signal voltage levels and loading;
- active pixel/line/frame timing;
- synchronized clock access/injection;
- any adapter wiring or buffering differences from DMG.

Once a valid 160x144x2-bit frame has been reconstructed, the downstream path should be identical to DMG:

```text
Game Boy DMG / SGB source
        |
160x144x2-bit framebuffer
        |
aspect-correct 160 -> 234 / 144 -> 240 scaler
        |
border generator
        |
palette mapping
        |
EXT0..EXT3
        |
RP2C02 composite NTSC
```

Do not claim universal electrical compatibility with every SGB revision/configuration until measured.

## Optional physical inputs for SGB-lite

- `P14`
- `P15`

These are **optional taps**. A DMG or SGB installation must be able to produce normal video without them.

They are included in the proposed connector/interface only to preserve a low-cost path for SGB-lite functionality. Leaving them unconnected must not disable capture, scaling, manual palette selection or composite output.

For SGB/SGB-CPU-compatible installations, P14/P15 may be observed passively to recover SGB command traffic.

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

A passive listener cannot force every SGB-enhanced game to transmit SGB commands on every configuration.

Some software may first attempt to detect SGB-specific behavior before using its SGB features. Therefore passive SGB-lite is opportunistic compatibility, not universal automatic SGB palette support.

This limitation does **not** invalidate the broader goal of using an SGB-compatible source for the normal Game Boy video path.

## SGB/SGB-CPU installations

The optional interface is intentionally retained for unusual Game Boy builds, original/modified SGB hardware, SGB CPU configurations or other systems where compatible Game Boy video and SGB signaling are accessible.

P14/P15 are inputs to the adapter only in the passive SGB-lite implementation. Version 1 does not drive or emulate them.

Each tested SGB installation should record exact hardware/revision and signal access in the compatibility/validation documentation.

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
