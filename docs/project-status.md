# Project Status

## Canonical repository

This repository is the canonical public repository for the project:

`SamDelorean/gameboy-rp2c02-crt-adapter`

An earlier accidentally named repository beginning with a leading hyphen is **not part of this project history** and should be ignored.

## Current phase

**Current work is centered on the hybrid virtual bench: SameBoy source, project scaler/palette bridge, and RP2C02 preview. Physical controller implementation is deferred from the emulator architecture.**

Earlier RP2350/Pico 2 and Arduino-Pico work is retained as historical prototype material, but it is not a current emulator requirement and must not drive the SGB palette path.

A separate `emulator/` virtual-bench path now exists in the same repository. Emulator V0.2 contains the project scaler/bridge, a reduced RP2C02 EXT/palette model, a 16:9 side-by-side comparison renderer, a pluggable Game Boy source interface, and an optional SameBoy-backed ROM source. The dependency-free build and geometry/source tests have been exercised locally; real linking/running against a locally built SameBoy library remains to be verified on a machine with the dependency available.

## Compatibility target

The project is planned as a **Game Boy DMG / SGB** video adapter.

DMG is the first reference platform for electrical validation. SGB/SGB-CPU-compatible source hardware is an explicit planned compatibility target and should use the same downstream framebuffer, scaler, border, palette and RP2C02 output path wherever the equivalent Game Boy video signals are accessible.

For the current emulator path, SGB contributes only an already-decoded global palette from SameBoy; P14/P15 transport is not emulated.

See [`source-compatibility.md`](source-compatibility.md).

## Decisions already consolidated

- RP2C02-class NTSC PPU architecture retained.
- Game Boy DMG / SGB source compatibility is an explicit architecture target.
- **RP2350 selected as the V1 controller family; Raspberry Pi Pico 2 is the preferred prototype module.**
- Arduino-Pico prototype work is historical/superseded for the current virtual-bench architecture.
- PIO + DMA retained for timing-critical Game Boy capture and EXT output.
- Direct Game Boy LCD/video capture retained.
- V0.2 theoretical PIO/DMA capture engine implemented; bench timing validation remains pending.
- Aspect-correct presentation fixed at 234x240 image inside 256x240, with 11-dot side borders.
- Ping-pong full-frame buffering retained.
- Common clock reference retained.
- V0.1 central interconnect fixed around Game Boy DMG/SGB -> Pico 2/RP2350 -> RP2C02.
- Minimized write-only PPU host interface retained: R/W fixed LOW, A1/A2 tied, EXT0..EXT3 shared with D0..D3, passive reset.
- Manual global palettes retained; a 16-preset candidate catalog is implemented in the virtual bench and awaits visual review before hardware freeze.
- SGB support in the virtual bench is limited to SameBoy-provided global palette state plus RGB555-to-RP2C02 translation.
- One-button manual override of SGB-derived palettes retained.
- Border generator separated logically from the scaler; black fixed for version 1.
- Clone PPU support treated as test-based compatibility.
- Hybrid emulator / virtual bench added under `emulator/`.
- SameBoy selected as the first optional Game Boy execution/source core for side-by-side comparison.
- Complete NES emulation remains out of scope for the virtual bench; the project owns a reduced RP2C02 EXT/palette model instead.

## Decisions still open

- final common clock-generator IC and its measured output conditioning,
- final voltage-level/interface components where bench measurements require them,
- exact physical DMG clock cut/injection point for the selected donor revision,
- exact SGB/SGB-CPU source configurations and signal-access points to validate,
- exact prototype PPU revision/device,
- final EXT-output PIO/DMA timing implementation after RP2C02 timing validation,
- final visual approval/freeze of the implemented 16-preset candidate palette table,
- final mixed-license declaration,
- production schematic and PCB,
- final interactive emulator frontend implementation,
- signal-level SameBoy callback integration for LCD-timing experiments.

The SGB external-clock replacement principle itself is not open; only its exact board-revision implementation and electrical validation remain to be documented.

## Emulator validation status

Current virtual-bench state:

```text
pattern source -> 160x144 four-shade frame
              -> project bridge
              -> 234x240 + 11/11 border
              -> reduced RP2C02 EXT/palette model
              -> 1280x720 comparison image
```

Optional SameBoy path:

```text
SameBoy DMG core
   -> normal framebuffer -----------------> left reference
   -> four-shade recovery -> bridge/PPU --> right comparison
```

Current automated checks cover the 160->234 horizontal mapping, 144->240 vertical mapping, 11/11 borders, and the source abstraction. SameBoy dependency bootstrap and adapter code are present; end-to-end ROM execution with the linked upstream library is the next validation step.

## Publication rule

Do not label an implementation detail or source configuration as finalized/compatible until it has either:

- been explicitly selected as a project decision, or
- been verified on representative hardware where measurement is required.

The same rule applies to emulator claims: a model or integration is a validation aid, not proof of physical hardware behavior unless it is cross-checked against documentation and bench measurements.
