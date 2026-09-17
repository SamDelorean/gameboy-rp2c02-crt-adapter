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
RP2C02 preview backend
      |
      | pinned johnmph/NESEmu Ricoh2C02 when enabled
      | reduced project model for dependency-free regression builds
      v
comparison frontend
```

The project does **not** emulate a complete NES either. The RP2C02 side uses only the PPU behavior needed by the proposed hardware adapter.

This division is intentional: if SameBoy or a focused RP2C02 implementation already models a piece of behavior, prefer using it rather than recreating that behavior locally.

The comparison output is designed around a 1280x720 16:9 canvas with two formally framed video regions:

- left: **GAME BOY REFERENCE**, the normal SameBoy output;
- right: **RP2C02 EXT PATH**, the same source image after the project bridge and RP2C02 palette/timing path.

The right panel explicitly identifies the `256x240` PPU region and the `11 + 234 + 11` composition so geometry, centering, duplication patterns and palette differences remain visible rather than hidden by the UI.

## Current V0.3

V0.3 keeps the dependency-free path and adds the first temporal and interactive comparison model. The optional donor-PPU build now replaces the preview's project-owned reduced PPU execution with a pinned real Ricoh2C02 implementation from `johnmph/NESEmu`, while preserving the reduced model as a regression oracle.

Implemented now:

- canonical horizontal `160 -> 234` center-sampled mapping;
- exact vertical `144 -> 240` `2,1,2` repetition pattern;
- `11 + 234 + 11` output composition;
- Game Boy shades use EXT indices `0..3` while EXT index `4` is reserved for the fixed black V1 side borders;
- reduced logical `EXT -> palette RAM` RP2C02 model for dependency-free tests;
- optional pinned `johnmph/NESEmu` Ricoh2C02 backend for the actual preview path;
- donor PPU input through its existing EXT interface and output through its existing pixel callback;
- all 64 six-bit RP2C02 color values are preserved by the monitor-preview LUT rather than discarding the two value/luma bits;
- simplified project `341x262` raster timing remains as an independent regression model;
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
- selectable virtual-bench RP2C02 palette presets;
- automated regression tests for bridge geometry, source abstraction/joypad, RP2C02 color/palette behavior, RP2C02 raster timing and clock scheduling;
- a donor-PPU pixel-exact test covering all `256x240 = 61,440` visible pixels;
- project-authored DMG boot stub and smoke-test ROM for copyright-clean functional CI;
- GitHub Actions coverage for dependency-free, SDL2, donor-PPU and combined SameBoy + donor-PPU builds.

The pinned SameBoy library, our adapter, the donor PPU and the SDL2 viewer are tested in CI. The SameBoy job also executes the project-authored smoke ROM and verifies non-uniform image data independently in the SameBoy reference and RP2C02 image regions, plus black side borders. No commercial Game Boy ROM or Nintendo boot ROM is bundled with the project.

The RGB LUT in `rp2c02_ext.c` is **only a monitor preview**. It distinguishes all six PPU color-code bits, but it does not synthesize or decode the analog composite waveform. The donor PPU produces the native six-bit RP2C02 color code; the LUT only turns that code into desktop RGB. Canonical project black remains `$0F`; color `$0D` should not be used for final presets.

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

This path intentionally uses the reduced project PPU model and requires no external emulator dependency.

To exercise the stock-clock drift scheduler without SameBoy:

```sh
./build/emulator/gbcrt_emu \
  --clock stock \
  --frames 1000 \
  --out build/emulator/comparison-stock.ppm
```

`--frames` counts RP2C02 output frames. In `STOCK` mode, the source does not necessarily advance for every output frame; in `SYNC` mode it advances 1:1.

## NESEmu-backed RP2C02 preview

The preferred PPU preview reuses only the Ricoh2C02 implementation from `johnmph/NESEmu`, pinned to:

```text
4966aa09259ef965d4b6bd2635a1dfe57a8569cb
```

Fetch the pinned donor checkout:

```sh
sh ./emulator/scripts/bootstrap_nesemu.sh
```

Then build the same virtual bench with the donor PPU enabled:

```sh
cmake -S emulator -B build/emulator-nesemu \
  -DGBCRT_ENABLE_NESEMU_PPU=ON \
  -DNESEMU_ROOT="$PWD/emulator/third_party/NESEmu"
cmake --build build/emulator-nesemu
ctest --test-dir build/emulator-nesemu --output-on-failure
./build/emulator-nesemu/gbcrt_emu \
  --clock sync \
  --out build/emulator-nesemu/comparison.ppm
