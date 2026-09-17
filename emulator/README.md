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
      | 341x262 simplified NTSC raster
      v
comparison frontend
```

The comparison output is designed around a 1280x720 16:9 canvas with two formally framed video regions:

- left: **GAME BOY REFERENCE**, the normal Game Boy/SameBoy output;
- right: **RP2C02 EXT PATH**, the same source frame after the project bridge and RP2C02 palette path.

The right panel explicitly identifies the `256x240` PPU region and the `11 + 234 + 11` composition so geometry, centering, duplication patterns and palette differences remain visible rather than hidden by the UI.

## Current V0.3

V0.3 keeps the dependency-free path and adds the first temporal comparison model.

Implemented now:

- canonical horizontal `160 -> 234` center-sampled mapping;
- exact vertical `144 -> 240` `2,1,2` repetition pattern;
- `11 + 234 + 11` output composition;
- reduced logical `EXT -> palette RAM` RP2C02 model;
- simplified RP2C02 `341x262` raster timing with visible region and VBlank boundaries;
- deterministic 160x144 two-bit test-pattern source;
- generic `gb_source` interface;
- optional SameBoy source using the normal SameBoy framebuffer as the left reference;
- recovery of the four final DMG shade indices from that same SameBoy frame for the right-hand bridge path;
- shared 1280x720 side-by-side renderer with framed/labelled regions;
- dependency-free PPM frontend;
- optional live SDL2 frontend;
- selectable Game Boy clock model: `STOCK` or `SYNC`;
- automated regression tests for bridge geometry, source abstraction, RP2C02 raster timing and clock scheduling;
- GitHub Actions build/test coverage for the dependency-free core, SDL2 viewer build and pinned SameBoy link build.

The RGB LUT in `rp2c02_ext.c` is **only a provisional monitor approximation**. It is not yet a composite NTSC waveform model.

## Clock comparison

The virtual bench can compare the timing consequence that motivated the hardware clock modification.

### STOCK

```text
Game Boy clock: 4.194304 MHz
Game Boy frame: ~59.7275006 Hz
RP2C02 frame:   ~60.0984776 Hz
```

The two domains drift. The output scheduler therefore has to repeat a Game Boy frame periodically. With the current simplified timing model the frame-slip interval is about `2.696 s`.

### SYNC

```text
Game Boy clock: ~4.220355488 MHz
RP2C02 frame:   ~60.0984776 Hz
```

The Game Boy frame rate is locked to the simplified RP2C02 frame rate, so the scheduler advances exactly one source frame for each PPU output frame.

This does not change the contents of an individual Game Boy frame. It models the **temporal relationship between the two machines**.

## Dependency-free build

From the repository root:

```sh
cmake -S emulator -B build/emulator
cmake --build build/emulator
ctest --test-dir build/emulator --output-on-failure
./build/emulator/gbcrt_emu --clock sync --out build/emulator/comparison.ppm
```

To exercise the stock-clock drift scheduler without SameBoy:

```sh
./build/emulator/gbcrt_emu \
  --clock stock \
  --frames 1000 \
  --out build/emulator/comparison-stock.ppm
```

`--frames` counts RP2C02 output frames. In `STOCK` mode, the source does not necessarily advance for every output frame; in `SYNC` mode it advances 1:1.

## SameBoy-backed ROM build

SameBoy is deliberately kept outside this repository. The helper script pins the first integration to commit:

```text
213a12ce93d66b105a113debd9396306066a7cfc
```

Bootstrap and build it with:

```sh
sh ./emulator/scripts/bootstrap_sameboy.sh
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
  --clock sync \
  --frames 120 \
  --out build/emulator-sameboy/comparison.ppm
```

No commercial ROM or Nintendo boot ROM is stored in this repository.

SameBoy itself contains open boot-ROM source, so a later integration may switch to a reproducibly built SameBoy boot ROM instead of requiring a user-supplied Nintendo image. That is intentionally kept separate from the first source-adapter step.

## Optional live SDL2 viewer

The live viewer is not required for tests or the PPM frontend. On a system with SDL2 development files installed:

```sh
cmake -S emulator -B build/emulator-viewer \
  -DGBCRT_ENABLE_SDL2=ON
cmake --build build/emulator-viewer
./build/emulator-viewer/gbcrt_viewer --clock sync
```

For the live SameBoy comparison, enable both optional integrations:

```sh
cmake -S emulator -B build/emulator-live \
  -DGBCRT_ENABLE_SDL2=ON \
  -DGBCRT_ENABLE_SAMEBOY=ON \
  -DSAMEBOY_ROOT="$PWD/emulator/third_party/SameBoy"
cmake --build build/emulator-live

./build/emulator-live/gbcrt_viewer \
  --rom /path/to/game.gb \
  --boot /path/to/dmg_boot.bin \
  --clock sync
```

Viewer controls:

- `M`: open/close the clock menu;
- `Up` / `Down`: choose `STOCK` or `SYNC` while the menu is open;
- `Enter`: apply the highlighted clock mode;
- `C`: toggle `STOCK`/`SYNC` immediately;
- `Space`: pause/resume;
- `Esc`: close the menu, or quit when the menu is closed;
- `Q`: quit.

The window title reports source, clock mode, Game Boy frame number, RP2C02 output-frame count and repeated-output count.

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

The emulator models only what the hardware project depends on:

- 341x262 NTSC raster timing where timing matters;
- VBlank/frame boundaries;
- palette RAM behavior relevant to EXT input;
- EXT0..EXT3 external index path;
- later, composite/NTSC color generation if useful for validation.

CPU, APU, mappers, CHR rendering, nametables, OAM and sprites are out of scope.

Pinky/Visual2C02-derived tests and NESdev documentation are useful independent references for validating the reduced PPU model; they are not a reason to import a complete NES emulator.

## Continuous integration

`.github/workflows/emulator-ci.yml` validates three paths:

1. dependency-free build, tests, and `STOCK`/`SYNC` execution;
2. SDL2 viewer compilation and core tests;
3. pinned SameBoy library/link integration and core tests.

A green core build is the minimum requirement for emulator changes. SameBoy and SDL2 remain optional for end users, but their integration is checked automatically so optional code does not silently rot.
