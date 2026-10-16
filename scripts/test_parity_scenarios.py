#!/usr/bin/env python3
"""Pins the saves.toml decisions reader in research/companion_levelup/tools/parity/scenarios.py (WI-37 moved the expected
decisions into saves.toml; dropping `dec.update(exp.get("decisions", {}))` must fail a test, not pass every gate)."""
import sys
import tomllib
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "research/companion_levelup/tools/parity"))
import scenarios  # noqa: E402

TOML = REPO / "tests/luo_scenarios/saves.toml"


class DecisionsReaderTest(unittest.TestCase):
    def test_synthetic_decision_line(self):
        out = scenarios.scenario_dat("SX", {"save": {}, "expect": {"decisions": {"atton.canLevelUp": "HOLD"}}})
        self.assertIn("DECISION atton.canLevelUp HOLD", out.splitlines())

    def test_saves_toml_decisions_reach_data_file(self):
        data = tomllib.loads(TOML.read_text())["scenario"]
        checked = 0
        for sid, sc in data.items():
            decisions = sc.get("expect", {}).get("decisions")
            if not decisions:
                continue
            lines = scenarios.scenario_dat(sid, sc).splitlines()
            for k, v in decisions.items():
                self.assertIn("DECISION %s %s" % (k, v), lines, sid)
            checked += 1
        self.assertGreaterEqual(checked, 7)


if __name__ == "__main__":
    unittest.main()
