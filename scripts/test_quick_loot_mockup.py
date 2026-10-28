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
        # cpp: "count in brackets, stack suffix for 2 and 999, none for 1"
        self.assertEqual(q.fmt_took(self.N3, [2, 1, 999], 3), "Took (3): Medpac x2, Blaster Pistol, Vibroblade x999")
        self.assertEqual(q.fmt_took(["Medpac"], None, 1), "Took: Medpac")

    def test_took_garbage_names(self):
        ng = ["Med\x01pac\xff", "", None, "   ", "Name\0Hidden"]
        self.assertEqual(q.fmt_took(ng, None, 5), "Took (5): Med?pac?, item, item, item, Name")

    def test_review_examples(self):
        # cpp: review examples 1 and 2, and the Took twin
        n2 = ["Medpac", "Blaster Pistol"]
        self.assertEqual(q.fmt_took(n2, [2, 1], 2, 2), "Took (4): Medpac x2, Blaster Pistol, +2 more")
        self.assertEqual(q.fmt_peek("F", 4, n2, [2, 1], 2), "[F] Take all (4): Medpac x2, Blaster Pistol, +2 more")
        real = ["Medpac", "Blaster Pistol", "Adhesive Grenade", "Lightsaber Crystal: Opila"] + ["Z%d" % i for i in range(12)]
        st = [2] + [1] * 15
        self.assertEqual(q.fmt_peek("F", 25, real, st, 16), "[F] Take all (25): Medpac x2, Blaster Pistol, +23 more")

    def test_took_56_boundary(self):
        a, b17, b18 = "A" * 20, "B" * 17, "B" * 18
        self.assertEqual(len(q.fmt_took([a, b17], [99, 9], 2)), 56)          # exactly 56 fits whole
        self.assertEqual(q.fmt_took([a, b18], [99, 9], 2), "Took (2): " + a + " x99, +1 more")   # 57 overflows

    def test_peek_label(self):
        self.assertEqual(q.fmt_peek("F", 3, self.N3, [2, 1, 1], 3), "[F] Take all (3): Medpac x2, Blaster Pistol, Vibroblade")
        self.assertEqual(q.fmt_peek("F", 1, ["Medpac"], None, 1), "[F] Take: Medpac")
        self.assertEqual(q.fmt_peek("F", 6, self.N3, [2, 1, 1], 3), "[F] Take all (6): Medpac x2, Blaster Pistol, +4 more")
        self.assertEqual(q.fmt_peek("F", 0, self.N3, [2, 1, 1], 0), "")
        self.assertEqual(q.fmt_peek("F", 4, self.N3, [2, 1, 1], 0), "")

    def test_peek_56_boundary(self):
        a, b, c = "A" * 20, "B" * 16, "B" * 17
        self.assertEqual(len(q.fmt_peek("F", 2, [a, b], None, 2)), 56)
        self.assertIn(", +1 more", q.fmt_peek("F", 2, [a, c], None, 2))

    def test_long_list_caps(self):
        nm = ["N" * 20] * 16
        t = q.fmt_peek("F", 25, nm, None, 16)
        self.assertLessEqual(len(t), q.PEEK_MAX_CHARS)
        k = int(t.rsplit("+", 1)[1].split()[0])
        self.assertEqual(t.count("N" * 20) + k, 25)

    def test_entry_cut(self):
        # cpp: entryName vectors
        self.assertEqual(q.entry_name("Q" * 20), "Q" * 20)
        self.assertEqual(q.entry_name("Q" * 21), "Q" * 17 + "...")
        self.assertEqual(q.entry_name("Lightsaber Crystal: Opila"), "Lightsaber Crysta...")
        self.assertEqual(q.entry_name("Abcdefghijklmn: --xyzw"), "Abcdefghijklmn...")
        self.assertEqual(len(q.fmt_took(["Z" * 100], None, 1)), 26)
        self.assertEqual(q.fmt_took(["Lightsaber Crystal: Opila"], [3], 1), "Took: Lightsaber Crysta... x3")
        self.assertEqual(len(q.sanitize_name("Z" * 100)), 40)

    def test_took_duration(self):
        self.assertEqual(q.toast_duration_ms(3, 0), 3000)
        self.assertEqual(q.toast_duration_ms(4, 0), 3500)
        self.assertEqual(q.toast_duration_ms(2, 1), 3500)
        self.assertLessEqual(q.toast_duration_ms(16, 4000), q.TOAST_CAP_MS)


class Geometry(unittest.TestCase):
    def test_rects(self):
        # cpp: toastRect / peekRectFixed; both labels are 1200 px
        self.assertEqual(q.PEEK_W, 1200)
        self.assertEqual(q.TOAST_W, 1200)
        self.assertEqual(q.toast_rect(100, 50, 200, 24, w=q.PEEK_W), (100 + 100 - 600, 50 + 24 + 20, 1200, 20))
        self.assertEqual(q.fixed_rect(0, 0, 3840, 1600), (1920 - 600, 992, 1200, 20))

    def test_clamp(self):
        self.assertEqual(q.clamp_label_x((-100, 5, 1200, 20), 0, 3840)[0], 24)
        self.assertEqual(q.clamp_label_x((3000, 5, 1200, 20), 0, 3840)[0], 3840 - 24 - 1200)
        self.assertEqual(q.clamp_label_x((1000, 5, 1200, 20), 0, 3840)[0], 1000)
        self.assertEqual(q.clamp_label_x((5, 5, 1200, 20), 0, 0)[0], 5)
        self.assertEqual(q.clamp_label_x((5, 5, 1200, 20), 0, 1000)[0], (1000 - 1200) // 2)
        self.assertEqual(q.toast_rect(100, 522, 300, 32, screen=(0, 3840))[0], 24)

    def test_backing(self):
        lab = (1000, 600, 1200, 20)
        self.assertEqual(q.back_rect(lab, 200), (990, 596, 1220, 28))
        self.assertEqual(q.back_rect(lab, 28)[2], 28 * 24 + 20)
        x, y, w, h = q.back_rect(lab, 28)
        self.assertEqual(x + w // 2, 1600)


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
        self.assertFalse(q.text_width("A" * 60, adv, 2.75) <= q.TOAST_W)


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
            self.assertEqual(len(rows), 8)
            self.assertEqual(len(written), 16)
            from PIL import Image
            self.assertEqual(Image.open(written[0]).size, (3840, 1600))
            self.assertTrue(all(r[7] for r in rows))   # every sample fits the 1200 px label on one line


if __name__ == "__main__":
    unittest.main()
