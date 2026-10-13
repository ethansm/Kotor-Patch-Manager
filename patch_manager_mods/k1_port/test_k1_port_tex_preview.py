#!/usr/bin/env python3
"""Tests for k1_port_tex_preview.py: quaternion and compose math, mesh dump loading, the software renderer (side
mapping, yaw, shape, determinism), shared framing, and render_looks end to end on a synthetic bake (reuses the
fixture of test_k1_port_tex_bake.py). Under system python3 the file re-executes in .venv.
"""
import contextlib
import io
import json
import math
import os
import sys
import tempfile
import traceback
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
VENV_PY = HERE.parents[1] / ".venv" / "bin" / "python"
try:
    import numpy  # noqa: F401
except ImportError:
    if VENV_PY.exists() and not os.environ.get("K1_TEX_REEXEC"):
        os.environ["K1_TEX_REEXEC"] = "1"
        os.execv(str(VENV_PY), [str(VENV_PY), str(Path(__file__).resolve())])
    print("SKIP: numpy not available (run with .venv/bin/python)")
    sys.exit(0)

import numpy as np  # noqa: E402
from PIL import Image  # noqa: E402

import k1_port_tex_preview as pv  # noqa: E402
import test_k1_port_tex_bake as tb  # noqa: E402

RED = (255, 0, 0)
BLUE = (0, 0, 255)
GREEN = (0, 255, 0)


def _quad_doc() -> dict:
    """One textured node: a unit quad in the xz plane (facing +y), UVs 0..1, u grows towards world -x."""
    return {"model": "n_commm", "skipped": [], "nodes": [{
        "name": "quad", "texture": "n_commm_body",
        "positions": [[1.0, 0.0, -1.0], [-1.0, 0.0, -1.0], [-1.0, 0.0, 1.0], [1.0, 0.0, 1.0]],
        "uvs": [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]],
        "faces": [[0, 1, 2], [0, 2, 3]]}]}


def _quad_mesh():
    pos = np.array([[1.0, 0.0, -1.0], [-1.0, 0.0, -1.0], [-1.0, 0.0, 1.0], [1.0, 0.0, 1.0]])
    uvs = np.array([[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]])
    faces = np.array([[0, 1, 2], [0, 2, 3]], dtype=np.int64)
    return pos, uvs, faces


def _split_tex(left, right) -> np.ndarray:
    """8x8 texture: columns 0-3 (u < 0.5) one colour, columns 4-7 the other."""
    tex = np.zeros((8, 8, 3), dtype=np.uint8)
    tex[:, :4] = left
    tex[:, 4:] = right
    return tex


def _matmul_vec(m, v):
    return [sum(m[i][k] * v[k] for k in range(3)) for i in range(3)]


# ------------------------------------------------------------------ (1) quaternion and compose math


def test_quat_identity_and_z_rotation():
    ident = pv._quat_matrix((0.0, 0.0, 0.0, 1.0))
    assert np.allclose(np.asarray(ident), np.eye(3), atol=1e-12), ident
    s = math.sqrt(0.5)
    rz = pv._quat_matrix((0.0, 0.0, s, s))  # 90 deg about +z
    out = _matmul_vec(rz, [1.0, 0.0, 0.0])
    assert np.allclose(out, [0.0, 1.0, 0.0], atol=1e-9), out


def test_compose_translates_through_rotated_parent():
    s = math.sqrt(0.5)
    parent = (pv._quat_matrix((0.0, 0.0, s, s)), [10.0, 0.0, 0.0])  # parent: 90 deg about z, at x=10
    r, t = pv._compose(parent, (1.0, 0.0, 0.0), (0.0, 0.0, 0.0, 1.0))  # child offset (1,0,0) in parent space
    assert np.allclose(t, [10.0, 1.0, 0.0], atol=1e-9), t
    assert np.allclose(r, parent[0], atol=1e-9), r
    # child rotation 90 deg about z stacks on the parent: 180 deg total, so (1,0,0) maps to (-1,0,0)
    r2, _ = pv._compose(parent, (0.0, 0.0, 0.0), (0.0, 0.0, s, s))
    assert np.allclose(_matmul_vec(r2, [1.0, 0.0, 0.0]), [-1.0, 0.0, 0.0], atol=1e-9), r2


# ------------------------------------------------------------------ (2) mesh dump loading


def test_load_mesh_shapes():
    with tempfile.TemporaryDirectory() as td:
        path = Path(td) / "quad.json"
        path.write_text(json.dumps(_quad_doc()), encoding="utf-8")
        pos, uvs, faces = pv.load_mesh(path)
    assert pos.shape == (4, 3) and uvs.shape == (4, 2) and faces.shape == (2, 3), (pos.shape, uvs.shape, faces.shape)


# ------------------------------------------------------------------ (3) renderer


