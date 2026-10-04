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
GAME050 = os.path.join(HERE, "fixtures", "game050")       # perf-lab 0.5.0 log content (see gen_fixtures.py)
ZERO = {n: 0 for n in ar.ALL_FILES}


def run(offsets=None, arms=None, game=GAME):
    return ar.run(game, ZERO if offsets is None else offsets, arms)


def run050(arms=None):
    return run(arms=arms, game=GAME050)


class FixtureSize(unittest.TestCase):
    def test_small_fixtures(self):
        for g in (GAME, GAME050):
            for n in os.listdir(g):
                self.assertLess(os.path.getsize(os.path.join(g, n)), 20 * 1024, n)


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


class Perf050(unittest.TestCase):
    """perf-lab 0.5.0 log content: fixtures/game050 (3 launches, see gen_fixtures.py)."""

    def test_split_and_module_header_not_a_flag(self):
        md, L, A = run050()
        self.assertEqual([x.pid for x in L], [200, 201, 202])
        self.assertEqual([x.version for x in L], ["0.5.0"] * 3)
        self.assertEqual([x.hwmip for x in L], [1, 0, 0])
        self.assertNotIn("module=", L[0].flags())
        self.assertNotIn(" module=", md.split("### Launch 1")[1].split("- first load")[0])

    def test_module_per_load_and_round_trip(self):
        md, L, A = run050()
        self.assertEqual([ar.mod_label(l) for l in A[1]["loads"]], ["menu -> 001EBO", "001EBO -> m01aa", "m01aa -> 001EBO"])
        self.assertEqual([l["module"] for l in A[1]["loads"]], ["001EBO", "m01aa", "001EBO"])
        self.assertEqual(A[1]["modules"], ["001EBO", "m01aa", "001EBO"])
        self.assertTrue(A[1]["round_trip"])
        self.assertEqual(A[1]["scen"]["S2"][0], "covered")
        self.assertIn("PASS round trip by module name: 001EBO -> m01aa -> 001EBO", md)
        self.assertIn("| 2 | 4000 | 001EBO -> m01aa |", md)
        # one module only: no round trip, rendered as "-" with the names
        self.assertFalse(A[2]["round_trip"])
        self.assertEqual(A[2]["scen"]["S2"][0], "not covered")
        self.assertIn("round trip: - (no module name seen again after a different one); modules seen: m01aa", md)

    def test_old_logs_have_no_modules(self):
        md, L, A = run()
        self.assertEqual(A[1]["modules"], [])
        self.assertEqual([ar.mod_label(l) for l in A[1]["loads"]], ["n/a"] * 3)
        self.assertIn("(heuristic)", A[1]["scen"]["S2"][1])          # S2 falls back to the UNLOAD-count heuristic

    def test_evt_s3_measured(self):
        md, L, A = run050()
        self.assertTrue(L[0].has_evt())
        self.assertEqual(L[0].evt_counts()["qsave"], 6)
        self.assertEqual(A[1]["scen"]["S3"][0], "covered")             # 2 save + 6 qsave + 3 autosave = 11 >= 10, 1 qload
        self.assertIn("11 save-type events", A[1]["scen"]["S3"][1])
        self.assertIn("measured: EVT", A[1]["scen"]["S3"][1])
        self.assertNotIn("(heuristic)", A[1]["scen"]["S3"][1])
        self.assertEqual(len(L[0].evt_list()), 6)
        self.assertIn("EVT counts (cumulative since attach): save=2 load=0 qsave=6 qload=1 autosave=3 optsave=1 sndapply=1", md)
        self.assertEqual(A[2]["scen"]["S3"][0], "not covered")         # EVT present, all zero: measured, not heuristic
        self.assertIn("measured: EVT", A[2]["scen"]["S3"][1])

    def test_evt_threshold_and_load_requirement(self):
        d = tempfile.mkdtemp()
        try:
            shutil.copytree(GAME050, os.path.join(d, "g"))
            path = os.path.join(d, "g", "perflab_metrics.txt")
            with open(path) as f:
                txt = f.read()
            # 10 saves but zero loads, then 9 saves + 1 load
            for counts, want in (("save=2 load=0 qsave=6 qload=0 autosave=3", "unknown"), ("save=2 load=1 qsave=5 qload=0 autosave=2", "not covered"),
                                 ("save=2 load=1 qsave=5 qload=0 autosave=3", "covered")):
                t2 = txt.replace("save=2 load=0 qsave=6 qload=1 autosave=3", counts)
                with open(path, "w") as f:
                    f.write(t2)
                _, L, A = run(game=os.path.join(d, "g"))
                self.assertEqual(A[1]["scen"]["S3"][0], want, counts)
        finally:
            shutil.rmtree(d)

    def test_heuristic_fallback_without_evt(self):
        md, L, A = run()                                               # 0.3.0/0.4.0 fixtures have no EVT lines
        self.assertFalse(L[3].has_evt())
        self.assertEqual(A[4]["scen"]["S3"][0], "covered")
        self.assertTrue(A[4]["scen"]["S3"][1].endswith("(heuristic)"), A[4]["scen"]["S3"][1])
        for i in (1, 2, 3):
            self.assertTrue(A[i]["scen"]["S3"][1].endswith("(heuristic)"), (i, A[i]["scen"]["S3"][1]))
        self.assertIn("no EVT lines in this launch (pre-0.5.0 log)", md)

    def test_sndri_verdicts(self):
        md, L, A = run050()
        self.assertEqual(A[1]["scen"]["SNDRI"][0], "toggled, no fire")  # sndapply=1, no FIRED sndri
        self.assertIn("sndapply=1 optsave=1", A[1]["scen"]["SNDRI"][1])
        self.assertEqual(A[2]["scen"]["SNDRI"][0], "not toggled")      # sndapply=0
        self.assertEqual(A[3]["scen"]["SNDRI"][0], "covered")          # FIRED sndri
        self.assertIn("fired", A[3]["scen"]["SNDRI"][1])
        self.assertIn("sndapply=2", A[3]["scen"]["SNDRI"][1])
        # 0.3.0 logs: previous behaviour unchanged
        _, _, A0 = run()
        self.assertEqual(A0[2]["scen"]["SNDRI"][0], "covered")
        self.assertEqual(A0[1]["scen"]["SNDRI"][0], "unknown")

    def test_tpctop_first_and_largest(self):
        md, L, A = run050()
        first, big = A[1]["tpc_first"], A[1]["tpc_largest"]
        self.assertEqual((first.n, first.module), (3, "001EBO"))
        self.assertEqual((big.n, big.module), (5, "m01aa"))
        self.assertEqual(len(big.tpctop["items"]), 20)
        self.assertEqual(len(big.restop["items"]), 10)
        self.assertEqual(big.tpctop["hdr"]["overflow"], 3)
        self.assertEqual(big.tpctop["items"][0], (1, 8155, "tex8192_01"))
        self.assertIn("TPCTOP top 20, first capture: block #3 LOAD_DOWN tick 5045000, module 001EBO; n=25 total_mb=180.50", md)
        self.assertIn("TPCTOP top 20, largest total_mb: block #5 LOAD_DOWN tick 5104000, module m01aa; n=61 total_mb=320.25 overflow=3", md)
        self.assertIn("| 20 | 7452 | tex8192_20 |", md)
        self.assertIn("RESTOP top 10 of the same block:", md)
        self.assertEqual(md.count("RESTOP top 10"), 1)                  # only for the largest capture
        self.assertIn("| 10 | 2002 | 15874 | res8192_10 |", md)
        self.assertIsNone(A[2]["tpc_first"])

    def test_va(self):
        md, L, A = run050()
        va = A[1]["va"]
        self.assertEqual((va["min"], va["last"], va["first"], va["trend"], va["warn"]), (190, 320, 700, "falling", True))
        self.assertIn("**WARN: min largest_free_mb 190 < 256 MB**", md)
        self.assertEqual((A[2]["va"]["min"], A[2]["va"]["warn"], A[2]["va"]["trend"]), (850, False, "flat"))
        self.assertIsNone(A[3]["va"])
        self.assertIn("190 WARN |", md)
        self.assertEqual(L[0].blocks[0].kv["VA"]["largest_free_at"], "0x20001000")

    def test_hwv_arms(self):
        md, L, A = run050(arms="hwmip")
        h = A[1]["hwv"]
        self.assertEqual((h["ok"], h["bad"], h["verdict"], h["first_bad"]), (47, 1, "FAIL", "0xde1/512/128"))
        self.assertEqual(len(h["fb"]), 2)
        self.assertEqual(h["fb"][1]["reason"], "glerror")
        self.assertIsNone(A[2]["hwv"])
        self.assertIn("### HWV (hwmip upload formats", md)
        self.assertIn("| 1 | 1 | 40 | 5 | 1 | 2 | 0 | 47 | 1 | **FAIL** | 0xde1/512/128 | 2 |", md)
        self.assertIn("**HWV lvl1 verdict: FAIL (launch 1)**", md)
        self.assertIn("| 1 | npot | 0xde1 | 300 | 200 | 0x1908 | 0x1401 |", md)
        self.assertIn("| 2 | glerror | 0xde1 | 512 | 512 | 0x1907 | 0x1401 |", md)

    def test_hwv_pass_when_no_bad(self):
        d = tempfile.mkdtemp()
        try:
            shutil.copytree(GAME050, os.path.join(d, "g"))
            path = os.path.join(d, "g", "perflab_metrics.txt")
            with open(path) as f:
                t = f.read().replace("lvl1_bad=1 lvl1_first_bad=0xde1/512/128", "lvl1_bad=0 lvl1_first_bad=-")
            with open(path, "w") as f:
                f.write(t)
            md, L, A = run(arms="hwmip", game=os.path.join(d, "g"))
            self.assertEqual(A[1]["hwv"]["verdict"], "PASS")
            self.assertIn("**HWV lvl1 verdict: PASS", md)
        finally:
            shutil.rmtree(d)

    def test_hwv_absent_on_old_logs(self):
        md, L, A = run(arms="hwmip")
        self.assertTrue(all(A[x.idx]["hwv"] is None for x in L))
        self.assertIn("no HWV lines in these launches", md)
        self.assertNotIn("TPCTOP", md)
        self.assertNotIn("EVT counts (cumulative", md)

    def test_new_byte_fields(self):
        md, L, A = run050()
        a = A[1]
        self.assertEqual((a["gms_calls"], a["gms_ms"], a["txi_kb"], a["requeue_kb"], a["redemB"]), (12, 34.5, 2048, 512, 524288))
        self.assertIn("gms_calls=12 gms_ms=34.5", md)
        self.assertIn("txi_via_tpc=57 (2048 kb)", md)
        self.assertIn("requeue_tpc_retained=5 (512 kb)", md)
        self.assertIn("redemB (last) 524288 B = 0.50 MB", md)
        self.assertEqual(A[2]["redemB"], 4096)                          # `redemB=<n>` token form
        self.assertEqual(L[0].blocks[7].rt[3007], 190 * 1048576)        # resident column unaffected by the appended redemB
        # absent on old logs (and no stray labels)
        md0, _, A0 = run()
        self.assertIsNone(A0[3]["gms_calls"])
        self.assertIsNone(A0[3]["txi_kb"])
        self.assertIsNone(A0[3]["redemB"])
        self.assertNotIn("gms_calls", md0)
        self.assertNotIn("redemB", md0)

    def test_mid_file_offset_still_parses(self):
        # an offset inside launch 1 of the 0.5.0 fixture must not crash and still finds launches 2 and 3
        with open(os.path.join(GAME050, "perflab_metrics.txt"), "rb") as f:
            data = f.read()
        off = data.index(b"TPCTOP 7 ") + 3
        _, L, _ = run({"perflab_metrics.txt": off}, game=GAME050)
        self.assertGreaterEqual(len([x for x in L if not x.stub]), 2)


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


