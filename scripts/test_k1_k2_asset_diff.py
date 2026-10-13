#!/usr/bin/env python3
"""Host-only tests for k1_k2_asset_diff.py (no game, no pykotor; synthetic resource maps in memory)."""
import collections
import random
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parent))
import k1_k2_asset_diff as d  # noqa: E402


def rec(h, *, primary="CHITIN:data.bif", copies=1, nodes="", textures=(), parse=""):
    return {"hash": h, "primary": primary, "copies": copies, "containers": primary, "nodes": nodes,
            "textures": list(textures), "mdl_parse": parse}


def sample_maps():
    k1 = {
        ("MDL", "i_belt_001"): rec("h1", nodes=3, textures=["tex_b", "tex_a"], parse="yes"),
        ("MDL", "w_x_001"): rec("same", nodes=2, textures=["t"], parse="yes"),
        ("MDL", "a_bad"): rec("hb", parse="parse-fail"),
        ("TPC", "iw_x_001"): rec("tpcA"),
        ("TGA", "zz"): rec("t"),
    }
    k2 = {
        ("MDL", "i_belt_001"): rec("h2", nodes=4, textures=["tex_b", "tex_c"], parse="yes"),
        ("MDL", "w_x_001"): rec("same", nodes=2, textures=["t"], parse="yes"),
        ("MDL", "a_bad"): rec("hc", parse="yes", textures=["q"]),
        ("TPC", "iw_x_001"): rec("tpcB"),
        ("WOK", "ww"): rec("wk"),
    }
    return k1, k2


def node(name, mesh_tex=None, children=()):
    mesh = SimpleNamespace(texture_1=mesh_tex) if mesh_tex is not None else None
    return SimpleNamespace(name=name, mesh=mesh, children=list(children))


class ClassifyTests(unittest.TestCase):
    def test_all_four_statuses(self):
        self.assertEqual(d.classify("aa", None), "only_k1")
        self.assertEqual(d.classify(None, "aa"), "only_k2")
        self.assertEqual(d.classify("aa", "aa"), "both_same_hash")
        self.assertEqual(d.classify("aa", "bb"), "both_different")

    def test_no_side_is_an_error(self):
        with self.assertRaises(ValueError):
            d.classify(None, None)

    def test_sha1_of_empty_bytes(self):
        self.assertEqual(d.sha1_hex(b""), "da39a3ee5e6b4b0d3255bfef95601890afd80709")


class FamilyTests(unittest.TestCase):
    def test_prefix_families(self):
        self.assertEqual(d.family_of("i_belt_001"), "i_")
        self.assertEqual(d.family_of("iw_lngswrd_001"), "iw")
        self.assertEqual(d.family_of("w_lngswrd_001"), "w")
        self.assertEqual(d.family_of("a_jedirobe_001"), "a")
        self.assertEqual(d.family_of("p_bastila"), "p")

    def test_item_icon_prefixes_and_literal_regex(self):
        self.assertEqual(d.family_of("ii_belt_010"), "icon")
        self.assertEqual(d.family_of("ia_class4_007"), "icon")
        self.assertEqual(d.family_of("ip_armor_001"), "icon")
        self.assertEqual(d.family_of("if_2weap01"), "icon")
        self.assertEqual(d.family_of("iarmor_001"), "icon")  # literal ^i[a-z]+_\d+ rule

    def test_family_table_labels_match_family_of(self):
        produced = {d.family_of(r) for r in ["i_belt_001", "iw_x_001", "w_x_001", "a_x_001", "p_x", "ii_x_001"]}
        self.assertEqual(set(d.FAMILIES), produced)

    def test_case_insensitive_and_non_focus(self):
        self.assertEqual(d.family_of("I_BELT_001"), "i_")
        self.assertEqual(d.family_of("W_LNGSWRD_001"), "w")
        self.assertEqual(d.family_of("tatooine"), "")
        self.assertEqual(d.family_of("ibelt"), "")
        self.assertEqual(d.family_of("iw"), "")
        self.assertEqual(d.family_of("ww"), "")

    def test_focus_filter_excludes_same_hash_and_non_focus(self):
        k1, k2 = sample_maps()
        rows = d.build_rows(k1, k2)
        focus = [(r["type"], r["resref"]) for r in rows if d.is_focus(r)]
        self.assertEqual(focus, [("MDL", "a_bad"), ("MDL", "i_belt_001"), ("TPC", "iw_x_001")])


class TextureTests(unittest.TestCase):
    def test_tex_list_diff(self):
        self.assertEqual(d.tex_list_diff(["a", "b", "c"], ["b", "d"]), (["a", "c"], ["d"]))
        self.assertEqual(d.tex_list_diff([], []), ([], []))

    def test_mdl_nodes_and_textures_synthetic_tree(self):
        root = node("root", children=[
            node("a", "Tex_B"),
            node("b", "NULL"),
            node("c", ""),
            node("d", "tex_a", [node("e", "tex_a")]),
            node("f", None),
        ])
        nodes, textures = d.mdl_nodes_and_textures(root)
        self.assertEqual(nodes, 7)
        self.assertEqual(textures, ["tex_a", "tex_b"])