def test_render_quad_side_mapping_and_yaw():
    mesh = _quad_mesh()
    tex = _split_tex(RED, BLUE)
    img = pv.render(mesh, tex, 0)
    assert img.shape == (380, 240, 3) and img.dtype == np.uint8, (img.shape, img.dtype)
    # screen left = u < 0.5 = red, screen right = blue; sample regions clear of the seam at x=120
    left = img[100:280, 30:90].astype(int)
    right = img[100:280, 150:210].astype(int)
    assert ((left[..., 0] > 200) & (left[..., 2] < 60)).all(), "left half of the quad is not red"
    assert ((right[..., 2] > 200) & (right[..., 0] < 60)).all(), "right half of the quad is not blue"
    corner = img[0, 0].astype(int)
    assert corner.max() - corner.min() < 8 and corner[0] < 128, f"corner {corner} is not the backdrop"
    # yaw 180 mirrors the view: the texture sides swap
    back = pv.render(mesh, tex, 180)
    assert ((back[100:280, 30:90, 2] > 200) & (back[100:280, 30:90, 0] < 60)).all(), "yaw 180 left is not blue"
    assert ((back[100:280, 150:210, 0] > 200) & (back[100:280, 150:210, 2] < 60)).all(), "yaw 180 right is not red"
    assert not np.array_equal(img, back)


def test_render_is_deterministic():
    mesh = _quad_mesh()
    tex = _split_tex(RED, BLUE)
    assert pv.render(mesh, tex, 35).tobytes() == pv.render(mesh, tex, 35).tobytes()


# ------------------------------------------------------------------ (4) shared framing


def test_view_bounds_symmetric():
    pos = np.array([[-0.5, 0.3, -1.0], [2.0, -0.4, 1.5], [0.7, 0.1, 0.2], [1.1, 0.0, -0.6]])
    mesh = (pos, np.zeros((4, 2)), np.array([[0, 1, 2]]))
    yaws = (0, 180, 90, 35)
    b = pv.view_bounds(mesh, yaws)
    assert b[0] == -b[1], b
    assert b[2] == pos[:, 2].min() and b[3] == pos[:, 2].max(), b
    assert pv.view_bounds(mesh, yaws) == b, "framing is not deterministic"
    for y in yaws:
        a = math.radians(y)
        sx = -(pos[:, 0] * math.cos(a) - pos[:, 1] * math.sin(a))
        assert sx.min() >= b[0] - 1e-9 and sx.max() <= b[1] + 1e-9, (y, sx, b)


def test_parent_and_look_share_framing():
    mesh = _quad_mesh()
    bounds = pv.view_bounds(mesh, (0, 180))
    red = pv.render(mesh, np.full((8, 8, 3), RED, np.uint8), 0, bounds=bounds).astype(int)
    green = pv.render(mesh, np.full((8, 8, 3), GREEN, np.uint8), 0, bounds=bounds).astype(int)
    cover_red = (red[..., 0] - red[..., 1]) > 60
    cover_green = (green[..., 1] - green[..., 0]) > 60
    assert cover_red.any(), "red texture did not cover any pixel"
    assert np.array_equal(cover_red, cover_green), "parent and look silhouettes differ under one framing"


# ------------------------------------------------------------------ (5) render_looks end to end


def test_render_looks_end_to_end():
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        _fx, out = tb._bake_e2e(root)
        bake = Path(out)
        looks = json.loads((bake / "bake.json").read_text(encoding="utf-8"))["looks"]
        assert looks, "bake has no looks"
        # one quad dump per model the bake names (the fixture has n_commm and n_commf looks)
        meshdir = root / "mesh"
        meshdir.mkdir(parents=True, exist_ok=True)
        for model in sorted({str(e.get("model")) for e in looks}):
            doc = dict(_quad_doc(), model=model)
            (meshdir / f"{model}.json").write_text(json.dumps(doc), encoding="utf-8")
        outdir = root / "preview"
        with contextlib.redirect_stdout(io.StringIO()):
            written = pv.render_looks(bake, meshdir, outdir, only=None, views=(0, 180))
        families = {str(e.get("family") or "looks") for e in looks}
        for e in looks:
            png = outdir / f"{e['id']}.png"
            assert png.is_file(), f"missing look sheet {png}"
            with Image.open(png) as im:
                assert im.width == 2 * pv.TILE_W, (e["id"], im.size)
        for fam in families:
            assert (outdir / f"contact_{fam}.png").is_file(), f"missing contact sheet for {fam}"
        assert len(written) == len(looks) + len(families), written


