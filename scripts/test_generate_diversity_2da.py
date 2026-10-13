#!/usr/bin/env python3
"""Tests for scripts/generate_diversity_2da.py: row set, blank cells, dancer texa, head cap, base pinning, header.

Every fixture is built in memory or written to a temp dir; the game dir and the repo's staged files are never touched.
Needs pykotor, so it re-execs with the repo .venv when pykotor is not importable (same pattern as test_gen_*.py)."""
import os
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
try:
    import pykotor  # noqa: F401
except ImportError:
    _venv = HERE.parent / ".venv" / "bin" / "python"
    if _venv.exists() and not os.environ.get("_GDIV_REEXEC"):
        os.environ["_GDIV_REEXEC"] = "1"
        os.execv(str(_venv), [str(_venv)] + sys.argv)
    raise
sys.path.insert(0, str(HERE))
import generate_diversity_2da as g  # noqa: E402
from pykotor.resource.formats.twoda import TwoDA, read_2da, write_2da  # noqa: E402

ALIEN_LABELS = ("Alien_KelDor_01", "Alien_KelDor_02", "Alien_Iridorian_01", "Alien_Iridorian_02")
APP_HEADERS = list(dict.fromkeys(
    ["label", "portrait", "racetex", "texaevil"]
    + sorted(set(g.MALE_COMM_TEMPLATE) | set(g.FEMALE_COMM_TEMPLATE) | set(g.CZERKA_OFF_F_TEMPLATE))))


def make_twoda(headers, rows):
    """rows: list of (row_label, {column: value}). Row labels are set to the row index, as the generator does."""
    t = TwoDA()
    for h in headers:
        t.add_column(h)
    for _, cells in rows:
        idx = t.add_row()
        t.set_label(idx, str(idx))
        for h in headers:
            t.set_cell(idx, h, cells.get(h, ""))
    return t


def base_appearance(n):
    """Synthetic pre-merge base with n rows (the real backup has 802)."""
    return make_twoda(APP_HEADERS, [(None, {"label": f"base{i}"}) for i in range(n)])


def heads_fixture(names):
    return make_twoda(g.HEADS_HEADERS, [(None, {"head": name}) for name in names])


def cell_rows(t):
    return [tuple(t.get_row(i).get_string(h) for h in t.get_headers()) for i in range(t.get_height())]


