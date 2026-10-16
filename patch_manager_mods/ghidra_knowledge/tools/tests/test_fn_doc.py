"""Tests for fn_doc.py.  Run from tools/: python3 -m unittest tests.test_fn_doc"""
import contextlib
import io
import re
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
TOOLS = HERE.parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import fn_doc  # noqa: E402

FIXTURE = HERE / "fixtures" / "dump_mini"
GOLDEN = HERE / "fixtures" / "fn_doc" / "golden_page.md"
CLS = "FixtureStats"
MEMBERS = [(0x775930, "GetStat"), (0x776DB0, "Permov"), (0x7680A0, "Dispatch"), (0x778200, "Recurse")]
OTHER_HASH = "4ab72fc1ab082f427e008cda32fc5602d27b4e12fef48c4a1a2c6f7b2f36fb5a"
KDB_SCHEMA = """
CREATE TABLE name_map (steam_addr TEXT, name TEXT, source TEXT, confidence TEXT);
CREATE TABLE functions (binary_hash TEXT, address TEXT, symbol TEXT, size INTEGER,
                        purpose TEXT, notes TEXT, discovered_date TEXT);
CREATE TABLE struct_fields (class TEXT, offset INTEGER, size INTEGER, type TEXT, name TEXT,
                            confidence TEXT, evidence TEXT, readers TEXT, writers TEXT,
                            source TEXT, date TEXT);
"""
NAMED_GETSTAT = "// NAMED GetStat\nint GetStat(int this) {\n  return this->m_nStat;\n}\n"
NAMED_DISPATCH = "// NAMED Dispatch\nvoid Dispatch(int this) {\n  this->vtbl[8]();\n}\n"
LEDGER_TSV = (
    "addr\tkind\tname\tconfidence\tevidence\ttopic\tdate\n"
    "0x007680e1\tcallsite\tvtbl slot 8 dispatch\tconfirmed\tfixture\tfn-doc-q1\t2026-10-15\n"
    "0x00900000\tcallsite\tfar away call\tconfirmed\tfixture\tfn-doc-q1\t2026-10-15\n"
    "0x007680a0\tfunction\tDispatch entry\tconfirmed\tfixture\tfn-doc-q1\t2026-10-15\n"
)


class Case:
    """Temp Steam DB, KDB, named_root and ledger dir for one test."""

    def __init__(self, tmp):
        self.tmp = Path(tmp)
        self.steam = self.tmp / "steam.db"
        self.kdb = self.tmp / "kdb.db"
        self.named = self.tmp / "named"
        self.ledger = self.tmp / "ledger"
        self._write_steam()
        self._write_kdb()
        self._write_named()
        self.ledger.mkdir()
        (self.ledger / "ledger.tsv").write_text(LEDGER_TSV, encoding="utf-8")

    def _write_steam(self):
        con = sqlite3.connect(self.steam)
        try:
            con.execute("CREATE TABLE functions (class_name TEXT, function_name TEXT, address INTEGER)")
            rows = [(CLS, fn, addr) for addr, fn in MEMBERS]
            rows.append(("OtherClass", "Thing", 0x775950))
            con.executemany("INSERT INTO functions VALUES (?,?,?)", rows)
            con.commit()
        finally:
            con.close()

    def _write_kdb(self):
        con = sqlite3.connect(self.kdb)
        try:
            con.executescript(KDB_SCHEMA)
            con.executemany("INSERT INTO functions VALUES (?,?,?,?,?,?,?)", [
                (fn_doc.BINARY_HASH, "00775930", "FixtureStats::GetStat", 25,
                 "Reads the stat field and dispatches to the helper.", None, "2026-10-15"),
                (fn_doc.BINARY_HASH, "00776db0", "Permov", 84, None, None, "2026-10-15"),
                (fn_doc.BINARY_HASH, "007680a0", "Dispatch", 71,
                 "Dispatches through a vtable slot.", None, "2026-10-15"),
                (OTHER_HASH, "00778200", "Recurse", 15, "WRONG HASH PURPOSE", None, "2026-10-15"),
            ])
            con.executemany("INSERT INTO struct_fields VALUES (?,?,?,?,?,?,?,?,?,?,?)", [
                (CLS, 0x224, 4, "int32", "m_nStat", "high", "", "0x0077593a", None,
                 "test", "2026-10-15"),
                (CLS, 0x8, 4, "ptr", "m_pOwner", "high", "", "0x007680a3", "0x007680ac",
                 "test", "2026-10-15"),
                (CLS, 0x10, 4, "int32", "m_unused", "low", "", "0x00900000", None,
                 "test", "2026-10-15"),
            ])
            con.commit()
        finally:
            con.close()

    def _write_named(self):
        (self.named / "77").mkdir(parents=True)
        (self.named / "76").mkdir(parents=True)
        (self.named / "77" / "0x00775930.c").write_text(NAMED_GETSTAT, encoding="utf-8")
        (self.named / "76" / "0x007680a0.c").write_text(NAMED_DISPATCH, encoding="utf-8")

    def paths(self):
        return ["--root", str(FIXTURE), "--steam-db", str(self.steam), "--kdb", str(self.kdb),
                "--named-root", str(self.named), "--ledger-dir", str(self.ledger)]


