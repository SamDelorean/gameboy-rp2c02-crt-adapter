# Project Status

## Canonical repository

This repository is the canonical public repository for the project:

`SamDelorean/gameboy-rp2c02-crt-adapter`

An earlier accidentally named repository beginning with a leading hyphen is **not part of this project history** and should be ignored.

## Current phase

Architecture and component-selection phase.

## Decisions already consolidated

- RP2C02-class NTSC PPU architecture retained.
- Direct DMG LCD capture retained.
- Fixed full-frame scaling retained.
- Ping-pong full-frame buffering retained.
- Common clock reference retained.
- Manual global palettes retained.
- Optional passive SGB-lite retained.
- Fixed black overscan for version 1.
- Clone PPU support treated as test-based compatibility.

## Decisions still open

- final digital controller,
- final common clock-generator IC,
- final voltage-level/interface components,
- exact prototype PPU revision/device,
- final palette preset count/table,
- final mixed-license declaration,
- production schematic and PCB.

## Publication rule

Do not label an implementation detail as finalized until it has either:

- been explicitly selected as a project decision, or
- been verified on representative hardware where measurement is required.
