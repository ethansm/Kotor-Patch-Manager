#!/usr/bin/env python3
"""Tests for scripts/luo_log_check.py (fixtures in scripts/testdata/luo_log_check/)."""
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import luo_log_check as L  # noqa: E402

FX = HERE / "testdata" / "luo_log_check"


def run(name, **kw):
    lines = (FX / name).read_text().splitlines()
    return L.run(lines, **kw)


def status(r, i):
    return r["v"][i]["status"]


class RealRuns(unittest.TestCase):
    def test_run4b_item6_pass(self):
        r = run("run4b.txt")
        self.assertEqual(status(r, 6), "PASS")
        self.assertEqual(r["v"][6]["lines"], [1, 4, 6])
        self.assertEqual(status(r, 3), "NOT-EXERCISED")
        self.assertEqual(r["exit"], 0)
        self.assertEqual(r["unknown_lines"], 0)
        self.assertEqual(r["ident"]["banners"], [(1, "0.3.0", "1")])

    def test_run4_hold_not_exercised(self):
        r = run("run4_only.txt")
        self.assertEqual(status(r, 3), "NOT-EXERCISED")
        self.assertEqual(status(r, 1), "PASS")
        self.assertEqual(status(r, 6), "PARTIAL")   # first load after launch = g00000000, no restart proof here
        self.assertEqual(status(r, 5), "PARTIAL")
        self.assertEqual(r["exit"], 0)

    def test_run4_file_with_4b_tail_has_item6_pass(self):
        r = run("run4_only.txt", lo=1)
        self.assertEqual(r["replay"]["status"], "NOT-EXERCISED")
        r2 = L.run((FX / "run4_only.txt").read_text().splitlines() + (FX / "run4b.txt").read_text().splitlines())
        self.assertEqual(status(r2, 6), "PASS")

    def test_range_and_md5(self):
        r = run("run4b.txt", lo=5, expect_md5="deadbeef")
        self.assertEqual(r["events"], 20)
        self.assertEqual(r["ident"]["banners"], [])
        self.assertIsNone(r["ident"]["checks"][0]["ok"])   # unverifiable, not a FAIL
        self.assertEqual(r["exit"], 0)


class Synthetic(unittest.TestCase):
    def test_hold_guard_banked_pass(self):
        r = run("synth_hold_pass.txt")
        self.assertEqual(status(r, 3), "PASS")
        self.assertEqual(status(r, 2), "PASS")
        self.assertEqual(status(r, 6), "PASS")
        self.assertEqual(r["replay"]["status"], "PASS")
        self.assertEqual(r["exit"], 0)

    def test_replay_ok_and_unconfigured_pc(self):
        r = run("synth_replay_ok.txt")
        self.assertEqual(r["replay"]["status"], "PASS")
        self.assertEqual(r["replay"]["rows"][0]["offline"], "PATH")
        self.assertEqual(status(r, 5), "PASS")     # pc call, no DECIDE, no rewrite lines
        self.assertEqual(status(r, 4), "NOT-EXERCISED")   # the manual call is unconfigured (no DECIDE)
        self.assertEqual(r["exit"], 0)

    def test_replay_divergence_fails(self):
        r = run("synth_replay_diverge.txt")
        self.assertEqual(r["replay"]["status"], "FAIL")
        self.assertIn("replay", r["fails"])
        self.assertEqual(r["exit"], 1)

    def test_manual_applied_fails(self):
        r = run("synth_manual_applied.txt")
        self.assertEqual(status(r, 4), "FAIL")
        self.assertEqual(r["exit"], 1)

    def test_fail_case(self):
        r = run("synth_fail.txt")
        self.assertEqual(status(r, 3), "FAIL")
        self.assertEqual(status(r, 2), "FAIL")
        self.assertEqual(status(r, 6), "FAIL")
        self.assertEqual(status(r, 7), "FAIL")
        self.assertEqual(r["exit"], 1)

    def test_replay_missing_fields(self):
        lines = [x for x in (FX / "synth_replay_ok.txt").read_text().splitlines() if "CONFIG source=playthrough" not in x]
        r = L.run(lines)
        self.assertEqual(r["replay"]["status"], "NOT-POSSIBLE")
        self.assertIn("hold/pause", r["replay"]["note"])


class Scenario(unittest.TestCase):
    def _toml(self, text):
        import tempfile
        d = tempfile.mkdtemp()
        p = Path(d) / "s.toml"
        p.write_text(text)
        return str(p)

    def test_scenario_pass_and_fail(self):
        f = self._toml('[scenario.A]\ntitle="t"\nv_items=[3]\n[scenario.A.expect]\nevents=["^HOLD tag=atton", "BANKED"]\nabsent=["APPLIED .*atton"]\n'
                       '[scenario.B]\ntitle="u"\n[scenario.B.expect]\nabsent=["HOLD .*atton"]\n')
        a = run("synth_hold_pass.txt", scenario="A", scenarios_file=f)
        self.assertEqual(a["scenario"]["status"], "PASS")
        b = run("synth_hold_pass.txt", scenario="B", scenarios_file=f)   # 'HOLD .*atton' also hits UNHOLD tag=atton: a known pitfall
        self.assertEqual(b["scenario"]["status"], "FAIL")
        self.assertEqual(b["exit"], 1)

    def test_missing_file_ok(self):
        r = run("run4b.txt", scenario="X", scenarios_file="/nonexistent.toml")
        self.assertEqual(r["scenario"]["status"], "SKIPPED")
        self.assertEqual(r["exit"], 0)

    def test_repo_scenarios_file_parses(self):
        if L.DEFAULT_SCENARIOS.exists():
            r = run("synth_hold_pass.txt", scenario="S1_atton_hold_default")
            self.assertIn(r["scenario"]["status"], ("PASS", "FAIL"))
            self.assertEqual(r["scenario"]["checks"][0]["type"], "event")


class Cli(unittest.TestCase):
    def test_main_exit_codes(self):
        import contextlib
        import io
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(L.main(["--log", str(FX / "run4b.txt"), "--from", "1"]), 0)
            self.assertEqual(L.main(["--log", str(FX / "synth_fail.txt"), "--json"]), 1)


if __name__ == "__main__":
    unittest.main()
