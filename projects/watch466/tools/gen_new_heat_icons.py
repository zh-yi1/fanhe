#!/usr/bin/env python3
"""Generate heat-page GPU bins from Output/bin/ui/new_ui/*.png.

Progress icons: full-screen PNGs are split into
  - new_progress_bg.bin   gray arc track (once, ~42KB)
  - new_progress_1..13.bin blue fill only, cropped (~1-5KB early frames)

Run: python tools/gen_new_heat_icons.py
     Output/bin/prebuild.bat
"""

from __future__ import annotations

import struct
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
BIN_DIR = ROOT / "Output" / "bin" / "ui" / "new_ui"
OUT_H = ROOT / "functions" / "new_heat_res.h"

BG_WHITE = (255, 255, 255)
WHITE565 = 0xFFFF
PROGRESS_FULL_W = 300

HEAT_ITEMS = (
    *(f"new_temp_{i}.png" for i in range(1, 6)),
    *(f"new_time_{i}.png" for i in range(1, 14)),
    *(f"new_progress_{i}.png" for i in range(1, 14)),
    "new_point.png",
    "new_blue_time.png",
    "new_white_time.png",
    "new_show.png",
)


def rgba565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def pack_gpu(w: int, h: int, pixels: list[int]) -> bytes:
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, w, h)
    for c in pixels:
        buf += struct.pack("<H", c)
    return bytes(buf)


def pixels_from_rgba(im: Image.Image, bg_rgb: tuple[int, int, int]) -> tuple[int, int, list[int]]:
    w, h = im.size
    px = im.load()
    br, bg, bb = bg_rgb
    out: list[int] = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 32:
                out.append(rgba565(br, bg, bb))
            else:
                blend = a / 255.0
                inv = 1.0 - blend
                rr = int(r * blend + br * inv + 0.5)
                gg = int(g * blend + bg * inv + 0.5)
                bb2 = int(b * blend + bb * inv + 0.5)
                out.append(rgba565(rr, gg, bb2))
    return w, h, out


def png_to_gpu(path: Path) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    w, h, pixels = pixels_from_rgba(im, BG_WHITE)
    return pack_gpu(w, h, pixels), w, h


def load_gpu_bin(path: Path) -> tuple[int, int, list[int]]:
    data = path.read_bytes()
    if len(data) < 8:
        raise ValueError(f"bad gpu bin: {path}")
    _, w, h = struct.unpack("<IHH", data[:8])
    count = w * h
    pixels = list(struct.unpack("<" + "H" * count, data[8 : 8 + count * 2]))
    return w, h, pixels


def save_gpu_bin(path: Path, w: int, h: int, pixels: list[int]) -> int:
    path.write_bytes(pack_gpu(w, h, pixels))
    return 8 + w * h * 2


def is_white(c: int) -> bool:
    return c >= 0xFFFE


def is_progress_blue(c: int) -> bool:
    if is_white(c):
        return False
    r = (c >> 11) & 0x1F
    g = (c >> 5) & 0x3F
    b = c & 0x1F
    return b >= 10 and b > r and g < 42


def is_track_gray(c: int) -> bool:
    if is_white(c):
        return False
    r = (c >> 11) & 0x1F
    g = (c >> 5) & 0x3F
    b = c & 0x1F
    return r >= 24 and g >= 56 and b >= 28 and not is_progress_blue(c)


def content_bbox(w: int, h: int, pixels: list[int], pred) -> tuple[int, int, int, int] | None:
    min_x, min_y = w, h
    max_x, max_y = -1, -1
    for y in range(h):
        for x in range(w):
            if pred(pixels[y * w + x]):
                min_x = min(min_x, x)
                min_y = min(min_y, y)
                max_x = max(max_x, x)
                max_y = max(max_y, y)
    if max_x < 0:
        return None
    pad = 1
    min_x = max(0, min_x - pad)
    min_y = max(0, min_y - pad)
    max_x = min(w - 1, max_x + pad)
    max_y = min(h - 1, max_y + pad)
    return min_x, min_y, max_x - min_x + 1, max_y - min_y + 1


