#!/usr/bin/env python3
"""test_npcvar_tables.py - unit tests for npcvar_tables.py (stdlib unittest, inline fixtures)."""
import csv
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import npcvar_tables as nt  # noqa: E402


def utc(src, resref, tag="", app_id="", app_label="", faction="5", conv="", on_spawn=""):
    return {"src": src, "resref": resref, "tag": tag, "app_id": app_id, "app_label": app_label,
            "faction": faction, "conv": conv, "on_spawn": on_spawn}


class ModuleKeyTests(unittest.TestCase):
    def test_module_stem_strips_suffixes_and_case(self):
        self.assertEqual(nt.module_key("module:302nar.rim"), "302nar")
        self.assertEqual(nt.module_key("module:302nar_s.rim"), "302nar")
        self.assertEqual(nt.module_key("module:204TEL.MOD"), "204tel")
        self.assertEqual(nt.module_key("module:221TEL_S.RIM"), "221tel")

    def test_non_module_src_has_no_key(self):
        self.assertEqual(nt.module_key("chitin"), "")
        self.assertEqual(nt.module_key("override"), "")

    def test_dedupe_key_collapses_case_and_suffix_variants(self):
        a = nt.dedupe_key(utc("module:302nar.rim", "N_Foo"))
        b = nt.dedupe_key(utc("module:302NAR_S.RIM", "n_foo"))
        self.assertEqual(a, ("302nar", "n_foo"))
        self.assertEqual(a, b)

    def test_dedupe_key_keeps_modules_and_resrefs_apart(self):
        base = nt.dedupe_key(utc("module:302nar.rim", "n_foo"))
        self.assertNotEqual(base, nt.dedupe_key(utc("module:301nar.rim", "n_foo")))
        self.assertNotEqual(base, nt.dedupe_key(utc("module:302nar.rim", "n_bar")))


class GroupTests(unittest.TestCase):
    def test_alien_prefixes(self):
        for resref in ("n_rodian01", "N_RODIAN", "c_jawa", "g_ithorian01", "n_twilekf", "n_tuskenf01"):
            with self.subTest(resref=resref):
                self.assertEqual(nt.classify_group(resref), "alien")

    def test_tsf_prefixes(self):
        for resref in ("n_tsfoff", "n_repsold_bh01", "n_repoff"):
            with self.subTest(resref=resref):
                self.assertEqual(nt.classify_group(resref), "tsf")

    def test_czerka_prefix(self):
        self.assertEqual(nt.classify_group("n_czerkaoff"), "czerka")

    def test_everything_else_is_other(self):
        for resref in ("n_janalorso", "czerka_employ001", "c_bantha", "ond_soldier"):
            with self.subTest(resref=resref):
                self.assertEqual(nt.classify_group(resref), "other")


class LabelPredicateTests(unittest.TestCase):
    def test_loose_labels(self):
        self.assertTrue(nt.is_loose("Commoner_Mal_White_01_A"))
        self.assertTrue(nt.is_loose("Onderon_Commoner_01"))
        self.assertTrue(nt.is_loose("Twilek_Commoner_01"))
        self.assertFalse(nt.is_loose("Dark_Jedi_Low_Male_01"))

    def test_strict_needs_loose_n_prefix_and_not_g(self):
        self.assertTrue(nt.is_strict("n_commm", "Commoner_Mal_White_01_A"))
        self.assertFalse(nt.is_strict("g_thug01", "Commoner_Mal_White_01_A"))
        self.assertFalse(nt.is_strict("c_bantha", "Commoner_Mal_White_01_A"))
        self.assertFalse(nt.is_strict("n_commm", "Dark_Jedi_Low_Male_01"))

    def test_hostile_keywords(self):
        self.assertTrue(nt.is_hostile("Dark_Jedi_Low_Male_01"))
        self.assertTrue(nt.is_hostile("Droid_Assassin_01"))
        self.assertTrue(nt.is_hostile("Turret_Sith"))
        self.assertFalse(nt.is_hostile("Commoner_Mal_White_01_A"))


class PoolCandidateTests(unittest.TestCase):
    def test_filter_join_and_order(self):
        utc_rows = [
            utc("chitin", "n_repsold", app_id="38", app_label="Repsold"),
            utc("chitin", "n_czerkaoff", app_id="18", on_spawn="k_def_spawn01"),
            utc("chitin", "n_rodian", app_id="39", on_spawn="k_def_spawn01"),
            utc("chitin", "n_bith", app_id="12", conv="202bith"),
            utc("chitin", "n_rakata", app_id="5", faction="1"),
            utc("override", "te_gand", app_id="569"),
            utc("module:302nar.rim", "n_commm", app_id="17"),
        ]
        appearances = [
            {"game": "k2", "app_id": "39", "walkdist": "1.4", "rundist": "2.1"},
            {"game": "k2", "app_id": "18", "walkdist": "1.2", "rundist": "2.4"},
            {"game": "k1", "app_id": "38", "walkdist": "9", "rundist": "9"},
            {"game": "k1", "app_id": "39", "walkdist": "9", "rundist": "9"},
        ]
        got = nt.pool_candidates(utc_rows, appearances)
        self.assertEqual(got, [
            {"resref": "n_rodian", "group": "alien", "app_id": "39",
             "walkdist": "1.4", "rundist": "2.1", "on_spawn": "k_def_spawn01"},
            {"resref": "n_repsold", "group": "tsf", "app_id": "38",
             "walkdist": "", "rundist": "", "on_spawn": ""},
            {"resref": "n_czerkaoff", "group": "czerka", "app_id": "18",
             "walkdist": "1.2", "rundist": "2.4", "on_spawn": "k_def_spawn01"},
        ])


class AreaCommonerTests(unittest.TestCase):
    def test_dedupe_and_counts(self):
        utc_rows = [
            utc("module:302nar.rim", "n_a", tag="a", app_label="Commoner_Mal_White_01_A"),
            utc("module:302nar_s.rim", "N_A", tag="a2", app_label="Commoner_Mal_White_01_A"),
            utc("module:302NAR.RIM", "g_b", tag="b", app_label="Dark_Jedi_Low_Male_01"),
            utc("module:302nar.rim", "n_c", tag="c", app_label="Twilek_Commoner_01"),
            utc("module:302nar.rim", "g_d", tag="d", app_label="Commoner_Mal_White_01_B"),
            utc("module:301nar.mod", "n_e", tag="e", app_label="Droid_Assassin_01"),
            utc("module:204TEL.MOD", "n_f", tag="f", app_label="Commoner_Fem_Asian_01_A"),
            utc("chitin", "n_rodian", tag="r", app_label="Commoner_Mal_White_01_A"),
        ]
        self.assertEqual(nt.area_commoners(utc_rows), [
            {"module": "204tel", "all": "1", "loose": "1", "strict": "1", "hostile": "0"},
            {"module": "302nar", "all": "4", "loose": "3", "strict": "2", "hostile": "1"},
        ])


class WriteTsvTests(unittest.TestCase):
    def test_round_trip_header_and_rows(self):
        rows = [{"module": "302nar", "all": "4", "loose": "3", "strict": "2", "hostile": "1"}]
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "out.tsv"
            nt.write_tsv(path, nt.AREA_COLUMNS, rows)
            with open(path, newline="", encoding="utf-8") as fh:
                got = list(csv.DictReader(fh, delimiter="\t"))
        self.assertEqual(got, rows)


if __name__ == "__main__":
    unittest.main()
