# Validation Plan

A design should not be called validated until the corresponding measurements are recorded.

The project has a cross-cutting **Game Boy DMG / SGB** compatibility target. DMG remains the first reference platform, but validated SGB/SGB-CPU-compatible source configurations should be tested against the same capture, clock, scaling, palette and output requirements rather than being treated only as a palette add-on.

## 1. Clock tests

Measure and record:

- PPU master clock frequency,
- modified Game Boy source clock frequency,
- jitter/edge quality where practical,
- long-term frame relationship,
- startup behavior.

Repeat the relevant checks on each validated source hardware configuration; do not assume an SGB source accepts the same clock injection/loading arrangement as a DMG without measurement.

## 2. Stand-alone PPU test

Before attaching Game Boy hardware:

- verify reset/init,
- write known palette entries,
- drive known `EXT0..EXT3` values,
- observe `/INT`,
- measure composite waveform,
- display test patterns on a known-good CRT/capture path.

## 3. Game Boy source capture test

For DMG first, then for each SGB/SGB-CPU-compatible source configuration available, verify:

- correct 160x144 reconstruction,
- stable line/frame boundaries,
- no missing/doubled pixels,
- no dropped lines,
- correct shade bit ordering,
- operation across representative games/test screens,
- documented signal-access points and voltage levels.

If an SGB source differs electrically or temporally from DMG, record the difference explicitly and keep adaptation outside the common framebuffer/scaler logic where possible.

## 4. Scaling and border test

Use deterministic source patterns:

- checkerboard,
- one-pixel vertical lines,
- one-pixel horizontal lines,
- borders/corners,
- four-shade ramps.

Confirm:

- `160 -> 234` horizontal mapping,
- `144 -> 240` vertical mapping,
- 11-dot left and right border regions,
- no tearing,
- no scaled intermediate framebuffer requirement,
- equivalent output geometry for DMG and validated SGB-compatible sources.

## 5. Palette/UI test

- verify every manual preset,
- verify button debounce and wraparound,
- verify palette writes do not produce visible transient corruption,
- verify the border generator remains at the intended version-1 value,
- verify manual user selection always overrides an SGB-derived palette.

## 6. SGB compatibility and SGB-lite test

Treat this as two related but distinct checks.

### 6.1 Common video-path compatibility

- verify the SGB/SGB-CPU source exposes a usable Game Boy video stream,
- verify capture into the same 160x144x2-bit framebuffer format,
- verify synchronized frame operation,
- verify the same scaler/output path can be used without SGB-specific image processing.

### 6.2 Optional SGB-lite signaling

- capture raw `P14/P15` packets,
- verify packet framing,
- verify direct palette command decode,
- verify RGB555 conversion,
- verify invalid/no-command fallback,
- verify cached SGB palette behavior,
- verify one-button manual override,
- record games/configurations that do and do not emit usable passive SGB palette traffic.

Initial commands:

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

Passing the common SGB video-path test does not imply that passive SGB palette detection works for every game, and failure to receive SGB commands must not break normal video or manual palette operation.

## 7. Clone PPU validation

Use the procedure in `docs/ppu-compatibility.md` and record each exact device independently.

## Evidence storage

Future measured artifacts should be organized under a reproducible structure, for example:

```text
tests/results/<date>-<source-hardware>-<ppu>/
```

Include instrument setup, source hardware/revision, firmware revision/commit, adapter-board revision and photographs/captures where relevant.