def _run(argv):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = fn_doc._cli(argv)
    return rc, out.getvalue(), err.getvalue()


def _section(text, a):
    """The '## 0x........' section of one function, up to the next function heading."""
    i = text.index("## 0x%08x " % a)
    j = text.find("\n## 0x", i + 1)
    return text[i:] if j < 0 else text[i:j]


class _Base(unittest.TestCase):
    def setUp(self):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        self.case = Case(td.name)
        self.tmp = Path(td.name)

    def _page(self, out_name="out", extra=()):
        out = self.tmp / out_name
        rc, _o, _e = _run(["page", CLS, "--out", str(out)] + self.case.paths() + list(extra))
        self.assertEqual(rc, 0)
        return out


class PageTests(_Base):
    def test_page_is_deterministic(self):
        a = self._page("run_a") / (CLS + ".md")
        b = self._page("run_b") / (CLS + ".md")
        self.assertEqual(a.read_bytes(), b.read_bytes())

    def test_page_matches_golden(self):
        out = self._page()
        self.assertEqual((out / (CLS + ".md")).read_bytes(), GOLDEN.read_bytes())

    def test_page_content(self):
        text = (self._page() / (CLS + ".md")).read_text(encoding="utf-8")
        self.assertTrue(text.startswith("# FixtureStats\n\n4 functions in the Steam DB for FixtureStats."))
        self.assertEqual(re.findall(r"^## 0x[0-9a-f]{8} ", text, re.M),
                         ["## 0x007680a0 ", "## 0x00775930 ", "## 0x00776db0 ", "## 0x00778200 "])
        sec = _section(text, 0x775930)
        self.assertIn("- Size 25 B, RET 0", sec)
        self.assertIn("  - 0x00775940 CALL 0x0085d260", sec)
        self.assertIn("- KDB purpose: Reads the stat field and dispatches to the helper.", sec)

    def test_callers_strings_imports(self):
        text = (self._page() / (CLS + ".md")).read_text(encoding="utf-8")
        sec = _section(text, 0x776DB0)
        self.assertIn("- Callers (3 total):", sec)
        self.assertIn("  - 0x007b3050 (site 0x007b31f4)", sec)
        self.assertIn('  - 0x009868a0 "permov06"', sec)
        self.assertIn("- Imports: timeGetTime (WINMM.DLL)", sec)
        self.assertIn("  - 0x00776dc0 import", sec)

    def test_split_threshold_writes_numbered_parts(self):
        out = self._page("split", extra=["--split-bytes", "300"])
        parts = sorted(p.name for p in out.glob(CLS + ".*.md"))
        self.assertEqual(parts, [CLS + ".1.md", CLS + ".2.md", CLS + ".3.md", CLS + ".4.md"])
        self.assertFalse((out / (CLS + ".md")).exists())
        headings = 0
        for name in parts:
            text = (out / name).read_text(encoding="utf-8")
            self.assertTrue(text.startswith("# FixtureStats\n"))
            headings += len(re.findall(r"^## 0x", text, re.M))
        self.assertEqual(headings, 4)

    def test_default_run_removes_stale_parts(self):
        out = self._page("split", extra=["--split-bytes", "300"])
        self._page("split")  # default threshold: whole file again
        self.assertTrue((out / (CLS + ".md")).exists())
        self.assertEqual(sorted(p.name for p in out.glob(CLS + ".*.md")), [])

    def test_missing_named_decompile(self):
        text = (self._page() / (CLS + ".md")).read_text(encoding="utf-8")
        self.assertIn("(no named decompile)", _section(text, 0x776DB0))
        self.assertIn("  return this->m_nStat;", _section(text, 0x775930))

    def test_struct_fields_matched_by_reader_range(self):
        text = (self._page() / (CLS + ".md")).read_text(encoding="utf-8")
        self.assertIn("  - 0x224 FixtureStats::m_nStat (int32) R", _section(text, 0x775930))
        sec = _section(text, 0x7680A0)
        self.assertIn("  - 0x8 FixtureStats::m_pOwner (ptr) RW", sec)
        self.assertNotIn("m_nStat", _section(text, 0x776DB0))
        self.assertNotIn("m_unused", text)

    def test_indirect_slots_and_ledger_callsites(self):
        text = (self._page() / (CLS + ".md")).read_text(encoding="utf-8")
        sec = _section(text, 0x7680A0)
        self.assertIn("  - 0x007680a6 slot +0x14", sec)
        self.assertIn("  - 0x007680e1 slot +0x8", sec)
        self.assertIn("- Ledger callsites in range (1):\n  - 0x007680e1 vtbl slot 8 dispatch", sec)
        self.assertNotIn("far away call", text)
        self.assertNotIn("Dispatch entry", text)

    def test_purpose_none_and_hash_filter(self):
        text = (self._page() / (CLS + ".md")).read_text(encoding="utf-8")
        self.assertIn("- KDB purpose: (none)", _section(text, 0x776DB0))
        self.assertIn("- KDB purpose: (none)", _section(text, 0x778200))
        self.assertNotIn("WRONG HASH PURPOSE", text)

    def test_unknown_class_fails(self):
        rc, _o, err = _run(["page", "NoSuchClass", "--out", str(self.tmp / "x")]
                           + self.case.paths())
        self.assertEqual(rc, 1)
        self.assertIn("no functions for class NoSuchClass", err)


