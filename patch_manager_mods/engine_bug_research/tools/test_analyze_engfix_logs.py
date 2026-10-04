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


# ---------------------------------------------------------------------------------------------------------------------
# 1.0.1 log=0 and 1.0.1-probe (PROBE W1..W5) tests
# ---------------------------------------------------------------------------------------------------------------------
C36 = "3.5999999964000002"
C_1920 = "2.6666666640000001"   # the 1920x1080 constant the smoke harness leaves in its synthetic SETTOP/SETBOTTOM


def load2(version="1.0.1", log=0, probe=None, pid=32):
    return tick() + ("k2-dialogue-letterbox-fix %s loaded: enabled=1 mode=h6 log=%d %sconst@0x009a7d30=%s "
                     "(2.3333330154418945) pid=%d" % (version, log, "" if probe is None else "probe=%d " % probe, VANILLA, pid))


def log0_launch():
    """A realistic game log=0 launch: load, first APPLY pair, first SETTOP, first SETBOTTOM (consistent c)."""
    return [load2()] + apply_lines() + [settop(new=C36), setbottom(new=C36)]


R4 = "(%d,%d,%d,%d)"


def p_w1(n, h, done=0, alpha=0x3F000000, skip=0, unreadable=False):
    return tick() + ("PROBE W1 n=%d skip=%d this=0x0c92d1e8 mode=1 done=%d alpha=0x%08x passedRect(x,y,w,h)=%s ctrlRect(x,y,w,h)=%s "
                     "targetH(+0x74)=267%s" % (n, skip, done, alpha, R4 % (0, 0, 3840, h), R4 % (0, 0, 3840, h),
                                                " [UNREADABLE]" if unreadable else ""))


def p_w2(n, y, done=0, alpha=0x3F000000, skip=0):
    return tick() + ("PROBE W2 n=%d skip=%d this=0x0c92d298 mode=2 done=%d alpha=0x%08x passedRect(x,y,w,h)=%s ctrlRect(x,y,w,h)=%s "
                     "targetH(+0x74)=267" % (n, skip, done, alpha, R4 % (0, y, 3840, 1600 - y), R4 % (0, y, 3840, 1600 - y)))


def p_w3(n, pred="WOULD-DRAW", alpha=0x3F000000, vt="letterbox", skip=0):
    vtbl = 0x009A7CBC if vt == "letterbox" else 0x00995D70
    return tick() + ("PROBE W3 n=%d skip=%d this=0x0c92d1e8 vtbl=0x%08x(%s) flags(+0x48)=0x00000044 bit2=1 bit6=1 rect(x,y,w,h)=%s "
                     "alpha=0x%08x color(+0x54..)=0x00000000,0x00000000,0x00000000 mgr=0x05bc0fb8 W=3840 H=1600 predicate=%s "
                     "dt=0x3c888889 DAT_a309b4=267 DAT_a309b8=1333 DAT_a309bc=1" % (n, skip, vtbl, vt, R4 % (0, 0, 3840, 267), alpha, pred))


def p_w3_null(n):
    return tick() + "PROBE W3 n=%d skip=0 this=NULL/UNREADABLE dt=0x3c888889" % n


def p_w4(n, eax=1, alpha=0x3F000000, skip=0):
    return tick() + ("PROBE W4 n=%d skip=%d this=0x0c92d1e8 mode=1 eax=%d fillRect(x,y,w,h)=%s alpha=0x%08x"
                     % (n, skip, eax, R4 % (0, 0, 3840, 267), alpha))


def p_w5(n, arg):
    return tick() + "PROBE W5 n=%d skip=0 arg=%d this=0x0b1b2c3c" % (n, arg)


def probe_dialogue(w1=True, w3=None, w4=None):
    """SETTOP + SETBOTTOM + probe lines. w3 / w4 = list of (pred, alpha) / eax values."""
    out = [settop(new=C36), setbottom(new=C36), p_w5(1, 1)]
    if w1:
        out += [p_w1(1, 8), p_w1(2, 16), p_w1(3, 267, done=1), p_w2(1, 1592), p_w2(2, 1333, done=1)]
    for i, (pred, alpha) in enumerate(w3 or [], 1):
        out.append(p_w3(i, pred=pred, alpha=alpha))
    for i, eax in enumerate(w4 or [], 1):
        out.append(p_w4(i, eax=eax))
    out.append(p_w5(2, 0))
    return out


def probe_run(*dialogue_blocks, pre=()):
    lines = [load2("1.0.1-probe", log=1, probe=1)] + apply_lines() + list(pre)
    for b in dialogue_blocks:
        lines += b
    return run_text("\n".join(lines))


HALF, ONE = 0x3F000000, 0x3F800000


