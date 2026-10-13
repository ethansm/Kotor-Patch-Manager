#!/usr/bin/env python3
"""Tests for scripts/check_diversity_assets.py. Pure stdlib: every input is an in-memory fixture (a mini
DiversityTables.h text, a mini DiversityRows.h text, and a dict of appearance rows), so no pykotor or game dir is needed."""
import sys
from unittest import mock
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
    for n in range(802, 818):
        rows[n] = row(f"Custom_{n}", "N_CommM", "N_CommM")
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

    def test_table_bound_constants_are_not_rows(self):
        text = ("constexpr uint16_t APP_FIRST_APPENDED = 802;\n"
                "constexpr uint16_t APP_COUNT = 812;\n"
                "constexpr uint16_t APP_TWILEK_FEMALE_UNDERWEAR = 811;\n")
        self.assertEqual(cda.parse_app_constants(text), {"APP_TWILEK_FEMALE_UNDERWEAR": 811})


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

    @mock.patch.object(cda, "KNOWN_BROKEN_APP_ROWS", frozenset({812}))
    def test_known_broken_row_fails_by_default(self):
        code, lines = self.run_it(rows_text=make_rows_header(CONSTS))
        self.assertEqual(code, 1)
        self.assertTrue(any(x.startswith("KNOWN-BROKEN (to be dropped by generator): APP_ALIEN_KELDOR_01=812")
                            for x in lines))
        self.assertEqual(lines[-1], "RESULT: FAIL (1 problem(s))")

    @mock.patch.object(cda, "KNOWN_BROKEN_APP_ROWS", frozenset({812}))
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

    @mock.patch.object(cda, "KNOWN_BROKEN_APP_ROWS", frozenset({812}))
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


def make_slots_header(male=(1, 2, 6, 9), female=(1, 2, 6, 9), fat_m=(34,), fat_f=(35,), counts=None):
    """Mini DiversitySlots.h in the generated layout. `counts` overrides the <name>Count value per array name."""
    counts = counts or {}

    def arr(name, slots):
        body = ", ".join(str(s) for s in slots)
        return (f"constexpr uint8_t {name}[] = {{{body}}};\n"
                f"constexpr size_t {name}Count = {counts.get(name, len(slots))};\n")
    return ("// DiversitySlots.h -- fixture\n#pragma once\nnamespace NpcDiversity {\n"
            + arr("kMaleSlots", male) + arr("kFemaleSlots", female)
            + arr("kFatMaleSlots", fat_m) + arr("kFatFemaleSlots", fat_f)
            + "} // namespace NpcDiversity\n")


# Small slot set keeps fixtures tiny. Its required set is the vanilla 1-11 plus the 17 (female array) and 34/35 (fat arrays).
SLOTS_HEADER = make_slots_header(female=(1, 2, 6, 9, 17))
SLOTS = tuple(range(1, 12)) + (17, 34, 35)
HEAD_SPEC = {
    192: ("llk_Ahrnell", "llk_Ahrnell"), 193: ("llk_Harra", "llk_Harra"), 194: ("llk_KehVehn", "llk_KehVehn"),
    195: ("llk_LupoShar", "llk_LupoShar"), 196: ("llk_DahnisH", "llk_DahnisH"),
    198: ("P_TwiHNGreen", "P_TwiNGreen"), 200: ("P_TwiHNGreen", "P_TwiNBlue"), 204: ("P_TwiHTRed", "P_TwiToneRed"),
}


def resource_rows():
    """Appearance rows 802-811 with the columns the resource check reads (same model/texture names as the real rows)."""
    spec = {n: ("N_CommM", "N_CommM") for n in range(802, 818)}
    spec[802] = ("N_CommM_Fat", "N_CommM")
    spec[803] = ("N_CommF_Fat", "N_CommF")
    spec[804] = ("N_CzerkaOff", "N_CzerkaOff")
    spec[809] = ("N_TwiDance_Boot", "N_CommF")
    spec[810] = ("N_TwiDance", "N_CommF")
    spec[811] = ("N_TwiUnd", "N_CommF")
    return {n: {"label": f"L{n}", "modela": m, "modelb": m, "texa": t, "texb": t} for n, (m, t) in spec.items()}


