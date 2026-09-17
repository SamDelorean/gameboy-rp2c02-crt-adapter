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

### 2. SGB palette compatibility in the virtual bench

SameBoy supplies already-decoded SGB palette state. The project consumes one global four-color RGB555 palette and translates it to RP2C02 codes. Physical P14/P15 acquisition is deferred and is not part of the current emulator contract.

This is optional. Failure to receive SGB palette traffic must never break the common video path or manual palette operation.

## Established SGB clock-modification approach

For project purposes, external clock injection into a Super Game Boy is treated as an **established modification technique**, based on prior hands-on implementation experience within the project.

The design principle is:

1. identify the clock/reference path that the SGB normally receives from the Super Nintendo host;
2. interrupt or isolate that original clock path;
3. inject the desired external Game Boy-domain clock at the SGB-side clock input instead;
4. ensure that the original and injected sources are never actively driving the same node at the same time.

This same class of modification has historically been used to run a Super Game Boy at native Game Boy speed by replacing the host-derived clock reference.

For this project, the method is repurposed so the SGB can receive the synchronized `GB_SYNC_CLK` derived from the common adapter reference.

Therefore **clock injection feasibility is not considered an open architectural question for SGB compatibility**. What remains implementation-specific is only:

- exact cut/isolation point for the selected SGB board revision;
- exact injection pad/pin;
- voltage and input-threshold verification;
- buffering/series damping if required;
- duty cycle and waveform quality at the receiving IC;
- confirmation that no host-derived clock remains connected in contention.

These installation details must be recorded for every validated SGB revision, but they do not change the common clock architecture.

## Planned source matrix

| Source/configuration | Common video path | Clock access | P14/P15 | SGB-lite | Status |
|---|---|---|---|---|---|
| Game Boy DMG | expected reference implementation | external synchronized clock planned | optional | opportunistic if present | planned / first validation target |
| Super Game Boy / SGB-CPU configuration | intended | established external-injection method; board-specific point to document | intended where accessible | intended passive support | planned / not yet electrically validated |

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
11. if applicable, verify the SameBoy-provided global SGB palette translation;
12. known limitations.

## Compatibility terminology

Use these terms carefully:

- **planned compatibility** — architecture reserves and intends support, but hardware has not yet been validated;
- **bench validated** — exact source configuration has passed the documented electrical/video tests;
- **SGB palette translation validated** — SameBoy-provided RGB555 palette state has been translated and previewed through the RP2C02 path;
- **full SGB emulation** — not a version-1 goal and must not be implied by either of the above.

## Design rule

Source-specific differences should be absorbed at the **source-interface/driver layer** whenever possible.

They should not create separate scaling, framebuffer, palette or RP2C02 output implementations unless measurements prove that such divergence is unavoidable.
