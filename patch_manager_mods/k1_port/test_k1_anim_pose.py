#!/usr/bin/env python3
"""Tests for k1_anim_pose.py: key lookup, slerp, position offset, clamping, missing-key fallback, bezier value
extraction, supermodel chain and anim lookup order, Euler round trip. Synthetic fixtures only (no game files).

  .venv/bin/python patch_manager_mods/k1_port/test_k1_anim_pose.py
"""
import math
import random
import sys
import traceback
from pathlib import Path
from types import SimpleNamespace as NS

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path

import k1_anim_pose as ap  # noqa: E402
import k1_port_tex_preview as tp  # noqa: E402

EPS = 1e-6


def vec(x, y, z):
    return NS(x=float(x), y=float(y), z=float(z))


def quat(x, y, z, w):
    return NS(x=float(x), y=float(y), z=float(z), w=float(w))


def ctrl(ctype, rows, is_bezier=False):
    return NS(controller_type=ctype, rows=[NS(time=float(t), data=list(d)) for t, d in rows], is_bezier=is_bezier)


def node(name, children=(), controllers=(), position=None, orientation=None):
    return NS(name=name, children=list(children), controllers=list(controllers),
              position=position or vec(0, 0, 0), orientation=orientation or quat(0, 0, 0, 1))


def close(a, b, tol=EPS):
    return abs(a - b) <= tol


def close_seq(a, b, tol=EPS):
    return len(a) == len(b) and all(close(x, y, tol) for x, y in zip(a, b))


def rot_z(deg):
    c, s = math.cos(math.radians(deg / 2)), math.sin(math.radians(deg / 2))
    return (0.0, 0.0, s, c)  # quaternion x, y, z, w


def test_key_lookup_between_keys():
    tracks = ap.build_tracks(node("root", controllers=[
        ctrl(POS, [(0.0, (0, 0, 0)), (2.0, (4, 0, 8))]),
    ]))
    got = ap._sample_vec(tracks["root"]["pos"], 1.0)
    assert close_seq(got, (2.0, 0.0, 4.0)), got
    got = ap._sample_vec(tracks["root"]["pos"], 0.5)
    assert close_seq(got, (1.0, 0.0, 2.0)), got


def test_slerp_midpoint_is_45_about_z():
    q = ap._sample_quat([(0.0, (0, 0, 0, 1)), (1.0, rot_z(90))], 0.5)
    angle = math.degrees(2 * math.atan2(q[2], q[3]))
    assert close(angle, 45.0, 1e-6), angle
    assert close(q[0], 0.0) and close(q[1], 0.0), q
    # The same midpoint through the matrix path: rotation of 45 deg about z.
    m = tp._quat_matrix(q)
    assert close(m[0][0], math.cos(math.radians(45)), 1e-6) and close(m[1][0], math.sin(math.radians(45)), 1e-6)


def test_slerp_takes_shortest_arc():
    # w = -1 is the same rotation as identity; the sample must not swing the long way round.
    q = ap._slerp((0, 0, 0, 1), (0, 0, 0, -1), 0.5)
    assert close(abs(q[3]), 1.0, 1e-6) and close(q[0], 0.0) and close(q[2], 0.0), q


def test_position_is_offset_from_rest():
    # Rest position (1, 2, 3); offset key (0.5, 0, 0) at t=0 -> local (1.5, 2, 3) -> world t the same (no parent).
    geom = node("root", position=vec(1, 2, 3))
    tracks = ap.build_tracks(node("root", controllers=[ctrl(POS, [(0.0, (0.5, 0.0, 0.0))])]))
    frame = ap.pose_nodes(geom, tracks, 0.0)
    assert close_seq(frame["root"]["t"], (1.5, 2.0, 3.0)), frame["root"]["t"]


def test_child_world_composition_uses_parent_rotation():
    # Parent rotated 90 deg about z; child local offset (1, 0, 0) lands at (0, 1, 0) in world.
    geom = node("root", orientation=quat(*rot_z(90)), children=[node("child", position=vec(1, 0, 0))])
    frame = ap.pose_nodes(geom, {}, 0.0)
    assert close_seq(frame["child"]["t"], (0.0, 1.0, 0.0)), frame["child"]["t"]


def test_clamp_outside_key_range():
    tracks = ap.build_tracks(node("root", controllers=[
        ctrl(POS, [(1.0, (10, 0, 0)), (2.0, (20, 0, 0))]),
    ]))
    assert close_seq(ap._sample_vec(tracks["root"]["pos"], -5.0), (10.0, 0.0, 0.0))
    assert close_seq(ap._sample_vec(tracks["root"]["pos"], 99.0), (20.0, 0.0, 0.0))


