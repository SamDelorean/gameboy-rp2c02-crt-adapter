# SameBoy RP2C02 extension architecture

## Purpose

The preferred desktop emulator is **SameBoy with an added alternate RP2C02 video output**, not a replacement frontend built around the SameBoy core.

The existing standalone code under `emulator/` remains a regression and experimentation bench for the bridge. User-facing development should preserve SameBoy and add the alternate output with the smallest maintainable patch surface.

For the RP2C02 preview itself, the preferred high-fidelity software path reuses the pinned `johnmph/NESEmu` Ricoh2C02 implementation rather than extending the project-owned reduced PPU model into a second NES emulator.

## Preserve SameBoy

Do not remove or replace ordinary SameBoy facilities merely because the RP2C02 path does not need them. Keep, unless an actual conflict is found:

- existing ROM/open-file flow;
- cartridge support;
- audio;
- joypad and controller support;
- pause/reset/turbo/rewind;
- save states;
- screenshots;
- OSD;
- existing menu/settings structure;
- normal Game Boy rendering.

The alternate path is an additional visualization capability.

## Minimal insertion points

The pinned SameBoy SDL frontend already has the required boundaries:

- `SDL/main.c` owns `active_pixel_buffer` and `previous_pixel_buffer`;
- SameBoy points the core at `active_pixel_buffer` through `GB_set_pixels_output()`;
- `rerender_screen()` calls `render_texture(active_pixel_buffer, ...)`;
- `SDL/gui.c` owns `render_texture()`, OSD/text and `run_gui()`;
- `SDL/main.c` already owns keyboard/controller/menu event handling.

Therefore the first extension should avoid changes to the Game Boy execution loop itself.

### Preferred flow

```text
SameBoy core
    |
    +-----------------------------> existing active_pixel_buffer
    |                                      |
    |                                      +-> existing SameBoy display
    |
    +-> four-shade logical image
            |
            v
      project bridge
      - 160x144 -> 234x240
      - 11/11 borders
      - EXT indices 0..4
            |
            v
      pinned NESEmu Ricoh2C02
      - EXT input
      - palette RAM/register path
      - 341x262 timing
      - 256x240 visible pixels
            |
            v
      existing comparison surface
```

The dependency-free reduced `rp2c02_ext` and `rp2c02_timing` implementation remains available for unit tests, regression comparison and builds that intentionally avoid the external PPU donor.

The first implementation may recover the four Game Boy shade indices from SameBoy's known DMG output or use an existing SameBoy internal raw-shade buffer where available. SGB mode should reuse SameBoy's already-decoded SGB palette state rather than reimplementing SGB transport.

## Project-owned module boundary

Keep the added logic small and explicit:

```text
SameBoy source -> project bridge -> RP2C02 donor wrapper -> existing renderer
```

The donor wrapper is `rp2c02_nesemu.cpp`. It exposes a C ABI so NESEmu's C++ templates do not leak into the rest of the C codebase.

The wrapper deliberately supplies only:

- the project's EXT nibble stream;
- the small write-side PPU register state;
- palette RAM contents;
- a pixel callback sink for native six-bit RP2C02 color codes.

It does not model RP2350/Arduino execution.

## SameBoy UI integration

The existing SameBoy interface remains primary.

Project-specific additions should be small:

1. **Video presentation mode**
   - normal SameBoy view;
   - optional side-by-side comparison view.

2. **Next palette**
   - one menu item and one hotkey;
   - optional small clickable button in comparison mode;
   - action cycles `AUTO/SGB -> manual 1 -> ... -> AUTO/SGB`.

3. **Minimal status**
   - current palette mode;
   - optional clock mode label for the engineering comparison.

Do not create a second general settings UI when SameBoy already has a suitable menu or configuration mechanism.

## Window composition

Comparison mode should use the existing SameBoy SDL window rather than launching a second emulator application.

Conceptual layout:

```text
+---------------------------------------------------------------+
| SameBoy menu / OSD                         [NEXT PALETTE]      |
+------------------------------+--------------------------------+
| GAME BOY REFERENCE           | RP2C02 OUTPUT                  |
|                              |                                |
| existing SameBoy image       | 256x240 adapter result         |
|                              |                                |
+------------------------------+--------------------------------+
```

The two image wells should have explicit frames so geometry and centering can be compared visually.

## RP2C02 scope

The preview needs only the PPU behavior used by this project:

- Ricoh 2C02 raster timing;
- 256x240 visible region;
- internal palette RAM and mirroring;
- rendering-disabled EXT input behavior;
- VBlank/frame progression;
- native six-bit RP2C02 color codes.

The pinned NESEmu core provides those behaviors. The project then converts the six-bit color code with its simple monitor-preview LUT. Analog composite/VOUT synthesis, CRT filters and decoder simulation are intentionally out of scope for this preview.

The integration does not add or run a 6502, APU, NES cartridge mapper, game CHR path or NESEmu frontend.

## Development discipline

The standalone `emulator/` regression tests remain authoritative for bridge geometry. The reduced project PPU model remains an independent regression oracle, while the optional NESEmu-backed build supplies the primary higher-fidelity RP2C02 preview.

The dedicated `rp2c02_nesemu_ext_path` test verifies all 61,440 visible pixels of a frame against known EXT indices and palette entries. CI also enables the donor backend in the pinned SameBoy smoke-ROM path.

The desired patch to SameBoy therefore remains mostly **presentation and plumbing**, not emulator reimplementation.
