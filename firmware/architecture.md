# Firmware Architecture and Operating Principles

This document records the **software design basis** of the project. The goal is that the firmware can be reimplemented on another controller without reconstructing the design history from chat logs.

The first implementation favors **small deterministic state machines** over general-purpose video-processing techniques.

## 1. Fundamental design rule

The firmware is not a generic video converter. It does not decode analog video, emulate an NES, or perform arbitrary resampling.

Its narrow job is:

```text
DMG LCD stream
    |
    v
capture 2-bit pixels
    |
    v
complete 160 x 144 source framebuffer
    |
    v
aspect-correct repetition scaler
    |
    v
border/frame generator + palette index
    |
    v
EXT0..EXT3
```

The RP2C02-compatible PPU remains responsible for NTSC raster timing, color generation and composite-video output.

## 2. Logical firmware modules

```text
clock_init
ppu_init
dmg_capture
frame_buffers
fixed_scaler
border_generator
ppu_ext_output
palette
sgb_listener       (optional)
user_input
diagnostics
```

The implementation may merge modules physically, but these responsibilities should remain conceptually separate.

## 3. Source image representation

The Game Boy DMG active image is:

```text
160 x 144 pixels
2 bits per pixel
4 shades
```

One source frame requires:

```text
160 x 144 x 2 bits = 5,760 bytes
```

Version 1 uses two complete source framebuffers:

```text
2 x 5,760 = 11,520 bytes = 11.25 KiB
```

The buffers store only the original DMG shade indices, not RGB values and not a pre-scaled image.

A practical packed representation is four 2-bit pixels per byte. The exact bit order may be chosen to suit the capture peripheral but must be documented and isolated behind access helpers.

## 4. Ping-pong framebuffer ownership

- **BACK** receives the frame currently being captured from the DMG.
- **FRONT** contains the last complete frame being displayed.
- FRONT and BACK exchange roles only at a defined complete-frame boundary.
- The output path must never read from a buffer while the capture path modifies it.

Conceptually:

```text
capture frame N into BACK
        |
frame complete
        |
mark BACK valid
        |
at safe presentation boundary
FRONT <-> BACK
        |
capture frame N+1 while displaying frame N
```

If a captured frame is incomplete or corrupt, discard it and continue displaying the last valid FRONT frame.

## 5. Aspect-correct presentation is the baseline

Version 1 does **not** stretch the 160 x 144 Game Boy image across the complete 256 x 240 PPU raster.

The selected presentation is:

```text
11 border | 234-dot Game Boy picture | 11 border
          x 240 lines high
```

In version 1 the two border regions are fixed black.

The Game Boy image therefore fills the raster height while preserving its intended geometry much more closely on an NTSC CRT.

This is the fixed version-1 scaling mode. Alternate stretch/crop modes are not part of the baseline user interface.

See [`../docs/scaling.md`](../docs/scaling.md) for the derivation and reference mapping.

## 6. Vertical scaler: 144 -> 240

The vertical relationship is exact:

```text
240 / 144 = 5 / 3
```

Every three source lines become five output lines:

```text
source: L0 L1 L2
repeat:  2  1  2
output: L0 L0 L1 L2 L2
```

Repeat the pattern 48 times per frame:

```text
48 x 3 = 144 source lines
48 x 5 = 240 output lines
```

No interpolation, filtering or floating point is required.

## 7. Horizontal scaler: 160 -> 234

The horizontal scaler emits every source pixel at least once and duplicates only the additional pixels required to reach 234 output dots.

```text
160 original emissions
+ 74 duplicate emissions
= 234 output dots
```

Each source pixel is therefore emitted either once or twice.

A simple integer accumulator is sufficient:

```c
error = centered_initial_phase;

for (x = 0; x < 160; x++) {
    pixel = source[x];
    emit(pixel);

    error += 74;
    if (error >= 160) {
        emit(pixel);
        error -= 160;
    }
}
```

