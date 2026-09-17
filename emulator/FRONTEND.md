# Frontend policy for the hybrid emulator

The hybrid emulator is a temporary engineering bench, not a new general-purpose Game Boy emulator frontend.

## Reuse SameBoy where practical

SameBoy already provides a mature SDL frontend with:

- `SDL/gui.c` / `run_gui()` for menu and OSD behavior;
- `SDL/font.c` for text rendering;
- `SDL/configuration.*` for persistent frontend configuration;
- platform-specific `SDL/open_dialog/*` helpers for ROM file selection;
- established keyboard/controller handling in `SDL/main.c`.

Those components are the preferred reference and reuse source when the hybrid bench needs equivalent frontend behavior. Do not independently design a large settings/menu system unless the existing SameBoy frontend cannot be adapted reasonably.

The full SameBoy `SDL/gui.c` is intentionally **not** copied wholesale into this repository. It is tightly coupled to SameBoy features that the hybrid bench does not need, including save states, shaders, debugger functions, recording and many emulator settings. Pulling that complete layer into the project would create more maintenance work than it removes.

Preferred strategy:

1. keep SameBoy itself responsible for Game Boy execution and ordinary input;
2. reuse or closely adapt small SameBoy SDL frontend pieces when they solve an actual requirement;
3. keep project-specific UI limited to the side-by-side comparison surface and a few alternate-output controls.

## Project-specific comparison UI

The hybrid bench adds only the UI needed for this project:

```text
+-------------------------------------------------------------+
| clock/status                   [ NEXT PALETTE ]              |
+----------------------------+--------------------------------+
| GAME BOY REFERENCE         | RP2C02 EXT PATH                |
|                            |                                |
| SameBoy output             | black-box adapted output       |
|                            |                                |
+----------------------------+--------------------------------+
```

The two video wells remain visibly framed so aspect ratio, scaling, borders and color mapping can be compared directly.

## One-button palette control

The virtual control intentionally mirrors the planned physical one-button UI without simulating its electronics or firmware.

Both of these perform the exact same logical operation:

- press `P`;
- click the on-screen `NEXT PALETTE [P]` button.

Each activation advances:

```text
AUTO/SGB
  -> manual preset 1
  -> manual preset 2
  -> ...
  -> manual preset N
  -> AUTO/SGB
```

No debounce, GPIO, RP2350/Arduino state machine, PIO, DMA or other physical implementation detail is represented in the emulator.

## Black-box adapter boundary

The main virtual path is deliberately abstract:

```text
SameBoy
  |
  | 160x144, 2-bit Game Boy image
  | optional already-decoded SGB palette
  v
project black-box adapter
  |
  | fixed scaler + side borders
  | palette mapping
  v
RP2C02 EXT model
```

The black box represents only the behavior needed to study the alternate video output. It must not become an emulator of the planned RP2350/Arduino implementation.

For SGB, SameBoy is allowed to decode its own SGB protocol and provide effective palette state directly. The project then maps those RGB555 colors to RP2C02 palette codes. The standalone `sgb_lite` packet decoder remains only as a future hardware/firmware validation asset for the documented P14/P15 implementation; it is not the primary emulator path.

## Future frontend reuse

When needed, prefer adapting SameBoy's existing frontend facilities for:

- ROM-open dialog;
- pause/menu conventions;
- controller configuration;
- OSD messages;
- persistent settings.

The comparison layout itself remains project-owned because SameBoy does not provide the required dual-video presentation.
