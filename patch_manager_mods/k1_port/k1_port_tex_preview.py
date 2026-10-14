#!/usr/bin/env python3
"""Offline 3D preview of baked texture looks: the real model meshes textured with parent and child atlases.

  mesh    (k1port venv)  dump a model's textured meshes (world space, UV set 1) to JSON
  render  (.venv)        render look PNGs from a bake dir: parent vs child, several views, plus contact sheets

  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python k1_port_tex_preview.py mesh --models n_commm n_commf --out DIR
  .venv/bin/python -I k1_port_tex_preview.py render --bake RUN --meshes DIR --out DIR [--ids A B] [--views 0 180 90 35]
        [--mode gap] [--gif]            quick yaw sheets + contact sheets
  .venv/bin/python -I k1_port_tex_preview.py helmets --meshes DIR --out DIR [--appearances ROWS] [--items MODELS]
        head-item previews: per appearance row a sheet (bare head, then one row per helmet model) + a family wall;
        `mesh --textures` first for the bodies, heads and I_Mask_* models (needs hooks: headhook, GoggleHook)
  .venv/bin/python -I k1_port_tex_preview.py dossier --bake RUN --meshes DIR --out DIR [--ids A B] [--shots ...]
        [--wall ...] [--size big] [--mode gap]    <id>_dossier.png (10 zooms/angles, parent above look) + wall_<family>_<shot>.png

Software rasteriser (numpy + PIL): orthographic camera, z-buffer, double-sided half-lambert shading, 2x supersampling,
texture sampled at row = v * H, column = u * W in stored row order (the --flip 0 convention of the masks). Heads are
separate models in KOTOR II, so characters show without a head; nodes with a NULL or out-of-range texture are skipped.
Nothing here touches the game dir; outputs go under --out only.
"""
from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path

DEFAULT_GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
VIEWS = (0, 180, 90, 35)  # yaw degrees: front, back, character's left side, three-quarter front
TILE_W, TILE_H = 240, 380
SS = 2  # supersampling factor


# ------------------------------------------------------------------ mesh dump (needs the patched pykotor venv)


def _quat_matrix(q):
    x, y, z, w = q
    n = math.sqrt(x * x + y * y + z * z + w * w) or 1.0
    x, y, z, w = x / n, y / n, z / n, w / n
    return [[1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)]]


def _compose(parent, pos, quat):
    """parent = (R, t); child local = (quat, pos). Returns the child's world (R, t)."""
    pr, pt = parent
    cr = _quat_matrix(quat)
    r = [[sum(pr[i][k] * cr[k][j] for k in range(3)) for j in range(3)] for i in range(3)]
    t = [pt[i] + sum(pr[i][k] * pos[k] for k in range(3)) for i in range(3)]
    return r, t


def open_install(game: Path):
    """(pykotor namespace, Installation): the index build is the slow part, so `mesh` shares one across models."""
    import k1_port_uvmask as uvm  # noqa: E402

    pk = uvm.load_pykotor()
    return pk, pk.Installation(str(game))


def _texture_get(pk, inst, name: str, rtype):
    """Like uvmask.install_get, then the TexturePacks (most character skins live in swpc_tex_tpa/tpb/tpc)."""
    import k1_port_uvmask as uvm  # noqa: E402

    res = uvm.install_get(pk, inst, name, rtype)
    if res is None:
        sl = pk.SearchLocation
        res = inst.resource(name, rtype, [sl.TEXTURES_TPA, sl.TEXTURES_TPB, sl.TEXTURES_TPC])
    return res


class ModelNotFound(Exception):
    """The install has no MDL/MDX pair for this resref."""


def dump_model(model: str, game: Path, tex_dir: Path | None = None, install=None,
               keep_wrapped: bool = False) -> dict:
    """Textured meshes (world space, UV set 1) plus `hooks`: every mesh-less node as {R, t} in model space
    (GoggleHook, headhook, ...). With tex_dir, each used texture is copied there as raw <resref>.tpc/.tga bytes
    (decoded later in the numpy venv by load_texture); the doc lists them under `textures`."""
    import k1_port_uvmask as uvm  # noqa: E402  pure python, needs pykotor only for load_pykotor

    pk, inst = install or open_install(game)
    mdl_res = uvm.install_get(pk, inst, model, pk.ResourceType.MDL)
    mdx_res = uvm.install_get(pk, inst, model, pk.ResourceType.MDX)
    if mdl_res is None or mdx_res is None:
        raise ModelNotFound(f"{model}: MDL/MDX not found in {game}")
    mdx = bytes(mdx_res.data)
    mdl = pk.read_mdl(bytes(mdl_res.data), source_ext=mdx, size_ext=len(mdx))
    nodes, skipped, hooks, nodes_all, refs = [], [], {}, {}, {}
    ident = ([[1, 0, 0], [0, 1, 0], [0, 0, 1]], [0.0, 0.0, 0.0])

    def walk(node, parent):
        p, o = node.position, node.orientation
        world = _compose(parent, (p.x, p.y, p.z), (o.x, o.y, o.z, o.w))
        mesh = getattr(node, "mesh", None)
        name = str(node.name)
        nodes_all.setdefault(name, {"R": [list(map(float, row)) for row in world[0]], "t": [float(x) for x in world[1]],
                                    "mesh": mesh is not None,
                                    "faces": len(getattr(mesh, "faces", None) or []) if mesh is not None else 0,
                                    "texture": str(getattr(mesh, "texture_1", "") or "").strip() if mesh is not None else ""})
        ref = getattr(getattr(node, "reference", None), "model", "")
        if ref:
            refs.setdefault(name, str(ref))  # REFERENCE node: another model hangs here (saber hilts)
        if mesh is None:
            hooks.setdefault(name, {"R": [list(map(float, row)) for row in world[0]], "t": [float(x) for x in world[1]]})
        else:
            tex = str(getattr(mesh, "texture_1", "") or "").strip()
            pos, uvs, faces = list(mesh.vertex_positions or []), list(mesh.vertex_uv1 or []), list(mesh.faces or [])
            if not faces or not uvs or tex.lower() in ("", "null"):
                skipped.append({"node": name, "reason": "no faces, uv or texture"})
            elif not keep_wrapped and not all(0.0 <= t.x <= 1.0 and 0.0 <= t.y <= 1.0 for t in uvs):
                skipped.append({"node": name, "reason": "uv out of range"})
            elif len(pos) < len(uvs):
                skipped.append({"node": name, "reason": "fewer positions than uvs"})
            else:
                r, t = world
                wp = [[sum(r[i][k] * v[k] for k in range(3)) + t[i] for i in range(3)]
                      for v in ((q.x, q.y, q.z) for q in pos[:len(uvs)])]
                node_doc = {"name": name, "texture": tex, "positions": wp,
                            "uvs": [[u.x, u.y] for u in uvs], "faces": [[f.v1, f.v2, f.v3] for f in faces]}
                if not all(0.0 <= t.x <= 1.0 and 0.0 <= t.y <= 1.0 for t in uvs):
                    node_doc["uv_wrap"] = True  # tiling UVs: load_parts wraps them, the atlas loaders skip the node
                nodes.append(node_doc)
        for child in node.children:
            walk(child, world)

    walk(mdl.root, ident)
    doc = {"model": model, "nodes": nodes, "skipped": skipped, "hooks": hooks, "nodes_all": nodes_all, "refs": refs,
           "textures": {}}
    for tex in sorted({n["texture"].lower() for n in nodes}):
        for ext, rtype in (("tga", pk.ResourceType.TGA), ("tpc", pk.ResourceType.TPC)):
            res = _texture_get(pk, inst, tex, rtype)
            if res is not None:
                doc["textures"][tex] = ext
                if tex_dir is not None:
                    tex_dir.mkdir(parents=True, exist_ok=True)
                    (tex_dir / f"{tex}.{ext}").write_bytes(bytes(res.data))
                break
    return doc


