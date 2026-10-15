"""Tests for static_facts.py (T2.1).  Run from tools/: python3 -m unittest tests.test_static_facts"""
import json
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
TOOLS = HERE.parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import static_facts as sf  # noqa: E402

FIXTURE = HERE / "fixtures" / "dump_mini"
TWINS = [0x775950, 0x775970, 0x775990]
KDB_SCHEMA = "CREATE TABLE name_map (steam_addr TEXT, name TEXT, source TEXT, confidence TEXT)"


class FixtureTests(unittest.TestCase):
    def setUp(self):
        self.d = sf.Dump(FIXTURE)

    def test_callers(self):
        callers = self.d.callers(0x776DB0)
        self.assertEqual({fn for _site, fn in callers}, {0x865250, 0x7B3050, 0x8652C0})
        sites = [site for site, _fn in callers]
        self.assertEqual(sites, sorted(sites))

    def test_callees_direct_and_import(self):
        self.assertEqual(self.d.callees(0x775930), [(0x775940, 0x85D260, "CALL")])
        self.assertIn((0x776DC0, None, "IMPORT"), self.d.callees(0x776DB0))

    def test_indirect_slots(self):
        got = [(site, slot) for site, slot, _how in self.d.indirect_slots(0x7680A0)]
        self.assertEqual(got, [(0x7680A6, 0x14), (0x7680E1, 0x8)])
        self.assertIn((0x7680A6, None, "INDIRECT"), self.d.callees(0x7680A0))

    def test_strings_globals_imports(self):
        self.assertEqual(self.d.strings(0x776DB0), [(0x9868A0, "permov06")])
        self.assertEqual(self.d.globals_(0x4087A0),
                         [(0xA1B480, "WRITE", "MOV dword ptr [0x00a1b480],EDX")])
        self.assertEqual(self.d.imports(0x776DB0), [("timeGetTime", "WINMM.DLL")])

    def test_size_ret(self):
        self.assertEqual(self.d.size_ret(0x776DB0), (84, 4))
        self.assertEqual(self.d.size_ret(0x775930), (25, 0))
        self.assertIsNone(self.d.size_ret(0xDEAD0000))

    def test_twins_forwarders(self):
        self.assertEqual(self.d.twins(0x775930), TWINS)
        self.assertNotIn(0x7759B0, self.d.twins(0x775930))
        self.assertEqual(self.d.twins(0x775950), [0x775930, 0x775970, 0x775990])

    def test_distinguishing_quotes_first_differing_line(self):
        dist = self.d.distinguishing(0x775930, self.d.twins(0x775930))
        self.assertEqual(dist, {t: "0x00775940: CALL 0x0085d260" for t in TWINS})

    def test_shape_hash(self):
        self.assertEqual(self.d.shape_hash(0x775930), self.d.shape_hash(0x775950))
        self.assertNotEqual(self.d.shape_hash(0x775930), self.d.shape_hash(0x7759B0))
        self.assertIsNone(self.d.shape_hash(0xDEAD0000))

    def test_norm_insn(self):
        self.assertEqual(sf._norm_insn("CALL 0x0085d260"), "CALL #")
        self.assertEqual(sf._norm_insn("MOV dword ptr [EBP + -0x4],ECX"),
                         "MOV dword ptr [EBP + -0x4],ECX")
        self.assertEqual(sf._norm_insn("MOV EAX,dword ptr [0x00a1b480]"),
                         "MOV EAX,dword ptr [#]")
        self.assertEqual(sf._norm_insn("RET"), "RET")

    def test_asm(self):
        self.assertEqual(self.d.asm(0x775930)[0], "PUSH EBP")
        self.assertEqual(len(self.d.asm(0x775930)), 10)
        self.assertIsNone(self.d.asm(0xDEAD0000))


class BranchAndImmediateTests(unittest.TestCase):
    """0x778000/0x778100 differ only by a PUSH immediate and sit at different addresses
    with a local JZ; 0x778200 has the same mnemonics but a different local jump offset."""

    def setUp(self):
        self.d = sf.Dump(FIXTURE)

    def test_local_branch_twins_match_across_addresses(self):
        self.assertEqual(self.d.twins(0x778000), [0x778100])
        self.assertEqual(self.d.twins(0x778100), [0x778000])
        self.assertEqual(self.d.twins(0x778200), [])

    def test_rel_texts_normalise_local_targets_only(self):
        rel = dict(self.d._rel_texts(0x778100))
        self.assertEqual(rel[0x778107], "JZ L+14")
        self.assertEqual(rel[0x778109], "CALL L+0")

    def test_distinguishing_string_immediate(self):
        dist = self.d.distinguishing(0x778000, [0x778100])
        self.assertEqual(dist, {0x778100: "0x00778000: PUSH 0x009868a0"})

    def test_callers_n_counts_distinct_functions(self):
        f = sf.facts(self.d, 0x778000)
        self.assertEqual(len(self.d.callers(0x778000)), 5)
        self.assertEqual(f["callers_n"], 3)
        self.assertEqual(len(f["callers"]), 3)
        self.assertEqual(len({fn for _s, fn, _n in f["callers"]}), 3)
        self.assertIn("- callers 3: 0x00778000, 0x00779000, 0x0077a000", sf.card_lines(f, 20))

    def test_blind_self_and_string_column_names(self):
        d = sf.Dump(FIXTURE, {0x778000: "Own::Secret", 0x778100: "Twin::Name"})
        text = json.dumps(sf.facts(d, 0x778000), default=str) + "\n".join(
            sf.card_lines(sf.facts(d, 0x778000), 20))
        self.assertNotIn("Own::Secret", text)
        self.assertNotIn("Twin::Name", text)  # twins carry addresses only
        # strings.tsv 'functions' column names never reach the card
        text = "\n".join(sf.card_lines(sf.facts(d, 0x778100), 20))
        self.assertNotIn("Leaky::Name", text)
        self.assertNotIn("Leaky::Name", json.dumps(sf.facts(d, 0x778100), default=str))