class Log0Tests(unittest.TestCase):
    def test_log0_realistic_launch_passes(self):
        ok, verdict, dlg, errs, launches, unparsed = run_text("\n".join(log0_launch()))
        self.assertTrue(ok, verdict)
        self.assertEqual((dlg, errs, len(launches)), (1, 0, 1))
        self.assertEqual(launches[0].warnings, [])
        self.assertEqual(launches[0].log, 0)
        self.assertEqual(unparsed, [])
        self.assertEqual(verdict, "VERDICT: PASS (1 launches, 1 dialogues, 0 errors)")

    def test_log0_later_resolution_not_compared_with_last_apply(self):
        # log=0: a later resolution change (1920x1080) is not logged; the SET lines are consistent with themselves only
        s_top = settop(h=180, w=1920, hh=1080, new="2.6666666648888892")
        s_bot = setbottom(y=900, h=180, w=1920, hh=1080, new="2.6666666648888892")
        ok, verdict, _, errs, launches, _ = run_text("\n".join([load2()] + apply_lines() + [s_top, s_bot]))
        self.assertTrue(ok, verdict)
        self.assertEqual(launches[0].warnings, [])
        # the same lines in a log=1 launch keep failing (W/H and c vs the last APPLY)
        ok, _, _, _, _, _ = run_text("\n".join([load()] + apply_lines() + [s_top, s_bot]))
        self.assertFalse(ok)

    def test_log0_inconsistent_c_is_warn_only(self):
        ok, verdict, _, errs, launches, _ = run_text("\n".join(
            [load2()] + apply_lines() + [settop(new=C_1920), setbottom(new=C_1920)]))
        self.assertTrue(ok, verdict)
        self.assertEqual(errs, 0)
        self.assertEqual(len(launches[0].warnings), 2)
        self.assertTrue(all("c=2.6666666640000001" in w for _, w in launches[0].warnings))

    def test_log0_c_mismatch_with_last_apply_is_error_for_log1(self):
        ok, verdict, _, _, launches, _ = run_text("\n".join(
            [load()] + apply_lines() + [settop(new=C_1920), setbottom(new=C_1920)]))
        self.assertFalse(ok)
        self.assertTrue(any("c=2.6666666640000001 != APPLY new" in r for _, r, _ in launches[0].errors))

    def test_log0_still_checks_geometry(self):
        # wrong bottom y and nonzero start h remain errors for log=0
        ok, _, _, _, launches, _ = run_text("\n".join([load2()] + apply_lines() + [settop(new=C36), setbottom(y=1400, new=C36)]))
        self.assertFalse(ok)
        self.assertTrue(any("SETBOTTOM target y 1400" in r for _, r, _ in launches[0].errors))
        ok, _, _, _, _, _ = run_text("\n".join([load2()] + apply_lines() + [settop(sh=5, new=C36), setbottom(new=C36)]))
        self.assertFalse(ok)

    def test_log0_wrong_bar_vs_c_is_warn(self):
        # target h inconsistent with c (here h=180 with the 3840x1600 c) is a warning on log=0 (error on log=1, see test_c_wrong_bar)
        ok, verdict, _, errs, launches, _ = run_text("\n".join(
            [load2()] + apply_lines() + [settop(h=180, new=C36), setbottom(h=180, y=1400, new=C36)]))
        self.assertTrue(ok, verdict)
        self.assertEqual(len(launches[0].warnings), 2)

    def test_smoke_fixtures_exact_invalid_error_and_c_warn(self):
        for name in ("letterbox_log_101_smoke_default.txt", "letterbox_log_101_smoke_log0.txt"):
            launches, unparsed, total = A.analyze_file(os.path.join(FIXTURES, name))
            ok, verdict, dlg, errs = A.summarize(launches, unparsed, total)
            L = launches[0]
            self.assertFalse(ok, name)
            self.assertEqual(errs, 1, name)
            self.assertEqual([r for _, r, _ in L.errors], ["INVALID"], name)
            self.assertFalse(any("!= APPLY new" in r for _, r, _ in L.errors), name)
            self.assertEqual(len(L.warnings), 2, name)
            self.assertTrue(all("c=2.6666666640000001" in w for _, w in L.warnings), name)
            self.assertEqual(dlg, 1)
            self.assertEqual(unparsed, [])

    def test_smoke_fixtures_without_invalid_line_pass(self):
        for name in ("letterbox_log_101_smoke_default.txt", "letterbox_log_101_smoke_log0.txt"):
            with open(os.path.join(FIXTURES, name)) as f:
                lines = [ln for ln in f.read().splitlines() if "INVALID" not in ln]
            ok, verdict, _, errs, launches, _ = run_text("\n".join(lines))
            self.assertTrue(ok, name + verdict)
            self.assertEqual(len(launches[0].warnings), 2)

    def test_version_and_probe_field_in_load_line(self):
        for version, probe in (("1.0.1", None), ("1.0.1-probe", 0), ("1.0.1-probe", 1)):
            ok, verdict, _, errs, launches, unparsed = run_text("\n".join(
                [load2(version, log=1, probe=probe)] + apply_lines() + [settop(new=C36), setbottom(new=C36)]))
            self.assertTrue(ok, "%s %s: %s" % (version, probe, verdict))
            self.assertEqual(launches[0].probe, probe)
            self.assertEqual(launches[0].pid, 32)

    def test_probe_zero_with_probe_lines_warns(self):
        _, _, _, _, launches, _ = run_text("\n".join(
            [load2("1.0.1-probe", log=1, probe=0)] + apply_lines() + [settop(new=C36), p_w5(1, 1)]))
        self.assertTrue(any("probe=0" in w for _, w in launches[0].warnings))


