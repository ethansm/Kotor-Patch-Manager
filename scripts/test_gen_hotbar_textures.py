#!/usr/bin/env python3
"""Tests for scripts/gen_hotbar_textures.py: all textures emitted, TGA headers sane, deterministic, committed .inc is fresh."""
import os
import struct
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
try:
    import PIL  # noqa: F401
except ImportError:  # run_tests.py launches plain py items with the system python; the generator needs Pillow from the repo venv
    _venv = HERE.parent / ".venv" / "bin" / "python"
    if _venv.exists() and not os.environ.get("_GHT_REEXEC"):
        os.environ["_GHT_REEXEC"] = "1"
        os.execv(str(_venv), [str(_venv)] + sys.argv)
    raise
sys.path.insert(0, str(HERE))
import gen_hotbar_textures as gt  # noqa: E402

EXPECTED = {"fph_slot": (128, 128), "fph_slot_hi": (128, 128), "fph_edit": (72, 128), "fph_edit_hi": (72, 128),
            "fph_tgt_lowhp": (32, 32), "fph_mode": (72, 128), "fph_mode_hi": (72, 128)}


def gen():
    with tempfile.TemporaryDirectory() as td:
        return [(p.stem, p.read_bytes()) for p in gt.make_all(Path(td))]


class GenHotbarTextures(unittest.TestCase):
    def test_all_names_and_headers(self):
        blobs = dict(gen())
        self.assertEqual(set(blobs), set(EXPECTED))
        for name, (w, h) in EXPECTED.items():
            d = blobs[name]
            self.assertEqual(d[2], 2, name)       # uncompressed truecolour
            self.assertEqual(d[16], 32, name)     # 32 bpp
            self.assertEqual(struct.unpack("<HH", d[12:16]), (w, h), name)
            self.assertGreaterEqual(len(d), 18 + d[0] + w * h * 4, name)

    def test_badge_alpha(self):
        d = dict(gen())["fph_tgt_lowhp"]
        off = 18 + d[0]
        w = h = 32
        def a(x, y):
            return d[off + (y * w + x) * 4 + 3]
        for x, y in ((0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1)):
            self.assertEqual(a(x, y), 0, (x, y))
        self.assertGreater(a(16, 16), 0)
        self.assertGreater(a(15, 15), 0)

    def test_deterministic(self):
        self.assertEqual(gen(), gen())

    def test_committed_inc_is_fresh(self):
        self.assertEqual(gt.render_inc(gen()), gt.INC.read_text(),
                         "HotbarTextures.inc is stale: run python3 scripts/gen_hotbar_textures.py")


if __name__ == "__main__":
    unittest.main()