The initial phase should distribute duplicates without visible left/right bias.

For verification, the center-sampled reference mapping is:

```text
source_x = floor((output_x + 0.5) * 160 / 234)
```

for `output_x = 0..233`.

The resulting line contains:

- 86 source pixels emitted once;
- 74 source pixels emitted twice;
- 234 output image dots total.

Since `gcd(160,234)=2`, an implementation may alternatively store a fixed 80-source-pixel -> 117-output-dot repetition pattern and use it twice per line.

## 8. Border / frame generator

Each visible PPU line is generated as:

```text
11 border dots
234 scaled image dots
11 border dots
```

Total:

```text
11 + 234 + 11 = 256
```

The two side regions are produced by the **border/frame generator**, not by modifying the Game Boy image and not by storing extra pixels in FRONT/BACK.

Version 1 fixes:

```text
border_mode = fixed black
```

The important architectural separation is:

- the scaler always maps 160 source pixels to 234 image dots;
- the border generator owns the remaining 22 raster positions;
- the framebuffer stores neither borders nor border color/effects.

This keeps open a low-cost future extension in which the border generator could emit a selected color, a color related to the active palette, or another simple deterministic effect. Such a future extension must not require changing the scaler or introducing a general graphics layer.

No border-selection UI is required in version 1.

## 9. No scaled framebuffer is required

Version 1 stores only the original 160 x 144 source frame.

Neither a 234 x 240 framebuffer nor a 256 x 240 framebuffer is required. Scaling and border generation happen while FRONT is read for output.

Advantages:

- no full-frame scaling copy;
- less RAM traffic;
- less memory use;
- deterministic mapping;
- palette changes never rewrite pixels;
- border changes, if introduced later, do not rewrite pixels;
- the framebuffer remains a faithful copy of the original DMG shade data.

A scaled framebuffer may be reconsidered only if measurements on the final controller show a compelling implementation benefit.

## 10. Palette-independent pixels

The framebuffer stores shade indices only:

```text
00 = DMG shade 0
01 = DMG shade 1
10 = DMG shade 2
11 = DMG shade 3
```

The selected palette determines which RP2C02 color each shade becomes.

Therefore:

- scaling duplicates shade indices, not colors;
- changing the palette does not touch FRONT/BACK;
- manual and SGB-derived palettes use the same output path;
- the border generator may remain independent of the four Game Boy shades.

The exact EXT coding remains isolated inside the output module.

## 11. DMG capture path

Capture must be deterministic and peripheral/DMA-driven where practical.

The final implementation must document:

- which DMG signal qualifies a valid pixel;
- sampling edge;
- LD0/LD1 significance;
- active-line start/end recognition;
- active-frame start/end recognition;
- blanking handling;
- DMA transfer unit;
- packed framebuffer format;
- overrun behavior;
- malformed/incomplete-frame behavior.

Do not assume all 456 DMG clocks in a line correspond to visible pixels. Only the 160 active LCD pixels are stored.

## 12. RP2C02 EXT output path

EXT0..EXT3 output must be deterministic and independent of ordinary foreground interrupt latency.

Preferred mechanisms:

- PIO/state-machine output;
- DMA-fed peripheral output;
- equivalent deterministic hardware on the final controller.

Pixel-rate GPIO bit-banging from ordinary interrupts is outside the intended architecture.

## 13. Frame synchronization

The project intentionally controls the Game Boy clock so that the DMG frame period and the simplified RP2C02 frame period derive from a common timing reference.

Firmware assumes:

```text
one complete DMG source frame
        corresponds to
one complete RP2C02 output frame
```

The RP2C02 `/INT` VBlank output is the preferred safe reference for:

- FRONT/BACK presentation changes;
- palette writes;
- low-rate user-interface state changes;
- diagnostics.

The two source framebuffers solve capture/display ownership and tearing; they are not a frame-rate-conversion reservoir.

## 14. Palette handling and one-button UI

Version 1 uses one momentary button.

