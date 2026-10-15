#!/usr/bin/env python3
"""test_query_ghidra.py - Unit test suite for query_ghidra.py (GHI-01 to GHI-06)"""

import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

REPO_ROOT = Path(__file__).resolve().parent.parent
SCRIPT = REPO_ROOT / "scripts" / "query_ghidra.py"

class TestQueryGhidra(unittest.TestCase):
    def run_cli(self, *args):
        cmd = [sys.executable, str(SCRIPT)] + list(args)
        res = subprocess.run(cmd, cwd=str(REPO_ROOT), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return res

    def test_stats(self):
        """Verify database connection and stats output."""
        res = self.run_cli("stats", "--json")
        self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
        data = json.loads(res.stdout)
        self.assertIn("server_functions", data)
        self.assertGreater(data["server_functions"], 10000)

    def test_ghi_01_hex_normalization(self):
        """GHI-01: Hex address format normalization ('0x0053f000', '53f000', '0x53f000')."""
        variants = ["0x0053f000", "0x53f000", "53F000", "53f000"]
        for v in variants:
            res = self.run_cli("address", v, "--json")
            self.assertEqual(res.returncode, 0, f"Failed on variant {v}: {res.stderr}")
            data = json.loads(res.stdout)
            self.assertTrue(any("GetPortraitId" in str(item) for item in data), f"GetPortraitId not in {data}")

    def test_ghi_02_symbol_lookup(self):
        """GHI-02: Symbol and function lookup."""
        res = self.run_cli("symbol", "CSWSCreature", "--json", "--limit", "5")
        self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
        data = json.loads(res.stdout)
        self.assertIsInstance(data, list)
        self.assertGreaterEqual(len(data), 1)
        self.assertTrue(any("CSWSCreature" in item.get("name", "") for item in data))

    def test_ghi_03_non_existent_symbol(self):
        """GHI-03: Non-existent symbol returns exit code 1 and clean JSON."""
        res = self.run_cli("symbol", "NonExistentFakeFunction12345", "--json")
        self.assertEqual(res.returncode, 1)
        data = json.loads(res.stdout)
        self.assertEqual(data.get("status"), "not_found")
        self.assertEqual(data.get("matches"), [])

    def test_ghi_04_sql_injection_safe(self):
        """GHI-04: Parameterized queries resist malformed input."""
        res = self.run_cli("symbol", "' OR '1'='1", "--json")
        self.assertEqual(res.returncode, 1)
        self.assertNotIn("syntax error", res.stderr.lower())

    def test_ghi_05_struct_lookup(self):
        """GHI-05: Struct field layout inspection."""
        res = self.run_cli("struct", "CSWSCreature", "--json", "--limit", "10")
        self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
        data = json.loads(res.stdout)
        self.assertGreaterEqual(len(data), 1)
        self.assertIn("offset_hex", data[0])
        self.assertIn("offset", data[0])

class TestReaLedger(unittest.TestCase):
    """GHI-06: rea-recon findings ledger (patch_manager_mods/ghidra_knowledge/rea_findings/*.tsv) is searchable."""

    HEADER = "addr\tkind\tname\tconfidence\tevidence\ttopic\tdate\n"

    def setUp(self):
        import tempfile
        self.tmp = tempfile.TemporaryDirectory()
        d = Path(self.tmp.name)
        (d / "20261015_wi87.tsv").write_text(
            self.HEADER + "0x007510b0\tfunction\tCSWGuiLabel scalar deleting dtor\tprobable\t"
            "procedure_assembly 0x7510b0: vftable slot 0\tWI-87\t2026-10-15\n", encoding="utf-8")
        (d / "bad.tsv").write_text("not\ta\theader\n0x007510b0\tx\n", encoding="utf-8")   # wrong header: skipped
        self.dir = str(d)

    def tearDown(self):
        self.tmp.cleanup()

    def run_cli(self, *args, ledger_dir=None):
        cmd = [sys.executable, str(SCRIPT)] + list(args) + ["--ledger-dir", ledger_dir or self.dir]
        return subprocess.run(cmd, cwd=str(REPO_ROOT), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    def new_ledger(self, files):
        """Write {file name: text} into a fresh temp ledger dir (removed after the test); returns its path."""
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        for name, text in files.items():
            (Path(tmp.name) / name).write_text(text, encoding="utf-8")
        return tmp.name

    CONFIRMED_ROW = "0x007510b0\tfunction\tCSWGuiLabel scalar deleting dtor\tconfirmed\tprocedure_assembly 0x7510b0: vftable slot 0\tWI-87\t2026-10-15\n"
    REFUTED_ROW = "0x007510b0\tfunction\tCSWGuiLabel scalar deleting dtor\trefuted\tprocedure_assembly 0x7510b0: not a vftable\tWI-88\t2026-10-16\n"

    def supersede_files(self):
        """Same addr/kind/name: confirmed in an earlier file, refuted in a later one."""
        return {
            "20261015_wi87.tsv": self.HEADER + self.CONFIRMED_ROW,
            "20261016_wi88.tsv": self.HEADER + self.REFUTED_ROW,
        }

    def test_address_finds_ledger_row_in_any_hex_form(self):
        for v in ("0x007510b0", "7510B0"):
            res = self.run_cli("address", v, "--json")
            self.assertEqual(res.returncode, 0, res.stderr)
            rows = [r for r in json.loads(res.stdout) if r.get("source_table") == "rea_finding"]
            self.assertEqual([r["name"] for r in rows], ["CSWGuiLabel scalar deleting dtor"])
            self.assertEqual(rows[0]["file"], "20261015_wi87.tsv")

    def test_symbol_finds_ledger_row_and_text_output_labels_it(self):
        res = self.run_cli("symbol", "CSWGuiLabel scalar")
        self.assertEqual(res.returncode, 0, res.stderr)
        self.assertIn("[rea] 0x007510b0", res.stdout)
        self.assertIn("evidence: procedure_assembly", res.stdout)

    def test_stats_counts_only_well_formed_rows(self):
        res = self.run_cli("stats", "--json")
        self.assertEqual(json.loads(res.stdout)["rea_findings"], 1)

    def test_superseded_row_is_flagged_in_json_and_text(self):
        d = self.new_ledger(self.supersede_files())
        res = self.run_cli("address", "0x007510b0", "--json", ledger_dir=d)
        self.assertEqual(res.returncode, 0, res.stderr)
        rows = {r["confidence"]: r for r in json.loads(res.stdout) if r.get("source_table") == "rea_finding"}
        self.assertEqual(set(rows), {"confirmed", "refuted"})
        self.assertIs(rows["confirmed"]["superseded"], True)
        self.assertIs(rows["refuted"]["superseded"], False)

        text = self.run_cli("address", "0x007510b0", ledger_dir=d)
        self.assertEqual(text.returncode, 0, text.stderr)
        tagged = [ln for ln in text.stdout.splitlines() if "[superseded]" in ln]
        self.assertEqual(len(tagged), 1, text.stdout)
        self.assertIn("| confirmed |", tagged[0])

    def test_eight_cell_row_is_skipped_and_counted(self):
        d = self.new_ledger({"20261015_wi87.tsv": self.HEADER + self.CONFIRMED_ROW
                             + "0x007510b0\tfunction\tEIGHT CELL ROW\tprobable\tev\tWI-87\t2026-10-15\textra\n"})
        res = self.run_cli("address", "0x007510b0", "--json", ledger_dir=d)
        self.assertEqual(res.returncode, 0, res.stderr)
        names = [r["name"] for r in json.loads(res.stdout) if r.get("source_table") == "rea_finding"]
        self.assertEqual(names, ["CSWGuiLabel scalar deleting dtor"])

        stats = json.loads(self.run_cli("stats", "--json", ledger_dir=d).stdout)
        self.assertEqual(stats["rea_findings"], 1)
        self.assertEqual(stats["rea_findings_malformed"], 1)
        text = self.run_cli("stats", ledger_dir=d)
        self.assertIn("rea_findings_malformed", text.stdout)

    def test_stats_reports_issue_count_as_int(self):
        res = self.run_cli("stats", "--json")
        self.assertEqual(res.returncode, 0, res.stderr)
        data = json.loads(res.stdout)
        self.assertIsInstance(data["rea_findings_issues"], int)
        self.assertIsInstance(data["rea_findings_malformed"], int)

    def test_fallback_when_rea_ledger_cannot_load(self):
        """If tools/rea_ledger.py cannot be loaded, rows still come back, with superseded False."""
        spec = importlib.util.spec_from_file_location("query_ghidra_fallback", SCRIPT)
        qg = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(qg)
        d = Path(self.new_ledger(self.supersede_files()))
        with mock.patch.object(qg, "REA_LEDGER_TOOL", d / "missing_rea_ledger.py"):
            rows = qg.load_ledger(d)
            issues = qg.ledger_issue_count(d)
        self.assertEqual(sorted(r["confidence"] for r in rows), ["confirmed", "refuted"])
        self.assertTrue(all(r["superseded"] is False for r in rows))
        self.assertEqual(issues, 0)


if __name__ == "__main__":
    unittest.main()
