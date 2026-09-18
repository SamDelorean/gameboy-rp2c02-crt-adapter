# Licensing

The licensing model for this repository is now final for the proof-of-concept release.

## Software — MIT

Project-authored software, emulator code, firmware/prototype code, tests, helper scripts, build logic, and CI/tooling are licensed under the **MIT License**.

The complete text is in [`LICENSE`](LICENSE).

This includes, unless a file states otherwise:

- `emulator/`
- `firmware/`
- project-authored tests and scripts
- CMake/build support
- GitHub Actions workflow files

## Hardware — CERN-OHL-W-2.0

Project-authored hardware design material under `hardware/` is licensed under the **CERN Open Hardware Licence Version 2 — Weakly Reciprocal (CERN-OHL-W-2.0)**.

The complete text is in [`hardware/LICENSE`](hardware/LICENSE).

## Documentation — CC BY-SA 4.0

Project-authored documentation under `docs/`, together with the root Markdown documentation such as `README.md`, `README_ES.md`, `ROADMAP.md`, `CONTRIBUTING.md`, and this file, is licensed under **Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)**.

The complete text is in [`docs/LICENSE`](docs/LICENSE).

## Third-party projects

The SameBoy core and johnmph/NESEmu core are **not vendored into this repository**; bootstrap scripts fetch pinned upstream revisions into ignored local directories.

The repository does embed two small open-source SameBoy boot-ROM resources (DMG and SGB), compiled from the pinned SameBoy source revision. Those bytes remain under SameBoy's Expat license and copyright notice rather than the project's MIT license. See [`emulator/third_party_licenses/SAMEBOY_LICENSE.txt`](emulator/third_party_licenses/SAMEBOY_LICENSE.txt).

Third-party projects and embedded third-party resources remain under their own licenses and copyright notices. See [`emulator/THIRD_PARTY.md`](emulator/THIRD_PARTY.md) for pinned revisions, roles, and license notes.

No commercial Game Boy ROM, Nintendo boot ROM, or other proprietary game image is distributed by this repository.

## Scope rule

If a file carries an explicit SPDX identifier or license notice, that file-level notice controls. Otherwise, use the directory/content-category allocation above.

Copyright for project-authored material remains with its respective contributor(s).
