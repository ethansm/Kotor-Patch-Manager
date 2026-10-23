#!/usr/bin/env python3
"""Tests for scripts/queue_hud_log_check.py and the queue-hud route in scripts/kotor2_logs.py. Synthetic fixtures only.
Run: python3 scripts/test_queue_hud_log_check.py   (run_tests.py discovers it as py:scripts/test_queue_hud_log_check.py)"""
import contextlib
import io
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kotor2_logs as K  # noqa: E402
import queue_hud_log_check as Q  # noqa: E402

# Golden strings: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp (testLogFormatGolden), copied exactly.
GOLDEN = [
    "CMD#12 m=0 cr=1a2b3c4 kind=item id=12d tgt=- dec=buffer fb=0 engq=0 buf=2 paused=0 combat=0",
    "CMD#7 m=? cr=0 kind=spell id=55 tgt=7f000001 dec=pass_unreadable fb=1 engq=unreadable buf=0 paused=1 combat=1",
    "CMD#4294967295 m=2 cr=ffffffff kind=? id=- tgt=- dec=drop_full fb=0 engq=3 buf=3 paused=0 combat=0",
    "WAIT cmd=5 m=0 why=engine_busy buf=2 engNe=1 age=120",
    "WAIT cmd=6 m=2 why=nrn_busy buf=0 engNe=0 age=0",
    "RESOLVE cmd=5 m=0 seen=1 reinj=1 age=3400",
    "RESOLVE cmd=6 m=? seen=0 reinj=0 age=0",
    "DROP cmd=1 m=1 why=combat_clear n=2",
    "DROP cmd=2 m=0 why=null_creature n=1",
    "DROP cmd=3 m=0 why=cancel n=1",
    "DROP cmd=5 m=2 why=buffer_full n=1",
    "DROP cmd=9 m=0 why=unreadable_creature n=3",
    "EDGE what=combat old=0 new=1 src=hud buf=3,0,1",
    "EDGE what=paused old=1 new=0 src=unreadable buf=",
    "QJ m=0 ne=2 types=[c,3f] round=1 cur=- aa8=0 ab8=1 paused=0 combat=0 ours=cmd5@120ms",
    "QJ m=1 ne=2 types=[c,3f] round=3 cur=3f aa8=1 ab8=0 paused=1 combat=1 ours=external",
    "QJ m=2 ne=10 types=[c,d,e,f,10,11,12,13,+2] round=1 cur=- aa8=0 ab8=0 paused=0 combat=0 ours=x@5ms",
    "QJ m=0 ne=0 types=[] round=0 cur=- aa8=0 ab8=0 paused=0 combat=0 ours=external",
    "QJ m=2 ne=16 types=[ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,ffffffff,+8] round=99 cur=7fffffff aa8=1 ab8=1 paused=1 combat=1 ours=abcdefghijklmnop@4294967295ms",
]


def stamped(body, n=0):
    return "T=%d f=- queue-hud/main %s" % (1000 + n, body)


def cmd(n, m=0, kind="item", dec="buffer"):
    return "CMD#%d m=%d cr=1a2b3c4 kind=%s id=12d tgt=- dec=%s fb=0 engq=0 buf=2 paused=0 combat=0" % (n, m, kind, dec)


def wait(n, m=0, why="engine_busy"):
    return "WAIT cmd=%d m=%d why=%s buf=2 engNe=1 age=120" % (n, m, why)


def resolve(n, m=0, seen=1, reinj=0, age=900):
    return "RESOLVE cmd=%d m=%d seen=%d reinj=%d age=%d" % (n, m, seen, reinj, age)


def drop(n, m=0, why="cancel", cnt=1):
    return "DROP cmd=%d m=%d why=%s n=%d" % (n, m, why, cnt)


def edge(what="combat", old=0, new=1, src="hud", buf="3,0,1"):
    return "EDGE what=%s old=%d new=%d src=%s buf=%s" % (what, old, new, src, buf)


def qj(m=0, types="[c,3f]", rnd=1, ours="external"):
    return "QJ m=%d ne=2 types=%s round=%d cur=- aa8=0 ab8=1 paused=0 combat=0 ours=%s" % (m, types, rnd, ours)


def _kw_line(kw, n, m=0, why=None):
    return "%s cmd=%d m=%d" % (kw, n, m) + ((" why=%s" % why) if why else "")


def reinject(n, m=0, why=None, attempt=None):
    return _kw_line("REINJECT", n, m, why) + ((" attempt=%d" % attempt) if attempt is not None else "")


def dispatch(n, m=0, why=None):
    return _kw_line("DISPATCH", n, m, why)


def cancel(n, m=0):
    return _kw_line("CANCEL", n, m)


REINJECT_NO_WHYS = ["ok", "inactive", "pending", "cancel", "max_reinject", "in_queue", "no_leader", "executed"]
EDGE_WHATS = ["combat", "paused", "menu", "party_table", "gather_m2", "creature", "journal_cap"]


def reinject_no(n, m=0, why="pending"):
    return "REINJECT_NO cmd=%d m=%d why=%s" % (n, m, why)


# Synthetic goldens for the six newer forms (C++ side is written elsewhere; these copy the spec lines exactly).
NEW_GOLDEN = [
    "VANISH cmd=3 m=0 kind=eaten_paused paused=1 exec=0 ne=0 round=0 cur=0 aa8=0 ab8=0 age=1592",
    "REINJECT cmd=3 m=0 why=eaten_paused attempt=2",
    "HANDOFF m=0 active=5 buffered=2 cmds=6,7",
    "HANDOFF m=0 active=- buffered=0 cmds=-",
    "HANDOFF_ONE cmd=6 m=0 seq=1/2",
    "STALE_CUR m=0 sub=1 since=0 aa8=0 ab8=0 paused=1",
    "IDENT_SWAP from=0 to=2",
]
VANISH_KIND_NAMES = ["executed", "unseen", "eaten_paused", "eaten_wait"]
OLD_REINJECT_BODY = ("re-injecting eaten item action for member 0 "
                     "(id=00001234, target=00000000, attempt=2) cmd=7 why=eaten_paused")


def vanish(n, m=0, kind="eaten_paused", age=1592, exec_=0):
    return "VANISH cmd=%d m=%d kind=%s paused=1 exec=%d ne=0 round=0 cur=0 aa8=0 ab8=0 age=%d" % (
        n, m, kind, exec_, age)


def handoff(m=0, active="5", buffered=0, cmds="-"):
    return "HANDOFF m=%d active=%s buffered=%d cmds=%s" % (m, active, buffered, cmds)


def handoff_one(n, m=0, seq="1/1"):
    return "HANDOFF_ONE cmd=%d m=%d seq=%s" % (n, m, seq)


def handoff_clear(m=0, n=4, ne=3, rnd=2):
    return "HANDOFF_CLEAR m=%d cmd=%d ne=%d round=%d" % (m, n, ne, rnd)


def handoff_held(n, m=0, act="drop", ms=0):
    return "HANDOFF_HELD m=%d cmd=%d act=%s ms=%d" % (m, n, act, ms)


def round_line(m=0, subs="1,10", cur="-", ne=3, types="3f,01,0d"):
    n = 0 if subs == "-" else len(subs.split(","))
    return "ROUND m=%d n=%d subs=%s cur=%s ne=%d types=%s" % (m, n, subs, cur, ne, types)


def at(t, body):
    """A body with an explicit KpLog T= stamp in ms (the time window of flag H); plain bodies carry no time."""
    return "T=%s f=- queue-hud/main %s" % (t, body)


# Synthetic goldens for the two handoff-order forms (copied from the spec lines).
NEW_ROUND_GOLDEN = [
    "HANDOFF_CLEAR m=0 cmd=4 ne=3 round=2",
    "ROUND m=0 n=2 subs=1,10 cur=- ne=3 types=3f,01,0d",
    "ROUND m=0 n=0 subs=- cur=- ne=0 types=-",
]


def stale(m=0, sub=1):
    return "STALE_CUR m=%d sub=%d since=0 aa8=0 ab8=0 paused=1" % (m, sub)


def ident_swap(frm=0, to=2):
    return "IDENT_SWAP from=%d to=%d" % (frm, to)


def wait_cur(n, m=0, age=3000):
    return "WAIT cmd=%d m=%d why=cur_busy buf=2 engNe=1 age=%d" % (n, m, age)


def mode(m=0, combat=0, soft="-", strip="shown", cancel="shown", banner="vanilla", why="ui", hostile="?", party="?"):
    """A MODE line with the grammar of fmtMode (menu and paused fixed at 0)."""
    return ("MODE m=%s combat=%d soft=%s menu=0 paused=0 hostile=%s party=%s strip=%s cancel=%s banner=%s why=%s"
            % (m, combat, soft, hostile, party, strip, cancel, banner, why))


def banner_act(act):
    return "BANNER act=%s soft=1 ctl=8 wr=8" % act


def config(show):
    """The startup line from QueueHud.cpp (chMain): show_in_combat=<n> followed by the 0.4.2 trace keys."""
    return ("pause_reinject=1 handoff_on_combat=1 handoff_clear_first=1 handoff_keep_new=1 handoff_resend_held=1"
            " show_in_combat=%d handoff_keep_unpaused=1 trace_mode=1 mode_cap=2000" % show)


def cancel_new(m, act, n, tail=""):
    return "CANCEL m=%d act=%s n=%d%s" % (m, act, n, tail)


# Goldens copied from queue_hud_logic_test.cpp (fmtMode checks (a)-(e)); (e) is the null-reason case that prints '?'.
MODE_GOLDEN = [
    "MODE m=0 combat=0 soft=- menu=0 paused=0 hostile=? party=? strip=shown cancel=shown banner=vanilla why=ui",
    "MODE m=2 combat=1 soft=1 menu=0 paused=0 hostile=0 party=1 strip=shown cancel=shown banner=hidden why=edge",
    "MODE m=1 combat=1 soft=0 menu=0 paused=0 hostile=1 party=0 strip=hidden:combat cancel=hidden:combat banner=restored why=target",
    "MODE m=? combat=? soft=- menu=0 paused=0 hostile=? party=? strip=hidden:stale cancel=hidden:off banner=hidden why=ui",
    "MODE m=3 combat=1 soft=- menu=0 paused=0 hostile=? party=? strip=? cancel=? banner=? why=?",
]


def parse(bodies):
    return Q.parse_lines([(i + 1, s) for i, s in enumerate(bodies)])


def flags_of(bodies):
    recs, _ = parse(bodies)
    return Q.qj_flags(recs)


