#!/usr/bin/env python3
"""Tests for k1_port_tex_overlay.py: cell/island regions, skin guard, layer ops, recipe validation and render.

Synthetic 64x64 atlases are built in memory (a gradient and flat fills), so no game files are needed. Decal shapes
come from k1_port_tex_decals (U1). Under system python3 the file re-executes itself with the repo venv, which has numpy.
"""
import json
import os
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
VENV_PY = HERE.parents[1] / ".venv" / "bin" / "python"  # HERE = <repo>/patch_manager_mods/k1_port

try:
    import numpy as np
except ImportError:
    if VENV_PY.exists() and not os.environ.get("K1_TEX_REEXEC"):
        os.environ["K1_TEX_REEXEC"] = "1"
        os.execv(str(VENV_PY), [str(VENV_PY), str(Path(__file__).resolve())])
    print("SKIP: numpy not available (run with .venv/bin/python)")
    sys.exit(0)

sys.path.insert(0, str(HERE))
import k1_port_tex_overlay as ov  # noqa: E402

W = H = 64
TESTS = []


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def expect(exc, fn, *args, **kwargs):
    try:
        fn(*args, **kwargs)
    except exc:
        return
    raise AssertionError(f"{fn.__name__} did not raise {exc.__name__}")


def gradient_u8():
    y, x = np.mgrid[0:H, 0:W]
    return np.stack([x * 4, y * 4, np.full((H, W), 128)], axis=-1).astype(np.uint8)


def flat_u8(rgb):
    return np.full((H, W, 3), rgb, dtype=np.uint8)


def to_f(u8):
    return u8.astype(np.float32) / np.float32(255.0)


def islands_top_left():
    islands = {"left": np.zeros((H, W), dtype=bool), "top": np.zeros((H, W), dtype=bool)}
    islands["left"][:, :32] = True
    islands["top"][:32, :] = True
    return islands


def good_recipe():
    return {
        "id": "P3a", "family": "commoner", "atlas": "n_commm05", "model": "n_commm", "note": "test", "seed": 1,
        "layers": [
            {"kind": "recolor", "op": "recolor_hsv", "cells": ["A1", "B1"], "islands": ["torso"],
             "avoid_cells": ["A2"], "card": "K067", "params": {"hue_shift_deg": 30, "sat_scale": 0.9}},
            {"kind": "wear", "op": "wear", "cells": ["C1"], "params": {"kind": "dust", "strength": 0.2}},
            {"kind": "emblem", "op": "decal", "cells": ["D1"],
             "params": {"name": "cog8", "cx": 50, "cy": 8, "size_px": 12, "colour": "#ffcc00"}},
        ],
    }


@test
def test_cell_mask_cells_and_bad_names():
    m = ov.cell_mask(W, H, ["A1"])
    check(m.dtype == bool and m.shape == (H, W), "cell mask dtype/shape")
    check(m.sum() == 16 * 16, "one cell is w/4 x h/4 px")
    check(m[0, 0] and not m[0, 16] and not m[16, 0], "A1 is top-left")
    m2 = ov.cell_mask(W, H, ["D4", "B2"])
    check(m2[63, 63] and m2[20, 20] and not m2[0, 0], "D4 bottom-right, B2 second row/col")
    check(m2.sum() == 2 * 16 * 16, "two cells, no overlap")
    for bad in ("E1", "A5", "a1", "A0", "A", "AA1", "", 7):
        expect(ValueError, ov.cell_mask, W, H, [bad])


@test
def test_skin_guard_predicate():
    skin = (200, 140, 110)
    red = (180, 30, 30)
    blue = (60, 90, 200)
    grey = (128, 128, 128)
    arr = np.array([[skin, red, blue, grey]], dtype=np.uint8)
    got = ov.skin_guard(arr)
    check(got.shape == (1, 4) and got.dtype == bool, "skin_guard shape/dtype")
    check(got.tolist() == [[True, False, False, False]], f"skin_guard flags {got.tolist()}")


