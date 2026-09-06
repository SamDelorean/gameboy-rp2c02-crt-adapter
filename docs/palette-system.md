# Palette System

## Global mapping

Version 1 applies one four-color mapping to the complete Game Boy image:

```text
DMG shade 0 -> PPU color A
DMG shade 1 -> PPU color B
DMG shade 2 -> PPU color C
DMG shade 3 -> PPU color D
```

## User interface

A single momentary button cycles through a curated preset list:

```text
preset = (preset + 1) mod N
```

Initial target: `N = 8` or `16`.

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
- keep the UI to one button in the first version.

## SGB/GBC palette conversion

Where a source palette is represented in RGB555/15-bit form, firmware or offline tooling may map each source color to a suitable PPU color.

A future conversion tool should preferably use a measured/modelled NES palette and perceptual color distance rather than simple Euclidean RGB distance.

## Overscan

Initial behavior: fixed black.

Possible later firmware alternatives:

- automatically use the darkest palette entry,
- use another dedicated EXT index,
- repeat edge pixels/lines.

These alternatives should remain implementation details rather than creating additional user-interface modes unless a strong use case appears.
