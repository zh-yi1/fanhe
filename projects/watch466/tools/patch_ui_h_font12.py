#!/usr/bin/env python3
"""Insert font_asc_12 into ui.h and shift following resource addresses."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FONT12_LEN = 0x271A  # 10010 bytes
INSERT_AFTER = "UI_LEN_0FONT_FONT_ASC_10_BIN"
NEW_BLOCK = f"""
#define UI_BUF_0FONT_FONT_ASC_12_BIN               UI_ADDR_BASE(0xccc4)
#define UI_LEN_0FONT_FONT_ASC_12_BIN               0x{FONT12_LEN:x}
"""
SHIFT = FONT12_LEN
THRESHOLD = 0xCCC4


def patch_ui_h(path: Path) -> None:
    text = path.read_text(encoding="utf-8")
    if "UI_BUF_0FONT_FONT_ASC_12_BIN" in text:
        print(f"{path.name}: already patched")
        return

    marker = f"#define {INSERT_AFTER}"
    if marker not in text:
        raise SystemExit(f"{path}: missing {INSERT_AFTER}")

    text = text.replace(
        f"{marker}                      0x5125",
        f"{marker}                      0x5125{NEW_BLOCK}",
        1,
    )

    def repl_addr(m: re.Match[str]) -> str:
        val = int(m.group(1), 16)
        if val >= THRESHOLD:
            val += SHIFT
        return f"UI_ADDR_BASE(0x{val:x})"

    text = re.sub(r"UI_ADDR_BASE\(0x([0-9a-fA-F]+)\)", repl_addr, text)
    path.write_text(text, encoding="utf-8")
    print(f"patched {path}")


def main() -> None:
    for rel in ("Output/bin/ui.h", "ui.h"):
        path = ROOT / rel
        if path.exists():
            patch_ui_h(path)


if __name__ == "__main__":
    main()
