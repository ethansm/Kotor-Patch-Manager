#!/usr/bin/env python3
"""Tests for rea_facts.py. Live calls go to tests/fake_rea_mcp.py, never the real REA."""

import contextlib
import datetime as dt
import io
import json
import shlex
import shutil
import sqlite3
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

TOOLS_DIR = Path(__file__).resolve().parents[1]
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import rea_facts as rf  # noqa: E402
import rea_ledger as rl  # noqa: E402

FAKE = str(TOOLS_DIR / "tests" / "fake_rea_mcp.py")
FIXTURES = TOOLS_DIR / "tests" / "fixtures" / "rea"
DUMP_MINI = TOOLS_DIR / "tests" / "fixtures" / "dump_mini"
HASH = "6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef"
TOPIC = "B13a-s1"

# A: real analyze/assembly fixtures; two indirect sites (slot 0x14, slot 0x8); no static callers.
ADDR_A = 0x007680A0
# B: static callers (one call site at 0x7b31f4); contiguous body; no indirect sites.
ADDR_B = 0x00776DB0
# C: no sites, no callers, contiguous body; no xrefs.
ADDR_C = 0x00775930
ADDR_MISSING = 0x00123456  # no fixture: the fake answers with "unknown address"


def analyze_payload(addr, ranges, total, span, non_contiguous):
    return {
        "operation": "analyze_function",
        "parameters": {"procedure": "0x%08x" % addr},
        "normalized_result": {"procedure": {"address": "0x%x" % addr, "body": {
            "available": True,
            "ranges": [{"start": "0x%x" % s, "end": "0x%x" % e} for s, e in ranges],
            "total_bytes": total, "span_bytes": span, "non_contiguous": non_contiguous}}},
        "confidence": "observed",
    }


def xrefs_payload(addr, sources):
    return {"operation": "xrefs", "parameters": {"address": "0x%08x" % addr},
            "normalized_result": list(sources), "confidence": "observed"}


def make_kdb(path, rows):
    con = sqlite3.connect(path)
    con.execute("DROP TABLE IF EXISTS class_methods")
    con.execute("CREATE TABLE class_methods (binary_hash TEXT, class_name TEXT, vtable_index INTEGER, "
                "address TEXT, symbol TEXT)")
    con.executemany("INSERT INTO class_methods VALUES (?,?,?,?,?)", rows)
    con.commit()
    con.close()


def make_steam(path, rows):
    con = sqlite3.connect(path)
    con.execute("DROP TABLE IF EXISTS functions")
    con.execute("CREATE TABLE functions (id INTEGER PRIMARY KEY, class_name TEXT, function_name TEXT, "
                "address INTEGER)")
    con.executemany("INSERT INTO functions (class_name, function_name, address) VALUES (?,?,?)", rows)
    con.commit()
    con.close()