```

This does **not** run NESEmu's CPU, APU, cartridge system, mapper code or frontend. The local C++ wrapper instantiates only `Ppu::Chip<Ricoh2C02>`, sends the project EXT nibble stream through `exts()`, loads palette/register state through the PPU write path, and receives the resulting six-bit color codes through `plotPixel()`.

The dedicated `rp2c02_nesemu_ext_path` regression verifies every visible pixel against known EXT indices and palette values, catching horizontal or vertical phase mistakes immediately.

## SameBoy-backed ROM build

SameBoy is deliberately kept outside this repository. The helper script pins the first integration to commit:

```text
213a12ce93d66b105a113debd9396306066a7cfc
```

For the preferred combined preview, bootstrap both external cores:

```sh
sh ./emulator/scripts/bootstrap_sameboy.sh
sh ./emulator/scripts/bootstrap_nesemu.sh
```

Then configure SameBoy plus the donor RP2C02:

```sh
cmake -S emulator -B build/emulator-sameboy \
  -DGBCRT_ENABLE_SAMEBOY=ON \
  -DSAMEBOY_ROOT="$PWD/emulator/third_party/SameBoy" \
  -DGBCRT_ENABLE_NESEMU_PPU=ON \
  -DNESEMU_ROOT="$PWD/emulator/third_party/NESEmu"

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

CI generates `gbcrt_boot_stub.bin` and `gbcrt_smoke.gb` from `emulator/tests/generate_smoke_rom.py`. These files are entirely project-authored and are used only to prove that actual Game Boy code executes inside SameBoy and reaches the donor RP2C02 preview path.

## Alternate-output palettes

Game Boy shades remain logical values `0..3`; changing the RP2C02 palette does not change SameBoy, source capture, scaling or framebuffer contents.

The virtual bench currently provides a small preview set:

```text
DMG GREEN
GRAYSCALE
AMBER
COOL BLUE
LILAC
```

These are **virtual-bench presets**, not a freeze of the final hardware preset count or exact code values. The hardware design still targets a small curated set plus optional `AUTO/SGB` behavior.

Within the bridge:

```text
EXT 0 -> Game Boy shade 0 (lightest)
EXT 1 -> Game Boy shade 1
EXT 2 -> Game Boy shade 2
EXT 3 -> Game Boy shade 3 (darkest)
EXT 4 -> V1 side-border color (fixed $0F black)
```

This separation is important: the fixed black side border no longer steals the palette entry required by Game Boy shade 0.

In the SDL viewer, `P` requests the next preview palette. The change is committed at the comparison-frame boundary, mirroring the hardware rule that PPU palette writes occur in a VBlank-safe interval.

## Optional live SDL2 viewer

The live viewer is not required for tests or the PPM frontend. On a system with SDL2 development files installed:

```sh
cmake -S emulator -B build/emulator-viewer \
  -DGBCRT_ENABLE_SDL2=ON
cmake --build build/emulator-viewer
./build/emulator-viewer/gbcrt_viewer --clock sync
```

For the live SameBoy + donor-PPU comparison, bootstrap both dependencies and enable all three optional integrations:

```sh
cmake -S emulator -B build/emulator-live \
  -DGBCRT_ENABLE_SDL2=ON \
  -DGBCRT_ENABLE_SAMEBOY=ON \
  -DSAMEBOY_ROOT="$PWD/emulator/third_party/SameBoy" \
  -DGBCRT_ENABLE_NESEMU_PPU=ON \
  -DNESEMU_ROOT="$PWD/emulator/third_party/NESEmu"
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
- `P`: cycle the alternate RP2C02 palette preset;
- `Space`: pause/resume;
- `Esc`: close the menu, or quit when the menu is closed;
- `Q`: quit.

The window title reports source, clock mode, active alternate-output palette, Game Boy frame number, RP2C02 output-frame count and repeated-output count.

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
                      donor Ricoh2C02 PPU
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

The preferred preview uses the pinned NESEmu Ricoh2C02 core only for the behavior the hardware project depends on:

- 341x262 NTSC PPU raster timing;
- VBlank/frame boundaries;
- palette RAM behavior relevant to EXT input;
- EXT0..EXT3 external index path;
- native six-bit RP2C02 color values.

The project-owned reduced PPU remains for dependency-free regression tests and independent comparison. CPU, APU, mappers, CHR game rendering, nametables, OAM/sprite game rendering and the donor frontend are out of scope.

Analog composite waveform generation, NTSC decoder simulation, CRT shaders and modern presentation filters are also out of scope. The desktop preview intentionally stops at a reasonable RGB visualization of the PPU's six-bit color code.

Pinky/Visual2C02-derived tests and NESdev documentation remain useful independent references. Emulator-to-emulator agreement is not treated as a substitute for later hardware validation.

## Continuous integration

`.github/workflows/emulator-ci.yml` validates four paths:

1. dependency-free build, regression tests, and `STOCK`/`SYNC` execution;
2. SDL2 viewer compilation and regression tests;
3. pinned NESEmu donor-PPU build, full-pixel EXT-path regression, and preview rendering;
4. pinned SameBoy + pinned NESEmu integration, project-authored ROM execution, and comparison-image validation.

These paths are expected to remain green before emulator changes are considered integrated. External cores and SDL2 remain optional for end users, but their integration is checked automatically so optional code does not silently rot.