def all_flags_of(bodies):
    recs, _ = parse(bodies)
    return Q.all_flags(recs)


def run(argv):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = Q.main(argv)
    return rc, buf.getvalue().splitlines()


def run_log(bodies, extra=()):
    """Write bodies as a log in a temp dir and run the analyzer (state file also temp)."""
    with tempfile.TemporaryDirectory() as d:
        log = os.path.join(d, "queue-hud_log.txt")
        Path(log).write_text("\n".join(stamped(b, i) for i, b in enumerate(bodies)) + "\n")
        return run([log, "--state", os.path.join(d, "bm.json"), *extra])


def run_raw(lines, extra=()):
    """Like run_log, but each line is written as given (keeps its own T= stamp, or none)."""
    with tempfile.TemporaryDirectory() as d:
        log = os.path.join(d, "queue-hud_log.txt")
        Path(log).write_text("\n".join(lines) + "\n")
        return run([log, "--state", os.path.join(d, "bm.json"), *extra])


class ParseTests(unittest.TestCase):
    def test_golden_lines_all_parse_exactly(self):
        recs, skipped = parse(GOLDEN)
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs],
                         ["CMD"] * 3 + ["WAIT"] * 2 + ["RESOLVE"] * 2 + ["DROP"] * 5 + ["EDGE"] * 2 + ["QJ"] * 5)

    def test_golden_lines_parse_when_stamped(self):
        recs, skipped = Q.parse_lines([(i + 1, stamped(s, i)) for i, s in enumerate(GOLDEN)])
        self.assertEqual(skipped, [])
        self.assertEqual(len(recs), len(GOLDEN))
        self.assertTrue(all(r["body"] == GOLDEN[i] for i, r in enumerate(recs)))

    def test_golden_values(self):
        by = {r["body"]: r for r in parse(GOLDEN)[0]}
        self.assertEqual((by[GOLDEN[1]]["cmd"], by[GOLDEN[1]]["m"], by[GOLDEN[1]]["f"]["engq"]), (7, None, "unreadable"))
        self.assertEqual((by[GOLDEN[2]]["cmd"], by[GOLDEN[2]]["m"]), (4294967295, 2))
        shown, tail = Q.qj_types(by[GOLDEN[16]])
        self.assertEqual((shown, tail), ([0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13], 2))
        shown, tail = Q.qj_types(by[GOLDEN[18]])
        self.assertEqual((len(shown), shown[0], tail), (8, 0xFFFFFFFF, 8))
        self.assertEqual(Q.qj_ours(by[GOLDEN[14]]), ("cmd5", 120))
        self.assertEqual(Q.qj_ours(by[GOLDEN[17]]), ("external", None))
        self.assertEqual(Q.qj_ours(by[GOLDEN[16]]), ("x", 5))

    def test_prefix_optional_and_non_queue_lines_ignored(self):
        a, _ = parse([cmd(3)])
        b, _ = Q.parse_lines([(1, cmd(3))])
        self.assertEqual(a[0]["body"], b[0]["body"])
        recs, skipped = parse(["hud built panel=08790760 layoutOpen=1", "re-injecting eaten Use action for member 0",
                               stamped("hud built panel=1 layoutOpen=1")])
        self.assertEqual((recs, skipped), ([], []))

    def test_reinject_no_every_reason_parses(self):
        recs, skipped = parse([reinject_no(5, 0, why) for why in REINJECT_NO_WHYS])
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs], ["REINJECT_NO"] * len(REINJECT_NO_WHYS))
        self.assertEqual([r["f"]["why"] for r in recs], REINJECT_NO_WHYS)

    def test_reinject_no_missing_why_is_skipped(self):
        recs, skipped = parse(["REINJECT_NO cmd=5 m=0"])
        self.assertEqual((recs, skipped), ([], [(1, "REINJECT_NO", "missing why")]))

    def test_edge_every_what_parses(self):
        recs, skipped = parse([edge(w, 0, 1, "hud", "") for w in EDGE_WHATS])
        self.assertEqual(skipped, [])
        self.assertEqual([r["f"]["what"] for r in recs], EDGE_WHATS)

    def test_edge_unknown_what_and_bad_old_new_are_skipped(self):
        recs, skipped = parse([edge("bogus", 0, 1, "hud", ""), edge("gather_mx", 0, 1, "hud", ""),
                               edge("combat", 2, 1, "hud", ""), edge("journal_cap", 0, 3, "hud", "")])
        self.assertEqual(recs, [])
        self.assertEqual([s[0] for s in skipped], [1, 2, 3, 4])

    def test_malformed_lines_are_skipped_not_fatal(self):
        bad = [
            "WAIT cmd=5 m=0 why=engine_busy buf=2 age=120",                                   # missing engNe
            "WAIT cmd=5 m=x why=engine_busy buf=2 engNe=1 age=120",                           # m not int or ?
            "RESOLVE cmd=5 m=0 seen=2 reinj=1 age=3400",                                      # seen not 0|1
            "RESOLVE cmd=5 m=0 seen=1 reinj=1 age=3400 junk",                                 # bare token
            "DROP cmd=1 m=1 why=combat_clear",                                                # missing n
            "EDGE what=combat old=0 new=1 src=hud buf=3,x,1",                                 # bad buf list
            "QJ m=0 ne=2 types=[zz] round=1 cur=- aa8=0 ab8=1 paused=0 combat=0 ours=external",  # bad hex
            "QJ m=0 ne=2 types=[c,3f] round=1 cur=- aa8=0 ab8=1 paused=0 combat=0",           # missing ours
            "CMD#abc m=0 cr=1 kind=item id=- tgt=- dec=buffer fb=0 engq=0 buf=0 paused=0 combat=0",  # bad number
            "CMD#5 m=0 cr=1 kind=item id=- tgt=- dec=buffer fb=0 engq=0 buf=0 paused=0",      # missing combat
            "CMD#5 m=0 cr=ZZ kind=item id=- tgt=- dec=buffer fb=0 engq=0 buf=0 paused=0 combat=0",  # bad cr hex
            "DROP cmd=1 m=1 why=combat_clear n=2 n=3",                                        # duplicate key
            "CMD#",                                                                           # bare CMD#
        ]
        recs, skipped = parse(bad + [cmd(5), resolve(5)])
        self.assertEqual(len(skipped), len(bad))
        self.assertEqual([r["kw"] for r in recs], ["CMD", "RESOLVE"])
        rc, out = run_log(bad + [cmd(5), resolve(5)])
        self.assertEqual(rc, 0)
        self.assertIn("parsed=2 skipped=13", out[0])
        self.assertTrue(out[1].startswith("SKIPPED 13: L1 WAIT missing engNe"))

    def test_existing_free_form_reinject_and_dispatch_lines_are_read_with_their_cmd(self):
        # Exact QueueHud.cpp text (QLOG re-inject / dispatch lines), with the KpLog stamp in front.
        raw = [
            "T=1200.500 f=- queue-hud/main re-injecting eaten item action for member 1 (id=00001234, target=00000000, attempt=2) cmd=7",
            "T=1201.000 f=- queue-hud/main dispatching buffered spell for member 2 (id=00005678, target=00000000, remaining=0) cmd=8",
        ]
        recs, skipped = Q.parse_lines([(i + 1, line) for i, line in enumerate(raw)])
        self.assertEqual(skipped, [])
        self.assertEqual([(r["kw"], r["cmd"], r["m"]) for r in recs], [("REINJECT", 7, 1), ("DISPATCH", 8, 2)])


class FlagTests(unittest.TestCase):
    # A: follower 0x3D wiped
    def test_A_fires_on_wipe_and_compares_hex_by_value(self):
        f = flags_of([qj(0, "[c,3D]", 2), qj(0, "[c]", 2, "external")])
        self.assertEqual([line for line, _ in f], [2])
        self.assertTrue(f[0][1].startswith("A L2 m=0 follower 0x3d wiped (was L1"))

    def test_A_fires_when_before_is_lowercase_3d(self):
        self.assertEqual(len(flags_of([qj(0, "[3d]"), qj(0, "[]", 0, "external")])), 1)

    def test_A_quiet_when_after_ours_is_cancel(self):
        self.assertEqual(flags_of([qj(0, "[3d]"), qj(0, "[]", 0, "cancel@5ms")]), [])

    def test_A_quiet_when_after_is_truncated(self):
        self.assertEqual(flags_of([qj(0, "[3d]"), qj(0, "[c,d,e,f,10,11,12,13,+2]")]), [])

    def test_A_quiet_when_still_present(self):
        self.assertEqual(flags_of([qj(0, "[3d]"), qj(0, "[3d,c]")]), [])

    def test_A_quiet_across_members(self):
        self.assertEqual(flags_of([qj(0, "[3d]"), qj(1, "[c]")]), [])

    # B: orphan round nodes
    def test_B_fires_when_round_not_decreasing_after_cancel(self):
        f = flags_of([qj(0, "[]", 3, "cancel@10ms"), qj(0, "[]", 3, "external")])
        self.assertEqual(len(f), 1)
        self.assertTrue(f[0][1].startswith("B L2 m=0 orphan-round round 3 -> 3 after ours=cancel (L1)"))

    def test_B_quiet_when_round_drops(self):
        self.assertEqual(flags_of([qj(0, "[]", 3, "cancel@10ms"), qj(0, "[]", 1, "external")]), [])

    def test_B_quiet_when_cancel_round_is_zero(self):
        self.assertEqual(flags_of([qj(0, "[]", 0, "cancel@10ms"), qj(0, "[]", 0, "external")]), [])

    # C: RESOLVE seen=0 with reinj>0
    def test_C_fires(self):
        f = flags_of([resolve(5, seen=0, reinj=1)])
        self.assertEqual([line for line, _ in f], [1])
        self.assertTrue(f[0][1].startswith("C L1 cmd=5 m=0 unseen-reinject seen=0 reinj=1"))

    def test_C_quiet(self):
        self.assertEqual(flags_of([resolve(5, seen=0, reinj=0), resolve(6, seen=1, reinj=2)]), [])


