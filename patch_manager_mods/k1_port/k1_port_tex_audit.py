#!/usr/bin/env python3
"""Coverage audit for baked texture looks: what each atlas's looks paint, which UV islands exist, what is left over.

  .venv/bin/python -I k1_port_tex_audit.py --bake RUN --out DIR [--masks ROOT] [--only ATLAS]

RUN is a bake output dir (bake.json, parents/, regions/, masks/<model>/). For every parent atlas it writes
DIR/<atlas>.png (2x2 panel: grid + cell names | island colours | painted union | gap map) and DIR/<atlas>.json
(per-cell painted and content fractions, island coverage, unlabelled and unpainted content). The gap map marks
content (luma > BG_LUMA) that no look paints: red = non-skin, yellow = skin-like, cyan = also outside every island.
Rows are STORED order. Read only; nothing is written outside DIR.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
import k1_port_tex_overlay as tov  # noqa: E402
import k1_port_tex_tpc as tp  # noqa: E402
import k1_port_uvmask_overlay as uvo  # noqa: E402

BG_LUMA = 24  # 0..255; darker texels count as atlas background, not content
PANEL = 512
PALETTE = [(255, 70, 70), (70, 170, 255), (255, 210, 0), (110, 255, 110), (200, 110, 255), (255, 150, 60),
           (90, 255, 230), (255, 110, 200), (180, 180, 180)]


def _luma(rgb):
    return rgb @ np.array([0.299, 0.587, 0.114], dtype=np.float32)


def _tile(arr_u8, size=PANEL):
    return Image.fromarray(arr_u8, "RGB").resize((size, size), Image.NEAREST)


def _grid(im, h, w):
    d = ImageDraw.Draw(im)
    for k in range(1, 4):
        d.line([(k * PANEL // 4, 0), (k * PANEL // 4, PANEL)], fill=(255, 255, 0))
        d.line([(0, k * PANEL // 4), (PANEL, k * PANEL // 4)], fill=(255, 255, 0))
    for r in range(4):
        for c in range(4):
            x, y = c * PANEL // 4 + 2, r * PANEL // 4 + 2
            d.rectangle([x, y, x + 16, y + 11], fill=(0, 0, 0))
            d.text((x + 2, y), f"{'ABCD'[c]}{r + 1}", fill=(255, 255, 0))
    return im


def _title(im, text):
    d = ImageDraw.Draw(im)
    d.rectangle([0, PANEL - 14, 6 * len(text) + 8, PANEL], fill=(0, 0, 0))
    d.text((4, PANEL - 12), text, fill=(255, 255, 255))
    return im


def gap_texture(rgb, painted, skin, any_island):
    """uint8 (h,w,3) gap image: dimmed atlas, red = unpainted content, yellow = unpainted skin-like, cyan = outside islands."""
    content = _luma(rgb.astype(np.float32)) > BG_LUMA
    gap = (rgb * 0.3).astype(np.uint8)
    unpaint = content & ~painted
    gap[unpaint & ~skin] = (255, 40, 40)
    gap[unpaint & skin] = (255, 220, 0)
    gap[unpaint & ~any_island] = (0, 230, 230)
    return gap


def recipe_models(looks_dir: Path) -> dict:
    """id -> model from the shipped recipes; older bake.json files do not record the model."""
    out = {}
    for f in sorted(Path(looks_dir).glob("*.json")):
        for rec in json.loads(f.read_text()):
            out[rec["id"]] = rec["model"]
    return out


def audit_atlas(atlas: str, run: Path, looks: list[dict], out: Path, masks_root: Path | None = None) -> dict:
    parent = (run / "parents" / f"{atlas}.tpc").read_bytes()
    rgb = np.ascontiguousarray(tp.decode_mip(parent, tp.parse(parent), 0)[:, :, :3])
    h, w = rgb.shape[:2]
    model = None
    painted = np.zeros((h, w), dtype=bool)
    for e in looks:
        painted |= uvo.read_mask(str(run / "regions" / f"{e['child']}.png"))
        model = model or e.get("model")
    mdir = (masks_root or run) / "masks" / model
    labels = list(json.loads((mdir / "masks.json").read_text())["labels"])
    islands = {lab: uvo.read_mask(str(mdir / f"{lab}.png")) for lab in labels}
    any_island = np.zeros((h, w), dtype=bool)
    for m in islands.values():
        any_island |= m
    skin = tov.skin_guard(rgb)
    content = _luma(rgb.astype(np.float32)) > BG_LUMA

    p1 = _title(_grid(_tile(rgb), h, w), f"{atlas} {w}x{h} stored order")
    isl = (rgb * 0.35).astype(np.uint8)
    for i, lab in enumerate(labels):
        col = np.array(PALETTE[i % len(PALETTE)], dtype=np.float32)
        isl[islands[lab]] = (rgb[islands[lab]] * 0.4 + col * 0.6).astype(np.uint8)
    p2 = _grid(_tile(isl), h, w)
    d = ImageDraw.Draw(p2)
    for i, lab in enumerate(labels):
        d.text((4, 4 + 11 * 0), "", fill=(255, 255, 255))
        d.rectangle([PANEL - 92, 2 + 12 * i, PANEL - 2, 13 + 12 * i], fill=(0, 0, 0))
        d.text((PANEL - 90, 2 + 12 * i), lab, fill=PALETTE[i % len(PALETTE)])
    pnt = (rgb * 0.35).astype(np.uint8)
    pnt[painted] = (rgb[painted] * 0.4 + np.array([60, 255, 60]) * 0.6).astype(np.uint8)
    p3 = _title(_grid(_tile(pnt), h, w), "green = painted by a look")
    unpaint = content & ~painted
    gap = gap_texture(rgb, painted, skin, any_island)
    p4 = _title(_grid(_tile(gap), h, w), "red gap, yellow skin-like gap, cyan outside every island")
    sheet = Image.new("RGB", (2 * PANEL, 2 * PANEL))
    for k, im in enumerate((p1, p2, p3, p4)):
        sheet.paste(im, ((k % 2) * PANEL, (k // 2) * PANEL))
    sheet.save(out / f"{atlas}.png")

    cells = {}
    for r in range(4):
        for c in range(4):
            sl = (slice(r * h // 4, (r + 1) * h // 4), slice(c * w // 4, (c + 1) * w // 4))
            n = content[sl].sum()
            cells[f"{'ABCD'[c]}{r + 1}"] = {
                "content": round(float(n) / content[sl].size, 3),
                "painted": round(float((painted[sl] & content[sl]).sum()) / max(1, n), 3),
                "skin_like": round(float((skin[sl] & content[sl]).sum()) / max(1, n), 3),
            }
    total = max(1, int(content.sum()))
    stats = {
        "atlas": atlas, "size": [w, h], "model": model, "looks": [e["id"] for e in looks],
        "content_fraction": round(total / (h * w), 3),
        "painted_of_content": round(float((painted & content).sum()) / total, 3),
        "unpainted_nonskin_of_content": round(float((unpaint & ~skin).sum()) / total, 3),
        "unpainted_skin_like_of_content": round(float((unpaint & skin).sum()) / total, 3),
        "content_outside_every_island": round(float((content & ~any_island).sum()) / total, 3),
        "islands": {lab: {"texels_of_atlas": round(float(m.mean()), 3),
                          "painted_by_looks": round(float((m & painted).sum()) / max(1, int(m.sum())), 3)}
                    for lab, m in islands.items()},
        "cells": cells,
    }
    (out / f"{atlas}.json").write_text(json.dumps(stats, indent=1) + "\n", encoding="utf-8")
    return stats


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--bake", required=True, help="bake output dir (bake.json, parents/, regions/, masks/)")
    ap.add_argument("--out", required=True)
    ap.add_argument("--masks", default=None, help="mask root holding masks/<model>/ (default: the bake dir)")
    ap.add_argument("--only", default=None, help="audit one atlas")
    args = ap.parse_args(argv)
    run, out = Path(args.bake).expanduser(), Path(args.out).expanduser()
    looks = json.loads((run / "bake.json").read_text())["looks"]
    models = recipe_models(HERE / "looks")
    for e in looks:
        e["model"] = e.get("model") or models.get(e["id"])
    out.mkdir(parents=True, exist_ok=True)
    by_atlas: dict[str, list[dict]] = {}
    for e in looks:
        by_atlas.setdefault(e["parent"], []).append(e)
    for atlas, group in sorted(by_atlas.items()):
        if args.only and atlas != args.only:
            continue
        st = audit_atlas(atlas, run, group, out, Path(args.masks).expanduser() if args.masks else None)
        print(f"{atlas}: painted {st['painted_of_content']:.0%} of content, unpainted non-skin "
              f"{st['unpainted_nonskin_of_content']:.0%}, outside islands {st['content_outside_every_island']:.0%}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
