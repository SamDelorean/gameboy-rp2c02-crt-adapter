# Game Boy RP2C02 CRT Adapter

A proof-of-concept project that explores an alternate Game Boy video path using a **real NES-style RP2C02 color/raster model** as the final palette and display stage.

The current public milestone is the **hybrid software virtual bench**. It runs Game Boy software through SameBoy, applies the project's fixed 160×144 → 234×240 bridge, and previews the result through a pinned Ricoh 2C02 implementation from johnmph/NESEmu.

> **Current release scope:** software proof of concept. Physical RP2C02/CRT hardware validation is future work and is not required to reproduce the software result.

## What the PoC proves

```text
SameBoy
  |
  | 160x144 Game Boy image / 4 logical shades
  v
project bridge
  |
  | fixed 160x144 -> 234x240 scaling
  | 11 black dots + 234 image dots + 11 black dots
  | global 4-color palette mapping
  v
RP2C02 preview backend
  |
  | pinned johnmph/NESEmu Ricoh2C02 implementation
  | native 6-bit PPU color codes
  v
SDL2 / PPM comparison output
```

The left side of the viewer is the normal SameBoy reference. The right side is the same Game Boy image after the project bridge and RP2C02 path.

## Implemented in the PoC

- SameBoy as the Game Boy CPU/memory/cartridge/LCD source.
- Fixed horizontal scaling `160 -> 234`.
- Exact vertical scaling `144 -> 240` using the `2,1,2` repetition pattern.
- Fixed `11 + 234 + 11` output geometry inside the PPU's 256-pixel visible width.
- Black side borders independent of the four Game Boy shades.
- Reduced project RP2C02 model for dependency-free regression tests.
- Optional pinned **johnmph/NESEmu Ricoh2C02** backend for the preferred preview.
- `STOCK` and synchronized `SYNC` Game Boy clock models.
- 16 candidate global manual RP2C02 palettes.
- Interactive SDL2 palette editor.
- SGB palette support limited to one already-decoded four-color RGB555 palette supplied by SameBoy.
- Project-authored boot stub and smoke ROM for copyright-clean CI.
- CI coverage for dependency-free, SDL2, donor-PPU, and SameBoy+donor-PPU builds.

## SGB scope

The project does **not** implement P14/P15/JOYP transport in the virtual bench.

SameBoy interprets SGB protocol behavior. The project receives only the resulting simple four-color RGB555 palette and translates those four entries to RP2C02 color codes for `AUTO/SGB`.

Not implemented:

- SGB regional attributes
- graphical borders
- tile transfers
- game identification
- regional/game database colorization
- full SGB emulation

## Build the complete PoC

Requirements on Linux:

- Git
- CMake
- C and C++ toolchains
- Make
- SDL2 development files
- Clang for the pinned SameBoy core build

From the repository root:

```sh
sh ./emulator/scripts/bootstrap_sameboy.sh
sh ./emulator/scripts/bootstrap_nesemu.sh

cmake -S emulator -B build/poc \
  -DCMAKE_BUILD_TYPE=Release \
  -DGBCRT_ENABLE_SDL2=ON \
  -DGBCRT_ENABLE_SAMEBOY=ON \
  -DSAMEBOY_ROOT="$PWD/emulator/third_party/SameBoy" \
  -DGBCRT_ENABLE_NESEMU_PPU=ON \
  -DNESEMU_ROOT="$PWD/emulator/third_party/NESEmu"

cmake --build build/poc --parallel
ctest --test-dir build/poc --output-on-failure
```

Generate the project-authored smoke ROM:

```sh
python3 emulator/tests/generate_smoke_rom.py build/poc-smoke
```

Run the live viewer:

```sh
./build/poc/gbcrt_viewer \
  --source-model dmg \
  --rom build/poc-smoke/gbcrt_smoke.gb \
  --boot build/poc-smoke/gbcrt_boot_stub.bin \
  --clock sync
```

For personal testing you may substitute a legally obtained Game Boy `.gb` image. No commercial ROMs are included in this repository.

See [`emulator/README.md`](emulator/README.md) for detailed build variants, controls, smoke tests, SameBoy integration, and donor-PPU notes.

## Viewer controls

- Arrow keys: Game Boy D-pad
- `Z`: A
- `X`: B
- `Backspace`: Select
- `Enter`: Start
- `P`: next global palette
- `E`: palette editor
- `M`: clock-mode menu
- `C`: toggle STOCK/SYNC
- `Space`: pause
- `Q`: quit

## Physical hardware status

The repository also contains design research for a future physical Game Boy → RP2C02 adapter. That work is **not part of the software PoC acceptance criterion**.

If physical implementation resumes, the current design principles remain:

- use Game Boy LCD logical video as the image source;
- preserve the fixed aspect-correct scaling geometry;
- use a common clock reference to avoid long-term frame drift;
- keep buffering minimal (the current authoritative target is a two-line working buffer rather than a full-frame ping-pong requirement);
- drive a real RP2C02/compatible PPU as the final video/color stage;
- validate EXT behavior, clock injection, voltage levels, and composite output on real hardware before making compatibility claims.

Earlier RP2350/Arduino-Pico material is retained as historical prototype/reference work, not as a requirement of the current virtual-bench architecture.

## Third-party dependencies

The project pins but does not vendor:

- **SameBoy** — Game Boy source core
- **johnmph/NESEmu** — Ricoh 2C02 donor implementation used only for the PPU preview backend

See [`emulator/THIRD_PARTY.md`](emulator/THIRD_PARTY.md).

## Licensing

This repository uses a mixed open-source/open-hardware licensing model:

- software: **MIT**
- hardware: **CERN-OHL-W-2.0**
- documentation: **CC BY-SA 4.0**

See [`LICENSES.md`](LICENSES.md) for the exact scope and full-license locations.

## Project language

English is the canonical technical language for the public repository. A Spanish overview is maintained in [`README_ES.md`](README_ES.md).
