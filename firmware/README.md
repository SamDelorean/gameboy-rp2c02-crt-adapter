# Firmware

Firmware source will be added after the controller selection is closed.

## Version 1 responsibilities

1. initialize the selected clock/control peripherals,
2. reset and minimally initialize the RP2C02-compatible PPU,
3. load the active palette,
4. capture the DMG LCD stream,
5. maintain ping-pong framebuffers,
6. scale 160x144 to 256x240,
7. drive `EXT0..EXT3` deterministically,
8. coordinate presentation/palette operations around VBlank,
9. scan the palette button,
10. optionally decode the minimal SGB palette subset,
11. provide bench-test patterns and diagnostics.

## Constraints

- no pixel-rate interrupt bit-banging,
- no NES CPU emulation,
- no normal tile/sprite rendering in version 1,
- no game recognition,
- SGB-lite must remain optional,
- timing constants and palette data should be isolated from hardware drivers.

See [`architecture.md`](architecture.md).
