#!/usr/bin/env python3
"""test_query_ghidra.py - Unit test suite for query_ghidra.py (GHI-01 to GHI-06)"""

import json
import subprocess
import sys
import unittest
from pathlib import Path

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

if __name__ == "__main__":
    unittest.main()
