#!/usr/bin/env python3
"""Unit tests for persist_findings.py (run from this directory:  python3 -m unittest   or   python3 test_persist_findings.py).

persist_findings.py opens cwd-relative paths (ghidra_knowledge/, engine_research_findings/), so every test runs it as a
subprocess in a temp work dir holding a fixture knowledge DB (name_map 6 cols, functions 7 cols) and a fixture report.
Covers: only non-empty argv[2] tokens promote rows to high; a missing argv[2] prints usage instead of an IndexError.
"""
import os
import shutil
import sqlite3
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
PERSIST = os.path.join(HERE, "persist_findings.py")
REPORT = (
    "| addr | name | conf | purpose | evidence |\n"
    "| 7054ce | FnAlpha | medium | alpha purpose | verified at 7054ce |\n"
    "| 705559 | FnBeta | low | beta purpose | static only |\n"
)
EXPECTED = {"0x007054ce": "high", "0x00705559": "low"}  # only 7054ce verified; FnBeta keeps its own 'low'
UNVERIFIED = {"0x007054ce": "medium", "0x00705559": "low"}  # nothing verified: each row keeps its own confidence


class PersistFindings(unittest.TestCase):
    def setUp(self):
        self.work = tempfile.mkdtemp(prefix="persist_findings_")
        self.addCleanup(shutil.rmtree, self.work, True)
        os.makedirs(os.path.join(self.work, "ghidra_knowledge"))
        os.makedirs(os.path.join(self.work, "engine_research_findings"))
        self.db = os.path.join(self.work, "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
        c = sqlite3.connect(self.db)
        c.execute("CREATE TABLE name_map(steam_addr, name, source, confidence, evidence, date)")
        c.execute("CREATE TABLE functions(binary_hash, address, name, c4, c5, c6, c7)")
        c.commit()
        c.close()
        with open(os.path.join(self.work, "engine_research_findings", "U9.md"), "w") as f:
            f.write(REPORT)

    def persist(self, *args):
        return subprocess.run([sys.executable, PERSIST, *args], cwd=self.work, capture_output=True, text=True)

    def name_map(self):
        c = sqlite3.connect(self.db)
        try:
            return dict(c.execute("SELECT steam_addr, confidence FROM name_map").fetchall())
        finally:
            c.close()

    def test_verified_address_high_unverified_not(self):
        r = self.persist("U9", "7054ce")
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(self.name_map(), EXPECTED)

    def test_empty_argv2_promotes_nothing(self):
        # An empty token used to match every evidence string, so every row became high.
        r = self.persist("U9", "")
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(self.name_map(), UNVERIFIED)

    def test_trailing_comma_does_not_match_every_row(self):
        # A trailing comma used to add '' to ver, which made FnBeta (evidence 'static only') high.
        r = self.persist("U9", "7054ce,")
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(self.name_map(), EXPECTED)

    def test_missing_argv2_is_usage_error_not_traceback(self):
        r = self.persist("U9")
        self.assertNotEqual(r.returncode, 0)
        self.assertNotIn("Traceback", r.stderr)
        self.assertIn("usage", r.stderr)
        self.assertEqual(self.name_map(), {})


if __name__ == "__main__":
    unittest.main()
