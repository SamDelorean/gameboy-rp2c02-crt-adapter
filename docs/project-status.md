# Project Status

## Canonical repository

This repository is the canonical public repository for the project:

`SamDelorean/gameboy-rp2c02-crt-adapter`

An earlier accidentally named repository beginning with a leading hyphen is **not part of this project history** and should be ignored.

## Current phase

**Architecture fixed at the central-interconnect level; firmware bring-up and pre-bench implementation are in progress.**

The controller is no longer under selection. The current hardware baseline is RP2350 / Raspberry Pi Pico 2, and firmware V0.2 already implements the theoretical Game Boy capture path with PIO + DMA. Bench validation is still required before compatibility claims or PCB freeze.

## Compatibility target

The project is planned as a **Game Boy DMG / SGB** video adapter.

DMG is the first reference platform for electrical validation. SGB/SGB-CPU-compatible source hardware is an explicit planned compatibility target and should use the same downstream framebuffer, scaler, border, palette and RP2C02 output path wherever the equivalent Game Boy video signals are accessible.

Optional `P14/P15` SGB-lite palette recovery is an additional feature, not the definition of SGB video compatibility.

See [`source-compatibility.md`](source-compatibility.md).

## Decisions already consolidated

- RP2C02-class NTSC PPU architecture retained.
- Game Boy DMG / SGB source compatibility is an explicit architecture target.
- **RP2350 selected as the V1 controller family; Raspberry Pi Pico 2 is the preferred prototype module.**
- **Arduino IDE + Arduino-Pico selected as the V1 development environment.**
- PIO + DMA retained for timing-critical Game Boy capture and EXT output.
- Direct Game Boy LCD/video capture retained.
- V0.2 theoretical PIO/DMA capture engine implemented; bench timing validation remains pending.
- Aspect-correct presentation fixed at 234x240 image inside 256x240, with 11-dot side borders.
- Ping-pong full-frame buffering retained.
- Common clock reference retained.
- V0.1 central interconnect fixed around Game Boy DMG/SGB -> Pico 2/RP2350 -> RP2C02.
- Minimized write-only PPU host interface retained: R/W fixed LOW, A1/A2 tied, EXT0..EXT3 shared with D0..D3, passive reset.
- Manual global palettes retained.
- Optional passive SGB-lite retained.
- One-button manual override of SGB-derived palettes retained.
- Border generator separated logically from the scaler; black fixed for version 1.
- Clone PPU support treated as test-based compatibility.

## Decisions still open

- final common clock-generator IC and its measured output conditioning,
- final voltage-level/interface components where bench measurements require them,
- exact physical DMG clock cut/injection point for the selected donor revision,
- exact SGB/SGB-CPU source configurations and signal-access points to validate,
- exact prototype PPU revision/device,
- final EXT-output PIO/DMA timing implementation after RP2C02 timing validation,
- final palette preset count/table,
- final mixed-license declaration,
- production schematic and PCB.

The SGB external-clock replacement principle itself is not open; only its exact board-revision implementation and electrical validation remain to be documented.

## Publication rule

Do not label an implementation detail or source configuration as finalized/compatible until it has either:

- been explicitly selected as a project decision, or
- been verified on representative hardware where measurement is required.
