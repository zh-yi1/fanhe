#!/usr/bin/env python3
"""Reverse-engineer FONT bin layout for font_asc.bin."""
from __future__ import annotations

import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FONT = ROOT / "Output/bin/ui/0font/font_asc.bin"


def main() -> None:
    d = FONT.read_bytes()
    print("size", len(d), "magic", d[:4], "height@14", d[14])
    print("header hex 0-64:", d[:64].hex(" "))
    start, end = struct.unpack_from("<HH", d, 48)
    print(f"code range @48: {start:#x} - {end:#x} ({end - start + 1} chars)")
    # widths table hypothesis
    n = end - start + 1
    for base in (72, 168, 360, 512):
        if base + n * 2 <= len(d):
            widths = [struct.unpack_from("<H", d, base + i * 2)[0] for i in range(min(n, 16))]
            print(f"widths@{base} first16:", widths)
    # offsets
    for base in (168, 360):
        offs = [struct.unpack_from("<I", d, base + i * 4)[0] for i in range(8)]
        print(f"u32 offsets@{base}:", [hex(x) for x in offs])


if __name__ == "__main__":
    main()
