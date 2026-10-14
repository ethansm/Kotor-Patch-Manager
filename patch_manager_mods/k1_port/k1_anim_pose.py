#!/usr/bin/env python3
"""Offline pose evaluator for KOTOR model animations (no game launch, reads the install read-only).

  pose   evaluate every node's world (R, t) at one time and write a nodes_all-shaped JSON
  table  per animation and bone: the hand (rhand) rotation relative to the bone, as Euler degrees at t=0 plus the
         min/max of each Euler axis over the clip, and the bone's own world-rotation deviation from t=0

  .venv/bin/python patch_manager_mods/k1_port/k1_anim_pose.py pose --model n_commm --anim pause1 --time 0.5 --out OUT.json
  .venv/bin/python patch_manager_mods/k1_port/k1_anim_pose.py table --model n_commm --anims pause1 walk run \
      --bones pelvis_g lthigh_g rthigh_g torso_g torsoUpr_g rhand

Conventions (match k1_port_tex_preview.py):
  * world R = parent R @ local R, local R from the quaternion (x, y, z, w); world t = parent t + parent R @ local pos.
  * Euler: R = Rz @ Ry @ Rx, degrees (tp.euler_matrix); matrix_to_euler is its exact inverse.
  * Animation keys: position keys are OFFSETS from the geometry node's rest position; orientation keys are absolute
    local quaternions. Position is lerped, orientation slerped (shortest arc). Outside the key range the first or last
    key is held. Bezier controllers keep only the value of each column (row data is value, in, out per column).
  * Animations are found by name in the model, then each supermodel in chain order; geometry always comes from the
    requested model. Nodes are matched by lowercase name; animation nodes with no geometry twin are ignored.

BIGGEST RISK (unverified): a geometry node with NO orientation key in the animation is assumed to keep its rest
orientation, and one with no position key keeps its rest position (offset 0). If the engine instead holds some other
pose for untracked nodes, every table value for those nodes is wrong.
"""
from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path

import k1_port_tex_preview as tp  # noqa: E402  pure-python quaternion/compose/euler helpers

HAND = "rhand"
SAMPLES = 16
POSITION = 8  # MDLControllerType.POSITION
ORIENTATION = 20  # MDLControllerType.ORIENTATION
IDENT = ([[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]], [0.0, 0.0, 0.0])


# ------------------------------------------------------------------ math


def matrix_to_euler(m):
    """Inverse of tp.euler_matrix: 3x3 -> (rx, ry, rz) in degrees for R = Rz @ Ry @ Rx.

    At gimbal lock (|ry| = 90) rz is set to 0 and rx absorbs the remaining rotation.
    """
    cos_b = math.hypot(m[0][0], m[1][0])
    b = math.atan2(-m[2][0], cos_b)
    if cos_b > 1e-9:
        a = math.atan2(m[2][1], m[2][2])
        c = math.atan2(m[1][0], m[0][0])
    else:
        sb = -m[2][0]  # +1 or -1
        c = 0.0
        a = math.atan2(sb * m[0][1], m[1][1])
    return (math.degrees(a), math.degrees(b), math.degrees(c))


def _matmul(p, q):
    return [[sum(p[i][k] * q[k][j] for k in range(3)) for j in range(3)] for i in range(3)]


def _transpose(m):
    return [[m[j][i] for j in range(3)] for i in range(3)]


def _rot_angle_deg(m):
    """Rotation angle of a 3x3 rotation matrix in degrees."""
    c = (m[0][0] + m[1][1] + m[2][2] - 1.0) / 2.0
    return math.degrees(math.acos(max(-1.0, min(1.0, c))))


def _wrap180(deg):
    return (deg + 180.0) % 360.0 - 180.0


def _unit(q):
    n = math.sqrt(sum(v * v for v in q)) or 1.0
    return tuple(v / n for v in q)


def _slerp(q0, q1, a):
    """Shortest-arc slerp of two quaternions (x, y, z, w); lerp when nearly parallel."""
    q0, q1 = _unit(q0), _unit(q1)
    dot = sum(x * y for x, y in zip(q0, q1))
    if dot < 0.0:
        q1 = tuple(-v for v in q1)
        dot = -dot
    if dot > 0.9995:
        return _unit(tuple(x + (y - x) * a for x, y in zip(q0, q1)))
    theta = math.acos(min(1.0, dot))
    s = math.sin(theta)
    w0 = math.sin((1.0 - a) * theta) / s
    w1 = math.sin(a * theta) / s
    return tuple(w0 * x + w1 * y for x, y in zip(q0, q1))


# ------------------------------------------------------------------ animation tracks


