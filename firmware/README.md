# Firmware

The source implementation is based on **Arduino IDE + Arduino-Pico** targeting **Raspberry Pi Pico 2 / RP2350**.

This choice is intentional: the Arduino workflow keeps installation, compilation, USB upload and basic debugging simple, while Arduino-Pico exposes the underlying Raspberry Pi Pico SDK so timing-critical code can use native **PIO, DMA and IRQ** instead of pixel-rate Arduino GPIO loops.

The canonical sketch is:

```text
firmware/arduino/GameBoyRP2C02CRT/GameBoyRP2C02CRT.ino
```

Current implementation level: **firmware V0.2**.

## Recommended development environment

1. Install Arduino IDE 2.x.
2. Add the Earle Philhower `arduino-pico` board-manager package.
3. Install/update the Raspberry Pi Pico/RP2040/RP2350 board package.
4. Select **Raspberry Pi Pico 2**.
5. Use the default ARM target for the first implementation.
6. Open `GameBoyRP2C02CRT.ino`, compile and upload normally by USB/BOOTSEL.

No additional Arduino libraries are required.

## Files in the Arduino sketch

```text
GameBoyRP2C02CRT.ino   main firmware / PPU / palette / framebuffer state
gb_capture_pio.h       PIO state machine for DMG/SGB LCD capture
gb_capture_engine.h    DMA transfer + raw-line normalization
```

The PIO instructions are encoded with the Pico SDK `pio_encode_*` helpers. A separately generated `pioasm` header is therefore not required for this first capture implementation.

## What V0.2 implements

- fixed Pico 2 GPIO map from `hardware/schematic-v0.1.md`;
- minimized write-only RP2C02 host bus;
- shared `EXT0..EXT3` / `D0..D3` GPIO topology;
- PPU register and palette writes;
- PPU warm-up handling and `/INT` VBlank counting;
- AUTO/SGB/manual palette button state machine;
- two packed 160x144x2-bit FRONT/BACK framebuffers;
- scaler-table generation for 160x144 -> 234x240 plus 11/11 border;
- **PIO-based Game Boy LCD capture** on GP0..GP5;
- **DMA transfer from the PIO RX FIFO** into a raw 144-line staging buffer;
- normalization from the physical LCD shift-register sequence into the project's packed framebuffer;
- protection against overwriting a BACK framebuffer still awaiting the PPU VBlank swap;
- static EXT-index output for independent RP2C02/palette/video bring-up.

## V0.2 capture model

The capture engine follows the behavior documented by prior Game Boy LCD captures:

```text
S rising        -> frame start
ST rising       -> line start
CP              -> LCD pixel shift clock
CPL rising      -> line latch
LD0/LD1         -> two-bit pixel value
```

The working model assumes **160 CP rising edges per active line**. The physical LCD path uses a 159-pixel shift register plus the direct LD0/LD1 value at CPL. Therefore the engine records:

```text
160 samples taken during CP-low before each CP rising edge
+ 1 final direct LD0/LD1 sample after the 160th CP falling edge
```

and reconstructs the visible line as:

```text
visible pixels 0..158 <- captured CP samples 1..159
visible pixel 159     <- final direct sample
```

The first captured CP sample is intentionally discarded, matching the documented LCD shift-register behavior.

PIO packs sixteen two-bit samples into each 32-bit RX word. Each active line produces:

```text
10 packed words for 160 CP samples
1 partial word for the final direct pixel
= 11 words/line

11 x 144 = 1,584 DMA words/frame
```

After 144 lines the DMA-complete interrupt stops the capture state machine. Software converts the raw frame to the normal 5,760-byte BACK framebuffer and re-arms the PIO/DMA engine for the next `S` frame edge.

## What remains bench-dependent

The V0.2 capture engine is a **theoretical implementation based on documented timing**, not yet a measured compatibility claim.

Bench validation must still check:

- the actual relationship of `LD0/LD1` to `CP` on the selected DMG and SGB hardware;
- that 160 accepted CP pulses are present on every active line under fine scrolling/window behavior;
- timing margin of the pre-rising-edge sampling strategy;
- exact `ST`, `CPL` and `S` phase relationships;
- behavior during LCD disable/re-enable or malformed/incomplete frames;
- electrical levels and power-sequencing behavior on the Pico 2 inputs.

If measurements show that one edge/phase assumption differs, the PIO program is intentionally isolated so it can be adjusted without changing framebuffer, scaler, palette or PPU-output architecture.

## Next firmware step

The remaining major timing-critical stub is the RP2C02 EXT output engine:

```text
front framebuffer
    -> fixed vertical/horizontal repetition
    -> 11-point border + 234-point image + 11-point border
    -> PIO/DMA
    -> EXT0..EXT3
```

That engine will replace the current static EXT test value. It must be tied to measured/validated RP2C02 raster timing rather than implemented with `digitalWrite()` at pixel rate.

SGB `P14/P15` palette-command decoding remains a later optional module.

## Compatibility target

Firmware uses a common **Game Boy DMG / SGB** source pipeline. Once a valid 160x144x2-bit frame is reconstructed, buffering, scaling, border generation, palette mapping and RP2C02 EXT output are common to both source families.

SGB-specific `P14/P15` packet decoding remains separate and optional.

## Constraints

- no pixel-rate interrupt bit-banging;
- no scaled 234x240 or 256x240 framebuffer requirement;
- no NES CPU emulation;
- no normal tile/sprite rendering in V1;
- no game recognition;
- no alternate stretch/zoom/crop modes in V1;
- SGB video compatibility uses the same downstream pipeline as DMG;
- full SGB emulation remains outside V1.

See [`architecture.md`](architecture.md), [`../hardware/schematic-v0.1.md`](../hardware/schematic-v0.1.md) and [`../docs/sgb-lite.md`](../docs/sgb-lite.md).
