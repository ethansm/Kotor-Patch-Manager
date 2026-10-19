#!/usr/bin/env python3
"""Tests for scripts/enemy_strip_log_check.py. Synthetic fixtures only (scripts/testdata/enemy_strip/).
Run: python3 -B scripts/test_enemy_strip_log_check.py   (run_tests.py discovers it as py:scripts/test_enemy_strip_log_check.py)"""
import contextlib
import io
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import enemy_strip_log_check as E  # noqa: E402
import kplog_parse as KP  # noqa: E402

FIXTURE = HERE / "testdata" / "enemy_strip" / "enemy-effects-strip_log.txt"

# Golden block for FIXTURE (hand-checked against the fixture lines: BUILD L4, strips L16 and L20, 4 eff lines).
EXPECTED_FIXTURE = [
    "ENEMY-STRIP lines=23 sessions=1 version=0.3.0-dev truncated=0",
    "BUILD ok slots=8/8 pill=1 (L4, builds=1)",
    "STRIP transitions=2 cap=no",
    "  L16 t=3001 tier=2 slots=2 [ip_fear,ip_stun]",
    "  L20 t=4010 tier=3 slots=1 [ip_fear]",
    "tiers seen: 2, 3",
    "resrefs seen (2): ip_fear, ip_stun",
    "CHANGED 2, eff lines 4",
    "WARN 2",
    "  1x image '_' not found: its icon is not drawn",
    "  1x server creature unresolved (target=HEX): effects and icon cache skipped",
    "ES-P0b facts",
    "  FEAR (type 8 state 8): spells=[9001]",
    "  type=3 state=- spells=[9002]",
    "  type=8 state=5 spells=[9003]",
    "  creator: nonzero=3/4 invalid=0 client-oid=1 server-oid-or-ptr=2 leaderMatch=1",
    "  awareness idx3 (UNPROVEN index): values=[14] reads=1 unreadable=1",
]


def body(msg, t=1000, level=None):
    """A KpLog core line for the enemy-effects-strip patch (level token only for WARN/ERR)."""
    return "T=%d.000 f=- enemy-effects-strip/main %s%s" % (t, level + " " if level else "", msg)


def summarize(raws):
    return E.render_text(E.analyze(list(enumerate(raws, 1))))


def run_main(argv):
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        rc = E.main(argv)
    return rc, out.getvalue().splitlines()


class FixtureParse(unittest.TestCase):
    def test_every_fixture_line_is_a_kplog_line(self):
        raws = FIXTURE.read_text(encoding="utf-8").splitlines()
        self.assertEqual(len(raws), 23)
        for n, raw in enumerate(raws, 1):
            p = KP.parse_line(raw)
            self.assertIsNotNone(p, "L%d not parsed by kplog_parse" % n)
            if raw.startswith("#"):
                self.assertEqual(p["kind"], "header", n)
            else:
                self.assertEqual(p["kind"], "line", n)
                self.assertEqual(p["patch"], "enemy-effects-strip", n)
                self.assertIsNotNone(p["t_ms"], n)

    def test_session_header_is_a_session(self):
        raws = FIXTURE.read_text(encoding="utf-8").splitlines()
        self.assertEqual(KP.parse_line(raws[1]).get("session"), "2026-10-18 09:00:00")


class Golden(unittest.TestCase):
    def test_fixture_block_is_exact(self):
        self.assertEqual(E.render_text(E.analyze(list(enumerate(FIXTURE.read_text(encoding="utf-8").splitlines(), 1)))),
                         EXPECTED_FIXTURE)

    def test_main_prints_the_block_and_returns_zero(self):
        rc, lines = run_main([str(FIXTURE)])
        self.assertEqual(rc, 0)
        self.assertEqual(lines, EXPECTED_FIXTURE)

    def test_missing_log_is_rc2(self):
        with tempfile.TemporaryDirectory() as d:
            rc, lines = run_main([os.path.join(d, "nope_log.txt")])
        self.assertEqual(rc, 2)
        self.assertTrue(lines[0].startswith("no log: "))


