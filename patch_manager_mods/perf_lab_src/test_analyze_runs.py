#!/usr/bin/env python3
"""Unit tests for analyze_runs.py against the synthetic fixtures in fixtures/game (regenerate: fixtures/gen_fixtures.py).

Run from this directory:  python3 -m unittest   or   python3 test_analyze_runs.py
"""
import contextlib
import io
import os
import shutil
import tempfile
import unittest

import analyze_runs as ar

HERE = os.path.dirname(os.path.abspath(__file__))
GAME = os.path.join(HERE, "fixtures", "game")
ZERO = {n: 0 for n in ar.ALL_FILES}


def run(offsets=None, arms=None, game=GAME):
    return ar.run(game, ZERO if offsets is None else offsets, arms)


class FixtureSize(unittest.TestCase):
    def test_small_fixtures(self):
        for n in os.listdir(GAME):
            self.assertLess(os.path.getsize(os.path.join(GAME, n)), 20 * 1024, n)


class LaunchSplit(unittest.TestCase):
    def test_split_count_and_match(self):
        md, L, A = run()
        self.assertEqual(len(L), 4)
        self.assertEqual([x.pid for x in L], [100, 101, 102, 103])
        self.assertEqual([x.version for x in L], ["0.3.0", "0.3.0", "0.4.0", "0.3.0"])
        self.assertEqual([len(x.blocks) for x in L], [10, 3, 3, 27])
        self.assertTrue(all(len(x.logs) == 4 for x in L), [sorted(x.logs) for x in L])
        self.assertEqual(L[0].wall_s, 36000 + 1000)       # 10:16:40 from tick 1_000_000
        self.assertIn("Launches found: 4", md)

    def test_attach_only_stub(self):
        d = tempfile.mkdtemp()
        try:
            shutil.copytree(GAME, os.path.join(d, "g"))
            with open(os.path.join(d, "g", "perflab_log.txt"), "a") as f:
                f.write("[9000000 t0] perf-lab 0.3.0 attach exe=1 game=1 alloc=1\n")
            md, L, A = run(game=os.path.join(d, "g"))
            self.assertEqual(len(L), 5)
            self.assertTrue(L[-1].stub)
            self.assertIn("STUB", md)
        finally:
            shutil.rmtree(d)


class Hang(unittest.TestCase):
    def test_hang_detected(self):
        md, L, A = run()
        self.assertEqual([A[x.idx]["hang"] for x in L], [False, True, False, False])
        self.assertIsNone(A[2]["first"])
        self.assertIn("**HANG", md)
        self.assertIn("**Hangs (UP without DOWN): launch 2**", md)

    def test_first_load_ms(self):
        md, L, A = run()
        self.assertEqual([A[i]["first"]["ms"] if A[i]["first"] else None for i in (1, 2, 3, 4)], [5000, None, 3000, 6000])


class Gate(unittest.TestCase):
    def test_gate(self):
        md, L, A = run()
        self.assertEqual([A[i]["gate"] for i in (1, 2, 3, 4)], ["PASS", "PASS", "FAIL", "PASS"])
        self.assertEqual(A[3]["other_ret"], 7)
        self.assertEqual(A[3]["ctrl_ret"], 1)
        self.assertIn("run B gate (other_tpc_retained == 0): FAIL", md)
        self.assertIn("Overall run B gate (other_tpc_retained == 0 in every launch): FAIL (launch 3)", md)

    def test_overall_pass(self):
        md, L, A = run({"perflab_metrics.txt": self._offset_of_launch(4)})
        self.assertEqual(sum(1 for x in L if not x.stub), 1)   # launch 4 only (earlier attach lines become stubs)
        self.assertEqual(A[L[-1].idx]["gate"], "PASS")
        self.assertIn("run B gate (other_tpc_retained == 0): PASS", md)

    @staticmethod
    def _offset_of_launch(k):
        with open(os.path.join(GAME, "perflab_metrics.txt"), "rb") as f:
            data = f.read()
        pos = -1
        for _ in range(k):
            pos = data.index(b"#METRICS 1 ", pos + 1)
        return pos


