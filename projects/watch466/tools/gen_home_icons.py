#!/usr/bin/env python3
"""Generate ui/home GPU bins for ui.bin (UI resource flash).

All Home/Heat page bitmaps are written to Output/bin/ui/home/*.bin and packed by prebuild.
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
STATUS_TEMP_DIGIT_W = 28
STATUS_TEMP_DIGIT_H = 48
STATUS_TEMPF_W = 36
STATUS_TEMPF_H = 36
FG_GREY = (140, 140, 140)
FG_GREEN = (0, 220, 80)
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

MODE_TAB_ITEMS = [
    ("Pasta.png", "pasta"),
    ("Chicken.png", "chicken"),
    ("Warm.png", "warm"),
]

STATUS_ITEMS = [
    ("bluetooth.png", "bluetooth"),
    ("lock.png", "lock"),
    ("battery_level.png", "battery_level"),
]

SYMBOL_ITEMS = [
    ("Vector.png", "degf_w"),
    ("Vector-1.png", "degf_g"),
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


def digit_png_to_gpu_tint_sized(path: Path, fg_rgb, width: int, height: int) -> bytes:
    im = Image.open(path).convert("RGBA")
    src_w, src_h = im.size
    scale = min(width / src_w, height / src_h)
    new_w = max(1, round(src_w * scale))
    new_h = max(1, round(src_h * scale))
    im = im.resize((new_w, new_h), Image.Resampling.NEAREST)
    canvas = Image.new("RGBA", (width, height), (0, 0, 0, 255))
    canvas.paste(im, ((width - new_w) // 2, (height - new_h) // 2), im)
    px = canvas.load()
    bg = rgba565(*BG_BLACK)
    fg = rgba565(*fg_rgb)
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, width, height)
    for y in range(height):
        for x in range(width):
            r, g, b, a = px[x, y]
            c = bg if a < 32 else fg
            buf += struct.pack("<H", c)
    return bytes(buf)


def symbol_png_to_gpu_tint_sized(path: Path, fg_rgb, width: int, height: int) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    im = im.resize((width, height), Image.Resampling.LANCZOS)
    px = im.load()
    bg = rgba565(*BG_BLACK)
    fg = rgba565(*fg_rgb)
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, width, height)
    for y in range(height):
        for x in range(width):
            r, g, b, a = px[x, y]
            c = bg if a < 32 else fg
            buf += struct.pack("<H", c)
    return bytes(buf), width, height


def digit_png_to_gpu_tint(path: Path, fg_rgb) -> bytes:
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
    fg = rgba565(*fg_rgb)
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, DIGIT_W, DIGIT_H)
    for y in range(DIGIT_H):
        for x in range(DIGIT_W):
            r, g, b, a = px[x, y]
            c = bg if a < 32 else fg
            buf += struct.pack("<H", c)
    return bytes(buf)


def symbol_png_to_gpu_tint(path: Path, fg_rgb) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    w, h = im.size
    px = im.load()
    bg = rgba565(*BG_BLACK)
    fg = rgba565(*fg_rgb)
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, w, h)
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            c = bg if a < 32 else fg
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


def colon_to_gpu(fg_rgb) -> bytes:
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
            fill=(*fg_rgb, 255),
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
    for _, stem in MODE_TAB_ITEMS:
        keep.add(f"{stem}_sel.bin")
        keep.add(f"{stem}_nor.bin")
    keep.update(["dash_sel.bin", "dash_nor.bin", "colon.bin", "colon_g.bin", "degf_gr.bin", "degf_gr_t.bin"])
    for d in range(10):
        keep.add(f"{d}.bin")
        keep.add(f"{d}_g.bin")
        keep.add(f"{d}_gr.bin")
        keep.add(f"{d}_gr_t.bin")
    for _, stem in STATUS_ITEMS:
        keep.add(f"{stem}.bin")
    for _, stem in SYMBOL_ITEMS:
        keep.add(f"{stem}.bin")
    for path in BIN_DIR.glob("*.bin"):
        if path.name not in keep:
            path.unlink()
            print(f"removed obsolete {path.name}")


def main():
    ensure_src_layout()
    digit_max_size = 0
    digit_grey_max_size = 0
    digit_green_max_size = 0
    digit_green_t_max_size = 0
    colon_size = 0
    colon_grey_size = 0
    status_sizes = {}
    symbol_sizes = {}

    for fname, stem in ICON_ITEMS:
        path = SRC_DIR / fname
        if not path.exists():
            raise SystemExit(f"missing {path}")
        for suffix, bg in (("sel", BG_BLUE), ("nor", BG_BLACK)):
            data = png_to_gpu(path, ICON_SIZE, ICON_SIZE, bg, FG_WHITE)
            write_bin(f"{stem}_{suffix}.bin", data, fname)

    for fname, stem in MODE_TAB_ITEMS:
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

        path_g = SRC_DIR / f"{d}-1.png"
        if not path_g.exists():
            raise SystemExit(f"missing grey digit {path_g}")
        data_g = digit_png_to_gpu(path_g)
        write_bin(f"{d}_g.bin", data_g, f"{d}-1.png")
        digit_grey_max_size = max(digit_grey_max_size, len(data_g))

        data_gr = digit_png_to_gpu_tint(path, FG_GREEN)
        write_bin(f"{d}_gr.bin", data_gr, f"{d}.png (green)")
        digit_green_max_size = max(digit_green_max_size, len(data_gr))

        data_gr_t = digit_png_to_gpu_tint_sized(path, FG_GREEN, STATUS_TEMP_DIGIT_W, STATUS_TEMP_DIGIT_H)
        write_bin(f"{d}_gr_t.bin", data_gr_t, f"{d}.png (green small)")
        digit_green_t_max_size = max(digit_green_t_max_size, len(data_gr_t))

    colon_data = colon_to_gpu(FG_WHITE)
    write_bin("colon.bin", colon_data, "(generated white)")
    colon_size = len(colon_data)

    colon_grey_data = colon_to_gpu(FG_GREY)
    write_bin("colon_g.bin", colon_grey_data, "(generated grey)")
    colon_grey_size = len(colon_grey_data)

    for fname, stem in STATUS_ITEMS:
        path = SRC_DIR / fname
        if not path.exists():
            raise SystemExit(f"missing {path}")
        data, sw, sh = png_native_to_gpu(path)
        write_bin(f"{stem}.bin", data, fname)
        status_sizes[stem] = (sw, sh, len(data))

    for fname, stem in SYMBOL_ITEMS:
        path = SRC_DIR / fname
        if not path.exists():
            raise SystemExit(f"missing {path}")
        data, sw, sh = png_native_to_gpu(path)
        write_bin(f"{stem}.bin", data, fname)
        symbol_sizes[stem] = (sw, sh, len(data))

    vector_path = SRC_DIR / "Vector.png"
    if not vector_path.exists():
        raise SystemExit(f"missing {vector_path}")
    degf_gr_data, degf_gr_w, degf_gr_h = symbol_png_to_gpu_tint(vector_path, FG_GREEN)
    write_bin("degf_gr.bin", degf_gr_data, "Vector.png (green)")
    symbol_sizes["degf_gr"] = (degf_gr_w, degf_gr_h, len(degf_gr_data))

    degf_gr_t_data, degf_gr_t_w, degf_gr_t_h = symbol_png_to_gpu_tint_sized(
        vector_path, FG_GREEN, STATUS_TEMPF_W, STATUS_TEMPF_H)
    write_bin("degf_gr_t.bin", degf_gr_t_data, "Vector.png (green small)")
    symbol_sizes["degf_gr_t"] = (degf_gr_t_w, degf_gr_t_h, len(degf_gr_t_data))

    bt_w, bt_h, _ = status_sizes["bluetooth"]
    lock_w, lock_h, _ = status_sizes["lock"]
    bat_w, bat_h, _ = status_sizes["battery_level"]
    degf_g_w, degf_g_h, degf_g_size = symbol_sizes["degf_g"]
    degf_gr_w, degf_gr_h, degf_gr_size = symbol_sizes["degf_gr"]
    degf_gr_t_w, degf_gr_t_h, degf_gr_t_size = symbol_sizes["degf_gr_t"]

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
        f"#define HOME_DIGIT_GREY_RAM_MAX_SIZE    {digit_grey_max_size}\n"
        f"#define HOME_DIGIT_GREEN_RAM_MAX_SIZE   {digit_green_max_size}\n"
        f"#define HOME_DIGIT_GREEN_T_RAM_MAX_SIZE {digit_green_t_max_size}\n"
        f"#define HOME_COLON_W                    {COLON_W}\n"
        f"#define HOME_COLON_H                    {DIGIT_H}\n"
        f"#define HOME_COLON_RAM_SIZE             {colon_size}\n"
        f"#define HOME_COLON_GREY_RAM_SIZE        {colon_grey_size}\n\n"
        f"#define HOME_TEMPF_W                    {degf_g_w}\n"
        f"#define HOME_TEMPF_H                    {degf_g_h}\n"
        f"#define HOME_TEMPF_RAM_SIZE             {degf_g_size}\n\n"
        f"#define HOME_TEMPF_GR_W                 {degf_gr_w}\n"
        f"#define HOME_TEMPF_GR_H                 {degf_gr_h}\n"
        f"#define HOME_TEMPF_GR_RAM_SIZE          {degf_gr_size}\n\n"
        f"#define HOME_STATUS_TEMP_DIGIT_W        {STATUS_TEMP_DIGIT_W}\n"
        f"#define HOME_STATUS_TEMP_DIGIT_H        {STATUS_TEMP_DIGIT_H}\n"
        f"#define HOME_STATUS_TEMPF_W             {STATUS_TEMPF_W}\n"
        f"#define HOME_STATUS_TEMPF_H             {STATUS_TEMPF_H}\n"
        f"#define HOME_STATUS_TEMPF_GR_T_RAM_SIZE {degf_gr_t_size}\n\n"
        f"#define MODE_STATUS_TEMP_DIGIT_W        HOME_STATUS_TEMP_DIGIT_W\n"
        f"#define MODE_STATUS_TEMP_DIGIT_H        HOME_STATUS_TEMP_DIGIT_H\n"
        f"#define MODE_STATUS_TEMPF_W             HOME_STATUS_TEMPF_W\n"
        f"#define MODE_STATUS_TEMPF_H             HOME_STATUS_TEMPF_H\n\n"
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
    for png in BIN_DIR.glob("*.png"):
        png.unlink()
        print(f"removed {png.name} from ui/home (use .bin only for prebuild)")
    print("\nNext: run Output/bin/prebuild.bat, then rebuild and flash ui.bin + app.bin")


if __name__ == "__main__":
    main()
