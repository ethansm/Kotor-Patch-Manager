#!/usr/bin/env python3
"""Tests for npc_div_texture_sheets (pure maths + sheet geometry; no game files needed)."""
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

try:
    import numpy as np
    from PIL import Image
    import npc_div_texture_sheets as T
except ImportError as e:          # system python3 has neither numpy nor Pillow: report SKIP-by-pass, run via .venv
    np = None
    IMPORT_ERR = e


def _pattern(kind, n=128):
    a = np.zeros((n, n, 3), dtype=np.uint8) + 120
    if kind == "panels":          # a body-like layout: dark seams on a grid
        a[:, ::32] = 20
        a[::32, :] = 20
    elif kind == "blob":          # a head-like layout: one centred disc
        yy, xx = np.mgrid[:n, :n]
        a[(yy - n // 2) ** 2 + (xx - n // 2) ** 2 < (n // 3) ** 2] = (200, 150, 120)
    return Image.fromarray(a)


@unittest.skipIf(np is None, "numpy/Pillow missing: run with .venv/bin/python")
class TextureSheetTests(unittest.TestCase):
    def test_ncorr_identity_and_flat(self):
        m = T.layout_mask(_pattern("panels"))
        self.assertAlmostEqual(T.ncorr(m, m), 1.0, places=6)
        self.assertEqual(T.ncorr(np.ones((4, 4)), m[:4, :4]), 0.0)    # flat input: no division by zero

    def test_layout_separates_panels_from_blob(self):
        a, b, c = (T.layout_mask(_pattern(k)) for k in ("panels", "panels", "blob"))
        self.assertGreater(T.ncorr(a, b), T.ncorr(a, c))

    def test_auc(self):
        self.assertEqual(T.auc([0.9, 0.8], [0.1, 0.2]), 1.0)
        self.assertEqual(T.auc([0.1], [0.9]), 0.0)
        self.assertEqual(T.auc([0.5], [0.5]), 0.5)
        self.assertNotEqual(T.auc([], [1]), T.auc([], [1]))           # NaN

    def test_skin_fraction(self):
        skin = Image.new("RGB", (8, 8), (200, 140, 110))
        blue = Image.new("RGB", (8, 8), (10, 20, 220))
        self.assertEqual(T.skin_fraction(skin), 1.0)
        self.assertEqual(T.skin_fraction(blue), 0.0)

    def test_shuffle_is_seeded_and_complete(self):
        items = list(range(30))
        self.assertEqual(T.shuffled(items, 7), T.shuffled(items, 7))
        self.assertNotEqual(T.shuffled(items, 7), T.shuffled(items, 8))
        self.assertEqual(sorted(T.shuffled(items, 7)), items)

    def test_chunk_and_sheet_geometry(self):
        groups = T.chunk(list(range(30)))
        self.assertEqual([len(g) for g in groups], [12, 12, 6])
        tiles = [_pattern("panels") for _ in range(12)]
        sheet = T.contact_sheet(tiles, ["T%02d" % i for i in range(12)])
        self.assertEqual(sheet.size, (T.SHEET_COLS * T.TILE_PX, T.SHEET_ROWS * T.TILE_PX))

    def test_pinned_lists(self):
        self.assertEqual(len(T.DECOYS), 30)
        self.assertEqual(len({n for n, _, _ in T.DECOYS}), 30)
        cats = {c for _, c, _ in T.DECOYS}
        self.assertTrue({"armor", "face_head", "creature_alien_droid"} <= cats)
        self.assertEqual(len(T.VANILLA_M) + len(T.VANILLA_F), 22)
        self.assertFalse(set(T.VANILLA_M) & {n for n, _, _ in T.DECOYS})


if __name__ == "__main__":
    unittest.main()
