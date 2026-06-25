#!/usr/bin/env python3
"""Parse FONT v1 (font_asc.bin) glyph offset table and extract glyph bitmaps."""
from __future__ import annotations

import struct
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    Image = None  # type: ignore

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "Output/bin/ui/0font/font_asc.bin"


def parse_v1(path: Path) -> tuple[int, int, list[int], int]:
    d = path.read_bytes()
    if d[:4] != b"FONT" or struct.unpack_from("<H", d, 12)[0] != 1:
        raise SystemExit("expect FONT v1")
    height = d[14]
    # offset table at 64, one u32 per glyph index (0x20..0x7e = 95 chars)
    count = 95
    offs = [struct.unpack_from("<I", d, 64 + i * 4)[0] for i in range(count)]
    # validate offsets - should be increasing and within file
    data_start = min(o for o in offs if o > 0)
    print(f"{path.name}: height={height} data_start={data_start} file={len(d)}")
    print("first offsets:", offs[:8])
    print("last offsets:", offs[-8:])
    # glyph sizes from delta
    sizes = []
    sorted_offs = sorted(set(offs))
    for i, o in enumerate(offs):
        nxt = len(d)
        for o2 in offs:
            if o2 > o:
                nxt = min(nxt, o2)
        sizes.append(nxt - o)
    print("size min/max/avg:", min(sizes), max(sizes), sum(sizes) // len(sizes))
    return height, count, offs, data_start


def main() -> None:
    parse_v1(SRC)


if __name__ == "__main__":
    main()