class RowTests(unittest.TestCase):
    def test_rows_status_and_texture_diff(self):
        k1, k2 = sample_maps()
        rows = {(r["type"], r["resref"]): r for r in d.build_rows(k1, k2)}
        self.assertEqual(rows[("MDL", "i_belt_001")]["status"], "both_different")
        self.assertEqual(rows[("MDL", "i_belt_001")]["mdl_tex_only_k1"], "tex_a")
        self.assertEqual(rows[("MDL", "i_belt_001")]["mdl_tex_only_k2"], "tex_c")
        self.assertEqual(rows[("MDL", "w_x_001")]["status"], "both_same_hash")
        self.assertEqual(rows[("MDL", "a_bad")]["status"], "both_different")
        self.assertEqual(rows[("MDL", "a_bad")]["mdl_tex_only_k2"], "")  # K1 parse failed: not compared
        self.assertEqual(rows[("TPC", "iw_x_001")]["mdl_tex_only_k1"], "")  # not an MDL
        self.assertEqual(rows[("TGA", "zz")]["status"], "only_k1")
        self.assertEqual(rows[("WOK", "ww")]["status"], "only_k2")

    def test_rows_sorted_by_type_then_resref(self):
        k1, k2 = sample_maps()
        keys = [(r["type"], r["resref"]) for r in d.build_rows(k1, k2)]
        self.assertEqual(keys, [("MDL", "a_bad"), ("MDL", "i_belt_001"), ("MDL", "w_x_001"),
                                ("TPC", "iw_x_001"), ("TGA", "zz"), ("WOK", "ww")])

    def test_rows_independent_of_insertion_order(self):
        k1, k2 = sample_maps()
        shuffled1 = dict(random.Random(7).sample(list(k1.items()), len(k1)))
        shuffled2 = dict(random.Random(9).sample(list(k2.items()), len(k2)))
        a = d.render_tsv(d.ROW_COLS, d.build_rows(k1, k2))
        b = d.render_tsv(d.ROW_COLS, d.build_rows(shuffled1, shuffled2))
        self.assertEqual(a, b)


class TsvTests(unittest.TestCase):
    def test_render_is_deterministic_and_rectangular(self):
        k1, k2 = sample_maps()
        text1 = d.render_tsv(d.ROW_COLS, d.build_rows(k1, k2))
        text2 = d.render_tsv(d.ROW_COLS, d.build_rows(k1, k2))
        self.assertEqual(text1, text2)
        lines = text1.split("\n")
        self.assertEqual(lines[0].split("\t"), d.ROW_COLS)
        self.assertEqual(lines[-1], "")  # trailing newline only
        for line in lines[1:-1]:
            self.assertEqual(len(line.split("\t")), len(d.ROW_COLS))

    def test_tabs_and_newlines_are_cleaned(self):
        row = {c: "" for c in d.ROW_COLS}
        row.update(type="TGA", resref="x\ty", family="", status="only_k1", k1_hash="a\nb")
        text = d.render_tsv(d.ROW_COLS, [row])
        self.assertEqual(len(text.split("\n")), 3)
        self.assertIn("x y", text)

    def test_write_outputs_twice_is_byte_identical(self):
        k1, k2 = sample_maps()
        rows = d.build_rows(k1, k2)
        games = (d.Game("K1", None), d.Game("K2", None))
        k1_dir, k2_dir = Path("/games/k1"), Path("/games/k2")
        names = ["asset_diff_all.tsv", "asset_diff_focus.tsv", "asset_diff_only_k1.tsv",
                 "asset_diff_mdl_different.tsv", "asset_diff_SUMMARY.md"]
        with tempfile.TemporaryDirectory() as tmp:
            out_a = Path(tmp) / "a"  # same out dir both runs: the SUMMARY embeds it
            self.assertTrue(d.write_outputs(out_a, rows, games, k1_dir, k2_dir))
            first = {name: (out_a / name).read_bytes() for name in names}
            self.assertTrue(d.write_outputs(out_a, list(reversed(rows)), games, k1_dir, k2_dir))
            for name in names:
                self.assertEqual((out_a / name).read_bytes(), first[name], name)
            focus = (out_a / "asset_diff_focus.tsv").read_text(encoding="utf-8").splitlines()
            self.assertEqual(len(focus), 1 + 3)
            only_k1 = (out_a / "asset_diff_only_k1.tsv").read_text(encoding="utf-8").splitlines()
            self.assertEqual(len(only_k1), 1 + 1)
            mdl = (out_a / "asset_diff_mdl_different.tsv").read_text(encoding="utf-8").splitlines()
            self.assertEqual(len(mdl), 1 + 2)

    def test_all_table_skipped_at_limit_and_stale_file_removed(self):
        k1, k2 = sample_maps()
        rows = d.build_rows(k1, k2)
        games = (d.Game("K1", None), d.Game("K2", None))
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp)
            self.assertTrue(d.write_outputs(out, rows, games, Path("/k1"), Path("/k2"), all_max_bytes=10**9))
            self.assertTrue((out / "asset_diff_all.tsv").exists())
            self.assertFalse(d.write_outputs(out, rows, games, Path("/k1"), Path("/k2"), all_max_bytes=10))
            self.assertFalse((out / "asset_diff_all.tsv").exists())
            self.assertTrue((out / "asset_diff_focus.tsv").exists())
            summary = (out / "asset_diff_SUMMARY.md").read_text(encoding="utf-8")
            self.assertIn("NOT written", summary)

    def test_summary_has_counts_and_no_timestamp(self):
        k1, k2 = sample_maps()
        rows = d.build_rows(k1, k2)
        games = (d.Game("K1", None), d.Game("K2", None))
        with tempfile.TemporaryDirectory() as tmp:
            d.write_outputs(Path(tmp), rows, games, Path("/k1"), Path("/k2"))
            summary = (Path(tmp) / "asset_diff_SUMMARY.md").read_text(encoding="utf-8")
        self.assertIn("| MDL | 0 | 0 | 1 | 2 | 3 |", summary)
        self.assertIn("| w | 0 | 0 | 1 | 0 | 1 |", summary)  # family row uses the family_of() label
        self.assertNotRegex(summary, r"\d{4}-\d{2}-\d{2}")


if __name__ == "__main__":
    unittest.main(verbosity=2)
