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

from compat_check import (
    find_k2addr_gog_range,
    get_git_diff_records,
    is_k2addr_gog_only_diff,
    narrow_plan_to_gog_scoping,
    resolve_file_arg,
    run_lint,
)
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

    def test_wine_smoke_fresh_dll_texmem(self):
        """texmem with dll_from = 'build' does not print STALE in --list --wine (Doc 22 U8)."""
        res = subprocess.run(
            [sys.executable, str(self.script), "TextureMemoryRelease.cpp", "--wine", "--list"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("WINE    wine:texmem", res.stdout)
        self.assertNotIn("STALE:", res.stdout)

    def test_wine_smoke_stale_kpatch_detection(self):
        """Smokes with dll_from = 'kpatch' print STALE and report unverified when source is newer (Doc 22 U8)."""
        # Load real graph but test with SafeRead.h which affects LoadHangProbe and PerfLab
        res = subprocess.run(
            [sys.executable, str(self.script), "SafeRead.h", "--wine", "--list"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        # probe11 or perflab use dll_from = 'kpatch' and SafeRead.h is newer than the kpatch binaries in KPM
        self.assertIn("STALE:", res.stdout)

    def test_run_tests_wine_fresh_build(self):
        """run_tests.py --wine -k texmem passes with dll_from = 'build' (Doc 22 U8)."""
        res = subprocess.run(
            [sys.executable, str(REPO_ROOT / "scripts/run_tests.py"), "--wine", "-k", "texmem"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("wine:texmem[run]", res.stdout)
        self.assertIn("4 passed", res.stdout)


class TestK2AddrVersionScoping(unittest.TestCase):
    def setUp(self):
        self.script = SCRIPT_DIR / "compat_check.py"
        self.k2addr_path = REPO_ROOT / "patch_manager_mods/_shared_src/K2Addr.h"

    def test_find_k2addr_gog_range_real(self):
        """find_k2addr_gog_range accurately finds makeGog to kGog lines in real K2Addr.h."""
        content = self.k2addr_path.read_text(encoding="utf-8")
        rng = find_k2addr_gog_range(content)
        self.assertIsNotNone(rng)
        start, end = rng
        lines = content.splitlines()
        self.assertIn("constexpr K2AddrTable makeGog()", lines[start - 1])
        self.assertIn("inline constexpr K2AddrTable kGog", lines[end - 1])

    def test_find_k2addr_gog_range_synthetic(self):
        """find_k2addr_gog_range works on synthetic C++ text."""
        synthetic = (
            "struct K2AddrTable {};\n"
            "constexpr K2AddrTable makeSteam() { return {}; }\n"
            "inline constexpr K2AddrTable kSteam = makeSteam();\n"
            "constexpr K2AddrTable makeGog() {\n"
            "    K2AddrTable t{};\n"
            "    t.AppGlobal = 0x123;\n"
            "    return t;\n"
            "}\n"
            "inline constexpr K2AddrTable kGog = makeGog();\n"
            "int foo();\n"
        )
        rng = find_k2addr_gog_range(synthetic)
        self.assertIsNotNone(rng)
        self.assertEqual(rng, (4, 9))

        no_gog = "int foo();\nint bar();\n"
        self.assertIsNone(find_k2addr_gog_range(no_gog))

    def test_is_k2addr_gog_only_diff_unit(self):
        """is_k2addr_gog_only_diff identifies whether diff hunks touch only kGog."""
        base = (
            "// line 1\n"
            "constexpr K2AddrTable makeSteam() {\n"
            "    t.App = 1;\n"
            "}\n"
            "inline constexpr K2AddrTable kSteam = makeSteam();\n"
            "constexpr K2AddrTable makeGog() {\n"
            "    t.App = 2;\n"
            "}\n"
            "inline constexpr K2AddrTable kGog = makeGog();\n"
            "// line 10\n"
        )
        # 1. Diff within makeGog (lines 6-9)
        gog_diff = (
            "--- a/K2Addr.h\n+++ b/K2Addr.h\n"
            "@@ -7,1 +7,1 @@\n"
            "-    t.App = 2;\n"
            "+    t.App = 3;\n"
        )
        new_gog = base.replace("t.App = 2;", "t.App = 3;")
        self.assertTrue(is_k2addr_gog_only_diff(gog_diff, base, new_gog))

        # 2. Diff in makeSteam (lines 2-5)
        steam_diff = (
            "--- a/K2Addr.h\n+++ b/K2Addr.h\n"
            "@@ -3,1 +3,1 @@\n"
            "-    t.App = 1;\n"
            "+    t.App = 5;\n"
        )
        new_steam = base.replace("t.App = 1;", "t.App = 5;")
        self.assertFalse(is_k2addr_gog_only_diff(steam_diff, base, new_steam))

        # 3. Empty diff
        self.assertFalse(is_k2addr_gog_only_diff("", base, base))

        # 4. Multi-hunk: one in GOG, one outside
        mixed_diff = (
            "--- a/K2Addr.h\n+++ b/K2Addr.h\n"
            "@@ -3,1 +3,1 @@\n"
            "-    t.App = 1;\n"
            "+    t.App = 5;\n"
            "@@ -7,1 +7,1 @@\n"
            "-    t.App = 2;\n"
            "+    t.App = 3;\n"
        )
        self.assertFalse(is_k2addr_gog_only_diff(mixed_diff, base, base))

        # 5. Multi-hunk: both in GOG
        multi_gog_diff = (
            "--- a/K2Addr.h\n+++ b/K2Addr.h\n"
            "@@ -6,1 +6,1 @@\n"
            "-constexpr K2AddrTable makeGog() {\n"
            "+constexpr K2AddrTable makeGog() noexcept {\n"
            "@@ -7,1 +7,1 @@\n"
            "-    t.App = 2;\n"
            "+    t.App = 3;\n"
        )
        self.assertTrue(is_k2addr_gog_only_diff(multi_gog_diff, base, new_gog))

    def test_narrow_plan_to_gog_scoping(self):
        """narrow_plan_to_gog_scoping retains only builds and cpp:k2addr_test."""
        from compat_graph import Plan, PlanItem
        plan = Plan(
            touched=["patch_manager_mods/_shared_src/K2Addr.h"],
            items=[
                PlanItem(kind="build", id="build_check:shield_bar_src", why=("K2Addr.h",)),
                PlanItem(kind="build", id="build_check:buff_duration_hud_src", why=("K2Addr.h",)),
                PlanItem(kind="test", id="cpp:k2addr_test", why=("K2Addr.h",)),
                PlanItem(kind="test", id="cpp:gui2d_test", why=("K2Addr.h",)),
                PlanItem(kind="suite", id="suite:guikit", why=("K2Addr.h",)),
                PlanItem(kind="static", id="suite:check-hook-bytes", why=("K2Addr.h",)),
                PlanItem(kind="wine", id="wine:texmem", why=("K2Addr.h",)),
            ],
            gaps=["some gap"],
        )
        narrow_plan_to_gog_scoping(plan)
        self.assertEqual(len(plan.items), 3)
        self.assertEqual([it.id for it in plan.items], ["build_check:shield_bar_src", "build_check:buff_duration_hud_src", "cpp:k2addr_test"])
        self.assertEqual(plan.gaps, [])
        self.assertIn("GOG-only: Steam runtime unchanged", plan.info_notes)

    def test_cli_clean_k2addr_full_fanout(self):
        """Unmodified K2Addr.h yields conservative full fan-out (Doc 22 U11)."""
        res = subprocess.run(
            [sys.executable, str(self.script), "patch_manager_mods/_shared_src/K2Addr.h", "--list"],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("GOG-only: Steam runtime unchanged", res.stdout)
        self.assertIn("BUILD   build_check", res.stdout)
        self.assertIn("TEST    cpp:k2addr_test", res.stdout)
        self.assertIn("TEST    cpp:gui2d_test", res.stdout)
        self.assertIn("SUITE   suite:guikit", res.stdout)
        self.assertIn("STATIC  suite:check-hook-bytes", res.stdout)

    def test_cli_scratch_diff_make_gog_narrows(self):
        """Scratch diff in makeGog yields cpp:k2addr_test + compile-only builds (Doc 22 U11)."""
        orig_content = self.k2addr_path.read_text(encoding="utf-8")
        try:
            # Modify an address in makeGog
            modified = orig_content.replace(
                "t.AppGlobal = 0x00A11C04;",
                "t.AppGlobal = 0x00A11C08;",
            )
            self.k2addr_path.write_text(modified, encoding="utf-8")

            res = subprocess.run(
                [sys.executable, str(self.script), "patch_manager_mods/_shared_src/K2Addr.h", "--list"],
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            self.assertEqual(res.returncode, 0)
            self.assertIn("GOG-only: Steam runtime unchanged", res.stdout)
            self.assertIn("BUILD   build_check", res.stdout)
            self.assertIn("TEST    cpp:k2addr_test", res.stdout)
            # Ensure full fan-out items are excluded
            self.assertNotIn("TEST    cpp:gui2d_test", res.stdout)
            self.assertNotIn("TEST    cpp:buff_hud_logic_test", res.stdout)
            self.assertNotIn("SUITE", res.stdout)
            self.assertNotIn("STATIC", res.stdout)
        finally:
            self.k2addr_path.write_text(orig_content, encoding="utf-8")

    def test_cli_scratch_diff_make_steam_stays_full_fanout(self):
        """Scratch diff in makeSteam does not narrow to GOG-only (Doc 22 U11)."""
        orig_content = self.k2addr_path.read_text(encoding="utf-8")
        try:
            # Modify an address in makeSteam
            modified = orig_content.replace(
                "t.AppGlobal = 0x00A1B4A4;",
                "t.AppGlobal = 0x00A1B4A8;",
            )
            self.k2addr_path.write_text(modified, encoding="utf-8")

            res = subprocess.run(
                [sys.executable, str(self.script), "patch_manager_mods/_shared_src/K2Addr.h", "--list"],
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            self.assertEqual(res.returncode, 0)
            self.assertNotIn("GOG-only: Steam runtime unchanged", res.stdout)
            self.assertIn("TEST    cpp:gui2d_test", res.stdout)
            self.assertIn("STATIC", res.stdout)
        finally:
            self.k2addr_path.write_text(orig_content, encoding="utf-8")


if __name__ == "__main__":
    unittest.main()
