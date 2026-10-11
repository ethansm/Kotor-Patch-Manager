#!/usr/bin/env python3
"""Tests for scripts/npc_div_log_check.py. Synthetic logs only: golden lines plus synthetic lines in the same grammar.
Run: python3 scripts/test_npc_div_log_check.py   (run_tests.py discovers it as py:scripts/test_npc_div_log_check.py)"""
import contextlib
import io
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import npc_div_log_check as N  # noqa: E402

# GOLDEN: mirrored from diversity_log_test.cpp
GOLD_BANNER0 = "BANNER v=1.0.0 kpl=1 sha=6A522E71 base=00400000 ts=5f3a1b2c"
GOLD_BANNER_HOOK = "BANNER hook=tex addr=0085e601 pre=ok"
GOLD_BANNER_ROWS = "BANNER rows app=802-811 n=10 head=192-196 n=5 appcount=816 headcount=249"
GOLD_TEX = ("TEX pd=0a1b2c3d fr=00001234 ret=00abcdef app=805 g=1 tv=1 dm=0 hd=0 ph=2 cur=n_commm01 "
            "h=06c232b2 st=1 slot=09 new=1 sw=1")
GOLD_RMP = ("RMP this=12345678 pd=0a1b2c3d oid=000003e9 tp=00000010 tl=6 tag=Bastila xy=12.5,-3.5 "
            "xyb=41480000,c0600000 app=805>806 h=06c232b2 why=ok")
GOLD_CEN_TEX = "CEN k=tex app=805 g=1 tv=1 dm=0 cur=n_commm01"
GOLD_CEN_REMAP = "CEN k=remap app=805 g=1 tv=1 tag=Bastila"
GOLD_STAT_TEX = ("STAT tex calls=10 nullg=0 unread=1 bcskip=2 nonc=3 comm=4 swap=5 frd=6 retd=7 skiprow=8 "
                 "stashhit=9 stashmiss=10")
GOLD_STAT_REMAP = "STAT remap calls=11 nullg=0 unread=1 bcskip=2 excl=3 tagun=4 chg=5 nochg=6 oidd=7 skiprow=8 skipg=9"
GOLD_STAT_ABI = "STAT abi app=1 g=2 dm=3"
GOLD_STAT_XY = "STAT xy x=-3.5..12.5 y=0.0..100.5"
GOLD_STAT_SLOT0 = "STAT slot g=0 00:3 01:0 02:0 03:0 04:0 05:7 06:0 07:0 08:0 09:0 10:0 11:0"
GOLD_STAT_APP = "STAT app k=tex 802:4 1000:9"
GOLD_STAT_READ = "STAT read 1:1 2:2 3:3 4:4 5:5 6:6 7:7 8:8 9:9 10:10"
GOLD_STAT_CENSUS = "STAT census keys=17 ovf=0"
GOLD_DSC0 = "DSC0 pd=0a1b2c3d hex=000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
GOLD_CRE0 = ("CRE0 this=12345678 hex="
             "000102030405060708090a0b0c0d0e0f"
             "101112131415161718191a1b1c1d1e1f"
             "202122232425262728292a2b2c2d2e2f"
             "303132333435363738393a3b3c3d3e3f")
GOLD_HOOKBYTES = "HOOKBYTES addr=0085e601 want=8b442404 got=e9000000"
GOLD_ABI = "ABI app=820 g=5 dm=2 tv=3 pd=0a1b2c3d where=remap"
GOLD_READ = "READ site=3 addr=0076d200 len=16 caller=0085e601 n=1"
GOLD_DUMP = "DUMP trigger=file path=npc-diversity.dump"
GOLDEN = [GOLD_BANNER0, GOLD_BANNER_HOOK, GOLD_BANNER_ROWS, GOLD_TEX, GOLD_RMP, GOLD_CEN_TEX, GOLD_CEN_REMAP,
          GOLD_STAT_TEX, GOLD_STAT_REMAP, GOLD_STAT_ABI, GOLD_STAT_XY, GOLD_STAT_SLOT0, GOLD_STAT_APP,
          GOLD_STAT_READ, GOLD_STAT_CENSUS, GOLD_DSC0, GOLD_CRE0, GOLD_HOOKBYTES, GOLD_ABI, GOLD_READ, GOLD_DUMP]