Recommended mode cycle:

```text
AUTO/SGB
manual preset 1
manual preset 2
...
manual preset N
-> AUTO/SGB
```

Visible palette writes occur during VBlank or another verified safe PPU interval.

Manual user selection has priority over incoming SGB palette traffic. A button press while an SGB-derived palette is visible exits AUTO/SGB and selects a manual preset.

## 15. Optional SGB-lite path

Optional inputs:

```text
P14
P15
```

Initial direct commands of interest:

```text
PAL01
PAL23
PAL03
PAL12
```

Processing model:

```text
P14/P15
   |
packet decoder
   |
RGB555 palette
   |
convert to suitable RP2C02 colors
   |
cache palette
   |
apply only if AUTO/SGB is selected
```

The SGB path does not alter capture, framebuffer or scaling logic.

While a manual preset is active, SGB packets may still be decoded and cached but must not replace the visible palette.

## 16. Boot sequence

Recommended initial sequence:

1. Place GPIO in safe states.
2. Hold the PPU in reset.
3. Configure/start the common clock system.
4. Initialize capture, scaler, border generator and output peripherals.
5. Initialize framebuffer ownership.
6. Release and initialize the PPU.
7. Load a known safe default Game Boy palette and fixed-black border value.
8. Capture one complete valid DMG frame into BACK.
9. Promote it to FRONT at the defined boundary.
10. Start normal EXT output.
11. Enter steady-state operation.

## 17. Error philosophy

Version 1 should fail visibly and predictably rather than implement elaborate recovery algorithms.

Examples:

- incomplete frame -> discard and retain previous FRONT;
- capture overrun -> flag diagnostic state and do not present corrupted buffer;
- invalid SGB packet -> discard;
- missed palette VBlank -> defer to next VBlank;
- button bounce -> debounce outside pixel timing.

## 18. Diagnostics are part of the product

Maintain reproducible bench modes for:

- fixed EXT color;
- four-color bars;
- checkerboard;
- horizontal and vertical line patterns;
- framebuffer address/count pattern;
- border geometry markers;
- explicit border-generator test color;
- VBlank indicator;
- capture frame counter;
- optional timing GPIO markers.

These are part of the validation procedure, not disposable debug code.

## 19. Controller-independent invariants

Unless a documented design revision supersedes them:

1. Capture the original 2-bit DMG LCD stream directly.
2. Keep two complete 160 x 144 source buffers.
3. Never modify FRONT while it is being displayed.
4. Preserve the Game Boy image aspect rather than stretching it to the full 256-dot width.
5. Present the image as 234 x 240 inside the 256 x 240 raster.
6. Reserve 11 output dots on each side for the border/frame generator; version 1 emits fixed black.
7. Keep border generation logically separate from scaling and source framebuffer storage.
8. Scale vertically by exact 3-source-lines -> 5-output-lines repetition.
9. Scale horizontally by deterministic 160 -> 234 one-or-two-times pixel repetition.
10. Do not require a 234 x 240 or 256 x 240 intermediate framebuffer.
11. Keep pixel storage independent of palette/color values.
12. Drive EXT with hardware-assisted deterministic I/O.
13. Perform palette changes during a safe PPU interval.
14. Keep SGB support optional and subordinate to manual user control.
15. Prefer transparent integer state machines over generalized graphics algorithms.

## 20. Rationale

The chosen architecture is intentionally simple:

- preserve the original Game Boy geometry;
- fill the CRT vertically;
- use the remaining width as a dedicated border/frame region;
- keep that region black in version 1 while preserving a clean future hook for simple cosmetic behavior;
- avoid interpolation and floating point;
- avoid a general-purpose video scaler;
- avoid a scaled framebuffer;
- remain easy to verify with an oscilloscope or logic analyzer;
- remain portable to another MCU or FPGA.

The project should remain a **small deterministic bridge between two pieces of period video hardware**, not evolve into a generic graphics processor.