class ProbeTests(unittest.TestCase):
    def diag(self, *blocks, **kw):
        ok, verdict, dlg, errs, launches, unparsed = probe_run(*blocks, **kw)
        self.assertTrue(ok, verdict)
        self.assertEqual(unparsed, [])
        return launches[0], [A.probe_summary(b)["diagnosis"] for b in launches[0].buckets]

    def test_diag_draw_never_called(self):
        _, d = self.diag(probe_dialogue(w1=False))
        self.assertEqual(d, ["Draw never called on the bar"])

    def test_diag_predicate_fails_most_common_term(self):
        _, d = self.diag(probe_dialogue(w3=[("FAIL:x+w<=W", HALF), ("FAIL:bit6", HALF), ("FAIL:x+w<=W", HALF)]))
        self.assertEqual(d, ["predicate fails: FAIL:x+w<=W"])

    def test_diag_predicate_fails_ignores_other_vtable_would_draw(self):
        # a WOULD-DRAW for a non-letterbox vtable must not count
        blk = probe_dialogue(w3=[("FAIL:h>0", HALF)]) + [p_w3(9, pred="WOULD-DRAW", vt="OTHER")]
        _, d = self.diag(blk)
        self.assertEqual(d, ["predicate fails: FAIL:h>0"])

    def test_diag_alpha_zero(self):
        _, d = self.diag(probe_dialogue(w3=[("WOULD-DRAW", 0)] * 2, w4=[1, 1]))
        self.assertEqual(d, ["alpha 0"])

    def test_diag_fill_rejected(self):
        _, d = self.diag(probe_dialogue(w3=[("WOULD-DRAW", HALF)] * 2, w4=[0, 0]))
        self.assertEqual(d, ["fill rejected (eax=0)"])

    def test_diag_bars_drawn(self):
        L, d = self.diag(probe_dialogue(w3=[("WOULD-DRAW", HALF)] * 3, w4=[1, 1, 1]))
        self.assertEqual(d, ["bars drawn (fill issued)"])
        ps = A.probe_summary(L.buckets[0])
        self.assertEqual(ps["W1"]["h_first"], [8, 16, 267])
        self.assertEqual(ps["W1"]["h_final"], 267)
        self.assertTrue(ps["W1"]["done_seen"])
        self.assertEqual(ps["W2"]["y_final"], 1333)
        self.assertEqual(ps["W2"]["y_first"][0], 1592)
        self.assertEqual(ps["W3"]["predicate"], {"WOULD-DRAW": 3})
        self.assertEqual((ps["W3"]["vtbl_letterbox"], ps["W3"]["vtbl_other"]), (3, 0))
        self.assertEqual(ps["W3"]["alpha"], {"0x3F000000 0.5 (semi-transparent letterbox)": 3})
        self.assertEqual((ps["W4"]["eax1"], ps["W4"]["eax0"]), (3, 0))
        self.assertEqual(ps["W5"]["args"], [1, 0])
        self.assertTrue(ps["W5"]["start_arg1"] and ps["W5"]["end_arg0"])

    def test_diag_first_rule_wins_mixed(self):
        # eax=0 and eax=1 both present with WOULD-DRAW -> bars drawn (rule 4 needs "no eax=1")
        _, d = self.diag(probe_dialogue(w3=[("WOULD-DRAW", ONE)], w4=[0, 1]))
        self.assertEqual(d, ["bars drawn (fill issued)"])

    def test_diag_inconclusive_without_w4(self):
        _, d = self.diag(probe_dialogue(w3=[("WOULD-DRAW", HALF)], w4=[]))
        self.assertTrue(d[0].startswith("inconclusive"))

    def test_two_dialogues_and_pre_dialogue_bucket(self):
        pre = [p_w5(1, 0), p_w3(1)]
        a = probe_dialogue(w3=[("WOULD-DRAW", HALF)], w4=[1])
        b = probe_dialogue(w1=False)
        L, d = self.diag(a, b, pre=pre)
        self.assertEqual(d, ["bars drawn (fill issued)", "Draw never called on the bar"])
        self.assertEqual(len(L.pre_bucket["W5"]), 1)
        self.assertEqual(len(L.pre_bucket["W3"]), 1)
        self.assertEqual(len(L.buckets[0]["W1"]), 3)
        self.assertEqual(len(L.buckets[1]["W1"]), 0)

    def test_probe_lines_never_fail_a_good_launch(self):
        blk = probe_dialogue(w3=[("WOULD-DRAW", HALF)], w4=[1])
        blk += [p_w1(4, 267, done=1, unreadable=True), p_w3_null(9),
                tick() + "PROBE cap (3000 lines) reached, further PROBE lines dropped"]
        ok, verdict, dlg, errs, launches, unparsed = probe_run(blk)
        self.assertTrue(ok, verdict)
        self.assertEqual(errs, 0)
        self.assertEqual(unparsed, [])
        self.assertIn("probe-cap(3000)", launches[0].cap)
        self.assertIn("cap reached", verdict)
        self.assertTrue(any("unreadable" in w for _, w in launches[0].warnings))

    def test_rate_limited_skip_field_accepted(self):
        blk = [settop(new=C36), setbottom(new=C36), p_w1(121, 267, done=1, skip=57), p_w3(121, skip=3), p_w4(121, skip=12)]
        ok, verdict, _, _, launches, unparsed = probe_run(blk)
        self.assertTrue(ok, verdict)
        self.assertEqual(unparsed, [])
        self.assertEqual(launches[0].buckets[0]["W1"][0]["skip"], 57)

    def test_unknown_probe_line_is_unparsed_warning_only(self):
        ok, verdict, _, _, _, unparsed = probe_run([settop(new=C36), tick() + "PROBE W9 n=1 skip=0 whatever"])
        self.assertTrue(ok, verdict)
        self.assertEqual(len(unparsed), 1)

    def test_text_and_json_report_diagnosis(self):
        import json
        blk = probe_dialogue(w3=[("WOULD-DRAW", HALF)] * 2, w4=[1, 1])
        text = "\n".join([load2("1.0.1-probe", log=1, probe=1)] + apply_lines() + blk)
        with tempfile.TemporaryDirectory() as td:
            p = os.path.join(td, "probe.txt")
            with open(p, "w") as f:
                f.write(text + "\n")
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                self.assertEqual(A.main(["x", p]), 0)
            out = buf.getvalue()
            self.assertIn("DIAGNOSIS: bars drawn (fill issued)", out)
            self.assertIn("W1 top-bar h:", out)
            self.assertIn("done=1 seen", out)
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                self.assertEqual(A.main(["x", p, "--json"]), 0)
            d = json.loads(buf.getvalue())
            pd = d["launch_details"][0]["probe_dialogues"]
            self.assertEqual(len(pd), 1)
            self.assertEqual(pd[0]["diagnosis"], "bars drawn (fill issued)")
            self.assertEqual(d["launch_details"][0]["probe"], 1)

    def test_old_launch_without_probe_prints_no_probe_section(self):
        text = "\n".join([load()] + apply_lines() + [settop(), setbottom()])
        launches, unparsed, total = A.analyze_text(text)
        ok, verdict, dlg, _ = A.summarize(launches, unparsed, total)
        self.assertNotIn("Probe (", A.render_text("x", launches, unparsed, total, ok, verdict, dlg))


