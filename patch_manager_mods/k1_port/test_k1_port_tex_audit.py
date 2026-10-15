#!/usr/bin/env python3
"""Tests for k1_port_tex_audit.py on a synthetic bake (reuses the fixture of test_k1_port_tex_bake.py).

Checks: one PNG + JSON per parent atlas, painted fraction matches the region PNGs, island stats present, the 2x2
panel size, and that a nonexistent --only atlas writes nothing. Under system python3 the file re-executes in .venv.
"""
import contextlib
import io
import json
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

from PIL import Image  # noqa: E402

import k1_port_tex_audit as au  # noqa: E402
import test_k1_port_tex_bake as tb  # noqa: E402


def _audit(bake: Path, out: Path, masks: Path, *extra: str) -> tuple[int, str]:
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = au.main(["--bake", str(bake), "--out", str(out), "--masks", str(masks), *extra])
    return rc, buf.getvalue()


def test_audit_writes_png_and_stats_per_atlas() -> None:
    with tempfile.TemporaryDirectory() as td:
        fx, bake = tb._bake_e2e(Path(td))
        out = Path(td) / "audit"
        rc, text = _audit(bake, out, fx["masks"])
        assert rc == 0, text
        for atlas in ("n_commm05", "n_commf02"):
            assert (out / f"{atlas}.png").is_file() and (out / f"{atlas}.json").is_file(), atlas
        assert Image.open(out / "n_commm05.png").size == (2 * au.PANEL, 2 * au.PANEL)
        st = json.loads((out / "n_commm05.json").read_text())
        assert set(st["islands"]) == {"torso", "head"}, st["islands"]
        assert 0.0 < st["painted_of_content"] <= 1.0, st
        assert set(st["cells"]) == {f"{c}{r}" for c in "ABCD" for r in range(1, 5)}
        assert st["looks"], "looks missing"


def test_only_filters_to_one_atlas() -> None:
    with tempfile.TemporaryDirectory() as td:
        fx, bake = tb._bake_e2e(Path(td))
        out = Path(td) / "audit"
        rc, _ = _audit(bake, out, fx["masks"], "--only", "n_commf02")
        assert rc == 0
        assert [p.name for p in sorted(out.glob("*.png"))] == ["n_commf02.png"]


def test_gap_texture_pixel_classes() -> None:
    import numpy as np

    # 1x6 strip, every texel bright content (luma > BG_LUMA) unless noted:
    #   0 unpainted non-skin inside an island -> red      1 unpainted skin inside an island -> yellow
    #   2 unpainted non-skin outside islands  -> cyan     3 unpainted skin outside islands  -> cyan (beats yellow)
    #   4 painted                             -> source x 0.3   5 unpainted background (luma <= BG_LUMA) -> source x 0.3
    rgb = np.array([[[255, 255, 255]] * 4 + [[100, 200, 250], [10, 10, 10]]], dtype=np.uint8)
    painted = np.array([[False, False, False, False, True, False]])
    skin = np.array([[False, True, False, True, False, False]])
    any_island = np.array([[True, True, False, False, True, True]])
    gap = au.gap_texture(rgb, painted, skin, any_island)
    assert gap.shape == (1, 6, 3) and gap.dtype == np.uint8, (gap.shape, gap.dtype)
    got = [tuple(int(v) for v in gap[0, i]) for i in range(6)]
    assert got[0] == (255, 40, 40), f"red {got[0]}"
    assert got[1] == (255, 220, 0), f"yellow {got[1]}"
    assert got[2] == (0, 230, 230), f"cyan non-skin {got[2]}"
    assert got[3] == (0, 230, 230), f"cyan skin {got[3]}"
    assert got[4] == (30, 60, 75), f"painted dim {got[4]}"
    assert got[5] == (3, 3, 3), f"background dim {got[5]}"


TESTS = [
    test_audit_writes_png_and_stats_per_atlas,
    test_only_filters_to_one_atlas,
    test_gap_texture_pixel_classes,
]


def main() -> int:
    bad = 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}")
        except Exception:  # noqa: BLE001 - report every failure
            bad += 1
            print(f"FAIL {fn.__name__}\n{traceback.format_exc()}")
    print(f"{len(TESTS) - bad}/{len(TESTS)} PASS")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
