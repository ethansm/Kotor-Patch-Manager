#!/usr/bin/env python3
"""test_plan_manager.py - Unit test suite for plan_manager.py"""

import atexit
import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SCRIPT = REPO_ROOT / "scripts" / "plan_manager.py"
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))
import os
os.environ["MASTER_LEDGER_TEST"] = "1"
# Sandbox dir for every file a test writes, so nothing lands in the real repo or its .claude state
_SANDBOX_DIR = Path(tempfile.mkdtemp(prefix="plan_manager_test_"))
atexit.register(shutil.rmtree, _SANDBOX_DIR, ignore_errors=True)
# Sandbox pointer and handoffs dir for CLI calls, so tests never rewrite the real .claude state
_SANDBOX_ACTIVE = _SANDBOX_DIR / "active_plan.txt"
_SANDBOX_HANDOFFS = _SANDBOX_DIR / "handoffs"

class TestPlanManager(unittest.TestCase):
    def setUp(self):
        # Every test starts with no pointer and no handoffs, so one test's leftovers never steer another
        _SANDBOX_ACTIVE.unlink(missing_ok=True)
        shutil.rmtree(_SANDBOX_HANDOFFS, ignore_errors=True)

    def make_plan(self, rel, body="# P\n| Id | Batch | Status |\n|---|---|---|\n| M1 | Init | TODO |\n| M2 | Build | TODO |\n"):
        p = _SANDBOX_DIR / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(body, encoding="utf-8")
        return p

    def run_cli(self, *args):
        env = dict(os.environ, PLAN_MANAGER_ACTIVE_FILE=str(_SANDBOX_ACTIVE), PLAN_MANAGER_HANDOFFS_DIR=str(_SANDBOX_HANDOFFS))
        cmd = [sys.executable, str(SCRIPT)] + list(args)
        res = subprocess.run(cmd, cwd=str(REPO_ROOT), env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
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
            self.assertIn("/plan-handoff", res.stdout)
            self.assertIn(f'/plan-handoff "{tmp_path.stem}:M1"', res.stdout)
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

    def test_handoff_writes_named_file_only(self):
        """Verify handoff writes only the plan-scoped file; the shared session_handoff.md is never written or rotated."""
        table_plan = "# Test Plan\n| Id | Batch | Status |\n|---|---|---|\n| M1 | Init | TODO |\n"
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp_root = Path(tmpdir)
            tmp_plan = tmp_root / "test_plan.md"
            tmp_plan.write_text(table_plan)
            handoff_dir = tmp_root / ".claude"
            named_path = handoff_dir / "handoffs" / "test_plan.handoff.md"
            shared_path = handoff_dir / "session_handoff.md"
            shared_prev_path = handoff_dir / "session_handoff.prev.md"

            # 1. First run writes the named file only
            res1 = self.run_cli("handoff", "--plan", str(tmp_plan), "--handoff-dir", str(handoff_dir), "Direction One")
            self.assertEqual(res1.returncode, 0, f"Stderr: {res1.stderr}")
            self.assertTrue(named_path.exists())
            self.assertIn("Direction One", named_path.read_text())
            self.assertFalse(shared_path.exists())
            self.assertFalse(shared_prev_path.exists())

            # 2. Second run overwrites the named file; still no shared file and no rotation
            res2 = self.run_cli("handoff", "--plan", str(tmp_plan), "--handoff-dir", str(handoff_dir), "Direction Two")
            self.assertEqual(res2.returncode, 0, f"Stderr: {res2.stderr}")
            self.assertIn("Direction Two", named_path.read_text())
            self.assertNotIn("Direction One", named_path.read_text())
            self.assertFalse(shared_path.exists())
            self.assertFalse(shared_prev_path.exists())
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

        # --handoff-dir isolates handoff output and keeps the real .claude/active_plan.txt untouched
        with tempfile.TemporaryDirectory() as h_dir:
            try:
                res = self.run_cli("--plan", str(tmp_path), "mark-done", "M1", "--state", "PARTIAL", "--notes", "halfway done", "--handoff-dir", h_dir)
                self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
                content = tmp_path.read_text()
                self.assertIn("| M1 | Init | PARTIAL | | halfway done |", content)

                res2 = self.run_cli("--plan", str(tmp_path), "mark-done", "M2", "--state", "BUILT", "--commit", "abc1234", "--handoff-dir", h_dir)
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
            self.assertIn("resume \"test_saved_state_plan:M2\"", res.stdout)

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
            h_path = tmp_root / "handoffs" / "test_plan.handoff.md"
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
        test_plan_path = _SANDBOX_DIR / "99_temp_autoresolve_test_plan.md"
        content = """# Temp Auto-Resolve Plan
| Id | Batch | Status |
|---|---|---|
| AR1 | Action 1 | TODO |
| AR2 | Action 2 | TODO |
"""
        test_plan_path.write_text(content, encoding="utf-8")
        try:
            # Point the sandbox pointer at the temp plan, so mark-done without --plan resolves it
            _SANDBOX_ACTIVE.write_text(str(test_plan_path), encoding="utf-8")
            # Call mark-done without --plan, passing phase AR1; keep handoff output in a temp dir
            with tempfile.TemporaryDirectory() as hdir:
                res = self.run_cli("mark-done", "AR1", "--notes", "auto resolved", "--handoff-dir", hdir)
                self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
                updated = test_plan_path.read_text(encoding="utf-8")
                self.assertIn("| AR1 | Action 1 | DONE |", updated)
        finally:
            test_plan_path.unlink(missing_ok=True)

    def test_resume_scoped_handoff_direction(self):
        """Verify resume reads direction from .claude/handoffs/<plan_stem>.handoff.md."""
        test_plan = _SANDBOX_DIR / "99_temp_scoped_handoff_plan.md"
        handoffs_dir = _SANDBOX_HANDOFFS
        handoffs_dir.mkdir(parents=True, exist_ok=True)
        scoped_file = handoffs_dir / "99_temp_scoped_handoff_plan.handoff.md"

        test_plan.write_text("# Temp Scoped Plan\n| Id | Batch | Status |\n|---|---|---|\n| SC1 | Action | TODO |\n", encoding="utf-8")
        scoped_file.write_text("# Handoff\n## 0. Next-Phase Direction\nExecute SC1 with private TMPDIR\n\n## 1. Provenance\n", encoding="utf-8")
        try:
            res = self.run_cli("--plan", str(test_plan), "resume")
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            self.assertIn("Handoff Direction: Execute SC1 with private TMPDIR", res.stdout)
            self.assertIn("resume \"99:SC1\"", res.stdout)

            res_json = self.run_cli("--plan", str(test_plan), "resume", "--json")
            self.assertEqual(res_json.returncode, 0)
            data = json.loads(res_json.stdout)
            self.assertEqual(data.get("handoff_direction"), "Execute SC1 with private TMPDIR")
        finally:
            test_plan.unlink(missing_ok=True)
            scoped_file.unlink(missing_ok=True)

    def test_delegate_dry_run(self):
        """Verify delegate --dry-run emits prompt and respects plan."""
        test_plan = _SANDBOX_DIR / "99_temp_delegate_plan.md"
        test_plan.write_text("# Temp Plan\n| Id | Batch | Status |\n|---|---|---|\n| DG1 | Delegate Batch | TODO |\n", encoding="utf-8")
        try:
            res = self.run_cli("--plan", str(test_plan), "delegate", "DG1", "--dry-run")
            self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
            self.assertIn("DRY-RUN DELEGATION PROMPT FOR PHASE DG1", res.stdout)
            self.assertIn("DG1: Delegate Batch", res.stdout)
            self.assertIn("agent_diff_check.py", res.stdout)
        finally:
            test_plan.unlink(missing_ok=True)

    # ---- wrong-plan / wrong-spot handoff regressions -------------------------------------------

    def test_handoff_honors_handoffs_dir_env(self):
        """handoff without --handoff-dir writes where resume reads (PLAN_MANAGER_HANDOFFS_DIR), never the real repo."""
        plan = self.make_plan("envprobe/env_probe_plan.md")
        stray = REPO_ROOT / ".claude" / "handoffs" / "env_probe_plan.handoff.md"
        try:
            res = self.run_cli("handoff", "--plan", str(plan), "Env probe")
            self.assertEqual(res.returncode, 0, res.stderr)
            self.assertTrue((_SANDBOX_HANDOFFS / "env_probe_plan.handoff.md").exists())
            self.assertFalse(stray.exists())
            self.assertIn("for plan", res.stdout)
        finally:
            stray.unlink(missing_ok=True)

    def test_handoff_refuses_free_text_and_empty_direction(self):
        """No plan token and no --plan: refuse (exit 2) instead of using the shared pointer; nothing is written."""
        other = self.make_plan("other/other_session_plan.md")
        _SANDBOX_ACTIVE.write_text(str(other), encoding="utf-8")
        for direction in ("fix the hotbar crash", "", "2 hotbar fixes left", "25 minutes of testing"):
            res = self.run_cli("handoff", direction)
            self.assertEqual(res.returncode, 2, f"{direction!r}: {res.stdout} {res.stderr}")
            self.assertIn("needs a plan", res.stderr)
        self.assertFalse(_SANDBOX_HANDOFFS.exists())
        self.assertEqual(_SANDBOX_ACTIVE.read_text(encoding="utf-8"), str(other))

    def test_read_only_commands_leave_pointer_alone(self):
        """resume/next/delegate with an explicit plan never re-point the pointer other sessions share."""
        a = self.make_plan("ro/plan_a.md")
        b = self.make_plan("ro/plan_b.md", "# B\n| Id | Batch | Status |\n|---|---|---|\n| DG1 | Batch | TODO |\n")
        _SANDBOX_ACTIVE.write_text(str(a), encoding="utf-8")
        for cmd in (["resume"], ["next"], ["delegate", "DG1", "--dry-run"]):
            res = self.run_cli("--plan", str(b), *cmd)
            self.assertEqual(res.returncode, 0, f"{cmd}: {res.stderr}")
            self.assertEqual(_SANDBOX_ACTIVE.read_text(encoding="utf-8"), str(a), f"{cmd} rewrote the pointer")

    def test_handoff_sets_pointer_and_dry_run_writes_nothing(self):
        a = self.make_plan("hp/plan_a.md")
        b = self.make_plan("hp/plan_b.md")
        _SANDBOX_ACTIVE.write_text(str(a), encoding="utf-8")
        dry = self.run_cli("handoff", "--plan", str(b), "--dry-run", "Preview")
        self.assertEqual(dry.returncode, 0, dry.stderr)
        self.assertIn("DRY RUN", dry.stdout)
        self.assertFalse(_SANDBOX_HANDOFFS.exists())
        self.assertEqual(_SANDBOX_ACTIVE.read_text(encoding="utf-8"), str(a))
        real = self.run_cli("handoff", "--plan", str(b), "Real")
        self.assertEqual(real.returncode, 0, real.stderr)
        self.assertEqual(_SANDBOX_ACTIVE.read_text(encoding="utf-8"), str(b))

    def test_dangling_pointer_is_an_error_not_a_guess(self):
        _SANDBOX_ACTIVE.write_text(str(_SANDBOX_DIR / "gone" / "renamed_plan.md"), encoding="utf-8")
        res = self.run_cli("resume")
        self.assertEqual(res.returncode, 2)
        self.assertIn("missing file", res.stderr)
        _SANDBOX_ACTIVE.write_text("", encoding="utf-8")  # empty pointer = no pointer, never Path('.')
        self.assertEqual(self.run_cli("resume").returncode, 0)

    def test_same_stem_plans_get_separate_handoffs(self):
        """Two plans sharing a stem must not overwrite each other's handoff; resume reads the right one."""
        a = self.make_plan("one/dup_plan.md")
        b = self.make_plan("two/dup_plan.md")
        self.assertEqual(self.run_cli("handoff", "--plan", str(a), "Direction A").returncode, 0)
        self.assertEqual(self.run_cli("handoff", "--plan", str(b), "Direction B").returncode, 0)
        self.assertIn("Direction A", (_SANDBOX_HANDOFFS / "dup_plan.handoff.md").read_text(encoding="utf-8"))
        self.assertIn("Direction B", (_SANDBOX_HANDOFFS / "two__dup_plan.handoff.md").read_text(encoding="utf-8"))
        self.assertIn("Handoff Direction: Direction B", self.run_cli("--plan", str(b), "resume").stdout)
        self.assertIn("Handoff Direction: Direction A", self.run_cli("--plan", str(a), "resume").stdout)

    def test_dead_ends_survive_regeneration(self):
        plan = self.make_plan("de/dead_end_plan.md")
        self.run_cli("handoff", "--plan", str(plan), "Direction One")
        h = _SANDBOX_HANDOFFS / "dead_end_plan.handoff.md"
        h.write_text(h.read_text(encoding="utf-8") + "- DEAD END: detour on 0x1234 crashes\n", encoding="utf-8")
        self.run_cli("handoff", "--plan", str(plan), "Direction Two")
        text = h.read_text(encoding="utf-8")
        self.assertIn("Direction Two", text)
        self.assertNotIn("Direction One", text)
        self.assertIn("DEAD END: detour on 0x1234 crashes", text)
        self.assertEqual(text.count("## 5. Dead Ends & Verification (manual)"), 1)

    def test_mark_done_no_handoff_keeps_direction(self):
        """handoff after mark-done --no-handoff is the last writer; plain mark-done would regenerate the handoff."""
        plan = self.make_plan("md/order_plan.md")
        with tempfile.TemporaryDirectory() as hd:
            self.run_cli("handoff", "--plan", str(plan), "--handoff-dir", hd, "Custom: play test first")
            res = self.run_cli("--plan", str(plan), "mark-done", "M1", "--state", "PARTIAL", "--no-handoff", "--handoff-dir", hd)
            self.assertEqual(res.returncode, 0, res.stderr)
            self.assertIn("Custom: play test first", (Path(hd) / "handoffs" / "order_plan.handoff.md").read_text(encoding="utf-8"))

    def test_mark_done_exact_prefixed_and_unmatched(self):
        plan = self.make_plan("md2/rows_plan.md", "# R\n| Id | Batch | Status |\n|---|---|---|\n| T6a | First | TODO |\n| T6 | Second | TODO |\n")
        with tempfile.TemporaryDirectory() as hd:
            ok = self.run_cli("--plan", str(plan), "mark-done", "T6", "--no-handoff", "--handoff-dir", hd)
            self.assertEqual(ok.returncode, 0, ok.stderr)
            txt = plan.read_text(encoding="utf-8")
            self.assertIn("| T6a | First | TODO |", txt)  # substring match must not hit the T6a row
            self.assertIn("| T6 | Second | DONE |", txt)
            ok2 = self.run_cli("--plan", str(plan), "mark-done", "24:T6a", "--state", "PARTIAL", "--no-handoff", "--handoff-dir", hd)
            self.assertEqual(ok2.returncode, 0, ok2.stderr)
            self.assertIn("| T6a | First | PARTIAL |", plan.read_text(encoding="utf-8"))
            before = plan.read_text(encoding="utf-8")
            bad = self.run_cli("--plan", str(plan), "mark-done", "ZZ9", "--handoff-dir", hd)
            self.assertEqual(bad.returncode, 1)
            self.assertEqual(plan.read_text(encoding="utf-8"), before)  # no stray checkbox appended
            self.assertNotIn("SAVED STATE", bad.stdout)
            self.assertFalse((Path(hd) / "handoffs").exists())

    def test_direction_picker_skips_done_phases(self):
        plan = self.make_plan("pk/pick_plan.md", "# P\n| Id | Batch | Status |\n|---|---|---|\n| M1 | Init | DONE |\n| M2 | Build | TODO |\n| M3 | Test | TODO |\n")
        with tempfile.TemporaryDirectory() as hd:
            res = self.run_cli("handoff", "--plan", str(plan), "--handoff-dir", hd, "Re-verify M1 then do M3")
            self.assertEqual(res.returncode, 0, res.stderr)
            text = (Path(hd) / "handoffs" / "pick_plan.handoff.md").read_text(encoding="utf-8")
            self.assertIn("## 2. Immediate Objective (M3)", text)
            self.assertNotIn("Objective (M1)", text)

    # ---- resolver ------------------------------------------------------------------------------

    def _patched_repo(self, tmp_root):
        import scripts.plan_manager as pm
        from unittest import mock
        return pm, mock.patch.multiple(pm, REPO_ROOT=tmp_root, ACTIVE_PLAN_FILE=_SANDBOX_ACTIVE)

    def test_resolver_stems_numbers_and_prose(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td).resolve()
            for rel in (".claude/plans/swarm_arch.md", "patch_manager_mods/02_buff_hud.md", "patch_manager_mods/02_buff_hud_feedback.md",
                        "patch_manager_mods/24_master.md", "patch_manager_mods/sub/dup_plan.md", "research/x/dup_plan.md",
                        "README.md", ".pytest_cache/README.md", ".claude/handoffs/24_master.handoff.md"):
                f = root / rel
                f.parent.mkdir(parents=True, exist_ok=True)
                f.write_text("# x\n", encoding="utf-8")
            pm, patcher = self._patched_repo(root)
            with patcher:
                self.assertEqual(pm.resolve_plan_shorthand("swarm_arch")[0], root / ".claude/plans/swarm_arch.md")  # .claude/plans is reachable
                self.assertIsNone(pm.resolve_plan_shorthand("README")[0])  # non-plan root docs and dot-dirs never resolve
                self.assertIsNone(pm.resolve_plan_shorthand(str(root / ".claude/handoffs/24_master.handoff.md"))[0])
                self.assertEqual(pm.resolve_plan_shorthand("24:T6")[0], root / "patch_manager_mods/24_master.md")
                self.assertEqual(pm.resolve_plan_shorthand("24 §12")[1], "12")
                self.assertIsNone(pm.resolve_plan_shorthand("2 hotbar fixes left")[0])  # prose, not doc 02
                self.assertIsNone(pm.resolve_plan_shorthand("25 minutes of testing")[0])
                with self.assertRaises(pm.AmbiguousPlan):
                    pm.resolve_plan_shorthand("dup_plan", strict=True)
                with self.assertRaises(pm.AmbiguousPlan):
                    pm.resolve_plan_shorthand("02", strict=True)
                self.assertIsNotNone(pm.resolve_plan_shorthand("dup_plan")[0])  # lenient callers get a deterministic pick

    # ---- parse_plan ----------------------------------------------------------------------------

    def _parse(self, body):
        from scripts.plan_manager import parse_plan
        p = self.make_plan(f"parse/parse_{abs(hash(body))}.md", body)
        return parse_plan(p)["phases"]

    def test_parse_preface_status_table_does_not_hide_phase_headers(self):
        phases = self._parse("# P\n| Gate | State |\n|---|---|\n| lint | ok |\n\n## Phase 1: Loader\n- [ ] a\n## Phase 2 — Saver\n- [ ] write saver\n")
        self.assertEqual([(p["ident"], p["name"]) for p in phases], [("Phase 1", "Loader"), ("Phase 2", "Saver")])
        self.assertEqual(phases[1]["tasks"], ["- [ ] write saver"])

    def test_parse_aligned_separator_rows_are_skipped(self):
        phases = self._parse("# P\n| Id | Batch | Status |\n|:---|:---:|---:|\n| M1 | Init | TODO |\n")
        self.assertEqual([p["ident"] for p in phases], ["M1"])

    def test_parse_progress_bullet_is_not_completion(self):
        phases = self._parse("# P\n## Phase 1: A\n## Phase 2: B\n## Execution log\n- Phase 1: started, blocked on review\n- Phase 2: done, verified\n")
        self.assertEqual([(p["ident"], p["done"]) for p in phases], [("Phase 1", False), ("Phase 2", True)])

    def test_parse_status_words(self):
        rows = [("A", "INSTALLED"), ("B", "NOT DONE, NEED GO-AHEAD"), ("C", "PARTIAL (3 of 5 DONE)"), ("D", "DONE (play test pending)"),
                ("E", "BUILT"), ("F", "**DONE** 10-05"), ("G", "TODO"), ("H", "PASS")]
        body = "# P\n| Id | Batch | Status |\n|---|---|---|\n" + "".join(f"| {i} | x | {s} |\n" for i, s in rows)
        done = {p["ident"]: p["done"] for p in self._parse(body)}
        self.assertEqual(done, {"A": True, "B": False, "C": False, "D": True, "E": False, "F": True, "G": False, "H": True})


if __name__ == "__main__":
    unittest.main()