# Golden line -> the kind the checker gives it (None = not a checked kind, ignored).
GOLD_KIND = {GOLD_BANNER0: "BANNER:v", GOLD_BANNER_HOOK: "BANNER:hook", GOLD_BANNER_ROWS: None, GOLD_TEX: "TEX",
             GOLD_RMP: "RMP", GOLD_CEN_TEX: "CEN", GOLD_CEN_REMAP: "CEN", GOLD_STAT_TEX: "STAT:tex",
             GOLD_STAT_REMAP: "STAT:remap", GOLD_STAT_ABI: "STAT:abi", GOLD_STAT_XY: "STAT:xy",
             GOLD_STAT_SLOT0: "STAT:slot", GOLD_STAT_APP: "STAT:app", GOLD_STAT_READ: "STAT:read",
             GOLD_STAT_CENSUS: "STAT:census", GOLD_DSC0: "DSC0", GOLD_CRE0: "CRE0", GOLD_HOOKBYTES: "HOOKBYTES",
             GOLD_ABI: "ABI", GOLD_READ: "READ", GOLD_DUMP: "DUMP"}


def stamped(body, n=0, level=""):
    """KpLog core shape: T=<ms>.<3-digit fraction> f=- patch/channel [LEVEL ]text (kplog_parse.RE_LINE)."""
    return "T=%d.000 f=- npc-diversity/main %s%s" % (1000 + n, level + " " if level else "", body)


def tex_line(app=805, g=1, tv=1, fr=0x1234, ret=0xABCDEF, slot=9, sw=1, st=1, cur="n_commm01"):
    return ("TEX pd=0a1b2c3d fr=%08x ret=%08x app=%d g=%d tv=%d dm=0 hd=0 ph=2 cur=%s h=06c232b2 st=%d slot=%02d new=1 sw=%d"
            % (fr, ret, app, g, tv, cur, st, slot, sw))


def rmp_line(oid=0x3E9, xy=(12.5, -3.5), before=805, after=806, tag="Bastila", why="ok"):
    return ("RMP this=12345678 pd=0a1b2c3d oid=%08x tp=00000010 tl=6 tag=%s xy=%.1f,%.1f xyb=41480000,c0600000 "
            "app=%d>%d h=06c232b2 why=%s" % (oid, tag, xy[0], xy[1], before, after, why))


def banner_v(v="1.0.0"):
    return "BANNER v=%s kpl=1 sha=6A522E71 base=00400000 ts=5f3a1b2c" % v


def hook(name="tex", pre="ok"):
    return "BANNER hook=%s addr=0085e601 pre=%s" % (name, pre)


def healthy(remap_pre="ok", tag="Bastila"):
    """A clean launch: banner, both hooks, 6 TEX swaps over 3 slots, 6 RMP lines with distinct oid/xy, golden STAT block."""
    body = [GOLD_BANNER0, hook("tex"), hook("remap", remap_pre), GOLD_BANNER_ROWS]
    body += [tex_line(805, 1, 1, fr=0x1000 + i, ret=0xABC0 + (i % 2), slot=9 + (i % 3)) for i in range(6)]
    body += [rmp_line(oid=0x3E9 + i, xy=(10.0 + i, -2.0 - i), before=805, after=806 if i % 2 == 0 else 805, tag=tag)
             for i in range(6)]
    body += [GOLD_STAT_TEX, GOLD_STAT_REMAP, "STAT app k=tex 805:6"]
    return body


def run_lines(raw_lines, extra=()):
    """Write raw lines as npc-diversity_log.txt in a temp dir and run the checker (state file temp too)."""
    with tempfile.TemporaryDirectory() as d:
        log = os.path.join(d, "npc-diversity_log.txt")
        Path(log).write_text("\n".join(raw_lines) + ("\n" if raw_lines else ""))
        return run([log, "--state", os.path.join(d, "bm.json"), *extra])


def run_bodies(bodies, extra=()):
    return run_lines([stamped(b, i) for i, b in enumerate(bodies)], extra)


def run(argv):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = N.main(argv)
    return rc, buf.getvalue().splitlines()


def parse(raw_lines):
    return N.parse_lines([(i + 1, s) for i, s in enumerate(raw_lines)])


