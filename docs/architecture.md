# Architecture

## Compatibility scope

The project is architected for **Game Boy DMG / SGB** source compatibility.

The common path is the Game Boy-compatible video stream itself: capture the 160x144 four-shade image, buffer it, scale it, map its shades to RP2C02 colors and generate composite NTSC video.

SGB-specific features such as passive palette-command recovery are layered on top and are not prerequisites for the common video path.

For any SGB/SGB-CPU source hardware used in practice, signal access, voltage levels, loading and clock injection must be measured and documented rather than assumed identical to a DMG.

## Functional chain

```text
Game Boy DMG / SGB video source
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
 aspect-correct fixed scaler
 160x144 -> 234x240
      |
 border generator
11 border + image + 11 border
      |
      v
 EXT0..EXT3
      |
      v
 NES/Nintendo video PPU
       (RP2C02 class)
      |
      v
 composite NTSC video
      |
      v
 CRT television
```

## Source interface

Common Game Boy video/timing signals presently expected:

- `LD0`
- `LD1`
- `CP`
- `CPL`
- `ST`
- `S`

Optional SGB-specific signaling taps:

- `P14`
- `P15`

The six video/timing signals form the baseline image-source interface. `P14/P15` are optional and exist only to add SGB-lite behavior when accessible.

The exact polarity, active window, sampling edge, voltage levels and loading requirements must be verified on real hardware for each supported source configuration before the schematic is frozen.

## Frame storage

The Game Boy image is 160 x 144 at 2 bits per pixel:

```text
160 x 144 x 2 bits = 5,760 bytes/frame
```

Version 1 intentionally uses two complete source buffers:

```text
2 x 5,760 = 11,520 bytes = 11.25 KiB
```

These buffers provide robust ping-pong capture/display ownership. They are not used as a frame-rate-conversion reservoir.

The framebuffer representation is intentionally source-independent: once a valid DMG or SGB-compatible source frame has been captured, the downstream scaler, palette and PPU-output path should be identical.

## Scaling and picture geometry

The PPU active raster remains 256 x 240, but the Game Boy image is **not stretched across the full 256-dot width**.

Version 1 uses:

```text
11 border | 234-dot Game Boy image | 11 border
           x 240 lines high
```

The design fills the raster vertically while preserving the apparent Game Boy picture proportions on the NTSC CRT.

Vertical scaling is exact:

```text
144 -> 240 = 5/3
3 source lines -> 5 output lines
```

Horizontal scaling is deterministic integer nearest-neighbor repetition:

```text
160 -> 234
160 base emissions + 74 duplicated pixels
```

Each source pixel is emitted once or twice. No interpolation, floating point or general-purpose scaler is required.

The side borders and the scaled image are generated while FRONT is read. No 234 x 240 or 256 x 240 intermediate framebuffer is required.

See [`scaling.md`](scaling.md) for the canonical algorithm and reference mapping.

## RP2C02 operating philosophy

Version 1 avoids normal NES background/sprite rendering.

The controller initializes PPU registers and palette RAM, then supplies external pixel/palette indices using `EXT0..EXT3`.

Provisional mapping:

```text
EXT1:EXT0 = Game Boy shade (0..3)
EXT3:EXT2 = palette-bank / reserved simple function
```

The exact use of the upper EXT bits can remain minimal in version 1.

## Frame/VBlank coordination

`/INT` from the PPU is the preferred deterministic VBlank reference for:

- palette writes;
- framebuffer presentation changes;
- non-time-critical UI housekeeping;
- diagnostics.

## Palette behavior

The four Game Boy shades remain independent of the scaler. Manual palette presets and optional SGB-derived palettes therefore change only the PPU color mapping, not the framebuffer or geometry.

One button cycles palette modes/presets, and manual selection always overrides an automatically received SGB palette.

## Border behavior

Version 1 uses fixed black for the 11-dot left and right side bars.

These bars are owned by a logical `border_generator` / output-composition block, not by the scaler or source framebuffer.

That separation is intentional. A later revision may allow a different simple border color or low-complexity effect without changing capture, scaling or framebuffer storage. Such behavior is deferred and is not a version-1 user option.

## SGB compatibility model

SGB support has two layers:

### Common SGB video compatibility

The project intends the same core video pipeline to work with an SGB/SGB-CPU-compatible source when the equivalent Game Boy video/timing and clock signals are accessible.

This must be validated experimentally. The repository should document every tested SGB hardware/configuration rather than imply universal drop-in electrical equivalence.

### Optional SGB-lite palette signaling

If `P14/P15` are connected, the controller may passively decode a limited subset of direct Super Game Boy palette commands.

This optional layer must never be required for:

- video capture,
- buffering,
- scaling,
- composite output,
- manual palette selection.

Full SGB emulation, active SGB identification behavior, spatial attributes and graphical SGB borders are outside version 1.
