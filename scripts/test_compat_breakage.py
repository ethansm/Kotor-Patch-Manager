#!/usr/bin/env python3
"""
scripts/test_compat_breakage.py - Proof by deliberate breakage (Doc 22 Phase U12).

Implements the five deliberate breakage scenarios in isolated scratch copies
(`tempfile.TemporaryDirectory`, never mutating the live repository):

  1. Break SafeRead.h (flip clamp):
     cpp:common_contract_test red with exit code 1 and BREAKING for hotbar.
  2. Break Gui2D.h (rename symbol used by buff HUD):
     build_check red for buff_duration_hud_src, shield_bar_src, queue_hud_src.
  3. Swap two fields in PortraitSlots.h:
     abi_check red naming changed field offsets and consumers.
  4. Delete PsDispatch.h:
     compat_check --why reports 'deleted PsDispatch.h' / 'includes deleted',
     ps_dispatch_test and provider build_check:portrait_slots_src red.
  5. Edit stub/windows.h:
     compat_check --list yields only host test roots, no DLL build items.
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


def make_scratch_repo(
    tmp_path: Path,
    copied_mods: tuple[str, ...] = ("_shared_src",),
    copy_research: bool = False,
) -> Path:
    """Create a lightweight scratch tree with copied targets and symlinked rest."""
    # Copy scripts so __file__.resolve() stays within tmp_path
    shutil.copytree(REPO_ROOT / "scripts", tmp_path / "scripts")

    # Set up patch_manager_mods
    (tmp_path / "patch_manager_mods").mkdir(parents=True, exist_ok=True)
    for mod_path in (REPO_ROOT / "patch_manager_mods").iterdir():
        if mod_path.name in copied_mods:
            shutil.copytree(mod_path, tmp_path / "patch_manager_mods" / mod_path.name)
        else:
            os.symlink(mod_path, tmp_path / "patch_manager_mods" / mod_path.name)

    # Optional research copy for stub tests
    if copy_research:
        shutil.copytree(REPO_ROOT / "research", tmp_path / "research")

    # Symlink remaining top-level items
    for item in REPO_ROOT.iterdir():
        if item.name not in ("scripts", "patch_manager_mods", ".git") and not (
            copy_research and item.name == "research"
        ):
            os.symlink(item, tmp_path / item.name)

    return tmp_path


class TestCompatBreakage(unittest.TestCase):
    def test_1_break_saferead_clamp(self):
        """1. Break SafeRead.h (flip a clamp): expect common_contract_test red with BREAKING."""
        with tempfile.TemporaryDirectory() as tmpdir:
            scratch = make_scratch_repo(
                Path(tmpdir),
                copied_mods=("_shared_src", "action_hotbar_src"),
            )

            # Flip the maxLen clamp (> to >=) in SafeRead.h
            sr_path = scratch / "patch_manager_mods/_shared_src/SafeRead.h"
            content = sr_path.read_text(encoding="utf-8")
            self.assertIn("len > maxLen", content)
            broken = content.replace("len > maxLen", "len >= maxLen")
            sr_path.write_text(broken, encoding="utf-8")

            # Run common_contract_test directly via run_tests.py
            tmp_env = dict(os.environ)
            tmp_env["TMPDIR"] = str(scratch / "tmp")
            (scratch / "tmp").mkdir()

            res = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/run_tests.py"),
                    "--build",
                    "--id",
                    "cpp:common_contract_test",
                ],
                cwd=scratch,
                env=tmp_env,
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 1, f"Expected common_contract_test to fail:\n{res.stdout}")
            self.assertIn("FAIL", res.stdout)
            self.assertIn("hotbar relies on: readExo accepts an 80-char name", res.stdout)

            # Run compat_check.py on SafeRead.h
            cc_res = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/compat_check.py"),
                    "patch_manager_mods/_shared_src/SafeRead.h",
                    "-j",
                    "8",
                ],
                cwd=scratch,
                env=tmp_env,
                capture_output=True,
                text=True,
            )
            self.assertEqual(cc_res.returncode, 1, f"Expected compat_check to fail:\n{cc_res.stdout}")
            self.assertTrue(
                "BREAKING for hotbar:" in cc_res.stdout or "BREAKING for action-hotbar:" in cc_res.stdout,
                f"Expected BREAKING message in stdout:\n{cc_res.stdout}",
            )

    def test_2_break_gui2d_symbol(self):
        """2. Break Gui2D.h (rename a symbol buff HUD uses): expect build_check red for affected mods."""
        with tempfile.TemporaryDirectory() as tmpdir:
            scratch = make_scratch_repo(
                Path(tmpdir),
                copied_mods=(
                    "_shared_src",
                    "buff_duration_hud_src",
                    "portrait_slots_src",
                    "shield_bar_src",
                    "queue_hud_src",
                ),
            )

            # Rename hudScale in Gui2D.h
            g2d_path = scratch / "patch_manager_mods/_shared_src/Gui2D.h"
            content = g2d_path.read_text(encoding="utf-8")
            self.assertIn("inline float hudScale()", content)
            broken = content.replace("inline float hudScale()", "inline float hudScaleBroken()")
            g2d_path.write_text(broken, encoding="utf-8")

            # Setup fake_kpm directory pointing Common/Gui2D.h to broken Gui2D.h
            fake_kpm = Path(tmpdir) / "fake_kpm"
            shutil.copytree(
                "/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager",
                fake_kpm,
                symlinks=True,
            )
            (fake_kpm / "Patches/Common/Gui2D.h").unlink()
            os.symlink(g2d_path, fake_kpm / "Patches/Common/Gui2D.h")

            # Expect build_check red for buff_duration_hud_src
            b_res_buff = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/build_check.py"),
                    "--mod",
                    "buff_duration_hud_src",
                    "--kpm-dir",
                    str(fake_kpm),
                ],
                cwd=scratch,
                capture_output=True,
                text=True,
            )
            self.assertEqual(b_res_buff.returncode, 1)
            self.assertIn("FAIL build: buff_duration_hud_src", b_res_buff.stdout)
            self.assertIn("hudScale", b_res_buff.stdout)

            # Expect build_check red for shield_bar_src
            b_res_shield = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/build_check.py"),
                    "--mod",
                    "shield_bar_src",
                    "--kpm-dir",
                    str(fake_kpm),
                ],
                cwd=scratch,
                capture_output=True,
                text=True,
            )
            self.assertEqual(b_res_shield.returncode, 1)
            self.assertIn("FAIL build: shield_bar_src", b_res_shield.stdout)

            # Expect build_check red for queue_hud_src
            b_res_queue = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/build_check.py"),
                    "--mod",
                    "queue_hud_src",
                    "--kpm-dir",
                    str(fake_kpm),
                ],
                cwd=scratch,
                capture_output=True,
                text=True,
            )
            self.assertEqual(b_res_queue.returncode, 1)
            self.assertIn("FAIL build: queue_hud_src", b_res_queue.stdout)

    def test_3_swap_portrait_slots_fields(self):
        """3. Swap two fields in PortraitSlots.h: expect ABI golden red."""
        with tempfile.TemporaryDirectory() as tmpdir:
            scratch = make_scratch_repo(
                Path(tmpdir),
                copied_mods=("_shared_src", "portrait_slots_src"),
            )

            ps_path = scratch / "patch_manager_mods/_shared_src/PortraitSlots.h"
            content = ps_path.read_text(encoding="utf-8")
            self.assertIn("    int32_t phase;\n    int32_t slot;", content)
            swapped = content.replace(
                "    int32_t phase;\n    int32_t slot;",
                "    int32_t slot;\n    int32_t phase;",
            )
            ps_path.write_text(swapped, encoding="utf-8")

            res = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/abi_check.py"),
                    "--header",
                    str(ps_path),
                ],
                cwd=scratch,
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 1)
            self.assertIn("BREAKING ABI for k2.portraitslots@1:", res.stdout)
            self.assertIn("field 'phase' offset changed from 4 to 8", res.stdout)
            self.assertIn("field 'slot' offset changed from 8 to 4", res.stdout)
            self.assertIn("Consumers reading k2.portraitslots@1:", res.stdout)

    def test_4_delete_ps_dispatch(self):
        """4. Delete PsDispatch.h: expect ps_dispatch_test and provider build red with 'includes deleted'."""
        with tempfile.TemporaryDirectory() as tmpdir:
            scratch = make_scratch_repo(
                Path(tmpdir),
                copied_mods=("_shared_src", "portrait_slots_src"),
            )

            # Delete PsDispatch.h
            target = scratch / "patch_manager_mods/portrait_slots_src/PsDispatch.h"
            self.assertTrue(target.is_file())
            target.unlink()

            # Verify compat_check plan identifies deleted header edge chain
            cc_res = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/compat_check.py"),
                    "patch_manager_mods/portrait_slots_src/PsDispatch.h",
                    "--list",
                    "--why",
                ],
                cwd=scratch,
                capture_output=True,
                text=True,
            )
            self.assertEqual(cc_res.returncode, 0)
            self.assertIn("includes deleted PsDispatch.h", cc_res.stdout)
            self.assertIn("deleted PsDispatch.h -> patch_manager_mods/portrait_slots_src/ps_dispatch_test.cpp", cc_res.stdout)

            # Test fails at compile time
            tmp_env = dict(os.environ)
            tmp_env["TMPDIR"] = str(scratch / "tmp")
            (scratch / "tmp").mkdir()

            t_res = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/run_tests.py"),
                    "--build",
                    "--id",
                    "cpp:ps_dispatch_test",
                ],
                cwd=scratch,
                env=tmp_env,
                capture_output=True,
                text=True,
            )
            self.assertEqual(t_res.returncode, 1)
            self.assertIn("FAIL", t_res.stdout)
            self.assertIn("PsDispatch.h: No such file or directory", t_res.stdout)

            # Provider build fails at compile time
            b_res = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/build_check.py"),
                    "--mod",
                    "portrait_slots_src",
                ],
                cwd=scratch,
                capture_output=True,
                text=True,
            )
            self.assertEqual(b_res.returncode, 1)
            self.assertIn("FAIL build: portrait_slots_src", b_res.stdout)
            self.assertIn("PsDispatch.h: No such file or directory", b_res.stdout)

    def test_5_edit_stub_windows(self):
        """5. Edit stub/windows.h: expect only host roots, no DLL build."""
        with tempfile.TemporaryDirectory() as tmpdir:
            scratch = make_scratch_repo(
                Path(tmpdir),
                copied_mods=("_shared_src",),
                copy_research=True,
            )

            stub_path = scratch / "research/companion_levelup/tools/parity/stub/windows.h"
            self.assertTrue(stub_path.is_file())
            content = stub_path.read_text(encoding="utf-8")
            stub_path.write_text(content + "\n// deliberate test edit\n", encoding="utf-8")

            res = subprocess.run(
                [
                    sys.executable,
                    str(scratch / "scripts/compat_check.py"),
                    "research/companion_levelup/tools/parity/stub/windows.h",
                    "--list",
                ],
                cwd=scratch,
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0)
            self.assertIn("SUITE", res.stdout)
            self.assertNotIn("BUILD", res.stdout)


if __name__ == "__main__":
    unittest.main()