class Compact(unittest.TestCase):
    """--latest / --summary / VERDICT (fixture GAME: 4 launches, launch 2 hangs, launch 3 gate FAIL)."""

    def render(self, latest=None, summary=False, game=GAME):
        return ar.run(game, ZERO, None, latest, summary)[0]

    def test_default_unchanged_plus_verdict(self):
        md = self.render()
        self.assertIn("## Sources", md)
        self.assertIn("## Summary (all launches)", md)
        self.assertIn("**Hangs (UP without DOWN): launch 2**", md)
        self.assertEqual(md.splitlines()[-1], "VERDICT: FAIL launches=4 hangs=1 gateB=FAIL")
        self.assertEqual(md, ar.run(GAME, ZERO)[0])

    def test_latest(self):
        md = self.render(latest=1)
        self.assertIn("## Summary (last 1 of 4 launches)", md)
        full = self.render()
        self.assertLess(len(md), len(full))
        self.assertIn("(other_tpc_retained == 0 in every launch) [all 4 launches]: FAIL (launch 3)", md)
        self.assertIn("**Hangs (UP without DOWN) [all 4 launches]: launch 2**", md)
        self.assertEqual(md.splitlines()[-1], "VERDICT: FAIL launches=4 hangs=1 gateB=FAIL")
        sec = md.split("## Summary", 1)[1].split("**Overall", 1)[0]
        ids = [l.split("|")[1].strip() for l in sec.splitlines() if l.startswith("| ")][1:]
        self.assertEqual(ids, ["4"])
        self.assertNotIn("### Launch 1:", md)
        self.assertIn("### Launch 4:", md)

    def test_latest_larger_than_launches_is_all(self):
        self.assertEqual(self.render(latest=99), self.render())

    def test_summary(self):
        md = self.render(summary=True)
        self.assertNotIn("## Sources", md)
        self.assertNotIn("## Launch details", md)
        self.assertIn("## Summary (all launches)", md)
        self.assertIn("Overall run B gate", md)
        self.assertEqual(md.splitlines()[-1], "VERDICT: FAIL launches=4 hangs=1 gateB=FAIL")
        self.assertLess(len(md), len(self.render()) // 2)

    def test_summary_latest(self):
        md = self.render(latest=1, summary=True)
        self.assertNotIn("## Launch details", md)
        self.assertIn("## Summary (last 1 of 4 launches)", md)
        self.assertTrue(md.endswith("gateB=FAIL\n"))

    def test_verdict_pass(self):
        md = self.render(game=GAME050)
        last = md.splitlines()[-1]
        self.assertTrue(last.startswith("VERDICT: "), last)
        _, L, A = ar.run(GAME050, ZERO)
        hangs = sum(1 for x in L if A[x.idx]["hang"])
        fail = any(A[x.idx]["gate"] == "FAIL" for x in L)
        self.assertEqual(last, "VERDICT: %s launches=%d hangs=%d gateB=%s" % ("FAIL" if hangs or fail else "PASS", len(L), hangs, "FAIL" if fail else "PASS"))

    def test_verdict_pass_single_launch(self):
        md = ar.run(GAME, {"perflab_metrics.txt": Gate._offset_of_launch(4)})[0]
        self.assertIn("gateB=PASS", md.splitlines()[-1])

    def test_cli_out_file_ends_with_verdict_and_flags(self):
        d = tempfile.mkdtemp()
        try:
            out = os.path.join(d, "o.md")
            off = os.path.join(d, "off.txt")
            with open(off, "w") as f:
                f.write("perflab_metrics.txt 0\n")
            ar.main(["--game-dir", GAME, "--from-offsets", off, "--summary", "--latest", "1", "--out", out])
            with open(out) as f:
                lines = f.read().splitlines()
            self.assertTrue(lines[-1].startswith("VERDICT: "))
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                ar.main(["--game-dir", GAME, "--from-offsets", off, "--latest", "2"])
            self.assertIn("last 2 of 4", buf.getvalue())
        finally:
            shutil.rmtree(d)


if __name__ == "__main__":
    unittest.main()
