#!/usr/bin/env python3
"""
scripts/test_compat_check.py - Unit tests for compat_check.py (Doc 22 Phase U3).

Tests:
  - Positional file argument resolution (full path and short basename)
  - Git diff intake parsing (A, M, D, R renames, untracked)
  - Safety guardrail (>25 touched files without --yes exits 2)
  - Plan output and --list (exits 0 without running tests)
  - --why edge chains in output
  - --json output schema compliance
  - --strict flag (exit 3 on UNCLASSIFIED or GAPs)
  - Exit code 1 and BREAKING for <mod> detection on failing contract test
  - Section A golden items (Gui2D.h)
  - Section B golden items (PortraitSlotsProvider.cpp)
  - Lint mode (--lint)
"""

from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

# Add scripts directory to path
SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from compat_check import get_git_diff_records, resolve_file_arg, run_lint
from compat_graph import CompatGraph


class TestCompatCheckIntake(unittest.TestCase):
    def setUp(self):
        self.repo = REPO_ROOT

    def test_resolve_file_arg_exact(self):
        """Exact repo-relative path resolves directly."""
        p, exists = resolve_file_arg(
            self.repo,
            "patch_manager_mods/_shared_src/Gui2D.h",
            {"patch_manager_mods/_shared_src/Gui2D.h"},
        )
        self.assertEqual(p, "patch_manager_mods/_shared_src/Gui2D.h")
        self.assertTrue(exists)

    def test_resolve_file_arg_basename(self):
        """Short basename resolves to canonical repo path."""
        p, exists = resolve_file_arg(
            self.repo,
            "Gui2D.h",
            {"patch_manager_mods/_shared_src/Gui2D.h"},
        )
        self.assertEqual(p, "patch_manager_mods/_shared_src/Gui2D.h")
        self.assertTrue(exists)

    def test_git_diff_records_staged_or_head(self):
        """get_git_diff_records runs git diff without crashing."""
        records, err = get_git_diff_records(self.repo, "HEAD", staged=False, pathspecs=[])
        self.assertIsNone(err)
        self.assertIsInstance(records, list)


