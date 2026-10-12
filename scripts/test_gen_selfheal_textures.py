#!/usr/bin/env python3
"""Tests for scripts/gen_selfheal_textures.py: names, TGA headers, cross/disc/toggle/ring geometry, deterministic, committed .inc fresh."""
import functools
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

TOG = ("csh_tog_off", "csh_tog_off_hi", "csh_tog_on", "csh_tog_on_hi")
EXPECTED = {"csh_cross": (32, 32), "csh_disc": (48, 48), "csh_disc_hi": (48, 48), **{n: (128, 32) for n in TOG},
            **{f"csh_ring{k}": (48, 48) for k in range(8)}}
ORDER = ["csh_cross", "csh_disc", "csh_disc_hi", *TOG, *[f"csh_ring{k}" for k in range(8)]]


def _fresh():
    with tempfile.TemporaryDirectory() as td:
        return [(p.stem, p.read_bytes()) for p in gt.make_all(Path(td))]


@functools.lru_cache(maxsize=None)
def _cached():
    return tuple(_fresh())


def gen():
    """Blobs in INC order. Rendered once per test run: the toggle and disc supersampling is the slow part."""
    return _cached()


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

    def test_inc_order_and_count(self):
        t = gt.INC.read_text()
        names = re.findall(r'^    \{ "(csh_\w+)", SHTex_', t, re.M)
        self.assertEqual(names, ORDER)
        self.assertEqual(names[-1], "csh_ring7")
        self.assertIn(f"kSelfHealTextureCount = {len(names)}u", t)

    def test_toggle_corners_transparent(self):
        blobs = dict(gen())
        for name in TOG:
            im = img(blobs[name])
            for xy in ((0, 0), (127, 0), (0, 31), (127, 31)):
                self.assertEqual(im.getpixel(xy)[3], 0, (name, xy))

    def test_toggle_pill_shape(self):
        blobs = dict(gen())
        # full-size pill: the left and right end columns are rim at mid-height (the cap reaches the canvas edge)
        for name in ("csh_tog_off", "csh_tog_on"):
            im = img(blobs[name])
            self.assertGreaterEqual(im.getpixel((0, 16))[3], 250, name)
            self.assertGreaterEqual(im.getpixel((127, 16))[3], 250, name)
        # hi pills are inset, so their end columns are empty
        for name in ("csh_tog_off_hi", "csh_tog_on_hi"):
            im = img(blobs[name])
            self.assertEqual(im.getpixel((0, 16))[3], 0, name)
            self.assertEqual(im.getpixel((127, 16))[3], 0, name)

    def test_toggle_knob_side(self):
        blobs = dict(gen())
        for name in TOG:
            on = name.startswith("csh_tog_on")
            im = img(blobs[name])
            half = lambda x0, x1: sum(im.getpixel((x, y))[3] for x in range(x0, x1) for y in range(32)) / ((x1 - x0) * 32)
            left, right = half(0, 64), half(64, 128)
            if on:
                self.assertGreater(right, left + 5, (name, left, right))   # knob on the right
            else:
                self.assertGreater(left, right + 5, (name, left, right))   # knob on the left
            knob, far = ((112, 16), (16, 16)) if on else ((16, 16), (112, 16))
            colour = (140, 255, 215) if on else (150, 160, 155)
            p = im.getpixel(knob)
            self.assertEqual(p[3], 255, (name, knob))
            self.assertTrue(all(abs(p[i] - t) <= 3 for i, t in enumerate(colour)), (name, p))
            self.assertLess(im.getpixel(far)[3], 255, (name, far))   # no knob at the far cap

    def test_toggle_rims(self):
        blobs = dict(gen())
        expect = {"csh_tog_off": ((64, 0), (60, 90, 85)), "csh_tog_on": ((64, 0), (10, 96, 80)),
                  "csh_tog_off_hi": ((64, 3), (208, 204, 170)), "csh_tog_on_hi": ((64, 3), (208, 204, 170))}
        for name, (xy, colour) in expect.items():
            p = img(blobs[name]).getpixel(xy)
            self.assertEqual(p[3], 255, (name, xy))
            self.assertTrue(all(abs(p[i] - t) <= 3 for i, t in enumerate(colour)), (name, p))

    def test_toggle_hi_halo(self):
        blobs = dict(gen())
        for name in ("csh_tog_off_hi", "csh_tog_on_hi"):
            im = img(blobs[name])
            p = im.getpixel((64, 2))   # just outside the beige rim (pill edge at y=3): halo alpha ~82
            self.assertTrue(40 <= p[3] <= 120, (name, p))
            self.assertTrue(all(abs(p[i] - t) <= 6 for i, t in enumerate((208, 204, 170))), (name, p))
            for k in range(128):   # the halo fades out before the canvas edge, so nothing is clipped
                for xy in ((k, 0), (k, 31)):
                    self.assertEqual(im.getpixel(xy)[3], 0, (name, xy))
            for k in range(32):
                for xy in ((0, k), (127, k)):
                    self.assertEqual(im.getpixel(xy)[3], 0, (name, xy))

    def test_toggle_hi_differs(self):
        blobs = dict(gen())
        beige = lambda p: p[3] > 200 and all(abs(p[i] - t) <= 3 for i, t in enumerate((208, 204, 170)))
        pixels = lambda data: [img(data).getpixel((x, y)) for y in range(32) for x in range(128)]
        for base in ("csh_tog_off", "csh_tog_on"):
            self.assertNotEqual(blobs[base], blobs[base + "_hi"])
            self.assertFalse(any(beige(p) for p in pixels(blobs[base])), base)
            self.assertTrue(any(beige(p) for p in pixels(blobs[base + "_hi"])), base + "_hi")

    def test_toggle_on_inner_glow(self):
        blobs = dict(gen())
        self.assertEqual(img(blobs["csh_tog_off"]).getpixel((64, 4)), (0, 0, 0, 175))   # plain fill, no glow
        r, g, b, a = img(blobs["csh_tog_on"]).getpixel((64, 4))
        self.assertGreater(a, 175)
        self.assertGreater(g, r + 15)
        self.assertGreater(b, r)

    def test_disc_corners_transparent(self):
        blobs = dict(gen())
        for name in ("csh_disc", "csh_disc_hi"):
            im = img(blobs[name])
            for xy in ((0, 0), (47, 0), (0, 47), (47, 47)):
                self.assertEqual(im.getpixel(xy)[3], 0, (name, xy))

    def test_disc_circular(self):
        # Pixel-band checks keep every sub-sample inside one radial zone, so they hold at any angle (0 and 45 deg included).
        blobs = dict(gen())
        norm, hi = img(blobs["csh_disc"]), img(blobs["csh_disc_hi"])
        rc = lambda x, y: math.hypot(x + 0.5 - 24, y + 0.5 - 24)
        for x in range(48):
            for y in range(48):
                r = rc(x, y)
                if 19.8 <= r <= 20.2:   # rim band: fully opaque in both discs
                    self.assertEqual(norm.getpixel((x, y))[3], 255, (x, y))
                    self.assertEqual(hi.getpixel((x, y))[3], 255, (x, y))
                elif r <= 18.2:         # fill: 175 in both discs
                    self.assertEqual(norm.getpixel((x, y))[3], 175, (x, y))
                    self.assertEqual(hi.getpixel((x, y))[3], 175, (x, y))
                elif r >= 21.8:         # outside the normal disc
                    self.assertEqual(norm.getpixel((x, y))[3], 0, (x, y))
                if 21.8 <= r <= 23.2:   # hi halo: linear 200 -> 0 over r 21..24, beige
                    p = hi.getpixel((x, y))
                    self.assertLessEqual(abs(p[3] - 200 * (24 - r) / 3), 4, (x, y, r))
                    self.assertTrue(all(abs(p[i] - t) <= 3 for i, t in enumerate((208, 204, 170))), (x, y, p))

    def test_disc_hi_centre_matches_normal(self):
        blobs = dict(gen())
        norm, hi = img(blobs["csh_disc"]), img(blobs["csh_disc_hi"])
        for xy in ((23, 23), (24, 23), (23, 24), (24, 24)):
            self.assertEqual(hi.getpixel(xy), norm.getpixel(xy), xy)
        self.assertEqual(hi.getpixel((24, 24)), (0, 0, 0, 175))

    def test_disc_rim_colours(self):
        blobs = dict(gen())
        rc = lambda x, y: math.hypot(x + 0.5 - 24, y + 0.5 - 24)
        for name, colour in (("csh_disc", (10, 96, 80)), ("csh_disc_hi", (208, 204, 170))):
            im = img(blobs[name])
            rim = [im.getpixel((x, y)) for x in range(48) for y in range(48) if 19.8 <= rc(x, y) <= 20.2]
            self.assertTrue(rim, name)
            for p in rim:
                self.assertTrue(all(abs(p[i] - t) <= 3 for i, t in enumerate(colour)), (name, p))

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
        self.assertEqual(list(gen()), _fresh())

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