class ViewTests(unittest.TestCase):
    def test_lifecycle_line(self):
        recs, _ = parse([cmd(5), wait(5), wait(5), dispatch(5), resolve(5, seen=1, reinj=0, age=900)])
        self.assertEqual(Q.life_line(Q.build_cmds(recs)[0]),
                         "cmd 5 m=0 item dec=buffer -> wait:engine_busy x2 -> dispatch -> RESOLVE seen=1 reinj=0 age=900")

    def test_lifecycle_reinject_decline_cancel_drop(self):
        recs, _ = parse([cmd(7), reinject(7), dispatch(7, why="engine_busy"), cancel(7), drop(7, 0, "cancel", 2)])
        self.assertEqual(Q.life_line(Q.build_cmds(recs)[0]),
                         "cmd 7 m=0 item dec=buffer -> reinject -> dispatch-declined:engine_busy -> cancel -> DROP why=cancel n=2")

    def test_lifecycle_without_outcome_and_without_cmd_line(self):
        recs, _ = parse([wait(4, why="nrn_busy")])
        self.assertEqual(Q.life_line(Q.build_cmds(recs)[0]), "cmd 4 m=0 (no CMD# line) -> wait:nrn_busy -> no outcome")

    def test_decline_histogram_counts_wait_and_reinject_dispatch_declines(self):
        rc, out = run_log([wait(1, why="engine_busy"), wait(1, why="engine_busy"), reinject(2, why="pending"),
                           dispatch(3, why="ok"), dispatch(3, why="engine_busy")])
        self.assertEqual(rc, 0)
        self.assertIn("DECLINES 4: engine_busy=3 pending=1", out)

    def test_reinject_no_is_decline_in_histogram_and_lifecycle(self):
        rc, out = run_log([cmd(1), reinject_no(1, 0, "pending"), reinject_no(1, 0, "max_reinject"), reinject_no(1, 0, "ok")])
        self.assertIn("DECLINES 2: pending=1 max_reinject=1", out)
        recs, _ = parse([cmd(1), reinject_no(1, 0, "inactive"), reinject_no(1, 0, "ok")])
        self.assertEqual(Q.life_line(Q.build_cmds(recs)[0]),
                         "cmd 1 m=0 item dec=buffer -> reinject-declined:inactive -> reinject-no:ok -> no outcome")

    def test_reinject_no_shown_in_cmd_view(self):
        rc, out = run_log([cmd(2), reinject_no(2, 0, "in_queue")], ["--cmd", "2"])
        self.assertIn("L2 REINJECT_NO cmd=2 m=0 why=in_queue", out)

    def test_edge_journal_cap_is_marker_others_are_transitions(self):
        rc, out = run_log([edge("journal_cap", 0, 0, "hud", ""), edge("party_table", 0, 1, "hud", "3,0,1")])
        self.assertIn("  L1 journal_cap marker src=hud buf=", out)
        self.assertIn("  L2 party_table 0->1 src=hud buf=3,0,1", out)

    def test_drop_unreadable_creature_from_reinject_branch(self):
        rc, out = run_log([cmd(4), reinject(4, 0), drop(4, 0, "unreadable_creature", 1)])
        self.assertIn("  L3 cmd=4 m=0 unreadable_creature n=1", out)
        self.assertIn("  cmd 4 m=0 item dec=buffer -> reinject -> DROP why=unreadable_creature n=1", out)

    def test_drops_and_edges_sections(self):
        rc, out = run_log([cmd(1, 1), drop(1, 1, "combat_clear", 2), edge(), edge("paused", 1, 0, "unreadable", "")])
        self.assertIn("DROPS 1", out)
        self.assertIn("  L2 cmd=1 m=1 combat_clear n=2", out)
        self.assertIn("EDGES 2", out)
        self.assertIn("  L3 combat 0->1 src=hud buf=3,0,1", out)
        self.assertIn("  L4 paused 1->0 src=unreadable buf=", out)

    def test_flags_section_in_default_view(self):
        rc, out = run_log([qj(0, "[3d]"), qj(0, "[c]")])
        self.assertIn("FLAGS 1", out)
        self.assertTrue(any(s.startswith("  A L2 m=0 follower 0x3d wiped") for s in out))

    def test_cmd_view_shows_only_that_command(self):
        rc, out = run_log([cmd(5), wait(5), cmd(6), resolve(6), resolve(5)], ["--cmd", "5"])
        self.assertEqual(rc, 0)
        self.assertIn("L1 CMD#5 m=0 cr=1a2b3c4 kind=item id=12d tgt=- dec=buffer fb=0 engq=0 buf=2 paused=0 combat=0", out)
        self.assertIn("L5 RESOLVE cmd=5 m=0 seen=1 reinj=0 age=900", out)
        self.assertFalse(any("cmd=6" in s or "CMD#6" in s or "cmd 6 " in s for s in out))

    def test_cmd_view_capped_at_30_lines(self):
        rc, out = run_log([cmd(9)] + [wait(9)] * 39, ["--cmd", "9"])
        self.assertEqual(len(out), 30)
        self.assertEqual(out[-1], "... +12 more")

    def test_cmd_view_no_lines(self):
        rc, out = run_log([cmd(1)], ["--cmd", "99"])
        self.assertEqual(out, ["cmd 99: no lines"])

    def test_member_filter(self):
        rc, out = run_log([cmd(1, 0), wait(1, 0), cmd(2, 1), drop(2, 1, "cancel", 1), edge(), qj(0, "[3d]"), qj(1, "[]")],
                          ["--member", "1"])
        self.assertEqual(rc, 0)
        self.assertIn("QUEUE-HUD lines=7 parsed=3 skipped=0 cmds=1 qj=1 member=1", out)
        self.assertIn("EDGES 0", out)
        self.assertFalse(any("m=0" in s or "cmd 1 " in s for s in out))

    def test_default_view_under_60_lines_with_tails(self):
        bodies = []
        for i in range(1, 101):
            bodies += [cmd(i), wait(i), resolve(i)]
        bodies += [drop(i, 0, "cancel", 1) for i in range(1, 101)]
        bodies += [edge("paused") for _ in range(100)]   # not a combat edge: flag J stays out of the FLAGS count
        for _ in range(50):
            bodies += [qj(0, "[3d]", 1), qj(0, "[c]", 1)]
        rc, out = run_log(bodies)
        self.assertEqual(rc, 0)
        self.assertLess(len(out), 60)
        self.assertIn("COMMANDS 100", out)
        self.assertIn("  ... +88 more", out)                                        # 100 lifecycles, COMMANDS cap 12
        self.assertIn("DROPS 100", out)
        self.assertIn("  ... +95 more", out)
        self.assertIn("EDGES 100", out)
        self.assertIn("  ... +92 more", out)
        self.assertIn("FLAGS 50", out)
        self.assertIn("  ... +42 more", out)

    def test_missing_log_returns_2(self):
        with tempfile.TemporaryDirectory() as d:
            rc, out = run([os.path.join(d, "nope.txt")])
        self.assertEqual((rc, out), (2, ["no log: %s" % os.path.join(d, "nope.txt")]))


class NewWindowTests(unittest.TestCase):
    def test_new_reads_only_appended_lines_and_detects_rotation(self):
        with tempfile.TemporaryDirectory() as d:
            log = os.path.join(d, "queue-hud_log.txt")
            state = os.path.join(d, "bm.json")
            Path(log).write_text("\n".join(stamped(b) for b in [cmd(1), wait(1), resolve(1)]) + "\n")
            rc, out = run([log, "--new", "--state", state])
            self.assertIn("new=L1-L3", out[0])
            self.assertIn("cmds=1", out[0])
            with open(log, "a") as f:
                f.write("\n".join(stamped(b) for b in [cmd(2), resolve(2)]) + "\n")
            rc, out = run([log, "--new", "--state", state])
            self.assertIn("new=L4-L5", out[0])
            self.assertTrue(any(s.strip().startswith("cmd 2 ") for s in out))
            self.assertFalse(any(s.strip().startswith("cmd 1 ") for s in out))
            key = str(Path(log).resolve())
            self.assertEqual(json.loads(Path(state).read_text())[key]["line"], 5)
            Path(log).write_text(stamped(cmd(3)) + "\n")
            rc, out = run([log, "--new", "--state", state])
            self.assertIn("new=L1-L1 rotated", out[0])

    def test_no_advance_keeps_bookmark(self):
        with tempfile.TemporaryDirectory() as d:
            log = os.path.join(d, "queue-hud_log.txt")
            state = os.path.join(d, "bm.json")
            Path(log).write_text(stamped(cmd(1)) + "\n" + stamped(resolve(1)) + "\n")
            for _ in range(2):
                rc, out = run([log, "--new", "--no-advance", "--state", state])
                self.assertIn("new=L1-L2", out[0])
            self.assertFalse(os.path.exists(state))


class RouteTests(unittest.TestCase):
    def test_route_table_and_aliases(self):
        self.assertEqual(K.ROUTES["queue-hud_log.txt"], ("queue_hud_log_check", "queuehud"))
        self.assertEqual(K.ALIASES["queue-hud"], "queue-hud_log.txt")
        self.assertEqual(K.ALIASES["queuehud"], "queue-hud_log.txt")

    def test_route_argv_and_kind(self):
        with tempfile.TemporaryDirectory() as d:
            argv = K.route("queue-hud_log.txt", d)
            self.assertTrue(argv[1].endswith(os.path.join("scripts", "queue_hud_log_check.py")))
            self.assertEqual(argv[2], os.path.join(d, "queue-hud_log.txt"))
            self.assertEqual(K.kind_of(os.path.join(d, "queue-hud_log.txt")), "queuehud")

    def test_alias_check_missing_file(self):
        with tempfile.TemporaryDirectory() as d:
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                rc = K.check("queue-hud", [], d)
        self.assertEqual((rc, buf.getvalue().strip()), (2, "missing: queue-hud"))

    def test_routed_argv_prints_analysis(self):
        # Same argv the check subcommand builds (route + extra args), with a temp bookmark so the real cache stays untouched.
        with tempfile.TemporaryDirectory() as d:
            Path(d, "queue-hud_log.txt").write_text(stamped(cmd(1)) + "\n" + stamped(resolve(1, seen=0, reinj=1)) + "\n")
            argv = K.route(K.ALIASES["queue-hud"], d) + ["--new", "--state", os.path.join(d, "bm.json")]
            p = subprocess.run(argv, capture_output=True, text=True)
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertIn("QUEUE-HUD lines=2 parsed=2", p.stdout)
        self.assertIn("FLAGS 1", p.stdout)


