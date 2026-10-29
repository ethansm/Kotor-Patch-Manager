#!/usr/bin/env python3
"""Tests for isf_config_page_mockup.py: page schema pinned to the brief, rows-visible monotonic in row height, render smoke test (needs the game)."""
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import isf_config_page_mockup as m   # noqa: E402

GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"


class SchemaTest(unittest.TestCase):
    def test_rows_and_choices(self):
        got = [(r["id"], r["label"], r["choices"]) for r in m.PAGE_ROWS]
        self.assertEqual(got, [
            ("sort_order", "Sort order", ["Category", "Base item", "Name", "Vanilla"]),
            ("row_height", "Row height", ["Compact", "Vanilla", "Large"]),
            ("icon_size", "Icon size", ["Small", "Vanilla", "Large"]),
            ("list_font", "List font", ["Small", "Large"]),
            ("dividers", "Category dividers", ["On", "Off"]),
            ("log_level", "Debug logging", ["Off", "On", "Verbose"]),
        ])

    def test_defaults_are_vanilla(self):
        s = m.style_from_values(m.default_values())
        self.assertEqual((s.row, s.icon, s.font, s.dividers), ("Vanilla", "Vanilla", "Large", "On"))
        self.assertEqual(s.row_h, 133)
        self.assertEqual(s.font_name, "dialogfont16x16")
        self.assertEqual(m.FONT_NAME["Small"], "dialogfont10x10")
        for r in m.PAGE_ROWS:
            self.assertTrue(0 <= r["default"] < len(r["choices"]))

    def test_icon_clamped_to_row(self):
        for s in m.COMBOS:
            self.assertLessEqual(s.icon_px, s.row_h - 2 * m.ICON_PAD)
        self.assertEqual(len(m.COMBOS), 18)


class MonotonicTest(unittest.TestCase):
    def test_row_px_order(self):
        self.assertLess(m.ROW_PX["Compact"], m.ROW_PX["Vanilla"])
        self.assertLess(m.ROW_PX["Vanilla"], m.ROW_PX["Large"])

    def test_capacity_monotonic(self):
        for avail in range(200, 1600, 7):
            c, v, l = (m.rows_capacity(avail, m.ROW_PX[k]) for k in ("Compact", "Vanilla", "Large"))
            self.assertTrue(c >= v >= l, (avail, c, v, l))

    def test_rows_from_top_monotonic(self):
        for avail in (500, 600, 701, 800, 900):
            for div in ("On", "Off"):
                n = [m.elements_from_top(m.category_elements(m.ListStyle(k, "Vanilla", "Large", div)), m.ListStyle(k, "Vanilla", "Large", div), avail)[1]
                     for k in ("Compact", "Vanilla", "Large")]
                self.assertTrue(n[0] >= n[1] >= n[2], (avail, div, n))


@unittest.skipUnless(GAME.is_dir(), "game folder needed")
class RenderTest(unittest.TestCase):
    def test_render(self):
        with tempfile.TemporaryDirectory() as td:
            img, info = m.render_page(m.default_values(), "row_height", Path(td) / "p.png")
            self.assertEqual(img.size, (3840, 1600))
            self.assertEqual(info["label_trunc"], [])
            self.assertLessEqual(info["preview_bottom"], info["preview_pane"][1] + info["preview_pane"][3])
            strip, stats, cell = m.render_strip(m.STRIP_STYLES)
            self.assertGreaterEqual(cell[0], 700)
            self.assertEqual(len(stats), 6)
            text, fails = m.fit_table()
            self.assertEqual(fails, 0, text)
            rows = [m.rows_capacity(m.geometry(m.isf._cpm())["geo"]["avail"], m.ROW_PX[k]) for k in ("Compact", "Vanilla", "Large")]
            self.assertTrue(rows[0] >= rows[1] >= rows[2])


if __name__ == "__main__":
    unittest.main()
