#!/usr/bin/env python3
"""Tests for k1_port_cr_sheets.py. Synthetic images and tempdirs only; no game files, no rar.

Run: python3 -u patch_manager_mods/k1_port/test_k1_port_cr_sheets.py
(re-execs under <repo>/.venv/bin/python when numpy is missing; exit 0 = all PASS)
"""
import contextlib
import io
import os
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent


def _repo_root(start):
    for p in [start, *start.parents]:
        if (p / ".git").exists():
            return p
    return start


try:
    import numpy  # noqa: F401
except ImportError:
    _venv_py = _repo_root(HERE) / ".venv" / "bin" / "python"
    if _venv_py.exists() and not os.environ.get("K1PORT_CR_TEST_REEXEC"):
        os.environ["K1PORT_CR_TEST_REEXEC"] = "1"
        os.execv(str(_venv_py), [str(_venv_py), "-u", str(Path(__file__).resolve()), *sys.argv[1:]])
    raise

import numpy as np  # noqa: E402
from PIL import Image  # noqa: E402

sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_cr_sheets as cs  # noqa: E402

TESTS = []


def test(fn):
    TESTS.append(fn)
    return fn


# ---- synthetic image helpers ----

def solid(color, size=64, mode="RGB"):
    return Image.new(mode, (size, size), color)