class DraftInputTests(_Base):
    def _draft(self, n, out_name="drafts"):
        out = self.tmp / out_name
        rc, _o, _e = _run(["draft-input", CLS, "--n", str(n), "--out", str(out)] + self.case.paths())
        self.assertEqual(rc, 0)
        return out / "_work"

    def test_chunks_by_n(self):
        work = self._draft(2)
        files = sorted(work.glob(CLS + "_*.md"))
        self.assertEqual(len(files), 2)  # ceil(4 / 2)
        first = files[0].read_text(encoding="utf-8")
        second = files[1].read_text(encoding="utf-8")
        self.assertIn("## 0x007680a0 ", first)
        self.assertIn("## 0x00775930 ", first)
        self.assertIn("## 0x00776db0 ", second)
        self.assertIn("## 0x00778200 ", second)
        self.assertIn("0x007680a0: PUSH EBP", first)
        self.assertIn("0x00778200: PUSH 0x009868a0", second)
        self.assertIn("addr<TAB>purpose<TAB>anchor", first)

    def test_stale_chunks_removed(self):
        self._draft(1)
        work = self._draft(4)
        self.assertEqual(sorted(p.name for p in work.glob(CLS + "_*.md")), [CLS + "_1.md"])


class CheckDraftsTests(_Base):
    def _check(self, rows):
        tsv = self.tmp / "drafts.tsv"
        tsv.write_text("\n".join(rows) + "\n", encoding="utf-8")
        out = self.tmp / "checked.tsv"
        rc, _o, err = _run(["check-drafts", str(tsv), "--out", str(out), "--root", str(FIXTURE)])
        self.assertEqual(rc, 0)
        results = [line.split("\t") for line in out.read_text(encoding="utf-8").splitlines()]
        return results, err

    def test_anchor_pass(self):
        res, err = self._check([
            "0x00775930\tReads the stat field then dispatches to the helper\t0x00775940: CALL 0x0085d260",
        ])
        self.assertEqual(res, [["0x00775930", "pass", "ok"]])
        self.assertIn("check-drafts: 1 rows, 1 pass, 0 fail", err)

    def test_anchor_wrong_text_fails(self):
        res, _err = self._check([
            "0x00775930\tReads the stat field then dispatches to the helper\t0x00775940: CALL 0x0085d999",
        ])
        self.assertEqual(res[0][1], "fail")
        self.assertIn("anchor text mismatch at 0x00775940", res[0][2])

    def test_normalisation_passes(self):
        res, _err = self._check([
            "0x0000776DB0\tCalls the imported timer through a computed pointer\t"
            "0x00776DC0: call DWORD PTR [0x00986484]",
        ])
        self.assertEqual(res, [["0x00776db0", "pass", "ok"]])

    def test_purpose_over_20_words_fails(self):
        purpose = " ".join(["word"] * 21)
        res, _err = self._check(["0x00775930\t%s\t0x00775940: CALL 0x0085d260" % purpose])
        self.assertEqual(res[0][1], "fail")
        self.assertIn("21 words", res[0][2])

    def test_address_only_purpose_fails(self):
        res, _err = self._check([
            "0x00775930\t0x00775940 FUN_00775930 LAB_0077593a\t0x00775940: CALL 0x0085d260",
        ])
        self.assertEqual(res[0][1], "fail")
        self.assertIn("address-only", res[0][2])

    def test_unknown_addr_fails(self):
        res, _err = self._check(["0x00DEAD00\tSome purpose words here\t0x00DEAD00: NOP"])
        self.assertEqual(res[0][0], "0x00dead00")
        self.assertEqual(res[0][1], "fail")
        self.assertIn("unknown addr", res[0][2])

    def test_anchor_shape_and_addr_not_in_asm(self):
        res, _err = self._check([
            "0x00775930\tReads the stat field then dispatches\t0x00775940 CALL 0x0085d260",
            "0x00775930\tReads the stat field then dispatches\t0x00775999: CALL 0x0085d260",
        ])
        self.assertIn("anchor not of form", res[0][2])
        self.assertIn("not in function asm", res[1][2])

    def test_bad_shape_header_and_blank_skipped(self):
        res, err = self._check([
            "addr\tpurpose\tanchor",
            "",
            "0x00775930\tonly two cells",
        ])
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0][1], "fail")
        self.assertIn("bad row shape", res[0][2])
        self.assertIn("check-drafts: 1 rows, 0 pass, 1 fail", err)


