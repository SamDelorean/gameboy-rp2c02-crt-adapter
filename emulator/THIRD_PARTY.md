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

The SameBoy **core is not vendored** into this repository. `emulator/scripts/bootstrap_sameboy.sh` clones the pinned upstream commit into the ignored `emulator/third_party/` directory and builds the static core archive.

Two 256-byte open-source boot ROM images (DMG and SGB) are embedded in `emulator/src/sameboy_bootroms.c`. They are reproducibly compiled from SameBoy's own `BootROMs/dmg_boot.asm` and `BootROMs/sgb_boot.asm` at the pinned revision using RGBDS v1.0.3. They are used only through SameBoy's documented `GB_set_boot_rom_load_callback()` / `GB_load_boot_rom_from_buffer()` frontend API so normal cartridges receive a model-appropriate boot sequence without requiring a proprietary Nintendo boot ROM.

The embedded boot-ROM resources retain SameBoy's copyright notice and Expat license. The complete notice is stored in `emulator/third_party_licenses/SAMEBOY_LICENSE.txt`.

The local adapter in `emulator/src/gb_source_sameboy.c` is project code written against SameBoy's public library API.

## johnmph/NESEmu RP2C02 donor

Repository: https://github.com/johnmph/NESEmu

Pinned integration commit:

```text
4966aa09259ef965d4b6bd2635a1dfe57a8569cb
```

Current role:

- optional donor implementation of the **Ricoh 2C02 PPU only** for the right-hand preview path;
- receives the project's already-scaled `EXT0..EXT3` nibble stream through NESEmu's public `exts(uint8_t)` entry point;
- receives the project's palette/register state through normal PPU register writes;
- returns native six-bit RP2C02 color codes through its `plotPixel()` graphics callback;
- does **not** bring in the 6502 CPU, APU, NES cartridge/mappers, CHR/game rendering, or NESEmu's frontend;
- does **not** synthesize analog NTSC/VOUT; desktop RGB conversion remains a simple preview LUT after the PPU color code is produced.

The wrapper is `emulator/src/rp2c02_nesemu.cpp` and exposes a small C ABI so the rest of the emulator remains C. The dependency-free reduced `rp2c02_ext`/timing path remains available as a regression oracle and fallback build.

NESEmu is **not vendored** into this repository. `emulator/scripts/bootstrap_nesemu.sh` fetches the pinned commit into the ignored `emulator/third_party/NESEmu` directory. The integration compiles the donor PPU templates and NESEmu's small `Common.cpp` helper only.

Licensing note: the pinned NESEmu repository is MIT licensed. The upstream copyright and license notice must be retained when redistributing builds or copied source that includes NESEmu code.

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

The normal ROM-backed path uses the embedded **open-source SameBoy boot ROMs** described above. `--boot` remains an optional explicit override for validation with another legally obtained compatible boot ROM.
