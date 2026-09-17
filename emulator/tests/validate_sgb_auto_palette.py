#!/usr/bin/env python3
"""Compare DMG fallback and SGB AUTO palette smoke renders.

The project-authored ROM draws the same static Game Boy image in both source
models, but in SGB mode it also transmits a real PAL01 packet through JOYP.
The left reference therefore should remain identical while the right RP2C02
image should change color because the passive SGB-lite palette is applied.
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

    # Exact video wells from comparison_render.c, excluding UI/labels.
    left_dmg = region_bytes(dmg, 85, 150, 480, 432)
    left_sgb = region_bytes(sgb, 85, 150, 480, 432)
    right_dmg = region_bytes(dmg, 699 + 11 * 2, 132, 234 * 2, 480)
    right_sgb = region_bytes(sgb, 699 + 11 * 2, 132, 234 * 2, 480)

    if left_dmg != left_sgb:
        raise SystemExit(
            "source/reference video changed between DMG and SGB smoke modes; "
            "the PAL01 test should affect only the alternate-output palette"
        )

    if right_dmg == right_sgb:
        raise SystemExit(
            "RP2C02 image did not change between fallback and SGB AUTO palette"
        )

    changed = sum(a != b for a, b in zip(right_dmg, right_sgb))
    print(
        "SGB AUTO palette OK: source image identical; "
        f"RP2C02 image changed in {changed} component bytes"
    )


if __name__ == "__main__":
    main()