class NewFormParseTests(unittest.TestCase):
    def test_new_golden_lines_parse_exactly(self):
        recs, skipped = parse(NEW_GOLDEN)
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs],
                         ["VANISH", "REINJECT", "HANDOFF", "HANDOFF", "HANDOFF_ONE", "STALE_CUR", "IDENT_SWAP"])

    def test_new_golden_values(self):
        by = {r["body"]: r for r in parse(NEW_GOLDEN)[0]}
        v = by[NEW_GOLDEN[0]]
        self.assertEqual((v["cmd"], v["m"], v["f"]["kind"], v["f"]["age"]), (3, 0, "eaten_paused", "1592"))
        self.assertEqual((by[NEW_GOLDEN[1]]["f"]["why"], by[NEW_GOLDEN[1]]["f"]["attempt"]), ("eaten_paused", "2"))
        self.assertEqual((by[NEW_GOLDEN[2]]["f"]["buffered"], by[NEW_GOLDEN[2]]["f"]["cmds"]), ("2", "6,7"))
        self.assertEqual((by[NEW_GOLDEN[3]]["f"]["active"], by[NEW_GOLDEN[3]]["f"]["cmds"]), ("-", "-"))
        self.assertEqual((by[NEW_GOLDEN[4]]["cmd"], by[NEW_GOLDEN[4]]["m"], by[NEW_GOLDEN[4]]["f"]["seq"]), (6, 0, "1/2"))
        self.assertEqual(by[NEW_GOLDEN[5]]["f"]["sub"], "1")
        self.assertEqual((by[NEW_GOLDEN[6]]["f"]["from"], by[NEW_GOLDEN[6]]["f"]["to"]), ("0", "2"))

    def test_vanish_every_kind_parses(self):
        recs, skipped = parse([vanish(3, kind=k) for k in VANISH_KIND_NAMES])
        self.assertEqual(skipped, [])
        self.assertEqual([r["f"]["kind"] for r in recs], VANISH_KIND_NAMES)

    def test_bad_new_forms_are_skipped_not_fatal(self):
        bad = [
            "VANISH cmd=3 m=0 kind=bogus paused=1 exec=0 ne=0 round=0 cur=0 aa8=0 ab8=0 age=1",
            "VANISH cmd=3 m=0 kind=unseen paused=1 exec=0 ne=0 round=0 cur=0 aa8=0 ab8=0",
            "HANDOFF_ONE cmd=6 m=0 seq=1-2",
            "HANDOFF m=0 active=5 buffered=x cmds=6",
            "IDENT_SWAP from=0",
            "STALE_CUR m=0 sub=x since=0 aa8=0 ab8=0 paused=1",
        ]
        recs, skipped = parse(bad)
        self.assertEqual(recs, [])
        self.assertEqual([(s[1], s[2]) for s in skipped], [
            ("VANISH", "malformed kind=bogus"), ("VANISH", "missing age"), ("HANDOFF_ONE", "malformed seq=1-2"),
            ("HANDOFF", "malformed buffered=x"), ("IDENT_SWAP", "missing to"), ("STALE_CUR", "malformed sub=x")])

    def test_old_free_form_reinject_with_trailing_why_parses(self):
        recs, skipped = Q.parse_lines([(1, stamped(OLD_REINJECT_BODY))])
        self.assertEqual(skipped, [])
        self.assertEqual([(r["kw"], r["cmd"], r["m"]) for r in recs], [("REINJECT", 7, 0)])
        self.assertEqual((recs[0]["f"]["why"], recs[0]["f"]["attempt"]), ("eaten_paused", "2"))

    def test_follower_wipe_reinject_with_paused_field_keeps_cmd_tail(self):
        recs, skipped = Q.parse_lines([(1, "re-injecting eaten item action for member 1 (id=00001234, target=00000000, attempt=2, paused=0) cmd=7")])
        self.assertEqual(skipped, [])
        self.assertEqual([(r["kw"], r["cmd"], r["m"]) for r in recs], [("REINJECT", 7, 1)])
        self.assertEqual(recs[0]["f"]["attempt"], "2")
        self.assertNotIn("why", recs[0]["f"])

    def test_old_free_form_reinject_without_why_has_no_why(self):
        recs, _ = Q.parse_lines([(1, "re-injecting eaten item action for member 1 (id=00001234, target=00000000, attempt=2) cmd=7")])
        self.assertNotIn("why", recs[0]["f"])
        self.assertEqual(recs[0]["f"]["attempt"], "2")


class NewLifecycleTests(unittest.TestCase):
    def test_vanish_and_numbered_reinject_steps_in_lifecycle(self):
        recs, _ = parse([cmd(3), vanish(3), reinject(3, why="eaten_paused", attempt=2), vanish(3),
                         reinject(3, why="eaten_paused", attempt=3), resolve(3, seen=1, reinj=2, age=900)])
        self.assertEqual(Q.life_line(Q.build_cmds(recs)[0]),
                         "cmd 3 m=0 item dec=buffer -> VANISH:eaten_paused -> REINJECT#2 -> VANISH:eaten_paused -> "
                         "REINJECT#3 -> RESOLVE seen=1 reinj=2 age=900")

    def test_numbered_reinject_falls_back_to_running_count_without_attempt(self):
        recs, _ = parse([cmd(3), reinject(3, why="eaten_paused"), reinject(3, why="eaten_paused")])
        self.assertEqual(Q.life_line(Q.build_cmds(recs)[0]), "cmd 3 m=0 item dec=buffer -> REINJECT#1 -> REINJECT#2 -> no outcome")

    def test_old_free_form_with_eaten_paused_renders_numbered(self):
        recs, _ = parse([cmd(7), OLD_REINJECT_BODY])
        self.assertEqual(Q.life_line(Q.build_cmds(recs)[0]), "cmd 7 m=0 item dec=buffer -> REINJECT#2 -> no outcome")

    def test_reinject_eaten_paused_is_not_a_decline(self):
        rc, out = run_log([cmd(3), reinject(3, why="eaten_paused", attempt=2)])
        self.assertIn("DECLINES 0: none", out)

    def test_other_reinject_why_is_still_a_decline(self):
        rc, out = run_log([cmd(3), reinject(3, why="engine_busy")])
        self.assertIn("DECLINES 1: engine_busy=1", out)
        self.assertIn("  cmd 3 m=0 item dec=buffer -> reinject-declined:engine_busy -> no outcome", out)

    def test_cmd_view_shows_vanish_line_and_step(self):
        rc, out = run_log([cmd(3), vanish(3)], ["--cmd", "3"])
        self.assertIn("L2 VANISH cmd=3 m=0 kind=eaten_paused paused=1 exec=0 ne=0 round=0 cur=0 aa8=0 ab8=0 age=1592", out)
        self.assertIn("cmd 3 m=0 item dec=buffer -> VANISH:eaten_paused -> no outcome", out)


class NewFlagTests(unittest.TestCase):
    # D: >= 4 VANISH kind=eaten_paused per command lifecycle
    def test_D_fires_at_fourth_eaten_paused(self):
        f = all_flags_of([cmd(3), vanish(3), vanish(3), vanish(3), vanish(3)])
        self.assertEqual(f, [(5, "D L5 cmd=3 m=0 VANISH kind=eaten_paused x4")])

    def test_D_quiet_at_three(self):
        self.assertEqual(all_flags_of([cmd(3), vanish(3), vanish(3), vanish(3)]), [])

    def test_D_counts_per_lifecycle_not_per_number(self):
        self.assertEqual(all_flags_of([cmd(3), vanish(3), vanish(3), vanish(3),
                                       cmd(3), vanish(3), vanish(3), vanish(3)]), [])

    # E: WAIT why=cur_busy age >= 3000 with no STALE_CUR for that member
    def test_E_fires_at_3000(self):
        self.assertEqual(all_flags_of([wait_cur(4, 0, 3000)]),
                         [(1, "E L1 m=0 WAIT why=cur_busy age=3000 with no STALE_CUR for this member")])

    def test_E_quiet_at_2999(self):
        self.assertEqual(all_flags_of([wait_cur(4, 0, 2999)]), [])

    def test_E_quiet_when_member_has_stale_cur(self):
        self.assertEqual(all_flags_of([wait_cur(4, 0, 3000), stale(0)]), [])

    def test_E_fires_when_stale_cur_is_for_another_member(self):
        self.assertEqual(len(all_flags_of([wait_cur(4, 0, 3000), stale(1)])), 1)

    def test_E_ignores_other_wait_reasons(self):
        self.assertEqual(all_flags_of(["WAIT cmd=4 m=0 why=engine_busy buf=2 engNe=1 age=9000"]), [])

    # F: HANDOFF buffered=n > 0 with fewer than n HANDOFF_ONE for that member before its next HANDOFF
    def test_F_fires_when_fewer_handoff_one_than_buffered(self):
        self.assertEqual(all_flags_of([handoff(0, "-", 2, "6,7"), handoff_one(6, 0, "1/2")]),
                         [(1, "F L1 m=0 HANDOFF buffered=2 but 1 HANDOFF_ONE")])

    def test_F_quiet_when_all_handoff_one_present(self):
        self.assertEqual(all_flags_of([handoff(0, "-", 2, "6,7"), handoff_one(6, 0, "1/2"), handoff_one(7, 0, "2/2")]), [])

    def test_F_quiet_when_buffered_zero(self):
        self.assertEqual(all_flags_of([handoff(0, "-", 0, "-")]), [])

    def test_F_window_closes_at_next_handoff_of_same_member(self):
        self.assertEqual(all_flags_of([handoff(0, "-", 1, "6"), handoff(0, "-", 1, "6"), handoff_one(6, 0, "1/1")]),
                         [(1, "F L1 m=0 HANDOFF buffered=1 but 0 HANDOFF_ONE")])

    def test_F_ignores_handoff_one_of_other_members(self):
        self.assertEqual(len(all_flags_of([handoff(0, "-", 1, "6"), handoff_one(6, 1, "1/1")])), 1)

    # G: two VANISH kind=executed, or two RESOLVE seen=1, for one command lifecycle
    def test_G_fires_on_two_vanish_executed(self):
        self.assertEqual(all_flags_of([cmd(3), vanish(3, kind="executed", exec_=1), vanish(3, kind="executed", exec_=1)]),
                         [(3, "G L3 cmd=3 m=0 executed twice (VANISH kind=executed x2, RESOLVE seen=1 x0)")])

    def test_G_quiet_when_vanish_executed_then_resolve_seen(self):
        self.assertEqual(all_flags_of([cmd(3), vanish(3, kind="executed", exec_=1), resolve(3, seen=1)]), [])

    def test_G_fires_on_two_resolve_seen_1(self):
        self.assertEqual(all_flags_of([cmd(3), resolve(3, seen=1), resolve(3, seen=1)]),
                         [(3, "G L3 cmd=3 m=0 executed twice (VANISH kind=executed x0, RESOLVE seen=1 x2)")])

    def test_G_quiet_across_lifecycles(self):
        self.assertEqual(all_flags_of([cmd(3), vanish(3, kind="executed", exec_=1),
                                       cmd(3), vanish(3, kind="executed", exec_=1)]), [])

    def test_new_flags_listed_in_flags_section(self):
        rc, out = run_log([handoff(0, "5", 1, "6"), wait_cur(4, 0, 3000)])
        self.assertIn("FLAGS 3", out)
        self.assertIn("  F L1 m=0 HANDOFF buffered=1 but 0 HANDOFF_ONE", out)
        self.assertIn("  I L1 m=0 HANDOFF active=5 never re-sent (trigger lost at hand-off)", out)
        self.assertIn("  E L2 m=0 WAIT why=cur_busy age=3000 with no STALE_CUR for this member", out)


