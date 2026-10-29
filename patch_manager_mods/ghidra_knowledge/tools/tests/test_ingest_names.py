"""Tests for ingest_names.py (name backfill from mod code).

Run from the tools dir: python3 -m unittest tests.test_ingest_names -v
Fixtures: tests/fixtures/ingest/ (fake mod tree + 5-row steam_functions.tsv); the KDB and the
ledger are built per test in a temp dir.  The real dump and the real KDB are never opened.
"""
import csv
import io
import os
import sqlite3
import sys
import tempfile
import unittest
from unittest import mock

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
import ingest_names as inn  # noqa: E402
import rea_ledger as rl  # noqa: E402

FIX = os.path.join(HERE, "fixtures", "ingest")
FUNCS = os.path.join(FIX, "steam_functions.tsv")
DATE = "2026-10-27"

KDB_ROWS = [
    ("0x00401000", "Foo::Bar", "high"),
    ("0x00402000", "Foo::Real", "high"),
    ("0x00403000", "Mgr::PlayThing", "medium"),
    ("0x00500000", "Old::Wrong", "wrong"),
]
LEDGER_ROWS = [
    ("0x00402010", "function", "Foo::Baz"),
    ("0x00402020", "function", "something else entirely"),
]


class StubDump:
    def __init__(self, asm):
        self._asm = asm

    def asm(self, a):
        return self._asm.get(a)


def run(argv, dump=None):
    out, err = io.StringIO(), io.StringIO()
    rc = inn.main(argv, dump=dump, out=out, err=err)
    return rc, out.getvalue(), err.getvalue()


def slurp(path, mode="rb"):
    with open(path, mode, **({} if "b" in mode else {"encoding": "utf-8"})) as fh:
        return fh.read()


def read_tsv(path):
    with open(path, encoding="utf-8", newline="") as fh:
        return list(csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE))


