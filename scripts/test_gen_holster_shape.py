#!/usr/bin/env python3
"""Unit tests for scripts/gen_holster_shape.py (stdlib unittest)."""

import os
import re
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import gen_holster_shape as gen  # noqa: E402

REPO = os.path.dirname(HERE)
SCRIPT = os.path.join(HERE, "gen_holster_shape.py")
R17_NAME = "R17_fit_survey_2026-09-29.txt"
R17 = os.path.join(
    REPO, "patch_manager_mods", "inventory_3d_viewport_src", "research", R17_NAME
)
INC = os.path.join(REPO, "patch_manager_mods", "holster_src", "HolsterShape.inc")

HEADER = "# R17 hierarchical bbox survey (test fixture)"
OUTLIERS = "# OUTLIERS (0 of 0)"

ENTRY_RE = re.compile(
    r'^    \{ "([^"]+)", (\d), (-?[0-9.]+)f, \{ (-?[0-9.]+)f, (-?[0-9.]+)f, (-?[0-9.]+)f \}, '
    r"(-?\d), (-?[0-9.]+)f \},$"
)


def row(stem, ext="0.100 0.200 0.300", c="0.000 0.000 0.000", letter="z", label="Test_Item"):
    return (
        f"{5:>3} {label:<28} {stem} ext=[{ext}] c=[{c}] long={letter} "
        "v=1 m=1 em=0 li=0 dg=0 sk=0 sb=0"
    )


def survey(*rows, after=()):
    lines = [HEADER, *rows, OUTLIERS, "    1 w_stunbaton_005 emitter/light/dangly", *after]
    return "\n".join(lines) + "\n"


def parse_entries(text):
    """Return (stems in file order, {stem: fields}) from generated .inc text."""
    order = []
    fields = {}
    for line in text.splitlines():
        match = ENTRY_RE.match(line)
        if not match:
            continue
        stem, axis, length, c0, c1, c2, sign, exit_len = match.groups()
        order.append(stem)
        fields[stem] = {
            "axis": int(axis),
            "len": float(length),
            "c": (float(c0), float(c1), float(c2)),
            "exitSign": int(sign),
            "exitLen": float(exit_len),
        }
    return order, fields


def entry_lines(text):
    return [line for line in text.splitlines() if line.startswith('    { "')]


