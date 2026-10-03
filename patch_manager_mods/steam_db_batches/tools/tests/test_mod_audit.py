"""Tests for mod_audit.py (read-only mod-address audit).  Fixtures: tiny fp.tsv + KDB + branch DB under a temp dir,
redirected with BD_* environment variables.  Run from tools/:  python3 -m unittest tests.test_mod_audit
"""
import contextlib
import io
import os
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import bdlib as bd  # noqa: E402
import mod_audit as ma  # noqa: E402

FP_HEADER = "addr\tsize\tretn\tn_insn\tcallconv\tthunk_target\tcallees\tstr_refs\tdisp\timm\tname\tnamespace\tsource\n"
# start, size
FUNCS = [(0x00500000, 0x100), (0x00500200, 0x80), (0x00500300, 0x40), (0x00500400, 0x40), (0x00500500, 0x40),
         (0x00500600, 0x40)]

SRC = {
    "foo_src/Foo.cpp": "\n".join([
        '    {0x00500000, 256, "SpawnCreature", "spawn"},',                       # 1 string literal, validated start, agrees
        'constexpr uintptr_t kHookMid = 0x00500050;  // SpawnCreature mid hook',     # 2 comment wins as label, offset 0x50
        '    {0x00500200, 128, "TotallyDifferent", nullptr},',                       # 3 label mismatch
        '#define ADDR_NM 0x00500300',                                              # 4 #define identifier, name_map only
        'constexpr uintptr_t kDataPtr = 0x00A00010;',                              # 5 data (global_pointers)
        'constexpr uintptr_t kLonger = 0x5000000;  int x = 0x0050000000;',         # 6 longer hex literals -> skipped
        'auto p = 0x00500400;',                                                    # 7 name_map untrusted, no label
        'auto q = 0x00450000;',                                                    # 8 inside no function, not data -> unknown
        '    {0x00500500, 64, "SpawnCreature", nullptr},',                           # 9 label is another function's name
        '    {0x00500600, 8, "Pulse", nullptr},',                                    # 10 ambiguous bare name (3 others) -> differs
        ""]),
    "foo_src/build/Gen.cpp": "int x = 0x00500000;\n",                             # skipped dir
    "foo_src/sub/Bar.h": "const auto a = 0x520100; // six digit form\n",           # 6-digit form, in no function
    "foo_src/notes.txt": "0x00500000\n",                                           # wrong suffix
    "bar_src/hooks.toml": '[[hooks]]\naddress = 0x00500000\nfunction = "SpawnCreature_hook"\n',
    "bar_src/notes.toml": 'description = "patches 0x00500000 somewhere"\n[[hooks]]\naddress = 0x00500200\nfunction = "UpdateTail"\n',
    "baz_src/Comment.cpp": "\n".join([
        'foo(0x00500000);  // totally unrelated words here',                      # 1 comment-only at a validated start
        'int name = 5;  // see 0x00500200 for Update',                              # 2 address inside a comment
        'static const unsigned kCtor = 0x00500050;',                               # 3 mid-function ident
        ""]),
}