class BlindnessTests(unittest.TestCase):
    def test_own_name_never_shown(self):
        names = {0x775930: "Secret::Name", 0x865250: "Foo::Bar"}
        d = sf.Dump(FIXTURE, names)
        f = sf.facts(d, 0x775930)
        self.assertNotIn("Secret::Name", json.dumps(f, default=str))
        self.assertNotIn("Secret::Name", "\n".join(sf.card_lines(f)))

    def test_caller_name_shown_then_hidden_by_exclude(self):
        names = {0x865250: "Foo::Bar"}
        d = sf.Dump(FIXTURE, names)
        shown = sf.facts(d, 0x776DB0)
        self.assertIn("Foo::Bar", [name for _s, fn, name in shown["callers"] if fn == 0x865250])
        hidden = sf.facts(d, 0x776DB0, exclude_addrs=(0x865250,))
        for _s, fn, name in hidden["callers"]:
            if fn == 0x865250:
                self.assertIsNone(name)
        self.assertNotIn("Foo::Bar", json.dumps(hidden, default=str))

    def test_load_names_kdb_exclude_sources(self):
        with tempfile.TemporaryDirectory() as td:
            td = Path(td)
            kdb = td / "k.db"
            con = sqlite3.connect(kdb)
            con.execute(KDB_SCHEMA)
            con.executemany("INSERT INTO name_map VALUES (?,?,?,?)", [
                ("0x00775930", "Dropped::Name", "B13a-z", "high"),
                ("0x00775950", "Kept::Name", "B99", "high"),
                ("0x00775970", "LowConf::Name", "B99", "low"),
            ])
            con.commit()
            con.close()
            missing = td / "no_steam.db"
            names = sf.load_names(steam_db=missing, kdb=kdb, exclude_sources=("B13a",))
            self.assertNotIn(0x775930, names)
            self.assertEqual(names[0x775950], "Kept::Name")
            self.assertNotIn(0x775970, names)
            everything = sf.load_names(steam_db=missing, kdb=kdb)
            self.assertEqual(everything[0x775930], "Dropped::Name")

    def test_load_names_steam_db_wins_and_formats_class(self):
        with tempfile.TemporaryDirectory() as td:
            td = Path(td)
            steam = td / "steam.db"
            con = sqlite3.connect(steam)
            con.execute("CREATE TABLE functions (class_name TEXT, function_name TEXT, address INTEGER)")
            con.execute("INSERT INTO functions VALUES (?,?,?)", ("Class", "Func", 0x775930))
            con.commit()
            con.close()
            kdb = td / "k.db"
            con = sqlite3.connect(kdb)
            con.execute(KDB_SCHEMA)
            con.execute("INSERT INTO name_map VALUES (?,?,?,?)",
                        ("0x00775930", "Kdb::Name", "B99", "high"))
            con.commit()
            con.close()
            names = sf.load_names(steam_db=steam, kdb=kdb)
            self.assertEqual(names[0x775930], "Class::Func")
            self.assertEqual(sf.load_names(steam_db=td / "none.db", kdb=td / "none2.db"), {})


class LoadNamesRobustnessTests(unittest.TestCase):
    def test_missing_tables_and_garbage_db_tolerated(self):
        with tempfile.TemporaryDirectory() as td:
            td = Path(td)
            empty = td / "empty.db"
            sqlite3.connect(empty).close()  # valid sqlite file, no tables
            junk = td / "junk.db"
            junk.write_bytes(b"not a database" * 50)
            self.assertEqual(sf.load_names(steam_db=empty, kdb=empty), {})
            self.assertEqual(sf.load_names(steam_db=junk, kdb=junk), {})

    def test_read_only_does_not_modify_db(self):
        with tempfile.TemporaryDirectory() as td:
            kdb = Path(td) / "k.db"
            con = sqlite3.connect(kdb)
            con.execute(KDB_SCHEMA)
            con.commit()
            con.close()
            before = kdb.read_bytes()
            sf.load_names(steam_db=Path(td) / "none.db", kdb=kdb)
            self.assertEqual(before, kdb.read_bytes())
            self.assertEqual([p.name for p in Path(td).iterdir()], ["k.db"])  # no -journal/-wal


