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

EXPECTED = {"csh_cross": (32, 32), **{f"csh_ring{k}": (48, 48) for k in range(8)}}


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
        self.assertEqual(im.getpixel((16, 16))[3], 255)
        self.assertEqual(im.getpixel((16, 16))[:3], (255, 255, 255))
        for xy in ((0, 0), (31, 0), (0, 31), (31, 31), (5, 5)):
            self.assertEqual(im.getpixel(xy)[3], 0, xy)
        self.assertGreater(im.getpixel((16, 2))[3], 0)   # arm reaches near the edge

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
        self.assertNotRegex(t, r"struct EmbeddedTexture\b")

    def test_committed_inc_is_fresh(self):
        self.assertEqual(gt.render_inc(gen()), gt.INC.read_text(),
                         "SelfHealTextures.inc is stale: run python3 scripts/gen_selfheal_textures.py")


if __name__ == "__main__":
    unittest.main()
