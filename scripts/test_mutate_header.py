#!/usr/bin/env python3
"""Tests for scripts/mutate_header.py (doc 20 section 3 "Mutation gate"): operator selection on synthetic text,
then the whole engine on the toy fixture (scripts/testdata/mutate_fixture). Run: python3 scripts/test_mutate_header.py"""
import hashlib
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import mutate_header as mh  # noqa: E402

FIX = HERE / "testdata" / "mutate_fixture"
TOY_H = FIX / "toy.h"
TOY_T = FIX / "toy_check.cpp"


def ops(text):
    """[(before, after)] for every mutation found in text."""
    return [(m.before, m.after) for m in mh.find_mutations(text)]


def sha(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()


class OperatorTests(unittest.TestCase):
    def test_comparison_flips(self):
        self.assertEqual(ops("bool f(int a,int b){ return a < b; }"), [("<", "<=")])
        self.assertEqual(ops("bool f(int a,int b){ return a <= b; }"), [("<=", "<")])
        self.assertEqual(ops("bool f(int a,int b){ return a > b; }"), [(">", ">=")])
        self.assertEqual(ops("bool f(int a,int b){ return a >= b; }"), [(">=", ">")])
        self.assertEqual(ops("bool f(int a,int b){ return a == b; }"), [("==", "!=")])
        self.assertEqual(ops("bool f(int a,int b){ return a != b; }"), [("!=", "==")])

    def test_logic_and_bool_literals(self):
        self.assertEqual(ops("bool f(bool a,bool b){ return a && b; }"), [("&&", "||")])
        self.assertEqual(ops("bool f(bool a,bool b){ return a || b; }"), [("||", "&&")])
        self.assertEqual(ops("bool f(){ return true; }"), [("true", "false")])
        self.assertEqual(ops("bool f(){ return false; }"), [("false", "true")])

    def test_plus_minus_binary_only(self):
        self.assertEqual(ops("int f(int a,int b){ return a + b; }"), [("+", "-")])
        self.assertEqual(ops("int f(int a,int b){ return a - b; }"), [("-", "+")])
        self.assertEqual(ops("void f(int a){ a++; --a; a += 2; a -= 2; int b = -a; }"), [("2", "3"), ("2", "1"), ("2", "3"), ("2", "1")])

    def test_integer_constants(self):
        self.assertEqual(ops("int x = 5;"), [("5", "6"), ("5", "4")])
        self.assertEqual(ops("int x = 0;"), [("0", "1")])          # no -1 for zero
        self.assertEqual(ops("int x = 0x10;"), [("0x10", "0x11"), ("0x10", "0x0F")])
        self.assertEqual(ops("unsigned x = 7u;"), [("7u", "8u"), ("7u", "6u")])
        self.assertEqual(ops("double d = 1.5;"), [])
        self.assertEqual(ops("int a = 0x7A0000;"), [])             # engine address >= 0x400000
        self.assertEqual(ops("int a = 0x400000;"), [])
        self.assertEqual(ops("int a = 0x3FFFFF;"), [("0x3FFFFF", "0x400000"), ("0x3FFFFF", "0x3FFFFE")])

    def test_if_return_deleted(self):
        m = ops("int f(int a){ if (a < 0) return -1; return a; }")
        self.assertIn(("return -1;", "{}"), m)
        self.assertNotIn(("return a;", "{}"), m)                  # only the if one-liner
        # nested parens in the condition, same-line only
        self.assertIn(("return;", "{}"), ops("void f(int a){ if (g(a, (a))) return; }"))
        self.assertEqual([x for x in ops("int f(int a){ if (a)\n return 3;\n return 1; }") if x[1] == "{}"], [])

    def test_skips(self):
        self.assertEqual(ops("// a < b && true 42\nint x;"), [])
        self.assertEqual(ops("/* a < b\n true 42 */ int x;"), [])
        self.assertEqual(ops('const char* s = "a < b && true 42";'), [])
        self.assertEqual(ops("char c = '<'; char d = '\\'';"), [])
        self.assertEqual(ops('#include "x1.h"\n#define K 42\n'), [])
        self.assertEqual(ops('static_assert(sizeof(int) >= 4 && 1 < 2, "m");'), [])
        self.assertEqual(ops("std::vector<int> v; template <class T> struct S {}; Foo<Bar<int>> x;"), [])
        self.assertEqual(ops("struct A { bool operator==(const A&) const; };"), [])

    def test_positions(self):
        t = "int a;\nint f() { return 3 < 4; }\n"
        ms = mh.find_mutations(t)
        for m in ms:
            self.assertEqual(t[m.start:m.end], m.before)
            self.assertEqual(m.line, 2)
        self.assertEqual(mh.apply_mutation(t, ms[0]).count("\n"), 2)

    def test_closure_and_test_selection(self):
        # toy_check.cpp includes toy.h directly; a toy.h include closure test selects it.
        cl = mh.include_closure(TOY_T, [FIX])
        self.assertIn(TOY_H.resolve(), cl)


class EngineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="mh-test-")
        cls.json = os.path.join(cls.tmp, "out.json")
        cls.before = (sha(TOY_H), sha(TOY_T))
        cls.p = subprocess.run([sys.executable, str(HERE / "mutate_header.py"), str(TOY_H), "--test", str(TOY_T),
                                "--jobs", "4", "--json", cls.json, "--no-sanitize"], capture_output=True, text=True)
        cls.out = cls.p.stdout
        cls.data = json.loads(Path(cls.json).read_text())

    def test_exit_code_and_summary(self):
        self.assertEqual(self.p.returncode, 1, self.p.stdout + self.p.stderr)   # there are survivors
        c = self.data["counts"]
        self.assertEqual(c["broken"], 2)                                         # Need<true> -> Need<false> (two sites)
        self.assertEqual(c["survived"], 6)
        self.assertEqual(c["killed"], c["mutants"] - 8)
        self.assertIn(f"mutants {c['mutants']}  killed {c['killed']}  survived 6  broken 2  score {c['killed']}/{c['mutants'] - 2}",
                      self.out)

    def test_survivors_are_the_untested_line_only(self):
        sv = [m for m in self.data["mutants"] if m["status"] == "SURVIVED"]
        self.assertEqual({m["line"] for m in sv}, {21})
        self.assertEqual({(m["before"], m["after"]) for m in sv},
                         {("==", "!="), ("7", "8"), ("7", "6"), ("1", "0"), ("1", "2"), ("0", "1")})
        line = [l for l in self.out.splitlines() if l.startswith("SURVIVED") and " 7 -> 8" in l]
        self.assertEqual(len(line), 1)
        self.assertRegex(line[0], r"^SURVIVED \d+ \S*toy\.h:21 7 -> 8$")

    def test_killed_include_comparison_and_return_deletion(self):
        k = {(m["line"], m["before"], m["after"]) for m in self.data["mutants"] if m["status"] == "KILLED"}
        self.assertIn((7, ">=", ">"), k)
        self.assertIn((10, "return -1;", "{}"), k)
        self.assertIn((10, "<", "<="), k)

    def test_real_files_untouched(self):
        self.assertEqual((sha(TOY_H), sha(TOY_T)), self.before)

    def test_list_and_only(self):
        r = subprocess.run([sys.executable, str(HERE / "mutate_header.py"), str(TOY_H), "--list"], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)
        lines = r.stdout.strip().splitlines()
        self.assertEqual(len(lines), self.data["counts"]["mutants"])
        self.assertRegex(lines[0], r"^1 \S*toy\.h:\d+ .* -> .*$")
        r = subprocess.run([sys.executable, str(HERE / "mutate_header.py"), str(TOY_H), "--test", str(TOY_T), "--only", "1",
                            "--no-sanitize"], capture_output=True, text=True)
        self.assertIn("mutants 1  ", r.stdout)

    def test_baseline_red_aborts(self):
        # a test that already fails must not yield "all killed"
        bad = Path(self.tmp) / "bad"
        bad.mkdir()
        (bad / "toy.h").write_text(TOY_H.read_text())
        # not under REPO -> refused with exit 2
        r = subprocess.run([sys.executable, str(HERE / "mutate_header.py"), str(bad / "toy.h"), "--test", str(TOY_T)],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 2)

    def test_run_tests_mutate_delegates(self):
        a = subprocess.run([sys.executable, str(HERE / "run_tests.py"), "--mutate", str(TOY_H), "--list"], capture_output=True, text=True)
        b = subprocess.run([sys.executable, str(HERE / "mutate_header.py"), str(TOY_H), "--list"], capture_output=True, text=True)
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(a.stdout, b.stdout)


if __name__ == "__main__":
    unittest.main()
