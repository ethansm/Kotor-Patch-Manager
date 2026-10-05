#!/usr/bin/env python3
"""test_check_shared_headers.py - Unit test suite for check_shared_headers.py"""

import io
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import check_shared_headers as csh  # noqa: E402


class TestCheckSharedHeaders(unittest.TestCase):
    def test_forwarder_content(self):
        text = csh.forwarder("SharedTypes.h")
        self.assertIn("#pragma once", text)
        self.assertIn('#include "../Common/SharedTypes.h"', text)

    def test_missing_common_dir_returns_2(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            fake_kpm = Path(tmpdir) / "kpm"
            fake_canonical = Path(tmpdir) / "canonical"
            fake_canonical.mkdir()

            with patch.object(csh, "KPM", fake_kpm), \
                 patch.object(csh, "CANONICAL", fake_canonical), \
                 patch("sys.argv", ["check_shared_headers.py"]), \
                 patch("sys.stdout", new_callable=io.StringIO):
                ret = csh.main()
                self.assertEqual(ret, 2)

    def test_drift_detected_when_out_of_sync(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            fake_kpm = Path(tmpdir) / "kpm"
            common = fake_kpm / "Patches" / "Common"
            shims = fake_kpm / "Patches" / "_shared"
            common.mkdir(parents=True)
            shims.mkdir(parents=True)

            fake_canonical = Path(tmpdir) / "canonical"
            fake_canonical.mkdir()
            (fake_canonical / "TestHeader.h").write_text("// Canonical version 2\n")

            # Common has outdated version
            (common / "TestHeader.h").write_text("// Canonical version 1\n")
            # Forwarder is missing

            with patch.object(csh, "KPM", fake_kpm), \
                 patch.object(csh, "CANONICAL", fake_canonical), \
                 patch.object(csh, "FOREIGN", []), \
                 patch("sys.argv", ["check_shared_headers.py"]), \
                 patch("sys.stdout", new_callable=io.StringIO) as mock_stdout:
                ret = csh.main()
                self.assertEqual(ret, 1)
                self.assertIn("DRIFT", mock_stdout.getvalue())

    def test_sync_fixes_drift(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            fake_kpm = Path(tmpdir) / "kpm"
            common = fake_kpm / "Patches" / "Common"
            shims = fake_kpm / "Patches" / "_shared"
            common.mkdir(parents=True)
            shims.mkdir(parents=True)

            fake_canonical = Path(tmpdir) / "canonical"
            fake_canonical.mkdir()
            content = "// Canonical Header Content\n"
            (fake_canonical / "TestHeader.h").write_text(content)

            # Run with --sync
            with patch.object(csh, "KPM", fake_kpm), \
                 patch.object(csh, "CANONICAL", fake_canonical), \
                 patch.object(csh, "FOREIGN", []), \
                 patch("sys.argv", ["check_shared_headers.py", "--sync"]), \
                 patch("sys.stdout", new_callable=io.StringIO):
                ret = csh.main()
                self.assertEqual(ret, 0)

            # Verify synced files
            self.assertEqual((common / "TestHeader.h").read_text(), content)
            self.assertEqual((shims / "TestHeader.h").read_text(), csh.forwarder("TestHeader.h"))

            # Run again without --sync to ensure clean parity
            with patch.object(csh, "KPM", fake_kpm), \
                 patch.object(csh, "CANONICAL", fake_canonical), \
                 patch.object(csh, "FOREIGN", []), \
                 patch("sys.argv", ["check_shared_headers.py"]), \
                 patch("sys.stdout", new_callable=io.StringIO):
                ret2 = csh.main()
                self.assertEqual(ret2, 0)


if __name__ == "__main__":
    unittest.main()
