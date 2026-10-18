#!/usr/bin/env python3
"""Tests for scripts/holster_editor_mockup.py. Run: .venv/bin/python scripts/test_holster_editor_mockup.py"""
import json
import re
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
try:
    import PIL  # noqa: F401  (run_tests.py runs py: items with system python3, which has no PIL: re-run under the repo venv)
except ImportError:
    import os
    _venv = HERE.parent / ".venv" / "bin" / "python"
    if not _venv.exists():
        print(f"SKIP: PIL missing and no {_venv}")
        sys.exit(0)
    os.execv(str(_venv), [str(_venv), str(Path(__file__).resolve())] + sys.argv[1:])
sys.path.insert(0, str(HERE))
import holster_editor_mockup as M  # noqa: E402

_BUILD = tempfile.mkdtemp(prefix="hse_mockup_test_")
_DUMP = None


def dump():
    global _DUMP
    if _DUMP is None:
        _DUMP = M.run_dump(*M.DEFAULT_PANES_3840x1600, build_dir=_BUILD)
    return _DUMP


def cpp_button_indices():
    """Button list derived from the C++ itemIsButton (HolsterEditor.cpp) by evaluating its PI_* ranges against HseLayoutV3.h constants."""
    h = (M.SRC_DIR / "HseLayoutV3.h").read_text()
    pi = {k: int(v) for k, v in re.findall(r"\b(PI_[A-Z_]+)\s*=\s*(\d+)", h)}
    k = {n: int(v) for n, v in re.findall(r"constexpr int (kV3(?:Weapons|Bones|Scopes|Modes|Stances|Rows|Groups))\s*=\s*(\d+)", h)}
    stride = int(re.search(r"constexpr int kRowStride\s*=\s*(\d+)", h).group(1))
    src = (M.SRC_DIR / "HolsterEditor.cpp").read_text()
    body = src[src.index("bool itemIsButton"):src.index("bool itemIsResetIcon")]
    out = set()
    for i in range(70):
        ok = any(f"i == {n}" in body and i == pi[n] for n in ("PI_MEMBER_PREV", "PI_MEMBER_NEXT", "PI_SAVE", "PI_REVERT", "PI_RESET_SCOPE", "PI_VIEW_RESET", "PI_VIEW_FLIP"))
        for name, base, cnt in (("PI_WEAPON", "PI_WEAPON", "kV3Weapons"), ("PI_BONE", "PI_BONE", "kV3Bones"), ("PI_SCOPE", "PI_SCOPE", "kV3Scopes"),
                                ("PI_MODE", "PI_MODE", "kV3Modes"), ("PI_STANCE", "PI_STANCE", "kV3Stances")):
            assert f"i >= {name}" in body, name
            ok = ok or pi[base] <= i < pi[base] + k[cnt]
        for g in range(k["kV3Groups"]):
            ok = ok or i == pi["PI_GROUP"] + g * 2 + 1
        for r in range(k["kV3Rows"]):
            ok = ok or i == pi["PI_ROW"] + r * stride + 4
        if ok:
            out.add(i)
    return out


class DumpTests(unittest.TestCase):
    def test_json_and_counts(self):
        d = dump()
        self.assertEqual(len(d["items"]), 70)
        self.assertEqual([it["i"] for it in d["items"]], list(range(70)))
        for key in ("column", "view", "preview"):
            self.assertEqual(len(d["panes"][key]), 4)
        self.assertEqual(set(d["rows"]), {"leftRowH", "inspRowH", "barH"})
        self.assertEqual(d["rows"]["leftRowH"], 34)

    def test_kinds_match_item_is_button(self):
        d = dump()
        buttons = {it["i"] for it in d["items"] if it["kind"] == "button"}
        self.assertEqual(buttons, cpp_button_indices())
        self.assertEqual(buttons, {0, 2, *range(6, 18), *range(19, 25), *range(26, 31), 33, 35, 40, 45, 50, 55, 60, 65, 69})
        self.assertEqual([it["i"] for it in d["items"] if it["kind"] == "view"], [25])


