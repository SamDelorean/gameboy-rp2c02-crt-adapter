#!/usr/bin/env python3
"""Compare DMG fallback and SameBoy-decoded SGB AUTO palette renders.

The virtual bench deliberately does not emulate P14/P15/JOYP transport in its
main path. SameBoy owns SGB command decoding and supplies already-decoded
palette state; project code only maps that palette into RP2C02 colors.

The left SameBoy reference is allowed to differ in SGB mode because SameBoy may
render its own SGB colorization there. The right RP2C02 region must differ from
the DMG fallback result when the SGB palette is active.
"""

from __future__ import annotations

import argparse
from pathlib import Path

WIDTH = 1280
HEIGHT = 720


def read_ppm(path: Path) -> bytes:
    raw = path.read_bytes()
    magic, dims, maximum, pixels = raw.split(b"\n", 3)
    if magic != b"P6" or dims != b"1280 720" or maximum != b"255":
        raise SystemExit(f"unexpected PPM header in {path}")
    if len(pixels) != WIDTH * HEIGHT * 3:
        raise SystemExit(f"unexpected PPM payload length in {path}")
    return pixels


def region_bytes(pixels: bytes, x0: int, y0: int, w: int, h: int) -> bytes:
    rows = []
    for y in range(y0, y0 + h):
        start = (y * WIDTH + x0) * 3
        rows.append(pixels[start : start + w * 3])
    return b"".join(rows)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("dmg", type=Path)
    parser.add_argument("sgb", type=Path)
    args = parser.parse_args()

    dmg = read_ppm(args.dmg)
    sgb = read_ppm(args.sgb)

    right_dmg = region_bytes(dmg, 699 + 11 * 2, 132, 234 * 2, 480)
    right_sgb = region_bytes(sgb, 699 + 11 * 2, 132, 234 * 2, 480)

    if right_dmg == right_sgb:
        raise SystemExit(
            "RP2C02 image did not change between DMG fallback and SameBoy SGB AUTO palette"
        )

    changed = sum(a != b for a, b in zip(right_dmg, right_sgb))
    print(
        "SGB AUTO palette OK: SameBoy supplied decoded palette state; "
        f"RP2C02 image changed in {changed} component bytes"
    )


if __name__ == "__main__":
    main()
