#!/usr/bin/env python3
"""Build font_asc_12.bin by scaling font_asc.bin glyphs (FONT v1, magic FONT).

Output: Output/bin/ui/0font/font_asc_12.bin
Run Output/bin/prebuild.bat afterward to refresh ui.h and ui.bin.
"""

from __future__ import annotations

import struct
import sys
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Pillow required: pip install Pillow", file=sys.stderr)
    raise

ROOT = Path(__file__).resolve().parents[1]
SRC_FONT = ROOT / "Output" / "bin" / "ui" / "0font" / "font_asc.bin"
OUT_FONT = ROOT / "Output" / "bin" / "ui" / "0font" / "font_asc_12.bin"
TARGET_HEIGHT = 12
GLYPH_COUNT = 96
INDEX_BASE = 72
ROW_STRIDE = 12
HEADER_SIZE = 64


def read_index(data: bytes) -> list[int]:
    return [struct.unpack_from("<H", data, INDEX_BASE + i * 3)[0] for i in range(GLYPH_COUNT)]


def decode_glyph_type2(blob: bytes) -> tuple[int, int, Image.Image] | None:
    if len(blob) < 6 or blob[0] != 0x02:
        return None
    if (len(blob) - 5) % ROW_STRIDE != 0:
        return None
    height = (len(blob) - 5) // ROW_STRIDE
    width = blob[1]
    if width == 0 or height == 0:
        return None

    img = Image.new("L", (width, height), 0)
    px = img.load()
    for row in range(height):
        row_off = 5 + row * ROW_STRIDE
        for col in range(width):
            byte = blob[row_off + (col >> 1)]
            nibble = (byte >> 4) if (col & 1) == 0 else (byte & 0x0F)
            px[col, row] = min(255, nibble * 17)
    return width, height, img


def decode_glyph_space(blob: bytes) -> tuple[int, int, Image.Image] | None:
    if len(blob) < 4 or blob[0] != 0x09:
        return None
    width = struct.unpack_from("<H", blob, 0)[0]
    if width == 0:
        width = 4
    img = Image.new("L", (width, max(1, TARGET_HEIGHT // 3)), 0)
    return width, img.height, img


def decode_glyph_pil(ch: str, width_hint: int) -> Image.Image:
    size = max(TARGET_HEIGHT + 2, 16)
    try:
        font = ImageFont.truetype("arial.ttf", TARGET_HEIGHT)
    except OSError:
        try:
            font = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", TARGET_HEIGHT)
        except OSError:
            font = ImageFont.load_default()
    pad = 2
    canvas = Image.new("L", (max(width_hint, 8) + pad * 2, TARGET_HEIGHT + pad * 2), 0)
    draw = ImageDraw.Draw(canvas)
    draw.text((pad, pad), ch, fill=255, font=font)
    bbox = canvas.getbbox()
    if bbox is None:
        return Image.new("L", (4, TARGET_HEIGHT), 0)
    return canvas.crop(bbox)


def trim_alpha(img: Image.Image) -> Image.Image:
    bbox = img.getbbox()
    if bbox is None:
        return Image.new("L", (1, 1), 0)
    return img.crop(bbox)


def encode_glyph_type2(img: Image.Image, row_stride: int) -> bytes:
    img = trim_alpha(img)
    width, height = img.size
    if width > 255:
        width = 255
        img = img.resize((width, height), Image.Resampling.LANCZOS)
    if height > 255:
        height = 255
        img = img.resize((width, height), Image.Resampling.LANCZOS)

    out = bytearray(5)
    out[0] = 0x02
    out[1] = width
    out[2] = height
    out[3] = height
    out[4] = 0

    px = img.load()
    for row in range(height):
        row_bytes = bytearray(row_stride)
        for col in range(width):
            nibble = min(15, px[col, row] // 17)
            idx = col >> 1
            if col & 1:
                row_bytes[idx] = (row_bytes[idx] & 0xF0) | nibble
            else:
                row_bytes[idx] = (row_bytes[idx] & 0x0F) | (nibble << 4)
        out.extend(row_bytes)
    return bytes(out)


def encode_glyph_space(img: Image.Image) -> bytes:
    img = trim_alpha(img)
    width = max(2, img.size[0])
    return struct.pack("<HBB", width, 0x01, 0x01) + b"\x00\x00\x00"


def scale_image(img: Image.Image, scale: float) -> Image.Image:
    w = max(1, int(round(img.size[0] * scale)))
    h = max(1, int(round(img.size[1] * scale)))
    if w == img.size[0] and h == img.size[1]:
        return img
    return img.resize((w, h), Image.Resampling.LANCZOS)


def build_scaled_font(src: bytes, target_h: int) -> bytes:
    if src[:4] != b"FONT":
        raise SystemExit("invalid FONT magic")
    src_h = src[14]
    if src_h == 0:
        raise SystemExit("invalid source font height")
    scale = target_h / src_h

    index = read_index(src)
    glyphs: list[bytes] = []
    max_w = 0

    for i in range(GLYPH_COUNT):
        ch = chr(0x20 + i)
        start = index[i]
        end = index[i + 1] if i + 1 < GLYPH_COUNT else len(src)
        blob = src[start:end]

        decoded = decode_glyph_type2(blob)
        if decoded is None and i == 0:
            decoded = decode_glyph_space(blob)
        if decoded is not None:
            width, _height, img = decoded
            img = scale_image(img, scale)
        else:
            hint = blob[1] if len(blob) > 1 else 8
            img = decode_glyph_pil(ch, max(4, int(hint * scale)))

        if i == 0 and ch == " ":
            glyph = encode_glyph_space(img)
        else:
            glyph = encode_glyph_type2(img, ROW_STRIDE)
            max_w = max(max_w, glyph[1])

        glyphs.append(glyph)

    packed = bytearray(src[:HEADER_SIZE])
    struct.pack_into("<H", packed, 12, 1)
    packed[14] = target_h
    packed[15] = min(255, max(int(round(src[15] * scale)), max_w + 2))

    index_bytes = INDEX_BASE + GLYPH_COUNT * 3
    if len(packed) < index_bytes:
        packed.extend(b"\x00" * (index_bytes - len(packed)))

    data_off = index_bytes
    new_offs: list[int] = []
    for glyph in glyphs:
        new_offs.append(data_off)
        packed.extend(glyph)
        data_off += len(glyph)

    for i, off in enumerate(new_offs):
        struct.pack_into("<H", packed, INDEX_BASE + i * 3, off)
        packed[INDEX_BASE + i * 3 + 2] = 0

    struct.pack_into("<I", packed, 4, len(packed))
    struct.pack_into("<I", packed, 8, HEADER_SIZE)
    return bytes(packed)


def main() -> None:
    if not SRC_FONT.exists():
        raise SystemExit(f"missing source font: {SRC_FONT}")

    src = SRC_FONT.read_bytes()
    out = build_scaled_font(src, TARGET_HEIGHT)
    OUT_FONT.write_bytes(out)
    print(
        f"wrote {OUT_FONT.name}: height {src[14]} -> {TARGET_HEIGHT}, "
        f"size {len(src)} -> {len(out)}"
    )


if __name__ == "__main__":
    main()
