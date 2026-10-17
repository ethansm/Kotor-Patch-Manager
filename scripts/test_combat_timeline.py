#!/usr/bin/env python3
"""Tests for scripts/combat_timeline.py and the `check combat` route in scripts/kotor2_logs.py. Synthetic fixtures only.
Run: python3 scripts/test_combat_timeline.py   (run_tests.py discovers it as py:scripts/test_combat_timeline.py)"""
import contextlib
import io
import json
import os
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import combat_timeline as C  # noqa: E402

MODE = "MODE m=0 combat=%s soft=0 menu=0 paused=0 hostile=1 party=1 strip=none cancel=none banner=none why=%s"
CS = "CSTATE combat=%s>%s paused=0>0 menu=0>0"


def ln(patch, t, body):
    return "T=%.3f f=- %s/main %s" % (t, patch, body)


class Tmp(unittest.TestCase):
    def setUp(self):
        self.d = tempfile.mkdtemp(prefix="combat_tl_")
        self.addCleanup(shutil.rmtree, self.d, ignore_errors=True)

    def w(self, name, lines, hdr="2026-10-17 12:00:00"):
        p = os.path.join(self.d, name)
        with open(p, "w") as f:
            f.write("# kplog session %s | kpatch.log v1 | patcher ABI 1\n" % hdr)
            f.write("\n".join(lines) + "\n")
        return p

    def run_ct(self, *args):
        o = io.StringIO()
        with contextlib.redirect_stdout(o):
            rc = C.main(list(args), out=o)
        return rc, o.getvalue()

    def lag_fixture(self, lag, extra_ai=()):
        """MODE + hotbar rise at T=2000, companion-ai rises `lag` ms later; all fall at 4000 (ai at 4000)."""
        self.w("queue-hud_log.txt", [ln("queue-hud", 1000, MODE % ("0", "init")), ln("queue-hud", 2000, MODE % ("1", "combat")),
                                     ln("queue-hud", 4000, MODE % ("0", "clear"))])
        self.w("combat-hotbar_log.txt", [ln("combat-hotbar", 1100, CS % ("?", "0")), ln("combat-hotbar", 2000, CS % ("0", "1")),
                                         ln("combat-hotbar", 4000, CS % ("1", "0"))])
        self.w("companion-ai_log.txt", [ln("companion-ai", 1200, CS % ("?", "0")), ln("companion-ai", 2000 + lag, CS % ("0", "1")),
                                        *extra_ai, ln("companion-ai", 4000, CS % ("1", "0"))])


