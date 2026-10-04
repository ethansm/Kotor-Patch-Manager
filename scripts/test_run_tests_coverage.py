#!/usr/bin/env python3
"""Tests for `run_tests.py --coverage`: the pure gcov parse/merge/format functions on synthetic .gcov text, and one real
gcov run on the toy fixture. Run: python3 scripts/test_run_tests_coverage.py"""
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import run_tests as rt  # noqa: E402

FIX = HERE / "testdata" / "mutate_fixture"

GCOV_A = """\
        -:    0:Source:../_shared_src/Foo.h
        -:    0:Graph:foo.gcno
        -:    1:// header
        -:    2:#pragma once
        3:    3:inline int f(int a) {
        3*:   4:  if (a) return 1;
    #####:    5:  return 2;
        -:    6:}
    =====:    7:inline int g() { return 0; }
"""
GCOV_B = """\
        -:    0:Source:../_shared_src/Foo.h
        -:    1:// header
        -:    2:#pragma once
    #####:    3:inline int f(int a) {
    #####:    4:  if (a) return 1;
        1:    5:  return 2;
        -:    6:}
    #####:    7:inline int g() { return 0; }
"""


class ParseMerge(unittest.TestCase):
    def test_parse(self):
        src, lines = rt.parse_gcov(GCOV_A)
        self.assertEqual(src, "../_shared_src/Foo.h")
        self.assertEqual(lines, {3: 3, 4: 3, 5: 0, 7: 0})     # '-' lines and line 0 are not executable

    def test_split(self):
        blocks = rt.split_gcov(GCOV_A + GCOV_B.replace("Foo.h", "Bar.h"))
        self.assertEqual([rt.parse_gcov(b)[0] for b in blocks], ["../_shared_src/Foo.h", "../_shared_src/Bar.h"])

    def test_merge_any_test_hits(self):
        acc = {}
        rt.merge_cov(acc, "/x/Foo.h", rt.parse_gcov(GCOV_A)[1])
        rt.merge_cov(acc, "/x/Foo.h", rt.parse_gcov(GCOV_B)[1])
        self.assertEqual(acc["/x/Foo.h"], {3: 3, 4: 3, 5: 1, 7: 0})
        self.assertEqual(rt.cov_counts(acc["/x/Foo.h"]), (3, 4))

    def test_format(self):
        self.assertEqual(rt.cov_line("a/B.h", 3, 4), "COV  75.0%  3/4  a/B.h")
        self.assertEqual(rt.cov_line("a/B.h", 0, 0), "COV  n/a  0/0  a/B.h")

    def test_summary_filters_and_sorts(self):
        acc = {"/r/z/Z.h": {1: 1}, "/r/a/A.h": {1: 0, 2: 1}, "/usr/include/x.h": {1: 1}}
        out = rt.cov_summary(acc, lambda p: p.startswith("/r/"), lambda p: p[3:])
        self.assertEqual(out, ["COV  50.0%  1/2  a/A.h", "COV  100.0%  1/1  z/Z.h"])


class Roots(unittest.TestCase):
    def test_roots_need_test_toml_and_skip_testdata(self):
        with tempfile.TemporaryDirectory() as t:
            b = Path(t).resolve()
            for d in ("a_src", "b_src", "a_src/testdata/fx"):
                (b / d).mkdir(parents=True)
            (b / "a_src" / "test.toml").write_text("")
            (b / "a_src" / "testdata" / "fx" / "test.toml").write_text("")
            self.assertEqual(rt.cov_roots(b), [b / "a_src"])
            roots = rt.cov_roots(b)
            self.assertTrue(rt._is_cov_header(b / "a_src" / "X.h", roots))
            self.assertFalse(rt._is_cov_header(b / "b_src" / "X.h", roots))
            self.assertFalse(rt._is_cov_header(b / "a_src" / "X.cpp", roots))

    def test_real_roots_include_known_dirs(self):
        names = {r.name for r in rt.cov_roots()}
        self.assertTrue({"_shared_src", "action_hotbar_src"} <= names, names)

    def test_same_stem_distinct_exe(self):
        a = rt.cpp_item(Path("/x/a_src/common_contract_test.cpp"), cfg={})
        b = rt.cpp_item(Path("/x/b_src/common_contract_test.cpp"), cfg={})
        self.assertNotEqual(a.cmd[0], b.cmd[0])
        self.assertEqual(a.build_cmd[-1 - 0] if False else a.cmd[0], a.build_cmd[a.build_cmd.index("-o") + 1])


class RealRun(unittest.TestCase):
    def test_toy_fixture(self):
        res = rt.run_coverage([FIX / "toy_check.cpp"], jobs=1, want=lambda p: p.name == "toy.h")
        self.assertEqual(res.failed, [])
        toy = (FIX / "toy.h").resolve()
        lines = res.acc[str(toy)]
        self.assertGreater(lines[7], 0)       # isAdult
        self.assertEqual(lines[21], 0)        # untested(): kept via -fkeep-inline-functions
        self.assertTrue(any(l.startswith("COV  ") and l.endswith("toy.h") for l in res.lines), res.lines)
        self.assertTrue((rt.TMP / "coverage").is_dir())


if __name__ == "__main__":
    unittest.main()
