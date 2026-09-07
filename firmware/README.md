# Firmware

The first source implementation is now based on **Arduino IDE + Arduino-Pico** targeting **Raspberry Pi Pico 2 / RP2350**.

This choice is intentional: the Arduino workflow keeps installation, compilation, USB upload and basic debugging simple, while the Arduino-Pico core is built on the Raspberry Pi Pico SDK and allows the firmware to use native RP2350 PIO, DMA, IRQ and hardware APIs where deterministic video timing requires them.

The canonical V0.1 sketch is:

```text
firmware/arduino/GameBoyRP2C02CRT/GameBoyRP2C02CRT.ino
```

## Recommended development environment

1. Install Arduino IDE 2.x.
2. Add the Arduino-Pico board-manager package from the Earle Philhower `arduino-pico` project.
3. Install/update the `Raspberry Pi Pico/RP2040/RP2350` board package.
4. Select **Raspberry Pi Pico 2** as the board.
5. Use the default ARM target for the first project implementation.
6. Open `GameBoyRP2C02CRT.ino`, compile, and upload by USB/BOOTSEL in the normal Pico workflow.

No additional Arduino libraries are required by V0.1.

## What V0.1 implements now

- the fixed Pico 2 GPIO map from `hardware/schematic-v0.1.md`;
- direct Game Boy DMG / SGB source-input pin setup;
- the minimized write-only RP2C02 host bus;
- the shared `EXT0..EXT3` / `D0..D3` GPIO topology;
- PPU register writes to `$2000`, `$2001`, `$2006`, `$2007`;
- PPU warm-up delay and rendering-disabled EXT-input operation;
- palette RAM loading through the RP2C02;
- safe-black fill for unused EXT palette indices;
- restoration of the PPU VRAM address outside palette RAM after palette writes;
- `/INT` / VBlank interrupt counting;
- single-button AUTO/SGB/manual-palette state machine;
- two complete packed 160x144x2-bit source framebuffers;
- deterministic scaler-table generation for 160x144 -> 234x240;
- scaler self-test verifying every source pixel/line repeats once or twice and the totals are exactly 234x240;
- startup four-shade test framebuffer;
- static EXT-index output for early RP2C02/palette/video bench tests;
- explicit capture, EXT-output and SGB-listener module boundaries for the next implementation step;
- USB serial diagnostics without consuming GPIO.

The four manual palettes in V0.1 are **provisional bring-up values**, not the final curated project palette set.

## Deliberately not guessed in V0.1

The timing-critical pixel engines are not implemented by ordinary Arduino GPIO loops.

The following functions are intentionally stubs until bench measurements freeze the relevant timing details:

```text
captureEngineInit()
captureEngineService()
extOutputEngineInit()
extOutputEngineService()
sgbListenerService()
```

V0.2 will replace the capture/output stubs with **PIO + DMA** or equivalent native RP2350 hardware-assisted logic.

Before writing the capture PIO program, the project will verify:

- exact `CP` sampling edge;
- `LD0/LD1` bit significance;
- active 160-pixel window;
- exact roles/redundancy of `CPL`, `ST`, `S`;
- suppressed/fine-scroll clock behavior;
- source timing after the synchronized clock modification.

Before writing the EXT-output PIO program, the project will verify the exact relationship between the RP2C02 master clock, output dot timing, `/INT`, and the externally driven EXT index window.

## Compatibility target

Firmware uses a common **Game Boy DMG / SGB** source pipeline. Once a valid 160x144x2-bit frame is reconstructed, buffering, scaling, border generation, palette mapping and RP2C02 EXT output are common to both source families.

SGB-specific `P14/P15` packet decoding remains a separate optional module and must not contaminate the common capture/scaler path.

## Version 1 responsibilities

1. initialize the controller and external clock-control interface where required;
2. minimally initialize the RP2C02-compatible PPU;
3. load the active palette;
4. capture the Game Boy DMG / SGB-compatible LCD/video stream;
5. maintain ping-pong 160x144x2-bit framebuffers;
6. scale vertically `144 -> 240` using fixed `5/3` repetition;
7. scale horizontally `160 -> 234` using deterministic integer nearest-neighbor repetition;
8. generate the 11-dot left/right border regions separately from the scaler;
9. drive `EXT0..EXT3` deterministically;
10. coordinate presentation/palette operations around VBlank;
11. scan the single palette button;
12. optionally decode the minimal SGB palette subset from `P14/P15`;
13. guarantee manual palette override of SGB-derived colors;
14. provide bench-test patterns and diagnostics.

## Constraints

- no pixel-rate interrupt bit-banging;
- no scaled 234x240 or 256x240 framebuffer requirement;
- no NES CPU emulation;
- no normal tile/sprite rendering in version 1;
- no game recognition;
- no alternate stretch/zoom/crop scaling modes in version 1;
- SGB video compatibility uses the same downstream pipeline as DMG;
- SGB-lite palette listening remains optional;
- full SGB emulation is outside version 1;
- timing constants, source-interface details and palette data remain isolated from hardware-independent scaler/output logic.

See [`architecture.md`](architecture.md), [`../hardware/schematic-v0.1.md`](../hardware/schematic-v0.1.md) and [`../docs/sgb-lite.md`](../docs/sgb-lite.md).
