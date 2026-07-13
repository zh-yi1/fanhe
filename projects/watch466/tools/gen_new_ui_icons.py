#!/usr/bin/env python3
"""Generate Output/bin/ui/new_ui/*.bin (white background) for func_new_home.c.

PNG sources (first match wins):
  Output/bin/ui/new_ui/new_*.png   (design assets, preferred)
  res/new_ui/logo.png ...          (legacy names)
  res/home/0m.png ...              (time digits fallback)

Tab icons (HEAT_0/HEAT_1/MODE_0/MODE_1/SETUP_0/SETUP_1):
  tools/gen_new_ui_tab_icons.py (run separately if ICON_ITEMS PNGs are missing)

Then: python tools/gen_new_ui_icons.py
      python tools/gen_new_ui_tab_icons.py
      Output/bin/prebuild.bat
"""

from __future__ import annotations

import struct
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SRC_DIR = ROOT / "res" / "new_ui"
HOME_SRC = ROOT / "res" / "home"
BIN_PNG_DIR = ROOT / "Output" / "bin" / "ui" / "new_ui"
BIN_DIR = BIN_PNG_DIR
OUT_H = ROOT / "functions" / "new_home_icon_res.h"

BG_WHITE = (255, 255, 255)
# 状态栏小图标：透明→白底，非透明→黑色（白底黑图）
BLACK_ICON_ON_WHITE = frozenset({"new_bluetooth"})

ICON_ITEMS = [
    (("new_logo.png", "logo.png"), "new_logo"),
    (("new_bluetooth.png", "bluetooth.png"), "new_bluetooth"),
    (("new_dl.png", "dl.png"), "new_dl"),
    (("new_dl1.png", "dl1.png"), "new_dl1"),
    (("new_dl2.png", "dl2.png"), "new_dl2"),
    (("new_dl3.png", "dl3.png"), "new_dl3"),
    (("new_dl4.png", "dl4.png"), "new_dl4"),
    (("new_charging_1.png",), "new_charging_1"),
    (("new_charging_2.png",), "new_charging_2"),
    (("new_charging_3.png",), "new_charging_3"),
    (("new_charging_4.png",), "new_charging_4"),
    (("new_lock.png",), "new_lock"),
    (("new_unlock.png",), "new_unlock"),
    (("didian.png",), "didian"),
]

TIME_ITEMS = (
    *(f"{d}m.png" for d in range(10)),
    "colonm.png",
    "AMm.png",
    "PMm.png",
)


def rgba565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def pack_gpu(w: int, h: int, pixels: list[int]) -> bytes:
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, w, h)
    for c in pixels:
        buf += struct.pack("<H", c)
    return bytes(buf)


def pixels_from_rgba(
    im: Image.Image,
    bg_rgb: tuple[int, int, int],
    *,
    black_icon: bool = False,
) -> tuple[int, int, list[int]]:
    w, h = im.size
    px = im.load()
    bg = rgba565(*bg_rgb)
    black = rgba565(0, 0, 0)
    out: list[int] = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 32:
                out.append(bg)
            elif black_icon:
                out.append(black)
            else:
                out.append(rgba565(r, g, b))
    return w, h, out


def crop_content_bbox(im: Image.Image, pad: int = 8) -> Image.Image:
    """裁掉全屏白底，仅保留图标区域（低电页用）。"""
    w, h = im.size
    px = im.load()
    minx, miny, maxx, maxy = w, h, 0, 0
    found = False
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 32:
                continue
            if r > 240 and g > 240 and b > 240:
                continue
            found = True
            minx = min(minx, x)
            miny = min(miny, y)
            maxx = max(maxx, x)
            maxy = max(maxy, y)
    if not found:
        return im
    minx = max(0, minx - pad)
    miny = max(0, miny - pad)
    maxx = min(w - 1, maxx + pad)
    maxy = min(h - 1, maxy + pad)
    return im.crop((minx, miny, maxx + 1, maxy + 1))


def pixels_from_rgba_didian(im: Image.Image, bg_rgb: tuple[int, int, int]) -> tuple[int, int, list[int]]:
    """低电图标：仅保留红色线条，其余一律白底，去掉抗锯齿暗边。"""
    w, h = im.size
    px = im.load()
    bg = rgba565(*bg_rgb)
    out: list[int] = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 32:
                out.append(bg)
            elif r >= 120 and g <= 140 and b <= 140 and r > g and r > b:
                out.append(rgba565(r, g, b))
            else:
                out.append(bg)
    return w, h, out


def upscale_pixels_nearest(w: int, h: int, pixels: list[int], scale: int) -> tuple[int, int, list[int]]:
    if scale <= 1:
        return w, h, pixels
    nw, nh = w * scale, h * scale
    out: list[int] = []
    for y in range(h):
        row = pixels[y * w:(y + 1) * w]
        for _dy in range(scale):
            for x in range(w):
                c = row[x]
                out.extend([c] * scale)
    return nw, nh, out


