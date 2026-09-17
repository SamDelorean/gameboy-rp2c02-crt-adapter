# Hybrid emulator / virtual bench

This directory is the software validation bench for the Game Boy RP2C02 CRT Adapter.

## SameBoy-first architecture

The virtual bench deliberately reuses **SameBoy as the Game Boy implementation**. We do not intend to build a second Game Boy emulator inside this repository.

SameBoy is the source of truth for Game Boy CPU, memory, cartridge behavior, LCD/PPU behavior, frame timing, joypad and the normal reference image. Project-owned emulator code focuses on the alternate video path:

```text
SameBoy Game Boy core
      |
      | normal reference frame + final four-shade Game Boy image
      v
project alternate-video bridge
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

The project does **not** emulate a complete NES either. The RP2C02 side models only the behavior needed by the proposed hardware adapter.

This division is intentional: if SameBoy already models a piece of Game Boy behavior, prefer using it rather than recreating that behavior locally.

The comparison output is designed around a 1280x720 16:9 canvas with two formally framed video regions:

- left: **GAME BOY REFERENCE**, the normal SameBoy output;
- right: **RP2C02 EXT PATH**, the same source image after the project bridge and RP2C02 palette/timing path.

The right panel explicitly identifies the `256x240` PPU region and the `11 + 234 + 11` composition so geometry, centering, duplication patterns and palette differences remain visible rather than hidden by the UI.

## Current V0.3

V0.3 keeps the dependency-free path and adds the first temporal and interactive comparison model.

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
- backend-independent eight-button Game Boy joypad interface;
- SameBoy joypad mapping through its public `GB_set_key_state()` API;
- shared 1280x720 side-by-side renderer with framed/labelled regions;
- dependency-free PPM frontend;
- optional playable SDL2 frontend;
- selectable Game Boy clock model: `STOCK` or `SYNC`;
- automated regression tests for bridge geometry, source abstraction/joypad, RP2C02 raster timing and clock scheduling;
- GitHub Actions build/test coverage for the dependency-free core, SDL2 viewer build and pinned SameBoy link build.

The pinned SameBoy library, our adapter, and the SDL2 viewer all compile in CI. No commercial Game Boy ROM is bundled with the project.

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

The CI timing test also executes 1000 output frames in both modes. `SYNC` produces zero repeated frames; the current `STOCK` model produces the expected periodic repeats from frame-rate drift.

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

Bootstrap and build the static core with:

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

A project-authored/open test ROM and boot stub may be used in CI so the SameBoy-backed path can actually execute Game Boy code without proprietary assets.

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

### Game Boy controls

When the clock menu is closed:

- arrow keys: D-pad;
- `Z`: A;
- `X`: B;
- `Backspace`: Select;
- `Enter`: Start.

All Game Boy keys are released automatically when the SDL window loses focus or when the clock menu opens, preventing stuck inputs.

### Viewer controls

- `M`: open/close the clock menu;
- `Up` / `Down`: choose `STOCK` or `SYNC` while the menu is open;
- `Enter`: apply the highlighted clock mode while the menu is open;
- `C`: toggle `STOCK`/`SYNC` immediately;
- `Space`: pause/resume;
- `Esc`: close the menu, or quit when the menu is closed;
- `Q`: quit.

The window title reports source, clock mode, Game Boy frame number, RP2C02 output-frame count and repeated-output count.

## Why the SameBoy adapter works at frame level

For the primary ROM-backed comparison we intentionally use SameBoy's normal final framebuffer instead of recreating its LCD pipeline.

SameBoy renders the normal 160x144 DMG frame using a fixed DMG palette. The adapter preserves that framebuffer for the **left reference image**, then maps those four known RGB values back to shade indices `0..3` for the project's **right-hand bridge path**.

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

This is the intended main software architecture because the project only needs an alternate video output, not another Game Boy emulator.

### Optional signal-level diagnostics

Exact `LD0/LD1/CP/CPL/ST/S`-style validation is useful only when checking the eventual physical capture interface. It is **not** a prerequisite for the normal virtual bench.

SameBoy already exposes SFC/SNES integration hooks for pixel, horizontal-reset and vertical-reset events:

```text
GB_set_icd_pixel_callback()
GB_set_icd_hreset_callback()
GB_set_icd_vreset_callback()
```

If we later need signal-level diagnostics, the preferred approach is to reuse those callbacks or add a very small maintainable SameBoy hook for the DMG path. Reimplementing the Game Boy LCD/PPU state machine in this repository is explicitly out of scope.

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
3. pinned SameBoy static-library/link integration and core tests.

The three paths are currently expected to remain green before emulator changes are considered integrated. SameBoy and SDL2 remain optional for end users, but their integration is checked automatically so optional code does not silently rot.
