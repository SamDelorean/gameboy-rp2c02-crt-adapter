# Palette Control and SGB Priority

## Current scope

Palette control in the virtual bench is purely logical. The project does not emulate Arduino/RP2350 state, debounce, GPIO, `P14/P15`, or JOYP transport. SameBoy owns SGB protocol interpretation and provides an already-decoded four-color palette when available.

## Mode model

```text
AUTO/SGB
manual preset 1
manual preset 2
...
manual preset N
-> AUTO/SGB
```

The exact manual preset count remains open; the current target is 8 or 16.

## AUTO/SGB behavior

When `AUTO/SGB` is selected:

- a valid four-color SGB palette supplied by SameBoy is translated from RGB555 to four RP2C02 color codes and becomes the global palette;
- if no valid SGB palette exists, a defined fallback palette is used;
- no regional attributes or graphical SGB border data are consumed.

## Manual override

Manual selection always wins. While a manual preset is active, changes in SameBoy's SGB palette state may be cached but do not alter the visible palette. Cycling back to `AUTO/SGB` re-enables the latest valid translated SGB palette.

```text
manual user selection > automatic SGB palette state
```

The virtual control is a menu/hotkey/button action only; it is not a simulation of physical button electronics.
