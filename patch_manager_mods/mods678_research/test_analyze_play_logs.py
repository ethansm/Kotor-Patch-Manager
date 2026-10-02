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


class RealHookTomls(Base):
    def test_default_tomls_match_the_task_spec(self):
        hooks, notes = apl.load_hooks(None)
        self.assertEqual(notes, [])
        by = {}
        for h in hooks:
            by.setdefault(h["patch"], []).append(h)
        self.assertEqual(len(by["companion-ai"]), 18)
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
        self.assertTrue(self.fails(r2, "companion-ai: 1/18"))

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
        self.chk(c, '"t=%lu A1 cr=%08x party=%d raw=%d eff=%d reason=%s dist=%.1f max=%.1f los=%d ranged=%d area=%s tgt=%08x -> %s new=%08x (wouldSpin=%u)"',
                 [100, 0x7f0001a4, 1, 11, 11, "range", 40.0, 2.2, 1, 1, "same", 0x7f000233, "end", 0xffffffff, 3], A.A1_RE,
                 groups=["100", "7f0001a4", "1", "11", "11", "range", "end", "ffffffff", "3"])
        self.chk(c, '"t=%lu A1 cr=%08x party=%d raw=%d eff=%d reason=%s dist=%.1f max=%.1f los=%d ranged=%d area=%s tgt=%08x -> %s new=%08x (wouldSpin=%u)"',
                 [100, 0x7f0001a4, 1, 11, 11, "range", 40.0, 2.2, 1, 1, "same", 0x7f000233, "spin (not listed)", 0xffffffff, 3], A.A1_RE,
                 groups=["100", "7f0001a4", "1", "11", "11", "range", "spin (not listed)"])
        self.chk(c, '"t=%lu E2 FALLBACK cr=%08x %s party=%d combat=%d round=%d script=%s a1spin=%d lastA1=%ldms"',
                 [100, 0x7f0001a4, "p_visas", 1, 1, 0, "k_hen_visasdlg", 0, 2900], A.E2_RE, groups=["100", "7f0001a4", "p_visas"])
        self.chk(c, '"t=%lu A1 summary: cr=%08x would-spin passes=%u/%ds ends=%u retargets=%u"', [100, 0x7f0001a4, 5, 10, 2, 3], A.A1SUM_RE,
                 groups=["100", "7f0001a4", "5", "10", "2", "3"])
        self.chk(c, '"t=%lu E1 SPIN cr=%08x %s party=%d stance=%d entries/s=%u', [100, 0x7f0001a2, "p_atton", 1, 11, 900, 31, 0, 1, 2991, 0x7f000233,
                 3, 11, 0, 0x7f0001a0, 0, 31.4, 2.25, 1], A.E1SPIN_RE, groups=["100", "7f0001a2", "p_atton", "1", "11"])
        self.chk(c, '"t=%lu E1 END cr=%08x %s spin lasted %.1f s"', [100, 0x7f0001a2, "p_atton", 18.0], A.E1END_RE)
        for how in ("keep", "clear"):
            self.chk(c, '"t=%lu FM1 bumpee=%08x %s party=%d mover=%08x %s combat=%d tgt=%08x round=%d queue=[%s]%s -> %s%s"',
                     [100, 0x7f0001a2, "p_atton", 1, 0x7f0001a0, "p_exile", 0, 0x7f000000, 0, "3d", "", how, " (fm1=0)"], A.FM1_RE,
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
        loaded = render_c(c_literal_at(r, '"ranged-ai 0.3.0 loaded'), ["Z:\\\\g", 1234])
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
        self.assertIn("Checklist: 16 PASS", p.stdout)

    def test_strict_exit_code(self):
        d = self.copy_case("recast")
        p = subprocess.run([sys.executable, str(HERE / "analyze_play_logs.py"), str(d), "--strict"] + MINI, capture_output=True, text=True)
        self.assertEqual(p.returncode, 2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
