# Contributing

This project is currently in the architecture/prototype phase. Contributions are welcome, especially when they improve reproducibility or replace assumptions with measurements.

## Useful contribution areas

- RP2C02 and clone-PPU EXT-mode measurements,
- DMG LCD timing captures,
- RP2040/RP2350/ESP32 deterministic-I/O experiments,
- clock-generator calculations,
- schematic review,
- composite-output measurements,
- PPU palette quantization,
- SGB packet decoding,
- PCB layout,
- documentation corrections.

## Evidence labels

When contributing technical information, clearly distinguish between:

- **confirmed on hardware**,
- **confirmed from primary/reference documentation**,
- **simulation only**,
- **hypothesis / proposed implementation**.

Do not present an untested clone PPU as compatible merely because it is marketed as an NES/Famicom replacement.

## Design philosophy

Version 1 should remain deliberately simple:

- no NES CPU emulation,
- no unnecessary tile/sprite subsystem,
- no game database,
- no regional colorization,
- no full SGB emulation,
- no pixel-rate interrupt bit-banging.

Features that can be added almost entirely in firmware or with a few optional signal taps are preferred over architectural expansion.
