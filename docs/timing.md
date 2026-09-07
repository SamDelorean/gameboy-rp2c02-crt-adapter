# Timing

## Game Boy DMG reference frame model

Working timing model for the first DMG validation target:

- 456 dots/clocks per line,
- 154 total lines per frame,
- 144 visible lines,
- 70,224 source clocks per frame.

Stock DMG master clock:

```text
4.194304 MHz
```

Stock frame rate is approximately 59.7275 Hz.

Validated SGB/SGB-CPU-compatible source configurations are expected to feed the same downstream 160x144 pipeline, but their exact physical signal access and timing relationship must be measured rather than assumed identical to a DMG board.

## RP2C02 NTSC frame model

Working values:

- master clock ~21.4772727 MHz,
- PPU dot clock = master / 4,
- 341 dots per line,
- 262 lines per frame.

With normal rendering disabled, the design can avoid relying on the odd-frame skipped-dot behavior associated with normal rendering.

Total PPU dots in the simplified full frame:

```text
341 x 262 = 89,342 dots
```

Working frame rate: approximately 60.09848 Hz.

## Modified Game Boy target clock

To make one complete Game Boy source frame correspond to one complete simplified PPU frame, use approximately:

```text
f_GB_target ≈ 4.2203555 MHz
```

For a DMG this is about 0.62% above the stock clock.

The working frequency ratio previously derived from the common timing model is:

```text
f_GB / f_PPU_master = 798 / 4061
```

This ratio is useful as an implementation target; final programmed divider values must still be checked against the actual oscillator and clock-generator capabilities.

## Common-reference requirement

The baseline timing architecture uses a **single master frequency reference** for both machines rather than two unrelated oscillators.

Conceptually:

```text
                 common frequency reference
                           |
                    programmable clock
                       generator
                           |
                 +---------+---------+
                 |                   |
                 v                   v
       PPU master clock          GB_SYNC_CLK
        ~21.4772727 MHz        ~4.2203555 MHz
                 |                   |
              RP2C02           Game Boy DMG / SGB
```

The purpose is not merely nominal frequency accuracy. A common reference prevents the Game Boy frame domain and the PPU raster domain from accumulating long-term relative drift.

## Design rationale — synchronize the source instead of compensating later

This clock architecture is deliberately used to simplify the entire adapter.

A stock DMG and an NTSC RP2C02 do not naturally run at exactly the same frame rate. If both were allowed to free-run from independent oscillators, the adapter would have to absorb the difference between two asynchronous video domains.

That would normally require some combination of:

- a deeper asynchronous framebuffer or frame queue,
- frame insertion or frame dropping,
- repeated frames,
- variable-rate readout,
- elastic buffering,
- more complex clock-domain-crossing logic,
- logic to detect and correct accumulated phase drift.

The project instead modifies the Game Boy-domain clock slightly so that the source finishes one complete frame in the same interval used by one complete simplified NTSC PPU frame.

The design objective is therefore:

```text
1 complete Game Boy source frame
        =
1 complete RP2C02 output frame
```

and not merely:

```text
Game Boy frame rate ≈ NTSC frame rate
```

The difference is important. The two devices are intended to be **frequency-locked by construction**, so the bridge does not need to continually reconcile two independent frame cadences.

### Consequence for buffering

The project still uses two small source framebuffers for clean capture/display ownership and to prevent tearing:

```text
2 x (160 x 144 x 2 bits) = 11,520 bytes
```

These buffers are not intended to perform frame-rate conversion.

Because the Game Boy source has been adapted to the output timing, there is no need for a large generalized video framebuffer whose purpose is to absorb long-term timing mismatch between source and display.

Likewise, the design does not require a full 256 x 240 output framebuffer solely to resynchronize the two systems. The fixed scaler can read the 160 x 144 FRONT buffer and emit the 234-dot image plus the two 11-dot border regions directly into the PPU EXT path.

### System-level simplification

This decision trades a small hardware modification to the donor Game Boy/SGB source for substantial simplification elsewhere:

```text
modify one clock domain
      instead of
build a full asynchronous frame-rate converter
```

The expected benefits are:

- fewer parts,
- less RAM,
- simpler firmware,
- simpler timing state machines,
- deterministic latency,
- no periodic frame drops or duplicates caused by free-running drift,
- easier oscilloscope validation,
- easier reproduction by third parties.

This is one of the central design principles of the project.

## Clock proposal 1 — Si5351A-based generator

The first concrete hardware proposal is a **Si5351A programmable clock generator** driven from one crystal/reference.

Proposed use:

```text
Si5351A common reference
   |
   |-- CLK0 -> RP2C02 master clock ≈ 21.4772727 MHz
   |
   `-- CLK1 -> GB_SYNC_CLK         ≈ 4.2203555 MHz
