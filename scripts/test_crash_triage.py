#!/usr/bin/env python3
"""Unit tests for scripts/crash_triage.py."""
import io
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import crash_triage as CT


class TestCrashTriage(unittest.TestCase):
    def test_proc_stopped(self):
        with mock.patch.object(CT, "get_game_pids", return_value=[]):
            buf = io.StringIO()
            with mock.patch("sys.stdout", buf):
                rc = CT.main(["proc"])
            self.assertEqual(rc, 0)
            self.assertIn("VERDICT: STOPPED", buf.getvalue())

    def test_proc_running(self):
        with mock.patch.object(CT, "get_game_pids", return_value=[(1234, "S:\\swkotor2.exe")]), \
             mock.patch.object(CT, "get_proc_stats", return_value={"stat": "S", "pcpu": "1.5", "etime": "10:00"}):
            buf = io.StringIO()
            with mock.patch("sys.stdout", buf):
                rc = CT.main(["proc"])
            self.assertEqual(rc, 0)
            self.assertIn("VERDICT: RUNNING", buf.getvalue())

    def test_proc_hang(self):
        with mock.patch.object(CT, "get_game_pids", return_value=[(1234, "S:\\swkotor2.exe")]), \
             mock.patch.object(CT, "get_proc_stats", return_value={"stat": "R", "pcpu": "98.5", "etime": "01:00"}):
            buf = io.StringIO()
            with mock.patch("sys.stdout", buf):
                rc = CT.main(["proc"])
            self.assertEqual(rc, 0)
            self.assertIn("VERDICT: HANG", buf.getvalue())

    def test_log_clean(self):
        with tempfile.NamedTemporaryFile("w", suffix=".log", delete=False) as f:
            f.write("Normal game output\nInitializing audio\n")
            f_path = f.name
        try:
            buf = io.StringIO()
            with mock.patch("sys.stdout", buf):
                rc = CT.main(["log", "--log", f_path])
            self.assertEqual(rc, 0)
            self.assertIn("VERDICT: CLEAN", buf.getvalue())
        finally:
            os.remove(f_path)

    def test_log_fault(self):
        with tempfile.NamedTemporaryFile("w", suffix=".log", delete=False) as f:
            f.write("Unhandled exception: page fault on read access to 0x00000000 in 32-bit code (0x0053f000)\n")
            f.write("Backtrace: --- Exception 0xc0000005 at 0x0053f000 ---\n")
            f_path = f.name
        try:
            buf = io.StringIO()
            with mock.patch("sys.stdout", buf):
                rc = CT.main(["log", "--log", f_path])
            self.assertEqual(rc, 0)
            self.assertIn("VERDICT: FAULT(0x0053f000)", buf.getvalue())
        finally:
            os.remove(f_path)

    def test_kpm_pass(self):
        with tempfile.NamedTemporaryFile("w", suffix=".log", delete=False) as f:
            f.write("Applied DETOUR at 0x00401000\n")
            f.write("Applied DETOUR at 0x00402000\n")
            f.write("SUCCESS: Patcher initialized\n")
            f_path = f.name
        try:
            buf = io.StringIO()
            with mock.patch("sys.stdout", buf):
                rc = CT.main(["kpm", "--log", f_path])
            self.assertEqual(rc, 0)
            self.assertIn("VERDICT: PASS(detours=2)", buf.getvalue())
        finally:
            os.remove(f_path)

    def test_kill_not_running(self):
        with mock.patch.object(CT, "get_game_pids", return_value=[]):
            buf = io.StringIO()
            with mock.patch("sys.stdout", buf):
                rc = CT.main(["kill"])
            self.assertEqual(rc, 0)
            self.assertIn("VERDICT: NOT_RUNNING", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
