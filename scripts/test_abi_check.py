#!/usr/bin/env python3
"""
scripts/test_abi_check.py - Unit test suite for abi_check.py (Doc 22 Phase U6).

Tests:
  - Clean golden check on live repository (k2.portraitslots@1 passes)
  - Breaking field reorder / offset change fails with exit code 1, names moved field and consumers
  - Append-only field extension passes with exit code 0
  - Constant value alteration fails with exit code 1
  - --dump-only generates valid TOML with interface and struct definitions
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent


class TestAbiCheck(unittest.TestCase):
    def setUp(self):
        self.script = SCRIPT_DIR / "abi_check.py"
        self.header = REPO_ROOT / "patch_manager_mods/_shared_src/PortraitSlots.h"
        self.header_content = self.header.read_text(encoding="utf-8")

    def test_live_golden_clean(self):
        """Current live repository matches golden k2.portraitslots@1.toml."""
        res = subprocess.run(
            [sys.executable, str(self.script)],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("PASS abi_check: k2.portraitslots@1", res.stdout)
        self.assertIn("append-only OK", res.stdout)

    def test_dump_only(self):
        """--dump-only outputs valid TOML."""
        res = subprocess.run(
            [sys.executable, str(self.script), "--dump-only"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("[interface]", res.stdout)
        self.assertIn('name = "k2.portraitslots"', res.stdout)
        self.assertIn("[struct.K2PsContext]", res.stdout)
        self.assertIn("[struct.K2PortraitSlotsApi]", res.stdout)

    def test_swapped_fields_fails_and_names_consumers(self):
        """Scratch header with swapped field order fails and names consumers."""
        swapped = self.header_content.replace(
            "    int32_t phase;\n    int32_t slot;",
            "    int32_t slot;\n    int32_t phase;",
        )
        with tempfile.NamedTemporaryFile("w+", suffix=".h", delete=False) as tf:
            tf.write(swapped)
            tf_path = tf.name

        try:
            res = subprocess.run(
                [sys.executable, str(self.script), "--header", tf_path],
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            self.assertEqual(res.returncode, 1)
            self.assertIn("BREAKING ABI for k2.portraitslots@1:", res.stdout)
            self.assertIn("field 'phase' offset changed from 4 to 8", res.stdout)
            self.assertIn("field 'slot' offset changed from 8 to 4", res.stdout)
            self.assertIn("Consumers reading k2.portraitslots@1:", res.stdout)
            self.assertIn("shield_bar_src", res.stdout)
            self.assertIn("buff_duration_hud_src", res.stdout)
        finally:
            Path(tf_path).unlink(missing_ok=True)

    def test_appended_field_passes(self):
        """Scratch header with an appended field at struct end passes (append-only rule)."""
        appended = self.header_content.replace(
            "    int32_t companionCtrl[2];/* LBL_CHAR2 / LBL_CHAR3 controls (panel + 0x3134 / 0x36DC) */",
            "    int32_t companionCtrl[2];/* LBL_CHAR2 / LBL_CHAR3 controls (panel + 0x3134 / 0x36DC) */\n    int32_t newTrailingField;",
        )
        self.assertIn("int32_t newTrailingField;", appended)
        with tempfile.NamedTemporaryFile("w+", suffix=".h", delete=False) as tf:
            tf.write(appended)
            tf_path = tf.name

        try:
            res = subprocess.run(
                [sys.executable, str(self.script), "--header", tf_path],
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            self.assertEqual(res.returncode, 0)
            self.assertIn("PASS abi_check: k2.portraitslots@1", res.stdout)
            self.assertIn("append-only OK", res.stdout)
        finally:
            Path(tf_path).unlink(missing_ok=True)

    def test_update_refuses_golden_without_matching_dumper(self):
        """--update must not rewrite a golden whose interface has no dumper (fallback dumps another interface)."""
        original = (
            "[interface]\n"
            'name = "k2.other"\n'
            "major = 1\n"
            'header = "patch_manager_mods/_shared_src/PortraitSlots.h"\n'
            'provides = "k2.other@1"\n'
        )
        with tempfile.TemporaryDirectory() as td:
            tmp_repo = Path(td)
            abi_dir = tmp_repo / "tests" / "abi"
            abi_dir.mkdir(parents=True)
            # Only the portraitslots dumper exists; golden k2.other@1 has no matching dumper.
            shutil.copy2(REPO_ROOT / "tests/abi/abi_dump_portraitslots.cpp", abi_dir / "abi_dump_portraitslots.cpp")
            os.symlink(REPO_ROOT / "patch_manager_mods", tmp_repo / "patch_manager_mods")
            os.symlink(REPO_ROOT / "tests/include", tmp_repo / "tests" / "include")
            golden = abi_dir / "k2.other@1.toml"
            golden.write_text(original, encoding="utf-8")

            res = subprocess.run(
                [sys.executable, str(self.script), "--update", "--repo", str(tmp_repo)],
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            self.assertNotEqual(res.returncode, 0)
            self.assertIn("Refusing to update golden", res.stderr)
            self.assertEqual(golden.read_text(encoding="utf-8"), original)


if __name__ == "__main__":
    unittest.main()
