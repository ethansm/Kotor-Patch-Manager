#!/usr/bin/env python3
"""Texture-look bake for the K1 texture port: island masks, recipe render with gated child TPCs, and review sheets.

Subcommands. `masks` builds island masks per model through the k1port venv (k1_port_uvmask.py build). `bake`
validates every recipe before writing anything, renders each look over its parent TPC (vanilla pack first, then the
stage pool), re-encodes only the blocks the region touches (apply_rgb_edit), names the child in the >= 36 resref range
per family prefix, writes OUT/tpc/<child>.tpc and runs the gate. `sheets` renders the review sheets (see
k1_port_tex_sheets.py). Output dirs must sit outside the repo, the game dir and the stage override.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
import k1_port_tex_dxt as tdx  # noqa: E402  stdlib + numpy
import k1_port_tex_gate as tg  # noqa: E402
import k1_port_tex_overlay as tov  # noqa: E402
import k1_port_tex_sheets as tsh  # noqa: E402
import k1_port_tex_tpc as tp  # noqa: E402
import k1_port_uvmask as uvm  # noqa: E402
import k1_port_uvmask_overlay as uvo  # noqa: E402

REPO = HERE.parents[1]
VENV_K1PORT = Path.home() / "Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python"
DEFAULT_OUT_ROOT = Path.home() / "Games/KOTOR2_Mod_Workspace/tex_looks"
DEFAULT_GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
POOL_DIR = REPO / "patch_manager_mods" / "npc_diversity_src" / "override"
ATLAS_FOR_MODEL = {
    "n_commm": "n_commm05",
    "n_commf": "n_commf02",
    "n_sithcommando": "n_sithcom01",
    "n_mandalorian": "n_mandalorian01",
}
FIRST_NN = 36
_PACK_INDEX: dict | None = None


class BakeError(Exception):
    """Operator-facing failure. `code` is the exit code: 1 gate failure, 2 usage, IO, invalid or unsupported input."""

    def __init__(self, message: str, code: int = 2):
        super().__init__(message)
        self.code = code


def _game(args: argparse.Namespace) -> Path:
    return Path(args.game).expanduser() if getattr(args, "game", None) else DEFAULT_GAME


def _check_out(out: Path, game: Path) -> Path:
    """Refuse output dirs inside the repo, the game dir or the stage override (checked before anything is written)."""
    target = out.expanduser().resolve()
    for label, root in (("repo", REPO), ("game dir", game), ("stage override", POOL_DIR)):
        root = root.expanduser().resolve()
        if target == root or root in target.parents:
            raise BakeError(f"refusing output {target}: it is inside the {label} {root}", 2)
    return target


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _recipe_sha(rec: dict) -> str:
    return _sha256(json.dumps(rec, sort_keys=True, separators=(",", ":")).encode("utf-8"))


def _load_json(path: Path, default):
    if not path.is_file():
        return default
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise BakeError(f"cannot read {path}: {exc}", 2) from exc


def load_recipes(path: Path, only: str | None) -> list[tuple[Path, dict]]:
    """A recipe file or a dir of *.json; each file holds a JSON list of recipe dicts or one dict."""
    if path.is_dir():
        files = sorted(path.glob("*.json"))
    elif path.is_file():
        files = [path]
    else:
        raise BakeError(f"recipes not found: {path}", 2)
    picked: list[tuple[Path, dict]] = []
    for f in files:
        try:
            data = json.loads(f.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            raise BakeError(f"cannot read recipes {f}: {exc}", 2) from exc
        for item in data if isinstance(data, list) else [data]:
            if not isinstance(item, dict):
                raise BakeError(f"{f}: every recipe must be a JSON object", 2)
            if only is None or str(item.get("id")) == only:
                picked.append((f, item))
    if not picked:
        raise BakeError(f"no recipes selected from {path}" + (f" with id {only!r}" if only else ""), 2)
    return picked


def _no_pack(name: str) -> None:
    return None


def _default_pack_lookup(name: str) -> bytes | None:
    """Vanilla pack lookup via scripts/npc_div_texture_sheets._pack_index (needs pykotor from .venv)."""
    global _PACK_INDEX
    if _PACK_INDEX is None:
        scripts = str(REPO / "scripts")
        if scripts not in sys.path:
            sys.path.insert(0, scripts)
        import npc_div_texture_sheets as nd  # noqa: lazy import, needs pykotor

        _PACK_INDEX = nd._pack_index()
    hit = _PACK_INDEX.get(name.lower())
    return None if hit is None else bytes(hit[1].data)


def resolve_parent(atlas: str, pool_dir: Path, pack_lookup=None) -> tuple[bytes, str]:
    """Vanilla pack first, then <pool_dir>/<Atlas>.tpc matched case-insensitively. Returns (bytes, source label)."""
    lookup = _default_pack_lookup if pack_lookup is None else pack_lookup
    pack_err = ""
    try:
        data = lookup(atlas)
    except Exception as exc:  # pykotor missing or pack unreadable: fall through to the pool
        data = None
        pack_err = f" (vanilla pack lookup failed: {type(exc).__name__}: {exc})"
    if data is not None:
        return bytes(data), f"pack:{atlas}"
    if pool_dir.is_dir():
        want = f"{atlas}.tpc".lower()
        for p in sorted(pool_dir.iterdir()):
            if p.is_file() and p.name.lower() == want:
                return p.read_bytes(), f"pool:{p.name}"
    raise BakeError(f"parent {atlas} not found in the vanilla pack or {pool_dir}{pack_err}", 2)


def load_islands(masks_root: Path, model: str, shape: tuple[int, int], cache: dict) -> dict[str, np.ndarray]:
    """Island masks for a model from masks_root/masks/<model>/<label>.png, labels taken from masks.json."""
    key = (model, shape)
    if key in cache:
        return cache[key]
    mdir = masks_root / "masks" / model
    meta = _load_json(mdir / "masks.json", None)
    if not isinstance(meta, dict) or not isinstance(meta.get("labels"), dict):
        raise BakeError(f"no usable {mdir / 'masks.json'}; run the masks subcommand for {model} first", 2)
    islands: dict[str, np.ndarray] = {}
    for label in meta["labels"]:
        png = mdir / f"{label}.png"
        if not png.is_file():
            raise BakeError(f"missing island mask {png}", 2)
        mask = uvo.read_mask(str(png))
        if mask.shape != shape:
            raise BakeError(
                f"island mask {png} is {mask.shape[1]}x{mask.shape[0]} but the atlas is {shape[1]}x{shape[0]}", 2
            )
        islands[label] = mask
    cache[key] = islands
    return islands


def _child_name(look_id: str, atlas: str, tpc_dir: Path, alloc: dict, prev: dict) -> str:
    """Re-bakes reuse the child name recorded for the same recipe id; new looks take the next free NN >= 36."""
    old = prev.get(look_id)
    if old and old.get("child"):
        return str(old["child"])
    prefix = re.sub(r"\d+$", "", atlas)
    nn = max(int(alloc.get(prefix, FIRST_NN - 1)) + 1, FIRST_NN)
    while (tpc_dir / f"{prefix}{nn:02d}.tpc").exists():
        nn += 1
    alloc[prefix] = nn
    return f"{prefix}{nn:02d}"


def _decode_rgb(b: bytes) -> np.ndarray:
    info = tp.parse(b)
    return np.ascontiguousarray(tp.decode_mip(b, info, 0)[:, :, :3])


def _bake_one(rec: dict, out: Path, alloc: dict, prev: dict, pool_dir: Path, masks_root: Path,
              pack_lookup, cache: dict) -> dict:
    rid, atlas, model = str(rec["id"]), str(rec["atlas"]), str(rec["model"])
    parent_bytes, source = resolve_parent(atlas, pool_dir, pack_lookup)
    try:
        info = tp.parse(parent_bytes)
    except ValueError as exc:
        raise BakeError(f"recipe {rid}: parent {atlas} ({source}) is not a sized DXT TPC: {exc}", 2) from exc
    parent_rgb = _decode_rgb(parent_bytes)
    h, w = parent_rgb.shape[:2]
    islands = load_islands(masks_root, model, (h, w), cache)
    try:
        child_rgb, union = tov.render_recipe(rec, parent_rgb, islands)
    except (KeyError, ValueError) as exc:
        raise BakeError(f"recipe {rid}: render failed: {exc}", 2) from exc
    union = np.asarray(union, dtype=bool)
    child_rgb = np.asarray(child_rgb, dtype=np.uint8)
    edit = union & np.any(child_rgb != parent_rgb, axis=2)  # re-encode only pixels a layer really changed
    child_bytes = tp.apply_rgb_edit(parent_bytes, child_rgb, edit)
    child = _child_name(rid, atlas, out / "tpc", alloc, prev)
    (out / "tpc" / f"{child}.tpc").write_bytes(child_bytes)
    (out / "parents" / f"{atlas}.tpc").write_bytes(parent_bytes)
    union_u8 = union.astype(np.uint8) * 255  # sheets show the region; the gate checks the edited pixels
    edit_u8 = edit.astype(np.uint8) * 255
    uvm.write_png_grey(str(out / "regions" / f"{child}.png"), w, h, bytearray(union_u8.tobytes()))
    siblings = sorted(n for n in os.listdir(out / "tpc") if n != f"{child}.tpc")
    raw_gate = tg.gate(parent_bytes, child_bytes, child, siblings, (w, h, edit_u8.tobytes()))
    gate = [{"id": str(g["id"]), "ok": bool(g["ok"]), "detail": str(g.get("detail", ""))} for g in raw_gate]
    changed_px = float(np.any(_decode_rgb(child_bytes) != parent_rgb, axis=2).mean())
    bb = tdx.BLOCK_BYTES[info["fmt"]]
    n_blocks = ((w + 3) // 4) * ((h + 3) // 4)
    o0, s0 = info["offsets"][0], info["sizes"][0]
    pa = np.frombuffer(parent_bytes, dtype=np.uint8, count=s0, offset=o0).reshape(n_blocks, bb)
    pc = np.frombuffer(child_bytes, dtype=np.uint8, count=s0, offset=o0).reshape(n_blocks, bb)
    changed_blocks = float(np.any(pa != pc, axis=1).mean())
    decals = [str(layer.get("params", {}).get("name")) for layer in rec["layers"] if layer.get("op") == "decal"]
    return {
        "id": rid,
        "family": str(rec.get("family", "")),
        "recipe_sha256": _recipe_sha(rec),
        "parent": atlas,
        "parent_sha256": _sha256(parent_bytes),
        "child": child,
        "child_sha256": _sha256(child_bytes),
        "changed_pixel_fraction": round(changed_px, 6),
        "changed_block_fraction": round(changed_blocks, 6),
        "decals": decals,
        "gate": gate,
        "ok": bool(gate) and all(g["ok"] for g in gate),
    }


def cmd_bake(args: argparse.Namespace) -> int:
    out = _check_out(Path(args.out), _game(args))
    pool_dir = Path(args.pool_dir).expanduser() if args.pool_dir else POOL_DIR
    masks_root = Path(args.masks).expanduser()
    recipes = load_recipes(Path(args.recipes).expanduser(), args.only)
    problems: list[str] = []
    ids: set[str] = set()
    for src, rec in recipes:
        rid = str(rec.get("id", "?"))
        if rid in ids:
            problems.append(f"{rid} ({src.name}): duplicate recipe id")
        ids.add(rid)
        problems += [f"{rid} ({src.name}): {rule}" for rule in tov.validate_recipe(rec)]
    if problems:
        raise BakeError("invalid recipe(s), nothing written:\n  " + "\n  ".join(problems), 2)
    for sub in ("tpc", "parents", "regions"):
        (out / sub).mkdir(parents=True, exist_ok=True)
    alloc = _load_json(out / "alloc.json", {})
    prev_looks = [e for e in _load_json(out / "bake.json", {}).get("looks", []) if isinstance(e, dict)]
    prev = {str(e.get("id")): e for e in prev_looks}
    lookup = _no_pack if args.no_pack else None
    cache: dict = {}
    results: list[dict] = []
    failures: list[str] = []
    for _src, rec in recipes:
        try:
            results.append(_bake_one(rec, out, alloc, prev, pool_dir, masks_root, lookup, cache))
        except BakeError as exc:
            failures.append(f"{rec.get('id')}: {exc}")
    (out / "alloc.json").write_text(json.dumps(alloc, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    new = {e["id"]: e for e in results}
    merged: list[dict] = []
    seen: set[str] = set()
    for e in prev_looks:
        eid = str(e.get("id"))
        merged.append(new.get(eid, e))
        seen.add(eid)
    merged += [e for e in results if e["id"] not in seen]
    run = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    (out / "bake.json").write_text(json.dumps({"run": run, "looks": merged}, indent=2) + "\n", encoding="utf-8")
    for e in results:
        pct = 100.0 * e["changed_pixel_fraction"]
        print(f"{e['id']}: {e['parent']} -> {e['child']} changed {pct:.1f}% gate {'PASS' if e['ok'] else 'FAIL'}")
        for g in e["gate"]:
            if not g["ok"]:
                print(f"  gate {g['id']}: {g['detail']}")
    for msg in failures:
        print(f"ERROR: {msg}", file=sys.stderr)
    if failures:
        return 2
    return 1 if any(not e["ok"] for e in results) else 0


def cmd_masks(args: argparse.Namespace) -> int:
    out = _check_out(Path(args.out), _game(args))
    specs: list[tuple[str, int]] = []
    for item in args.models:
        model, sep, size = item.partition(":")
        if model not in ATLAS_FOR_MODEL or not sep or not size.isdigit():
            raise BakeError(
                f"bad --models entry {item!r}: use MODEL:SIZE, MODEL one of {', '.join(sorted(ATLAS_FOR_MODEL))}", 2
            )
        specs.append((model, int(size)))
    if not VENV_K1PORT.is_file():
        raise BakeError(f"k1port venv python not found: {VENV_K1PORT}", 2)
    for model, size in specs:
        mdir = out / "masks" / model
        if (mdir / "masks.json").is_file() and not args.force:
            print(f"skip {model}: {mdir / 'masks.json'} exists (use --force to rebuild)")
            continue
        mdir.mkdir(parents=True, exist_ok=True)
        cmd = [
            str(VENV_K1PORT), str(HERE / "k1_port_uvmask.py"), "build", "--model", model,
            "--atlas", ATLAS_FOR_MODEL[model], "--size", str(size), "--flip", "0", "--out", str(mdir),
        ]
        print(f"build {model} {size}px -> {mdir}", flush=True)
        rc = subprocess.run(cmd).returncode
        if rc != 0:
            raise BakeError(f"mask build for {model} failed with exit {rc}", 1)
    return 0


def cmd_sheets(args: argparse.Namespace) -> int:
    out = _check_out(Path(args.out), DEFAULT_GAME)
    try:
        paths = tsh.render_sheets(Path(args.bake).expanduser() / "bake.json", out)
    except tsh.SheetsError as exc:
        raise BakeError(str(exc), 2) from exc
    for path in paths:
        print(path)
    return 0


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description="Texture-look bake: masks, bake (recipes -> gated child TPCs), sheets.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    m = sub.add_parser("masks", help="build island masks per model through the k1port venv")
    m.add_argument("--models", nargs="+", required=True, metavar="MODEL:SIZE")
    m.add_argument("--out", required=True, help=f"output root (suggested {DEFAULT_OUT_ROOT})")
    m.add_argument("--force", action="store_true", help="rebuild models that already have masks.json")
    m.add_argument("--game", default=None, help="game dir to refuse as output (default: Steam install)")
    b = sub.add_parser("bake", help="validate recipes, render looks, gate the children")
    b.add_argument("--recipes", required=True, help="recipe JSON file, or a dir of *.json (list or single dict)")
    b.add_argument("--masks", required=True, help="mask root holding masks/<model>/ (the masks --out)")
    b.add_argument("--out", required=True)
    b.add_argument("--only", default=None, help="bake only this recipe id")
    b.add_argument("--pool-dir", default=None, help=f"stage pool of parent TPCs (default {POOL_DIR})")
    b.add_argument("--game", default=None, help="game dir to refuse as output (default: Steam install)")
    b.add_argument("--no-pack", action="store_true", help="skip the vanilla pack; parents come from --pool-dir")
    s = sub.add_parser("sheets", help="render review sheets from a bake dir")
    s.add_argument("--bake", required=True, help="bake output dir (holds bake.json)")
    s.add_argument("--out", required=True)
    return ap


def main(argv: list[str] | None = None) -> int:
    try:
        args = build_parser().parse_args(argv)
    except SystemExit as exc:
        return int(exc.code or 0)
    try:
        if args.cmd == "masks":
            return cmd_masks(args)
        if args.cmd == "bake":
            return cmd_bake(args)
        return cmd_sheets(args)
    except BakeError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return exc.code


if __name__ == "__main__":
    sys.exit(main())
