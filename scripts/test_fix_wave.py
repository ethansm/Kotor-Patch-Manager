#!/usr/bin/env python3
"""CLI smoke tests for fix_wave.py: every subcommand's --help, and `plan` on the FX3 fixture in a temp waves root."""
import contextlib
import csv
import io
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS))

import fix_wave  # noqa: E402

FIXTURE = SCRIPTS / "testdata" / "fix_wave" / "fx3_items.tsv"


def run(argv):
    out = io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
        rc = fix_wave.main(argv)
    return rc, out.getvalue()


class TestCli(unittest.TestCase):
    def test_help_each_subcommand(self):
        for cmd in fix_wave.COMMANDS:
            with self.subTest(cmd=cmd):
                rc, out = run([cmd, "--help"])
                self.assertEqual(rc, 0)
                self.assertIn("usage", out.lower())

    def test_top_level_help_and_unknown(self):
        self.assertEqual(run(["--help"])[0], 0)
        self.assertEqual(run([])[0], 2)
        self.assertEqual(run(["bogus"])[0], 2)

    def test_plan_fx3_fixture(self):
        with tempfile.TemporaryDirectory() as td:
            rc, out = run(["plan", str(FIXTURE), "--wave", "vwt", "--waves-root", td])
            self.assertEqual(rc, 0, out)
            rows = list(csv.DictReader(open(Path(td) / "vwt" / "batches.tsv", newline=""), delimiter="\t"))
            self.assertGreaterEqual(len(rows), 2)
            self.assertEqual(sum(int(r["units"]) for r in rows), 23)
            for r in rows:
                self.assertLessEqual(int(r["units"]), 10)
                self.assertLessEqual(int(r["agents"]), 60)
                self.assertLessEqual(float(r["test_usd"]), 15.0)
            self.assertEqual(rows[0]["wave_id"], "vwt-A")

    def test_plan_refuses_fx3_as_one_batch(self):
        with tempfile.TemporaryDirectory() as td:
            # FX3's real shape: 23 units in one batch must not fit under the default caps.
            rc, _ = run(["plan", str(FIXTURE), "--wave", "one", "--waves-root", td, "--max-units", "23"])
            rows = list(csv.DictReader(open(Path(td) / "one" / "batches.tsv", newline=""), delimiter="\t"))
            self.assertEqual(rc, 0)
            self.assertGreaterEqual(len(rows), 2, "cost/agent caps still split FX3")

    def test_script_entry_point(self):
        r = subprocess.run([sys.executable, str(SCRIPTS / "fix_wave.py"), "status", "--help"], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)


if __name__ == "__main__":
    unittest.main()