def cmd_mesh(args) -> int:
    out = Path(args.out).expanduser()
    out.mkdir(parents=True, exist_ok=True)
    install = open_install(Path(args.game).expanduser())
    failed, missing = [], []
    for model in args.models:
        try:
            doc = dump_model(model, Path(args.game).expanduser(), out / "tex" if args.textures else None, install,
                             keep_wrapped=args.textures)
        except ModelNotFound:
            missing.append(model)
            print(f"{model}: NOT FOUND")
            continue
        except (ValueError, OverflowError, KeyError, IndexError) as exc:  # a model pykotor cannot parse (K1 NaN planes)
            failed.append(model)
            print(f"{model}: FAILED {type(exc).__name__}: {exc}", file=sys.stderr)
            continue
        (out / f"{model}.json").write_text(json.dumps(doc), encoding="utf-8")
        tris = sum(len(n["faces"]) for n in doc["nodes"])
        reasons: dict = {}
        for sk in doc["skipped"]:
            reasons[sk["reason"]] = reasons.get(sk["reason"], 0) + 1
        print(f"{model}: {len(doc['nodes'])} textured nodes, {tris} triangles, {len(doc['hooks'])} hooks, "
              f"skipped {reasons or 0}")
    if missing:
        print(f"{len(missing)} model(s) not found: {' '.join(missing)}", file=sys.stderr)
    if failed:
        print(f"{len(failed)} model(s) failed to parse: {' '.join(failed)}", file=sys.stderr)
    return 0 if len(missing) + len(failed) < len(args.models) else 1


# ------------------------------------------------------------------ renderer (numpy + PIL)


def _np():
    import numpy as np

    return np


def load_mesh(path: Path):
    """(positions (n,3), uvs (n,2), faces (m,3)) merged over every textured node of a mesh dump."""
    np = _np()
    doc = json.loads(Path(path).read_text(encoding="utf-8"))
    pos, uvs, faces, base = [], [], [], 0
    for node in doc["nodes"]:
        if node.get("uv_wrap"):
            continue
        pos += node["positions"]
        uvs += node["uvs"]
        faces += [[a + base, b + base, c + base] for a, b, c in node["faces"]]
        base += len(node["uvs"])
    return (np.asarray(pos, dtype=np.float64), np.asarray(uvs, dtype=np.float64), np.asarray(faces, dtype=np.int64))


def _sample(tex, u, v):
    """Bilinear sample of tex (h,w,3) float at uv arrays; row = v * H, col = u * W."""
    np = _np()
    h, w = tex.shape[:2]
    x = np.clip(u * w - 0.5, 0, w - 1)
    y = np.clip(v * h - 0.5, 0, h - 1)
    x0, y0 = np.floor(x).astype(int), np.floor(y).astype(int)
    x1, y1 = np.minimum(x0 + 1, w - 1), np.minimum(y0 + 1, h - 1)
    fx, fy = (x - x0)[:, None], (y - y0)[:, None]
    top = tex[y0, x0] * (1 - fx) + tex[y0, x1] * fx
    bot = tex[y1, x0] * (1 - fx) + tex[y1, x1] * fx
    return top * (1 - fy) + bot * fy


def render(mesh, tex_rgb_u8, yaw_deg: float, width: int = TILE_W, height: int = TILE_H, bounds=None):
    """Render one view; returns an (height, width, 3) uint8 image. bounds = (x0, x1, z0, z1) fixes the framing."""
    pos, uvs, faces = mesh
    return render_scene([(pos, uvs, faces, tex_rgb_u8)], yaw_deg, width, height, bounds)


def render_scene(parts, yaw_deg: float, width: int = TILE_W, height: int = TILE_H, bounds=None):
    """Render several (positions, uvs, faces, tex_rgb_u8) parts into one image with a shared z-buffer."""
    np = _np()
    a = math.radians(yaw_deg)
    ca, sa = math.cos(a), math.sin(a)
    prep = []
    for pos, uvs, faces, tex_u8 in parts:
        px = pos[:, 0] * ca - pos[:, 1] * sa  # rotate about z (up)
        py = pos[:, 0] * sa + pos[:, 1] * ca  # py = depth towards the camera at yaw 0 (character faces +y)
        prep.append((px, py, pos[:, 2], uvs, faces, np.asarray(tex_u8, dtype=np.float64) / 255.0))
    if bounds is None:
        sxs = np.concatenate([-p[0] for p in prep])
        zs = np.concatenate([p[2] for p in prep])
        bounds = (sxs.min(), sxs.max(), zs.min(), zs.max())  # viewer at +y sees the character's right on the left
    x0, x1, z0, z1 = bounds
    W, H = width * SS, height * SS
    scale = 0.92 * min(W / max(x1 - x0, 1e-6), H / max(z1 - z0, 1e-6))
    cx, cz = (x0 + x1) / 2, (z0 + z1) / 2
    img = np.zeros((H, W, 3), dtype=np.float64)
    img[:] = np.linspace(0.30, 0.18, H)[:, None, None]  # soft vertical backdrop
    zbuf = np.full((H, W), -1e9)
    light = np.array([-0.35, 0.8, 0.5])
    light /= np.linalg.norm(light)
    for px, py, pz, uvs, faces, tex in prep:
        sx = (-px - cx) * scale + W / 2
        sy = H / 2 - (pz - cz) * scale
        p3 = np.stack([-px, py, pz], axis=1)
        fx, fy = sx[faces], sy[faces]  # drop faces entirely outside the image before the python loop
        keep = ~((fx.max(axis=1) < 0) | (fx.min(axis=1) > W) | (fy.max(axis=1) < 0) | (fy.min(axis=1) > H))
        for f in faces[keep]:
            i0, i1, i2 = f
            xs, ys = sx[[i0, i1, i2]], sy[[i0, i1, i2]]
            bx0, bx1 = max(int(math.floor(xs.min())), 0), min(int(math.ceil(xs.max())), W - 1)
            by0, by1 = max(int(math.floor(ys.min())), 0), min(int(math.ceil(ys.max())), H - 1)
            if bx1 < bx0 or by1 < by0:
                continue
            d = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2])
            if abs(d) < 1e-9:
                continue
            gx, gy = np.meshgrid(np.arange(bx0, bx1 + 1) + 0.5, np.arange(by0, by1 + 1) + 0.5)
            l0 = ((ys[1] - ys[2]) * (gx - xs[2]) + (xs[2] - xs[1]) * (gy - ys[2])) / d
            l1 = ((ys[2] - ys[0]) * (gx - xs[2]) + (xs[0] - xs[2]) * (gy - ys[2])) / d
            l2 = 1 - l0 - l1
            inside = (l0 >= -1e-4) & (l1 >= -1e-4) & (l2 >= -1e-4)
            if not inside.any():
                continue
            depth = l0 * p3[i0, 1] + l1 * p3[i1, 1] + l2 * p3[i2, 1]
            sub = zbuf[by0:by1 + 1, bx0:bx1 + 1]
            win = inside & (depth > sub)
            if not win.any():
                continue
            u = (l0 * uvs[i0, 0] + l1 * uvs[i1, 0] + l2 * uvs[i2, 0])[win]
            v = (l0 * uvs[i0, 1] + l1 * uvs[i1, 1] + l2 * uvs[i2, 1])[win]
            n = np.cross(p3[i1] - p3[i0], p3[i2] - p3[i0])
            nn = np.linalg.norm(n)
            shade = 0.55
            if nn > 1e-12:
                n = n / nn
                if n[1] < 0:
                    n = -n  # double-sided: face the camera
                shade = 0.5 + 0.5 * max(0.0, float(n @ light))
            sub[win] = depth[win]
            img[by0:by1 + 1, bx0:bx1 + 1][win] = np.clip(_sample(tex, u, v) * shade * 1.12, 0, 1)
    out = img.reshape(height, SS, width, SS, 3).mean(axis=(1, 3))
    return np.clip(out * 255 + 0.5, 0, 255).astype(np.uint8)


