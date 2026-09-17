#!/usr/bin/env python3
"""Generate a tiny project-authored DMG boot stub and smoke-test ROM.

The files produced by this script contain no Nintendo boot ROM or commercial
software.  They exist only to make the SameBoy-backed alternate-video path
executable in CI.
"""

from __future__ import annotations

import argparse
from pathlib import Path

ROM_SIZE = 32 * 1024


def emit(rom: bytearray, address: int, data: bytes) -> int:
    rom[address : address + len(data)] = data
    return address + len(data)


def make_boot_stub() -> bytes:
    boot = bytearray(0x100)
    # Reset entry: jump directly to the cartridge entry point.  The cartridge
    # itself disables the boot-ROM mapping through FF50 before configuring LCD.
    boot[0:3] = bytes((0xC3, 0x00, 0x01))  # JP $0100
    return bytes(boot)


def make_rom() -> bytes:
    rom = bytearray([0x00] * ROM_SIZE)

    # Standard cartridge entry area.  Keep the header region available by
    # jumping to project code at $0150.
    rom[0x0100:0x0104] = bytes((0xC3, 0x50, 0x01, 0x00))  # JP $0150; NOP

    title = b"GBCRT-SMOKE"
    rom[0x0134 : 0x0134 + len(title)] = title
    rom[0x0143] = 0x00  # DMG-compatible
    rom[0x0146] = 0x00  # no SGB requirement
    rom[0x0147] = 0x00  # ROM ONLY
    rom[0x0148] = 0x00  # 32 KiB
    rom[0x0149] = 0x00  # no cartridge RAM
    rom[0x014A] = 0x01  # non-Japanese destination
    rom[0x014B] = 0x00
    rom[0x014C] = 0x00

    pc = 0x0150
    code = bytearray()

    # Basic deterministic DMG setup.  SameBoy continues to provide all CPU,
    # memory and LCD/PPU behavior; this code merely creates a visible pattern.
    code += bytes((0xF3,))                    # DI
    code += bytes((0x31, 0xFE, 0xFF))        # LD SP,$FFFE
    code += bytes((0x3E, 0x01))              # LD A,$01
    code += bytes((0xE0, 0x50))              # LDH ($FF50),A ; unmap boot stub
    code += bytes((0xAF,))                    # XOR A
    code += bytes((0xE0, 0x40))              # LDH ($FF40),A ; LCD off
    code += bytes((0x3E, 0xE4))              # LD A,$E4
    code += bytes((0xE0, 0x47))              # LDH ($FF47),A ; BGP 0,1,2,3

    # Tile 0: alternating shade 1 / shade 2 pixels on every line.
    code += bytes((0x21, 0x00, 0x80))        # LD HL,$8000
    code += bytes((0x06, 0x08))              # LD B,8
    tile_loop = len(code)
    code += bytes((0x3E, 0xAA))              # LD A,$AA
    code += bytes((0x22,))                    # LD (HL+),A
    code += bytes((0x3E, 0x55))              # LD A,$55
    code += bytes((0x22,))                    # LD (HL+),A
    code += bytes((0x05,))                    # DEC B
    offset = tile_loop - (len(code) + 2)
    code += bytes((0x20, offset & 0xFF))      # JR NZ,tile_loop

    # Fill the entire $9800 background map with tile 0.  C starts at zero so
    # DEC C naturally gives 256 iterations; B repeats that four times.
    code += bytes((0x21, 0x00, 0x98))        # LD HL,$9800
    code += bytes((0xAF,))                    # XOR A
    code += bytes((0x06, 0x04))              # LD B,4
    code += bytes((0x0E, 0x00))              # LD C,0
    map_loop = len(code)
    code += bytes((0x22,))                    # LD (HL+),A
    code += bytes((0x0D,))                    # DEC C
    inner_offset = map_loop - (len(code) + 2)
    code += bytes((0x20, inner_offset & 0xFF))
    code += bytes((0x05,))                    # DEC B
    outer_offset = map_loop - (len(code) + 2)
    code += bytes((0x20, outer_offset & 0xFF))

    code += bytes((0xAF,))                    # XOR A
    code += bytes((0xE0, 0x42))              # LDH ($FF42),A ; SCY=0
    code += bytes((0xE0, 0x43))              # LDH ($FF43),A ; SCX=0
    code += bytes((0x3E, 0x91))              # LCD on, BG on, $8000 tiles
    code += bytes((0xE0, 0x40))              # LDH ($FF40),A
    code += bytes((0x18, 0xFE))              # JR $ ; display forever

    pc = emit(rom, pc, bytes(code))
    if pc >= 0x0200:
        raise RuntimeError("smoke-test program unexpectedly grew beyond $01FF")

    # Header checksum.  The boot stub intentionally does not validate the
    # Nintendo logo; the checksum is still kept internally consistent.
    checksum = 0
    for value in rom[0x0134:0x014D]:
        checksum = (checksum - value - 1) & 0xFF
    rom[0x014D] = checksum

    # Global checksum, excluding the checksum bytes themselves.
    total = sum(rom[:0x014E]) + sum(rom[0x0150:])
    rom[0x014E] = (total >> 8) & 0xFF
    rom[0x014F] = total & 0xFF

    return bytes(rom)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("output_dir", type=Path)
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    boot_path = args.output_dir / "gbcrt_boot_stub.bin"
    rom_path = args.output_dir / "gbcrt_smoke.gb"

    boot_path.write_bytes(make_boot_stub())
    rom_path.write_bytes(make_rom())

    print(boot_path)
    print(rom_path)


if __name__ == "__main__":
    main()
