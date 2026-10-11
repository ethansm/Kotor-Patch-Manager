#!/usr/bin/env python3
"""
scripts/test_agent_diff_check.py - Unit tests for agent_diff_check.py test gate (BG02).

`run_tests.py -k` exits 0 when the filter matches nothing, so the gate must
require a summary line showing at least one passed test.
"""

from __future__ import annotations

import subprocess
import sys
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import agent_diff_check as adc  # noqa: E402

OK_SUMMARY = "\n3 passed, 0 failed, 0 error, 0 skipped (3 items) in 1.0s"
ZERO_SUMMARY = "\n0 passed, 0 failed, 0 error, 0 skipped (0 items) in 0.1s"
SKIP_SUMMARY = "\n0 passed, 0 failed, 0 error, 2 skipped (2 items) in 0.1s"
FAIL_SUMMARY = "\n1 passed, 1 failed, 0 error, 0 skipped (2 items) in 0.5s"


def _res(rc, out):
    return SimpleNamespace(returncode=rc, stdout=out, stderr="")


class TestsRanOk(unittest.TestCase):
    def test_passing_run(self):
        self.assertEqual(adc.tests_ran_ok(0, OK_SUMMARY), (True, ""))

    def test_zero_items(self):
        ok, reason = adc.tests_ran_ok(0, ZERO_SUMMARY)
        self.assertFalse(ok)
        self.assertIn("0 test items", reason)

    def test_all_skipped(self):
        ok, _ = adc.tests_ran_ok(0, SKIP_SUMMARY)
        self.assertFalse(ok)

    def test_nonzero_exit(self):
        ok, reason = adc.tests_ran_ok(1, FAIL_SUMMARY)
        self.assertFalse(ok)
        self.assertIn("exit code 1", reason)

    def test_no_summary(self):
        ok, reason = adc.tests_ran_ok(0, "something else")
        self.assertFalse(ok)
        self.assertIn("summary", reason)


class RunTests(unittest.TestCase):
    def test_no_filters(self):
        self.assertEqual(adc.run_tests([]), (True, "No specific tests requested."))

    def test_typo_filter_fails(self):
        with mock.patch.object(adc.subprocess, "run", return_value=_res(0, ZERO_SUMMARY)):
            ok, out = adc.run_tests(["zz"])
        self.assertFalse(ok)
        self.assertIn("FAIL -k zz", out)

    def test_good_filter_passes(self):
        with mock.patch.object(adc.subprocess, "run", return_value=_res(0, OK_SUMMARY)):
            ok, _ = adc.run_tests(["a"])
        self.assertTrue(ok)

    def test_one_bad_filter_fails_gate(self):
        with mock.patch.object(adc.subprocess, "run", side_effect=[_res(0, OK_SUMMARY), _res(0, ZERO_SUMMARY)]):
            ok, out = adc.run_tests(["a", "zz"])
        self.assertFalse(ok)
        self.assertIn("FAIL -k zz", out)
        self.assertNotIn("FAIL -k a:", out)

    def test_timeout_fails(self):
        with mock.patch.object(adc.subprocess, "run", side_effect=subprocess.TimeoutExpired("x", 1)):
            ok, out = adc.run_tests(["a"])
        self.assertFalse(ok)
        self.assertIn("timed out", out)


if __name__ == "__main__":
    unittest.main()