# ------------------------------------------------------------------ scene parts (several models, own textures)


def load_texture(tex_dir: Path, resref: str):
    """(h, w, 3) uint8 from <tex_dir>/<resref>.tga|.tpc as written by `mesh --textures`; None when absent or
    unsupported (the caller draws a flat grey instead)."""
    np = _np()
    base = Path(tex_dir) / resref.lower()
    tga, tpc = base.with_suffix(".tga"), base.with_suffix(".tpc")
    if tga.is_file():
        from PIL import Image

        arr = np.asarray(Image.open(tga).convert("RGB"))
        # PIL returns top-row-first, undoing a bottom-origin TGA's row order; sampling expects the stored order
        # (same as a TPC), so put a bottom-origin file back (descriptor bit 5 clear)
        with tga.open("rb") as fh:
            head = fh.read(18)
        return arr if len(head) == 18 and head[17] & 0x20 else np.ascontiguousarray(arr[::-1])
    if tpc.is_file():
        blob = tpc.read_bytes()
        try:
            return _Textures._rgb(blob)
        except ValueError:
            pass
        w, h, ptype = int.from_bytes(blob[8:10], "little"), int.from_bytes(blob[10:12], "little"), blob[12]
        bpp = {2: 3, 4: 4}.get(ptype)
        if int.from_bytes(blob[0:4], "little") == 0 and bpp and len(blob) >= 128 + w * h * bpp:
            raw = np.frombuffer(blob, dtype=np.uint8, count=w * h * bpp, offset=128).reshape(h, w, bpp)
            return np.ascontiguousarray(raw[:, :, :3])
    return None


def load_parts(path: Path, tex_dir: Path | None, transform=None, fallback=(150, 150, 150)):
    """One (positions, uvs, faces, tex) part per textured node of a mesh dump. transform = (R, t) places the
    model: pos' = R @ pos + t. A node whose texture is missing gets a flat `fallback` colour texture."""
    np = _np()
    doc = json.loads(Path(path).read_text(encoding="utf-8"))
    parts, cache = [], {}
    for node in doc["nodes"]:
        pos = np.asarray(node["positions"], dtype=np.float64)
        if transform is not None:
            r, t = np.asarray(transform[0], dtype=np.float64), np.asarray(transform[1], dtype=np.float64)
            pos = pos @ r.T + t
        name = node["texture"].lower()
        if name not in cache:
            tex = load_texture(tex_dir, name) if tex_dir else None
            cache[name] = tex if tex is not None else np.full((4, 4, 3), fallback, dtype=np.uint8)
        uvs = np.asarray(node["uvs"], dtype=np.float64)
        if node.get("uv_wrap"):
            uvs = uvs % 1.0
        parts.append((pos, uvs, np.asarray(node["faces"], dtype=np.int64), cache[name]))
    return parts


def hook_transform(path: Path, names):
    """(R, t) of the first of `names` found among the dump's hooks (case-insensitive), else None."""
    hooks = {k.lower(): v for k, v in json.loads(Path(path).read_text(encoding="utf-8")).get("hooks", {}).items()}
    for n in names:
        if n.lower() in hooks:
            return hooks[n.lower()]["R"], hooks[n.lower()]["t"]
    return None


def compose_transforms(outer, inner):
    """(R, t) applying `inner` first, then `outer`."""
    np = _np()
    ro, to = np.asarray(outer[0], dtype=np.float64), np.asarray(outer[1], dtype=np.float64)
    ri, ti = np.asarray(inner[0], dtype=np.float64), np.asarray(inner[1], dtype=np.float64)
    return (ro @ ri).tolist(), (ro @ ti + to).tolist()


def parts_bounds(parts, yaw_deg: float, pad: float = 0.12):
    """Framing (x0, x1, z0, z1) around the union of the parts for one yaw, padded by a fraction of the larger side."""
    np = _np()
    a = math.radians(yaw_deg)
    sx = np.concatenate([-(p[0][:, 0] * math.cos(a) - p[0][:, 1] * math.sin(a)) for p in parts])
    z = np.concatenate([p[0][:, 2] for p in parts])
    w, h = float(sx.max() - sx.min()), float(z.max() - z.min())
    m = pad * max(w, h, 1e-6)
    return (float(sx.min()) - m, float(sx.max()) + m, float(z.min()) - m, float(z.max()) + m)


def view_bounds(mesh, yaws):
    """One framing for all views so parent and child tiles line up: the union over the requested yaws."""
    np = _np()
    pos = mesh[0]
    xs, zs = [], pos[:, 2]
    for y in yaws:
        a = math.radians(y)
        xs.append(-(pos[:, 0] * math.cos(a) - pos[:, 1] * math.sin(a)))
    allx = np.concatenate(xs)
    half = max(abs(allx.min()), abs(allx.max()))
    return (-half, half, zs.min(), zs.max())



# ------------------------------------------------------------------ camera shots (zoomed framings of one mesh)

SHOT_W, SHOT_H = 220, 300
# name: (yaw degrees, z_lo, z_hi, half). z_* are fractions of the mesh height (0 = soles, 1 = top of the mesh; the
# commoner meshes have no head, so there 1 is the collar). When framing, half "left"/"right" keeps only the vertices
# outside the central 33% on that screen side (an arm close-up) and "core" only those inside it (a torso close-up).
# Screen-left at yaw 0 is the character's right side.
SHOTS = {
    "full_front": (0, 0.0, 1.0, None),
    "full_back": (180, 0.0, 1.0, None),
    "full_side": (90, 0.0, 1.0, None),
    "full_3q": (35, 0.0, 1.0, None),
    "torso_front": (0, 0.50, 0.88, "core"),
    "torso_back": (180, 0.50, 0.88, "core"),
    "head_3q": (35, 0.80, 1.0, None),
    "head_front": (0, 0.80, 1.0, None),
    "arm_right": (0, 0.42, 0.85, "left"),
    "arm_left": (0, 0.42, 0.85, "right"),
    "legs_front": (0, 0.0, 0.5, None),
    "legs_back": (180, 0.0, 0.5, None),
}
DOSSIER = ("full_front", "full_back", "full_side", "full_3q", "torso_front",
           "torso_back", "head_3q", "arm_right", "arm_left", "legs_front")
WALL = ("torso_front", "torso_back", "head_3q")


def shot_bounds(mesh, shot: str):
    """(yaw, (x0, x1, z0, z1)) framing for a named shot. Full-body shots are symmetric in x so tiles line up."""
    np = _np()
    yaw, lo, hi, half = SHOTS[shot]
    pos = mesh[0]
    a = math.radians(yaw)
    sx = -(pos[:, 0] * math.cos(a) - pos[:, 1] * math.sin(a))
    zmin, zmax = float(pos[:, 2].min()), float(pos[:, 2].max())
    zlo, zhi = zmin + lo * (zmax - zmin), zmin + hi * (zmax - zmin)
    sel = (pos[:, 2] >= zlo) & (pos[:, 2] <= zhi)
    m = float(np.abs(sx).max())
    if half == "left":
        sel &= sx < -0.33 * m
    elif half == "right":
        sel &= sx > 0.33 * m
    elif half == "core":
        sel &= np.abs(sx) < 0.33 * m
    if not sel.any():
        sel = np.ones(len(pos), dtype=bool)
    x0, x1 = float(sx[sel].min()), float(sx[sel].max())
    if half is None and shot.startswith("full"):
        x0, x1 = -max(abs(x0), abs(x1)), max(abs(x0), abs(x1))
    pad = 0.06 * (x1 - x0)
    return yaw, (x0 - pad, x1 + pad, zlo, zhi)


def render_shots(mesh, tex_rgb_u8, shots, scale: int = 1) -> dict:
    out = {}
    for s in shots:
        yaw, bounds = shot_bounds(mesh, s)
        out[s] = render(mesh, tex_rgb_u8, yaw, SHOT_W * scale, SHOT_H * scale, bounds)
    return out