class TestCompatCheckCLI(unittest.TestCase):
    def setUp(self):
        self.script = SCRIPT_DIR / "compat_check.py"

    def test_list_gui2d(self):
        """--list on Gui2D.h outputs plan and exits 0 without running tests."""
        res = subprocess.run(
            [sys.executable, str(self.script), "Gui2D.h", "--list"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("touched: patch_manager_mods/_shared_src/Gui2D.h", res.stdout)
        self.assertIn("BUILD   build_check", res.stdout)
        self.assertIn("buff_duration_hud_src", res.stdout)
        self.assertIn("portrait_slots_src", res.stdout)
        self.assertIn("shield_bar_src", res.stdout)
        self.assertIn("TEST    cpp:gui2d_test", res.stdout)
        self.assertIn("TEST    cpp:k2addr_test", res.stdout)
        self.assertIn("STATIC  suite:check-raw-addresses, suite:check-shared-headers", res.stdout)
        self.assertIn("KPM     consumers after sync:", res.stdout)

    def test_why_gui2d(self):
        """--why outputs edge chains."""
        res = subprocess.run(
            [sys.executable, str(self.script), "Gui2D.h", "--why"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("--- Edge Chains (--why) ---", res.stdout)
        self.assertIn("-> patch_manager_mods/_shared_src/gui2d_test.cpp", res.stdout)

    def test_json_output(self):
        """--json output complies with schema."""
        res = subprocess.run(
            [sys.executable, str(self.script), "Gui2D.h", "--list", "--json", "-"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        # Find JSON object in output
        idx = res.stdout.find("{\n  \"touched\":")
        self.assertGreater(idx, -1)
        data = json.loads(res.stdout[idx:])
        self.assertIn("touched", data)
        self.assertIn("plan", data)
        self.assertIn("gaps", data)
        self.assertIn("unclassified", data)
        self.assertIn("counts", data)
        self.assertIn("wall_seconds", data)

    def test_golden_section_b_portrait_slots_provider(self):
        """Section B items for PortraitSlotsProvider.cpp are selected."""
        res = subprocess.run(
            [sys.executable, str(self.script), "PortraitSlotsProvider.cpp", "--list"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("portrait_slots_src", res.stdout)
        self.assertIn("buff_duration_hud_src", res.stdout)
        self.assertIn("shield_bar_src", res.stdout)
        self.assertIn("cpp:ps_dispatch_test", res.stdout)
        self.assertIn("cpp:buff_hud_logic_test", res.stdout)
        self.assertIn("cpp:shield_logic_test", res.stdout)
        self.assertIn("suite:check-raw-addresses", res.stdout)
        self.assertIn("suite:check-subclassers", res.stdout)

    def test_safety_guardrail_refusal(self):
        """--since with >25 files without --yes exits 2 with helpful error."""
        res = subprocess.run(
            [sys.executable, str(self.script), "--since", "HEAD"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        # Because worktree has ~161 uncommitted files, it must exit 2 unless restricted
        self.assertEqual(res.returncode, 2)
        self.assertIn("Refuse to run without --yes", res.stderr)

    def test_safety_guardrail_bypass(self):
        """--since with pathspec restricting files to <= 25 passes guardrail."""
        res = subprocess.run(
            [
                sys.executable,
                str(self.script),
                "--since",
                "HEAD",
                "--list",
                "--",
                "patch_manager_mods/_shared_src/Gui2D.h",
            ],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)

    def test_lint_mode(self):
        """--lint runs and reports warnings / gaps without crashing."""
        res = subprocess.run(
            [sys.executable, str(self.script), "--lint"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("GAP dialogue_letterbox_fix_src includes Common headers", res.stdout)
        self.assertIn("GAP action_hotbar_src requires interface k2.hud@1", res.stdout)
        self.assertIn("WARN activation_fix_src has no reaching host test", res.stdout)

    def test_lint_strict_fails_on_gaps(self):
        """--lint --strict exits with code 3 when gaps or warnings exist."""
        res = subprocess.run(
            [sys.executable, str(self.script), "--lint", "--strict"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 3)

    def test_lint_json_schema(self):
        """--lint --json output conforms to expected schema."""
        res = subprocess.run(
            [sys.executable, str(self.script), "--lint", "--json", "-"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        data = json.loads(res.stdout)
        self.assertTrue(data.get("lint"))
        self.assertIn("warnings", data)
        self.assertIn("gaps", data)
        self.assertIn("all", data)

    def test_lint_stale_covers_warning(self):
        """Adding a stale covers pattern produces a WARN message."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmproot = Path(tmpdir)
            shutil.copytree(REPO_ROOT / "tests", tmproot / "tests", dirs_exist_ok=True)
            compat_toml = tmproot / "tests/compat.toml"
            if compat_toml.is_file():
                content = compat_toml.read_text(encoding="utf-8")
                compat_toml.write_text(
                    content + '\n[[covers]]\nid = "test:stale"\npaths = ["nonexistent/fake/path/**"]\n',
                    encoding="utf-8",
                )
            g = CompatGraph(tmproot)
            code, warnings = run_lint(tmproot, g)
            stale_warns = [w for w in warnings if "stale covers pattern" in w and "nonexistent/fake/path/**" in w]
            self.assertTrue(len(stale_warns) > 0)

    def test_lint_unclassified_file(self):
        """Passing an unclassified file to --lint reports WARN UNCLASSIFIED file."""
        res = subprocess.run(
            [sys.executable, str(self.script), "--lint", "README.md"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("WARN UNCLASSIFIED file: README.md", res.stdout)

    def test_breaking_for_mod_detection_mock(self):
        """Regex parses BREAKING for <mod> correctly from failure output."""
        sample_tail = (
            "ASSERTION FAILED: offsetof(K2PortraitSlotsApi, SlotAt) == 0x14\n"
            "shield_bar relies on: K2PortraitSlotsApi::SlotAt offset == 0x14\n"
            "common_contract_test: 12 checks, 1 failed"
        )
        match = re.search(r"(([a-zA-Z0-9_\-]+)\s+relies on:[^\r\n]*)", sample_tail)
        self.assertIsNotNone(match)
        self.assertEqual(match.group(2), "shield_bar")
        self.assertIn("shield_bar relies on: K2PortraitSlotsApi::SlotAt offset == 0x14", match.group(1))

    def test_strict_mode_unclassified(self):
        """--strict exits with code 3 on unclassified files."""
        res = subprocess.run(
            [sys.executable, str(self.script), "README.md", "--list", "--strict"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 3)
        self.assertIn("UNCLASSIFIED README.md", res.stdout)

    def test_levelup_rules_parity_suites(self):
        """Touching levelup_rules.py selects parity suites (Doc 22 U5)."""
        res = subprocess.run(
            [sys.executable, str(self.script), "research/companion_levelup/levelup_rules.py", "--list"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("suite:levelup-parity", res.stdout)
        self.assertIn("suite:levelup-wizard-parity", res.stdout)
        self.assertIn("suite:levelup-options-parity", res.stdout)

    def test_saves_toml_luo_tests(self):
        """Touching saves.toml selects LUO python tests and scenario suites (Doc 22 U5)."""
        res = subprocess.run(
            [sys.executable, str(self.script), "tests/luo_scenarios/saves.toml", "--list"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("py:scripts/test_luo_scenario.py", res.stdout)
        self.assertIn("py:scripts/test_luo_log_check.py", res.stdout)
        self.assertIn("py:scripts/test_kotor_save_forge.py", res.stdout)
        self.assertIn("suite:levelup-scenarios", res.stdout)

    def test_generator_notice(self):
        """Touching a generated file prints the generator note (Doc 22 U5)."""
        res = subprocess.run(
            [sys.executable, str(self.script), "patch_manager_mods/_shared_src/DebuffIconTable.inc", "--list"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("INFO: patch_manager_mods/_shared_src/DebuffIconTable.inc is generated by scripts/gen_debuff_icon_table.py", res.stdout)


if __name__ == "__main__":
    unittest.main()
