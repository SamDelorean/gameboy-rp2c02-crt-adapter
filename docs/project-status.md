# Project Status

## Canonical repository

This repository is the canonical public repository for the project:

`SamDelorean/gameboy-rp2c02-crt-adapter`

An earlier accidentally named repository beginning with a leading hyphen is **not part of this project history** and should be ignored.

## Current phase

Architecture and component-selection phase.

## Compatibility target

The project is planned as a **Game Boy DMG / SGB** video adapter.

DMG is the first reference platform for electrical validation. SGB/SGB-CPU-compatible source hardware is an explicit planned compatibility target and should use the same downstream framebuffer, scaler, border, palette and RP2C02 output path wherever the equivalent Game Boy video signals are accessible.

Optional `P14/P15` SGB-lite palette recovery is an additional feature, not the definition of SGB video compatibility.

See [`source-compatibility.md`](source-compatibility.md).

## Decisions already consolidated

- RP2C02-class NTSC PPU architecture retained.
- Game Boy DMG / SGB source compatibility is an explicit architecture target.
- Direct Game Boy LCD/video capture retained.
- Aspect-correct presentation fixed at 234x240 image inside 256x240, with 11-dot side borders.
- Ping-pong full-frame buffering retained.
- Common clock reference retained.
- Manual global palettes retained.
- Optional passive SGB-lite retained.
- One-button manual override of SGB-derived palettes retained.
- Border generator separated logically from the scaler; black fixed for version 1.
- Clone PPU support treated as test-based compatibility.

## Decisions still open

- final digital controller,
- final common clock-generator IC,
- final voltage-level/interface components,
- exact DMG clock injection implementation,
- exact SGB/SGB-CPU source configurations and clock/signal-access method to validate,
- exact prototype PPU revision/device,
- final palette preset count/table,
- final mixed-license declaration,
- production schematic and PCB.

## Publication rule

Do not label an implementation detail or source configuration as finalized/compatible until it has either:

- been explicitly selected as a project decision, or
- been verified on representative hardware where measurement is required.