# ------------------------------------------------------------------ helmets on heads (scene kit + sheets)

HELM_SHOTS = {"helm_front": 0.0, "helm_3q": 35.0, "helm_side": 90.0}  # name: yaw degrees
HELM_TILE_W, HELM_TILE_H = 170, 210
HEAD_FRAME_PAD = 0.30
GOGGLE_NODES = ("GoggleHook",)
HEAD_NODES = ("headhook",)
NO_HELMET, OK, NO_NODE, ATTACH_UNKNOWN = "none", "OK", "NO_NODE", "ATTACH_UNKNOWN"


class HelmetKit:
    """Loads mesh dumps (from `mesh --textures`) once and builds head-and-helmet scenes.

    modeltype B: head model sits at the body's headhook; the helmet is the item model reparented to the head's
    GoggleHook (status NO_NODE when the head has none: the engine drops the item). Modeltypes F and L have no
    separate head and no known attach path (ATTACH_UNKNOWN): drawn without a helmet.
    """

    def __init__(self, meshes: Path, tex_dir: Path | None = None):
        self.meshes = Path(meshes)
        self.tex_dir = Path(tex_dir) if tex_dir else self.meshes / "tex"
        self._parts: dict = {}

    def has(self, model: str) -> bool:
        return (self.meshes / f"{model.lower()}.json").is_file()

    def parts(self, model: str):
        key = model.lower()
        if key not in self._parts:
            self._parts[key] = load_parts(self.meshes / f"{key}.json", self.tex_dir)
        return self._parts[key]

    def hook(self, model: str, names):
        return hook_transform(self.meshes / f"{model.lower()}.json", names)

    @staticmethod
    def _place(parts, tf):
        if tf is None:
            return list(parts)
        np = _np()
        r, t = np.asarray(tf[0], dtype=np.float64), np.asarray(tf[1], dtype=np.float64)
        return [(pos @ r.T + t, uvs, faces, tex) for pos, uvs, faces, tex in parts]

    def scene(self, modeltype: str, body: str, head: str, mask: str | None):
        """(parts, head_parts, status). head_parts frame the shot; status is one of NO_HELMET, OK, NO_NODE,
        ATTACH_UNKNOWN. The helmet is drawn only when status is OK."""
        body_parts = self.parts(body) if self.has(body) else []
        if modeltype != "B" or not head or not self.has(head):
            return body_parts, self._top_region(body_parts), (NO_HELMET if mask is None else ATTACH_UNKNOWN)
        head_tf = self.hook(body, HEAD_NODES)
        head_parts = self._place(self.parts(head), head_tf)
        parts = body_parts + head_parts
        if mask is None:
            return parts, head_parts, NO_HELMET
        goggle = self.hook(head, GOGGLE_NODES)
        if goggle is None or not self.has(mask):
            return parts, head_parts, NO_NODE
        item_tf = compose_transforms(head_tf, goggle) if head_tf is not None else goggle
        return parts + self._place(self.parts(mask), item_tf), head_parts, OK

    @staticmethod
    def _top_region(parts, depth: float = 0.28):
        """Vertices of the parts within `depth` metres of the top: the framing target when there is no head model."""
        np = _np()
        if not parts:
            return []
        zmax = max(float(p[0][:, 2].max()) for p in parts)
        out = []
        for pos, uvs, faces, tex in parts:
            sel = pos[pos[:, 2] >= zmax - depth]
            if len(sel):
                out.append((sel, uvs, faces, tex))
        return out


def helmet_tile(kit: HelmetKit, row: dict, mask: str | None, shot: str, scale: int = 1):
    """One tile (uint8 image, status) of an appearance row with `mask` (None = bare head) from a HELM_SHOTS shot."""
    parts, head_parts, status = kit.scene(row["modeltype"], row["body_model"], row["head_model"], mask)
    yaw = HELM_SHOTS[shot]
    if not parts:
        np = _np()
        return np.zeros((HELM_TILE_H * scale, HELM_TILE_W * scale, 3), np.uint8), status
    # frame on the bare head so every row of a sheet shares one camera
    bare, bare_head, _ = kit.scene(row["modeltype"], row["body_model"], row["head_model"], None)
    bounds = parts_bounds(bare_head or bare, yaw, HEAD_FRAME_PAD)
    tile = render_scene(parts, yaw, HELM_TILE_W * scale, HELM_TILE_H * scale, bounds)
    return tile, status


def read_tsv(path: Path) -> list[dict]:
    import csv

    with open(path, newline="", encoding="utf-8") as fh:
        return list(csv.DictReader(fh, delimiter="\t"))


def helmet_models(items_tsv: Path, baseitem: str = "44") -> list[str]:
    """Distinct model_resref (lower-case, sorted) of the K2 UTIs of one base item."""
    return sorted({r["model_resref"].lower() for r in read_tsv(items_tsv)
                   if r["baseitem_id"] == baseitem and r["model_resref"]})


def head_rows(wear_tsv: Path, ids=None) -> list[dict]:
    """Appearance rows that have a head-item template (n_utc_with_head_item > 0), optionally limited to `ids`."""
    rows = [r for r in read_tsv(wear_tsv) if int(r["n_utc_with_head_item"] or 0) > 0]
    if ids:
        want = {str(i) for i in ids}
        rows = [r for r in rows if r["row"] in want]
    return rows


def helmet_label(row: dict, status: str) -> str:
    base = f"{row['row']} {row['label']} [{row['modeltype']}]"
    note = {OK: "", NO_NODE: " NO_NODE (engine drops it)", ATTACH_UNKNOWN: " ATTACH UNKNOWN"}.get(status, "")
    return base + note


