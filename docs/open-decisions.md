# Open Engineering Decisions

This file tracks choices that should remain visibly open until they are closed by analysis or bench validation.

## 1. Digital controller

Leading candidate: RP2040 / Pico.

Alternatives retained:

- RP2350 / Pico 2,
- ESP32-class MCU with suitable deterministic I/O,
- practical FPGA solution.

Close only after a concrete resource/timing plan and representative test.

## 2. Common clock generator

Required outputs:

- ~21.4772727 MHz for the PPU,
- ~4.2203555 MHz for the modified DMG.

Selection criteria:

- common reference,
- adequate jitter/edge quality,
- simple control or fixed configuration,
- low cost,
- easy availability,
- compatible electrical output levels.

## 3. Electrical level adaptation

Must be derived from measured/verified voltage domains and thresholds. Do not freeze a buffer/translator solely from convenience.

## 4. PPU reference device

A specific RP2C02 revision or validated clone should be selected for the first bench fixture.

## 5. Clone PPU support

Maintain a test-based compatibility matrix. Candidate families are not automatically compatible.

## 6. Palette table

Open points:

- 8 versus 16 initial presets,
- exact color-code choices,
- measured/modelled PPU palette basis,
- RGB555 conversion method,
- handling of unsafe/problematic PPU color codes.

## 7. SGB-lite behavior

Initial scope is limited to passive direct-palette commands. Active SGB identification emulation is explicitly deferred unless later justified.

## 8. Final license declaration

Current recommendation:

- CERN-OHL-W-2.0 hardware,
- MIT firmware/software,
- CC BY-SA 4.0 documentation.

Do not add final license texts until the project owner explicitly closes this choice.
