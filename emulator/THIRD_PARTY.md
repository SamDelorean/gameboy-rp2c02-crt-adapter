# Emulator references and third-party code policy

The hybrid emulator is intended to remain small and project-specific. External emulators are used as source cores, validation references, or test oracles only where that reduces duplicated work.

## SameBoy

Repository: https://github.com/LIJI32/SameBoy

Pinned first-integration commit:

```text
213a12ce93d66b105a113debd9396306066a7cfc
```

Current role:

- preferred first Game Boy execution/source core;
- normal Game Boy render retained as the left-hand comparison reference;
- the same rendered DMG frame is converted back to four final shade indices for the right-hand bridge path in emulator V0.2;
- later signal-level work may use/extend SameBoy's SFC/SNES integration callbacks for pixel and H/V reset events;
- no need to reuse SameBoy's complete desktop frontend.

Licensing note: the general SameBoy core is distributed under the Expat/MIT-style license in its repository. iOS/HexFiend-specific exceptions are not needed for this project. Any imported or modified SameBoy source must retain the required copyright/license notices.

SameBoy is **not vendored** into this repository. `emulator/scripts/bootstrap_sameboy.sh` clones the pinned upstream commit into the ignored `emulator/third_party/` directory and invokes `make lib`.

The local adapter in `emulator/src/gb_source_sameboy.c` is project code written against SameBoy's public library API.

## Pinky / Visual2C02 tests

Repository: https://github.com/koute/pinky

Planned role:

- independent reference for RP2C02 behavior;
- especially useful because Pinky includes an RP2C02 test suite generated with help from Visual2C02 transistor-level simulation;
- validation oracle for the reduced project PPU model rather than a complete NES core to embed.

Pinky is available under MIT and Apache-2.0 licensing in its repository.

No Pinky source is copied into this repository.

## NESdev / Pan Docs

Behavioral claims should still be checked against primary/public technical documentation and, where relevant, real hardware measurements. Emulator-to-emulator agreement alone is not treated as hardware proof.

## ROM and boot-ROM policy

No copyrighted commercial Game Boy ROM images or Nintendo boot ROM images are stored or distributed in this repository. Users supply their own legally obtained ROMs for local testing.

SameBoy also contains open boot-ROM source; a later reproducible integration may build and use that source instead of requiring a separate Nintendo boot ROM image.
