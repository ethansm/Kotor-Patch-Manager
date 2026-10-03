import hashlib
import io
import os
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ingest_server as ing  # noqa: E402

FN_HEADER = "addr\tsize\tretn\tname\tnamespace\tcallconv\tprototype\tsource\tthunk\n"
SYM_HEADER = "addr\tname\tnamespace\tsymbol_type\tsource\n"
TYPES_HEADER = "category\ttype_name\tkind\tsize\tordinal\tfield_offset\tfield_name\tfield_type\n"

STEAM_FUNCS = [
    # addr, size, retn, name, ns, cc, proto, source, thunk
    ("0x00521360", 10, 0, "Constructor", "CSWSArea", "__thiscall", "void Constructor(void)", "USER_DEFINED", "false"),
    ("0x00521400", 20, 4, "CSWSArea__Destroy", "", "__thiscall", "void f(void)", "USER_DEFINED", "false"),
    ("0x00521500", 30, 0, "CFoo", "CFoo", "__thiscall", "void f(void)", "USER_DEFINED", "false"),
    ("0x00521600", 40, 8, "Other", "CBar", "__thiscall", "void f(void)", "USER_DEFINED", "false"),
    ("0x00521700", 50, 0, "OnlyServer", "", "__cdecl", "void f(void)", "IMPORTED", "false"),
    ("0x00521800", 60, 0, "autoname", "", "__cdecl", "void f(void)", "ANALYSIS", "true"),
    ("0x00521900", 70, 0, "FUN_00521900", "", "__cdecl", "void f(void)", "DEFAULT", "false"),
    ("0x00521a00", 80, 0, "thunk_FUN_00521a00", "", "__cdecl", "void f(void)", "USER_DEFINED", "false"),
]
STEAM_SYMS = [
    ("0x00900000", "g_table", "", "Label", "USER_DEFINED"),
    ("0x00900004", "vftable", "CFoo", "Label", "IMPORTED"),
    ("0x00900008", "auto_label", "", "Label", "ANALYSIS"),
    ("0x0090000c", "DAT_0090000c", "", "Label", "DEFAULT"),
]
GOG_FUNCS = [
    ("0x00401000", 10, 0, "Alpha", "CGog", "__thiscall", "void f(void)", "USER_DEFINED", "false"),
    ("0x00401100", 10, 0, "Beta", "CGog", "__thiscall", "void f(void)", "USER_DEFINED", "false"),
    ("0x00401200", 10, 0, "Gamma", "CGog", "__thiscall", "void f(void)", "USER_DEFINED", "false"),
    ("0x00401300", 10, 0, "FUN_00401300", "", "__thiscall", "void f(void)", "DEFAULT", "false"),
]
TYPES = [
    ("/k2", "Foo", "struct", 8, 0, 0, "a", "int"),
    ("/k2", "Foo", "struct", 8, 1, 4, "b", "int"),
    ("/k2", "Color", "enum", 4, 0, 1, "RED", ""),
]


def write_tsv(path: Path, header: str, rows) -> None:
    path.write_text(header + "".join("\t".join(str(c) for c in r) + "\n" for r in rows))


