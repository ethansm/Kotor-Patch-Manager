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

    def test_handoff_rotation(self):
        """Verify handoff writes session_handoff.md and rotates previous."""
        res = self.run_cli("handoff", "Test direction for phase X")
        self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
        handoff_path = REPO_ROOT / ".claude" / "session_handoff.md"
        self.assertTrue(handoff_path.exists())
        content = handoff_path.read_text()
        self.assertIn("Test direction for phase X", content)
        self.assertIn("## 0. Next-Phase Direction", content)
        self.assertIn("## 3. Strict Context Budget & Read Policy", content)
        self.assertIn("Fresh Session Bootstrap", content)

if __name__ == "__main__":
    unittest.main()
