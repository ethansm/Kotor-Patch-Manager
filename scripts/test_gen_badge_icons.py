#!/usr/bin/env python3
"""Tests for scripts/gen_badge_icons.py and patch_manager_mods/buff_duration_hud_src/BadgeIconTex.inc.

Run standalone: python3 scripts/test_gen_badge_icons.py (exit 0 on pass).
"""
import math
import re
import struct
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import gen_badge_icons as gbi  # noqa: E402

ARRAY_RE = re.compile(r"static const unsigned char (\w+)\[(\d+)\] = \{(.*?)\};", re.S)
HEX_RE = re.compile(r"0x([0-9a-fA-F]{2})")
TABLE_RE = re.compile(r'\{ "(\w+)", (\w+), sizeof \2 \}')
# kBadgeTex order is the HUD's contract: [0] launch, [1..3] tier 2..4 (BuffDurationHUD.cpp indexes kBadgeTex[t - 1])
ORDER = [("bh_badge_launch", "kBadgeLaunchTex"), ("bh_badge_t2", "kBadgeT2Tex"), ("bh_badge_t3", "kBadgeT3Tex"), ("bh_badge_t4", "kBadgeT4Tex")]
CLEAR_R = 18.0   # the vanilla grenade sphere / device body sits inside this radius around the centre: badges must not cover it


def alpha_at(data, x, y):
    """Alpha of pixel (x, y) in an uncompressed 32-bit TGA (either row order: the clear disc is symmetric)."""
    w = struct.unpack_from("<H", data, 12)[0]
    off = 18 + data[0] + (y * w + x) * 4
    return data[off + 3]


class BadgeIconTexTest(unittest.TestCase):
    def setUp(self):
        self.text = gbi.INC.read_text()
        self.arrays = {i: bytes(int(h, 16) for h in HEX_RE.findall(b)) for i, _, b in ARRAY_RE.findall(self.text)}

    def test_on_disk_matches_generator(self):
        self.assertEqual(gbi.render_inc(gbi.build()), self.text, "BadgeIconTex.inc is stale; run scripts/gen_badge_icons.py")
        self.assertEqual(gbi.main(["--check"]), 0)

    def test_table_order(self):
        self.assertEqual(TABLE_RE.findall(self.text), ORDER)
        self.assertIn(f"kBadgeTexCount = {len(ORDER)};", self.text)

    def test_each_is_64x64_32bit_tga(self):
        for _, ident in ORDER:
            d = self.arrays[ident]
            self.assertEqual((d[1], d[2], d[16]), (0, 2, 32), f"{ident}: not uncompressed 32-bit")
            self.assertEqual(struct.unpack_from("<HH", d, 12), (64, 64), ident)
            self.assertEqual(len(d) - 18 - d[0] >= 64 * 64 * 4, True, ident)

    def test_centre_clear_and_badge_visible(self):
        for _, ident in ORDER:
            d = self.arrays[ident]
            covered = [(x, y) for y in range(64) for x in range(64)
                       if math.hypot(x + 0.5 - 32, y + 0.5 - 32) < CLEAR_R and alpha_at(d, x, y)]
            self.assertEqual(covered, [], f"{ident}: badge covers the icon centre")
            self.assertGreater(sum(1 for y in range(64) for x in range(64) if alpha_at(d, x, y) > 200), 20, f"{ident}: badge is empty")

    def test_tier_bar_count_grows(self):
        solid = [sum(1 for y in range(64) for x in range(64) if alpha_at(self.arrays[i], x, y) > 200) for _, i in ORDER[1:]]
        self.assertLess(solid[0], solid[1]); self.assertLess(solid[1], solid[2])


if __name__ == "__main__":
    unittest.main()
