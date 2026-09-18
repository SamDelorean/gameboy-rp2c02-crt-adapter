# Changelog

## v0.1.2-poc — 2026-09-17

### Fixed

- CRT/PPU preview now uses a physically representative NTSC display proportion instead of drawing 256x240 active PPU samples as square pixels.
- Game Boy and CRT comparison images now have exactly the same displayed height (432 pixels), so their physical proportion difference is visible only in width.
- Game Boy reference remains 480x432; CRT preview is 576x432 (4:3).
- The CRT preview follows the NESdev 280x240 NTSC picture model: 12 side-border samples + 256 active PPU samples + 12 side-border samples.

### Unchanged

- Internal adapter geometry remains 160x144 -> 234x240 inside the 256x240 PPU raster with 11+234+11 composition.
- SameBoy, RP2C02/NESEmu behavior, palettes, clock modes, and SGB handling are unchanged.

### Project status

The software proof-of-concept is considered complete at this release. Future work, if any, belongs to physical hardware validation rather than the desktop PoC.

## v0.1.1-poc — 2026-09-17

### Fixed

- Normal DMG/SGB cartridge startup now follows SameBoy's documented boot-ROM callback API instead of using the project smoke-test stub as a general boot image.
- Embedded open-source SameBoy DMG/SGB boot-ROM resources are compiled from the pinned SameBoy revision and retain SameBoy's Expat license notice.
- `--boot` is now an optional explicit override rather than a mandatory argument.
- CI smoke cartridges no longer write `$FF50` themselves, preventing the previous false-green startup test.
- CGB-only cartridges are rejected with an explicit model-compatibility message instead of being forced through the DMG/SGB PoC path.

### Unchanged

- SameBoy core remains unmodified at pinned revision `213a12ce93d66b105a113debd9396306066a7cfc`.
- The scaler, RP2C02 backend, palette system, and SGB palette abstraction are unchanged.

## v0.1.0-poc — 2026-09-17

First reproducible hybrid-emulator proof of concept.

### Added

- SameBoy-backed Game Boy source.
- Fixed 160×144 → 234×240 bridge with 11-dot side borders.
- Reduced RP2C02 regression model.
- Pinned johnmph/NESEmu Ricoh2C02 preview backend.
- SDL2 side-by-side viewer.
- STOCK and SYNC clock models.
- 16 candidate global RP2C02 palettes.
- Interactive palette editor.
- Simple AUTO/SGB palette path using state already decoded by SameBoy.
- Project-authored DMG boot stub and smoke ROM.
- GitHub Actions coverage for core, SDL2, NESEmu, and SameBoy+NESEmu builds.

### Scope

This release is a **software proof of concept**. It does not claim physical RP2C02 EXT compatibility, CRT colorimetry accuracy, Game Boy electrical timing validation, or a production hardware implementation.

### Licensing

Licensing is finalized for this release:

- software: MIT;
- hardware: CERN-OHL-W-2.0;
- documentation: CC BY-SA 4.0.
