#!/usr/bin/env python3
"""Tests for analyze_engfix_logs.py. Run: python3 test_analyze_engfix_logs.py (from any cwd)."""
import contextlib
import io
import os
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
FIXTURES = os.path.join(HERE, "..", "fixtures")
sys.path.insert(0, HERE)
import analyze_engfix_logs as A  # noqa: E402

VANILLA = "00 00 00 80 aa aa 02 40"
T = [1000000]


def tick():
    T[0] += 7
    return "[%d] " % T[0]


def load(bytes_=VANILLA, mode="h6", pid=312):
    return tick() + ("k2-dialogue-letterbox-fix 1.0.0 loaded: enabled=1 mode=%s log=1 const@0x009a7d30=%s "
                     "(2.3333330154418945) pid=%d" % (mode, bytes_, pid))


def apply_lines(w=3840, h=1600, new="3.5999999964000002", vbar=-22, nbar=267, x=1066, fs="2.0", panel=200, reply=1333,
                protect_ok="protect RW ok, old prot=0x2; protect restore ok -> 0x2", verify="OK", src="SetScreenSize"):
    return [
        tick() + "APPLY src=%s W=%d H=%d mode=h6 old=2.3333330154418945 new=%s vanillaBar=%d newBar=%d X=%d fs=%s "
                 "panelH=%d replyY=%d" % (src, w, h, new, vbar, nbar, x, fs, panel, reply),
        tick() + "APPLY src=%s: wrote (%s); verify %s, now de 1a 51 cc cc cc 0c 40 = %s" % (src, protect_ok, verify, new),
    ]


def settop(h=267, w=3840, hh=1600, new="3.5999999964000002", sh=0):
    return tick() + ("SETTOP lb=0c92d1e8 mode(+0x88, before this store)=0 start(x,y,w,h)=(0,0,%d,%d) "
                     "target(x,y,w,h)=(0,0,%d,%d) mgr=05bc0fb8 W=%d H=%d c=%s" % (w, sh, w, h, w, hh, new))


def setbottom(y=1333, h=267, w=3840, hh=1600, new="3.5999999964000002", sy=None):
    if sy is None:
        sy = hh
    return tick() + ("SETBOTTOM lb=0c92d298 mode(+0x88, before this store)=0 start(x,y,w,h)=(0,%d,%d,%d) "
                     "target(x,y,w,h)=(0,%d,%d,%d) mgr=05bc0fb8 W=%d H=%d c=%s" % (sy, w, h, y, w, h, w, hh, new))


def run_text(text):
    launches, unparsed, total = A.analyze_text(text)
    ok, verdict, dialogues, errors = A.summarize(launches, unparsed, total)
    return ok, verdict, dialogues, errors, launches, unparsed


class FixtureTests(unittest.TestCase):
    def run_cli(self, path, *extra):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = A.main(["analyze_engfix_logs.py", path] + list(extra))
        return rc, buf.getvalue()

    def test_1002_0238_no_dialogue(self):
        p = os.path.join(FIXTURES, "letterbox_log_1002_0238.txt")
        with open(p) as f:
            self.assertEqual(f.read().count("SETTOP"), 0)
        rc, out = self.run_cli(p)
        self.assertEqual(rc, 0)
        self.assertIn("VERDICT: PASS - no dialogue yet", out)
        self.assertIn("4 launches", out.splitlines()[-1])

    def test_1003_0112_pass(self):
        p = os.path.join(FIXTURES, "letterbox_log_1003_0112.txt")
        rc, out = self.run_cli(p)
        self.assertEqual(rc, 0, out)
        launches, unparsed, total = A.analyze_file(p)
        ok, verdict, dialogues, errors = A.summarize(launches, unparsed, total)
        self.assertTrue(ok)
        self.assertEqual(dialogues, 12)
        self.assertEqual(len(launches), 13)  # 13 `loaded:` lines in the 63-line fixture (>= 4 required)
        self.assertEqual(errors, 0)
        self.assertEqual(unparsed, [])
        self.assertIn("VERDICT: PASS (13 launches, 12 dialogues, 0 errors)", out)

    def test_json(self):
        import json
        p = os.path.join(FIXTURES, "letterbox_log_1003_0112.txt")
        rc, out = self.run_cli(p, "--json")
        self.assertEqual(rc, 0)
        d = json.loads(out)
        self.assertEqual(d["dialogues"], 12)
        self.assertTrue(d["ok"])

    def test_cli_exit_code_fail_and_missing_file(self):
        with tempfile.TemporaryDirectory() as td:
            p = os.path.join(td, "bad.txt")
            with open(p, "w") as f:
                f.write("\n".join([load("00 00 00 00 00 00 00 40")] + apply_lines()) + "\n")
            rc, out = self.run_cli(p)
            self.assertEqual(rc, 1)
            self.assertIn("VERDICT: FAIL", out)
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(A.main(["x", os.path.join(td, "missing.txt")]), 2)