def png_to_gpu_didian(path: Path, *, scale: int = 1) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    im = crop_content_bbox(im)
    w, h, pixels = pixels_from_rgba_didian(im, BG_WHITE)
    w, h, pixels = upscale_pixels_nearest(w, h, pixels, scale)
    return pack_gpu(w, h, pixels), w, h


def png_to_gpu(path: Path, *, black_icon: bool = False) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    w, h, pixels = pixels_from_rgba(im, BG_WHITE, black_icon=black_icon)
    return pack_gpu(w, h, pixels), w, h


def resolve_src(names: tuple[str, ...]) -> Path:
    for base in (BIN_PNG_DIR, SRC_DIR, HOME_SRC):
        for name in names:
            p = base / name
            if p.exists():
                return p
    raise SystemExit(f"missing {' / '.join(names)} (put PNG in Output/bin/ui/new_ui/ or res/new_ui/ or res/home/)")


def write_bin(out_name: str, data: bytes, src: Path) -> None:
    BIN_DIR.mkdir(parents=True, exist_ok=True)
    out = BIN_DIR / out_name
    out.write_bytes(data)
    print(f"{out_name}: {len(data)} bytes <- {src}")


def emit_header(sizes: dict[str, tuple[int, int]]) -> None:
    lines = [
        "#ifndef _NEW_HOME_ICON_RES_H",
        "#define _NEW_HOME_ICON_RES_H",
        "",
        "/* Generated by tools/gen_new_ui_icons.py */",
        "",
    ]
    key_map = {
        "new_logo": "NEW_HOME_LOGO",
        "new_bluetooth": "NEW_HOME_BT",
        "new_dl": "NEW_HOME_BAT_DL",
        "new_dl1": "NEW_HOME_BAT_DL1",
        "new_dl2": "NEW_HOME_BAT_DL2",
        "new_dl3": "NEW_HOME_BAT_DL3",
        "new_dl4": "NEW_HOME_BAT_DL4",
        "new_charging_1": "NEW_HOME_BAT_CHG1",
        "new_charging_2": "NEW_HOME_BAT_CHG2",
        "new_charging_3": "NEW_HOME_BAT_CHG3",
        "new_charging_4": "NEW_HOME_BAT_CHG4",
        "new_lock": "NEW_UI_LOCK",
        "new_unlock": "NEW_UI_UNLOCK",
        "didian": "NEW_UI_DIDIAN",
    }
    for stem, prefix in key_map.items():
        if stem in sizes:
            w, h = sizes[stem]
            lines.append(f"#define {prefix}_W                 {w}")
            lines.append(f"#define {prefix}_H                 {h}")
            lines.append(f"#define {prefix}_RAM_SIZE          (8 + {w} * {h} * 2)")
            lines.append("")
    if "new_dl" in sizes:
        w, h = sizes["new_dl"]
        lines.append(f"#define NEW_HOME_BAT_W                  {w}")
        lines.append(f"#define NEW_HOME_BAT_H                  {h}")
        lines.append("")
    if "new_bluetooth" in sizes:
        w, h = sizes["new_bluetooth"]
        lines.append(f"#define NEW_HOME_BT_W                   {w}")
        lines.append(f"#define NEW_HOME_BT_H                   {h}")
        lines.append("")
    lines.append("#endif")
    OUT_H.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"wrote {OUT_H}")


def read_bin_size(bin_path: Path) -> tuple[int, int]:
    data = bin_path.read_bytes()
    if len(data) < 8:
        raise ValueError(f"bad gpu bin: {bin_path}")
    w, h = struct.unpack("<HH", data[4:8])
    return w, h


def process_icon(png_names: tuple[str, ...], stem: str, sizes: dict[str, tuple[int, int]]) -> None:
    bin_path = BIN_DIR / f"{stem}.bin"
    try:
        src = resolve_src(png_names)
        if stem == "didian":
            data, w, h = png_to_gpu_didian(src)
        else:
            data, w, h = png_to_gpu(src, black_icon=(stem in BLACK_ICON_ON_WHITE))
        write_bin(f"{stem}.bin", data, src)
    except SystemExit:
        if not bin_path.exists():
            raise
        data = bin_path.read_bytes()
        w, h = read_bin_size(bin_path)
        print(f"{stem}.bin: keep existing {len(data)} bytes")
        src = bin_path
    sizes[stem] = (w, h)


def main() -> None:
    SRC_DIR.mkdir(parents=True, exist_ok=True)
    sizes: dict[str, tuple[int, int]] = {}

    for png_names, stem in ICON_ITEMS:
        process_icon(png_names, stem, sizes)

    for png_name in TIME_ITEMS:
        stem = "new_" + png_name.replace(".png", "")
        src = resolve_src((png_name,))
        data, w, h = png_to_gpu(src)
        write_bin(f"{stem}.bin", data, src)

    emit_header(sizes)
    print("\nTab icons: python tools/gen_new_ui_tab_icons.py")
    print("Next: run Output/bin/prebuild.bat, rebuild and flash ui.bin + app.bin")


if __name__ == "__main__":
    main()