class FactsCase(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.ledger = self.root / "ledger"
        self.cache = self.root / "cache"
        self.lock = self.root / "lock"
        self.call_log = self.root / "calls.log"
        self.state = self.root / "state.json"
        self.exe = self.root / "swkotor2.exe"
        self.exe.write_bytes(b"not a real exe; the fake never reads it")
        self.fixtures = self.root / "fixtures"
        shutil.copytree(FIXTURES, self.fixtures)
        self.write_fixture("analyze_function", ADDR_B, analyze_payload(
            ADDR_B, [(ADDR_B, ADDR_B + 83)], 84, 84, False))
        self.write_fixture("xrefs", ADDR_B, xrefs_payload(ADDR_B, ["0x7b31f4", "0x407654", "0x407a7c"]))
        self.write_fixture("analyze_function", ADDR_C, analyze_payload(
            ADDR_C, [(ADDR_C, ADDR_C + 24)], 25, 25, False))
        self.write_fixture("xrefs", ADDR_C, xrefs_payload(ADDR_C, []))
        self.write_fixture("xrefs", ADDR_A, xrefs_payload(ADDR_A, ["0x407654", "0x407a7c"]))
        self.kdb = self.root / "kdb.db"
        make_kdb(self.kdb, [
            (HASH, "CSWFoo", 5, "00894110", "Foo"),
            (HASH, "CSWBar", 5, "0040e980", "Bar"),
        ])
        self.steam = self.root / "steam.db"
        make_steam(self.steam, [("CSWFoo", "Foo", ADDR_A)])
        self.addr_file = self.root / "addrs.txt"
        self.addr_file.write_text("0x007680a0\n0x00776db0\n0x00775930\n", encoding="utf-8")
        guard_patch = mock.patch.object(rf, "guard", lambda *a, **k: None)
        guard_patch.start()
        self.addCleanup(guard_patch.stop)

    def tearDown(self):
        self._tmp.cleanup()

    def _one(self, addr):
        path = self.root / ("one_%08x.txt" % addr)
        path.write_text("0x%08x\n" % addr, encoding="utf-8")
        return str(path)

    def write_fixture(self, tool, addr, payload):
        path = self.fixtures / ("%s__0x%08x.json" % (tool, addr))
        path.write_text(json.dumps(payload), encoding="utf-8")

    def cli(self, *argv, ledger=None, mode="", state=False, env=None):
        fake_env = {"FAKE_REA_CALL_LOG": str(self.call_log), "FAKE_REA_FIXTURES": str(self.fixtures)}
        if mode:
            fake_env["FAKE_REA_MODE"] = mode
        if state:
            fake_env["FAKE_REA_STATE"] = str(self.state)
        fake_env.update(env or {})
        overrides = {"exe": self.exe, "env": fake_env, "warm": None,
                     "cache_dir": self.cache, "lock_dir": self.lock}
        base = ["run", "--topic", TOPIC, "--ledger-dir", str(ledger or self.ledger),
                "--dump-root", str(DUMP_MINI), "--kdb", str(self.kdb), "--steam-db", str(self.steam),
                "--no-preflight", "--cmd", shlex.join([sys.executable, FAKE])]
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = rf.main(base + [str(x) for x in argv], session_overrides=overrides)
        return code, out.getvalue(), err.getvalue()

    def static_only(self, *argv, ledger=None):
        """Same as cli() but without a fake session."""
        base = ["run", "--topic", TOPIC, "--ledger-dir", str(ledger or self.ledger),
                "--dump-root", str(DUMP_MINI), "--kdb", str(self.kdb), "--steam-db", str(self.steam),
                "--no-live"]
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = rf.main(base + [str(x) for x in argv])
        return code, out.getvalue(), err.getvalue()

    def rows(self, ledger=None):
        return [{k: r[k] for k in rl.COLS} for r in rl.read_rows(ledger or self.ledger)]

    def by_addr(self, addr, rows=None):
        return [r for r in (rows if rows is not None else self.rows()) if r["addr"] == "0x%08x" % addr]

    def row_keys(self, ledger=None):
        return [tuple(r[k] for k in rl.COLS) for r in self.rows(ledger)]


class RowShapeTests(FactsCase):
    def test_rows_are_lint_clean(self):
        code, out, err = self.cli("--addrs", str(self.addr_file))
        self.assertEqual(code, 0, err)
        findings = [f for f in rl.lint(self.ledger) if "L8" not in str(f)]
        self.assertEqual(findings, [])

    def test_summary_line_counts_addresses_and_rows(self):
        code, out, _ = self.cli("--addrs", str(self.addr_file))
        self.assertEqual(code, 0)
        self.assertRegex(out, r"addresses 3, rows \d+, skipped 0 \(resume\)")
        self.assertEqual(len(self.rows()), int(out.split("rows ")[1].split(",")[0]))


class StaticCallsiteTests(FactsCase):
    def test_slot_0x14_callsite_is_confirmed_for_single_same_class_candidate(self):
        code, out, err = self.cli("--addrs", self._one(ADDR_A))
        self.assertEqual(code, 0, err)
        sites = [r for r in self.rows() if r["kind"] == "callsite"]
        by_addr = {r["addr"]: r for r in sites}
        site = by_addr["0x007680a6"]
        self.assertEqual(site["name"], "slot 0x14 candidates: CSWBar::Bar@0x0040e980, CSWFoo::Foo@0x00894110")
        self.assertEqual(site["confidence"], "confirmed")
        self.assertEqual(site["evidence"], "static_facts 0x007680a6: direct [eax + 0x14]")

    def test_unmatched_slot_is_probable_with_no_candidates(self):
        code, _, err = self.cli("--addrs", self._one(ADDR_A))
        self.assertEqual(code, 0, err)
        other = {r["addr"]: r for r in self.rows() if r["kind"] == "callsite"}["0x007680e1"]
        self.assertEqual(other["name"], "slot 0x8 candidates: none")
        self.assertEqual(other["confidence"], "probable")

    def test_two_same_class_candidates_are_probable(self):
        make_kdb(self.kdb, [
            (HASH, "CSWFoo", 5, "00894110", "Foo"),
            (HASH, "CSWFoo", 5, "0040e980", "Foo2"),
        ])
        code, _, err = self.cli("--addrs", self._one(ADDR_A))
        self.assertEqual(code, 0, err)
        site = {r["addr"]: r for r in self.rows() if r["kind"] == "callsite"}["0x007680a6"]
        self.assertEqual(site["confidence"], "probable")

    def test_max_sites_caps_callsite_rows_per_address(self):
        code, _, err = self.cli("--addrs", self._one(ADDR_A), "--max-sites", "1")
        self.assertEqual(code, 0, err)
        sites = [r for r in self.rows() if r["kind"] == "callsite"]
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]["addr"], "0x007680a6")

    def test_no_live_writes_static_rows_only_and_calls_no_rea(self):
        code, out, err = self.static_only("--addrs", self._one(ADDR_A))
        self.assertEqual(code, 0, err)
        self.assertFalse(self.call_log.exists())
        rows = self.rows()
        self.assertEqual({r["kind"] for r in rows}, {"callsite"})
        self.assertEqual(len(rows), 2)


