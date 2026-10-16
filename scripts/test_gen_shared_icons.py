#!/usr/bin/env python3
"""Tests for scripts/gen_shared_icons.py and patch_manager_mods/_shared_src/GearIconTex.inc.

Run standalone: python3 scripts/test_gen_shared_icons.py (exit 0 on pass).
"""
import re
import struct
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import gen_shared_icons as gsi  # noqa: E402

ARRAY_RE = re.compile(r"static const unsigned char (\w+)\[(\d+)\] = \{(.*?)\};", re.S)
HEX_RE = re.compile(r"0x([0-9a-fA-F]{2})")
TABLE_RE = re.compile(r'\{ "(\w+)", (\w+), sizeof \2 \}')
# identifier -> (resref, width, height)
EXPECT = {
    "kGearTex": ("e3d_gear", 64, 64),
    "kGearHiTex": ("e3d_gear_hi", 64, 64),
    "kFillTex": ("e3d_fill", 8, 8),
    "kTrackTex": ("e3d_trk", 64, 8),
    "kThumbTex": ("e3d_thm", 16, 16),
}
LUO_PAIRS = (("kGearTex", "kLuoGear"), ("kGearHiTex", "kLuoGearHi"), ("kFillTex", "kLuoFill"))
OPAQUE = ("kFillTex", "kTrackTex", "kThumbTex")


def parse_arrays(text):
    """identifier -> (declared size, bytes) for every byte array in a generated .inc."""
    arrays = {}
    for ident, size, body in ARRAY_RE.findall(text):
        arrays[ident] = (int(size), bytes(int(h, 16) for h in HEX_RE.findall(body)))
    return arrays


class GearIconTexTest(unittest.TestCase):
    def setUp(self):
        self.text = gsi.INC.read_text()
        self.arrays = parse_arrays(self.text)
        self.luo = {ident: data for ident, (_, data) in parse_arrays(gsi.LUO_INC.read_text()).items()}

    def test_on_disk_matches_generator(self):
        generated = gsi.render_inc(gsi.build())
        self.assertEqual(generated, self.text, "GearIconTex.inc is stale; run scripts/gen_shared_icons.py")
        self.assertEqual(generated, gsi.render_inc(gsi.build()), "generator output is not deterministic")
        self.assertEqual(gsi.main(["--check"]), 0)

    def test_arrays_present_and_sized(self):
        self.assertEqual(sorted(self.arrays), sorted(EXPECT))
        for ident, (size, data) in self.arrays.items():
            self.assertEqual(len(data), size, ident)

    def test_each_array_is_valid_tga(self):
        luo_footer = self.luo["kLuoFill"][-26:]
        for ident, (resref, w, h) in EXPECT.items():
            size, data = self.arrays[ident]
            self.assertEqual(data[1], 0, f"{ident}: colour map present")
            self.assertEqual(data[2], 2, f"{ident}: image type is not uncompressed truecolour")
            self.assertEqual(struct.unpack("<HH", data[12:16]), (w, h), ident)
            self.assertEqual(data[16], 32, f"{ident}: bpp is not 32")
            self.assertEqual(size, 18 + data[0] + w * h * 4 + 26, f"{ident}: byte size")
            self.assertEqual(data[-26:], luo_footer, f"{ident}: footer differs from luo footer")
            self.assertEqual(data[-18:], b"TRUEVISION-XFILE.\x00", ident)

    def test_gear_fill_bytes_equal_luo(self):
        for ident, luo_ident in LUO_PAIRS:
            self.assertEqual(self.arrays[ident][1], self.luo[luo_ident], f"{ident} != {luo_ident}")

    def test_opaque_alpha(self):
        for ident in OPAQUE:
            _, w, h = EXPECT[ident]
            pixels = self.arrays[ident][1][18:18 + w * h * 4]
            self.assertTrue(all(a == 255 for a in pixels[3::4]), f"{ident}: non-opaque pixel")

    def test_resrefs_and_table(self):
        pairs = TABLE_RE.findall(self.text)
        self.assertEqual(pairs, [(EXPECT[ident][0], ident) for ident in EXPECT])
        for resref, _ in pairs:
            self.assertLessEqual(len(resref), 16, resref)
        self.assertIn("static const unsigned kEditTexCount = 5;", self.text)
        self.assertLessEqual(len(pairs), 16)


if __name__ == "__main__":
    unittest.main()