class MissingDumpTests(unittest.TestCase):
    def test_missing_root_no_raise(self):
        with tempfile.TemporaryDirectory() as td:
            d = sf.Dump(Path(td) / "absent")
            f = sf.facts(d, 0x775930)
            self.assertFalse(f["asm_present"])
            for key in ("callers", "callees", "indirect", "strings", "globals", "imports", "twins"):
                self.assertEqual(f[key], [], key)
            self.assertIsNone(f["size"])
            self.assertIsNone(d.asm(0x775930))
            self.assertTrue(sf.card_lines(f))


class CardTests(unittest.TestCase):
    def test_card_deterministic_and_capped(self):
        f = sf.facts(sf.Dump(FIXTURE), 0x775930)
        first = sf.card_lines(f)
        self.assertEqual(first, sf.card_lines(f))
        self.assertIn("- size 25 B, RET 0", first)
        self.assertTrue(any(line.startswith("- twins: 0x00775950") for line in first))
        self.assertIn("(distinguishing: 0x00775940: CALL 0x0085d260)", "\n".join(first))
        small = sf.card_lines(f, max_lines=2)
        self.assertLessEqual(len(small), 2)
        self.assertTrue(small[0].startswith("- ... ("))
        self.assertTrue(small[-1].startswith("- twins: 0x00775950"))

    def test_twins_line_never_truncated(self):
        f = {"size": 40, "ret": 4, "asm_present": True,
             "callers_n": 9, "callers": [(0x1000 + i, 0x2000 + i, "C::F%d" % i) for i in range(9)],
             "callees": [(0x3000 + i, 0x4000 + i, "CALL", "N%d" % i) for i in range(12)],
             "indirect": [(0x5000 + i, 0x8, "x") for i in range(7)],
             "strings": [(0x6000 + i, "s%d" % i) for i in range(6)],
             "globals": [(0x7000 + i, "WRITE", "mov") for i in range(5)],
             "imports": [("imp%d" % i, "LIB.DLL") for i in range(4)],
             "twins": [0x775950, 0x775970], "distinguishing": {0x775950: "0x00775940: CALL 0x0085d260"}}
        none = dict(f, twins=[], distinguishing={})
        for cap in (1, 2, 3, 4, 10, 40):
            for fx, want in ((f, "- twins: 0x00775950, 0x00775970 (distinguishing: 0x00775940: CALL 0x0085d260)"),
                             (none, "- twins: none")):
                out = sf.card_lines(fx, max_lines=cap)
                self.assertLessEqual(len(out), cap, (cap, out))
                self.assertEqual(sum(l.startswith("- twins:") for l in out), 1, (cap, out))
                self.assertEqual(out[-1], want, cap)
                self.assertEqual(out, sf.card_lines(fx, max_lines=cap))
        four = sf.card_lines(f, max_lines=4)
        self.assertEqual(four[0], "- size 40 B, RET 4")
        self.assertTrue(four[-2].startswith("- ... ("))
        self.assertEqual(sf.card_lines(f, max_lines=0), [])

    def test_card_callers_and_indirect_lines(self):
        d = sf.Dump(FIXTURE, {0x865250: "Foo::Bar"})
        lines = sf.card_lines(sf.facts(d, 0x776DB0), max_lines=20)
        self.assertIn("- callers 3: 0x007b3050, 0x00865250 Foo::Bar, 0x008652c0", lines)
        ind = sf.card_lines(sf.facts(d, 0x7680A0), max_lines=20)
        self.assertIn("- indirect: 0x007680a6 slot 0x14, 0x007680e1 slot 0x8", ind)


@unittest.skipUnless((sf.DUMP_ROOT / "callgraph.tsv").exists(), "real Ghidra dump not present")
class RealDumpGoldTests(unittest.TestCase):
    def setUp(self):
        self.d = sf.Dump(sf.DUMP_ROOT)

    def test_callers_776db0(self):
        self.assertEqual({fn for _s, fn in self.d.callers(0x776DB0)},
                         {0x865250, 0x7B3050, 0x8652C0})

    def test_size_ret_776db0(self):
        self.assertEqual(self.d.size_ret(0x776DB0), (84, 4))

    def test_twins_775930_contains_775950(self):
        self.assertIn(0x775950, self.d.twins(0x775930))

    def test_twins_with_internal_branches(self):
        self.assertIn(0x463C80, self.d.twins(0x463D00))
        self.assertEqual(self.d.distinguishing(0x463D00, [0x463C80]),
                         {0x463C80: "0x00463d2a: CALL 0x00463f10"})


if __name__ == "__main__":
    unittest.main()