class _FixtureCase(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = tmp.name

    def write(self, name, text):
        path = os.path.join(self.dir, name)
        with open(path, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        return path

    def generate(self, survey_text):
        src = self.write("survey.txt", survey_text)
        out = os.path.join(self.dir, "out.inc")
        self.assertEqual(gen.main([src, out]), 0)
        with open(out, encoding="utf-8") as fh:
            return fh.read()

    def run_cli(self, *args):
        result = subprocess.run(
            [sys.executable, SCRIPT, *args],
            cwd=self.dir,
            capture_output=True,
            text=True,
        )
        return result.returncode


class PlantedFixtureTests(_FixtureCase):
    ROW_LGHT = row(
        "w_lghtsbr_001", ext="0.203 0.203 1.320", c="-0.000 0.000 0.459",
        letter="z", label="Lightsaber",
    )
    ROW_PSTL = row(
        "w_blstrpstl_001", ext="0.081 0.405 0.220", c="0.002 0.134 0.044",
        letter="y", label="Blaster_Pistol",
    )
    ROW_VBRD = row(
        "w_vbrdblswd_003", ext="0.100 0.300 0.900", c="0.010 -0.020 0.450",
        letter="z", label="Vibro_Double_Sword",
    )

    def test_three_rows_fields_and_sort(self):
        text = self.generate(survey(self.ROW_LGHT, self.ROW_PSTL, self.ROW_VBRD))
        order, ents = parse_entries(text)
        self.assertEqual(order, ["w_blstrpstl_001", "w_lghtsbr_001", "w_vbrdblswd_003"])
        self.assertEqual(len(entry_lines(text)), 3)
        self.assertIn("static const unsigned kShapeCount = 3;", text)

        light = ents["w_lghtsbr_001"]
        self.assertEqual(light["axis"], 2)
        self.assertAlmostEqual(light["len"], 1.3200, delta=0.00005)
        self.assertEqual(light["c"], (0.0, 0.0, 0.459))
        self.assertEqual((light["exitSign"], light["exitLen"]), (0, 0.08))

        pistol = ents["w_blstrpstl_001"]
        self.assertEqual(pistol["axis"], 1)
        self.assertAlmostEqual(pistol["len"], 0.4050, delta=0.00005)
        self.assertEqual(pistol["c"], (0.002, 0.134, 0.044))
        self.assertEqual((pistol["exitSign"], pistol["exitLen"]), (0, 0.12))

        vibro = ents["w_vbrdblswd_003"]
        self.assertEqual(vibro["axis"], 2)
        self.assertAlmostEqual(vibro["len"], 0.9, delta=0.00005)
        self.assertEqual(vibro["c"], (0.01, -0.02, 0.45))
        self.assertEqual((vibro["exitSign"], vibro["exitLen"]), (-1, 0.12))

    def test_exact_entry_line_and_no_negative_zero(self):
        text = self.generate(survey(self.ROW_LGHT, self.ROW_PSTL, self.ROW_VBRD))
        self.assertIn(
            '    { "w_lghtsbr_001", 2, 1.3200f, { 0.0000f, 0.0000f, 0.4590f }, 0, 0.0800f },',
            text,
        )
        self.assertNotIn("-0.0000", text)

    def test_header_and_struct_lines(self):
        text = self.generate(survey(self.ROW_LGHT))
        lines = text.splitlines()
        self.assertEqual(
            lines[0],
            "// generated by scripts/gen_holster_shape.py from survey.txt; do not edit",
        )
        self.assertIn(
            "struct ShapeEntry { const char* stem; unsigned char axis; float len; "
            "float c[3]; signed char exitSign; float exitLen; };",
            lines,
        )
        self.assertIn("static const ShapeEntry kShapes[] = {", lines)
        self.assertEqual(lines[-1], "static const unsigned kShapeCount = 1;")
        self.assertTrue(text.endswith("\n"))


class DedupTests(_FixtureCase):
    ROW_LGHT = PlantedFixtureTests.ROW_LGHT

    def test_identical_repeat_dropped_silently(self):
        # Same ext, c and long; only the value of v= differs, which is not compared.
        repeat = self.ROW_LGHT.replace("v=1 ", "v=999 ")
        text = self.generate(survey(self.ROW_LGHT, repeat))
        self.assertEqual(len(entry_lines(text)), 1)
        self.assertNotIn("dup conflict", text)

    def test_conflicting_ext_keeps_first_and_reports_lines(self):
        later = self.ROW_LGHT.replace("ext=[0.203 0.203 1.320]", "ext=[0.203 0.203 1.100]")
        pistol = PlantedFixtureTests.ROW_PSTL
        # Physical lines: header 1, lightsaber 2, pistol 3, conflicting repeat 4.
        text = self.generate(survey(self.ROW_LGHT, pistol, later))
        self.assertIn("// dup conflict w_lghtsbr_001: kept line 2, dropped line 4", text)
        order, ents = parse_entries(text)
        self.assertEqual(order, ["w_blstrpstl_001", "w_lghtsbr_001"])
        self.assertAlmostEqual(ents["w_lghtsbr_001"]["len"], 1.3200, delta=0.00005)
        self.assertIn("static const unsigned kShapeCount = 2;", text)

    def test_conflict_on_c_or_long_is_reported(self):
        c_diff = self.ROW_LGHT.replace("c=[-0.000 0.000 0.459]", "c=[0.100 0.000 0.459]")
        long_diff = self.ROW_LGHT.replace("long=z", "long=y")
        for repeat, label in ((c_diff, "c"), (long_diff, "long")):
            with self.subTest(field=label):
                text = self.generate(survey(self.ROW_LGHT, repeat))
                self.assertIn("// dup conflict w_lghtsbr_001: kept line 2, dropped line 3", text)
                self.assertEqual(len(entry_lines(text)), 1)

    def test_stem_case_folds_to_one_key(self):
        upper = self.ROW_LGHT.replace("w_lghtsbr_001", "w_LGHTSBR_001")
        # The key is lowercased, so the uppercase repeat is an identical duplicate.
        text = self.generate(survey(self.ROW_LGHT, upper))
        self.assertIn('{ "w_lghtsbr_001",', text)
        self.assertNotIn("dup conflict", text)
        self.assertEqual(len(entry_lines(text)), 1)


class FilterAndOrderTests(_FixtureCase):
    def test_sort_is_bytewise_on_lowercase_stem(self):
        rows = [
            row("w_zzz_001", label="Z"),
            row("w_aaa_9", label="A9"),
            row("w_aaa_10", label="A10"),
            row("w_aaa_001", label="A001"),
        ]
        text = self.generate(survey(*rows))
        order, _ = parse_entries(text)
        self.assertEqual(order, ["w_aaa_001", "w_aaa_10", "w_aaa_9", "w_zzz_001"])

    def test_unknown_family_gets_zero_exit(self):
        text = self.generate(survey(row("w_zzfake_001")))
        _, ents = parse_entries(text)
        self.assertEqual((ents["w_zzfake_001"]["exitSign"], ents["w_zzfake_001"]["exitLen"]), (0, 0.0))
        self.assertIn('{ "w_zzfake_001", 2, 0.3000f, { 0.0000f, 0.0000f, 0.0000f }, 0, 0.0000f },', text)

    def test_family_suffix_stripped_for_table_lookup(self):
        text = self.generate(survey(row("w_qtrstaff_009", letter="z")))
        _, ents = parse_entries(text)
        self.assertEqual((ents["w_qtrstaff_009"]["exitSign"], ents["w_qtrstaff_009"]["exitLen"]), (0, 0.10))

    def test_multiword_label_is_matched(self):
        text = self.generate(survey(row("w_qtrstaff_002", label="Quarter Staff Long")))
        order, _ = parse_entries(text)
        self.assertEqual(order, ["w_qtrstaff_002"])

    def test_a_i_empty_and_post_outlier_rows_ignored(self):
        rows = [
            row("w_lghtsbr_001", label="Lightsaber", letter="z"),
            row("a_jedirobe_001", label="Jedi_Robe", letter="y"),
            row("i_something_001", label="Icon", letter="z"),
            "  4 Empty_Item                 w_blstrpstl_006 EMPTY ",
        ]
        text = self.generate(survey(*rows, after=(row("w_afteroutlier_001"),)))
        order, _ = parse_entries(text)
        self.assertEqual(order, ["w_lghtsbr_001"])
        self.assertNotIn("jedirobe", text)
        self.assertNotIn("i_something", text)
        self.assertNotIn("afteroutlier", text)
        self.assertIn("static const unsigned kShapeCount = 1;", text)

    def test_malformed_w_row_is_an_error(self):
        bad = "  5 Bad_Row                    w_qtrstaff_003 ext=[0.1 0.2 0.3] c=[0 0 0] long=q v=1"
        with self.assertRaises(ValueError):
            gen.generate_text(survey(bad), "survey.txt")

    def test_no_w_rows_is_an_error(self):
        with self.assertRaises(ValueError):
            gen.generate_text(survey(), "survey.txt")


class CheckFlagTests(_FixtureCase):
    def test_check_exit_codes(self):
        src = self.write("survey.txt", survey(PlantedFixtureTests.ROW_LGHT))
        out = os.path.join(self.dir, "out.inc")
        self.assertEqual(self.run_cli(src, out), 0)
        self.assertEqual(self.run_cli(src, out, "--check"), 0)
        with open(out, "a", encoding="utf-8") as fh:
            fh.write("// edited\n")
        self.assertEqual(self.run_cli(src, out, "--check"), 1)
        os.remove(out)
        self.assertEqual(self.run_cli(src, out, "--check"), 1)

    def test_usage_error_exits_two(self):
        self.assertEqual(self.run_cli(), 2)


class RealSurveyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with open(R17, encoding="utf-8") as fh:
            cls.text = gen.generate_text(fh.read(), R17_NAME)
        cls.order, cls.ents = parse_entries(cls.text)

    def test_entry_count(self):
        self.assertEqual(len(self.order), 152)
        self.assertEqual(len(entry_lines(self.text)), 152)
        self.assertIn("static const unsigned kShapeCount = 152;", self.text)

    def test_stems_sorted_and_unique(self):
        self.assertEqual(self.order, sorted(self.order))
        self.assertEqual(len(set(self.order)), len(self.order))

    def test_lghtsbr_axis_and_len(self):
        ent = self.ents["w_lghtsbr_001"]
        self.assertEqual(ent["axis"], 2)
        self.assertAlmostEqual(ent["len"], 1.3200, delta=0.0005)

    def test_blstrpstl_axis_and_len(self):
        ent = self.ents["w_blstrpstl_001"]
        self.assertEqual(ent["axis"], 1)
        self.assertAlmostEqual(ent["len"], 0.4050, delta=0.0005)

    def test_every_dblsbr_len(self):
        dbl = [s for s in self.order if s.startswith("w_dblsbr_")]
        self.assertEqual(len(dbl), 10)
        for stem in dbl:
            with self.subTest(stem=stem):
                self.assertAlmostEqual(self.ents[stem]["len"], 2.464, delta=0.001)

    def test_no_negative_zero(self):
        self.assertNotIn("-0.0000", self.text)

    def test_committed_inc_passes_check(self):
        result = subprocess.run(
            [sys.executable, SCRIPT, R17, INC, "--check"],
            cwd=HERE,
            capture_output=True,
            text=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()
