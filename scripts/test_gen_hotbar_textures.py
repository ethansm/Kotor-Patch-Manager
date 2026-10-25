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
            "fph_tgt_lowhp": (92, 48), "fph_tgt_self": (92, 48), "fph_mode": (72, 128), "fph_mode_hi": (72, 128),
            "fph_arrow_prev": (56, 70), "fph_arrow_next": (56, 70), "fph_choice_lowhp": (64, 64), "fph_choice_self": (64, 64)}


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

    @staticmethod
    def _px(name):
        d = dict(gen())[name]
        off = 18 + d[0]
        w, h = struct.unpack("<HH", d[12:16])
        top_down = bool(d[17] & 0x20)
        def px(x, y):   # B, G, R, A at image coordinates (x right, y down)
            row = y if top_down else h - 1 - y
            i = off + (row * w + x) * 4
            return d[i:i + 4]
        return px, w, h

    def test_badge_alpha(self):
        px, w, h = self._px("fph_tgt_lowhp")   # TP2: cross disc at the right end, left part transparent
        for x, y in ((0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1), (10, h // 2), (w - h - 2, h // 2)):
            self.assertEqual(px(x, y)[3], 0, (x, y))
        cx = w - h // 2
        self.assertGreater(px(cx, h // 2)[3], 0)
        b, g, r, a = px(cx, h // 2)
        self.assertTrue(r > 180 and g < 100, (b, g, r, a))   # red cross centre

    def test_self_badge(self):
        px, w, h = self._px("fph_tgt_self")   # TP2: 'Self' pill filling the badge
        for x, y in ((0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1)):
            self.assertEqual(px(x, y)[3], 0, (x, y))
        allpx = [px(x, y) for y in range(h) for x in range(w)]
        mint = [p for p in allpx if p[3] == 255 and p[1] > 180 and p[0] > 140 and p[2] < 160]
        self.assertGreater(len(mint), 60)   # the text
        self.assertEqual(len([p for p in allpx if p[3] == 255 and p[2] > 180 and p[1] < 100]), 0)   # no red
        self.assertGreater(px(3, h // 2)[3], 0); self.assertGreater(px(w - 4, h // 2)[3], 0)       # pill spans the width

    def test_choice_textures(self):
        px, w, h = self._px("fph_choice_lowhp")
        self.assertEqual((w, h), (64, 64)); self.assertEqual(px(0, 0)[3], 0); self.assertGreater(px(32, 32)[3], 0)
        px, w, h = self._px("fph_choice_self")
        self.assertEqual(px(32, 4)[3], 0); self.assertEqual(px(32, 59)[3], 0); self.assertGreater(px(4, 32)[3], 0)

    def test_deterministic(self):
        self.assertEqual(gen(), gen())

    def test_committed_inc_is_fresh(self):
        self.assertEqual(gt.render_inc(gen()), gt.INC.read_text(),
                         "HotbarTextures.inc is stale: run python3 scripts/gen_hotbar_textures.py")

    def test_textures_test_expected_table_matches_generator(self):
        want = gt.render_expected(gen())
        cpp = (gt.REPO / "patch_manager_mods/action_hotbar_src/textures_test.cpp").read_text()
        self.assertIn(want, cpp, "textures_test.cpp kExpected is stale: paste the output of "
                                 "python3 scripts/gen_hotbar_textures.py --emit-expected")

    def test_emit_expected_cli(self):
        import subprocess
        r = subprocess.run([sys.executable, str(HERE / "gen_hotbar_textures.py"), "--emit-expected"],
                           capture_output=True, text=True, check=True)
        self.assertEqual(r.stdout, gt.render_expected(gen()))


if __name__ == "__main__":
    unittest.main()