def test_missing_orientation_key_falls_back_to_rest():
    # Geometry node rests at 90 deg about z; the animation only has a position key for it.
    geom = node("root", orientation=quat(*rot_z(90)), position=vec(0, 0, 0))
    tracks = ap.build_tracks(node("root", controllers=[ctrl(POS, [(0.0, (0, 0, 0)), (1.0, (1, 0, 0))])]))
    assert tracks["root"]["ori"] is None
    frame = ap.pose_nodes(geom, tracks, 0.5)
    expected = tp._quat_matrix(rot_z(90))
    for i in range(3):
        for j in range(3):
            assert close(frame["root"]["R"][i][j], expected[i][j], 1e-9), (i, j, frame["root"]["R"])


def test_missing_position_key_keeps_rest_position():
    geom = node("root", position=vec(4, 5, 6), children=[node("bone", position=vec(0, 1, 0))])
    tracks = ap.build_tracks(node("root", controllers=[ctrl(ORI, [(0.0, rot_z(0)), (1.0, rot_z(0))])]))
    frame = ap.pose_nodes(geom, tracks, 0.0)
    assert close_seq(frame["root"]["t"], (4.0, 5.0, 6.0)), frame["root"]["t"]
    assert close_seq(frame["bone"]["t"], (4.0, 6.0, 6.0)), frame["bone"]["t"]


def test_bezier_keeps_value_only():
    # Bezier position row: per column (value, in, out) -> values are indices 0, 3, 6.
    row = [1.0, 9.0, 9.0, 2.0, 9.0, 9.0, 3.0, 9.0, 9.0]
    tracks = ap.build_tracks(node("root", controllers=[ctrl(POS, [(0.0, row)], is_bezier=True)]))
    assert close_seq(tracks["root"]["pos"][0][1], (1.0, 2.0, 3.0)), tracks["root"]["pos"]


def test_chain_and_anim_lookup_order():
    anim_model = NS(name="pause1", anim_length=1.0, root=node("root"))
    anim_sup = NS(name="walk", anim_length=2.0, root=node("root"))
    anim_sup2 = NS(name="pause1", anim_length=3.0, root=node("root"))  # shadowed by the model's own pause1
    mdls = {
        "n_commm": NS(supermodel="s_female02", anims=[anim_model]),
        "s_female02": NS(supermodel="s_female01", anims=[anim_sup]),
        "s_female01": NS(supermodel="NULL", anims=[anim_sup2]),
    }
    chain, missing = ap.model_chain(mdls.get, "n_commm")
    assert [n for n, _ in chain] == ["n_commm", "s_female02", "s_female01"], chain
    assert missing is None
    assert ap.find_anim(chain, "pause1") is anim_model
    assert ap.find_anim(chain, "WALK") is anim_sup
    assert ap.find_anim(chain, "run") is None


def test_chain_stops_on_missing_and_cycles():
    mdls = {"a": NS(supermodel="b", anims=[]), "b": NS(supermodel="a", anims=[])}
    chain, missing = ap.model_chain(mdls.get, "a")
    assert [n for n, _ in chain] == ["a", "b"] and missing is None  # cycle stops quietly
    mdls = {"a": NS(supermodel="ghost", anims=[])}
    chain, missing = ap.model_chain(mdls.get, "a")
    assert [n for n, _ in chain] == ["a"] and missing == "ghost"


def test_euler_round_trip():
    rng = random.Random(7)
    for _ in range(200):
        a, c = rng.uniform(-179.0, 179.0), rng.uniform(-179.0, 179.0)
        b = rng.uniform(-80.0, 80.0)
        back = ap.matrix_to_euler(tp.euler_matrix(a, b, c))
        assert close_seq(back, (a, b, c), 1e-6), ((a, b, c), back)


def test_euler_gimbal_lock_reproduces_matrix():
    for b in (90.0, -90.0):
        m = tp.euler_matrix(30.0, b, 45.0)
        back = tp.euler_matrix(*ap.matrix_to_euler(m))
        for i in range(3):
            for j in range(3):
                assert close(back[i][j], m[i][j], 1e-6), (b, i, j)


POS, ORI = ap.POSITION, ap.ORIENTATION

TESTS = [v for k, v in sorted(globals().items()) if k.startswith("test_") and callable(v)]


def main() -> int:
    failed = 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}")
        except Exception:  # noqa: BLE001
            failed += 1
            print(f"FAIL {fn.__name__}")
            traceback.print_exc()
    print(f"{len(TESTS) - failed}/{len(TESTS)} passed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
