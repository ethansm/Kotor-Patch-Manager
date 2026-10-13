#!/usr/bin/env python3
"""Tests for scripts/npcvar_roster_pool.py (stdlib unittest, inline fixtures)."""
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import npcvar_roster_pool as rp  # noqa: E402

EXPECTED_ELIGIBLE = {
    # alien (17)
    "n_bith", "n_devaronian01", "n_duros", "n_gand", "n_gran", "n_quarren01", "n_rodian",
    "n_sullustan01", "n_trandobrn", "n_trandogrn", "n_tuskenf01", "n_twilekf", "n_twilekm",
    "n_walrusman", "n_weequay01", "n_wookief", "n_wookiem",
    # tsf (5)
    "n_repoff", "n_repsold", "n_repsold_bh01", "n_tsfcmdr", "n_tsfoff",
    # czerka (1)
    "n_czerkaoff",
}


def pool_row(resref, group="alien", walkdist="1.813"):
    return {"resref": resref, "group": group, "app_id": "1", "walkdist": walkdist, "rundist": "3.66"}


class SpeedClassTests(unittest.TestCase):
    def test_below_1_5_is_slow(self):
        self.assertEqual(rp.speed_class(1.49), "slow")
        self.assertEqual(rp.speed_class(1.2), "slow")

    def test_1_5_is_normal_lower_boundary(self):
        self.assertEqual(rp.speed_class(1.5), "normal")

    def test_2_2_is_normal_upper_boundary(self):
        self.assertEqual(rp.speed_class(2.2), "normal")
        self.assertEqual(rp.speed_class(1.9), "normal")

    def test_above_2_2_is_fast(self):
        self.assertEqual(rp.speed_class(2.21), "fast")
        self.assertEqual(rp.speed_class(2.6), "fast")


class EligibilityTests(unittest.TestCase):
    def test_unarmed_n_alien_is_eligible(self):
        self.assertEqual(rp.eligibility(pool_row("n_rodian"), False), (True, "ok"))

    def test_armed_row_is_ineligible(self):
        self.assertEqual(rp.eligibility(pool_row("n_gand"), True), (False, "weapon slot filled"))

    def test_c_prefix_is_ineligible(self):
        self.assertEqual(rp.eligibility(pool_row("c_brith"), False), (False, "non-n_ prefix"))

    def test_group_other_is_ineligible(self):
        self.assertEqual(rp.eligibility(pool_row("n_commm", group="other"), False), (False, "group other"))

    def test_prefix_checked_before_group(self):
        self.assertEqual(rp.eligibility(pool_row("c_jawa", group="other"), False), (False, "non-n_ prefix"))

    def test_group_checked_before_armed(self):
        self.assertEqual(rp.eligibility(pool_row("n_commm", group="other"), True), (False, "group other"))

    def test_tsf_and_czerka_are_eligible(self):
        self.assertEqual(rp.eligibility(pool_row("n_repoff", group="tsf"), False), (True, "ok"))
        self.assertEqual(rp.eligibility(pool_row("n_czerkaoff", group="czerka"), False), (True, "ok"))


class BuildRowsTests(unittest.TestCase):
    def test_missing_armed_entry_counts_as_unarmed(self):
        out = rp.build_rows([pool_row("n_rodian")], {})
        self.assertEqual(out[0]["eligible"], "true")
        self.assertEqual(out[0]["reason"], "ok")

    def test_armed_flag_taken_from_map(self):
        out = rp.build_rows([pool_row("n_gand")], {"n_gand": True})
        self.assertEqual(out[0]["eligible"], "false")
        self.assertEqual(out[0]["reason"], "weapon slot filled")

    def test_sorted_by_group_order_then_resref(self):
        pool = [
            pool_row("c_jawa", group="other"),
            pool_row("n_czerkaoff", group="czerka"),
            pool_row("n_wookiem", group="alien"),
            pool_row("n_tsfoff", group="tsf"),
            pool_row("n_bith", group="alien"),
        ]
        out = rp.build_rows(pool, {})
        self.assertEqual(
            [r["resref"] for r in out],
            ["n_bith", "n_wookiem", "n_tsfoff", "n_czerkaoff", "c_jawa"],
        )

    def test_speed_class_and_walkdist_text_kept(self):
        out = rp.build_rows([pool_row("n_tuskenf01", walkdist="1.2")], {})
        self.assertEqual(out[0]["speed_class"], "slow")
        self.assertEqual(out[0]["walkdist"], "1.2")


class RealFilesTests(unittest.TestCase):
    @unittest.skipUnless(rp.POOL_IN.exists() and rp.ARMED_IN.exists(), "real pool TSVs absent")
    def test_real_files_yield_23_eligible(self):
        rows = rp.build_rows(rp.load_pool(rp.POOL_IN), rp.load_armed(rp.ARMED_IN))
        eligible = {r["resref"] for r in rows if r["eligible"] == "true"}
        self.assertEqual(len(eligible), 23)
        self.assertEqual(eligible, EXPECTED_ELIGIBLE)


if __name__ == "__main__":
    unittest.main()
