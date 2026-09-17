# Emulator references and third-party code policy

The hybrid emulator is intended to remain small and project-specific. External emulators are used as source cores, validation references, or test oracles only where that reduces duplicated work.

## SameBoy

Repository: https://github.com/LIJI32/SameBoy

Planned role:

- preferred first Game Boy execution/source core;
- normal Game Boy render retained as the left-hand comparison reference;
- 2-bit pixel path exported through the narrow integration callbacks already intended for SFC/SNES/SGB embedding;
- no need to reuse SameBoy's complete desktop frontend.

Licensing note: the general SameBoy core is distributed under the Expat/MIT-style license in its repository. iOS/HexFiend-specific exceptions are not needed for this project. Any imported or modified SameBoy source must retain the required copyright/license notices.

No SameBoy source has been copied into this repository as of emulator V0.1.

## Pinky / Visual2C02 tests

Repository: https://github.com/koute/pinky

Planned role:

- independent reference for RP2C02 behavior;
- especially useful because Pinky includes an RP2C02 test suite generated with help from Visual2C02 transistor-level simulation;
- validation oracle for the reduced project PPU model rather than a complete NES core to embed.

Pinky is available under MIT and Apache-2.0 licensing in its repository.

No Pinky source has been copied into this repository as of emulator V0.1.

## NESdev / Pan Docs

Behavioral claims should still be checked against primary/public technical documentation and, where relevant, real hardware measurements. Emulator-to-emulator agreement alone is not treated as hardware proof.

## ROM policy

No copyrighted commercial Game Boy ROM images are stored or distributed in this repository. Users supply their own legally obtained test/homebrew ROMs when the SameBoy-backed source path is added.
