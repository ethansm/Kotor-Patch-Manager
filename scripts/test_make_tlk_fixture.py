#!/usr/bin/env python3
"""Tests for action_hotbar_src/testdata/make_tlk_fixture.py: the committed fixture.tlk is what the generator builds (--check)."""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
TESTDATA = HERE.parent / "patch_manager_mods/action_hotbar_src/testdata"
sys.path.insert(0, str(TESTDATA))
import make_tlk_fixture as mk  # noqa: E402


class MakeTlkFixture(unittest.TestCase):
    def test_check_cli_passes_on_committed_fixture(self):
        r = subprocess.run([sys.executable, str(TESTDATA / "make_tlk_fixture.py"), "--check"], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)

    def test_check_detects_stale_fixture(self):
        data = (TESTDATA / "fixture.tlk").read_bytes()
        with tempfile.TemporaryDirectory() as td:
            p = Path(td) / "fixture.tlk"
            p.write_bytes(data)
            self.assertTrue(mk.check(p))
            p.write_bytes(data[:-1] + bytes([data[-1] ^ 1]))
            self.assertFalse(mk.check(p))
            p.write_bytes(data + b"x")
            self.assertFalse(mk.check(p))
            self.assertFalse(mk.check(Path(td) / "missing.tlk"))

    def test_check_does_not_touch_fixture(self):
        before = (TESTDATA / "fixture.tlk").stat().st_mtime_ns
        mk.check()
        self.assertEqual((TESTDATA / "fixture.tlk").stat().st_mtime_ns, before)


if __name__ == "__main__":
    unittest.main()
