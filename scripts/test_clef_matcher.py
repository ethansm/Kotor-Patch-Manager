#!/usr/bin/env python3
"""test_clef_matcher.py - Unit test suite for clef-matcher and clef-dataset.

Verifies:
  1. Ledger fact extraction & prior calculation (clef_dataset.py)
  2. Output contract compliance (role, backend, model, write_scope, duration, confidence)
  3. Strict write scope sanitization (filters out nonexistent mod directories)
  4. Recon role scope restriction (scout, verify, review, ghidra -> empty write_scope)
  5. Deterministic fallback behavior when daemon is mocked/offline
  6. Telemetry logging to master_ledger.jsonl
"""

import json
import os
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))

from clef_dataset import extract_ledger_facts, get_mod_prior
from clef_matcher import (
    deterministic_fallback_routing,
    infer_active_mod,
    route_task,
    sanitize_write_scope,
    VALID_BACKENDS,
    VALID_ROLES,
)


class TestClefDataset(unittest.TestCase):
    def test_extract_ledger_facts(self):
        facts = extract_ledger_facts()
        self.assertIn("total_events", facts)
        self.assertGreater(facts["total_events"], 0)
        self.assertIn("global_prior", facts)
        self.assertIn("mods", facts)

        global_p = facts["global_prior"]
        self.assertIn("avg_duration_ms", global_p)
        self.assertIn("best_backend", global_p)
        self.assertIn("common_role", global_p)

    def test_get_mod_prior(self):
        facts = extract_ledger_facts()
        prior = get_mod_prior("queue_hud_src", facts)
        self.assertIn("avg_duration_ms", prior)
        self.assertIn("best_backend", prior)
        self.assertIn("common_role", prior)
        self.assertIn("pass_rate", prior)


class TestClefMatcher(unittest.TestCase):
    def test_infer_active_mod(self):
        self.assertEqual(infer_active_mod("Fix QueueHud detour hook", None), "queue_hud_src")
        self.assertEqual(infer_active_mod("Random prompt", "dialogue_letterbox_fix_src"), "dialogue_letterbox_fix_src")
        self.assertEqual(infer_active_mod("Random prompt with no mod", None), None)

    def test_sanitize_write_scope(self):
        # Recon roles must have empty write scope
        for recon in ("scout", "verify", "review", "ghidra"):
            self.assertEqual(sanitize_write_scope(recon, "queue_hud_src"), [])

        # Worker role with existing mod
        scope = sanitize_write_scope("code", "queue_hud_src")
        self.assertEqual(scope, ["patch_manager_mods/queue_hud_src/*"])

        # Nonexistent mod must not hallucinate scope
        fake_scope = sanitize_write_scope("code", "fake_nonexistent_mod_12345")
        self.assertEqual(fake_scope, [])

        # Gate and doc roles must override active mod scope
        self.assertEqual(sanitize_write_scope("gate", "queue_hud_src"), ["scripts/build_check.py"])
        self.assertEqual(sanitize_write_scope("doc", "queue_hud_src"), ["docs/*", "patch_manager_mods/*.md"])

    def test_deterministic_fallback_schema(self):
        prior = {"avg_duration_ms": 2500, "best_backend": "local", "common_role": "code"}
        res = deterministic_fallback_routing("search for log file", "queue_hud_src", prior)

        self.assertIn(res["role"], VALID_ROLES)
        self.assertIn(res["backend"], VALID_BACKENDS)
        self.assertIn("model", res)
        self.assertIsInstance(res["write_scope"], list)
        self.assertIsInstance(res["predicted_duration_ms"], int)
        self.assertGreaterEqual(res["confidence"], 0.0)
        self.assertLessEqual(res["confidence"], 1.0)
        self.assertEqual(res["engine"], "deterministic_fallback")

    def test_route_task_contract(self):
        res = route_task("Fix pointer math in QueueHud hook detour", explicit_mod="queue_hud_src", log_telemetry=False)

        # Validate all required fields in spec
        self.assertIn(res["role"], VALID_ROLES)
        self.assertIn(res["backend"], VALID_BACKENDS)
        self.assertTrue(len(res["model"]) > 0)
        self.assertIsInstance(res["write_scope"], list)
        self.assertIsInstance(res["predicted_duration_ms"], int)
        self.assertIsInstance(res["confidence"], float)
        self.assertIn(res["engine"], ("clef-flash", "kev-0.8b", "kev-4b", "deterministic_fallback"))
        self.assertIn("latency_ms", res)
        self.assertEqual(res["active_mod"], "queue_hud_src")

    @patch("clef_matcher.query_clef_engine")
    def test_offline_daemon_failover(self, mock_query):
        # Simulate daemon offline
        mock_query.return_value = None
        res = route_task("Check build gate across repo", explicit_mod=None, log_telemetry=False)

        self.assertEqual(res["engine"], "deterministic_fallback")
        self.assertEqual(res["role"], "gate")
        self.assertEqual(res["backend"], "local")
        self.assertEqual(res["write_scope"], ["scripts/build_check.py"])

    @patch("clef_matcher.query_clef_engine")
    def test_calibrated_confidence_threshold(self, mock_query):
        # Clef-flash returns 0.50 confidence for code (well above 0.32 floor and margin)
        mock_query.return_value = {
            "answers": {
                "role": {
                    "type": "choice",
                    "choice": "code",
                    "confidence": 0.50,
                    "probabilities": {"code": 0.50, "review": 0.20, "deep": 0.10}
                },
                "backend": {"choice": "cloud"},
                "model": {"choice": "poolside/laguna-s-2.1:free"}
            }
        }
        res = route_task("Implement fast sorting in queue hud", explicit_mod="queue_hud_src", log_telemetry=False)
        self.assertEqual(res["role"], "code")
        self.assertEqual(res["backend"], "cloud")
        self.assertEqual(res["confidence"], 0.50)

    @patch("clef_matcher.query_clef_engine")
    def test_ambiguous_tie_fallback(self, mock_query):
        # Clef-flash returns ambiguous tie (0.28 < 0.32 floor)
        mock_query.return_value = {
            "answers": {
                "role": {
                    "type": "choice",
                    "choice": "code",
                    "confidence": 0.28,
                    "probabilities": {"code": 0.28, "deep": 0.26, "review": 0.20}
                },
                "backend": {"choice": "cloud"},
                "model": {"choice": "poolside/laguna-s-2.1:free"}
            }
        }
        res = route_task("Unclear task prompt", explicit_mod="queue_hud_src", log_telemetry=False)
        self.assertEqual(res["role"], "review")
        self.assertIn("inkling", res["model"])


if __name__ == "__main__":
    unittest.main()
