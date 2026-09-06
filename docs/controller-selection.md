# Controller Selection

The controller is intentionally not frozen yet.

## Hard requirements

The selected device must provide:

- at least 11.25 KiB of practical framebuffer RAM,
- deterministic capture of the DMG LCD stream,
- deterministic output of PPU EXT pixel indices,
- sufficient GPIO for the DMG interface, PPU control bus, optional SGB taps, clock-control interface, button, and debug,
- low component cost,
- practical hand assembly or module-level prototyping,
- accessible development tools,
- a design that does not depend on pixel-rate interrupt bit-banging.

## RP2040 / Raspberry Pi Pico

Current leading candidate.

Advantages:

- 264 KiB SRAM,
- PIO state machines,
- DMA,
- inexpensive modules,
- easy castellated-module prototyping,
- mature toolchain,
- enough memory to choose robust full-frame buffering instead of clever minimum-buffer schemes.

The final selection should nevertheless be based on an explicit PIO/DMA resource plan and timing test rather than on headline specifications alone.

## RP2350 / Pico 2

Provides considerably more processing margin than required. It is attractive if price, availability, and module convenience are comparable, but the additional capability is not presently necessary.

## ESP32-class devices

Potentially viable, especially variants with suitable DMA/peripheral routing.

Evaluation must focus on deterministic input/output behavior. CPU clock rate alone is not evidence of suitability.

## FPGA

Advantages:

- excellent deterministic timing,
- straightforward parallel pipelines,
- natural fit for capture/scaling/output logic.

Currently deprioritized because the project favors low cost and hand-buildability, and many attractive FPGA parts with sufficient embedded RAM are less convenient to assemble.

## Decision rule

Choose the **simplest low-cost part that demonstrates the complete deterministic data path on hardware**.