def _tall_mesh():
    """A 'person': two limbs at x = -1 / +1 and a torso column at x = 0, heights 0..3."""
    pos = np.array([[cx + dx, 0, z] for cx in (-1.0, 0.0, 1.0) for z in (0.0, 1.5, 3.0) for dx in (-0.1, 0.1)],
                   dtype=np.float64)
    uvs = np.zeros((len(pos), 2))
    faces = np.array([[k, k + 1, k + 2] for k in range(0, len(pos), 6)], dtype=np.int64)
    return pos, uvs, faces


def test_shot_bounds_framing():
    mesh = _tall_mesh()
    yaw, (x0, x1, z0, z1) = pv.shot_bounds(mesh, "full_front")
    assert yaw == 0 and abs(x0 + x1) < 1e-9 and x1 > 1.1 and z0 == 0.0 and z1 == 3.0
    _, (a0, a1, _, _) = pv.shot_bounds(mesh, "arm_right")  # screen-left limb only
    _, (b0, b1, _, _) = pv.shot_bounds(mesh, "arm_left")
    assert a1 < 0 < b0 and a1 - a0 < 0.5 and b1 - b0 < 0.5, (a0, a1, b0, b1)
    _, (c0, c1, _, _) = pv.shot_bounds(mesh, "torso_front")  # core only: the torso column
    assert -0.3 < c0 < 0 < c1 < 0.3, (c0, c1)
    _, (_, _, t0, t1) = pv.shot_bounds(mesh, "head_3q")
    assert abs(t0 - 2.4) < 1e-9 and t1 == 3.0
    for name in pv.SHOTS:  # every preset frames something finite
        assert all(math.isfinite(v) for v in pv.shot_bounds(mesh, name)[1]), name


def test_render_shots_shapes_and_zoom():
    mesh = _tall_mesh()
    tex = _split_tex(RED, BLUE)
    tiles = pv.render_shots(mesh, tex, ("full_front", "torso_front"))
    assert all(t.shape == (pv.SHOT_H, pv.SHOT_W, 3) and t.dtype == np.uint8 for t in tiles.values())
    big = pv.render_shots(mesh, tex, ("full_front",), scale=2)["full_front"]
    assert big.shape == (2 * pv.SHOT_H, 2 * pv.SHOT_W, 3)


def test_dossiers_and_gap_mode_end_to_end():
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        fx, out = tb._bake_e2e(root)
        bake = Path(out)
        looks = json.loads((bake / "bake.json").read_text(encoding="utf-8"))["looks"]
        meshdir = root / "mesh"
        meshdir.mkdir(parents=True, exist_ok=True)
        for model in sorted({str(e.get("model")) for e in looks}):
            (meshdir / f"{model}.json").write_text(json.dumps(dict(_quad_doc(), model=model)), encoding="utf-8")
        shots, wall = ("full_front", "torso_front", "head_3q"), ("torso_front",)
        families = {str(e.get("family") or "looks") for e in looks}
        for mode, suffix in (("look", ""), ("gap", "_gap")):
            outdir = root / f"dossier{suffix}"
            with contextlib.redirect_stdout(io.StringIO()):
                written = pv.render_dossiers(bake, meshdir, outdir, shots=shots, wall=wall, mode=mode,
                                             masks=Path(fx["masks"]), cols=2)
            for e in looks:
                png = outdir / f"{e['id']}_dossier{suffix}.png"
                with Image.open(png) as im:  # 2 shots per band, 2 bands, parent + look rows
                    assert im.width == 2 * pv.SHOT_W and im.height == 16 + 2 * 2 * pv.SHOT_H, (png, im.size)
            for fam in families:
                assert (outdir / f"wall_{fam}_torso_front{suffix}.png").is_file(), (fam, mode)
            assert len(written) == len(looks) + len(families)
        only = looks[0]["id"]  # --ids filter
        with contextlib.redirect_stdout(io.StringIO()):
            w = pv.render_dossiers(bake, meshdir, root / "one", only=[only], shots=("full_front",), wall=())
        assert [p.name for p in w] == [f"{only}_dossier.png"], w


TESTS = [
    test_quat_identity_and_z_rotation,
    test_compose_translates_through_rotated_parent,
    test_load_mesh_shapes,
    test_render_quad_side_mapping_and_yaw,
    test_render_is_deterministic,
    test_view_bounds_symmetric,
    test_parent_and_look_share_framing,
    test_render_looks_end_to_end,
    test_shot_bounds_framing,
    test_render_shots_shapes_and_zoom,
    test_dossiers_and_gap_mode_end_to_end,
]


def main() -> int:
    failed = []
    for fn in TESTS:
        try:
            fn()
        except Exception:  # noqa: BLE001  report every failure, keep going
            failed.append(fn.__name__)
            print(f"FAIL {fn.__name__}")
            traceback.print_exc()
        else:
            print(f"PASS {fn.__name__}")
    print(f"{len(TESTS) - len(failed)}/{len(TESTS)} PASS")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