def resource_heads():
    return {n: {"head": h, "alttexture": a} for n, (h, a) in HEAD_SPEC.items()}


def full_set(exclude=()):
    """Lowercase file names that satisfy every need of the fixture rows, minus `exclude` (case-insensitive)."""
    out = set()
    for need, _ in cda.build_needs(resource_rows(), resource_heads(), SLOTS):
        out.update(f.lower() for f in need.files)
    return frozenset(out - {e.lower() for e in exclude})


class ParseDiversitySlotsTests(unittest.TestCase):
    def test_required_is_union_of_four_arrays_plus_vanilla(self):
        slots, problems = cda.parse_diversity_slots(make_slots_header(male=(1, 6, 9, 16), female=(1, 6, 9, 17)))
        self.assertEqual(problems, [])
        self.assertEqual(slots, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 16, 17, 34, 35])

    def test_shared_fixture_matches_slots_constant(self):
        self.assertEqual(cda.parse_diversity_slots(SLOTS_HEADER), (list(SLOTS), []))

    def test_commented_declaration_is_ignored(self):
        slots, problems = cda.parse_diversity_slots("// constexpr uint8_t kMaleSlots[] = {99};\n" + make_slots_header())
        self.assertEqual(problems, [])
        self.assertNotIn(99, slots)

    def test_slot_zero_fails_and_is_not_required(self):
        slots, problems = cda.parse_diversity_slots(make_slots_header(male=(0, 1, 6, 9)))
        self.assertEqual(problems, ["DiversitySlots.h kMaleSlots: slot(s) outside 1..255: [0]"])
        self.assertNotIn(0, slots)

    def test_slot_300_fails(self):
        slots, problems = cda.parse_diversity_slots(make_slots_header(female=(1, 6, 9, 300)))
        self.assertEqual(problems, ["DiversitySlots.h kFemaleSlots: slot(s) outside 1..255: [300]"])
        self.assertNotIn(300, slots)

    def test_negative_slot_is_reported_not_dropped(self):
        _, problems = cda.parse_diversity_slots(make_slots_header(fat_m=(-1, 34)))
        self.assertEqual(problems, ["DiversitySlots.h kFatMaleSlots: slot(s) outside 1..255: [-1]"])

    def test_slot_six_or_nine_missing_from_male_or_female_fails(self):
        _, p_female = cda.parse_diversity_slots(make_slots_header(female=(1, 9)))
        self.assertEqual(p_female, ["DiversitySlots.h kFemaleSlots: default tv slot(s) [6] missing (need 06 and 09)"])
        _, p_male = cda.parse_diversity_slots(make_slots_header(male=(1, 6)))
        self.assertEqual(p_male, ["DiversitySlots.h kMaleSlots: default tv slot(s) [9] missing (need 06 and 09)"])

    def test_slot_six_nine_only_required_in_male_and_female(self):
        _, problems = cda.parse_diversity_slots(make_slots_header(fat_m=(34,), fat_f=(35,)))
        self.assertEqual(problems, [])

    def test_count_must_match_array_length(self):
        _, problems = cda.parse_diversity_slots(make_slots_header(counts={"kMaleSlots": 3}))
        self.assertEqual(problems, ["DiversitySlots.h kMaleSlotsCount=3 but kMaleSlots has 4 entries"])

    def test_missing_array_fails(self):
        text = "\n".join(line for line in make_slots_header().splitlines() if "kFatFemaleSlots" not in line)
        _, problems = cda.parse_diversity_slots(text)
        self.assertEqual(problems, ["kFatFemaleSlots[] not parsed from DiversitySlots.h"])

    def test_no_arrays_fails_for_each_and_still_requires_vanilla(self):
        slots, problems = cda.parse_diversity_slots("namespace X {}\n")
        self.assertEqual(slots, list(cda.VANILLA_SLOTS))
        self.assertEqual(len(problems), 4)