class LiveFactTests(FactsCase):
    def test_body_row_lists_three_ranges_and_last_ret(self):
        code, _, err = self.cli("--addrs", self._one(ADDR_A))
        self.assertEqual(code, 0, err)
        body = [r for r in self.by_addr(ADDR_A) if r["kind"] == "function"]
        self.assertEqual(len(body), 1)
        self.assertEqual(body[0]["name"], "body ranges 0x7680a0-0x768296;0x768299-0x76829f;0x7682a2-0x7682a7 "
                                          "total 516 span 520")
        self.assertEqual(body[0]["evidence"], "analyze_function 0x007680a0: 0x007682a5: RET 0x8")
        self.assertEqual(body[0]["confidence"], "confirmed")

    def test_data_refs_exclude_static_code_callers(self):
        code, _, err = self.cli("--addrs", self._one(ADDR_B))
        self.assertEqual(code, 0, err)
        xref = [r for r in self.by_addr(ADDR_B) if r["kind"] == "xref"]
        self.assertEqual(len(xref), 1)
        self.assertEqual(xref[0]["name"], "data refs 2: 0x407654,0x407a7c (first 8)")
        self.assertEqual(xref[0]["evidence"], "xrefs 0x00776db0: 3 sources, 1 code call sites")
        self.assertEqual(xref[0]["confidence"], "confirmed")

    def test_contiguous_body_and_no_refs_writes_marker(self):
        code, _, err = self.cli("--addrs", self._one(ADDR_C))
        self.assertEqual(code, 0, err)
        rows = self.by_addr(ADDR_C)
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["name"], "no live-only facts")
        self.assertEqual(rows[0]["evidence"], "rea_facts 0x00775930: checked")
        self.assertEqual(rows[0]["confidence"], "probable")

    def test_per_address_rea_error_is_recorded_and_run_continues(self):
        path = self.root / "mixed.txt"
        path.write_text("0x%08x\n0x%08x\n" % (ADDR_MISSING, ADDR_C), encoding="utf-8")
        code, _, err = self.cli("--addrs", str(path))
        self.assertEqual(code, 0, err)
        missing = self.by_addr(ADDR_MISSING)
        self.assertEqual(len(missing), 1)
        self.assertTrue(missing[0]["name"].startswith("error: unknown address"), missing[0]["name"])
        self.assertEqual(missing[0]["evidence"], "rea_facts 0x00123456: body failed")
        self.assertEqual(len(self.by_addr(ADDR_C)), 1)


