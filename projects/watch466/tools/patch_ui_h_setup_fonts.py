#!/usr/bin/env python3
"""Patch ui.h 0font section after regenerating font_asc_10/8 with gen_font_asc_scaled.py."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# Offsets after font_asc.bin (must match loadresdir order in ui/0font)
FONT_CHAIN = [
    ("UI_BUF_0FONT_FONT_BIN", 0x0, 0x2A7A),
    ("UI_BUF_0FONT_FONT_ASC_BIN", None, 0x5125),
    ("UI_BUF_0FONT_FONT_ASC_10_BIN", None, 0x1F5E),
    ("UI_BUF_0FONT_FONT_ASC_8_BIN", None, 0x1BCE),
    ("UI_BUF_0FONT_FONT_ASC_12_BIN", None, 0x271A),
    ("UI_BUF_0FONT_FONT_ASC_16_BIN", None, 0x32BA),
    ("UI_BUF_0FONT_FONT_NUM_24_BIN", None, 0x1788),
    ("UI_BUF_0FONT_FONT_NUM_38_BIN", None, 0x35D9),
    ("UI_BUF_0FONT_FONT_NUM_46_BIN", None, 0x4D58),
]

THRESHOLD = 0xCCC4  # first addr that moves when font_asc_10 shrinks


def build_0font_block() -> str:
    addr = 0
    lines: list[str] = []
    for name, _fixed, length in FONT_CHAIN:
        lines.append(f"#define {name:<42} UI_ADDR_BASE(0x{addr:x})")
        lines.append(f"#define UI_LEN_{name[7:]:<42} 0x{length:x}")
        lines.append("")
        addr += length
    return "\n".join(lines).rstrip() + "\n"


def patch_ui_h(path: Path) -> None:
    text = path.read_text(encoding="utf-8")
    start = text.find("#define UI_BUF_0FONT_FONT_BIN")
    end = text.find("#define UI_BUF_0GPU_GPU_BIN")
    if start < 0 or end < 0:
        raise SystemExit(f"{path}: cannot find 0font block")

    old_block = text[start:end]
    new_block = build_0font_block()

    # Shift all resource addresses at/after old font_asc_12 base
    new_12_addr = 0x7B9F + 0x1F5E + 0x1BCE  # after asc + 10 + 8
    shift = THRESHOLD - new_12_addr

    tail = text[end:]
    tail = re.sub(
        r"UI_ADDR_BASE\(0x([0-9a-fA-F]+)\)",
        lambda m: (
            f"UI_ADDR_BASE(0x{int(m.group(1), 16) - shift:x})"
            if int(m.group(1), 16) >= THRESHOLD
            else m.group(0)
        ),
        tail,
    )

    path.write_text(text[:start] + new_block + tail, encoding="utf-8")
    print(f"patched {path} (shift -0x{shift:x} after 0font)")


def main() -> None:
    for rel in ("ui.h", "Output/bin/ui.h"):
        path = ROOT / rel
        if path.exists():
            patch_ui_h(path)


if __name__ == "__main__":
    main()
