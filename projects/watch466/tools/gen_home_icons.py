#!/usr/bin/env python3
"""Generate ui/home GPU bins for ui.bin (UI resource flash).

All Home page bitmaps are written to Output/bin/ui/home/*.bin and packed by prebuild.
PNG sources live in res/home/ (kept out of ui/ to avoid duplicate packing).

After running this script, run Output/bin/prebuild.bat to refresh ui.h.
"""

from pathlib import Path
from PIL import Image, ImageDraw
import shutil
import struct

ICON_SIZE = 52
DASH_W = 40
DASH_H = 3
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

ICON_ITEMS = [
    ("HEAT_ONE.png", "heat"),
    ("MODE_ONE.png", "mode"),
    ("SETUP_ONE.png", "setup"),
]

STATUS_ITEMS = [
    ("bluetooth.png", "bluetooth"),
    ("lock.png", "lock"),
    ("battery_level.png", "battery_level"),
]


def rgba565(r, g, b):
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


def png_to_gpu(path: Path, width: int, height: int, bg_rgb, fg_rgb=None) -> bytes:
    im = Image.open(path).convert("RGBA")
    im = im.resize((width, height), Image.Resampling.LANCZOS)
    px = im.load()
    bg = rgba565(*bg_rgb)
    fg = rgba565(*fg_rgb) if fg_rgb else None
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, width, height)
    for y in range(height):
        for x in range(width):
            r, g, b, a = px[x, y]
            if a < 32:
                c = bg
            elif fg is not None:
                c = fg
            else:
                c = rgba565(r, g, b)
            buf += struct.pack("<H", c)
    return bytes(buf)


def png_native_to_gpu(path: Path) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    w, h = im.size
    px = im.load()
    bg = rgba565(*BG_BLACK)
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, w, h)
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            c = bg if a < 32 else rgba565(r, g, b)
            buf += struct.pack("<H", c)
    return bytes(buf), w, h


def digit_png_to_gpu(path: Path) -> bytes:
    im = Image.open(path).convert("RGBA")
    src_w, src_h = im.size
    scale = min(DIGIT_W / src_w, DIGIT_H / src_h)
    new_w = max(1, round(src_w * scale))
    new_h = max(1, round(src_h * scale))
    im = im.resize((new_w, new_h), Image.Resampling.NEAREST)
    canvas = Image.new("RGBA", (DIGIT_W, DIGIT_H), (0, 0, 0, 255))
    canvas.paste(im, ((DIGIT_W - new_w) // 2, (DIGIT_H - new_h) // 2), im)
    px = canvas.load()
    bg = rgba565(*BG_BLACK)
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, DIGIT_W, DIGIT_H)
    for y in range(DIGIT_H):
        for x in range(DIGIT_W):
            r, g, b, a = px[x, y]
            c = bg if a < 32 else rgba565(r, g, b)
            buf += struct.pack("<H", c)
    return bytes(buf)


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
    px = im.load()
    bg = rgba565(*BG_BLACK)
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, COLON_W, DIGIT_H)
    for y in range(DIGIT_H):
        for x in range(COLON_W):
            r, g, b, a = px[x, y]
            c = bg if a < 32 else rgba565(r, g, b)
            buf += struct.pack("<H", c)
    return bytes(buf)


def write_bin(name: str, data: bytes, src: str) -> None:
    out = BIN_DIR / name
    out.write_bytes(data)
    print(f"{name}: {len(data)} bytes <- {src}")


def cleanup_obsolete_bins() -> None:
    keep = set()
    for fname, stem in ICON_ITEMS:
        keep.add(f"{stem}_sel.bin")
        keep.add(f"{stem}_nor.bin")
    keep.update(["dash_sel.bin", "dash_nor.bin", "colon.bin"])
    for d in range(10):
        keep.add(f"{d}.bin")
    for _, stem in STATUS_ITEMS:
        keep.add(f"{stem}.bin")
    for path in BIN_DIR.glob("*.bin"):
        if path.name not in keep:
            path.unlink()
            print(f"removed obsolete {path.name}")