class Fixture(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = Path(self._tmp.name)
        self.server = self.tmp / "server"
        steam = self.server / ing.STEAM_PROGRAM
        gog = self.server / ing.GOG_PROGRAM
        steam.mkdir(parents=True)
        gog.mkdir()
        # file prefix differs from the folder name on purpose
        write_tsv(steam / "swkotor2_aspyr.exe_functions.tsv", FN_HEADER, STEAM_FUNCS)
        write_tsv(steam / "swkotor2_aspyr.exe_symbols.tsv", SYM_HEADER, STEAM_SYMS)
        write_tsv(steam / "types.tsv", TYPES_HEADER, TYPES)
        write_tsv(gog / "swkotor2.exe_functions.tsv", FN_HEADER, GOG_FUNCS)
        write_tsv(gog / "swkotor2.exe_symbols.tsv", SYM_HEADER, [])

        self.kdb = self.tmp / "kdb.db"
        c = sqlite3.connect(self.kdb)
        c.execute("CREATE TABLE name_map(steam_addr text primary key,name text,source text,confidence text,"
                  "evidence text,date text)")
        c.executemany("INSERT INTO name_map VALUES(?,?,?,?,?,?)", [
            ("0x00521360", "CSWSArea::Constructor", "s", "high", "", ""),    # agree (server: Class::Constructor)
            ("0x00521400", "CSWSArea::Destroy", "s", "high", "", ""),        # agree (Class__Func)
            ("0x00521500", "CFoo::Constructor", "s", "medium", "", ""),      # agree (Class::Class)
            ("0x00521600", "CBar::Different", "s", "high", "", ""),          # conflict
            ("0x00900004", "CFoo::vftable", "s", "high", "", ""),            # symbol agree
            ("0x00900000", "other_name", "s", "low", "", ""),                # symbol conflict
        ])
        c.commit()
        c.close()

        self.branch = self.tmp / "branch.db"
        self._funcs_db(self.branch, [
            ("CSWSArea", "Constructor", 0x521360),
            ("CSWSArea", "Destroy", 0x521400),
            ("CBar", "Other", 0x521600),          # agree here, conflict in name_map
            ("CQ", "Q", 0x521700),                # conflict: server says OnlyServer
        ])
        self.gogdb = self.tmp / "gog.db"
        self._funcs_db(self.gogdb, [
            ("CGog", "Alpha", 0x401000),          # agree
            ("CGog", "Wrong", 0x401100),          # disagree
            ("CGog", "DbOnly", 0x401500),         # db-only, no server fn
            ("CGog", "DbOnly2", 0x401300),        # db-only, untrusted server fn
        ])                                        # 0x401200 Gamma: server-only
        self.env = mock.patch.dict(os.environ, {
            "BD_SERVER": str(self.server), "BD_KDB": str(self.kdb),
            "BD_BRANCHDB": str(self.branch), "BD_GOGDB": str(self.gogdb)})
        self.env.start()

    def tearDown(self):
        self.env.stop()
        self._tmp.cleanup()

    @staticmethod
    def _funcs_db(path, rows):
        c = sqlite3.connect(path)
        c.execute("CREATE TABLE functions(id INTEGER PRIMARY KEY AUTOINCREMENT, class_name TEXT NOT NULL, "
                  "function_name TEXT NOT NULL, address INTEGER NOT NULL, notes TEXT, calling_convention TEXT, "
                  "param_size_bytes INTEGER, UNIQUE(class_name, function_name))")
        c.executemany("INSERT INTO functions(class_name,function_name,address) VALUES(?,?,?)", rows)
        c.commit()
        c.close()

    def run_main(self, *argv):
        """Fixture expectations count every function row: default to --functions all unless a test says otherwise."""
        buf = io.StringIO()
        argv = list(argv) if "--functions" in argv else list(argv) + ["--functions", "all"]
        rc = ing.main(argv, out=buf)
        return rc, buf.getvalue()

    def test_trusted_only_mode_keeps_trusted_functions(self):
        rc, _ = self.run_main("--all", "--apply", "--functions", "trusted")
        self.assertEqual(rc, 0)
        self.assertEqual(self.scalar("SELECT count(*) FROM server_functions WHERE trust != 'trusted'"), 0)
        self.assertGreater(self.scalar("SELECT count(*) FROM server_functions"), 0)
        _, out = self.run_main("--all", "--apply", "--functions", "trusted")
        self.assertIn("idempotency: re-plan after apply = 0 changes -> PASS", out)

    def scalar(self, sql, *params):
        c = sqlite3.connect(self.kdb)
        try:
            return c.execute(sql, params).fetchone()[0]
        finally:
            c.close()


class ApplyTests(Fixture):
    def test_apply_creates_tables_and_counts(self):
        rc, out = self.run_main("--all", "--apply")
        self.assertEqual(rc, 0, out)
        steam, gog = ing.STEAM_PROGRAM, ing.GOG_PROGRAM
        self.assertEqual(self.scalar("SELECT count(*) FROM server_functions WHERE program=?", steam), 8)
        self.assertEqual(self.scalar("SELECT count(*) FROM server_functions WHERE program=?", gog), 4)
        self.assertEqual(self.scalar("SELECT count(*) FROM server_symbols WHERE program=?", steam), 2)
        self.assertEqual(self.scalar("SELECT count(*) FROM server_types WHERE program=?", steam), 3)
        self.assertEqual(self.scalar("SELECT count(*) FROM server_types WHERE program=?", gog), 0)
        self.assertEqual(self.scalar("SELECT thunk FROM server_functions WHERE addr='0x00521800'"), 1)
        self.assertEqual(self.scalar("SELECT retn FROM server_functions WHERE addr='0x00521400'"), 4)
        self.assertEqual(self.scalar("SELECT size FROM server_functions WHERE addr='0x00521400'"), 20)
        self.assertIn("integrity_check: ok", out)
        self.assertIn("PASS", out)
        self.assertNotIn("FAIL", out)

    def test_second_apply_changes_nothing(self):
        self.run_main("--all", "--apply")
        rc, out = self.run_main("--all", "--apply")
        self.assertEqual(rc, 0, out)
        self.assertIn("re-plan after apply = 0 changes", out)
        self.assertNotRegex(out, r"insert\s+[1-9]")
        self.assertNotRegex(out, r"update\s+[1-9]")
        self.assertIn("applied: server_functions 0, server_symbols 0, server_types 0", out)

    def test_changed_value_is_updated_once(self):
        self.run_main("--programs", ing.GOG_PROGRAM, "--apply")
        p = self.server / ing.GOG_PROGRAM / "swkotor2.exe_functions.tsv"
        p.write_text(p.read_text().replace("\tAlpha\t", "\tAlpha2\t"))
        rc, out = self.run_main("--programs", ing.GOG_PROGRAM, "--apply")
        self.assertIn("update      1", out)
        self.assertEqual(self.scalar("SELECT name FROM server_functions WHERE addr='0x00401000'"), "Alpha2")
        _, out2 = self.run_main("--programs", ing.GOG_PROGRAM, "--apply")
        self.assertIn("applied: server_functions 0", out2)

    def test_trust_mapping(self):
        self.run_main("--programs", ing.STEAM_PROGRAM, "--apply")
        got = dict(sqlite3.connect(self.kdb).execute("SELECT source, trust FROM server_functions").fetchall())
        self.assertEqual(got, {"USER_DEFINED": "trusted", "IMPORTED": "trusted", "ANALYSIS": "auto", "DEFAULT": "none"})

    def test_symbols_filtered_to_user_defined_and_imported(self):
        self.run_main("--programs", ing.STEAM_PROGRAM, "--apply")
        got = sorted(sqlite3.connect(self.kdb).execute("SELECT name, source FROM server_symbols").fetchall())
        self.assertEqual(got, [("g_table", "USER_DEFINED"), ("vftable", "IMPORTED")])

    def test_ddl_matches_design(self):
        self.run_main("--programs", ing.STEAM_PROGRAM, "--apply")
        c = sqlite3.connect(self.kdb)
        cols = [r[1] for r in c.execute("PRAGMA table_info(server_functions)")]
        self.assertEqual(cols, ["program", "addr", "name", "namespace", "callconv", "prototype", "source",
                                "trust", "size", "retn", "thunk", "export_date"])
        pk = [r[1] for r in c.execute("PRAGMA table_info(server_symbols)") if r[5]]
        self.assertEqual(pk, ["program", "addr", "name", "namespace"])

    def test_export_date_is_file_mtime(self):
        f = self.server / ing.GOG_PROGRAM / "swkotor2.exe_functions.tsv"
        os.utime(f, (1_700_000_000, 1_700_000_000))
        self.run_main("--programs", ing.GOG_PROGRAM, "--apply")
        self.assertEqual(self.scalar("SELECT DISTINCT export_date FROM server_functions"),
                         ing.dt.date.fromtimestamp(1_700_000_000).isoformat())


class DryRunTests(Fixture):
    def digest(self):
        return hashlib.sha256(self.kdb.read_bytes()).hexdigest(), self.kdb.stat().st_mtime_ns

    def test_dry_run_writes_nothing(self):
        before = self.digest()
        rc, out = self.run_main("--all")
        self.assertEqual(before, self.digest())
        self.assertIn("dry run", out)
        self.assertIn("insert      8", out)
        self.assertEqual(self.scalar("SELECT count(*) FROM sqlite_master WHERE name LIKE 'server_%'"), 0)
        self.assertEqual(rc, 0, out)

    def test_dry_run_with_report_writes_nothing(self):
        before = self.digest()
        self.run_main("--all", "--report", str(self.tmp / "r.md"))
        self.assertEqual(before, self.digest())
        self.assertTrue((self.tmp / "r.md").exists())

    def test_requires_selection(self):
        with self.assertRaises(SystemExit), mock.patch("sys.stderr", io.StringIO()):
            self.run_main()


class NormalizeTests(unittest.TestCase):
    def test_cases(self):
        n = ing.normalize
        self.assertEqual(n("CSWSArea__Destroy"), "CSWSArea::Destroy")
        self.assertEqual(n("CFoo::CFoo"), "CFoo::Constructor")
        self.assertEqual(n("CFoo::~CFoo"), "CFoo::Destructor")
        self.assertEqual(n("CExoArrayList<CExoString>::CExoArrayList<CExoString>"), "CExoArrayList<CExoString>::Constructor")
        self.assertEqual(n("thunk_CFoo::Bar"), "CFoo::Bar")
        self.assertEqual(n("FUN_00401000"), None)
        self.assertEqual(n("thunk_FUN_00401000"), None)
        self.assertEqual(n("FUN_MyThing"), "MyThing")
        self.assertEqual(n("operator new[]"), "operator new[]")
        self.assertEqual(n("A::B::C"), "A::B::C")
        self.assertEqual(n("cfoo::bar"), "cfoo::bar")
        self.assertNotEqual(n("CFoo::Bar"), n("CFoo::bar"))  # case-sensitive
        self.assertEqual(n("__security_check"), "__security_check")


class ReportTests(Fixture):
    def report(self):
        out = self.tmp / "report.md"
        _, stdout = self.run_main("--all", "--report", str(out))
        return out.read_text(), stdout

    def test_steam_classification(self):
        text, stdout = self.report()
        # trusted Steam funcs: 6 named + 1 thunk_FUN (default, skipped); 5 USER_DEFINED/IMPORTED real names
        # name_map: agree x3 (ctor, Class__Func, Class::Class), conflict x1, server-only x1 (OnlyServer)
        self.assertIn("steam_functions_name_map: {'agree': 3, 'conflict': 1, 'server-only': 1}", stdout)
        # branch: agree x3 (ctor, destroy, Other), conflict x1 (OnlyServer vs CQ::Q), server-only x1 (CFoo)
        self.assertIn("steam_functions_branch: {'agree': 3, 'conflict': 1, 'server-only': 1}", stdout)
        self.assertIn("steam_symbols_name_map: {'agree': 1, 'conflict': 1, 'server-only': 0}", stdout)
        self.assertIn("| 0x00521600 | CBar::Other | CBar::Different |", text)
        self.assertIn("| 0x00521700 | OnlyServer | CQ::Q |", text)

    def test_gog_comparison(self):
        text, stdout = self.report()
        self.assertIn("gog: {'agree': 1, 'disagree': 1, 'server-only': 1, 'db-only': 2}", stdout)
        self.assertIn("| 0x00401100 | CGog::Beta | CGog::Wrong |", text)
        self.assertIn("1 are server functions with an untrusted name", text)

    def test_per_program_counts(self):
        text, _ = self.report()
        self.assertIn(f"| {ing.STEAM_PROGRAM} | 8 |", text)
        self.assertIn("ANALYSIS 1, DEFAULT 1, IMPORTED 1, USER_DEFINED 5", text)


if __name__ == "__main__":
    unittest.main()
