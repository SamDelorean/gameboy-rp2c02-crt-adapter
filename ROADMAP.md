# Roadmap

This roadmap separates the **completed software proof of concept** from any future physical-hardware program.

## Milestone A — Hybrid emulator proof of concept — COMPLETE

The software PoC is complete when the following are reproducible from the public repository:

- SameBoy executes the Game Boy workload.
- The project recovers the final four logical Game Boy shades.
- The fixed bridge produces a 234×240 image with 11-dot black side borders.
- A pinned Ricoh 2C02 implementation from johnmph/NESEmu consumes the EXT/palette path.
- The normal Game Boy reference and RP2C02 path can be viewed side by side.
- STOCK and SYNC clock models are available.
- Manual palettes and the palette editor work.
- SameBoy-provided simple SGB palette state can drive AUTO/SGB.
- Copyright-clean smoke tests exercise both DMG and SGB paths.
- CI validates core, SDL2, NESEmu, and SameBoy+NESEmu configurations.

This milestone is the target of the `v0.1.0-poc` release.

## Milestone B — PoC maintenance — OPTIONAL

Only changes that materially improve reproducibility or fix defects are required after the PoC release.

Possible maintenance work:

- portability fixes for additional Linux distributions;
- clearer dependency/bootstrap diagnostics;
- viewer usability fixes;
- test coverage for newly discovered regressions;
- improved RP2C02 desktop color reference if a better measured/modelled basis is adopted.

Fine palette tuning is not a release blocker.

## Milestone C — Physical RP2C02 EXT validation — FUTURE

If the hardware project resumes, first prove the PPU concept independently of Game Boy capture:

- reset and minimally initialize a real RP2C02 or candidate compatible PPU;
- write known palette values;
- drive static and changing EXT0..EXT3 patterns;
- verify visible composite output;
- verify VBlank and register behavior;
- document clone compatibility with measurements.

Emulator agreement is useful evidence but does not substitute for this bench test.

## Milestone D — Common-clock validation — FUTURE

Validate a common reference that can generate approximately:

- RP2C02 master: 21.4772727 MHz
- synchronized Game Boy source: 4.2203555 MHz

The current Si5351A calculation remains a candidate implementation, not a hardware-validated release requirement.

## Milestone E — Game Boy signal capture — FUTURE

Validate the physical Game Boy LCD interface:

- LD0
- LD1
- CP
- CPL
- ST
- S

Measure sampling phase, polarity, voltage/loading, line/frame boundaries, and behavior across representative rendering cases.

The physical implementation should preserve the already-proven logical image contract while remaining independent of the desktop-emulator implementation.

## Milestone F — Minimal hardware buffering/scaling — FUTURE

The authoritative physical buffering target is a **minimal two-line working buffer/register**, sufficient to support the fixed scaling operation.

Do not treat earlier full-frame ping-pong framebuffer experiments as a current architectural requirement.

Preserve the fixed PoC geometry:

- 160×144 source
- 234×240 image
- 11-dot black side borders
- no general-purpose scaler

## Milestone G — Physical SGB palette input — OPTIONAL/FUTURE

The software PoC does not implement P14/P15 transport. It uses SGB palette state already decoded by SameBoy.

If physical SGB palette recovery is ever revisited, it should remain an optional layer that can provide one simple global four-color palette without changing the main video path. Regional attributes, graphical borders, and full SGB emulation remain out of scope.

## Milestone H — Reproducible hardware release — FUTURE

Only after real-hardware validation should a hardware release include:

- validated schematics;
- KiCad sources;
- BOM;
- clock and power details;
- measured PPU/EXT behavior;
- installation/signal-access documentation;
- compatibility matrix;
- manufacturing outputs where appropriate.

The software PoC can remain complete independently of these future milestones.