# ---------------------------------------------------------------------------------------------------------------------
# 1.0.2 (KpLog.h) prefixes: legacy / new core / old-core fallback, other-patch filtering, session header, W3 + W3b join
# ---------------------------------------------------------------------------------------------------------------------
import json  # noqa: E402
import re  # noqa: E402

PID = "k2-dialogue-letterbox-fix"
LEGACY_PREFIX = re.compile(r"^\[\d+\] ")


def bare(line):
    return LEGACY_PREFIX.sub("", line)


def channel_of(msg):
    if msg.startswith("PROBE W4 "):
        return "probe_w4"
    if msg.startswith("PROBE "):
        return "probe"
    if msg.startswith(("SETTOP", "SETBOTTOM")):
        return "set"
    return "main"


def core_line(msg, t, tag="", frame="-", patch=PID, ch=None):
    return "T=%d.%03d f=%s %s/%s %s%s" % (t // 1000, t % 1000, frame, patch, ch or channel_of(msg), tag, msg)


def fb_line(msg, t, tag="", patch=PID, ch=None):
    return "%d %s/%s %s%s" % (t, patch, ch or channel_of(msg), tag, msg)


def to_core(legacy_lines, header=True):
    out = ["# kplog session 2026-10-03 22:40:01 | kpatch.log v1 | patcher ABI 1", "# KOTOR_VERSION_SHA=abcdef"] if header else []
    for i, ln in enumerate(legacy_lines):
        m = bare(ln)
        out.append(core_line(m, 1500 + 7 * i, tag="ERR " if A.classify_error(m) and not m.startswith("PROBE") else "",
                             frame=str(i) if i % 2 else "-"))
    return out


def to_fallback(legacy_lines):
    return [fb_line(bare(ln), 2930091408 + 5 * i, tag="WARN " if A.classify_error(bare(ln)) and not bare(ln).startswith("PROBE") else "")
            for i, ln in enumerate(legacy_lines)]


def w3_split(n, pred="WOULD-DRAW", alpha=0x3F000000, vt="letterbox", skip=0, labels="short", unr=""):
    """(W3, W3b) bare messages, as the 1.0.2 probe prints them. labels='long' keeps the legacy `flags(+0x48)=`/`rect(x,y,w,h)=` labels."""
    vtbl = 0x009A7CBC if vt == "letterbox" else 0x00995D70
    fl, rc = ("flags(+0x48)", "rect(x,y,w,h)") if labels == "long" else ("flags", "rect")
    a = ("PROBE W3 n=%d skip=%d this=0x0c92d1e8 vtbl=0x%08x(%s) %s=0x00000044 bit2=1 bit6=1 %s=%s W=3840 H=1600 predicate=%s dt=0x3c888889"
         % (n, skip, vtbl, vt, fl, rc, R4 % (0, 0, 3840, 267), pred))
    b = ("PROBE W3b n=%d skip=%d alpha=0x%08x color(+0x54..)=0x00000000,0x00000000,0x00000000 mgr=0x05bc0fb8 "
         "DAT_a309b4=267 DAT_a309b8=1333 DAT_a309bc=1%s" % (n, skip, alpha, unr))
    return a, b


def probe_launch_legacy():
    """Legacy (unsplit) probe launch with W3 records; returns (lines, split-equivalent bare messages)."""
    head = [load2("1.0.2-probe", log=1, probe=1)] + apply_lines()
    dlg = [settop(new=C36), setbottom(new=C36), p_w5(1, 1), p_w1(1, 8), p_w1(2, 267, done=1), p_w2(1, 1592), p_w2(2, 1333, done=1),
           p_w3(1, pred="WOULD-DRAW"), p_w3(2, pred="FAIL:bit6", vt="OTHER"), p_w4(1, eax=1), p_w5(2, 0)]
    return head + dlg


def split_equivalent(lines):
    """Replace each legacy `PROBE W3 ... ` line by its W3 + W3b pair (same field values)."""
    out = []
    for ln in lines:
        m = re.match(r"^\[(\d+)\] PROBE W3 n=(\d+) skip=(\d+) this=0x(\w+) vtbl=0x(\w+)\((\w+)\) flags\(\+0x48\)=(\S+) bit2=(\d) bit6=(\d) "
                     r"rect\(x,y,w,h\)=(\S+) alpha=(\S+) (color\(\+0x54\.\.\)=\S+) mgr=(\S+) W=(\d+) H=(\d+) predicate=(\S+) dt=(\S+) "
                     r"(DAT_a309b4=\S+ DAT_a309b8=\S+ DAT_a309bc=\S+)$", ln)
        if not m:
            out.append(ln)
            continue
        g = m.groups()
        out.append("[%s] PROBE W3 n=%s skip=%s this=0x%s vtbl=0x%s(%s) flags=%s bit2=%s bit6=%s rect=%s W=%s H=%s predicate=%s dt=%s"
                   % (g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7], g[8], g[9], g[13], g[14], g[15], g[16]))
        out.append("[%s] PROBE W3b n=%s skip=%s alpha=%s %s mgr=%s %s" % (g[0], g[1], g[2], g[10], g[11], g[12], g[17]))
    return out


