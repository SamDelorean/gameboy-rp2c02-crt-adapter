# Frontend policy for the hybrid emulator

The preferred user-facing emulator is **SameBoy with an added RP2C02 alternate-video mode**.

The separate program under `emulator/` remains a useful engineering/regression bench, but it is not intended to replace SameBoy's mature application frontend.

## SameBoy stays intact

Preserve SameBoy's existing SDL application and ordinary features wherever practical:

- ROM/open-file flow;
- Game Boy execution and rendering;
- audio;
- joypad/controller support;
- pause/reset/turbo/rewind;
- save states and screenshots;
- menu/settings behavior;
- OSD and text rendering;
- persistent configuration.

SameBoy already provides the relevant frontend building blocks:

- `SDL/gui.c` / `run_gui()` for menu and OSD behavior;
- `SDL/font.c` for text rendering;
- `SDL/configuration.*` for persistent frontend configuration;
- platform-specific `SDL/open_dialog/*` helpers for ROM file selection;
- keyboard/controller handling in `SDL/main.c`.

The project should **extend those facilities**, not recreate them in another application.

## What the project adds

Only the behavior specific to this experiment needs to be added to SameBoy:

1. a second RP2C02 video pipeline;
2. an optional side-by-side comparison presentation;
3. one logical `NEXT PALETTE` action;
4. minimal status information useful for comparison.

Conceptual presentation:

```text
+-------------------------------------------------------------+
| SameBoy menu / OSD                 [ NEXT PALETTE ]          |
+----------------------------+--------------------------------+
| GAME BOY REFERENCE         | RP2C02 OUTPUT                  |
|                            |                                |
| existing SameBoy output    | black-box adapted output       |
|                            |                                |
+----------------------------+--------------------------------+
```

The two video wells remain explicitly framed so aspect ratio, scaling, borders and color mapping can be compared directly.

Normal SameBoy display mode should remain available and behave as upstream does.

## One-button palette control

The virtual control mirrors only the *logical effect* of the planned physical one-button UI.

A SameBoy menu item/hotkey and, in comparison mode, an optional small clickable button perform the same action:

```text
AUTO/SGB
  -> manual preset 1
  -> manual preset 2
  -> ...
  -> manual preset N
  -> AUTO/SGB
```

The standalone SDL comparison bench currently uses `P` and a clickable `NEXT PALETTE [P]` control. The SameBoy extension should expose an equivalent action through its existing input/menu conventions.

No debounce, GPIO, RP2350/Arduino state machine, PIO, DMA or other physical implementation detail belongs in the desktop emulator.

## Black-box adapter boundary

The virtual alternate path is deliberately abstract:

```text
SameBoy
  |
  | 160x144, four-shade Game Boy image
  | optional SGB palette already interpreted by SameBoy
  v
project black-box adapter
  |
  | fixed scaler + side borders
  | palette mapping
  v
RP2C02 EXT model
```

The black box represents only the behavior required to study the alternate video output. It must not become an emulator of the planned RP2350/Arduino implementation.

For SGB, reuse SameBoy's existing SGB implementation and consume effective palette state directly. There is no project-owned P14/P15/JOYP transport decoder in the virtual bench; project code only translates the already-decoded four-color RGB555 palette to RP2C02 codes.

## Existing standalone frontend

The current custom SDL viewer is retained because it is useful for:

- CI and regression testing;
- bridge/scaler development;
- RP2C02-model debugging;
- generated smoke-ROM tests;
- fast experiments independent of the full SameBoy application.

Do not continue expanding it into a second general-purpose emulator frontend.

## Integration discipline

Prefer a small patch surface against pinned/upstream SameBoy:

- add project-owned alternate-video modules;
- call them from existing SameBoy rendering boundaries;
- add a small number of UI/menu hooks;
- keep upstream SameBoy code otherwise recognizable and maintainable.

See `SAMEBOY_EXTENSION.md` for the insertion-point design.
