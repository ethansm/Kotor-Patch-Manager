#!/usr/bin/env python3
"""Offline 3D preview of baked texture looks: the real model meshes textured with parent and child atlases.

  mesh    (k1port venv)  dump a model's textured meshes (world space, UV set 1) to JSON
  render  (.venv)        render look PNGs from a bake dir: parent vs child, several views, plus contact sheets

  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python k1_port_tex_preview.py mesh --models n_commm n_commf --out DIR
  .venv/bin/python -I k1_port_tex_preview.py render --bake RUN --meshes DIR --out DIR [--ids A B] [--views 0 180 90 35]
        [--mode gap] [--gif]            quick yaw sheets + contact sheets
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


def dump_model(model: str, game: Path) -> dict:
    import k1_port_uvmask as uvm  # noqa: E402  pure python, needs pykotor only for load_pykotor

    pk = uvm.load_pykotor()
    inst = pk.Installation(str(game))
    mdl_res = uvm.install_get(pk, inst, model, pk.ResourceType.MDL)
    mdx_res = uvm.install_get(pk, inst, model, pk.ResourceType.MDX)
    if mdl_res is None or mdx_res is None:
        raise SystemExit(f"{model}: MDL/MDX not found in {game}")
    mdx = bytes(mdx_res.data)
    mdl = pk.read_mdl(bytes(mdl_res.data), source_ext=mdx, size_ext=len(mdx))
    nodes, skipped = [], []
    ident = ([[1, 0, 0], [0, 1, 0], [0, 0, 1]], [0.0, 0.0, 0.0])

    def walk(node, parent):
        p, o = node.position, node.orientation
        world = _compose(parent, (p.x, p.y, p.z), (o.x, o.y, o.z, o.w))
        mesh = getattr(node, "mesh", None)
        if mesh is not None:
            name = str(node.name)
            tex = str(getattr(mesh, "texture_1", "") or "").strip()
            pos, uvs, faces = list(mesh.vertex_positions or []), list(mesh.vertex_uv1 or []), list(mesh.faces or [])
            if not faces or not uvs or tex.lower() in ("", "null"):
                skipped.append({"node": name, "reason": "no faces, uv or texture"})
            elif not all(0.0 <= t.x <= 1.0 and 0.0 <= t.y <= 1.0 for t in uvs):
                skipped.append({"node": name, "reason": "uv out of range"})
            elif len(pos) < len(uvs):
                skipped.append({"node": name, "reason": "fewer positions than uvs"})
            else:
                r, t = world
                wp = [[sum(r[i][k] * v[k] for k in range(3)) + t[i] for i in range(3)]
                      for v in ((q.x, q.y, q.z) for q in pos[:len(uvs)])]
                nodes.append({"name": name, "texture": tex, "positions": wp,
                              "uvs": [[u.x, u.y] for u in uvs], "faces": [[f.v1, f.v2, f.v3] for f in faces]})
        for child in node.children:
            walk(child, world)

    walk(mdl.root, ident)
    return {"model": model, "nodes": nodes, "skipped": skipped}


def cmd_mesh(args) -> int:
    out = Path(args.out).expanduser()
    out.mkdir(parents=True, exist_ok=True)
    for model in args.models:
        doc = dump_model(model, Path(args.game).expanduser())
        (out / f"{model}.json").write_text(json.dumps(doc), encoding="utf-8")
        tris = sum(len(n["faces"]) for n in doc["nodes"])
        reasons: dict = {}
        for sk in doc["skipped"]:
            reasons[sk["reason"]] = reasons.get(sk["reason"], 0) + 1
        print(f"{model}: {len(doc['nodes'])} textured nodes, {tris} triangles, skipped {reasons or 0}")
    return 0


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
    np = _np()
    pos, uvs, faces = mesh
    tex = np.asarray(tex_rgb_u8, dtype=np.float64) / 255.0
    a = math.radians(yaw_deg)
    ca, sa = math.cos(a), math.sin(a)
    px = pos[:, 0] * ca - pos[:, 1] * sa  # rotate about z (up)
    py = pos[:, 0] * sa + pos[:, 1] * ca  # py = depth towards the camera at yaw 0 (character faces +y)
    pz = pos[:, 2]
    sx_world = -px  # viewer at +y sees the character's right on the viewer's left
    if bounds is None:
        bounds = (sx_world.min(), sx_world.max(), pz.min(), pz.max())
    x0, x1, z0, z1 = bounds
    W, H = width * SS, height * SS
    scale = 0.92 * min(W / max(x1 - x0, 1e-6), H / max(z1 - z0, 1e-6))
    cx, cz = (x0 + x1) / 2, (z0 + z1) / 2
    sx = (sx_world - cx) * scale + W / 2
    sy = H / 2 - (pz - cz) * scale
    img = np.zeros((H, W, 3), dtype=np.float64)
    img[:] = np.linspace(0.30, 0.18, H)[:, None, None]  # soft vertical backdrop
    zbuf = np.full((H, W), -1e9)
    light = np.array([-0.35, 0.8, 0.5])
    light /= np.linalg.norm(light)
    p3 = np.stack([-px, py, pz], axis=1)
    for f in faces:
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


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    m = sub.add_parser("mesh", help="dump textured meshes (run with the k1port venv)")
    m.add_argument("--models", nargs="+", required=True)
    m.add_argument("--out", required=True)
    m.add_argument("--game", default=str(DEFAULT_GAME), help="K2 install, read only")
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
    return ap


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    return {"mesh": cmd_mesh, "render": cmd_render, "dossier": cmd_dossier}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