def structured_gray(size=128):
    """Checkerboard of 16px cells plus a row ramp: strong, regular edges."""
    y, x = np.mgrid[0:size, 0:size]
    cell = ((x // 16) + (y // 16)) % 2
    p = np.where(cell == 1, 200, 40) + (y // 32) * 10
    return np.clip(p, 0, 255).astype(np.float64)


def tint(gray, rgb_scale):
    arr = np.stack([gray * s for s in rgb_scale], axis=-1)
    return Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), "RGB")


def structured_rgb():
    return tint(structured_gray(), (1.0, 0.6, 0.3))


def hue_shifted_rgb():
    # Same spatial luminance structure, different hue: every pixel is a constant multiple of the base.
    return tint(structured_gray(), (0.3, 1.0, 0.6))


def noise_rgb(seed, size=128):
    rng = np.random.default_rng(seed)
    return Image.fromarray(rng.integers(0, 256, (size, size, 3), dtype=np.uint8), "RGB")


# ---- pure functions ----

@test
def test_mad_identical_is_zero():
    img = structured_rgb()
    assert cs.mad(img, img) == 0.0


@test
def test_mad_black_white_is_255():
    v = cs.mad(solid((0, 0, 0)), solid((255, 255, 255)))
    assert abs(v - 255.0) < 1e-6, v


@test
def test_mad_accepts_rgba():
    v = cs.mad(solid((10, 20, 30, 255), mode="RGBA"), solid((10, 20, 30), mode="RGB"))
    assert v == 0.0, v


@test
def test_hist_dist_identical_is_zero():
    img = structured_rgb()
    assert cs.hist_dist(img, img) == 0.0


@test
def test_hist_dist_disjoint_is_two():
    v = cs.hist_dist(solid((0, 0, 0)), solid((255, 255, 255)))
    assert abs(v - 2.0) < 1e-9, v


@test
def test_hist_dist_within_range():
    v = cs.hist_dist(noise_rgb(1), noise_rgb(2))
    assert 0.0 < v <= 2.0, v


@test
def test_layout_corr_identical_is_one():
    img = structured_rgb()
    assert cs.layout_corr(img, img) > 0.999


@test
def test_layout_corr_flat_is_zero():
    assert cs.layout_corr(solid((90, 90, 90)), structured_rgb()) == 0.0
    assert cs.layout_corr(solid((90, 90, 90)), solid((5, 5, 5))) == 0.0


@test
def test_classify_status_table():
    cases = [
        (0.0, 0.0, False, "nobase"),
        (0.0, 1.0, False, "nobase"),
        (0.0, 0.0, True, "identical"),
        (1.99, 0.0, True, "identical"),
        (1.99, 0.99, True, "identical"),  # identical wins over palette
        (2.0, 0.95, True, "palette"),
        (2.0, 0.9, True, "palette"),  # boundary: corr >= 0.9
        (50.0, 0.9, True, "palette"),
        (2.0, 0.89, True, "redesign"),
        (50.0, -0.5, True, "redesign"),
    ]
    for mad_v, corr, has, want in cases:
        got = cs.classify_status(mad_v, corr, has)
        assert got == want, (mad_v, corr, has, got, want)


@test
def test_shifted_hue_copy_is_palette():
    a, b = structured_rgb(), hue_shifted_rgb()
    m, c = cs.mad(a, b), cs.layout_corr(a, b)
    assert m >= 2, m
    assert c >= 0.9, c
    assert cs.classify_status(m, c, True) == "palette"


@test
def test_different_random_pattern_is_redesign():
    a, b = noise_rgb(1), noise_rgb(2)
    m, c = cs.mad(a, b), cs.layout_corr(a, b)
    assert c < 0.9, c
    assert cs.classify_status(m, c, True) == "redesign"


@test
def test_chunk_rows_eleven_items():
    got = cs.chunk_rows(list(range(11)), 5)
    assert [len(r) for r in got] == [5, 5, 1], got
    assert got[0] == [0, 1, 2, 3, 4] and got[2] == [10]


@test
def test_chunk_rows_empty_and_default():
    assert cs.chunk_rows([]) == []
    assert [len(r) for r in cs.chunk_rows(list(range(6)))] == [5, 1]


@test
def test_make_row_size_and_panel_colours():
    row = cs.make_row(solid((255, 0, 0)), None, solid((0, 0, 255)), "n_test")
    assert row.size == (768, 256), row.size
    assert row.mode == "RGB"
    assert row.getpixel((200, 200)) == (255, 0, 0)  # K1 panel
    assert row.getpixel((400, 200)) == (128, 128, 128)  # K2 None -> grey
    assert row.getpixel((700, 200)) == (0, 0, 255)  # CR panel


@test
def test_make_row_all_none_is_grey():
    row = cs.make_row(None, None, None, "n_grey")
    assert row.size == (768, 256)
    for x in (100, 400, 700):
        assert row.getpixel((x, 200)) == (128, 128, 128), x


@test
def test_make_row_label_is_yellow_top_left():
    row = cs.make_row(None, None, None, "n_label")
    # The default font is antialiased, so accept yellow-ish pixels (R,G high, B ~0).
    yellow = [row.getpixel((x, y)) for x in range(0, 60) for y in range(0, 14)
              if row.getpixel((x, y))[0] >= 200 and row.getpixel((x, y))[1] >= 200 and row.getpixel((x, y))[2] <= 20]
    assert yellow, "no yellow stem pixels in top-left label bar"
    black = [row.getpixel((x, y)) for x in range(0, 60) for y in range(0, 14)
             if row.getpixel((x, y)) == (0, 0, 0)]
    assert black, "no black label bar in top-left"


@test
def test_make_sheet_height_and_limits():
    row = cs.make_row(None, None, None, "n_s")
    sheet5 = cs.make_sheet([row] * 5)
    assert sheet5.size == (768, 1280), sheet5.size
    assert cs.make_sheet([row]).size == (768, 256)
    for bad in ([], [row] * 6):
        try:
            cs.make_sheet(bad)
        except ValueError:
            continue
        raise AssertionError(f"make_sheet accepted {len(bad)} rows")


# ---- loaders: missing game dirs give None, never abort ----

@test
def test_loaders_missing_game_dirs_return_none():
    saved = cs.K1_GAME, cs.K2_GAME
    try:
        with tempfile.TemporaryDirectory() as tmp:
            cs.K1_GAME = Path(tmp) / "no_k1"
            cs.K2_GAME = Path(tmp) / "no_k2"
            assert cs.load_k1("n_nothere") is None
            assert cs.load_k2("n_nothere") == (None, None)
    finally:
        cs.K1_GAME, cs.K2_GAME = saved


# ---- CLI end to end on a tempdir shard with monkeypatched base loaders ----

def _write_tga(path, img):
    img.save(path, format="TGA")


@test
def test_diff_and_sheets_end_to_end():
    saved = cs.load_k1, cs.load_k2
    base_same = structured_rgb()
    base_palette = hue_shifted_rgb()
    k1_noise = noise_rgb(7)
    k2_map = {"n_a": base_same, "n_b": base_palette}
    k1_map = {"n_c": k1_noise}

    def fake_k2(stem):
        img = k2_map.get(stem)
        return (img, "K2pack" if img is not None else None)

    def fake_k1(stem):
        return k1_map.get(stem)

    try:
        cs.load_k2, cs.load_k1 = fake_k2, fake_k1
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            cr, out = tmp / "cr", tmp / "out"
            shard = cr / "ab"
            shard.mkdir(parents=True)
            _write_tga(shard / "n_a.tga", base_same)  # identical to K2 base
            _write_tga(shard / "n_b.tga", structured_rgb())  # palette shift vs K2 base
            _write_tga(shard / "n_c.tga", structured_rgb())  # K1-only base, random pattern -> redesign
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                assert cs.main(["diff", "--shard", "ab", "--cr-dir", str(cr), "--out", str(out)]) == 0
            tsv = out / "cr_vs_base_ab.tsv"
            lines = tsv.read_text(encoding="utf-8").splitlines()
            assert lines[0] == cs.TSV_HEADER, lines[0]
            rows = {r["stem"]: r for r in (dict(zip(cs.TSV_COLS, ln.split("\t"))) for ln in lines[1:])}
            assert set(rows) == {"n_a", "n_b", "n_c"}, rows.keys()
            assert rows["n_a"]["status"] == "identical" and rows["n_a"]["base_src"] == "K2pack"
            assert rows["n_b"]["status"] == "palette" and rows["n_b"]["base_src"] == "K2pack"
            assert rows["n_c"]["status"] == "redesign" and rows["n_c"]["base_src"] == "K1pack"
            assert rows["n_a"]["cr_dims"] == "128x128", rows["n_a"]["cr_dims"]
            assert "identical=1" in buf.getvalue(), buf.getvalue()

            with contextlib.redirect_stdout(io.StringIO()):
                assert cs.main(["sheets", "--shard", "ab", "--cr-dir", str(cr), "--out", str(out)]) == 0
            pngs = sorted((out / "sheets").glob("ab_*.png"))
            assert len(pngs) == 1, pngs
            assert Image.open(pngs[0]).size == (768, 512), Image.open(pngs[0]).size
            index = out / "sheets_index.tsv"
            idx = index.read_text(encoding="utf-8").splitlines()
            assert idx == ["sheets/ab_01.png\tn_b,n_c"], idx

            # Re-running sheets replaces this shard's index rows instead of appending.
            with contextlib.redirect_stdout(io.StringIO()):
                cs.main(["sheets", "--shard", "ab", "--cr-dir", str(cr), "--out", str(out)])
            assert index.read_text(encoding="utf-8").splitlines() == idx
    finally:
        cs.load_k1, cs.load_k2 = saved


@test
def test_corrupt_tga_records_error_and_run_continues():
    saved = cs.load_k1, cs.load_k2
    base = structured_rgb()

    def fake_k2(stem):
        return (base, "K2override") if stem in ("n_bad", "n_ok") else (None, None)

    try:
        cs.load_k2, cs.load_k1 = fake_k2, (lambda stem: None)
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            cr, out = tmp / "cr", tmp / "out"
            shard = cr / "cd"
            shard.mkdir(parents=True)
            (shard / "n_bad.tga").write_bytes(b"not a tga at all")
            _write_tga(shard / "n_ok.tga", base)
            with contextlib.redirect_stdout(io.StringIO()):
                assert cs.main(["diff", "--shard", "cd", "--cr-dir", str(cr), "--out", str(out)]) == 0
            rows = {}
            for ln in (out / "cr_vs_base_cd.tsv").read_text(encoding="utf-8").splitlines()[1:]:
                parts = ln.split("\t")
                rows[parts[0]] = parts
            assert rows["n_bad"][-1] == "error" and rows["n_bad"][1] == "K2override", rows["n_bad"]
            assert rows["n_ok"][-1] == "identical", rows["n_ok"]
    finally:
        cs.load_k1, cs.load_k2 = saved


def main():
    failed = 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}", flush=True)
        except Exception as exc:  # report every failure, keep going
            failed += 1
            print(f"FAIL {fn.__name__}: {type(exc).__name__}: {exc}", flush=True)
    print(f"{len(TESTS) - failed}/{len(TESTS)} PASS", flush=True)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
