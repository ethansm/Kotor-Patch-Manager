#!/usr/bin/env python3
"""Tests for scripts/holster_place_verdict.py. Synthetic fixtures only; never reads the real game dir.
Run: python3 scripts/test_holster_place_verdict.py"""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
SCRIPT = str(HERE / "holster_place_verdict.py")

LOADED = ("holster 0.1.0-dev loaded (P0 probe, log-only): probe=1 channels=comb,attach,swap,anim,gob,place "
          "max_lines=6000 gob_every=120 table=ok")
LOADED_NO_PLACE = LOADED.replace("gob,place", "gob")
COMB = "COMB creature=1 old=x new=1 edge=y tick=5"
BONE_OK = "BONETEST patched rhand -> pelvis_g"
BONE_BAD = "BONETEST refused rhand: rc=-1 seen=AA BB CC DD EE"
TID_GOB = "TID gobupdate=111"
TID_PLACE = "TID place=111"
CAP = "cap reached max_lines=6000: further lines dropped"
RELOAD_ON = "RELOAD place=1 rot=(-90.0,0.0,0.0) off=(0.200,0.000,0.000)"
RELOAD_OFF = "RELOAD place=1 rot=(0.0,0.0,0.0) off=(0.000,0.000,0.000)"
P0 = (1.0, 2.0, 3.0)
P_MOVED = (1.2, 2.0, 3.0)
Q_ID = (0.0, 0.0, 0.0, 1.0)


def stamped(body, n=0):
    return "T=%d f=- holster/main %s" % (1000 + n, body)


def place_line(seq, pos0, pos1, quat0=Q_ID, quat1=Q_ID, gob=0x1A2B):
    return ("PLACE n=%d gob=%x node=3c4d beh=0 gobvt=7f00 pos0=(%.3f,%.3f,%.3f) quat0=(%.3f,%.3f,%.3f,%.3f) "
            "pos1=(%.3f,%.3f,%.3f) quat1=(%.3f,%.3f,%.3f,%.3f)") % (
        seq, gob, *pos0, *quat0, *pos1, *quat1)


def baseline_run(loaded=LOADED, tid_place=True, bone=(BONE_OK,), cap=False):
    body = [loaded, COMB] + list(bone) + [TID_GOB]
    if tid_place:
        body.append(TID_PLACE)
    if cap:
        body.append(CAP)
    return body


def place_run(bad_quat=False):
    # Segment 1: RELOAD_ON with the object moved 0.2 along x. Segment 2: RELOAD_OFF, nothing moves.
    q_first = (0.0, 0.0, 0.0, 2.0) if bad_quat else Q_ID
    seg1 = [place_line(1, P0, P_MOVED, quat1=q_first), place_line(2, P0, P_MOVED), place_line(3, P0, P_MOVED)]
    seg2 = [place_line(4, P0, P0), place_line(5, P0, P0)]
    return baseline_run() + [RELOAD_ON] + seg1 + [RELOAD_OFF] + seg2


class HolsterPlaceVerdictTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name)
        self.log = self.dir / "holster_log.txt"
        self.rotated = self.dir / "holster_log.txt.1"

    def _write(self, bodies, path=None):
        path = path or self.log
        path.write_text("\n".join(stamped(b, i) for i, b in enumerate(bodies)) + "\n", encoding="utf-8")

    def _run(self, stage, log=None):
        cmd = [sys.executable, SCRIPT, "--log", str(log or self.log), "--stage", stage,
               "--game-dir", str(self.dir)]
        proc = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace",
                              timeout=60)
        return proc.returncode, proc.stdout, proc.stderr

    def test_baseline_pass_on_healthy_run(self):
        self._write(baseline_run())
        code, out, err = self._run("baseline")
        self.assertEqual(code, 0, out + err)
        self.assertRegex(out, r"(?m)^RUN loaded=yes source=live(\s|$)")
        self.assertRegex(out, r"(?m)^BONETEST patched=1 refused=0")
        self.assertRegex(out, r"(?m)^TID gobupdate=111 place=111")
        self.assertRegex(out, r"(?m)^CAP none")
        self.assertRegex(out, r"(?m)^VERDICT baseline: PASS")

    def test_bonetest_refused_fails_baseline(self):
        self._write(baseline_run(bone=(BONE_OK, BONE_BAD)))
        code, out, err = self._run("baseline")
        self.assertEqual(code, 1, out + err)
        self.assertRegex(out, r"(?m)^BONETEST patched=1 refused=1")
        self.assertRegex(out, r"(?m)^VERDICT baseline: FAIL")

    def test_missing_tid_place_fails_baseline(self):
        self._write(baseline_run(tid_place=False))
        code, out, err = self._run("baseline")
        self.assertEqual(code, 1, out + err)
        self.assertRegex(out, r"(?m)^TID gobupdate=111 place=missing")
        self.assertRegex(out, r"(?m)^VERDICT baseline: FAIL")

    def test_ch_place_off_prints_na_and_baseline_still_passes(self):
        self._write(baseline_run(loaded=LOADED_NO_PLACE))
        code, out, err = self._run("baseline")
        self.assertEqual(code, 0, out + err)
        self.assertRegex(out, r"(?m)^PLACE n/a: ch_place off")
        self.assertNotRegex(out, r"(?m)^PLACE n=")
        self.assertRegex(out, r"(?m)^VERDICT baseline: PASS")

    def test_reload_segments_split_place_stage_passes(self):
        self._write(place_run())
        code, out, err = self._run("place")
        self.assertEqual(code, 0, out + err)
        self.assertRegex(out, r"(?m)^RELOAD n=2")
        self.assertRegex(out, r"(?m)^PLACE n=5")
        self.assertRegex(out, r"(?m)^SEG#1 place=1 n=3 mean_move=0\.2000 quat_ok=yes")
        self.assertRegex(out, r"(?m)^SEG#2 place=1 n=2 mean_move=0\.0000 quat_ok=yes")
        self.assertRegex(out, r"(?m)^VERDICT place: PASS")

    def test_non_unit_quat1_fails_place_stage(self):
        self._write(place_run(bad_quat=True))
        code, out, err = self._run("place")
        self.assertEqual(code, 1, out + err)
        self.assertRegex(out, r"(?m)^VERDICT place: FAIL")

    def test_cap_reached_warns(self):
        self._write(baseline_run(cap=True))
        code, out, err = self._run("baseline")
        self.assertEqual(code, 0, out + err)
        self.assertRegex(out, r"(?m)^CAP reached")
        self.assertRegex(out, r"(?m)^VERDICT baseline: WARN")

    def test_run_split_across_rotated_log_and_live(self):
        # .1 holds an older run (junk refused line) then the real run's loaded line and BONETEST patched.
        # Live has no loaded line, so the run is .1 from its last loaded line plus the live file.
        self._write([LOADED, BONE_BAD, LOADED, BONE_OK, TID_GOB], path=self.rotated)
        self._write([COMB, TID_PLACE])
        code, out, err = self._run("baseline")
        self.assertEqual(code, 0, out + err)
        self.assertRegex(out, r"(?m)^RUN loaded=yes source=live\+\.1")
        self.assertRegex(out, r"(?m)^BONETEST patched=1 refused=0")
        self.assertRegex(out, r"(?m)^VERDICT baseline: PASS")

    def test_only_last_loaded_run_is_counted(self):
        first = [LOADED, BONE_BAD, CAP, TID_GOB]
        self._write(first + baseline_run())
        code, out, err = self._run("baseline")
        self.assertEqual(code, 0, out + err)
        self.assertRegex(out, r"(?m)^BONETEST patched=1 refused=0")
        self.assertRegex(out, r"(?m)^CAP none")
        self.assertRegex(out, r"(?m)^VERDICT baseline: PASS")

    def test_missing_log_exits_2(self):
        code, out, err = self._run("baseline", log=self.dir / "does_not_exist.txt")
        self.assertEqual(code, 2, out + err)

    def test_output_bounded_for_large_log(self):
        body = [LOADED, COMB, BONE_OK, TID_GOB, TID_PLACE]
        seq = 0
        for _ in range(20):
            body.append(RELOAD_ON)
            for _ in range(10):
                seq += 1
                body.append(place_line(seq, P0, P_MOVED))
        self._write(body)
        code, out, err = self._run("place")
        self.assertEqual(code, 0, out + err)
        lines = out.splitlines()
        self.assertLessEqual(len(lines), 40, out)
        for line in lines:
            self.assertLessEqual(len(line), 120, line)
        self.assertRegex(out, r"(?m)^PLACE n=200")
        self.assertRegex(out, r"(?m)^RELOAD n=20")
        self.assertRegex(out, r"(?m)^VERDICT place: PASS")

    def test_place_stage_unchanged_positions_fails(self):
        body = baseline_run() + [RELOAD_ON] + [place_line(i, P0, P0) for i in (1, 2, 3)]
        self._write(body)
        code, out, err = self._run("place")
        self.assertEqual(code, 1, out + err)
        self.assertRegex(out, r"(?m)^SEG#1 place=1 n=3 mean_move=0\.0000")
        self.assertRegex(out, r"(?m)^VERDICT place: FAIL")


if __name__ == "__main__":
    unittest.main()
