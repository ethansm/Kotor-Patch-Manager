#!/usr/bin/env python3
"""Tests for check_gui_ids.py: synthetic registries over in-memory control lists (no game dir, no pykotor except the
one test that reads a real .gui, which skips when pykotor is missing; run with .venv/bin/python to include it)."""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import check_gui_ids as cg
from check_gui_ids import Control, Finding

VANILLA = [Control(i, f"V{i}", (0, 0, 10, 10)) for i in range(10)]   # ids 0..9, next free id 10


def static(mod, lo, hi, rects=(), **kw):
    return {"mod": mod, "ids": [lo, hi], "rects": [list(r) for r in rects], **kw}


def levels(findings, level):
    return [f for f in findings if f.level == level]


class PureChecks(unittest.TestCase):
    def test_overlapping_static_ranges_fail(self):
        f = cg.check_static_overlap("x.gui", [static("a", 10, 20), static("b", 20, 30)])
        self.assertEqual(len(levels(f, "FAIL")), 1)
        self.assertIn("overlap", f[0].msg)

    def test_adjacent_static_ranges_ok(self):
        self.assertEqual(cg.check_static_overlap("x.gui", [static("a", 10, 20), static("b", 21, 30)]), [])

    def test_range_at_or_below_vanilla_max_fails(self):
        self.assertEqual(len(cg.check_vs_vanilla("x.gui", [static("a", 9, 12)], 9)), 1)    # starts at the max
        self.assertEqual(len(cg.check_vs_vanilla("x.gui", [static("a", 5, 12)], 9)), 1)    # reaches into vanilla
        self.assertEqual(cg.check_vs_vanilla("x.gui", [static("a", 10, 12)], 9), [])

    def test_rects_of_different_mods_intersect_fail(self):
        a = static("a", 10, 11, [(0, 0, 100, 100)])
        b = static("b", 12, 13, [(50, 50, 100, 100)])
        self.assertEqual(len(levels(cg.check_rect_collisions("x.gui", [a, b]), "FAIL")), 1)

    def test_rects_touching_or_same_mod_ok(self):
        a = static("a", 10, 11, [(0, 0, 100, 100), (50, 50, 10, 10)])    # same mod may overlap itself
        b = static("b", 12, 13, [(100, 0, 50, 50)])                       # shares only an edge with a
        self.assertEqual(cg.check_rect_collisions("x.gui", [a, b]), [])

    def test_append_rect_counts_in_collisions(self):
        a = static("a", 10, 11, [(0, 0, 100, 100)])
        ap = {"mod": "c", "count": 3, "rects": [[10, 10, 5, 5]]}
        self.assertEqual(len(cg.check_rect_collisions("x.gui", [a, ap])), 1)

    def test_live_matches(self):
        live = VANILLA + [Control(10, "LBL_A_0", (0, 0, 1, 1)), Control(11, "LBL_A_1", (0, 0, 1, 1))]
        f = cg.check_live("x.gui", [static("a", 10, 11, tag_prefix="LBL_A_")], live, 9)
        self.assertEqual(levels(f, "FAIL"), [])

    def test_live_missing_id_wrong_tag_stray_and_duplicate_fail(self):
        e = static("a", 10, 12, tag_prefix="LBL_A_")
        base = VANILLA + [Control(10, "LBL_A_0", (0, 0, 1, 1))]
        missing = cg.check_live("x.gui", [e], base + [Control(11, "LBL_A_1", (0, 0, 1, 1))], 9)
        self.assertIn("id 12 missing", levels(missing, "FAIL")[0].msg)
        wrong = cg.check_live("x.gui", [e], base + [Control(11, "OTHER", (0, 0, 1, 1)), Control(12, "LBL_A_2", (0, 0, 1, 1))], 9)
        self.assertIn("OTHER", levels(wrong, "FAIL")[0].msg)
        full = base + [Control(11, "LBL_A_1", (0, 0, 1, 1)), Control(12, "LBL_A_2", (0, 0, 1, 1))]
        stray = cg.check_live("x.gui", [e], full + [Control(13, "LBL_A_3", (0, 0, 1, 1))], 9)
        self.assertIn("unregistered", levels(stray, "FAIL")[0].msg)
        dup = cg.check_live("x.gui", [e], full + [Control(11, "LBL_A_1", (0, 0, 1, 1))], 9)
        self.assertIn("2 controls with id 11", levels(dup, "FAIL")[0].msg)

    def test_live_exact_tags_list_is_in_id_order(self):
        e = static("a", 10, 11, tags=["T_ONE", "T_TWO"])
        ok = [Control(10, "T_ONE", (0, 0, 1, 1)), Control(11, "T_TWO", (0, 0, 1, 1))]
        self.assertEqual(levels(cg.check_live("x.gui", [e], VANILLA + ok, 9), "FAIL"), [])
        swapped = [Control(10, "T_TWO", (0, 0, 1, 1)), Control(11, "T_ONE", (0, 0, 1, 1))]
        self.assertEqual(len(levels(cg.check_live("x.gui", [e], VANILLA + swapped, 9), "FAIL")), 1)

    def test_append_base(self):
        statics = [static("a", 10, 14), static("b", 15, 16)]
        f = cg.append_bases("x.gui", statics, [{"mod": "c", "count": 4, "after": ["a"]}, {"mod": "d", "count": 2}], 10)
        self.assertIn("ids 15..18", f[0].msg)    # 10 + 5 static ids of `a`
        self.assertIn("ids 10..11", f[1].msg)    # no `after`: starts at the vanilla next free id
        bad = cg.append_bases("x.gui", statics, [{"mod": "c", "count": 4, "after": ["nope"]}], 10)
        self.assertEqual(bad[0].level, "FAIL")


