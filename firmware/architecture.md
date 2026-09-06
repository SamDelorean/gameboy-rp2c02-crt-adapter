# Firmware Architecture

Suggested modules:

```text
clock_init
ppu_init
dmg_capture
frame_buffers
scaler
ppu_ext_output
palette
sgb_listener   (optional)
diagnostics
```

## Boot sequence

1. Place GPIO in safe states.
2. Hold the PPU in reset.
3. Configure/start the required clocks.
4. Start deterministic capture/output resources.
5. Release and initialize the PPU.
6. Load a safe default palette.
7. Capture one complete source frame.
8. Start EXT output.
9. Enter normal frame-processing operation.

## Capture path

The capture path should be peripheral/DMA driven where possible.

The implementation must explicitly document:

- sample edge,
- pixel packing format,
- line boundary detection,
- frame boundary detection,
- DMA buffer ownership,
- overrun behavior.

## Framebuffer ownership

Use two complete source-frame buffers:

- **back**: currently being captured,
- **front**: currently being read by the scaler/output path.

Swap only at an explicitly defined frame boundary.

## Scaler

Because both scale factors are rational constants, implement fixed indexing/repetition rather than a generic resampler.

## Output path

`EXT0..EXT3` timing must be deterministic and independent of foreground CPU load.

## Palette handling

Palette changes should be deferred to a safe VBlank interval. The manual preset selection and optional SGB-derived palette should feed the same final palette-loading mechanism.

## Diagnostics

The first firmware should contain compile-time or explicit bench modes for:

- fixed EXT color,
- four-color bars,
- checkerboard,
- horizontal/vertical line patterns,
- framebuffer counter/pattern,
- VBlank indication.

These modes are part of the validation strategy, not disposable debug code.