@test
def test_region_feather_and_islands():
    reg = ov.region(W, H, ["A1"], {}, [])
    check(reg.dtype == np.float32 and reg.min() >= 0.0 and reg.max() <= 1.0, "region dtype/range")
    check(reg[5, 5] == 1.0, "interior of cell is 1")
    check(reg[5, 16] > 0.0, "1-px feather halo outside cell")
    check(reg[5, 18] == 0.0, "two px outside cell is 0")
    islands = islands_top_left()
    check(ov.region(W, H, ["D4"], islands, ["top"]).max() == 0.0, "cell outside island is empty")
    check(ov.region(W, H, ["D4"], islands, []).max() == 1.0, "empty island list = no restriction")
    mixed = ov.region(W, H, ["A1", "D4"], islands, ["top"])
    check(mixed[5, 5] == 1.0 and mixed[60, 60] == 0.0, "island restricts only the D4 cell")
    expect(KeyError, ov.region, W, H, ["A1"], islands, ["nope"])
    avoided = ov.region(W, H, ["A1", "B1"], {}, [], avoid_cells=["B1"])
    check(avoided[5, 5] == 1.0 and avoided[5, 20] == 0.0, "avoid_cells removes B1")


@test
def test_every_op_leaves_outside_region_exact():
    rgb = to_f(gradient_u8())
    reg = ov.region(W, H, ["A1", "C3"], {"all": np.ones((H, W), dtype=bool)}, ["all"])
    outside = reg == 0
    cases = {
        "recolor_hsv": dict(hue_shift_deg=90, sat_scale=0.5, val_scale=1.2),
        "band": dict(orientation="diagonal", period_px=6, width_px=3, colour="#00ff00", opacity=0.8, angle_deg=15),
        "decal": dict(name="star5", cx=8, cy=8, size_px=20, colour="#ff0000", opacity=0.9),
        "wear": dict(kind="stain", strength=0.5, seed=3),
        "tint": dict(colour="#336699", strength=0.4),
        "colorize": dict(colour="#b5651d", strength=0.8),
    }
    check(set(cases) == set(ov.OPS), "one case per op")
    for name, params in cases.items():
        out = ov.OPS[name](rgb, reg, **params)
        check(out.dtype == np.float32 and out.shape == rgb.shape, f"{name} dtype/shape")
        check(np.array_equal(out[outside], rgb[outside]), f"{name} changed pixels outside region")
        check(not np.array_equal(out[reg > 0], rgb[reg > 0]), f"{name} changed nothing inside region")


@test
def test_recolor_hue_moves_and_skips_skin():
    rgb = np.zeros((H, W, 3), dtype=np.float32)
    rgb[:, :32] = to_f(np.array([180, 30, 30], dtype=np.uint8))  # red, not skin
    rgb[:, 32:] = to_f(np.array([200, 140, 110], dtype=np.uint8))  # skin
    reg = np.ones((H, W), dtype=np.float32)
    out = ov.recolor_hsv(rgb, reg, hue_shift_deg=120)
    red_out = out[:, :32]
    check(np.allclose(red_out[..., 0], 30 / 255, atol=1 / 255), "red -> green: R channel")
    check(np.allclose(red_out[..., 1], 180 / 255, atol=1 / 255), "red -> green: G channel")
    check(np.allclose(red_out[..., 2], 30 / 255, atol=1 / 255), "red -> green: B channel")
    check(np.array_equal(out[:, 32:], rgb[:, 32:]), "skin pixels kept when keep_skin=True")
    out_no_guard = ov.recolor_hsv(rgb, reg, hue_shift_deg=120, keep_skin=False)
    check(not np.array_equal(out_no_guard[:, 32:], rgb[:, 32:]), "skin shifts when keep_skin=False")
    identity = ov.recolor_hsv(rgb, reg)
    check(np.allclose(identity, rgb, atol=1e-5), "zero shift and unit scales round-trip HSV")


