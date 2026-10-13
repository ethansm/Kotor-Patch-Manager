#!/usr/bin/env python3
"""test_npcvar_roster_areas.py - unit tests for npcvar_roster_areas.py (stdlib unittest, inline fixtures)."""
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import npcvar_roster_areas as nr  # noqa: E402


def tsv_rows(*rows):
    """Inline TSV text: each tuple is one row, tab-joined; returns the lines parse_tsv expects."""
    return ["\t".join(r) for r in rows]


COMMONERS_HDR = ("module", "all", "loose", "strict", "hostile")
GEOMETRY_HDR = ("module", "n_waypoints", "n_creatures", "n_wok", "walkable_faces", "notes")


def roster(commoner_rows, geometry_rows):
    return nr.build_roster(nr.parse_tsv(tsv_rows(COMMONERS_HDR, *commoner_rows)),
                           nr.parse_tsv(tsv_rows(GEOMETRY_HDR, *geometry_rows)))


class EligibilityGateTests(unittest.TestCase):
    def test_all_gates_at_their_minimum_pass(self):
        self.assertEqual(nr.area_eligible(5, 40, 300), (True, "ok"))

    def test_strict_boundary(self):
        self.assertEqual(nr.area_eligible(4, 40, 300), (False, "strict<5"))
        self.assertTrue(nr.area_eligible(5, 40, 300)[0])

    def test_waypoints_boundary(self):
        self.assertEqual(nr.area_eligible(5, 39, 300), (False, "waypoints<40"))
        self.assertTrue(nr.area_eligible(5, 40, 300)[0])

    def test_faces_boundary(self):
        self.assertEqual(nr.area_eligible(5, 40, 299), (False, "faces<300"))
        self.assertTrue(nr.area_eligible(5, 40, 300)[0])

    def test_failing_gates_join_with_plus_in_gate_order(self):
        self.assertEqual(nr.area_eligible(4, 39, 299), (False, "strict<5+waypoints<40+faces<300"))
        self.assertEqual(nr.area_eligible(12, 17, 196), (False, "waypoints<40+faces<300"))
        self.assertEqual(nr.area_eligible(4, 40, 299), (False, "strict<5+faces<300"))
        self.assertEqual(nr.area_eligible(0, 21, 559), (False, "strict<5+waypoints<40"))


class SpawnBudgetTests(unittest.TestCase):
    def test_quarter_of_strict(self):
        self.assertEqual(nr.spawn_budget(19), 4)
        self.assertEqual(nr.spawn_budget(11), 2)
        self.assertEqual(nr.spawn_budget(8), 2)
        self.assertEqual(nr.spawn_budget(7), 1)
        self.assertEqual(nr.spawn_budget(5), 1)

    def test_minimum_of_one(self):
        self.assertEqual(nr.spawn_budget(0), 1)
        self.assertEqual(nr.spawn_budget(3), 1)

    def test_cap_default_and_override(self):
        self.assertEqual(nr.SPAWN_CAP, 4)
        self.assertEqual(nr.spawn_budget(40), 4)
        self.assertEqual(nr.spawn_budget(40, cap=3), 3)
        self.assertEqual(nr.spawn_budget(40, cap=10), 10)


class JoinTests(unittest.TestCase):
    def test_join_gates_budget_and_skips_one_sided_modules(self):
        rows = roster(
            commoner_rows=[
                ("333ccc", "10", "5", "9", "0"),  # only in commoners: skipped
                ("222bbb", "10", "5", "4", "0"),  # strict 4: ineligible, budget forced to 0
                ("111aaa", "10", "5", "5", "99"),  # hostile 99 is not a gate
            ],
            geometry_rows=[
                ("222bbb", "50", "1", "1", "400", "note with ; and = and \"quote"),
                ("111aaa", "40", "1", "1", "300", "plain"),
                ("444ddd", "60", "1", "1", "400", "only in geometry: skipped"),
            ],
        )
        self.assertEqual([r["module"] for r in rows], ["111aaa", "222bbb"])
        ok, bad = rows
        self.assertEqual(ok["eligible"], "true")
        self.assertEqual(ok["reason"], "ok")
        self.assertEqual(ok["budget"], 1)
        self.assertEqual(bad["eligible"], "false")
        self.assertEqual(bad["reason"], "strict<5")
        self.assertEqual(bad["budget"], 0)

    def test_output_sorted_by_module(self):
        rows = roster(
            commoner_rows=[("302nar", "51", "23", "19", "2"), ("203tel", "37", "14", "8", "0")],
            geometry_rows=[("302nar", "50", "69", "35", "739", ""), ("203tel", "93", "20", "16", "371", "")],
        )
        self.assertEqual([r["module"] for r in rows], ["203tel", "302nar"])

    def test_region_lookup_applies_to_ineligible_rows_too(self):
        rows = roster(
            commoner_rows=[("207tel", "37", "14", "12", "2"), ("682dan", "15", "15", "0", "0"),
                           ("999xxx", "10", "5", "9", "0")],
            geometry_rows=[("207tel", "17", "32", "2", "196", ""), ("682dan", "21", "2", "21", "559", ""),
                           ("999xxx", "60", "1", "1", "400", "")],
        )
        by_mod = {r["module"]: r for r in rows}
        self.assertEqual(by_mod["207tel"]["region"], "telos")
        self.assertEqual(by_mod["207tel"]["reason"], "waypoints<40+faces<300")
        self.assertEqual(by_mod["682dan"]["region"], "dantooine")
        self.assertEqual(by_mod["682dan"]["reason"], "strict<5+waypoints<40")
        self.assertEqual(by_mod["999xxx"]["region"], "")
        self.assertEqual(by_mod["999xxx"]["budget"], 2)

    def test_notes_with_quote_characters_parse_literally(self):
        parsed = nr.parse_tsv(tsv_rows(GEOMETRY_HDR, ("111aaa", "40", "1", "1", "300", "a \"b")))
        self.assertEqual(parsed[0]["notes"], "a \"b")

    def test_write_roster_header_and_row(self):
        rows = roster(
            commoner_rows=[("301nar", "79", "12", "11", "0")],
            geometry_rows=[("301nar", "135", "41", "12", "578", "")],
        )
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "roster_areas.tsv"
            nr.write_roster(rows, out)
            lines = out.read_text(encoding="utf-8").splitlines()
        self.assertEqual(lines[0], "\t".join(nr.ROSTER_COLUMNS))
        self.assertEqual(lines[1], "301nar\t11\t135\t578\ttrue\tok\t2\tnar_shaddaa")


class RealDataTests(unittest.TestCase):
    def test_real_tables_give_four_eligible_areas_with_expected_budgets(self):
        if not (nr.COMMONERS_TSV.is_file() and nr.GEOMETRY_TSV.is_file()):
            self.skipTest("real area tables absent")
        rows = nr.build_roster(nr.read_tsv(nr.COMMONERS_TSV), nr.read_tsv(nr.GEOMETRY_TSV))
        by_mod = {r["module"]: r for r in rows}
        eligible = {m: r["budget"] for m, r in by_mod.items() if r["eligible"] == "true"}
        self.assertEqual(eligible, {"302nar": 4, "301nar": 2, "203tel": 2, "204tel": 1})
        self.assertEqual(by_mod["207tel"]["reason"], "waypoints<40+faces<300")
        self.assertEqual(by_mod["682dan"]["reason"], "strict<5+waypoints<40")


if __name__ == "__main__":
    unittest.main()
