#!/usr/bin/env python3
"""Validate that the SameBoy and RP2C02 comparison regions contain video."""

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
    expected = WIDTH * HEIGHT * 3
    if len(pixels) != expected:
        raise SystemExit(
            f"unexpected pixel payload: {len(pixels)} bytes, expected {expected}"
        )
    return pixels


def pixel(pixels: bytes, x: int, y: int) -> tuple[int, int, int]:
    i = (y * WIDTH + x) * 3
    return pixels[i], pixels[i + 1], pixels[i + 2]


def sampled_colors(
    pixels: bytes,
    x0: int,
    y0: int,
    width: int,
    height: int,
    step: int,
) -> set[tuple[int, int, int]]:
    colors: set[tuple[int, int, int]] = set()
    for y in range(y0, y0 + height, step):
        for x in range(x0, x0 + width, step):
            colors.add(pixel(pixels, x, y))
    return colors


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("ppm", type=Path)
    args = parser.parse_args()

    pixels = read_ppm(args.ppm)

    # Exact video wells from comparison_render.c.
    left = sampled_colors(pixels, 85, 150, 480, 432, 6)
    right = sampled_colors(pixels, 699, 132, 512, 480, 6)

    if len(left) < 2:
        raise SystemExit(f"SameBoy reference region is uniform: {left}")
    if len(right) < 2:
        raise SystemExit(f"RP2C02 alternate-output region is uniform: {right}")

    print(
        "smoke PPM OK: "
        f"SameBoy reference has {len(left)} sampled colors; "
        f"RP2C02 path has {len(right)} sampled colors"
    )


if __name__ == "__main__":
    main()