REGISTRY = """
[[gui]]
file = "x.gui"
[[gui.static]]
mod = "mod-a"
ids = [10, 11]
tag_prefix = "LBL_A_"
rects = [[0, 0, 100, 100]]
source = "A.cpp:1"
source_regex = ["kFirst = 10;"]
[[gui.append]]
mod = "mod-b"
count = 3
after = ["mod-a"]
rects = [[200, 0, 50, 50]]
"""


class WholeRun(unittest.TestCase):
    """cg.run over a fake repo, originals and override, with an injected loader."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        t = Path(self.tmp.name)
        self.repo, self.orig, self.ovr = t / "repo", t / "orig", t / "ovr"
        for d in (self.orig, self.ovr):
            d.mkdir()
            (d / "x.gui").write_bytes(b"")
        mod = self.repo / "patch_manager_mods" / "a_src"
        mod.mkdir(parents=True)
        (mod / "manifest.toml").write_text('[patch]\nid = "mod-a"\n')
        (mod / "A.cpp").write_text("constexpr int kFirst = 10;\n")
        bmod = self.repo / "patch_manager_mods" / "b_src"
        bmod.mkdir()
        (bmod / "manifest.toml").write_text('[patch]\nid = "mod-b"\n')
        self.reg = t / "gui_ids.toml"
        self.reg.write_text(REGISTRY)
        self.live = VANILLA + [Control(10, "LBL_A_0", (0, 0, 1, 1)), Control(11, "LBL_A_1", (0, 0, 1, 1))]

    def tearDown(self):
        self.tmp.cleanup()

    def go(self, registry=None, live=None):
        if registry is not None:
            self.reg.write_text(registry)
        files = {str(self.orig / "x.gui"): VANILLA, str(self.ovr / "x.gui"): live or self.live}
        return cg.run(self.reg, self.orig, self.ovr, self.repo, loader=lambda p: files[str(p)])

    def test_clean_registry_has_no_fail(self):
        f = self.go()
        self.assertEqual(levels(f, "FAIL"), [], f)
        self.assertTrue(any("ids 12..14" in x.msg for x in f))    # append base printed

    def test_source_regex_mismatch_fails(self):
        (self.repo / "patch_manager_mods" / "a_src" / "A.cpp").write_text("constexpr int kFirst = 12;\n")
        f = levels(self.go(), "FAIL")
        self.assertEqual(len(f), 1)
        self.assertIn("source_regex", f[0].msg)

    def test_unknown_mod_fails(self):
        f = levels(self.go(REGISTRY.replace('mod = "mod-b"', 'mod = "mod-zzz"')), "FAIL")
        self.assertTrue(any("mod-zzz" in x.msg for x in f))

    def test_overlapping_statics_fail(self):
        extra = '[[gui.static]]\nmod = "mod-b"\nids = [11, 12]\nrects = []\n'
        f = levels(self.go(REGISTRY.replace("[[gui.append]]", extra + "[[gui.append]]")), "FAIL")
        self.assertTrue(any("overlap" in x.msg for x in f))

    def test_static_below_vanilla_max_fails(self):
        f = levels(self.go(REGISTRY.replace("ids = [10, 11]", "ids = [9, 10]")), "FAIL")
        self.assertTrue(any("vanilla max id 9" in x.msg for x in f))

    def test_intersecting_rects_fail(self):
        f = levels(self.go(REGISTRY.replace("[[200, 0, 50, 50]]", "[[50, 50, 50, 50]]")), "FAIL")
        self.assertTrue(any("intersect" in x.msg for x in f))

    def test_live_drift_fails(self):
        f = levels(self.go(live=VANILLA + [Control(10, "LBL_A_0", (0, 0, 1, 1))]), "FAIL")
        self.assertTrue(any("id 11 missing" in x.msg for x in f))

    def test_absent_files_skip(self):
        (self.ovr / "x.gui").unlink()
        f = self.go()
        self.assertEqual(levels(f, "FAIL"), [])
        self.assertTrue(any(x.level == "SKIP" and "live override" in x.msg for x in f))
        (self.orig / "x.gui").unlink()
        self.assertTrue(any(x.level == "SKIP" and "vanilla original" in x.msg for x in self.go()))

    def test_repo_registry_is_well_formed(self):
        """The shipped registry parses and has no pure (file-free) failures."""
        import tomllib
        reg = tomllib.loads(cg.DEFAULT_REGISTRY.read_text(encoding="utf-8"))
        for g in reg["gui"]:
            self.assertEqual(cg.check_static_overlap(g["file"], g.get("static", [])), [])
            self.assertEqual(cg.check_rect_collisions(g["file"], g.get("static", []) + g.get("append", [])), [])


class RealGui(unittest.TestCase):
    def test_load_controls_reads_a_gui(self):
        try:
            import pykotor.resource.formats.gff  # noqa: F401
        except ImportError:
            self.skipTest("pykotor not available (run with .venv python)")
        orig = cg.DEFAULT_ORIGINALS / "equip_p.gui"
        if not orig.is_file():
            self.skipTest(f"{orig} absent")
        ctls = cg.load_controls(orig)
        self.assertEqual((len(ctls), max(c.id for c in ctls)), (49, 48))
        self.assertEqual(len(ctls[0].rect), 4)


if __name__ == "__main__":
    unittest.main()
