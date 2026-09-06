# Palette Control and SGB Priority

This document defines the user-visible palette-control behavior for version 1.

## Design goals

- keep the user interface to one momentary pushbutton;
- keep Super Game Boy support optional;
- allow SGB-aware software to provide a useful global palette when available;
- never allow SGB traffic to trap the user in an automatic palette;
- keep palette selection independent of capture and scaling.

## Optional SGB inputs

`P14` and `P15` are optional listener inputs.

They may be connected on SGB-capable CPU/installations to recover supported palette commands, but the adapter must function normally without them.

No base feature depends on P14/P15:

- DMG LCD capture does not depend on them;
- scaling does not depend on them;
- EXT output does not depend on them;
- manual palettes do not depend on them;
- composite output does not depend on them.

## Mode model

The single button cycles through one automatic mode plus the manual presets:

```text
AUTO/SGB
manual preset 1
manual preset 2
...
manual preset N
-> AUTO/SGB
```

The exact number of manual presets remains open; the current target is 8 or 16.

## AUTO/SGB behavior

When AUTO/SGB is selected:

- a valid supported SGB palette may become the visible global four-color palette;
- if no valid SGB palette exists, a defined fallback palette is used;
- the latest valid SGB palette may be cached for later reuse.

## Manual override behavior

The pushbutton always has priority over automatic SGB palette updates.

If an SGB-derived palette is currently visible:

```text
button press
    -> leave AUTO/SGB
    -> select manual preset 1
    -> schedule palette update at safe VBlank
```

While any manual preset is active:

- P14/P15 traffic may still be decoded;
- a valid SGB palette may be cached;
- incoming SGB commands must not change the visible palette.

SGB control becomes active again only after the user intentionally cycles back to AUTO/SGB.

Priority rule:

```text
manual user selection > incoming SGB palette traffic
```

The only exception is when the user has deliberately selected AUTO/SGB.

## Why this behavior was chosen

SGB-lite is an enhancement, not the primary operating mode. A game-provided palette may be attractive or historically appropriate, but the hardware remains a general-purpose Game Boy-to-CRT adapter.

Therefore the user must always retain immediate control over colorization with the same single switch used on a normal DMG installation.

This also prevents surprising behavior if a game emits a palette that the user does not want or if a decoded SGB palette is visually poor after conversion to the RP2C02 color space.

## Firmware state recommendation

A minimal implementation can use:

```text
palette_mode
cached_sgb_palette
cached_sgb_valid
palette_pending
```

Suggested state meanings:

```text
palette_mode = 0       -> AUTO/SGB
palette_mode = 1..N    -> manual preset 1..N
```

On a valid SGB palette command:

```text
cache converted palette
cached_sgb_valid = true
if palette_mode == AUTO/SGB:
    request palette update
```

On button press:

```text
palette_mode = next mode
request palette update
```

When returning to AUTO/SGB:

- use the cached SGB palette immediately if valid;
- otherwise use the defined fallback palette.

All visible palette writes should still occur during a safe PPU interval such as VBlank.