def main():
    ensure_src_layout()
    digit_max_size = 0
    colon_size = 0
    status_sizes = {}

    for fname, stem in ICON_ITEMS:
        path = SRC_DIR / fname
        if not path.exists():
            raise SystemExit(f"missing {path}")
        for suffix, bg in (("sel", BG_BLUE), ("nor", BG_BLACK)):
            data = png_to_gpu(path, ICON_SIZE, ICON_SIZE, bg, FG_WHITE)
            write_bin(f"{stem}_{suffix}.bin", data, fname)

    for src_name, out_name in (
        ("dash_white.png", "dash_sel.bin"),
        ("dash_blue.png", "dash_nor.bin"),
    ):
        path = SRC_DIR / src_name
        if not path.exists():
            raise SystemExit(f"missing {path}")
        data = png_to_gpu(path, DASH_W, DASH_H, BG_BLACK, None)
        write_bin(out_name, data, src_name)

    for d in range(10):
        path = SRC_DIR / f"{d}.png"
        if not path.exists():
            raise SystemExit(f"missing {path}")
        data = digit_png_to_gpu(path)
        write_bin(f"{d}.bin", data, f"{d}.png")
        digit_max_size = max(digit_max_size, len(data))

    colon_data = colon_to_gpu()
    write_bin("colon.bin", colon_data, "(generated)")
    colon_size = len(colon_data)

    for fname, stem in STATUS_ITEMS:
        path = SRC_DIR / fname
        if not path.exists():
            raise SystemExit(f"missing {path}")
        data, sw, sh = png_native_to_gpu(path)
        write_bin(f"{stem}.bin", data, fname)
        status_sizes[stem] = (sw, sh, len(data))

    bt_w, bt_h, _ = status_sizes["bluetooth"]
    lock_w, lock_h, _ = status_sizes["lock"]
    bat_w, bat_h, _ = status_sizes["battery_level"]

    OUT_H.write_text(
        "#ifndef _HOME_ICON_RES_H\n"
        "#define _HOME_ICON_RES_H\n\n"
        "/* Layout constants generated by tools/gen_home_icons.py */\n\n"
        f"#define HOME_ICON_RAM_W                 {ICON_SIZE}\n"
        f"#define HOME_ICON_RAM_H                 {ICON_SIZE}\n"
        f"#define HOME_ICON_RAM_SIZE              (8 + HOME_ICON_RAM_W * HOME_ICON_RAM_H * 2)\n\n"
        f"#define HOME_DASH_RAM_W                 {DASH_W}\n"
        f"#define HOME_DASH_RAM_H                 {DASH_H}\n"
        f"#define HOME_DASH_RAM_SIZE              (8 + HOME_DASH_RAM_W * HOME_DASH_RAM_H * 2)\n\n"
        f"#define HOME_DIGIT_W                    {DIGIT_W}\n"
        f"#define HOME_DIGIT_H                    {DIGIT_H}\n"
        f"#define HOME_DIGIT_MAX_W                {DIGIT_W}\n"
        f"#define HOME_DIGIT_RAM_MAX_SIZE         {digit_max_size}\n"
        f"#define HOME_COLON_W                    {COLON_W}\n"
        f"#define HOME_COLON_H                    {DIGIT_H}\n"
        f"#define HOME_COLON_RAM_SIZE             {colon_size}\n\n"
        f"#define HOME_STATUS_BT_W                {bt_w}\n"
        f"#define HOME_STATUS_BT_H                {bt_h}\n"
        f"#define HOME_STATUS_LOCK_W              {lock_w}\n"
        f"#define HOME_STATUS_LOCK_H              {lock_h}\n"
        f"#define HOME_STATUS_BAT_W               {bat_w}\n"
        f"#define HOME_STATUS_BAT_H               {bat_h}\n"
        f"#define HOME_STATUS_BT_RAM_SIZE         (8 + HOME_STATUS_BT_W * HOME_STATUS_BT_H * 2)\n"
        f"#define HOME_STATUS_LOCK_RAM_SIZE       (8 + HOME_STATUS_LOCK_W * HOME_STATUS_LOCK_H * 2)\n"
        f"#define HOME_STATUS_BAT_RAM_SIZE        (8 + HOME_STATUS_BAT_W * HOME_STATUS_BAT_H * 2)\n\n"
        "#endif\n",
        encoding="utf-8",
    )
    print("written:", OUT_H)
    cleanup_obsolete_bins()
    print("\nNext: run Output/bin/prebuild.bat, then rebuild and flash ui.bin + app.bin")


if __name__ == "__main__":
    main()