class AuditCase(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        for k in ("BD_SERVER", "BD_KDB", "BD_BRANCHDB", "BD_DUMP", "BD_EXE", "BD_ROOT"):
            self.addCleanup(self._restore, k, os.environ.get(k))
        sd = self.root / "server" / bd.STEAM_PROGRAM_DEFAULT
        sd.mkdir(parents=True)
        (sd / "fp.tsv").write_text(FP_HEADER + "".join(
            "0x%08x\t%d\t0\t5\t__stdcall\t\t\t\t\t\tFUN_%08x\t\tDEFAULT\n" % (a, n, a) for a, n in FUNCS))
        self.kdb = self.root / "kdb.db"
        con = sqlite3.connect(str(self.kdb))
        con.execute("CREATE TABLE steam_validated(steam_addr TEXT NOT NULL, name TEXT NOT NULL, verdict TEXT NOT NULL, "
                    "tier TEXT, gog_addr TEXT, batch TEXT NOT NULL, fp_sha TEXT, evidence_ref TEXT, date TEXT, "
                    "PRIMARY KEY(steam_addr, name))")
        con.execute("CREATE TABLE name_map(steam_addr text primary key,name text,source text,confidence text,"
                    "evidence text,date text)")
        for a, n in ((0x500000, "CFoo::SpawnCreature"), (0x500200, "CBar::Update"), (0x500500, "CQux::Rotate"),
                     (0x500600, "CZed::Zap")):
            con.execute("insert into steam_validated values(?,?,?,?,?,?,?,?,?)",
                        (bd.hx8(a), n, "verified", "opus", "", "B0", "", "", "2026-10-03"))
        con.execute("insert into name_map values('0x00500300','CBaz::Notify','src','high','','')")
        con.execute("insert into name_map values('0x00500400','CQux::Poke','src','wrong','','')")
        for i in range(3):                                    # "Pulse" names 3 other functions: too ambiguous
            con.execute("insert into name_map values(?,?,?,?,?,?)", ("0x0060%04x" % i, "CP%d::Pulse" % i, "src", "high", "", ""))
        con.commit()
        con.close()
        self.branch = self.root / "branch.db"
        con = sqlite3.connect(str(self.branch))
        con.execute("CREATE TABLE global_pointers(id INTEGER PRIMARY KEY AUTOINCREMENT, pointer_name TEXT NOT NULL UNIQUE, "
                    "address INTEGER NOT NULL, notes TEXT)")
        con.execute("insert into global_pointers(pointer_name,address) values('g_Thing', ?)", (0x00A00010,))
        con.commit()
        con.close()
        os.environ.update(BD_SERVER=str(self.root / "server"), BD_KDB=str(self.kdb), BD_BRANCHDB=str(self.branch),
                          BD_DUMP=str(self.root / "nodump"), BD_EXE=str(self.root / "no.exe"), BD_ROOT=str(self.root))
        self.src = self.root / "src"
        for rel, text in SRC.items():
            p = self.src / rel
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(text)
        self.out = self.root / "out"

    def _restore(self, k, v):
        if v is None:
            os.environ.pop(k, None)
        else:
            os.environ[k] = v

    def rows(self):
        ref = ma.Ref(bd.get_paths())
        return ma.audit(self.src, ref)

    def by(self, addr, mod="foo_src"):
        got = [r for r in self.rows() if r["addr"] == bd.hx8(addr) and r["mod"] == mod]
        self.assertTrue(got, "no row for %x" % addr)
        return got[0]


class ClassifyTests(AuditCase):
    def test_validated_agree_string_label(self):
        r = self.by(0x500000)
        self.assertEqual((r["status"], r["label"], r["label_src"], r["offset"], r["mid"]),
                         ("validated-agree", "SpawnCreature", "literal", "0", ""))
        self.assertEqual(r["validated_name"], "CFoo::SpawnCreature")

    def test_mid_function_hook_gets_no_verdict(self):
        r = self.by(0x500050)
        self.assertEqual((r["func_start"], r["offset"], r["mid"]), ("0x00500000", "80", "mid"))
        self.assertEqual((r["label"], r["label_src"]), ("kHookMid", "ident"))      # ident beats the trailing comment
        self.assertEqual(r["status"], "validated")                                  # a mid-function label is never judged
        r = self.by(0x500050, "baz_src")
        self.assertEqual((r["label"], r["status"]), ("kCtor", "validated"))

    def test_comment_only_label_gets_no_verdict(self):
        r = [x for x in self.rows() if x["file_line"].endswith("Comment.cpp:1")][0]
        self.assertEqual((r["label_src"], r["status"]), ("comment", "validated"))
        r = [x for x in self.rows() if x["file_line"].endswith("Comment.cpp:2")][0]      # `name = 5; // see 0x..`
        self.assertEqual((r["label_src"], r["label"]), ("comment", "see 0x00500200 for Update"))
        self.assertEqual(r["status"], "validated")                                  # never judged (and `name` is not used)

    def test_label_mismatch(self):
        r = self.by(0x500200)
        self.assertEqual(r["status"], "validated-label-differs")
        self.assertEqual((r["label"], r["validated_name"], r["conflict_with"]), ("TotallyDifferent", "CBar::Update", ""))

    def test_label_conflict_names_the_other_address(self):
        r = self.by(0x500500)
        self.assertEqual(r["status"], "validated-label-conflict")
        self.assertEqual(r["conflict_with"], "0x00500000 CFoo::SpawnCreature")
        self.assertEqual(r["validated_name"], "CQux::Rotate")

    def test_ambiguous_bare_name_is_only_differs(self):
        r = self.by(0x500600)
        self.assertEqual((r["label"], r["status"], r["conflict_with"]), ("Pulse", "validated-label-differs", ""))

    def test_name_map_trusted_with_define_label(self):
        r = self.by(0x500300)
        self.assertEqual((r["status"], r["label"], r["label_src"], r["kdb_name"]),
                         ("name_map-trusted", "ADDR_NM", "ident", "CBaz::Notify"))

    def test_name_map_untrusted_confidence_wrong(self):
        r = self.by(0x500400)
        self.assertEqual((r["status"], r["label"]), ("name_map-untrusted", "p"))   # `auto p = ...` -> identifier p

    def test_data_via_global_pointers(self):
        r = self.by(0x00A00010)
        self.assertEqual((r["status"], r["label"], r["kdb_name"], r["func_start"]), ("data", "kDataPtr", "g_Thing", ""))

    def test_unknown_below_functions(self):
        self.assertEqual(self.by(0x450000)["status"], "unknown")

    def test_past_last_function_is_data(self):
        self.assertEqual(self.by(0x520100)["status"], "data")        # 6-digit literal, beyond the last function

    def test_toml_block_name_label(self):
        r = self.by(0x500000, "bar_src")
        self.assertEqual((r["label"], r["label_src"], r["status"]), ("SpawnCreature_hook", "toml", "validated-agree"))

    def test_toml_non_name_key_is_not_a_label(self):
        r = [x for x in self.rows() if x["file_line"].endswith("notes.toml:1")][0]
        self.assertEqual((r["label_src"], r["label"], r["status"]), ("none", "(none)", "validated"))
        r = [x for x in self.rows() if x["file_line"].endswith("notes.toml:3")][0]
        self.assertEqual((r["label"], r["status"]), ("UpdateTail", "validated-agree"))     # word "update" is shared


class ScannerTests(AuditCase):
    def test_skips_longer_hex_literals(self):
        addrs = {r["addr"] for r in self.rows()}
        self.assertNotIn("0x01500000", addrs)
        self.assertNotIn("0x00500000" + "00", addrs)
        # 0x5000000 (7 digits) and 0x0050000000 (10 digits) contain 6/8-digit prefixes but must produce no hit
        self.assertFalse([r for r in self.rows() if r["file_line"].endswith("Foo.cpp:6")])

    def test_regex_forms(self):
        f = lambda s: [m.group(0) for m in ma.ADDR_RE.finditer(s)]        # noqa: E731
        self.assertEqual(f("a=0x00401000u, b=0x4010A0, c=0x00A1B480;"), ["0x00401000", "0x4010A0", "0x00A1B480"])
        self.assertEqual(f("0x0040100 0x004010000 0x1401000 0xA01000 0xdead00401000 0x00C01000"), [])
        self.assertEqual(f("x0x00401000 foo_0x00401000"), [])

    def test_skips_build_dirs_and_other_suffixes(self):
        files = {p.name for _m, p in ma.iter_source_files(self.src)}
        self.assertEqual(files, {"Foo.cpp", "Bar.h", "hooks.toml", "notes.toml", "Comment.cpp"})

    def test_dedupe_same_line(self):
        p = self.src / "foo_src" / "Dup.cpp"
        p.write_text('f(0x00500000, 0x00500000, "SpawnCreature");\n')
        got = [r for r in self.rows() if r["file_line"].endswith("Dup.cpp:1")]
        self.assertEqual(len(got), 1)


class LabelTests(unittest.TestCase):
    def lab(self, line, toml=False):
        return ma.line_labels(line, ma.ADDR_RE.search(line).start(), toml)[:2]

    def test_label_extraction(self):
        self.assertEqual(self.lab('{0x0066C7A0, 483, "LevelUpHenchman", "levelup-henchman"}'), ("LevelUpHenchman", "literal"))
        self.assertEqual(self.lab('log("addr %08x ok", 0x0066C7A0);'), ("", "none"))              # prose literal ignored
        self.assertEqual(self.lab("foo(0x0066C7A0); // patches the henchman path"), ("patches the henchman path", "comment"))
        self.assertEqual(self.lab("constexpr uintptr_t kFooBar = 0x0066C7A0;"), ("kFooBar", "ident"))
        self.assertEqual(self.lab("constexpr uintptr_t kFooBar = 0x0066C7A0; // note"), ("kFooBar", "ident"))
        self.assertEqual(self.lab("#define ADDR_X 0x0066C7A0"), ("ADDR_X", "ident"))
        self.assertEqual(self.lab("{0x0066C7A0, 12}"), ("", "none"))
        self.assertEqual(self.lab("int name = 1; // at 0x0066C7A0"), ("at 0x0066C7A0", "comment"))   # address inside a comment
        self.assertEqual(self.lab('x = "http://a" 0x0066C7A0'), ("x", "ident"))                     # // inside a string is no comment
        self.assertEqual(self.lab("x = 1  # note here 0x0066C7A0", toml=True), ("note here 0x0066C7A0", "comment"))
        self.assertEqual(self.lab('name = "Foo" # 0x0066C7A0 x', toml=True)[1], "comment")
        self.assertEqual(self.lab("function = 0x0066C7A0", toml=True), ("function", "toml"))
        self.assertEqual(self.lab("description = 0x0066C7A0", toml=True), ("", "none"))
        self.assertEqual(len(self.lab("f(0x0066C7A0); // " + "w" * 200)[0]), 60)

    def test_words_and_synonyms(self):
        self.assertEqual(ma.words("CSWSCreature::AIUpdate"), {"update"})                 # "AI" < 3 chars
        self.assertEqual(ma.words("kHookFnPtr_GetWorldTime"), {"get", "world", "time"})  # stop words dropped
        self.assertEqual(ma.words("FnExoCtor"), {"exo", "constructor"})
        self.assertEqual(ma.words("CExoString::CStrConstructor"), {"str", "constructor"})
        self.assertEqual(ma.words("Dtor"), {"destructor"})
        self.assertEqual(ma.words("InitFoo_66C95C"), {"initialize", "foo"})              # hex-looking token dropped
        self.assertEqual(ma.words("p"), set())

    def test_normalize_and_fits(self):
        self.assertEqual(ma.normalize("kFooBar"), "foobar")
        self.assertEqual(ma.normalize("g_Party"), "party")
        self.assertEqual(ma.normalize("ADDR_Thing"), "thing")
        self.assertEqual(ma.normalize("CSWSCreature::AIUpdate"), "aiupdate")
        self.assertEqual(ma.normalize("party"), "party")                     # no stripping of a plain lower-case p
        self.assertTrue(ma.fits("kExecuteCommand", "Global::ExecuteCommandAddMultiClass"))     # substring rule
        self.assertTrue(ma.fits("Foo::AddMultiClass", "ExecuteCommandAddMultiClass"))
        self.assertTrue(ma.fits("FnExoCtor", "CExoString::CStrConstructor"))                   # synonym
        self.assertTrue(ma.fits("DrawHook", "CSWGuiInGameInventory::Draw"))                    # shared word
        self.assertFalse(ma.fits("LevelUpHenchman", "Global::ExecuteCommandAddMultiClass"))
        self.assertFalse(ma.fits("InventoryTick", "CSWGuiInGameInventory::Draw"))              # class part is not compared
        self.assertFalse(ma.fits("p", "CFoo::Pulse"))                                          # short label never fits


class OutputTests(AuditCase):
    def run_cli(self, *extra):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = ma.main(["--src-root", str(self.src), "--out", str(self.out)] + list(extra))
        return rc, buf.getvalue()

    def test_cli_writes_tsv_report_and_summary(self):
        rc, out = self.run_cli()
        self.assertEqual(rc, 0)
        self.assertIn("validated-label-differs=2", out)             # TotallyDifferent + Pulse (ambiguous)
        self.assertIn("validated-label-conflict=1", out)
        rows = list(bd.iter_tsv(self.out / "mod_audit.tsv"))
        self.assertEqual(list(rows[0]), ma.TSV_COLS)
        self.assertEqual(len({(r["mod"], r["addr"], r["file_line"]) for r in rows}), len(rows))
        report = (self.out / "REPORT.md").read_text()
        self.assertIn("## validated-label-conflict (1)", report)
        self.assertLess(report.index("## validated-label-conflict"), report.index("## validated-label-differs"))
        self.assertIn("## validated-label-differs (2)", report)
        self.assertIn("0x00500000 CFoo::SpawnCreature", report)
        self.assertIn("TotallyDifferent", report)
        self.assertIn("| CBaz |", report)                                     # class table from name_map names
        self.assertLess(len(report.splitlines()), 200)

    def test_read_only_and_branch_db_override(self):
        before = {p: p.stat().st_mtime_ns for p in (self.kdb, self.branch)}
        other = self.root / "other_branch.db"
        con = sqlite3.connect(str(other))
        con.execute("CREATE TABLE global_pointers(id INTEGER PRIMARY KEY, pointer_name TEXT, address INTEGER, notes TEXT)")
        con.commit()
        con.close()
        self.run_cli("--branch-db", str(other))
        r = [r for r in bd.iter_tsv(self.out / "mod_audit.tsv") if r["addr"] == "0x00a00010"][0]
        self.assertEqual(r["kdb_name"], "")                                  # the empty override branch DB was used
        self.assertEqual(r["status"], "data")                                # still past the last function
        self.assertEqual({p: p.stat().st_mtime_ns for p in (self.kdb, self.branch)}, before)
        with self.assertRaises(sqlite3.OperationalError):
            bd.connect_ro(self.kdb).execute("create table t(x)")


if __name__ == "__main__":
    unittest.main()
