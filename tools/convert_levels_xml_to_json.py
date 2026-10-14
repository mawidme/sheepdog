#!/usr/bin/env python3
"""Converts level files from the old XML format (Qt version) to the JSON format
described in src/game/level_data.hpp.

    python tools/convert_levels_xml_to_json.py levels/*.xml --out assets/levels

Coordinates keep their meaning (fence pole centres, top-left corners of props and
water, centres of dogs and sheep), so converted levels play exactly as before.
"""
import argparse
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


def ints(el, *names):
    try:
        return [int(float(el.attrib[n])) for n in names]
    except KeyError as e:
        raise ValueError(f"<{el.tag}> is missing attribute {e}") from None


def convert(xml_path):
    root = ET.parse(xml_path).getroot()
    if root.tag != "level":
        raise ValueError("root element is not <level>")

    settings = root.find("settings")
    width, height = ints(settings.find("size"), "width", "height")
    (time,) = ints(settings.find("time"), "seconds")

    level = {"version": 1, "size": [width, height], "time": time}
    fences, water, bushes, trees, exits = [], [], [], [], []
    tiles = root.find("tiles")
    for el in (tiles if tiles is not None else []):
        if el.tag == "fence":
            x0, y0, x1, y1 = ints(el, "xStart", "yStart", "xEnd", "yEnd")
            fences.append({"from": [x0, y0], "to": [x1, y1]})
        elif el.tag == "water":
            x0, y0, x1, y1, style = ints(el, "xStart", "yStart", "xEnd", "yEnd", "style")
            water.append({"from": [x0, y0], "to": [x1, y1], "style": style})
        elif el.tag in ("bush", "tree", "exit"):
            x, y, style = ints(el, "x", "y", "style")
            {"bush": bushes, "tree": trees, "exit": exits}[el.tag].append({"pos": [x, y], "style": style})
        else:
            print(f"{xml_path}: ignoring <{el.tag}> in tiles", file=sys.stderr)

    for key, items in (("fences", fences), ("water", water), ("bushes", bushes), ("trees", trees), ("exits", exits)):
        if items:
            level[key] = items

    chars = root.find("characters")
    level["dogs"] = [ints(el, "x", "y") for el in chars.findall("dog")]
    level["sheep"] = [ints(el, "x", "y") for el in chars.findall("sheep")]
    return level


def dump(level):
    """JSON with one element per line and [x, y] pairs kept on one line, for readable diffs."""
    def value(v):
        return json.dumps(v, separators=(", ", ": "))

    lines = ["{"]
    keys = list(level)
    for i, key in enumerate(keys):
        v = level[key]
        comma = "," if i < len(keys) - 1 else ""
        if isinstance(v, list) and v and not all(isinstance(x, int) for x in v):
            lines.append(f'  "{key}": [')
            lines += [f"    {value(item)}" + ("," if j < len(v) - 1 else "") for j, item in enumerate(v)]
            lines.append(f"  ]{comma}")
        else:
            lines.append(f'  "{key}": {value(v)}{comma}')
    lines.append("}")
    return "\n".join(lines) + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("files", nargs="+", type=Path)
    ap.add_argument("--out", type=Path, required=True, help="output directory")
    args = ap.parse_args()

    args.out.mkdir(parents=True, exist_ok=True)
    for path in args.files:
        level = convert(path)
        out = args.out / (path.stem + ".json")
        out.write_text(dump(level), encoding="utf-8", newline="\n")
        print(f"{path} -> {out}: {len(level.get('fences', []))} fences, {len(level['sheep'])} sheep")


if __name__ == "__main__":
    main()