class Grammar(unittest.TestCase):
    def test_strip_and_eff_regexes_match_source_formats(self):
        self.assertTrue(E.RE_STRIP.match("strip: target=00b2c3d4 tier=2 slots=2 [ip_fear,ip_stun]"))
        self.assertTrue(E.RE_STRIP.match("strip: target=00b2c3d4 tier=0 slots=0 []"))
        self.assertTrue(E.RE_EFF.match("eff[3] type=3 sub=1 spell=9002 dur=? creator=00000000 creatorIsLeader=0 ratio=- "
                                       "buff=- debuff=ip_stun state=-"))
        self.assertFalse(E.RE_EFF.match("eff[2] slot unreadable (ptr=00000000)"))
        self.assertTrue(E.RE_AW.match("leader skills base=00f0e1d2 raw=[1,2,3,14,5,6,7,8] awareness(idx3)=14 (UNPROVEN index)"))
        self.assertTrue(E.RE_AW_UNK.match("leader skills: awareness=? (srvLeader=00d4e5f6 stats=00000000 base=00000000)"))
        self.assertTrue(E.RE_BUILD.match("BUILD: slots=8/8 pill=1"))
        self.assertTrue(E.RE_BUILD.match("BUILD: slots=8/8 pill=1 (MaxIcons=5 Enabled=1)"))   # 0.4.4 line
        self.assertIsNone(E.RE_BUILD.match("BUILD: slots=8/8 pill=1 (junk)"))
        self.assertTrue(E.RE_CHANGED.match("effects CHANGED sig=1a2b3c4d count=4"))

    def test_empty_resref_list_gives_no_resrefs(self):
        s = E.analyze([(1, body("strip: target=00000001 tier=0 slots=0 []", t=5))])
        self.assertEqual(s["strips"][0]["resrefs"], [])
        self.assertEqual(s["strips"][0]["slots"], 0)


class Build(unittest.TestCase):
    def test_not_seen(self):
        self.assertEqual(summarize([])[1], "BUILD not seen")

    def test_warn_build_is_fail(self):
        out = summarize([body("BUILD: slots=5/8 pill=0", level="WARN")])
        self.assertEqual(out[1], "BUILD FAIL (L1: BUILD: slots=5/8 pill=0, builds=1)")

    def test_partial_build_at_info_is_fail(self):
        out = summarize([body("BUILD: slots=7/8 pill=1")])
        self.assertTrue(out[1].startswith("BUILD FAIL (L1:"), out[1])

    def test_create_failed_is_fail_not_off(self):
        for msg in ("BUILD: icon 3 create failed (why=2 vtable=00000000): drawing disabled for this HUD",
                    "BUILD: pill 0 append failed (control 00c0ffee left hidden): drawing disabled for this HUD"):
            out = summarize([body(msg, level="WARN")])
            self.assertTrue(out[1].startswith("BUILD FAIL (L1: BUILD: "), out[1])
            self.assertNotIn("BUILD off", out[1])

    def test_no_panel_is_fail(self):
        out = summarize([body("BUILD: no panel in the context", level="WARN")])
        self.assertEqual(out[1], "BUILD FAIL (L1: BUILD: no panel in the context, builds=1)")

    def test_disabled_is_off(self):
        out = summarize([body("BUILD: disabled ([Strip] Enabled=0)")])
        self.assertEqual(out[1], "BUILD off (L1: BUILD: disabled ([Strip] Enabled=0))")

    def test_last_build_wins(self):
        out = summarize([body("BUILD: slots=5/8 pill=0", level="WARN"), body("BUILD: slots=8/8 pill=1", t=2)])
        self.assertEqual(out[1], "BUILD ok slots=8/8 pill=1 (L2, builds=2)")


class Strips(unittest.TestCase):
    def test_cap_line_is_not_a_transition(self):
        out = summarize([body("strip: log cap reached (300 lines): no further strip lines this session")])
        self.assertEqual(out[2], "STRIP transitions=0 cap=hit at L1")

    def test_tail_is_last_five_and_resref_lists_capped(self):
        raws = [body("strip: target=00000001 tier=%d slots=1 [r%d]" % (i % 4, i), t=i) for i in range(7)]
        out = summarize(raws)
        self.assertEqual(out[2], "STRIP transitions=7 cap=no")
        self.assertEqual(out[3], "  L3 t=2 tier=2 slots=1 [r2]")
        self.assertEqual(out[7], "  L7 t=6 tier=2 slots=1 [r6]")
        self.assertEqual(out[8], "tiers seen: 0, 1, 2, 3")
        self.assertEqual(out[9], "resrefs seen (7): r0, r1, r2, r3, r4, ...")
        many = summarize([body("strip: target=00000001 tier=1 slots=7 [a,b,c,d,e,f,g]")])
        self.assertEqual(many[3], "  L1 t=1000 tier=1 slots=7 [a,b,c,d,e,...]")

    def test_bare_line_is_accepted_with_no_time(self):
        out = summarize(["strip: target=00000001 tier=1 slots=1 [ip_fear]"])
        self.assertEqual(out[3], "  L1 t=- tier=1 slots=1 [ip_fear]")


