#!/usr/bin/env python3
"""Render U4 texture masks over a K1-port atlas for visual QA.

Why: a human must confirm each UV mask lands on the intended atlas region before U2/U3 trust it.
Output per label: a dimmed-atlas overlay PNG, plus contact.png (atlas first, then each label).
Orientation: load_file/load_game return display order; load_atlas flips once back to STORED order,
and masks are used as stored (row 0 at top). Run with the repo .venv python (numpy + PIL).
"""
import argparse
import json
import math
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

REPO = Path(__file__).resolve().parents[2]
SCRIPTS = REPO / "scripts"
STAGE = Path("~/Games/KOTOR2_Mod_Workspace/k1_port_stage/override").expanduser()
PALETTE = [(255, 70, 70), (70, 170, 255), (255, 210, 0), (110, 255, 110),
           (255, 90, 220), (255, 150, 40), (160, 110, 255), (0, 230, 200)]
DIM = 0.5
ALPHA = 0.55
COLS = 4
LABEL_PX = 14
BAND_PX = 16
FOOTER = "rows in STORED order (row 0 at top); atlas flipped back from loader output"


class AtlasError(Exception):
    pass


def load_atlas(spec):
    """Atlas RGBA in STORED order. spec = existing file path, else texture name (staged override, then vanilla)."""
    sys.path.insert(0, str(SCRIPTS))
    import npc_div_texture_sheets as sheets

    p = Path(spec).expanduser()
    if p.is_file():
        img = sheets.load_file(p)[0]
    else:
        hit = next((STAGE / f"{spec}{ext}" for ext in (".tpc", ".tga") if (STAGE / f"{spec}{ext}").is_file()), None)
        if hit is not None:
            img = sheets.load_file(hit)[0]
        else:
            got = sheets.load_game(spec)
            if got is None:
                raise AtlasError(f"atlas '{spec}' is not a file, not in {STAGE}, and not a vanilla texture")
            img = got[0]
    return img.transpose(Image.FLIP_TOP_BOTTOM)  # the only orientation step: display order -> stored order


def read_mask(path):
    """Bool (h, w) array in stored order. Greyscale PNG, or RGB PNG using red; value >= 128 is in mask."""
    im = Image.open(path)
    if im.mode in ("RGB", "RGBA"):
        return np.asarray(im)[..., 0] >= 128
    return np.asarray(im.convert("L")) >= 128


def coverage_pct(mb):
    return 100.0 * float(mb.mean())


def _interior(mb):
    p = np.pad(mb, 1, constant_values=False)
    return p[1:-1, 1:-1] & p[:-2, 1:-1] & p[2:, 1:-1] & p[1:-1, :-2] & p[1:-1, 2:]


def _mask_bool(m, h, w):
    if isinstance(m, (bytes, bytearray)):
        if len(m) != h * w:
            raise ValueError(f"mask has {len(m)} bytes, atlas needs {h * w}")
        return np.frombuffer(bytes(m), dtype=np.uint8).reshape(h, w) >= 128
    arr = np.asarray(m)
    if arr.shape != (h, w):
        raise ValueError(f"mask shape {arr.shape} != atlas {(h, w)}")
    return arr if arr.dtype == bool else arr >= 128


def _fit(im, tile):
    s = min(tile / im.width, tile / im.height)
    sz = (max(1, round(im.width * s)), max(1, round(im.height * s)))
    canvas = Image.new("RGB", (tile, tile), (40, 40, 40))
    canvas.paste(im.resize(sz, Image.LANCZOS), ((tile - sz[0]) // 2, (tile - sz[1]) // 2))
    return canvas


def _caption(canvas, x, y, text):
    d = ImageDraw.Draw(canvas)
    d.rectangle([x, y, x + 6 * len(text) + 8, y + LABEL_PX], fill=(0, 0, 0))
    d.text((x + 4, y + 2), text, fill=(255, 255, 0))


def compose(atlas_rgba, masks, captions=None, tile=512):
    """Pure. atlas_rgba: PIL image in stored order. masks: {label: bool array | uint8 array | bytes} at atlas size.
    captions: {label: text}. Returns ({label: RGB overlay at atlas size}, RGB contact sheet)."""
    captions = captions or {}
    atlas_rgb = atlas_rgba.convert("RGB")
    w, h = atlas_rgb.size
    dim = (np.asarray(atlas_rgb, dtype=np.float32) * DIM).astype(np.uint8)
    labels = sorted(masks)
    overlays = {}
    for i, label in enumerate(labels):
        mb = _mask_bool(masks[label], h, w)
        col = PALETTE[i % len(PALETTE)]
        out = dim.copy()
        out[mb] = np.round(dim[mb] * (1 - ALPHA) + np.array(col, dtype=np.float64) * ALPHA).astype(np.uint8)
        out[mb & ~_interior(mb)] = col  # 1px outline
        overlays[label] = Image.fromarray(out, "RGB")
    tiles = [atlas_rgb] + [overlays[lab] for lab in labels]
    names = ["atlas"] + labels
    cols = min(COLS, len(tiles))
    rows = math.ceil(len(tiles) / cols)
    sheet = Image.new("RGB", (cols * tile, rows * tile + BAND_PX), (40, 40, 40))
    for k, (im, name) in enumerate(zip(tiles, names)):
        x, y = (k % cols) * tile, (k // cols) * tile
        sheet.paste(_fit(im, tile), (x, y))
        _caption(sheet, x, y, f"atlas {w}x{h}" if k == 0 else captions.get(name, name))
    ImageDraw.Draw(sheet).text((4, rows * tile + 3), FOOTER, fill=(255, 255, 0))
    return overlays, sheet


def render(args):
    out = Path(args.out).expanduser().resolve()
    if not args.allow_repo and (out == REPO or REPO in out.parents):
        print(f"refused: --out {out} is inside the repo; choose another dir or pass --allow-repo", file=sys.stderr)
        return 2
    mdir = Path(args.masks).expanduser()
    meta = json.loads((mdir / "masks.json").read_text(encoding="utf-8"))
    atlas = load_atlas(args.atlas)
    w, h = atlas.size
    masks = {lab: read_mask(mdir / f"{lab}.png") for lab in sorted(meta["labels"])}
    for lab, mb in masks.items():
        if mb.shape != (h, w):
            raise AtlasError(f"mask {lab} is {mb.shape[1]}x{mb.shape[0]}, atlas is {w}x{h}")
    captions = {lab: f"{lab} {coverage_pct(mb):.1f}%" for lab, mb in masks.items()}
    overlays, sheet = compose(atlas, masks, captions, tile=args.tile)
    out.mkdir(parents=True, exist_ok=True)
    for lab, im in overlays.items():
        im.save(out / f"{lab}.png")
    sheet.save(out / "contact.png")
    print(f"wrote {len(overlays)} overlays + contact.png to {out}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description="Mask overlay QA for K1-port atlases.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("render", help="overlay each mask on the atlas")
    r.add_argument("--masks", required=True, help="U4 mask dir (masks.json + <label>.png)")
    r.add_argument("--atlas", required=True, help="atlas file path, or texture name")
    r.add_argument("--out", required=True, help="output dir (no default; must be outside the repo)")
    r.add_argument("--tile", type=int, default=512, help="contact sheet tile px")
    r.add_argument("--allow-repo", action="store_true", help="permit --out inside the repo")
    args = ap.parse_args(argv)
    try:
        return render(args)
    except (AtlasError, OSError, ValueError, KeyError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
