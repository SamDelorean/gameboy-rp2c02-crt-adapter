# SameBoy RP2C02 extension architecture

## Purpose

The preferred desktop emulator is **SameBoy with an added alternate RP2C02 video output**, not a replacement frontend built around the SameBoy core.

The existing standalone code under `emulator/` remains a regression and experimentation bench for the bridge and reduced RP2C02 model. User-facing development should preserve SameBoy and add the alternate output with the smallest maintainable patch surface.

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

### Proposed flow

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
      gbcrt_adapter
      - 160x144 -> 234x240
      - 11/11 borders
      - palette mapping
            |
            v
      reduced RP2C02 output
            |
            v
      second comparison surface
```

The first implementation may recover the four Game Boy shade indices from SameBoy's known DMG output or use an existing SameBoy internal raw-shade buffer where available. SGB mode should reuse SameBoy's already-decoded SGB palette state rather than reimplementing SGB transport.

## Project-owned module boundary

Keep the added logic in a small module that can be compiled both by the standalone regression bench and by the SameBoy extension.

Suggested interface:

```c
typedef struct gbcrt_alt_video gbcrt_alt_video_t;

void gbcrt_alt_video_init(gbcrt_alt_video_t *state);
void gbcrt_alt_video_set_palette_mode(gbcrt_alt_video_t *state, unsigned mode);
void gbcrt_alt_video_next_palette(gbcrt_alt_video_t *state);

void gbcrt_alt_video_render(
    gbcrt_alt_video_t *state,
    const uint8_t shade[144][160],
    const uint16_t sgb_rgb555[4],
    bool sgb_palette_valid,
    uint32_t out_rgb[240][256]);
```

This is deliberately a logical adapter. It does not model RP2350/Arduino execution.

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

The extension models only the PPU behavior needed by this project:

- 341x262 timing model;
- 256x240 visible region;
- palette RAM subset and mirroring;
- rendering-disabled EXT behavior;
- VBlank boundary;
- 64-code RP2C02 display palette for desktop visualization.

It does not add a 6502, APU, NES cartridge mapper, CHR rendering or sprite engine.

## Development discipline

The standalone `emulator/` regression tests remain authoritative for bridge geometry and the reduced PPU model. SameBoy integration should call that same project-owned logic rather than fork a second copy inside SameBoy.

The desired patch to SameBoy is therefore mostly **presentation and plumbing**, not emulator reimplementation.