class Basic(Tmp):
    def fixture(self):
        self.w("queue-hud_log.txt", [
            ln("queue-hud", 1000, MODE % ("0", "init")),
            ln("queue-hud", 2000, "EDGE what=combat old=0 new=1 src=hud buf=3,0,1"),
            ln("queue-hud", 2000.1, MODE % ("1", "combat")),
            ln("queue-hud", 2500, "HANDOFF m=0 active=5 buffered=2 cmds=6,7"),
            ln("queue-hud", 3000, "ROUND m=0 n=2 subs=1,10 cur=- ne=3 types=3f,01,0d"),
            ln("queue-hud", 3500, "BANNER act=hide soft=1 ctl=8 wr=8"),
            ln("queue-hud", 5000, "EDGE what=combat old=1 new=0 src=hud buf="),
            ln("queue-hud", 5000.1, MODE % ("0", "clear")),
            ln("queue-hud", 8000, "EDGE what=combat old=0 new=1 src=hud buf="),
            ln("queue-hud", 8000.1, MODE % ("1", "combat")),
            ln("queue-hud", 9000, MODE % ("0", "clear")),
        ])
        self.w("combat-hotbar_log.txt", [
            ln("combat-hotbar", 1200, CS % ("?", "0")),
            ln("combat-hotbar", 2000.2, CS % ("0", "1")),
            ln("combat-hotbar", 2100, "CSTATE cap reached"),
            ln("combat-hotbar", 5000.15, CS % ("1", "0")),
            ln("combat-hotbar", 8000.2, CS % ("0", "1")),
            ln("combat-hotbar", 9000.1, CS % ("1", "0")),
        ])
        self.w("companion-ai_log.txt", [
            ln("companion-ai", 1300, CS % ("?", "0")),
            ln("companion-ai", 2000.3, CS % ("0", "1")),
            ln("companion-ai", 2000.7, "t=2000 E1 SPIN cr=00001234 Bastila party=1 stance=0"),
            ln("companion-ai", 2800, "E2 FALLBACK cr=00001234 Bastila party=1 combat=1 round=1"),
            ln("companion-ai", 5000.2, CS % ("1", "0")),
            ln("companion-ai", 8000.3, CS % ("0", "1")),
            ln("companion-ai", 9000.2, CS % ("1", "0")),
        ])

    def test_three_mods_two_combats_ordered(self):
        self.fixture()
        rc, out = self.run_ct("--dir", self.d)
        self.assertEqual(rc, 0)
        self.assertIn("COMBAT #1 T=2000.000..5000.200 dur=3.000s", out)
        self.assertIn("COMBAT #2 T=8000.000..9000.200 dur=1.000s", out)
        body = out.split("COMBAT #1")[1].split("COMBAT #2")[0]
        times = [float(x.split()[0]) for x in body.splitlines()[1:] if x[:1].isdigit()]
        self.assertEqual(times, sorted(times))
        # first window: edge (2000), mode, cstate x2, handoff, round, banner, e1, e2, falls
        mods = [x.split()[1] for x in body.splitlines()[1:] if x[:1].isdigit()]
        self.assertEqual({"queue-hud", "combat-hotbar", "companion-ai"}, set(mods))
        self.assertIn("2000.700  companion-ai  E1 SPIN cr=00001234", body)  # t= stamp dropped
        self.assertIn("E2 FALLBACK", body)
        self.assertIn("HANDOFF m=0 active=5", body)
        self.assertIn("ROUND m=0 n=2", body)
        self.assertIn("BANNER act=hide", body)
        self.assertLess(body.index("EDGE what=combat old=0 new=1"), body.index("MODE m=0 combat=1"))
        self.assertIn("SUMMARY combats=2 lines=", out)

    def test_edge_only_log_closes_on_falling_edge(self):
        self.w("queue-hud_log.txt", [ln("queue-hud", 1000, "EDGE what=combat old=0 new=1 src=hud buf="),
                                     ln("queue-hud", 3000, "EDGE what=combat old=1 new=0 src=hud buf=")])
        _, out = self.run_ct("--dir", self.d)
        self.assertIn("COMBAT #1 T=1000.000..3000.000 dur=2.000s\n", out)

    def test_cap_reached_ignored(self):
        self.fixture()
        _, out = self.run_ct("--dir", self.d)
        self.assertNotIn("cap reached", out)

    def test_window_not_closed_by_first_falling_edge(self):
        self.fixture()
        _, out = self.run_ct("--dir", self.d, "--combat", "1")
        self.assertIn("5000.200", out)   # last source (companion-ai) closes the window, not the 5000.000 EDGE

    def test_combat_selector(self):
        self.fixture()
        rc, out = self.run_ct("--dir", self.d, "--combat", "2")
        self.assertEqual(rc, 0)
        self.assertNotIn("COMBAT #1", out)
        self.assertIn("COMBAT #2 T=8000.000", out)
        self.assertIn("SUMMARY combats=1", out)

    def test_max_lines_truncates(self):
        self.fixture()
        _, out = self.run_ct("--dir", self.d, "--combat", "1", "--max-lines", "3")
        body = [x for x in out.splitlines() if x[:1].isdigit()]
        self.assertEqual(len(body), 3)
        self.assertRegex(out, r"\(\+\d+ more\)")
        _, full = self.run_ct("--dir", self.d, "--combat", "1", "--max-lines", "100")
        self.assertNotIn("more)", full)

    def test_explicit_files_and_json(self):
        self.fixture()
        files = [os.path.join(self.d, n) for n in ("queue-hud_log.txt", "combat-hotbar_log.txt", "companion-ai_log.txt")]
        rc, out = self.run_ct(*files, "--json", "--max-lines", "2")
        self.assertEqual(rc, 0)
        j = json.loads(out)
        self.assertEqual(j["summary"]["combats"], 2)
        self.assertEqual(len(j["combats"][0]["lines"]), 2)
        self.assertGreater(j["combats"][0]["more"], 0)


