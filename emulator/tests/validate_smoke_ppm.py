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
    parser.add_argument(
        "--allow-uniform-left",
        action="store_true",
        help="allow SameBoy reference to be uniform (used by SGB HLE smoke tests)",
    )
    args = parser.parse_args()

    pixels = read_ppm(args.ppm)

    # Both displays use the same 432-pixel height. The Game Boy preserves its
    # 160:144 shape (480x432); the CRT preview presents the 280x240 NTSC clean
    # aperture as 576x432 4:3.
    left = sampled_colors(pixels, 75, 150, 480, 432, 5)
    right_image = sampled_colors(pixels, 657, 150, 576, 432, 5)

    if len(left) < 2 and not args.allow_uniform_left:
        raise SystemExit(f"SameBoy reference region is uniform: {left}")
    if len(right_image) < 2:
        raise SystemExit(
            f"RP2C02 alternate-output image is uniform: {right_image}"
        )

    # Check the project-owned 11-dot borders inside the 256-dot PPU active
    # raster, not merely the additional 12+12 NTSC presentation padding.
    def display_x_for_ppu_x(ppu_x: int) -> int:
        target = 12 + ppu_x
        dx = (target * 576 + 279) // 280
        while (dx * 280) // 576 < target:
            dx += 1
        return 657 + dx

    y_mid = 150 + (120 * 432) // 240
    left_border = pixel(pixels, display_x_for_ppu_x(0), y_mid)
    right_border = pixel(pixels, display_x_for_ppu_x(255), y_mid)
    if left_border != (0, 0, 0) or right_border != (0, 0, 0):
        raise SystemExit(
            f"RP2C02 side border is not black: {left_border}, {right_border}"
        )

    print(
        "smoke PPM OK: "
        f"SameBoy reference has {len(left)} sampled colors"
        f"{' (allowed for SGB HLE)' if len(left) < 2 else ''}; "
        f"RP2C02 image has {len(right_image)} sampled colors; "
        "dedicated side borders are black"
    )


if __name__ == "__main__":
    main()
