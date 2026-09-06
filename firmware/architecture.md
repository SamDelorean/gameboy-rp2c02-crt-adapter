# Firmware Architecture and Operating Principles

This document records the **software design basis** of the project. It is intentionally more specific than a normal implementation note: the goal is that the firmware can be reimplemented on another controller without having to reconstruct the reasoning from the project history.

The first implementation should favor **simple, deterministic algorithms** over general-purpose video-processing techniques.

## 1. Fundamental design rule

The firmware is not a video converter in the conventional sense.

It does not decode an analog signal, perform arbitrary resampling, emulate an NES, or synthesize a framebuffer at a higher color depth.

Its job is deliberately narrow:

```text
DMG LCD stream
    |
    v
capture 2-bit pixels
    |
    v
complete source framebuffer
    |
    v
fixed integer/rational repetition scaler
    |
    v
2-bit shade -> RP2C02 palette index
    |
    v
EXT0..EXT3
```

The RP2C02-compatible PPU remains responsible for NTSC raster timing, palette lookup/color generation, and composite-video output.

## 2. Firmware modules

The logical modules are:

```text
clock_init
ppu_init
dmg_capture
frame_buffers
fixed_scaler
ppu_ext_output
palette
sgb_listener       (optional)
user_input
diagnostics
```

The physical implementation may merge modules where appropriate, but these responsibilities should remain conceptually separate.

## 3. Source image representation

The Game Boy DMG active image is:

```text
160 x 144 pixels
2 bits per pixel
4 shades
```

A complete source frame therefore requires:

```text
160 x 144 x 2 bits = 46,080 bits = 5,760 bytes
```

The baseline design uses **two complete source framebuffers**:

```text
2 x 5,760 = 11,520 bytes = 11.25 KiB
```

This is intentional. RAM minimization is not a design objective for version 1.

### Suggested packing

A natural packed format is four 2-bit pixels per byte:

```text
bit 7..6 = pixel n
bit 5..4 = pixel n+1
bit 3..2 = pixel n+2
bit 1..0 = pixel n+3
```

The exact bit order may be changed if the capture peripheral makes another arrangement more efficient, but it must be documented and isolated behind access helpers/macros so that the scaler and tests do not depend on an undocumented packing convention.

## 4. Ping-pong framebuffer ownership

Use two complete buffers:

- **back buffer**: receives the frame currently being captured from the DMG;
- **front buffer**: is read by the output/scaler path.

The two roles are exchanged only at a defined complete-frame boundary.

The intended state machine is:

```text
capture frame N into BACK
        |
        v
frame N complete
        |
        v
mark BACK complete
        |
        v
at safe presentation boundary:
FRONT <-> BACK
        |
        v
capture frame N+1 while displaying frame N
```

### Invariant

The output path must never read from the buffer currently being modified by the capture path.

This avoids tearing and makes timing analysis much simpler than a line-buffer design.

## 5. Why fixed nearest-neighbor scaling

The source and destination dimensions have exact rational relationships:

```text
horizontal: 256 / 160 = 8 / 5
vertical:   240 / 144 = 5 / 3
```

Therefore there is no reason to use:

- bilinear interpolation,
- bicubic interpolation,
- floating-point arithmetic,
- arbitrary-ratio resampling,
- filtering kernels,
- fractional frame accumulation.

The desired aesthetic is also appropriate for Game Boy graphics: preserve each original 2-bit pixel value exactly and enlarge only by **repetition**.

## 6. Horizontal scaler: 160 -> 256

Every group of five source pixels becomes eight output pixels.

A center-phased nearest-neighbor repetition pattern is:

```text
source:   A B C D E
repeat:   2 1 2 1 2
output:   A A B C C D E E
```

This pattern is repeated exactly 32 times:

```text
32 groups x 5 source pixels = 160
32 groups x 8 output pixels = 256
```

### Preferred implementation

Do not calculate a division for each destination pixel if the target peripheral architecture makes a periodic generator simpler.

Conceptually:

```c
static const uint8_t h_repeat[5] = {2, 1, 2, 1, 2};

for each source line:
    for group = 0..31:
        for phase = 0..4:
            pixel = next_source_pixel();
            emit pixel h_repeat[phase] times;
```