def summary_key(text):
    """Everything the verdict / diagnosis depends on, without file/line numbers."""
    launches, unparsed, total = A.analyze_text(text)
    ok, verdict, dlg, errs = A.summarize(launches, unparsed, total)
    return {"ok": ok, "verdict": verdict, "dialogues": dlg, "unparsed": len(unparsed),
            "launches": [(L.mode, L.pid, L.settop, L.setbottom, [e[1] for e in L.errors],
                          [A.probe_summary(b) for b in L.buckets if any(b[t] for t in ("W1", "W2", "W3", "W4", "W5"))])
                         for L in launches]}


def strip_lines(key):
    """probe_summary carries settop_line numbers; drop them for cross-shape comparisons."""
    for lau in key["launches"]:
        for ps in lau[5]:
            ps.pop("settop_line", None)
    return key


class PrefixShapeTests(unittest.TestCase):
    def setUp(self):
        self.legacy = probe_launch_legacy()
        self.split = split_equivalent(self.legacy)
        self.base = strip_lines(summary_key("\n".join(self.legacy)))

    def test_baseline_is_a_real_probe_run(self):
        self.assertTrue(self.base["ok"])
        self.assertEqual(self.base["unparsed"], 0)
        self.assertEqual(self.base["launches"][0][5][0]["W3"]["n"], 2)

    def test_core_shape_same_result_as_legacy(self):
        text = "\n".join(to_core(self.split))
        self.assertEqual(strip_lines(summary_key(text)), self.base)

    def test_core_shape_without_session_header(self):
        self.assertEqual(strip_lines(summary_key("\n".join(to_core(self.split, header=False)))), self.base)

    def test_fallback_shape_same_result_as_legacy(self):
        self.assertEqual(strip_lines(summary_key("\n".join(to_fallback(self.split)))), self.base)

    def test_legacy_unsplit_w3_still_works(self):
        launches, unparsed, _ = A.analyze_text("\n".join(self.legacy))
        self.assertEqual(unparsed, [])
        self.assertEqual([r["alpha"] for r in launches[0].buckets[0]["W3"]], [HALF, HALF])

    def test_legacy_prefix_with_split_w3_lines(self):
        # prefix shape and W3 shape are independent
        self.assertEqual(strip_lines(summary_key("\n".join(self.split))), self.base)

    def test_crlf_lines(self):
        self.assertEqual(strip_lines(summary_key("\r\n".join(to_core(self.split)))), self.base)

    def test_mixed_shapes_in_one_file(self):
        core = to_core(self.split, header=False)
        text = "\n".join(self.legacy[:2] + core[2:5] + to_fallback(self.split)[5:])
        k = strip_lines(summary_key(text))
        self.assertEqual(k["unparsed"], 0)
        self.assertTrue(k["ok"])

    def test_cli_on_core_file(self):
        with tempfile.TemporaryDirectory() as td:
            p = os.path.join(td, "k2-dialogue-letterbox-fix_log.txt")
            with open(p, "w") as f:
                f.write("\n".join(to_core(self.split)) + "\n")
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                self.assertEqual(A.main(["x", p]), 0)
            self.assertIn("VERDICT: PASS (1 launches, 1 dialogues, 0 errors)", buf.getvalue())
            self.assertIn("DIAGNOSIS: bars drawn (fill issued)", buf.getvalue())

    def test_usage_names_both_files(self):
        self.assertIn("dialogue_letterbox_fix_log.txt", A.__doc__)
        self.assertIn("k2-dialogue-letterbox-fix_log.txt", A.__doc__)


class FilteringTests(unittest.TestCase):
    def test_other_patch_lines_dropped_core(self):
        good = to_core([load2("1.0.2", log=1)] + apply_lines() + [settop(new=C36), setbottom(new=C36)], header=False)
        noise = [core_line("APPLY src=X W=1 H=1: FOREIGN value", 1600, patch="other-mod", ch="main"),
                 core_line("something unparseable here", 1601, patch="mouse-probe", ch="probe"),
                 "T=1602.000 f=- MARK before the cutscene",
                 "# kplog session 2026-10-03 22:40:01 | kpatch.log v1 | patcher ABI 1".replace("# kplog", "# other")]
        text = "\n".join(good[:3] + noise + good[3:])
        launches, unparsed, total = A.analyze_text(text)
        self.assertEqual(unparsed, [])
        self.assertEqual(len(launches), 1)
        self.assertEqual(launches[0].errors, [])
        self.assertEqual(total, len(good))  # dropped lines are not counted
        self.assertTrue(A.summarize(launches, unparsed, total)[0])

    def test_other_patch_lines_dropped_fallback(self):
        text = "\n".join([fb_line(bare(load2("1.0.2", log=1)), 1000), fb_line("boom FOREIGN", 1001, patch="other-mod", ch="main")])
        launches, unparsed, _ = A.analyze_text(text)
        self.assertEqual((len(launches), unparsed, launches[0].errors), (1, [], []))

    def test_patch_id_must_match_exactly(self):
        text = "\n".join([core_line(bare(load2("1.0.2", log=1)), 1000),
                          core_line("APPLY src=X W=1 H=1: FOREIGN", 1001, patch=PID + "-probe", ch="main")])
        launches, unparsed, _ = A.analyze_text(text)
        self.assertEqual(launches[0].errors, [])

    def test_garbage_is_unparsed_not_dropped(self):
        launches, unparsed, total = A.analyze_text("\n".join([load2("1.0.2", log=1), "not a log line", "[12] ???"]))
        self.assertEqual([u[1] for u in unparsed], ["not a log line", "[12] ???"])

    def test_header_only_file_has_no_launch(self):
        launches, unparsed, total = A.analyze_text("# kplog session 2026-10-03 22:40:01 | kpatch.log v1\n# other header\n")
        ok, verdict, _, _ = A.summarize(launches, unparsed, total)
        self.assertEqual((launches, total, ok), ([], 0, False))
        self.assertIn("no launch / load line found", verdict)


