# Technical References

This file tracks the main external technical references used by the project. Claims in released documentation should preferentially cite primary documentation or reproducible measurements.

## RP2C02 / NES PPU

- NESdev — PPU rendering  
  https://www.nesdev.org/wiki/PPU_rendering

- NESdev — PPU registers  
  https://www.nesdev.org/wiki/PPU_registers

- NESdev — PPU pinout  
  https://www.nesdev.org/wiki/PPU_pinout

- NESdev — PPU frame timing  
  https://www.nesdev.org/wiki/PPU_frame_timing

- NESdev — PPU palettes  
  https://www.nesdev.org/wiki/PPU_palettes

- NESdev — PPU variants  
  https://www.nesdev.org/wiki/PPU_variants

- NESdev — Errata  
  https://www.nesdev.org/wiki/Errata

## Direct project inspiration — ANES dual-PPU work

- decrazyo — **Advanced Nintendo Entertainment System (ANES)**  
  https://github.com/decrazyo/anes

- ANES project video — dual-PUU / external-video demonstration  
  https://www.youtube.com/watch?v=V2kaV_m4iNU

ANES is an important **conceptual inspiration** for this project. It modifies an NES to use two `RP2C02` PPUs and revisits the otherwise-unused `EXT0..EXT3` path as a way for one picture generator to contribute pixel/palette-index information to another PPU.

The Game Boy RP2C02 CRT Adapter takes that same underlying PPU capability in a different direction: instead of using a second NES PPU as the external picture source, the project reconstructs the Game Boy DMG / SGB 2-bit image in a small digital controller and drives the RP2C02 `EXT0..EXT3` inputs directly.

This reference is recorded as the historical trigger for the architecture, not as the sole technical authority for EXT behavior. Electrical and register-level claims remain cross-checked against NESdev documentation and bench measurements.

Related dual-PPU references from the ANES ecosystem:

- decrazyo — dual-PPU demo  
  https://github.com/decrazyo/dual-ppu-demo

- VinglesSmi — DualPPUTest  
  https://github.com/VinglesSmi/DualPPUTest

## Game Boy / Super Game Boy

- Pan Docs — Rendering  
  https://gbdev.io/pandocs/Rendering.html

- Pan Docs — Specifications  
  https://gbdev.io/pandocs/Specifications.html

- Pan Docs — SGB Command Packet  
  https://gbdev.io/pandocs/SGB_Command_Packet.html

- Pan Docs — SGB Command Summary  
  https://gbdev.io/pandocs/SGB_Command_Summary.html

- Pan Docs — SGB Command Palettes  
  https://gbdev.io/pandocs/SGB_Command_Palettes.html

- Pan Docs — SGB Unlocking  
  https://gbdev.io/pandocs/SGB_Unlocking.html

## Direct DMG LCD capture precedent

- Thomas Spurden — Capturing the Gameboy LCD with an FPGA  
  https://thomas.spurden.name/blog/capturing-gb-lcd/

This is a practical precedent for reconstructing the DMG display from `LD0`, `LD1`, `CP`, `CPL`, `ST`, and `S`.

## Reference policy

- Forum discussions may be useful engineering leads but should not be the sole authority when stronger documentation exists.
- Seller claims are not compatibility evidence.
- Clock values, EXT behavior, clone compatibility, and the final electrical interface should be reverified against real hardware before a schematic is marked validated.
