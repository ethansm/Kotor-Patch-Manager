#!/usr/bin/env python3
"""Stdlib unittest for scripts/quick_loot_mockup.py: formatter port vs quick_loot_logic_test.cpp vectors, width measurement
on a synthetic glyph table, and (skipped without PIL/pykotor/the install) a real render smoke test.

run: python3 -m unittest scripts/test_quick_loot_mockup.py
"""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import quick_loot_mockup as q  # noqa: E402


class FormatterVectors(unittest.TestCase):
    N3 = ["Medpac", "Blaster Pistol", "Vibroblade"]

    def test_took_stack_suffix(self):
        # cpp: "stack suffix for 2 and 999, none for 1"
        self.assertEqual(q.fmt_took(self.N3, [2, 1, 999], 3), "Took: Medpac x2, Blaster Pistol, Vibroblade x999")

    def test_took_garbage_names(self):
        # cpp: "garbage bytes become ?, empty/null/blank become item, NUL ends the name"
        ng = ["Med\x01pac\xff", "", None, "   ", "Name\0Hidden"]
        self.assertEqual(q.fmt_took(ng, None, 5), "Took: Med?pac?, item, item, item, Name")

    def test_took_81_chars_overflows(self):
        # cpp: "81 chars overflows to +1 more"
        a, b33 = "A" * 40, "B" * 33
        self.assertEqual(q.fmt_took([a, b33], None, 2), "Took: " + a + ", +1 more")
        self.assertEqual(len(q.fmt_took([a, "B" * 32], None, 2)), 80)   # exactly 80 fits whole

    def test_peek_label(self):
        # cpp: all shown / singular / +K more / empty
        self.assertEqual(q.fmt_peek("F", 3, self.N3, [2, 1, 1], 3), "3 items  [F] take all: Medpac x2, Blaster Pistol, Vibroblade")
        self.assertEqual(q.fmt_peek("F", 1, ["Medpac"], None, 1), "1 item  [F] take all: Medpac")
        self.assertEqual(q.fmt_peek("F", 6, self.N3, [2, 1, 1], 3),
                         "6 items  [F] take all: Medpac x2, Blaster Pistol, Vibroblade, +3 more")
        self.assertEqual(q.fmt_peek("F", 0, self.N3, [2, 1, 1], 0), "")

    def test_peek_100_boundary(self):
        a, b, c = "A" * 38, "B" * 37, "B" * 38
        self.assertEqual(len(q.fmt_peek("F", 2, [a, b], None, 2)), 100)
        self.assertIn(", +1 more", q.fmt_peek("F", 2, [a, c], None, 2))

    def test_long_list_caps(self):
        nm = ["N" * 20] * 16
        t = q.fmt_peek("F", 25, nm, None, 16)
        self.assertLessEqual(len(t), q.PEEK_MAX_CHARS)
        k = int(t.rsplit("+", 1)[1].split()[0])
        self.assertEqual(t.count("N" * 20) + k, 25)

    def test_name_cut_at_40(self):
        self.assertEqual(len(q.fmt_took(["Z" * 100], None, 1)), 46)


class Geometry(unittest.TestCase):
    def test_rects(self):
        # cpp: peekRectName(100, 50, 200, 24) and peekRectFixed(0, 0, 1920, 1080)
        self.assertEqual(q.toast_rect(100, 50, 200, 24, w=q.PEEK_W), (100 + 100 - q.PEEK_W // 2, 50 + 24 + 20, q.PEEK_W, 20))
        self.assertEqual(q.fixed_rect(0, 0, 1920, 1080), (960 - q.PEEK_W // 2, 669, q.PEEK_W, 20))


class WidthMeasure(unittest.TestCase):
    def test_synthetic_glyph_table(self):
        adv = [0] * 256
        adv[ord("A")], adv[ord("B")], adv[ord(" ")], adv[ord("?")] = 8, 6, 3, 5
        self.assertEqual(q.text_width("AB A", adv, 2.0), (8 + 6 + 3 + 8) * 2.0)
        self.assertEqual(q.text_width("", adv, 2.75), 0)
        self.assertEqual(q.text_width("中", adv, 1.0), 5)   # non-Latin1 measures as '?'
        # fit rule: width <= label width
        w = q.text_width("A" * 10, adv, 2.75)
        self.assertEqual(w, 220.0)
        self.assertTrue(w <= q.TOAST_W)
        self.assertFalse(q.text_width("A" * 20, adv, 2.75) <= q.TOAST_W)


class RenderSmoke(unittest.TestCase):
    def test_render(self):
        try:
            import PIL  # noqa: F401
            cpm = q._cpm()
            if not (cpm.GAME / "override").is_dir():
                self.skipTest("game install missing")
            cpm.inst()
        except Exception as e:  # missing PIL / pykotor / install
            self.skipTest(f"render prerequisites missing: {e}")
        with tempfile.TemporaryDirectory() as d:
            rows, table, written = q.render_all(d)
            self.assertEqual(len(rows), 6)
            self.assertEqual(len(written), 12)
            from PIL import Image
            self.assertEqual(Image.open(written[0]).size, (3840, 1600))
            self.assertTrue(rows[4][7])   # the 1-item peek fits 760 px


if __name__ == "__main__":
    unittest.main()