class LevelTagTests(unittest.TestCase):
    def test_warn_tag_stripped_from_valid_line(self):
        lines = [core_line(bare(load2("1.0.2", log=1)), 1000)] + [core_line(bare(a), 1010 + i, tag="WARN ") for i, a in enumerate(apply_lines())]
        launches, unparsed, _ = A.analyze_text("\n".join(lines))
        self.assertEqual((unparsed, launches[0].errors), ([], []))
        self.assertEqual(launches[0].applies[0]["result"], "verify_ok")

    def test_err_tag_with_error_text_still_fails(self):
        msg = "APPLY src=SetScreenSize W=3840 H=1600: FOREIGN value 00 00 00 00 00 00 00 40 2: another mod owns 0x009a7d30, not writing"
        for line in (core_line(msg, 1010, tag="ERR "), fb_line(msg, 1010, tag="ERR ")):
            ok, verdict, _, errs, launches, unparsed = run_text("\n".join([core_line(bare(load2("1.0.2", log=1)), 1000), line]))
            self.assertFalse(ok)
            self.assertEqual(unparsed, [])
            self.assertIn("FOREIGN", verdict)
            self.assertTrue(any("FOREIGN" in r for _, r, _ in launches[0].errors))
            self.assertNotIn("ERR ", launches[0].errors[0][2])  # the tag is not part of the reported text

    def test_level_tag_is_not_a_verdict(self):
        # an ERR-tagged line whose text is benign must not fail the launch
        lines = [core_line(bare(load2("1.0.2", log=1)), 1000)] + [core_line(bare(a), 1010 + i) for i, a in enumerate(apply_lines())]
        lines.append(core_line("unknown mode 'zz' in dialogue_letterbox_fix.ini, using h6", 1020, tag="WARN "))
        ok, verdict, _, _, launches, unparsed = run_text("\n".join(lines))
        self.assertTrue(ok, verdict)
        self.assertEqual(len(launches[0].warnings), 1)


class SessionBoundaryTests(unittest.TestCase):
    HDR = "# kplog session 2026-10-03 22:%02d:01 | kpatch.log v1 | patcher ABI 1"

    def launch(self, t0, pid):
        return [core_line(bare(load2("1.0.2", log=1, pid=pid)), t0)] + [core_line(bare(a), t0 + 1 + i) for i, a in enumerate(apply_lines())]

    def test_header_then_load_is_one_launch(self):
        launches, unparsed, total = A.analyze_text("\n".join([self.HDR % 1] + self.launch(1000, 11)))
        self.assertEqual((len(launches), launches[0].pid, launches[0].has_load, unparsed), (1, 11, True, []))

    def test_two_sessions_two_launches(self):
        text = "\n".join([self.HDR % 1] + self.launch(1000, 11) + [self.HDR % 5] + self.launch(900, 22))
        launches, _, _ = A.analyze_text(text)
        self.assertEqual([(L.pid, len(L.applies)) for L in launches], [(11, 1), (22, 1)])

    def test_consecutive_headers_do_not_make_empty_launches(self):
        text = "\n".join([self.HDR % 1, self.HDR % 2] + self.launch(1000, 11) + [self.HDR % 5, self.HDR % 6] + self.launch(900, 22))
        launches, _, _ = A.analyze_text(text)
        self.assertEqual([L.pid for L in launches], [11, 22])

    def test_header_starts_new_launch_even_without_load_line(self):
        # a session whose load line was lost: its lines must not be glued to the previous launch
        body = [core_line(bare(a), 1100 + i) for i, a in enumerate(apply_lines())]
        text = "\n".join([self.HDR % 1] + self.launch(1000, 11) + [self.HDR % 5] + body)
        launches, _, _ = A.analyze_text(text)
        self.assertEqual([(L.pid, L.has_load) for L in launches], [(11, True), (None, False)])

    def test_load_line_still_splits_launches_without_headers(self):
        text = "\n".join(self.launch(1000, 11) + self.launch(1100, 22) + [core_line(bare(load2("1.0.2", pid=33)), 1200)])
        launches, _, _ = A.analyze_text(text)
        self.assertEqual([L.pid for L in launches], [11, 22, 33])

    def test_launch_line_number_is_the_load_line_after_a_header(self):
        launches, _, _ = A.analyze_text("\n".join([self.HDR % 1] + self.launch(1000, 11)))
        self.assertEqual(launches[0].line_no, 2)


