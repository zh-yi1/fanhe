#!/usr/bin/env python3
"""Generate ui/home GPU bins for ui.bin (UI resource flash).

PNG sources live in res/home/ (ui/home/*.png are moved here on first run).
Output: Output/bin/ui/home/*.bin + functions/home_icon_res.h

After running this script, run Output/bin/prebuild.bat to refresh ui.h.
"""

from __future__ import annotations

import shutil
import struct
from pathlib import Path

from PIL import Image, ImageDraw

ICON_SIZE = 52
DIGIT_W = 48
DIGIT_H = 82
COLON_W = 24

ROOT = Path(__file__).resolve().parents[1]
SRC_DIR = ROOT / "res" / "home"
BIN_DIR = ROOT / "Output" / "bin" / "ui" / "home"
OUT_H = ROOT / "functions" / "home_icon_res.h"

BG_BLUE = (4, 109, 217)
BG_BLACK = (0, 0, 0)
FG_WHITE = (255, 255, 255)

NAV_ICON_ITEMS = [
    ("heat.png", "heat"),
    ("mode.png", "mode"),
    ("setting.png", "setup"),
]

STATUS_ITEMS = [
    ("bluetooth.png", "bluetooth"),
    ("lock.png", "lock"),
    ("battery_level.png", "battery_level"),
]

NATIVE_DIGIT_ITEMS = [
    *(f"w{d}x.png" for d in range(10)),
    *(f"b{d}x.png" for d in range(10)),
    "wbx.png",
    "bhx.png",
]


def rgba565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def ensure_src_layout() -> None:
    SRC_DIR.mkdir(parents=True, exist_ok=True)
    BIN_DIR.mkdir(parents=True, exist_ok=True)
    if any(SRC_DIR.glob("*.png")):
        for png in BIN_DIR.glob("*.png"):
            png.unlink()
        return
    moved = 0
    for png in sorted(BIN_DIR.glob("*.png")):
        shutil.move(str(png), str(SRC_DIR / png.name))
        moved += 1
    if moved:
        print(f"moved {moved} png(s) from ui/home -> res/home")


def pack_gpu(w: int, h: int, pixels: list[int]) -> bytes:
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, w, h)
    for c in pixels:
        buf += struct.pack("<H", c)
    return bytes(buf)


def pixels_from_rgba(
    im: Image.Image,
    bg_rgb: tuple[int, int, int],
    fg_rgb: tuple[int, int, int] | None = None,
) -> tuple[int, int, list[int]]:
    w, h = im.size
    px = im.load()
    bg = rgba565(*bg_rgb)
    fg = rgba565(*fg_rgb) if fg_rgb else None
    out: list[int] = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 32:
                out.append(bg)
            elif fg is not None:
                out.append(fg)
            else:
                out.append(rgba565(r, g, b))
    return w, h, out


def png_to_gpu(
    path: Path,
    width: int,
    height: int,
    bg_rgb: tuple[int, int, int],
    fg_rgb: tuple[int, int, int] | None,
) -> bytes:
    im = Image.open(path).convert("RGBA")
    im = im.resize((width, height), Image.Resampling.LANCZOS)
    w, h, pixels = pixels_from_rgba(im, bg_rgb, fg_rgb)
    return pack_gpu(w, h, pixels)


def png_native_to_gpu(path: Path) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    w, h, pixels = pixels_from_rgba(im, BG_BLACK, None)
    return pack_gpu(w, h, pixels), w, h


def nav_icon_to_gpu(
    path: Path,
    bg_rgb: tuple[int, int, int],
    fg_rgb: tuple[int, int, int],
) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    w, h, pixels = pixels_from_rgba(im, bg_rgb, fg_rgb)
    return pack_gpu(w, h, pixels), w, h