The actual implementation may instead use PIO/DMA descriptors, a lookup table, unrolled code, or a small state machine.

The **observable mapping must remain equivalent**.

## 7. Vertical scaler: 144 -> 240

Every group of three source lines becomes five output lines.

Use the analogous center-phased pattern:

```text
source lines: L0 L1 L2
repeat:        2  1  2
output:       L0 L0 L1 L2 L2
```

This pattern is repeated exactly 48 times:

```text
48 groups x 3 source lines = 144
48 groups x 5 output lines = 240
```

### Preferred implementation

```c
static const uint8_t v_repeat[3] = {2, 1, 2};

for source_line_group = 0..47:
    for phase = 0..2:
        line = source_line[phase];
        output_scaled_line(line) v_repeat[phase] times;
```

Again, the real implementation may be a DMA/PIO state machine rather than literal C loops.

## 8. Equivalent coordinate mapping

For verification and reference, the same center-phased nearest-neighbor mapping can be expressed mathematically as:

```text
source_x = floor((output_x + 0.5) * 160 / 256)
source_y = floor((output_y + 0.5) * 144 / 240)
```

which reduces to:

```text
source_x = floor((output_x + 0.5) * 5 / 8)
source_y = floor((output_y + 0.5) * 3 / 5)
```

The periodic repetition tables above are preferred because they make the implementation and timing obvious.

## 9. No intermediate 256 x 240 framebuffer is required

Version 1 should store the **160 x 144 source frame only**.

The 256 x 240 representation can be generated while reading the front buffer for output.

This provides several advantages:

- less RAM traffic;
- less memory use;
- no extra full-frame copy;
- scaling behavior is deterministic;
- palette changes do not require rewriting pixels;
- the framebuffer remains a faithful copy of the original DMG shade data.

If a future controller architecture benefits materially from a pre-scaled buffer, that may be evaluated separately, but it is not the baseline design.

## 10. Pixel values remain palette-independent

The framebuffer stores **shade indices**, not RGB values.

Conceptually:

```text
00 = DMG shade 0
01 = DMG shade 1
10 = DMG shade 2
11 = DMG shade 3
```

The active palette determines what RP2C02 color each shade becomes.

Therefore changing palette does not alter the framebuffer and does not invoke the scaler.

```text
2-bit framebuffer pixel
        |
        +--> EXT1:EXT0

palette/bank selection
        |
        +--> EXT3:EXT2 (as required by the selected mode)
```

The exact EXT coding is kept behind the output module so that future palette-bank use does not contaminate the capture/scaler code.

## 11. DMG capture path

Capture must be deterministic and peripheral/DMA driven where practical.

For the final target controller, document explicitly:

- which physical DMG signal qualifies a valid pixel;
- the sampling edge;
- LD0/LD1 bit significance;
- active-line start/end recognition;
- active-frame start/end recognition;
- blanking handling;
- DMA transfer unit;
- packed framebuffer format;
- overrun behavior;
- malformed/incomplete-frame behavior.

### Important rule

Do not assume that all 456 DMG dots of a line correspond to the 160 visible pixels.

Only the active LCD pixel burst is captured into the framebuffer. The precise relationship must be validated against hardware measurements/documentation before the capture state machine is frozen.

## 12. Output path

The RP2C02 EXT stream must be deterministic and must not depend on foreground CPU interrupt latency.

Preferred mechanisms include:

- PIO/state-machine output;
- DMA-fed peripheral output;
- equivalent deterministic hardware peripheral on the final controller.

Pixel-rate GPIO bit-banging from ordinary interrupts is explicitly outside the intended architecture.

## 13. Frame synchronization

The project intentionally modifies/controls the DMG clock so that the DMG frame period and the RP2C02 frame period can be locked to a common reference.

Working values are documented elsewhere, but the firmware architecture assumes:

```text
one completed DMG source frame
        corresponds to
one RP2C02 output frame
```

The RP2C02 `/INT` VBlank output is the preferred presentation reference for:

