#!/usr/bin/env python3
"""Tests for scripts/kotor2_logs.py (route() is unit-tested without executing; status/check use a temp game dir)."""
import contextlib
import io
import os
import shutil
import sys
import tempfile
import time
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kotor2_logs as K  # noqa: E402

LIVE = ["buffhud_log.txt", "perflab_log.txt", "perflab_metrics.txt", "kpatch.log", "force_power_hotbar_log.txt", "action_hotbar_log.txt",
        "k2-dialogue-letterbox-fix_log.txt", "levelup_override_log.txt", "inv3d_log.txt", "mouse-probe_log.txt"]
ROTATED = ["force_power_hotbar_log.1.txt", "action_hotbar_log.1.txt", "force_power_hotbar_log.build1b_test.txt", "force_power_hotbar_log.build3_mouse_report.txt",
           "inv3d_log_235439_prev.txt", "inv3d_log_v1_20260929_213925.txt", "mouseprobe_log.run23.txt", "shieldprobe_log.crash22.txt",
           "shieldprobe_log_pre11_1805.txt", "shieldprobe_log_pre_counterbump.txt", "shieldprobe_log.prev21.txt", "shieldprobe_log_session4a.txt",
           "buffhud_log_b7dbebe9_baseline.txt", "buffhud_log.old.txt", "mouseprobe_log.txt.bak"]


class Tmp(unittest.TestCase):
    def setUp(self):
        self.g = tempfile.mkdtemp(prefix="k2logs_test_")
        self.addCleanup(shutil.rmtree, self.g, ignore_errors=True)

    def w(self, name, text="line1\nline2\n", age_h=0.0):
        p = os.path.join(self.g, name)
        with open(p, "w") as f:
            f.write(text)
        t = time.time() - age_h * 3600
        os.utime(p, (t, t))
        return p


class Rules(unittest.TestCase):
    def test_live_vs_rotated(self):
        for n in LIVE:
            self.assertTrue(K.is_live_log(n), n)
        for n in ROTATED:
            self.assertFalse(K.is_live_log(n), n)
        self.assertFalse(K.is_live_log("ReShade.log"))
        self.assertFalse(K.is_live_log("AR_ERROR.LOG"))


class CheckHelp(unittest.TestCase):
    def test_check_help_lists_passthrough_flags(self):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = K.main(["check", "--help"])
        self.assertEqual(rc, 0)
        for flag in ("--new", "--no-advance", "--all-launches", "--launch", "--game-dir"):
            self.assertIn(flag, buf.getvalue())


class Routing(Tmp):
    def argv(self, name, text="x\n"):
        self.w(name, text)
        return K.route(name, self.g)

    def test_engfix(self):
        a = self.argv("k2-dialogue-letterbox-fix_log.txt", "plain\n")
        self.assertTrue(a[1].endswith("patch_manager_mods/engine_bug_research/tools/analyze_engfix_logs.py"))
        self.assertEqual(a[2], os.path.join(self.g, "k2-dialogue-letterbox-fix_log.txt"))

    def test_luo(self):
        a = self.argv("levelup_override_log.txt")
        self.assertTrue(a[1].endswith("scripts/luo_log_check.py"))
        self.assertEqual(a[2:], ["--log", os.path.join(self.g, "levelup_override_log.txt")])

    def test_runs_class(self):
        for n in ("perflab_log.txt", "perflab_metrics.txt", "robustnessfixes_log.txt", "texmem_log.txt", "activationfix_log.txt",
                  "equipiconfix_log.txt", "k2-texture-memory-release_log.txt", "k2-activation-fix_log.txt", "k2-equip-icon-fix_log.txt",
                  "perf-lab_log.txt", "k2-robustness-fixes_log.txt"):
            a = self.argv(n)
            self.assertTrue(a[1].endswith("patch_manager_mods/perf_lab_src/analyze_runs.py"), n)
            self.assertEqual(a[2:], ["--game-dir", self.g, "--summary", "--latest", "1"], n)

    def test_playlogs_class(self):
        for n in ("ai_trace.txt", "ranged_ai_log.txt", "ranged-ai_log.txt", "inventory_sort_filter_log.txt"):
            a = self.argv(n)
            self.assertTrue(a[1].endswith("kotor2_logs.py"), n)
            self.assertEqual(a[2:], ["_playlogs", self.g], n)

    def test_kplog_header_goes_to_triage(self):
        a = self.argv("portrait-slots_log.txt", "# kplog session 2026-10-04 | kpatch.log v1\nx\n")
        self.assertTrue(a[1].endswith("scripts/log_triage.py"))
        self.assertEqual(K.parser_label(a[2]), "log_triage+kplog")

    def test_default_triage(self):
        a = self.argv("buffhud_log.txt", "buffhud loaded\n")
        self.assertTrue(a[1].endswith("scripts/log_triage.py"))
        self.assertEqual(a[2], os.path.join(self.g, "buffhud_log.txt"))

    def test_path_name_not_joined(self):
        p = self.w("levelup_override_log.txt")
        self.assertEqual(K.route(p, "/nonexistent")[3], p)

    def test_routes_table_printed(self):
        buf = io.StringIO()
        K.print_routes(buf)
        t = buf.getvalue()
        self.assertIn("perflab_metrics.txt -> analyze_runs", t)
        self.assertIn("levelup_override_log.txt -> luo_log_check", t)
        self.assertEqual(t.splitlines(), sorted(t.splitlines()[:-2]) + t.splitlines()[-2:])


