# Open Engineering Decisions

This file tracks choices that should remain visibly open until they are closed by analysis or bench validation.

## 1. Digital controller

Leading candidate: RP2040 / Pico.

Alternatives retained:

- RP2350 / Pico 2,
- ESP32-class MCU with suitable deterministic I/O,
- practical FPGA solution.

Close only after a concrete resource/timing plan and representative test.

The final controller choice must leave enough GPIO/peripheral flexibility for the common Game Boy DMG / SGB capture path plus optional `P14/P15` SGB-lite inputs.

## 2. Common clock generator

Required outputs:

- ~21.4772727 MHz for the PPU,
- ~4.2203555 MHz for the synchronized Game Boy source timing target.

Selection criteria:

- common reference,
- adequate jitter/edge quality,
- simple control or fixed configuration,
- low cost,
- easy availability,
- compatible electrical output levels.

The exact clock injection/isolation method remains to be validated separately for the selected DMG board and any SGB/SGB-CPU-compatible source hardware.

## 3. Electrical level adaptation

Must be derived from measured/verified voltage domains and thresholds. Do not freeze a buffer/translator solely from convenience.

This applies independently to:

- DMG source signals,
- SGB/SGB-CPU source signals,
- optional `P14/P15`,
- RP2C02/clone interfaces.

## 4. SGB source compatibility

SGB is a planned compatibility target, but exact supported hardware/configurations are still open until measured.

Need to close:

- exact SGB/SGB-CPU hardware revision/configuration used for first validation,
- access points for Game Boy video/timing signals or equivalents,
- voltage/loading differences from DMG,
- synchronized clock access/injection method,
- whether any source-specific input adapter is required.

The goal is to keep source-specific differences confined to the source-interface layer and reuse the common 160x144 framebuffer/scaler/output pipeline.

## 5. PPU reference device

A specific RP2C02 revision or validated clone should be selected for the first bench fixture.

## 6. Clone PPU support

Maintain a test-based compatibility matrix. Candidate families are not automatically compatible.

## 7. Palette table

Open points:

- 8 versus 16 initial presets,
- exact color-code choices,
- measured/modelled PPU palette basis,
- RGB555 conversion method,
- handling of unsafe/problematic PPU color codes.

## 8. SGB-lite behavior

Initial scope is limited to passive direct-palette commands on optional `P14/P15`.

Open validation points include:

- which real games/configurations emit usable packets passively,
- exact packet-decoder timing tolerances,
- RGB555-to-RP2C02 quantization details,
- fallback palette behavior.

Active SGB identification emulation remains explicitly deferred unless later justified.

SGB video-path compatibility does not depend on closing every SGB-lite question.

## 9. Border behavior beyond version 1

Version 1 uses fixed black for the two 11-dot side borders.

Possible future simple behavior such as palette-related border color or other low-complexity effects is deliberately deferred. Any such feature must remain inside the border/output-composition block and must not change the aspect-correct scaler or source framebuffer.

## 10. Final license declaration

Current recommendation:

- CERN-OHL-W-2.0 hardware,
- MIT firmware/software,
- CC BY-SA 4.0 documentation.

Do not add final license texts until the project owner explicitly closes this choice.
