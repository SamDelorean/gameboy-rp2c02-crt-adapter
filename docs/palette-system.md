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

In the virtual bench, if SameBoy exposes a valid SGB four-color palette, AUTO/SGB may apply the translated palette globally.

If no valid SGB palette has been received, AUTO/SGB uses a defined fallback palette, for example the normal DMG-green or another safe default selected during firmware finalization.

The most recently supplied valid SGB palette may be cached even while a manual mode is selected.

## Manual override rule

The user must always be able to override an SGB-derived palette with the same single button.

Therefore:

- if an SGB palette is currently visible, the next button press leaves AUTO/SGB and selects manual preset 1;
- while any manual preset is active, updated SGB palette state must not change the visible palette;
- SameBoy may continue updating the cached SGB palette in the background;
- only after the user cycles back to AUTO/SGB may SGB regain control of the visible palette.

This establishes a simple priority rule:

```text
manual user selection > automatic SGB palette state
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

## Virtual-bench palette editor

The SDL engineering viewer may expose a palette editor for building and validating the curated preset catalog. This editor is a development tool only; it does not change the V1 hardware one-button user interface.

The editor operates directly in RP2C02 color-code space (`$00-$3F`). It presents the four Game Boy shades explicitly as:

```text
SHADE 0  -> lightest
SHADE 1
SHADE 2
SHADE 3  -> darkest
```

The display should make the `LIGHTEST -> DARKEST` direction visually obvious so a user does not need to remember Game Boy shade polarity while constructing a palette. Editing changes only the four palette entries; capture, scaling and the framebuffer remain unchanged.

The full 64-code RP2C02 table may be shown in the editor because this is an engineering tool, even though the final hardware catalog exposes only curated presets. The right-hand RP2C02 preview remains visible while editing so contrast and shade separation can be judged on live Game Boy output.

Useful editor-only operations are `REVERSE`, `RESET`, `PREV`, `NEXT` and `DONE`. A custom edit remains session-local until it is deliberately promoted into the source preset catalog.

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