def render_helmet_sheets(meshes: Path, rows: list[dict], masks: list[str], out: Path, tex_dir: Path | None = None,
                         shots=tuple(HELM_SHOTS), wall_item: str | None = None, wall_cols: int = 8,
                         scale: int = 1) -> list[Path]:
    """<row>_helmets.png per appearance (row 0 = bare head, then one row per mask model, columns = shots) and
    wall_helmets.png (one tile per appearance wearing `wall_item`, shot helm_3q, status in the label)."""
    from PIL import Image, ImageDraw

    out.mkdir(parents=True, exist_ok=True)
    kit = HelmetKit(meshes, tex_dir)
    W, H = HELM_TILE_W * scale, HELM_TILE_H * scale
    written: list[Path] = []
    wall: list = []
    for row in rows:
        if not kit.has(row["body_model"]):
            print(f"skip {row['row']}: no mesh dump for body {row['body_model']}", file=sys.stderr)
            continue
        status0 = kit.scene(row["modeltype"], row["body_model"], row["head_model"], masks[0] if masks else None)[2]
        # F/L: no attach path is known, so no helmet is drawn and every item row would repeat the bare head
        variants = [None] if status0 == ATTACH_UNKNOWN else [None] + list(masks)
        sheet = Image.new("RGB", (len(shots) * W, 16 + len(variants) * H), (30, 30, 30))
        d = ImageDraw.Draw(sheet)
        d.text((4, 2), f"{helmet_label(row, status0)}  body {row['body_model']} head {row['head_model']}",
               fill=(255, 255, 0))
        for r, mask in enumerate(variants):
            for c, shot in enumerate(shots):
                tile, _ = helmet_tile(kit, row, mask, shot, scale)
                sheet.paste(Image.fromarray(tile), (c * W, 16 + r * H))
            _label(d, (2, 18 + r * H), mask or "bare head")
        path = out / f"{row['row']}_helmets.png"
        sheet.save(path)
        written.append(path)
        print(f"{row['row']}: {path}")
        if wall_item and kit.has(wall_item):
            tile, status = helmet_tile(kit, row, wall_item, "helm_3q", scale)
            wall.append((f"{row['row']} {row['label']}", f"[{row['modeltype']}] {status}", tile))
    if wall:
        rowsn = (len(wall) + wall_cols - 1) // wall_cols
        sheet = Image.new("RGB", (wall_cols * W, rowsn * H), (30, 30, 30))
        d = ImageDraw.Draw(sheet)
        fit = int((W - 6) / 6)
        for k, (lab, st, tile) in enumerate(wall):
            x, y = (k % wall_cols) * W, (k // wall_cols) * H
            sheet.paste(Image.fromarray(tile), (x, y))
            _label(d, (x + 2, y + 2), lab[:fit])
            _label(d, (x + 2, y + 15), st[:fit], fill=(255, 255, 0) if st.endswith(OK) else (255, 140, 60))
        path = out / f"wall_helmets_{wall_item}.png"
        sheet.save(path)
        written.append(path)
    return written


# ------------------------------------------------------------------ holstered weapons on bodies (HW3)

WEAPON_SHOTS = {"front": 0.0, "side": 90.0, "back": 180.0}  # name: yaw degrees
WEAPON_TILE_W, WEAPON_TILE_H = 230, 300
WEAPON_BONES = ("pelvis_g", "lthigh_g", "rthigh_g", "torso_g", "torsoUpr_g")
BODY_BAND = (0.35, 0.95)  # camera band as a fraction of body height: knees to shoulders


def euler_matrix(rx: float, ry: float, rz: float):
    """3x3 rotation (nested lists) for degrees about x, then y, then z (R = Rz @ Ry @ Rx)."""
    a, b, c = (math.radians(v) for v in (rx, ry, rz))
    ca, sa, cb, sb, cc, sc = math.cos(a), math.sin(a), math.cos(b), math.sin(b), math.cos(c), math.sin(c)
    rx_m = [[1, 0, 0], [0, ca, -sa], [0, sa, ca]]
    ry_m = [[cb, 0, sb], [0, 1, 0], [-sb, 0, cb]]
    rz_m = [[cc, -sc, 0], [sc, cc, 0], [0, 0, 1]]
    mm = lambda p, q: [[sum(p[i][k] * q[k][j] for k in range(3)) for j in range(3)] for i in range(3)]  # noqa: E731
    return mm(rz_m, mm(ry_m, rx_m))


class WeaponKit(HelmetKit):
    """HelmetKit plus the full node table (`nodes_all`, skin bones included) and REFERENCE nodes (`refs`)."""

    def _doc(self, model: str) -> dict:
        return json.loads((self.meshes / f"{model.lower()}.json").read_text(encoding="utf-8"))

    def node(self, model: str, names):
        """(R, t) of the first of `names` among the dump's nodes_all, then its hooks (case-insensitive), else None."""
        doc = self._doc(model)
        for table in (doc.get("nodes_all", {}), doc.get("hooks", {})):
            low = {k.lower(): v for k, v in table.items()}
            for n in names:
                if n.lower() in low:
                    return low[n.lower()]["R"], low[n.lower()]["t"]
        return None

    def weapon_parts(self, weapon: str):
        """The weapon's own meshes plus every REFERENCE model that has a dump (a saber hilt hangs on ref_hilt)."""
        parts = list(self.parts(weapon))
        for node, ref in self._doc(weapon).get("refs", {}).items():
            tf = self.node(weapon, [node])
            if self.has(ref) and tf is not None:
                parts += self._place(self.parts(ref), tf)
        return parts

    def scene(self, body: str, weapon: str, bone: str, rotate: bool = True, rot=(0.0, 0.0, 0.0), offset=(0.0, 0.0, 0.0),
              hand: str = ""):
        """(parts, status): body + weapon with the weapon origin on `bone`; status OK or NO_NODE (body only).
        rot = euler degrees (about x, then y, then z) and offset = metres, both applied in the bone frame before the bone.
        hand = a hand node name (rhand/lhand): the weapon takes that node's rotation (its held orientation), at the bone."""
        tf = self.node(body, [bone])
        if tf is None:
            return self.parts(body), NO_NODE
        if not rotate:
            tf = ([[1, 0, 0], [0, 1, 0], [0, 0, 1]], tf[1])
        if hand:
            h = self.node(body, [hand])
            if h is None:
                return self.parts(body), NO_NODE
            tf = (h[0], tf[1])
        if any(rot) or any(offset):
            tf = compose_transforms(tf, (euler_matrix(*rot), list(offset)))
        return self.parts(body) + self._place(self.weapon_parts(weapon), tf), OK


def body_band(parts, lo_hi=BODY_BAND):
    """One part holding the body vertices between the lo and hi fractions of its height: the shared camera target."""
    np = _np()
    zmin = min(float(p[0][:, 2].min()) for p in parts)
    zmax = max(float(p[0][:, 2].max()) for p in parts)
    lo, hi = zmin + lo_hi[0] * (zmax - zmin), zmin + lo_hi[1] * (zmax - zmin)
    sel = np.concatenate([p[0][(p[0][:, 2] >= lo) & (p[0][:, 2] <= hi)] for p in parts])
    if not len(sel):
        return list(parts)  # nothing in the band: frame the whole body
    return [(sel, parts[0][1], parts[0][2], parts[0][3])]


def weapon_tile(kit: WeaponKit, body: str, weapon: str, bone: str, shot: str, rotate: bool = True, scale: int = 1,
                rot=(0.0, 0.0, 0.0), offset=(0.0, 0.0, 0.0), hand: str = ""):
    """(uint8 image, status) of `weapon` on `bone` of `body`, framed on the bare body's torso band."""
    parts, status = kit.scene(body, weapon, bone, rotate, rot, offset, hand)
    yaw = WEAPON_SHOTS[shot]
    bounds = parts_bounds(body_band(kit.parts(body)), yaw, 0.10)
    return render_scene(parts, yaw, WEAPON_TILE_W * scale, WEAPON_TILE_H * scale, bounds), status


def render_weapon_sheets(meshes: Path, bodies, weapons, bones, out: Path, shots=tuple(WEAPON_SHOTS),
                         rotate: bool = True, scale: int = 1, rot=(0.0, 0.0, 0.0), offset=(0.0, 0.0, 0.0),
                         tag: str = "", hand: str = "") -> list[Path]:
    """<body>_<shot>.png per body and shot: rows = weapon, columns = bone. REST pose only."""
    from PIL import Image, ImageDraw

    out.mkdir(parents=True, exist_ok=True)
    kit = WeaponKit(meshes)
    W, H = WEAPON_TILE_W * scale, WEAPON_TILE_H * scale
    written: list[Path] = []
    weapons = [w for w in weapons if kit.has(w) or print(f"skip weapon {w}: no mesh dump", file=sys.stderr)]
    for body in bodies:
        if not kit.has(body):
            print(f"skip body {body}: no mesh dump", file=sys.stderr)
            continue
        for shot in shots:
            sheet = Image.new("RGB", (len(bones) * W, 16 + len(weapons) * H), (30, 30, 30))
            d = ImageDraw.Draw(sheet)
            mode = "bone rotation applied" if rotate else "translation only"
            pose = (f", rot {tuple(rot)} deg, offset {tuple(offset)} m" if any(rot) or any(offset) else "") + (f", rotation of {hand}" if hand else "")
            d.text((4, 2), f"{body} {shot}: weapon origin on bone, REST pose, {mode}{pose}", fill=(255, 255, 0))
            for r, weapon in enumerate(weapons):
                for c, bone in enumerate(bones):
                    tile, status = weapon_tile(kit, body, weapon, bone, shot, rotate, scale, rot, offset, hand)
                    sheet.paste(Image.fromarray(tile), (c * W, 16 + r * H))
                    _label(d, (c * W + 2, 18 + r * H), bone if status == OK else f"{bone} NO_NODE",
                           fill=(255, 255, 0) if status == OK else (255, 140, 60))
                _label(d, (2, 18 + r * H + 13), weapon)
            path = out / f"{body.lower()}_{shot}{tag}.png"
            sheet.save(path)
            written.append(path)
            print(f"{body} {shot}: {path}")
    return written


def cmd_weapons(args) -> int:
    paths = render_weapon_sheets(Path(args.meshes).expanduser(), [b.lower() for b in args.bodies],
                                 [w.lower() for w in args.weapons], args.bones,
                                 Path(args.out).expanduser(), shots=tuple(args.shots), rotate=not args.no_rotate,
                                 scale=2 if args.size == "big" else 1, rot=tuple(args.rot), offset=tuple(args.offset),
                                 tag=args.tag, hand=args.hand_rot)
    print(f"wrote {len(paths)} sheets ({len(args.bodies)} bodies x {len(args.shots)} shots; rest pose)")
    return 0 if paths else 2


# ------------------------------------------------------------------ K1 vs K2 model comparison (C3)

CMP_YAWS = (0.0, 90.0)  # per model, each side: front and side view
CMP_TILE_W, CMP_TILE_H = 220, 150


def model_stats(path: Path) -> dict:
    """Mesh facts of one dump: textured nodes, triangles, bbox extents (x, y, z) in metres, texture names."""
    np = _np()
    doc = json.loads(Path(path).read_text(encoding="utf-8"))
    nodes = [n for n in doc["nodes"] if n["positions"]]
    pos = np.concatenate([np.asarray(n["positions"], dtype=np.float64) for n in nodes]) if nodes else np.zeros((0, 3))
    ext = (pos.max(axis=0) - pos.min(axis=0)).tolist() if len(pos) else [0.0, 0.0, 0.0]
    return {"nodes": len(doc["nodes"]), "tris": sum(len(n["faces"]) for n in doc["nodes"]),
            "extent": [round(v, 4) for v in ext], "textures": sorted(set(n["texture"].lower() for n in doc["nodes"]))}


def compare_rows(k1_dir: Path, k2_dir: Path, models) -> list[dict]:
    """One row per model: stats from each side's dump (None when that side has no dump) and a verdict string.
    verdict = K1_ONLY, K2_ONLY, SAME (triangles and extents within 1 mm), TRIS (triangle count differs) or SIZE."""
    rows = []
    for m in models:
        p1, p2 = Path(k1_dir) / f"{m}.json", Path(k2_dir) / f"{m}.json"
        s1 = model_stats(p1) if p1.is_file() else None
        s2 = model_stats(p2) if p2.is_file() else None
        if s1 and s2:
            if s1["tris"] != s2["tris"]:
                verdict = "TRIS"
            elif max(abs(a - b) for a, b in zip(s1["extent"], s2["extent"])) > 0.001:
                verdict = "SIZE"
            else:
                verdict = "SAME"
        else:
            verdict = "K1_ONLY" if s1 else "K2_ONLY" if s2 else "NONE"
        rows.append({"model": m, "k1": s1, "k2": s2, "verdict": verdict})
    return rows


def compare_tile(path: Path | None, tex_dir: Path, other: Path | None, yaw: float, scale: int = 1):
    """(h, w, 3) uint8 view of one model, framed on the union with `other` so both sides share a scale;
    a dark placeholder when `path` is absent."""
    np = _np()
    W, H = CMP_TILE_W * scale, CMP_TILE_H * scale
    if path is None or not Path(path).is_file():
        return np.full((H, W, 3), 45, dtype=np.uint8)
    parts = load_parts(Path(path), tex_dir)
    frame = list(parts)
    if other is not None and Path(other).is_file():
        frame += [(p[0], p[1], p[2], p[3]) for p in load_parts(Path(other), None)]
    if not parts:
        return np.full((H, W, 3), 45, dtype=np.uint8)
    return render_scene(parts, yaw, W, H, parts_bounds(frame, yaw, 0.08))


def render_compare_sheets(k1_dir: Path, k2_dir: Path, models, out: Path, per_sheet: int = 8, scale: int = 1):
    """compare_NN.png sheets (one row per model: K1 front, K1 side, K2 front, K2 side) and compare.tsv."""
    from PIL import Image, ImageDraw

    out.mkdir(parents=True, exist_ok=True)
    rows = compare_rows(k1_dir, k2_dir, models)
    W, H = CMP_TILE_W * scale, CMP_TILE_H * scale
    written = []
    for s in range(0, len(rows), per_sheet):
        chunk = rows[s:s + per_sheet]
        sheet = Image.new("RGB", (4 * W, 16 + len(chunk) * H), (30, 30, 30))
        d = ImageDraw.Draw(sheet)
        d.text((4, 2), "cols: K1 front | K1 side | K2 front | K2 side   (shared scale per row)", fill=(255, 255, 0))
        for r, row in enumerate(chunk):
            m = row["model"]
            j1, j2 = Path(k1_dir) / f"{m}.json", Path(k2_dir) / f"{m}.json"
            for side, (j, tex) in enumerate(((j1, Path(k1_dir) / "tex"), (j2, Path(k2_dir) / "tex"))):
                other = j2 if side == 0 else j1
                for c, yaw in enumerate(CMP_YAWS):
                    tile = compare_tile(j, tex, other, yaw, scale)
                    sheet.paste(Image.fromarray(tile), ((side * 2 + c) * W, 16 + r * H))
            s1, s2 = row["k1"], row["k2"]
            _label(d, (2, 18 + r * H), f"{m} [{row['verdict']}]")
            _label(d, (2, 31 + r * H), f"K1 {s1['nodes']}n {s1['tris']}t" if s1 else "K1 none")
            _label(d, (2 * W + 2, 31 + r * H), f"K2 {s2['nodes']}n {s2['tris']}t" if s2 else "K2 none")
        path = out / f"compare_{s // per_sheet:02d}.png"
        sheet.save(path)
        written.append(path)
    lines = ["model\tverdict\tk1_nodes\tk1_tris\tk1_extent\tk2_nodes\tk2_tris\tk2_extent"]
    for row in rows:
        s1, s2 = row["k1"], row["k2"]
        f = lambda s, k: "" if not s else (",".join(map(str, s[k])) if k == "extent" else str(s[k]))  # noqa: E731
        lines.append("\t".join([row["model"], row["verdict"], f(s1, "nodes"), f(s1, "tris"), f(s1, "extent"),
                                f(s2, "nodes"), f(s2, "tris"), f(s2, "extent")]))
    (out / "compare.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8")
    written.append(out / "compare.tsv")
    return written


def cmd_compare(args) -> int:
    models = [m.lower() for m in args.models] if args.models else \
        [ln.strip().lower() for ln in Path(args.list).read_text(encoding="utf-8").splitlines() if ln.strip()]
    paths = render_compare_sheets(Path(args.k1).expanduser(), Path(args.k2).expanduser(), models,
                                  Path(args.out).expanduser(), scale=2 if args.size == "big" else 1)
    print(f"wrote {len(paths)} files ({len(models)} models)")
    return 0 if paths else 2


# ------------------------------------------------------------------ look + texture sources


def _entries(bake: Path, meshes: Path, only):
    """(bake.json entry, mesh dump path) per requested look; older bakes without 'model' use the shipped recipes."""
    doc = json.loads((bake / "bake.json").read_text(encoding="utf-8"))
    ids = {only} if isinstance(only, str) else (set(only) if only else None)
    models = None
    for e in doc["looks"]:
        if ids is not None and e["id"] not in ids:
            continue
        if not e.get("model"):
            if models is None:
                import k1_port_tex_audit as au  # noqa: E402

                models = au.recipe_models(HERE / "looks")
            e["model"] = models.get(e["id"])
        mfile = meshes / f"{e['model']}.json"
        if not mfile.is_file():
            print(f"skip {e['id']}: no mesh dump {mfile}", file=sys.stderr)
            continue
        yield e, mfile


class _Textures:
    """Decoded parent and look atlases of a bake dir. mode 'look' = the baked child; 'gap' = the audit gap image."""

    def __init__(self, bake: Path, masks: Path | None, mode: str):
        self.bake, self.masks, self.mode = bake, masks or bake, mode
        self._parents: dict = {}
        self._islands: dict = {}

    @staticmethod
    def _rgb(blob: bytes):
        import k1_port_tex_tpc as tp  # noqa: E402

        return _np().ascontiguousarray(tp.decode_mip(blob, tp.parse(blob), 0)[:, :, :3])

    def parent(self, e):
        if e["parent"] not in self._parents:
            self._parents[e["parent"]] = self._rgb((self.bake / "parents" / f"{e['parent']}.tpc").read_bytes())
        return self._parents[e["parent"]]

    def look(self, e):
        if self.mode == "gap":
            return self._gap(e)
        return self._rgb((self.bake / "tpc" / f"{e['child']}.tpc").read_bytes())

    def _gap(self, e):
        import k1_port_tex_audit as au  # noqa: E402
        import k1_port_tex_overlay as tov  # noqa: E402
        import k1_port_uvmask_overlay as uvo  # noqa: E402

        np = _np()
        rgb = self.parent(e)
        if e["model"] not in self._islands:
            mdir = self.masks / "masks" / e["model"]
            labels = json.loads((mdir / "masks.json").read_text())["labels"]
            anyi = np.zeros(rgb.shape[:2], dtype=bool)
            for lab in labels:
                anyi |= uvo.read_mask(str(mdir / f"{lab}.png"))
            self._islands[e["model"]] = anyi
        painted = uvo.read_mask(str(self.bake / "regions" / f"{e['child']}.png"))
        return au.gap_texture(rgb, painted, tov.skin_guard(rgb), self._islands[e["model"]])


def _label(draw, xy, text, fill=(255, 255, 0)):
    x, y = xy
    draw.rectangle([x, y, x + 6 * len(text) + 4, y + 11], fill=(0, 0, 0))
    draw.text((x + 2, y), text, fill=fill)


# ------------------------------------------------------------------ quick sheets (render)


def render_looks(bake: Path, meshes: Path, out: Path, only=None, views=VIEWS, mode: str = "look",
                 masks: Path | None = None, gif: bool = False) -> list[Path]:
    """Per look: <id>.png (top = parent, bottom = look, one column per yaw) and contact_<family>.png (front + back)."""
    from PIL import Image, ImageDraw

    out.mkdir(parents=True, exist_ok=True)
    tx = _Textures(bake, masks, mode)
    cache: dict = {}
    written: list[Path] = []
    rows: dict[str, list] = {}
    for e, mfile in _entries(bake, meshes, only):
        model = e["model"]
        if model not in cache:
            mesh = load_mesh(mfile)
            cache[model] = (mesh, view_bounds(mesh, views), {})
        mesh, bounds, parent_tiles = cache[model]
        if e["parent"] not in parent_tiles:
            parent_tiles[e["parent"]] = [render(mesh, tx.parent(e), y, bounds=bounds) for y in views]
        look_rgb = tx.look(e)
        c_tiles = [render(mesh, look_rgb, y, bounds=bounds) for y in views]
        p_tiles = parent_tiles[e["parent"]]
        n = len(views)
        sheet = Image.new("RGB", (n * TILE_W, 2 * TILE_H + 16), (30, 30, 30))
        for k in range(n):
            sheet.paste(Image.fromarray(p_tiles[k]), (k * TILE_W, 16))
            sheet.paste(Image.fromarray(c_tiles[k]), (k * TILE_W, TILE_H + 16))
        d = ImageDraw.Draw(sheet)
        what = "gap map" if mode == "gap" else "look"
        d.text((4, 2), f"{e['id']}  {e['parent']} (top: parent) -> {e['child']} (bottom: {what})  yaw {list(views)}",
               fill=(255, 255, 0))
        path = out / (f"{e['id']}.png" if mode == "look" else f"{e['id']}_gap.png")
        sheet.save(path)
        written.append(path)
        if gif:
            written.append(_turntable(mesh, tx.parent(e), look_rgb, path.with_suffix(".gif")))
        rows.setdefault(str(e.get("family") or "looks"), []).append((e, c_tiles))
        print(f"{e['id']}: {path}")
    for fam, items in rows.items():
        cols = 4
        gw, gh = len(items[0][1][:2]) * TILE_W, TILE_H + 14
        sheet = Image.new("RGB", (cols * gw, ((len(items) + cols - 1) // cols) * gh), (30, 30, 30))
        d = ImageDraw.Draw(sheet)
        for k, (e, tiles) in enumerate(items):
            x, y = (k % cols) * gw, (k // cols) * gh
            for j in range(min(2, len(tiles))):
                sheet.paste(Image.fromarray(tiles[j]), (x + j * TILE_W, y + 14))
            d.text((x + 3, y + 1), f"{e['id']} -> {e['child']} (front, back)", fill=(255, 255, 0))
        path = out / (f"contact_{fam}.png" if mode == "look" else f"contact_{fam}_gap.png")
        sheet.save(path)
        written.append(path)
    return written


def _turntable(mesh, parent_rgb, look_rgb, path: Path, frames: int = 16) -> Path:
    """GIF of parent | look turning through `frames` yaws (0 .. 360 - 360/frames)."""
    from PIL import Image

    yaws = [360.0 * k / frames for k in range(frames)]
    bounds = view_bounds(mesh, yaws)
    imgs = []
    for y in yaws:
        pair = _np().concatenate([render(mesh, parent_rgb, y, bounds=bounds), render(mesh, look_rgb, y, bounds=bounds)],
                                 axis=1)
        imgs.append(Image.fromarray(pair))
    imgs[0].save(path, save_all=True, append_images=imgs[1:], duration=90, loop=0)
    return path


# ------------------------------------------------------------------ dossiers: many zooms and angles per look


def render_dossiers(bake: Path, meshes: Path, out: Path, only=None, shots=DOSSIER, wall=WALL, mode: str = "look",
                    masks: Path | None = None, scale: int = 1, cols: int = 5, wall_cols: int = 6) -> list[Path]:
    """<id>_dossier.png per look (parent row above look row, `cols` shots per band) and wall_<family>_<shot>.png
    (the parent atlases first, then every look of the family, same camera) for the close-up shots in `wall`."""
    from PIL import Image, ImageDraw

    out.mkdir(parents=True, exist_ok=True)
    tx = _Textures(bake, masks, mode)
    W, H = SHOT_W * scale, SHOT_H * scale
    need = list(dict.fromkeys(list(shots) + list(wall)))
    mesh_cache: dict = {}
    parent_tiles: dict = {}
    walls: dict[str, dict] = {}
    written: list[Path] = []
    suffix = "dossier" if mode == "look" else "dossier_gap"
    for e, mfile in _entries(bake, meshes, only):
        model = e["model"]
        if model not in mesh_cache:
            mesh_cache[model] = load_mesh(mfile)
        mesh = mesh_cache[model]
        pkey = (model, e["parent"])
        if pkey not in parent_tiles:
            parent_tiles[pkey] = render_shots(mesh, tx.parent(e), need, scale)
        ptiles, ltiles = parent_tiles[pkey], render_shots(mesh, tx.look(e), need, scale)
        bands = [list(shots[i:i + cols]) for i in range(0, len(shots), cols)]
        sheet = Image.new("RGB", (cols * W, 16 + len(bands) * 2 * H), (30, 30, 30))
        d = ImageDraw.Draw(sheet)
        what = "gap map" if mode == "gap" else "look"
        d.text((4, 2), f"{e['id']} ({e['family']})  parent {e['parent']} on top, {what} {e['child']} below",
               fill=(255, 255, 0))
        for b, band in enumerate(bands):
            for k, s in enumerate(band):
                x, y = k * W, 16 + b * 2 * H
                sheet.paste(Image.fromarray(ptiles[s]), (x, y))
                sheet.paste(Image.fromarray(ltiles[s]), (x, y + H))
                _label(d, (x + 2, y + 2), f"{s} | parent")
                _label(d, (x + 2, y + H + 2), f"{s} | {e['id']}")
        path = out / f"{e['id']}_{suffix}.png"
        sheet.save(path)
        written.append(path)
        fam = walls.setdefault(str(e.get("family") or "looks"), {})
        for s in wall:
            items = fam.setdefault(s, [])
            if not any(lab == f"parent {e['parent']}" for lab, _ in items):
                items.append((f"parent {e['parent']}", ptiles[s]))
            items.append((e["id"], ltiles[s]))
        print(f"{e['id']}: {path}")
    for famname, by_shot in walls.items():
        for s, items in by_shot.items():
            rowsn = (len(items) + wall_cols - 1) // wall_cols
            sheet = Image.new("RGB", (wall_cols * W, rowsn * H), (30, 30, 30))
            d = ImageDraw.Draw(sheet)
            for k, (lab, tile) in enumerate(items):
                x, y = (k % wall_cols) * W, (k // wall_cols) * H
                sheet.paste(Image.fromarray(tile), (x, y))
                _label(d, (x + 2, y + 2), lab)
            path = out / (f"wall_{famname}_{s}.png" if mode == "look" else f"wall_{famname}_{s}_gap.png")
            sheet.save(path)
            written.append(path)
    return written


# ------------------------------------------------------------------ cli


def _common(args):
    return (Path(args.bake).expanduser(), Path(args.meshes).expanduser(), Path(args.out).expanduser(),
            args.ids or None)


def cmd_render(args) -> int:
    bake, meshes, out, ids = _common(args)
    paths = render_looks(bake, meshes, out, ids, tuple(args.views), args.mode,
                         Path(args.masks).expanduser() if args.masks else None, args.gif)
    print(f"wrote {len(paths)} files")
    return 0 if paths else 2


def cmd_dossier(args) -> int:
    bake, meshes, out, ids = _common(args)
    shots = tuple(args.shots)
    bad = [s for s in list(shots) + list(args.wall) if s not in SHOTS]
    if bad:
        print(f"unknown shot(s) {bad}; choose from {sorted(SHOTS)}", file=sys.stderr)
        return 2
    paths = render_dossiers(bake, meshes, out, ids, shots, tuple(args.wall), args.mode,
                            Path(args.masks).expanduser() if args.masks else None, 2 if args.size == "big" else 1)
    print(f"wrote {len(paths)} files")
    return 0 if paths else 2


def cmd_helmets(args) -> int:
    meshes = Path(args.meshes).expanduser()
    rows = head_rows(Path(args.wear), None if args.appearances in (None, ["all54"]) else args.appearances)
    masks = ([m.lower() for m in args.items] if args.items and args.items != ["distinct-models"]
             else helmet_models(Path(args.items_tsv)))
    paths = render_helmet_sheets(meshes, rows, masks, Path(args.out).expanduser(),
                                 Path(args.tex).expanduser() if args.tex else None,
                                 wall_item=(args.wall_item or "").lower() or None, scale=2 if args.size == "big" else 1)
    print(f"wrote {len(paths)} files ({len(rows)} appearances x {len(masks)} helmet models)")
    return 0 if paths else 2


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    m = sub.add_parser("mesh", help="dump textured meshes (run with the k1port venv)")
    m.add_argument("--models", nargs="+", required=True)
    m.add_argument("--out", required=True)
    m.add_argument("--game", default=str(DEFAULT_GAME), help="K2 install, read only")
    m.add_argument("--textures", action="store_true", help="also copy each used texture to <out>/tex/ as raw bytes")
    for name, helptext in (("render", "quick parent-vs-look sheets by yaw, plus contact sheets (run with .venv)"),
                           ("dossier", "many zooms and angles per look, plus per-family close-up walls (run with .venv)")):
        r = sub.add_parser(name, help=helptext)
        r.add_argument("--bake", required=True)
        r.add_argument("--meshes", required=True)
        r.add_argument("--out", required=True)
        r.add_argument("--ids", "--only", dest="ids", nargs="+", default=None, help="look ids (default: all)")
        r.add_argument("--mode", choices=("look", "gap"), default="look",
                       help="gap: texture the mesh with the audit gap image so unpainted regions glow")
        r.add_argument("--masks", default=None, help="mask root for --mode gap (default: the bake dir)")
        if name == "render":
            r.add_argument("--views", nargs="+", type=float, default=list(VIEWS))
            r.add_argument("--gif", action="store_true", help="also write a 16-frame turntable GIF per look")
        else:
            r.add_argument("--shots", nargs="+", default=list(DOSSIER), help=f"from {sorted(SHOTS)}")
            r.add_argument("--wall", nargs="*", default=list(WALL), help="shots for the per-family walls ([] = none)")
            r.add_argument("--size", choices=("normal", "big"), default="normal", help="big = 2x tiles")
    h = sub.add_parser("helmets", help="head-item previews: appearance rows x helmet models (run with .venv)")
    h.add_argument("--meshes", required=True, help="dir of `mesh --textures` dumps (bodies, heads, helmets)")
    h.add_argument("--tex", default=None, help="texture dir (default <meshes>/tex)")
    h.add_argument("--out", required=True)
    h.add_argument("--wear", default=str(HERE.parents[1] / ".claude/research/npc_equipment/head_wearability.tsv"))
    h.add_argument("--items-tsv", default=str(HERE.parents[1] / ".claude/research/k1_items/items_k2.tsv"))
    h.add_argument("--appearances", nargs="*", default=None, help="appearance rows, or all54 (default)")
    h.add_argument("--items", nargs="*", default=None, help="helmet model resrefs, or distinct-models (default)")
    h.add_argument("--wall-item", default="i_mask_007", help="helmet model drawn on the family wall ('' = no wall)")
    h.add_argument("--size", choices=("normal", "big"), default="normal")
    w = sub.add_parser("weapons", help="weapon-on-bone previews, REST pose: bodies x weapons x bones (run with .venv)")
    w.add_argument("--meshes", required=True, help="dir of `mesh --textures` dumps (bodies, weapons, saber hilts)")
    w.add_argument("--out", required=True)
    w.add_argument("--bodies", nargs="+", required=True)
    w.add_argument("--weapons", nargs="+", required=True)
    w.add_argument("--bones", nargs="+", default=list(WEAPON_BONES))
    w.add_argument("--shots", nargs="+", default=list(WEAPON_SHOTS), choices=sorted(WEAPON_SHOTS))
    w.add_argument("--no-rotate", action="store_true", help="place the weapon with the bone translation only")
    w.add_argument("--rot", nargs=3, type=float, default=[0.0, 0.0, 0.0], metavar=("RX", "RY", "RZ"),
                   help="weapon euler degrees about x, y, z in the bone frame (e.g. -90 0 0 turns a +y barrel down)")
    w.add_argument("--offset", nargs=3, type=float, default=[0.0, 0.0, 0.0], metavar=("X", "Y", "Z"),
                   help="weapon offset in metres in the bone frame (x = character's right, y = forward, z = up)")
    w.add_argument("--hand-rot", default="", metavar="NODE",
                   help="give the weapon the rotation of this node (rhand/lhand): its held orientation, placed at each bone")
    w.add_argument("--tag", default="", help="suffix for the output file names, e.g. _barrel_down")
    w.add_argument("--size", choices=("normal", "big"), default="normal")
    c = sub.add_parser("compare", help="K1 vs K2 model sheets: two `mesh --textures` dump dirs, one row per model")
    c.add_argument("--k1", required=True, help="dump dir made with --game <K1 install>")
    c.add_argument("--k2", required=True, help="dump dir made with --game <K2 install>")
    c.add_argument("--out", required=True)
    c.add_argument("--list", default=None, help="file with one model resref per line")
    c.add_argument("--models", nargs="*", default=None)
    c.add_argument("--size", choices=("normal", "big"), default="normal")
    return ap


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    if args.cmd == "compare" and not (args.list or args.models):
        raise SystemExit("compare needs --list or --models")
    return {"mesh": cmd_mesh, "render": cmd_render, "dossier": cmd_dossier, "helmets": cmd_helmets,
            "weapons": cmd_weapons, "compare": cmd_compare}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
