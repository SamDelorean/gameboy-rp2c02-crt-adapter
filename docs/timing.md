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

## Common-reference requirement

Preferred architecture:

```text
shared reference
   |-- PPU master clock  ~21.4772727 MHz
   `-- DMG target clock  ~4.2203555 MHz
```

The purpose is not merely nominal frequency accuracy; it is to prevent the two frame domains from slowly drifting with respect to one another.

## Validation requirements

Before freezing the design, measure on real hardware:

- actual PPU master clock,
- actual modified DMG clock,
- frame-to-frame phase behavior,
- LCD active-pixel burst relative to the 456-clock line,
- PPU VBlank `/INT` timing,
- whether any implementation-specific clone-PPU timing difference affects the chosen relationship.