class Status(Tmp):
    def run_status(self, hours=24):
        buf = io.StringIO()
        rc = K.status(self.g, hours, out=buf)
        return rc, buf.getvalue().splitlines()

    def test_skip_stale_empty_and_columns(self):
        self.w("buffhud_log.txt", "a\nb\nc\n", age_h=3.2)
        self.w("perflab_log.txt", "x\n", age_h=0.1)
        self.w("old_log.txt", "x\n", age_h=100)
        self.w("older_log.txt", "x\n", age_h=200)
        self.w("empty_log.txt", "")
        self.w("notes.txt", "x\n")
        for n in ROTATED[:5]:
            self.w(n)
        rc, ls = self.run_status()
        self.assertEqual(rc, 0)
        self.assertEqual(ls[0], "buffhud_log.txt  1KB  3h12m  3L  log_triage")
        self.assertTrue(ls[1].startswith("perflab_log.txt  1KB  6m  1L  analyze_runs"), ls[1])
        self.assertEqual(ls[2:], ["stale: 2 logs", "skipped: 5 rotated/archived"])

    def test_since_hours(self):
        self.w("buffhud_log.txt", age_h=3)
        self.assertEqual(self.run_status(2)[1][0], "stale: 1 logs")
        self.assertTrue(self.run_status(4)[1][0].startswith("buffhud_log.txt"))

    def test_missing_dir(self):
        buf = io.StringIO()
        self.assertEqual(K.status(os.path.join(self.g, "nope"), 24, out=buf), 2)


class Check(Tmp):
    def test_missing_rc2(self):
        with contextlib.redirect_stdout(io.StringIO()) as o:
            self.assertEqual(K.main(["check", "nope_log.txt", "--game-dir", self.g]), 2)
        self.assertEqual(o.getvalue(), "missing: nope_log.txt\n")

    def test_check_passthrough_rc_and_extra_args(self):
        self.w("buffhud_log.txt", "buffhud loaded\nERROR bad thing\n")
        import subprocess
        r = subprocess.run([sys.executable, str(HERE / "kotor2_logs.py"), "check", "buffhud_log.txt", "--game-dir", self.g, "--json"],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0)
        self.assertTrue(r.stdout.startswith("{"))

    def test_check_runs_triage_on_temp_log(self):
        import subprocess
        self.w("buffhud_log.txt", "buffhud loaded\nERROR bad thing\n")
        r = subprocess.run([sys.executable, str(HERE / "kotor2_logs.py"), "check", "buffhud_log.txt", "--game-dir", self.g],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0)
        self.assertIn("VERDICT: ALERTS(1)", r.stdout)

    def test_kplog_hint(self):
        import subprocess
        self.w("x_log.txt", "# kplog session 2026-10-04 00:00:00 | kpatch.log v1\nx loaded\n")
        r = subprocess.run([sys.executable, str(HERE / "kotor2_logs.py"), "check", "x_log.txt", "--game-dir", self.g],
                           capture_output=True, text=True)
        self.assertEqual(r.stdout.splitlines()[-1], K.KPLOG_HINT)

    def test_cli_routes_and_unknown(self):
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(K.main(["routes"]), 0)
            self.assertEqual(K.main(["bogus"]), 2)


if __name__ == "__main__":
    unittest.main()
