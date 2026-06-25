#!/usr/bin/env python3
"""Parse FONT bin glyph layout by validating offset chains."""
from __future__ import annotations

import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def parse_font(path: Path) -> None:
    d = path.read_bytes()
    if d[:4] != b"FONT":
        raise SystemExit(f"{path.name}: bad magic")
    h = d[14]
    print(f"\n=== {path.name} size={len(d)} height@14={h} ===")
    # scan for plausible width table: sequence of small u16 4..40
    best = None
    for base in range(48, min(600, len(d) - 200), 2):
        widths = []
        ok = True
        for i in range(96):
            if base + (i + 1) * 2 > len(d):
                ok = False
                break
            w = struct.unpack_from("<H", d, base + i * 2)[0]
            if w > 80:
                ok = False
                break
            widths.append(w)
        if ok and 32 <= len(widths) <= 96:
            avg = sum(widths) / len(widths)
            if 4 <= avg <= 30:
                score = abs(len(widths) - 95) + abs(avg - h * 0.4)
                if best is None or score < best[0]:
                    best = (score, base, widths)
    if best:
        _, base, widths = best
        print(f"width table candidate @ {base}: n={len(widths)} avg={sum(widths)/len(widths):.1f}")
        print("  first20:", widths[:20])
        print("  max:", max(widths))
        off_base = base + len(widths) * 2
        # align to 4
        off_base = (off_base + 3) & ~3
        print(f"offset table guess @ {off_base}")
        offs = []
        for i in range(min(len(widths), 20)):
            if off_base + (i + 1) * 4 > len(d):
                break
            offs.append(struct.unpack_from("<I", d, off_base + i * 4)[0])
        print("  first offsets:", [hex(o) for o in offs[:10]])
        if offs:
            data_base = min(o for o in offs if o < len(d))
            print(f"  min offset {data_base}, tail={len(d)-data_base}")


def main() -> None:
    font_dir = ROOT / "Output/bin/ui/0font"
    for name in ("font_asc.bin", "font_num_24.bin", "font.bin"):
        p = font_dir / name
        if p.exists():
            parse_font(p)


if __name__ == "__main__":
    main()
