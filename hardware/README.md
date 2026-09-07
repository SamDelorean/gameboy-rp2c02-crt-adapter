# Hardware

No production-ready schematic has been released yet.

## Compatibility target

The hardware is planned for **Game Boy DMG / SGB** source compatibility.

The common video path should be reusable anywhere the required Game Boy LCD data/timing signals and synchronized clock access are available. SGB-specific `P14/P15` connections are additional optional inputs for SGB-lite palette recovery, not requirements for basic video operation.

Do not assume every SGB implementation is electrically identical to a DMG. Any SGB/SGB-CPU installation used by the project must have its exact signal access, voltage levels, loading and clock interface documented and validated.

## Planned schematic blocks

1. Power and decoupling
2. Game Boy DMG / SGB video-source input conditioning
3. Common clock generation
4. Digital controller / programming
5. RP2C02-compatible PPU control bus
6. `EXT0..EXT3` interface
7. Composite-video output
8. Palette button
9. Optional `P14/P15` SGB header/test points
10. Border/output composition block in firmware interface definition
11. Debug/test points

## Hardware priorities

- Favor hand-solderable packages or castellated modules for prototypes.
- Avoid BGA unless a later design has a compelling reason.
- Keep the PPU electrically replaceable where practical.
- Expose useful clocks and sync/debug nodes.
- Reserve optional `P14/P15` without making them mandatory for normal operation.
- Treat DMG and SGB compatibility as measured properties of the source interface.
- Treat clone PPU compatibility as a measured property.
- Do not freeze the level-shifting solution until the actual source/controller/PPU voltage requirements are checked.

## Preferred donor Game Boy for experimentation

For early destructive or semi-destructive DMG prototyping, prefer a Game Boy DMG whose LCD assembly is already damaged beyond reasonable repair rather than sacrificing a complete working console.

A suitable donor should still have:

- a functional main logic board and CPU,
- reliable cartridge execution,
- intact LCD-interface traces and connector area,
- observable `LD0`, `LD1`, `CP`, `CPL`, `ST` and `S` signals,
- a usable clock domain for the planned external clock modification.

LCD glass damage, severe column/row failure, damaged polarizer, or another display fault that makes restoration impractical is acceptable. Damage to the CPU board, LCD signal generation, or the relevant traces may make the unit unsuitable even if the screen itself is already bad.

The project should prioritize reuse of otherwise non-restorable donor hardware whenever practical.

## SGB hardware planning

For SGB-capable source hardware, the project should document:

- where the equivalent Game Boy LCD/video signals are accessible,
- whether their electrical characteristics match the DMG path,
- how the synchronized source clock is injected or derived,
- where `P14/P15` can be observed passively,
- whether any extra isolation or connector adaptation is required.

The baseline board should, where practical, expose enough test pads/header options that SGB validation does not require redesigning the entire adapter.

## Recommended debug points

- GND
- controller supply
- PPU supply
- Game Boy frame/line timing reference
- PPU `/INT`
- PPU master clock
- modified Game Boy source clock
- `EXT0`
- `EXT1`
- `EXT2`
- `EXT3`
- optional `P14`
- optional `P15`
- composite output

See [`interfaces.md`](interfaces.md) for the draft signal inventory.