class GoldenParseTests(unittest.TestCase):
    def test_every_golden_line_has_its_kind_stamped_and_bare(self):
        for g in GOLDEN:
            want = GOLD_KIND[g]
            for raw in (stamped(g), g):
                recs, bad = parse([raw])
                self.assertEqual(bad, 0, g)
                self.assertEqual([r["kind"] for r in recs], [want] if want else [], g)

    def test_golden_block_is_complete(self):
        self.assertEqual(sorted(GOLD_KIND), sorted(GOLDEN))

    def test_golden_values(self):
        by = {r["kind"]: r for r in parse([stamped(GOLD_TEX)])[0]}
        t = by["TEX"]["v"]
        self.assertEqual((t["app"], t["g"], t["tv"], t["sw"], t["st"], t["slot"]), (805, 1, 1, 1, 1, 9))
        self.assertEqual((t["fr"], t["ret"], t["pd"], t["h"]), (0x1234, 0xABCDEF, 0x0A1B2C3D, 0x06C232B2))
        rmp = parse([GOLD_RMP])[0][0]
        self.assertEqual((rmp["v"]["oid"], rmp["v"]["tp"], rmp["v"]["tl"]), (0x3E9, 0x10, 6))
        self.assertEqual((rmp["v"]["before"], rmp["v"]["after"], rmp["f"]["xy"], rmp["f"]["tag"], rmp["f"]["why"]),
                         (805, 806, "12.5,-3.5", "Bastila", "ok"))
        hk = parse([GOLD_BANNER_HOOK])[0][0]
        self.assertEqual((hk["f"]["hook"], hk["f"]["pre"], hk["v"]["addr"]), ("tex", "ok", 0x0085E601))
        bv = parse([GOLD_BANNER0])[0][0]
        self.assertEqual((bv["f"]["v"], bv["v"]["kpl"], bv["v"]["base"]), ("1.0.0", 1, 0x00400000))
        st = {r["kind"]: r for r in parse([GOLD_STAT_TEX, GOLD_STAT_REMAP, GOLD_STAT_APP, GOLD_STAT_SLOT0, GOLD_READ,
                                           GOLD_HOOKBYTES, GOLD_ABI, GOLD_CEN_TEX])[0]}
        self.assertEqual((st["STAT:tex"]["v"]["calls"], st["STAT:tex"]["v"]["comm"], st["STAT:tex"]["v"]["swap"]), (10, 4, 5))
        self.assertEqual((st["STAT:remap"]["v"]["calls"], st["STAT:remap"]["v"]["chg"], st["STAT:remap"]["v"]["nochg"],
                          st["STAT:remap"]["v"]["excl"], st["STAT:remap"]["v"]["tagun"]), (11, 5, 6, 3, 4))
        self.assertEqual(st["STAT:app"]["pairs"], [(802, 4), (1000, 9)])
        self.assertEqual(st["STAT:app"]["f"]["k"], "tex")
        self.assertEqual(st["STAT:slot"]["pairs"][0], (0, 3))
        self.assertEqual(st["STAT:slot"]["pairs"][5], (5, 7))
        rd = st["READ"]["v"]
        self.assertEqual((rd["site"], rd["addr"], rd["len"], rd["caller"], rd["n"]), (3, 0x76D200, 16, 0x0085E601, 1))
        self.assertEqual((st["HOOKBYTES"]["f"]["want"], st["HOOKBYTES"]["f"]["got"]), ("8b442404", "e9000000"))
        self.assertEqual((st["ABI"]["v"]["app"], st["ABI"]["v"]["g"], st["ABI"]["f"]["where"]), (820, 5, "remap"))
        self.assertEqual(st["CEN"]["v"]["tv"], 1)

    def test_malformed_known_lines_are_counted_not_fatal(self):
        bad = [
            "TEX pd=zz fr=00001234 ret=00abcdef app=805 g=1 tv=1 dm=0 hd=0 ph=2 cur=n_commm01 h=06c232b2 st=1 slot=09 new=1 sw=1",
            "RMP this=12345678 pd=0a1b2c3d oid=000003e9 tp=00000010 tl=6 tag=Bastila xy=1.0,2.0 xyb=0,0 app=805 h=06c232b2 why=ok",
            "TEX pd=0a1b2c3d fr=00001234 ret=00abcdef app=805 g=1 tv=1 dm=0 hd=0 ph=2 cur=n_commm01 h=06c232b2 st=1 slot=09 new=1",
            "BANNER hook=tex addr=zz pre=ok",
            "STAT tex calls=x nullg=0 unread=1 bcskip=2 nonc=3 comm=4 swap=5 frd=6 retd=7",
            "TEX app=805 app=806",
        ]
        recs, n_bad = parse(bad + [GOLD_TEX])
        self.assertEqual(n_bad, len(bad))
        self.assertEqual([r["kind"] for r in recs], ["TEX"])

    def test_unknown_header_and_fallback_shapes(self):
        recs, n_bad = parse(["hud built panel=08790760 layoutOpen=1", "# kplog session 2026-10-11 10:00:00 | kpatch.log v1",
                             "T=1000.000 f=- npc-diversity/main MARKER x=1"])
        self.assertEqual((recs, n_bad), ([], 0))
        recs, n_bad = parse(["57843211 npc-diversity/main WARN READ site=3 addr=0076d200 len=16 caller=0085e601 n=1",
                             "57843212 npc-diversity/main BANNER v=1.0.0 kpl=1 sha=6A522E71 base=00400000 ts=5f3a1b2c"])
        self.assertEqual((n_bad, [r["kind"] for r in recs]), (0, ["READ", "BANNER:v"]))

    def test_rmp_app_must_be_a_pair(self):
        recs, n_bad = parse([GOLD_RMP.replace("app=805>806", "app=805-806")])
        self.assertEqual((recs, n_bad), ([], 1))

    def test_excluded_tag_regex(self):
        for tag in ("Atton", "001_Kreia", "npc_bao_dur_2", "n_mandalore", "t3m4", "c_hk_47", "Mira_1", "player"):
            self.assertIsNotNone(N.EXCLUDED_TAG.match(tag), tag)
        for tag in ("Bastila", "Kreiax", "Mirax", "Gizka", "", "Visasx"):
            self.assertIsNone(N.EXCLUDED_TAG.match(tag), tag)


