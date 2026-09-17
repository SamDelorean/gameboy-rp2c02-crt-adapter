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

If no valid SGB palette has been received, AUTO/SGB falls back to preset 1, `DMG LCD` (`$38/$28/$18/$08`).

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

The virtual bench now uses **16 candidate manual presets**, plus the separate `AUTO/SGB` mode. The count is therefore provisionally closed at 16; individual entries may still be replaced or reordered during visual review before hardware freeze.

## Candidate 16-preset catalog

The virtual bench now carries a 16-entry candidate catalog. This is the set to exercise with real DMG software before declaring the hardware table frozen. `AUTO/SGB` remains a separate logical mode and is not counted among these 16 manual presets.

| # | Preset | Shade 0 | Shade 1 | Shade 2 | Shade 3 | Purpose |
|---:|---|---:|---:|---:|---:|---|
| 1 | DMG LCD | `$38` | `$28` | `$18` | `$08` | Closest coherent RP2C02 approximation of SameBoy's classic DMG LCD reference. |
| 2 | POCKET LCD | `$38` | `$10` | `$2D` | `$08` | Lower-saturation Pocket-style approximation; neutral middle levels are intentional. |
| 3 | LIGHT TEAL | `$3B` | `$2C` | `$1C` | `$0C` | Teal family inspired by Game Boy Light appearance. |
| 4 | GRAYSCALE | `$30` | `$10` | `$00` | `$0F` | Neutral four-level reference. |
| 5 | LIME | `$39` | `$29` | `$19` | `$09` | Yellow-green ramp. |
| 6 | GREEN | `$3A` | `$2A` | `$1A` | `$0A` | Saturated green ramp. |
| 7 | MINT | `$3B` | `$2B` | `$1B` | `$0B` | Blue-green/mint ramp. |
| 8 | CYAN | `$3C` | `$2C` | `$1C` | `$0C` | Cyan ramp. |
| 9 | SKY BLUE | `$31` | `$21` | `$11` | `$01` | Light blue ramp. |
| 10 | BLUE | `$32` | `$22` | `$12` | `$02` | Deeper blue ramp. |
| 11 | VIOLET | `$33` | `$23` | `$13` | `$03` | Violet ramp. |
| 12 | LILAC | `$34` | `$24` | `$14` | `$04` | Lilac/purple ramp. |
| 13 | ROSE | `$35` | `$25` | `$15` | `$05` | Rose/magenta ramp. |
| 14 | RED | `$36` | `$26` | `$16` | `$06` | Red ramp. |
| 15 | AMBER | `$37` | `$27` | `$17` | `$07` | Amber/orange ramp. |
| 16 | HIGH CONTRAST | `$30` | `$10` | `$2D` | `$0F` | Neutral high-separation diagnostic/utility ramp. |

All entries obey the current monitor-LUT luminance order `shade 0 > shade 1 > shade 2 > shade 3`, contain four distinct codes, and avoid `$0D`. The catalog deliberately does not encode game identity; every preset is global and must remain usable with arbitrary four-shade DMG imagery.

The first three LCD-inspired entries are approximations, not claims of exact CRT colorimetry. Their source references are SameBoy's DMG, MGB/Pocket and GBL/Light display palettes at the project-pinned SameBoy revision. The remaining entries are native RP2C02 hue ramps chosen for predictable four-level contrast.

Visual review with several games may still replace or reorder entries before the hardware list is frozen. The palette editor is the intended tool for that review.

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

Where SameBoy exposes an SGB palette in RGB555/15-bit form, the virtual bench maps each source color to a suitable PPU color.

The current conversion still uses the provisional monitor LUT and simple RGB distance. A later refinement should wait until the project fixes a measured/modelled RP2C02 palette basis; otherwise a more elaborate metric would only add false precision.

The converted palette should preserve the four-shade ordering required by the Game Boy image whenever practical.

## Overscan

Initial behavior: fixed black.

Possible later firmware alternatives:

- automatically use the darkest palette entry,
- use another dedicated EXT index,
- repeat edge pixels/lines.

These alternatives should remain implementation details rather than creating additional user-interface modes unless a strong use case appears.
