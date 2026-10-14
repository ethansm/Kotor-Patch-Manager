#!/usr/bin/env python3
"""Pure-logic tests for k1_import_companions.py (no game, no pykotor, no ingest run)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import k1_import_companions as ic  # noqa: E402


def dump(*textures):
    return {"nodes": [{"name": f"n{i}", "texture": t} for i, t in enumerate(textures)]}


class PlanTests(unittest.TestCase):
    def test_texture_names_distinct_case_insensitive(self):
        self.assertEqual(ic.texture_names(dump("P_CandBB01", "p_candbb01", None, "P_Other")),
                         ["P_CandBB01", "P_Other"])

    def test_plan_names(self):
        p = ic.plan_model("p_candbb", dump("P_CandBB01"))
        self.assertEqual(p["new_resref"], "k1p_candbb")
        self.assertEqual(p["textures"], [("P_CandBB01", "k1p_candbb01")])

    def test_resref_too_long(self):
        with self.assertRaises(ValueError):
            ic.plan_model("p_bastilabb02xx", dump("a"))

    def test_texture_too_long(self):
        with self.assertRaises(ValueError):
            ic.plan_model("p_x", dump("a" * 30))

    def test_bastila_fits(self):
        self.assertEqual(ic.plan_model("p_bastilabb02", dump("P_BastilaBB01"))["new_resref"], "k1p_bastilabb02")

    def test_no_textures(self):
        with self.assertRaises(ValueError):
            ic.plan_model("p_x", {"nodes": []})

    def test_ingest_cmd_dry_run_and_apply(self):
        p = ic.plan_model("p_candh", dump("P_CandH01"))
        dry = ic.ingest_cmd(p, Path("/s"), apply=False)
        self.assertIn("--dry-run", dry)
        self.assertEqual(dry[dry.index("--prefer-source") + 1], "characters_revisited_main")
        self.assertIn("P_CandH01=k1p_candh01", dry)
        self.assertNotIn("--dry-run", ic.ingest_cmd(p, None, apply=True))


if __name__ == "__main__":
    unittest.main()
