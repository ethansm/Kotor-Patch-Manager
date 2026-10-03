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


if __name__ == "__main__":
    unittest.main(verbosity=2)
