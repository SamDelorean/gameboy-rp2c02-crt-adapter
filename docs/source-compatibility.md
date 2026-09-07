# Game Boy DMG / SGB Source Compatibility

## Purpose

This document tracks compatibility of the **source side** of the adapter.

The project is not intended to be DMG-only. The target is **Game Boy DMG / SGB** compatibility, using the same downstream video architecture wherever an equivalent Game Boy video stream can be accessed.

## Compatibility layers

### 1. Common video-path compatibility

A source is compatible with the core adapter when it can provide, directly or through documented adaptation:

- 160x144 Game Boy pixel data,
- four 2-bit shade values,
- reliable line/frame timing,
- a usable synchronized clock interface,
- electrical levels that can be safely adapted to the controller.

Once captured, all supported sources use the same:

```text
160x144x2-bit framebuffer
        |
aspect-correct scaler
        |
234x240 image
        |
border generator
        |
palette mapping
        |
RP2C02 EXT output
```

### 2. Optional SGB-lite compatibility

An SGB-capable source may additionally expose `P14/P15` so firmware can passively recover selected direct Super Game Boy palette commands.

This is optional. Failure to receive SGB palette traffic must never break the common video path or manual palette operation.

## Planned source matrix

| Source/configuration | Common video path | Clock access | P14/P15 | SGB-lite | Status |
|---|---|---|---|---|---|
| Game Boy DMG | expected reference implementation | external synchronized clock planned | optional | opportunistic if present | planned / first validation target |
| Super Game Boy / SGB-CPU configuration | intended | must be documented per hardware | intended where accessible | intended passive support | planned / not yet electrically validated |

Add exact board revisions/configurations as real hardware is tested.

## Required evidence for declaring a source supported

For each exact source hardware/configuration, record:

1. hardware identity/revision;
2. signal-access points;
3. `LD0`, `LD1`, `CP`, `CPL`, `ST`, `S` or documented equivalents;
4. signal voltage levels and input/output direction;
5. active-pixel timing;
6. line/frame timing;
7. synchronized clock injection/access method;
8. loading/buffering/level-shifting used;
9. successful reconstruction of a 160x144x2-bit frame;
10. successful aspect-correct output through the common scaler;
11. if applicable, `P14/P15` capture and SGB-lite behavior;
12. known limitations.

## Compatibility terminology

Use these terms carefully:

- **planned compatibility** — architecture reserves and intends support, but hardware has not yet been validated;
- **bench validated** — exact source configuration has passed the documented electrical/video tests;
- **SGB-lite validated** — supported passive palette commands have also been observed and decoded;
- **full SGB emulation** — not a version-1 goal and must not be implied by either of the above.

## Design rule

Source-specific differences should be absorbed at the **source-interface/driver layer** whenever possible.

They should not create separate scaling, framebuffer, palette or RP2C02 output implementations unless measurements prove that such divergence is unavoidable.
