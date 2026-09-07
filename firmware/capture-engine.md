# Game Boy LCD Capture Engine — V0.2

Status: **theoretical implementation complete; bench validation pending**.

This document records the first concrete RP2350 PIO + DMA capture implementation for the Game Boy DMG / SGB source path.

## Source signals

Canonical Pico 2 mapping:

```text
GP0  LD0
GP1  LD1
GP2  CP
GP3  CPL
GP4  ST
GP5  S
```

The six signals are contiguous so PIO can read `LD0/LD1` as a two-bit field while independently waiting on the timing GPIOs.

## Documented LCD behavior used by V0.2

The implementation is based on published Game Boy LCD captures showing:

- rising `S` marks frame start;
- rising `ST` marks line start;
- `LD0/LD1` are shifted by rising `CP`;
- the LCD source driver behaves as a 159-pixel shift register plus a direct two-bit pixel at `CPL`;
- captures show 160 accepted `CP` rising edges on active lines;
- the first generated/shifted value is displaced out before the line latch;
- sampling naively on the falling edge of `CP` is easier but produces the commonly reported one-pixel horizontal shift.

V0.2 therefore models one visible line as:

```text
pre-CP samples:  S0 S1 S2 ... S159
                  X  |--------------|
                     visible 0..158

final direct LD0/LD1 after CP #160
                     -> visible pixel 159
```

`S0` is intentionally discarded.

## PIO strategy

The capture state machine:

1. waits for `S` low then rising, so it starts only at a complete new frame;
2. waits for `ST` high;
3. enters a known `CP`-low phase;
4. repeats 160 times:
   - samples `LD0/LD1` while `CP` is low;
   - waits for `CP` rising;
   - waits for `CP` falling;
5. samples `LD0/LD1` once more after the 160th falling edge;
6. waits for `CPL` rising;
7. pushes the final partial word;
8. waits for `ST` low and repeats for the next line.

The 160 CP samples are counted as five groups of 32 using the PIO X/Y scratch registers.

## PIO packing

Input shift direction is LEFT and autopush occurs every 32 bits.

Sixteen two-bit pixels therefore produce one RX FIFO word:

```text
bits 31..30  sample 0
bits 29..28  sample 1
...
bits  1..0   sample 15
```

Per active line:

```text
10 full words = 160 CP samples
1 partial word = final direct pixel
11 words total
```

Per frame:

```text
11 words/line x 144 lines = 1,584 32-bit DMA transfers
                              6,336 raw bytes
```

The 6,336-byte raw staging buffer is not a presentation framebuffer and does not change the project decision to retain two 5,760-byte source FRONT/BACK buffers.

## DMA / frame ownership

DMA drains the PIO RX FIFO for exactly 1,584 words. On completion:

- the DMA IRQ stops the PIO immediately;
- software normalizes the stable raw buffer into BACK;
- if BACK is already pending presentation, the new raw frame is dropped rather than overwriting it;
- capture is re-armed to wait for the next rising `S` edge.

This preserves the invariant that the buffer awaiting presentation is never modified.

## Raw-to-framebuffer normalization

For each line:

```text
visible 0   <- raw sample 1
visible 1   <- raw sample 2
...
visible 158 <- raw sample 159
visible 159 <- final direct sample
```

Four visible two-bit pixels are packed per byte in the existing project format, producing exactly 40 bytes per line and 5,760 bytes per frame.

## Why not capture on CP falling edge only?

Published captures report that `LD0/LD1` change extremely soon after the rising `CP` edge. Sampling at the falling edge is simple and often usable, but it corresponds to the next pixel value and yields a one-pixel horizontal shift if treated as the current shifted pixel.

The V0.2 PIO program instead samples during the low phase before the next rising edge and separately captures the final direct pixel.

## Items that remain OPEN until bench validation

The following are implementation assumptions, not yet hardware-confirmed project claims:

1. the exact timing margin from `ST`/`S` edges to the first accepted `CP` pulse;
2. 160 accepted `CP` pulses on every active line across games and fine-scroll/window cases;
3. the stability interval of the final direct LD0/LD1 value before `CPL`;
4. phase/timing differences between DMG and SGB hardware;
5. LCD disable/re-enable behavior and recovery from malformed/incomplete frames;
6. electrical edge quality and input behavior with the synchronized Game Boy clock modification.

If measurement changes one of these assumptions, only the isolated capture PIO/normalizer should need adjustment; framebuffer, scaler, border, palette and RP2C02 output architecture remain unchanged.

## Source files

```text
arduino/GameBoyRP2C02CRT/gb_capture_pio.h
arduino/GameBoyRP2C02CRT/gb_capture_engine.h
arduino/GameBoyRP2C02CRT/GameBoyRP2C02CRT.ino
```
