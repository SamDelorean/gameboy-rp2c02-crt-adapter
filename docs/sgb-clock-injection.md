# Super Game Boy External Clock Injection — Design Note

## Status

**SET at design-principle level.**

For this project, external clock injection into a Super Game Boy is treated as an established and previously implemented modification technique rather than an unresolved architectural risk.

The project owner has prior hands-on experience performing this class of SGB modification to run the Super Game Boy at native Game Boy speed.

This note records the design principle so future schematics and installation drawings preserve it.

## Principle

The Super Game Boy normally receives a Game Boy-domain timing reference derived from the Super Nintendo host clocking arrangement.

The modification strategy is to:

1. identify the host-derived clock path feeding the SGB Game Boy subsystem;
2. interrupt, cut, isolate, or otherwise disable that original clock path;
3. inject an external clock at the SGB-side clock input;
4. prevent the original and replacement sources from ever driving the same node simultaneously.

Conceptually:

```text
SNES-derived clock path
        X  <- interrupt/isolate here
        |
        +--------------------------+
                                   |
common project reference           |
        |                          |
     Si5351A                       |
        |                          |
   GB_SYNC_CLK --------------------+
                                   |
                         SGB Game Boy subsystem
```

## Prior practical implementation

The project owner's previous SGB speed-correction installations used a very simple implementation:

```text
SNES-derived SGB clock trace
        X   <- physical trace cut / isolation
        |
        +-------------------------------> original path disconnected

5 V active oscillator module
        |
        +-------------------------------> injected at the SGB-side clock node
```

The remembered implementation used a **5 V active oscillator** as the replacement source. The host-derived clock trace was physically interrupted and the oscillator output was injected downstream of that interruption into the SGB Game Boy clock domain.

This prior implementation is useful evidence for the present architecture because it demonstrates that the SGB clock domain can be separated from the SNES-derived source and driven externally with very little additional hardware.

The historical oscillator frequency was chosen to make the SGB run at normal/native Game Boy speed. The exact oscillator part number, frequency tolerance, cut point and injection point from those past installations are not currently being asserted from memory and should be documented from the actual board used when reproducing the modification.

## Use in this project

Historically, this type of modification can be used to replace the SGB's host-derived timing with a clock chosen to reproduce native Game Boy speed.

In the Game Boy RP2C02 CRT Adapter, the same mechanism is used for a different system-level goal: feed the SGB from the project's synchronized Game Boy clock so that the SGB source frame cadence remains locked to the RP2C02 output domain.

Working project target:

```text
GB_SYNC_CLK ≈ 4.2203555 MHz
```

The exact final value remains tied to the common-clock architecture and its bench validation.

The new design does **not** require retaining a separate 5 V can oscillator. The historical 5 V oscillator implementation is the proof-of-principle precedent. The project intends to replace that source with `GB_SYNC_CLK` derived from the common programmable clock generator, followed by whatever buffering or level adaptation measurements show to be appropriate for the SGB input.

## What is considered resolved

The following is **not** treated as an open design question:

- whether an SGB clock can be externally replaced in principle;
- whether the host-derived clock path can be interrupted and a replacement clock injected;
- whether this technique is suitable as the basis for integrating SGB into the common synchronized-clock architecture.

## What still must be documented per board revision

For every exact SGB board/revision used in the project, record:

- board identifier/revision;
- original clock source/path;
- exact trace, resistor, jumper, via, pad, or pin used to isolate the original source;
- exact injection point;
- receiving IC/pin where practical;
- nominal and measured voltage levels;
- required buffer or level adaptation;
- whether a 5 V logic-level clock is required or merely tolerated by that revision;
- series damping if used;
- measured frequency and duty cycle;
- rise/fall time and ringing;
- startup behavior;
- confirmation that the original source is not still driving the node;
- successful operation under the synchronized project clock.

These are implementation details, not a reconsideration of the architecture.

## Design rule

The SGB installation should use the same logical `GB_SYNC_CLK` interface as the DMG installation.

Source-specific differences belong in the **clock injection adapter / installation documentation**, not in the framebuffer, scaler, palette, border, or RP2C02 output architecture.

## Evidence classification

This design note is currently classified as **project prior hardware experience / established implementation knowledge**.

Where external schematics, modification guides, or board-level references are later collected, they should be added as supporting references. Their absence does not make the clock-replacement principle an open project question.
