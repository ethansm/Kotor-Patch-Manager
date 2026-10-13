#!/usr/bin/env python3
"""test_npcvar_anim_pool.py - unit tests for npcvar_anim_pool.py (stdlib unittest, inline fixtures)."""
import csv
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import npcvar_anim_pool as ap  # noqa: E402

IDLES = ["pause1", "pause2", "pause3", "standstill", "neutral", "good", "evil"]
AMBIENT = ["pausesh", "scanning", "shrug", "rolleyes", "touchheart", "nodyes", "greeting"]

TWO_ROWS = """\
# fixture header
## Per appearance row
row  12 Alien_Bith_01                type=F model=N_Bith         chain=[n_bith > s_male02 > s_male01] n_anims=297
        idles: pause1, pause2, pause3, standstill, neutral, good, evil
        flourish: pausesh(2.0s), scanning(5.1s), salute(1.6s), victory(1.3s), equip(1.0s)
row 185 Commoner_Fem_White_01_A      type=B model=N_CommF        chain=[n_commf > s_female03 > s_female02 > s_female01 > s_male02 > s_male01] n_anims=460
        idles: pause1, pause2, pause3, standstill, neutral, good, evil
        flourish: shrug(1.6s), salute(1.7s), bow(2.5s), itemequip(1.3s)
## Per supermodel chain
chain [n_bith > s_male02 > s_male01]: 297 anims
     stance-like: c10a1, c10a2
"""

DUPLICATE = """\
row  47 Twilek_Female_01_A           type=B model=PFBCM          chain=[pfbcm > s_female03 > s_female02] n_anims=460
        idles: pause1, pause2, pause3, standstill, neutral, good, evil
        flourish: salute(1.7s), bow(2.5s)
row 185 Commoner_Fem_White_01_A      type=B model=PFBCM          chain=[pfbcm > s_female03] n_anims=460
        idles: pause1
        flourish: salute(9.9s), taunt(9.9s), greeting(9.9s)
"""


class ClassifyTests(unittest.TestCase):
    def test_ambient(self):
        for name in AMBIENT:
            with self.subTest(name=name):
                self.assertEqual(ap.classify(name), "ambient")

    def test_military(self):
        self.assertEqual(ap.classify("salute"), "military")

    def test_excluded_and_unlisted(self):
        for name in ["victory", "taunt", "bow", "powerup", "point", "equip", "itemequip", "fakeanim"]:
            with self.subTest(name=name):
                self.assertEqual(ap.classify(name), "excluded")

    def test_idle(self):
        for name in IDLES + ["cpause1", "cpause2"]:
            with self.subTest(name=name):
                self.assertEqual(ap.classify(name), "idle")


class ParseTests(unittest.TestCase):
    def setUp(self):
        self.models = ap.parse_rows(TWO_ROWS)

    def test_two_models_in_file_order(self):
        self.assertEqual([m["model"] for m in self.models], ["N_Bith", "N_CommF"])

    def test_row_and_chain(self):
        bith, commf = self.models
        self.assertEqual(bith["row"], 12)
        self.assertEqual(bith["chain"], ["n_bith", "s_male02", "s_male01"])
        self.assertEqual(commf["row"], 185)
        self.assertEqual(len(commf["chain"]), 6)

    def test_idles(self):
        for m in self.models:
            with self.subTest(model=m["model"]):
                self.assertEqual(m["idles"], IDLES)

    def test_flourish_seconds_as_float(self):
        bith = self.models[0]
        self.assertEqual(bith["flourishes"], {"pausesh": 2.0, "scanning": 5.1, "salute": 1.6,
                                              "victory": 1.3, "equip": 1.0})
        self.assertIsInstance(bith["flourishes"]["salute"], float)

    def test_section_headers_and_chain_lines_ignored(self):
        self.assertEqual(len(self.models), 2)
        self.assertNotIn("c10a1", self.models[0]["flourishes"])

    def test_dedupe_keeps_first_row(self):
        models = ap.parse_rows(DUPLICATE)
        self.assertEqual(len(models), 1)
        self.assertEqual(models[0]["row"], 47)
        self.assertEqual(models[0]["chain"], ["pfbcm", "s_female03", "s_female02"])
        self.assertEqual(models[0]["idles"], IDLES)
        self.assertEqual(models[0]["flourishes"], {"salute": 1.7, "bow": 2.5})