class W3JoinTests(unittest.TestCase):
    def run_w3(self, *w3_msgs, extra=()):
        lines = [core_line(bare(load2("1.0.2-probe", log=1, probe=1)), 1000)] + [core_line(bare(a), 1001 + i) for i, a in enumerate(apply_lines())]
        lines += [core_line(bare(settop(new=C36)), 1010), core_line(bare(p_w1(1, 8)), 1011)]
        lines += [core_line(m, 1020 + i) for i, m in enumerate(w3_msgs)]
        lines += [core_line(m, 1100 + i) for i, m in enumerate(extra)]
        return run_text("\n".join(lines))

    def test_join_fills_alpha_and_pred(self):
        ok, verdict, _, _, launches, unparsed = self.run_w3(*w3_split(1, pred="WOULD-DRAW", alpha=0))
        self.assertTrue(ok, verdict)
        self.assertEqual(unparsed, [])
        rec = launches[0].buckets[0]["W3"][0]
        self.assertEqual((rec["alpha"], rec["pred"], rec["letterbox"], rec["unreadable"], rec["awaiting_w3b"]), (0, "WOULD-DRAW", True, False, False))
        self.assertEqual(A.probe_summary(launches[0].buckets[0])["diagnosis"], "alpha 0")
        self.assertEqual(launches[0].warnings, [])

    def test_join_equals_unsplit_summary(self):
        _, _, _, _, l_split, _ = self.run_w3(*w3_split(1, alpha=HALF), *w3_split(2, pred="FAIL:bit6", vt="OTHER", alpha=ONE))
        _, _, _, _, l_leg, _ = probe_run([settop(new=C36), p_w1(1, 8), p_w3(1, alpha=HALF), p_w3(2, pred="FAIL:bit6", vt="OTHER", alpha=ONE)])
        a, b = A.probe_summary(l_split[0].buckets[0]), A.probe_summary(l_leg[0].buckets[0])
        for k in ("W1", "W3", "diagnosis"):
            self.assertEqual(a[k], b[k])

    def test_long_labels_also_accepted(self):
        ok, verdict, _, _, launches, unparsed = self.run_w3(*w3_split(1, labels="long"))
        self.assertEqual(unparsed, [])
        self.assertEqual(launches[0].buckets[0]["W3"][0]["alpha"], HALF)

    def test_w3b_joins_the_matching_n_not_the_latest(self):
        a1, b1 = w3_split(1, alpha=HALF)
        a2, b2 = w3_split(2, pred="FAIL:h>0", alpha=ONE)
        ok, verdict, _, _, launches, unparsed = self.run_w3(a1, a2, b1, b2)
        self.assertEqual(unparsed, [])
        recs = launches[0].buckets[0]["W3"]
        self.assertEqual([(r["n"], r["alpha"]) for r in recs], [(1, HALF), (2, ONE)])

    def test_w3_without_w3b_warns_and_alpha_unknown(self):
        a, _ = w3_split(1)
        ok, verdict, _, _, launches, unparsed = self.run_w3(a)
        self.assertTrue(ok)
        self.assertEqual(unparsed, [])
        self.assertIsNone(launches[0].buckets[0]["W3"][0]["alpha"])
        self.assertTrue(any("no matching W3b" in w[1] for w in launches[0].warnings))
        A.probe_summary(launches[0].buckets[0])  # must not raise

    def test_orphan_w3b_is_unparsed_warning_only(self):
        _, b = w3_split(7)
        ok, verdict, _, _, launches, unparsed = self.run_w3(b)
        self.assertTrue(ok)
        self.assertEqual(len(unparsed), 1)
        self.assertIn("PROBE W3b n=7", unparsed[0][1])

    def test_w3b_with_other_n_does_not_join(self):
        a, _ = w3_split(1)
        _, b = w3_split(2)
        ok, verdict, _, _, launches, unparsed = self.run_w3(a, b)
        self.assertEqual(len(unparsed), 1)
        self.assertIsNone(launches[0].buckets[0]["W3"][0]["alpha"])

    def test_w3b_unreadable_tail_sets_unreadable_and_warns(self):
        for tail in (" [SOME-UNREADABLE]", " unreadable"):
            a, b = w3_split(1, unr=tail)
            ok, verdict, _, _, launches, unparsed = self.run_w3(a, b)
            self.assertTrue(ok, verdict)  # PROBE lines never fail a launch
            self.assertEqual(unparsed, [])
            self.assertTrue(launches[0].buckets[0]["W3"][0]["unreadable"])
            self.assertTrue(any("unreadable memory" in w[1] for w in launches[0].warnings))

    def test_null_w3_needs_no_w3b(self):
        ok, verdict, _, _, launches, unparsed = self.run_w3("PROBE W3 n=1 skip=0 this=NULL/UNREADABLE dt=0x3c888889")
        self.assertEqual(unparsed, [])
        self.assertFalse(any("W3b" in w[1] for w in launches[0].warnings))

    def test_w3b_does_not_join_across_dialogues(self):
        a, _ = w3_split(1)
        _, b = w3_split(1)
        lines = ([core_line(bare(load2("1.0.2-probe", log=1, probe=1)), 1000)] + [core_line(bare(x), 1001 + i) for i, x in enumerate(apply_lines())]
                 + [core_line(bare(settop(new=C36)), 1010), core_line(a, 1011), core_line(bare(settop(new=C36)), 1012), core_line(b, 1013)])
        launches, unparsed, _ = A.analyze_text("\n".join(lines))
        self.assertEqual(len(unparsed), 1)

    def test_probe_lines_counts_both_physical_lines(self):
        ok, verdict, _, _, launches, unparsed = self.run_w3(*w3_split(1))
        self.assertEqual(launches[0].probe_lines, 3)  # W1 + W3 + W3b


class SyntheticFixtureTests(unittest.TestCase):
    """fixtures/letterbox_log_102_synthetic_{core,fallback}.txt = letterbox_log_101_smoke_default.txt converted to the 1.0.2 prefixes."""

    @staticmethod
    def key(name):
        path = os.path.join(FIXTURES, name)
        launches, unparsed, total = A.analyze_file(path)
        ok, verdict, dlg, errs = A.summarize(launches, unparsed, total)
        d = A.build_json(name, launches, unparsed, total, ok, verdict, dlg)
        for L in d["launch_details"]:
            L.pop("line", None)
            for a in L["applies"]:
                a.pop("line", None)
            for coll in ("errors", "warnings"):
                for e in L[coll]:
                    e.pop("line", None)
        d.pop("lines")
        d.pop("file")
        d["unparsed"] = [u["text"] for u in d["unparsed"]]
        return d

    def test_core_matches_legacy_smoke(self):
        self.assertEqual(self.key("letterbox_log_102_synthetic_core.txt"), self.key("letterbox_log_101_smoke_default.txt"))

    def test_fallback_matches_legacy_smoke(self):
        self.assertEqual(self.key("letterbox_log_102_synthetic_fallback.txt"), self.key("letterbox_log_101_smoke_default.txt"))

    def test_core_fixture_has_noise_that_is_dropped(self):
        with open(os.path.join(FIXTURES, "letterbox_log_102_synthetic_core.txt")) as f:
            t = f.read()
        self.assertIn("# kplog session", t)
        self.assertIn("other-patch/", t)
        self.assertIn("MARK", t)


