# Open Engineering Decisions

This file tracks choices that should remain visibly open until they are closed by analysis or bench validation.

## Closed reference — digital controller

The V1 controller choice is **closed**: **RP2350**, with **Raspberry Pi Pico 2** as the preferred prototype/module implementation.

The earlier RP2040/Pico candidate is retained only as historical comparison in `controller-selection.md`; it is not an open project choice.

Historical prototype work exists for RP2350/Pico 2, but the current virtual-bench architecture is independent of Arduino/RP2350 implementation details. Physical realization is deferred and may be revisited without changing the emulator contract.

See [`controller-selection.md`](controller-selection.md), [`design-decisions.md`](design-decisions.md), and [`../firmware/README.md`](../firmware/README.md).

## 1. Common clock generator

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

The common-reference architecture is SET. The exact generator IC remains OPEN; Si5351A remains proposal 1 pending validation.

The physical clock injection/isolation point must still be documented and validated for the selected DMG board and each SGB/SGB-CPU-compatible source revision. For SGB, external clock replacement itself is already treated as an established design principle; see [`sgb-clock-injection.md`](sgb-clock-injection.md).

## 2. Electrical level adaptation

Must be derived from measured/verified voltage domains and thresholds. Do not freeze a buffer/translator solely from convenience.

This applies independently to:

- DMG source signals,
- SGB/SGB-CPU source signals,
- optional `P14/P15`,
- RP2C02/clone interfaces.

The V1 baseline intentionally avoids blanket level shifting because RP2350 fault-tolerant digital GPIO can receive the Game Boy-side 5 V signals on the appropriate pins. Bench work must still validate power sequencing, thresholds, and the 3.3 V -> RP2C02 direction.

## 3. SGB source compatibility

SGB is a planned compatibility target, but exact supported hardware/configurations are still open until measured.

Need to close:

- exact SGB/SGB-CPU hardware revision/configuration used for first validation,
- access points for Game Boy video/timing signals or equivalents,
- voltage/loading differences from DMG,
- exact physical clock cut/injection point for each validated board revision,
- whether any source-specific input adapter is required.

The goal is to keep source-specific differences confined to the source-interface layer and reuse the common 160x144 framebuffer/scaler/output pipeline.

## 4. PPU reference device

A specific RP2C02 revision or validated clone should be selected for the first bench fixture.

## 5. Clone PPU support

Maintain a test-based compatibility matrix. Candidate families are not automatically compatible.

## 6. Palette table

The virtual bench now implements a **16-preset candidate catalog** with exact RP2C02 codes and excludes `$0D`. Preset count is provisionally closed at 16.

Still open before hardware freeze:

- visual review/replacement/reordering of individual presets across representative DMG software;
- measured/modelled RP2C02 palette basis for color-translation work;
- final RGB555-to-RP2C02 quantization metric.

## 7. SGB palette handling

For the current virtual bench this is closed at a deliberately small scope: SameBoy interprets SGB protocol state and project code accepts only the resulting global four-color RGB555 palette. The project translates those four colors to RP2C02 codes.

Still open:

- final/measured RP2C02 palette basis for RGB555 quantization;
- final quantization metric.

The `AUTO/SGB` fallback is now preset 1, `DMG LCD` (`$38/$28/$18/$08`).

Physical `P14/P15` transport and a project-owned SGB packet decoder are outside the current emulator scope.

## 8. Border behavior beyond version 1

Version 1 uses fixed black for the two 11-dot side borders.

Possible future simple behavior such as palette-related border color or other low-complexity effects is deliberately deferred. Any such feature must remain inside the border/output-composition block and must not change the aspect-correct scaler or source framebuffer.

## 9. Final license declaration

Current recommendation:

- CERN-OHL-W-2.0 hardware,
- MIT firmware/software,
- CC BY-SA 4.0 documentation.

Do not add final license texts until the project owner explicitly closes this choice.
