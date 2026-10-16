#!/usr/bin/env python3
"""Tests for scripts/gen_hse_layout.py. Stdlib only. Reads the committed HseViewGui.inc; writes only a temp copy."""
import contextlib
import io
import re
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import gen_hse_layout as hge  # noqa: E402


def _parse_inc(text):
    return bytes(int(h, 16) for h in re.findall(r"0x([0-9a-f]{2})", text))


class GenHseLayoutTests(unittest.TestCase):
    def setUp(self):
        self.blob = hge.build_gui_bytes()

    def test_gui_signature_and_v32(self):
        self.assertTrue(self.blob.startswith(b"GUI "))
        self.assertIn(b"V3.2", self.blob)
        self.assertEqual(self.blob[4:8], b"V3.2")

    def test_hse_view_tag_appears_once(self):
        self.assertEqual(self.blob.count(b"HSE_VIEW"), 1)

    def test_two_runs_are_byte_identical(self):
        self.assertEqual(hge.build_gui_bytes(), hge.build_gui_bytes())

    def test_committed_inc_matches_generator(self):
        text = hge.INC_PATH.read_text(encoding="ascii")
        self.assertEqual(_parse_inc(text), self.blob)

    def test_check_exits_0_on_committed_inc(self):
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(hge.main(["--check"]), 0)

    def test_check_exits_1_on_one_byte_mutated_copy(self):
        mutated = bytearray(self.blob)
        mutated[10] ^= 0x01
        with tempfile.TemporaryDirectory() as tmp:
            copy = Path(tmp) / "HseViewGui.inc"
            copy.write_bytes(hge.render_inc(bytes(mutated)).encode("ascii"))
            err = io.StringIO()
            with contextlib.redirect_stderr(err):
                rc = hge.main(["--check", "--inc", str(copy)])
        self.assertEqual(rc, 1)
        self.assertIn("stale", err.getvalue())

    def test_bounds_guard_fires_on_forged_oversized_count(self):
        with self.assertRaises(AssertionError):
            hge.build_gui_bytes(list_count=10 ** 6)

    def test_bounds_guard_accepts_real_count(self):
        self.assertEqual(hge.build_gui_bytes(list_count=2), self.blob)


if __name__ == "__main__":
    unittest.main()