class TsvRowTests(unittest.TestCase):
    def setUp(self):
        self.rows = ap.tsv_rows(ap.parse_rows(TWO_ROWS))

    def test_row_count_is_idles_plus_flourishes(self):
        self.assertEqual(len(self.rows), 2 * len(IDLES) + 5 + 4)

    def test_sorted_by_model_then_anim(self):
        keys = [(r["model"], r["anim_name"]) for r in self.rows]
        self.assertEqual(keys, sorted(keys))

    def test_idle_rows_have_empty_length(self):
        idle_rows = [r for r in self.rows if r["kind"] == "idle"]
        self.assertEqual(len(idle_rows), 2 * len(IDLES))
        for r in idle_rows:
            self.assertEqual(r["length_s"], "")
            self.assertEqual(r["class"], "idle")

    def test_flourish_rows_carry_length_and_class(self):
        by_key = {(r["model"], r["anim_name"]): r for r in self.rows if r["kind"] == "flourish"}
        self.assertEqual(by_key[("N_Bith", "salute")]["length_s"], "1.6")
        self.assertEqual(by_key[("N_Bith", "salute")]["class"], "military")
        self.assertEqual(by_key[("N_Bith", "scanning")]["class"], "ambient")
        self.assertEqual(by_key[("N_Bith", "victory")]["class"], "excluded")
        self.assertEqual(by_key[("N_CommF", "itemequip")]["class"], "excluded")
        self.assertEqual(by_key[("N_CommF", "bow")]["row"], 185)
        self.assertEqual(by_key[("N_CommF", "bow")]["chain_len"], 6)

    def test_ambient_names_in_fixture(self):
        self.assertEqual(ap.ambient_names(ap.parse_rows(TWO_ROWS)), {"pausesh", "scanning", "shrug"})

    def test_write_tsv_header_and_rows(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "anim_pool.tsv"
            ap.write_tsv(out, self.rows)
            text = out.read_text(encoding="utf-8")
            self.assertEqual(text.splitlines()[0],
                             "model\trow\tchain_len\tanim_name\tkind\tlength_s\tclass")
            with out.open(newline="", encoding="utf-8") as fh:
                back = list(csv.DictReader(fh, delimiter="\t"))
            self.assertEqual(len(back), len(self.rows))
            self.assertEqual(back[0]["model"], self.rows[0]["model"])


ROWS_FILE = REPO_ROOT / ".claude" / "research" / "npc_variety" / "u03b_target_rows.txt"


@unittest.skipUnless(ROWS_FILE.exists(), "u03b_target_rows.txt not present")
class RealFileTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.models = ap.parse_rows(ROWS_FILE.read_text(encoding="utf-8"))
        cls.by_name = {m["model"]: m for m in cls.models}

    def test_thirteen_distinct_models(self):
        names = [m["model"] for m in self.models]
        self.assertEqual(len(names), 13)
        self.assertEqual(len(set(names)), 13)

    def test_chain_lengths(self):
        self.assertEqual(len(self.by_name["N_Bith"]["chain"]), 3)
        self.assertEqual(len(self.by_name["N_CzerkaOff"]["chain"]), 5)

    def test_salute_seconds(self):
        self.assertEqual(self.by_name["N_Bith"]["flourishes"]["salute"], 1.6)
        self.assertEqual(self.by_name["N_CommF"]["flourishes"]["salute"], 1.7)

    def test_every_model_has_all_seven_idles(self):
        required = ["pause1", "pause2", "pause3", "standstill", "neutral", "good", "evil"]
        for m in self.models:
            with self.subTest(model=m["model"]):
                for idle in required:
                    self.assertIn(idle, m["idles"])

    def test_every_model_has_every_ambient_name(self):
        self.assertEqual(len(ap.ambient_names(self.models)), 7)
        for m in self.models:
            for name in AMBIENT:
                with self.subTest(model=m["model"], name=name):
                    self.assertIn(name, m["flourishes"])


if __name__ == "__main__":
    unittest.main()
