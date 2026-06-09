#!/usr/bin/env python3
"""Convert Mode page PNGs to same-name GPU bins in Output/bin/ui/home/.

Only these files are processed (PNG -> same-name .bin):
  pasta.png, chicken.png, insulation.png, while_line.png, blue_line.png
  g0.png..g9.png, gh.png (°F), gs.png (°C) (top green temperature)

Tab icons: transparent pixels filled black (unselected); func_mode.c tints black->blue when selected.

Run Output/bin/prebuild.bat afterward to pack ui.bin and refresh ui.h.
"""

from __future__ import annotations

import re
import struct
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
RES_DIR = ROOT / "res" / "home"
BIN_DIR = ROOT / "Output" / "bin" / "ui" / "home"
OUT_H = ROOT / "functions" / "home_icon_res.h"

MODE_ICON_ALIASES: dict[str, tuple[str, ...]] = {
    "pasta.png": ("Pasta.png",),
    "chicken.png": ("Chicken.png",),
    "insulation.png": ("Warm.png", "warm.png"),
}

MODE_ICON_PNGS = (
    "pasta.png",
    "chicken.png",
    "insulation.png",
)

MODE_LINE_PNGS = (
    "while_line.png",
    "blue_line.png",
)

MODE_GREEN_PNGS = (
    *(f"g{d}.png" for d in range(10)),
    "gh.png",
    "gs.png",
)

# Icon transparent fill: black (selected tab tints to blue in func_mode.c)
BG_BLACK = (0, 0, 0)

OBSOLETE_BINS = (
    "pasta_sel.bin",
    "pasta_nor.bin",
    "chicken_sel.bin",
    "chicken_nor.bin",
    "warm_sel.bin",
    "warm_nor.bin",
    "insulation_sel.bin",
    "insulation_nor.bin",
    "dash_sel.bin",
    "dash_nor.bin",
)


def rgba565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def find_png(name: str, aliases: tuple[str, ...] = ()) -> Path | None:
    candidates = (name,) + aliases
    for cand in candidates:
        for base in (BIN_DIR, RES_DIR):
            direct = base / cand
            if direct.exists():
                return direct
            alt = base / (cand[:1].upper() + cand[1:])
            if alt.exists():
                return alt
    return None


def gpu_size_from_bin(path: Path) -> tuple[int, int]:
    data = path.read_bytes()
    if len(data) < 8:
        raise SystemExit(f"invalid bin (too short): {path.name}")
    magic, w, h = struct.unpack("<IHH", data[:8])
    if magic != 0x24150 or w == 0 or h == 0:
        raise SystemExit(f"invalid GPU bin: {path.name}")
    need = 8 + w * h * 2
    if need != len(data):
        raise SystemExit(f"size mismatch {path.name}: file {len(data)} need {need}")
    return w, h


def convert_png(name: str, aliases: tuple[str, ...], bg_rgb: tuple[int, int, int]) -> tuple[int, int]:
    bin_name = Path(name).stem + ".bin"
    out = BIN_DIR / bin_name
    src = find_png(name, aliases)
    if src is None:
        if out.exists():
            w, h = gpu_size_from_bin(out)
            print(f"{bin_name}: keep existing ({len(out.read_bytes())} bytes)")
            return w, h
        raise SystemExit(f"missing {name} and {bin_name}")
    data, w, h = png_to_gpu(src, bg_rgb)
    write_bin(bin_name, data, src)
    return w, h


def png_to_gpu(path: Path, bg_rgb: tuple[int, int, int]) -> tuple[bytes, int, int]:
    im = Image.open(path).convert("RGBA")
    w, h = im.size
    px = im.load()
    bg = rgba565(*bg_rgb)
    buf = bytearray()
    buf += struct.pack("<IHH", 0x24150, w, h)
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            c = bg if a < 32 else rgba565(r, g, b)
            buf += struct.pack("<H", c)
    return bytes(buf), w, h


def write_bin(name: str, data: bytes, src: Path) -> None:
    BIN_DIR.mkdir(parents=True, exist_ok=True)
    out = BIN_DIR / name
    out.write_bytes(data)
    print(f"{name}: {len(data)} bytes <- {src.name}")


def remove_obsolete() -> None:
    for name in OBSOLETE_BINS:
        path = BIN_DIR / name
        if path.exists():
            path.unlink()
            print(f"removed obsolete {name}")


def remove_mode_png_from_bin_dir() -> None:
    """Only remove Mode source PNGs; leave setup/other PNGs for gen_home_icons.py."""
    names: set[str] = set()
    for png_name in MODE_ICON_PNGS + MODE_LINE_PNGS + MODE_GREEN_PNGS:
        names.add(png_name)
        names.add(png_name[:1].upper() + png_name[1:])
        for alias in MODE_ICON_ALIASES.get(png_name, ()):
            names.add(alias)
            names.add(alias[:1].upper() + alias[1:])
    for name in sorted(names):
        path = BIN_DIR / name
        if path.exists():
            path.unlink()
            print(f"removed mode png from ui/home: {name}")


