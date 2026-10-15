#!/usr/bin/env python3
"""Renders the app icon (img/icon.svg) for all platforms. Results are committed; re-run after
changing the icon. Requires resvg (see tools/bake_images.cmake) and Pillow.

    python tools/bake_icons.py [--resvg path/to/resvg]

Outputs:
  app/src/main/res/mipmap-*/ic_launcher_foreground.png   Android adaptive icon foreground
  cmake/ios/Assets.xcassets/AppIcon.appiconset/          iOS app icon (1024 px, opaque)
  cmake/windows/sheepdog.ico                             Windows executable icon
  assets/icon.png                                        window icon (desktop)
"""
import argparse
import io
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
SVG = ROOT / "img" / "icon.svg"
BACKGROUND = (55, 139, 0)  # the icon's own green, behind / around it

# Android adaptive icons: 108 dp layer, the icon fits the 72 dp safe zone
ANDROID_DENSITIES = {"mdpi": 1.0, "hdpi": 1.5, "xhdpi": 2.0, "xxhdpi": 3.0, "xxxhdpi": 4.0}


def find_resvg(explicit):
    candidates = [explicit, os.environ.get("RESVG"), shutil.which("resvg"),
                  str(Path.home() / "tools" / "resvg" / "resvg.exe"), str(Path.home() / ".cargo" / "bin" / "resvg")]
    for c in candidates:
        if c and Path(c).exists():
            return c
    raise SystemExit("resvg not found; install it or pass --resvg")


def render(resvg, size):
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / "icon.png"
        subprocess.run([resvg, "-w", str(size), "-h", str(size), str(SVG), str(out)], check=True)
        return Image.open(io.BytesIO(out.read_bytes())).convert("RGBA")


def on_background(img):
    bg = Image.new("RGBA", img.size, BACKGROUND + (255,))
    bg.alpha_composite(img)
    return bg.convert("RGB")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--resvg")
    resvg = find_resvg(ap.parse_args().resvg)

    # Android: foreground layer (icon in the safe zone, transparent around it)
    res = ROOT / "app" / "src" / "main" / "res"
    for density, scale in ANDROID_DENSITIES.items():
        layer = round(108 * scale)
        icon = render(resvg, round(76 * scale))  # slightly larger than 72 dp: the icon's corners are rounded
        fg = Image.new("RGBA", (layer, layer), (0, 0, 0, 0))
        fg.alpha_composite(icon, ((layer - icon.width) // 2, (layer - icon.height) // 2))
        out = res / f"mipmap-{density}"
        out.mkdir(parents=True, exist_ok=True)
        fg.save(out / "ic_launcher_foreground.png", optimize=True)

    # iOS: single 1024 px icon, no transparency allowed; iOS rounds the corners itself
    ios = ROOT / "cmake" / "ios" / "Assets.xcassets" / "AppIcon.appiconset"
    ios.mkdir(parents=True, exist_ok=True)
    on_background(render(resvg, 1024)).save(ios / "icon-1024.png", optimize=True)

    # Windows .ico with the usual sizes
    win = ROOT / "cmake" / "windows"
    win.mkdir(parents=True, exist_ok=True)
    render(resvg, 256).save(win / "sheepdog.ico", sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (256, 256)])

    # window icon
    render(resvg, 256).save(ROOT / "assets" / "icon.png", optimize=True)
    print("icons written")


if __name__ == "__main__":
    main()