def digit_png_to_gpu(path: Path) -> bytes:
    im = Image.open(path).convert("RGBA")
    src_w, src_h = im.size
    scale = min(DIGIT_W / src_w, DIGIT_H / src_h)
    new_w = max(1, round(src_w * scale))
    new_h = max(1, round(src_h * scale))
    im = im.resize((new_w, new_h), Image.Resampling.NEAREST)
    canvas = Image.new("RGBA", (DIGIT_W, DIGIT_H), (0, 0, 0, 255))
    canvas.paste(im, ((DIGIT_W - new_w) // 2, (DIGIT_H - new_h) // 2), im)
    _, _, pixels = pixels_from_rgba(canvas, BG_BLACK, None)
    return pack_gpu(DIGIT_W, DIGIT_H, pixels)


def colon_to_gpu() -> bytes:
    im = Image.new("RGBA", (COLON_W, DIGIT_H), (0, 0, 0, 255))
    draw = ImageDraw.Draw(im)
    dot_w = 10
    dot_h = 10
    cx = COLON_W // 2
    y1 = DIGIT_H // 3 - dot_h // 2
    y2 = DIGIT_H * 2 // 3 - dot_h // 2
    for y in (y1, y2):
        draw.rectangle(
            (cx - dot_w // 2, y, cx + dot_w // 2 - 1, y + dot_h - 1),
            fill=(255, 255, 255, 255),
        )
    _, _, pixels = pixels_from_rgba(im, BG_BLACK, None)
    return pack_gpu(COLON_W, DIGIT_H, pixels)


def write_bin(name: str, data: bytes, src: str) -> None:
    out = BIN_DIR / name
    out.write_bytes(data)
    print(f"{name}: {len(data)} bytes <- {src}")


def require_src(name: str) -> Path:
    path = SRC_DIR / name
    if not path.exists():
        raise SystemExit(f"missing {path}")
    return path


def cleanup_obsolete_bins(keep: set[str]) -> None:
    for path in BIN_DIR.glob("*.bin"):
        if path.name not in keep:
            path.unlink()
            print(f"removed obsolete {path.name}")


def main() -> None:
    ensure_src_layout()
    keep: set[str] = set()
    digit_max_size = 0
    colon_size = 0
    status_sizes: dict[str, tuple[int, int, int]] = {}
    nav_sizes: dict[str, tuple[int, int, int]] = {}
    dash_w = 30
    dash_h = 2
    heat_sizes: dict[str, tuple[int, int, int]] = {}

    for fname, stem in NAV_ICON_ITEMS:
        path = require_src(fname)
        for suffix, bg, out_suffix in (
            ("sel", BG_BLUE, "_sel"),
            ("nor", BG_BLACK, ""),
        ):
            data, w, h = nav_icon_to_gpu(path, bg, FG_WHITE)
            out_name = f"{stem}{out_suffix}.bin"
            write_bin(out_name, data, fname)
            keep.add(out_name)
            if suffix == "nor":
                nav_sizes[stem] = (w, h, len(data))

    dash_w = 30
    dash_h = 2

    for d in range(10):
        fname = f"{d}.png"
        path = require_src(fname)
        data = digit_png_to_gpu(path)
        out_name = f"{d}.bin"
        write_bin(out_name, data, fname)
        keep.add(out_name)
        digit_max_size = max(digit_max_size, len(data))

    colon_data = colon_to_gpu()
    write_bin("colon.bin", colon_data, "(generated)")
    keep.add("colon.bin")
    colon_size = len(colon_data)

    for fname in NATIVE_DIGIT_ITEMS:
        path = require_src(fname)
        data, w, h = png_native_to_gpu(path)
        out_name = Path(fname).stem + ".bin"
        write_bin(out_name, data, fname)
        keep.add(out_name)
        stem = Path(fname).stem
        if stem.endswith("x") and (stem.startswith("w") or stem.startswith("b")):
            heat_sizes[stem] = (w, h, len(data))

    for fname, stem in STATUS_ITEMS:
        path = require_src(fname)
        data, w, h = png_native_to_gpu(path)
        out_name = f"{stem}.bin"
        write_bin(out_name, data, fname)
        keep.add(out_name)
        status_sizes[stem] = (w, h, len(data))

    cleanup_obsolete_bins(keep)

    bt_w, bt_h, _ = status_sizes["bluetooth"]
    lock_w, lock_h, _ = status_sizes["lock"]
    bat_w, bat_h, _ = status_sizes["battery_level"]
    heat_w, heat_h, _ = nav_sizes["heat"]
    mode_w, mode_h, _ = nav_sizes["mode"]
    setup_w, setup_h, _ = nav_sizes["setup"]

    heat_w_digit_max = max((heat_sizes[k][2] for k in heat_sizes if k.startswith("w")), default=4522)
    heat_b_digit_max = max((heat_sizes[k][2] for k in heat_sizes if k.startswith("b") and k != "bhx"), default=4522)
    heat_w_max_h = max((heat_sizes[k][1] for k in heat_sizes if k.startswith("w")), default=61)
    heat_b_max_h = max((heat_sizes[k][1] for k in heat_sizes if k.startswith("b") and k != "bhx"), default=61)

    def heat_macro(stem: str) -> str:
        if stem not in heat_sizes:
            return ""
        w, h, _ = heat_sizes[stem]
        tag = stem.upper()
        return f"#define HEAT_{tag}_W                     {w}\n#define HEAT_{tag}_H                     {h}\n"

    heat_detail = "".join(heat_macro(f"w{d}x") for d in range(10))
    heat_detail += "".join(heat_macro(f"b{d}x") for d in range(10))
    if "wbx" in heat_sizes:
        w, h, sz = heat_sizes["wbx"]
        heat_detail += (
            f"#define HEAT_WBX_W                      {w}\n"
            f"#define HEAT_WBX_H                      {h}\n"
            f"#define HEAT_WBX_RAM_SIZE               {sz}\n"
        )
    if "bhx" in heat_sizes:
        w, h, sz = heat_sizes["bhx"]
        heat_detail += (
            f"#define HEAT_BHX_W                      {w}\n"
            f"#define HEAT_BHX_H                      {h}\n"
            f"#define HEAT_BHX_RAM_SIZE               {sz}\n"
        )

    OUT_H.write_text(
        "\n".join(
            [
                "#ifndef _HOME_ICON_RES_H",
                "#define _HOME_ICON_RES_H",
                "",
                "/* Layout constants generated by tools/gen_home_icons.py */",
                "",
                f"#define HOME_ICON_RAM_W                 {ICON_SIZE}",
                f"#define HOME_ICON_RAM_H                 {ICON_SIZE}",
                f"#define HOME_ICON_RAM_SIZE              (8 + HOME_ICON_RAM_W * HOME_ICON_RAM_H * 2)",
                "",
                f"#define HOME_NAV_HEAT_W                 {heat_w}",
                f"#define HOME_NAV_HEAT_H                 {heat_h}",
                f"#define HOME_NAV_MODE_W                 {mode_w}",
                f"#define HOME_NAV_MODE_H                 {mode_h}",
                f"#define HOME_NAV_SETUP_W                {setup_w}",
                f"#define HOME_NAV_SETUP_H                {setup_h}",
                f"#define HOME_NAV_ICON_MAX_H             {max(heat_h, mode_h, setup_h)}",
                f"#define HOME_NAV_ICON_RAM_MAX_SIZE      {max(heat_w * heat_h, mode_w * mode_h, setup_w * setup_h) * 2 + 8}",
                "",
                f"#define HOME_DASH_RAM_W                 {dash_w}",
                f"#define HOME_DASH_RAM_H                 {dash_h}",
                f"#define HOME_DASH_RAM_SIZE              (8 + HOME_DASH_RAM_W * HOME_DASH_RAM_H * 2)",
                "",
                f"#define HOME_DIGIT_W                    {DIGIT_W}",
                f"#define HOME_DIGIT_H                    {DIGIT_H}",
                f"#define HOME_DIGIT_MAX_W                {DIGIT_W}",
                f"#define HOME_DIGIT_RAM_MAX_SIZE         {digit_max_size}",
                f"#define HOME_DIGIT_GREY_RAM_MAX_SIZE    {digit_max_size}",
                f"#define HOME_DIGIT_GREEN_RAM_MAX_SIZE   {digit_max_size}",
                f"#define HOME_DIGIT_GREEN_T_RAM_MAX_SIZE {digit_max_size // 3}",
                f"#define HOME_COLON_W                    {COLON_W}",
                f"#define HOME_COLON_H                    {DIGIT_H}",
                f"#define HOME_COLON_RAM_SIZE             {colon_size}",
                f"#define HOME_COLON_GREY_RAM_SIZE        {colon_size}",
                "",
                f"#define HOME_TEMPF_W                    65",
                f"#define HOME_TEMPF_H                    65",
                f"#define HOME_TEMPF_RAM_SIZE             8458",
                "",
                f"#define HOME_TEMPF_GR_W                 65",
                f"#define HOME_TEMPF_GR_H                 65",
                f"#define HOME_TEMPF_GR_RAM_SIZE          8458",
                "",
                f"#define HOME_STATUS_TEMP_DIGIT_W        28",
                f"#define HOME_STATUS_TEMP_DIGIT_H        48",
                f"#define HOME_STATUS_TEMPF_W             36",
                f"#define HOME_STATUS_TEMPF_H             36",
                f"#define HOME_STATUS_TEMPF_GR_T_RAM_SIZE 2600",
                "",
                f"#define HOME_STATUS_BT_W                {bt_w}",
                f"#define HOME_STATUS_BT_H                {bt_h}",
                f"#define HOME_STATUS_LOCK_W              {lock_w}",
                f"#define HOME_STATUS_LOCK_H              {lock_h}",
                f"#define HOME_STATUS_BAT_W               {bat_w}",
                f"#define HOME_STATUS_BAT_H               {bat_h}",
                f"#define HOME_STATUS_BT_RAM_SIZE         (8 + HOME_STATUS_BT_W * HOME_STATUS_BT_H * 2)",
                f"#define HOME_STATUS_LOCK_RAM_SIZE       (8 + HOME_STATUS_LOCK_W * HOME_STATUS_LOCK_H * 2)",
                f"#define HOME_STATUS_BAT_RAM_SIZE        (8 + HOME_STATUS_BAT_W * HOME_STATUS_BAT_H * 2)",
                "",
                "#define MODE_STATUS_TEMP_DIGIT_W        HOME_STATUS_TEMP_DIGIT_W",
                "#define MODE_STATUS_TEMP_DIGIT_H        HOME_STATUS_TEMP_DIGIT_H",
                "#define MODE_STATUS_TEMPF_W             HOME_STATUS_TEMPF_W",
                "#define MODE_STATUS_TEMPF_H             HOME_STATUS_TEMPF_H",
                "",
                "/* func_heat.c: w0x=白字时, b0x=灰字分/温度, wbx=冒号, bhx=灰°F */",
                f"#define HEAT_W_DIGIT_MAX_H              {heat_w_max_h}",
                f"#define HEAT_B_DIGIT_MAX_H              {heat_b_max_h}",
                f"#define HEAT_W_DIGIT_RAM_MAX_SIZE       {heat_w_digit_max}",
                f"#define HEAT_B_DIGIT_RAM_MAX_SIZE       {heat_b_digit_max}",
                heat_detail.rstrip(),
                "",
                "#endif",
                "",
            ]
        ),
        encoding="utf-8",
    )

    print(f"written: {OUT_H}")
    print("\nNext: run Output/bin/prebuild.bat, then rebuild and flash ui.bin + app.bin")


if __name__ == "__main__":
    main()