def _keys(ctrl, width: int):
    """Sorted [(time, value tuple of `width` floats)] from a controller. Bezier rows are value,in,out per column."""
    stride = 3 if getattr(ctrl, "is_bezier", False) else 1
    out = []
    for row in ctrl.rows:
        data = [float(v) for v in row.data]
        if len(data) < stride * width:
            continue
        out.append((float(row.time), tuple(data[stride * c] for c in range(width))))
    out.sort(key=lambda kv: kv[0])
    return out


def build_tracks(anim_root) -> dict:
    """lowercase node name -> {"pos": keys or None, "ori": keys or None} for one animation's node tree."""
    tracks: dict = {}

    def walk(node):
        name = str(node.name).lower()
        entry = tracks.setdefault(name, {"pos": None, "ori": None})
        for ctrl in getattr(node, "controllers", None) or []:
            ctype = int(ctrl.controller_type)
            if ctype == POSITION and entry["pos"] is None:
                entry["pos"] = _keys(ctrl, 3) or None
            elif ctype == ORIENTATION and entry["ori"] is None:
                entry["ori"] = _keys(ctrl, 4) or None
        for child in node.children:
            walk(child)

    if anim_root is not None:
        walk(anim_root)
    return tracks


def _bracket(keys, t: float):
    """(v0, v1, a) around time t; holds the first/last key outside the key range."""
    if t <= keys[0][0]:
        return keys[0][1], keys[0][1], 0.0
    if t >= keys[-1][0]:
        return keys[-1][1], keys[-1][1], 0.0
    for (t0, v0), (t1, v1) in zip(keys, keys[1:]):
        if t0 <= t <= t1:
            span = t1 - t0
            return v0, v1, ((t - t0) / span) if span > 0 else 0.0
    return keys[-1][1], keys[-1][1], 0.0  # unreachable for sorted keys


def _sample_vec(keys, t: float):
    v0, v1, a = _bracket(keys, t)
    return tuple(x + (y - x) * a for x, y in zip(v0, v1))


def _sample_quat(keys, t: float):
    v0, v1, a = _bracket(keys, t)
    return _slerp(v0, v1, a)


# ------------------------------------------------------------------ pose


def pose_nodes(geom_root, tracks: dict, t: float) -> dict:
    """nodes_all-shaped {lowercase name: {"R": 3x3, "t": [x, y, z]}} of world transforms at time t.

    BIGGEST RISK: nodes without a position key keep rest position (offset 0); nodes without an orientation key keep
    their rest orientation. Unverified against the engine.
    """
    out: dict = {}

    def walk(node, parent):
        name = str(node.name).lower()
        p, o = node.position, node.orientation
        rest_pos = (float(p.x), float(p.y), float(p.z))
        rest_quat = (float(o.x), float(o.y), float(o.z), float(o.w))
        tr = tracks.get(name, {})
        pos = rest_pos
        if tr.get("pos"):
            off = _sample_vec(tr["pos"], t)
            pos = tuple(rest_pos[i] + off[i] for i in range(3))
        quat = _sample_quat(tr["ori"], t) if tr.get("ori") else rest_quat
        world = tp._compose(parent, pos, quat)
        out.setdefault(name, {"R": [[float(v) for v in row] for row in world[0]],
                              "t": [float(v) for v in world[1]]})
        for child in node.children:
            walk(child, world)

    walk(geom_root, IDENT)
    return out


# ------------------------------------------------------------------ model and chain access


def model_chain(load, name: str):
    """([(lowercase name, mdl), ...], missing) for the model then its supermodels.

    `load(name)` -> parsed MDL or None. NULL/empty stops the chain quietly; a named supermodel that cannot be loaded
    stops it and is returned as `missing`. A cycle stops the walk.
    """
    chain, seen, missing = [], set(), None
    cur = name.lower()
    while cur and cur != "null" and cur not in seen:
        seen.add(cur)
        mdl = load(cur)
        if mdl is None:
            missing = cur
            break
        chain.append((cur, mdl))
        cur = str(getattr(mdl, "supermodel", "") or "").strip().lower()
    return chain, missing


def find_anim(chain, anim_name: str):
    """First animation with this name in model-then-supermodel order, or None."""
    want = anim_name.lower()
    for _, mdl in chain:
        for anim in getattr(mdl, "anims", None) or []:
            if str(anim.name).lower() == want:
                return anim
    return None


def _read_model(pk, inst, name: str):
    import k1_port_uvmask as uvm  # noqa: E402  pure python; pykotor via load_pykotor

    mdl_res = uvm.install_get(pk, inst, name, pk.ResourceType.MDL)
    mdx_res = uvm.install_get(pk, inst, name, pk.ResourceType.MDX)
    if mdl_res is None or mdx_res is None:
        return None
    mdx = bytes(mdx_res.data)
    return pk.read_mdl(bytes(mdl_res.data), source_ext=mdx, size_ext=len(mdx))


