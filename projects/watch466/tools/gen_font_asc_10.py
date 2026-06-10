#!/usr/bin/env python3
"""Build font_asc_10.bin (10px ASCII) from font_asc.bin for Time page H/Min labels.

Output: Output/bin/ui/0font/font_asc_10.bin
Run Output/bin/prebuild.bat afterward to refresh ui.h and ui.bin.
"""

from __future__ import annotations

import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC_FONT = ROOT / "Output" / "bin" / "ui" / "0font" / "font_asc.bin"
OUT_FONT = ROOT / "Output" / "bin" / "ui" / "0font" / "font_asc_10.bin"
FONT_HEIGHT = 10


def main() -> None:
    if not SRC_FONT.exists():
        raise SystemExit(f"missing source font: {SRC_FONT}")

    data = bytearray(SRC_FONT.read_bytes())
    if data[:4] != b"FONT":
        raise SystemExit(f"invalid FONT magic in {SRC_FONT.name}")

    old_h = data[14]
    data[14] = FONT_HEIGHT
    OUT_FONT.write_bytes(data)
    print(f"wrote {OUT_FONT.name}: height {old_h} -> {FONT_HEIGHT}, size {len(data)}")


if __name__ == "__main__":
    main()
