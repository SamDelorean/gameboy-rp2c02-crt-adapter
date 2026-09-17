# Hybrid emulator / virtual bench

This directory is the software validation bench for the Game Boy RP2C02 CRT Adapter.

It intentionally does **not** emulate a complete NES. The target architecture is:

```text
Game Boy source core
      |
      | 2-bit LCD pixel stream / 160x144 frame
      v
virtual adapter bridge
      |
      | 160x144 -> 234x240
      | 11 border + image + 11 border
      v
reduced RP2C02 EXT model
      |
      v
comparison frontend
```

The long-term frontend will show two synchronized views side by side on a 16:9 canvas:

- left: normal Game Boy/SameBoy reference output;
- right: the same frame after the project's bridge and RP2C02 EXT/palette path.

This makes geometry, centering, duplication patterns and palette differences directly visible.

## Current V0.1

The first checked-in version is deliberately dependency-light. It contains:

- the canonical horizontal 160 -> 234 center-sampled mapping;
- the exact vertical 144 -> 240 2,1,2 repetition pattern;
- the 11 + 234 + 11 output composition;
- a reduced logical EXT -> palette-RAM RP2C02 model;
- a deterministic 160x144 two-bit test-pattern source;
- a 1280x720 side-by-side comparison renderer that writes a PPM image;
- an automated geometry test.

The RGB LUT in `rp2c02_ext.c` is **only a provisional monitor approximation**. It is not yet a composite NTSC waveform model.

## Build

```sh
cmake -S emulator -B build/emulator
cmake --build build/emulator
ctest --test-dir build/emulator --output-on-failure
./build/emulator/gbcrt_emu build/emulator/comparison.ppm
```

The generated `comparison.ppm` is a static preview of the planned comparison UI.

## SameBoy integration plan

SameBoy is the preferred first Game Boy source because its core is highly documented and exposes integration callbacks used for SFC/SNES/SGB-style embedding, including pixel and horizontal/vertical reset callbacks.

The integration boundary will remain narrow:

```text
SameBoy core
  -> 2-bit pixel events / frame reference
  -> gb_source adapter
  -> bridge
```

We do not need to fork a complete frontend. The normal SameBoy frame will be retained as the left-hand reference while the raw 2-bit path feeds the virtual adapter on the right.

No ROM images are stored in this repository.

## RP2C02 model policy

The emulator should model only what the hardware project actually depends on:

- 341x262 NTSC raster timing where timing matters;
- VBlank/frame boundaries;
- palette RAM behavior relevant to EXT input;
- EXT0..EXT3 external index path;
- later, composite/NTSC color generation if useful for validation.

CPU, APU, mappers, CHR rendering, nametables, OAM and sprites are out of scope.

Pinky/Visual2C02-derived tests and NESdev documentation are useful independent references for validating the reduced PPU model; they are not a reason to import a complete NES emulator.
