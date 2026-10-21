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
        _DUMP = M.run_dump(*M.DEFAULT_PANES_3840x1600, desc=M.default_desc(M.DEFAULT_PANES_3840x1600[2]), build_dir=_BUILD)
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
    for i in range(74):
        ok = any(f"i == {n}" in body and i == pi[n] for n in ("PI_MEMBER_PREV", "PI_MEMBER_NEXT", "PI_SAVE", "PI_REVERT", "PI_RESET_SCOPE", "PI_REMOVE", "PI_VIEW_RESET", "PI_VIEW_FLIP", "PI_PAUSE"))
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
        self.assertEqual(len(d["items"]), 74)
        self.assertEqual([it["i"] for it in d["items"]], list(range(74)))
        for key in ("column", "view", "preview"):
            self.assertEqual(len(d["panes"][key]), 4)
        self.assertEqual(d["panes"]["desc"], list(M.default_desc(M.DEFAULT_PANES_3840x1600[2])))
        self.assertEqual(set(d["rows"]), {"leftRowH", "inspRowH", "barH"})
        self.assertGreaterEqual(d["rows"]["leftRowH"], 34)   # grows with the layout constants (34 in 0.4.3, 52 now)

    def test_kinds_match_item_is_button(self):
        d = dump()
        buttons = {it["i"] for it in d["items"] if it["kind"] == "button"}
        self.assertEqual(buttons, cpp_button_indices())
        self.assertEqual(buttons, {0, 2, *range(6, 18), *range(19, 25), *range(26, 31), 33, 35, 40, 45, 50, 55, 60, 65, 69, 72, 73})
        self.assertEqual([it["i"] for it in d["items"] if it["kind"] == "view"], [25])


class RemoveMenuTests(unittest.TestCase):
    def _side(self, extra=()):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / "p.png"
            M.main(["--panes", "3840x1600", "--mode", "engine-model", "--caption-fix", "1", "--crop", "--build-dir", _BUILD, "--out", str(out), *extra])
            return json.loads(out.with_suffix(".json").read_text())

    def test_labels_and_toggle(self):
        by = {it["i"]: it for it in self._side()["items"]}
        self.assertEqual([by[i]["text"] for i in (13, 14, 15)], ["This weapon model", "All melee weapons", "This body + armor"])
        self.assertEqual(by[69]["text"], "...")   # the "Reset" button became the small "..." toggle
        self.assertEqual(by[69]["kind"], "button")
        for i in (13, 14, 15, 69):   # every new caption is drawn whole, not cut with "..."
            self.assertEqual(by[i]["shown"], by[i]["text"], i)
            self.assertFalse(by[i]["wraps"], i)
        self.assertTrue(by[68]["text"].startswith("Save changes go to "))   # the live default sentence of the current scope

    def test_remove_closed_is_hidden(self):
        side = self._side()
        r = {it["i"]: it for it in side["items"]}[72]
        self.assertTrue(r.get("hidden"))
        self.assertFalse(r["drawn"])
        self.assertFalse(side["remove_open"])

    def test_remove_open_is_drawn_and_in_the_inspector(self):
        side = self._side(["--remove-open"])
        by = {it["i"]: it for it in side["items"]}
        r = by[72]
        self.assertEqual((r["kind"], r["text"], r["shown"]), ("button", "Remove saved placement", "Remove saved placement"))
        self.assertTrue(r["drawn"] and not r.get("wraps"))
        self.assertTrue(side["remove_open"])
        pv = side["panes"]["preview"]
        b = r["box"]
        self.assertTrue(pv[0] <= b[0] and b[0] + b[2] <= pv[0] + pv[2] and pv[1] <= b[1] and b[1] + b[3] <= pv[1] + pv[3])   # inside the preview (inspector) pane
        self.assertGreaterEqual(b[1], by[66]["box"][1] + by[66]["box"][3])   # under the note
        self.assertTrue(by[68]["text"].startswith("Removes the saved placement of "))   # the left box describes what Remove does
        self.assertFalse(by[68]["overflow"])