class Smoke102FixtureTests(unittest.TestCase):
    """fixtures/letterbox_log_102_{fallback,probe_fallback,probe_core}.txt = real output of the 1.0.2 / 1.0.2-probe Wine smoke
    harness (dialogue_letterbox_fix_src/smoke_dialogueletterbox.cpp, modes h6 / probe / core). The data is synthetic (fake
    objects, deliberate INVALID/FOREIGN calls), so FAIL verdicts are expected, like the legacy letterbox_log_101_smoke_default."""

    @staticmethod
    def run_fixture(name):
        launches, unparsed, total = A.analyze_file(os.path.join(FIXTURES, name))
        ok, verdict, dlg, errs = A.summarize(launches, unparsed, total)
        return launches, unparsed, total, ok, verdict, dlg

    @staticmethod
    def reasons(L):
        return sorted(e[1].split(":")[0].split(" (")[0] for e in L.errors)

    def test_fallback_h6_shipping(self):
        launches, unparsed, total, ok, verdict, dlg = self.run_fixture("letterbox_log_102_fallback.txt")
        self.assertEqual((len(launches), total, len(unparsed), dlg), (1, 24, 0, 1))
        L = launches[0]
        self.assertEqual((L.mode, L.log, L.pid, L.probe, L.probe_lines), ("h6", 1, 32, None, 0))
        # CRLF fallback lines parse; the only errors are the harness's deliberate W=0 / H=0 / null-manager calls
        self.assertFalse(ok)
        self.assertEqual(len(L.errors), 3)
        self.assertIn("INVALID x2", verdict)
        self.assertIn("null pointer x1", verdict)

    def test_probe_fallback_counts_and_w3_join(self):
        launches, unparsed, total, ok, verdict, dlg = self.run_fixture("letterbox_log_102_probe_fallback.txt")
        self.assertEqual((len(launches), total, len(unparsed), dlg), (1, 3599, 0, 13))
        L = launches[0]
        self.assertEqual((L.probe, L.pid), (1, 224))
        # 3000 gate lines (W1/W2/W3/W4/W5) + 581 W3b lines that ride on their W3
        self.assertEqual(L.probe_lines, 3581)
        self.assertEqual(L.cap, ["probe-cap(3000)"])
        buckets = [L.pre_bucket] + list(L.buckets)
        w3 = [r for b in buckets for r in b["W3"]]
        self.assertEqual(len(w3), 581)
        self.assertTrue(all(r["alpha"] == 0x3F000000 and not r.get("awaiting_w3b") for r in w3), "every W3 joined with its W3b (alpha known)")
        ps = A.probe_summary(L.pre_bucket)
        self.assertEqual((ps["W3"]["n"], ps["W3"]["vtbl_letterbox"], ps["W3"]["vtbl_other"]), (8, 8, 0))
        self.assertEqual(ps["W3"]["alpha"], {"0x3F000000 0.5 (semi-transparent letterbox)": 8})
        self.assertEqual(ps["W3"]["predicate"]["WOULD-DRAW"], 1)
        self.assertEqual((ps["W5"]["start_arg1"], ps["W5"]["end_arg0"]), (True, True))
        # the harness's fake SETTOP has start h 267 != 0: the only failing-line reason
        self.assertFalse(ok)
        self.assertEqual(len(L.errors), 13)
        self.assertEqual(verdict.count("SETTOP start h"), 1)

    def test_probe_core_shape(self):
        launches, unparsed, total, ok, verdict, dlg = self.run_fixture("letterbox_log_102_probe_core.txt")
        # the `# kplog session` header and the load line right after it are ONE launch
        self.assertEqual((len(launches), total, len(unparsed), dlg), (1, 16, 0, 2))
        L = launches[0]
        self.assertEqual((L.probe, L.pid, L.probe_lines), (1, 32, 8))
        b1 = A.probe_summary(L.buckets[0])
        self.assertEqual((b1["W1"]["n"], b1["W2"]["n"], b1["W3"]["n"], b1["W4"]["n"], b1["W5"]["n"]), (1, 1, 1, 1, 2))
        self.assertEqual(b1["W3"]["alpha"], {"0x3F000000 0.5 (semi-transparent letterbox)": 1})   # W3 joined with W3b, alpha known
        self.assertEqual(b1["diagnosis"], "bars drawn (fill issued)")
        # this run deliberately starts with a FOREIGN constant (2.0): reported, not hidden
        self.assertFalse(ok)
        self.assertIn("FOREIGN x1", verdict)
        self.assertIn("start bytes not vanilla x1", verdict)
        self.assertIn("INVALID x1", verdict)

    def test_probe_fixtures_agree_on_probe_shapes(self):
        # core and fallback shapes of the same probe lines give the same per-window parse of the first dialogue block
        la, *_ = self.run_fixture("letterbox_log_102_probe_core.txt")
        lb, *_ = self.run_fixture("letterbox_log_102_probe_fallback.txt")
        a = A.probe_summary(la[0].buckets[0])
        b = A.probe_summary(lb[0].pre_bucket)
        for w in ("W1", "W4", "W5"):
            self.assertEqual({k: v for k, v in a[w].items() if k != "n"}, {k: v for k, v in b[w].items() if k != "n"}, w)


if __name__ == "__main__":
    unittest.main(verbosity=2)