class SyntheticTests(unittest.TestCase):
    def test_a_pass_pair(self):
        ok, verdict, dlg, errs, launches, unparsed = run_text(
            "\n".join([load()] + apply_lines() + [settop(), setbottom()]))
        self.assertTrue(ok, verdict)
        self.assertEqual((dlg, errs, len(launches)), (1, 0, 1))
        self.assertEqual(verdict, "VERDICT: PASS (1 launches, 1 dialogues, 0 errors)")
        self.assertEqual(unparsed, [])

    def test_b_foreign(self):
        text = "\n".join([
            load(),
            tick() + "APPLY src=SetScreenSize W=3840 H=1600: FOREIGN value 00 00 00 00 00 00 00 40 2: "
                     "another mod owns 0x009a7d30, not writing"])
        ok, verdict, _, errs, _, _ = run_text(text)
        self.assertFalse(ok)
        self.assertGreaterEqual(errs, 1)
        self.assertIn("FOREIGN", verdict)

    def test_c_wrong_bar(self):
        ok, verdict, _, errs, launches, _ = run_text(
            "\n".join([load()] + apply_lines() + [settop(h=180), setbottom()]))
        self.assertFalse(ok)
        self.assertTrue(any("target h 180" in r for _, r, _ in launches[0].errors))

    def test_d_non_vanilla_start_bytes(self):
        ok, verdict, _, _, launches, _ = run_text(
            "\n".join([load("00 00 00 00 00 00 00 40")] + apply_lines()))
        self.assertFalse(ok)
        self.assertTrue(any("start bytes" in r for _, r, _ in launches[0].errors))

    def test_e_protect_failure(self):
        # restore failed (the line still contains "protect RW ok" but also FAILED)
        bad = apply_lines(protect_ok="protect RW ok, old prot=0x2; protect restore FAILED -> 0x0")
        ok, verdict, _, _, _, _ = run_text("\n".join([load()] + bad))
        self.assertFalse(ok)
        # VirtualProtect(RW) failure line
        text = "\n".join([load(), apply_lines()[0],
                          tick() + "APPLY src=SetScreenSize: VirtualProtect(RW) FAILED err=487, no write"])
        ok, verdict, _, _, _, _ = run_text(text)
        self.assertFalse(ok)
        # a bare protect line without ok
        text = "\n".join([load(), tick() + "protect restore went wrong"])
        self.assertFalse(run_text(text)[0])

    def test_success_line_is_not_a_protect_error(self):
        ok, _, _, errs, _, _ = run_text("\n".join([load()] + apply_lines()))
        self.assertTrue(ok)
        self.assertEqual(errs, 0)

    def test_f_setbottom_wrong_y(self):
        ok, verdict, _, _, launches, _ = run_text(
            "\n".join([load()] + apply_lines() + [settop(), setbottom(y=1400)]))
        self.assertFalse(ok)
        self.assertTrue(any("SETBOTTOM target y 1400" in r for _, r, _ in launches[0].errors))

    def test_g_1920x1080(self):
        # bar 180; per the DLL formula fs = 1 + 0.5*floor(max(1920-1022,0)/1024) = 1.0 (panel 100), y = 1080 - max(180, 100) = 900
        new = "2.6666666648888892"
        lines = [load()] + apply_lines(w=1920, h=1080, new=new, vbar=129, nbar=180, x=720, fs="1.0", panel=100, reply=900) \
            + [settop(h=180, w=1920, hh=1080, new=new), setbottom(y=900, h=180, w=1920, hh=1080, new=new)]
        ok, verdict, dlg, errs, _, _ = run_text("\n".join(lines))
        self.assertTrue(ok, verdict)
        self.assertEqual((dlg, errs), (1, 0))

    def test_g2_1920x1080_wrong_fs_rejected(self):
        # the user brief said fs 1.5 / panel 150 at 1920; the DLL formula gives 1.0 / 100, so 1.5 must be flagged
        new = "2.6666666648888892"
        lines = [load()] + apply_lines(w=1920, h=1080, new=new, vbar=129, nbar=180, x=720, fs="1.5", panel=150, reply=900)
        self.assertFalse(run_text("\n".join(lines))[0])

    def test_late_fallback(self):
        # SETTOP logged before the apply (old constant, vanilla bar -22), then the LATE notice + APPLY
        lines = [load(),
                 settop(h=-22, new="2.3333330154418945"),
                 tick() + "SETTOP: no SetScreenSize hit seen yet, applying late fallback (the bar just computed used the old constant)"] \
            + apply_lines(src="LATE(SetTop)") + [setbottom()]
        ok, verdict, dlg, errs, launches, unparsed = run_text("\n".join(lines))
        self.assertTrue(ok, verdict)
        self.assertEqual(launches[0].late, ["SetTop"])
        self.assertEqual(dlg, 1)
        self.assertEqual(unparsed, [])

    def test_cap_lines_are_flagged_not_failed(self):
        lines = [load()] + apply_lines() + [
            settop(), setbottom(),
            tick() + "SETTOP/SETBOTTOM log cap (400 lines) reached, further lines dropped",
            tick() + "log cap (5000 lines) reached, further lines dropped"]
        ok, verdict, _, _, launches, unparsed = run_text("\n".join(lines))
        self.assertTrue(ok, verdict)
        self.assertEqual(len(launches[0].cap), 2)
        self.assertIn("cap reached", verdict)
        self.assertEqual(unparsed, [])

    def test_null_manager_is_error(self):
        text = "\n".join([load(), tick() + "dlbOnScreenSize: manager pointer [ebp-0xC] is null, skipping"])
        self.assertFalse(run_text(text)[0])

    def test_invalid_size_is_error(self):
        text = "\n".join([load(), tick() + "APPLY src=SetScreenSize W=0 H=0: INVALID size, no write"])
        self.assertFalse(run_text(text)[0])

    def test_disabled_and_unchanged_pass(self):
        text = "\n".join([
            load(),
            tick() + "APPLY src=SetScreenSize W=3840 H=1600: DISABLED, no write",
            load(pid=313),
        ] + [apply_lines()[0], tick() + "APPLY src=SetScreenSize: unchanged (already 3.5999999964000002)"])
        ok, verdict, _, errs, launches, _ = run_text(text)
        self.assertTrue(ok, verdict)
        self.assertEqual(launches[1].applies[0]["result"], "unchanged")
        self.assertEqual(launches[0].applies[0]["result"], "disabled")

    def test_wrong_w_h_crosscheck(self):
        ok, _, _, _, launches, _ = run_text(
            "\n".join([load()] + apply_lines() + [settop(w=1920, hh=1080), setbottom()]))
        self.assertFalse(ok)

    def test_settop_start_h_nonzero(self):
        ok, _, _, _, _, _ = run_text("\n".join([load()] + apply_lines() + [settop(sh=5), setbottom()]))
        self.assertFalse(ok)

    def test_unparsed_is_warning_only(self):
        ok, verdict, _, _, _, unparsed = run_text(
            "\n".join([load()] + apply_lines() + [tick() + "something new the DLL never says", "garbage"]))
        self.assertTrue(ok, verdict)
        self.assertEqual(len(unparsed), 2)

    def test_verify_mismatch(self):
        bad = apply_lines(verify="MISMATCH") + [tick() + "APPLY src=SetScreenSize: verify MISMATCH after write"]
        self.assertFalse(run_text("\n".join([load()] + bad))[0])


if __name__ == "__main__":
    unittest.main(verbosity=2)