class Disagree(Tmp):
    def count(self, lag):
        self.lag_fixture(lag)
        rc, out = self.run_ct("--dir", self.d)
        self.assertEqual(rc, 0)
        return out

    def test_300ms_flagged(self):
        out = self.count(300)
        self.assertIn("DISAGREE T=2000.000..2300.000 dur=300ms combat=1:[combat-hotbar,queue-hud/MODE] combat=0:[companion-ai]", out)
        self.assertIn("disagreements=1", out)

    def test_200ms_not_flagged(self):
        out = self.count(200)
        self.assertNotIn("DISAGREE", out)
        self.assertIn("disagreements=0", out)

    def test_exactly_250ms_not_flagged(self):
        self.assertNotIn("DISAGREE", self.count(250))

    def test_question_mark_does_not_vote(self):
        self.w("queue-hud_log.txt", [ln("queue-hud", 2000, MODE % ("1", "combat")), ln("queue-hud", 4000, MODE % ("0", "clear"))])
        self.w("combat-hotbar_log.txt", [ln("combat-hotbar", 1500, CS % ("?", "?")), ln("combat-hotbar", 4000, CS % ("?", "0"))])
        _, out = self.run_ct("--dir", self.d)
        self.assertIn("COMBAT #1", out)
        self.assertNotIn("DISAGREE", out)

    def test_other_member_mode_is_not_a_source(self):
        self.w("queue-hud_log.txt", [ln("queue-hud", 1000, MODE % ("0", "init")), ln("queue-hud", 2000, MODE % ("1", "combat")),
                                     ln("queue-hud", 2000.5, "MODE m=1 combat=0 soft=0 menu=0 paused=0 hostile=1 party=1 strip=a cancel=b banner=c why=x"),
                                     ln("queue-hud", 4000, MODE % ("0", "clear"))])
        _, out = self.run_ct("--dir", self.d)
        self.assertNotIn("DISAGREE", out)
        self.assertIn("MODE m=1 combat=0", out)


class Inputs(Tmp):
    def test_missing_file_rc2(self):
        rc, out = self.run_ct(os.path.join(self.d, "nope_log.txt"))
        self.assertEqual(rc, 2)
        self.assertIn("missing:", out)

    def test_missing_dir_rc2(self):
        rc, out = self.run_ct("--dir", os.path.join(self.d, "nodir"))
        self.assertEqual(rc, 2)
        self.assertIn("missing:", out)

    def test_empty_dir_rc2(self):
        rc, _ = self.run_ct("--dir", self.d)
        self.assertEqual(rc, 2)

    def test_bad_session_rc2(self):
        self.lag_fixture(0)
        rc, _ = self.run_ct("--dir", self.d, "--session", "5")
        self.assertEqual(rc, 2)

    def test_no_combat_rc0(self):
        self.w("queue-hud_log.txt", [ln("queue-hud", 1000, MODE % ("0", "init"))])
        rc, out = self.run_ct("--dir", self.d)
        self.assertEqual(rc, 0)
        self.assertIn("SUMMARY combats=0 lines=0 disagreements=0", out)

    def test_sessions_selected(self):
        self.w("queue-hud_log.txt", [ln("queue-hud", 1000, MODE % ("0", "init")), ln("queue-hud", 2000, MODE % ("1", "combat")),
                                     ln("queue-hud", 3000, MODE % ("0", "clear"))], hdr="2026-10-17 10:00:00")
        with open(os.path.join(self.d, "queue-hud_log.txt"), "a") as f:
            f.write("# kplog session 2026-10-17 12:00:00 | kpatch.log v1\n" + ln("queue-hud", 1000, MODE % ("0", "init")) + "\n")
        _, out = self.run_ct("--dir", self.d)
        self.assertIn("SESSION 2/2", out)
        self.assertIn("combats=0", out)
        _, out = self.run_ct("--dir", self.d, "--session", "1")
        self.assertIn("SESSION 1/2", out)
        self.assertIn("COMBAT #1", out)

    def test_route_runs_via_kotor2_logs(self):
        import subprocess
        self.lag_fixture(300)
        r = subprocess.run([sys.executable, str(HERE / "kotor2_logs.py"), "check", "combat", "--game-dir", self.d, "--combat", "1"],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0)
        self.assertIn("COMBAT #1", r.stdout)


if __name__ == "__main__":
    unittest.main()