class CheckTests(unittest.TestCase):
    def test_healthy_log_passes(self):
        rc, out = run_bodies(healthy())
        self.assertEqual(rc, 0, out)
        self.assertLessEqual(len(out), 40)
        self.assertIn("RESULT PASS (0 FAIL, 0 WARN)", out)
        for head in ("PASS 1 banner: v=1.0.0, 2 hooks pre ok/jmp", "PASS 2 READ/ABI: no READ or ABI lines",
                     "PASS 3 HOOKBYTES: no HOOKBYTES lines", "PASS 4 RMP: n=6 chg=3",
                     "PASS 5 TEX: n=6 sw=1=6", "PASS 6 excluded-tag safety: 0 changed excluded-tag rows"):
            self.assertIn(head, out)
        self.assertTrue(out[0].startswith("NPC-DIV-LOG launches=1 (last shown) v=1.0.0 lines="))

    def test_info_lines_for_cen_and_last_stat(self):
        rc, out = run_bodies(healthy())
        self.assertIn("INFO 7 CEN: tex-side (app,g,tv)=1, app in final STAT app k=tex=1 (CEN k=tex lines=0)", out)
        self.assertIn("INFO 8 last STAT: tex calls=10 comm=4 swap=5 | remap calls=11 chg=5 nochg=6 excl=3 tagun=4", out)

    def test_hook2_never_fired_fails(self):
        bodies = [banner_v(), hook("tex"), hook("remap"), GOLD_BANNER_ROWS] + [tex_line(slot=9 + i) for i in range(3)]
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 1)
        self.assertIn("FAIL 4 RMP: 0 RMP lines: hook 2 did not fire for existing creatures: use decision-gate branch 3", out)
        self.assertIn("RESULT FAIL (1 FAIL, 0 WARN)", out)

    def test_read_line_fails_and_shows_first_two_only(self):
        bodies = healthy() + [GOLD_READ, GOLD_ABI, GOLD_READ]
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 1)
        self.assertIn("FAIL 2 READ/ABI: 3 READ and ABI line(s): hook read or ABI check failed", out)
        first = len(healthy()) + 1
        self.assertIn("  L%d READ site=3 addr=0076d200 len=16 caller=0085e601 n=1" % first, out)
        self.assertIn("  L%d ABI app=820 g=5 dm=2 tv=3 pd=0a1b2c3d where=remap" % (first + 1), out)
        self.assertNotIn("  L%d READ site=3 addr=0076d200 len=16 caller=0085e601 n=1" % (first + 2), out)

    def test_hookbytes_line_fails(self):
        rc, out = run_bodies(healthy() + [GOLD_HOOKBYTES])
        self.assertEqual(rc, 1)
        self.assertIn("FAIL 3 HOOKBYTES: 1 HOOKBYTES line(s): hook bytes differ from the expected prologue", out)

    def test_excluded_tag_changed_fails(self):
        bodies = healthy() + [rmp_line(oid=0x999, xy=(1.0, 2.0), before=805, after=806, tag="001_Kreia")]
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 1)
        self.assertIn("FAIL 6 excluded-tag safety: 1 changed RMP line(s) with an excluded tag "
                      "(first L%d tag=001_Kreia app=805>806)" % len(bodies), out)

    def test_excluded_tag_unchanged_passes(self):
        bodies = healthy() + [rmp_line(oid=0x999, xy=(1.0, 2.0), before=805, after=805, tag="Atton")]
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 0, out)
        self.assertIn("PASS 6 excluded-tag safety: 0 changed excluded-tag rows", out)

    def test_single_slot_per_pair_warns(self):
        bodies = [banner_v(), hook("tex"), hook("remap")] + [tex_line(805, 1, slot=9) for _ in range(3)] \
            + [tex_line(806, 1, slot=12) for _ in range(3)] + [rmp_line(oid=0x3E9 + i, xy=(1.0 + i, 2.0)) for i in range(3)]
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 0, out)
        self.assertIn("WARN 5 TEX: every (app,g) has one slot (2 pairs): seed collapse? (H1)", out)
        self.assertIn("RESULT WARN (0 FAIL, 1 WARN)", out)

    def test_no_swap_lines_fail(self):
        bodies = [banner_v(), hook("tex"), hook("remap")] + [tex_line(sw=0, slot=0) for _ in range(4)]
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 1)
        self.assertIn("FAIL 5 TEX: 0 of 4 TEX lines with sw=1: no swap path reached", out)

    def test_constant_offsets_fail(self):
        bodies = [banner_v(), hook("tex"), hook("remap")] + [tex_line()] + [rmp_line(oid=0x3E9, xy=(1.0, 2.0))] * 3
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 1)
        self.assertIn("FAIL 4 RMP: offsets invalid (oid/xy constant): oid distinct=1 xy distinct=1", out)

    def test_empty_tags_warn(self):
        rc, out = run_bodies(healthy(tag=""))
        self.assertEqual(rc, 0, out)
        self.assertIn("WARN 4 RMP: tag offset unverified: tag empty on all 6 RMP lines", out)

    def test_bad_remap_pre_fails(self):
        rc, out = run_bodies(healthy(remap_pre="bad"))
        self.assertEqual(rc, 1)
        self.assertIn("FAIL 1 banner: remap pre=bad (want ok or jmp)", out)

    def test_jmp_pre_is_accepted(self):
        rc, out = run_bodies(healthy(remap_pre="jmp"))
        self.assertEqual(rc, 0, out)
        self.assertIn("PASS 1 banner: v=1.0.0, 2 hooks pre ok/jmp", out)

    def test_missing_hook_line_fails(self):
        bodies = [b for b in healthy() if b != hook("remap")]
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 1)
        self.assertIn("FAIL 1 banner: expected 2 BANNER hook lines, found 1", out)

    def test_empty_log_fails(self):
        rc, out = run_lines([])
        self.assertEqual(rc, 1)
        self.assertEqual(out[0], "NPC-DIV-LOG launches=0 (last shown) v=none lines=0 parsed=0 malformed=0")
        self.assertIn("FAIL 1 banner: no BANNER v= line: mod not loaded or log cut", out)
        self.assertIn("FAIL 4 RMP: 0 RMP lines: hook 2 did not fire for existing creatures: use decision-gate branch 3", out)
        self.assertIn("RESULT FAIL (2 FAIL, 1 WARN)", out)

    def test_only_last_launch_is_judged(self):
        bodies = [banner_v("0.9.0"), GOLD_READ, hook("tex", "bad")] + healthy()
        rc, out = run_bodies(bodies)
        self.assertEqual(rc, 0, out)
        self.assertIn("NPC-DIV-LOG launches=2 (last shown) v=1.0.0 lines=%d parsed=" % len(bodies), out[0])

    def test_fallback_and_warn_prefix_shapes_pass(self):
        body = healthy()
        raw = [stamped(b, i, "WARN" if b.startswith("RMP") else "") for i, b in enumerate(body)]
        raw[0] = "57843211 npc-diversity/main " + body[0]
        rc, out = run_lines(raw)
        self.assertEqual(rc, 0, out)
        self.assertIn("PASS 1 banner: v=1.0.0, 2 hooks pre ok/jmp", out)

    def test_output_is_capped_at_40_lines(self):
        bodies = [banner_v(), hook("tex"), hook("remap")]
        bodies += [tex_line(app=800 + i, g=1, slot=i % 4) for i in range(30)]
        bodies += [rmp_line(oid=0x3E9 + i, xy=(1.0 + i, 2.0)) for i in range(4)]
        rc, out = run_bodies(bodies)
        self.assertLessEqual(len(out), 40)
        self.assertTrue(any(s.endswith("... +22 more pairs") for s in out), out)

    def test_cen_counts_tex_tuples_in_final_app_histogram(self):
        bodies = [banner_v(), hook("tex"), hook("remap"), tex_line(805, 1, 1), tex_line(806, 1, 1)]
        bodies += [rmp_line(oid=0x3E9 + i, xy=(1.0 + i, 2.0)) for i in range(2)]
        bodies += ["STAT app k=tex 805:3", GOLD_STAT_TEX, "STAT app k=tex 805:3"]
        rc, out = run_bodies(bodies)
        self.assertIn("INFO 7 CEN: tex-side (app,g,tv)=2, app in final STAT app k=tex=1 (CEN k=tex lines=0)", out)

    def test_missing_log_returns_2(self):
        with tempfile.TemporaryDirectory() as d:
            rc, out = run([os.path.join(d, "nope.txt")])
        self.assertEqual((rc, out), (2, ["no log: %s" % os.path.join(d, "nope.txt")]))


