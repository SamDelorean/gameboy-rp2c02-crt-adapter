# Palette System

## Global mapping

Version 1 applies one four-color mapping to the complete Game Boy image:

```text
DMG shade 0 -> PPU color A
DMG shade 1 -> PPU color B
DMG shade 2 -> PPU color C
DMG shade 3 -> PPU color D
```

The framebuffer remains palette-independent. Palette changes only alter the mapping loaded into the RP2C02-compatible PPU.

## One-button user interface

The first version uses a single momentary pushbutton for all palette-mode changes. No menu display or second button is required.

Recommended mode order:

```text
0  AUTO/SGB
1  manual preset 1
2  manual preset 2
3  manual preset 3
...
N  manual preset N
   -> wrap to AUTO/SGB
```

A debounced press advances exactly one mode.

## AUTO/SGB mode

If optional `P14/P15` inputs are connected and a supported direct Super Game Boy palette command is decoded, AUTO/SGB may apply the resulting four-color palette globally.

If no valid SGB palette has been received, AUTO/SGB uses a defined fallback palette, for example the normal DMG-green or another safe default selected during firmware finalization.

The most recently decoded valid SGB palette may be cached even while a manual mode is selected.

## Manual override rule

The user must always be able to override an SGB-derived palette with the same single button.

Therefore:

- if an SGB palette is currently visible, the next button press leaves AUTO/SGB and selects manual preset 1;
- while any manual preset is active, newly received SGB commands must not change the visible palette;
- SGB traffic may still be decoded/cached in the background;
- only after the user cycles back to AUTO/SGB may SGB regain control of the visible palette.

This establishes a simple priority rule:

```text
manual user selection > incoming SGB palette traffic
```

except when the user has deliberately selected AUTO/SGB.

## Manual preset count

Current target: `8` or `16` curated manual presets, plus the AUTO/SGB mode.

The exact number remains open until the useful palette set is finalized.

## Candidate preset categories

- grayscale,
- classic DMG green,
- warm LCD green/yellow,
- amber,
- sepia,
- blue,
- blue-gray,
- selected SGB-inspired palettes,
- selected GBC-inspired palettes.

## Design rules

- do not expose every possible RP2C02 color combination,
- preserve a sensible luminance ordering between DMG shades,
- avoid known problematic PPU color codes where appropriate,
- perform palette updates during VBlank,
- keep the UI to one button in the first version,
- keep SGB control optional,
- never allow SGB traffic to prevent manual palette selection.

## SGB/GBC palette conversion

Where a source palette is represented in RGB555/15-bit form, firmware or offline tooling may map each source color to a suitable PPU color.

A future conversion tool should preferably use a measured/modelled NES palette and perceptual color distance rather than simple Euclidean RGB distance.

The converted palette should preserve the four-shade ordering required by the Game Boy image whenever practical.

## Overscan

Initial behavior: fixed black.

Possible later firmware alternatives:

- automatically use the darkest palette entry,
- use another dedicated EXT index,
- repeat edge pixels/lines.

These alternatives should remain implementation details rather than creating additional user-interface modes unless a strong use case appears.
