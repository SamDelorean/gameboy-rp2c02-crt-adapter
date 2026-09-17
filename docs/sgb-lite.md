# Super Game Boy Palette Handling — Current Scope

## Current rule

The current project does **not** emulate or implement the physical `P14/P15` / JOYP transport path. SameBoy already interprets Super Game Boy protocol traffic in the virtual bench, so duplicating that machinery would add complexity without helping the RP2C02 adapter study.

For SGB operation the project consumes only the simple global palette state already decoded by SameBoy.

```text
SameBoy SGB state
      |
      | four RGB555 colors
      v
project palette translator
      |
      | four RP2C02 color codes
      v
AUTO/SGB global palette
```

## What is preserved

- one global four-color palette only;
- Game Boy shade indices `0..3` stay in the same order;
- RGB555 colors are translated to the nearest suitable RP2C02 color code;
- problematic RP2C02 code `$0D` is avoided by the current translator;
- manual presets always remain available;
- `AUTO/SGB` uses the SameBoy-provided palette when valid and a normal fallback otherwise.

## Deliberate non-goals

The project does not reproduce SGB regional color attributes, graphical borders, tile transfers, controller-ID behavior, or a project-owned SGB packet decoder. It also does not simulate Arduino/RP2350 execution for SGB palette handling.

Physical acquisition of SGB palette information, if revisited later, is outside the current emulator architecture and must not drive or complicate the virtual bench.
