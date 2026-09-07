# Firmware

Firmware source will be added after the controller selection is closed.

## Compatibility target

Firmware is planned around a common **Game Boy DMG / SGB** source pipeline. Once a valid 160x144x2-bit Game Boy frame is reconstructed, buffering, scaling, border generation, palette mapping and RP2C02 EXT output should be identical regardless of whether the source hardware is DMG or a validated SGB/SGB-CPU-compatible configuration.

SGB-specific `P14/P15` packet decoding is a separate optional module and must not contaminate the common capture/scaler path.

## Version 1 responsibilities

1. initialize the selected clock/control peripherals,
2. reset and minimally initialize the RP2C02-compatible PPU,
3. load the active palette,
4. capture the Game Boy DMG / SGB-compatible LCD/video stream,
5. maintain ping-pong 160x144x2-bit framebuffers,
6. scale vertically `144 -> 240` using fixed `5/3` repetition,
7. scale horizontally `160 -> 234` using deterministic integer nearest-neighbor repetition,
8. generate the 11-dot left/right border regions separately from the scaler,
9. drive `EXT0..EXT3` deterministically,
10. coordinate presentation/palette operations around VBlank,
11. scan the single palette button,
12. optionally decode the minimal SGB palette subset from `P14/P15`,
13. guarantee manual palette override of SGB-derived colors,
14. provide bench-test patterns and diagnostics for both source types where available.

## Constraints

- no pixel-rate interrupt bit-banging,
- no scaled 234x240 or 256x240 framebuffer requirement,
- no NES CPU emulation,
- no normal tile/sprite rendering in version 1,
- no game recognition,
- no alternate stretch/zoom/crop scaling modes in version 1,
- SGB video compatibility should use the same downstream pipeline as DMG,
- SGB-lite palette listening must remain optional,
- full SGB emulation is outside version 1,
- timing constants, source-interface details and palette data should be isolated from hardware-independent scaler/output logic.

See [`architecture.md`](architecture.md) and [`../docs/sgb-lite.md`](../docs/sgb-lite.md).
