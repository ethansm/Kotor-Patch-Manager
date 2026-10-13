#!/usr/bin/env python3
"""Tests for scripts/npcvar_anim_ids.py (stdlib unittest, inline fixtures)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import npcvar_anim_ids as mod  # noqa: E402


NWSCRIPT_FIXTURE = """\
int ANIMATION_LOOPING_PAUSE2        = 1;
//int ANIMATION_LOOPING_LOOK_FAR    = 5;
int ANIMATION_LOOPING_TALK_NORMAL   = 5;
int ANIMATION_ROOM_SCRIPTLOOP01     = 1;
int ANIMATION_FIREFORGET_GREETING   = 106;
//int ANIMATION_FIREFORGET_STEAL    = 106;
int ANIMATION_PLACEABLE_OPEN        = 202;//AWD-OEI 07/06/2004
int OBJECT_SELF = 0;
"""


class ParseNwscriptConstantsTests(unittest.TestCase):
    def test_live_lines_parsed_and_commented_lines_ignored(self):
        got = mod.parse_nwscript_constants(NWSCRIPT_FIXTURE)
        self.assertEqual(got[1], "ANIMATION_LOOPING_PAUSE2")
        self.assertEqual(got[5], "ANIMATION_LOOPING_TALK_NORMAL")
        self.assertEqual(got[106], "ANIMATION_FIREFORGET_GREETING")
        self.assertEqual(got[202], "ANIMATION_PLACEABLE_OPEN")
        self.assertNotIn("ANIMATION_LOOPING_LOOK_FAR", got.values())
        self.assertNotIn("ANIMATION_FIREFORGET_STEAL", got.values())

    def test_first_live_line_wins_on_shared_value(self):
        got = mod.parse_nwscript_constants(NWSCRIPT_FIXTURE)
        self.assertEqual(got[1], "ANIMATION_LOOPING_PAUSE2")

    def test_non_animation_ints_ignored(self):
        self.assertNotIn(0, mod.parse_nwscript_constants(NWSCRIPT_FIXTURE))

    def test_empty_text(self):
        self.assertEqual(mod.parse_nwscript_constants(""), {})


class JoinTests(unittest.TestCase):
    def test_join_sorts_and_maps_constants_and_flags(self):
        constants = {1: "ANIMATION_LOOPING_PAUSE2", 0: "ANIMATION_LOOPING_PAUSE"}
        rows = {
            7: {"name": "pause2", "fireforget": False, "looping": True, "stationary": True},
            0: {"name": "walk", "fireforget": False, "looping": False, "stationary": False},
            1: {"name": "walkinj", "fireforget": None, "looping": False, "stationary": False},
        }
        got = mod.join(constants, rows)
        self.assertEqual([r["id"] for r in got], [0, 1, 7])
        self.assertEqual(got[0]["nwscript_constant"], "ANIMATION_LOOPING_PAUSE")
        self.assertEqual(got[2]["nwscript_constant"], "")
        self.assertEqual(got[2]["looping"], True)
        self.assertIsNone(got[1]["fireforget"])
        self.assertEqual(got[0]["name"], "walk")

    def test_join_sets_claim_column(self):
        got = mod.join({}, {1: {"name": "pause2", "fireforget": False, "looping": True, "stationary": True},
                            2: {"name": "run", "fireforget": False, "looping": False, "stationary": False}})
        self.assertIs(got[0]["v1_claim_ok"], True)
        self.assertIsNone(got[1]["v1_claim_ok"])

    def test_count_mismatches(self):
        records = [{"v1_claim_ok": True}, {"v1_claim_ok": False}, {"v1_claim_ok": None}, {"v1_claim_ok": False}]
        self.assertEqual(mod.count_mismatches(records), 2)


class ClaimOkTests(unittest.TestCase):
    def test_exact_claims(self):
        self.assertIs(mod.claim_ok(1, "pause2"), True)
        self.assertIs(mod.claim_ok(24, "pause3"), True)
        self.assertIs(mod.claim_ok(104, "salute"), True)
        self.assertIs(mod.claim_ok(105, "bow"), True)
        self.assertIs(mod.claim_ok(106, "greeting"), True)
        self.assertIs(mod.claim_ok(107, "taunt"), True)

    def test_exact_claims_reject_other_names(self):
        self.assertIs(mod.claim_ok(1, "walkinj"), False)
        self.assertIs(mod.claim_ok(24, "meditate"), False)
        self.assertIs(mod.claim_ok(104, "c2d1"), False)
        self.assertIs(mod.claim_ok(107, "taunter"), False)

    def test_contains_claims(self):
        self.assertIs(mod.claim_ok(100, "headturnl"), True)
        self.assertIs(mod.claim_ok(101, "turn_right"), True)
        self.assertIs(mod.claim_ok(102, "pausesh"), True)
        self.assertIs(mod.claim_ok(102, "scratch_head"), True)
        self.assertIs(mod.claim_ok(103, "pausebrd"), True)
        self.assertIs(mod.claim_ok(103, "bored"), True)
        self.assertIs(mod.claim_ok(102, "c2p4"), False)
        self.assertIs(mod.claim_ok(100, "c2p2"), False)

    def test_startswith_claim(self):
        self.assertIs(mod.claim_ok(108, "victory1"), True)
        self.assertIs(mod.claim_ok(108, "c2d5"), False)
        self.assertIs(mod.claim_ok(108, "myvictory"), False)

    def test_case_insensitive_and_blank_name(self):
        self.assertIs(mod.claim_ok(1, "Pause2"), True)
        self.assertIs(mod.claim_ok(1, ""), False)
        self.assertIs(mod.claim_ok(1, None), False)

    def test_unclaimed_ids_are_none(self):
        self.assertIsNone(mod.claim_ok(0, "walk"))
        self.assertIsNone(mod.claim_ok(999, "pause2"))


class FormatTsvTests(unittest.TestCase):
    def test_header_and_boolean_cells(self):
        records = [
            {"id": 1, "name": "walkinj", "nwscript_constant": "ANIMATION_LOOPING_PAUSE2",
             "fireforget": None, "looping": False, "stationary": True, "v1_claim_ok": False},
        ]
        text = mod.format_tsv(records)
        lines = text.splitlines()
        self.assertEqual(lines[0], "id\tname\tnwscript_constant\tfireforget\tlooping\tstationary\tv1_claim_ok")
        self.assertEqual(lines[1], "1\twalkinj\tANIMATION_LOOPING_PAUSE2\t\tfalse\ttrue\tfalse")

    def test_blank_claim_cell(self):
        text = mod.format_tsv([{"id": 2, "name": "run", "nwscript_constant": "", "fireforget": False,
                                "looping": False, "stationary": False, "v1_claim_ok": None}])
        self.assertTrue(text.splitlines()[1].endswith("\t"))


class FlagAndCellTests(unittest.TestCase):
    def test_parse_flag(self):
        self.assertIs(mod.parse_flag("1"), True)
        self.assertIs(mod.parse_flag("0"), False)
        self.assertIsNone(mod.parse_flag(""))
        self.assertIsNone(mod.parse_flag(None))
        self.assertIsNone(mod.parse_flag("2"))

    def test_clean_null_cell(self):
        self.assertEqual(mod._clean("****"), "")
        self.assertEqual(mod._clean(" pause2 "), "pause2")


def _load_real_rows():
    """Real animations.2da rows, or a skip reason when the game dir or pykotor is absent."""
    rows, reason = mod.load_animation_rows(mod.DEFAULT_GAME_DIR)
    return rows, reason


class RealDataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rows, cls.reason = _load_real_rows()

    def setUp(self):
        if self.rows is None:
            self.skipTest(f"real animations.2da unavailable: {self.reason}")

    def test_all_selected_ids_present(self):
        self.assertTrue(set(mod.SELECTED_IDS) <= set(self.rows))

    def test_known_real_names(self):
        self.assertEqual(self.rows[0]["name"], "walk")
        self.assertEqual(self.rows[7]["name"], "pause2")
        self.assertIs(self.rows[7]["looping"], True)
        self.assertIs(self.rows[7]["fireforget"], False)

    def test_nwscript_constant_for_pause2(self):
        if not mod.NWSCRIPT.is_file():
            self.skipTest("nwscript.nss not present")
        constants = mod.parse_nwscript_constants(mod.NWSCRIPT.read_text(encoding="utf-8", errors="replace"))
        self.assertEqual(constants[1], "ANIMATION_LOOPING_PAUSE2")


if __name__ == "__main__":
    unittest.main()