class DescBoxTests(unittest.TestCase):
    def _side(self, extra=()):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / "p.png"
            M.main(["--panes", "3840x1600", "--mode", "engine-model", "--caption-fix", "1", "--crop", "--build-dir", _BUILD, "--out", str(out), *extra])
            return json.loads(out.with_suffix(".json").read_text())

    def test_default_desc_derivation(self):
        # bottom 15% of the original pane (preview = top 85%): 746 * 150 // 850 = 131 px, right under the preview, same x / width
        self.assertEqual(M.default_desc((1953, 357, 1113, 746)), (1953, 1103, 1113, 131))
        # the default 3840x1600 panes carry the live ini margin_px 120 (column grows left, preview grows right), so the offline page matches the live one
        col, view, prev = M.DEFAULT_PANES_3840x1600
        self.assertEqual((M.MARGIN_PX, col, view, prev), (120, (652, 357, 1252, 757), (1345, 357, 576, 896), (1953, 357, 1233, 746)))
        self.assertEqual(M.default_desc(prev), (1953, 1103, 1233, 131))

    def test_both_boxes_same_bottom_and_no_text_overlap(self):
        side = self._side()
        by = {it["i"]: it for it in side["items"]}
        left, right = by[67]["box"], by[70]["box"]
        self.assertTrue(left[2] > 0 and left[3] > 0 and right[2] > 0 and right[3] > 0)
        self.assertEqual(left[1] + left[3], right[1] + right[3])
        d = side["panes"]["desc"]   # the right box is the description pane grown by the cover (desc_cover_px 6): contains it, bottom + 6, left - 6, top stays under the preview
        self.assertEqual(right, [d[0] - 6, d[1], d[2] + 6, d[3] + 6])   # the right edge stays at the preview right (the mockup has no right margin)
        self.assertEqual(by[68]["text"], M.LEFT_DESC_TEXT)
        self.assertEqual(by[71]["text"], M.RIGHT_DESC_TEXT)
        self.assertEqual((by[67]["text"], by[70]["text"]), ("", ""))   # the frames carry no caption (the old table put hint text on them -> two labels at one spot)
        for i in (68, 71):   # text stays inside its box
            self.assertFalse(by[i]["overflow"], i)
            tr, bx = by[i]["text_rect"], by[i]["box"]
            self.assertTrue(bx[0] <= tr[0] and tr[0] + tr[2] <= bx[0] + bx[2] and bx[1] <= tr[1] and tr[1] + tr[3] <= bx[1] + bx[3], (i, tr, bx))
        rects = [(it["i"], it["text_rect"]) for it in side["items"] if it.get("text_rect")]
        self.assertTrue(len(rects) > 40)
        bad = [(a, b) for k, (a, ra) in enumerate(rects) for b, rb in rects[k + 1:]
               if min(ra[0] + ra[2], rb[0] + rb[2]) > max(ra[0], rb[0]) and min(ra[1] + ra[3], rb[1] + rb[3]) > max(ra[1], rb[1])]
        self.assertEqual(bad, [])

    def test_desc_option_and_absent(self):
        side = self._side(["--desc", "1953,1103,1113,160"])
        by = {it["i"]: it for it in side["items"]}
        self.assertEqual(by[70]["box"], [1947, 1103, 1125, 166])   # the pane grown by the default cover (6) on all four sides: the margin-widened preview (live) no longer clamps its right edge
        self.assertEqual(by[67]["box"][1] + by[67]["box"][3], 1269)
        side = self._side(["--desc", "1953,1103,1113,160", "--desc-cover-px", "0"])
        by = {it["i"]: it for it in side["items"]}
        self.assertEqual(by[70]["box"], [1953, 1103, 1113, 160])   # cover 0 = the old behaviour
        self.assertEqual(by[67]["box"][1] + by[67]["box"][3], 1263)
        side = self._side(["--no-desc"])
        self.assertEqual(side["panes"]["desc"], [0, 0, 0, 0])


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