@test
def test_band_period_and_width_along_row():
    rgb = np.full((H, W, 3), 0.5, dtype=np.float32)
    reg = np.ones((H, W), dtype=np.float32)
    out = ov.band(rgb, reg, orientation="vertical", period_px=8, width_px=2, colour="#000000", opacity=1.0)
    dark = np.flatnonzero(out[10, :, 0] < 1e-6)
    check(dark.size == 16, f"vertical band: {dark.size} stripe px per row, want 16")
    check(dark[:4].tolist() == [0, 1, 8, 9], f"vertical band positions {dark[:4].tolist()}")
    out_h = ov.band(rgb, reg, orientation="horizontal", period_px=8, width_px=2, colour="#000000")
    dark_h = np.flatnonzero(out_h[:, 5, 0] < 1e-6)
    check(dark_h[:4].tolist() == [0, 1, 8, 9] and dark_h.size == 16, "horizontal band counted down a column")
    half = ov.band(rgb, reg, orientation="vertical", period_px=8, width_px=2, colour="#000000", opacity=0.5)
    check(abs(float(half[10, 0, 0]) - 0.25) < 1e-6, "opacity 0.5 halves the stripe colour")
    expect(ValueError, ov.band, rgb, reg, orientation="vertical", period_px=4, width_px=6, colour="#000000",
           opacity=1.0)


@test
def test_decal_clipped_by_half_region():
    rgb = np.zeros((H, W, 3), dtype=np.float32)
    reg = np.zeros((H, W), dtype=np.float32)
    reg[:, :32] = 1.0
    out = ov.decal(rgb, reg, "medallion", cx=32, cy=32, size_px=24, colour="#ffffff")
    changed = np.any(out != rgb, axis=-1)
    check(changed.any(), "decal drew inside the region")
    check(not changed[:, 32:].any(), "decal clipped to the left half")
    multiply_white = ov.decal(rgb + 0.5, reg, "medallion", cx=32, cy=32, size_px=24, colour="#ffffff",
                              blend="multiply")
    check(np.array_equal(multiply_white, rgb + 0.5), "multiply by white is identity")


@test
def test_decal_cropped_at_edge_and_unknown_name():
    rgb = np.zeros((H, W, 3), dtype=np.float32)
    reg = np.ones((H, W), dtype=np.float32)
    corner = ov.decal(rgb, reg, "star5", cx=0, cy=0, size_px=16)
    check(np.any(corner != rgb), "corner decal partly drawn")
    off = ov.decal(rgb, reg, "star5", cx=-100, cy=-100, size_px=16)
    check(np.array_equal(off, rgb), "fully off-atlas decal is a no-op")
    expect(KeyError, ov.decal, rgb, reg, "dragon", cx=8, cy=8, size_px=16)


@test
def test_wear_deterministic_per_seed():
    rgb = to_f(gradient_u8())
    reg = np.ones((H, W), dtype=np.float32)
    for kind in ("dust", "fade", "stain", "scuff"):
        a = ov.wear(rgb, reg, kind=kind, strength=0.4, seed=5)
        b = ov.wear(rgb, reg, kind=kind, strength=0.4, seed=5)
        c = ov.wear(rgb, reg, kind=kind, strength=0.4, seed=6)
        check(np.array_equal(a, b), f"{kind}: same seed gives same output")
        check(not np.array_equal(a, c), f"{kind}: different seed gives different output")
        check(not np.array_equal(a, rgb), f"{kind}: changes the pixels")


@test
def test_tint_lerps_towards_colour():
    rgb = np.full((H, W, 3), 0.2, dtype=np.float32)
    reg = np.ones((H, W), dtype=np.float32)
    out = ov.tint(rgb, reg, "#336699", strength=0.4)
    want = 0.6 * 0.2 + 0.4 * np.array([0x33, 0x66, 0x99], dtype=np.float32) / 255.0
    check(np.allclose(out[0, 0], want, atol=1e-5), f"tint lerp {out[0, 0].tolist()} vs {want.tolist()}")