class NewSectionTests(unittest.TestCase):
    def test_handoffs_one_line_per_handoff_with_follow_count(self):
        rc, out = run_log([handoff(0, "5", 2, "6,7"), handoff_one(6, 0, "1/2"), handoff_one(7, 0, "2/2"), handoff(1, "-", 0, "-")])
        self.assertIn("HANDOFFS 2", out)
        self.assertIn("  L1 m=0 active=5 buffered=2 cmds=6,7 -> HANDOFF_ONE x2", out)
        self.assertIn("  L4 m=1 active=- buffered=0 cmds=- -> HANDOFF_ONE x0", out)
        self.assertIn("FLAGS 1", out)
        self.assertIn("  I L1 m=0 HANDOFF active=5 never re-sent (trigger lost at hand-off)", out)

    def test_handoffs_none_and_ident_swap_suffix(self):
        rc, out = run_log([ident_swap(0, 2)])
        self.assertIn("HANDOFFS 0 (IDENT_SWAP x1)", out)
        self.assertIn("VANISHES 0", out)
        self.assertIn("STALE_CUR 0", out)

    def test_vanishes_histogram_and_below_d_threshold(self):
        bodies = [vanish(1, kind="executed", exec_=1)] + [vanish(2, kind="eaten_paused")] * 3 + [vanish(3, kind="eaten_wait")]
        rc, out = run_log(bodies)
        self.assertIn("VANISHES 5", out)
        self.assertIn("  executed=1 unseen=0 eaten_paused=3 eaten_wait=1", out)
        self.assertIn("FLAGS 0", out)

    def test_stale_cur_count_and_members(self):
        rc, out = run_log([stale(0), stale(0), stale(1)])
        self.assertIn("STALE_CUR 3", out)
        self.assertIn("  m=0 x2 first L1", out)
        self.assertIn("  m=1 x1 first L3", out)

    def test_default_view_worst_case_is_exactly_74_lines(self):
        # Every section over its cap, plus one SKIPPED line: 1+1+1+14+7+10+4+6+2+4+14+10 = 74 (COMMANDS cap 12, ROUNDS cap 4,
        # MODES cap 12 = title + 12 rows + "(+N more)").
        bodies = [cmd(100 + i) for i in range(20)]                                  # 20 lifecycles, COMMANDS shows 12
        bodies += [drop(200 + i, 0, "cancel", 1) for i in range(6)]                 # 6 DROP lifecycles, DROPS 6
        bodies += [edge() for _ in range(9)]                                        # EDGES 9
        bodies += [handoff(m, "-", 0, "-") for m in range(3)]                       # HANDOFFS 3 (+ suffix below)
        bodies += [handoff(3, "5", 1, "6"), handoff(4, "5", 1, "6"), handoff(4, "5", 1, "6")]   # 3 F flags, HANDOFFS 6
        bodies += [vanish(5)]                                                       # VANISHES 1
        bodies += [stale(0), stale(1), stale(2)]                                    # STALE_CUR 3 (3 members)
        bodies += [ident_swap(), handoff_clear(4)]                                  # HANDOFFS title suffix (2 kinds)
        bodies += [round_line(0) for _ in range(5)]                                 # ROUNDS 5, 4 shown + 1 more
        bodies += [wait(7)]                                                         # DECLINES 1, lifecycle cmd 7
        bodies += [wait_cur(9, 5, 3000), wait_cur(9, 5, 3000)]                      # 2 E flags (member 5 has no STALE)
        bodies += ["DROP cmd=1 m=1 why=combat_clear"]                               # SKIPPED 1 (missing n)
        bodies += [mode(0, 0) for _ in range(13)]                                   # MODES 13: 12 rows + (+1 more), no flags
        for _ in range(5):
            bodies += [qj(0, "[3d]", 1), qj(0, "[c]", 1)]                           # 5 flags A
        rc, out = run_log(bodies)
        self.assertEqual(rc, 0)
        self.assertEqual(len(out), 74)
        self.assertIn("MODES 13", out)
        self.assertEqual(out[out.index("MODES 13") + 13], "  (+1 more)")
        self.assertIn("SKIPPED 1: L", out[1])
        self.assertIn("COMMANDS 29", out)                                           # 20 + 6 DROP + cmd 5, 7, 9
        self.assertIn("  ... +17 more", out)                                        # 29 lifecycles, 12 shown
        self.assertIn("DROPS 6", out)
        self.assertIn("  ... +1 more", out)
        self.assertIn("EDGES 9", out)
        self.assertIn("HANDOFFS 6 (IDENT_SWAP x1) (clear x1)", out)
        self.assertIn("  ... +4 more", out)                                         # 6 HANDOFFS, 2 shown
        i = out.index("ROUNDS 5")
        self.assertEqual(out[i + 5], "  ... +1 more")                              # ROUNDS block is title + 4 + more = 6
        self.assertEqual(out[i + 6], "VANISHES 1")
        self.assertIn("VANISHES 1", out)
        self.assertIn("STALE_CUR 3", out)
        self.assertIn("  ... +1 more", out)                                         # 3 members, 2 shown
        self.assertIn("FLAGS 13", out)                                              # 3 F + 3 I + 2 E + 5 A; 8 shown, +5 more
        self.assertIn("  ... +5 more", out)                                         # 13 flags, 8 shown


class RoundParseTests(unittest.TestCase):
    def test_golden_round_detach_parses(self):
        recs, skipped = parse(["ROUND_DETACH m=0 n=1 act=detach ok=1 why=held_head"])
        self.assertEqual(skipped, [])
        self.assertEqual((recs[0]["kw"], recs[0]["f"]["act"], recs[0]["f"]["why"]), ("ROUND_DETACH", "detach", "held_head"))

    def test_golden_round_rot_parses(self):
        recs, skipped = parse(["ROUND_ROT m=0 n=3 moved=2 ok=1"])
        self.assertEqual(skipped, [])
        self.assertEqual((recs[0]["kw"], recs[0]["f"]["moved"], recs[0]["f"]["ok"]), ("ROUND_ROT", "2", "1"))

    def test_golden_handoff_keep_parses(self):
        recs, skipped = parse(["HANDOFF_KEEP m=0 cmd=7 ne=5 round=2"])
        self.assertEqual(skipped, [])
        self.assertEqual(recs[0]["kw"], "HANDOFF_KEEP")
        self.assertEqual((recs[0]["cmd"], recs[0]["m"], recs[0]["f"]["ne"], recs[0]["f"]["round"]), (7, 0, "5", "2"))

    def test_golden_handoff_clear_and_round_parse_exactly(self):
        recs, skipped = parse(NEW_ROUND_GOLDEN)
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs], ["HANDOFF_CLEAR", "ROUND", "ROUND"])

    def test_golden_round_values(self):
        clear, full, empty = parse(NEW_ROUND_GOLDEN)[0]
        self.assertEqual((clear["cmd"], clear["m"], clear["f"]["ne"], clear["f"]["round"]), (4, 0, "3", "2"))
        self.assertEqual((full["f"]["n"], full["f"]["subs"], full["f"]["cur"], full["f"]["types"]),
                         ("2", "1,10", "-", "3f,01,0d"))
        self.assertEqual((empty["f"]["n"], empty["f"]["subs"], empty["f"]["types"]), ("0", "-", "-"))

    def test_round_cur_is_decimal_and_bad_forms_are_skipped(self):
        bad = ["ROUND m=0 n=1 subs=1 cur=3f ne=0 types=-", "ROUND m=0 n=2 subs=1,x cur=- ne=3 types=-",
               "ROUND m=0 n=1 subs=1 cur=- ne=0 types=zz", "ROUND m=0 n=1 subs=1 cur=- ne=0",
               "HANDOFF_CLEAR m=0 cmd=4 ne=3"]
        recs, skipped = parse(bad)
        self.assertEqual(recs, [])
        self.assertEqual([(s[1], s[2]) for s in skipped], [
            ("ROUND", "malformed cur=3f"), ("ROUND", "malformed subs=1,x"), ("ROUND", "malformed types=zz"),
            ("ROUND", "missing types"), ("HANDOFF_CLEAR", "missing round")])

    def test_stamp_is_kept_on_records(self):
        recs, _ = Q.parse_lines([(1, at(1234.5, round_line(0))), (2, round_line(0))])
        self.assertEqual([r["t"] for r in recs], [1234.5, None])


class HandoffClearAndRoundSectionTests(unittest.TestCase):
    def test_handoff_clear_counts_in_handoffs_title_and_makes_no_lifecycle(self):
        rc, out = run_log([handoff(0, "5", 1, "6"), handoff_one(6, 0, "1/1"), handoff_clear(4), handoff_clear(5)])
        self.assertIn("HANDOFFS 1 (clear x2)", out)
        self.assertIn("  L1 m=0 active=5 buffered=1 cmds=6 -> HANDOFF_ONE x1", out)
        self.assertIn("COMMANDS 0", out)
        self.assertIn("FLAGS 1", out)                                               # only I: active 5 never re-sent
        self.assertIn("  I L1 m=0 HANDOFF active=5 never re-sent (trigger lost at hand-off)", out)

    def test_handoff_clear_alone_shows_zero_handoffs_with_suffix(self):
        rc, out = run_log([handoff_clear(4)])
        self.assertIn("HANDOFFS 0 (clear x1)", out)

    def test_rounds_timeline_shows_four_of_six_then_more(self):
        bodies = [round_line(0, "1,10", "-", 3, "3f,01,0d"), round_line(1, "-", "-", 0, "-"), round_line(0, "9", "2", 1, "01"),
                  round_line(0, "10,1", "3", 2, "0d,01"), round_line(2, "1", "-", 1, "3f"), round_line(2, "1", "-", 1, "3f")]
        rc, out = run_log(bodies)
        i = out.index("ROUNDS 6")
        self.assertEqual(out[i + 1:i + 5], [
            "  L1 m=0 n=2 subs=1,10 cur=- ne=3 types=3f,01,0d",
            "  L2 m=1 n=0 subs=- cur=- ne=0 types=-",
            "  L3 m=0 n=1 subs=9 cur=2 ne=1 types=01",
            "  L4 m=0 n=2 subs=10,1 cur=3 ne=2 types=0d,01"])
        self.assertEqual(out[i + 5], "  ... +2 more")
        self.assertEqual(out[i + 6], "VANISHES 0")
        self.assertIn("FLAGS 0", out)

    def test_rounds_none_when_absent(self):
        rc, out = run_log([cmd(1)])
        i = out.index("ROUNDS 0")
        self.assertEqual(out[i + 1], "  (none)")