class NewWindowTests(unittest.TestCase):
    def test_new_reads_only_appended_lines_then_reports_none(self):
        with tempfile.TemporaryDirectory() as d:
            log = os.path.join(d, "npc-diversity_log.txt")
            state = os.path.join(d, "bm.json")
            first = healthy()
            Path(log).write_text("\n".join(stamped(b, i) for i, b in enumerate(first)) + "\n")
            rc, out = run([log, "--new", "--state", state])
            self.assertEqual(rc, 0, out)
            self.assertIn(" new=L1-L%d" % len(first), out[0])
            with open(log, "a") as f:
                f.write(stamped(tex_line(), 99) + "\n" + stamped(rmp_line(oid=0x123, xy=(3.0, 4.0)), 100) + "\n")
            rc, out = run([log, "--new", "--state", state])
            self.assertIn(" new=L%d-L%d" % (len(first) + 1, len(first) + 2), out[0])
            self.assertIn("WARN 1 banner: no BANNER v= in --new window (continuation; banner not judged)", out)
            self.assertEqual(json.loads(Path(state).read_text())[str(Path(log).resolve())]["line"], len(first) + 2)
            rc, out = run([log, "--new", "--state", state])
            self.assertEqual((rc, out), (0, ["NPC-DIV-LOG new=none (no lines since last check)"]))

    def test_new_no_advance_keeps_bookmark(self):
        with tempfile.TemporaryDirectory() as d:
            log = os.path.join(d, "npc-diversity_log.txt")
            state = os.path.join(d, "bm.json")
            Path(log).write_text(stamped(banner_v()) + "\n")
            for _ in range(2):
                rc, out = run([log, "--new", "--no-advance", "--state", state])
                self.assertIn(" new=L1-L1", out[0])
            self.assertFalse(os.path.exists(state))


if __name__ == "__main__":
    unittest.main()