```

The two outputs therefore inherit the same reference and remain frequency-related even if the absolute reference has a small static error.

This remains **proposal 1**, not a frozen clock-component decision. Issue #2 is now dedicated to selecting and validating the final common clock generator; the RP2350/Pico 2 controller decision is already closed.

### Output conditioning

The Si5351A should not be treated as automatically suitable for direct connection to every vintage input. Each output path must be evaluated for:

- logic-high/logic-low compatibility,
- voltage domain,
- edge rate,
- fanout/loading,
- ringing and trace length,
- required series damping,
- whether a dedicated buffer/level translator is needed.

A small logic buffer such as a **74AHCT125-class device** is a current candidate where TTL-compatible buffering or isolation is useful, but the exact part is **not frozen** until measurements confirm the requirements of the selected Game Boy source and PPU/cloned PPU.

### DMG clock injection

The DMG branch requires replacing or overriding the stock clock source with the ~4.2203555 MHz synchronized clock.

The final implementation must document:

1. exact injection point for the selected DMG motherboard revision,
2. whether the original oscillator is disconnected, disabled or otherwise isolated,
3. required amplitude and duty cycle,
4. acceptable source impedance/loading,
5. startup behavior,
6. whether an intermediate buffer is necessary.

The design must not drive an external clock against an active original oscillator.

### SGB clock injection

For SGB hardware, external clock replacement is already treated as an established project design principle: interrupt/isolate the SNES-derived clock route and inject `GB_SYNC_CLK` on the SGB side. The exact cut point, injection point and electrical conditioning remain board-revision-specific validation items. See `sgb-clock-injection.md`.

### PPU clock branch

The PPU branch supplies the normal NTSC-class master clock directly or through the chosen output-conditioning stage.

Prototype validation must verify that the generated ~21.4772727 MHz clock produces:

- stable PPU startup,
- correct raster rate,
- usable color subcarrier/composite behavior,
- stable `/INT` / VBlank timing,
- acceptable behavior on both original and candidate clone PPUs.

## Startup strategy

The V1 baseline uses a passive-high PPU `/RESET`; it does **not** dedicate an RP2350 GPIO to reset the PPU. Startup therefore follows the minimized hardware topology:

1. establish/configure the separate common clock subsystem if programming is required,
2. verify that `PPU_CLK_IN` and `GB_CLK_IN` are stable,
3. keep PPU `/CS` inactive while the RP2350/Pico 2 boots; `/RESET` remains high through its hardware pull-up,
4. allow the RP2C02 to run through its documented post-power/warm-up interval before relying on register writes,
5. initialize the write-only PPU interface and palette through `$2000`, `$2001`, `$2006` and `$2007`,
6. enable/use `/INT` as the VBlank reference while keeping PPUCTRL bit 6 in EXT-input mode,
7. allow the DMG/SGB source to run from `GB_SYNC_CLK`,
8. start/validate PIO + DMA capture after valid source frame/line markers are observed,
9. begin deterministic EXT output once the output engine and raster phase are valid.

A bench pad may still force PPU `/RESET` low during development, but a firmware-controlled reset pulse is not part of the V1 baseline.

Exact power sequencing remains subject to bench validation, especially where 5 V source signals meet RP2350 fault-tolerant GPIO.

## Controller timing implementation

The V1 digital controller is **RP2350 / Raspberry Pi Pico 2**. Timing-critical video work uses PIO + DMA; Arduino-Pico is the development environment but pixel-rate operation does not use ordinary `digitalWrite()` loops or conventional interrupt bit-banging.

Firmware V0.2 already implements the theoretical Game Boy capture side with PIO + DMA. The deterministic `EXT0..EXT3` output engine remains the next major timing block and must be aligned to validated RP2C02 raster timing.

## Status of the proposal

The **shared-reference architecture is a design basis**.

The **Si5351A + buffered output implementation is proposal 1**, not yet a production-frozen circuit. It remains subject to bench validation of jitter, duty cycle, electrical levels, startup behavior and image stability.

## Validation requirements

Before freezing the design, measure on real hardware:

- actual PPU master clock,
- actual modified Game Boy source clock,
- frequency ratio between both outputs,
- frame-to-frame phase behavior,
- long-duration drift behavior,
- clock amplitude and duty cycle at the actual IC pins,
- overshoot/ringing after buffering,
- LCD active-pixel burst relative to the 456-clock line,
- PPU VBlank `/INT` timing,
- RP2350 PIO/DMA capture timing margins,
- RP2350 EXT-output timing margins,
- whether any implementation-specific clone-PPU timing difference affects the chosen relationship.
