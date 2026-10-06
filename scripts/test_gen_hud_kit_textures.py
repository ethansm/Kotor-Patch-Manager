#!/usr/bin/env python3
"""Tests for scripts/gen_hud_kit_textures.py: names, sizes, frame colours/alpha, X glyph, disc, determinism, committed .inc fresh."""
import io
import os
import re
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
try:
    import PIL  # noqa: F401
except ImportError:
    _venv = HERE.parent / ".venv" / "bin" / "python"
    if _venv.exists() and not os.environ.get("_GHK_REEXEC"):
        os.environ["_GHK_REEXEC"] = "1"
        os.execv(str(_venv), [str(_venv)] + sys.argv)
    raise
from PIL import Image  # noqa: E402
sys.path.insert(0, str(HERE))
import gen_hud_kit_textures as gt  # noqa: E402


def gen():
    with tempfile.TemporaryDirectory() as td:
        return [(p.stem, p.read_bytes()) for p in gt.make_all(Path(td))]


def img(data):
    return Image.open(io.BytesIO(data)).convert("RGBA")


def close(a, b, tol):
    return all(abs(x - y) <= tol for x, y in zip(a, b))


class GenHudKitTextures(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.blobs = gen()
        cls.im = {n: img(d) for n, d in cls.blobs}

    def test_names_order_sizes(self):
        self.assertEqual([n for n, _ in self.blobs], ["kit_frame", "kit_frame_hi", "kit_cancel", "kit_cancel_hi", "kit_disc"])
        for n, im in self.im.items():
            self.assertEqual(im.size, (48, 48) if n == "kit_disc" else (64, 64))

    def test_border_colours(self):
        # border band is ~3 px wide at 64; sample mid-edge pixel 1 px in
        self.assertTrue(close(self.im["kit_frame"].getpixel((32, 1))[:3], (0, 44, 26), 12))
        self.assertTrue(close(self.im["kit_frame_hi"].getpixel((32, 1))[:3], (208, 204, 170), 12))
        self.assertEqual(self.im["kit_frame"].getpixel((32, 1))[3], 255)
        self.assertEqual(self.im["kit_frame"].getpixel((0, 0))[3], 0)   # rounded corner transparent

    def test_centre_alpha(self):
        for n in ("kit_frame", "kit_frame_hi"):
            self.assertLessEqual(abs(self.im[n].getpixel((32, 32))[3] - 175), 2)

    def test_x_glyph(self):
        for n in ("kit_cancel", "kit_cancel_hi"):
            im = self.im[n]
            self.assertEqual(im.getpixel((32, 32))[3], 255)
            for p in ((32, 32), (24, 24), (40, 40), (24, 40), (40, 24)):
                r, g, b, a = im.getpixel(p)
                self.assertEqual(a, 255, (n, p))
                self.assertTrue(g > r and b > r, (n, p, (r, g, b)))
        # off-diagonal stays a plain frame fill
        self.assertLessEqual(abs(self.im["kit_cancel"].getpixel((32, 14))[3] - 175), 2)
        # frame border unchanged by glyph
        self.assertTrue(close(self.im["kit_cancel_hi"].getpixel((32, 1))[:3], (208, 204, 170), 12))

    def test_disc(self):
        im = self.im["kit_disc"]
        c = im.getpixel((24, 24))
        self.assertLessEqual(abs(c[3] - 175), 3)
        self.assertLessEqual(max(c[:3]), 3)
        rim = im.getpixel((24 + 20, 24))   # inside the 2 px rim
        for got, want in zip(rim[:3], (10, 96, 80)):
            self.assertLessEqual(abs(got - want), 12, rim)
        self.assertGreater(rim[3], 230)
        xs = [x for x in range(48) if im.getpixel((x, 24))[3] > 128]
        self.assertLessEqual(abs((xs[-1] - xs[0] + 1) / 2 - gt.DISC_RADIUS), 1.0)
        self.assertEqual(im.getpixel((0, 0))[3], 0)

    def test_deterministic(self):
        self.assertEqual(self.blobs, gen())

    def test_inc_format(self):
        txt = gt.render_inc(self.blobs)
        self.assertEqual(re.findall(r'\{ "(kit_\w+)", HKTex_', txt), list(gt.ALL_NAMES))
        self.assertIn("static const embtex::Tex kHudKitTextures[] = {", txt)
        self.assertIn("kHudKitTextureCount = 5u;", txt)
        self.assertNotIn("struct ", txt)

    def test_committed_inc_fresh(self):
        if not gt.INC.exists():
            self.skipTest("inc not generated")
        self.assertEqual(gt.INC.read_text(), gt.render_inc(self.blobs))


if __name__ == "__main__":
    unittest.main()
