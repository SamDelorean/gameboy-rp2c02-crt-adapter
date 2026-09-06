# Timing

## DMG frame model

Working timing model:

- 456 dots/clocks per line,
- 154 total lines per frame,
- 144 visible lines,
- 70,224 source clocks per frame.

Stock DMG master clock:

```text
4.194304 MHz
```

Stock frame rate is approximately 59.7275 Hz.

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

To make one complete DMG frame correspond to one complete simplified PPU frame, use approximately:

```text
f_DMG_target ≈ 4.2203555 MHz
```

This is about 0.62% above the stock DMG clock.

The working frequency ratio previously derived from the common timing model is:

```text
f_DMG / f_PPU_master = 798 / 4061
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
       PPU master clock       modified DMG clock
        ~21.4772727 MHz        ~4.2203555 MHz
                 |                   |
              RP2C02             DMG CPU/SoC
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

The project instead modifies the Game Boy clock slightly so that the DMG finishes one complete source frame in the same interval used by one complete simplified NTSC PPU frame.

The design objective is therefore:

```text
1 complete DMG frame
        =
1 complete RP2C02 output frame
```

and not merely:

```text
DMG frame rate ≈ NTSC frame rate
```

The difference is important. The two devices are intended to be **frequency-locked by construction**, so the bridge does not need to continually reconcile two independent frame cadences.

### Consequence for buffering

The project still uses two small source framebuffers for clean capture/display ownership and to prevent tearing:

```text
2 x (160 x 144 x 2 bits) = 11,520 bytes
```

These buffers are not intended to perform frame-rate conversion.

Because the Game Boy has been adapted to the output timing, there is no need for a large generalized video framebuffer whose purpose is to absorb long-term timing mismatch between source and display.

Likewise, the design does not require a full 256 x 240 output framebuffer solely to resynchronize the two systems. The fixed scaler can read the 160 x 144 FRONT buffer and emit the repeated pixel/line pattern directly into the PPU EXT path.

### System-level simplification

This decision trades a small hardware modification to the donor Game Boy for substantial simplification elsewhere:

```text
modify one clock
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
   `-- CLK1 -> modified DMG clock   ≈ 4.2203555 MHz
```

The two outputs therefore inherit the same reference and remain frequency-related even if the absolute reference has a small static error.

This is currently the preferred prototype architecture because it keeps the clock solution compact, inexpensive and programmable while providing independent outputs from one source.

### Output conditioning

The Si5351A should not be treated as automatically suitable for direct connection to every vintage input. Each output path must be evaluated for:

- logic-high/logic-low compatibility,
- voltage domain,
- edge rate,
- fanout/loading,
- ringing and trace length,
- required series damping,
- whether a dedicated buffer/level translator is needed.

A small logic buffer such as a **74AHCT125-class device** is a current candidate where TTL-compatible buffering or isolation is useful, but the exact part is **not frozen** until measurements confirm the requirements of the selected DMG board and PPU/cloned PPU.

### DMG clock injection

The Game Boy branch requires replacing or overriding the stock clock source with the ~4.2203555 MHz synchronized clock.

The final implementation must document:

1. exact injection point for the selected DMG motherboard revision,
2. whether the original oscillator is disconnected, disabled or otherwise isolated,
3. required amplitude and duty cycle,
4. acceptable source impedance/loading,
5. startup/reset behavior,
6. whether an intermediate buffer is necessary.

The design should not simply drive an external clock against an active original oscillator.

### PPU clock branch

The PPU branch supplies the normal NTSC-class master clock directly or through the chosen output-conditioning stage.

Prototype validation must verify that the generated ~21.4772727 MHz clock produces:

- stable PPU startup,
- correct raster rate,
- usable color subcarrier/composite behavior,
- stable `/INT` / VBlank timing,
- acceptable behavior on both original and candidate clone PPUs.

## Startup strategy

The first implementation should bring the system up in a controlled order:

1. configure the programmable clock source,
2. establish stable PPU and DMG clock outputs,
3. keep the PPU and controller-side interfaces in safe/reset states,
4. release/reset the PPU only after its master clock is valid,
5. allow the DMG to run from the synchronized clock,
6. begin frame capture and output only after valid synchronization markers are observed.

Exact sequencing remains subject to bench validation.

## Status of the proposal

The **shared-reference architecture is a design basis**.

The **Si5351A + buffered output implementation is proposal 1**, not yet a production-frozen circuit. It remains subject to bench validation of jitter, duty cycle, electrical levels, startup behavior and image stability.

## Validation requirements

Before freezing the design, measure on real hardware:

- actual PPU master clock,
- actual modified DMG clock,
- frequency ratio between both outputs,
- frame-to-frame phase behavior,
- long-duration drift behavior,
- clock amplitude and duty cycle at the actual IC pins,
- overshoot/ringing after buffering,
- LCD active-pixel burst relative to the 456-clock line,
- PPU VBlank `/INT` timing,
- whether any implementation-specific clone-PPU timing difference affects the chosen relationship.