class Counters(unittest.TestCase):
    def test_kt_and_robustness_and_activation(self):
        md, L, A = run()
        self.assertEqual(A[1]["kt_uaf"], 2)
        self.assertEqual(A[3]["kt_sid"], 1)
        rb = L[1].logs["robustnessfixes_log.txt"]
        self.assertEqual(len(rb.where("FIRED")), 2)
        self.assertEqual(L[1].logs["activationfix_log.txt"].count("IGNORED"), 2)
        self.assertEqual(L[2].texmem_state(), "patched")
        self.assertEqual(L[2].logs["texmem_log.txt"].counters()["tpc_freed"], 120)
        self.assertIn("FIRED vmid neg=1 last_id=77", md)

    def test_scenarios(self):
        md, L, A = run()
        self.assertEqual(A[1]["scen"]["S1"][0], "covered")
        self.assertEqual(A[1]["scen"]["S2"][0], "covered")
        self.assertEqual(A[4]["scen"]["S3"][0], "covered")
        self.assertEqual(A[4]["scen"]["_cl"]["quicksave_pairs"], 10)
        self.assertEqual(A[4]["scen"]["_cl"]["quickloads"], 1)
        self.assertEqual(A[2]["scen"]["SNDRI"][0], "covered")
        self.assertEqual(A[1]["scen"]["SNDRI"][0], "unknown")

    def test_gl_generic_hwmip(self):
        md, L, A = run()
        self.assertEqual(A[3]["hw"]["hwmip_calls"], 40)
        self.assertEqual(A[3]["loads"][0]["gl"]["hwmip_hw"], 38)
        self.assertEqual(A[3]["loads"][0]["gl"]["mipmaps"], 60)
        self.assertEqual(A[3]["loads"][0]["gl"]["mip_ms"], 100.0)
        self.assertIn("hwmip_fallback_npot", md)

    def test_frame_hist(self):
        md, L, A = run()
        fs = A[3]["frame"]
        self.assertEqual(fs["n"], 100)
        self.assertEqual(fs["p50"], "16-31")
        self.assertEqual(fs["p95"], "32-63")


class Offsets(unittest.TestCase):
    def test_mid_line_offset_skips_partial_line(self):
        path = os.path.join(GAME, "perflab_log.txt")
        with open(path, "rb") as f:
            data = f.read()
        second = data.index(b"\n") + 1
        info = ar.ReadInfo("perflab_log.txt", path, second + 5)       # inside line 2
        lines = list(ar.read_lines(info))
        self.assertTrue(info.partial_skipped)
        self.assertEqual(lines[0], data.split(b"\n")[2].decode())
        info = ar.ReadInfo("perflab_log.txt", path, second)           # exactly at a line start: keep it
        lines = list(ar.read_lines(info))
        self.assertFalse(info.partial_skipped)
        self.assertEqual(lines[0], data.split(b"\n")[1].decode())

    def test_offset_beyond_eof(self):
        info = ar.ReadInfo("texmem_log.txt", os.path.join(GAME, "texmem_log.txt"), 10 ** 9)
        self.assertEqual(list(ar.read_lines(info)), [])
        self.assertTrue(info.beyond_eof)

    def test_mid_line_metrics_offset_drops_partial_block_lines(self):
        off = Gate._offset_of_launch(4)
        _, L, _ = run({"perflab_metrics.txt": off - 7})                 # mid-line, inside launch 3's last #END line
        self.assertEqual(len(L) - sum(1 for x in L if x.stub), 1)
        self.assertEqual(sum(1 for x in L if not x.stub and x.blocks[0].n == 1), 1)

    def test_offsets_file(self):
        d = tempfile.mkdtemp()
        try:
            p = os.path.join(d, "off.txt")
            with open(p, "w") as f:
                f.write("# comment\nperflab_metrics.txt 123 extra cols\ntexmem_log.txt 7  # trailing\n\n")
            self.assertEqual(ar.load_offsets(p), {"perflab_metrics.txt": 123, "texmem_log.txt": 7})
        finally:
            shutil.rmtree(d)

    def test_default_offsets(self):
        self.assertEqual(ar.DEFAULT_OFFSETS["perflab_metrics.txt"], 10828745)
        self.assertEqual(ar.DEFAULT_OFFSETS["robustnessfixes_log.txt"], 0)