class HandoffOrderFlagTests(unittest.TestCase):
    # H: first non-empty-subs ROUND of m within 1500 ms after a HANDOFF_ONE of m; its first subtype must be 9 or 10
    def test_H_raised_when_first_subtype_is_an_attack_node(self):
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/1")), at(1200, round_line(0, "1,10"))]),
                         [(2, "H L2 m=0 handoff-not-first subs=1,10 (+200 ms after HANDOFF_ONE L1)")])

    def test_H_clean_when_first_subtype_is_9_or_10(self):
        for subs in ("10", "10,1", "9", "9,1"):
            with self.subTest(subs=subs):
                self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/1")), at(1200, round_line(0, subs))]), [])

    def test_H_window_is_inclusive_at_1500_and_quiet_past_it(self):
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/1")), at(2500, round_line(0, "1"))]),
                         [(2, "H L2 m=0 handoff-not-first subs=1 (+1500 ms after HANDOFF_ONE L1)")])
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/1")), at(2501, round_line(0, "1"))]), [])

    def test_H_skips_empty_subs_rounds(self):
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/1")), at(1100, round_line(0, "-")),
                                       at(1200, round_line(0, "1"))]),
                         [(3, "H L3 m=0 handoff-not-first subs=1 (+200 ms after HANDOFF_ONE L1)")])
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/1")), at(1100, round_line(0, "-"))]), [])

    def test_H_judges_only_the_first_qualifying_round(self):
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/1")), at(1100, round_line(0, "10")),
                                       at(1200, round_line(0, "1"))]), [])

    def test_H_reports_a_round_once_for_repeated_handoff_one(self):
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/2")), at(1100, handoff_one(7, 0, "2/2")),
                                       at(1200, round_line(0, "1"))]),
                         [(3, "H L3 m=0 handoff-not-first subs=1 (+200 ms after HANDOFF_ONE L1)")])

    def test_H_is_per_member(self):
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 0, "1/1")), at(1100, round_line(1, "1"))]), [])
        self.assertEqual(all_flags_of([at(1000, handoff_one(6, 1, "1/1")), at(1100, round_line(0, "1"))]), [])
        self.assertEqual(len(all_flags_of([at(1000, handoff_one(6, 1, "1/1")), at(1100, round_line(1, "1"))])), 1)

    def test_H_needs_stamps(self):
        self.assertEqual(all_flags_of([handoff_one(6, 0, "1/1"), round_line(0, "1")]), [])

    def test_H_listed_in_flags_section(self):
        rc, out = run_log([handoff_one(6, 0, "1/1"), round_line(0, "1,10")])   # run_log stamps T=1000 and T=1001
        self.assertIn("FLAGS 1", out)
        self.assertIn("  H L2 m=0 handoff-not-first subs=1,10 (+1 ms after HANDOFF_ONE L1)", out)


class HandoffTriggerFlagTests(unittest.TestCase):
    # I: HANDOFF active=<n> (not -) with neither HANDOFF_ONE cmd=<n> nor HANDOFF_HELD cmd=<n> act=drop in its window
    def test_HANDOFF_HELD_parses_every_act_and_rejects_unknown_act(self):
        recs, skipped = parse(["HANDOFF_HELD m=0 cmd=8 act=resend ms=70", "HANDOFF_HELD m=? cmd=0 act=wait ms=0",
                               "HANDOFF_HELD m=2 cmd=12 act=drop ms=1500", "HANDOFF_HELD m=0 cmd=8 act=bogus ms=1"])
        self.assertEqual([(r["kw"], r["m"], r["cmd"], r["f"]["act"]) for r in recs],
                         [("HANDOFF_HELD", 0, 8, "resend"), ("HANDOFF_HELD", None, 0, "wait"), ("HANDOFF_HELD", 2, 12, "drop")])
        self.assertEqual([(n, label) for n, label, _ in skipped], [(4, "HANDOFF_HELD")])

    def test_I_quiet_when_active_is_re_sent(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 1, "8"), handoff_one(8, 0, "1/1")]), [])

    def test_I_fires_when_active_never_re_sent(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 0, "-")]),
                         [(1, "I L1 m=0 HANDOFF active=8 never re-sent (trigger lost at hand-off)")])

    def test_I_quiet_when_held_resend_is_followed_by_handoff_one(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 1, "8"), handoff_held(8, 0, "resend", 70), handoff_one(8, 0, "1/1")]), [])

    def test_I_quiet_when_held_drop_settles_active_without_handoff_one(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 0, "-"), handoff_held(8, 0, "drop", 1500)]), [])

    def test_I_quiet_when_active_dropped_executed(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 0, "-"), drop(8, 0, "executed")]), [])

    def test_I_quiet_when_active_dropped_refused(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 0, "-"), drop(8, 0, "refused")]), [])

    def test_I_fires_when_active_dropped_for_other_reason(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 0, "-"), drop(8, 0, "cancel")]),
                         [(1, "I L1 m=0 HANDOFF active=8 never re-sent (trigger lost at hand-off)")])

    def test_I_fires_when_held_resend_has_no_handoff_one(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 0, "-"), handoff_held(8, 0, "resend", 70)]),
                         [(1, "I L1 m=0 HANDOFF active=8 never re-sent (trigger lost at hand-off)")])

    def test_I_quiet_when_active_is_dash(self):
        self.assertEqual(all_flags_of([handoff(0, "-", 0, "-")]), [])

    def test_I_window_closes_at_next_handoff_of_same_member(self):
        self.assertEqual(all_flags_of([handoff(0, "8", 0, "-"), handoff(0, "-", 0, "-"), handoff_one(8, 0, "1/1")]),
                         [(1, "I L1 m=0 HANDOFF active=8 never re-sent (trigger lost at hand-off)")])

    def test_I_ignores_handoff_one_and_held_of_other_members(self):
        self.assertEqual(len(all_flags_of([handoff(0, "8", 0, "-"), handoff_one(8, 1, "1/1"), handoff_held(8, 1, "drop")])), 1)

    def test_I_listed_in_flags_section(self):
        rc, out = run_log([handoff(0, "8", 0, "-")])
        self.assertIn("FLAGS 1", out)
        self.assertIn("  I L1 m=0 HANDOFF active=8 never re-sent (trigger lost at hand-off)", out)


def handoff_out(n, m=0, exec_=1, first="120", end=8000):
    return "HANDOFF_OUT m=%d cmd=%d exec=%d firstExecMs=%s endMs=%d" % (m, n, exec_, first, end)


CMBT_BODY = ("CMBT_FIELDS m=0 cr520=80000448,00000000,00000000,00000000,00000000,00000000,00000000,00000000,"
             "00000000,00000000,00000000,000000ab t550=7f000001 t564=00000000 t568=ffffffff")


class T4TraceTests(unittest.TestCase):
    def test_new_lines_parse(self):
        recs, skipped = parse([handoff_out(6), handoff_out(7, 1, 0, "-", 8003), "CLICK t=123456 x=640 y=360", "CLICK t=7 x=- y=-",
                               CMBT_BODY, "CMBT_FIELDS m=2 cr520=?,00000001 t550=? t564=00000005 t568=?"])
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs], ["HANDOFF_OUT"] * 2 + ["CLICK"] * 2 + ["CMBT_FIELDS"] * 2)
        self.assertEqual((recs[0]["cmd"], recs[0]["m"], recs[1]["f"]["firstExecMs"]), (6, 0, "-"))

    def test_bad_new_lines_are_skipped(self):
        bad = ["HANDOFF_OUT m=0 cmd=6 exec=2 firstExecMs=1 endMs=2", "HANDOFF_OUT m=0 cmd=6 exec=1 firstExecMs=x endMs=2",
               "HANDOFF_OUT m=0 cmd=6 exec=1 firstExecMs=1", "CLICK t=7 x=1", "CLICK t=a x=1 y=1",
               "CMBT_FIELDS m=0 cr520=zz t550=? t564=? t568=?", "CMBT_FIELDS m=0 cr520=00000000 t550=1 t564=? t568=?"]
        recs, skipped = parse(bad)
        self.assertEqual(recs, [])
        self.assertEqual(len(skipped), len(bad))

    def test_J_fires_for_combat_edge_without_round(self):
        rc, out = run_log([edge("combat", 0, 1)])
        self.assertIn("FLAGS 1", out)
        self.assertIn("  J L1 combat edge with no ROUND within 1500 ms", out)

    def test_J_quiet_when_round_follows(self):
        recs, _ = Q.parse_lines([(1, at(1000, edge("combat", 0, 1))), (2, at(2400, round_line()))])
        self.assertEqual(Q.edge_silent_flags(recs), [])

    def test_J_fires_when_round_comes_too_late_or_before(self):
        recs, _ = Q.parse_lines([(1, at(500, round_line())), (2, at(1000, edge("combat", 0, 1))), (3, at(2501, round_line()))])
        self.assertEqual(Q.edge_silent_flags(recs), [(2, "J L2 combat edge with no ROUND within 1500 ms")])

    def test_J_ignores_non_combat_edges_exits_and_bare_lines(self):
        recs, _ = Q.parse_lines([(1, at(1000, edge("paused", 0, 1))), (2, at(2000, edge("combat", 1, 0))), (3, edge("combat", 0, 1))])
        self.assertEqual(Q.edge_silent_flags(recs), [])


def spell_args(n, where="hook", m=0, pos="0,0,0"):
    return ("SPELL_ARGS where=%s cmd=%d m=%d spell=2c tgt=00dbee74 multi=1 meta=0 dom=0 pos=%s flag=0 posaddr=00a1b2c0"
            % (where, n, m, pos))


def spell_out(n, m=0, exec_=1, first="120", end=8000):
    return "SPELL_OUT m=%d cmd=%d exec=%d firstExecMs=%s endMs=%d" % (m, n, exec_, first, end)


