#!/usr/bin/env python3
"""Review sheets for baked texture looks: per family one row per look (parent | child | amplified diff | region),
plus index.png with every child tile in a 4-column grid. Reads a bake dir written by k1_port_tex_bake.py.

Rows keep the STORED order of bake.json (row 0 at top). Captions use the k1_port_uvmask_overlay._caption style.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
import k1_port_tex_tpc as tp  # noqa: E402
import k1_port_uvmask_overlay as uvo  # noqa: E402

TILE = 256
GAP = 4
BAND = uvo.LABEL_PX + 6
BG = (24, 24, 24)
FOOTER = "rows in STORED order (row 0 at top)"


class SheetsError(ValueError):
    """Bad or missing bake input; the bake CLI maps it to exit 2."""


def _load_json(path: Path) -> dict:
    if not path.is_file():
        raise SheetsError(f"missing {path}")
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise SheetsError(f"cannot read {path}: {exc}") from exc
    if not isinstance(data, dict):
        raise SheetsError(f"{path} is not a JSON object")
    return data


def _decode_file(path: Path) -> np.ndarray:
    if not path.is_file():
        raise SheetsError(f"missing {path}")
    b = path.read_bytes()
    return np.ascontiguousarray(tp.decode_mip(b, tp.parse(b), 0)[:, :, :3])


def _safe(name: str) -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", name) or "unfiled"


def _fit(rgb: np.ndarray) -> Image.Image:
    """Aspect-preserving fit into a TILE x TILE black cell, centred."""
    im = Image.fromarray(np.ascontiguousarray(rgb, dtype=np.uint8))
    s = min(TILE / im.width, TILE / im.height)
    im = im.resize((max(1, round(im.width * s)), max(1, round(im.height * s))), Image.Resampling.LANCZOS)
    canvas = Image.new("RGB", (TILE, TILE), (0, 0, 0))
    canvas.paste(im, ((TILE - im.width) // 2, (TILE - im.height) // 2))
    return canvas


def _row(caption: str, parent: np.ndarray, child: np.ndarray, region: np.ndarray) -> Image.Image:
    diff = np.clip(np.abs(child.astype(np.int16) - parent.astype(np.int16)) * 4, 0, 255).astype(np.uint8)
    mask = np.repeat((np.asarray(region, dtype=bool).astype(np.uint8) * 255)[:, :, None], 3, axis=2)
    tiles = [("parent", parent), ("child", child), ("diff x4", diff), ("region", mask)]
    width = len(tiles) * TILE + (len(tiles) - 1) * GAP
    row = Image.new("RGB", (width, 2 * BAND + TILE), BG)
    for i, (name, arr) in enumerate(tiles):
        x = i * (TILE + GAP)
        row.paste(_fit(arr), (x, 2 * BAND))
        uvo._caption(row, x + 4, BAND + 3, name)
    uvo._caption(row, 4, 3, caption)
    return row


def _stack(rows: list[Image.Image]) -> Image.Image:
    width = max(r.width for r in rows)
    height = sum(r.height for r in rows) + GAP * (len(rows) - 1) + BAND
    sheet = Image.new("RGB", (width, height), BG)
    y = 0
    for r in rows:
        sheet.paste(r, (0, y))
        y += r.height + GAP
    uvo._caption(sheet, 4, height - BAND + 3, FOOTER)
    return sheet


def _grid(cells: list[tuple[str, Image.Image]], cols: int = 4) -> Image.Image:
    rows_n = -(-len(cells) // cols)
    cell_h = BAND + TILE
    width = cols * TILE + (cols - 1) * GAP
    height = rows_n * cell_h + (rows_n - 1) * GAP + BAND
    sheet = Image.new("RGB", (width, height), BG)
    for idx, (caption, tile) in enumerate(cells):
        r, c = divmod(idx, cols)
        x, y = c * (TILE + GAP), r * (cell_h + GAP)
        sheet.paste(tile, (x, y + BAND))
        uvo._caption(sheet, x + 4, y + 3, caption)
    uvo._caption(sheet, 4, height - BAND + 3, FOOTER)
    return sheet


def render_sheets(bake_json: Path, out_dir: Path) -> list[Path]:
    """Write <family>.png per family and index.png into out_dir. Returns the written paths."""
    bake_json = Path(bake_json)
    looks = _load_json(bake_json).get("looks")
    if not looks:
        raise SheetsError(f"no looks in {bake_json}")
    base = bake_json.parent
    out_dir.mkdir(parents=True, exist_ok=True)
    parents: dict[str, np.ndarray] = {}
    families: dict[str, list[Image.Image]] = {}
    cells: list[tuple[str, Image.Image]] = []
    for look in looks:
        atlas, child = str(look["parent"]), str(look["child"])
        if atlas not in parents:
            parents[atlas] = _decode_file(base / "parents" / f"{atlas}.tpc")
        child_rgb = _decode_file(base / "tpc" / f"{child}.tpc")
        region_path = base / "regions" / f"{child}.png"
        if not region_path.is_file():
            raise SheetsError(f"missing {region_path}")
        region = uvo.read_mask(str(region_path))
        pct = 100.0 * float(look.get("changed_pixel_fraction", 0.0))
        caption = f"{look['id']} {atlas}->{child} changed {pct:.1f}%"
        families.setdefault(str(look.get("family") or "unfiled"), []).append(
            _row(caption, parents[atlas], child_rgb, region)
        )
        cells.append((f"{look['id']} {child} changed {pct:.1f}%", _fit(child_rgb)))
    written: list[Path] = []
    for fam, rows in families.items():
        path = out_dir / f"{_safe(fam)}.png"
        _stack(rows).save(path)
        written.append(path)
    index = out_dir / "index.png"
    _grid(cells).save(index)
    return written + [index]
