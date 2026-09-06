# PPU Compatibility Matrix

The project is designed around the behavior of an NTSC **RP2C02-class PPU**, but it should not require an original Ricoh-branded part if a compatible discrete clone can satisfy the actual electrical and functional requirements.

## Compatibility is test-based

For this project, "NES compatible" is not enough.

A candidate PPU must be tested specifically for:

1. `EXT0..EXT3` usable as external inputs in the required operating mode.
2. Palette RAM writable and addressable as expected.
3. Register/reset interface compatible with the controller design.
4. `/INT` / VBlank behavior usable for synchronization.
5. NTSC raster timing compatible with the clocking model.
6. Stable composite-video output.
7. Acceptable palette/color behavior.

## Status vocabulary

| Status | Meaning |
|---|---|
| **Reference** | Primary device used to define expected behavior. |
| **Validated** | Tested in this project and confirmed suitable. |
| **Partially validated** | Some required functions confirmed; test set incomplete. |
| **Candidate** | Historically/documentarily plausible but not yet tested here. |
| **Unsupported** | Known incompatibility with one or more required functions. |
| **Unknown** | Insufficient information. |

## Initial matrix

| Device | Region | Type | Project status | Notes |
|---|---|---|---|---|
| Ricoh RP2C02 family | NTSC | Original discrete NES PPU | Reference / validation pending | Target behavior for the project. Exact revision used in bench testing must be recorded. |
| UMC UA6528 | NTSC | Discrete clone PPU | Candidate | Historically documented as an RP2C02-class clone. EXT behavior must be explicitly tested. |
| WDL6528 | varies by specific part | Clone family | Candidate | Do not assume pin/function identity from the family name alone. |
| GS87008 | clone family | Clone PPU | Candidate | Requires pinout, region, timing, EXT, palette, and composite verification. |
| Integrated NOAC/COB devices | varies | Integrated system-on-chip | Not a direct substitute | Even if internally PPU-derived, required EXT/register signals may not be externally accessible. |

## Bench-test procedure

Every candidate should be evaluated using the same fixture and firmware:

- known PPU clock,
- controlled reset,
- minimal register initialization,
- four known palette entries,
- static EXT test patterns,
- VBlank `/INT` observation,
- composite waveform observation,
- captured CRT/video image,
- color comparison against reference PPU where practical.

Record:

- exact top marking,
- package,
- donor console/board if known,
- measured clock,
- supply voltage,
- pinout differences,
- observed image/color differences,
- pass/fail for each required behavior.

No clone should be moved to **Validated** based solely on a forum report, seller listing, or the fact that it runs ordinary Famicom software.
