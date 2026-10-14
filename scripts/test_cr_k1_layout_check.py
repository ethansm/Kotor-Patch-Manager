#!/usr/bin/env python3
"""Pure-logic tests for cr_k1_layout_check.py (no pykotor, no numpy, no game)."""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cr_k1_layout_check as m  # noqa: E402

CR_HEADER = "stem\tbase_src\tcr_dims\tbase_dims\tmad\thist_dist\tcorr\tstatus\n"
ASSET_HEADER = "type\tresref\tfamily\tstatus\tk1_hash\tk1_primary\n"


def _cr_line(stem, base_src, status):
    return f"{stem}\t{base_src}\t2048x2048\t256x256\t1.0\t0.1\t0.95\t{status}\n"


def _asset_line(rtype, resref, k1_hash):
    return f"{rtype}\t{resref}\t\tboth\t{k1_hash}\tCHITIN:x\n"


class BuildRowsTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.wave = Path(self._tmp.name) / "w9"
        self.wave.mkdir()
        self.asset = Path(self._tmp.name) / "asset_diff_all.tsv"

    def tearDown(self):
        self._tmp.cleanup()

    def _write(self, path, text):
        path.write_text(text, encoding="utf-8")

    def test_shard_name_from_filename(self):
        self.assertEqual(m.shard_of(Path("cr_vs_base_body_player.tsv")), "body_player")
        self.assertEqual(m.shard_of(Path("cr_vs_base_updates.tsv")), "updates")
        with self.assertRaises(ValueError):
            m.shard_of(Path("cr_classified.tsv"))

    def test_k2pack_rows_need_palette_and_k1_tpc(self):
        self._write(self.wave / "cr_vs_base_creature.tsv",
                    CR_HEADER
                    + _cr_line("c_keep", "K2pack", "palette")       # in K1 TPC set -> kept
                    + _cr_line("c_drop", "K2pack", "palette")       # not a K1 TPC -> dropped
                    + _cr_line("c_redo", "K2pack", "redesign"))     # wrong status -> dropped
        self._write(self.asset,
                    ASSET_HEADER
                    + _asset_line("TPC", "C_KEEP", "abc123")        # case-insensitive match
                    + _asset_line("MDL", "c_drop", "abc123")        # wrong type -> not a TPC
                    + _asset_line("TPC", "c_redo", "abc123")
                    + _asset_line("TPC", "c_drop", ""))             # empty k1_hash -> not counted
        k2, control = m.build_rows(self.wave, self.asset)
        self.assertEqual(k2, [("c_keep", "creature", m.SET_K2)])
        self.assertEqual(control, [])

    def test_control_rows_are_k1pack_palette_or_identical(self):
        self._write(self.wave / "cr_vs_base_party.tsv",
                    CR_HEADER
                    + _cr_line("p_a", "K1pack", "palette")
                    + _cr_line("p_b", "K1pack", "identical")
                    + _cr_line("p_c", "K1pack", "redesign")         # dropped
                    + _cr_line("p_d", "K2pack", "identical"))       # K2 base -> not control
        self._write(self.asset, ASSET_HEADER)
        k2, control = m.build_rows(self.wave, self.asset)
        self.assertEqual(k2, [])
        self.assertEqual(control, [("p_a", "party", m.SET_CONTROL), ("p_b", "party", m.SET_CONTROL)])

    def test_rows_across_shards_keep_file_order_and_dedupe_within_a_shard(self):
        self._write(self.wave / "cr_vs_base_a.tsv",
                    CR_HEADER + _cr_line("dup1", "K1pack", "palette") + _cr_line("dup1", "K1pack", "palette"))
        self._write(self.wave / "cr_vs_base_b.tsv", CR_HEADER + _cr_line("dup1", "K1pack", "palette"))
        self._write(self.asset, ASSET_HEADER)
        _, control = m.build_rows(self.wave, self.asset)
        self.assertEqual(control, [("dup1", "a", m.SET_CONTROL), ("dup1", "b", m.SET_CONTROL)])

    def test_short_rows_do_not_crash(self):
        self._write(self.wave / "cr_vs_base_x.tsv", CR_HEADER + "stemonly\n")
        self._write(self.asset, ASSET_HEADER + "TPC\tstemonly\n")
        self.assertEqual(m.build_rows(self.wave, self.asset), ([], []))


class RowFormatTest(unittest.TestCase):
    def test_header_is_exact(self):
        self.assertEqual("\t".join(m.HEADER), "stem\tshard\tcr_dims\tk1_dims\tcorr_vs_k1\tstatus\tset")

    def test_full_row(self):
        row = m.format_row("pfbal01", "body_player", "2048x2048", "256x256", 0.970621, "palette", m.SET_CONTROL)
        self.assertEqual(row, "pfbal01\tbody_player\t2048x2048\t256x256\t0.9706\tpalette\tcontrol")

    def test_nobase_row_uses_dashes(self):
        row = m.format_row("zz", "other", "2048x2048", None, None, "nobase", m.SET_K2)
        self.assertEqual(row, "zz\tother\t2048x2048\t-\t-\tnobase\tk2pack_in_k1")
        self.assertEqual(len(row.split("\t")), len(m.HEADER))


class StatusThresholdTest(unittest.TestCase):
    """Status is classify_status from k1_port_cr_sheets; these pin the palette/redesign edge."""

    def test_just_above_edge_is_palette(self):
        self.assertEqual(m.cs.classify_status(10.0, 0.9002, True), "palette")

    def test_just_below_edge_is_redesign(self):
        self.assertEqual(m.cs.classify_status(10.0, 0.8994, True), "redesign")

    def test_missing_base_is_nobase(self):
        self.assertEqual(m.cs.classify_status(None, None, False), "nobase")

    def test_near_identical_is_identical(self):
        self.assertEqual(m.cs.classify_status(1.5, 0.99, True), "identical")


class PathTest(unittest.TestCase):
    def test_cr_tga_path(self):
        self.assertEqual(m.cr_tga_path(Path("/cr"), "body_player", "pfbal01"),
                         Path("/cr/body_player/pfbal01.tga"))


if __name__ == "__main__":
    unittest.main()