class WrapTests(unittest.TestCase):
    def test_engine_scale(self):
        self.assertAlmostEqual(M.engine_scale(3840), 1.98)   # live probe: 1.98 px per texel at 3840
        self.assertAlmostEqual(M.fit_scale(2.65), 1.9875)    # new DLL fit measure ~ the engine measure
        self.assertAlmostEqual(M.fit_scale(2.5), 1.875)

    def test_wrap_unit(self):
        sc = M.engine_scale(3840)
        w = M.text_px("Upper back", sc)
        self.assertEqual(M.wrap_text("Upper back", w + 1, sc), ["Upper back"])
        self.assertEqual(M.wrap_text("Upper back", w - 1, sc), ["Upper", "back"])
        self.assertEqual(len(M.wrap_text("Roll/Turn", M.text_px("Roll/Turn", sc) - 1, sc)), 2)   # one long word breaks at character level
        self.assertEqual(M.wrap_text("", 100, sc), [""])

    def test_block_centring(self):
        _, _, y1 = M.place_caption("button", (0, 0, 200, 34), mode="ideal", lines=1)
        _, _, y2 = M.place_caption("button", (0, 0, 200, 34), mode="ideal", lines=2)
        _, _, y3 = M.place_caption("button", (0, 0, 200, 34), mode="ideal", lines=3)
        self.assertEqual((y1, y2, y3), (1, -15, -31))   # (B - n*32) // 2

    def _wraps(self, fit_args, fix=1, mode="engine-model"):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / "p.png"
            M.main(["--panes", "3840x1600", "--mode", mode, "--caption-fix", str(fix), "--crop", "--build-dir", _BUILD, "--out", str(out)] + fit_args)
            return json.loads(out.with_suffix(".json").read_text())

    def test_legacy_fit_reproduces_wraps(self):
        # Calibration against the 0.4.3-dev live probe (10-18 16:37): the scope buttons were 142x34 boxes then (CAPTION i=13..15 btn=...).
        # The current HseLayoutV3.h is wider, so the live boxes are pinned in a copy of the dump. Live also showed i=8 "Upper back" lines=2;
        # the model cuts it to one line at 142 px (known gap, F3 notes), so only the scope rows are asserted.
        d = json.loads(json.dumps(dump()))
        live = {13: [776, 613, 142, 34], 14: [920, 613, 142, 34], 15: [1064, 613, 142, 34]}
        for it in d["items"]:
            if it["i"] in live:
                it["box"] = live[it["i"]]
        with tempfile.TemporaryDirectory() as td:
            dj = Path(td) / "live043.json"
            dj.write_text(json.dumps(d))
            side = self._wraps(["--legacy-fit", "--dump-json", str(dj)])
        by = {it["i"]: it for it in side["items"]}
        self.assertEqual((side["button_fit_pad"], side["label_fit_pad"], side["text_k"]), (8, 8, 2.5))
        self.assertGreaterEqual(sum(1 for i in (13, 14, 15) if by[i]["wraps"]), 2, [by[i]["shown"] for i in (13, 14, 15)])   # scope buttons

    def test_new_fit_no_button_wraps(self):
        for mode in ("engine-model", "ideal"):
            side = self._wraps([], mode=mode)
            self.assertEqual((side["button_fit_pad"], side["label_fit_pad"], side["text_k"]), (20, 8, 2.65))
            bad = [(it["i"], it["shown"]) for it in side["items"] if it["kind"] == "button" and it["wraps"]]
            self.assertEqual(bad, [], mode)
            self.assertTrue(all("lines" in it and "wraps" in it for it in side["items"] if it["kind"] != "view"))

    def test_inspector_values(self):
        self.assertEqual([M.caption_for(36 + r * 5 + 3) for r in (0, 1, 3)], ["0.0 cm", "-11.0 cm", "30.5 deg"])


class SmokeTests(unittest.TestCase):
    def test_render(self):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / "p.png"
            self.assertEqual(M.main(["--panes", "3840x1600", "--mode", "engine-model", "--caption-fix", "1", "--crop", "--build-dir", _BUILD, "--out", str(out)]), 0)
            self.assertTrue(out.stat().st_size > 1000)
            side = json.loads(out.with_suffix(".json").read_text())
            self.assertEqual(len(side["items"]), 74)
            self.assertIn("crop", side)
            btn = [it for it in side["items"] if it["kind"] == "button" and it["box"][3] >= 34]
            self.assertTrue(btn and all(it["drawn"] for it in btn if it["text"] and not it.get("hidden")))


if __name__ == "__main__":
    unittest.main(verbosity=1)
