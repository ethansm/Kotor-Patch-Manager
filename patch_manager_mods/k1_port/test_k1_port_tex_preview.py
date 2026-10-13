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


# ------------------------------------------------------------------ (6) scenes, hooks, helmets


def _put_tex(td: Path, name: str, rgb) -> None:
    Image.fromarray(np.full((8, 8, 3), rgb, np.uint8)).save(td / f"{name}.tga")


def _quad_dump(model: str, tex: str, half: float, y: float, hooks=None) -> dict:
    n = {"name": "q", "texture": tex,
         "positions": [[half, y, -half], [-half, y, -half], [-half, y, half], [half, y, half]],
         "uvs": [[0, 0], [1, 0], [1, 1], [0, 1]], "faces": [[0, 1, 2], [0, 2, 3]]}
    return {"model": model, "nodes": [n], "skipped": [], "hooks": hooks or {}, "textures": {tex: "tga"}}


def _helmet_fixture(root: Path, goggle: bool = True) -> Path:
    """Body hook at z=1 (headhook), head quad (blue, +-0.2) with an optional GoggleHook at its local (0,0,0.05),
    helmet quad (red, +-0.05) nearer the camera; the world has the head centred at z=1."""
    ident = [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
    mdir = root / "mesh"
    (mdir / "tex").mkdir(parents=True)
    for tex, rgb in (("bodytex", GREEN), ("headtex", BLUE), ("masktex", RED)):
        _put_tex(mdir / "tex", tex, rgb)
    dumps = {
        "bodym": _quad_dump("bodym", "bodytex", 0.3, 0.0, {"headhook": {"R": ident, "t": [0.0, 0.0, 1.0]}}),
        "headm": _quad_dump("headm", "headtex", 0.2, 0.0,
                            {"GoggleHook": {"R": ident, "t": [0.0, 0.0, 0.05]}} if goggle else {}),
        "i_mask_x": _quad_dump("i_mask_x", "masktex", 0.05, 0.1),
    }
    for name, doc in dumps.items():
        (mdir / f"{name}.json").write_text(json.dumps(doc), encoding="utf-8")
    return mdir


ROW_B = {"row": "1", "label": "T", "modeltype": "B", "body_model": "bodym", "head_model": "headm",
         "head_source": "normalhead", "has_goggle_hook": "yes", "body_headhook": "yes", "n_utc": "1",
         "n_utc_with_head_item": "1"}


def test_render_scene_zbuffer_and_wrapper():
    near = (np.array([[0.5, 0.1, -0.5], [-0.5, 0.1, -0.5], [-0.5, 0.1, 0.5], [0.5, 0.1, 0.5]]),) + _quad_mesh()[1:]
    far = _quad_mesh()
    red, blue = _split_tex(RED, RED), _split_tex(BLUE, BLUE)
    b = (-1.0, 1.0, -1.0, 1.0)
    # the nearer part (larger y = towards the camera) wins regardless of draw order
    for parts in ([far + (blue,), near + (red,)], [near + (red,), far + (blue,)]):
        img = pv.render_scene([(p[0], p[1], p[2], p[3]) for p in parts], 0, bounds=b).astype(int)
        c = img[190, 120]
        assert c[0] > 200 and c[2] < 60, c  # centre is red (near), not blue
        e = img[190, 30]
        assert e[2] > 200 and e[0] < 60, e  # outside the near quad only the far one shows
    one = pv.render(far, _split_tex(RED, BLUE), 35)
    two = pv.render_scene([far + (_split_tex(RED, BLUE),)], 35)
    assert np.array_equal(one, two)


def test_hook_transform_and_compose():
    with tempfile.TemporaryDirectory() as td:
        path = Path(td) / "m.json"
        path.write_text(json.dumps({"nodes": [], "hooks": {"GoggleHook": {"R": [[1, 0, 0], [0, 1, 0], [0, 0, 1]],
                                                                         "t": [0, 0.1, 0.2]}}}), encoding="utf-8")
        r, t = pv.hook_transform(path, ("goggleHOOK",))
        assert t == [0, 0.1, 0.2] and r[0] == [1, 0, 0]
        assert pv.hook_transform(path, ("headhook",)) is None
    s = math.sqrt(0.5)
    outer = (pv._quat_matrix((0.0, 0.0, s, s)), [10.0, 0.0, 0.0])
    r, t = pv.compose_transforms(outer, ([[1, 0, 0], [0, 1, 0], [0, 0, 1]], [1.0, 0.0, 0.0]))
    assert np.allclose(t, [10.0, 1.0, 0.0], atol=1e-9), t  # inner offset (1,0,0) rotated 90 deg about z


def test_load_parts_textures_and_fallback():
    with tempfile.TemporaryDirectory() as td:
        mdir = _helmet_fixture(Path(td))
        parts = pv.load_parts(mdir / "headm.json", mdir / "tex")
        assert len(parts) == 1 and parts[0][3].shape == (8, 8, 3) and tuple(parts[0][3][0, 0]) == BLUE
        moved = pv.load_parts(mdir / "headm.json", mdir / "tex", transform=([[1, 0, 0], [0, 1, 0], [0, 0, 1]],
                                                                         [0, 0, 2.0]))
        assert abs(moved[0][0][:, 2].mean() - 2.0) < 1e-9
        missing = pv.load_parts(mdir / "headm.json", mdir / "nope")  # no texture file: flat fallback
        assert missing[0][3].shape == (4, 4, 3)


def test_helmet_scene_status_and_placement():
    with tempfile.TemporaryDirectory() as td:
        mdir = _helmet_fixture(Path(td))
        kit = pv.HelmetKit(mdir)
        parts, head, status = kit.scene("B", "bodym", "headm", "i_mask_x")
        assert status == pv.OK and len(parts) == 3 and len(head) == 1
        # head sits at the body headhook (z=1); the helmet at headhook + GoggleHook (z=1.05)
        assert abs(head[0][0][:, 2].mean() - 1.0) < 1e-9
        assert abs(parts[2][0][:, 2].mean() - 1.05) < 1e-9, parts[2][0]
        assert kit.scene("B", "bodym", "headm", None)[2] == pv.NO_HELMET
        assert kit.scene("F", "bodym", "", "i_mask_x")[2] == pv.ATTACH_UNKNOWN  # no known F/L attach path
    with tempfile.TemporaryDirectory() as td:
        kit = pv.HelmetKit(_helmet_fixture(Path(td), goggle=False))
        parts, _, status = kit.scene("B", "bodym", "headm", "i_mask_x")
        assert status == pv.NO_NODE and len(parts) == 2, (status, len(parts))  # engine drops the item


def test_helmet_tile_shows_helmet_over_head():
    with tempfile.TemporaryDirectory() as td:
        kit = pv.HelmetKit(_helmet_fixture(Path(td)))
        bare, s0 = pv.helmet_tile(kit, ROW_B, None, "helm_front")
        helm, s1 = pv.helmet_tile(kit, ROW_B, "i_mask_x", "helm_front")
        assert bare.shape == (pv.HELM_TILE_H, pv.HELM_TILE_W, 3) and s0 == pv.NO_HELMET and s1 == pv.OK
        reds = lambda im: int(((im[..., 0].astype(int) - im[..., 2].astype(int)) > 100).sum())  # noqa: E731
        assert reds(bare) == 0 and reds(helm) > 40, (reds(bare), reds(helm))
        # same camera: the blue head silhouette outside the helmet is unchanged
        diff = (bare.astype(int) != helm.astype(int)).any(axis=2)
        ys, xs = np.nonzero(diff)
        assert len(ys) and xs.min() > 40 and xs.max() < pv.HELM_TILE_W - 40, (xs.min(), xs.max())


def test_helmet_sheets_cli_inputs_and_end_to_end():
    import csv

    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        mdir = _helmet_fixture(root)
        wear, items = root / "wear.tsv", root / "items.tsv"
        cols = list(ROW_B)
        with open(wear, "w", newline="") as fh:
            w = csv.writer(fh, delimiter="\t")
            w.writerow(cols)
            w.writerow([ROW_B[c] for c in cols])
            w.writerow(["2", "none", "B", "bodym", "headm", "normalhead", "yes", "yes", "3", "0"])
        with open(items, "w", newline="") as fh:
            w = csv.writer(fh, delimiter="\t")
            w.writerow(["resref", "baseitem_id", "model_resref"])
            w.writerows([["a", "44", "I_Mask_X"], ["b", "44", "I_Mask_X"], ["c", "45", "I_Other"], ["d", "44", ""]])
        assert [r["row"] for r in pv.head_rows(wear)] == ["1"]  # only rows with a head-item template
        assert pv.head_rows(wear, ["2"]) == []
        assert pv.helmet_models(items) == ["i_mask_x"]
        out = root / "out"
        with contextlib.redirect_stdout(io.StringIO()):
            written = pv.render_helmet_sheets(mdir, pv.head_rows(wear), ["i_mask_x"], out, wall_item="i_mask_x")
        names = sorted(p.name for p in written)
        assert names == ["1_helmets.png", "wall_helmets_i_mask_x.png"], names
        with Image.open(out / "1_helmets.png") as im:
            assert im.size == (3 * pv.HELM_TILE_W, 16 + 2 * pv.HELM_TILE_H), im.size
        assert pv.helmet_label(ROW_B, pv.NO_NODE).endswith("NO_NODE (engine drops it)")
        assert "ATTACH UNKNOWN" in pv.helmet_label(dict(ROW_B, modeltype="F"), pv.ATTACH_UNKNOWN)


def _cmp_side(root: Path, name: str, models: dict) -> Path:
    """A dump dir {model: (half, tex_rgb, extra_tri)} for the K1/K2 comparison tests."""
    d = root / name
    (d / "tex").mkdir(parents=True)
    for model, (half, rgb, extra) in models.items():
        _put_tex(d / "tex", f"{model}_tex", rgb)
        doc = _quad_dump(model, f"{model}_tex", half, 0.0)
        if extra:
            doc["nodes"][0]["faces"].append([0, 1, 3])
        (d / f"{model}.json").write_text(json.dumps(doc), encoding="utf-8")
    return d


def test_compare_rows_verdicts():
    with tempfile.TemporaryDirectory() as td:
        k1 = _cmp_side(Path(td), "k1", {"same": (0.2, RED, False), "tris": (0.2, RED, True), "size": (0.2, RED, False),
                                        "only1": (0.2, RED, False)})
        k2 = _cmp_side(Path(td), "k2", {"same": (0.2, GREEN, False), "tris": (0.2, RED, False),
                                        "size": (0.3, RED, False), "only2": (0.2, RED, False)})
        got = {r["model"]: r["verdict"] for r in pv.compare_rows(k1, k2, ["same", "tris", "size", "only1", "only2", "none"])}
        # a texture change alone is not a mesh difference
        assert got == {"same": "SAME", "tris": "TRIS", "size": "SIZE", "only1": "K1_ONLY", "only2": "K2_ONLY",
                       "none": "NONE"}, got


def test_compare_sheets_end_to_end_and_placeholder():
    with tempfile.TemporaryDirectory() as td:
        k1 = _cmp_side(Path(td), "k1", {"gun": (0.2, RED, False), "solo": (0.2, RED, False)})
        k2 = _cmp_side(Path(td), "k2", {"gun": (0.2, GREEN, False)})
        out = Path(td) / "out"
        paths = pv.render_compare_sheets(k1, k2, ["gun", "solo"], out)
        assert [p.name for p in paths] == ["compare_00.png", "compare.tsv"], paths
        sheet = np.asarray(Image.open(out / "compare_00.png").convert("RGB"))
        W, H = pv.CMP_TILE_W, pv.CMP_TILE_H
        assert sheet.shape == (16 + 2 * H, 4 * W, 3)
        row0 = sheet[16:16 + H]
        k1_px, k2_px = row0[:, :W].reshape(-1, 3), row0[:, 2 * W:3 * W].reshape(-1, 3)
        assert (k1_px[:, 0] > 150).sum() > 500 and (k2_px[:, 1] > 100).sum() > 500  # red vs green model
        solo_k2 = sheet[16 + H:16 + 2 * H, 2 * W:3 * W]
        assert (solo_k2 == 45).all() or (solo_k2[40:, :] == 45).all()  # placeholder (label strip aside)
        lines = (out / "compare.tsv").read_text(encoding="utf-8").splitlines()
        assert lines[0].startswith("model\tverdict") and lines[2].split("\t")[1] == "K1_ONLY", lines
        assert pv.main(["compare", "--k1", str(k1), "--k2", str(k2), "--models", "gun", "--out", str(out / "cli")]) == 0
        assert (out / "cli" / "compare_00.png").is_file()


def test_load_texture_tga_origin_uses_stored_row_order():
    """Rows are returned as stored, whichever origin flag the TGA has (the TPC convention); a bottom-origin
    file used to come back flipped, which scrambled every head and body skin shipped as TGA."""
    with tempfile.TemporaryDirectory() as td:
        for name, desc in (("bottom", 0x00), ("top", 0x20)):
            hdr = bytes([0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 24, desc])
            rows = bytes([0, 0, 255] * 2) + bytes([255, 0, 0] * 2)  # stored row 0 = red (BGR), row 1 = blue
            (Path(td) / f"{name}.tga").write_bytes(hdr + rows)
            got = pv.load_texture(Path(td), name)
            assert tuple(got[0, 0]) == RED and tuple(got[1, 0]) == BLUE, (name, got.tolist())


def test_dump_model_signature_keeps_old_keys():
    import inspect

    assert list(inspect.signature(pv.dump_model).parameters)[:2] == ["model", "game"]
    assert "tex_dir" in inspect.signature(pv.dump_model).parameters


def test_fl_rows_get_one_bare_row():
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        mdir = _helmet_fixture(root)
        row = dict(ROW_B, modeltype="F", head_model="")
        with contextlib.redirect_stdout(io.StringIO()):
            written = pv.render_helmet_sheets(mdir, [row], ["i_mask_x"], root / "out")
        with Image.open(written[0]) as im:  # bare head only: no attach path is known for F/L
            assert im.size == (3 * pv.HELM_TILE_W, 16 + pv.HELM_TILE_H), im.size


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
    test_render_scene_zbuffer_and_wrapper,
    test_hook_transform_and_compose,
    test_load_parts_textures_and_fallback,
    test_helmet_scene_status_and_placement,
    test_helmet_tile_shows_helmet_over_head,
    test_helmet_sheets_cli_inputs_and_end_to_end,
    test_dump_model_signature_keeps_old_keys,
    test_fl_rows_get_one_bare_row,
    test_load_texture_tga_origin_uses_stored_row_order,
    test_compare_rows_verdicts,
    test_compare_sheets_end_to_end_and_placeholder,
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
