# Contributing

The current public milestone is the **hybrid software proof of concept**. Contributions that improve reproducibility, correctness, portability, or documentation are welcome.

## Software PoC contributions

Useful areas include:

- SameBoy integration fixes;
- RP2C02/NESEmu backend fixes;
- bridge/scaling regression tests;
- SDL2 viewer usability;
- build/bootstrap portability;
- clock-model validation;
- SGB global-palette translation;
- documentation corrections.

The virtual bench intentionally does **not** emulate Arduino/RP2350, PIO, DMA, GPIO, debounce, or P14/P15 electrical transport.

## Physical-hardware research

Hardware contributions are welcome as a separate future track, especially when they replace assumptions with measurements:

- RP2C02/clone EXT-mode captures;
- Game Boy LCD timing captures;
- clock-generator measurements;
- voltage/loading measurements;
- composite-output captures;
- schematic/PCB review.

Do not present emulator agreement or an untested replacement PPU as proof of physical compatibility.

## Evidence labels

When contributing technical claims, distinguish among:

- **confirmed on hardware**;
- **confirmed from primary/reference documentation**;
- **validated by emulator/software test only**;
- **hypothesis / proposed implementation**.

## Scope discipline

Keep the PoC small:

- no second Game Boy emulator;
- no full NES CPU/APU/cartridge subsystem;
- no game database;
- no regional SGB colorization;
- no graphical SGB borders;
- no full SGB emulation;
- no analog NTSC simulation unless it directly serves a future measured-hardware validation task.

## Licensing

By contributing project-authored material, you agree that it may be distributed under the repository's applicable license for that content category:

- software: MIT;
- hardware: CERN-OHL-W-2.0;
- documentation: CC BY-SA 4.0.

See [`LICENSES.md`](LICENSES.md).
