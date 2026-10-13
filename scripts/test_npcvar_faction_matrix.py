#!/usr/bin/env python3
"""Tests for scripts/npcvar_faction_matrix.py (stdlib unittest, inline fixtures)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import npcvar_faction_matrix as fm  # noqa: E402


def _game_available() -> bool:
    if not (fm.DEFAULT_GAME_DIR / "chitin.key").is_file():
        return False
    try:
        import pykotor  # noqa: F401
    except ImportError:
        return False
    return True


class ParseStandardFactionsTests(unittest.TestCase):
    def test_parses_live_lines_and_skips_comments(self):
        text = "\n".join([
            "int STANDARD_FACTION_HOSTILE_1      = 1;",
            "// int STANDARD_FACTION_OLD         = 99;",
            "int STANDARD_FACTION_NEUTRAL        = 5;",
            "int OTHER_CONSTANT                  = 7;",
        ])
        self.assertEqual(
            fm.parse_standard_factions(text),
            {"STANDARD_FACTION_HOSTILE_1": 1, "STANDARD_FACTION_NEUTRAL": 5},
        )

    def test_keeps_gap_ids(self):
        text = "int STANDARD_FACTION_ONE_ON_ONE     = 22;\nint STANDARD_FACTION_PARTYPUPPET    = 23;"
        self.assertEqual(
            fm.parse_standard_factions(text),
            {"STANDARD_FACTION_ONE_ON_ONE": 22, "STANDARD_FACTION_PARTYPUPPET": 23},
        )

    def test_empty_text_returns_empty(self):
        self.assertEqual(fm.parse_standard_factions(""), {})


class BuildMatrixTests(unittest.TestCase):
    def test_hostile_flag_at_threshold_boundary(self):
        rows = {1: {1: 100, 2: 49}, 2: {1: 50, 2: 0}}
        records = fm.build_matrix(rows, {1, 2})
        got = [(r["faction_a"], r["faction_b"], r["repute_value"], r["hostile"]) for r in records]
        self.assertEqual(got, [
            (1, 1, 100, False),
            (1, 2, 49, True),
            (2, 1, 50, False),
            (2, 2, 0, True),
        ])
        self.assertEqual(fm.count_hostile(records), 2)

    def test_ids_filter_and_missing_ids_ignored(self):
        rows = {1: {1: 100, 2: 0}, 2: {1: 0, 2: 100}}
        records = fm.build_matrix(rows, {1, 9})
        self.assertEqual([(r["faction_a"], r["faction_b"]) for r in records], [(1, 1)])

    def test_out_of_scale_value_blanks_hostile(self):
        rows = {1: {1: 150, 2: 0}, 2: {1: 0, 2: 100}}
        records = fm.build_matrix(rows, {1, 2})
        self.assertTrue(all(r["hostile"] is None for r in records))
        self.assertEqual(fm.count_hostile(records), 0)

    def test_format_tsv_header_rule_and_cells(self):
        rows = {1: {1: 100, 2: 0}, 2: {1: 150, 2: 50}}
        text = fm.format_tsv(fm.build_matrix(rows, {1, 2}))
        lines = text.splitlines()
        self.assertTrue(lines[0].startswith("# rule: hostile = true when repute_value < 50"))
        self.assertEqual(lines[1], "faction_a\tfaction_b\trepute_value\thostile")
        self.assertEqual(lines[2], "1\t1\t100\t")
        self.assertEqual(lines[3], "1\t2\t0\t")

    def test_format_tsv_uses_true_false(self):
        rows = {1: {1: 100, 2: 0}, 2: {1: 50, 2: 0}}
        text = fm.format_tsv(fm.build_matrix(rows, {1, 2}))
        self.assertIn("1\t1\t100\tfalse", text)
        self.assertIn("1\t2\t0\ttrue", text)


@unittest.skipUnless(_game_available(), "K2 game dir or pykotor not available")
class RealDataTests(unittest.TestCase):
    rows = None

    @classmethod
    def setUpClass(cls):
        cls.rows, cls.reason = fm.load_repute_rows(fm.DEFAULT_GAME_DIR)

    def test_repute_matrix_shape_and_known_cells(self):
        rows = self.rows
        self.assertIsNotNone(rows, self.reason)
        self.assertEqual(sorted(rows), list(range(24)))
        self.assertEqual(rows[1][1], 100)  # Hostile_1 toward Hostile_1
        self.assertEqual(rows[1][2], 0)    # Hostile_1 toward Friendly_1
        self.assertEqual(rows[0][2], 100)  # Player toward Friendly_1
        self.assertEqual(rows[5][5], 100)  # Neutral toward Neutral

    def test_standard_matrix_has_twenty_factions(self):
        text = fm.NWSCRIPT.read_text(encoding="utf-8", errors="replace")
        ids = set(fm.parse_standard_factions(text).values()) | {fm.COMMONER_FACTION_ID}
        self.assertIsNotNone(self.rows, self.reason)
        records = fm.build_matrix(self.rows, ids)
        self.assertEqual(len({r["faction_a"] for r in records}), 20)


if __name__ == "__main__":
    unittest.main()
