#!/usr/bin/env python3
"""Tests for npc_div_merge_labels.decide (pure; stdlib)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import npc_div_merge_labels as M  # noqa: E402


def vote(layout="yes", cat="commoner_clothing", conf=4, outfit="grey shirt"):
    return {"layout_matches_ref": layout, "category": cat, "confidence": conf, "outfit": outfit}


class DecideTests(unittest.TestCase):
    def test_good_slot_allowed_for_its_gender_only(self):
        d = M.decide(12, "M", "n_commmb01", 0.45, [vote()])
        self.assertEqual(d["layout_ok"], {"M": "yes", "F": "no", "FatM": "no", "FatF": "no"})
        self.assertTrue(d["keep"])

    def test_head_texture_rejected(self):
        d = M.decide(20, "M", "n_comm_b_m02", 0.04, [vote("no", "face_head", 5)])
        self.assertFalse(d["keep"])
        self.assertTrue(all(v == "no" for v in d["layout_ok"].values()))

    def test_vanilla_slots_never_rejected(self):
        for slot in (6, 9, 1, 11):
            d = M.decide(slot, "F", "n_commf%02d" % slot, 0.0, [vote("no", "armor", 5)])
            self.assertTrue(d["keep"])
            self.assertEqual(d["layout_ok"]["F"], "yes")

    def test_fat_native_only_in_fat_family(self):
        d = M.decide(35, "F", "n_fatcomf01", 0.44, [vote()])
        self.assertEqual(d["layout_ok"]["F"], "no")
        self.assertEqual(d["layout_ok"]["FatF"], "yes")

    def test_uv_below_threshold_blocks_even_with_yes_votes(self):
        d = M.decide(25, "M", "n_comm_w_m", 0.05, [vote()])
        self.assertFalse(d["keep"])
        self.assertIn("uv/vote conflict", d["review"])

    def test_disagreeing_votes_need_review_and_are_not_accepted(self):
        d = M.decide(14, "M", "x", 0.5, [vote(), vote("no", "armor")])
        self.assertIn("votes disagree", d["review"])
        self.assertEqual(d["layout_ok"]["M"], "no")

    def test_ragged_outfit_held_back(self):
        d = M.decide(30, "F", "n_commfx", 0.27, [vote(outfit="ragged blood-stained clothes", conf=3)])
        self.assertEqual(d["layout_ok"]["F"], "unsure")
        self.assertTrue(d["flags"])

    def test_alt_body_recorded_for_other_meshes(self):
        d = M.decide(28, "M", "n_ondcomm01", -0.09, [vote("no", "robe", 4)])
        self.assertFalse(d["keep"])
        self.assertEqual(d["alt_body"]["mesh"], "N_OndComM1")
        self.assertIn(472, d["alt_body"]["rows"])

    def test_user_vetoed_czerka_look_slot_removed(self):
        # 10-17: vanilla N_CommM10 (dark tunic, orange diagonal stripe, chest badge) reads as a Czerka uniform on civilians.
        d = M.decide(10, "M", "n_commm10", 0.48, [vote(outfit="dark brown shirt, orange stripe, olive trousers", conf=4)])
        self.assertEqual(d["layout_ok"]["M"], "no")
        self.assertTrue(d["keep"])   # vanilla file is never dropped; only the pick is vetoed

    def test_user_vetoed_rag_slots_removed(self):
        # 10-16: the two spot-check trial slots (ragged / patchwork rags) were vetoed after the in-game check.
        d = M.decide(16, "M", "n_commmd01", 0.27, [vote(outfit="ragged clothes", conf=3)])
        self.assertEqual(d["layout_ok"]["M"], "no")
        self.assertFalse(d["keep"])
        d = M.decide(17, "F", "n_commfd01", 0.20, [vote("unsure", "blank_garbled", 2)])
        self.assertEqual(d["layout_ok"]["F"], "no")
        self.assertFalse(d["keep"])

    def test_adjudication_applies(self):
        d = M.decide(34, "M", "n_fatcomm01", 0.41, [vote("unsure", conf=2)])
        self.assertEqual(d["layout_ok"]["FatM"], "yes")


if __name__ == "__main__":
    unittest.main()