class Arms(unittest.TestCase):
    def test_texmem_arms(self):
        md, L, A = run(arms="texmem")
        self.assertIn("## Arms: texmem state", md)
        row = {l.split("|")[1].strip(): [c.strip() for c in l.split("|")[2:-1]] for l in md.splitlines() if l.startswith("| DISABLED") or l.startswith("| patched")}
        self.assertEqual(row["DISABLED"][0], "3")                  # n
        self.assertEqual(row["DISABLED"][1], "5500")               # median first load of 5000 (L1) and 6000 (L4); hung L2 skipped
        self.assertEqual(row["patched"][0], "1")
        self.assertEqual(row["patched"][1], "3000")
        self.assertEqual(row["patched"][5], "120")                 # tpc_freed
        self.assertEqual(row["patched"][6], "3")                   # tpc_kept_controller
        self.assertEqual(row["patched"][7], "12")                  # requeue_tpc_retained

    def test_hwmip_arms(self):
        md, L, A = run(arms="hwmip")
        self.assertIn("## Arms: hwmip state", md)
        row = {l.split("|")[1].strip(): [c.strip() for c in l.split("|")[2:-1]] for l in md.splitlines() if l.startswith("| hwmip=")}
        self.assertEqual(sorted(row), ["hwmip=0", "hwmip=1"])
        self.assertEqual(row["hwmip=0"][0], "3")
        self.assertEqual(row["hwmip=1"][0], "1")
        self.assertEqual(row["hwmip=1"][2], "100")                 # first-load mip_ms
        self.assertEqual(row["hwmip=0"][2], "550")                 # median(500, 600)
        self.assertIn("median hwmip_calls", md)
        self.assertEqual([x.hwmip for x in L], [0, 0, 1, 0])

    def test_no_arms_section_by_default(self):
        md, _, _ = run()
        self.assertNotIn("## Arms", md)


class Missing(unittest.TestCase):
    def test_missing_files_tolerated(self):
        d = tempfile.mkdtemp()
        try:
            md, L, A = run(game=d)
            self.assertEqual(L, [])
            self.assertIn("ABSENT", md)
            md, L, A = run(game=os.path.join(d, "does_not_exist"))
            self.assertIn("(no launches)", md)
        finally:
            shutil.rmtree(d)

    def test_only_metrics_present(self):
        d = tempfile.mkdtemp()
        try:
            shutil.copy(os.path.join(GAME, "perflab_metrics.txt"), d)
            md, L, A = run(game=d)
            self.assertEqual(len(L), 4)
            self.assertTrue(all(not x.logs for x in L))
            self.assertIn("no log block matched", md)
        finally:
            shutil.rmtree(d)


class Cli(unittest.TestCase):
    def test_cli_stdout_and_out_file(self):
        d = tempfile.mkdtemp()
        try:
            off = os.path.join(d, "off.txt")
            with open(off, "w") as f:
                f.write("perflab_metrics.txt 0\n")
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                self.assertEqual(ar.main(["--game-dir", GAME, "--from-offsets", off, "--arms", "texmem"]), 0)
            self.assertIn("# perf-lab run analysis", buf.getvalue())
            out = os.path.join(d, "o.md")
            ar.main(["--game-dir", GAME, "--from-offsets", off, "--out", out])
            with open(out) as f:
                self.assertIn("## Summary", f.read())
        finally:
            shutil.rmtree(d)


if __name__ == "__main__":
    unittest.main()
