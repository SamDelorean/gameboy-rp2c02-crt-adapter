# Hybrid emulator / virtual bench

This directory is the software validation bench for the Game Boy RP2C02 CRT Adapter.

It intentionally does **not** emulate a complete NES. The target architecture is:

```text
Game Boy source core
      |
      | normal reference frame + 2-bit four-shade frame
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

The comparison output is designed around a 16:9 canvas:

- left: normal Game Boy/SameBoy reference output;
- right: the same frame after the project's bridge and RP2C02 EXT/palette path.

This makes geometry, centering, duplication patterns and palette differences directly visible.

## Current V0.2

V0.2 keeps the dependency-free V0.1 path and adds a pluggable Game Boy source interface plus an optional SameBoy-backed ROM source.

Implemented now:

- canonical horizontal `160 -> 234` center-sampled mapping;
- exact vertical `144 -> 240` `2,1,2` repetition pattern;
- `11 + 234 + 11` output composition;
- reduced logical `EXT -> palette RAM` RP2C02 model;
- deterministic 160x144 two-bit test-pattern source;
- generic `gb_source` interface;
- optional SameBoy source using the normal SameBoy framebuffer as the left reference;
- recovery of the four final DMG shade indices from that same SameBoy frame for the right-hand bridge path;
- 1280x720 side-by-side comparison renderer that writes a PPM image;
- automated bridge-geometry regression test.

The RGB LUT in `rp2c02_ext.c` is **only a provisional monitor approximation**. It is not yet a composite NTSC waveform model.

## Dependency-free build

From the repository root:

```sh
cmake -S emulator -B build/emulator
cmake --build build/emulator
ctest --test-dir build/emulator --output-on-failure
./build/emulator/gbcrt_emu --out build/emulator/comparison.ppm
```

This uses the built-in two-bit pattern source and does not require SameBoy.

## SameBoy-backed ROM build

SameBoy is deliberately kept outside this repository. A helper script pins the first integration to commit:

```text
213a12ce93d66b105a113debd9396306066a7cfc
```

Bootstrap and build it with:

```sh
./emulator/scripts/bootstrap_sameboy.sh
```

Then configure this emulator with the resulting checkout:

```sh
cmake -S emulator -B build/emulator-sameboy \
  -DGBCRT_ENABLE_SAMEBOY=ON \
  -DSAMEBOY_ROOT="$PWD/emulator/third_party/SameBoy"

cmake --build build/emulator-sameboy
```

Run a user-supplied ROM and DMG boot ROM:

```sh
./build/emulator-sameboy/gbcrt_emu \
  --rom /path/to/game.gb \
  --boot /path/to/dmg_boot.bin \
  --frames 120 \
  --out build/emulator-sameboy/comparison.ppm
```

No commercial ROM or Nintendo boot ROM is stored in this repository.

SameBoy itself contains open boot-ROM source, so a later integration may switch to a reproducibly built SameBoy boot ROM instead of requiring a user-supplied Nintendo image. That is intentionally kept separate from the first source-adapter step.

## Why the first SameBoy adapter works at frame level

For the first ROM-backed comparison we intentionally do not patch SameBoy's pixel pipeline.

SameBoy renders the normal 160x144 DMG frame using a fixed DMG palette. The adapter preserves that framebuffer for the **left reference image**, then maps those four known RGB values back to shade indices `0..3` for the project's **right-hand bridge path**.

This provides one Game Boy execution core and one frame source for both views:

```text
             SameBoy DMG core
                    |
             rendered 160x144
               /           \
              /             \
     normal reference    four-shade recovery
          left                 |
                               v
                         project bridge
                               |
                               v
                         RP2C02 EXT model
                              right
```

This is sufficient for validating geometry, palette mapping and frame-level behavior.

A later signal-level mode may use or extend SameBoy's SFC/SNES integration callbacks to expose pixel/H-reset/V-reset events directly when we want to compare the theoretical `LD0/LD1/CP/CPL/ST/S` capture model against emulator timing.

## RP2C02 model policy

The emulator should model only what the hardware project actually depends on:

- 341x262 NTSC raster timing where timing matters;
- VBlank/frame boundaries;
- palette RAM behavior relevant to EXT input;
- EXT0..EXT3 external index path;
- later, composite/NTSC color generation if useful for validation.

CPU, APU, mappers, CHR rendering, nametables, OAM and sprites are out of scope.

Pinky/Visual2C02-derived tests and NESdev documentation are useful independent references for validating the reduced PPU model; they are not a reason to import a complete NES emulator.
