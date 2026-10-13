#!/usr/bin/env python3
"""Tests for k1_port_uvmask_overlay.py. Tempdirs only; nothing is written next to the repo.

Needs numpy + PIL: under a bare interpreter it re-execs REPO/.venv/bin/python, else it SKIPs (exit 0).
The real-atlas test SKIPs when the staged K1 override k1p_commm04.tpc is absent.
"""
import json
import os
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
try:
    import numpy as np
    from PIL import Image
except ImportError:
    VENV_PY = REPO / ".venv" / "bin" / "python"
    if VENV_PY.exists() and not os.environ.get("K1_TEX_REEXEC"):
        os.environ["K1_TEX_REEXEC"] = "1"
        os.execv(str(VENV_PY), [str(VENV_PY), os.path.abspath(__file__)])
    print("SKIP all: numpy/PIL not available")
    sys.exit(0)

sys.path.insert(0, str(HERE))
import k1_port_uvmask_overlay as ov  # noqa: E402

CLI = HERE / "k1_port_uvmask_overlay.py"
TESTS = []
DIM_GREY = (100, 100, 100)  # atlas grey 200 dimmed by DIM


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def solid(w, h, v):
    return Image.new("RGB", (w, h), (v, v, v))


def make_tga(path: Path, w: int, h: int) -> Path:
    """Uncompressed 32-bit TGA, solid grey 200, bottom-left origin (descriptor 8 = 8 alpha bits)."""
    header = struct.pack("<BBBHHBHHHHBB", 0, 0, 2, 0, 0, 0, 0, 0, w, h, 32, 8)
    path.write_bytes(header + bytes([200, 200, 200, 255]) * (w * h))
    return path


def make_masks(d: Path, w: int, h: int, labels) -> Path:
    """U4-style mask dir: masks.json + <label>.png; each label fills its first row block."""
    d.mkdir(parents=True, exist_ok=True)
    for i, lab in enumerate(labels):
        arr = np.zeros((h, w), dtype=np.uint8)
        arr[i:i + 2, :2] = 255
        Image.fromarray(arr, "L").save(d / f"{lab}.png")
    meta = {"labels": {lab: {"coverage": 0.0} for lab in labels}, "flip": "none", "size": [w, h]}
    (d / "masks.json").write_text(json.dumps(meta), encoding="utf-8")
    return d


def run_cli(args):
    return subprocess.run([sys.executable, str(CLI), *args], capture_output=True, text=True)


@test
def test_mask_shades_toward_label():
    mb = np.zeros((5, 5), dtype=bool)
    mb[1:4, 1:4] = True
    overlays, _ = ov.compose(solid(5, 5, 200), {"lab": mb}, {}, tile=32)
    px = np.asarray(overlays["lab"]).astype(int)
    col = np.array(ov.PALETTE[0])
    dim = np.array(DIM_GREY)
    centre = px[2, 2]
    check(np.abs(centre - col).sum() < np.abs(dim - col).sum(), f"centre {centre} not shaded toward {col}")
    check(not (centre == dim).all(), "masked centre equals dimmed value")
    check((px[1, 1] == col).all(), f"outline pixel {px[1, 1]} is not label colour")
    check((px[0, 0] == dim).all(), f"unmasked pixel {px[0, 0]} is not dimmed")


@test
def test_unmasked_dim_identical_across_labels():
    a = np.zeros((6, 6), dtype=bool)
    a[0:2, 0:2] = True
    b = np.zeros((6, 6), dtype=bool)
    b[3:5, 3:5] = True
    overlays, _ = ov.compose(solid(6, 6, 200), {"a": a, "b": b}, {}, tile=32)
    pa, pb = np.asarray(overlays["a"]), np.asarray(overlays["b"])
    for y, x in [(5, 0), (0, 5), (2, 2)]:
        check(tuple(pa[y, x]) == tuple(pb[y, x]) == DIM_GREY,
              f"unmasked ({y},{x}) differs: a={tuple(pa[y, x])} b={tuple(pb[y, x])}")