def crop_pixels(fw: int, fh: int, pixels: list[int], x: int, y: int, cw: int, ch: int) -> list[int]:
    out: list[int] = []
    for row in range(ch):
        for col in range(cw):
            sx, sy = x + col, y + row
            if 0 <= sx < fw and 0 <= sy < fh:
                out.append(pixels[sy * fw + sx])
            else:
                out.append(WHITE565)
    return out


class ProgressPack:
    def __init__(self, w: int, h: int, anchor_x: int, anchor_y: int, ram_size: int):
        self.w = w
        self.h = h
        self.anchor_x = anchor_x
        self.anchor_y = anchor_y
        self.ram_size = ram_size


def repack_progress_icons() -> tuple[ProgressPack, list[ProgressPack]] | None:
    src_path = BIN_DIR / "new_progress_1.bin"
    if not src_path.exists():
        src_path = BIN_DIR / "new_progress_1.png"
        if not src_path.exists():
            return None
        data, fw, fh = png_to_gpu(src_path)
        src_path = BIN_DIR / "new_progress_1.bin"
        src_path.write_bytes(data)
    else:
        fw, fh, _ = load_gpu_bin(src_path)

    if fw < PROGRESS_FULL_W:
        print(f"progress already compact ({fw}x{fh}), skip repack")
        return None

    track_fw, track_fh, track_src = load_gpu_bin(BIN_DIR / "new_progress_1.bin")
    track_px = [p if is_track_gray(p) else WHITE565 for p in track_src]
    track_box = content_bbox(track_fw, track_fh, track_px, is_track_gray)
    if track_box is None:
        print("WARN: no gray track in new_progress_1, skip repack")
        return None

    tx, ty, tw, th = track_box
    track_crop = crop_pixels(track_fw, track_fh, track_px, tx, ty, tw, th)
    bg_size = save_gpu_bin(BIN_DIR / "new_progress_bg.bin", tw, th, track_crop)
    bg_pack = ProgressPack(tw, th, tx + tw // 2, ty + th // 2, bg_size)
    print(f"new_progress_bg.bin: {bg_size} bytes ({tw}x{th}) anchor=({bg_pack.anchor_x},{bg_pack.anchor_y})")

    overlays: list[ProgressPack] = []
    for i in range(1, 14):
        bin_path = BIN_DIR / f"new_progress_{i}.bin"
        if not bin_path.exists():
            png_path = BIN_DIR / f"new_progress_{i}.png"
            if png_path.exists():
                data, _, _ = png_to_gpu(png_path)
                bin_path.write_bytes(data)
            else:
                overlays.append(ProgressPack(0, 0, 0, 0, 0))
                continue

        pw, ph, src = load_gpu_bin(bin_path)
        blue_px = [p if is_progress_blue(p) else WHITE565 for p in src]
        box = content_bbox(pw, ph, blue_px, is_progress_blue)
        if box is None:
            overlays.append(ProgressPack(0, 0, 0, 0, 0))
            out_path = BIN_DIR / f"new_progress_{i}.bin"
            save_gpu_bin(out_path, 1, 1, [WHITE565])
            print(f"new_progress_{i}.bin: empty overlay")
            continue

        ox, oy, ow, oh = box
        crop = crop_pixels(pw, ph, blue_px, ox, oy, ow, oh)
        out_path = BIN_DIR / f"new_progress_{i}.bin"
        sz = save_gpu_bin(out_path, ow, oh, crop)
        pack = ProgressPack(ow, oh, ox + ow // 2, oy + oh // 2, sz)
        overlays.append(pack)
        print(f"new_progress_{i}.bin: {sz} bytes ({ow}x{oh}) anchor=({pack.anchor_x},{pack.anchor_y})")

    return bg_pack, overlays


def stem_from_png(name: str) -> str:
    return Path(name).stem


def macro_prefix(stem: str) -> str:
    return "NEW_HEAT_" + stem.upper().replace(".", "_")


def load_existing_define_lines(path: Path) -> dict[str, str]:
    defines: dict[str, str] = {}

    if not path.exists():
        return defines
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line.startswith("#define "):
            continue
        parts = line.split()
        if len(parts) >= 3:
            defines[parts[1]] = parts[2]
    return defines


def parse_item_size(defines: dict[str, str], stem: str) -> tuple[int, int, int] | None:
    tag = macro_prefix(stem)
    w_key = f"{tag}_W"
    h_key = f"{tag}_H"
    sz_key = f"{tag}_RAM_SIZE"
    if w_key not in defines or h_key not in defines or sz_key not in defines:
        return None
    return int(defines[w_key]), int(defines[h_key]), int(defines[sz_key])


def main() -> None:
    existing = load_existing_define_lines(OUT_H)
    BIN_DIR.mkdir(parents=True, exist_ok=True)
    sizes: dict[str, tuple[int, int, int]] = {}
    progress_anchors: dict[str, tuple[int, int]] = {}
    track_max = 0
    badge_max = 0
    point_sz = 0
    progress_max = 0
    overlay_max = 0
    show_sz = 0
    progress_bg: ProgressPack | None = None
    progress_overlays: list[ProgressPack] | None = None

    for png_name in HEAT_ITEMS:
        src = BIN_DIR / png_name
        stem = stem_from_png(png_name)
        if not src.exists():
            cached = parse_item_size(existing, stem)
            if cached is not None:
                sizes[stem] = cached
                print(f"KEEP: {stem} from {OUT_H.name}")
            else:
                print(f"SKIP: {png_name} not found")
            continue
        data, w, h = png_to_gpu(src)
        stem = stem_from_png(png_name)
        out = BIN_DIR / f"{stem}.bin"
        out.write_bytes(data)
        sizes[stem] = (w, h, len(data))
        print(f"{stem}.bin: {len(data)} bytes ({w}x{h}) <- {png_name}")
        if stem.startswith("new_temp_") or stem.startswith("new_time_"):
            track_max = max(track_max, len(data))
        elif stem in ("new_blue_time", "new_white_time"):
            badge_max = max(badge_max, len(data))
        elif stem == "new_point":
            point_sz = len(data)
        elif stem.startswith("new_progress_"):
            progress_max = max(progress_max, len(data))
        elif stem == "new_show":
            show_sz = len(data)

    overlay_max = 0
    repacked = repack_progress_icons()
    if repacked is not None:
        progress_bg, progress_overlays = repacked
        sizes["new_progress_bg"] = (
            progress_bg.w,
            progress_bg.h,
            progress_bg.ram_size,
        )
        progress_anchors["new_progress_bg"] = (
            progress_bg.anchor_x,
            progress_bg.anchor_y,
        )
        # 裁剪后的 progress 尺寸才是真实 RAM 需求；全屏 320x240 临时 bin 不参与统计
        progress_max = progress_bg.ram_size
        overlay_max = 0
        for i, pack in enumerate(progress_overlays, start=1):
            stem = f"new_progress_{i}"
            sizes[stem] = (pack.w, pack.h, pack.ram_size)
            progress_anchors[stem] = (pack.anchor_x, pack.anchor_y)
            progress_max = max(progress_max, pack.ram_size)
            overlay_max = max(overlay_max, pack.ram_size)

    for stem, (_, _, sz) in sizes.items():
        if stem.startswith("new_temp_") or stem.startswith("new_time_"):
            track_max = max(track_max, sz)
        elif stem in ("new_blue_time", "new_white_time"):
            badge_max = max(badge_max, sz)
        elif stem == "new_point":
            point_sz = sz
        elif stem == "new_progress_bg":
            progress_max = max(progress_max, sz)
        elif stem == "new_show":
            show_sz = sz

    if not sizes:
        print("No heat PNGs found.")
        return

    lines = [
        "#ifndef _NEW_HEAT_RES_H",
        "#define _NEW_HEAT_RES_H",
        "",
        "/* Generated by tools/gen_new_heat_icons.py */",
        "",
        f"#define NEW_HEAT_TRACK_RAM_SIZE           {track_max or int(existing.get('NEW_HEAT_TRACK_RAM_SIZE', '3536'))}",
        f"#define NEW_HEAT_BADGE_RAM_SIZE           {badge_max or int(existing.get('NEW_HEAT_BADGE_RAM_SIZE', '2978'))}",
        f"#define NEW_HEAT_POINT_RAM_SIZE           {point_sz or int(existing.get('NEW_HEAT_POINT_RAM_SIZE', '1466'))}",
        "",
        "#define NEW_HEAT_PROGRESS_CNT                 13",
        f"#define NEW_HEAT_PROGRESS_RAM_SIZE            {progress_max or int(existing.get('NEW_HEAT_PROGRESS_RAM_SIZE', '44416'))}",
        f"#define NEW_HEAT_PROGRESS_OVERLAY_RAM_SIZE    {overlay_max or int(existing.get('NEW_HEAT_PROGRESS_OVERLAY_RAM_SIZE', '43568'))}",
        f"#define NEW_HEAT_SHOW_RAM_SIZE                {show_sz or int(existing.get('NEW_HEAT_SHOW_RAM_SIZE', '23696'))}",
        "",
        "#define NEW_HEAT_TEMP_CNT                 5",
        "#define NEW_HEAT_TIME_CNT                 13",
        "",
    ]

    for stem, (w, h, sz) in sorted(sizes.items()):
        tag = macro_prefix(stem)
        lines.append(f"#define {tag}_W                 {w}")
        lines.append(f"#define {tag}_H                 {h}")
        lines.append(f"#define {tag}_RAM_SIZE          {sz}")
        if stem in progress_anchors:
            ax, ay = progress_anchors[stem]
            lines.append(f"#define {tag}_ANCHOR_X            {ax}")
            lines.append(f"#define {tag}_ANCHOR_Y            {ay}")
        lines.append("")

    if "new_point" in sizes:
        w, h, _ = sizes["new_point"]
        lines.append(f"#define NEW_HEAT_POINT_W                  {w}")
        lines.append(f"#define NEW_HEAT_POINT_H                  {h}")
        lines.append("")
    elif "NEW_HEAT_POINT_W" in existing:
        lines.append(f"#define NEW_HEAT_POINT_W                  {existing['NEW_HEAT_POINT_W']}")
        lines.append(f"#define NEW_HEAT_POINT_H                  {existing['NEW_HEAT_POINT_H']}")
        lines.append("")

    if "new_blue_time" in sizes:
        w, h, _ = sizes["new_blue_time"]
        lines.append(f"#define NEW_HEAT_BADGE_W                  {w}")
        lines.append(f"#define NEW_HEAT_BADGE_H                  {h}")
        lines.append("")
    elif "NEW_HEAT_BADGE_W" in existing:
        lines.append(f"#define NEW_HEAT_BADGE_W                  {existing['NEW_HEAT_BADGE_W']}")
        lines.append(f"#define NEW_HEAT_BADGE_H                  {existing['NEW_HEAT_BADGE_H']}")
        lines.append("")

    track_w = max(
        (sizes[k][0] for k in sizes if k.startswith("new_temp_") or k.startswith("new_time_")),
        default=int(existing.get("NEW_HEAT_SLIDER_W", "252")),
    )
    lines.append(f"#define NEW_HEAT_SLIDER_W                 {track_w}")
    lines.append("")
    lines.append("#endif")
    lines.append("")

    OUT_H.write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {OUT_H}")
    print("Next: run Output/bin/prebuild.bat")


if __name__ == "__main__":
    main()
