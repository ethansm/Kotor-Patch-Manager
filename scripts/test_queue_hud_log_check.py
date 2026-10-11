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


def reinject(n, m=0, why=None):
    return _kw_line("REINJECT", n, m, why)


def dispatch(n, m=0, why=None):
    return _kw_line("DISPATCH", n, m, why)


def cancel(n, m=0):
    return _kw_line("CANCEL", n, m)


REINJECT_NO_WHYS = ["ok", "inactive", "pending", "cancel", "max_reinject", "in_queue", "no_leader"]
EDGE_WHATS = ["combat", "paused", "menu", "party_table", "gather_m2", "creature", "journal_cap"]


def reinject_no(n, m=0, why="pending"):
    return "REINJECT_NO cmd=%d m=%d why=%s" % (n, m, why)


def parse(bodies):
    return Q.parse_lines([(i + 1, s) for i, s in enumerate(bodies)])


def flags_of(bodies):
    recs, _ = parse(bodies)
    return Q.qj_flags(recs)


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
        bodies += [edge() for _ in range(100)]
        for _ in range(50):
            bodies += [qj(0, "[3d]", 1), qj(0, "[c]", 1)]
        rc, out = run_log(bodies)
        self.assertEqual(rc, 0)
        self.assertLess(len(out), 60)
        self.assertIn("COMMANDS 100", out)
        self.assertIn("  ... +82 more", out)
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


if __name__ == "__main__":
    unittest.main()
