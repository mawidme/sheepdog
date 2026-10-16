#!/usr/bin/env python3
"""Bakes the UI font into bitmap atlases: assets/fonts/<scale>x/ui.png + ui.json for the
same image scales as the baked images (tools/bake_images.cmake). The results are committed;
re-run after changing the font, size or character set. Requires Pillow (pip install pillow).

    python tools/bake_font.py

Glyphs are white with alpha, so the game can tint them. Metrics in ui.json are in logical
units (texels / scale); glyph rectangles in texels of that scale's atlas.
"""
import json
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
FONT = ROOT / "tools" / "fonts" / "DejaVuSans-Bold.ttf"
LICENSE = ROOT / "tools" / "fonts" / "DejaVu-LICENSE.txt"
OUT = ROOT / "assets" / "fonts"

SCALES = [1, 2, 4]          # must match the game's image scales
BASE_SIZE = 36              # logical pixel size the atlas is rendered at; smaller text is scaled down
CHARS = [chr(c) for c in range(32, 127)]
ATLAS_WIDTH_AT_1X = 512


def bake(scale):
    font = ImageFont.truetype(str(FONT), BASE_SIZE * scale)
    ascent, descent = font.getmetrics()
    pad = 2 * scale
    width = ATLAS_WIDTH_AT_1X * scale

    # place glyphs in rows
    glyphs = {}
    x, y, row_h = pad, pad, 0
    boxes = {}
    for ch in CHARS:
        l, t, r, b = font.getbbox(ch, anchor="ls")  # relative to the baseline origin
        w, h = max(0, r - l), max(0, b - t)
        if x + w + pad > width:
            x, y, row_h = pad, y + row_h + pad, 0
        boxes[ch] = (x, y, w, h, l, t)
        x += w + pad
        row_h = max(row_h, h)
    height = y + row_h + pad

    img = Image.new("RGBA", (width, height), (255, 255, 255, 0))
    draw = ImageDraw.Draw(img)
    for ch, (gx, gy, w, h, l, t) in boxes.items():
        if w and h:
            draw.text((gx - l, gy - t), ch, font=font, fill=(255, 255, 255, 255), anchor="ls")
        glyphs[str(ord(ch))] = {
            "rect": [gx, gy, w, h],                      # texels
            "offset": [l / scale, t / scale],            # logical, from the pen position on the baseline
            "advance": font.getlength(ch) / scale,       # logical
        }

    out_dir = OUT / f"{scale}x"
    out_dir.mkdir(parents=True, exist_ok=True)
    img.save(out_dir / "ui.png", optimize=True)
    meta = {"size": BASE_SIZE, "ascent": ascent / scale, "descent": descent / scale, "glyphs": glyphs}
    (out_dir / "ui.json").write_text(json.dumps(meta, separators=(",", ":")), encoding="utf-8")
    print(f"{scale}x: {width}x{height} atlas, {len(glyphs)} glyphs")


def main():
    for scale in SCALES:
        bake(scale)
    shutil.copyfile(LICENSE, OUT / "LICENSE-DejaVu.txt")


if __name__ == "__main__":
    main()