class Warns(unittest.TestCase):
    def test_kinds_are_masked_and_counted(self):
        out = summarize([body("image 'ip_a' not found: its icon is not drawn", level="WARN"),
                         body("image 'ip_b' not found: its icon is not drawn", level="WARN"),
                         body("server creature unresolved (target=00b2c3d4): effects and icon cache skipped", level="WARN")])
        w = out.index("WARN 3")
        self.assertEqual(out[w + 1], "  2x image '_' not found: its icon is not drawn")
        self.assertEqual(out[w + 2], "  1x server creature unresolved (target=HEX): effects and icon cache skipped")

    def test_kinds_capped_at_five(self):
        words = ["alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta"]
        raws = [body("%s went wrong" % w, level="WARN") for w in words]
        out = summarize(raws)
        w = out.index("WARN 7")
        self.assertEqual(out[w + 1:w + 6], ["  1x %s went wrong" % x for x in words[:5]])
        self.assertEqual(out[w + 6], "  ...")

    def test_err_counts_as_warn(self):
        out = summarize([body("boom", level="ERR")])
        self.assertIn("WARN 1", out)


class Facts(unittest.TestCase):
    def test_fear_not_seen_is_called_out(self):
        out = summarize([body("eff[0] type=3 sub=1 spell=9002 dur=? creator=00000000 creatorIsLeader=0 ratio=- buff=- "
                              "debuff=- state=-")])
        self.assertIn("  FEAR (type 8 state 8): not seen", out)
        self.assertIn("  type=3 state=- spells=[9002]", out)

    def test_fear_spells_and_key_list_cap(self):
        raws = [body("eff[%d] type=8 sub=0 spell=%d dur=? creator=00000000 creatorIsLeader=0 ratio=- buff=ip_fear "
                     "debuff=- state=8" % (i, 100 + i)) for i in range(7)]
        raws += [body("eff[%d] type=%d sub=0 spell=1 dur=? creator=00000000 creatorIsLeader=0 ratio=- buff=- debuff=- "
                      "state=-" % (i, 20 + i)) for i in range(7)]
        out = summarize(raws)
        self.assertIn("  FEAR (type 8 state 8): spells=[100, 101, 102, 103, 104, ...]", out)
        pairs = [x for x in out if x.startswith("  type=")]
        self.assertEqual(len(pairs), 5)
        self.assertEqual(out[-3], "  ...")  # creator and awareness lines follow the capped key list

    def test_unreadable_spell_is_excluded(self):
        s = E.analyze([(1, body("eff[0] type=8 sub=0 spell=-1 dur=? creator=00000000 creatorIsLeader=0 ratio=- buff=- "
                                "debuff=- state=8"))])
        self.assertEqual(s["spells"], {})
        self.assertEqual(s["eff_lines"], 1)

    def test_creator_buckets(self):
        def eff(i, creator, leader="0"):
            return body("eff[%d] type=3 sub=0 spell=1 dur=? creator=%s creatorIsLeader=%s ratio=- buff=- debuff=- state=-"
                        % (i, creator, leader))
        creators = [("00000000", "0"), ("7f000000", "0"), ("80000000", "0"), ("ffffffff", "0"), ("7effffff", "0"),
                    ("00000001", "0"), ("00d4e5f6", "1"), ("0a1b2c3d", "0")]
        raws = [eff(i, c, l) for i, (c, l) in enumerate(creators)]
        s = E.analyze(list(enumerate(raws, 1)))
        self.assertEqual((s["eff_lines"], s["creator_nonzero"], s["creator_invalid"], s["creator_client"],
                          s["creator_server"], s["creator_leader"]), (8, 7, 1, 2, 4, 1))
        self.assertIn("  creator: nonzero=7/8 invalid=1 client-oid=2 server-oid-or-ptr=4 leaderMatch=1", summarize(raws))

    def test_awareness_values_and_unknown(self):
        out = summarize([body("leader skills base=00f0e1d2 raw=[1,2,3,14,5,6,7,8] awareness(idx3)=14 (UNPROVEN index)"),
                         body("leader skills base=00f0e1d2 raw=[1,2,3,3,5,6,7,8] awareness(idx3)=3 (UNPROVEN index)", t=2),
                         body("leader skills: awareness=? (srvLeader=00d4e5f6 stats=00000000 base=00000000)", t=3)])
        self.assertEqual(out[-1], "  awareness idx3 (UNPROVEN index): values=[3, 14] reads=2 unreadable=1")

    def test_awareness_none(self):
        out = summarize([body("world time day=12 ms=3000")])
        self.assertEqual(out[-1], "  awareness idx3 (UNPROVEN index): values=[none] reads=0 unreadable=0")