class CrashAndResumeTests(FactsCase):
    def test_crash_keeps_finished_addresses_and_rerun_completes_without_duplicates(self):
        # Address order is A (0x7680a0), C (0x775930), B (0x776db0). Answers: open(1) A.analyze(2)
        # A.procedure_assembly(3) A.xrefs(4) C.analyze(5); C.xrefs hits crash_after:5 and the reconnect
        # fails too. C is half done, so its rows must be discarded, not kept.
        code, out, err = self.cli("--addrs", self.addr_file, mode="crash_after:5", state=True)
        self.assertEqual(code, 1, err)
        self.assertIn("REA session lost", err)
        self.assertEqual({r["addr"] for r in self.rows()}, {"0x%08x" % ADDR_A, "0x007680a6", "0x007680e1"})

        code, out, err = self.cli("--addrs", self.addr_file)
        self.assertEqual(code, 0, err)
        self.assertRegex(out, r"addresses 3, rows 2, skipped 1 \(resume\)")
        keys = self.row_keys()
        self.assertEqual(len(keys), len(set(keys)))

        clean = self.root / "clean_ledger"
        code, _, err = self.cli("--addrs", self.addr_file, ledger=clean)
        self.assertEqual(code, 0, err)
        self.assertEqual(sorted(keys), sorted(self.row_keys(clean)))

    def test_rerun_on_finished_file_skips_everything(self):
        code, _, err = self.cli("--addrs", self.addr_file)
        self.assertEqual(code, 0, err)
        before = (self.ledger / "{}_{}.tsv".format(dt.date.today().strftime("%Y%m%d"), TOPIC)).read_bytes()
        code, out, err = self.cli("--addrs", self.addr_file)
        self.assertEqual(code, 0, err)
        self.assertRegex(out, r"addresses 3, rows 0, skipped 3 \(resume\)")
        after = (self.ledger / "{}_{}.tsv".format(dt.date.today().strftime("%Y%m%d"), TOPIC)).read_bytes()
        self.assertEqual(before, after)


class KeysAndTopicTests(FactsCase):
    def test_keys_tsv_reads_only_steam_addr_and_matches_addrs(self):
        keys = self.root / "keys.tsv"
        keys.write_text(
            "card_id\tgog\tsteam_addr\tkind\n"
            "c1\tg1\t0x007680a0\tfunction\n"
            "c2\tg2\t0x00776db0\tcallsite\n"
            "c3\tg3\t0x00775930\txref\n"
            "c4\tg4\t\tbogus\n",
            encoding="utf-8")
        ledger_keys = self.root / "ledger_keys"
        code, _, err = self.cli("--keys", str(keys), ledger=ledger_keys)
        self.assertEqual(code, 0, err)
        ledger_addrs = self.root / "ledger_addrs"
        code, _, err = self.cli("--addrs", self.addr_file, ledger=ledger_addrs)
        self.assertEqual(code, 0, err)
        self.assertEqual(self.row_keys(ledger_keys), self.row_keys(ledger_addrs))

    def test_topic_collision_in_other_file_is_refused(self):
        seed = {"addr": "0x00000001", "kind": "xref", "name": "seed", "confidence": "probable",
                "evidence": "seed 0x00000001: x", "topic": TOPIC, "date": "2020-01-01"}
        self.ledger.mkdir(parents=True, exist_ok=True)
        rl.append_rows(self.ledger, "20200101_%s.tsv" % TOPIC, [seed])
        code, _, err = self.cli("--addrs", self.addr_file)
        self.assertEqual(code, 2)
        self.assertIn("already used", err)
        self.assertEqual(len(self.rows()), 1)

    def test_bad_topic_exits_two(self):
        base = ["run", "--addrs", str(self.addr_file), "--topic", "B13a", "--ledger-dir", str(self.ledger),
                "--dump-root", str(DUMP_MINI), "--kdb", str(self.kdb), "--steam-db", str(self.steam),
                "--no-live"]
        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            code = rf.main(base)
        self.assertEqual(code, 2)
        self.assertIn("topic", err.getvalue())


class DeterminismTests(FactsCase):
    def test_two_fresh_runs_write_identical_files(self):
        dir_one = self.root / "ledger_one"
        dir_two = self.root / "ledger_two"
        self.assertEqual(self.cli("--addrs", self.addr_file, ledger=dir_one)[0], 0)
        self.assertEqual(self.cli("--addrs", self.addr_file, ledger=dir_two)[0], 0)
        name = "{}_{}.tsv".format(dt.date.today().strftime("%Y%m%d"), TOPIC)
        self.assertEqual((dir_one / name).read_bytes(), (dir_two / name).read_bytes())


class CliTests(FactsCase):
    def test_script_entry_point_runs_static_only(self):
        script = TOOLS_DIR / "rea_facts.py"
        proc = subprocess.run(
            [sys.executable, str(script), "run", "--addrs", str(self.addr_file), "--topic", TOPIC,
             "--ledger-dir", str(self.ledger), "--dump-root", str(DUMP_MINI), "--kdb", str(self.kdb),
             "--steam-db", str(self.steam), "--no-live"],
            capture_output=True, text=True, timeout=120)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertIn("addresses 3, rows", proc.stdout)


if __name__ == "__main__":
    unittest.main()
