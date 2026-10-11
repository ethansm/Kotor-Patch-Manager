#!/usr/bin/env python3
"""Tests for scripts/check_diversity_assets.py. Pure stdlib: every input is an in-memory fixture (a mini
DiversityTables.h text, a mini DiversityRows.h text, and a dict of appearance rows), so no pykotor or game dir is needed."""
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import check_diversity_assets as cda  # noqa: E402


def make_tables(male=(17, 190), female=(14, 185), twi_m=(416,), twi_f=(280,), noise=()):
    """Mini DiversityTables.h in the same layout as the real file. `noise` adds commented-out cases inside the male
    function body; the parser must ignore them."""
    def fn(name, rows, extra=()):
        cases = "\n".join(f"        case {n}:" for n in rows)
        comments = "\n".join(f"        // case {n}: commented out" for n in extra)
        return (f"inline bool {name}(uint16_t app) {{\n    switch (app) {{\n{comments}\n{cases}\n"
                f"            return true;\n        default:\n            return false;\n    }}\n}}\n")
    return ("#pragma once\nnamespace NpcDiversity {\n"
            + fn("IsGenericMaleCommoner", male, noise) + fn("IsGenericFemaleCommoner", female)
            + fn("IsGenericTwilekMale", twi_m) + fn("IsGenericTwilekFemale", twi_f)
            + "} // namespace NpcDiversity\n")


def make_rows_header(consts):
    return "namespace NpcDiversity {\n" + "".join(
        f"constexpr uint16_t {name} = {n};\n" for name, n in consts) + "}\n"


def row(label, modela, texa="N_X"):
    return {"label": label, "modela": modela, "texa": texa}


def good_rows():
    rows = {
        17: row("Commoner_Mal_Asian_01_A", "N_CommM"),
        190: row("Commoner_Mal_White_01_A", "N_CommM"),
        14: row("Commoner_Fem_Asian_01_A", "N_CommF"),
        185: row("Commoner_Fem_White_01_A", "N_CommF"),
        416: row("Twilek_Commoner_M_01_A", "N_CommM"),
        280: row("Twilek_Commoner_F_01_D", "N_CommF"),
    }
    for n in range(802, 812):
        rows[n] = row(f"Custom_{n}", "N_CommM", "N_CommM")
    rows[812] = row("Alien_KelDor_01", "N_KelDor", "N_KelDor")
    return rows


CONSTS = [("APP_COMMONER_FAT_MALE", 802), ("APP_TWILEK_FEMALE_UNDERWEAR", 811), ("APP_ALIEN_KELDOR_01", 812)]


class ParseTablesTests(unittest.TestCase):
    def test_reads_case_rows_per_function_in_source_order(self):
        t = cda.parse_tables(make_tables(male=(17, 190, 345), female=(14,), twi_m=(416, 606), twi_f=(280, 446)))
        self.assertEqual(t, {
            "IsGenericMaleCommoner": [17, 190, 345],
            "IsGenericFemaleCommoner": [14],
            "IsGenericTwilekMale": [416, 606],
            "IsGenericTwilekFemale": [280, 446],
        })

    def test_commented_case_is_ignored(self):
        t = cda.parse_tables(make_tables(male=(17,), noise=(300, 310)))
        self.assertEqual(t["IsGenericMaleCommoner"], [17])

    def test_inline_comment_case_is_ignored(self):
        text = make_tables(male=(17,)).replace("case 17:", "case 17: // case 999:")
        self.assertEqual(cda.parse_tables(text)["IsGenericMaleCommoner"], [17])

    def test_missing_function_is_absent(self):
        text = make_tables().replace("IsGenericTwilekFemale", "SomethingElse")
        self.assertNotIn("IsGenericTwilekFemale", cda.parse_tables(text))

    def test_multiple_case_labels_on_one_line(self):
        text = "inline bool IsGenericMaleCommoner(uint16_t app) {\n    switch (app) {\n        case 1: case 2: case 3:\n"
        self.assertEqual(cda.parse_tables(text)["IsGenericMaleCommoner"], [1, 2, 3])


class ParseAppConstantsTests(unittest.TestCase):
    def test_reads_app_constants_and_skips_head_constants(self):
        text = ("constexpr uint16_t APP_COMMONER_FAT_MALE = 802;\n"
                "// constexpr uint16_t APP_FAKE = 999;\n"
                "constexpr uint8_t HEAD_LLK_AHRNELL = 192;\n")
        self.assertEqual(cda.parse_app_constants(text), {"APP_COMMONER_FAT_MALE": 802})


