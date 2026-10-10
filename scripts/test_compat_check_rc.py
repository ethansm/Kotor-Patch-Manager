#!/usr/bin/env python3
"""Pins compat_check.main() run_build_mods() exit-code handling.

Builds are faked: subprocess.run is patched in compat_check, so no real
build_check.py, run_tests.py, wine or network calls happen. All writes go to a
TemporaryDirectory (tempfile.tempdir is redirected into it).

Run: python3 scripts/test_compat_check_rc.py -v
"""
import json
import sys
import tempfile
import unittest
from pathlib import Path
from subprocess import CompletedProcess
from unittest import mock

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
sys.path.insert(0, str(HERE))
import compat_check  # noqa: E402

SHIELD_FILE = "patch_manager_mods/shield_bar_src/ShieldBar.cpp"


def _fake_run_factory(build_rc, build_stdout, out_dir):
    """Return a fake subprocess.run that answers build_check.py with the given rc/stdout."""

    def fake_run(cmd, *args, **kwargs):
        cmd = list(cmd)
        script = Path(cmd[1]).name if len(cmd) > 1 else ""
        if script == "build_check.py":
            return CompletedProcess(cmd, build_rc, stdout=build_stdout, stderr=None)
        # run_tests.py and anything else: succeed with no output; write empty results JSON if asked.
        if "--json" in cmd:
            json_path = Path(cmd[cmd.index("--json") + 1])
            json_path.write_text(json.dumps({"results": []}), encoding="utf-8")
        return CompletedProcess(cmd, 0, stdout="", stderr=None)

    return fake_run


class CompatCheckBuildRcTests(unittest.TestCase):
    def _run_main(self, build_rc, build_stdout):
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            json_out = tmp_path / "out.json"
            argv = [
                "compat_check.py",
                "--repo", str(REPO),
                "--json", str(json_out),
                SHIELD_FILE,
            ]
            fake = _fake_run_factory(build_rc, build_stdout, tmp_path)
            with mock.patch.object(sys, "argv", argv), \
                    mock.patch.object(tempfile, "tempdir", str(tmp_path)), \
                    mock.patch.object(compat_check.subprocess, "run", side_effect=fake):
                rc = compat_check.main()
            data = json.loads(json_out.read_text(encoding="utf-8"))
        return rc, data

    def test_rc1_without_fail_line_adds_build_check_error(self):
        rc, data = self._run_main(1, "building shield_bar_src...\nsome noise\n")
        errs = [r for r in data["results"] if r["id"] == "build_check"]
        self.assertEqual(len(errs), 1, data["results"])
        self.assertEqual(errs[0]["status"], "ERROR")
        self.assertEqual(errs[0]["kind"], "build")
        self.assertIn("exited 1", errs[0]["reason"])
        self.assertNotEqual(rc, 0)
        self.assertGreaterEqual(data["counts"]["ERROR"], 1)

    def test_rc1_with_fail_line_keeps_fail_and_no_error(self):
        out = "FAIL build: shield_bar_src: compile/link failed\n"
        rc, data = self._run_main(1, out)
        fails = [r for r in data["results"]
                 if r["id"] == "build_check:shield_bar_src" and r["status"] == "FAIL"]
        self.assertEqual(len(fails), 1, data["results"])
        self.assertFalse([r for r in data["results"] if r["id"] == "build_check"])
        self.assertNotEqual(rc, 0)

    @unittest.expectedFailure
    def test_known_gap_parenthesised_fail_line_parses(self):
        # FAIL regex in run_build_mods is r"FAIL\s+build:\s+([^\s:]+):\s*(.*)$" and does not
        # accept a "(Label)" group after the mod id, so this line falls through to ERROR.
        out = "FAIL build: shield_bar_src (ShieldBar): compile/link failed\n"
        _, data = self._run_main(1, out)
        self.assertTrue([r for r in data["results"]
                         if r["id"] == "build_check:shield_bar_src" and r["status"] == "FAIL"])

    def test_rc0_with_pass_line_no_error(self):
        out = "PASS build: shield_bar_src (ShieldBar): ok, 1.2s\n"
        rc, data = self._run_main(0, out)
        passes = [r for r in data["results"]
                  if r["id"] == "build_check:shield_bar_src" and r["status"] == "PASS"]
        self.assertEqual(len(passes), 1, data["results"])
        self.assertFalse([r for r in data["results"] if r["id"] == "build_check"])
        self.assertFalse([r for r in data["results"] if r["status"] == "ERROR"])
        self.assertEqual(data["counts"].get("ERROR", 0), 0)


if __name__ == "__main__":
    unittest.main()