# Synthetic goldens for the three spell and soft-combat forms (copied from the spec lines; not yet in queue_hud_logic_test.cpp).
SPELL_GOLDEN = [
    "SPELL_ARGS where=hook cmd=9 m=0 spell=2c tgt=00dbee74 multi=1 meta=0 dom=0 pos=3f800000,40000000,40400000 flag=0 posaddr=12ab30",
    "SPELL_ARGS where=replay cmd=4 m=0 spell=2c tgt=00dbee74 multi=1 meta=0 dom=0 pos=0,0,0 flag=0 posaddr=19f000",
    "SPELL_ARGS where=replay cmd=13 m=2 spell=1f tgt=ffffffff multi=0 meta=2 dom=0 pos=0,0,0 flag=5 posaddr=19f000",
    "SPELL_OUT m=0 cmd=4 exec=0 firstExecMs=- endMs=8000",
    "SPELL_OUT m=2 cmd=13 exec=1 firstExecMs=120 endMs=8003",
    "SOFT_CAND m=0 hostile=1 party=0 none=0 cr0=3 cr1=000000ab",
    "SOFT_CAND m=1 hostile=0 party=0 none=1 cr0=0 cr1=0",
    "SOFT_CAND m=2 hostile=0 party=0 none=1 cr0=? cr1=?",
]
K_MSG = "buffered spell replayed, never executed (SPELL_OUT exec=0 or absent)"


class SpellTraceTests(unittest.TestCase):
    def test_spell_and_soft_cand_golden_lines_parse_exactly(self):
        recs, skipped = parse(SPELL_GOLDEN)
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs], ["SPELL_ARGS"] * 3 + ["SPELL_OUT"] * 2 + ["SOFT_CAND"] * 3)
        self.assertEqual((recs[1]["f"]["where"], recs[1]["f"]["pos"], recs[1]["f"]["tgt"]), ("replay", "0,0,0", "00dbee74"))
        self.assertEqual((recs[1]["f"]["flag"], recs[1]["f"]["posaddr"]), ("0", "19f000"))
        self.assertEqual((recs[2]["cmd"], recs[2]["m"], recs[2]["f"]["pos"], recs[2]["f"]["flag"]), (13, 2, "0,0,0", "5"))
        self.assertEqual((recs[3]["f"]["exec"], recs[3]["f"]["firstExecMs"], recs[4]["f"]["endMs"]), ("0", "-", "8003"))
        self.assertEqual((recs[5]["f"]["cr1"], recs[5]["cmd"]), ("000000ab", None))

    def test_spell_and_soft_cand_golden_lines_parse_when_stamped(self):
        recs, skipped = Q.parse_lines([(i + 1, at(1000 + i, b)) for i, b in enumerate(SPELL_GOLDEN)])
        self.assertEqual(skipped, [])
        self.assertEqual(len(recs), len(SPELL_GOLDEN))

    def test_spell_bad_forms_are_skipped_not_fatal(self):
        bad = [
            "SPELL_ARGS where=cast cmd=4 m=0 spell=2c tgt=0 multi=1 meta=0 dom=0 pos=0,0,0 flag=0 posaddr=0",
            "SPELL_ARGS where=replay cmd=4 m=0 spell=zz tgt=0 multi=1 meta=0 dom=0 pos=0,0,0 flag=0 posaddr=0",
            "SPELL_ARGS where=replay cmd=4 m=0 spell=2c tgt=- multi=1 meta=0 dom=0 pos=0,0,0 flag=0 posaddr=0",
            "SPELL_ARGS where=replay cmd=4 m=0 spell=2c tgt=0 multi=1 meta=0 dom=0 pos=1,2 flag=0 posaddr=0",
            "SPELL_ARGS where=replay cmd=4 m=0 spell=2c tgt=0 multi=1 meta=0 dom=0 pos=0,0,0 flag=x posaddr=0",
            "SPELL_ARGS where=hook cmd=9 m=0 spell=2c arg2=0 tgt=0 multi=1 meta=0 dom=0 posptr=0 pos=orig",   # the pre-0.4.6 form
            "SPELL_ARGS where=hook cmd=9 m=0 spell=2c tgt=0 multi=1 meta=0 dom=0 pos=0,0,0 flag=0",
            "SPELL_OUT m=0 cmd=4 exec=2 firstExecMs=- endMs=8000",
            "SPELL_OUT m=0 cmd=4 exec=1 firstExecMs=-",
            "SOFT_CAND m=0 hostile=2 party=0 none=0 cr0=3 cr1=0",
            "SOFT_CAND m=0 hostile=1 party=0 none=0 cr0=3 cr1=xyz",
        ]
        recs, skipped = parse(bad)
        self.assertEqual(recs, [])
        self.assertEqual(len(skipped), len(bad))

    def test_cmd_view_shows_spell_lines_of_that_command_only(self):
        rc, out = run_log([cmd(4), spell_args(4, "replay"), spell_out(4, exec_=0, first="-"),
                           spell_args(9, "hook"), spell_out(9, exec_=1)], ("--cmd", "4"))
        self.assertEqual(rc, 0)
        self.assertTrue(out[0].startswith("cmd 4 "))
        self.assertTrue(out[1].startswith("L1 CMD#4 "))   # existing raw CMD row, then the spell rows in log order
        self.assertEqual(out[2:], ["L2 " + spell_args(4, "replay"), "L3 " + spell_out(4, exec_=0, first="-")])

    def test_spell_lines_add_no_command_lifecycle(self):
        rc, out = run_log([spell_args(9, "hook"), spell_out(9, exec_=1), SPELL_GOLDEN[-1]])
        self.assertIn("cmds=0", out[0])
        self.assertIn("COMMANDS 0", out)
        rc, out = run_log([spell_args(9, "hook"), spell_out(9, exec_=1)], ("--cmd", "9"))
        self.assertEqual(out, ["L1 " + spell_args(9, "hook"), "L2 " + spell_out(9, exec_=1)])


class SpellFlagTests(unittest.TestCase):
    # K: SPELL_ARGS where=replay with no SPELL_OUT exec=1 for the same (m, cmd)
    def test_K_fires_on_replay_with_exec_0(self):
        self.assertEqual(all_flags_of([spell_args(4, "replay"), spell_out(4, exec_=0, first="-")]),
                         [(1, "K L1 cmd=4 m=0 " + K_MSG)])

    def test_K_fires_on_replay_with_no_spell_out(self):
        self.assertEqual(all_flags_of([spell_args(4, "replay")]), [(1, "K L1 cmd=4 m=0 " + K_MSG)])

    def test_K_quiet_on_replay_with_exec_1(self):
        self.assertEqual(all_flags_of([spell_args(4, "replay"), spell_out(4, exec_=1)]), [])

    def test_K_quiet_on_hook_only_dispatch_now(self):
        self.assertEqual(all_flags_of([spell_args(9, "hook")]), [])
        self.assertEqual(all_flags_of([spell_args(9, "hook"), spell_out(9, exec_=1)]), [])

    def test_K_keyed_by_member_and_cmd(self):
        self.assertEqual(all_flags_of([spell_args(4, "replay", m=0), spell_out(4, m=1, exec_=1)]),
                         [(1, "K L1 cmd=4 m=0 " + K_MSG)])

    def test_K_reported_once_per_command_at_first_replay(self):
        self.assertEqual(all_flags_of([spell_args(4, "replay"), spell_args(4, "replay")]), [(1, "K L1 cmd=4 m=0 " + K_MSG)])

    def test_K_listed_in_flags_section(self):
        rc, out = run_log([spell_args(4, "replay"), spell_out(4, exec_=0, first="-")])
        self.assertIn("FLAGS 1", out)
        self.assertIn("  K L1 cmd=4 m=0 " + K_MSG, out)


# Synthetic BANNER lines in the grammar of the fmtBanner goldens (queue_hud_logic_test.cpp, BANNER block near line 2448).
# The values are not copied from that block.
BANNER_GOLDEN = [
    "BANNER act=hide soft=1 ctl=8 wr=8",
    "BANNER act=restore soft=0 ctl=0 wr=0",
    "BANNER act=forget soft=1 ctl=2 wr=5",
    "BANNER act=reset soft=0 ctl=12 wr=3",
]


class BannerTraceTests(unittest.TestCase):
    def test_banner_golden_lines_parse_exactly_and_malformed_act_is_skipped(self):
        recs, skipped = parse(BANNER_GOLDEN)
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs], ["BANNER"] * 4)
        self.assertEqual([r["f"]["act"] for r in recs], ["hide", "restore", "forget", "reset"])
        self.assertEqual((recs[0]["f"]["soft"], recs[0]["f"]["ctl"], recs[0]["f"]["wr"]), ("1", "8", "8"))
        self.assertEqual((recs[0]["cmd"], recs[0]["m"]), (None, None))
        recs, skipped = parse(["BANNER act=bogus soft=1 ctl=8 wr=8"])
        self.assertEqual((recs, len(skipped)), ([], 1))


class ModeParseTests(unittest.TestCase):
    def test_mode_goldens_parse_exactly(self):
        recs, skipped = parse(MODE_GOLDEN)
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs], ["MODE"] * 5)
        self.assertEqual([r["m"] for r in recs], [0, 2, 1, None, 3])
        self.assertEqual((recs[0]["f"]["soft"], recs[0]["f"]["hostile"], recs[0]["f"]["why"]), ("-", "?", "ui"))
        self.assertEqual((recs[2]["f"]["strip"], recs[2]["f"]["banner"]), ("hidden:combat", "restored"))
        self.assertEqual((recs[3]["f"]["combat"], recs[3]["f"]["cancel"]), ("?", "hidden:off"))
        self.assertEqual((recs[4]["f"]["strip"], recs[4]["f"]["why"]), ("?", "?"))

    def test_mode_cap_line_is_counted_not_skipped(self):
        recs, skipped = parse(["MODE cap reached"])
        self.assertEqual((skipped, [r["kw"] for r in recs]), ([], ["MODE_CAP"]))

    def test_mode_malformed_values_and_missing_field_are_skipped(self):
        bad = [
            mode(strip="bogus"), mode(soft="2"), mode(why="nope"), mode(banner="shown"), mode(cancel="hidden:bogus"),
            "MODE m=0 combat=1 soft=0 menu=0 paused=0 hostile=0 party=0 strip=shown cancel=shown banner=hidden",  # no why
            "MODE m=0 combat=2 soft=0 menu=0 paused=0 hostile=0 party=0 strip=shown cancel=shown banner=hidden why=ui",
        ]
        recs, skipped = parse(bad + [mode(0, 0)])
        self.assertEqual(len(skipped), len(bad))
        self.assertEqual([r["kw"] for r in recs], ["MODE"])

    def test_startup_line_parses_as_config_and_bad_value_is_ignored(self):
        recs, skipped = parse([config(1)])
        self.assertEqual((skipped, [(r["kw"], r["f"]["show_in_combat"]) for r in recs]), ([], [("CONFIG", "1")]))
        self.assertEqual(parse([config(2)]), ([], []))


