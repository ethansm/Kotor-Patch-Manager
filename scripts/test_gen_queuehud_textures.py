#!/usr/bin/env python3
"""Tests for scripts/gen_queuehud_textures.py: names, sizes, frame colours/alpha, X glyph, determinism, committed .inc fresh."""
import io
import re
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
from PIL import Image  # noqa: E402
sys.path.insert(0, str(HERE))
import gen_queuehud_textures as gt  # noqa: E402


def gen():
    with tempfile.TemporaryDirectory() as td:
        return [(p.stem, p.read_bytes()) for p in gt.make_all(Path(td))]


def img(data):
    return Image.open(io.BytesIO(data)).convert("RGBA")


def close(a, b, tol):
    return all(abs(x - y) <= tol for x, y in zip(a, b))


class GenQueueHudTextures(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.blobs = gen()
        cls.im = {n: img(d) for n, d in cls.blobs}

    def test_names_order_sizes(self):
        self.assertEqual([n for n, _ in self.blobs], ["qh_frame", "qh_frame_hi", "qh_cancel", "qh_cancel_hi"])
        for im in self.im.values():
            self.assertEqual(im.size, (64, 64))

    def test_border_colours(self):
        # border band is ~3 px wide at 64; sample mid-edge pixel 1 px in
        self.assertTrue(close(self.im["qh_frame"].getpixel((32, 1))[:3], (0, 44, 26), 12))
        self.assertTrue(close(self.im["qh_frame_hi"].getpixel((32, 1))[:3], (208, 204, 170), 12))
        self.assertEqual(self.im["qh_frame"].getpixel((32, 1))[3], 255)
        self.assertEqual(self.im["qh_frame"].getpixel((0, 0))[3], 0)   # rounded corner transparent

    def test_centre_alpha(self):
        for n in ("qh_frame", "qh_frame_hi"):
            self.assertLessEqual(abs(self.im[n].getpixel((32, 32))[3] - 175), 2)

    def test_x_glyph(self):
        for n in ("qh_cancel", "qh_cancel_hi"):
            im = self.im[n]
            self.assertEqual(im.getpixel((32, 32))[3], 255)
            for p in ((32, 32), (24, 24), (40, 40), (24, 40), (40, 24)):
                r, g, b, a = im.getpixel(p)
                self.assertEqual(a, 255, (n, p))
                self.assertTrue(g > r and b > r, (n, p, (r, g, b)))
        # off-diagonal stays a plain frame fill
        self.assertLessEqual(abs(self.im["qh_cancel"].getpixel((32, 14))[3] - 175), 2)
        # frame border unchanged by glyph
        self.assertTrue(close(self.im["qh_cancel_hi"].getpixel((32, 1))[:3], (208, 204, 170), 12))

    def test_deterministic(self):
        self.assertEqual(self.blobs, gen())

    def test_inc_format(self):
        txt = gt.render_inc(self.blobs)
        self.assertEqual(re.findall(r'\{ "(qh_\w+)", QHTex_', txt), list(gt.ALL_NAMES))
        self.assertIn("static const embtex::Tex kQueueHudTextures[] = {", txt)
        self.assertIn("kQueueHudTextureCount = 4u;", txt)
        self.assertNotIn("struct ", txt)

    def test_committed_inc_fresh(self):
        if not gt.INC.exists():
            self.skipTest("inc not generated")
        self.assertEqual(gt.INC.read_text(), gt.render_inc(self.blobs))


if __name__ == "__main__":
    unittest.main()