def patch_home_icon_res(sizes: dict[str, tuple[int, int]]) -> None:
    pasta_w, pasta_h = sizes["pasta"]
    chicken_w, chicken_h = sizes["chicken"]
    ins_w, ins_h = sizes["insulation"]
    line_w, line_h = sizes["while_line"]
    icon_max_h = max(pasta_h, chicken_h, ins_h)
    icon_max_bytes = max(
        8 + pasta_w * pasta_h * 2,
        8 + chicken_w * chicken_h * 2,
        8 + ins_w * ins_h * 2,
    )
    line_bytes = 8 + line_w * line_h * 2

    block = (
        "/* Mode tab icons/lines (tools/gen_mode_icons.py) */\n"
        f"#define MODE_TAB_PASTA_W                 {pasta_w}\n"
        f"#define MODE_TAB_PASTA_H                 {pasta_h}\n"
        f"#define MODE_TAB_CHICKEN_W               {chicken_w}\n"
        f"#define MODE_TAB_CHICKEN_H               {chicken_h}\n"
        f"#define MODE_TAB_INSULATION_W            {ins_w}\n"
        f"#define MODE_TAB_INSULATION_H            {ins_h}\n"
        f"#define MODE_TAB_ICON_MAX_H             {icon_max_h}\n"
        f"#define MODE_TAB_ICON_RAM_MAX_SIZE       {icon_max_bytes}\n"
        f"#define MODE_TAB_LINE_W                  {line_w}\n"
        f"#define MODE_TAB_LINE_H                  {line_h}\n"
        f"#define MODE_TAB_LINE_RAM_SIZE           {line_bytes}\n"
    )

    text = OUT_H.read_text(encoding="utf-8")
    text = re.sub(
        r"/\* Mode tab icons/lines \(tools/gen_mode_icons\.py\) \*/\n"
        r"#define MODE_TAB_PASTA_W.*?"
        r"#define MODE_TAB_LINE_RAM_SIZE           \d+\n",
        block,
        text,
        flags=re.DOTALL,
    )
    if "MODE_TAB_PASTA_W" not in text:
        text = text.replace("\n#endif\n", f"\n{block}\n#endif\n")
    OUT_H.write_text(text, encoding="utf-8")
    print(f"updated: {OUT_H}")


def patch_green_status_res(g_sizes: dict[str, tuple[int, int]]) -> None:
    digit_keys = [f"g{d}" for d in range(10)]
    digit_max_bytes = max(8 + g_sizes[k][0] * g_sizes[k][1] * 2 for k in digit_keys)
    digit_max_h = max(g_sizes[k][1] for k in digit_keys)
    gh_w, gh_h = g_sizes["gh"]
    gh_bytes = 8 + gh_w * gh_h * 2
    gs_w, gs_h = g_sizes["gs"]
    gs_bytes = 8 + gs_w * gs_h * 2
    sym_max_bytes = max(gh_bytes, gs_bytes)

    lines = [
        "/* Mode status green temp g0..g9 + gh/gs (tools/gen_mode_icons.py) */",
        f"#define MODE_G_DIGIT_MAX_H              {digit_max_h}",
        f"#define MODE_G_DIGIT_RAM_MAX_SIZE       {digit_max_bytes}",
    ]
    for d in range(10):
        w, h = g_sizes[f"g{d}"]
        lines.append(f"#define MODE_G{d}_W                      {w}")
        lines.append(f"#define MODE_G{d}_H                      {h}")
    lines.extend([
        f"#define MODE_GH_W                       {gh_w}",
        f"#define MODE_GH_H                       {gh_h}",
        f"#define MODE_GH_RAM_SIZE                {gh_bytes}",
        f"#define MODE_GS_W                       {gs_w}",
        f"#define MODE_GS_H                       {gs_h}",
        f"#define MODE_GS_RAM_SIZE                {gs_bytes}",
        f"#define MODE_G_SYM_RAM_MAX_SIZE         {sym_max_bytes}",
        "",
    ])
    block = "\n".join(lines)

    text = OUT_H.read_text(encoding="utf-8")
    text = re.sub(
        r"/\* Mode status green temp g0\.\.g9 \+ gh(?:/gs)? \(tools/gen_mode_icons\.py\) \*/\n"
        r"#define MODE_G_DIGIT_MAX_H.*?#define MODE_G_SYM_RAM_MAX_SIZE         \d+\n",
        block,
        text,
        flags=re.DOTALL,
    )
    text = re.sub(
        r"/\* Mode status green temp g0\.\.g9 \+ gh \(tools/gen_mode_icons\.py\) \*/\n"
        r"#define MODE_G_DIGIT_MAX_H.*?#define MODE_GH_RAM_SIZE                \d+\n",
        block,
        text,
        flags=re.DOTALL,
    )
    if "MODE_G_DIGIT_MAX_H" not in text:
        text = text.replace(
            "\n#endif\n",
            f"\n{block}\n#endif\n",
        )
    OUT_H.write_text(text, encoding="utf-8")
    print(f"updated green temp sizes in {OUT_H}")


def main() -> None:
    sizes: dict[str, tuple[int, int]] = {}
    for png_name in MODE_ICON_PNGS:
        w, h = convert_png(png_name, MODE_ICON_ALIASES.get(png_name, ()), BG_BLACK)
        sizes[Path(png_name).stem] = (w, h)

    for png_name in MODE_LINE_PNGS:
        w, h = convert_png(png_name, (), BG_BLACK)
        sizes[Path(png_name).stem] = (w, h)

    remove_obsolete()

    g_sizes: dict[str, tuple[int, int]] = {}
    for png_name in MODE_GREEN_PNGS:
        w, h = convert_png(png_name, (), BG_BLACK)
        g_sizes[Path(png_name).stem] = (w, h)

    patch_home_icon_res(sizes)
    patch_green_status_res(g_sizes)
    remove_mode_png_from_bin_dir()

    print("\nNext: run Output/bin/prebuild.bat, then rebuild and flash ui.bin + app.bin")


if __name__ == "__main__":
    main()
