#!/usr/bin/env python3
"""Tests for k1_port_tex_bake: bake/sheets/masks CLI on synthetic 64x64 DXT parents, run with the repo .venv.

Covers: end-to-end bake (gate, blocks outside the region byte-identical, DXT5 alpha bytes, child allocation, re-bake
stability), uncompressed parent refused, output dirs inside the repo or game dir refused, an invalid recipe refused
before any write, sheets output, parent resolution order, and masks argument checks. Everything runs in tempdirs on
synthetic data; no game files are read.
"""
import contextlib
import io
import json
import os
import struct
import sys
import tempfile
import traceback
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
REPO = HERE.parents[1]


def _reexec_if_no_numpy() -> None:
    try:
        import numpy  # noqa: F401
    except ImportError:
        venv_py = REPO / ".venv" / "bin" / "python"
        if venv_py.exists() and not os.environ.get("K1_TEX_REEXEC"):
            os.environ["K1_TEX_REEXEC"] = "1"
            os.execv(str(venv_py), [str(venv_py), str(Path(__file__).resolve()), *sys.argv[1:]])
        print("SKIP: numpy not installed")
        sys.exit(0)


_reexec_if_no_numpy()
import numpy as np  # noqa: E402
from PIL import Image  # noqa: E402

import k1_port_tex_bake as bk  # noqa: E402
import k1_port_tex_dxt as tdx  # noqa: E402
import k1_port_tex_overlay as tov  # noqa: E402
import k1_port_tex_sheets as tsh  # noqa: E402
import k1_port_tex_tpc as tp  # noqa: E402
import k1_port_uvmask as uvm  # noqa: E402
import k1_port_uvmask_overlay as uvo  # noqa: E402

W = H = 64
MIPS = 7
PIXEL_TYPE = {"dxt1": 2, "dxt5": 4}
PARENT_FILE = {"n_commm05": "n_commm05.tpc", "n_commf02": "N_COMMF02.TPC"}  # uppercase on purpose: case-insensitive


