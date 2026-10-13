#!/usr/bin/env python3
"""Rasterise per-label UV island masks for a KotOR 2 model (K1-port texture overlay, U4).

  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python k1_port_uvmask.py build --model n_commm --atlas n_commm05 \\
      [--size 1024] [--size-h H] [--flip 0] --out DIR [--game DIR] [--mdl FILE --mdx FILE]

Why: the overlay needs texel masks per body part (torso, arms, head, ...) with no numpy or PIL, so this is pure Python
and runs under the patched pykotor venv (the stock .venv misreads K2 MDX offsets). Only mesh nodes whose UV set 1 lies
inside [0,1] count. NODE_RULES maps node names to labels. Writes DIR/<label>.png (8-bit grey, 255 = in mask, stored
row order) and DIR/masks.json. Exit: 0 written, 1 model or atlas unreadable, 2 bad arguments.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import logging
import math
import re
import struct
import sys
import zlib
from pathlib import Path
from types import SimpleNamespace

DEFAULT_GAME = Path("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II").expanduser()
EPS = 1e-9
TORSO_ARM_X = 0.21  # |centroid x| above this on a body node -> 'torso_arm' (probe threshold)
LEGS_Z = 0.80  # centroid z below this (and not torso_arm) on a body node -> 'legs' (probe threshold)

WHOLE, SPLIT, ARM = "whole", "split", "arm"
# Table of (regex searched case-insensitively in the node name, label, mode). First match wins.
#   SPLIT: every face of the node goes to <label> (the anchor coverage is the whole node). Faces with |centroid x|
#          above TORSO_ARM_X also go to 'torso_arm', and the rest with centroid z below LEGS_Z go to 'legs'. Those two
#          are nested sub-islands of <label>, so the overlaps report shows them against it.
#   ARM:   all arm nodes form one 'arms' island; larm/rarm are also written only when L and R UV sets differ.
#   WHOLE: every face of the node goes to <label>. Names matching no row become WHOLE islands named by sanitize().
NODE_RULES = [
    (r"^(torso|chest)", "torso", SPLIT),
    (r"^body", "body", SPLIT),
    (r"^head", "head", WHOLE),
    (r"^(arm|[lr]_?arm)", "arms", ARM),
    (r"shoulder", "shoulders", WHOLE),
]
ARM_LEFT = re.compile(r"^l_?arm|arm_?l$", re.IGNORECASE)
ARM_RIGHT = re.compile(r"^r_?arm|arm_?r$", re.IGNORECASE)


class UsageError(Exception):
    """Bad arguments. Exit 2."""


class InputError(Exception):
    """The game, model or atlas cannot be read. Exit 1."""


# ---------------------------------------------------------------- grouping (no pykotor needed)


def sanitize(name: str) -> str:
    s = re.sub(r"[^a-z0-9_]+", "_", name.lower()).strip("_")
    return s or "node"


def resolve_label(name: str) -> tuple[str, str]:
    for pattern, label, mode in NODE_RULES:
        if re.search(pattern, name, re.IGNORECASE):
            return label, mode
    return sanitize(name), WHOLE


def arm_side(name: str) -> str | None:
    if ARM_LEFT.search(name):
        return "L"
    if ARM_RIGHT.search(name):
        return "R"
    return None


def _add(groups: dict, label: str, tris: list, name: str) -> None:
    g = groups.setdefault(label, {"tris": [], "nodes": []})
    g["tris"].extend(tris)
    if name not in g["nodes"]:
        g["nodes"].append(name)


def _tri(mesh, f) -> tuple:
    uv = mesh.vertex_uv1
    return tuple((uv[i].x, uv[i].y) for i in (f.v1, f.v2, f.v3))


def _raster(tri, W: int, H: int, flip: int, grid: bytearray) -> None:
    """Set grid[row * W + col] = 255 for every pixel centre inside tri (UV space). flip: row = (1 - v) * H."""
    pts = [(u * W, ((1.0 - v) if flip else v) * H) for u, v in tri]
    (x0, y0), (x1, y1), (x2, y2) = pts
    minx = max(0, math.floor(min(x0, x1, x2)))
    maxx = min(W - 1, math.ceil(max(x0, x1, x2)))
    miny = max(0, math.floor(min(y0, y1, y2)))
    maxy = min(H - 1, math.ceil(max(y0, y1, y2)))
    denom = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2)
    if abs(denom) < 1e-12:
        return
    for py in range(miny, maxy + 1):
        cy = py + 0.5
        row = py * W
        for px in range(minx, maxx + 1):
            cx = px + 0.5
            a = ((y1 - y2) * (cx - x2) + (x2 - x1) * (cy - y2)) / denom
            b = ((y2 - y0) * (cx - x2) + (x0 - x2) * (cy - y2)) / denom
            c = 1.0 - a - b
            if a >= -EPS and b >= -EPS and c >= -EPS:
                grid[row + px] = 255


def _and_count(a: bytearray, b: bytearray) -> int:
    """Texels set in both masks. Each byte is 0 or 255, so one big-int AND counts them exactly."""
    n = len(a)
    both = int.from_bytes(a, "big") & int.from_bytes(b, "big")
    return both.to_bytes(n, "big").count(255)


def resolve_dims(size: int, size_h: int | None, atlas_wh: tuple[int, int]) -> tuple[int, int]:
    aw, ah = atlas_wh
    if size_h is None:
        if aw != ah:
            raise UsageError(f"atlas is {aw}x{ah}, not 1:1; pass --size-h to set the mask height")
        return size, size
    return size, size_h


def build_masks(nodes, size: int, flip: int, size_h: int | None = None) -> dict:
    """Group UV set 1 faces of duck-typed nodes (.name, .mesh.faces/.vertex_uv1/.vertex_positions) into labels.

    Returns {width, height, masks: {label: bytearray 0/255 in stored row order}, labels, skipped, overlaps}.
    Coverage = texels / (width * height). Overlap keys are "a&b" with a < b.
    """
    if flip not in (0, 1):
        raise UsageError("flip must be 0 or 1")
    W = int(size)
    H = int(size_h) if size_h else W
    if W < 1 or H < 1:
        raise UsageError("size must be >= 1")
    groups: dict[str, dict] = {}
    skipped: list[dict] = []
    side_tris: dict[str, list] = {"L": [], "R": []}
    side_nodes: dict[str, list] = {"L": [], "R": []}
    side_uv: dict[str, set] = {"L": set(), "R": set()}

    for node in nodes:
        mesh = getattr(node, "mesh", None)
        if mesh is None:
            continue
        name = str(getattr(node, "name", None) or "")
        faces = list(mesh.faces or [])
        uvs = list(mesh.vertex_uv1 or [])
        pos = list(mesh.vertex_positions or [])
        if not faces:
            skipped.append({"node": name, "reason": "no_faces"})
            continue
        if not uvs:
            skipped.append({"node": name, "reason": "no_uv"})
            continue
        if not all(0.0 <= t.x <= 1.0 and 0.0 <= t.y <= 1.0 for t in uvs):
            skipped.append({"node": name, "reason": "out_of_range"})
            continue
        tex = getattr(mesh, "texture_1", None)
        if isinstance(tex, str) and tex.strip().lower() in ("", "null"):
            # no texture on the node (n_mandalorian head_g): its UVs belong to another surface, not the atlas
            skipped.append({"node": name, "reason": "no_texture"})
            continue
        label, mode = resolve_label(name)
        limit = min(len(uvs), len(pos)) if mode == SPLIT else len(uvs)
        if any(i < 0 or i >= limit for f in faces for i in (f.v1, f.v2, f.v3)):
            skipped.append({"node": name, "reason": "bad_index"})
            continue
        tris = [_tri(mesh, f) for f in faces]
        if mode == ARM:
            _add(groups, "arms", tris, name)
            side = arm_side(name)
            if side:
                side_tris[side].extend(tris)
                side_nodes[side].append(name)
                side_uv[side].update((round(u, 6), round(v, 6)) for tri in tris for u, v in tri)
        elif mode == SPLIT:
            _add(groups, label, tris, name)
            buckets: dict[str, list] = {}
            for f, tri in zip(faces, tris):
                ids = (f.v1, f.v2, f.v3)
                cx = sum(pos[i].x for i in ids) / 3.0
                cz = sum(pos[i].z for i in ids) / 3.0
                key = "torso_arm" if abs(cx) > TORSO_ARM_X else ("legs" if cz < LEGS_Z else None)
                if key is not None:
                    buckets.setdefault(key, []).append(tri)
            for key, group_tris in buckets.items():
                _add(groups, key, group_tris, name)
        else:
            _add(groups, label, tris, name)

    if side_nodes["L"] and side_nodes["R"] and side_uv["L"] != side_uv["R"]:
        for side, label in (("L", "larm"), ("R", "rarm")):
            g = groups.setdefault(label, {"tris": [], "nodes": []})
            g["tris"].extend(side_tris[side])
            g["nodes"].extend(side_nodes[side])

    total = W * H
    masks: dict[str, bytearray] = {}
    for label in sorted(groups):
        grid = bytearray(total)
        for tri in groups[label]["tris"]:
            _raster(tri, W, H, flip, grid)
        masks[label] = grid
    labels = {}
    for label, grid in masks.items():
        texels = grid.count(255)
        labels[label] = {"coverage": round(texels / total, 6), "nodes": groups[label]["nodes"], "texels": texels}
    names = sorted(masks)
    overlaps = {f"{a}&{b}": _and_count(masks[a], masks[b]) for i, a in enumerate(names) for b in names[i + 1:]}
    return {"width": W, "height": H, "masks": masks, "labels": labels, "skipped": skipped, "overlaps": overlaps}


# ---------------------------------------------------------------- output


def write_png_grey(path, w: int, h: int, grid: bytearray) -> None:
    """8-bit greyscale PNG, rows in the order given (filter type 0 on every row)."""
    raw = bytearray()
    for y in range(h):
        raw.append(0)
        raw += grid[y * w:(y + 1) * w]

    def chunk(tag: bytes, data: bytes) -> bytes:
        body = tag + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", w, h, 8, 0, 0, 0, 0)
    Path(path).write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr)
                           + chunk(b"IDAT", zlib.compress(bytes(raw), 6)) + chunk(b"IEND", b""))


def write_outputs(out_dir, result: dict, meta: dict) -> dict:
    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    W, H = result["width"], result["height"]
    for label, grid in result["masks"].items():
        write_png_grey(out / f"{label}.png", W, H, grid)
    doc = {
        "model": meta["model"], "atlas": meta["atlas"], "size": meta["size"], "size_h": H, "flip": meta["flip"],
        "source": meta["source"], "labels": result["labels"], "skipped": result["skipped"],
        "overlaps": result["overlaps"],
    }
    (out / "masks.json").write_text(json.dumps(doc, indent=2) + "\n", encoding="utf-8")
    return doc


# ---------------------------------------------------------------- game access (pykotor imported lazily)


def tpc_dims(data: bytes) -> tuple[int, int]:
    """Width and height from a TPC header: uint32 data_size, float32 alpha, uint16 width, uint16 height, ..."""
    if len(data) < 128:
        raise InputError("atlas TPC is shorter than its 128-byte header")
    _, _, w, h, _, _ = struct.unpack_from("<IfHHBB", data, 0)
    return w, h


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def walk_nodes(node):
    yield node
    for child in node.children:
        yield from walk_nodes(child)


def load_pykotor() -> SimpleNamespace:
    logging.disable(logging.CRITICAL)  # pykotor logs heavily while opening an Installation
    try:
        from pykotor.extract.installation import Installation, SearchLocation
        from pykotor.resource.formats.mdl import read_mdl
        from pykotor.resource.type import ResourceType
    except ImportError as exc:
        raise InputError(f"pykotor not importable ({exc}); run with the patched venv "
                         "~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python") from exc
    return SimpleNamespace(Installation=Installation, SearchLocation=SearchLocation,
                           read_mdl=read_mdl, ResourceType=ResourceType)


def install_get(pk: SimpleNamespace, inst, name: str, rtype):
    order = [pk.SearchLocation.OVERRIDE, pk.SearchLocation.CHITIN, pk.SearchLocation.MODULES]
    return inst.resource(name, rtype, order)


def _is_within(path: Path, parent: Path) -> bool:
    try:
        path.resolve().relative_to(parent.resolve())
        return True
    except ValueError:
        return False


# ---------------------------------------------------------------- CLI


def run_build(args) -> int:
    if args.flip not in (0, 1):
        raise UsageError("--flip must be 0 or 1")
    if args.size < 1 or (args.size_h is not None and args.size_h < 1):
        raise UsageError("--size and --size-h must be >= 1")
    if args.model and (args.mdl or args.mdx):
        raise UsageError("use --model or --mdl/--mdx, not both")
    if bool(args.mdl) != bool(args.mdx):
        raise UsageError("--mdl and --mdx go together")
    if not args.model and not args.mdl:
        raise UsageError("give --model NAME or --mdl FILE --mdx FILE")
    game = Path(args.game).expanduser()
    out = Path(args.out).expanduser()
    if _is_within(out, game):
        raise UsageError(f"--out {out} is inside the game dir {game}; refused")

    pk = load_pykotor()
    if not game.is_dir():
        raise InputError(f"game dir not found: {game}")
    try:
        inst = pk.Installation(str(game))
    except Exception as exc:  # noqa: BLE001 - pykotor raises several error types for a bad dir
        raise InputError(f"cannot open the game dir {game}: {type(exc).__name__}: {exc}") from exc
    atlas_res = install_get(pk, inst, args.atlas, pk.ResourceType.TPC)
    if atlas_res is None:
        raise InputError(f"atlas {args.atlas} (TPC) not found in {game}")
    W, H = resolve_dims(args.size, args.size_h, tpc_dims(bytes(atlas_res.data)))

    if args.model:
        model = args.model
        mdl_res = install_get(pk, inst, model, pk.ResourceType.MDL)
        mdx_res = install_get(pk, inst, model, pk.ResourceType.MDX)
        if mdl_res is None or mdx_res is None:
            raise InputError(f"{model}: MDL/MDX not found in {game}")
        mdl_bytes, mdx_bytes = bytes(mdl_res.data), bytes(mdx_res.data)
    else:
        mdl_path, mdx_path = Path(args.mdl).expanduser(), Path(args.mdx).expanduser()
        model = mdl_path.stem
        try:
            mdl_bytes, mdx_bytes = mdl_path.read_bytes(), mdx_path.read_bytes()
        except OSError as exc:
            raise InputError(f"cannot read {mdl_path} / {mdx_path}: {exc}") from exc

    try:
        mdl = pk.read_mdl(mdl_bytes, source_ext=mdx_bytes, size_ext=len(mdx_bytes))
    except Exception as exc:  # noqa: BLE001 - pykotor raises several error types for a bad model
        raise InputError(f"read_mdl failed for {model}: {type(exc).__name__}: {exc}") from exc

    result = build_masks(walk_nodes(mdl.root), args.size, args.flip, size_h=H)
    meta = {"model": model, "atlas": args.atlas, "size": args.size, "flip": args.flip,
            "source": {"mdl_sha256": sha256(mdl_bytes), "mdx_sha256": sha256(mdx_bytes)}}
    doc = write_outputs(out, result, meta)
    for label, info in doc["labels"].items():
        print(f"{label}: coverage {info['coverage'] * 100:.2f}% texels {info['texels']}")
    print(f"wrote {len(result['masks'])} masks ({W}x{H}) and masks.json to {out}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="UV island masks per body-part label for the K1-port texture overlay.")
    sub = p.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build", help="rasterise the label masks for one model")
    b.add_argument("--model", help="model resref in the game install (for example n_commm)")
    b.add_argument("--mdl", help="MDL file, alternative to --model (used with --mdx)")
    b.add_argument("--mdx", help="MDX file, alternative to --model (used with --mdl)")
    b.add_argument("--atlas", required=True, help="atlas TPC resref; read from the game for its aspect ratio")
    b.add_argument("--size", type=int, default=1024, help="mask width and height in texels (default 1024)")
    b.add_argument("--size-h", type=int, default=None, help="mask height; required when the atlas is not 1:1")
    b.add_argument("--flip", type=int, default=0, help="1: row = (1 - v) * H; 0: row = v * H (default 0)")
    b.add_argument("--out", required=True, help="output directory (may not be inside the game dir)")
    b.add_argument("--game", default=str(DEFAULT_GAME), help="K2 install, read only")
    return p


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        return run_build(args)
    except UsageError as exc:
        print(f"usage error: {exc}", file=sys.stderr)
        return 2
    except InputError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