class FileStatusTests(unittest.TestCase):
    def test_patch_and_chitin_are_ok_live_is_live_nowhere_is_missing(self):
        src = {"patch": frozenset({"a.tpc"}), "chitin": frozenset({"b.mdl"}), "live": frozenset({"c.tpc"})}
        self.assertEqual(cda.file_status("A.TPC", src), cda.OK)  # lookup is case-insensitive
        self.assertEqual(cda.file_status("b.mdl", src), cda.OK)
        self.assertEqual(cda.file_status("c.tpc", src), cda.LIVE)
        self.assertEqual(cda.file_status("d.tpc", src), cda.MISSING)

    def test_patch_is_checked_before_live(self):
        src = {"patch": frozenset({"a.tpc"}), "live": frozenset({"a.tpc"})}
        self.assertEqual(cda.file_status("a.tpc", src), cda.OK)

    def test_unavailable_source_turns_a_miss_into_unknown(self):
        self.assertEqual(cda.file_status("d.tpc", {"patch": frozenset(), "chitin": None, "live": frozenset()}),
                         cda.UNKNOWN)
        self.assertEqual(cda.file_status("d.tpc", {"patch": frozenset(), "chitin": frozenset(), "live": None}),
                         cda.UNKNOWN)

    def test_hit_in_patch_is_ok_even_when_other_sources_are_unavailable(self):
        src = {"patch": frozenset({"a.tpc"}), "chitin": None, "live": None}
        self.assertEqual(cda.file_status("a.tpc", src), cda.OK)


class NeedStatusTests(unittest.TestCase):
    def test_model_needs_both_mdl_and_mdx(self):
        self.assertEqual(cda.need_status(cda.model_need("X"), {"patch": frozenset({"x.mdl"}), "live": frozenset()}),
                         cda.MISSING)
        self.assertEqual(cda.need_status(cda.model_need("X"), {"patch": frozenset({"x.mdl", "x.mdx"}),
                                                               "live": frozenset()}), cda.OK)

    def test_texture_is_ok_with_tga_alone(self):
        src = {"patch": frozenset({"n_commm10.tga"}), "live": frozenset()}
        self.assertEqual(cda.need_status(cda.texture_need("N_CommM10"), src), cda.OK)

    def test_texture_live_when_only_live_has_an_alternative(self):
        src = {"patch": frozenset(), "live": frozenset({"p.tpc"})}
        self.assertEqual(cda.need_status(cda.texture_need("P"), src), cda.LIVE)

    def test_model_with_mdx_nowhere_is_missing_even_if_mdl_is_live(self):
        self.assertEqual(cda.need_status(cda.model_need("P"), {"patch": frozenset(), "live": frozenset({"p.mdl"})}),
                         cda.MISSING)

    def test_model_pair_in_live_is_live(self):
        src = {"patch": frozenset(), "live": frozenset({"p.mdl", "p.mdx"})}
        self.assertEqual(cda.need_status(cda.model_need("P"), src), cda.LIVE)


class BuildNeedsTests(unittest.TestCase):
    def test_commoner_bases_expand_to_every_slot_others_to_variant_01(self):
        labels = {n.label for n, _ in cda.build_needs(resource_rows(), resource_heads(), SLOTS)}
        for s in SLOTS:
            self.assertIn(f"N_CommM{s:02d}", labels)
            self.assertIn(f"N_CommF{s:02d}", labels)
        self.assertIn("N_CzerkaOff01", labels)
        self.assertNotIn("N_CzerkaOff02", labels)

    def test_references_collapse_per_file_set_and_name_their_rows(self):
        needs = cda.build_needs(resource_rows(), resource_heads(), SLOTS)
        refs = {n.label: r for n, r in needs if n.mode == "any"}
        self.assertIn("app805.texa", refs["N_CommM01"])
        self.assertEqual(len([n for n, _ in needs if n.label == "N_CommM01"]), 1)
        self.assertEqual(
            next(r for n, r in needs if n.label == "P_TwiHNGreen"), ["head198.head", "head200.head"])

    def test_head_alttexture_is_an_exact_name_without_variant_suffix(self):
        names = [n for n, _ in cda.build_needs(resource_rows(), resource_heads(), SLOTS)
                 if n.label == "llk_Ahrnell" and n.mode == "any"]
        self.assertEqual([n.files for n in names], [("llk_Ahrnell.tpc", "llk_Ahrnell.tga")])