@test
def test_colorize_tints_grey_and_keeps_detail():
    grey = np.tile(np.linspace(0.3, 0.9, W, dtype=np.float32)[None, :, None], (H, 1, 3))
    reg = np.zeros((H, W), dtype=np.float32)
    reg[:, W // 2:] = 1.0
    out = ov.colorize(grey, reg, "#b5651d", strength=1.0)
    check(np.array_equal(out[:, :W // 2], grey[:, :W // 2]), "untouched outside region")
    px = out[0, W - 1]
    check(px[0] > px[1] > px[2], f"copper ordering {px.tolist()}")
    check(out[0, W // 2 + 1, 0] < out[0, W - 1, 0], "brighter plate stays brighter (detail kept)")
    try:
        ov.colorize(grey, reg, "#b5651d", strength=2.0)
        check(False, "strength 2.0 accepted")
    except ValueError:
        pass


@test
def test_ops_table():
    check(set(ov.OPS) == {"recolor_hsv", "band", "decal", "wear", "tint", "colorize"}, "OPS keys")
    check(ov.EMBLEM_OPS == {"decal"}, "EMBLEM_OPS")


@test
def test_validate_accepts_good_recipe():
    problems = ov.validate_recipe(good_recipe())
    check(problems == [], f"good recipe rejected: {problems}")


@test
def test_validate_rejects_each_rule():
    cases = {}

    r = good_recipe()
    r["layers"].append(dict(r["layers"][0]))
    cases["4 layers"] = r

    r = good_recipe()
    r["layers"] = []
    cases["0 layers"] = r

    r = good_recipe()
    r["layers"][1]["kind"] = "emblem"
    cases["2 emblem kinds"] = r

    r = good_recipe()
    r["layers"][1]["op"] = "decal"
    r["layers"][1]["params"] = {"name": "star5", "cx": 4, "cy": 4, "size_px": 8}
    cases["2 decal-op layers"] = r

    r = good_recipe()
    r["layers"][0]["cells"] = ["E1"]
    cases["bad cell E1"] = r

    r = good_recipe()
    r["layers"][0]["op"] = "spin"
    cases["unknown op"] = r

    r = good_recipe()
    r["layers"][2]["params"]["name"] = "dragon"
    cases["unknown decal"] = r

    r = good_recipe()
    r["layers"][2]["params"]["colour"] = "#ffcc0"
    cases["bad colour short"] = r

    r = good_recipe()
    r["layers"][2]["params"]["colour"] = "ffcc00"
    cases["bad colour no hash"] = r

    r = good_recipe()
    r["layers"][0]["kind"] = "paint"
    cases["bad layer kind"] = r

    r = good_recipe()
    del r["atlas"]
    cases["missing key"] = r

    r = good_recipe()
    r["seed"] = "1"
    cases["seed not int"] = r

    r = good_recipe()
    r["layers"][0]["params"]["bogus"] = 1
    cases["unknown param"] = r

    for label, recipe in cases.items():
        problems = ov.validate_recipe(recipe)
        check(len(problems) > 0, f"validate_recipe accepted: {label}")


@test
def test_render_recipe_union_and_untouched_outside():
    islands = {"torso": np.ones((H, W), dtype=bool)}
    parent = gradient_u8()
    recipe = good_recipe()
    out, union = ov.render_recipe(recipe, parent, islands)
    check(out.dtype == np.uint8 and out.shape == parent.shape, "render output dtype/shape")
    expected = (
        (ov.region(W, H, ["A1", "B1"], islands, ["torso"], ["A2"]) > 0)
        | (ov.region(W, H, ["C1"], islands, []) > 0)
        | (ov.region(W, H, ["D1"], islands, []) > 0)
    )
    check(union.dtype == bool and np.array_equal(union, expected), "union equals union of layer regions")
    check(np.array_equal(out[~union], parent[~union]), "pixels outside union are byte-exact")
    check(not np.array_equal(out[union], parent[union]), "render changed something inside union")
    out2, union2 = ov.render_recipe(recipe, parent, islands)
    check(np.array_equal(out, out2) and np.array_equal(union, union2), "render is deterministic")


@test
def test_render_recipe_raises_on_invalid():
    bad = good_recipe()
    bad["layers"][0]["op"] = "spin"
    expect(ValueError, ov.render_recipe, bad, gradient_u8(), islands_top_left())


@test
def test_load_recipe_roundtrip():
    with tempfile.TemporaryDirectory() as td:
        path = Path(td) / "recipe.json"
        path.write_text(json.dumps(good_recipe()), encoding="utf-8")
        check(ov.load_recipe(path) == good_recipe(), "load_recipe round trip")


def main():
    failed = 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}")
        except AssertionError as exc:
            failed += 1
            print(f"FAIL {fn.__name__}: {exc}")
        except Exception as exc:  # noqa: BLE001 - report any crash as a failure
            failed += 1
            print(f"ERROR {fn.__name__}: {type(exc).__name__}: {exc}")
    print(f"{len(TESTS) - failed}/{len(TESTS)} PASS")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