class CancelParseTests(unittest.TestCase):
    def test_new_cancel_forms_parse_and_are_not_skipped(self):
        recs, skipped = parse([cancel_new(0, "click", 2), cancel_new(0, "done", 0),
                               cancel_new(1, "refused", 3, " why=no server creature"),
                               cancel_new(0, "click", 2, " route=ServerClearAll")])
        self.assertEqual(skipped, [])
        self.assertEqual([r["kw"] for r in recs], ["CANCEL"] * 4)
        self.assertEqual([r["m"] for r in recs], [0, 0, 1, 0])
        self.assertEqual(recs[2]["f"]["why"], "no server creature")
        self.assertEqual(recs[3]["f"]["route"], "ServerClearAll")
        self.assertIsNone(recs[0]["cmd"])

    def test_bad_cancel_forms_are_skipped(self):
        bad = ["CANCEL m=0 act=maybe n=2", "CANCEL m=0 act=click", "CANCEL m=0 n=2"]
        recs, skipped = parse(bad)
        self.assertEqual(recs, [])
        self.assertEqual([s[2] for s in skipped], ["malformed act=maybe", "missing n", "missing cmd"])

    def test_legacy_cancel_cmd_form_still_parses(self):
        recs, skipped = parse(["CANCEL cmd=5 m=0", "CANCEL cmd=6"])
        self.assertEqual(skipped, [])
        self.assertEqual([(r["cmd"], r["m"]) for r in recs], [(5, 0), (6, None)])

    def test_new_cancel_lines_open_no_lifecycle_and_are_not_skipped_in_view(self):
        rc, out = run_log([cancel_new(0, "click", 2)])
        self.assertEqual(out[0], "QUEUE-HUD lines=1 parsed=1 skipped=0 cmds=0 qj=0")
        self.assertIn("COMMANDS 0", out)


class ModeSectionTests(unittest.TestCase):
    def test_modes_rows_title_and_none(self):
        rc, out = run_log([mode(0, 0), mode(1, 1, soft="0", strip="hidden:combat", cancel="hidden:combat",
                                            banner="restored", why="target")])
        self.assertEqual(rc, 0)
        i = out.index("MODES 2")
        self.assertEqual(out[i + 1], "  L1 T=1000 m=0 combat=0 soft=- strip=shown cancel=shown banner=vanilla why=ui")
        self.assertEqual(out[i + 2], "  L2 T=1001 m=1 combat=1 soft=0 strip=hidden:combat cancel=hidden:combat "
                                     "banner=restored why=target")
        self.assertIn("FLAGS 0", out)
        self.assertIn("MODES 0", run_log([cmd(1)])[1])

    def test_modes_capped_at_12_rows_then_more(self):
        rc, out = run_log([mode(m, 0) for m in range(13)])
        i = out.index("MODES 13")
        rows = out[i + 1:i + 13]
        self.assertEqual(len(rows), 12)
        self.assertTrue(all(row.startswith("  L") for row in rows))
        self.assertEqual(rows[-1].split()[1:3], ["T=1011", "m=11"])
        self.assertEqual(out[i + 13], "  (+1 more)")

    def test_modes_cap_line_counted_in_title_and_not_skipped(self):
        rc, out = run_log([mode(0, 0), "MODE cap reached"])
        self.assertEqual(out[0], "QUEUE-HUD lines=2 parsed=2 skipped=0 cmds=0 qj=0")
        self.assertIn("MODES 1 (cap reached x1)", out)

    def test_modes_respects_member_filter(self):
        rc, out = run_log([mode(0, 0), mode(1, 0), mode(1, 0, strip="hidden:combat")], ["--member", "1"])
        i = out.index("MODES 2")
        self.assertTrue(all(" m=1 " in row for row in out[i + 1:i + 3]))

    def test_modes_unknown_member_and_bare_line_has_no_time(self):
        rc, out = run_raw([mode("?", 0)])
        self.assertIn("MODES 1", out)
        self.assertEqual(out[out.index("MODES 1") + 1],
                         "  L1 T=- m=? combat=0 soft=- strip=shown cancel=shown banner=vanilla why=ui")


class ModeFlagTests(unittest.TestCase):
    QUIET_SOFT_DROP = mode(0, 1, soft="0", strip="hidden:combat", cancel="hidden:combat", banner="hidden", why="ended")

    # L: soft 1->0 in combat, last BANNER was hide, no BANNER restore within 500 ms after the MODE
    def test_L_fires_when_soft_drops_in_combat_with_banner_hidden_and_no_restore(self):
        self.assertEqual(all_flags_of([at(900, mode(0, 1, soft="1")), at(1000, banner_act("hide")),
                                       at(1100, self.QUIET_SOFT_DROP)]),
                         [(3, "L L3 m=0 soft 1->0 in combat, banner hidden by L2, no restore within 500 ms")])

    def test_L_quiet_when_restore_lands_at_400_ms(self):
        self.assertEqual(all_flags_of([at(900, mode(0, 1, soft="1")), at(1000, banner_act("hide")),
                                       at(1100, self.QUIET_SOFT_DROP), at(1500, banner_act("restore"))]), [])

    def test_L_quiet_when_restore_lands_at_exactly_500_ms(self):
        self.assertEqual(all_flags_of([at(900, mode(0, 1, soft="1")), at(1000, banner_act("hide")),
                                       at(1100, self.QUIET_SOFT_DROP), at(1600, banner_act("restore"))]), [])

    def test_L_fires_when_restore_comes_after_500_ms(self):
        f = all_flags_of([at(900, mode(0, 1, soft="1")), at(1000, banner_act("hide")),
                          at(1100, self.QUIET_SOFT_DROP), at(1700, banner_act("restore"))])
        self.assertEqual([line for line, _ in f], [3])

    def test_L_quiet_without_its_preconditions(self):
        cases = {
            "soft never 1 before": [at(1000, banner_act("hide")), at(1100, self.QUIET_SOFT_DROP)],
            "last banner not hide": [at(900, mode(0, 1, soft="1")), at(1000, banner_act("forget")),
                                     at(1100, self.QUIET_SOFT_DROP)],
            "restore between hide and mode": [at(900, mode(0, 1, soft="1")), at(1000, banner_act("hide")),
                                              at(1050, banner_act("restore")), at(1100, self.QUIET_SOFT_DROP)],
            "drop outside combat": [at(900, mode(0, 1, soft="1")), at(1000, banner_act("hide")),
                                    at(1100, mode(0, 0, soft="0", strip="hidden:combat", cancel="hidden:combat"))],
        }
        for name, bodies in cases.items():
            with self.subTest(case=name):
                self.assertEqual([text for _, text in all_flags_of(bodies) if text.startswith("L ")], [])

    def test_L_bare_lines_are_not_judged(self):
        self.assertEqual(all_flags_of([mode(0, 1, soft="1"), banner_act("hide"), self.QUIET_SOFT_DROP]), [])

    def test_L_fires_under_member_filter_because_banner_context_is_kept(self):
        rc, out = run_raw([at(900, mode(0, 1, soft="1")), at(1000, banner_act("hide")), at(1100, self.QUIET_SOFT_DROP)],
                          ["--member", "0"])
        self.assertIn("FLAGS 1", out)
        self.assertIn("  L L3 m=0 soft 1->0 in combat, banner hidden by L2, no restore within 500 ms", out)

    # M: strip=shown in combat with soft=0, unless the startup line says show_in_combat=1
    def test_M_quiet_without_startup_line_mod_default_is_1(self):
        self.assertEqual(all_flags_of([mode(0, 1, soft="0", strip="shown", cancel="hidden:combat")]), [])

    def test_M_fires_with_startup_show_in_combat_0(self):
        self.assertEqual(all_flags_of([config(0), mode(0, 1, soft="0", strip="shown", cancel="hidden:combat")]),
                         [(2, "M L2 m=0 strip=shown with combat=1 soft=0 (show_in_combat=0)")])

    def test_M_suppressed_by_startup_show_in_combat_1(self):
        self.assertEqual(all_flags_of([config(1), mode(0, 1, soft="0", strip="shown", cancel="hidden:combat")]), [])

    def test_M_uses_the_latest_startup_line(self):
        self.assertEqual(all_flags_of([config(1), config(0), mode(0, 1, soft="0", strip="shown", cancel="hidden:combat")]),
                         [(3, "M L3 m=0 strip=shown with combat=1 soft=0 (show_in_combat=0)")])

    def test_M_quiet_outside_combat_soft_0_or_strip_hidden(self):
        for body in [mode(0, 0, soft="-"), mode(0, 1, soft="1"), mode(0, 1, soft="-"),
                     mode(0, 1, soft="0", strip="hidden:combat", cancel="hidden:combat")]:
            with self.subTest(body=body):
                self.assertEqual(all_flags_of([body]), [])

    def test_M_under_member_filter_still_sees_startup_line(self):
        rc, out = run_log([config(1), mode(0, 1, soft="0", strip="shown", cancel="hidden:combat")], ["--member", "0"])
        self.assertIn("FLAGS 0", out)

    # N: cancel=shown in combat with soft=0 (the startup line does not suppress it)
    def test_N_fires_on_cancel_shown_in_combat_soft_0(self):
        self.assertEqual(all_flags_of([mode(0, 1, soft="0", strip="hidden:combat", cancel="shown")]),
                         [(1, "N L1 m=0 cancel=shown with combat=1 soft=0")])

    def test_N_not_suppressed_by_startup_line_and_quiet_otherwise(self):
        self.assertEqual(len(all_flags_of([config(1), mode(0, 1, soft="0", strip="hidden:combat", cancel="shown")])), 1)
        for body in [mode(0, 0, soft="-"), mode(0, 1, soft="1"), mode(0, 1, soft="0", strip="hidden:combat", cancel="hidden:off")]:
            with self.subTest(body=body):
                self.assertEqual(all_flags_of([body]), [])

    def test_M_and_N_listed_in_flags_section(self):
        rc, out = run_log([config(0), mode(0, 1, soft="0", strip="shown", cancel="shown")])
        self.assertIn("FLAGS 2", out)
        self.assertIn("  M L2 m=0 strip=shown with combat=1 soft=0 (show_in_combat=0)", out)
        self.assertIn("  N L2 m=0 cancel=shown with combat=1 soft=0", out)


if __name__ == "__main__":
    unittest.main()