class Session:
    """One pykotor Installation plus a per-run cache of parsed models (lowercase name -> MDL or None)."""

    def __init__(self, game: Path):
        self.pk, self.inst = tp.open_install(game)
        self._cache: dict = {}

    def load(self, name: str):
        key = name.lower()
        if key not in self._cache:
            self._cache[key] = _read_model(self.pk, self.inst, key)
        return self._cache[key]


# ------------------------------------------------------------------ commands


def _sample_times(length: float):
    if length <= 0.0:
        return [0.0]
    return [length * i / (SAMPLES - 1) for i in range(SAMPLES)]


def cmd_pose(args) -> int:
    sess = Session(Path(args.game).expanduser())
    mdl = sess.load(args.model)
    if mdl is None:
        print(f"ERROR model not found: {args.model}", file=sys.stderr)
        return 1
    chain, missing = model_chain(sess.load, args.model)
    if missing:
        print(f"WARN supermodel not found, chain stops at {missing}", file=sys.stderr)
    anim = find_anim(chain, args.anim)
    if anim is None:
        print(f"ERROR unknown anim: {args.anim} (searched {' -> '.join(n for n, _ in chain)})", file=sys.stderr)
        return 1
    nodes = pose_nodes(mdl.root, build_tracks(anim.root), args.time)
    out = Path(args.out).expanduser()
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(nodes), encoding="utf-8")
    print(f"{args.model} {anim.name} t={args.time} -> {len(nodes)} nodes -> {out}")
    return 0


def cmd_table(args) -> int:
    sess = Session(Path(args.game).expanduser())
    mdl = sess.load(args.model)
    if mdl is None:
        print(f"ERROR model not found: {args.model}", file=sys.stderr)
        return 1
    chain, missing = model_chain(sess.load, args.model)
    if missing:
        print(f"WARN supermodel not found, chain stops at {missing}", file=sys.stderr)
    geom = mdl.root
    warned: set = set()
    for an in args.anims:
        anim = find_anim(chain, an)
        if anim is None:
            print(f"WARN unknown anim: {an}", file=sys.stderr)
            continue
        length = float(anim.anim_length or 0.0)
        tracks = build_tracks(anim.root)
        frames = [pose_nodes(geom, tracks, t) for t in _sample_times(length)]
        if HAND not in frames[0]:
            print(f"WARN unknown hand node: {HAND} in {args.model}", file=sys.stderr)
            return 1
        for bone in args.bones:
            key = bone.lower()
            if key not in frames[0]:
                if key not in warned:
                    print(f"WARN unknown bone: {bone} in {args.model}", file=sys.stderr)
                    warned.add(key)
                continue
            rb0 = frames[0][key]["R"]
            rel_eulers, devs = [], []
            for f in frames:
                rb, rh = f[key]["R"], f[HAND]["R"]
                rel_eulers.append(matrix_to_euler(_matmul(_transpose(rb), rh)))
                devs.append(_rot_angle_deg(_matmul(_transpose(rb0), rb)))
            e0 = rel_eulers[0]
            rng = []
            for ax in range(3):
                vals = [e0[ax] + _wrap180(e[ax] - e0[ax]) for e in rel_eulers]
                rng.append((min(vals), max(vals)))
            print(f"{an:<10} {bone:<12} len={length:5.2f} rel0=({e0[0]:7.1f},{e0[1]:7.1f},{e0[2]:7.1f}) "
                  f"rx=[{rng[0][0]:7.1f},{rng[0][1]:7.1f}] ry=[{rng[1][0]:7.1f},{rng[1][1]:7.1f}] "
                  f"rz=[{rng[2][0]:7.1f},{rng[2][1]:7.1f}] bone_dev_max={max(devs):6.1f}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    game_default = str(tp.DEFAULT_GAME)

    pose = sub.add_parser("pose", help="world (R, t) of every node at one time -> JSON")
    pose.add_argument("--model", required=True)
    pose.add_argument("--anim", required=True)
    pose.add_argument("--time", type=float, required=True, help="seconds from animation start")
    pose.add_argument("--out", required=True)
    pose.add_argument("--game", default=game_default)
    pose.set_defaults(func=cmd_pose)

    table = sub.add_parser("table", help="rhand-relative-to-bone Euler table per animation")
    table.add_argument("--model", required=True)
    table.add_argument("--anims", nargs="+", required=True)
    table.add_argument("--bones", nargs="+", required=True)
    table.add_argument("--game", default=game_default)
    table.set_defaults(func=cmd_table)
    return ap


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
