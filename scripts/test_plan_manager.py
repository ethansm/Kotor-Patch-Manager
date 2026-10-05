#!/usr/bin/env python3
"""test_plan_manager.py - Unit test suite for plan_manager.py"""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SCRIPT = REPO_ROOT / "scripts" / "plan_manager.py"
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

class TestPlanManager(unittest.TestCase):
    def run_cli(self, *args):
        cmd = [sys.executable, str(SCRIPT)] + list(args)
        res = subprocess.run(cmd, cwd=str(REPO_ROOT), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return res

    def test_resume_json(self):
        """Verify resume emits valid structured JSON with plan and git state."""
        res = self.run_cli("resume", "--json")
        self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
        data = json.loads(res.stdout)
        self.assertIn("plan_path", data)
        self.assertIn("branch", data)
        self.assertIn("progress", data)
        self.assertIn("current_phase", data)

    def test_table_plan_parsing(self):
        """Verify parsing of markdown table plans (e.g. doc 24)."""
        table_plan = """# Master Test Plan
| Id | Batch | Status |
|---|---|---|
| M1 | Init | DONE |
| M2 | Build | TODO |
"""
        with tempfile.NamedTemporaryFile("w", suffix="_plan.md", delete=False) as fp:
            fp.write(table_plan)
            tmp_path = Path(fp.name)

        try:
            from scripts.plan_manager import parse_plan
            res = parse_plan(tmp_path)
            self.assertEqual(len(res["phases"]), 2)
            self.assertTrue(res["phases"][0]["done"])
            self.assertFalse(res["phases"][1]["done"])
            self.assertEqual(res["phases"][1]["ident"], "M2")
        finally:
            tmp_path.unlink()

    def test_next_contract(self):
        """Verify next --contract appends the agent_contract.py template; default output has none."""
        table_plan = "# P\n| Id | Batch | Status |\n|---|---|---|\n| M1 | Init | TODO |\n"
        with tempfile.NamedTemporaryFile("w", suffix="_plan.md", delete=False) as fp:
            fp.write(table_plan)
            tmp_path = Path(fp.name)
        try:
            res = self.run_cli("next", "--plan", str(tmp_path), "--contract", "recon")
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            self.assertTrue(res.stdout.rstrip().splitlines()[-1].startswith("VERDICT: "), res.stdout)
            plain = self.run_cli("next", "--plan", str(tmp_path))
            self.assertNotIn("Output Schema", plain.stdout)
        finally:
            tmp_path.unlink()

    def test_handoff_rotation(self):
        """Verify handoff writes session_handoff.md and rotates previous in isolated temp dir."""
        table_plan = "# Test Plan\n| Id | Batch | Status |\n|---|---|---|\n| M1 | Init | TODO |\n"
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp_root = Path(tmpdir)
            tmp_plan = tmp_root / "test_plan.md"
            tmp_plan.write_text(table_plan)
            handoff_dir = tmp_root / ".claude"

            # 1. First run writes session_handoff.md
            res1 = self.run_cli("handoff", "--plan", str(tmp_plan), "--handoff-dir", str(handoff_dir), "Direction One")
            self.assertEqual(res1.returncode, 0, f"Stderr: {res1.stderr}")
            h_path = handoff_dir / "session_handoff.md"
            prev_path = handoff_dir / "session_handoff.prev.md"
            self.assertTrue(h_path.exists())
            self.assertFalse(prev_path.exists())
            self.assertIn("Direction One", h_path.read_text())

            # 2. Second run rotates previous to session_handoff.prev.md
            res2 = self.run_cli("handoff", "--plan", str(tmp_plan), "--handoff-dir", str(handoff_dir), "Direction Two")
            self.assertEqual(res2.returncode, 0, f"Stderr: {res2.stderr}")
            self.assertTrue(prev_path.exists())
            self.assertIn("Direction One", prev_path.read_text())
            self.assertIn("Direction Two", h_path.read_text())
    def test_mark_done_custom_state(self):
        """Verify mark-done supports custom --state (e.g. PARTIAL, BUILT)."""
        table_plan = """# Master Test Plan
| Id | Batch | Status | Commit | Notes |
|---|---|---|---|---|
| M1 | Init | TODO | | |
| M2 | Build | TODO | | |
"""
        with tempfile.NamedTemporaryFile("w", suffix="_plan.md", delete=False) as fp:
            fp.write(table_plan)
            tmp_path = Path(fp.name)

        try:
            res = self.run_cli("--plan", str(tmp_path), "mark-done", "M1", "--state", "PARTIAL", "--notes", "halfway done")
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            content = tmp_path.read_text()
            self.assertIn("| M1 | Init | PARTIAL | | halfway done |", content)

            res2 = self.run_cli("--plan", str(tmp_path), "mark-done", "M2", "--state", "BUILT", "--commit", "abc1234")
            self.assertEqual(res2.returncode, 0, f"Stderr: {res2.stderr}")
            content2 = tmp_path.read_text()
            self.assertIn("| M2 | Build | BUILT | abc1234 | |", content2)
        finally:
            tmp_path.unlink()

if __name__ == "__main__":
    unittest.main()