def _make_parent(fmt: str, seed: int) -> bytes:
    """Random DXT body, alpha halves scrambled for DXT5, then normalised to a smooth gradient by apply_rgb_edit."""
    rng = np.random.default_rng(seed)
    sizes = [
        ((max(1, W >> k) + 3) // 4) * ((max(1, H >> k) + 3) // 4) * tdx.BLOCK_BYTES[fmt] for k in range(MIPS)
    ]
    body = rng.integers(0, 256, sum(sizes), dtype=np.uint8)
    if fmt == "dxt5":
        blocks = body.reshape(-1, 16)
        blocks[:, :8] = rng.integers(0, 256, (blocks.shape[0], 8), dtype=np.uint8)
        body = blocks.reshape(-1)
    header = struct.pack("<IfHHBB", sizes[0], 1.0, W, H, PIXEL_TYPE[fmt], MIPS).ljust(128, b"\0")
    ys, xs = np.mgrid[0:H, 0:W]
    grad = np.stack([xs * 4, ys * 4, np.full((H, W), 128)], axis=2).astype(np.uint8)
    return tp.apply_rgb_edit(header + body.tobytes(), grad, np.ones((H, W), dtype=bool))


def _write_masks(root: Path, model: str, labels: dict) -> None:
    mdir = root / "masks" / model
    mdir.mkdir(parents=True, exist_ok=True)
    meta: dict = {"labels": {}}
    for label, (x0, y0, x1, y1) in labels.items():
        m = np.zeros((H, W), dtype=bool)
        m[y0:y1, x0:x1] = True
        uvm.write_png_grey(str(mdir / f"{label}.png"), W, H, bytearray((m.astype(np.uint8) * 255).tobytes()))
        meta["labels"][label] = {"coverage": float(m.mean()), "texels": int(m.sum())}
    (mdir / "masks.json").write_text(json.dumps(meta), encoding="utf-8")


_LAYERS_T1 = [
    {"kind": "recolor", "op": "recolor_hsv", "cells": ["A1", "B1"], "islands": ["torso"],
     "params": {"hue_shift_deg": 40, "sat_scale": 1.2}},
    {"kind": "trim", "op": "band", "cells": ["C3"], "islands": ["head"],
     "params": {"orientation": "horizontal", "period_px": 8, "width_px": 2, "colour": "#ffffff", "opacity": 0.5}},
]


def _recipe(rid: str) -> dict:
    base = {"id": rid, "family": "commoner", "atlas": "n_commm05", "model": "n_commm", "seed": 1,
            "note": "synthetic test look", "layers": [dict(l, params=dict(l["params"])) for l in _LAYERS_T1]}
    if rid == "T2":
        base["layers"] = [{"kind": "recolor", "op": "tint", "cells": ["B3", "C3"], "islands": ["head"],
                           "params": {"colour": "#804020", "strength": 0.3}}]
    if rid == "T3":
        base.update(family="guard", atlas="n_commf02", model="n_commf", seed=3, layers=[
            {"kind": "wear", "op": "wear", "cells": ["A1", "A2", "A3", "A4"], "islands": [],
             "params": {"kind": "dust", "strength": 0.2, "direction": "down"}}])
    if rid == "T4":  # parent is uncompressed in the pool, so this look must fail
        base["atlas"] = "n_uncomp01"
    return base


def _fixture(root: Path) -> dict:
    pool = root / "pool"
    pool.mkdir()
    (pool / "n_commm05.tpc").write_bytes(_make_parent("dxt5", 5))
    (pool / PARENT_FILE["n_commf02"]).write_bytes(_make_parent("dxt1", 7))
    mroot = root / "maskroot"
    _write_masks(mroot, "n_commm", {"torso": (8, 0, 40, 32), "head": (20, 40, 44, 56)})
    _write_masks(mroot, "n_commf", {"torso": (4, 4, 60, 60)})
    recdir = root / "recipes"
    recdir.mkdir()
    (recdir / "commoner.json").write_text(json.dumps([_recipe("T1"), _recipe("T2")]), encoding="utf-8")
    (recdir / "guard.json").write_text(json.dumps([_recipe("T3")]), encoding="utf-8")
    return {"pool": pool, "masks": mroot, "recipes": recdir}


def _run(argv: list) -> tuple[int, str]:
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf), contextlib.redirect_stderr(buf):
        rc = bk.main(argv)
    return rc, buf.getvalue()


def _bake_argv(fx: dict, out: Path, recipes: Path | None = None, *extra: str) -> list:
    return ["bake", "--recipes", str(recipes or fx["recipes"]), "--masks", str(fx["masks"]), "--out", str(out),
            "--pool-dir", str(fx["pool"]), "--no-pack", *extra]


def _bake_e2e(root: Path) -> tuple[dict, Path]:
    fx = _fixture(root)
    out = root / "bake"
    rc, log = _run(_bake_argv(fx, out))
    assert rc == 0, f"bake exit {rc}: {log}"
    return fx, out


