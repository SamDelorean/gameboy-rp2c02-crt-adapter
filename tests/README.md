# Validation Plan

A design should not be called validated until the corresponding measurements are recorded.

## 1. Clock tests

Measure and record:

- PPU master clock frequency,
- modified DMG clock frequency,
- jitter/edge quality where practical,
- long-term frame relationship,
- startup behavior.

## 2. Stand-alone PPU test

Before attaching a Game Boy:

- verify reset/init,
- write known palette entries,
- drive known `EXT0..EXT3` values,
- observe `/INT`,
- measure composite waveform,
- display test patterns on a known-good CRT/capture path.

## 3. DMG capture test

Verify:

- correct 160x144 reconstruction,
- stable line/frame boundaries,
- no missing/doubled pixels,
- no dropped lines,
- correct shade bit ordering,
- operation across representative games/test screens.

## 4. Scaling test

Use deterministic source patterns:

- checkerboard,
- one-pixel vertical lines,
- one-pixel horizontal lines,
- borders/corners,
- four-shade ramps.

Confirm exact 256x240 output coverage and no tearing.

## 5. Palette/UI test

- verify every preset,
- verify button debounce and wraparound,
- verify palette writes do not produce visible transient corruption,
- verify overscan remains at the intended value.

## 6. Optional SGB-lite test

- capture raw P14/P15 packets,
- verify packet framing,
- verify direct palette command decode,
- verify RGB555 conversion,
- verify invalid/no-command fallback,
- test with normal DMG and any available SGB-CPU installation separately.

## 7. Clone PPU validation

Use the procedure in `docs/ppu-compatibility.md` and record each exact device independently.

## Evidence storage

Future measured artifacts should be organized under a reproducible structure, for example:

```text
tests/results/<date>-<hardware>/
```

Include instrument setup, firmware revision/commit, board revision, and photographs/captures where relevant.