class Base(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.tmp = self._tmp.name
        self.db = os.path.join(self.tmp, "kdb.db")
        con = sqlite3.connect(self.db)
        con.execute("create table name_map(steam_addr text primary key, name text, source text, "
                    "confidence text, evidence text, date text)")
        for a, n, c in KDB_ROWS:
            con.execute("insert into name_map values (?,?,?,?,?,?)", (a, n, "test", c, "ev", "2026-10-01"))
        con.commit()
        con.close()
        self.ledger = os.path.join(self.tmp, "ledger")
        os.mkdir(self.ledger)
        rl.append_rows(self.ledger, "20261001_seed.tsv", [
            {"addr": a, "kind": k, "name": n, "confidence": "confirmed",
             "evidence": "%s: PUSH EBP" % a, "topic": "seed", "date": "2026-10-01"}
            for a, k, n in LEDGER_ROWS])
        self.common = ["--kdb", self.db, "--ledger-dir", self.ledger, "--functions", FUNCS, "--root", FIX]

    def extract_rows(self):
        out = os.path.join(self.tmp, "c.tsv")
        rc, _o, _e = run(["extract", "--source", "modcode", "--out", out] + self.common)
        self.assertEqual(rc, 0)
        return out, read_tsv(out)

    def find(self, rows, addr, name):
        got = [r for r in rows if r["addr"] == addr and r["name"] == name]
        self.assertTrue(got, f"no row {addr} {name}")
        return got[0]


class ExtractTests(Base):
    def test_header_and_sorted(self):
        out, rows = self.extract_rows()
        with open(out, encoding="utf-8") as fh:
            self.assertEqual(fh.readline().rstrip("\n").split("\t"), list(inn.EXTRACT_COLS))
        keys = [(r["addr"], r["src"]) for r in rows]
        self.assertEqual(keys, sorted(keys))
        self.assertEqual(len({(r["addr"], r["name"], r["src"]) for r in rows}), len(rows))

    def test_pattern_a_hooks_toml(self):
        _o, rows = self.extract_rows()
        bar = self.find(rows, "0x00401000", "Foo::Bar")
        self.assertEqual(bar["form"], "qualified")
        self.assertTrue(bar["src"].endswith("foo.hooks.toml:4"), bar["src"])
        self.assertEqual(self.find(rows, "0x00402010", "Foo::Baz")["src"].split(":")[0],
                         "patch_manager_mods/foo_src/foo.hooks.toml")
        # no class comment (0x00402000) and comment more than 6 lines above (0x00403004)
        self.assertFalse([r for r in rows if r["addr"] == "0x00403004"])
        self.assertFalse([r for r in rows if r["addr"] == "0x00402000" and r["src"].endswith("hooks.toml:8")])

    def test_pattern_b_constexpr_bare(self):
        _o, rows = self.extract_rows()
        r = self.find(rows, "0x00403000", "PlayThing")
        self.assertEqual((r["form"], r["kind"]), ("bare", "function"))
        self.assertEqual(r["src"], "patch_manager_mods/foo_src/foo.h:2")

    def test_pattern_c_comments(self):
        _o, rows = self.extract_rows()
        self.assertEqual(self.find(rows, "0x00500000", "Foo::Qux")["src"].split(":")[1], "1")  # name then address
        self.assertTrue(self.find(rows, "0x00600000", "Baz::Zed"))  # /* ... */ with @
        r = self.find(rows, "0x00600004", "Baz::Inner")  # address then name
        self.assertEqual(r["kind"], "callsite")

    def test_kind_fn_start_offset(self):
        _o, rows = self.extract_rows()
        fn = self.find(rows, "0x00500000", "Foo::Qux")
        self.assertEqual((fn["kind"], fn["fn_start"], fn["offset"]), ("function", "0x00500000", "0x0"))
        cs = self.find(rows, "0x00600004", "Baz::Inner")
        self.assertEqual((cs["kind"], cs["fn_start"], cs["offset"]), ("callsite", "0x00600000", "0x4"))
        cs2 = self.find(rows, "0x00402010", "Foo::Baz")
        self.assertEqual((cs2["fn_start"], cs2["offset"]), ("0x00402000", "0x10"))
        g = self.find(rows, "0x00990010", "SomeGlobal")
        self.assertEqual((g["kind"], g["fn_start"], g["offset"]), ("global", "", ""))

    def test_status_kdb(self):
        _o, rows = self.extract_rows()
        agrees = self.find(rows, "0x00401000", "Foo::Bar")
        self.assertEqual((agrees["status"], agrees["kdb_name"]), ("agrees", "Foo::Bar"))
        bare = self.find(rows, "0x00403000", "PlayThing")  # bare name agrees with Class::Name
        self.assertEqual((bare["status"], bare["kdb_name"]), ("agrees", "Mgr::PlayThing"))
        conf = self.find(rows, "0x00402000", "kConflictFn")
        self.assertEqual((conf["status"], conf["kdb_name"]), ("conflicts", "Foo::Real"))
        self.assertEqual(self.find(rows, "0x00990010", "SomeGlobal")["status"], "new")

    def test_wrong_rows_ignored(self):
        _o, rows = self.extract_rows()
        r = self.find(rows, "0x00500000", "Foo::Qux")
        self.assertEqual((r["status"], r["kdb_name"]), ("new", ""))

    def test_status_ledger(self):
        _o, rows = self.extract_rows()
        same = self.find(rows, "0x00402010", "Foo::Baz")
        self.assertEqual((same["status"], same["kdb_name"]), ("agrees", "rea:Foo::Baz"))
        diff = self.find(rows, "0x00402020", "Foo::Other")
        self.assertEqual((diff["status"], diff["kdb_name"]), ("conflicts", "rea:something else entirely"))

    def test_dump_excerpts_excluded(self):
        _o, rows = self.extract_rows()
        self.assertFalse([r for r in rows if r["name"] == "Hidden::Fn"])

    def test_kdb_opened_read_only(self):
        before = slurp(self.db)
        self.extract_rows()
        self.assertEqual(slurp(self.db), before)


class NotesTests(Base):
    def setUp(self):
        super().setUp()
        self.nroot = os.path.join(FIX, "notes_tree")
        self.ncommon = ["--kdb", self.db, "--ledger-dir", self.ledger, "--functions", FUNCS, "--root", self.nroot]

    def notes_rows(self, *extra):
        out = os.path.join(self.tmp, "n.tsv")
        rc, o, _e = run(["extract", "--source", "notes", "--out", out] + list(extra) + self.ncommon)
        self.assertEqual(rc, 0)
        return out, read_tsv(out), o

    def test_header_has_cites_and_ranked(self):
        out, rows, _o = self.notes_rows()
        with open(out, encoding="utf-8") as fh:
            self.assertEqual(fh.readline().rstrip("\n").split("\t"), list(inn.EXTRACT_COLS) + ["cites"])
        cites = [int(r["cites"]) for r in rows]
        self.assertEqual(cites, sorted(cites, reverse=True))
        self.assertEqual(rows[0]["name"], "Foo::Bar")

    def test_table_row_with_header_named_column(self):
        _o, rows, _x = self.notes_rows()
        # header "what" is not name-like, so the first identifier-only cell is used
        r = self.find(rows, "0x00600000", "Gui::OnClick")
        self.assertEqual(r["src"], ".claude/research/tables.md:12")
        self.assertEqual(r["kind"], "function")
        # header "Function" names the column
        self.assertTrue(self.find(rows, "0x00500000", "SomeThing")["src"].endswith("tables.md:8"))

    def test_prose_and_hex_cells_rejected(self):
        _o, rows, _x = self.notes_rows()
        self.assertFalse([r for r in rows if r["addr"] == "0x00402000"])  # prose cell
        self.assertFalse([r for r in rows if r["addr"] == "0x00600004"])  # "hello world"
        self.assertFalse([r for r in rows if r["addr"] == "0x00600008"])  # plain lowercase word
        self.assertFalse([r for r in rows if r["name"].startswith("0x")])
        self.assertFalse([r for r in rows if r["addr"] == "0x00403000" and r["src"].endswith("tables.md:7")])

    def test_tool_names_rejected(self):
        for junk in ("procedure_pseudo_code", "read_bytes", "invalid_parameter", "search_strings"):
            self.assertFalse(inn.name_like(junk), junk)
        self.assertTrue(inn.name_like("CSWGuiListBox::AddControls"))
        self.assertTrue(inn.name_like("Read_Helper"))

    def test_cites_counts_distinct_lines(self):
        _o, rows, _x = self.notes_rows()
        bar = [r for r in rows if r["name"] == "Foo::Bar"]
        self.assertEqual({r["cites"] for r in bar}, {"3"})
        self.assertEqual(len(bar), 3)  # table row, plain line, more.md; duplicate pattern hits on one line collapse
        self.assertEqual(self.find(rows, "0x00990010", "g_Table")["kind"], "global")
        self.assertEqual(self.find(rows, "0x00403000", "Mgr::PlayThing")["status"], "agrees")

    def test_top_keeps_most_cited_new_only(self):
        _o, rows, out = self.notes_rows("--top", "1")
        # Foo::Bar (3 cites) and Mgr::PlayThing agree with the KDB, so only new rows compete; tie goes to the lowest addr
        self.assertTrue(all(r["status"] == "new" for r in rows))
        self.assertEqual(len({r["addr"] for r in rows}), 1)
        self.assertEqual(rows[0]["addr"], "0x00500000")
        _o2, all_rows, _x = self.notes_rows()
        _o3, top2, _x = self.notes_rows("--top", "2")
        self.assertEqual(len({r["addr"] for r in top2}), 2)
        self.assertGreater(len(all_rows), len(top2))

    def test_ledger_reads_notes_output_and_dry_run_out(self):
        out, _rows, _o = self.notes_rows()
        dump = StubDump({0x500000: ["PUSH EBP"], 0x600000: ["MOV EAX,1"], 0x990010: None})
        rows_out = os.path.join(self.tmp, "dry.tsv")
        rc, o, _e = run(["ledger", "--in", out, "--topic", "ingest-notes", "--date", DATE, "--out", rows_out]
                        + self.ncommon, dump=dump)
        self.assertEqual(rc, 0)
        self.assertIn("dry-run: would-write=", o)
        got = read_tsv(rows_out)
        with open(rows_out, encoding="utf-8") as fh:
            self.assertEqual(fh.readline().rstrip("\n").split("\t"), list(rl.COLS))
        self.assertEqual({r["name"] for r in got}, {"SomeThing::Other", "Gui::OnClick", "g_Table"})  # tie: first by src
        self.assertEqual(os.listdir(self.ledger), ["20261001_seed.tsv"])  # dry run never writes the ledger

    def test_modcode_output_unchanged(self):
        out, rows = self.extract_rows()
        with open(out, encoding="utf-8") as fh:
            self.assertEqual(fh.readline().rstrip("\n").split("\t"), list(inn.EXTRACT_COLS))
        self.assertNotIn("cites", rows[0])
        self.assertEqual(len(rows), 11)


class GapTests(Base):
    def test_gap_counts(self):
        res = inn.gap("all", self.db, self.ledger, FUNCS, FIX)
        mc = res["mod_code"]
        self.assertEqual(mc["files"], 3)  # dump_excerpts file excluded
        self.assertEqual(mc["cited"], 5)
        self.assertEqual(mc["unknown"], 2)  # 0x00500000 ('wrong' in KDB) and 0x00600000
        self.assertEqual({t["fn"] for t in mc["top"]}, {"0x00500000", "0x00600000"})
        notes = res["research_notes"]
        self.assertEqual((notes["files"], notes["cited"], notes["unknown"]), (1, 3, 2))

    def test_gap_source_filter_and_json(self):
        rc, out, _e = run(["gap", "--source", "modcode", "--json"] + self.common)
        self.assertEqual(rc, 0)
        import json
        self.assertEqual(list(json.loads(out)), ["mod_code"])
        rc, out, _e = run(["gap", "--source", "notes"] + self.common)
        self.assertIn("research_notes: files=1 functions cited=3 unnamed/unledgered=2", out)


FP_HEAD = "addr size retn n_insn callconv thunk_target callees str_refs disp imm name namespace source".split()


def fp_row(addr, size, name, ns="", callees="", strs=""):
    return {"addr": addr, "size": size, "retn": "0", "n_insn": "9", "callconv": "__stdcall",
            "thunk_target": "", "callees": callees, "str_refs": strs, "disp": "", "imm": "",
            "name": name, "namespace": ns, "source": "DEFAULT"}


def write_fp(server, program, rows):
    d = os.path.join(server, program)
    os.makedirs(d)
    with open(os.path.join(d, "fp.tsv"), "w", encoding="utf-8", newline="") as fh:
        fh.write("\t".join(FP_HEAD) + "\n")
        for r in rows:
            fh.write("\t".join(r[c] for c in FP_HEAD) + "\n")


def fake_gap(*_a, **_kw):
    def top(*pairs):
        return [{"fn": a, "cites": c, "in": ["x"]} for a, c in pairs]
    ten = [("0x00401000", 3), ("0x00402000", 1), ("0x00403000", 1), ("0x00404000", 2),
           ("0x00405000", 5), ("0x00406000", 1), ("0x00407000", 1)]
    return {"research_notes": {"files": 1, "cited": 1, "unknown": len(ten), "top": top(*ten)},
            "mod_code": {"files": 1, "cited": 1, "unknown": 1, "top": top(("0x00401000", 4))}}


class TwinsTests(Base):
    def setUp(self):
        super().setUp()
        self.server = os.path.join(self.tmp, "server")
        # Steam gap fns: 401 unique+named twin, 402 steam key repeated, 403 gog key repeated,
        # 404 FUN_ twin, 405 empty namespace twin, 406 no twin in GOG, 407 no fp row at all
        write_fp(self.server, inn.FP_STEAM, [
            fp_row("0x00401000", "40", "FUN_00401000", callees="0x1,0x2", strs="0x9"),
            fp_row("0x00402000", "41", "FUN_00402000"),
            fp_row("0x00402100", "41", "FUN_00402100"),
            fp_row("0x00403000", "42", "FUN_00403000"),
            fp_row("0x00404000", "43", "FUN_00404000"),
            fp_row("0x00405000", "44", "FUN_00405000"),
            fp_row("0x00406000", "45", "FUN_00406000"),
        ])
        write_fp(self.server, inn.FP_GOG, [
            fp_row("0x00501000", "40", "Open", ns="Foo", callees="0x5,0x6", strs="0x7"),
            fp_row("0x00502000", "41", "Dup", ns="Foo"),
            fp_row("0x00503000", "42", "DupA", ns="Foo"),
            fp_row("0x00503100", "42", "DupB", ns="Foo"),
            fp_row("0x00504000", "43", "FUN_00504000"),
            fp_row("0x00505000", "44", "Helper"),
        ])
        self.out_tsv = os.path.join(self.tmp, "twins.tsv")
        self.args = ["twins", "--server", self.server, "--out", self.out_tsv] + self.common

    def run_twins(self):
        with mock.patch.object(inn, "gap", fake_gap):
            return run(self.args)

    def test_unique_twin_emitted_and_global_namespace(self):
        rc, _o, _e = self.run_twins()
        self.assertEqual(rc, 0)
        rows = read_tsv(self.out_tsv)
        self.assertEqual([tuple(r.values()) for r in rows], [
            ("0x00501000", "0x00401000", "fptwin", "Foo::Open", "7"),  # 3 + 4 cites summed over groups
            ("0x00505000", "0x00405000", "fptwin", "Global::Helper", "5"),
        ])

    def test_header(self):
        self.run_twins()
        with open(self.out_tsv, encoding="utf-8") as fh:
            self.assertEqual(fh.readline().rstrip("\n").split("\t"), list(inn.TWIN_COLS))

    def test_summary_counts(self):
        rc, out, _e = self.run_twins()
        self.assertEqual(rc, 0)
        self.assertEqual(out.strip(), "twins: gap=7 named=2 unnamed=1 ambiguous=2 no_twin=1 no_fp=1 -> " + self.out_tsv)

    def test_key_repeated_either_side_is_ambiguous(self):
        res, counts = None, None
        with mock.patch.object(inn, "gap", fake_gap):
            res, counts = inn.twins(self.server)
        self.assertEqual(counts["ambiguous"], 2)
        self.assertNotIn("0x00402000", {r["steam"] for r in res})
        self.assertNotIn("0x00403000", {r["steam"] for r in res})

    def test_missing_fp_returns_2(self):
        self.args[2] = os.path.join(self.tmp, "nowhere")
        rc, _o, err = self.run_twins()
        self.assertEqual(rc, 2)
        self.assertIn("ingest_names:", err)

    def test_server_env_default(self):
        self.args[1:3] = []
        with mock.patch.dict(os.environ, {"BD_SERVER": self.server}):
            rc, out, _e = self.run_twins()
        self.assertEqual(rc, 0)
        self.assertIn("named=2", out)


    def test_buckets_file_lists_every_gap_fn(self):
        gf = os.path.join(self.tmp, "gog_functions.tsv")
        with open(gf, "w", encoding="utf-8") as fh:
            fh.write("gog_addr\tclass\tfunction\tcallconv\tparam_size_bytes\tnotes\n00501000\tFoo\tOpen\t\t0\t\n")
        bpath = os.path.join(self.tmp, "buckets.tsv")
        self.args += ["--buckets", bpath, "--gog-functions", gf]
        rc, out, _e = self.run_twins()
        self.assertEqual(rc, 0)
        self.assertIn("twins: buckets=7 -> " + bpath, out)
        rows = {r["steam"]: r for r in read_tsv(bpath)}
        self.assertEqual(list(read_tsv(bpath)[0]), list(inn.BUCKET_COLS))
        self.assertEqual({a: r["bucket"] for a, r in rows.items()}, {
            "0x00401000": "named", "0x00402000": "ambiguous", "0x00403000": "ambiguous", "0x00404000": "unnamed",
            "0x00405000": "named", "0x00406000": "no_twin", "0x00407000": "no_fp"})
        self.assertEqual(rows["0x00402000"]["steam_peers"], "0x00402100")
        self.assertEqual(rows["0x00403000"]["gog_addrs"], "0x00503000,0x00503100")
        self.assertEqual(rows["0x00403000"]["name"], "Foo::DupA,Foo::DupB")
        self.assertEqual(rows["0x00401000"]["in_gog_functions"], "yes")
        self.assertEqual(rows["0x00405000"]["in_gog_functions"], "no")
        self.assertEqual(rows["0x00402000"]["order_gog"], "")

    def test_order_flag_adds_count_but_unequal_groups_never_pair(self):
        self.args.append("--order")
        rc, out, _e = self.run_twins()
        self.assertEqual(rc, 0)
        self.assertIn("no_fp=1 order=0 -> ", out)
        self.assertEqual({r["source"] for r in read_tsv(self.out_tsv)}, {"fptwin"})


class OrderPairTests(unittest.TestCase):
    S, G = [0x401000, 0x402000], [0x501000, 0x502000]

    def test_monotone_run_pairs(self):
        anchors = [(0x400000, 0x500000), (0x401800, 0x501800), (0x403000, 0x503000)]
        self.assertEqual(inn.order_pairs(self.S, self.G, anchors), list(zip(self.S, self.G)))

    def test_anchor_inside_span_breaks_order(self):
        self.assertEqual(inn.order_pairs(self.S, self.G, [(0x401800, 0x502800)]), [])

    def test_nearest_outside_anchor_breaks_order(self):
        # steam before the group, gog after the first GOG member: crossing
        self.assertEqual(inn.order_pairs(self.S, self.G, [(0x400000, 0x501800)]), [])

    def test_far_anchor_out_of_run_ignored(self):
        anchors = [(0x100000, 0x900000), (0x400000, 0x500000), (0x403000, 0x503000)]
        self.assertEqual(len(inn.order_pairs(self.S, self.G, anchors)), 2)

    def test_unequal_or_single_groups(self):
        self.assertEqual(inn.order_pairs(self.S, self.G[:1], []), [])
        self.assertEqual(inn.order_pairs(self.S[:1], self.G[:1], []), [])

    def test_anchor_same_steam_other_gog_breaks(self):
        self.assertEqual(inn.order_pairs(self.S, self.G, [(0x401000, 0x501100)]), [])


class OrderTwinsTests(Base):
    def setUp(self):
        super().setUp()
        self.server = os.path.join(self.tmp, "server")
        # group A (size 41): steam 401/402, gog 501/502 -> pairs by order; group B (size 42): crossing anchor
        write_fp(self.server, inn.FP_STEAM, [
            fp_row("0x00401000", "41", "FUN_00401000"), fp_row("0x00402000", "41", "FUN_00402000"),
            fp_row("0x00403000", "40", "FUN_00403000"),
            fp_row("0x00404000", "42", "FUN_00404000"), fp_row("0x00405000", "42", "FUN_00405000"),
            fp_row("0x00406000", "43", "FUN_00406000"), fp_row("0x00407000", "41", "FUN_00407000")])
        write_fp(self.server, inn.FP_GOG, [
            fp_row("0x00501000", "41", "A1", ns="Foo"), fp_row("0x00502000", "41", "FUN_00502000"),
            fp_row("0x00503000", "40", "Mid", ns="Foo"),
            fp_row("0x00504000", "42", "B1", ns="Foo"), fp_row("0x00505000", "42", "B2", ns="Foo"),
            fp_row("0x00503500", "43", "Cross", ns="Foo"), fp_row("0x00507000", "41", "A3", ns="Foo")])
        self.out_tsv = os.path.join(self.tmp, "twins.tsv")
        self.bpath = os.path.join(self.tmp, "b.tsv")
        self.args = ["twins", "--server", self.server, "--out", self.out_tsv, "--order",
                     "--buckets", self.bpath, "--gog-functions", os.path.join(self.tmp, "none.tsv")] + self.common
        with open(self.args[-len(self.common) - 1], "w", encoding="utf-8") as fh:
            fh.write("gog_addr\n")

    def gap7(self, *_a, **_kw):
        top = [{"fn": "0x%08x" % a, "cites": 1, "in": ["x"]} for a in (0x401000, 0x402000, 0x404000, 0x405000)]
        return {"research_notes": {"files": 1, "cited": 4, "unknown": 4, "top": top}}

    def test_order_rows(self):
        with mock.patch.object(inn, "gap", self.gap7):
            rc, out, _e = run(self.args)
        self.assertEqual(rc, 0)
        # group A (3 steam: 401, 402, 407 vs 3 gog: 501, 502, 507) runs in order around anchor 403<->503;
        # 402<->502 pairs but GOG is unnamed -> bucket only.  Group B crosses unique twin 406<->5035.
        self.assertIn("order=1", out)
        rows = read_tsv(self.out_tsv)
        self.assertEqual([(r["steam"], r["gog"], r["source"], r["name"]) for r in rows],
                         [("0x00401000", "0x00501000", "fptwin_order", "Foo::A1")])
        b = {r["steam"]: r for r in read_tsv(self.bpath)}
        self.assertEqual(b["0x00402000"]["order_gog"], "0x00502000")
        self.assertEqual(b["0x00402000"]["in_gog_functions"], "no")
        self.assertEqual(b["0x00404000"]["order_gog"], "")
        self.assertEqual(b["0x00404000"]["bucket"], "ambiguous")


class TwinExtractTests(unittest.TestCase):
    def test_only_named_absent_non_fid(self):
        rows = [{"gog": "0x00501000", "steam": "0x00401000", "source": "fptwin", "name": "Global::_strcmp"},
                {"gog": "0x00502000", "steam": "0x00402000", "source": "fptwin", "name": "Foo::InList"},
                {"gog": "0x00503000", "steam": "0x00403000", "source": "fptwin", "name": "Global::FID_conflict:x"},
                {"gog": "0x00504000", "steam": "0x00404000", "source": "fptwin_order", "name": "Foo::Ord"}]
        buckets = [{"steam": "0x00401000", "bucket": "named", "in_gog_functions": "no"},
                   {"steam": "0x00402000", "bucket": "named", "in_gog_functions": "yes"},
                   {"steam": "0x00403000", "bucket": "named", "in_gog_functions": "no"},
                   {"steam": "0x00404000", "bucket": "ambiguous", "in_gog_functions": "no"}]
        ex = inn.twin_extract_rows(rows, buckets)
        self.assertEqual([(r["addr"], r["name"], r["src"], r["status"]) for r in ex],
                         [("0x00401000", "Global::_strcmp", "fptwin GOG 0x00501000", "new")])
        self.assertEqual(set(ex[0]), set(inn.EXTRACT_COLS))


class GraphPairTests(Base):
    """Anchors (unique fp twins, sizes 11-16) map callers/callees; gap fns sit in size 50/60 groups or have no twin."""

    def setUp(self):
        super().setUp()
        self.server = os.path.join(self.tmp, "server")
        write_fp(self.server, inn.FP_STEAM, [
            fp_row("0x00402000", "50", "FUN_00402000"), fp_row("0x00403000", "50", "FUN_00403000"),
            fp_row("0x00404000", "11", "FUN_00404000", callees="0x00402000"),
            fp_row("0x00405000", "12", "FUN_00405000", callees="0x00402000"),
            fp_row("0x00406000", "60", "FUN_00406000"), fp_row("0x00407000", "60", "FUN_00407000"),
            fp_row("0x00408000", "13", "FUN_00408000", callees="0x00406000,0x00407000"),
            fp_row("0x00408800", "17", "FUN_00408800", callees="0x00406000,0x00407000"),
            fp_row("0x00409000", "70", "FUN_00409000", callees="0x0040b000"),
            fp_row("0x0040a000", "14", "FUN_0040a000", callees="0x00409000"),
            fp_row("0x0040b000", "15", "FUN_0040b000"),
            fp_row("0x0040c000", "80", "FUN_0040c000"),
            fp_row("0x0040d000", "16", "FUN_0040d000", callees="0x0040c000")])
        write_fp(self.server, inn.FP_GOG, [
            fp_row("0x00502000", "50", "Win", ns="Foo"), fp_row("0x00503000", "50", "Lose", ns="Foo"),
            fp_row("0x00504000", "11", "C1", ns="Foo", callees="0x00502000"),
            fp_row("0x00505000", "12", "C2", ns="Foo", callees="0x00502000"),
            fp_row("0x00506000", "60", "T1", ns="Foo"), fp_row("0x00507000", "60", "T2", ns="Foo"),
            fp_row("0x00508000", "13", "C3", ns="Foo", callees="0x00506000,0x00507000"),
            fp_row("0x00508800", "17", "C4", ns="Foo", callees="0x00506000,0x00507000"),
            fp_row("0x00509000", "71", "NoTwin", ns="Foo", callees="0x0050b000"),
            fp_row("0x0050a000", "14", "C5", ns="Foo", callees="0x00509000"),
            fp_row("0x0050b000", "15", "X", ns="Foo"),
            fp_row("0x0050d000", "16", "C6", ns="Foo", callees="0x0050b000")])
        self.bpath = os.path.join(self.tmp, "b.tsv")
        with open(self.bpath, "w", encoding="utf-8") as fh:
            fh.write("steam\tbucket\tgog_addrs\n"
                     "0x00402000\tambiguous\t0x00502000,0x00503000\n"
                     "0x00406000\tambiguous\t0x00506000,0x00507000\n"
                     "0x00409000\tno_twin\t\n"
                     "0x0040c000\tno_twin\t\n")
        self.out_tsv = os.path.join(self.tmp, "g.tsv")

    def test_graphpair(self):
        rc, out, _e = run(["graphpair", "--server", self.server, "--buckets", self.bpath, "--out", self.out_tsv]
                          + self.common)
        self.assertEqual(rc, 0)
        rows = {r["steam"]: r for r in read_tsv(self.out_tsv)}
        # ambiguous pair decided by 2 mapped caller hits
        self.assertEqual((rows["0x00402000"]["gog"], rows["0x00402000"]["name"], rows["0x00402000"]["score"],
                          rows["0x00402000"]["margin"], rows["0x00402000"]["fp_equal"]),
                         ("0x00502000", "Foo::Win", "2", "2", "yes"))
        # tie 2:2 -> no row
        self.assertNotIn("0x00406000", rows)
        # no_twin found via its mapped caller's GOG callee (+ mapped callee hit)
        self.assertEqual((rows["0x00409000"]["gog"], rows["0x00409000"]["fp_equal"], rows["0x00409000"]["bucket"]),
                         ("0x00509000", "no", "no_twin"))
        # the only candidate (C6's callee X) is already mapped -> excluded
        self.assertNotIn("0x0040c000", rows)
        self.assertIn("ambiguous=1/2", out)
        self.assertIn("no_twin=1/2", out)
        self.assertIn("no_cand=1", out)
        self.assertIn("undecided=1", out)

    def test_conflict_dropped(self):
        # the same GOG fn won by two rows ships for neither
        with open(self.bpath, "w", encoding="utf-8") as fh:
            fh.write("steam\tbucket\tgog_addrs\n0x00402000\tambiguous\t0x00502000,0x00503000\n"
                     "0x00402000\tambiguous\t0x00502000\n")
        rc, out, _e = run(["graphpair", "--server", self.server, "--buckets", self.bpath, "--out", self.out_tsv]
                          + self.common)
        self.assertEqual(rc, 0)
        self.assertEqual(read_tsv(self.out_tsv), [])
        self.assertIn("conflict=2", out)


def write_strings(server, program, pairs):
    with open(os.path.join(server, program, "strings.tsv"), "w", encoding="utf-8") as fh:
        fh.write("addr\ttext\n" + "".join("%s\t%s\n" % p for p in pairs))


def vrow(addr, name, ns="", disp="0x10,0x20,0x30,0x40", size="100", strs=""):
    r = fp_row(addr, size, name, ns=ns, strs=strs)
    r["disp"] = disp
    return r


class MacnamesTests(Base):
    def setUp(self):
        super().setUp()
        self.server = os.path.join(self.tmp, "server")
        write_fp(self.server, inn.FP_GOG, [
            vrow("0x005ff000", "A", ns="Foo"), vrow("0x00600000", "FUN_00600000"),
            vrow("0x00600800", "Named", ns="Foo"), vrow("0x00601000", "B", ns="Foo"),
            vrow("0x00602000", "FUN_00602000", strs="0x700")])
        write_fp(self.server, inn.FP_MAC, [
            vrow("0x00001000", "Good", ns="Foo"), vrow("0x00002000", "Bad", ns="Foo", disp="0x500,0x600"),
            vrow("0x00003000", "Other", ns="Bar"), vrow("0x00004000", "Str", ns="Baz", strs="0x900")])
        write_strings(self.server, inn.FP_GOG, [("0x700", "hello")])
        write_strings(self.server, inn.FP_MAC, [("0x900", "hello")])
        self.buckets = os.path.join(self.tmp, "b.tsv")
        inn.write_tsv(self.buckets, [
            {"steam": "0x00400000", "bucket": "unnamed", "gog_addrs": "0x00600000", "steam_peers": "", "name": "",
             "cites": 1, "in_gog_functions": "no", "order_gog": ""},
            {"steam": "0x00400800", "bucket": "ambiguous", "gog_addrs": "0x00600800,0x00601000", "steam_peers": "",
             "name": "Foo::Named,Foo::B", "cites": 1, "in_gog_functions": "yes", "order_gog": "0x00600800"},
            {"steam": "0x00401000", "bucket": "named", "gog_addrs": "0x00601000", "steam_peers": "", "name": "Foo::B",
             "cites": 1, "in_gog_functions": "yes", "order_gog": ""}], inn.BUCKET_COLS)
        self.out_tsv = os.path.join(self.tmp, "mac.tsv")
        self.ex = os.path.join(self.tmp, "ex.tsv")
        self.args = ["macnames", "--server", self.server, "--out", self.out_tsv]

    def test_class_neighbours_rank_and_keep(self):
        rc, out, _e = run(self.args + ["--buckets", self.buckets, "--extract-out", self.ex])
        self.assertEqual(rc, 0)
        self.assertIn("macnames: targets=1 keep=1", out)  # order row with a named GOG fn left to B18
        (r,) = read_tsv(self.out_tsv)
        self.assertEqual((r["gog"], r["steam"], r["mac_name"], r["fit"], r["margin"], r["keep"]),
                         ("0x00600000", "0x00400000", "Foo::Good", "1.00", "0.75", "yes"))
        self.assertNotIn("Bar::Other", r["top3"])  # class not near the target, no shared string
        (e,) = read_tsv(self.ex)
        self.assertEqual((e["addr"], e["kind"], e["name"], e["status"]), ("0x00400000", "function", "Foo::Good", "new"))
        self.assertIn("via GOG 0x00600000", e["src"])

    def test_shared_string_adds_candidate_and_lowers_margin(self):
        rc, _o, _e = run(self.args + ["--gog", "0x00602000"])
        self.assertEqual(rc, 0)
        (r,) = read_tsv(self.out_tsv)
        self.assertIn("Baz::Str", r["top3"])
        self.assertEqual(r["margin"], "0.00")
        self.assertEqual(r["keep"], "no")
        self.assertEqual(r["steam"], "")

    def test_target_file_keeps_named_suspects(self):
        t = os.path.join(self.tmp, "sus.tsv")
        with open(t, "w", encoding="utf-8") as fh:
            fh.write("gog_addr\tgog_name\tsteam_counterpart\n0x00600800\tFoo::Named\t0x00400800\n")
        rc, out, _e = run(self.args + ["--targets", t])
        self.assertEqual(rc, 0)
        (r,) = read_tsv(self.out_tsv)
        self.assertEqual((r["gog_name"], r["steam"]), ("Foo::Named", "0x00400800"))


class LedgerTests(Base):
    def setUp(self):
        super().setUp()
        self.extract_path, self.rows = self.extract_rows()
        # 0x00500000 has no asm -> skipped; 0x00990010 is a global quoted from source
        self.dump = StubDump({0x00600000: ["PUSH EBP", "MOV EBP,ESP"], 0x00403000: ["RET"]})
        self.args = ["ledger", "--in", self.extract_path, "--topic", "ingest-modcode", "--date", DATE] + self.common
        self.target = os.path.join(self.ledger, "20261027_ingest-modcode.tsv")

    def snapshot(self):
        return {f: slurp(os.path.join(self.ledger, f)) for f in sorted(os.listdir(self.ledger))}

    def test_dry_run_writes_nothing(self):
        before = self.snapshot()
        rc, out, _e = run(self.args, self.dump)
        self.assertEqual(rc, 0)
        self.assertIn("dry-run: would-write=2 skipped=1 conflicts=1", out)
        self.assertEqual(self.snapshot(), before)
        self.assertFalse(os.path.exists(os.path.join(self.tmp, "conflicts.tsv")))

    def test_apply_rows_pass_lint(self):
        rc, out, _e = run(self.args + ["--apply"], self.dump)
        self.assertEqual(rc, 0, out)
        rows = [r for r in rl.read_rows(self.ledger) if r["file"] == "20261027_ingest-modcode.tsv"]
        self.assertEqual({(r["addr"], r["kind"], r["name"]) for r in rows},
                         {("0x00600000", "function", "Baz::Zed"), ("0x00990010", "global", "SomeGlobal")})
        for r in rows:
            self.assertEqual((r["confidence"], r["topic"], r["date"]), ("probable", "ingest-modcode", DATE))
            self.assertLessEqual(len(r["evidence"]), 200)
            self.assertIn("| src patch_manager_mods/foo_src/", r["evidence"])
        fn = [r for r in rows if r["kind"] == "function"][0]
        self.assertTrue(fn["evidence"].startswith("0x00600000: PUSH EBP | src "))
        gl = [r for r in rows if r["kind"] == "global"][0]
        self.assertIn("constexpr int SomeGlobal = 0x00990010;", gl["evidence"])
        self.assertEqual(rl.lint(self.ledger), [])

    def test_only_new_function_global(self):
        run(self.args + ["--apply"], self.dump)
        names = {r["name"] for r in rl.read_rows(self.ledger) if r["file"] == "20261027_ingest-modcode.tsv"}
        # agrees/conflicts (KDB or ledger), callsites and 'wrong'-overridden-but-no-asm rows never land
        for bad in ("Foo::Bar", "PlayThing", "kConflictFn", "Foo::Baz", "Foo::Other", "Baz::Inner", "Foo::Qux"):
            self.assertNotIn(bad, names)

    def test_kdb_conflicts_never_written(self):
        run(self.args + ["--apply"], self.dump)
        text = slurp(self.target, "r")
        self.assertNotIn("kConflictFn", text)
        self.assertNotIn("Foo::Real", text)
        conflicts = slurp(os.path.join(self.tmp, "conflicts.tsv"), "r")
        self.assertNotIn("kConflictFn", conflicts)

    def test_multi_name_most_frequent_wins(self):
        rc, _o, _e = run(self.args + ["--apply"], self.dump)
        self.assertEqual(rc, 0)
        conflicts = read_tsv(os.path.join(self.tmp, "conflicts.tsv"))
        self.assertEqual(len(conflicts), 1)
        c = conflicts[0]
        self.assertEqual((c["addr"], c["chosen_name"], c["other_name"]), ("0x00600000", "Baz::Zed", "ZedAlt"))
        self.assertTrue(c["src"].startswith("patch_manager_mods/foo_src/foo.h:"))
        with open(os.path.join(self.tmp, "conflicts.tsv"), encoding="utf-8") as fh:
            self.assertEqual(fh.readline().rstrip("\n").split("\t"), ["addr", "chosen_name", "other_name", "src"])

    def test_tie_goes_to_first_by_src(self):
        rows = [
            {"addr": "0x00500000", "kind": "function", "name": "B::Two", "form": "qualified",
             "src": "patch_manager_mods/foo_src/foo.cpp:1", "fn_start": "", "offset": "", "status": "new", "kdb_name": ""},
            {"addr": "0x00500000", "kind": "function", "name": "A::One", "form": "qualified",
             "src": "patch_manager_mods/foo_src/foo.cpp:2", "fn_start": "", "offset": "", "status": "new", "kdb_name": ""},
        ]
        out, conflicts, skipped = inn.build_ledger_rows(rows, StubDump({0x00500000: ["RET"]}), FIX, "t", DATE)
        self.assertEqual([r["name"] for r in out], ["B::Two"])
        self.assertEqual(conflicts[0]["other_name"], "A::One")
        self.assertEqual(skipped, [])

    def test_second_apply_refused(self):
        rc, _o, _e = run(self.args + ["--apply"], self.dump)
        self.assertEqual(rc, 0)
        before = slurp(self.target)
        rc, _o, err = run(self.args + ["--apply"], self.dump)
        self.assertEqual(rc, 2)
        self.assertIn("already exists", err)
        self.assertEqual(slurp(self.target), before)

    def test_evidence_truncated_and_anchored(self):
        long_insn = "MOV EAX," + "A" * 400
        rows = [{"addr": "0x00600000", "kind": "function", "name": "Baz::Zed", "form": "qualified",
                 "src": "patch_manager_mods/foo_src/foo.cpp:2", "fn_start": "", "offset": "", "status": "new",
                 "kdb_name": ""}]
        out, _c, _s = inn.build_ledger_rows(rows, StubDump({0x00600000: [long_insn]}), FIX, "t", DATE)
        self.assertEqual(len(out[0]["evidence"]), 200)
        self.assertTrue(out[0]["evidence"].startswith("0x00600000: MOV EAX,"))

    def test_bad_date_rejected(self):
        args = [a if a != DATE else "27/10/2026" for a in self.args]
        rc, _o, err = run(args, self.dump)
        self.assertEqual(rc, 2)
        self.assertIn("--date", err)


def _card_rows():
    rows = []
    for i in range(16):
        rows.append({"addr": "0x%08x" % (0x00410000 + i * 0x2000), "kind": "function",
                     "name": "Fn%02d" % i, "evidence": "0x0: PUSH EBP | src a.h:%d" % i})
    for i in range(12):
        rows.append({"addr": "0x%08x" % (0x009a0000 + i * 0x2000), "kind": "global",
                     "name": "Glob%02d" % i, "evidence": "0x0: x | src b.h:%d" % i})
    # a close neighbour pair that must never decoy each other
    rows.append({"addr": "0x009f42a4", "kind": "global", "name": "ScreenW", "evidence": "| src c.h:1"})
    rows.append({"addr": "0x009f42a8", "kind": "global", "name": "ScreenH", "evidence": "| src c.h:2"})
    return rows


def _facts(a):
    return ["- facts for 0x%08x" % a]


class CardsTest(unittest.TestCase):
    def build(self, seed=7):
        return inn.build_cards(_card_rows(), _facts, _facts, decoy_every=4, seed=seed)

    def test_decoy_ratio(self):
        cards, key = self.build()
        rows = _card_rows()
        self.assertEqual(sum(k["is_decoy"] for k in key), len(rows) // 4)
        self.assertEqual(len(cards), len(rows) + len(rows) // 4)
        self.assertEqual(sum(1 for k in key if not k["is_decoy"]), len(rows))

    def test_decoys_same_kind_far_and_wrong_name(self):
        _cards, key = self.build()
        by_addr = {r["addr"]: r for r in _card_rows()}
        for k in key:
            if not k["is_decoy"]:
                continue
            src = by_addr[k["facts_addr"]]
            self.assertEqual(src["kind"], k["kind"])
            self.assertNotEqual(src["name"], k["name"])
            self.assertGreaterEqual(abs(int(src["addr"], 16) - int(k["addr"], 16)), inn.DECOY_MIN_DIST)

    def test_card_shows_facts_of_facts_addr(self):
        cards, key = self.build()
        for c, k in zip(cards, key):
            self.assertEqual(c["addr"], k["facts_addr"])
            self.assertEqual(c["name"], k["name"])
            self.assertEqual(c["facts"], _facts(int(k["facts_addr"], 16)))

    def test_decoy_pool_names_come_from_pool_and_prefer_same_qualifier(self):
        rows = [{"addr": "0x%08x" % (0x00910000 + i * 0x2000), "kind": "function", "name": "Global::_crt%02d" % i,
                 "evidence": "src fptwin GOG 0x1"} for i in range(8)]
        pool = [{"addr": "0x%08x" % (0x00420000 + i * 0x2000), "kind": "function",
                 "name": ("Global::_pool%02d" if i < inn.DECOY_SAME_QUAL else "CFoo::M%02d") % i,
                 "evidence": "| src kdb name_map"} for i in range(inn.DECOY_SAME_QUAL + 10)]
        _cards, key = inn.build_cards(rows, _facts, _facts, decoy_every=1, seed=5, decoy_pool=pool)
        decoys = [k for k in key if k["is_decoy"]]
        self.assertEqual(len(decoys), len(rows))
        for k in decoys:
            self.assertTrue(k["name"].startswith("Global::_pool"), k["name"])
            self.assertEqual(k["src"], "kdb name_map")

    def test_kdb_decoy_pool_functions_only(self):
        with tempfile.TemporaryDirectory() as td:
            db = os.path.join(td, "k.db")
            con = sqlite3.connect(db)
            con.execute("create table name_map(steam_addr text, name text, confidence text)")
            con.executemany("insert into name_map values (?,?,?)", [
                ("0x00401000", "Foo::A", "high"), ("0x009a0000", "g_Glob", "high"), ("0x00402000", "Foo::W", "wrong")])
            con.commit()
            con.close()
            self.assertEqual([r["name"] for r in inn.kdb_decoy_pool(db)], ["Foo::A"])

    def test_deterministic_with_seed(self):
        self.assertEqual(self.build(3), self.build(3))
        self.assertNotEqual(self.build(3)[1], self.build(4)[1])

    def test_key_never_in_chunks(self):
        cards, key = self.build()
        with tempfile.TemporaryDirectory() as td:
            paths = inn.write_cards(cards, key, td, chunk=12)
            self.assertEqual(len(paths), -(-len(cards) // 12))
            text = "".join(open(p).read() for p in paths)
            self.assertNotIn("is_decoy", text)
            self.assertNotIn("decoy", text.lower())
            self.assertNotIn("src a.h", text)
            for k in key:
                self.assertIn("## %s\n" % k["card"], text)
            self.assertTrue(os.path.exists(os.path.join(td, "key.tsv")))


class ScoreTest(unittest.TestCase):
    KEY = [
        {"card": "C01", "addr": "0x1", "kind": "function", "name": "A", "is_decoy": "0", "facts_addr": "0x1",
         "src": "a.h:1"},
        {"card": "C02", "addr": "0x2", "kind": "global", "name": "B", "is_decoy": "0", "facts_addr": "0x2",
         "src": "b.h:1"},
        {"card": "C03", "addr": "0x2", "kind": "global", "name": "B", "is_decoy": "1", "facts_addr": "0x9",
         "src": "b.h:1"},
        {"card": "C04", "addr": "0x1", "kind": "function", "name": "A", "is_decoy": "1", "facts_addr": "0x8",
         "src": "a.h:1"},
    ]

    def test_pass_promotes_consistent_real_rows_only(self):
        v = {"C01": ("consistent", "q1"), "C02": ("unsure", ""), "C03": ("inconsistent", ""),
             "C04": ("inconsistent", "")}
        res = inn.score(self.KEY, v)
        self.assertTrue(res["pass"])
        self.assertEqual(res["per_kind"], {"function": (1, 1), "global": (1, 1)})
        self.assertEqual([p["name"] for p in res["promote"]], ["A"])
        self.assertEqual(res["promote"][0]["src"], "a.h:1")

    def test_fail_below_threshold_and_missing(self):
        res = inn.score(self.KEY, {"C01": ("consistent", ""), "C03": ("consistent", "")})
        self.assertFalse(res["pass"])
        self.assertEqual(res["rejected"], 0)
        self.assertEqual(res["missing"], ["C02", "C04"])

    def test_cli_score_writes_promote_only_on_pass(self):
        with tempfile.TemporaryDirectory() as td:
            kp = os.path.join(td, "key.tsv")
            inn.write_tsv(kp, self.KEY, inn.KEY_COLS)
            vp = os.path.join(td, "verdicts_1.tsv")
            with open(vp, "w") as fh:
                fh.write("card\tverdict\tquote\nC01\tconsistent\tq\nC03\tINCONSISTENT\tq\nC04\tbogus\tq\n")
            out = os.path.join(td, "promote.tsv")
            argv = ["score", "--key", kp, "--verdicts", os.path.join(td, "verdicts_*.tsv"), "--out", out]
            buf = io.StringIO()
            self.assertEqual(inn.main(argv, out=buf), 1)
            self.assertFalse(os.path.exists(out))
            self.assertIn("FAIL", buf.getvalue())
            with open(vp, "a") as fh:
                fh.write("C04\tinconsistent\tq\n")
            self.assertEqual(inn.main(argv, out=io.StringIO()), 0)
            self.assertEqual([r["name"] for r in inn.read_tsv(out)], ["A"])


class ListPairingTest(unittest.TestCase):
    def test_comma_list_does_not_pair_name_with_next_addr(self):
        line = "- callers 3: 0x0040ee40 CSWGuiPanel::StartLoad, 0x00411b20, 0x00415080 CSWGuiBorder::Load; 0x00416720"
        pairs = {(m.group(1), m.group(2)) for m in inn.ADDR_THEN_NAME_RX.finditer(line)}
        self.assertEqual(pairs, {("0x0040ee40", "CSWGuiPanel::StartLoad"), ("0x00415080", "CSWGuiBorder::Load")})
        self.assertEqual(list(inn.NAME_THEN_ADDR_RX.finditer(line)), [])
        self.assertEqual(inn.NAME_THEN_ADDR_RX.search("Foo::Bar at 0x00401000").group(2), "0x00401000")
        self.assertEqual(inn.NAME_THEN_ADDR_RX.search("`Foo::Bar` (0x00401000)").group(2), "0x00401000")

    def test_ingest_research_dir_skipped(self):
        with tempfile.TemporaryDirectory() as td:
            for rel in (".claude/research/ingest/i3/cards/chunk_1.md", ".claude/research/x/README.md"):
                os.makedirs(os.path.dirname(os.path.join(td, rel)), exist_ok=True)
                open(os.path.join(td, rel), "w").close()
            files = [f[len(td) + 1:] for f in inn.group_files(td, "research_notes")]
            self.assertEqual(files, [".claude/research/x/README.md"])


class DecompExcerptTest(unittest.TestCase):
    C = """// FN 00500000 FUN_00500000 size=10 ret=4
void __thiscall FUN_00500000(int param_1)

{
  undefined4 uVar1;
  int local_8;

  local_8 = 0;
  uVar1 = FUN_00600000(param_1);
  FUN_00700000();
  DAT_00a00000 = "hello";
  FUN_00500000(1);
  return;
}
"""
    NAMES = {0x00600000: "Foo::Bar", 0x00a00000: "gThing", 0x00500000: "Leak::Me"}

    def test_names_substituted_self_blind_decls_dropped(self):
        out = inn.decomp_excerpt(self.C, 0x00500000, self.NAMES, n=10)
        text = "\n".join(out)
        self.assertTrue(out[0].startswith("- decomp: void __thiscall this_fn(int param_1)"))
        self.assertIn("Foo::Bar(param_1)", text)
        self.assertIn('gThing = "hello"', text)
        self.assertIn("FUN_00700000()", text)
        self.assertNotIn("Leak::Me", text)
        self.assertNotIn("undefined4 uVar1;", text)

    def test_named_and_string_lines_first_source_order_kept(self):
        out = inn.decomp_excerpt(self.C, 0x00500000, self.NAMES, n=3)
        self.assertEqual(len(out), 3)
        self.assertIn("Foo::Bar", out[1])
        self.assertIn("gThing", out[2])

    def test_empty_text(self):
        self.assertEqual(inn.decomp_excerpt("", 0x1, {}), [])


if __name__ == "__main__":
    unittest.main()