- front/back buffer exchange;
- palette updates;
- user-interface state changes;
- optional diagnostics.

The exact swap point must be chosen so the output engine never changes source buffers in the middle of the visible picture.

## 14. Palette handling

Palette selection is deliberately simple.

The user has one button that cycles through a curated list of complete four-color palettes.

Conceptually:

```c
if (button_pressed) {
    palette_index = (palette_index + 1) % palette_count;
    palette_pending = true;
}

if (vblank && palette_pending) {
    ppu_load_palette(palettes[palette_index]);
    palette_pending = false;
}
```

Palette writes should occur during a safe VBlank interval.

No per-pixel or per-region color computation is required in manual mode.

## 15. Optional SGB-lite path

The passive SGB listener is an independent optional input path.

```text
P14/P15
   |
packet decoder
   |
direct SGB palette command
   |
RGB555 -> nearest suitable RP2C02 colors
   |
normal palette-loading mechanism
```

Initial direct commands:

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

The SGB path does **not** alter the capture or scaling algorithms.

If no usable SGB command appears, the normal manual palette remains active.

## 16. Overscan / border behavior

Version 1 uses fixed black outside the Game Boy picture area where the chosen PPU/EXT timing requires an explicit border value.

This behavior should be generated by the output state machine, not written into the source framebuffer.

The border is not a user-facing option in version 1.

## 17. Boot sequence

Recommended sequence:

1. Put all GPIO in electrically safe states.
2. Hold the RP2C02-compatible PPU in reset.
3. Configure/start the common clock system as required.
4. Initialize capture and deterministic output peripherals.
5. Initialize framebuffer ownership/state.
6. Release and initialize the PPU.
7. Load a known safe default palette.
8. Capture one complete valid DMG frame into the back buffer.
9. Promote it to front buffer at the defined boundary.
10. Start normal EXT output.
11. Enter steady-state operation.

## 18. Error philosophy

Version 1 should fail visibly and predictably rather than attempting elaborate recovery algorithms.

Examples:

- incomplete DMG frame: discard it and retain the previous front frame;
- capture overrun: flag diagnostic state and do not present the corrupted buffer;
- missing SGB packet: ignore it;
- invalid SGB packet: discard it;
- palette update missed during VBlank: defer to the next VBlank;
- button bounce: debounce at low rate; never let it affect pixel timing.

The last known complete framebuffer should remain displayable whenever possible.

## 19. Diagnostics are part of the design

The firmware should provide explicit bench modes for:

- fixed EXT color;
- four-color bars;
- checkerboard;
- horizontal-line pattern;
- vertical-line pattern;
- framebuffer address/count pattern;
- VBlank indicator;
- capture frame counter;
- optional timing GPIO markers.

These modes are not disposable debug code. They form the reproducible validation procedure for the controller, the RP2C02, and compatible clone PPUs.

## 20. Controller-independent invariants

Even if the final controller changes from RP2040 to another MCU/FPGA, retain these principles unless a documented design revision explicitly supersedes them:

1. Capture the original 2-bit DMG LCD data directly.
2. Keep two complete 160 x 144 source buffers in version 1.
3. Never modify the front buffer while it is being displayed.
4. Scale only by deterministic nearest-neighbor repetition.
5. Horizontal scaling is exactly 5 source pixels -> 8 output pixels.
6. Vertical scaling is exactly 3 source lines -> 5 output lines.
7. Do not require a 256 x 240 intermediate framebuffer.
8. Keep pixel storage independent of palette/color values.
9. Drive EXT deterministically with hardware-assisted I/O.
10. Perform palette updates at a safe PPU interval.
11. Keep SGB support optional and independent of normal operation.
12. Prefer a small number of transparent state machines over clever generalized video-processing code.

## 21. Rationale

The purpose of these choices is not merely to reduce code size.

They make the system:

- easier to understand;
- easier to validate on an oscilloscope/logic analyzer;
- easier to port;
- easier for another contributor to reproduce;
- less sensitive to CPU load;
- less likely to hide timing errors behind large software abstractions.

The project should remain a **small deterministic bridge between two pieces of period video hardware**, not evolve accidentally into a generic graphics processor.
