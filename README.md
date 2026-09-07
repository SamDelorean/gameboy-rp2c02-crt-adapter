# Game Boy RP2C02 CRT Adapter

An open hardware and firmware project to connect a **Nintendo Game Boy** to a television/CRT by using the video PPU from the original **Nintendo Entertainment System / NES** (**RP2C02**) as the final NTSC video generator.

In simple terms, the project takes the Game Boy's LCD pixel data, adapts and scales it digitally, and feeds it to the NES video chip so that the Game Boy image can be displayed as standard composite NTSC video on a television.

Two additional goals are central to the design:

- provide **changeable color palettes**, including user-selected presets and optional palette information recovered from compatible Super Game Boy signaling;
- scale the original 160x144 Game Boy image to a **full-screen 256x240 NTSC picture** using a deliberately simple fixed repetition algorithm, avoiding a complex general-purpose scaler or high-cost video-processing hardware.

The same architecture therefore uses the NES PPU not only to generate the television signal, but also as the final color/palette stage for the four original Game Boy shades.

> **Project status:** architecture and component-selection phase. No production-ready schematic or validated firmware release exists yet.

## Concept

```text
Game Boy DMG / SGB-CPU
        |
        | LD0, LD1, CP, CPL, ST, S
        v
+------------------------------+
| low-cost digital controller  |
| capture + ping-pong buffers  |
| simple fixed full-screen     |
| scaling 160x144 -> 256x240   |
| palette + optional SGB-lite  |
+------------------------------+
        |
        | EXT0..EXT3
        v
 NES / Nintendo video PPU
       (RP2C02)
        |
        | composite NTSC
        v
   television / CRT
```

A common frequency reference is planned for both the PPU and a modified Game Boy clock so that the two frame domains remain locked rather than free-running.

## Current architecture decisions

- NTSC **RP2C02 or functionally compatible discrete clone PPU** as the final video stage.
- Direct capture of the DMG LCD interface: `LD0`, `LD1`, `CP`, `CPL`, `ST`, and `S`.
- Optional `P14/P15` taps for passive Super Game Boy palette-command listening.
- Two complete 160x144x2-bit framebuffers (11,520 bytes total) for robust ping-pong operation.
- Full-screen fixed nearest-neighbor scaling from 160x144 to 256x240.
- Horizontal ratio: `8/5`, implemented by periodic source-pixel repetition.
- Vertical ratio: `5/3`, implemented by periodic source-line repetition.
- Scaling is intentionally simple and deterministic; no general-purpose video scaler is required.
- No required intermediate 256x240 framebuffer in the baseline design.
- RP2C02 normal tile/sprite rendering disabled for the first implementation.
- External palette indices driven through `EXT0..EXT3`.
- One button cycles curated global four-color palettes.
- Optional SGB-derived palettes may be received automatically, but the user can always override them with the same button.
- Initial overscan/border behavior: fixed black.
- No game database, no cartridge identification, and no regional colorization in version 1.
- Early experiments should preferentially use DMG donor units with LCDs that are no longer reasonably repairable, while preserving restorable consoles.

The consolidated decision record is maintained in [`docs/design-decisions.md`](docs/design-decisions.md). The controller-independent software operating principles and exact scaling patterns are documented in [`firmware/architecture.md`](firmware/architecture.md).

## Working clock targets

- RP2C02 master clock: approximately **21.4772727 MHz**.
- Modified DMG clock: approximately **4.2203555 MHz**.

Both should be derived from one reference. The final clock-generator IC is still under selection.

## Controller selection

The controller is **not frozen yet**.

The current leading candidate is **RP2040 / Raspberry Pi Pico** because it provides ample SRAM, PIO, DMA, low cost, and easy module-level prototyping. RP2350, ESP32-class MCUs, and suitable FPGAs remain alternatives until deterministic capture/output timing is demonstrated.

See [`docs/controller-selection.md`](docs/controller-selection.md).

## Palette system

The four DMG shades are mapped globally to four PPU colors. The active palette is intentionally changeable rather than fixed.

The first firmware is expected to provide:

- a small curated set of useful manual palettes selected with one button;
- optional automatic SGB-derived palette selection when compatible P14/P15 traffic is available;
- immediate manual override of an SGB-derived palette by pressing the same button.

Candidate categories include:

- neutral grayscale,
- classic DMG green,
- warm green/yellow,
- amber,
- sepia,
- blue / blue-gray,
- selected SGB/GBC-inspired mappings.

Palette writes should occur during VBlank.

## Optional SGB-lite mode

If `P14/P15` are connected, firmware may passively decode the direct Super Game Boy palette commands:

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

Received RGB555 colors can be quantized to suitable RP2C02 colors and used as a global four-color palette.

This remains optional: if no valid SGB command is observed, normal manual-palette operation continues.

The initial implementation deliberately does **not** emulate SGB controller-ID behavior, spatial attributes, tile transfers, or graphical borders.

## PPU compatibility philosophy

The project is **not tied to original Ricoh-branded RP2C02 chips**. Discrete NTSC clone PPUs salvaged from Famiclones or obtained as old stock may be usable.

Candidate families such as the **UMC UA6528** should be validated rather than assumed compatible. For this project, a clone must specifically demonstrate:

- usable `EXT0..EXT3` external-input operation,
- palette RAM behavior compatible with the design,
- suitable reset/register interface,
- `/INT` / VBlank operation,
- compatible NTSC raster timing,
- stable composite output.

A chip merely being able to run NES software does not prove compatibility with this unusual EXT-input use case. A formal compatibility matrix will be added as devices are tested.

## Explicit non-goals for version 1

- NES CPU emulation.
- NES background/sprite graphics.
- Game identification.
- SGB regional attribute colorization.
- SGB graphical borders.
- Full SGB emulation.
- Internal CRT deflection modification in this project branch.

## Repository structure

```text
docs/        theory, architecture, timing, palettes, references
hardware/    interfaces, schematic planning, PCB sources later
firmware/    firmware architecture and source later
tests/       bench validation and compatibility procedures
```

See [`ROADMAP.md`](ROADMAP.md) for the development sequence.

## Safety and vintage-hardware notes

This project interfaces with vintage ICs and modified Game Boy hardware. Always verify voltage domains, loading, clock amplitudes, and pin direction before connection.

On stock NES hardware, the PPU EXT pins are normally grounded; they must not remain hard-grounded when externally driven.

## Licensing

This repository contains hardware, firmware, and documentation, so a mixed-license model is being prepared rather than applying one generic license to everything.

Current proposal:

- hardware: **CERN-OHL-W-2.0**,
- firmware/software: **MIT**,
- documentation: **CC BY-SA 4.0**.

The licensing structure will be finalized before the first reproducible hardware release.

## Project language

The canonical technical README is in English for broader collaboration. Spanish documentation may also be maintained where useful.
