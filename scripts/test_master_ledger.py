#!/usr/bin/env python3
"""test_master_ledger.py - Unit tests for master_ledger.py."""

import argparse
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import master_ledger as ml


class TestMasterLedger(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.TemporaryDirectory()
        self.ledger_path = Path(self.tmpdir.name) / "test_ledger.jsonl"
        self.orig_agent = os.environ.get("AGENT_ID")

    def tearDown(self):
        self.tmpdir.cleanup()
        if self.orig_agent is not None:
            os.environ["AGENT_ID"] = self.orig_agent
        elif "AGENT_ID" in os.environ:
            del os.environ["AGENT_ID"]

    def test_log_and_read_event(self):
        rec = ml.log_event("HOTSWAP_APPLY", {"mod": "queue-hud", "backup": "b1"}, self.ledger_path)
        self.assertEqual(rec["event"], "HOTSWAP_APPLY")
        self.assertEqual(rec["mod"], "queue_hud_src")
        self.assertEqual(rec["backup"], "b1")

        events = ml.read_events(self.ledger_path)
        self.assertEqual(len(events), 1)
        self.assertEqual(events[0]["event"], "HOTSWAP_APPLY")
        self.assertEqual(events[0]["mod"], "queue_hud_src")

    def test_format_event_line(self):
        rec = {
            "ts": "2026-10-07T14:20:00Z",
            "event": "HOTSWAP_APPLY",
            "agent": "test-agent",
            "mod": "combat-hotbar",
            "backup": "b_123",
        }
        line = ml.format_event_line(rec)
        self.assertIn("HOTSWAP_APPLY", line)
        self.assertIn("mod=combat-hotbar", line)
        self.assertIn("test-agent", line)

    def test_metrics_and_parent_agent(self):
        os.environ["PARENT_AGENT_ID"] = "gemini-flash"
        os.environ["AGENT_ID"] = "opencode:code"
        try:
            rec = ml.log_event("OPENCODE_RUN", {
                "role": "code",
                "backend": "local",
                "duration_ms": 2500,
                "tokens_in": 12000,
                "tokens_out": 450,
            }, self.ledger_path)
            self.assertEqual(rec["parent_agent"], "gemini-flash")
            self.assertEqual(rec["agent"], "opencode:code")
            self.assertEqual(rec["duration_ms"], 2500)
            line = ml.format_event_line(rec)
            self.assertIn("opencode:code <- gemini-flash", line)
            self.assertIn("duration=2.5s", line)
            self.assertIn("tokens=12.0k/450", line)
            self.assertIn("backend=local", line)
        finally:
            del os.environ["PARENT_AGENT_ID"]
            del os.environ["AGENT_ID"]

    def test_filter_events(self):
        ml.log_event("HOTSWAP_APPLY", {"mod": "queue-hud"}, self.ledger_path)
        ml.log_event("PHASE_DONE", {"plan": "33_plan.md", "phase": "U1"}, self.ledger_path)
        ml.log_event("HOTSWAP_APPLY", {"mod": "combat-hotbar"}, self.ledger_path)

        events = ml.read_events(self.ledger_path)
        self.assertEqual(len(events), 3)

        # Filter by event
        filtered = [e for e in events if e.get("event") == "HOTSWAP_APPLY"]
        self.assertEqual(len(filtered), 2)

    def test_collision_detection(self):
        # Log an action by another agent
        os.environ["AGENT_ID"] = "agent-alpha"
        ml.log_event("HOTSWAP_APPLY", {"mod": "queue-hud"}, self.ledger_path)

        # Check collision as agent-beta
        os.environ["AGENT_ID"] = "agent-beta"
        args = type("Args", (), {"mod": "queue-hud", "within_minutes": 30})()
        ret = ml.cmd_check_collision(args, self.ledger_path)
        self.assertEqual(ret, 1)

        # Check collision for untouched mod
        args_clean = type("Args", (), {"mod": "untouched-mod", "within_minutes": 30})()
        ret_clean = ml.cmd_check_collision(args_clean, self.ledger_path)
        self.assertEqual(ret_clean, 0)

    def test_canonical_and_kpm_mod_names(self):
        self.assertEqual(ml.canonical_mod_name("queue-hud"), "queue_hud_src")
        self.assertEqual(ml.canonical_mod_name("npc-diversity"), "npc_diversity_src")
        self.assertEqual(ml.canonical_mod_name("patch_manager_mods/combat_hotbar_src/"), "combat_hotbar_src")
        self.assertEqual(ml.canonical_mod_name("_shared"), "_shared_src")
        self.assertEqual(ml.canonical_mod_name("queue"), "queue_hud_src")
        self.assertEqual(ml.kpm_mod_name("queue_hud_src"), "queue-hud")
        self.assertEqual(ml.kpm_mod_name("combat_hotbar_src"), "combat-hotbar")

    def test_test_isolation_guard(self):
        # When under test harness and no explicit ledger path, log_event must not write to disk
        rec = ml.log_event("TEST_ISOLATION", {"test": True})
        self.assertEqual(rec["event"], "TEST_ISOLATION")
        # Ensure DEFAULT_LEDGER does not contain TEST_ISOLATION
        if ml.DEFAULT_LEDGER.is_file():
            content = ml.DEFAULT_LEDGER.read_text(encoding="utf-8")
            self.assertNotIn("TEST_ISOLATION", content)

    def test_prune_test_data(self):
        # Create temp ledger with mix of real and test pollution events
        ml.log_event("HOTSWAP_APPLY", {"mod": "queue_hud_src", "backup": "real_backup"}, self.ledger_path)
        ml.log_event("HOTSWAP_APPLY", {"mod": "demo", "backup": "demo_swap_123"}, self.ledger_path)
        ml.log_event("BUILD_GATE", {"error": "gate-unittest failure"}, self.ledger_path)

        events_before = ml.read_events(self.ledger_path)
        self.assertEqual(len(events_before), 3)

        args = argparse.Namespace()
        ml.cmd_prune_test_data(args, self.ledger_path)

        events_after = ml.read_events(self.ledger_path)
        self.assertEqual(len(events_after), 1)
        self.assertEqual(events_after[0]["mod"], "queue_hud_src")


if __name__ == "__main__":
    import argparse
    unittest.main()
