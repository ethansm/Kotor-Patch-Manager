#!/usr/bin/env python3
"""test_subagent_tools.py - Unit tests for agent_diff_check.py and subagent_prompt.py."""

import json
import subprocess
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))

import agent_diff_check as adc
import subagent_prompt as sap


class TestSubagentTools(unittest.TestCase):
    def test_scope_checker(self):
        paths = ["patch_manager_mods/queue_hud_src/QueueHud.cpp", "scripts/test.py"]
        allowed = ["patch_manager_mods/queue_hud_src/*", "scripts/*"]
        violations = adc.check_scope(paths, allowed)
        self.assertEqual(violations, [])

        disallowed = ["patch_manager_mods/combat_hotbar_src/*"]
        violations = adc.check_scope(paths, disallowed)
        self.assertEqual(len(violations), 2)

    def test_get_changed_mods(self):
        paths = [
            "patch_manager_mods/queue_hud_src/QueueHud.cpp",
            "patch_manager_mods/action_hotbar_src/ActionHotbar.cpp",
            "scripts/some_script.py"
        ]
        mods = adc.get_changed_mods(paths)
        self.assertEqual(mods, ["action_hotbar_src", "queue_hud_src"])

    def test_subagent_prompt_phase_resolution(self):
        plan_path = sap.resolve_phase_and_plan("QH3")
        self.assertIsNotNone(plan_path)
        self.assertTrue(plan_path.exists())

        info = sap.extract_phase_info(plan_path, "QH3")
        title, desc, files, cmds, mod_scope = info
        self.assertIn("QH3", title)
        self.assertIn("Queue HUD", title)
        self.assertTrue(len(cmds) > 0)
        self.assertTrue(any("build_check.py" in c for c in cmds))
        self.assertIsNotNone(mod_scope)
        self.assertIn("queue_hud_src", mod_scope)

    def test_subagent_prompt_builder(self):
        prompt = sap.build_self_contained_prompt(
            "T1", "Test Phase", "Implement XYZ", ["test.cpp"], ["python3 test.py"]
        )
        self.assertIn("Test Phase", prompt)
        self.assertIn("test.cpp", prompt)
        self.assertIn("python3 test.py", prompt)
        self.assertIn("agent_diff_check.py", prompt)
        self.assertIn("Output Schema: WORKER", prompt)

    def test_subagent_prompt_role_variants(self):
        # Scout: Recon schema, read-only
        prompt_scout = sap.build_self_contained_prompt(
            "T1", "Scout Phase", "Explore XYZ", ["test.cpp"], [], role="scout"
        )
        self.assertIn("Output Schema: RECON", prompt_scout)
        self.assertNotIn("Output Schema: WORKER", prompt_scout)
        self.assertNotIn("Plan Specification Requirements", prompt_scout)

        # Verify: Recon schema, commands present
        prompt_verify = sap.build_self_contained_prompt(
            "T1", "Verify Phase", "Check tests", [], ["python3 scripts/run_tests.py -k test"], role="verify"
        )
        self.assertIn("Output Schema: RECON", prompt_verify)
        self.assertIn("python3 scripts/run_tests.py -k test", prompt_verify)

        # Ghidra: Recon schema, Ghidra instructions
        prompt_ghidra = sap.build_self_contained_prompt(
            "T1", "Ghidra Phase", "Inspect SceneCamVt", [], [], role="ghidra",
            ghidra_symbols=["SceneCamVt_Attach", "0x0053f000"]
        )
        self.assertIn("Output Schema: RECON", prompt_ghidra)
        self.assertIn("SceneCamVt_Attach", prompt_ghidra)
        self.assertIn("Ghidra Analysis & Reversing Focus", prompt_ghidra)

    def test_subagent_prompt_mod_scope_and_symbol_extraction(self):
        symbols = sap.extract_ghidra_symbols("Reference K2Addr::SceneCamVt_Attach and address 0x00418d20")
        self.assertIn("SceneCamVt_Attach", symbols)
        self.assertIn("0x00418d20", symbols)

        prompt = sap.build_self_contained_prompt(
            "T1", "Code Phase", "Build ABC", ["mod.cpp"], ["python3 test.py"],
            mod_scope="patch_manager_mods/queue_hud_src/", role="code"
        )
        self.assertIn("patch_manager_mods/queue_hud_src/", prompt)
        self.assertIn("Owning Mod & Scope Boundary", prompt)

    def test_diff_check_baseline_and_patch(self):
        import tempfile
        with tempfile.NamedTemporaryFile("w", suffix=".patch", delete=False) as tf:
            patch_path = Path(tf.name)
        try:
            cmd = [sys.executable, str(REPO_ROOT / "scripts" / "agent_diff_check.py"), "--mod", "queue_hud_src", "--patch-out", str(patch_path), "--json"]
            res = subprocess.run(cmd, cwd=str(REPO_ROOT), capture_output=True, text=True)
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            data = json.loads(res.stdout)
            self.assertIn("verdict", data)
            self.assertIn("changed_files", data)
        finally:
            patch_path.unlink(missing_ok=True)


if __name__ == "__main__":
    unittest.main()
