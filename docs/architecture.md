# Architecture

## Functional chain

```text
DMG LCD interface
      |
      v
 deterministic capture
      |
      v
 back framebuffer
      |
   frame swap
      |
      v
 front framebuffer
      |
 fixed 160x144 -> 256x240 scaler
      |
      v
 EXT0..EXT3
      |
      v
 RP2C02-compatible NTSC PPU
      |
      v
 composite video
```

## Source interface

Primary DMG signals:

- `LD0`
- `LD1`
- `CP`
- `CPL`
- `ST`
- `S`

Optional SGB-related taps:

- `P14`
- `P15`

The exact polarity, active window, sampling edge, voltage levels, and loading requirements must be verified on real hardware before the schematic is frozen.

## Frame storage

The DMG image is 160x144 at 2 bits per pixel.

```text
160 x 144 x 2 bit = 46,080 bit = 5,760 bytes/frame
```

The first implementation intentionally uses two complete buffers:

```text
2 x 5,760 = 11,520 bytes = 11.25 KiB
```

This is a deliberate robustness choice. Memory minimization is not a first-order requirement.

## Scaling

Target raster: 256x240.

Exact spatial ratios:

```text
X = 256 / 160 = 8 / 5
Y = 240 / 144 = 5 / 3
```

The scaler can therefore be fixed-function nearest-neighbor logic. A generic arbitrary-ratio scaler is unnecessary.

## RP2C02 operating philosophy

Version 1 avoids normal NES background/sprite rendering.

The digital controller initializes PPU registers and palette RAM, then supplies external pixel/palette indices using `EXT0..EXT3`.

Provisional mapping:

```text
EXT1:EXT0 = DMG shade (0..3)
EXT3:EXT2 = palette-bank / reserved simple function
```

The exact use of the upper EXT bits can remain minimal in version 1.

## Frame/VBlank coordination

`/INT` from the PPU should be used as a deterministic VBlank reference for operations such as:

- palette writes,
- framebuffer presentation changes,
- non-time-critical UI housekeeping,
- diagnostics.

## Overscan

Initial behavior: fixed black.

Future firmware may optionally derive border color from the selected palette or extend edge pixels, but this should not add UI complexity to the initial design.

## Optional SGB-lite

If `P14/P15` are connected, the controller may passively decode a limited subset of direct Super Game Boy palette commands. This is an optional enhancement and must not be required for normal operation.
