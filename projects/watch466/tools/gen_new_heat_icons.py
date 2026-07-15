#!/usr/bin/env python3
"""Generate heat-page GPU bins from Output/bin/ui/new_ui/*.png.

Progress icons: full-screen PNGs are split into
  - new_progress_bg.bin   gray arc track (once, ~42KB)
  - new_progress_1..13.bin blue fill only, cropped (~1-5KB early frames)

Run: python tools/gen_new_heat_icons.py
     Output/bin/prebuild.bat
"""

from __future__ import annotations

import math
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
    *(f"new_temp_{i}.png" for i in range(1, 8)),
    *(f"new_time_{i}.png" for i in range(1, 14)),
    *(f"new_progress_{i}.png" for i in range(1, 14)),
    "new_point.png",
    "new_blue_time.png",
    "new_white_time.png",
    "new_show.png",
)


def rgba565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


POINT_BLUE565 = rgba565(68, 95, 255)
POINT_SHADOW565 = rgba565(190, 190, 190)


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


def point_to_gpu() -> tuple[bytes, int, int]:
    """27x27 占位 bin（ui 打包用）；实际圆点由 new_heat_point_util 运行时 4x4 超采样绘制。"""
    size = 27
    cx = cy = size / 2.0
    blue_r = 6.1
    ring_outer = 10.3
    ss = 4
    pixels: list[int] = []
    for y in range(size):
        for x in range(size):
            sum_r = sum_g = sum_b = 0
            for si in range(ss):
                for sj in range(ss):
                    px = x + (sj + 0.5) / ss
                    py = y + (si + 0.5) / ss
                    dist = math.hypot(px - cx, py - cy)
                    if dist <= blue_r:
                        c = POINT_BLUE565
                    elif dist <= ring_outer:
                        c = WHITE565
                    else:
                        c = WHITE565
                    sum_r += (c >> 11) & 0x1F
                    sum_g += (c >> 5) & 0x3F
                    sum_b += c & 0x1F
            r = sum_r // 16
            g = sum_g // 16
            b = sum_b // 16
            pixels.append(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F))
    return pack_gpu(size, size, pixels), size, size


def track_tips_from_bin(path: Path) -> tuple[int, int] | None:
    if not path.exists():
        return None
    w, h, pixels = load_gpu_bin(path)
    filled: list[tuple[int, int]] = []
    for y in range(h):
        for x in range(w):
            if pixels[y * w + x] != WHITE565:
                filled.append((x, y))
    if not filled:
        return None
    left_x = min(x for x, _ in filled)
    right_x = max(x for x, _ in filled)
    return left_x, right_x


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
    def __init__(self, w: int, h: int, anchor_x: int, anchor_y: int, ram_size: int,
                 tip_x: int = 0, tip_y: int = 0):
        self.w = w
        self.h = h
        self.anchor_x = anchor_x
        self.anchor_y = anchor_y
        self.ram_size = ram_size
        self.tip_x = tip_x
        self.tip_y = tip_y


def arc_tip_countdown(blues: list[tuple[int, int]], cx: int, cy: int, idx: int) -> tuple[int, int]:
    """倒计时圆点：近满弧(idx>=11)取左侧蓝灰交界最下缘，其余取 CCW 增长端。"""
    if not blues:
        return 0, 0
    if idx >= 11:
        blue_set = set(blues)
        left_max_x = cx - 84
        best = None
        for x, y in blues:
            if x > left_max_x:
                continue
            boundary = False
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                if (x + dx, y + dy) not in blue_set:
                    boundary = True
                    break
            if not boundary:
                continue
            if best is None or y > best[1] or (y == best[1] and x < best[0]):
                best = (x, y)
        if best is not None:
            return best
    return arc_tip_growth_end(blues, cx, cy, full_circle=(idx == 13))


