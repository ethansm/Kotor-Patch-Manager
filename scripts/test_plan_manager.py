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
        self.assertIn("upcoming_phases", data)
        self.assertIn("next_handoff", data)

    def test_resume_upcoming_batches_text(self):
        """Verify resume text output shows upcoming batches and next handoff step."""
        table_plan = """# Pipeline Plan
| Id | Batch | Status |
|---|---|---|
| M1 | Init | TODO |
| M2 | Build | TODO |
| M3 | Test | TODO |
"""
        with tempfile.NamedTemporaryFile("w", suffix="_plan.md", delete=False) as fp:
            fp.write(table_plan)
            tmp_path = Path(fp.name)

        try:
            res = self.run_cli("--plan", str(tmp_path), "resume")
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            self.assertIn("Upcoming Batches:", res.stdout)
            self.assertIn("M2 — Build", res.stdout)
            self.assertIn("M3 — Test", res.stdout)
            self.assertIn("Next Steps:", res.stdout)
            self.assertIn("/plan-handoff M1", res.stdout)
        finally:
            tmp_path.unlink()

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

    def test_mark_done_auto_handoff_and_saved_state(self):
        """Verify mark-done automatically generates handoff and prints Saved State block."""
        table_plan = """# Master Test Plan
| Id | Batch | Status | Commit | Notes |
|---|---|---|---|---|
| M1 | Init | TODO | | |
| M2 | Build | TODO | | |
"""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp_root = Path(tmpdir)
            tmp_plan = tmp_root / "test_saved_state_plan.md"
            tmp_plan.write_text(table_plan, encoding="utf-8")
            h_dir = tmp_root / ".claude"

            res = self.run_cli("--plan", str(tmp_plan), "mark-done", "M1", "--commit", "feedbee", "--handoff-dir", str(h_dir))
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            self.assertIn("=== SAVED STATE (Safe to /clear) ===", res.stdout)
            self.assertIn("Completed: M1 (DONE) @ feedbee", res.stdout)
            self.assertIn("Next Phase: M2 — Build", res.stdout)
            self.assertIn("Resume: python3 scripts/plan_manager.py resume M2", res.stdout)

            # Verify handoff file was written automatically
            scoped_h = h_dir / "handoffs" / "test_saved_state_plan.handoff.md"
            self.assertTrue(scoped_h.exists())
            self.assertIn("Execute M2: Build", scoped_h.read_text(encoding="utf-8"))

    def test_handoff_long_direction_text(self):
        """Verify handoff does not crash with OSError when passed long direction text >255 chars."""
        table_plan = "# Test Plan\n| Id | Batch | Status |\n|---|---|---|\n| M1 | Init | TODO |\n"
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp_root = Path(tmpdir)
            tmp_plan = tmp_root / "test_plan.md"
            tmp_plan.write_text(table_plan)
            long_prompt = "next plan should focus on reviewing our kotor2 skills and finding better prompts, techniques, strategies or scripts " * 5
            res = self.run_cli("--plan", str(tmp_plan), "handoff", long_prompt, "--handoff-dir", str(tmp_root))
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            h_path = tmp_root / "session_handoff.md"
            self.assertTrue(h_path.exists())
            self.assertIn("reviewing our kotor2 skills", h_path.read_text())

    def test_handoff_shorthand_disambiguation(self):
        """Verify shorthand resolution disambiguates between candidate docs sharing the same prefix."""
        from scripts.plan_manager import resolve_plan_shorthand
        p, rest = resolve_plan_shorthand("10 — Rank 7: Perf-Lab 0.6.0 Telemetry (10_engine_optimization_research.md)")
        self.assertIsNotNone(p)
        self.assertEqual(p.name, "10_engine_optimization_research.md")
        self.assertIn("Rank 7", rest)

    def test_list_plans_cli(self):
        """Verify list subcommand outputs known plans in text and JSON modes."""
        res_text = self.run_cli("list")
        self.assertEqual(res_text.returncode, 0)
        self.assertIn("KNOWN MASTER & SUBDOMAIN PLANS", res_text.stdout)

        res_json = self.run_cli("list", "--json")
        self.assertEqual(res_json.returncode, 0)
        data = json.loads(res_json.stdout)
        self.assertIsInstance(data, list)
        self.assertTrue(any(p["stem"] == "22_compat_check_plan" for p in data))

    def test_mark_done_auto_resolves_plan_from_phase(self):
        """Verify mark-done finds the owning plan doc from the phase token even if another plan is active."""
        test_plan_path = REPO_ROOT / "patch_manager_mods" / "99_temp_autoresolve_test_plan.md"
        content = """# Temp Auto-Resolve Plan
| Id | Batch | Status |
|---|---|---|
| AR1 | Action 1 | TODO |
| AR2 | Action 2 | TODO |
"""
        test_plan_path.write_text(content, encoding="utf-8")
        try:
            # Call mark-done without --plan, passing phase AR1
            res = self.run_cli("mark-done", "AR1", "--notes", "auto resolved")
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            updated = test_plan_path.read_text(encoding="utf-8")
            self.assertIn("| AR1 | Action 1 | DONE |", updated)
        finally:
            test_plan_path.unlink(missing_ok=True)

    def test_resume_scoped_handoff_direction(self):
        """Verify resume reads direction from .claude/handoffs/<plan_stem>.handoff.md."""
        test_plan = REPO_ROOT / "patch_manager_mods" / "99_temp_scoped_handoff_plan.md"
        handoffs_dir = REPO_ROOT / ".claude" / "handoffs"
        handoffs_dir.mkdir(parents=True, exist_ok=True)
        scoped_file = handoffs_dir / "99_temp_scoped_handoff_plan.handoff.md"

        test_plan.write_text("# Temp Scoped Plan\n| Id | Batch | Status |\n|---|---|---|\n| SC1 | Action | TODO |\n", encoding="utf-8")
        scoped_file.write_text("# Handoff\n## 0. Next-Phase Direction\nExecute SC1 with private TMPDIR\n\n## 1. Provenance\n", encoding="utf-8")
        try:
            res = self.run_cli("--plan", str(test_plan), "resume")
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            self.assertIn("Handoff Direction: Execute SC1 with private TMPDIR", res.stdout)
            self.assertIn("Resume: python3 scripts/plan_manager.py resume SC1", res.stdout)

            res_json = self.run_cli("--plan", str(test_plan), "resume", "--json")
            self.assertEqual(res_json.returncode, 0)
            data = json.loads(res_json.stdout)
            self.assertEqual(data.get("handoff_direction"), "Execute SC1 with private TMPDIR")
        finally:
            test_plan.unlink(missing_ok=True)
            scoped_file.unlink(missing_ok=True)


if __name__ == "__main__":
    unittest.main()

