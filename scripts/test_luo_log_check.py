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

    def test_range_and_crc(self):
        r = run("run4b.txt", lo=5, expect_crc="deadbeef")
        self.assertEqual(r["events"], 20)
        self.assertEqual(r["ident"]["banners"], [])
        self.assertIsNone(r["ident"]["checks"][0]["ok"])   # unverifiable, not a FAIL
        self.assertEqual(r["exit"], 0)

    def test_config_warning_and_hold_suppressed_suffix(self):
        r = run("synth_config_warning.txt")
        self.assertEqual(r["unknown_lines"], 0)
        self.assertEqual(r["kinds"].get("CONFIG_WARNING"), 1)
        self.assertEqual(r["kinds"].get("HOLD"), 2)
        self.assertEqual(r["kinds"].get("UNHOLD"), 1)

    def test_log_cap_marker_parses_and_fails_v7(self):
        # KpLog.h writes one raw `--- log cap reached ---` line (no timestamp); the old `== log line cap reached ==` grammar never matched it.
        r = run("synth_log_cap.txt")
        self.assertEqual(r["unknown_lines"], 0)
        self.assertEqual(r["kinds"].get("LOG_CAP"), 1)
        self.assertEqual(status(r, 7), "FAIL")

    def test_instant_apply_now_lines_parse(self):
        r = run("synth_apply_now.txt")
        self.assertEqual(r["unknown_lines"], 0)
        self.assertEqual(r["kinds"].get("INSTANT_APPLYNOW"), 3)
        self.assertEqual(r["kinds"].get("INSTANT"), 1)

    def test_banner_crc32(self):
        r = run("synth_banner_crc.txt", expect_crc="cbf43926", expect_version="1.0.0")
        self.assertEqual(r["ident"]["banners"], [(1, "1.0.0", "1")])
        self.assertEqual(r["ident"]["crc32"], [(1, "cbf43926")])
        self.assertEqual([c["ok"] for c in r["ident"]["checks"]], [True, True])
        self.assertEqual(r["unknown_lines"], 0)
        r = run("synth_banner_crc.txt", expect_crc="00000001")
        self.assertIs(r["ident"]["checks"][0]["ok"], False)
        self.assertEqual(r["exit"], 1)   # a crc32 mismatch is a failing identity check
        r = run("run4b.txt", expect_crc="cbf43926")   # a pre-1.0.0 banner: unverifiable, not a FAIL
        self.assertIsNone(r["ident"]["checks"][0]["ok"])


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

    def test_portrait_refresh_via_autolevelup_is_auto(self):
        # leader instant level-up: flow=portrait-refresh but the call stack has (AutoLevelup) -> auto, V4 must not FAIL
        r = run("synth_portrait_refresh_auto.txt")
        self.assertEqual(status(r, 4), "NOT-EXERCISED")
        self.assertNotIn(4, r["fails"])

    def test_fail_case(self):
        r = run("synth_fail.txt")
        self.assertEqual(status(r, 3), "FAIL")
        self.assertEqual(status(r, 2), "FAIL")
        self.assertEqual(status(r, 6), "FAIL")
        self.assertEqual(status(r, 7), "FAIL")
        self.assertEqual(r["exit"], 1)

    def test_carry_lines_parse(self):
        r = run("synth_carry.txt")
        self.assertEqual(r["unknown_lines"], 0)
        self.assertEqual(r["kinds"].get("CARRY"), 2)
        self.assertEqual(r["kinds"].get("CARRYREAD"), 1)
        self.assertEqual(r["kinds"].get("CARRY_DISABLED"), 2)   # no rule sections + malformed rule line N
        self.assertEqual(r["exit"], 0)

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

    def test_bookmark_new_and_advance(self):
        import contextlib
        import io
        import tempfile
        d = tempfile.mkdtemp()
        try:
            log_p = Path(d) / "test_log.txt"
            state_p = Path(d) / "bm.json"
            log_p.write_text("line 1\nline 2\nline 3\n")

            with contextlib.redirect_stdout(io.StringIO()):
                rc = L.main(["--log", str(log_p), "--new", "--state", str(state_p), "--json"])
                self.assertEqual(rc, 0)

            # Check that state was written with line 3
            bm = L.load_bookmarks(state_p)
            self.assertEqual(bm[str(log_p.resolve())]["line"], 3)

            # Append two lines
            with open(log_p, "a") as f:
                f.write("line 4\nline 5\n")

            # Run with --new --no-advance
            with contextlib.redirect_stdout(io.StringIO()) as out:
                rc = L.main(["--log", str(log_p), "--new", "--no-advance", "--state", str(state_p), "--json"])
                self.assertEqual(rc, 0)
                import json
                data = json.loads(out.getvalue())
                self.assertEqual(data["range"], [4, 5])

            # Verify bookmark was NOT advanced
            bm2 = L.load_bookmarks(state_p)
            self.assertEqual(bm2[str(log_p.resolve())]["line"], 3)

            # Run with --new (should advance)
            with contextlib.redirect_stdout(io.StringIO()) as out:
                rc = L.main(["--log", str(log_p), "--new", "--state", str(state_p), "--json"])
                self.assertEqual(rc, 0)
                data = json.loads(out.getvalue())
                self.assertEqual(data["range"], [4, 5])

            bm3 = L.load_bookmarks(state_p)
            self.assertEqual(bm3[str(log_p.resolve())]["line"], 5)
        finally:
            import shutil
            shutil.rmtree(d, ignore_errors=True)


if __name__ == "__main__":
    unittest.main()