@test
def test_mask_threshold_is_128():
    overlays, _ = ov.compose(solid(2, 2, 200), {"t": bytes([127, 128, 0, 255])}, {}, tile=16)
    px = np.asarray(overlays["t"])
    check(tuple(px[0, 0]) == DIM_GREY, "value 127 treated as in-mask")
    check(tuple(px[0, 1]) == ov.PALETTE[0], "value 128 not treated as in-mask")


@test
def test_contact_sheet_dimensions():
    atlas = solid(8, 4, 90)
    masks = {f"m{i}": np.zeros((4, 8), dtype=bool) for i in range(5)}
    _, sheet = ov.compose(atlas, masks, {}, tile=64)
    check(sheet.size == (4 * 64, 2 * 64 + ov.BAND_PX), f"6 tiles gave {sheet.size}")
    _, sheet1 = ov.compose(atlas, {"m0": np.zeros((4, 8), dtype=bool)}, {}, tile=64)
    check(sheet1.size == (2 * 64, 64 + ov.BAND_PX), f"2 tiles gave {sheet1.size}")


@test
def test_cli_refuses_out_inside_repo():
    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        masks = make_masks(td / "masks", 4, 4, ["alpha"])
        atlas = make_tga(td / "atlas.tga", 4, 4)
        target = REPO / "patch_manager_mods" / "k1_port" / "__u05_refusal_probe__"
        check(not target.exists(), f"probe dir already exists: {target}")
        r = run_cli(["render", "--masks", str(masks), "--atlas", str(atlas), "--out", str(target)])
        check(r.returncode == 2, f"expected exit 2, got {r.returncode}: {r.stderr.strip()}")
        check(not target.exists(), "refused run still created the repo dir")


@test
def test_cli_render_writes_overlays_and_contact():
    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        masks = make_masks(td / "masks", 4, 4, ["alpha", "beta"])
        atlas = make_tga(td / "atlas.tga", 4, 4)
        out = td / "out"
        r = run_cli(["render", "--masks", str(masks), "--atlas", str(atlas), "--out", str(out), "--tile", "32"])
        check(r.returncode == 0, f"exit {r.returncode}: {r.stderr.strip()}")
        for name in ("alpha.png", "beta.png", "contact.png"):
            check((out / name).is_file(), f"missing {name}")
        sheet = Image.open(out / "contact.png")
        check(sheet.size == (3 * 32, 32 + ov.BAND_PX), f"contact size {sheet.size}")


@test
def test_cli_rejects_mask_size_mismatch():
    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        masks = make_masks(td / "masks", 4, 4, ["alpha"])
        atlas = make_tga(td / "atlas.tga", 8, 8)
        r = run_cli(["render", "--masks", str(masks), "--atlas", str(atlas), "--out", str(td / "out")])
        check(r.returncode == 2, f"expected exit 2, got {r.returncode}")
        check(not (td / "out").exists(), "output written despite size mismatch")


@test
def test_unknown_atlas_name_is_error():
    try:
        ov.load_atlas("__u05_no_such_texture__")
    except ov.AtlasError:
        return
    raise AssertionError("unknown texture name did not raise AtlasError")


@test
def test_real_staged_atlas_composes():
    if not (ov.STAGE / "k1p_commm04.tpc").is_file():
        raise Skip(f"{ov.STAGE / 'k1p_commm04.tpc'} absent")
    atlas = ov.load_atlas("k1p_commm04")
    check(atlas.size == (512, 512), f"staged atlas size {atlas.size}")
    mb = np.zeros((512, 512), dtype=bool)
    mb[:64, :64] = True
    overlays, sheet = ov.compose(atlas, {"corner": mb}, {}, tile=128)
    check(overlays["corner"].size == (512, 512), "overlay size mismatch")
    check(sheet.size == (2 * 128, 128 + ov.BAND_PX), f"sheet size {sheet.size}")


def main():
    passed = 0
    for fn in TESTS:
        name = fn.__name__
        try:
            fn()
            print(f"PASS {name}")
            passed += 1
        except Skip as e:
            print(f"SKIP {name}: {e}")
            passed += 1
        except Exception as e:  # AssertionError or unexpected error: both are failures
            print(f"FAIL {name}: {e}")
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
