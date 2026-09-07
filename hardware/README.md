# Hardware

A first **connection-level schematic basis** now exists for the central V1 signal path:

- [`schematic-v0.1.md`](schematic-v0.1.md) — Game Boy DMG / SGB → Raspberry Pi Pico 2 → RP2C02 interconnect schematic;
- [`netlist-v0.1.csv`](netlist-v0.1.csv) — exact V0.1 connectivity table suitable for later KiCad capture/checking;
- [`interfaces.md`](interfaces.md) — broader signal/electrical contract and validation notes.

This is **not yet a production-ready PCB schematic**. It deliberately freezes the central interconnection while keeping clock generation, power implementation and downstream analog/video output as separate subsystems.

## Compatibility target

The hardware is planned for **Game Boy DMG / SGB** source compatibility.

The common video path should be reusable anywhere the required Game Boy LCD data/timing signals and synchronized clock access are available. SGB-specific `P14/P15` connections are additional optional inputs for SGB-lite palette recovery, not requirements for basic video operation.

Do not assume every SGB implementation is electrically identical to a DMG. Any SGB/SGB-CPU installation used by the project must have its exact signal access, voltage levels, loading and clock interface documented and validated.

## V0.1 schematic partition

The first schematic sheet is intentionally limited to:

```text
Game Boy DMG / SGB source taps
          |
          v
Raspberry Pi Pico 2 / RP2350
          |
          v
RP2C02-compatible NTSC PPU
```

The following are represented only by off-sheet flags/interfaces:

- `PPU_CLK_IN` → PPU clock input, target ~21.4772727 MHz NTSC;
- `GB_CLK_IN` → synchronized Game Boy/SGB clock-injection point, current target ~4.2203555 MHz;
- `PPU_VIDEO_RAW` ← RP2C02 pin 21 `VOUT`;
- PPU 5 V supply and Pico 2 power input.

This separation is deliberate. The clock generator will be developed as its own circuit. The raw PPU video node will feed a later, independent output stage.

## Video-output scope

The V0.1 sheet ends at `PPU_VIDEO_RAW`.

The RP2C02 raw composite output is not treated as a finished 75-ohm television output. A minimal stock-style composite amplifier may be documented later as a separate sheet.

Other existing NES video modifications — for example S-Video-oriented, HDMI-oriented or other downstream approaches — may be reusable in principle, but they are outside the baseline Game Boy capture/scaler/EXT design and must be validated independently. The central project should not become dependent on any one downstream video-output modification.

## Planned schematic sheets / blocks

1. **Central interconnect V0.1** — DMG/SGB taps, Pico 2, RP2C02, passives, button, raw video handoff. **Defined.**
2. **Clock generation** — common reference and separate `PPU_CLK_IN` / `GB_CLK_IN` outputs. **Separate / pending.**
3. **Power** — final 5 V / Pico VSYS sourcing, filtering and sequencing. **Pending.**
4. **Video output** — raw PPU VOUT buffering / 75-ohm composite stage or other optional downstream implementation. **Pending.**
5. Optional prototype/debug/test-point sheet or carrier-board details as required.

## Hardware priorities

- Favor hand-solderable packages or castellated modules for prototypes.
- Use Raspberry Pi Pico 2 / RP2350 as the V1 controller basis.
- Minimize additional logic ICs; use direct GPIO sharing and fixed/passive PPU states where validated.
- Avoid BGA unless a later design has a compelling reason.
- Keep the PPU electrically replaceable where practical.
- Expose useful clocks and sync/debug nodes.
- Reserve optional `P14/P15` without making them mandatory for normal operation.
- Treat DMG and SGB compatibility as measured properties of the source interface.
- Treat clone PPU compatibility as a measured property.
- Add buffers/level shifting only where actual measurements show they are required.

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
- `PPU_CLK_IN`
- `GB_CLK_IN`
- `EXT0`
- `EXT1`
- `EXT2`
- `EXT3`
- optional `P14`
- optional `P15`
- `PPU_VIDEO_RAW`
