#!/usr/bin/env python3
"""Tests for persist_purposes.py. Every test builds its own temp KDB; the real database is never opened."""

import contextlib
import io
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

import persist_purposes as pp  # noqa: E402

FULL = pp.BINARY_HASH
PREFIX = FULL[:8]
DATE = "2026-10-15"
TSV_NAME = "purposes.tsv"
MARKER = "persist_purposes purposes.tsv"

SCHEMA = """CREATE TABLE functions (
    binary_hash TEXT NOT NULL,
    address TEXT NOT NULL,
    symbol TEXT,
    size INTEGER,
    purpose TEXT,
    notes TEXT,
    discovered_date TEXT,
    PRIMARY KEY (binary_hash, address)
)"""

# (binary_hash, address, symbol, size, purpose, notes)
SEED = [
    (FULL, "00100000", "FUN_00100000", 10, None, None),  # NULL purpose: update
    (FULL, "00200000", "FUN_00200000", 20, "human text", None),  # human-owned: protected
    (FULL, "00300000", "FUN_00300000", 30, "old purpose", "persist_purposes old.tsv"),  # own row: update
    (FULL, "00600000", "FUN_00600000", 60, "stable", "persist_purposes old.tsv"),  # own row: same
    (FULL, "00700000", None, None, None, None),  # NULL purpose, no symbol/size: fill from TSV
    (FULL, "00800000", "FUN_00800000", 80, "", "auto-imported"),  # empty purpose: append marker
    (PREFIX, "006a4260", "PREFIX_ROW", 5, "prefix row", None),  # 8-char hash: never touched
    ("kotor2_steam_aspyr", "00300000", "STEAM", 1, "steam", None),  # other binary: never touched
]

TSV_TEXT = (
    "addr\tpurpose\tsymbol\tsize\tdecision\trationale\n"
    "00100000\tnull row purpose\tignored_symbol\t99\tkeep\tx\n"
    "0x00200000\treplacement for human\t\t\tkeep\tx\n"
    "00300000\tnew purpose\t\t\tfix\tx\n"
    "00400000\tdropped row\t\t\tdrop\tx\n"
    "0x006A4260\tfresh purpose\tFUN_006a4260\t42\tkeep\tx\n"
    "00500000\tfixed insert\t\t\tfix\tx\n"
    "00600000\tstable\t\t\tkeep\tx\n"
    "00700000\tfill test\tFUN_00700000\t77\tkeep\tx\n"
    "00800000\tappended purpose\t\t\tkeep\tx\n"
)


def make_db(path):
    con = sqlite3.connect(str(path))
    con.execute(SCHEMA)
    con.executemany(
        "INSERT INTO functions (binary_hash, address, symbol, size, purpose, notes) VALUES (?,?,?,?,?,?)",
        SEED,
    )
    con.commit()
    con.close()


def dump(path):
    con = sqlite3.connect(str(path))
    try:
        rows = con.execute(
            "SELECT binary_hash, address, symbol, size, purpose, notes, discovered_date "
            "FROM functions ORDER BY binary_hash, address"
        ).fetchall()
    finally:
        con.close()
    return {(r[0], r[1]): tuple(r) for r in rows}


def run_main(*args):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        code = pp.main(list(args))
    return code, out.getvalue(), err.getvalue()


class PersistPurposesTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name)
        self.db = self.dir / "kdb.db"
        make_db(self.db)
        self.tsv = self.dir / TSV_NAME
        self.tsv.write_text(TSV_TEXT, encoding="utf-8")

    def invoke(self, *extra):
        return run_main(str(self.tsv), "--kdb", str(self.db), "--date", DATE, *extra)

    def test_dry_run_writes_nothing(self):
        before_bytes = self.db.read_bytes()
        before_mtime = self.db.stat().st_mtime_ns
        before_rows = dump(self.db)
        code, out, err = self.invoke()
        self.assertEqual(code, 0, err)
        self.assertIn("insert 0x006a4260", out)
        self.assertIn("protected 0x00200000", out)
        self.assertIn("persist_purposes: 8 rows, insert 2, update 4, same 1, protected 1 (dry-run)", out)
        self.assertIn("integrity: ok", out)
        self.assertEqual(self.db.read_bytes(), before_bytes)
        self.assertEqual(self.db.stat().st_mtime_ns, before_mtime)
        self.assertEqual(dump(self.db), before_rows)

    def test_apply_inserts_updates_and_protects(self):
        code, out, err = self.invoke("--apply")
        self.assertEqual(code, 0, err)
        self.assertIn("persist_purposes: 8 rows, insert 2, update 4, same 1, protected 1 (applied)", out)
        self.assertIn("integrity: ok", out)
        db = dump(self.db)
        # NULL purpose: purpose filled, existing symbol and size kept, marker written.
        self.assertEqual(db[(FULL, "00100000")], (FULL, "00100000", "FUN_00100000", 10, "null row purpose", MARKER, None))
        # Human-owned row: untouched.
        self.assertEqual(db[(FULL, "00200000")], (FULL, "00200000", "FUN_00200000", 20, "human text", None, None))
        # Own row: purpose updated, notes left as they were.
        self.assertEqual(db[(FULL, "00300000")], (FULL, "00300000", "FUN_00300000", 30, "new purpose", "persist_purposes old.tsv", None))
        # Own row with identical purpose: untouched.
        self.assertEqual(db[(FULL, "00600000")], (FULL, "00600000", "FUN_00600000", 60, "stable", "persist_purposes old.tsv", None))
        # NULL symbol and size filled from the TSV.
        self.assertEqual(db[(FULL, "00700000")], (FULL, "00700000", "FUN_00700000", 77, "fill test", MARKER, None))
        # Empty purpose with other notes: marker appended.
        self.assertEqual(db[(FULL, "00800000")], (FULL, "00800000", "FUN_00800000", 80, "appended purpose", f"auto-imported ; {MARKER}", None))
        # Inserts carry the full binary hash, the marker and the --date value.
        self.assertEqual(db[(FULL, "006a4260")], (FULL, "006a4260", "FUN_006a4260", 42, "fresh purpose", MARKER, DATE))
        self.assertEqual(db[(FULL, "00500000")], (FULL, "00500000", None, None, "fixed insert", MARKER, DATE))
        # Other binaries and the 8-char hash row are untouched.
        self.assertEqual(db[(PREFIX, "006a4260")], (PREFIX, "006a4260", "PREFIX_ROW", 5, "prefix row", None, None))
        self.assertEqual(db[("kotor2_steam_aspyr", "00300000")], ("kotor2_steam_aspyr", "00300000", "STEAM", 1, "steam", None, None))

    def test_second_apply_changes_nothing(self):
        self.invoke("--apply")
        before = dump(self.db)
        code, out, err = self.invoke("--apply")
        self.assertEqual(code, 0, err)
        self.assertIn("persist_purposes: 8 rows, insert 0, update 0, same 7, protected 1 (applied)", out)
        self.assertEqual(dump(self.db), before)

    def test_decision_drop_rows_are_skipped(self):
        code, out, err = self.invoke("--apply")
        self.assertEqual(code, 0, err)
        self.assertNotIn("00400000", out)
        self.assertNotIn((FULL, "00400000"), dump(self.db))

    def test_addr_normalised_to_eight_lowercase_hex(self):
        self.invoke("--apply")
        db = dump(self.db)
        addresses = [addr for (_, addr) in db]
        self.assertTrue(all(a == a.lower() and not a.startswith("0x") and len(a) == 8 for a in addresses))
        self.assertIn((FULL, "006a4260"), db)

    def test_full_binary_hash_is_used_not_prefix(self):
        self.invoke("--apply")
        db = dump(self.db)
        self.assertEqual(db[(FULL, "006a4260")][4], "fresh purpose")
        self.assertEqual(db[(PREFIX, "006a4260")][4], "prefix row")
        self.assertFalse(any(bh not in (FULL, PREFIX, "kotor2_steam_aspyr") for (bh, _) in db))

    def test_duplicate_addr_exits_2_and_writes_nothing(self):
        self.tsv.write_text("addr\tpurpose\n00100000\ta\n0x00100000\tb\n", encoding="utf-8")
        before = self.db.read_bytes()
        code, out, err = self.invoke("--apply")
        self.assertEqual(code, 2)
        self.assertIn("duplicate", err)
        self.assertEqual(self.db.read_bytes(), before)

    def test_bad_input_exits_2(self):
        self.tsv.write_text("addr\tnotes\n00100000\tx\n", encoding="utf-8")
        code, _, err = self.invoke()
        self.assertEqual(code, 2)
        self.assertIn("missing column", err)
        self.tsv.write_text("addr\tpurpose\nzzzz\tx\n", encoding="utf-8")
        code, _, err = self.invoke()
        self.assertEqual(code, 2)
        self.assertIn("bad address", err)

    def test_missing_database_exits_2_without_creating_it(self):
        missing = self.dir / "nope.db"
        code, _, err = run_main(str(self.tsv), "--kdb", str(missing), "--apply")
        self.assertEqual(code, 2)
        self.assertIn("database not found", err)
        self.assertFalse(missing.exists())

    def test_source_override_sets_marker(self):
        code, _, err = self.invoke("--apply", "--source", "custom.tsv")
        self.assertEqual(code, 0, err)
        self.assertEqual(dump(self.db)[(FULL, "006a4260")][5], "persist_purposes custom.tsv")

    def test_integrity_failure_rolls_back(self):
        before = dump(self.db)
        with mock.patch.object(pp, "integrity_check", return_value=["*** row 3 missing from index"]):
            code, out, err = self.invoke("--apply")
        self.assertEqual(code, 1)
        self.assertIn("rolled back", out)
        self.assertIn("integrity: FAILED", out)
        self.assertEqual(dump(self.db), before)

    def test_dry_run_connection_is_read_only(self):
        def try_write(conn, rows, marker, date):
            conn.execute("UPDATE functions SET purpose='x'")
            return []
        with mock.patch.object(pp, "plan_actions", side_effect=try_write):
            with self.assertRaises(sqlite3.OperationalError):
                pp.run(self.db, [], False, MARKER, DATE)

    def test_exception_mid_apply_rolls_back(self):
        before = dump(self.db)
        real = pp.plan_actions

        def boom(conn, rows, marker, date):
            acts = real(conn, rows, marker, date)
            for a in acts:
                if a.sql:
                    conn.execute(a.sql, a.params)
            raise sqlite3.OperationalError("simulated")
        with mock.patch.object(pp, "plan_actions", side_effect=boom):
            code, _out, err = self.invoke("--apply")
        self.assertEqual(code, 1)
        self.assertIn("simulated", err)
        self.assertEqual(dump(self.db), before)

    def test_tsv_without_decision_column_uses_all_rows(self):
        self.tsv.write_text("addr\tpurpose\n00100000\tfirst\n00900000\tsecond\n", encoding="utf-8")
        code, _out, err = self.invoke("--apply")
        self.assertEqual(code, 0, err)
        db = dump(self.db)
        self.assertEqual(db[(FULL, "00100000")][4], "first")
        self.assertEqual(db[(FULL, "00900000")][4], "second")

    def test_bom_blank_purpose_and_blank_lines(self):
        self.tsv.write_text("\ufeffaddr\tpurpose\n\n00100000\t\n00900000\tfilled\n", encoding="utf-8")
        code, out, err = self.invoke("--apply")
        self.assertEqual(code, 0, err)
        self.assertIn("1 rows, insert 1", out)
        db = dump(self.db)
        self.assertIsNone(db[(FULL, "00100000")][4])
        self.assertEqual(db[(FULL, "00900000")][4], "filled")

    def test_whitespace_purpose_is_treated_as_empty(self):
        con = sqlite3.connect(str(self.db))
        con.execute("UPDATE functions SET purpose='  ' WHERE address='00200000'")
        con.commit()
        con.close()
        self.invoke("--apply")
        self.assertEqual(dump(self.db)[(FULL, "00200000")][4], "replacement for human")

    def test_decision_matching_is_case_insensitive_and_unknown_dropped(self):
        self.tsv.write_text("addr\tpurpose\tdecision\n00900000\ta\tKEEP\n00910000\tb\tmaybe\n00920000\tc\t\n",
                            encoding="utf-8")
        self.invoke("--apply")
        db = dump(self.db)
        self.assertIn((FULL, "00900000"), db)
        self.assertNotIn((FULL, "00910000"), db)
        self.assertNotIn((FULL, "00920000"), db)


if __name__ == "__main__":
    unittest.main()
