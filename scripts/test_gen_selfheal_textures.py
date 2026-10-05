#!/usr/bin/env python3
"""Tests for scripts/gen_selfheal_textures.py: names, TGA headers, cross/ring geometry, deterministic, committed .inc fresh."""
import io
import math
import os
import re
import struct
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
try:
    import PIL  # noqa: F401
except ImportError:
    _venv = HERE.parent / ".venv" / "bin" / "python"
    if _venv.exists() and not os.environ.get("_GST_REEXEC"):
        os.environ["_GST_REEXEC"] = "1"
        os.execv(str(_venv), [str(_venv)] + sys.argv)
    raise
from PIL import Image  # noqa: E402
sys.path.insert(0, str(HERE))
import gen_selfheal_textures as gt  # noqa: E402

EXPECTED = {"csh_cross": (32, 32), **{f"csh_ring{k}": (48, 48) for k in range(8)}, "csh_disc": (48, 48)}


def gen():
    with tempfile.TemporaryDirectory() as td:
        return [(p.stem, p.read_bytes()) for p in gt.make_all(Path(td))]


def img(data):
    return Image.open(io.BytesIO(data)).convert("RGBA")


class GenSelfHealTextures(unittest.TestCase):
    def test_names_and_headers(self):
        blobs = dict(gen())
        self.assertEqual(set(blobs), set(EXPECTED))
        for name, (w, h) in EXPECTED.items():
            d = blobs[name]
            self.assertRegex(name, r"^[a-z0-9_]{1,16}$")
            self.assertEqual(d[2], 2, name)
            self.assertEqual(d[16], 32, name)
            self.assertEqual(struct.unpack("<HH", d[12:16]), (w, h), name)
            self.assertGreaterEqual(len(d), 18 + d[0] + w * h * 4, name)

    def test_cross(self):
        im = img(dict(gen())["csh_cross"])
        r, g, b, a = im.getpixel((16, 16))
        self.assertEqual(a, 255)
        self.assertGreater(g, r)   # mint/teal, not white or grey
        self.assertGreater(b, r)
        for xy in ((0, 0), (31, 0), (0, 31), (31, 31), (5, 5)):
            self.assertLessEqual(im.getpixel(xy)[3], 12, xy)   # at most faint glow

    def test_cross_v3(self):
        im = img(dict(gen())["csh_cross"])
        lum = lambda p: sum(p[:3]) / 3
        self.assertGreater(lum(im.getpixel((16, 7))), lum(im.getpixel((16, 25))) + 20)   # top body row brighter than bottom (vertical gradient)
        # a dark outline pixel near (6,60,50)
        self.assertTrue(any(p[3] > 200 and all(abs(p[i] - t) <= 25 for i, t in enumerate((6, 60, 50)))
                            for p in im.getdata()), "no outline-coloured pixel")
        xs = [x for y in range(32) for x in range(32) if im.getpixel((x, y))[3] >= 250]
        self.assertLessEqual(max(xs) - min(xs) + 1, 24)   # smaller inside the circle than v2 (~30)
        self.assertGreaterEqual(max(xs) - min(xs) + 1, 20)
        ys = [y for y in range(32) for x in range(32) if im.getpixel((x, y))[3] >= 250]
        self.assertLessEqual(max(ys) - min(ys) + 1, 24)

    def test_disc(self):
        im = img(dict(gen())["csh_disc"])
        c = im.getpixel((24, 24))
        self.assertLessEqual(abs(c[3] - 175), 3)
        self.assertLessEqual(max(c[:3]), 3)
        rim = im.getpixel((24 + 20, 24))   # inside the 2 px rim
        for got, want in zip(rim[:3], (10, 96, 80)):
            self.assertLessEqual(abs(got - want), 12, rim)
        self.assertGreater(rim[3], 230)
        # radius from the alpha extent along the centre rows, as a coverage sum
        xs = [x for x in range(48) if im.getpixel((x, 24))[3] > 128]
        radius = (xs[-1] - xs[0] + 1) / 2
        self.assertLessEqual(abs(radius - gt.RING_RADIUS), 1.0)
        self.assertEqual(im.getpixel((0, 0))[3], 0)

    def test_disc_is_last_in_inc(self):
        t = gt.INC.read_text()
        names = re.findall(r'^    \{ "(csh_\w+)", SHTex_', t, re.M)
        self.assertEqual(names[-1], "csh_disc")
        self.assertEqual(names[0], "csh_cross")
        self.assertIn(f"kSelfHealTextureCount = {len(names)}u", t)

    def test_ring_frames_differ(self):
        b = [d for n, d in gen() if n.startswith("csh_ring")]
        self.assertEqual(len(set(b)), 8)

    def test_head_angle(self):
        # head of frame k is at k*45 deg (0 = up, clockwise); compare against the alpha-weighted centroid of the brightest pixels
        for name, d in gen():
            if not name.startswith("csh_ring"):
                continue
            k = int(name[len("csh_ring"):])
            im = img(d)
            sx = sy = 0.0
            for y in range(48):
                for x in range(48):
                    a = im.getpixel((x, y))[3]
                    if a >= 230:
                        sx += (x + 0.5 - 24) * a
                        sy += (y + 0.5 - 24) * a
            ang = math.degrees(math.atan2(sx, -sy)) % 360
            diff = abs((ang - k * 45 + 180) % 360 - 180)
            self.assertLess(diff, 15, (name, ang))

    def test_deterministic(self):
        self.assertEqual(gen(), gen())

    def test_private_names_in_inc(self):
        t = gt.INC.read_text()
        self.assertIn("kSelfHealTextures", t)
        self.assertIn("embtex::Tex", t)
        self.assertNotRegex(t, r"struct \w*EmbeddedTexture\b")

    def test_committed_inc_is_fresh(self):
        self.assertEqual(gt.render_inc(gen()), gt.INC.read_text(),
                         "SelfHealTextures.inc is stale: run python3 scripts/gen_selfheal_textures.py")


if __name__ == "__main__":
    unittest.main()