class DraftCapAndOrderTests(_Base):
    def test_draft_caps_long_lists(self):
        f = {"callees": [(0x1000 + i, 0x2000 + i, "CALL", None) for i in range(50)],
             "strings": [(0x3000 + i, "s%d" % i) for i in range(50)],
             "globals": [(0x4000 + i, "READ", "MOV EAX,[x]") for i in range(50)]}
        cap = fn_doc.DRAFT_LIST_CAP
        for lines in (fn_doc._callees_lines(f, cap), fn_doc._strings_lines(f, cap),
                      fn_doc._globals_lines(f, cap)):
            self.assertEqual(len(lines), 1 + cap + 1)
            self.assertIn("%d more" % (50 - cap), lines[-1])
        self.assertEqual(len(fn_doc._callees_lines(f)), 51)  # page lists stay uncapped

    def test_draft_asm_cap_notes_truncation(self):
        with mock.patch.object(fn_doc, "DRAFT_ASM_LINES", 3):
            out = self.tmp / "capped"
            rc, _o, _e = _run(["draft-input", CLS, "--n", "4", "--out", str(out)]
                              + self.case.paths())
        self.assertEqual(rc, 0)
        text = (out / "_work" / (CLS + "_1.md")).read_text(encoding="utf-8")
        self.assertRegex(text, r"- Asm \(\d+ lines; first 3 shown\):")

    def test_draft_missing_named_decompile_handled(self):
        text = (self.tmp / "d")
        _run(["draft-input", CLS, "--n", "4", "--out", str(text)] + self.case.paths())
        body = (text / "_work" / (CLS + "_1.md")).read_text(encoding="utf-8")
        self.assertIn("(no named decompile)", _section(body, 0x776DB0))

    def test_tied_members_are_ordered_by_name(self):
        con = sqlite3.connect(self.case.steam)
        con.execute("INSERT INTO functions VALUES (?,?,?)", (CLS, "AAlias", 0x775930))
        con.commit()
        con.close()
        members = fn_doc._class_members(str(self.case.steam), CLS)
        self.assertEqual(members[1], (0x775930, CLS + "::AAlias"))
        self.assertEqual(len(members), 4)