class CheckResourcesTests(unittest.TestCase):
    def check(self, sources, rows=None, heads=None, slots=SLOTS):
        return cda.check_resources(rows if rows is not None else resource_rows(),
                                   heads if heads is not None else resource_heads(), slots, sources)

    def test_everything_in_patch_passes(self):
        rep = self.check({"patch": full_set(), "chitin": frozenset(), "live": frozenset()})
        self.assertEqual((rep.fails, rep.live, rep.unknown), ([], [], []))
        self.assertEqual(rep.ok, rep.checked)
        self.assertGreater(rep.checked, 0)

    def test_tga_alone_satisfies_a_commoner_slot(self):
        rep = self.check({"patch": full_set(exclude={"N_CommM10.tpc"}), "live": frozenset()})
        self.assertEqual(rep.fails, [])

    def test_missing_commoner_slot_fails_and_names_slot_and_row(self):
        rep = self.check({"patch": full_set(exclude={"N_CommM02.tpc", "N_CommM02.tga"}), "live": frozenset()})
        self.assertEqual(len(rep.fails), 1)
        self.assertIn("N_CommM02", rep.fails[0])
        self.assertIn("app802.texa", rep.fails[0])

    def test_missing_fat_slot_texture_fails_under_union(self):
        # 34 is only in kFatMaleSlots, but the union rule still requires N_CommM34 for the commoner base
        rep = self.check({"patch": full_set(exclude={"N_CommM34.tpc", "N_CommM34.tga"}), "live": frozenset()})
        self.assertEqual(len(rep.fails), 1)
        self.assertIn("N_CommM34", rep.fails[0])

    def test_model_missing_mdx_fails(self):
        rep = self.check({"patch": full_set(exclude={"n_commm_fat.mdx"}), "live": frozenset()})
        self.assertEqual(len(rep.fails), 1)
        self.assertIn("N_CommM_Fat", rep.fails[0])

    def test_live_only_prerequisite_is_reported_not_failed(self):
        patch = full_set(exclude={"P_TwiHNGreen.mdl", "P_TwiHNGreen.mdx"})
        rep = self.check({"patch": patch, "live": frozenset({"p_twihngreen.mdl", "p_twihngreen.mdx"})})
        self.assertEqual(rep.fails, [])
        self.assertEqual(rep.live, ["LIVE-ONLY (prerequisite) P_TwiHNGreen: needed by head198.head, head200.head"])

    def test_absent_live_turns_unresolved_names_into_warnings(self):
        rep = self.check({"patch": full_set(exclude={"P_TwiNGreen.tpc", "P_TwiNGreen.tga"}),
                          "chitin": frozenset(), "live": None})
        self.assertEqual(rep.fails, [])
        self.assertEqual(len(rep.unknown), 1)
        self.assertTrue(rep.unknown[0].startswith("WARN P_TwiNGreen unresolved, source unavailable (live)"))

    def test_name_found_nowhere_fails_when_every_source_is_present(self):
        rep = self.check({"patch": full_set(exclude={"n_commm_fat.mdl"}), "chitin": frozenset(), "live": frozenset()})
        self.assertEqual(len(rep.fails), 1)
        self.assertIn("N_CommM_Fat", rep.fails[0])

    def test_chitin_satisfies_a_vanilla_model(self):
        rep = self.check({"patch": full_set(exclude={"n_commm.mdx"}), "chitin": frozenset({"n_commm.mdx"}),
                          "live": frozenset()})
        self.assertEqual(rep.fails, [])

    def test_missing_heads_row_appearance_row_and_empty_slots_fail(self):
        heads = resource_heads()
        del heads[198]
        rows = resource_rows()
        del rows[805]
        rep = self.check({"patch": full_set(), "live": frozenset()}, rows=rows, heads=heads)
        self.assertIn("heads row 198 missing from heads.2da", rep.fails)
        self.assertIn("appearance row 805 missing from appearance.2da", rep.fails)
        rep2 = self.check({"patch": full_set(), "live": frozenset()}, slots=())
        self.assertTrue(any("no slots parsed from DiversitySlots.h" in x for x in rep2.fails))


