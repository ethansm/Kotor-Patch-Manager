#!/usr/bin/env python3
"""test_npcvar_utc_geom.py - unit tests for npcvar_utc_geom.py (stdlib unittest, inline fakes).

The tested helpers are pure, so this file runs under plain python3 with no pykotor.
"""
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import npcvar_utc_geom as ng  # noqa: E402


class FakeMaterial:
    def __init__(self, walk):
        self._walk = walk

    def walkable(self):
        return self._walk


class FakeFace:
    def __init__(self, walk):
        self.material = FakeMaterial(walk)


class WeaponSlotTests(unittest.TestCase):
    def test_hand_and_claw_slots_count_as_weapon(self):
        for slot in ("RIGHT_HAND", "LEFT_HAND", "CLAW1", "CLAW2", "CLAW3"):
            with self.subTest(slot=slot):
                self.assertTrue(ng.weapon_slot_filled([(slot, "w_blstr_01")]))

    def test_armour_and_other_slots_do_not_count(self):
        for slot in ("HEAD", "ARMOR", "GAUNTLET", "BELT", "IMPLANT", "RIGHT_ARM", "LEFT_ARM", "HIDE"):
            with self.subTest(slot=slot):
                self.assertFalse(ng.weapon_slot_filled([(slot, "n_armor")]))

    def test_blank_weapon_resref_does_not_count(self):
        self.assertFalse(ng.weapon_slot_filled([("RIGHT_HAND", "")]))
        self.assertFalse(ng.weapon_slot_filled([("RIGHT_HAND", "   ")]))

    def test_any_weapon_slot_among_several_is_enough(self):
        self.assertTrue(ng.weapon_slot_filled([("HEAD", "h_01"), ("CLAW2", "c_01")]))

    def test_empty_equipment_is_false(self):
        self.assertFalse(ng.weapon_slot_filled([]))


class SummarizeItemsTests(unittest.TestCase):
    def test_equipment_then_inventory_in_order(self):
        got = ng.summarize_items(
            [("ARMOR", "n_armor"), ("RIGHT_HAND", "w1")],
            ["m_medpac", "w2"],
        )
        self.assertEqual(got, {"n_items": "4", "weapon_slot_filled": "true",
                               "item_resrefs": "n_armor,w1,m_medpac,w2"})

    def test_blank_entries_are_skipped(self):
        got = ng.summarize_items([("LEFT_HAND", "")], ["", "x"])
        self.assertEqual(got, {"n_items": "1", "weapon_slot_filled": "false", "item_resrefs": "x"})

    def test_no_items(self):
        got = ng.summarize_items([], [])
        self.assertEqual(got, {"n_items": "0", "weapon_slot_filled": "false", "item_resrefs": ""})


class PoolRowTests(unittest.TestCase):
    def test_missing_utc_writes_na_row(self):
        self.assertEqual(ng.pool_row("c_gone", None), {
            "resref": "c_gone", "n_items": "NA", "weapon_slot_filled": "NA", "item_resrefs": ""})

    def test_present_utc_row_has_only_pool_columns(self):
        row = ng.pool_row("g_jawa01", ([("CLAW1", "c_claw")], ["m_food"]))
        self.assertEqual(sorted(row), sorted(ng.POOL_COLUMNS))
        self.assertEqual(row, {"resref": "g_jawa01", "n_items": "2",
                               "weapon_slot_filled": "true", "item_resrefs": "c_claw,m_food"})


class CountWalkableTests(unittest.TestCase):
    def test_counts_total_and_walkable(self):
        faces = [FakeFace(True), FakeFace(False), FakeFace(True), FakeFace(False), FakeFace(False)]
        self.assertEqual(ng.count_walkable(faces), (5, 2))

    def test_empty_mesh(self):
        self.assertEqual(ng.count_walkable([]), (0, 0))

    def test_accepts_a_generator(self):
        self.assertEqual(ng.count_walkable(FakeFace(w) for w in (True, True, False)), (3, 2))


class UniqueNamesTests(unittest.TestCase):
    def test_dedupes_case_insensitively_keeping_first_spelling(self):
        self.assertEqual(ng.unique_names(["302NARa", "302nara", "302NARb"]), ["302NARa", "302NARb"])

    def test_drops_blank_names_and_keeps_order(self):
        self.assertEqual(ng.unique_names(["", " 204TELb ", "204TELa", "204telb"]), ["204TELb", "204TELa"])


class AreaRowTests(unittest.TestCase):
    ROOMS = {"n_rooms": 35, "n_wok": 35, "faces": 3429, "walkable": 739, "missing": [], "override": 0}

    def test_mod_counts_with_rim_difference_noted(self):
        row = ng.area_row("302nar", (50, 69), (24, 67), self.ROOMS)
        self.assertEqual(sorted(row), sorted(ng.AREA_COLUMNS))
        self.assertEqual(row["n_waypoints"], "50")
        self.assertEqual(row["n_creatures"], "69")
        self.assertEqual(row["n_wok"], "35")
        self.assertEqual(row["walkable_faces"], "739")
        self.assertIn("GIT from .mod", row["notes"])
        self.assertIn("rim GIT wp=24 cre=67", row["notes"])
        self.assertIn("LYT rooms=35", row["notes"])

    def test_identical_rim_counts_are_not_repeated(self):
        row = ng.area_row("204tel", (48, 17), (48, 17), self.ROOMS)
        self.assertNotIn("rim GIT", row["notes"])

    def test_rim_only_and_no_lyt(self):
        row = ng.area_row("x", None, (3, 4), None)
        self.assertEqual((row["n_waypoints"], row["n_creatures"]), ("3", "4"))
        self.assertEqual((row["n_wok"], row["walkable_faces"]), ("NA", "NA"))
        self.assertIn("GIT from .rim (no .mod)", row["notes"])
        self.assertIn("no LYT", row["notes"])

    def test_no_git_is_na(self):
        row = ng.area_row("x", None, None, None)
        self.assertEqual((row["n_waypoints"], row["n_creatures"]), ("NA", "NA"))
        self.assertIn("no GIT found", row["notes"])

    def test_missing_wok_and_override_are_listed(self):
        rooms = dict(self.ROOMS, n_wok=6, missing=["204TELz"], override=6)
        row = ng.area_row("204tel", (48, 17), None, rooms)
        self.assertIn("missing WOK: 204TELz", row["notes"])
        self.assertIn("WOK from Override=6", row["notes"])


class ColumnContractTests(unittest.TestCase):
    def test_column_lists_match_brief(self):
        self.assertEqual(ng.POOL_COLUMNS, ["resref", "n_items", "weapon_slot_filled", "item_resrefs"])
        self.assertEqual(ng.AREA_COLUMNS,
                         ["module", "n_waypoints", "n_creatures", "n_wok", "walkable_faces", "notes"])
        self.assertEqual(ng.MODULES, ("302nar", "301nar", "682dan", "207tel", "203tel", "204tel"))


if __name__ == "__main__":
    unittest.main()