class CheckDraftsStrictnessTests(_Base):
    _check = CheckDraftsTests._check

    def test_anchor_from_another_function_fails(self):
        res, _err = self._check([
            "0x00775930\tReads the stat field then dispatches\t0x007680a0: PUSH EBP",
        ])
        self.assertEqual(res[0][1], "fail")
        self.assertIn("not in function asm", res[0][2])

    def test_different_instruction_same_operands_fails(self):
        res, _err = self._check([
            "0x00775930\tReads the stat field then dispatches\t0x00775940: JMP 0x0085d260",
        ])
        self.assertEqual(res[0][1], "fail")

    def test_duplicate_addr_fails(self):
        row = "0x00775930\tReads the stat field then dispatches\t0x00775940: CALL 0x0085d260"
        res, _err = self._check([row, row])
        self.assertEqual([r[1] for r in res], ["pass", "fail"])
        self.assertIn("duplicate", res[1][2])

    def test_only_exact_addr_header_is_skipped(self):
        res, _err = self._check([
            "addr\tpurpose\tanchor",
            "address_typo\tReads the stat field then dispatches\t0x00775940: CALL 0x0085d260",
        ])
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0][1], "fail")

    def test_twenty_words_pass_boundary(self):
        purpose = " ".join(["word"] * 20)
        res, _err = self._check(["0x00775930\t%s\t0x00775940: CALL 0x0085d260" % purpose])
        self.assertEqual(res[0][1], "pass")

    def test_empty_purpose_and_missing_anchor_cell_fail(self):
        res, _err = self._check([
            "0x00775930\t\t0x00775940: CALL 0x0085d260",
            "0x00775930\tReads the stat field then dispatches\t",
        ])
        self.assertIn("purpose empty", res[0][2])
        self.assertIn("anchor not of form", res[1][2])


class HelperTests(unittest.TestCase):
    def test_hex_and_word_helpers(self):
        self.assertEqual(fn_doc._norm_text("MOV dword ptr [0x0000ABCD],EAX"), "mov [0xabcd],eax")
        self.assertEqual(fn_doc._alpha_words("0x00775940 FUN_00775930 Calls it"), 2)
        self.assertIsNone(fn_doc._parse_addr("775940"))
        self.assertEqual(fn_doc._parse_addr("0x775940"), 0x775940)


if __name__ == "__main__":
    unittest.main()