class MergeAppearanceTests(unittest.TestCase):
    def setUp(self):
        self.app = base_appearance(802)
        self.head_map = {}
        self.mapping, _ = g.merge_appearance_table(self.app, self.head_map)

    def test_appended_rows_are_802_to_817_without_alien_rows(self):
        self.assertEqual(self.app.get_height(), 818)
        self.assertEqual(sorted(self.mapping.values()), list(range(802, 818)))
        self.assertEqual(self.mapping["Commoner_Fat_Male"], 802)
        self.assertEqual(self.mapping["Twilek_Female_Underwear"], 811)
        self.assertEqual(self.mapping["Commoner_Dirty_Male"], 812)
        self.assertEqual(self.mapping["Twilek_Female_Tone_C"], 817)
        for label in ALIEN_LABELS:
            self.assertNotIn(label, self.mapping)
            self.assertIsNone(g._index_of(self.app, "label", label))

    def test_appended_cells_are_blank_not_literal_stars(self):
        for i in range(802, 818):
            row = self.app.get_row(i)
            for h in self.app.get_headers():
                self.assertNotEqual(row.get_string(h), "****", f"row {i} column {h}")
        self.assertEqual(self.app.get_row(802).get_string("portrait"), "")
        self.assertEqual(self.app.get_row(802).get_string("racetex"), "")
        self.assertEqual(self.app.get_row(802).get_string("backuphead"), "")

    def test_dancer_texa_follows_vanilla_twilek_rows(self):
        texa = {label: self.app.get_row(i).get_string("texa") for label, i in self.mapping.items()}
        self.assertEqual(texa["Twilek_Female_Dancer_Boot"], "N_TwiDanceA")
        self.assertEqual(texa["Twilek_Female_Dancer"], "N_TwiDanceB")
        self.assertEqual(texa["Twilek_Female_Underwear"], "N_CommF")  # no vanilla N_TwiUnd row: unchanged
        self.assertEqual(self.app.get_row(self.mapping["Twilek_Female_Dancer"]).get_string("texb"), "N_TwiDanceB")

    def test_track_a_rows_use_k1_origin_textures(self):
        def cell(label, col):
            return self.app.get_row(self.mapping[label]).get_string(col)
        for label, model, tex, head in (
            ("Commoner_Dirty_Male", "N_CommM", "N_CommMD", "9"),
            ("Commoner_Dirty_Female", "N_CommF", "N_CommFD", "8"),
            ("Czerka_Protocol_Officer_Female", "N_CommF", "N_CommFC", "83"),
            ("Twilek_Female_Tone_A", "N_TwilekF", "N_TwilekFA", "61"),
            ("Twilek_Female_Tone_B", "N_TwilekF", "N_TwilekFB", "61"),
            ("Twilek_Female_Tone_C", "N_TwilekF", "N_TwilekFC", "61"),
        ):
            self.assertEqual(cell(label, "modela"), model, label)
            self.assertEqual(cell(label, "modelb"), model, label)
            self.assertEqual(cell(label, "texa"), tex, label)
            self.assertEqual(cell(label, "texb"), tex, label)
            self.assertEqual(cell(label, "normalhead"), head, label)
        self.assertEqual(cell("Twilek_Female_Tone_A", "race"), "N_TwilekF")

    def test_generated_header_has_no_alien_rows_and_checked_bounds(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "DiversityRows.h"
            g.generate_cpp_header(self.mapping, self.head_map, out, app_first=802, app_count=818,
                                  head_first=249, head_count=249)
            text = out.read_text()
        self.assertIn("Auto-generated", text)
        self.assertNotIn("APP_ALIEN", text)
        self.assertNotIn("KELDOR", text)
        self.assertNotIn("IRIDORIAN", text)
        self.assertIn("constexpr uint16_t APP_FIRST_APPENDED = 802;", text)
        self.assertIn("constexpr uint16_t APP_COUNT = 818;", text)
        self.assertIn("constexpr uint16_t HEAD_FIRST_APPENDED = 249;", text)
        self.assertIn("constexpr uint16_t HEAD_COUNT = 249;", text)
        self.assertIn("static_assert(APP_COMMONER_FAT_MALE == APP_FIRST_APPENDED", text)
        self.assertIn("static_assert(APP_TWILEK_FEMALE_UNDERWEAR < APP_COUNT", text)
        self.assertIn("constexpr uint16_t APP_TWILEK_FEMALE_UNDERWEAR = 811;", text)
        self.assertIn("constexpr uint16_t APP_COMMONER_DIRTY_MALE = 812;", text)
        self.assertIn("constexpr uint16_t APP_TWILEK_FEMALE_TONE_C = 817;", text)


class HeadCapTests(unittest.TestCase):
    def test_refuses_to_grow_past_254_rows(self):
        heads = heads_fixture([f"P_{i}" for i in range(254)])  # none of NEW_HEADS present
        with self.assertRaises(RuntimeError):
            g.merge_heads_table(heads)
        self.assertEqual(heads.get_height(), 254)

    def test_allows_growth_up_to_exactly_254_rows(self):
        heads = heads_fixture([f"P_{i}" for i in range(249)])  # 5 NEW_HEADS fit exactly: 249 + 5 = 254
        mapping, _ = g.merge_heads_table(heads)
        self.assertEqual(heads.get_height(), 254)
        self.assertEqual(max(mapping.values()), 253)


class BasePinningTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.live = self.root / "override"
        self.backup = self.live / g.PREMERGE_BACKUP_DIRNAME
        self.live.mkdir()
        self.backup.mkdir()

    def tearDown(self):
        self._tmp.cleanup()

    def write_pair(self, app_rows, head_names, *, base_app_rows=None, base_head_names=None):
        """Writes the live pair and the backup pair. Backup defaults to the live rows unless base_* is given."""
        base_app_rows = app_rows if base_app_rows is None else base_app_rows
        base_head_names = head_names if base_head_names is None else base_head_names
        write_2da(make_twoda(["label", "modela"], [(None, {"label": l, "modela": m}) for l, m in app_rows]),
                  self.live / "appearance.2da")
        write_2da(heads_fixture(head_names), self.live / "heads.2da")
        write_2da(make_twoda(["label", "modela"], [(None, {"label": l, "modela": m}) for l, m in base_app_rows]),
                  self.backup / "appearance.2da.bak")
        write_2da(heads_fixture(base_head_names), self.backup / "heads.2da.bak")

    def base_rows(self, n):
        return [(f"base{i}", "N_CommM") for i in range(n)]

    def test_matching_base_with_generator_rows_passes(self):
        rows = self.base_rows(4) + [("Commoner_Fat_Male", "N_CommM_Fat")]
        self.write_pair(rows, ["P_a", "P_b"], base_app_rows=self.base_rows(4), base_head_names=["P_a", "P_b"])
        self.assertEqual(g.verify_base_unchanged(self.live, self.backup), [])

    def test_changed_base_row_is_reported_by_number(self):
        live_rows = self.base_rows(4)
        live_rows[2] = ("base2", "N_CommF")  # modela changed
        self.write_pair(live_rows, ["P_a"], base_app_rows=self.base_rows(4), base_head_names=["P_a"])
        problems = g.verify_base_unchanged(self.live, self.backup)
        self.assertEqual(len(problems), 1)
        self.assertIn("appearance.2da", problems[0])
        self.assertIn("first 5 rows: [2]", problems[0])

    def test_only_first_five_differing_rows_are_listed(self):
        live_rows = [(l, "N_Changed") for l, _ in self.base_rows(8)]
        self.write_pair(live_rows, ["P_a"], base_app_rows=self.base_rows(8), base_head_names=["P_a"])
        problems = g.verify_base_unchanged(self.live, self.backup)
        self.assertTrue(any("8 of 8 base rows differ" in p and "first 5 rows: [0, 1, 2, 3, 4]" in p for p in problems),
                        problems)

    def test_first_extra_row_must_be_the_first_generator_row(self):
        rows = self.base_rows(4) + [("Something_Else", "N_CommM")]
        self.write_pair(rows, ["P_a"], base_app_rows=self.base_rows(4), base_head_names=["P_a"])
        problems = g.verify_base_unchanged(self.live, self.backup)
        self.assertTrue(any("expected the first appended row 'Commoner_Fat_Male'" in p for p in problems), problems)

    def test_extra_heads_are_refused_when_no_new_head_is_expected(self):
        all_new = [h["head"] for h in g.NEW_HEADS] + ["P_a"]  # every NEW_HEADS entry already in the base
        self.write_pair(self.base_rows(4), all_new + ["P_extra"],
                        base_app_rows=self.base_rows(4), base_head_names=all_new)
        problems = g.verify_base_unchanged(self.live, self.backup)
        self.assertTrue(any("heads.2da" in p and "none expected" in p for p in problems), problems)

    def test_missing_live_file_is_a_problem(self):
        self.write_pair(self.base_rows(4), ["P_a"])
        (self.live / "heads.2da").unlink()
        problems = g.verify_base_unchanged(self.live, self.backup)
        self.assertTrue(any("heads.2da: live or backup file missing" in p for p in problems), problems)


class BackupOnceTests(unittest.TestCase):
    def test_first_backup_is_kept_and_never_overwritten(self):
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / "live.2da"
            dst = Path(tmp) / "bak.2da.bak"
            src.write_text("A")
            self.assertTrue(g.backup_once(src, dst))
            src.write_text("B")
            self.assertFalse(g.backup_once(src, dst))
            self.assertEqual(dst.read_text(), "A")


class ReadBackPinnedTests(unittest.TestCase):
    def test_round_trip_keeps_blank_cells_blank(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "appearance.2da"
            app = base_appearance(2)
            write_2da(app, path)
            back = read_2da(path)
            self.assertEqual(cell_rows(back), cell_rows(app))
            self.assertEqual(back.get_row(0).get_string("portrait"), "")


if __name__ == "__main__":
    unittest.main()
