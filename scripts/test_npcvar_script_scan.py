#!/usr/bin/env python3
"""Tests for scripts/npcvar_script_scan.py (stdlib unittest, inline fixtures)."""
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import npcvar_script_scan as ss  # noqa: E402


class CommentAndStringTests(unittest.TestCase):
    def test_line_comment_call_not_counted(self):
        text = "void main() {\n    // ActionAttack(oTarget);\n}\n"
        self.assertNotIn("ActionAttack", ss.scan_script(text)["counts"])

    def test_block_comment_calls_not_counted(self):
        text = "/* EffectHaste(oX, 1);\n   SetPlotFlag(oY, 1); */\nvoid main() {}\n"
        counts = ss.scan_script(text)["counts"]
        self.assertNotIn("EffectHaste", counts)
        self.assertNotIn("SetPlotFlag", counts)

    def test_call_inside_string_literal_not_counted(self):
        text = 'void main() {\n    string s = "ExecuteScript(\\"x\\", OBJECT_SELF)";\n}\n'
        self.assertNotIn("ExecuteScript", ss.scan_script(text)["counts"])

    def test_comment_marker_inside_string_does_not_hide_real_call(self):
        text = 'void main() {\n    string s = "http://x"; PlaySound("snd_a");\n}\n'
        self.assertEqual(ss.scan_script(text)["counts"], {"PlaySound": 1})


class CountTests(unittest.TestCase):
    def test_two_effect_haste_calls_count_two(self):
        text = "void main() {\n    EffectHaste(oA);\n    EffectHaste( oB ) ;\n}\n"
        self.assertEqual(ss.scan_script(text)["counts"], {"EffectHaste": 2})

    def test_word_boundary_rejects_longer_identifier(self):
        text = "void main() { CutsceneActionAttack(oA); PlaySoundEx(oB); }\n"
        self.assertEqual(ss.scan_script(text)["counts"], {})


class ArgumentCaptureTests(unittest.TestCase):
    def test_get_local_number_captured(self):
        text = "void main() { int n = GetLocalNumber(OBJECT_SELF, 29); }\n"
        self.assertEqual(ss.scan_script(text)["local_args"], ["GetLocalNumber(OBJECT_SELF, 29)"])

    def test_get_local_number_whitespace_normalised(self):
        text = "void main() { int n = GetLocalNumber( OBJECT_SELF ,29 ); }\n"
        self.assertEqual(ss.scan_script(text)["local_args"], ["GetLocalNumber(OBJECT_SELF, 29)"])

    def test_get_object_by_tag_literal_captured(self):
        text = 'void main() { object o = GetObjectByTag("npc_x"); }\n'
        self.assertEqual(ss.scan_script(text)["tags"], ["npc_x"])

    def test_get_waypoint_by_tag_literal_captured(self):
        text = 'void main() { object w = GetWaypointByTag( "WP_Y" ); }\n'
        self.assertEqual(ss.scan_script(text)["tags"], ["WP_Y"])

    def test_tag_in_comment_or_string_not_captured(self):
        text = (
            '// GetObjectByTag("hidden_a")\n'
            'void main() { string s = "GetObjectByTag(\\"hidden_b\\")"; }\n'
        )
        self.assertEqual(ss.scan_script(text)["tags"], [])

    def test_non_literal_tag_argument_not_captured(self):
        text = "void main() { object o = GetObjectByTag(sTag); }\n"
        self.assertEqual(ss.scan_script(text)["tags"], [])


class TsvRowTests(unittest.TestCase):
    def test_rows_sorted_by_routine_then_count_desc_then_script(self):
        results = {
            "b_script": {"counts": {"PlaySound": 1, "EffectHaste": 3}, "local_args": [], "tags": []},
            "a_script": {"counts": {"PlaySound": 1}, "local_args": [], "tags": []},
            "c_script": {"counts": {"PlaySound": 4}, "local_args": [], "tags": []},
        }
        got = [(r["routine"], r["script"], r["count"]) for r in ss.tsv_rows(results)]
        self.assertEqual(got, [
            ("EffectHaste", "b_script", 3),
            ("PlaySound", "c_script", 4),
            ("PlaySound", "a_script", 1),
            ("PlaySound", "b_script", 1),
        ])


class RealDataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.results = ss.scan_dir(ss.NSS_DIR) if ss.NSS_DIR.is_dir() else None

    def setUp(self):
        if self.results is None:
            self.skipTest(f"{ss.NSS_DIR} absent")

    def test_force_move_to_location_present(self):
        hits = [name for name, res in self.results.items()
                if "ActionForceMoveToLocation" in res["counts"]]
        self.assertGreaterEqual(len(hits), 1)

    def test_cutscene_attack_or_fake_spell_script_count(self):
        # Measured 10-13: zero live calls. CutsceneAttack appears only in a // comment in
        # k_inc_fakecombat; ActionCastFakeSpellAtObject only as an nwscript prototype (excluded).
        hits = [name for name, res in self.results.items()
                if "CutsceneAttack" in res["counts"] or "ActionCastFakeSpellAtObject" in res["counts"]]
        self.assertEqual(len(hits), 0)

    def test_commented_cutscene_attack_in_fakecombat_not_counted(self):
        res = self.results.get("k_inc_fakecombat")
        if res is None:
            self.skipTest("k_inc_fakecombat.nss absent")
        self.assertNotIn("CutsceneAttack", res["counts"])


if __name__ == "__main__":
    unittest.main()