class Header(unittest.TestCase):
    def test_version_sessions_truncated(self):
        raws = ["# kplog session 2026-10-18 09:00:00 | kpatch.log v1", body("enemy-effects-strip 0.3.0-dev loaded (Enabled=1)"),
                "# kplog session 2026-10-18 10:00:00 | kpatch.log v1",
                body("panel detail x=1 [...]", t=5)]
        out = summarize(raws)
        self.assertEqual(out[0], "ENEMY-STRIP lines=4 sessions=2 version=0.3.0-dev truncated=1")


class NewBookmark(unittest.TestCase):
    def setUp(self):
        self.d = tempfile.mkdtemp(prefix="enemy_strip_test_")
        self.addCleanup(shutil.rmtree, self.d, ignore_errors=True)
        self.log = os.path.join(self.d, "enemy-effects-strip_log.txt")
        self.state = os.path.join(self.d, "bm.json")
        shutil.copyfile(FIXTURE, self.log)

    def args(self, *extra):
        return [self.log, "--new", "--state", self.state] + list(extra)

    def test_first_run_all_then_none(self):
        rc, out = run_main(self.args())
        self.assertEqual(rc, 0)
        self.assertEqual(out[0], "ENEMY-STRIP lines=23 sessions=1 version=0.3.0-dev truncated=0 new=L1-L23")
        rc, out = run_main(self.args())
        self.assertEqual(rc, 0)
        self.assertEqual(out, ["ENEMY-STRIP new=none (no lines since last check)"])

    def test_sees_only_appended_lines(self):
        run_main(self.args())
        with open(self.log, "a", encoding="utf-8") as f:
            f.write(body("strip: target=00b2c3d4 tier=1 slots=1 [ip_fear]", t=5000) + "\n")
        rc, out = run_main(self.args())
        self.assertEqual(rc, 0)
        self.assertEqual(out[0], "ENEMY-STRIP lines=1 sessions=0 version=- truncated=0 new=L24-L24")
        self.assertEqual(out[2], "STRIP transitions=1 cap=no")
        self.assertEqual(out[3], "  L24 t=5000 tier=1 slots=1 [ip_fear]")

    def test_no_advance_keeps_bookmark(self):
        run_main(self.args())
        with open(self.log, "a", encoding="utf-8") as f:
            f.write(body("strip: target=00b2c3d4 tier=1 slots=1 [ip_fear]", t=5000) + "\n")
        rc, out = run_main(self.args("--no-advance"))
        self.assertEqual(out[0], "ENEMY-STRIP lines=1 sessions=0 version=- truncated=0 new=L24-L24")
        rc, out = run_main(self.args())
        self.assertEqual(out[0], "ENEMY-STRIP lines=1 sessions=0 version=- truncated=0 new=L24-L24")
        rc, out = run_main(self.args())
        self.assertEqual(out, ["ENEMY-STRIP new=none (no lines since last check)"])

    def test_inode_change_restarts_even_when_larger(self):
        run_main(self.args())
        new = self.log + ".new"
        with open(new, "w", encoding="utf-8") as f:
            f.write(FIXTURE.read_text(encoding="utf-8") * 2)
        os.replace(new, self.log)  # new inode, bigger than the bookmarked size and line count
        rc, out = run_main(self.args())
        self.assertEqual(out[0], "ENEMY-STRIP lines=46 sessions=2 version=0.3.0-dev truncated=0 new=L1-L46 rotated")

    def test_rotated_log_restarts_at_line_one(self):
        run_main(self.args())
        with open(self.log, "w", encoding="utf-8") as f:
            f.write(body("enemy-effects-strip 0.3.0-dev loaded (Enabled=1)", t=1) + "\n")
        rc, out = run_main(self.args())
        self.assertEqual(out[0], "ENEMY-STRIP lines=1 sessions=0 version=0.3.0-dev truncated=0 new=L1-L1 rotated")


class Cli(unittest.TestCase):
    def test_fixture_runs_as_script_without_traceback(self):
        r = subprocess.run([sys.executable, "-B", str(HERE / "enemy_strip_log_check.py"), str(FIXTURE)],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertNotIn("Traceback", r.stderr)
        self.assertEqual(r.stdout.splitlines(), EXPECTED_FIXTURE)


if __name__ == "__main__":
    unittest.main()
