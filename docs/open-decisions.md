# Open Engineering Decisions

This file tracks choices that should remain visibly open until they are closed by analysis or bench validation.

## Closed reference — digital controller

The V1 controller choice is **closed**: **RP2350**, with **Raspberry Pi Pico 2** as the preferred prototype/module implementation.

The earlier RP2040/Pico candidate is retained only as historical comparison in `controller-selection.md`; it is not an open project choice.

Current controller-dependent baseline:

- Arduino IDE + Arduino-Pico development environment;
- RP2350 PIO + DMA for timing-critical capture/output;
- 21 GPIO for the DMG baseline and 23 GPIO with optional `P14/P15`;
- direct minimized write-only PPU interface with no shift-register/latch baseline.

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

Open points:

- 8 versus 16 initial presets,
- exact color-code choices,
- measured/modelled PPU palette basis,
- RGB555 conversion method,
- handling of unsafe/problematic PPU color codes.

## 7. SGB-lite behavior

Initial scope is limited to passive direct-palette commands on optional `P14/P15`.

Open validation points include:

- which real games/configurations emit usable packets passively,
- exact packet-decoder timing tolerances,
- RGB555-to-RP2C02 quantization details,
- fallback palette behavior.

Active SGB identification emulation remains explicitly deferred unless later justified.

SGB video-path compatibility does not depend on closing every SGB-lite question.

## 8. Border behavior beyond version 1

Version 1 uses fixed black for the two 11-dot side borders.

Possible future simple behavior such as palette-related border color or other low-complexity effects is deliberately deferred. Any such feature must remain inside the border/output-composition block and must not change the aspect-correct scaler or source framebuffer.

## 9. Final license declaration

Current recommendation:

- CERN-OHL-W-2.0 hardware,
- MIT firmware/software,
- CC BY-SA 4.0 documentation.

Do not add final license texts until the project owner explicitly closes this choice.