def composite_progress_blue(idx: int, bg_pack: ProgressPack,
                          overlays: list[ProgressPack]) -> list[tuple[int, int]]:
    """合成灰轨+蓝弧，返回全屏坐标蓝像素列表。"""
    bw, bh, bg_px = load_gpu_bin(BIN_DIR / "new_progress_bg.bin")
    bg_ax, bg_ay = bg_pack.anchor_x, bg_pack.anchor_y
    comp = bg_px[:]
    if idx <= 1:
        return []
    pack = overlays[idx - 1]
    if pack.w == 0 or pack.h == 0:
        return []
    ow, oh, ov = load_gpu_bin(BIN_DIR / f"new_progress_{idx}.bin")
    dx0 = (pack.anchor_x - ow // 2) - (bg_ax - bw // 2)
    dy0 = (pack.anchor_y - oh // 2) - (bg_ay - bh // 2)
    ox, oy = bg_ax - bw // 2, bg_ay - bh // 2
    for y in range(oh):
        for x in range(ow):
            c = ov[y * ow + x]
            if c == WHITE565:
                continue
            dx, dy = dx0 + x, dy0 + y
            if 0 <= dx < bw and 0 <= dy < bh:
                comp[dy * bw + dx] = c
    return [(ox + x, oy + y) for y in range(bh) for x in range(bw)
            if is_progress_blue(comp[y * bw + x])]


def arc_tip_growth_end(blues: list[tuple[int, int]], cx: int, cy: int,
                       full_circle: bool = False) -> tuple[int, int]:
    """蓝弧增长端：沿 CCW 扩展的最前沿外缘点；满圈帧取外缘最大极角。"""
    if not blues:
        return 0, 0
    if full_circle:
        dists = [((p[0] - cx) ** 2 + (p[1] - cy) ** 2) for p in blues]
        md = max(dists)
        threshold = md * 88 // 100
        outer = [p for p, d in zip(blues, dists) if d >= threshold]
        if not outer:
            outer = blues
        return max(outer, key=lambda p: math.atan2(p[1] - cy, p[0] - cx))

    angs = [(p, math.atan2(p[1] - cy, p[0] - cx)) for p in blues]
    min_a = min(a for _, a in angs)
    band = 0.12
    cand = [p for p, a in angs if a <= min_a + band]
    if not cand:
        cand = [p for p, _ in angs]
    return max(cand, key=lambda p: (p[0] - cx) ** 2 + (p[1] - cy) ** 2)


def arc_tip_from_blue(pw: int, ph: int, pixels: list[int], cx: int, cy: int,
                      full_circle: bool = False) -> tuple[int, int]:
    blues = [
        (x, y)
        for y in range(ph)
        for x in range(pw)
        if is_progress_blue(pixels[y * pw + x])
    ]
    return arc_tip_growth_end(blues, cx, cy, full_circle)


def tip_from_cropped_bin(ow: int, oh: int, pixels: list[int],
                         arc_cx: int, arc_cy: int,
                         anchor_x: int, anchor_y: int,
                         full_circle: bool = False) -> tuple[int, int]:
    ox = anchor_x - ow // 2
    oy = anchor_y - oh // 2
    blues = [
        (x + ox, y + oy)
        for y in range(oh)
        for x in range(ow)
        if is_progress_blue(pixels[y * ow + x])
    ]
    tip = arc_tip_growth_end(blues, arc_cx, arc_cy, full_circle)
    if tip == (0, 0):
        return anchor_x, anchor_y
    return tip


def arc_start_tip_from_bg(bw: int, bh: int, pixels: list[int],
                          arc_cx: int, arc_cy: int,
                          anchor_x: int, anchor_y: int) -> tuple[int, int]:
    """初始圆点位置：灰色轨道外缘最低点（屏幕 Y 最大）。"""
    ox = anchor_x - bw // 2
    oy = anchor_y - bh // 2
    grays = [
        (x + ox, y + oy)
        for y in range(bh)
        for x in range(bw)
        if is_track_gray(pixels[y * bw + x])
    ]
    if not grays:
        return anchor_x, anchor_y + bh // 2
    dists = [((p[0] - arc_cx) ** 2 + (p[1] - arc_cy) ** 2) for p in grays]
    md = max(dists)
    threshold = md * 92 // 100
    outer = [p for p, d in zip(grays, dists) if d >= threshold]
    if not outer:
        outer = grays
    return max(outer, key=lambda p: p[1])


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
    arc_cx, arc_cy = bg_pack.anchor_x, bg_pack.anchor_y
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
        tip_x, tip_y = arc_tip_from_blue(pw, ph, blue_px, arc_cx, arc_cy, full_circle=(i == 13))
        pack = ProgressPack(ow, oh, ox + ow // 2, oy + oh // 2, sz, tip_x, tip_y)
        overlays.append(pack)
        print(f"new_progress_{i}.bin: {sz} bytes ({ow}x{oh}) anchor=({pack.anchor_x},{pack.anchor_y}) tip=({tip_x},{tip_y})")

    return bg_pack, overlays


def stem_from_png(name: str) -> str:
    return Path(name).stem


def macro_prefix(stem: str) -> str:
    return "NEW_HEAT_" + stem.upper().replace(".", "_")


# 裁剪后 progress 锚点（全屏坐标）；无 PNG 重打包时从已有 bin + 此表计算 tip
DEFAULT_PROGRESS_ANCHORS: dict[str, tuple[int, int]] = {
    "new_progress_bg": (160, 109),
    **{f"new_progress_{i}": (0, 0) for i in range(1, 14)},
}
DEFAULT_PROGRESS_ANCHORS.update({
    "new_progress_2": (241, 149),
    "new_progress_3": (241, 137),
    "new_progress_4": (235, 125),
    "new_progress_5": (226, 116),
    "new_progress_6": (214, 110),
    "new_progress_7": (201, 108),
    "new_progress_8": (189, 108),
    "new_progress_9": (177, 108),
    "new_progress_10": (168, 108),
    "new_progress_11": (162, 108),
    "new_progress_12": (160, 108),
    "new_progress_13": (160, 108),
})


def parse_anchor(defines: dict[str, str], stem: str) -> tuple[int, int] | None:
    tag = macro_prefix(stem)
    x_key = f"{tag}_ANCHOR_X"
    y_key = f"{tag}_ANCHOR_Y"
    if x_key not in defines or y_key not in defines:
        return None
    return int(defines[x_key]), int(defines[y_key])


def fill_progress_metadata(existing: dict[str, str],
                           sizes: dict[str, tuple[int, int, int]],
                           progress_anchors: dict[str, tuple[int, int]],
                           progress_tips: dict[str, tuple[int, int]]) -> None:
    bg_path = BIN_DIR / "new_progress_bg.bin"
    if "new_progress_bg" not in sizes and bg_path.exists():
        cached = parse_item_size(existing, "new_progress_bg")
        if cached is not None:
            sizes["new_progress_bg"] = cached
        else:
            w, h, _ = load_gpu_bin(bg_path)
            sizes["new_progress_bg"] = (w, h, bg_path.stat().st_size)

    for stem in ("new_progress_bg", *(f"new_progress_{i}" for i in range(1, 14))):
        anchor = parse_anchor(existing, stem)
        if anchor is None:
            anchor = DEFAULT_PROGRESS_ANCHORS.get(stem)
        if anchor is not None:
            progress_anchors[stem] = anchor

    bg_anchor = progress_anchors.get("new_progress_bg")
    if bg_anchor is None and bg_path.exists():
        w, h, _ = load_gpu_bin(bg_path)
        bg_anchor = (w // 2, h // 2)
        progress_anchors["new_progress_bg"] = bg_anchor
        print(f"WARN: new_progress_bg anchor inferred ({bg_anchor[0]},{bg_anchor[1]})")
    if bg_anchor is None:
        return

    arc_cx, arc_cy = bg_anchor
    bg_pack = None
    progress_overlay_packs: list[ProgressPack] = []
    if bg_path.exists():
        bw, bh, bg_px = load_gpu_bin(bg_path)
        bg_pack = ProgressPack(bw, bh, arc_cx, arc_cy, bg_path.stat().st_size)
        start_x, start_y = arc_start_tip_from_bg(bw, bh, bg_px, arc_cx, arc_cy, arc_cx, arc_cy)
        progress_tips["new_progress_1"] = (start_x, start_y)
        print(f"new_progress_1: start tip=({start_x},{start_y})")
        for i in range(1, 14):
            stem = f"new_progress_{i}"
            anchor = progress_anchors.get(stem, (0, 0))
            bin_path = BIN_DIR / f"{stem}.bin"
            if i == 1 or not bin_path.exists():
                progress_overlay_packs.append(ProgressPack(0, 0, 0, 0, 0))
                continue
            ow, oh, _ = load_gpu_bin(bin_path)
            progress_overlay_packs.append(
                ProgressPack(ow, oh, anchor[0], anchor[1], bin_path.stat().st_size))

    for i in range(2, 14):
        stem = f"new_progress_{i}"
        bin_path = BIN_DIR / f"{stem}.bin"
        if not bin_path.exists():
            continue
        ow, oh, pixels = load_gpu_bin(bin_path)
        if ow == 0 or oh == 0:
            continue
        anchor = progress_anchors.get(stem)
        if anchor is None:
            continue
        ax, ay = anchor
        if i >= 11 and bg_pack is not None and len(progress_overlay_packs) >= i:
            blues = composite_progress_blue(i, bg_pack, progress_overlay_packs)
            tip_x, tip_y = arc_tip_countdown(blues, arc_cx, arc_cy, i)
        else:
            tip_x, tip_y = tip_from_cropped_bin(ow, oh, pixels, arc_cx, arc_cy, ax, ay,
                                                full_circle=(i == 13))
        progress_tips[stem] = (tip_x, tip_y)
        print(f"{stem}: anchor=({ax},{ay}) tip=({tip_x},{tip_y})")


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
    progress_tips: dict[str, tuple[int, int]] = {}
    track_tips: dict[str, tuple[int, int]] = {}
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
        if stem == "new_point":
            data, w, h = point_to_gpu()
            out = BIN_DIR / f"{stem}.bin"
            out.write_bytes(data)
            sizes[stem] = (w, h, len(data))
            point_sz = len(data)
            print(f"{stem}.bin: {len(data)} bytes ({w}x{h}) <- generated")
            continue
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
            tips = track_tips_from_bin(out)
            if tips is not None:
                track_tips[stem] = tips
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
            progress_tips[stem] = (pack.tip_x, pack.tip_y)
            progress_max = max(progress_max, pack.ram_size)
            overlay_max = max(overlay_max, pack.ram_size)

    fill_progress_metadata(existing, sizes, progress_anchors, progress_tips)

    for i in range(1, 8):
        stem = f"new_temp_{i}"
        if stem in track_tips:
            continue
        tips = track_tips_from_bin(BIN_DIR / f"{stem}.bin")
        if tips is not None:
            track_tips[stem] = tips
    for i in range(1, 14):
        stem = f"new_time_{i}"
        if stem in track_tips:
            continue
        tips = track_tips_from_bin(BIN_DIR / f"{stem}.bin")
        if tips is not None:
            track_tips[stem] = tips

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
            if stem in progress_tips:
                tx, ty = progress_tips[stem]
                lines.append(f"#define {tag}_TIP_X            {tx}")
                lines.append(f"#define {tag}_TIP_Y            {ty}")
        if stem in track_tips:
            left_x, right_x = track_tips[stem]
            lines.append(f"#define {tag}_TIP_LEFT_X            {left_x}")
            lines.append(f"#define {tag}_TIP_RIGHT_X           {right_x}")
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