class PlacementTests(unittest.TestCase):
    def test_inner_rect(self):
        # 34 high, 432 wide: ih = 2 -> 2//16 == 0 and 2 < 8 -> centre, hi 0; x = X+16, w = 400 +/- min(9,16)
        self.assertEqual(M.inner_rect((100, 200, 432, 34)), (100 + 16 - 9, 200 + 17 - 9, 400 + 18, 18))
        # a box under 2 G collapses on x too
        self.assertEqual(M.inner_rect((10, 10, 20, 20)), (10 + 10 - 9, 10 + 10 - 9, 18, 18))

    def test_drop_rule(self):
        rect, drawn, _ = M.place_caption("button", (0, 0, 200, 34))
        self.assertEqual(rect[3], 18)
        self.assertFalse(drawn)
        _, drawn, y0 = M.place_caption("button", (0, 0, 200, 34), no_drop=True)
        self.assertTrue(drawn)
        self.assertEqual(y0, 8 + (18 - 32) // 2)   # rect y = 17 - 9
        # a tall button keeps its caption: H=64 -> inner h 32, rect h 50, y = 7 -> y0 = 7 + 9
        _, drawn, y0 = M.place_caption("button", (0, 0, 200, 64))
        self.assertTrue(drawn)
        self.assertEqual(y0, 16)

    def test_caption_fix(self):
        rect, drawn, y0 = M.place_caption("button", (100, 200, 432, 34), fix=1)
        self.assertEqual((rect[1], rect[3]), (200, 34))
        self.assertEqual(rect[0], 100 + 16 - 9)
        self.assertTrue(drawn)
        self.assertEqual(y0, 200 + 1)
        # engine width 0 (m=0 on a 34 px box) -> (X+4, Y, W-8, H)
        rect, drawn, _ = M.place_caption("button", (100, 200, 34, 34), fix=1, m=0)
        self.assertEqual(rect, (104, 200, 26, 34))
        self.assertTrue(drawn)

    def test_ideal_and_labels(self):
        rect, drawn, y0 = M.place_caption("button", (10, 20, 100, 34), mode="ideal")
        self.assertEqual((rect, drawn, y0), ((10, 20, 100, 34), True, 21))
        rect, drawn, _ = M.place_caption("label", (10, 20, 100, 22))
        self.assertEqual(rect, (10, 20, 100, 22))
        self.assertTrue(drawn)

    def test_text_scale_and_widths(self):
        self.assertEqual(M.text_scale(3840), 2.0)
        self.assertEqual(M.text_scale(3440), 2.0)
        self.assertEqual(M.text_scale(1920), 1.0)
        self.assertEqual(len(M.texel_widths()["dialogfont16x16"]), 256)
        self.assertEqual(M.fit_text("Save", 1000, 2.0), "Save")
        self.assertTrue(M.fit_text("Upper back", 60, 2.0).endswith("..."))

    def test_geom_for(self):
        col, view, prev = M.geom_for(3440, 1440)
        self.assertEqual(view, (1345, 357, 576, 896))
        self.assertEqual(col, (150, 357, 400, 896))
        self.assertEqual(prev, (1925, 357, 500, 700))
        self.assertEqual(M.geom_for(1920, 1080)[1], (1345 * 1920 // 3440, 357 * 1080 // 1440, 576 * 1920 // 3440, 896 * 1080 // 1440))


class SmokeTests(unittest.TestCase):
    def test_render(self):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / "p.png"
            self.assertEqual(M.main(["--panes", "3840x1600", "--mode", "engine-model", "--caption-fix", "1", "--crop", "--build-dir", _BUILD, "--out", str(out)]), 0)
            self.assertTrue(out.stat().st_size > 1000)
            side = json.loads(out.with_suffix(".json").read_text())
            self.assertEqual(len(side["items"]), 70)
            self.assertIn("crop", side)
            btn = [it for it in side["items"] if it["kind"] == "button" and it["box"][3] == 34]
            self.assertTrue(btn and all(it["drawn"] for it in btn if it["text"]))


if __name__ == "__main__":
    unittest.main(verbosity=1)