def _check_untouched_blocks(fx: dict, out: Path, look: dict) -> None:
    rec = _recipe(look["id"])
    parent = (fx["pool"] / PARENT_FILE[rec["atlas"]]).read_bytes()
    child = (out / "tpc" / f"{look['child']}.tpc").read_bytes()
    info = tp.parse(parent)
    parent_rgb = np.ascontiguousarray(tp.decode_mip(parent, info, 0)[:, :, :3])
    mdir = fx["masks"] / "masks" / rec["model"]
    islands = {p.stem: uvo.read_mask(str(p)) for p in sorted(mdir.glob("*.png"))}
    _rgb, union = tov.render_recipe(rec, parent_rgb, islands)
    union = np.asarray(union, dtype=bool)
    h, w = union.shape
    touch = union.reshape(h // 4, 4, w // 4, 4).any(axis=(1, 3)).reshape(-1)
    bb = tdx.BLOCK_BYTES[info["fmt"]]
    n = info["sizes"][0]
    pa = np.frombuffer(parent, np.uint8, count=n, offset=info["offsets"][0]).reshape(-1, bb)
    pc = np.frombuffer(child, np.uint8, count=n, offset=info["offsets"][0]).reshape(-1, bb)
    same = np.all(pa == pc, axis=1)
    assert np.all(same[~touch]), f"{look['id']}: a block outside the region changed"
    assert (~same).any(), f"{look['id']}: nothing changed inside the region"


def test_resolve_parent_pack_then_pool() -> None:
    with tempfile.TemporaryDirectory() as td:
        pool = Path(td) / "pool"
        pool.mkdir()
        (pool / "N_COMMF02.TPC").write_bytes(b"POOL")
        data, label = bk.resolve_parent("n_commf02", pool, pack_lookup=lambda n: None)
        assert data == b"POOL" and label == "pool:N_COMMF02.TPC", (data, label)
        data, label = bk.resolve_parent("n_commf02", pool, pack_lookup=lambda n: b"PACK")
        assert data == b"PACK" and label == "pack:n_commf02", (data, label)
        try:
            bk.resolve_parent("n_nothere01", pool, pack_lookup=lambda n: None)
        except bk.BakeError as exc:
            assert exc.code == 2, exc.code
        else:
            raise AssertionError("missing parent did not raise BakeError")


def test_bake_end_to_end_gate_blocks_alpha_and_names() -> None:
    with tempfile.TemporaryDirectory() as td:
        fx, out = _bake_e2e(Path(td))
        doc = json.loads((out / "bake.json").read_text(encoding="utf-8"))
        looks = {e["id"]: e for e in doc["looks"]}
        assert [e["id"] for e in doc["looks"]] == ["T1", "T2", "T3"], list(looks)
        names = {k: v["child"] for k, v in looks.items()}
        assert names == {"T1": "n_commm36", "T2": "n_commm37", "T3": "n_commf36"}, names
        assert json.loads((out / "alloc.json").read_text(encoding="utf-8")) == {"n_commm": 37, "n_commf": 36}
        for e in looks.values():
            assert e["ok"] and e["gate"] and all(g["ok"] for g in e["gate"]), e["gate"]
        _check_untouched_blocks(fx, out, looks["T1"])
        _check_untouched_blocks(fx, out, looks["T3"])
        assert looks["T1"]["changed_block_fraction"] > 0
        assert looks["T3"]["changed_block_fraction"] > 0
        parent = (fx["pool"] / PARENT_FILE["n_commm05"]).read_bytes()
        child = (out / "tpc" / "n_commm36.tpc").read_bytes()
        assert len(child) == len(parent)
        pa = np.frombuffer(parent, np.uint8, offset=128).reshape(-1, 16)[:, :8]
        ca = np.frombuffer(child, np.uint8, offset=128).reshape(-1, 16)[:, :8]
        assert np.array_equal(pa, ca), "DXT5 alpha bytes changed"


def test_rebake_keeps_child_names_and_counter() -> None:
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        fx, out = _bake_e2e(root)
        first = {e["id"]: e["child"] for e in json.loads((out / "bake.json").read_text())["looks"]}
        rc, log = _run(_bake_argv(fx, out))
        assert rc == 0, f"re-bake exit {rc}: {log}"
        second = {e["id"]: e["child"] for e in json.loads((out / "bake.json").read_text())["looks"]}
        assert first == second, (first, second)
        assert json.loads((out / "alloc.json").read_text()) == {"n_commm": 37, "n_commf": 36}
        assert sorted(os.listdir(out / "tpc")) == ["n_commf36.tpc", "n_commm36.tpc", "n_commm37.tpc"]


def test_uncompressed_parent_fails_exit_2_rest_finish() -> None:
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        fx = _fixture(root)
        (fx["pool"] / "n_uncomp01.tpc").write_bytes(
            struct.pack("<IfHHBB", 0, 1.0, W, H, 1, 1).ljust(128, b"\0") + bytes(W * H * 4))
        recdir = root / "recipes2"
        recdir.mkdir()
        (recdir / "mixed.json").write_text(json.dumps([_recipe("T1"), _recipe("T4")]), encoding="utf-8")
        out = root / "bake2"
        rc, log = _run(_bake_argv(fx, out, recdir))
        assert rc == 2, f"expected exit 2, got {rc}: {log}"
        ids = [e["id"] for e in json.loads((out / "bake.json").read_text())["looks"]]
        assert ids == ["T1"], ids
        assert not any(n.startswith("n_uncomp") for n in os.listdir(out / "tpc"))


def test_output_inside_repo_or_game_refused() -> None:
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        fx = _fixture(root)
        probe = REPO / "_u03_refused_probe"
        rc, log = _run(_bake_argv(fx, probe))
        assert rc == 2, f"repo output: expected 2, got {rc}: {log}"
        assert not probe.exists(), "refused repo output dir was created"
        game = root / "game"
        rc, log = _run(_bake_argv(fx, game / "tex", None, "--game", str(game)))
        assert rc == 2, f"game output: expected 2, got {rc}: {log}"
        assert not (game / "tex").exists(), "refused game output dir was created"


def test_invalid_recipe_refused_before_any_write() -> None:
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        fx = _fixture(root)
        bad = _recipe("T5")
        bad["layers"] = bad["layers"] + [dict(l, params=dict(l["params"])) for l in _LAYERS_T1]  # 4 layers
        recdir = root / "recipes3"
        recdir.mkdir()
        (recdir / "bad.json").write_text(json.dumps([_recipe("T1"), bad]), encoding="utf-8")
        out = root / "bake3"
        rc, log = _run(_bake_argv(fx, out, recdir))
        assert rc == 2, f"expected 2, got {rc}: {log}"
        assert "T5" in log and "nothing written" in log, log
        assert not out.exists(), "output dir written despite invalid recipe"


def test_sheets_write_index_and_family_pngs() -> None:
    with tempfile.TemporaryDirectory() as td:
        _fx, out = _bake_e2e(Path(td))
        sheets = Path(td) / "sheets"
        rc, log = _run(["sheets", "--bake", str(out), "--out", str(sheets)])
        assert rc == 0, f"sheets exit {rc}: {log}"
        for name in ("index.png", "commoner.png", "guard.png"):
            assert (sheets / name).is_file(), f"missing {name}"
        width = 4 * tsh.TILE + 3 * tsh.GAP
        with Image.open(sheets / "index.png") as im:
            assert im.width == width and im.height > tsh.TILE, im.size
        with Image.open(sheets / "commoner.png") as im:
            assert im.width == width and im.height > 2 * tsh.TILE, im.size


def test_masks_and_argparse_rejections() -> None:
    with tempfile.TemporaryDirectory() as td:
        target = Path(td) / "m"
        rc, _ = _run(["masks", "--models", "n_bogus:512", "--out", str(target)])
        assert rc == 2, rc
        assert not target.exists()
        rc, _ = _run(["masks", "--out", str(target)])
        assert rc == 2, rc


TESTS = [
    test_resolve_parent_pack_then_pool,
    test_bake_end_to_end_gate_blocks_alpha_and_names,
    test_rebake_keeps_child_names_and_counter,
    test_uncompressed_parent_fails_exit_2_rest_finish,
    test_output_inside_repo_or_game_refused,
    test_invalid_recipe_refused_before_any_write,
    test_sheets_write_index_and_family_pngs,
    test_masks_and_argparse_rejections,
]


def main() -> int:
    failures = 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}")
        except Exception:
            failures += 1
            print(f"FAIL {fn.__name__}")
            traceback.print_exc()
    print(f"{len(TESTS) - failures}/{len(TESTS)} PASS")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
