# Project Status

## Canonical repository

This repository is the canonical public repository for the project:

`SamDelorean/gameboy-rp2c02-crt-adapter`

An earlier accidentally named repository beginning with a leading hyphen is **not part of this project history** and should be ignored.

## Current phase

The **desktop/software proof of concept is complete**.

Final software release:

`v0.1.2-poc — Final software proof of concept`

The release proves the hybrid path:

```text
SameBoy
  -> four logical Game Boy shades
  -> project scaler / bridge
  -> 234x240 image + 11/11 project borders
  -> RP2C02 EXT / palette path
  -> pinned Ricoh2C02 preview backend
  -> SDL2 comparison viewer
```

Any future work belongs to a separate **physical-hardware validation phase**. The completed software PoC should not be reopened merely to pursue unmeasured hardware assumptions.

## Final software architecture

- SameBoy is the Game Boy source of truth; the project does not maintain a second Game Boy emulator.
- SameBoy remains pinned at `213a12ce93d66b105a113debd9396306066a7cfc`.
- The SameBoy core is unmodified; the project integrates through its public API.
- Normal DMG/SGB cartridge startup follows SameBoy's documented boot-ROM callback API.
- Open-source SameBoy DMG/SGB boot ROM resources are embedded from the same pinned revision.
- `--boot` remains only as an explicit override.
- CGB-only cartridges are rejected explicitly because the PoC target is DMG/SGB.
- The project bridge scales `160x144 -> 234x240`.
- The RP2C02 active raster remains `256x240` with fixed project composition `11 + 234 + 11`.
- The preferred RP2C02 preview uses the pinned Ricoh2C02 implementation from johnmph/NESEmu; the reduced project model remains a regression oracle.
- Manual palette support consists of 16 global candidate presets plus `AUTO/SGB`.
- SameBoy-provided SGB palette state is limited to one decoded four-color RGB555 palette.
- The virtual bench does **not** emulate P14/P15/JOYP transport, regional attributes, graphical borders, tile transfers, game identification, or full SGB behavior.
- The physical-design buffering criterion is a **minimal two-line working buffer/register**. Earlier full-frame ping-pong framebuffer descriptions are historical and are not a current architectural requirement.

## Final comparison-view geometry

The viewer deliberately compares the two outputs at the same displayed height:

- Game Boy reference: `480x432`, preserving the logical `160:144` shape.
- CRT / RP2C02 preview: `576x432`, representing a 4:3 NTSC picture.

For physical-proportion preview only, the desktop renderer uses the NES NTSC `280x240` picture model:

```text
12 side-border samples + 256 active PPU samples + 12 side-border samples
= 280x240
```

This is a presentation transform only. It does not change the adapter raster, scaler, EXT data, palette path, or physical buffer requirements.

## Validation status

Final software validation is complete:

- local CTest suite: **9/9 PASS**;
- DMG smoke path: **PASS**;
- SGB smoke path: **PASS** beyond SameBoy's documented 200-frame HLE intro interval;
- AUTO/SGB palette-difference regression: **PASS**;
- donor RP2C02/NESEmu path: **PASS**;
- SDL2 viewer build: **PASS**;
- CGB-only compatibility rejection: **PASS**;
- normal cartridge startup validated after replacing the project smoke boot stub with the proper SameBoy boot-ROM callback integration;
- final `main` CI run `35293211190`: **4/4 jobs green**.

At final release, the repository had 0 open PRs and 0 open software issues.

## Licensing

The release licensing model is final:

- software: **MIT**;
- hardware: **CERN-OHL-W-2.0**;
- documentation: **CC BY-SA 4.0**.

Third-party components and embedded third-party resources retain their own license notices.

## Physical design status

The software PoC does **not** claim physical validation.

The following are future hardware tasks only if the project is reopened:

- validate RP2C02 / compatible-clone EXT0..EXT3 behavior on a real bench;
- validate the common-clock generator and output conditioning;
- validate Game Boy clock injection on the chosen donor revision;
- validate Game Boy LCD signal capture and electrical loading;
- validate voltage-level adaptation;
- validate composite output and CRT behavior;
- validate physical SGB source compatibility;
- freeze production schematics / PCB only after measurements.

The historical RP2350/Pico 2 and Arduino-Pico work remains reference/prototype material rather than a requirement of the completed desktop PoC.

## Publication rule

Do not label an implementation detail or hardware configuration as finalized/compatible until it has either:

- been explicitly selected as a project decision, or
- been verified on representative hardware where measurement is required.

An emulator or software model is evidence and a validation aid; it is not proof of physical hardware behavior unless cross-checked against documentation and bench measurements.

## Backup state

The project Drive structure is organized under `Game Boy CRT Driver - RP2C02`.

The final backup snapshot created on 2026-11-18 is stored under:

`06_Cierre_Final_v0.1.2-poc`

It contains snapshots of the canonical project documents plus a master closeout/status document. GitHub remains the canonical source for code and public technical history.

## T160 — temporary file exchange through Google Drive

Canonical temporary path:

`/home/spc/GoogleDrive/apit160/temporal-trabajo`

- Use this folder only as a **temporary** exchange area when copying a small, task-specific set of working files materially reduces remote-tool overhead.
- Prefer one subfolder per task and copy only the files that are strictly necessary.
- Local originals remain the source of truth; the Drive copy never replaces the repository, local state, bitácora, or backup.
- Keep the temporary area clean when a task is closed, removing only that task's files/subfolders.
- Do not upload ROMs, dumps, proprietary/derived firmware, Ghidra projects containing firmware, OAuth credentials, tokens, cookies, keys, secrets, or other restricted material without explicit authorization.
- Drive is not part of the critical path. If unavailable, continue with grouped remote operations rather than blocking the task.
- Use of this temporary area does not authorize commits, pushes, publication, scope expansion, or deletion of original sources.