class RunChecksTests(unittest.TestCase):
    def run_it(self, tables=None, rows_text=None, rows=None, allow=False):
        return cda.run_checks(tables if tables is not None else make_tables(),
                              rows_text if rows_text is not None else make_rows_header(CONSTS[:2]),
                              rows if rows is not None else good_rows(), allow)

    def test_clean_fixture_passes(self):
        code, lines = self.run_it()
        self.assertEqual(code, 0, "\n".join(lines))
        self.assertEqual(lines[-1], "RESULT: PASS")

    def test_label_mismatch_is_warning_not_failure(self):
        rows = good_rows()
        rows[17] = row("Commoner_Dirty_Mal_Asian", "N_CommM")
        code, lines = self.run_it(rows=rows)
        self.assertEqual(code, 0)
        self.assertIn("WARN IsGenericMaleCommoner: label not Commoner_Mal_*: 17=Commoner_Dirty_Mal_Asian", lines)

    def test_twilek_label_mismatch_is_warning(self):
        rows = good_rows()
        rows[280] = row("Twilek_Commoner_M_01_X", "N_CommF")  # female table, male-looking label; modela still matches
        code, lines = self.run_it(rows=rows)
        self.assertEqual(code, 0)
        self.assertIn("WARN IsGenericTwilekFemale: label not Twilek_Commoner_F_*: 280=Twilek_Commoner_M_01_X", lines)

    def test_gender_mismatch_fails(self):
        rows = good_rows()
        rows[185] = row("Commoner_Fem_White_01_A", "N_CommM")
        code, lines = self.run_it(rows=rows)
        self.assertEqual(code, 1)
        self.assertIn("FAIL IsGenericFemaleCommoner: row 185 (Commoner_Fem_White_01_A) modela='N_CommM', expected N_CommF*",
                      lines)

    def test_male_table_with_female_model_fails(self):
        rows = good_rows()
        rows[416] = row("Twilek_Commoner_M_01_A", "N_CommF")
        code, _ = self.run_it(rows=rows)
        self.assertEqual(code, 1)

    def test_missing_table_row_fails(self):
        rows = good_rows()
        del rows[190]
        code, lines = self.run_it(rows=rows)
        self.assertEqual(code, 1)
        self.assertIn("FAIL IsGenericMaleCommoner: row 190 missing from appearance.2da", lines)

    def test_missing_table_function_fails(self):
        code, lines = self.run_it(tables=make_tables().replace("IsGenericTwilekFemale", "Gone"))
        self.assertEqual(code, 1)
        self.assertTrue(any("IsGenericTwilekFemale" in x and "no `case N:` rows" in x for x in lines))

    def test_known_broken_row_fails_by_default(self):
        code, lines = self.run_it(rows_text=make_rows_header(CONSTS))
        self.assertEqual(code, 1)
        self.assertTrue(any(x.startswith("KNOWN-BROKEN (to be dropped by generator): APP_ALIEN_KELDOR_01=812")
                            for x in lines))
        self.assertEqual(lines[-1], "RESULT: FAIL (1 problem(s))")

    def test_known_broken_row_passes_with_flag(self):
        code, lines = self.run_it(rows_text=make_rows_header(CONSTS), allow=True)
        self.assertEqual(code, 0)
        self.assertTrue(any("[accepted: --allow-known-broken]" in x for x in lines))
        self.assertEqual(lines[-1], "RESULT: PASS")

    def test_app_row_missing_fails(self):
        rows = good_rows()
        del rows[802]
        code, lines = self.run_it(rows_text=make_rows_header(CONSTS[:1]), rows=rows)
        self.assertEqual(code, 1)
        self.assertIn("FAIL APP_COMMONER_FAT_MALE: row 802 missing from appearance.2da", lines)

    def test_empty_texa_fails_for_802_to_811(self):
        rows = good_rows()
        rows[802]["texa"] = ""  # load_appearance maps the 2DA null marker to ""
        code, lines = self.run_it(rows_text=make_rows_header([("APP_COMMONER_FAT_MALE", 802)]), rows=rows)
        self.assertEqual(code, 1)
        self.assertIn("FAIL APP_COMMONER_FAT_MALE: row 802 has empty texa", lines)

    def test_empty_modela_fails(self):
        rows = good_rows()
        rows[802]["modela"] = ""
        code, lines = self.run_it(rows_text=make_rows_header([("APP_COMMONER_FAT_MALE", 802)]), rows=rows)
        self.assertEqual(code, 1)
        self.assertIn("FAIL APP_COMMONER_FAT_MALE: row 802 has empty modela", lines)

    def test_empty_texa_on_known_broken_row_is_not_a_second_failure(self):
        rows = good_rows()
        rows[812]["texa"] = ""
        code, lines = self.run_it(rows_text=make_rows_header([("APP_ALIEN_KELDOR_01", 812)]), rows=rows, allow=True)
        self.assertEqual(code, 0)
        self.assertFalse(any(x.startswith("FAIL") for x in lines))

    def test_no_app_constants_fails(self):
        code, lines = self.run_it(rows_text="namespace NpcDiversity {}\n")
        self.assertEqual(code, 1)
        self.assertIn("FAIL no APP_* constants parsed from DiversityRows.h", lines)

    def test_output_fits_summary_budget(self):
        tables = make_tables(male=tuple(range(1000, 1012)), female=tuple(range(2000, 2012)))  # 24 table rows, all missing
        code, lines = self.run_it(tables=tables, rows={}, rows_text=make_rows_header(CONSTS))
        self.assertEqual(code, 1)
        self.assertLessEqual(len(lines), 25)
        self.assertTrue(any(x.startswith("... and ") and x.endswith(" more FAIL lines") for x in lines))


if __name__ == "__main__":
    unittest.main()
