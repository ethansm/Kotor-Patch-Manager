#!/usr/bin/env python3
"""Unit tests for analyze_play_logs.py. Run: python3 test_analyze_play_logs.py

Each fixture under testdata/playlogs/<case>/ holds synthetic logs in the exact formats the C++ sources write.
The fixtures use the tiny hook tomls in testdata/playlogs/_hooks (so they do not depend on the real hook addresses);
the tests that use the real tomls build their steam log from the tomls at run time.
Tests always work on a temp COPY: the tool writes REPORT.md into the directory it is given.
"""
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import analyze_play_logs as apl  # noqa: E402

FIX = HERE / "testdata" / "playlogs"
REAL = Path.home() / "Games" / "KOTOR2_Mod_Workspace" / "playlogs_mods68_1001_2335"
MINI = ["--hooks-toml", f"companion-ai={FIX / '_hooks' / 'companion-ai.hooks.toml'}",
        "--hooks-toml", f"ranged-ai={FIX / '_hooks' / 'ranged-ai.hooks.toml'}",
        "--hooks-toml", f"inventory-sort-filter={FIX / '_hooks' / 'inventory-sort-filter.hooks.toml'}"]


class Base(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.tmp = Path(self._tmp.name)

    def copy_case(self, case: str) -> Path:
        dst = Path(tempfile.mkdtemp(dir=self.tmp)) / case
        shutil.copytree(FIX / case, dst)
        return dst

    def run_case(self, case: str, *extra, mini=True) -> dict:
        d = self.copy_case(case)
        return apl.run_full([str(d)] + (MINI if mini else []) + list(extra))

    @staticmethod
    def fails(r: dict, needle: str = "") -> list:
        return [m for s, _src, m in r["an"].items if s == "FAIL" and needle in m]

    @staticmethod
    def status(r: dict, cid: str) -> str:
        return next(st for i, st, _s, _e in r["checklist"] if i == cid)


class HealthySession(Base):
    def test_no_fail_and_report_written(self):
        d = self.copy_case("healthy")
        r = apl.run_full([str(d)] + MINI)
        self.assertEqual(r["rc"], 0)
        self.assertTrue((d / "REPORT.md").is_file())
        self.assertEqual([m for s, _, m in r["an"].items if s == "FAIL"], [])
        self.assertEqual([m for s, _, m in r["an"].items if s == "WARN"], [])

    def test_detour_table_all_ok_and_consumed_parsed(self):
        r = self.run_case("healthy")
        S = r["S"]
        self.assertEqual(len(S["rows"]), 6)
        self.assertTrue(all(row["status"] == "OK" for row in S["rows"]), [row["status"] for row in S["rows"]])
        self.assertEqual((S["consumed_parsed"], S["consumed_expected"]), (4, 4))
        self.assertEqual(S["n_err"], 1)                      # the benign winebth err: line
        self.assertEqual(S["av_probe"], 1)                   # AV followed by IsBadReadPtr
        self.assertEqual(S["av_other"], [])

    def test_checklist_statuses(self):
        r = self.run_case("healthy")
        ids = [i for i, *_ in r["checklist"]]
        self.assertEqual(ids, [i for i, _ in apl.CHECKLIST])
        for cid in ("M6-2", "M6-3", "M6-4", "M8-1", "M8-2", "M8-3", "M8-4", "M8-5", "CAI-1", "CAI-2", "CAI-3", "CAI-5", "CAI-6",
                    "CAI-7", "CAI-8", "E3"):
            self.assertEqual(self.status(r, cid), "PASS", cid)
        self.assertEqual(self.status(r, "CAI-4"), "OBSERVE")   # visual only
        self.assertEqual(self.status(r, "M6-1"), "OBSERVE")    # visual only

    def test_script_markers_counted_per_creature(self):
        T = self.run_case("healthy")["T"]
        mt = T["marker_total"]
        for k, n in (("Party E1", 3), ("Party E1 skipped", 2), ("Party E1 exit", 1), ("Party H1", 1), ("Party MEDPAC", 1), ("Party M1", 1),
                     ("Party M4", 1), ("Party M10", 1), ("Party E3", 1), ("Party E8", 1), ("Kite step", 3), ("Kite K2 cap", 1)):
            self.assertEqual(mt[k], n, k)
        self.assertEqual(T["marker_cr"]["Kite step"]["7f0001a4"], 3)
        self.assertEqual(T["marker_cr"]["Party M1"]["7f0001a2"], 1)

    def test_trace_sections(self):
        T = self.run_case("healthy")["T"]
        self.assertEqual(T["t1"]["tid"], 111)
        self.assertEqual({f["tid"] for f in T["fm4"]}, {111})
        self.assertEqual((T["fm1"]["keep"], T["fm1"]["clear"]), (1, 1))
        self.assertEqual(len(T["a1"]), 2)
        self.assertEqual(T["a1_missing"], [])
        self.assertEqual(T["a1sum"]["7f0001a4"]["passes"], 2)
        self.assertEqual([e["calls"] for e in T["e3"]], [1180.0, 1210.0])
        self.assertEqual(sum(d[0] for d in T["drops"]), 214)
        self.assertEqual(T["repeats"], (5, 1))
        self.assertEqual(T["pa_count"]["7f0001a5"][("A", "k_hen_jedidlg", "1012")], 1)

    def test_ranged_sections(self):
        RG = self.run_case("healthy")["RG"]
        self.assertEqual(RG["s_counts"]["S1"], 1)
        self.assertEqual(RG["h1"][11]["los0"], 2)
        self.assertEqual(len(RG["h2"]), 1)
        self.assertTrue(RG["h2"][0]["eq"])
        self.assertEqual(sum(RG["h3"].values()), 3)

    def test_inventory_sections(self):
        I = self.run_case("healthy")["I"]
        self.assertEqual(len(I["flushes"]), 5)
        self.assertEqual([c["state"] for c in I["cycles"]], [1, 2, 3, 0])
        self.assertEqual([f["state"] for f in I["flushes"]], [0, 0, 1, 2, 3])
        self.assertEqual(I["errs"], {})

    def test_window_restricts_the_e3_answer(self):
        # fixture clock: ai_trace header 01:00:00 at t=1000000; stance-11 los=0 lines at 01:00:20 and 01:20:00
        r_all = self.run_case("healthy")
        self.assertEqual(len(r_all["RG"]["h1_los0"][11]), 2)
        r1 = self.run_case("healthy", "--window", "00:59-01:05")
        self.assertEqual(len(r1["RG"]["h1_los0"][11]), 1)
        self.assertEqual(r1["RG"]["h1_los0"][11][0][0], 8)
        r2 = self.run_case("healthy", "--window", "01:15-01:25")
        lines = r2["RG"]["h1_los0"][11]
        self.assertEqual(len(lines), 1)
        self.assertIn("-> hold=1", lines[0][1])
        r3 = self.run_case("healthy", "--window", "03:00-03:10")
        self.assertEqual(self.status(r3, "E3"), "OBSERVE")

    def test_window_without_anchor_is_reported(self):
        d = self.tmp / "noclock"
        d.mkdir()
        shutil.copy(FIX / "healthy" / "ranged_ai_log.txt", d)
        r = apl.run_full([str(d), "--window", "01:00-01:05", "--no-write"] + MINI)
        self.assertTrue(any("ignored" in m for _s, _src, m in r["an"].items))
        r = apl.run_full([str(d), "--window", "01:00-01:05", "--no-write", "--anchor", "2026-10-02 01:00:00=1000000"] + MINI)
        self.assertEqual(len(r["RG"]["h1_los0"][11]), 1)


class DetectedProblems(Base):
    def test_a1_without_e2(self):
        r = self.run_case("a1_no_e2")
        T = r["T"]
        self.assertEqual(len(T["a1_need"]), 3)
        self.assertEqual(len(T["a1_missing"]), 2)             # no E2 at all, and an E2 5 s later (> 3.5 s)
        self.assertEqual({x["ln"] for x in T["a1_missing"]}, {6, 7})
        self.assertTrue(self.fails(r, "no E2 FALLBACK"))
        self.assertEqual(self.status(r, "CAI-1"), "FAIL")

    def test_e1_spin_while_a1_on(self):
        r = self.run_case("e1_spin_a1")
        self.assertEqual(len(r["T"]["e1spin_bad"]), 1)
        self.assertTrue(self.fails(r, "E1 SPIN"))

    def test_fm4_tid_mismatch(self):
        r = self.run_case("fm4_tid_mismatch")
        self.assertEqual([f["tid"] for f in r["T"]["tid_bad"]], [222])
        self.assertTrue(self.fails(r, "FM4 tid mismatch"))
        self.assertEqual(self.status(r, "CAI-8"), "FAIL")

    def test_fm4_unpaired(self):
        r = self.run_case("fm4_unpaired")
        self.assertEqual(r["T"]["fm4_unpaired"], 1)
        self.assertEqual(r["T"]["tid_bad"], [])
        self.assertTrue(self.fails(r, "unpaired"))

    def test_missing_detour(self):
        r = self.run_case("missing_detour")
        missing = {row["address"] for row in r["S"]["rows"] if row["status"] == "MISSING"}
        self.assertEqual(missing, {0x006D853E, 0x0074DBDD})
        self.assertEqual(len(self.fails(r, "no 'Applied' line")), 2)
        self.assertEqual(r["S"]["consumed_parsed"], 3)        # the A1 gate's consumed_exit_address line is absent too (4 expected)
        self.assertEqual(r["S"]["consumed_expected"], 4)
        self.assertIn("MISSING", r["report"])

    def test_recast(self):
        r = self.run_case("recast")
        rec = r["T"]["recast"]
        self.assertEqual(len(rec), 1)                         # 5 rounds flagged; the 3-round creature is allowed
        self.assertEqual((rec[0]["cr"], rec[0]["n"], rec[0]["code"], rec[0]["spell"]), ("7f0001a2", 5, "2300", "28"))
        self.assertTrue(self.fails(r, "recast pattern"))
        self.assertEqual(self.status(r, "CAI-2"), "FAIL")

    def test_e1_skipped_too_long(self):
        r = self.run_case("e1_skipped_long")
        bad = r["T"]["e1_skip_bad"]
        self.assertEqual(len(bad), 1)
        self.assertAlmostEqual(bad[0]["gap"], 35000, delta=1)
        self.assertTrue(bad[0]["had_attack"])
        self.assertTrue(self.fails(r, "Party E1: skipped for"))
        self.assertEqual(self.status(r, "CAI-6"), "FAIL")

    def test_healthy_e1_skipped_is_not_flagged(self):
        self.assertEqual(self.run_case("healthy")["T"]["e1_skip_bad"], [])

    def test_unparseable_config(self):
        r = self.run_case("unparseable_config")
        self.assertEqual(len(r["T"]["cfg_bad"]), 1)
        self.assertTrue(self.fails(r, "unparseable value"))
        self.assertIn("**FLAGGED**", r["report"])

    def test_file_cap_marker(self):
        r = self.run_case("file_cap")
        self.assertEqual(len(r["T"]["caps"]), 1)
        self.assertTrue(any("file cap" in m for s, _, m in r["an"].items if s == "WARN"))

    def test_inventory_kplog_fallback_shape(self):
        # 0.2.0+ lines carry the KpLog fallback prefix "<ms> inventory-sort-filter/main "; same answers as the raw shape
        raw, kp = self.run_case("healthy")["I"], self.run_case("inventory_kplog")["I"]
        for k in ("cfg", "flushes", "empty_flushes", "cycles"):
            self.assertEqual([x["state"] if isinstance(x, dict) and "state" in x else 1 for x in kp[k]],
                             [x["state"] if isinstance(x, dict) and "state" in x else 1 for x in raw[k]], k)
        self.assertEqual((kp["clicks"], len(kp["loaded"]), kp["errs"]), (raw["clicks"], len(raw["loaded"]), raw["errs"]))
        self.assertEqual(len(kp["flushes"]), 5)

    def test_inventory_core_shape_and_names(self):
        # KpLog core file inventory-sort-filter_log.txt ("# kplog session" header + "T=..." prefix): same answers as the fallback shape
        kp, core = self.run_case("inventory_kplog")["I"], self.run_case("inventory_core")["I"]
        self.assertEqual(core["name"], "inventory-sort-filter_log.txt")
        self.assertEqual((core["clicks"], len(core["cfg"]), len(core["flushes"]), len(core["loaded"]), core["errs"]),
                         (kp["clicks"], len(kp["cfg"]), len(kp["flushes"]), len(kp["loaded"]), kp["errs"]))
        self.assertEqual(len(core["loaded"]), 1)
        d = self.copy_case("inventory_core")
        shutil.copy(FIX / "inventory_kplog" / "inventory_sort_filter_log.txt", d)
        for n in apl.INV_NAMES:
            os.utime(d / n, ns=(10**18, 10**18))
        self.assertEqual(apl.find_files(d)["inv"].name, "inventory_sort_filter_log.txt")
        os.utime(d / "inventory-sort-filter_log.txt", ns=(2 * 10**18, 2 * 10**18))
        self.assertEqual(apl.find_files(d)["inv"].name, "inventory-sort-filter_log.txt")

    def test_inventory_errors(self):
        r = self.run_case("inventory_errors")
        self.assertEqual(set(r["I"]["errs"]), {"bad frame", "counter"})
        self.assertEqual(self.status(r, "M6-1"), "FAIL")
        self.assertTrue(self.fails(r, "'bad frame'"))
        self.assertTrue(self.fails(r, "'counter'"))


class MissingFiles(Base):
    def test_only_steam_log(self):
        d = self.copy_case("missing_files")
        r = apl.run_full([str(d)] + MINI)
        self.assertEqual(r["rc"], 0)
        text = (d / "REPORT.md").read_text()
        for name in ("ai_trace.txt", "ranged_ai_log.txt", "inventory_sort_filter_log.txt"):
            self.assertIn(f"{name}: not present.", text)
        self.assertIn("**missing**", text)
        self.assertEqual([st for _, st, *_ in r["checklist"]].count("OBSERVE"), len(apl.CHECKLIST))
        self.assertTrue(all(row["status"] == "OK" for row in r["S"]["rows"]))

    def test_empty_directory(self):
        d = self.tmp / "empty"
        d.mkdir()
        r = apl.run_full([str(d)] + MINI)
        self.assertEqual(r["rc"], 0)
        text = (d / "REPORT.md").read_text()
        self.assertIn("steam log: not present.", text)
        self.assertEqual(len(r["checklist"]), len(apl.CHECKLIST))

    def test_not_a_directory(self):
        self.assertEqual(apl.run_full([str(self.tmp / "nope")])["rc"], 1)


class RangedLogShapes(Base):
    """ranged-ai 0.4.0 (KpLog) shapes give the same counts as the legacy raw file."""
    def counts(self, case):
        r = self.run_case(case)
        RG = r["RG"]
        return r, (dict(RG["s_counts"]), sum(c["n"] for c in RG["h1"].values()), len(RG["h2"]), sum(RG["h3"].values()), len(RG["cfgs"]), len(RG["loaded"]))

    def test_same_counts_in_every_shape(self):
        _, legacy = self.counts("healthy")
        for case in ("ranged_fallback", "ranged_core"):
            r, c = self.counts(case)
            self.assertEqual(c, legacy, case)
            self.assertEqual(self.fails(r), [], case)
            self.assertIsNone(r["RG"]["cap"], case)

    def test_core_file_name_is_found(self):
        r, _ = self.counts("ranged_core")
        self.assertEqual(r["RG"]["name"], "ranged-ai_log.txt")

    def test_both_names_tie_goes_to_legacy(self):
        d = self.copy_case("ranged_core")
        shutil.copy(FIX / "ranged_fallback" / "ranged_ai_log.txt", d)
        for n in ("ranged_ai_log.txt", "ranged-ai_log.txt"):
            os.utime(d / n, ns=(10**18, 10**18))
        self.assertEqual(apl.find_files(d)["ranged"].name, "ranged_ai_log.txt")
        os.utime(d / "ranged-ai_log.txt", ns=(2 * 10**18, 2 * 10**18))
        self.assertEqual(apl.find_files(d)["ranged"].name, "ranged-ai_log.txt")

    def test_dropped_marks_are_one_observe_anomaly(self):
        r, c = self.counts("ranged_core_drops")
        self.assertEqual(c, self.counts("healthy")[1])
        drops = [m for s, _src, m in r["an"].items if "KpLog MARK/DROPPED" in m]
        self.assertEqual(len(drops), 1)
        self.assertIn("2 KpLog MARK/DROPPED", drops[0])
        self.assertEqual(r["RG"]["drops"][0].split()[0] != "", True)
        self.assertEqual([m for s, _src, m in self.counts("ranged_core")[0]["an"].items if "MARK/DROPPED" in m], [])

    def test_cap_line_matches_through_the_prefix(self):
        d = self.copy_case("ranged_fallback")
        with open(d / "ranged_ai_log.txt", "a") as f:
            f.write("1999999 ranged-ai/main -- log cap reached, further lines dropped --\n")
        r = apl.run_full([str(d)] + MINI)
        self.assertIsNotNone(r["RG"]["cap"])


class RealHookTomls(Base):
    def test_default_tomls_match_the_task_spec(self):
        hooks, notes = apl.load_hooks(None)
        self.assertEqual(notes, [])
        by = {}
        for h in hooks:
            by.setdefault(h["patch"], []).append(h)
        self.assertEqual(len(by["companion-ai"]), 31)
        self.assertEqual(len(by["ranged-ai"]), 7)
        self.assertIn("inventory-sort-filter", by)
        self.assertEqual(len({(h["patch"], h["address"]) for h in hooks}), len(hooks))
        self.assertEqual(sum(1 for h in hooks if h["consumed"] is not None), 3 + 5 + len(
            [h for h in by["inventory-sort-filter"] if h["consumed"] is not None]))

    def _steam_for(self, hooks, skip=(), skip_consumed=()):
        lines = []
        for h in hooks:
            if h["address"] in skip:
                continue
            if h["consumed"] is not None and h["address"] not in skip_consumed:
                lines.append(f'1.0:0138:013c:warn:debugstr:OutputDebugStringA "[Config] Parsed consumed_exit_address = 0x{h["consumed"]:08X} '
                             f'for hook at 0x{h["address"]:08X}\\n"')
            lines.append(f'1.0:0138:013c:warn:debugstr:OutputDebugStringA "[Config] Loaded DETOUR hook: {h["patch"]} -> {h["function"]} '
                         f'@ 0x{h["address"]:08X} (6 bytes)\\n"')
            lines.append(f'1.0:0138:013c:warn:debugstr:OutputDebugStringA "[KotorPatcher] Applied DETOUR hook at 0x{h["address"]:08X} '
                         f'-> {h["function"]}\\n"')
        return "\n".join(lines) + "\n"

    def test_complete_and_one_missing(self):
        hooks, _ = apl.load_hooks(None)
        d = self.tmp / "full"
        d.mkdir()
        (d / "steam-208580.log").write_text(self._steam_for(hooks))
        r = apl.run_full([str(d), "--no-write"])
        self.assertEqual([x for x in r["S"]["rows"] if x["status"] != "OK"], [])
        self.assertEqual(r["S"]["consumed_parsed"], r["S"]["consumed_expected"])
        self.assertEqual(self.fails(r, ""), [])
        # drop the A1 gate hook (consumed exit) of companion-ai
        gate = next(h for h in hooks if h["function"] == "CompanionAiA1Gate")
        d2 = self.tmp / "one_missing"
        d2.mkdir()
        (d2 / "steam-208580.log").write_text(self._steam_for(hooks, skip={gate["address"]}))
        r2 = apl.run_full([str(d2), "--no-write"])
        self.assertEqual([x["address"] for x in r2["S"]["rows"] if x["status"] == "MISSING"], [gate["address"]])
        self.assertTrue(self.fails(r2, "companion-ai: 1/31"))

    def test_applied_but_consumed_exit_not_parsed(self):
        hooks, _ = apl.load_hooks(None)
        gate = next(h for h in hooks if h["function"] == "CompanionAiA1Gate")
        d = self.tmp / "noconsumed"
        d.mkdir()
        (d / "steam-208580.log").write_text(self._steam_for(hooks, skip_consumed={gate["address"]}))
        r = apl.run_full([str(d), "--no-write"])
        self.assertEqual(r["S"]["consumed_parsed"], r["S"]["consumed_expected"] - 1)
        self.assertTrue(self.fails(r, "consumed_exit_address not parsed"))

    def test_hooks_toml_override(self):
        hooks, notes = apl.load_hooks([f"x={FIX / '_hooks' / 'ranged-ai.hooks.toml'}"])
        self.assertEqual([h["patch"] for h in hooks], ["x", "x"])
        self.assertEqual(notes, [])


class RealSavedLogs(Base):
    @unittest.skipUnless(REAL.is_dir(), "saved playlogs dir not present")
    def test_copy_of_saved_session_without_ai_trace(self):
        existed = (REAL / "REPORT.md").exists()
        d = self.tmp / "playlogs_copy"
        shutil.copytree(REAL, d)
        self.assertFalse((d / "ai_trace.txt").exists())
        r = apl.run_full([str(d)])
        self.assertEqual(r["rc"], 0)
        text = (d / "REPORT.md").read_text()
        self.assertTrue(text.startswith("# Play-log report"))
        self.assertIn("ai_trace.txt: not present.", text)
        self.assertEqual(r["S"]["rows"] and sum(1 for x in r["S"]["rows"] if x["patch"] == "ranged-ai" and x["status"] == "OK"), 7)
        self.assertEqual(r["RG"]["s_counts"]["S2"], 3)       # the saved ranged log has three S2 lines
        self.assertEqual(len(r["checklist"]), len(apl.CHECKLIST))
        self.assertEqual((REAL / "REPORT.md").exists(), existed)   # the original directory is never written

class ProbeChecks(Base):
    """Phase-4 probe checks (PR-*): PASS / FAIL / OBSERVE per PROBE_DESIGN.md section 2."""
    HDR = ("# companion-ai 0.2.0-probe session 2026-10-02 02:00:00 t=2000000 pid=1234 tid=100 cwd=Z:\\game ini=Z:\\game\\companion_ai.ini\n"
           "# config fixes a1=1 a1_stances=11800 a1_party_only=1 a1_retarget=1 a1_retarget_stances=11800 a1_rethink_ms=3000 p1=0 p1_interval_ms=250 "
           "fm1=1 fm1_rethink_ms=0 fm4=1 a4=1 c1=1 c1_party_only=1 a3=0 log=1\n")
    END = "t=2300000 S lines=200 dropped=0 deduped=0 capDropped=0 | flips=40 prints=38 aur=4 e1=0 e1spins=0 e1b=0 e2=2 e3=11\n"
    PR_IDS = ["PR-FM2B", "PR-FM2", "PR-FM3", "PR-SLOT6", "PR-FM5", "PR-E9", "PR-P2", "PR-PF", "PR-RAR", "PR-SHOUT", "PR-SS", "PR-E4",
              "PR-PARTY", "PR-A1X", "PR-RAH2"]

    def trace(self, *lines, ranged=None) -> dict:
        d = Path(tempfile.mkdtemp(dir=self.tmp)) / "case"
        d.mkdir()
        (d / "ai_trace.txt").write_text(self.HDR + "\n".join(lines) + "\n" + self.END)
        if ranged is not None:
            (d / "ranged_ai_log.txt").write_text("\n".join(ranged) + "\n")
        return apl.run_full([str(d), "--no-write"] + MINI)

    def test_every_check_is_in_the_checklist(self):
        self.assertEqual([i for i, _ in apl.CHECKLIST if i.startswith("PR-")], self.PR_IDS)
        self.assertEqual(apl.PR_CHECK_IDS, self.PR_IDS)
        self.assertEqual(apl.EXPECTED_HOOK_COUNTS["companion-ai"], 31)

    def test_pass_fixture(self):
        r = self.run_case("probe_pass")
        for cid in self.PR_IDS:
            self.assertEqual(self.status(r, cid), "PASS", (cid, [x for x in r["checklist"] if x[0] == cid]))
        self.assertEqual([m for s, _, m in r["an"].items if s in ("FAIL", "WARN")], [])
        self.assertEqual((r["P"]["n_pr"], r["P"]["n_prs"]), (25, 9))                  # the fixture has 25 PR and 9 PRS lines
        self.assertNotIn("t= PR", "".join(r["T"]["other_counts"]))      # probe lines are not 'other/unrecognised'
        self.assertEqual(r["T"]["n_pa"], 0)                              # and never P/A script lines (PA_RE must not match "PR")
        self.assertNotIn("unparsed", r["P"])

    def test_fail_fixture(self):
        d = self.copy_case("probe_fail")
        r = apl.run_full([str(d)] + MINI)
        for cid in self.PR_IDS:
            self.assertEqual(self.status(r, cid), "PASS" if cid == "PR-SLOT6" else "FAIL", (cid, [x for x in r["checklist"] if x[0] == cid]))
        fails = [m for s, _, m in r["an"].items if s == "FAIL"]
        self.assertEqual(len(fails), 14)
        self.assertTrue(all(m for m in fails))
        text = (d / "REPORT.md").read_text()
        self.assertIn("## 8. Phase-4 probe summaries", text)
        self.assertIn("Last PRS line per probe", text)
        self.assertEqual(apl.run_full([str(self.copy_case("probe_fail")), "--strict"] + MINI)["rc"], 2)

    def test_healthy_session_observes_probe_checks(self):
        r = self.run_case("healthy")
        for cid in self.PR_IDS:
            self.assertEqual(self.status(r, cid), "PASS" if cid == "PR-RAH2" else "OBSERVE", cid)

    def test_no_probe_lines_all_observe(self):
        r = self.trace("t=2001000 P 7f0001a2 p_atton k_ai_master ev=1003 d=1 | GENERIC DEBUG *************** START DETERMINE COMBAT ROUND p_atton")
        for cid in self.PR_IDS:
            self.assertEqual(self.status(r, cid), "OBSERVE", cid)

    def test_p2_thresholds(self):
        for avg, want in ((3900, "PASS"), (4500, "PASS"), (3499, "FAIL"), (3500, "OBSERVE"), (3899, "OBSERVE"), (4501, "OBSERVE")):
            r = self.trace(f"t=2100000 PRS P2 cr=7f0001a2 runs=10 avg={avg} max=5000 ctrlRuns=1")
            self.assertEqual(self.status(r, "PR-P2"), want, avg)
        r = self.trace("t=2100000 PRS P2 cr=7f0001a2 runs=2 avg=0 max=0 ctrlRuns=0")   # too few runs for an average
        self.assertEqual(self.status(r, "PR-P2"), "OBSERVE")
        r = self.trace("t=2100000 PRS P2 cr=7f0001a2 runs=10 avg=4000 max=4100 ctrlRuns=0", "t=2100000 PRS P2 cr=7f0001a4 runs=10 avg=3000 max=4100 ctrlRuns=0")
        self.assertEqual(self.status(r, "PR-P2"), "FAIL")                                # one fast companion fails the lot

    def test_fm5_threshold(self):
        for mx, want in ((4, "PASS"), (5, "FAIL")):
            r = self.trace(f"t=2100000 PRS RS cr=7f0001a2 s6a=0 s6b=0 s6c=0 s7=0 s12=50 s1=0 max12ps={mx}")
            self.assertEqual(self.status(r, "PR-FM5"), want, mx)
        r = self.trace("t=2100000 PRS RS cr=7f0001a2 s6a=1 s6b=0 s6c=0 s7=0 s12=0 s1=0 max12ps=0")
        self.assertEqual(self.status(r, "PR-FM5"), "OBSERVE")

    def test_fm3(self):
        r = self.trace("t=2100000 PRS FM3 passes=0 lost=0 removes=0 beforeCursor=0")
        self.assertEqual(self.status(r, "PR-FM3"), "OBSERVE")
        r = self.trace("t=2100000 PRS FM3 passes=60 lost=6 removes=9 beforeCursor=2")
        self.assertEqual(self.status(r, "PR-FM3"), "PASS")
        self.assertIn("lost/min", [x for x in r["checklist"] if x[0] == "PR-FM3"][0][2])
        r = self.trace("t=2100000 PRS FM3 passes=60 lost=6 removes=9 beforeCursor=2",
                       "t=2100001 PR FM3L level=4 sentinel=0a1b2c3d now=4 removed=0 done=1 elapsed=1 slice=2")
        self.assertEqual(self.status(r, "PR-FM3"), "FAIL")
        r = self.trace("t=2100001 PR FM3L level=4 sentinel=0a1b2c3d now=2 removed=0 done=1 elapsed=1 slice=2",
                       "t=2100002 PR FM3L level=2 sentinel=0a1b2c3d now=2 removed=1 done=1 elapsed=1 slice=2")
        self.assertEqual(self.status(r, "PR-FM3"), "OBSERVE")                           # lost lines but no PRS with passes: nothing to rate

    def test_fm2_pairs(self):
        base = "t=%d PR FM2R cr=7f0001a2 tgt=7f000233 other=0 inr=0 range=2.50 d=12.30 prog=0.00 n=%d"
        two = [base % (2011000, 1), base % (2011300, 2)]
        self.assertEqual(self.status(self.trace(*two), "PR-FM2"), "OBSERVE")             # n < 3
        three = two + [base % (2011600, 3)]
        self.assertEqual(self.status(self.trace(*three), "PR-FM2"), "FAIL")              # no FM2C/FM2F
        c = "t=2005000 PR FM2C cr=7f0001a2 tgt=7f000233 res=3 code=-4 cached=-1 cp=0 calls=4 n2=0 n3=1"
        self.assertEqual(self.status(self.trace(c, *three), "PR-FM2"), "PASS")
        far = "t=1900000 PR FM2C cr=7f0001a2 tgt=7f000233 res=3 code=-4 cached=-1 cp=0 calls=4 n2=0 n3=1"   # > 30 s away
        self.assertEqual(self.status(self.trace(far, *three), "PR-FM2"), "FAIL")
        f = "t=2012000 PR FM2F cr=7f0001a2 tgt=7f000233 grp=1 n=3 span=900ms ctrl=1"
        self.assertEqual(self.status(self.trace(f, *three), "PR-FM2"), "PASS")
        # FM2B
        self.assertEqual(self.status(self.trace(c), "PR-FM2B"), "PASS")
        self.assertEqual(self.status(self.trace(c.replace("cr=7f0001a2", "cr=00000000")), "PR-FM2B"), "FAIL")
        self.assertEqual(self.status(self.trace(c.replace("res=3", "res=4")), "PR-FM2B"), "FAIL")

    def test_e9_and_slot6(self):
        s7 = "t=2020300 PR RS site=S7 cr=7f0001a2 p_atton party=1 empty=0 spk=7f000300 spkParty=0 pat=15 spkTag=%s"
        shx = "t=2030000 PR SHX end spk=7f000300 type=%d n_mandsold party=0 r=35 seen=0 idx=3 count=3 last=7f000304 text=help me"
        e9 = "t=2100000 PRS E9 deliveries=%d"
        r = self.trace(e9 % 3, s7 % "n_mandsold", s7 % "n_mandsold", shx % 5)
        self.assertEqual(self.status(r, "PR-E9"), "PASS")                                 # the count is budget-free; the tags are reported
        self.assertIn("n_mandsold x2", [x for x in r["checklist"] if x[0] == "PR-E9"][0][2])
        self.assertEqual(self.status(self.trace(e9 % 3), "PR-E9"), "PASS")                # no S7 line needed (S7 budget used up)
        self.assertEqual(self.status(self.trace(e9 % 0, shx % 5), "PR-E9"), "FAIL")      # creature r=35 shouts, zero deliveries
        self.assertEqual(self.status(self.trace(e9 % 0, shx % 3), "PR-E9"), "OBSERVE")   # speaker is a placeable, not a creature
        self.assertEqual(self.status(self.trace(e9 % 0, shx % 5 + "\n" + shx.replace("party=0", "party=1") % 5), "PR-E9"), "FAIL")
        self.assertEqual(self.status(self.trace(e9 % 0, (shx % 5).replace("party=0", "party=1")), "PR-E9"), "OBSERVE")   # party speaker
        self.assertEqual(self.status(self.trace(shx % 5), "PR-E9"), "OBSERVE")            # no PRS E9 line: no data, not zero
        self.assertEqual(self.status(self.trace(e9 % 0, (shx % 5).replace("r=35", "r=20")), "PR-E9"), "OBSERVE")
        self.assertEqual(self.status(self.trace(s7 % "x"), "PR-E9"), "OBSERVE")           # S7 lines alone do not decide
        self.assertEqual(self.status(self.trace(), "PR-E9"), "OBSERVE")
        s6 = "t=2020000 PR RS site=%s cr=7f0001a2 p_atton party=%d empty=0"
        self.assertEqual(self.status(self.trace(s6 % ("S6A", 1)), "PR-SLOT6"), "PASS")
        self.assertEqual(self.status(self.trace(s6 % ("S6C", 1)), "PR-SLOT6"), "OBSERVE")   # S6C alone does not answer the cadence question
        self.assertEqual(self.status(self.trace(s6 % ("S6B", 0)), "PR-SLOT6"), "OBSERVE")

    def test_pf_rar_shout(self):
        pf = "t=2001500 PR PF cr=7f0001a2 p_atton old=1 new=0 apply=1 site=%s"
        prs = "t=2100000 PRS PF saveA=%d saveB=%d xferA=1 xferB=1 other=0"
        self.assertEqual(self.status(self.trace(pf % "SAVE_A", prs % (2, 2)), "PR-PF"), "PASS")
        self.assertEqual(self.status(self.trace(pf % "SAVE_A", prs % (2, 1)), "PR-PF"), "FAIL")
        self.assertEqual(self.status(self.trace(pf % "?", prs % (2, 2)), "PR-PF"), "FAIL")
        rar = "t=2100000 PRS RAR cr=7f0001a2 ispc=0 n=9 %s"
        self.assertEqual(self.status(self.trace(rar % "c:1/0/7/1 11:0/0/1/0"), "PR-RAR"), "PASS")
        self.assertEqual(self.status(self.trace(rar % "c:1/0/7/1 0:0/0/4/0"), "PR-RAR"), "PASS")      # type 0 / > 0x47 with result 3 only
        self.assertEqual(self.status(self.trace(rar % "c:1/0/7/1 0:0/0/4/1"), "PR-RAR"), "FAIL")
        self.assertEqual(self.status(self.trace(rar % "0:0/0/0/0+o3"), "PR-RAR"), "FAIL")             # out-of-range type with an "other" result
        self.assertEqual(apl.parse_rar_hist("c:1/2/3/4+o5 11:0/0/1/0"), {0xC: (1, 2, 3, 4, 5), 0x11: (0, 0, 1, 0, 0)})
        sh = "t=2100000 PRS SHX shouts=%d r35=1 partySpk=0 end=%d xbreak=0 noarea=0 abortStartCr=%d abortStartOther=%d abortMid=%d"
        self.assertEqual(self.status(self.trace(sh % (5, 5, 0, 0, 0)), "PR-SHOUT"), "PASS")
        self.assertEqual(self.status(self.trace(sh % (5, 4, 1, 0, 0)), "PR-SHOUT"), "FAIL")          # creature speaker, stale id
        self.assertEqual(self.status(self.trace(sh % (5, 4, 0, 0, 1)), "PR-SHOUT"), "FAIL")          # ABORT-MID
        self.assertEqual(self.status(self.trace(sh % (0, 0, 0, 0, 0)), "PR-SHOUT"), "OBSERVE")
        # SF5: ABORT-START of a non-creature speaker (not in the area list, no creature at x >= sx-r) is the vanilla no-listener case: INFO only
        r = self.trace(sh % (5, 3, 0, 2, 0))
        self.assertEqual(self.status(r, "PR-SHOUT"), "PASS")
        self.assertTrue(any(s == "INFO" and "non-creature speaker" in m for s, _, m in r["an"].items))
        ln = "t=2030100 PR SHX %s spk=7f000300 type=%d n_x party=0 r=35 seen=0 idx=0 count=3 last=7f000304 text=help me"
        self.assertEqual(self.status(self.trace(ln % ("ABORT-START", 6)), "PR-SHOUT"), "OBSERVE")   # lines alone: placeable speaker, not the bug
        self.assertEqual(self.status(self.trace(ln % ("ABORT-START", 5)), "PR-SHOUT"), "FAIL")
        self.assertEqual(self.status(self.trace(ln % ("ABORT-MID", 6)), "PR-SHOUT"), "FAIL")

    def test_ss_window_and_budget(self):
        ss = "t=2040000 PR SS1 cr=7f0001a2 p_atton state 0->%d combat=1"
        s6c = "t=%d PR RS site=S6C cr=7f0001a2 p_atton party=1 empty=0"
        self.assertEqual(self.status(self.trace(ss % 5, s6c % 2059999), "PR-SS"), "PASS")
        self.assertEqual(self.status(self.trace(ss % 5, s6c % 2100001), "PR-SS"), "FAIL")           # 60.001 s later
        self.assertEqual(self.status(self.trace(ss % 5), "PR-SS"), "FAIL")
        self.assertEqual(self.status(self.trace(ss % 2), "PR-SS"), "OBSERVE")                        # state 2 does not remove the slot-6 script
        self.assertEqual(self.status(self.trace(ss % 0x14), "PR-SS"), "OBSERVE")
        self.assertEqual(self.status(self.trace(ss % 0x13), "PR-SS"), "FAIL")
        late = self.trace("t=2290000 PR SS1 cr=7f0001a2 p_atton state 0->5 combat=1")               # the log ends inside the 60 s window
        self.assertEqual(self.status(late, "PR-SS"), "OBSERVE")
        many = [s6c % (2000000 + 10 * i) for i in range(20)]                                          # S6C line budget (20) used up: inconclusive
        self.assertEqual(self.status(self.trace(ss % 5, *many), "PR-SS"), "OBSERVE")
        r = self.trace("-- probe RAR line budget reached --", "-- probe RS line budget reached --")
        self.assertTrue(any(s == "INFO" and "budget" in m and "RAR" in m and "RS" in m for s, _, m in r["an"].items))

    def test_sf4_budget_and_drop_markers_make_absence_unjudgeable(self):
        ss = "t=2040000 PR SS1 cr=7f0001a2 p_atton state 0->5 combat=1"
        rs_budget = "-- probe RS line budget reached --"
        # no S6C PR line, no PRS RS: with the RS budget gone before the SS1 the absence proves nothing
        self.assertEqual(self.status(self.trace("t=2000000 PR PF cr=7f0001a2 p_atton old=1 new=0 apply=1 site=SAVE_A", rs_budget, ss), "PR-SS"), "OBSERVE")
        self.assertEqual(self.status(self.trace(ss, "t=2041000 PR SS1 cr=7f0001a2 p_atton state 0->5 combat=1", rs_budget), "PR-SS"), "OBSERVE")   # ran out inside the window
        self.assertEqual(self.status(self.trace(ss), "PR-SS"), "FAIL")                      # control: no marker
        # a budget marker only for another probe does not matter
        self.assertEqual(self.status(self.trace("t=2000000 PR PF cr=7f0001a2 p_atton old=1 new=0 apply=1 site=SAVE_A", "-- probe RAR line budget reached --", ss), "PR-SS"), "FAIL")
        # the budget-free PRS RS counters judge instead: a rise within 60 s (+ one summary) is a run, none is a miss, whatever the marker says
        prs = "t=%d PRS RS cr=7f0001a2 s6a=0 s6b=0 s6c=%d s7=0 s12=0 s1=0 max12ps=0"
        self.assertEqual(self.status(self.trace(prs % (2030000, 1), rs_budget, ss, prs % (2095000, 2)), "PR-SS"), "PASS")    # rise 55 s after
        self.assertEqual(self.status(self.trace(prs % (2030000, 1), rs_budget, ss, prs % (2105000, 2)), "PR-SS"), "PASS")    # 65 s: inside window + summary_sec
        self.assertEqual(self.status(self.trace(prs % (2030000, 1), rs_budget, ss, prs % (2111000, 2)), "PR-SS"), "FAIL")    # 71 s: outside
        self.assertEqual(self.status(self.trace(prs % (2030000, 1), rs_budget, ss, prs % (2060000, 1)), "PR-SS"), "FAIL")    # no s6c rise
        self.assertEqual(self.status(self.trace(prs % (2030000, 1), ss, prs % (2060000, 1), "t=2290000 PRS RS cr=7f0001a4 s6a=1 s6b=0 s6c=0 s7=0 s12=0 s1=0 max12ps=0"),
                                     "PR-SS"), "FAIL")
        # a per-second-cap drop marker inside the window (PR-line fallback only)
        drop = "-- ai_trace: 40 lines dropped by the per-second cap (200/s) at t=2050000 --"
        self.assertEqual(self.status(self.trace(ss, drop), "PR-SS"), "OBSERVE")
        self.assertEqual(self.status(self.trace(ss, "-- ai_trace: 40 lines dropped by the per-second cap (200/s) at t=2200000 --"), "PR-SS"), "FAIL")
        # PR-FM2: n >= 3 pair without FM2C/FM2F is FAIL only while those probes' budgets held
        base = "t=%d PR FM2R cr=7f0001a2 tgt=7f000233 other=0 inr=0 range=2.50 d=12.30 prog=0.00 n=%d"
        three = [base % (2011000, 1), base % (2011300, 2), base % (2011600, 3)]
        self.assertEqual(self.status(self.trace(*three), "PR-FM2"), "FAIL")
        self.assertEqual(self.status(self.trace("-- probe FM2C line budget reached --", *three), "PR-FM2"), "OBSERVE")
        self.assertEqual(self.status(self.trace("-- probe FM2F line budget reached --", *three), "PR-FM2"), "OBSERVE")
        self.assertEqual(self.status(self.trace("-- probe FM2R line budget reached --", *three), "PR-FM2"), "FAIL")          # not an FM2C/FM2F budget
        self.assertEqual(self.status(self.trace(*three, "-- ai_trace: 40 lines dropped by the per-second cap (200/s) at t=2011700 --"), "PR-FM2"), "OBSERVE")
        c = "t=2005000 PR FM2C cr=7f0001a2 tgt=7f000233 res=3 code=-4 cached=-1 cp=0 calls=4 n2=0 n3=1"
        self.assertEqual(self.status(self.trace(c, "-- probe FM2C line budget reached --", *three), "PR-FM2"), "PASS")      # evidence present
        # PR-A1X: an end whose FOLLOWUP window ran past the A1XF budget is not judged
        a1 = "t=2080000 A1 cr=7f0001a4 party=1 raw=11 eff=11 reason=range dist=40.0 max=2.2 los=1 ranged=1 area=same tgt=7f000233 -> end new=ffffffff (wouldSpin=1) c524=0 f570=3000 combat=1"
        pad = "t=2081000 PR A1X STRICT cr=7f0001a4 new=7f000240 d2=9.00 lim2=5.29 -> ok"
        self.assertEqual(self.status(self.trace(a1), "PR-A1X"), "FAIL")
        r = self.trace(a1, pad, "-- probe A1XF line budget reached --")
        self.assertEqual(self.status(r, "PR-A1X"), "PASS")                                   # budget ran out inside the window: the end is not judged
        self.assertIn("1 not judged", [x for x in r["checklist"] if x[0] == "PR-A1X"][0][2])
        self.assertEqual(self.status(self.trace("-- probe A1XF line budget reached --", a1), "PR-A1X"), "PASS")
        self.assertEqual(self.status(self.trace(a1, "-- probe A1X line budget reached --"), "PR-A1X"), "FAIL")                # STRICT budget is irrelevant
        self.assertEqual(self.status(self.trace(a1, "t=2082000 PR A1X FOLLOWUP cr=7f0001a4 site=S6B after=2000ms", "-- probe A1XF line budget reached --"), "PR-A1X"), "PASS")
        late = "t=2090000 PR A1X STRICT cr=7f0001a4 new=7f000240 d2=9.00 lim2=5.29 -> ok"
        self.assertEqual(self.status(self.trace(a1, late, "-- probe A1XF line budget reached --"), "PR-A1X"), "FAIL")       # budget held for the whole window: judged
        self.assertEqual(self.status(self.trace(a1, "-- ai_trace: 3 lines dropped by the per-second cap (200/s) at t=2082000 --"), "PR-A1X"), "PASS")

    def test_sf2_a1_summary_strict_counts(self):
        a1 = "t=2080000 A1 cr=7f0001a4 party=1 raw=11 eff=11 reason=range dist=40.0 max=2.2 los=1 ranged=1 area=same tgt=7f000233 -> retarget new=7f000240 (wouldSpin=1) c524=0 f570=3000 combat=1"
        r = self.trace(a1, "t=2090000 A1 summary: cr=7f0001a4 would-spin passes=9/10s ends=0 retargets=9 strict=9/4",
                       "t=2100000 A1 summary: cr=7f0001a4 would-spin passes=3/10s ends=0 retargets=3 strict=3/1",
                       "t=2080010 PR A1X STRICT cr=7f0001a4 new=7f000240 d2=9.00 lim2=5.29 -> ok")
        self.assertIn("STRICT would-reject 5/12 (A1 summaries)", [x for x in r["checklist"] if x[0] == "PR-A1X"][0][2])   # budget-free totals beat the throttled lines
        r = self.trace(a1, "t=2090000 A1 summary: cr=7f0001a4 would-spin passes=9/10s ends=0 retargets=9")       # old summary format: no strict counts
        self.assertIn("STRICT would-reject 0/0", [x for x in r["checklist"] if x[0] == "PR-A1X"][0][2])

    def test_e4_party_a1x(self):
        g = "t=2050000 PR GCA cr=7f0001a2 res=%d lastRun=%x moving=1"
        self.assertEqual(self.status(self.trace(g % (3, 0x11)), "PR-E4"), "PASS")
        self.assertEqual(self.status(self.trace(g % (39, 0x3F)), "PR-E4"), "PASS")
        self.assertEqual(self.status(self.trace(g % (38, 0x11)), "PR-E4"), "OBSERVE")
        self.assertEqual(self.status(self.trace(g % (3, 0x1)), "PR-E4"), "OBSERVE")
        self.assertEqual(self.status(self.trace(g % (0, 0x11)), "PR-E4"), "FAIL")
        self.assertEqual(self.status(self.trace(g % (0, 0x11), g % (3, 0xC)), "PR-E4"), "FAIL")      # PASS needs no FAIL line
        pt = "t=2001000 PR PARTY why=%s ctrl=7f0001a0 f=%d/1 pobj=%s f=1/1 pc=7f0001a0 tbl=2 [0:7f0001a0:1:1,1:7f0001a2:%d:0]"
        self.assertEqual(self.status(self.trace(pt % ("load", 1, "7f0001a0", 1)), "PR-PARTY"), "PASS")
        self.assertEqual(self.status(self.trace(pt % ("load", 1, "7f0001a0", 0)), "PR-PARTY"), "FAIL")
        self.assertEqual(self.status(self.trace(pt % ("load", 0, "7f0001a0", 1)), "PR-PARTY"), "FAIL")
        self.assertEqual(self.status(self.trace(pt % ("pf", 1, "7f0001a0", 0)), "PR-PARTY"), "PASS")     # mid-save lines are not judged
        r = self.trace(pt % ("leader", 1, "7f0001a2", 1))
        self.assertEqual(self.status(r, "PR-PARTY"), "PASS")
        self.assertIn("pobj != ctrl after a leader switch x1", [x for x in r["checklist"] if x[0] == "PR-PARTY"][0][2])
        a1 = "t=2080000 A1 cr=7f0001a4 party=1 raw=11 eff=11 reason=range dist=40.0 max=2.2 los=1 ranged=1 area=same tgt=7f000233 -> %s new=%s (wouldSpin=1)%s"
        sfx = " c524=0 f570=3000 combat=1"
        e2 = "t=2083050 E2 FALLBACK cr=7f0001a4 p_visas party=1 combat=1 round=0 script=k_hen_visasdlg a1spin=0 lastA1=3050ms"
        end = a1 % ("end", "ffffffff", sfx)
        f = "t=%d PR A1X FOLLOWUP cr=7f0001a4 site=S6B after=%dms"
        self.assertEqual(self.status(self.trace(a1 % ("end", "ffffffff", ""), e2), "PR-A1X"), "OBSERVE")   # A1 line without the A1X suffix
        self.assertEqual(self.status(self.trace(end, e2, f % (2084000, 4000)), "PR-A1X"), "PASS")             # rethink + 1000 = 4000 ms
        self.assertEqual(self.status(self.trace(end, e2, f % (2084001, 4001)), "PR-A1X"), "FAIL")
        self.assertEqual(self.status(self.trace(end, e2), "PR-A1X"), "FAIL")
        orph = "t=2084100 PR A1X ORPHAN cr=7f0001a4 since=4100ms"
        self.assertEqual(self.status(self.trace(end, e2, f % (2083100, 3100), orph), "PR-A1X"), "FAIL")
        rt = a1 % ("retarget", "7f000240", sfx)
        self.assertEqual(self.status(self.trace(rt, orph), "PR-A1X"), "PASS")                                 # a retarget needs no slot-6 run
        strict = "t=2080010 PR A1X STRICT cr=7f0001a4 new=7f000240 d2=9.00 lim2=5.29 -> %s"
        r = self.trace(end, e2, f % (2083100, 3100), strict % "would-reject", strict % "ok")
        self.assertIn("STRICT would-reject 1/2", [x for x in r["checklist"] if x[0] == "PR-A1X"][0][2])

    def test_rah2_window(self):
        h2 = "H2 t=%d cr=7f0001a4 party=1 pc=0 stance=16 ranged=1 los=0 me=(1.00,2.00,0.00) lastBlocked=(1.00,2.00,0.00) -> retarget=%d"
        h1 = ("H1 t=%d cr=%s party=1 pc=0 stance=16 ranged=1 los=1 dist2=9.00 max=15.00 desired=10.00 inRange=%d area=0000aaaa/0000aaaa tgt=7f000233 ttype=5 "
              "backoff=0 rangedLocal=1 end52c=%s me=(1.00,2.00,0.00) tgtpos=(8.00,2.00,0.00) lastBlocked=(0.00,0.00,0.00) -> hold=0")
        def run(*rl):
            return self.status(self.trace(ranged=list(rl)), "PR-RAH2")
        self.assertEqual(run(h2 % (1000, 1), h1 % (3000, "7f0001a4", 0, "7f000240")), "FAIL")      # exactly 2000 ms later
        self.assertEqual(run(h2 % (1000, 1), h1 % (3001, "7f0001a4", 0, "7f000240")), "PASS")      # 2001 ms: outside
        self.assertEqual(run(h2 % (1000, 1), h1 % (2000, "7f0001a2", 0, "7f000240"), h1 % (2000, "7f0001a4", 0, "00000000"),
                             h1 % (2000, "7f0001a4", 1, "7f000240")), "PASS")                      # other creature / end52c 0 / inRange=1
        self.assertEqual(run(h2 % (1000, 0), h1 % (1500, "7f0001a4", 0, "7f000240")), "OBSERVE")    # retarget=0: the case did not occur
        self.assertEqual(run(h1 % (1500, "7f0001a4", 0, "7f000240")), "OBSERVE")
        self.assertEqual(self.status(self.trace(), "PR-RAH2"), "OBSERVE")                           # no ranged log


# ---- the regexes against the real printf format strings in the C++ sources --------------------------------------------
def c_literal_at(src: str, anchor: str) -> str:
    """The C string literal (adjacent literals concatenated) that starts with `anchor` (anchor = opening quote + start of text)."""
    i = src.index(anchor)
    out = []
    while True:
        assert src[i] == '"'
        j = i + 1
        buf = []
        while src[j] != '"':
            if src[j] == "\\":
                buf.append({"n": "\n", "t": "\t", "\\": "\\", '"': '"'}.get(src[j + 1], src[j + 1]))
                j += 2
            else:
                buf.append(src[j]); j += 1
        out.append("".join(buf))
        k = j + 1
        while src[k] in " \t\r\n":
            k += 1
        if src[k] == '"':
            i = k
            continue
        return "".join(out)


SPEC = __import__("re").compile(r"%[-+ #0]*\d*(?:\.\d+)?(?:ll|l|h|z)?[diouxXfeEgGcs%]")


def render_c(fmt: str, args: list) -> str:
    it = iter(args)

    def sub(m):
        spec = m.group(0)
        if spec == "%%":
            return "%"
        return spec.replace("ll", "l") % next(it)

    out = SPEC.sub(sub, fmt)
    assert next(it, None) is None, "unused args for " + fmt[:40]
    return out


class TraceShapes(Base):
    """companion-ai 0.2.1 (KpLog) trace shapes give the same verdicts and counts as the legacy raw ai_trace.txt.
    core = companion-ai_log.txt: a '# kplog session' header, then 'T=1000.000 f=- companion-ai/<ch> <old line>' per line.
    fallback = ai_trace.txt in the old-core shape: '1000 companion-ai/<ch> <old line>' per line, no header."""
    CORE_HDR = "# kplog session 2026-10-02 02:00:00 | kpatch.log v1 | patcher ABI 1"

    @staticmethod
    def channel(text: str) -> str:
        return "probe" if (" PR " in text or " PRS " in text or text.startswith("-- probe ")) else "main"

    def shaped_dir(self, rows, shape) -> Path:
        d = Path(tempfile.mkdtemp(dir=self.tmp)) / "case"
        d.mkdir()
        if shape == "core":
            (d / "companion-ai_log.txt").write_text("\n".join([self.CORE_HDR] + [f"T=1000.000 f=- companion-ai/{self.channel(s)} {s}" for s in rows]) + "\n")
        elif shape == "fallback":
            (d / "ai_trace.txt").write_text("\n".join(f"1000 companion-ai/{self.channel(s)} {s}" for s in rows) + "\n")
        else:
            (d / "ai_trace.txt").write_text("\n".join(rows) + "\n")
        return d

    def run_raw_core(self, raw_rows) -> dict:
        d = Path(tempfile.mkdtemp(dir=self.tmp)) / "case"
        d.mkdir()
        (d / "companion-ai_log.txt").write_text("\n".join([self.CORE_HDR] + raw_rows) + "\n")
        return apl.run_full([str(d), "--no-write"] + MINI)

    @staticmethod
    def verdicts(r) -> tuple:
        return ([(i, st) for i, st, _s, _e in r["checklist"]], sorted(s for s, _src, _m in r["an"].items), r["T"]["n_lines"], r["T"]["n_pa"],
                r["P"]["n_pr"], r["P"]["n_prs"], r["P"]["budget"], len(r["T"]["drops"]))

    def test_core_and_fallback_match_legacy(self):
        for case in ("probe_pass", "probe_fail"):
            rows = (FIX / case / "ai_trace.txt").read_text().splitlines()
            want = self.verdicts(apl.run_full([str(self.shaped_dir(rows, "legacy"))] + MINI))
            for shape, name in (("core", "companion-ai_log.txt"), ("fallback", "ai_trace.txt")):
                r = apl.run_full([str(self.shaped_dir(rows, shape))] + MINI)
                self.assertEqual(r["T"]["name"], name, (case, shape))
                self.assertEqual(self.verdicts(r), want, (case, shape))

    def test_core_name_wins_over_legacy_name(self):
        d = self.copy_case("probe_pass")
        shutil.copy(d / "ai_trace.txt", d / "companion-ai_log.txt")
        self.assertEqual(apl.find_files(d)["trace"].name, "companion-ai_log.txt")
        (d / "companion-ai_log.txt").unlink()
        self.assertEqual(apl.find_files(d)["trace"].name, "ai_trace.txt")

    def test_split_long_lines_are_rejoined(self):
        # the core cuts a message at 232 bytes: companion-ai 0.2.1 splits at spaces into `t=<ms> + <rest>` pieces (`++` = cut mid-word)
        whole = "t=5000 S lines=5 " + " ".join(f"grp{i}{{a={i} b=2}}" for i in range(30)) + " tail" + "x" * 30
        cut = whole.index("grp10")
        mid = len(whole) - 20
        pieces = [whole[:cut - 1], "t=5000 + " + whole[cut:mid], "t=5000 ++ " + whole[mid:]]
        for shape in ("core", "fallback"):
            rows = [f"T=1000.000 f=- companion-ai/main {p}" if shape == "core" else f"1000 companion-ai/main {p}" for p in pieces]
            d = Path(tempfile.mkdtemp(dir=self.tmp)) / "case"
            d.mkdir()
            (d / ("companion-ai_log.txt" if shape == "core" else "ai_trace.txt")).write_text("\n".join(([self.CORE_HDR] if shape == "core" else []) + rows) + "\n")
            got = [m for _ln, k, m in apl.read_trace_lines(next(d.iterdir())) if k == "line"]
            self.assertEqual(got, [whole], shape)

    def test_kplog_dropped_is_a_drop_and_mark_is_not(self):
        hdr = [f"T=1000.000 f=- companion-ai/main {s}" for s in ProbeChecks.HDR.splitlines()]
        end = [f"T=1000.000 f=- companion-ai/main {s}" for s in ProbeChecks.END.splitlines()]
        ss = "T=1000.000 f=- companion-ai/probe t=2040000 PR SS1 cr=7f0001a2 p_atton state 0->5 combat=1"
        r = self.run_raw_core(hdr + [ss, "T=1000.000 f=- companion-ai/kplog DROPPED 40 (ring 0, rate 200)"] + end)
        self.assertEqual(self.status(r, "PR-SS"), "OBSERVE")
        self.assertEqual([d[0] for d in r["T"]["drops"]], [40])
        r = self.run_raw_core(hdr + [ss, "T=1000.000 f=- MARK before the cutscene"] + end)
        self.assertEqual(self.status(r, "PR-SS"), "FAIL")
        self.assertEqual(r["T"]["drops"], [])


class FormatsMatchSource(unittest.TestCase):
    """Render the printf format strings found in the C++ sources and check that the tool's regexes parse them."""
    MODS = apl.MODS_DIR

    @classmethod
    def setUpClass(cls):
        cls.cai = (cls.MODS / "companion_ai_src" / "CompanionAI.cpp").read_text()
        cls.rai = (cls.MODS / "ranged_ai_src" / "RangedAI.cpp").read_text()
        cls.isf = (cls.MODS / "inventory_sort_filter_src" / "InventorySortFilter.cpp").read_text()

    def chk(self, src, anchor, args, rx, *, groups=None):
        line = render_c(c_literal_at(src, anchor), args)
        m = rx.match(line)
        self.assertIsNotNone(m, f"regex {rx.pattern[:50]!r} does not match source format -> {line!r}")
        if groups:
            self.assertEqual(m.groups()[:len(groups)], tuple(groups), line)
        return line

    def test_companion_ai_lines(self):
        c, A = self.cai, apl
        self.chk(c, '"t=%lu P %08x %s %s ev=%d d=%d | %s"', [100, 0x7f0001a2, "p_atton", "k_ai_master", 1003, 1, "hello world"], A.PA_RE,
                 groups=["100", "P", "7f0001a2", "p_atton", "k_ai_master", "1003", "1", "hello world"])
        self.chk(c, '"t=%lu A %08x %s %s ev=%d d=%d | %s @(%d,%d) life=%.1f"', [100, 0x7f0001a2, "-", "k_ai_master", 1012, 1, "txt", 0, 15, 2.0],
                 A.PA_RE, groups=["100", "A", "7f0001a2", "-", "k_ai_master", "1012", "1"])
        self.chk(c, '"t=%lu A1 cr=%08x party=%d raw=%d eff=%d reason=%s dist=%.1f max=%.1f los=%d ranged=%d area=%s tgt=%08x -> %s new=%08x (wouldSpin=%u)%s"',
                 [100, 0x7f0001a4, 1, 11, 11, "range", 40.0, 2.2, 1, 1, "same", 0x7f000233, "end", 0xffffffff, 3, ""], A.A1_RE,
                 groups=["100", "7f0001a4", "1", "11", "11", "range", "end", "ffffffff", "3"])
        self.chk(c, '"t=%lu A1 cr=%08x party=%d raw=%d eff=%d reason=%s dist=%.1f max=%.1f los=%d ranged=%d area=%s tgt=%08x -> %s new=%08x (wouldSpin=%u)%s"',
                 [100, 0x7f0001a4, 1, 11, 11, "range", 40.0, 2.2, 1, 1, "same", 0x7f000233, "spin (not listed)", 0xffffffff, 3, " c524=0 f570=2900 combat=1"], A.A1_RE,
                 groups=["100", "7f0001a4", "1", "11", "11", "range", "spin (not listed)"])
        self.chk(c, '"t=%lu E2 FALLBACK cr=%08x %s party=%d combat=%d round=%d script=%s a1spin=%d lastA1=%ldms"',
                 [100, 0x7f0001a4, "p_visas", 1, 1, 0, "k_hen_visasdlg", 0, 2900], A.E2_RE, groups=["100", "7f0001a4", "p_visas"])
        self.chk(c, '"t=%lu A1 summary: cr=%08x would-spin passes=%u/%ds ends=%u retargets=%u strict=%u/%u"', [100, 0x7f0001a4, 5, 10, 2, 3, 7, 2], A.A1SUM_RE,
                 groups=["100", "7f0001a4", "5", "10", "2", "3", "7", "2"])
        self.chk(c, '"t=%lu E1 SPIN cr=%08x %s party=%d stance=%d entries/s=%u', [100, 0x7f0001a2, "p_atton", 1, 11, 900, 31, 0, 1, 2991, 0x7f000233,
                 3, 11, 0, 0x7f0001a0, 0, 31.4, 2.25, 1], A.E1SPIN_RE, groups=["100", "7f0001a2", "p_atton", "1", "11"])
        self.chk(c, '"t=%lu E1 END cr=%08x %s spin lasted %.1f s"', [100, 0x7f0001a2, "p_atton", 18.0], A.E1END_RE)
        for how in ("keep", "clear"):
            why = "keep" if how == "keep" else "off"   # the " (fm1=0)" suffix pairs with why=off
            self.chk(c, '"t=%lu FM1 bumpee=%08x %s party=%d mover=%08x %s combat=%d tgt=%08x round=%d queue=[%s]%s -> %s why=%s%s"',
                     [100, 0x7f0001a2, "p_atton", 1, 0x7f0001a0, "p_exile", 0, 0x7f000000, 0, "3d", "", how, why, " (fm1=0)"], A.FM1_RE,
                     groups=["100", "7f0001a2", "p_atton", "1", how])
        self.chk(c, '"t=%lu FM4 cr=%08x %s tid=%lu fi=%08x live:', [100, 0x7f0001a2, "p_atton", 111, 0xa1b2c3d, 0x7f0001a0, 3.2, 0, 0x7f0001a0, 3.2, 0,
                 "restored (mode 1)"], A.FM4_RE, groups=["100", "7f0001a2", "p_atton", "111", "0a1b2c3d"])
        self.chk(c, '"# T1 first ShipBuild flip: tid=%lu (load tid=%lu) script=%s"', [111, 100, "k_ai_master"], A.T1_RE, groups=["111", "100", "k_ai_master"])
        self.chk(c, '"t=%lu E3 P1 calls/s=%.0f creatures=%d%s maxPerCreature/s=%.1f elapsed>=4000:%d%% (p1=%d interval=%d)"',
                 [100, 1180.0, 19, "+", 62.0, 100, 0, 4000], A.E3_RE, groups=["100", "1180", "19", "+", "62.0", "100", "0", "4000"])
        s_fmt = c_literal_at(c, '"t=%lu S lines=')
        n_specs = len([m for m in SPEC.finditer(s_fmt) if m.group(0) != "%%"])
        s_line = render_c(s_fmt, [100] + list(range(1, n_specs)))
        self.assertTrue(A.S_RE.match(s_line), s_line)
        d = {k: int(v) for k, v in __import__("re").findall(r"(\w+)=(\d+)", s_line)}
        self.assertIn("lines", d)
        self.assertIn("fm4{snap=", s_line)
        self.assertIn("restore=", s_line)
        self.chk(c, '"-- ai_trace: file cap %d MB reached at t=%lu;', [64, 5], A.CAP_RE, groups=["64", "5"])
        self.chk(c, '"-- ai_trace: %u lines dropped by the per-second cap (%d/s) at t=%lu --', [214, 200, 5], A.DROP_RE, groups=["214", "200", "5"])
        self.chk(c, '"t=%lu R previous line repeated %ux', [100, 5], A.REPEAT_RE, groups=["100", "5"])
        self.chk(c, '"# companion-ai %s session %s t=%lu pid=%lu tid=%lu cwd=%s ini=%s"', ["0.1.0", "2026-10-02 01:00:00", 1000000, 1234, 100, "Z:\\g", "Z:\\g\\i.ini"],
                 A.HEADER_RE, groups=["0.1.0", "2026-10-02 01:00:00", "1000000", "1234", "100"])
        self.chk(c, '"# config: unparseable value for: %s (flags', ["a1_stances,p1"], A.CONFIG_BAD_RE, groups=["a1_stances,p1"])

    def test_probe_lines(self):
        """Every Phase-4 probe regex against the printf literal of CompanionAI.cpp (the fixtures are hand-written: this ties them to the source)."""
        c, A = self.cai, apl
        self.chk(c, '"t=%lu PR FM2C cr=%08x tgt=%08x res=%d code=%d cached=%d cp=%d calls=%d n2=%u n3=%u"',
                 [100, 0x7f0001a2, 0x7f000233, 2, -4, -1, 0, 3, 1, 0], A.PR_FM2C_RE, groups=["100", "7f0001a2", "7f000233", "2", "-4", "-1", "0", "3", "1", "0"])
        self.chk(c, '"t=%lu PR FM2F cr=%08x tgt=%08x grp=%u n=%u span=%lums ctrl=%d"', [100, 0x7f0001a2, 0x7f000233, 2, 3, 2100, 1], A.PR_FM2F_RE,
                 groups=["100", "7f0001a2", "7f000233", "2", "3", "2100", "1"])
        self.chk(c, '"t=%lu PR FM2R cr=%08x tgt=%08x other=%d inr=%d range=%.2f d=%.2f prog=%.2f n=%u"', [100, 0x7f0001a2, 0x7f000233, 0, 1, 2.5, 12.3, -0.1, 4],
                 A.PR_FM2R_RE, groups=["100", "7f0001a2", "7f000233", "0", "1", "2.50", "12.30", "-0.10", "4"])
        self.chk(c, '"t=%lu PR FM3L level=%d sentinel=%08x now=%d removed=%d done=%d elapsed=%llu slice=%llu"', [100, 2, 0xa1b2c3d, 0, 0, 1, 1800, 1500],
                 A.PR_FM3L_RE, groups=["100", "2", "0a1b2c3d", "0", "0", "1", "1800", "1500"])
        self.chk(c, '"t=%lu PR FM3X id=%08x level=%d idx=%d cursor=%d sentinel=%d"', [100, 0x7f0001a2, 2, 5, 6, 1], A.PR_FM3X_RE,
                 groups=["100", "7f0001a2", "2", "5", "6", "1"])
        self.chk(c, '"t=%lu PRS FM3 passes=%u lost=%u removes=%u beforeCursor=%u"', [100, 9, 3, 4, 1], A.PRS_FM3_RE, groups=["100", "9", "3", "4", "1"])
        rs = '"t=%lu PR RS site=%s cr=%08x %s party=%d empty=%d%s"'
        self.chk(c, rs, [100, "S6A", 0x7f0001a2, "p_atton", 1, 0, ""], A.PR_RS_RE, groups=["100", "S6A", "7f0001a2", "p_atton", "1", "0", None, None, None])
        sfx = c_literal_at(c, '" spk=%08x spkParty=%d pat=%d spkTag=%s"')
        self.chk(c, rs, [100, "S7", 0x7f0001a2, "p_atton", 1, 1, render_c(sfx, [0x7f000300, 0, 15, "n_mandsold"])], A.PR_RS_RE,
                 groups=["100", "S7", "7f0001a2", "p_atton", "1", "1", "7f000300", "0", "15", "n_mandsold"])
        self.chk(c, '"t=%lu PRS E9 deliveries=%u"', [100, 4], A.PRS_E9_RE, groups=["100", "4"])
        self.chk(c, '"t=%lu PRS RS cr=%08x s6a=%u s6b=%u s6c=%u s7=%u s12=%u s1=%u max12ps=%u"', [100, 0x7f0001a2, 1, 2, 3, 4, 5, 6, 7], A.PRS_RS_RE,
                 groups=["100", "7f0001a2", "1", "2", "3", "4", "5", "6", "7"])
        self.chk(c, '"t=%lu PR P2 cr=%08x ctrl=%d elapsed=%u combat=%d"', [100, 0x7f0001a2, 1, 4012, 0], A.PR_P2_RE, groups=["100", "7f0001a2", "1", "4012", "0"])
        self.chk(c, '"t=%lu PRS P2 cr=%08x runs=%u avg=%u max=%u ctrlRuns=%u"', [100, 0x7f0001a2, 10, 4010, 4200, 2], A.PRS_P2_RE,
                 groups=["100", "7f0001a2", "10", "4010", "4200", "2"])
        self.chk(c, '"t=%lu PR PF cr=%08x %s old=%d new=%d apply=%d site=%s"', [100, 0x7f0001a2, "p_atton", 1, 0, 1, "SAVE_A"], A.PR_PF_RE,
                 groups=["100", "7f0001a2", "p_atton", "1", "0", "1", "SAVE_A"])
        self.chk(c, '"t=%lu PRS PF saveA=%u saveB=%u xferA=%u xferB=%u other=%u"', [100, 3, 3, 1, 1, 0], A.PRS_PF_RE, groups=["100", "3", "3", "1", "1", "0"])
        self.chk(c, '"t=%lu PRS RAR cr=%08x ispc=%d n=%u %s"', [100, 0x7f0001a2, 0, 500, "c:5/0/490/5 0:1/0/2/0+o3"], A.PRS_RAR_RE,
                 groups=["100", "7f0001a2", "0", "500", "c:5/0/490/5 0:1/0/2/0+o3"])
        self.chk(c, '"t=%lu PR RAR MOVESTATE cr=%08x type=%x res=%d state=%d"', [100, 0x7f0001a2, 0x11, 2, 5], A.PR_RAR_MOVE_RE, groups=["100", "7f0001a2", "11", "2", "5"])
        self.chk(c, '"t=%lu PR RAR REQ cr=%08x type=%x n=%u"', [100, 0x7f0001a2, 0x3f, 200], A.PR_RAR_REQ_RE, groups=["100", "7f0001a2", "3f", "200"])
        self.chk(c, '"t=%lu PR SHX %s spk=%08x type=%d %s party=%d r=%.0f seen=%d idx=%d count=%d last=%08x text=%s"',
                 [100, "ABORT-MID", 0x7f000300, 5, "n_mandsold", 0, 35.0, 0, 2, 3, 0x7f000304, "help me"], A.PR_SHX_RE,
                 groups=["100", "ABORT-MID", "7f000300", "5", "n_mandsold", "0", "35", "0", "2", "3", "7f000304", "help me"])
        self.chk(c, '"t=%lu PRS SHX shouts=%u r35=%u partySpk=%u end=%u xbreak=%u noarea=%u abortStartCr=%u abortStartOther=%u abortMid=%u"',
                 [100, 12, 4, 1, 10, 2, 0, 1, 2, 3], A.PRS_SHX_RE, groups=["100", "12", "4", "1", "10", "2", "0", "1", "2", "3"])
        self.chk(c, '"t=%lu PR SS1 cr=%08x %s state %d->%d combat=%d"', [100, 0x7f0001a2, "p_atton", 0, 5, 1], A.PR_SS1_RE, groups=["100", "7f0001a2", "p_atton", "0", "5", "1"])
        self.chk(c, '"t=%lu PR GCA cr=%08x res=%d lastRun=%x moving=%d"', [100, 0x7f0001a2, 3, 0x11, 1], A.PR_GCA_RE, groups=["100", "7f0001a2", "3", "11", "1"])
        self.chk(c, '"t=%lu PRS GCA cr=%08x %s"', [100, 0x7f0001a2, "r3:12 r38:40"], A.PRS_GCA_RE, groups=["100", "7f0001a2", "r3:12 r38:40"])
        self.chk(c, '"t=%lu PR PARTY why=%s ctrl=%08x f=%d/%d pobj=%08x f=%d/%d pc=%08x tbl=%d [%s]"',
                 [100, "load", 0x7f0001a0, 1, 1, 0x7f0001a0, 1, 1, 0x7f0001a0, 3, "0:7f0001a0:1:1,1:7f0001a2:1:0"], A.PR_PARTY_RE,
                 groups=["100", "load", "7f0001a0", "1", "1", "7f0001a0", "1", "1", "7f0001a0", "3", "0:7f0001a0:1:1,1:7f0001a2:1:0"])
        self.chk(c, '"t=%lu PR A1X STRICT cr=%08x new=%08x d2=%.2f lim2=%.2f -> %s"', [100, 0x7f0001a4, 0x7f000240, 9.0, 5.29, "would-reject"], A.PR_A1X_STRICT_RE,
                 groups=["100", "7f0001a4", "7f000240", "9.00", "5.29", "would-reject"])
        self.chk(c, '"t=%lu PR A1X FOLLOWUP cr=%08x site=%s after=%ldms"', [100, 0x7f0001a4, "S6B", 3100], A.PR_A1X_FOLLOW_RE, groups=["100", "7f0001a4", "S6B", "3100"])
        self.chk(c, '"t=%lu PR A1X ORPHAN cr=%08x since=%ldms"', [100, 0x7f0001a4, 4100], A.PR_A1X_ORPHAN_RE, groups=["100", "7f0001a4", "4100"])
        a1 = render_c(c_literal_at(c, '"t=%lu A1 cr=%08x party=%d raw=%d eff=%d reason=%s dist=%.1f'),
                      [100, 0x7f0001a4, 1, 11, 11, "range", 40.0, 2.2, 1, 1, "same", 0x7f000233, "end", 0xffffffff, 3,
                       render_c(c_literal_at(c, '" c524=%d f570=%d combat=%d"'), [0, 3000, 1])])
        self.assertTrue(A.A1_RE.match(a1), a1)
        self.assertEqual(A.A1X_SUFFIX_RE.search(a1).groups(), ("0", "3000", "1"))
        m = A.BUDGET_RE.match(render_c(c_literal_at(c, '"-- probe %s line budget reached --"'), ["RAR"]))
        self.assertEqual(m.group(1), "RAR")
        probes = render_c(c_literal_at(c, '"# config probes fm2c=%d'), [1] * 14)
        self.assertTrue(A.CONFIG_RE.match(probes))
        self.assertEqual(len(apl.kv(probes)), 14)
        # PR lines are never script lines (the P/A regex needs " P " / " A " as the second token)
        self.assertIsNone(A.PA_RE.match("t=100 PR RS site=S6A cr=7f0001a2 p_atton party=1 empty=0"))

    def test_every_probe_line_format_has_a_parser(self):
        import re
        kinds = set(re.findall(r'"t=%lu (PRS? \w+(?: (?:MOVESTATE|REQ|STRICT|FOLLOWUP|ORPHAN))?)', self.cai))
        self.assertEqual(len(kinds), 25, sorted(kinds))
        self.assertEqual(len(apl._PR_PARSERS), len(kinds))

    def test_config_fixes_line_carries_the_keys_the_tool_reads(self):
        fmt = c_literal_at(self.cai, '"# config fixes a1=%d')
        n = len([m for m in SPEC.finditer(fmt) if m.group(0) != "%%"])
        line = render_c(fmt, [1] * n)
        kv = apl.kv(line)
        for key in ("a1", "a1_stances", "a1_party_only", "fm4"):
            self.assertIn(key, kv)
        self.assertTrue(apl.CONFIG_RE.match(line))

    def test_ranged_ai_lines(self):
        r, A = self.rai, apl
        h1 = c_literal_at(r, '"H1 t=%lu cr=%08x party=%d pc=%d stance=%d')
        n = len([m for m in SPEC.finditer(h1) if m.group(0) != "%%"])
        args = [100, 0x7f0001a2, 1, 0, 11, 1, 0, 36.0, 15.0, 10.0, 1, 0xaaaa, 0xaaaa, 0x7f000233, 5, 0, 1, 0, 1.0, 2.0, 0.0, 8.0, 2.0, 0.0, 1.0, 2.0, 0.0, 1]
        self.assertEqual(n, len(args))
        m = A.R_H1_RE.match(render_c(h1, args))
        self.assertIsNotNone(m)
        self.assertEqual((m.group(1), m.group(5), m.group(7), m.group(8), m.group(15)), ("100", "11", "0", "1", "1"))
        self.assertEqual(m.group(9, 10, 11), m.group(12, 13, 14))
        self.chk(r, '"H2 t=%lu cr=%08x party=%d pc=%d stance=%d ranged=%d los=%d me=(%.2f,%.2f,%.2f) lastBlocked=(%.2f,%.2f,%.2f) -> retarget=%d"',
                 [100, 0x7f0001a4, 1, 0, 16, 1, 0, 1.0, 2.0, 0.0, 1.0, 2.0, 0.0, 1], A.R_H2_RE, groups=["100", "7f0001a4", "1", "0", "16", "1", "0"])
        self.chk(r, '"H3 t=%lu cr=%08x stance=16 ranged=%d inRange=%d kite_approach=%d -> %d%s"', [100, 0x7f0001a4, 0, 1, 1, 16, " (approach)"], A.R_H3_RE,
                 groups=["100", "7f0001a4", "0", "1", "1", "16"])
        self.chk(r, '"S1 t=%lu stance menu build: stance %d kept (vanilla clamp skipped)"', [100, 16], A.R_S_RE, groups=["S1", "100"])
        self.chk(r, '"S2 t=%lu stance row %d (%08x): id=0x%x icon=%s"', [100, 4, 0x13fc0c4c, 0x415, "ib_ranged"], A.R_S_RE, groups=["S2", "100"])
        self.chk(r, '"S3 t=%lu stance click: id=0x%x stance %d -> %d msg strref=%d"', [100, 0x415, 11, 16, 136539], A.R_S3_RE, groups=["100", "415", "11", "16", "136539"])
        self.chk(r, '"S4 t=%lu HUD stance marker -> 0x%x"', [100, 0x415], A.R_S_RE, groups=["S4", "100"])
        self.assertTrue(A.R_CFG_RE.match(c_literal_at(r, '"config: hold=%d')))
        loaded = render_c(c_literal_at(r, '"ranged-ai 0.4.0 loaded'), ["Z:\\\\g", 1234, 99])
        self.assertTrue(A.R_LOADED_RE.match(loaded), loaded)

    def test_inventory_lines(self):
        i, A = self.isf, apl
        self.chk(i, '"populate: panel=%08x mode=%d extra=%d creature=%08x ebp=%08x rows-so-far=%d"', [0xa000001, 1, 0, 0x7f0001a2, 0x19f000, 0], A.INV_POP_RE,
                 groups=["0a000001", "1", "0", "7f0001a2"])
        self.chk(i, '"flush: panel=%08x creature=%08x rows=%d seen=%d dropped=%d direct=%d usable=%d tint-candidates=%d sort=%d tint=%d extra=%d selected=%d counter=%d"',
                 [0xa000001, 0x7f0001a2, 40, 42, 2, 0, 25, 15, 1, 1, 0, 0, 40], A.INV_FLUSH_RE,
                 groups=["0a000001", "7f0001a2", "40", "42", "2", "0", "25", "15", "1", "1", "0", "0", "40"])
        self.chk(i, '"flush: no backpack rows this populate (mode=%d extra=%d)"', [0, 0], A.INV_FLUSH_EMPTY_RE, groups=["0", "0"])
        self.chk(i, '"tab click: cycle -> state %d (engine mode %d, extra=%d)"', [2, 0, 1], A.INV_CYCLE_RE, groups=["2", "0", "1"])
        self.assertTrue(A.INV_CFG_RE.match(c_literal_at(i, '"config: sort=%d')))
        for anchor, args in (('"buffer: bad frame panel=%08x', [0, 0]), ('"row: counter %d -> %d (expected +1) for item %08x', [1, 3, 5])):
            line = render_c(c_literal_at(i, anchor), args)
            self.assertTrue(any(rx.search(line) for _k, rx in A.INV_ERR_RES), line)

    def test_script_marker_texts_exist_in_the_script_sources(self):
        """Every marker regex must be satisfiable by a PrintString text in party_ai_fixes.nss / kite_routine.nss / party_ai_patches.py."""
        import re
        base = self.MODS / "ranged_ai_src" / "script"
        text = "\n".join((base / f).read_text() for f in ("party_ai_fixes.nss", "kite_routine.nss", "party_ai_patches.py"))
        for key, rx, _src in apl.MARKERS:
            prefix = re.split(r"\(|\\d", rx)[0]            # the static text before any group: the rest is built at run time
            self.assertIn(prefix, text, f"marker {key!r} ({rx}) not found in the script sources")
        self.assertIn("combat exit", text)


class Cli(Base):
    def test_cli_writes_report_into_the_given_dir(self):
        d = self.copy_case("healthy")
        p = subprocess.run([sys.executable, str(HERE / "analyze_play_logs.py"), str(d)] + MINI, capture_output=True, text=True)
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertTrue((d / "REPORT.md").is_file())
        self.assertIn("Checklist: 17 PASS", p.stdout)   # 16 + PR-RAH2 (the fixture H2 retarget has no end52c follow-up)

    def test_strict_exit_code(self):
        d = self.copy_case("recast")
        p = subprocess.run([sys.executable, str(HERE / "analyze_play_logs.py"), str(d), "--strict"] + MINI, capture_output=True, text=True)
        self.assertEqual(p.returncode, 2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
