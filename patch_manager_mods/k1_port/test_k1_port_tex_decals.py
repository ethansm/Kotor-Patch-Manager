#!/usr/bin/env python3
"""Tests for k1_port_tex_decals.py: alpha range, coverage, determinism, centring, rotation, and name lookup.

Every decal is rendered at size 64. Checks: float32 (size, size) output in [0, 1], mean-alpha coverage in
[0.05, 0.90], full 0..1 contrast, bounding-box centre near the frame centre, identical output on repeat calls.
Under system python3 the file re-executes itself with the repo venv, which has numpy and PIL.
"""
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
VENV_PY = HERE.parents[1] / ".venv" / "bin" / "python"  # HERE = <repo>/patch_manager_mods/k1_port

try:
    import numpy as np
    import PIL  # noqa: F401
except ImportError:
    if VENV_PY.exists() and not os.environ.get("K1_TEX_REEXEC"):
        os.environ["K1_TEX_REEXEC"] = "1"
        os.execv(str(VENV_PY), [str(VENV_PY), str(Path(__file__).resolve())])
    print("SKIP: numpy/PIL not available (run with .venv/bin/python)")
    sys.exit(0)

sys.path.insert(0, str(HERE))
import k1_port_tex_decals as dec  # noqa: E402

SIZE = 64
EXPECTED = sorted(
    [
        "cog8", "sprout", "hawk", "chevrons3", "crate_stencil", "patch_round",
        "jaig_eyes", "medallion", "star5", "bar_stack", "sun_disc", "swoop_stripes",
    ]
)  # fmt: skip
TESTS = []


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def bbox_centre_offset(alpha: np.ndarray) -> tuple[float, float]:
    """Offset (dx, dy) in pixels of the bounding box of alpha > 0.5 from the frame centre."""
    rows, cols = np.nonzero(alpha > 0.5)
    mid = (SIZE - 1) / 2.0
    return (cols.min() + cols.max()) / 2.0 - mid, (rows.min() + rows.max()) / 2.0 - mid


@test
def test_registry_has_twelve_names():
    check(dec.list_decals() == EXPECTED, f"list_decals() = {dec.list_decals()}")
    check(set(dec.DECALS) == set(EXPECTED), f"DECALS keys = {sorted(dec.DECALS)}")


@test
def test_every_decal_shape_dtype_range():
    for name in EXPECTED:
        a = dec.render_decal(name, SIZE)
        check(a.shape == (SIZE, SIZE), f"{name}: shape {a.shape}")
        check(a.dtype == np.float32, f"{name}: dtype {a.dtype}")
        check(float(a.min()) >= 0.0 and float(a.max()) <= 1.0, f"{name}: range [{a.min()}, {a.max()}]")


@test
def test_every_decal_coverage_and_contrast():
    for name in EXPECTED:
        a = dec.render_decal(name, SIZE)
        cov = float(a.mean())
        check(0.05 <= cov <= 0.90, f"{name}: coverage {cov:.3f} outside [0.05, 0.90]")
        check(float(a.max()) > 0.9 and float(a.min()) < 0.1, f"{name}: no full on/off contrast (constant-like)")


@test
def test_every_decal_deterministic():
    for name in EXPECTED:
        check(np.array_equal(dec.render_decal(name, SIZE), dec.render_decal(name, SIZE)), f"{name}: not repeatable")


@test
def test_every_decal_centred():
    for name in EXPECTED:
        dx, dy = bbox_centre_offset(dec.render_decal(name, SIZE))
        tol = 0.06 * SIZE  # 3.84 px at size 64
        check(abs(dx) <= tol and abs(dy) <= tol, f"{name}: bbox centre offset ({dx:.2f}, {dy:.2f}) px")


@test
def test_rotation_zero_is_identity_and_rotation_keeps_shape():
    for name in EXPECTED:
        base = dec.render_decal(name, SIZE)
        check(np.array_equal(dec.render_decal(name, SIZE, 0.0), base), f"{name}: rotation 0 changed output")
        check(np.array_equal(dec.render_decal(name, SIZE, 360.0), base), f"{name}: rotation 360 changed output")
        rot = dec.render_decal(name, SIZE, 37.0)
        check(rot.shape == (SIZE, SIZE), f"{name}: rotated shape {rot.shape}")
        check(rot.dtype == np.float32, f"{name}: rotated dtype {rot.dtype}")
        check(float(rot.min()) >= 0.0 and float(rot.max()) <= 1.0, f"{name}: rotated range out of [0, 1]")
        check(abs(float(rot.mean()) - float(base.mean())) < 0.03, f"{name}: rotation changed coverage")


@test
def test_rotation_changes_asymmetric_shape():
    base = dec.render_decal("sprout", SIZE)
    check(not np.array_equal(dec.render_decal("sprout", SIZE, 90.0), base), "sprout rotated 90 equals unrotated")


@test
def test_unknown_name_raises_keyerror():
    try:
        dec.render_decal("no_such_decal", SIZE)
    except KeyError:
        return
    raise AssertionError("render_decal('no_such_decal') did not raise KeyError")


def main() -> int:
    failed = 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}")
        except Exception as exc:  # noqa: BLE001 - report every failing test, keep going
            failed += 1
            print(f"FAIL {fn.__name__}: {exc}")
    print(f"{len(TESTS) - failed}/{len(TESTS)} PASS")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