class RunChecksResourceTests(unittest.TestCase):
    def run_it(self, sources):
        rows = {**good_rows(), **resource_rows()}
        res = cda.ResourceInputs(rows=rows, heads=resource_heads(), slots=SLOTS, sources=sources)
        return cda.run_checks(make_tables(), make_rows_header(CONSTS[:2]), rows, False, res)

    def test_clean_resources_pass_and_summary_line_is_printed(self):
        code, lines = self.run_it({"patch": full_set(), "chitin": frozenset(), "live": frozenset()})
        self.assertEqual(code, 0, "\n".join(lines))
        self.assertTrue(any(x.startswith("RESOURCES: ") and " 0 LIVE-ONLY" in x and " 0 FAIL" in x for x in lines))
        self.assertEqual(lines[-1], "RESULT: PASS")

    def test_missing_resource_fails_the_run(self):
        code, lines = self.run_it({"patch": full_set(exclude={"N_CommM01.tpc", "N_CommM01.tga"}),
                                   "chitin": frozenset(), "live": frozenset()})
        self.assertEqual(code, 1)
        self.assertTrue(any(x.startswith("FAIL N_CommM01 not found") for x in lines))
        self.assertEqual(lines[-1], "RESULT: FAIL (1 problem(s))")

    def test_rejected_pool_file_in_override_is_ignored(self):
        extra = frozenset({"n_commm20.tpc", "n_commf20.tga", "n_commf33.tpc"})  # in no slot array, so never checked
        code, lines = self.run_it({"patch": full_set() | extra, "chitin": frozenset(), "live": frozenset()})
        self.assertEqual(code, 0, "\n".join(lines))
        self.assertFalse(any(x.startswith(("WARN", "FAIL", "LIVE-ONLY")) for x in lines), "\n".join(lines))

    def test_slot_problem_from_diversity_slots_fails_the_run(self):
        rows = {**good_rows(), **resource_rows()}
        res = cda.ResourceInputs(rows=rows, heads=resource_heads(), slots=SLOTS,
                                 slot_problems=["DiversitySlots.h kMaleSlots: slot(s) outside 1..255: [0]"],
                                 sources={"patch": full_set(), "chitin": frozenset(), "live": frozenset()})
        code, lines = cda.run_checks(make_tables(), make_rows_header(CONSTS[:2]), rows, False, res)
        self.assertEqual(code, 1)
        self.assertIn("FAIL DiversitySlots.h kMaleSlots: slot(s) outside 1..255: [0]", lines)

    def test_live_only_prerequisite_prints_but_run_passes(self):
        code, lines = self.run_it({"patch": full_set(exclude={"P_TwiHNGreen.mdl", "P_TwiHNGreen.mdx"}),
                                   "chitin": frozenset(), "live": frozenset({"p_twihngreen.mdl", "p_twihngreen.mdx"})})
        self.assertEqual(code, 0, "\n".join(lines))
        self.assertIn("LIVE-ONLY (prerequisite) P_TwiHNGreen: needed by head198.head, head200.head", lines)
        self.assertEqual(lines[-1], "RESULT: PASS")

    def test_without_resources_no_resource_lines_are_printed(self):
        code, lines = cda.run_checks(make_tables(), make_rows_header(CONSTS[:2]), good_rows())
        self.assertFalse(any(x.startswith("RESOURCES:") for x in lines))
        self.assertEqual(code, 0)


if __name__ == "__main__":
    unittest.main()
