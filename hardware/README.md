# Hardware

No production-ready schematic has been released yet.

## Planned schematic blocks

1. Power and decoupling
2. DMG LCD input conditioning
3. Common clock generation
4. Digital controller / programming
5. RP2C02-compatible PPU control bus
6. `EXT0..EXT3` interface
7. Composite-video output
8. Palette button
9. Optional `P14/P15` SGB header
10. Debug/test points

## Hardware priorities

- Favor hand-solderable packages or castellated modules for prototypes.
- Avoid BGA unless a later design has a compelling reason.
- Keep the PPU electrically replaceable where practical.
- Expose useful clocks and sync/debug nodes.
- Treat clone PPU compatibility as a measured property.
- Do not freeze the level-shifting solution until the actual DMG/controller/PPU voltage requirements are checked.

## Preferred donor Game Boy for experimentation

For early destructive or semi-destructive prototyping, prefer a Game Boy DMG whose LCD assembly is already damaged beyond reasonable repair rather than sacrificing a complete working console.

A suitable donor should still have:

- a functional main logic board and CPU,
- reliable cartridge execution,
- intact LCD-interface traces and connector area,
- observable `LD0`, `LD1`, `CP`, `CPL`, `ST` and `S` signals,
- a usable clock domain for the planned external clock modification.

LCD glass damage, severe column/row failure, damaged polarizer, or another display fault that makes restoration impractical is acceptable. Damage to the CPU board, LCD signal generation, or the relevant traces may make the unit unsuitable even if the screen itself is already bad.

The project should prioritize reuse of otherwise non-restorable donor hardware whenever practical.

## Recommended debug points

- GND
- controller supply
- PPU supply
- DMG frame/line timing reference
- PPU `/INT`
- PPU master clock
- modified DMG clock
- `EXT0`
- `EXT1`
- `EXT2`
- `EXT3`
- composite output

See [`interfaces.md`](interfaces.md) for the draft signal inventory.
