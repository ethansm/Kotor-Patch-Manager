#!/usr/bin/env python3
"""Tests for run_tests.py canonical wine-smoke includes (doc 24 H8 / doc 36)."""

import os
import sys
import tempfile
import tomllib
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import run_tests as rt  # noqa: E402

SHARED = rt.REPO / "patch_manager_mods" / "_shared_src"


def flags_with(common):
    with mock.patch.dict(os.environ, {"KPM_COMMON_DIR": str(common)}):
        return rt.canonical_include_flags()


class CanonicalIncludeTests(unittest.TestCase):
    def test_canonical_flags_order_and_env_override(self):
        with tempfile.TemporaryDirectory() as d:
            self.assertEqual(flags_with(d), [f"-I{SHARED}", f"-I{d}"])

    def test_missing_common_dir_skipped(self):
        with tempfile.TemporaryDirectory() as d:
            self.assertEqual(flags_with(Path(d) / "nope"), [f"-I{SHARED}"])

    def test_smoke_build_cmd_has_canonical_includes_before_cxxflags(self):
        seen = {}

        def fake_run(cmd, cwd, env=None, timeout=300):
            seen["cmd"] = cmd
            return ("FAIL", "x", "")
        with tempfile.TemporaryDirectory() as d, \
                mock.patch.dict(os.environ, {"KPM_COMMON_DIR": d}), \
                mock.patch.object(rt, "run_cmd", fake_run), \
                mock.patch.object(rt, "TMP", Path(d) / "tmp"):
            rt.smoke_build({"name": "t", "src": "x.cpp", "cxxflags": ["-static"]})
            cmd = seen["cmd"]
            self.assertIn(f"-I{SHARED}", cmd)
            self.assertIn(f"-I{d}", cmd)
            self.assertLess(cmd.index(f"-I{SHARED}"), cmd.index(f"-I{d}"))
            self.assertLess(cmd.index(f"-I{d}"), cmd.index("-static"))
            self.assertFalse([a for a in cmd if "Patches/Common" in a])

    def test_scratch_kpm_dir_does_not_change_includes(self):
        with tempfile.TemporaryDirectory() as common, tempfile.TemporaryDirectory() as scratch:
            want = flags_with(common)
            with mock.patch.object(rt, "KPM_DIR", Path(scratch)):
                self.assertEqual(flags_with(common), want)

    def test_smokes_toml_has_no_hardcoded_include_dirs(self):
        data = tomllib.loads(rt.SMOKES_TOML.read_text())
        bad = [f for s in data["smoke"] for f in s.get("cxxflags", [])
               if f.startswith("-I") and any(k in f for k in ("$HOME", "$KPM_DIR", "_shared_src"))]
        self.assertEqual(bad, [])


if __name__ == "__main__":
    unittest.main()
