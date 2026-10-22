#!/usr/bin/env python3
"""Tests for scripts/holster_log_check.py and the holster route in scripts/kotor2_logs.py. Synthetic fixtures only.
Run: python3 scripts/test_holster_log_check.py   (run_tests.py discovers it as py:scripts/test_holster_log_check.py)"""
import contextlib
import io
import json
import math
import os
import subprocess
import sys
import tempfile
import unittest
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import holster_log_check as H  # noqa: E402
import kotor2_logs as K  # noqa: E402

LOADED = ("holster 0.1.0-dev loaded (P0 probe, log-only): probe=0 channels=comb,attach,swap,anim,gob "
          "max_lines=60000 gob_every=50 table=ok")
BONE = "BONETEST patched rhand -> p_rhand"
CAP = "cap reached max_lines=60000: further lines dropped"
STATUS = "status: loaded=yes 0.1.0-dev | cap reached=yes max_lines=60000 | BONETEST=patched x1 | TID gobupdate=absent place=absent | RELOAD 0"


def stamped(body, n=0):
    return "T=%d f=- holster/main %s" % (1000 + n, body)


def comb(creature, old, new, edge, tick=1000):
    return "COMB creature=%x old=%d new=%d edge=%d tick=%d" % (creature, old, new, edge, tick)


def attach(tag, creature, body, hr, hl, obj=0xABC, tick=1300):
    return "ATTACH obj=%x tag='%s' creature=%x body=%x handleR=%x handleL=%x tick=%d" % (
        obj, tag, creature, body, hr, hl, tick)


def swap(tick=1400):
    return "SWAP exec=401000 " + " ".join("d%d=%x" % (i, i * 16) for i in range(8)) + " tick=%d" % tick


def anim_res(self_, id_hex, tick=1500):
    return "ANIMRES self=%x id=%s style=0 gate=1 cc=2 d4=3 e8=4 tick=%d" % (self_, id_hex, tick)


def anim_send(creature, id_hex, tick=1600):
    return "ANIMSEND creature=%x id=%s tick=%d" % (creature, id_hex, tick)


def gob_line(n, gob, parent, pos, quat):
    return "GOB n=%d gob=%x parent18=%x pos=(%.3f,%.3f,%.3f) quat=(%.3f,%.3f,%.3f,%.3f)" % (n, gob, parent, *pos, *quat)


def place(seq, gob, gobvt, pos0, quat0, pos1, quat1, node=0x77, beh=0x88):
    return ("PLACE n=%d gob=%x node=%x beh=%x gobvt=%x pos0=(%.3f,%.3f,%.3f) quat0=(%.3f,%.3f,%.3f,%.3f) "
            "pos1=(%.3f,%.3f,%.3f) quat1=(%.3f,%.3f,%.3f,%.3f)") % (seq, gob, node, beh, gobvt, *pos0, *quat0, *pos1, *quat1)


PLACE_OK_A = place(1, 0x9200, 0x1f, (0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0), (0.1, 0.0, 0.0), (0.985, 0.0, 0.0, 0.174))
PLACE_BAD = place(2, 0x9300, 0x1f, (1.0, 1.0, 1.0), (1.0, 0.0, 0.0, 0.0), (1.0, 1.0, 1.5), (1.0, 0.0, 0.0, 0.100))   # norm 1.005
PLACE_OK_B = place(3, 0x9200, 0x7, (0.1, 0.0, 0.0), (0.985, 0.0, 0.0, 0.174), (0.2, 0.0, 0.0), (0.985, 0.0, 0.0, 0.174))


def fixture_bodies():
    """Every line kind, stamped and bare mixed. Gob 0x9000 turns 20 deg about z; its third sample is the same rotation
    with the sign flipped. Gob 0x9100 is constant except z. PLACE: gob 0x9200 twice (last one is the shown one), gob 0x9300
    once with a non-unit quat1 that must be flagged."""
    bodies = [
        LOADED, BONE,
        comb(0x2A1, 0, 1, 1), comb(0x1F0, 0, 1, 1), comb(0x2A1, 1, 0, 2), comb(0x1F0, 1, 0, 2), comb(0x2A1, 0, 1, 3),
        attach("hand_r", 0x1F0, 0x5, 0x9, 0x0), attach("hand_r", 0x1F0, 0x5, 0x9, 0x0, tick=1310),
        attach("hand_l", 0x1F0, 0x5, 0x9, 0xA, tick=1320), attach("hand_r", 0x2A1, 0x7, 0x0, 0x0, obj=0xABE, tick=1330),
        swap(1400), swap(1410), swap(1420),
        anim_res(0x10, "1f"), anim_res(0x10, "001F"), anim_res(0x11, "7"), anim_res(0x11, "1f"),
        anim_send(0x2A1, "1f"), anim_send(0x2A1, "1f"),
        gob_line(25, 0x9000, 0x1234, (0.0, 1.0, 2.0), (1.0, 0.0, 0.0, 0.0)),
        gob_line(25, 0x9100, 0x1234, (5.0, 5.0, 5.0), (1.0, 0.0, 0.0, 0.0)),
        gob_line(50, 0x9000, 0x1234, (0.1, 1.0, 2.2), (0.985, 0.0, 0.0, 0.174)),
        gob_line(50, 0x9100, 0x1234, (5.0, 5.0, 5.5), (1.0, 0.0, 0.0, 0.0)),
        gob_line(75, 0x9000, 0x1234, (0.2, 1.0, 2.4), (-0.985, 0.0, 0.0, -0.174)),
        PLACE_OK_A, PLACE_BAD, PLACE_OK_B,
        CAP,
    ]
    return bodies


def fixture():
    return [stamped(b, i) if i % 3 == 0 else b for i, b in enumerate(fixture_bodies())]


def worst_case():
    """Every section over its cap, plus one unparsable COMB line. The report must stay at 47 lines (40 + 3 GOBREUSE/FLOURISH/SLIDE lines + 3 PARTYHOLD/MOVEHOLD/DELAY lines + 1 REACH line)."""
    bodies = [LOADED, BONE]
    bodies += [comb(0x100 + c, 0, 1, 1) for c in range(6)]                       # 6 creatures, 4 shown
    for c in range(5):
        for hr in range(1, 5):
            bodies.append(attach("t", 0x200 + c, 0x5, hr, 0))                    # 5 creatures x 4 tuples, 3 creatures shown
    bodies += [swap(1)]
    bodies += [anim_res(0x1, "%x" % (0x10 + i)) for i in range(12)]              # 12 ids, top 10
    bodies += [anim_send(0x1, "1f")]
    for g in range(10):
        bodies += [gob_line(k + 1, 0xA00 + g, 0x1, (0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0)) for k in range(g + 1)]
    bodies += ["COMB creature=zz old=0 new=1 edge=1 tick=1"]                    # skipped
    bodies += [CAP]
    return bodies


def place_worst():
    """31 parsed PLACE lines over 6 gobs and 12 gobvt values (line 31 has quat1 norm 1.118), plus one unparsable PLACE."""
    bodies = [place(i + 1, 0x9400 + i % 6, 0x10 + i % 12, (0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0),
                    (float(i), 0.0, 0.0), (1.0, 0.0, 0.0, 0.0)) for i in range(30)]
    bodies.append(place(31, 0x9400, 0x10, (0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0), (0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.5)))
    bodies.append("PLACE n=32 gob=9400 node=77 beh=88 gobvt=10 pos0=(0.000,0.000,0.000)")   # missing quat0/pos1/quat1
    return bodies


def parse(bodies):
    return H.parse_lines([(i + 1, s) for i, s in enumerate(bodies)])


def run(argv):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = H.main(argv)
    return rc, buf.getvalue().splitlines()


def run_log(bodies, extra=()):
    """Write bodies as a log in a temp dir and run the analyzer (bookmark file also temp)."""
    with tempfile.TemporaryDirectory() as d:
        log = os.path.join(d, "holster_log.txt")
        Path(log).write_text("\n".join(bodies) + "\n")
        return run([log, "--state", os.path.join(d, "bm.json"), *extra])


class ParseTests(unittest.TestCase):
    def test_every_kind_parses_with_no_skips(self):
        recs, skipped, main = parse(fixture())
        self.assertEqual(skipped, [])
        self.assertEqual(Counter(r["kind"] for r in recs),
                         Counter({"COMB": 5, "ATTACH": 4, "SWAP": 3, "ANIMRES": 4, "ANIMSEND": 2, "GOB": 5, "PLACE": 3}))
        self.assertEqual(main["loaded"], ["0.1.0-dev"])
        self.assertEqual(main["cap"], ["60000"])
        self.assertEqual(main["bonetest"], Counter({"patched": 1}))

    def test_stamp_is_optional(self):
        a = parse([comb(0x1F0, 0, 1, 1)])[0][0]
        b = parse([stamped(comb(0x1F0, 0, 1, 1))])[0][0]
        self.assertEqual({k: v for k, v in a.items() if k != "line"}, {k: v for k, v in b.items() if k != "line"})

    def test_unreadable_comb_is_counted_not_skipped(self):
        lines = ["COMB creature=zz old=0 new=1 edge=1 tick=5",
                 "COMB creature=1f0 old=? new=2 edge=? tick=7",
                 "COMB creature=1f0 old=? new=3 edge=? tick=8",
                 comb(0x1F0, 0, 1, 1)]
        recs, skipped, main = parse(lines)
        self.assertEqual(main["unreadable"], 2)
        self.assertEqual([(k, t) for _, k, t in skipped], [("COMB", lines[0])])
        self.assertEqual(len(recs), 1)
        rc, out = run_log(lines)
        self.assertEqual(rc, 0)
        self.assertIn("COMB edges=1 creatures=1 unreadable=2", out)
        self.assertTrue(any(l.startswith("HOLSTER lines=4 parsed=1 skipped=1") for l in out), out)

    def test_unreadable_comb_accepts_stamp_and_header_only_log(self):
        rc, out = run_log([stamped("COMB creature=2a old=? new=0 edge=? tick=9")])
        self.assertEqual(rc, 0)
        self.assertIn("COMB edges=0 creatures=0 unreadable=1", out)
        self.assertTrue(any(l.startswith("HOLSTER lines=1 parsed=0 skipped=0") for l in out), out)

    def test_no_unreadable_leaves_comb_header_unchanged(self):
        rc, out = run_log([comb(0x1F0, 0, 1, 1)])
        self.assertEqual(rc, 0)
        self.assertIn("COMB edges=1 creatures=1", out)
        self.assertFalse(any("unreadable" in l for l in out), out)

    def test_half_unknown_comb_stays_skipped(self):
        # old=? with a known edge (or new=?) is not what the emitter prints: it must not be hidden as unreadable.
        bad = ["COMB creature=1f0 old=? new=2 edge=7 tick=1100", "COMB creature=1f0 old=? new=? edge=? tick=1101"]
        recs, skipped, main = parse(bad)
        self.assertEqual(recs, [])
        self.assertEqual(main["unreadable"], 0)
        self.assertEqual([k for _, k, _ in skipped], ["COMB", "COMB"])

    def test_hex_is_normalised(self):
        recs, _, _ = parse(["ANIMRES self=1 id=001F style=0 gate=1 cc=2 d4=3 e8=4 tick=9"])
        self.assertEqual(recs[0]["id"], "1f")

    def test_known_keyword_with_bad_fields_is_skipped_not_fatal(self):
        bad = ["COMB creature=zz old=0 new=1 edge=1 tick=5",
               "GOB n=1 gob=9000 parent18=1234 pos=(1.0,2.0) quat=(1,0,0,0)"]
        recs, skipped, _ = parse(bad)
        self.assertEqual(recs, [])
        self.assertEqual([(s[0], s[1]) for s in skipped], [(1, "COMB"), (2, "GOB")])

    def test_unrelated_lines_are_ignored(self):
        recs, skipped, _ = parse(["hud built panel=1", "IDENT_SWAP from=0 to=2", "SWAPPED later", "ATTACHED nothing"])
        self.assertEqual((recs, skipped), ([], []))

    def test_mean_angle_of_sign_aligned_quats(self):
        # identity, 20 deg about z, and the same rotation with the sign flipped: aligned mean is a 13.366 deg turn
        angle = H._quat_angle_deg([(1.0, 0.0, 0.0, 0.0), (0.985, 0.0, 0.0, 0.174), (-0.985, 0.0, 0.0, -0.174)])
        self.assertAlmostEqual(angle, 13.366, delta=0.005)

    def test_mean_angle_identity_and_degenerate(self):
        self.assertAlmostEqual(H._quat_angle_deg([(1.0, 0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0)]), 0.0)
        self.assertIsNone(H._quat_angle_deg([(0.0, 0.0, 0.0, 0.0)]))


class ReportTests(unittest.TestCase):
    def setUp(self):
        self.rc, self.out = run_log(fixture())

    def test_header_and_status(self):
        self.assertEqual(self.rc, 0)
        self.assertEqual(self.out[0], "HOLSTER lines=29 parsed=26 skipped=0")
        self.assertEqual(self.out[1], STATUS)

    def test_status_when_absent(self):
        rc, out = run_log(["hud built panel=1"])
        self.assertEqual(out[1], "status: loaded=no | cap reached=no | BONETEST=absent | TID gobupdate=absent place=absent | RELOAD 0")

    def test_comb_edges_per_creature(self):
        self.assertIn("COMB edges=5 creatures=2", self.out)
        self.assertIn("  creature=0x2a1 edges=3 last new=1", self.out)
        self.assertIn("  creature=0x1f0 edges=2 last new=0", self.out)
        self.assertLess(self.out.index("  creature=0x2a1 edges=3 last new=1"),
                        self.out.index("  creature=0x1f0 edges=2 last new=0"))

    def test_attach_tags_and_tuples_per_creature(self):
        self.assertIn("ATTACH n=4 creatures=2 tags: 'hand_r'=3 'hand_l'=1", self.out)
        self.assertIn("  creature=0x1f0 distinct=2 (body/handleR/handleL): 5/9/0 x2; 5/9/a x1", self.out)
        self.assertIn("  creature=0x2a1 distinct=1 (body/handleR/handleL): 7/0/0 x1", self.out)

    def test_swap_total(self):
        self.assertIn("SWAP total=3", self.out)

    def test_anim_histograms(self):
        self.assertIn("ANIMRES n=4 distinct=2 top10: 0x1f x3, 0x7 x1 | style per creature (self style/gate): 10 0/1 x2, 11 0/1 x2", self.out)
        self.assertIn("ANIMSEND n=2 distinct=1 top10: 0x1f x2", self.out)

    def test_gob_stats_for_changing_and_still_gobs(self):
        self.assertIn("GOB lines=5 gobs=2", self.out)
        self.assertIn("  gob=0x9000 samples=3 parent18=0x1234 sign_flips=1 rot_mean=13.37 deg", self.out)
        self.assertIn("    pos x 0.000..0.200 mean 0.100 | y 1.000..1.000 mean 1.000 | z 2.000..2.400 mean 2.200", self.out)
        self.assertIn("    quat w -0.985..1.000 | x 0.000..0.000 | y 0.000..0.000 | z -0.174..0.174", self.out)
        self.assertIn("  gob=0x9100 samples=2 parent18=0x1234 sign_flips=0 rot_mean=0.00 deg", self.out)
        self.assertIn("    pos x 5.000..5.000 mean 5.000 | y 5.000..5.000 mean 5.000 | z 5.000..5.500 mean 5.250", self.out)
        self.assertIn("    quat w 1.000..1.000 | x 0.000..0.000 | y 0.000..0.000 | z 0.000..0.000", self.out)
        self.assertLess(self.out.index("  gob=0x9000 samples=3 parent18=0x1234 sign_flips=1 rot_mean=13.37 deg"),
                        self.out.index("  gob=0x9100 samples=2 parent18=0x1234 sign_flips=0 rot_mean=0.00 deg"))

    def test_no_raw_example_when_nothing_skipped(self):
        self.assertFalse(any(s.startswith("skipped e.g.") for s in self.out))

    def test_gob_view_keeps_only_top_eight_by_samples(self):
        bodies = []
        for g in range(10):
            bodies += [gob_line(k + 1, 0xA00 + g, 0x1, (0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0)) for k in range(g + 1)]
        rc, out = run_log(bodies)
        self.assertIn("GOB lines=55 gobs=10 (top 8 by samples shown)", out)
        self.assertIn("  gob=0xa09 samples=10 parent18=0x1 sign_flips=0 rot_mean=0.00 deg", out)
        self.assertFalse(any(s.startswith("  gob=0xa00 ") or s.startswith("  gob=0xa01 ") for s in out))

    def test_worst_case_is_capped_at_47_lines(self):
        rc, out = run_log(worst_case())
        self.assertEqual(rc, 0)
        self.assertEqual(len(out), 47)
        self.assertIn("COMB edges=6 creatures=6 (top 4 shown)", out)
        self.assertIn("ATTACH n=20 creatures=5 tags: 't'=20 (first 3 creatures shown)", out)
        self.assertTrue(any(s.startswith("ANIMRES n=12 distinct=12 top10:") and "(+2 more ids) | style per creature" in s for s in out))
        self.assertIn("GOB lines=55 gobs=10 (top 8 by samples shown)", out)
        self.assertTrue(out[-1].startswith("skipped e.g. L") and "COMB creature=zz" in out[-1])

    def test_missing_log_returns_2(self):
        with tempfile.TemporaryDirectory() as d:
            missing = os.path.join(d, "holster_log.txt")
            rc, out = run([missing])
        self.assertEqual((rc, out), (2, ["no log: %s" % missing]))


class PlaceTests(unittest.TestCase):
    def setUp(self):
        self.rc, self.out = run_log(fixture_bodies())
        self.bad_line = fixture_bodies().index(PLACE_BAD) + 1

    def test_place_records_parse_with_fields(self):
        recs, skipped, _ = parse(fixture_bodies())
        places = [r for r in recs if r["kind"] == "PLACE"]
        self.assertEqual(skipped, [])
        self.assertEqual([(r["seq"], r["gob"], r["gobvt"]) for r in places],
                         [(1, "9200", "1f"), (2, "9300", "1f"), (3, "9200", "7")])
        self.assertEqual((places[0]["node"], places[0]["beh"]), ("77", "88"))
        self.assertEqual(places[1]["quat1"], [1.0, 0.0, 0.0, 0.1])

    def test_place_counts_gobvt_and_shows_last_sample_per_gob(self):
        self.assertIn("PLACE lines=3 gobs=2 gobvt distinct=2: 0x1f x2, 0x7 x1", self.out)
        self.assertIn("  gob=0x9200 samples=2 last pos0=(0.100,0.000,0.000) quat0=(0.985,0.000,0.000,0.174)", self.out)
        self.assertIn("    -> pos1=(0.200,0.000,0.000) quat1=(0.985,0.000,0.000,0.174)", self.out)
        self.assertIn("  gob=0x9300 samples=1 last pos0=(1.000,1.000,1.000) quat0=(1.000,0.000,0.000,0.000)", self.out)
        self.assertIn("    -> pos1=(1.000,1.000,1.500) quat1=(1.000,0.000,0.000,0.100)", self.out)
        self.assertLess(self.out.index("  gob=0x9200 samples=2 last pos0=(0.100,0.000,0.000) quat0=(0.985,0.000,0.000,0.174)"),
                        self.out.index("  gob=0x9300 samples=1 last pos0=(1.000,1.000,1.000) quat0=(1.000,0.000,0.000,0.000)"))

    def test_non_unit_quat1_is_flagged_with_count_and_line(self):
        problems = [s for s in self.out if s.startswith("PROBLEM ")]
        self.assertEqual(problems, ["PROBLEM PLACE quat1 |norm-1|>0.002: count=1 worst=0.0050 e.g. L%d gob=0x9300"
                                    % self.bad_line])
        self.assertFalse(any(s.startswith("PLACE quat1 unit-norm ok") for s in self.out))

    def test_unit_quat1_lines_are_not_flagged(self):
        rc, out = run_log([PLACE_OK_A, PLACE_OK_B])
        self.assertIn("PLACE quat1 unit-norm ok: 2 lines within 0.002", out)
        self.assertFalse(any(s.startswith("PROBLEM ") for s in out))

    def test_unparsable_place_line_is_skipped_not_fatal(self):
        bad = "PLACE n=1 gob=9200 node=77 beh=88 gobvt=1f pos0=(0.000,0.000,0.000) quat0=(1.000,0.000,0.000,0.000)"
        recs, skipped, _ = parse([bad])
        self.assertEqual(recs, [])
        self.assertEqual([(s[0], s[1]) for s in skipped], [(1, "PLACE")])

    def test_unrelated_place_words_are_ignored(self):
        self.assertEqual(parse(["PLACED ok", "PLACEHOLDER n=1"])[:2], ([], []))

    def test_place_view_caps_gobs_and_report_stays_within_47_lines(self):
        rc, out = run_log(worst_case()[:-1] + place_worst() + [CAP])
        self.assertEqual(rc, 0)
        self.assertLessEqual(len(out), 47)
        self.assertIn("GOB lines=55 gobs=10 (top 4 by samples shown)", out)
        self.assertIn("PLACE lines=31 gobs=6 (top 4 by samples shown) gobvt distinct=12: 0x10 x4, 0x11 x3, 0x12 x3, "
                      "0x13 x3, 0x14 x3, 0x15 x3, 0x16 x2, 0x17 x2, 0x18 x2, 0x19 x2 (+2 more)", out)
        self.assertTrue(any(s.startswith("PROBLEM PLACE quat1 |norm-1|>0.002: count=1 worst=0.1180") and "gob=0x9400" in s
                            for s in out))
        self.assertEqual(len([s for s in out if s.startswith("  gob=0x94")]), 4)
        self.assertTrue(out[-1].startswith("skipped e.g. L") and "COMB creature=zz" in out[-1])

    def test_logs_without_place_lines_have_no_place_section(self):
        rc, out = run_log(worst_case())
        self.assertFalse(any(s.startswith(("PLACE", "PROBLEM")) for s in out))
        self.assertIn("GOB lines=55 gobs=10 (top 8 by samples shown)", out)


class NewWindowTests(unittest.TestCase):
    def test_new_reads_only_appended_lines(self):
        with tempfile.TemporaryDirectory() as d:
            log = os.path.join(d, "holster_log.txt")
            state = os.path.join(d, "bm.json")
            Path(log).write_text("\n".join([LOADED, swap(1), comb(0x1F0, 0, 1, 1)]) + "\n")
            rc, out = run([log, "--new", "--state", state])
            self.assertIn("new=L1-L3", out[0])
            self.assertIn("SWAP total=1", out)
            with open(log, "a") as f:
                f.write("\n".join([swap(2), swap(3)]) + "\n")
            rc, out = run([log, "--new", "--state", state])
            self.assertIn("new=L4-L5", out[0])
            self.assertIn("SWAP total=2", out)
            self.assertIn("COMB edges=0 creatures=0", out)
            key = str(Path(log).resolve())
            self.assertEqual(json.loads(Path(state).read_text())[key]["line"], 5)
            rc, out = run([log, "--new", "--state", state])
            self.assertIn("new=none", out[0])
            self.assertIn("SWAP total=0", out)

    def test_no_advance_keeps_bookmark(self):
        with tempfile.TemporaryDirectory() as d:
            log = os.path.join(d, "holster_log.txt")
            state = os.path.join(d, "bm.json")
            Path(log).write_text(swap(1) + "\n" + swap(2) + "\n")
            for _ in range(2):
                rc, out = run([log, "--new", "--no-advance", "--state", state])
                self.assertIn("new=L1-L2", out[0])
            self.assertFalse(os.path.exists(state))


class RouteTests(unittest.TestCase):
    def test_route_table_and_alias(self):
        self.assertEqual(K.ROUTES["holster_log.txt"], ("holster_log_check", "holster"))
        self.assertEqual(K.ALIASES["holster"], "holster_log.txt")

    def test_route_argv_and_kind(self):
        with tempfile.TemporaryDirectory() as d:
            argv = K.route("holster_log.txt", d)
            self.assertTrue(argv[1].endswith(os.path.join("scripts", "holster_log_check.py")))
            self.assertEqual(argv[2], os.path.join(d, "holster_log.txt"))
            self.assertEqual(K.kind_of(os.path.join(d, "holster_log.txt")), "holster")

    def test_alias_check_missing_file(self):
        with tempfile.TemporaryDirectory() as d:
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                rc = K.check("holster", [], d)
        self.assertEqual((rc, buf.getvalue().strip()), (2, "missing: holster"))

    def test_routed_argv_prints_analysis(self):
        # Same argv the check subcommand builds (route + extra args), with a temp bookmark so the real cache stays untouched.
        with tempfile.TemporaryDirectory() as d:
            Path(d, "holster_log.txt").write_text(stamped(swap(1)) + "\n" + comb(0x1F0, 0, 1, 1) + "\n")
            argv = K.route(K.ALIASES["holster"], d) + ["--new", "--state", os.path.join(d, "bm.json")]
            p = subprocess.run(argv, capture_output=True, text=True)
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertIn("HOLSTER lines=2 parsed=2", p.stdout)
        self.assertIn("SWAP total=1", p.stdout)



class MainFactTests(unittest.TestCase):
    def _status(self, lines):
        _, _, main = H.parse_lines(list(enumerate(lines, 1)))
        return H._status(main)

    def test_tid_same_thread_and_reload_reported(self):
        st = self._status(["[holster] TID gobupdate=324", "[holster] TID place=324",
                           "[holster] RELOAD place=1 rot=(180.0,0.0,0.0) off=(0.200,0.000,0.000)"])
        self.assertIn("TID gobupdate=324 place=324 (same thread)", st)
        self.assertIn("RELOAD 1 last: place=1 rot=(180.0,0.0,0.0)", st)

    def test_tid_different_threads_flagged(self):
        st = self._status(["TID gobupdate=1", "TID place=2"])
        self.assertIn("(DIFFERENT threads)", st)


class PlaceDiagTests(unittest.TestCase):
    def test_refuse_node_tune_summarised(self):
        lines = ["TUNE boot place=1 rot=(0.0,0.0,0.0) off=(0.000,0.000,0.000) gobvt=0 bone=pelvis_g",
                 "PLACEREFUSE reason=nottarget n=1 gob=1 node=2 gobvt=0", "PLACEREFUSE reason=nottarget n=5 gob=1 node=2 gobvt=0",
                 "PLACEREFUSE reason=nohandle n=2 gob=1 node=2 gobvt=0",
                 "PLACENODE node=2 gob=1 t=2 step=0 name=rhand target=0", "PLACENODE node=3 gob=1 t=9 step=3 name=- target=0"]
        _, _, main = H.parse_lines(list(enumerate(lines, 1)))
        out = H._place_diag(main)
        self.assertEqual(out[0], "TUNE boot place=1 rot=(0.0,0.0,0.0) off=(0.000,0.000,0.000) gobvt=0 bone=pelvis_g")
        self.assertEqual(out[1], "PLACEREFUSE (n>=): nottarget=5, nohandle=2")
        self.assertTrue(out[2].startswith("PLACENODE distinct=2: "))
        self.assertIn("t=2 step=0 name=rhand target=0 x1", out[2])

    def test_owner_body_vs_comb(self):
        lines = ["COMB creature=0A10 old=0 new=1 edge=1 tick=5",
                 "PLACENODE node=2 gob=1 t=2 step=0 name=pelvis_g target=1 bvt=99 gname=g mname=w_Lngswrd_001 cat=melee parent=7 owner=0a10 body=P_AttonBB",
                 "PLACENODE node=3 gob=4 t=2 step=0 name=pelvis_g target=1 bvt=99 gname=g mname=w_Lngswrd_001 cat=melee parent=8 owner=bb body=N_CommM",
                 "PLACENODE node=5 gob=6 t=2 step=0 name=rhand target=0 bvt=99 gname=g mname=- cat=unknown parent=8 owner=cc body=N_CommM"]
        _, _, main = H.parse_lines(list(enumerate(lines, 1)))
        own = [l for l in H._place_diag(main) if l.startswith("OWNER")]
        self.assertEqual(own, ["OWNER (weapon parent+0x1D8 / body): 0xa10 P_AttonBB comb=yes x1, 0xbb N_CommM comb=no x1"])
        self.assertIn("cat=melee x2", H._place_diag(main)[0])   # old key format unchanged by the new tail (both weapon lines share it)

    def test_old_log_has_no_diag_lines(self):
        _, _, main = H.parse_lines([(1, "COMB creature=1 old=0 new=1 edge=1")])
        self.assertEqual(H._place_diag(main), [])

    def test_previewnode_and_camnode(self):   # HE0 (doc 42 HW-EDIT)
        lines = ["PLACENODE node=2 gob=1 t=2 step=0 name=pelvis_g target=1 bvt=99 gname=g mname=w_Lngswrd_001 cat=melee parent=7 owner=0a10 body=P_AttonBB",
                 "PREVIEWNODE gob=11 node=12 name=rhand parent=13 owner=0 body=P_AttonBB mname=w_Lngswrd_001 cat=melee gobvt=98b5cc tid=42 tick=9",
                 "PREVIEWNODE gob=14 node=15 name=lhand parent=13 owner=0 body=P_AttonBB mname=w_VibroBld_002 cat=melee gobvt=98b5cc tid=42 tick=9",
                 "CAMNODE parent=13 pmodel=P_AttonBB pvt=98b5cc pparent=16 ppmodel=charrec_light ppvt=98b5cc camhook_p=0 camhook_pp=00ABC0"]
        _, _, main = H.parse_lines(list(enumerate(lines, 1)))
        out = H._place_diag(main)
        self.assertIn("PREVIEWNODE n=2 parents=1 tids=42", out)
        self.assertIn("  parent=0x13 body=P_AttonBB owner=0x0 weapons=rhand:w_Lngswrd_001/melee lhand:w_VibroBld_002/melee", out)
        self.assertIn("PREVIEW mname vs in-world target=1 mname: shared=w_lngswrd_001 preview-only=w_vibrobld_002", out)
        self.assertIn("PREVIEW body vs in-world body: shared=P_AttonBB preview-only=-", out)
        self.assertIn("CAMNODE parent=0x13 P_AttonBB camhook=0 | pparent=0x16 charrec_light camhook=abc0", out)

def gob_reuse(gob=0x1A2B3C, node=0x4D5E6F, oldm="w_blaste_01", newm="w_vbsword_01"):
    return "GOBREUSE gob=%x node=%x oldm=%s newm=%s" % (gob, node, oldm, newm)


def flourish_open(slot=0):
    return "FLOURISH open slot=%d cli=1234 srv=5678 p524=8000 p528=0 b11E8=2" % slot


def flourish_close(slot=0):
    return "FLOURISH close slot=%d p520=0 p524=0" % slot


def slide_start(dir_="draw"):
    return "SLIDE start dir=%s owner=1234 gob=5678 left=0 from=0.00 ms=350" % dir_


def slide_end(dir_="stow", frames=21, maxdt=49):
    return "SLIDE end dir=%s owner=1234 ms=350 frames=%d maxdt=%d" % (dir_, frames, maxdt)


class EventTests(unittest.TestCase):
    """GOBREUSE / FLOURISH / SLIDE report lines."""

    def test_planted_positive_gobreuse_one_line(self):
        rc, out = run_log([gob_reuse()])
        self.assertEqual(rc, 0)
        self.assertIn("GOBREUSE total=1", out)

    def test_no_gobreuse_prints_zero(self):
        rc, out = run_log([swap()])
        self.assertIn("GOBREUSE total=0", out)
        self.assertIn("FLOURISH open=0 close=0", out)
        self.assertIn("SLIDE start=0 end=0 draw=0 stow=0", out)

    def test_gobreuse_fields_and_stamp(self):
        recs, skipped, _ = parse([stamped(gob_reuse(0x001A, 0x2B, "a_b", "c_d"))])
        self.assertEqual(skipped, [])
        self.assertEqual((recs[0]["gob"], recs[0]["node"], recs[0]["oldm"], recs[0]["newm"]), ("1a", "2b", "a_b", "c_d"))

    def test_flourish_counts(self):
        rc, out = run_log([flourish_open(0), stamped(flourish_open(1)), flourish_open(0), flourish_close(0), stamped(flourish_close(1))])
        self.assertIn("FLOURISH open=3 close=2", out)

    def test_slide_counts_split_and_extremes(self):
        lines = [slide_start("draw"), slide_start("draw"), slide_start("stow"),
                 slide_end("draw", frames=30, maxdt=20), stamped(slide_end("stow", frames=21, maxdt=49)),
                 slide_end("draw", frames=5, maxdt=33)]
        rc, out = run_log(lines)
        self.assertIn("SLIDE start=3 end=3 draw=2 stow=1 frames min=5 max=30 maxdt max=49", out)

    def test_malformed_lines_are_skipped_not_fatal(self):
        bad = ["GOBREUSE gob=zz node=1 oldm=a newm=b", "GOBREUSE gob=1", "FLOURISH open slot=0 cli=1", "FLOURISH bogus slot=0",
               "SLIDE end dir=stow owner=1 ms=350 frames=2x maxdt=4", "SLIDE start dir=draw owner=1", "SLIDE"]
        rc, out = run_log(bad + [gob_reuse()])
        self.assertEqual(rc, 0)
        self.assertIn("GOBREUSE total=1", out)
        self.assertIn("FLOURISH open=0 close=0", out)
        self.assertIn("SLIDE start=0 end=0 draw=0 stow=0", out)
        self.assertTrue(any(l.startswith("HOLSTER lines=8 parsed=1 skipped=6") for l in out), out)

    def test_existing_gob_and_events_do_not_collide(self):
        recs, _, _ = parse([gob_line(1, 0x9000, 0x1, (0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0)), gob_reuse()])
        self.assertEqual([r["kind"] for r in recs], ["GOB", "GOBREUSE"])


def party_hold(state="on", n=1):
    return "PARTYHOLD %s n=%d" % (state, n)


def move_hold(slot=0):
    return "MOVEHOLD slot=%d skip cut (moving)" % slot


def delay_cut(kind="combat", wrote=1):
    return "DELAY slot=0 cut before=6486 elapsed=1514 kind=%s wrote=%d" % (kind, wrote)


class ToggleEventTests(unittest.TestCase):
    """PARTYHOLD / MOVEHOLD / DELAY report lines."""

    def test_planted_positive_movehold(self):
        rc, out = run_log([move_hold()])
        self.assertEqual(rc, 0)
        self.assertIn("MOVEHOLD total=1", out)

    def test_planted_negative_movehold_and_all_zero(self):
        rc, out = run_log([swap()])
        self.assertIn("MOVEHOLD total=0", out)
        self.assertIn("PARTYHOLD on=0 off=0", out)
        self.assertIn("DELAY cuts=0 combat=0 flourish=0", out)

    def test_partyhold_counts(self):
        rc, out = run_log([party_hold("on", 1), stamped(party_hold("off", 0)), party_hold("on", 2)])
        self.assertIn("PARTYHOLD on=2 off=1", out)

    def test_movehold_counts_with_stamp(self):
        rc, out = run_log([move_hold(0), stamped(move_hold(1)), move_hold(0)])
        self.assertIn("MOVEHOLD total=3", out)

    def test_delay_counts_by_kind_and_wrote(self):
        rc, out = run_log([delay_cut("combat"), stamped(delay_cut("flourish")), delay_cut("combat"), delay_cut("combat", wrote=0)])
        self.assertIn("DELAY cuts=3 combat=2 flourish=1", out)

    def test_malformed_lines_ignored(self):
        bad = ["PARTYHOLD maybe n=1", "PARTYHOLD on", "MOVEHOLD slot=x skip cut (moving)", "MOVEHOLD slot=0 skip cut",
               "DELAY slot=0 cut before=1 elapsed=2 kind=other wrote=1", "DELAY slot=0 cut before=a elapsed=2 kind=combat wrote=1",
               "DELAY slot=0 cut before=1 elapsed=2 kind=combat", "PARTYHOLD", "MOVEHOLD", "DELAY"]
        rc, out = run_log(bad + [party_hold("off", 0), move_hold(), delay_cut("flourish")])
        self.assertEqual(rc, 0)
        self.assertIn("PARTYHOLD on=0 off=1", out)
        self.assertIn("MOVEHOLD total=1", out)
        self.assertIn("DELAY cuts=1 combat=0 flourish=1", out)
        self.assertTrue(any(l.startswith("HOLSTER lines=13 parsed=0 skipped=0") for l in out), out)

    def test_toggle_events_do_not_become_records(self):
        recs, skipped, _ = parse([party_hold(), move_hold(), delay_cut()])
        self.assertEqual((recs, skipped), ([], []))


class Hw24ReachTests(unittest.TestCase):
    """HW24 (0.12.0-dev): REACH wrist/hinge/g/cap fields, SLIDE cut and RESID one-liners."""

    NEW_REACH = ("REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 "
                 "u=1.00 lt=(0,0,0) err=0.010 persist=0 wrote=7 wr=88 hg=1 f=0.40 dzA=0.22 fy=-0.10 fx=0.05 g=130 cap=90")
    OLD_REACH = ("REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 "
                 "u=1.00 lt=(0,0,0) err=0.010 persist=0 wrote=3")

    def test_new_reach_line_parses_wrist_hinge_g_cap(self):
        _, _, main = parse([self.NEW_REACH])
        self.assertEqual(len(main["reach"]), 1)
        out = " ".join(H._reach_events(main))
        self.assertIn("wrist=1", out)
        self.assertIn("hg=1", out)
        self.assertIn("g max=130", out)
        self.assertIn("over_cap=1", out)

    def test_old_reach_line_still_parses_without_wrist_or_g(self):
        _, _, main = parse([self.OLD_REACH])
        self.assertEqual(len(main["reach"]), 1)
        out = " ".join(H._reach_events(main))
        self.assertIn("wrist=0", out)
        self.assertNotIn("g max", out)

    def test_slide_cut_unreadable_counted(self):
        recs, skipped, main = parse(["SLIDE cut reason=unreadable owner=1 gob=2 left=0 dir=draw frac=0.40"])
        self.assertEqual((recs, skipped), ([], []))   # counted, not a skipped malformed SLIDE line
        self.assertEqual(main["slidecut"]["unreadable"], 1)
        self.assertIn("cut=unreadable:1", " ".join(H._reach_events(main)))

    def test_resid_line_reports_max_position_and_angle(self):
        _, _, main = parse(["RESID gob=2 left=0 dir=draw carryW=0.50 f=0.50 pos_cm=3.2 ang=40 dq=0.900 style=3"])
        self.assertIn("resid max=3.2cm/40deg", " ".join(H._reach_events(main)))


class Hw25HoldTests(unittest.TestCase):
    """HW25 (0.14.0-dev): REACH hold= tail on the full line, held-frame lines by refusal kind; SLIDE cut and RESID counting unaffected."""

    HOLD_REACH = ("REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 "
                  "u=1.00 lt=(0,0,0) err=0.010 persist=0 wrote=7 wr=88 hg=1 f=0.40 dzA=0.22 fy=-0.10 fx=0.05 g=130 cap=90 hold=2")
    HOLD0_REACH = HOLD_REACH.replace("hold=2", "hold=0")

    def test_planted_hold_tail_and_held_lines_counted(self):
        lines = [self.HOLD_REACH, self.HOLD0_REACH,
                 "REACH owner=1a hold=1 why=pose solved=7 w=0.50", "REACH owner=1a hold=2 why=pose solved=7 w=0.50", "REACH owner=1a hold=1 why=solve solved=3 w=0.40"]
        _, skipped, main = parse(lines)
        self.assertEqual(skipped, [])
        self.assertEqual(len(main["reach"]), 2)          # the short held lines are not full REACH steps
        self.assertEqual(main["reachhold"], {"pose": 2, "solve": 1})
        out = " ".join(H._reach_events(main))
        self.assertIn("hold=pose:2,solve:1 after_hold=1 max=2", out)
        self.assertIn("wrist=2", out)                    # the hold= tail did not disturb the older fields
        self.assertIn("g max=130", out)

    def test_planted_negative_no_hold_no_suffix(self):
        _, _, main = parse([Hw24ReachTests.NEW_REACH, Hw24ReachTests.OLD_REACH, self.HOLD0_REACH])
        self.assertEqual(len(main["reach"]), 3)
        self.assertEqual(sum(main["reachhold"].values()), 0)
        self.assertNotIn(" hold=", " ".join(H._reach_events(main)))

    def test_slide_cut_and_resid_still_counted_next_to_hold(self):
        _, skipped, main = parse(["SLIDE cut reason=first owner=1 gob=2 left=0 dir=stow frac=0.40", self.HOLD_REACH,
                                  "RESID gob=2 left=0 dir=draw carryW=0.50 f=0.50 pos_cm=3.2 ang=40 dq=0.900 style=3"])
        self.assertEqual(skipped, [])
        out = " ".join(H._reach_events(main))
        self.assertIn("cut=first:1", out)
        self.assertIn("resid max=3.2cm/40deg", out)
        self.assertIn("after_hold=1 max=2", out)


class Hw29TwistTests(unittest.TestCase):
    """HW29 (0.17.0-dev): REACH tw= sw= tail after bk=; older lines without it are unaffected."""

    TW_REACH = (Hw25HoldTests.HOLD0_REACH + " topo=hinge bk=1 tw=74 sw=41")
    TW0_REACH = (Hw25HoldTests.HOLD0_REACH + " topo=hip bk=0 tw=0 sw=12")

    def test_planted_tw_sw_tail_counted(self):
        _, skipped, main = parse([self.TW_REACH, self.TW0_REACH])
        self.assertEqual(skipped, [])
        self.assertEqual(main["reachtw"], [(74.0, 41.0), (0.0, 12.0)])
        self.assertEqual(len(main["reach"]), 2)
        out = " ".join(H._reach_events(main))
        self.assertIn("tw max=74 sw max=41 twisted=1", out)
        self.assertIn("g max=130", out)                  # the older fields are undisturbed
        self.assertIn("topo=hinge+bk:1,hip:1", out)

    def test_planted_negative_no_tail_no_suffix(self):
        _, _, main = parse([Hw24ReachTests.NEW_REACH, Hw25HoldTests.HOLD_REACH])
        self.assertEqual(main["reachtw"], [])
        self.assertNotIn(" tw max=", " ".join(H._reach_events(main)))


class Hw31ExtractTests(unittest.TestCase):
    """HW31 (0.18.0-dev): REACH ex= of= tail after sw=; older lines without it are unaffected."""

    EX_REACH = (Hw25HoldTests.HOLD0_REACH + " topo=hinge bk=1 tw=74 sw=41 ex=0.62 of=0.35")
    EX0_REACH = (Hw25HoldTests.HOLD0_REACH + " topo=hip bk=0 tw=0 sw=12 ex=0.00 of=0.50")

    def test_planted_ex_of_tail_counted(self):
        _, skipped, main = parse([self.EX_REACH, self.EX0_REACH])
        self.assertEqual(skipped, [])
        self.assertEqual(main["reachex"], [(0.62, 0.35), (0.0, 0.5)])
        self.assertEqual(main["reachtw"], [(74.0, 41.0), (0.0, 12.0)])   # the 0.17 tail is undisturbed
        out = " ".join(H._reach_events(main))
        self.assertIn("ex max=0.62 extracting=1 of max=0.50", out)

    def test_planted_negative_old_tail_no_suffix(self):
        _, _, main = parse([Hw29TwistTests.TW_REACH, Hw24ReachTests.NEW_REACH])
        self.assertEqual(main["reachex"], [])
        self.assertNotIn(" ex max=", " ".join(H._reach_events(main)))


class Hw16AnimStyleTests(unittest.TestCase):
    """HW16 (0.13.0-dev): ANIMSTYLE counter and the hol/comb tail of the ANIMRES line."""
    ANIMRES_HOL = "ANIMRES self=2a1 id=2714 style=4 gate=1 cc=2 d4=3 e8=4 hol=1 comb=0 tick=1500"
    ANIMRES_HOL0 = "ANIMRES self=2a1 id=2714 style=4 gate=1 cc=2 d4=3 e8=4 hol=0 comb=-1 tick=1500"

    def test_planted_positive_animstyle_counted(self):
        lines = ["ANIMSTYLE owner=2a1 id=2714 style=4->8", "ANIMSTYLE owner=2a1 id=2713 style=7->8", "ANIMSTYLE owner=1f0 id=2712 style=2->8"]
        recs, skipped, main = parse(lines)
        self.assertEqual((recs, skipped), ([], []))   # not a keyword record, not a malformed line
        self.assertEqual(len(main["animstyle"]), 3)
        out = H._animstyle(main, recs)
        self.assertIn("ANIMSTYLE n=3 owners=2", out)
        self.assertIn("styles 2->8,4->8,7->8", out)

    def test_planted_negative_no_animstyle_no_suffix(self):
        recs, _, main = parse(["ANIMSTYLE owner=zz id=2714 style=x->y", comb(1, 0, 1, 1)])
        self.assertEqual(H._animstyle(main, recs), "")
        self.assertEqual(main.get("animstyle", []), [])

    def test_animres_with_hol_comb_parses_and_counts_hol1(self):
        recs, skipped, main = parse([self.ANIMRES_HOL, self.ANIMRES_HOL0, anim_res(0x10, "2714")])
        self.assertEqual(skipped, [])   # the 0.5.2+ line used to land in skipped
        self.assertEqual([r["hol"] for r in recs], [1, 0, -1])
        self.assertIn(" hol=1 ANIMRES=1", H._animstyle(main, recs))

    def test_animres_with_trailing_own_parses(self):
        """0.16.0-dev: own=<creature> is appended after tick=; older lines without it still parse (own = "")."""
        withown = self.ANIMRES_HOL + " own=2a0"
        zero = self.ANIMRES_HOL0 + " own=0"
        recs, skipped, main = parse([withown, zero, self.ANIMRES_HOL])
        self.assertEqual(skipped, [])
        self.assertEqual([r["hol"] for r in recs], [1, 0, 1])
        self.assertEqual([r["own"] for r in recs], ["2a0", "0", ""])
        self.assertEqual([r["self"] for r in recs], ["2a1"] * 3)
        self.assertIn(" hol=1 ANIMRES=2", H._animstyle(main, recs))

    def test_animres_own_non_hex_tail_is_ignored(self):
        """The ANIMRES pattern is not end-anchored (as before): a non-hex own= tail keeps the record and gives own = ""."""
        recs, skipped, _ = parse([self.ANIMRES_HOL + " own=zz"])
        self.assertEqual(skipped, [])
        self.assertEqual([r["own"] for r in recs], [""])

    def test_report_line_cap_unchanged(self):
        recs, skipped, main = parse([LOADED, self.ANIMRES_HOL, "ANIMSTYLE owner=2a1 id=2714 style=4->8"])
        out = H.render(recs, skipped, main, 3, "")
        self.assertTrue(any(l.startswith("ANIMRES n=1") and "ANIMSTYLE n=1 owners=1" in l for l in out))
        self.assertLessEqual(len(out), 47)


class Hw27Hw28ConfigTests(unittest.TestCase):
    """HW27/HW28 (0.15.0-dev): REACH topo=/bk= tail, CONFIG slide= timing, PLACEREFUSE notholsterable."""

    TOPO_REACH = ("REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 "
                  "u=0.28 lt=(0,0,0) err=0.010 persist=0 wrote=7 wr=88 hg=0 f=0.40 dzA=-0.52 fy=-0.03 fx=-0.28 g=130 cap=120 hold=0 topo=hip bk=1")
    HINGE_REACH = TOPO_REACH.replace("topo=hip bk=1", "topo=hinge bk=1").replace("hg=0", "hg=1")
    CONFIG = ("CONFIG toggle_key=0x77 draw_hold_ms=8000 combat_exit_ms=1500 flourish_exit_ms=1500 hostile_range=30.0 hostile_backoff_ms=1000 delay_party=1 flourish_draw=1 "
              "slide=lightsaber:350/350/30,melee:520/600/45,qstaff:570/700/45,pistol:300/350/30,rifle:520/600/45,heavy:570/700/45 resid_ms=240 wrist_max=120 "
              "move_hold_ms=1500 party_combat_hold=1 reach_test=1 reach_carry=1 reach_hinge=1 unarmed_anim=1 (active=1: needs probe=1, bone_test=1 and place_mode=drawgate)")

    def test_planted_topo_and_slide_config_reported(self):
        _, skipped, main = parse([self.TOPO_REACH, self.TOPO_REACH, self.HINGE_REACH, self.CONFIG])
        self.assertEqual(skipped, [])
        self.assertEqual(len(main["reach"]), 3)              # the new tail did not break the full-line parse
        self.assertEqual(main["reachtopo"], {("hip", 1): 2, ("hinge", 1): 1})
        out = " ".join(H._reach_events(main))
        self.assertIn("topo=hinge+bk:1,hip+bk:2", out)
        self.assertIn("slide=lightsaber:350/350/30,melee:520/600/45,qstaff:570/700/45,pistol:300/350/30,rifle:520/600/45,heavy:570/700/45 resid_ms=240 wrist_max=120", out)
        self.assertIn("g max=130", out)                      # older fields unaffected

    def test_planted_negative_old_lines_have_no_topo_or_slide(self):
        _, _, main = parse([Hw24ReachTests.NEW_REACH, Hw25HoldTests.HOLD_REACH, "CONFIG toggle_key=0x77 draw_slide_ms=350 stow_slide_ms=350 reach_test=1"])
        self.assertEqual(sum(main["reachtopo"].values()), 0)
        self.assertEqual(main["slidecfg"], [])
        out = " ".join(H._reach_events(main))
        self.assertNotIn(" topo=", out)
        self.assertNotIn(" slide=", out)

    def test_notholsterable_refusal_counted_with_max_n(self):
        _, skipped, main = parse(["PLACEREFUSE reason=notholsterable n=1 gob=2a node=3b gobvt=4c", "PLACEREFUSE reason=notholsterable n=7 gob=2a node=3b gobvt=4c"])
        self.assertEqual(skipped, [])
        self.assertEqual(main["refuse"]["notholsterable"], 7)
        self.assertIn("notholsterable=7", " ".join(H._place_diag(main)))


def mount_line(rel="(0.5,0.5,0.5,0.5)", oq="(1,0,0,0)", bone="torso_g", cat=1, bq="(0.7071,0,0.7071,0)", mq="(0.5,0.5,0.5,0.5)"):
    return "MOUNT gob=1a2b3c stem=w_lghtsbr_001 bone=%s bq=%s mq=%s oq=%s rel=%s cat=%d" % (bone, bq, mq, oq, rel, cat)


MOUNT_OK = mount_line()
MOUNT_FLIP = mount_line(rel="(-0.5,-0.5,-0.5,-0.5)")   # same rotation, opposite sign: must align to MOUNT_OK, not pull the median apart
MOUNT_BAD = mount_line(rel="(0.5,0.5,abc,0.5)")         # unparsable quat: skipped
MOUNT_NOFACING = mount_line(rel="nan", oq="nan")         # owner facing unknown: nofacing, excluded from the median
MOUNT_BODIES = [MOUNT_OK, MOUNT_FLIP, MOUNT_BAD, MOUNT_NOFACING]
MOUNT_HEAD = "MOUNT lines=3 nofacing=1 skipped=1 groups=1"
MOUNT_ROW = "  cat=1 bone=torso_g count=3 nofacing=1 median rel=(0.500,0.500,0.500,0.500) spread=0.00 deg"


class Hw35MountTests(unittest.TestCase):
    def test_mount_records_parse_and_bad_quat_is_skipped_not_fatal(self):
        recs, skipped, _ = parse(MOUNT_BODIES)
        mounts = [r for r in recs if r["kind"] == "MOUNT"]
        self.assertEqual([r["line"] for r in mounts], [1, 2, 4])
        self.assertEqual([(s[0], s[1]) for s in skipped], [(3, "MOUNT")])
        self.assertEqual((mounts[0]["gob"], mounts[0]["stem"], mounts[0]["bone"], mounts[0]["cat"]), ("1a2b3c", "w_lghtsbr_001", "torso_g", 1))
        self.assertEqual(mounts[0]["rel"], [0.5, 0.5, 0.5, 0.5])
        self.assertEqual(mounts[0]["bq"], [0.7071, 0.0, 0.7071, 0.0])

    def test_nofacing_record_has_no_oq_or_rel(self):
        recs, _, _ = parse([MOUNT_NOFACING])
        self.assertEqual(len(recs), 1)
        self.assertTrue(recs[0]["nofacing"])
        self.assertIsNone(recs[0]["rel"])
        self.assertIsNone(recs[0]["oq"])

    def test_zero_or_short_quat_is_skipped(self):
        recs, skipped, _ = parse([mount_line(rel="(0,0,0,0)"), mount_line(rel="(0.5,0.5,0.5)")])
        self.assertEqual(recs, [])
        self.assertEqual([(s[0], s[1]) for s in skipped], [(1, "MOUNT"), (2, "MOUNT")])

    def test_old_chplace_mount_line_is_neither_record_nor_skipped(self):
        recs, skipped, main = parse(["MOUNT 1a2b parent=5 bone=torso_g node=7 a=0 b=0"])
        self.assertEqual((recs, skipped), ([], []))
        self.assertEqual(main["mount"], Counter({("1a2b", "torso_g"): 1}))

    def test_median_aligns_sign_and_spread_is_zero_for_same_rotation(self):
        recs, skipped, _ = parse(MOUNT_BODIES)
        self.assertEqual(H._mounts(recs, skipped), [MOUNT_HEAD, MOUNT_ROW])

    def test_spread_is_angle_from_median(self):
        # two samples at identity and one 10 deg about x: median is identity, spread is the 10 deg outlier
        recs, skipped, _ = parse([mount_line(rel="(1,0,0,0)"), mount_line(rel="(1,0,0,0)"), mount_line(rel="(0.99619,0.08716,0,0)")])
        self.assertEqual(H._mounts(recs, skipped)[1],
                         "  cat=1 bone=torso_g count=3 nofacing=0 median rel=(1.000,0.000,0.000,0.000) spread=10.00 deg")

    def test_report_has_mount_section_only_when_mount_lines_exist(self):
        rc, out = run_log(MOUNT_BODIES)
        self.assertEqual(rc, 0)
        self.assertIn("parsed=3 skipped=1", out[0])
        self.assertIn(MOUNT_HEAD, out)
        self.assertIn(MOUNT_ROW, out)
        self.assertEqual(out.index(MOUNT_HEAD) + 1, out.index(MOUNT_ROW))
        rc, out = run_log(fixture_bodies())
        self.assertFalse(any(s.startswith("MOUNT ") for s in out))

    def test_mounts_flag_prints_only_the_mount_section(self):
        rc, out = run_log(MOUNT_BODIES, ["--mounts"])
        self.assertEqual(rc, 0)
        self.assertEqual(out, [MOUNT_HEAD, MOUNT_ROW])

    def test_mounts_flag_without_mount_lines_prints_zero_section(self):
        rc, out = run_log([LOADED], ["--mounts"])
        self.assertEqual(out, ["MOUNT lines=0 nofacing=0 skipped=0 groups=0"])

    def test_worst_case_with_place_and_mounts_stays_within_47_lines(self):
        mounts = [mount_line(bone="b%d" % i, cat=i % 2 + 1) for i in range(5)] + [MOUNT_NOFACING]
        rc, out = run_log(worst_case()[:-1] + place_worst() + mounts + [CAP])
        self.assertEqual(rc, 0)
        self.assertLessEqual(len(out), 47)
        self.assertIn("MOUNT lines=6 nofacing=1 skipped=0 groups=6 (top 3 by samples shown)", out)
        self.assertEqual(len([s for s in out if s.startswith("  cat=")]), 3)
        self.assertIn("GOB lines=55 gobs=10 (top 2 by samples shown)", out)


# HW38 golden: a window with no dly= / vo= / pb= and no wr/f/dzA/fy/fx on its REACH line prints the report captured before HW38.
GOLDEN_REACH = ("REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 "
                "u=1.00 lt=(0,0,0) err=0.010 persist=0 wrote=3")
GOLDEN_BODIES = (fixture_bodies()[:-1]
                 + [slide_start("draw"), slide_end("draw"), "SLIDE cut reason=first owner=1234 gob=5678 left=0 dir=stow frac=0.40"]
                 + MOUNT_BODIES[:2] + [GOLDEN_REACH, CAP])
GOLDEN_REPORT = [
    "HOLSTER lines=35 parsed=30 skipped=0",
    "status: loaded=yes 0.1.0-dev | cap reached=yes max_lines=60000 | BONETEST=patched x1 | TID gobupdate=absent place=absent | RELOAD 0",
    "COMB edges=5 creatures=2",
    "  creature=0x2a1 edges=3 last new=1",
    "  creature=0x1f0 edges=2 last new=0",
    "ATTACH n=4 creatures=2 tags: 'hand_r'=3 'hand_l'=1",
    "  creature=0x1f0 distinct=2 (body/handleR/handleL): 5/9/0 x2; 5/9/a x1",
    "  creature=0x2a1 distinct=1 (body/handleR/handleL): 7/0/0 x1",
    "SWAP total=3",
    "GOBREUSE total=0",
    "FLOURISH open=0 close=0",
    "SLIDE start=1 end=1 draw=1 stow=0 frames min=21 max=21 maxdt max=49",
    "PARTYHOLD on=0 off=0",
    "MOVEHOLD total=0",
    "DELAY cuts=0 combat=0 flourish=0",
    "REACH found=0 steps=1 persist1=0 wrote_both=1 refuse=none err min=0.010 med=0.010 max=0.010 reach<1=0 wrist=0 hg=0 cut=first:1",
    "ANIMRES n=4 distinct=2 top10: 0x1f x3, 0x7 x1 | style per creature (self style/gate): 10 0/1 x2, 11 0/1 x2",
    "ANIMSEND n=2 distinct=1 top10: 0x1f x2",
    "GOB lines=5 gobs=2",
    "  gob=0x9000 samples=3 parent18=0x1234 sign_flips=1 rot_mean=13.37 deg",
    "    pos x 0.000..0.200 mean 0.100 | y 1.000..1.000 mean 1.000 | z 2.000..2.400 mean 2.200",
    "    quat w -0.985..1.000 | x 0.000..0.000 | y 0.000..0.000 | z -0.174..0.174",
    "  gob=0x9100 samples=2 parent18=0x1234 sign_flips=0 rot_mean=0.00 deg",
    "    pos x 5.000..5.000 mean 5.000 | y 5.000..5.000 mean 5.000 | z 5.000..5.500 mean 5.250",
    "    quat w 1.000..1.000 | x 0.000..0.000 | y 0.000..0.000 | z 0.000..0.000",
    "PLACE lines=3 gobs=2 gobvt distinct=2: 0x1f x2, 0x7 x1",
    "PROBLEM PLACE quat1 |norm-1|>0.002: count=1 worst=0.0050 e.g. L27 gob=0x9300",
    "  gob=0x9200 samples=2 last pos0=(0.100,0.000,0.000) quat0=(0.985,0.000,0.000,0.174)",
    "    -> pos1=(0.200,0.000,0.000) quat1=(0.985,0.000,0.000,0.174)",
    "  gob=0x9300 samples=1 last pos0=(1.000,1.000,1.000) quat0=(1.000,0.000,0.000,0.000)",
    "    -> pos1=(1.000,1.000,1.500) quat1=(1.000,0.000,0.000,0.100)",
    "MOUNT lines=2 nofacing=0 skipped=0 groups=1",
    "  cat=1 bone=torso_g count=2 nofacing=0 median rel=(0.500,0.500,0.500,0.500) spread=0.00 deg",
]


def reach38(f=0.40, tw=40, sw=30, g=70, cap=90, ex=0.0, of_=0.0, vo=None, pb=None, wr=88):
    """A 0.20-style REACH line in the Holster.cpp tail order. vo/pb None = the field is absent (an older log)."""
    line = ("REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 "
            "u=1.00 lt=(0,0,0) err=0.010 persist=0 wrote=7 wr=%d hg=1 f=%.2f dzA=0.22 fy=-0.10 fx=0.05 g=%d cap=%d hold=0 "
            "topo=hip bk=1 tw=%d sw=%d ex=%.2f of=%.2f") % (wr, f, g, cap, tw, sw, ex, of_)
    if vo is not None:
        line += " vo=%.3f" % vo
    if pb is not None:
        line += " pb=%.1f" % pb
    return line


def slide38(ev, owner=0x1F, gob=0x2A, left=1, dly=None, dir_="draw"):
    """SLIDE start (dly= appended when given), end (owner only, as Holster.cpp prints it) or cut (names the Gob)."""
    if ev == "start":
        line = "SLIDE start dir=%s owner=%x gob=%x left=%d from=0.00 ms=520" % (dir_, owner, gob, left)
        return line if dly is None else line + " dly=%d" % dly
    if ev == "end":
        return "SLIDE end dir=%s owner=%x ms=520 frames=30 maxdt=20" % (dir_, owner)
    return "SLIDE cut reason=first owner=%x gob=%x left=%d dir=%s frac=0.10" % (owner, gob, left, dir_)


def rel_deg(theta):
    """A mount rel quat (w, x, y, z) for a rotation of theta degrees about x."""
    h = math.radians(theta) / 2.0
    return "(%.6f,%.6f,0,0)" % (math.cos(h), math.sin(h))


def run_err(argv):
    """(rc, stdout lines, stderr text) for argv that need no log file (the flag errors return before the file check)."""
    err = io.StringIO()
    with contextlib.redirect_stderr(err):
        rc, out = run(argv)
    return rc, out, err.getvalue()


class Hw38RowTests(unittest.TestCase):
    """HW38 --row SM-L11..SM-L15: a planted PASS and FAIL window per row, NO-DATA on an empty window, and the boundaries."""

    def verdict(self, row, bodies, rc, word):
        got_rc, out = run_log(bodies, ["--row", row])
        self.assertEqual(got_rc, rc, out)
        self.assertEqual(out[-1], "%s verdict: %s" % (row, word), out)
        self.assertTrue(all(line.startswith(row + " ") for line in out), out)
        return out

    def test_sm_l11_pass_at_the_caps(self):
        out = self.verdict("SM-L11", [reach38(tw=80, sw=60, g=85, cap=90), reach38(tw=40, sw=30, g=70, cap=90)], 0, "PASS")
        self.assertIn("SM-L11 tw<=80: PASS max 80 over 2 line(s)", out)
        self.assertIn("SM-L11 sw<=60: PASS max 60 over 2 line(s)", out)
        self.assertIn("SM-L11 g-cap<20: PASS max -5 over 2 line(s)", out)

    def test_sm_l11_fail_over_tw_and_gcap_of_exactly_20_fails(self):
        out = self.verdict("SM-L11", [reach38(tw=95)], 1, "FAIL")
        self.assertIn("SM-L11 tw<=80: FAIL max 95 over 1 line(s)", out)
        out = self.verdict("SM-L11", [reach38(g=110, cap=90)], 1, "FAIL")   # g-cap = 20 exactly: the rule is < 20
        self.assertIn("SM-L11 g-cap<20: FAIL max 20 over 1 line(s)", out)

    def test_sm_l11_no_data_on_empty_window(self):
        self.verdict("SM-L11", [LOADED], 3, "NO-DATA")

    def test_sm_l12_pass_fail_no_data(self):
        out = self.verdict("SM-L12", [reach38(ex=0.62, of_=0.00), reach38(ex=0.00, of_=1.00)], 0, "PASS")
        self.assertIn("SM-L12 of=0 through lift: PASS of<=0.05 on all 1 extracting frames (max ex=0.62)", out)
        self.assertIn("SM-L12 of reaches 1: PASS max of=1.00", out)
        out = self.verdict("SM-L12", [reach38(ex=0.62, of_=0.35)], 1, "FAIL")
        self.assertIn("SM-L12 of=0 through lift: FAIL of>0.05 on 1 of 1 extracting frames (max of=0.35)", out)
        self.verdict("SM-L12", [LOADED], 3, "NO-DATA")

    def test_sm_l13_pass_and_spread_fail_just_over_5(self):
        out = self.verdict("SM-L13", [mount_line(rel=rel_deg(0)), mount_line(rel=rel_deg(0)), mount_line(rel=rel_deg(4.9))], 0, "PASS")
        self.assertIn("SM-L13 spread<5 deg: PASS 1 group(s), max spread=4.90 deg (cat=1 bone=torso_g)", out)
        self.assertIn("SM-L13 nofacing==0: PASS nofacing=0 in all 1 group(s)", out)
        out = self.verdict("SM-L13", [mount_line(rel=rel_deg(0)), mount_line(rel=rel_deg(0)), mount_line(rel=rel_deg(5.2))], 1, "FAIL")
        self.assertIn("SM-L13 spread<5 deg: FAIL 1 group(s) at or above 5 deg; worst cat=1 bone=torso_g spread=5.20", out)

    def test_sm_l13_nofacing_fails_and_old_mount_lines_are_no_data(self):
        out = self.verdict("SM-L13", [mount_line(rel=rel_deg(0)), MOUNT_NOFACING], 1, "FAIL")
        self.assertIn("SM-L13 nofacing==0: FAIL nofacing>0 in 1 group(s); first cat=1 bone=torso_g nofacing=1", out)
        self.verdict("SM-L13", [LOADED, "MOUNT 1a2b parent=5 bone=torso_g node=7 a=0 b=0"], 3, "NO-DATA")

    def test_spread_exactly_five_degrees_is_a_fail(self):
        self.assertEqual(H._spread_clause([(5.0, 1, "torso_g")])[1], "FAIL")
        self.assertEqual(H._spread_clause([(4.999, 1, "torso_g")])[1], "PASS")
        self.assertEqual(H._spread_clause([])[1], "NO-DATA")

    def test_sm_l14_pass_with_dly_left_reversal_and_cut(self):
        bodies = [slide38("start", gob=0x2A, left=1, dly=60), slide38("start", gob=0x2B, left=0), slide38("end"), slide38("end")]
        out = self.verdict("SM-L14", bodies, 0, "PASS")
        self.assertIn("SM-L14 dly 50..70 ms on left: PASS left dly=60..60 ms over 1 left start(s)", out)
        self.assertIn("SM-L14 no dly on right: PASS no dly= on a right Gob (1 left starts)", out)
        self.assertIn("SM-L14 every start ends or is cut: PASS 2 start(s): 2 ended or cut, 0 superseded by a reversal", out)
        reversal = [slide38("start", gob=0x2A, left=1, dly=60), slide38("start", gob=0x2A, left=1), slide38("end")]
        out = self.verdict("SM-L14", reversal, 0, "PASS")   # a second start on the same Gob supersedes the first: not stuck
        self.assertIn("SM-L14 every start ends or is cut: PASS 2 start(s): 1 ended or cut, 1 superseded by a reversal", out)
        self.verdict("SM-L14", [slide38("start", gob=0x2A, left=1, dly=60), slide38("cut", gob=0x2A, left=1)], 0, "PASS")

    def test_sm_l14_stuck_slide_and_dly_on_right_gob_fail(self):
        bodies = [slide38("start", gob=0x2A, left=1, dly=60), slide38("start", gob=0x2B, left=0, dly=40), slide38("end")]
        out = self.verdict("SM-L14", bodies, 1, "FAIL")
        self.assertIn("SM-L14 no dly on right: FAIL dly=40 on right Gob gob=0x2b (1 right starts)", out)
        self.assertIn("SM-L14 every start ends or is cut: FAIL 1 start(s) never ended (stuck): first gob=0x2b dir=draw", out)
        out = self.verdict("SM-L14", [slide38("start", gob=0x2A, left=1, dly=90), slide38("end")], 1, "FAIL")
        self.assertIn("SM-L14 dly 50..70 ms on left: FAIL left dly=90 outside 50..70 ms (1 of 1 left starts)", out)

    def test_sm_l14_no_dly_field_is_no_data_even_with_slides(self):
        out = self.verdict("SM-L14", [slide38("start", gob=0x2A, left=1), slide38("end")], 3, "NO-DATA")
        self.assertIn("SM-L14 dly 50..70 ms on left: NO-DATA no dly= field in window", out)
        self.verdict("SM-L14", [LOADED], 3, "NO-DATA")

    def test_sm_l15_pass_vo_zero_at_edges_and_pb_up_to_f_030(self):
        ok = [reach38(f=0.00, vo=0.000, pb=0.0), reach38(f=0.50, vo=0.110, pb=0.0), reach38(f=1.00, vo=0.000, pb=0.0),
              reach38(f=0.20, vo=0.080, pb=3.2), reach38(f=0.30, vo=0.000, pb=2.0)]
        out = self.verdict("SM-L15", ok, 0, "PASS")
        self.assertIn("SM-L15 vo=0 at reach/release: PASS vo=0 at reach/release on all 5 lines (vo<0.005 m counts as 0)", out)
        self.assertIn("SM-L15 vo peaks mid-carry: PASS vo max=0.110 mid-carry over 3 line(s)", out)
        self.assertIn("SM-L15 pb>0 only at f<=0.30: PASS pb>0 only at f<=0.30 (max pb=3.2 over 5 line(s))", out)

    def test_sm_l15_fail_vo_at_edge_and_pb_past_f_030(self):
        out = self.verdict("SM-L15", [reach38(f=1.00, vo=0.050, pb=0.0), reach38(f=0.50, vo=0.110, pb=5.0)], 1, "FAIL")
        self.assertIn("SM-L15 vo=0 at reach/release: FAIL vo>=0.005 m at reach/release on 1 of 2 lines (max vo=0.050; vo<0.005 m counts as 0)", out)
        self.assertIn("SM-L15 pb>0 only at f<=0.30: FAIL pb>0 at f>0.30 on 1 line(s) (max pb=5.0)", out)

    def test_sm_l15_old_log_without_vo_is_no_data_never_fail(self):
        out = self.verdict("SM-L15", [reach38(f=0.5), reach38(f=1.0)], 3, "NO-DATA")
        self.assertIn("SM-L15 vo=0 at reach/release: NO-DATA no vo= field (older log)", out)
        self.assertIn("SM-L15 pb>0 only at f<=0.30: NO-DATA no pb= field (older log)", out)
        self.verdict("SM-L15", [LOADED], 3, "NO-DATA")

    def test_carry_edges_are_the_printed_endpoints(self):
        self.assertFalse(H._mid_carry(0.00))
        self.assertFalse(H._mid_carry(1.00))
        self.assertTrue(H._mid_carry(0.01))
        self.assertTrue(H._mid_carry(0.99))

    def test_sm_l14_dly_bounds_are_inclusive_50_to_70(self):
        for v in (50, 70, 49, 71):
            word = "PASS" if v in (50, 70) else "FAIL"
            with self.subTest(dly=v):
                out = self.verdict("SM-L14", [slide38("start", gob=0x2A, left=1, dly=v), slide38("end")], 0 if word == "PASS" else 1, word)
                if word == "FAIL":
                    self.assertIn("SM-L14 dly 50..70 ms on left: FAIL left dly=%d outside 50..70 ms (1 of 1 left starts)" % v, out)

    def test_sm_l14_cut_closes_a_running_start_and_a_cut_on_the_other_gob_does_not(self):
        out = self.verdict("SM-L14", [slide38("start", gob=0x2A, left=1, dly=60), slide38("cut", gob=0x2A, left=1)], 0, "PASS")
        self.assertIn("SM-L14 every start ends or is cut: PASS 1 start(s): 1 ended or cut, 0 superseded by a reversal", out)
        out = self.verdict("SM-L14", [slide38("start", gob=0x2A, left=1, dly=60), slide38("cut", gob=0x2B, left=0)], 1, "FAIL")
        self.assertIn("SM-L14 every start ends or is cut: FAIL 1 start(s) never ended (stuck): first gob=0x2a dir=draw", out)

    def test_sm_l15_pb_boundary_f_030_passes_and_031_fails(self):
        rc, out = run_log([reach38(f=0.30, vo=0.000, pb=2.0)], ["--row", "SM-L15"])
        self.assertIn("SM-L15 pb>0 only at f<=0.30: PASS pb>0 only at f<=0.30 (max pb=2.0 over 1 line(s))", out)
        rc, out = run_log([reach38(f=0.31, vo=0.000, pb=2.0)], ["--row", "SM-L15"])
        self.assertIn("SM-L15 pb>0 only at f<=0.30: FAIL pb>0 at f>0.30 on 1 line(s) (max pb=2.0)", out)
        self.assertEqual(rc, 1)

    def test_sm_l15_vo_edge_tolerance_0004_passes_and_0006_fails(self):
        ok = [reach38(f=0.00, vo=0.004, pb=0.0), reach38(f=0.50, vo=0.110, pb=0.0)]
        out = self.verdict("SM-L15", ok, 0, "PASS")
        self.assertIn("SM-L15 vo=0 at reach/release: PASS vo=0 at reach/release on all 2 lines (vo<0.005 m counts as 0)", out)
        bad = [reach38(f=1.00, vo=0.006, pb=0.0), reach38(f=0.50, vo=0.110, pb=0.0)]
        out = self.verdict("SM-L15", bad, 1, "FAIL")
        self.assertIn("SM-L15 vo=0 at reach/release: FAIL vo>=0.005 m at reach/release on 1 of 2 lines (max vo=0.006; vo<0.005 m counts as 0)", out)

    def test_sm_l15_mid_carry_peak_present_passes_absent_is_no_data(self):
        rc, out = run_log([reach38(f=0.50, vo=0.020, pb=0.0)], ["--row", "SM-L15"])
        self.assertIn("SM-L15 vo peaks mid-carry: PASS vo max=0.020 mid-carry over 1 line(s)", out)
        rc, out = run_log([reach38(f=0.00, vo=0.000, pb=0.0), reach38(f=0.50, vo=0.000, pb=0.0)], ["--row", "SM-L15"])
        self.assertIn("SM-L15 vo peaks mid-carry: NO-DATA vo=0 mid-carry: no rifle or quarterstaff carry in window", out)
        self.assertEqual(rc, 3)

    def test_sm_l12_of_reaches_1_threshold_pinned_at_099(self):
        rc, out = run_log([reach38(ex=0.00, of_=0.98)], ["--row", "SM-L12"])
        self.assertIn("SM-L12 of reaches 1: NO-DATA max of=0.98: no full orientation (a reversed draw stops short)", out)
        rc, out = run_log([reach38(ex=0.00, of_=0.99)], ["--row", "SM-L12"])
        self.assertIn("SM-L12 of reaches 1: PASS max of=0.99", out)

    def test_sm_l12_extract_lift_threshold_pinned_at_005(self):
        rc, out = run_log([reach38(ex=0.05, of_=0.50)], ["--row", "SM-L12"])
        self.assertIn("SM-L12 of=0 through lift: NO-DATA no extracting frame (ex<=0.05 on all 1 lines): lift not seen", out)
        rc, out = run_log([reach38(ex=0.06, of_=0.50)], ["--row", "SM-L12"])
        self.assertIn("SM-L12 of=0 through lift: FAIL of>0.05 on 1 of 1 extracting frames (max of=0.50)", out)
        self.assertEqual(rc, 1)

    def test_unknown_row_and_flag_errors_exit_2(self):
        rc, out, err = run_err(["--row", "SM-L99", "/nonexistent.txt"])
        self.assertEqual(rc, 2)
        self.assertIn("unknown row SM-L99", err)
        rc, out, err = run_err(["--json", "/nonexistent.txt"])
        self.assertEqual(rc, 2)
        self.assertIn("--json needs --mounts", err)
        rc, out, err = run_err(["--row", "SM-L13", "--mounts", "/nonexistent.txt"])
        self.assertEqual(rc, 2)


class Hw38ReportTests(unittest.TestCase):
    """HW38 default-report summaries: each line appears only when its field is in the window."""

    def test_dly_vo_pb_and_stats_lines_when_fields_present(self):
        rc, out = run_log([slide38("start", gob=0x2A, left=1, dly=60), slide38("end"), slide38("start", gob=0x2B, left=0), slide38("end"),
                           reach38(f=0.2, vo=0.080, pb=3.2), reach38(f=0.5, vo=0.110, pb=0.0), reach38(f=1.0, vo=0.050, pb=0.0)])
        self.assertEqual(rc, 0)
        self.assertIn("SLIDE dly n=1 max=60 per Gob: gob=0x2a L n=1 max=60", out)
        self.assertIn("REACH HW38 vo max=0.110 vo>0 outside mid-carry=1 (33.3% of 3 lines) | pb max=3.2 pb>0 at f>0.30=0 (of 3 lines)", out)
        self.assertIn("REACH min/median/max: wr 88/88/88 | f 0.20/0.50/1.00 | dzA 0.22/0.22/0.22 | fy -0.10/-0.10/-0.10 | fx 0.05/0.05/0.05", out)

    def test_worst_case_with_hw38_lines_stays_within_50(self):
        """The 47-line worst case plus the three HW38 lines: the real cap is 50."""
        bodies = worst_case()[:-1] + [slide38("start", gob=0x2A, left=1, dly=60), slide38("end"), reach38(f=0.20, vo=0.080, pb=3.2), CAP]
        rc, out = run_log(bodies)
        self.assertEqual(rc, 0)
        self.assertLessEqual(len(out), 50)
        self.assertEqual(sum(1 for line in out if line.startswith(("SLIDE dly", "REACH HW38", "REACH min/median/max"))), 3)

    def test_no_hw38_line_for_old_reach_lines_without_vo_pb(self):
        rc, out = run_log([reach38(f=0.5, tw=40, sw=30)])
        self.assertFalse(any(line.startswith("REACH HW38") or line.startswith("SLIDE dly") for line in out))
        self.assertIn("REACH min/median/max: wr 88/88/88 | f 0.50/0.50/0.50 | dzA 0.22/0.22/0.22 | fy -0.10/-0.10/-0.10 | fx 0.05/0.05/0.05", out)


class Hw38JsonTests(unittest.TestCase):
    """HW38 --mounts --json: exact keys, sorted by cat then bone, nothing else on stdout."""

    def test_mounts_json_round_trips_with_exact_keys(self):
        bodies = [mount_line(rel=rel_deg(0)), mount_line(rel=rel_deg(0)), mount_line(bone="a_bone", cat=2, rel=rel_deg(0)), MOUNT_NOFACING, MOUNT_BAD]
        rc, out = run_log(bodies, ["--mounts", "--json"])
        self.assertEqual(rc, 0)
        self.assertEqual(len(out), 1)
        data = json.loads(out[0])
        self.assertEqual([list(d.keys()) for d in data], [["cat", "bone", "count", "nofacing", "rel", "spread"]] * 2)
        self.assertEqual((data[0]["cat"], data[0]["bone"], data[0]["count"], data[0]["nofacing"]), (1, "torso_g", 3, 1))
        self.assertAlmostEqual(data[0]["spread"], 0.0, places=6)
        self.assertAlmostEqual(data[0]["rel"][0], 1.0, places=5)
        self.assertEqual((data[1]["cat"], data[1]["bone"], data[1]["count"], data[1]["nofacing"]), (2, "a_bone", 1, 0))

    def test_mounts_json_errors_go_to_stderr_and_stdout_stays_empty(self):
        rc, out, err = run_err(["--mounts", "--json", "/nonexistent.txt"])
        self.assertEqual(rc, 2)
        self.assertEqual(out, [])
        self.assertIn("no log: /nonexistent.txt", err)

    def test_json_null_rel_and_spread_for_all_nofacing_and_empty_list_without_mounts(self):
        rc, out = run_log([MOUNT_NOFACING], ["--mounts", "--json"])
        self.assertEqual(json.loads(out[0]), [{"cat": 1, "bone": "torso_g", "count": 1, "nofacing": 1, "rel": None, "spread": None}])
        rc, out = run_log([LOADED], ["--mounts", "--json"])
        self.assertEqual(json.loads(out[0]), [])


class Hw38GoldenTests(unittest.TestCase):
    """A window with none of the HW38 fields prints the report captured before HW38 (line for line)."""

    def test_window_without_new_fields_prints_the_pre_hw38_report(self):
        rc, out = run_log(GOLDEN_BODIES)
        self.assertEqual(rc, 0)
        self.assertEqual(out, GOLDEN_REPORT)


def split40(line, seq):
    """HW40: one full REACH line as the three KPM-cut pieces of holster 0.20.2-dev: piece 1 is the head up to H=(..) with ` #<seq>`,
    piece 2 `REACH+ #<seq> owner=.. T=(..)..wrote=N`, piece 3 `REACH+ #<seq> owner=.. wr=..` (the rest). Joined, they are the line again."""
    head, after_t = line.split(" T=(", 1)
    mid, tail = ("T=(" + after_t).split(" wr=", 1)
    return ["%s #%d" % (head, seq), "REACH+ #%d owner=1a %s" % (seq, mid), "REACH+ #%d owner=1a wr=%s" % (seq, tail)]


SPLIT_A = reach38(f=0.20, vo=0.080, pb=3.2, tw=80, sw=60, g=85, cap=90, ex=0.62, of_=0.00)
SPLIT_B = reach38(f=1.00, vo=0.000, pb=0.0, tw=40, sw=30, g=70, cap=90, ex=0.00, of_=1.00)


class Hw40SplitReachTests(unittest.TestCase):
    """HW40 (holster 0.20.2-dev): a REACH step cut into three log lines by the KPM 232-byte limit joins back to the old single line."""

    def test_three_pieces_for_two_seqs_with_a_slide_between_match_the_single_line_window(self):
        single = [stamped(SPLIT_A, 1), stamped(slide38("start", gob=0x2A, left=1), 2), stamped(SPLIT_B, 3), stamped(slide38("end"), 4)]
        a, b = split40(SPLIT_A, 1), split40(SPLIT_B, 2)
        split = [stamped(a[0], 1), stamped(slide38("start", gob=0x2A, left=1), 2), stamped(a[1], 3), stamped(b[0], 4),
                 stamped(a[2], 5), stamped(b[1], 6), stamped(slide38("end"), 7), stamped(b[2], 8)]
        rc_s, out_s = run_log(single)
        rc_p, out_p = run_log(split)
        self.assertEqual(rc_p, rc_s)
        self.assertEqual(out_p[1:], out_s[1:])   # line 1 carries the raw line count (8 vs 4); the rest of the report matches line for line
        reach = [x for x in out_p if x.startswith("REACH found=")]
        self.assertEqual(len(reach), 1, out_p)
        self.assertIn("REACH found=0 steps=2 ", reach[0])
        self.assertNotIn("partial=", reach[0])
        _, _, main_s = parse(single)
        _, _, main_p = parse(split)
        self.assertEqual(main_p["reach"], main_s["reach"])
        self.assertEqual(main_p["vo"], main_s["vo"])
        self.assertEqual(main_p["pb"], main_s["pb"])
        self.assertEqual(main_p["reach_partial"], 0)

    def test_missing_piece_three_is_partial_and_steps_count_only_complete_steps(self):
        a, b = split40(SPLIT_A, 1), split40(SPLIT_B, 2)
        bodies = [stamped(a[0], 1), stamped(a[1], 2), stamped(a[2], 3), stamped(b[0], 4), stamped(b[1], 5)]   # seq 2 has no piece 3
        rc, out = run_log(bodies)
        reach = [x for x in out if x.startswith("REACH found=")]
        self.assertIn("REACH found=0 steps=1 ", reach[0])
        self.assertIn(" partial=1", reach[0])
        _, _, main = parse(bodies)
        self.assertEqual(main["reach_partial"], 1)
        self.assertEqual(len(main["reach"]), 1)
        rc, out = run_log([stamped(b[0], 1), stamped(b[2], 2)])   # seq 1 has no piece 2: head and piece 3 only
        self.assertIn("REACH found=0 steps=0 ", [x for x in out if x.startswith("REACH found=")][0])
        self.assertIn(" partial=1", [x for x in out if x.startswith("REACH found=")][0])

    def test_orphan_continuations_are_dropped_silently_and_counted_as_partial(self):
        a, b = split40(SPLIT_A, 1), split40(SPLIT_B, 2)
        bodies = [stamped(a[1], 1), stamped(a[2], 2), stamped(b[0], 3), stamped(b[1], 4), stamped(b[2], 5)]   # seq 1's piece 1 is before the window
        _, skipped, main = parse(bodies)
        self.assertEqual(skipped, [])
        self.assertEqual(main["reach_partial"], 1)
        self.assertEqual(len(main["reach"]), 1)
        rc, out = run_log(bodies)
        self.assertIn(" partial=1", [x for x in out if x.startswith("REACH found=")][0])
        # a continuation written before its piece 1 is an orphan too: the piece 1 that follows has no continuations after it
        _, skipped, main = parse([stamped(a[1], 1), stamped(a[2], 2), stamped(a[0], 3)])
        self.assertEqual((skipped, main["reach_partial"], len(main["reach"])), ([], 1, 0))

    def test_sm_l15_passes_on_a_split_window_with_vo_mid_carry_and_zero_at_edges(self):
        ok = [reach38(f=0.00, vo=0.000, pb=0.0), reach38(f=0.50, vo=0.110, pb=0.0), reach38(f=1.00, vo=0.000, pb=0.0),
              reach38(f=0.20, vo=0.080, pb=3.2), reach38(f=0.30, vo=0.000, pb=2.0)]
        bodies = [stamped(p, i) for i, line in enumerate(ok) for p in split40(line, i + 1)]
        rc, out = run_log(bodies, ["--row", "SM-L15"])
        self.assertEqual(rc, 0, out)
        self.assertEqual(out[-1], "SM-L15 verdict: PASS")
        self.assertIn("SM-L15 vo=0 at reach/release: PASS vo=0 at reach/release on all 5 lines (vo<0.005 m counts as 0)", out)
        self.assertIn("SM-L15 vo peaks mid-carry: PASS vo max=0.110 mid-carry over 3 line(s)", out)
        self.assertIn("SM-L15 pb>0 only at f<=0.30: PASS pb>0 only at f<=0.30 (max pb=3.2 over 5 line(s))", out)

    def test_sm_l11_reads_tw_sw_and_g_cap_from_piece_three(self):
        bodies = [stamped(p, 1) for p in split40(reach38(tw=80, sw=60, g=85, cap=90), 1)]
        rc, out = run_log(bodies, ["--row", "SM-L11"])
        self.assertEqual(rc, 0, out)
        self.assertIn("SM-L11 tw<=80: PASS max 80 over 1 line(s)", out)
        self.assertIn("SM-L11 sw<=60: PASS max 60 over 1 line(s)", out)
        self.assertIn("SM-L11 g-cap<20: PASS max -5 over 1 line(s)", out)
        rc, out = run_log([stamped(p, 1) for p in split40(reach38(g=110, cap=90), 1)], ["--row", "SM-L11"])
        self.assertEqual(rc, 1, out)
        self.assertIn("SM-L11 g-cap<20: FAIL max 20 over 1 line(s)", out)
        rc, out = run_log([stamped(p, 1) for p in split40(SPLIT_A, 1)])
        self.assertIn(" g max=85 over_cap=0", [x for x in out if x.startswith("REACH found=")][0])

    def test_old_single_line_window_is_unchanged_and_shows_no_partial(self):
        window = [(i + 1, s) for i, s in enumerate(GOLDEN_BODIES)]
        joined, partial = H._join_reach(window)
        self.assertEqual(joined, window)
        self.assertEqual(partial, 0)
        rc, out = run_log(GOLDEN_BODIES)
        self.assertEqual(rc, 0)
        self.assertFalse(any("partial=" in x for x in out), out)


# ---- HW41 (holster 0.20.3-dev): STOWHOLD lines, the summary line and --row SM-L16 ----

# Captured BEFORE the HW41 edit (pre-edit checker output on this window): the default report and --row SM-L15 must not change for it.
OLD_STYLE_BODIES = [
    "T=1000 f=- holster/main holster 0.20.2-dev loaded (probe)",
    "T=1001 f=- holster/place SLIDE start dir=draw owner=1f gob=2a left=1 from=0.00 ms=520 dly=60",
    "T=1002 f=- holster/place SLIDE start dir=stow owner=1f gob=2b left=0 from=0.00 ms=520",
    "T=1003 f=- holster/place SLIDE end dir=draw owner=1f ms=520 frames=30 maxdt=20",
    "T=1004 f=- holster/place SLIDE end dir=stow owner=1f ms=520 frames=21 maxdt=49",
    "T=1005 f=- holster/main REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 u=1.00 lt=(0,0,0) err=0.010 "
    "persist=0 wrote=7 wr=88 hg=1 f=0.20 dzA=0.22 fy=-0.10 fx=0.05 g=70 cap=90 hold=0 topo=hip bk=1 tw=40 sw=30 ex=0.62 of=0.00 vo=0.080 pb=3.2",
    "T=1006 f=- holster/main REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 u=1.00 lt=(0,0,0) err=0.010 "
    "persist=0 wrote=7 wr=88 hg=1 f=0.50 dzA=0.22 fy=-0.10 fx=0.05 g=70 cap=90 hold=0 topo=hip bk=1 tw=40 sw=30 ex=0.00 of=0.50 vo=0.110 pb=0.0",
    "T=1007 f=- holster/main REACH owner=1a w=0.50 reach=1.00 S=(0,0,0) E=(0,0,0) H=(0,0,0) T=(0,0,0) Enew=(0,0,0) side=R cat=2 u=1.00 lt=(0,0,0) err=0.010 "
    "persist=0 wrote=7 wr=88 hg=1 f=1.00 dzA=0.22 fy=-0.10 fx=0.05 g=70 cap=90 hold=0 topo=hip bk=1 tw=40 sw=30 ex=0.00 of=1.00 vo=0.000 pb=0.0",
    "T=1008 f=- holster/main SLIDE cut reason=first owner=1f gob=2a left=1 dir=draw frac=0.10",
]
OLD_STYLE_REPORT = [
    "HOLSTER lines=9 parsed=4 skipped=0",
    "status: loaded=yes 0.20.2-dev | cap reached=no | BONETEST=absent | TID gobupdate=absent place=absent | RELOAD 0",
    "COMB edges=0 creatures=0",
    "ATTACH n=0 creatures=0 tags: none",
    "SWAP total=0",
    "GOBREUSE total=0",
    "FLOURISH open=0 close=0",
    "SLIDE start=2 end=2 draw=1 stow=1 frames min=21 max=30 maxdt max=49",
    "SLIDE dly n=1 max=60 per Gob: gob=0x2a L n=1 max=60",
    "PARTYHOLD on=0 off=0",
    "MOVEHOLD total=0",
    "DELAY cuts=0 combat=0 flourish=0",
    "REACH found=0 steps=3 persist1=0 wrote_both=3 refuse=none err min=0.010 med=0.010 max=0.010 reach<1=0 wrist=3 hg=3 g max=70 over_cap=0 cut=first:1 "
    "topo=hip+bk:3 tw max=40 sw max=30 twisted=3 ex max=0.62 extracting=1 of max=1.00",
    "REACH HW38 vo max=0.110 vo>0 outside mid-carry=0 (0.0% of 3 lines) | pb max=3.2 pb>0 at f>0.30=0 (of 3 lines)",
    "REACH min/median/max: wr 88/88/88 | f 0.20/0.50/1.00 | dzA 0.22/0.22/0.22 | fy -0.10/-0.10/-0.10 | fx 0.05/0.05/0.05",
    "ANIMRES n=0 distinct=0 top10: none",
    "ANIMSEND n=0 distinct=0 top10: none",
    "GOB lines=0 gobs=0",
]
OLD_STYLE_ROW_L15 = [
    "SM-L15 vo=0 at reach/release: PASS vo=0 at reach/release on all 3 lines (vo<0.005 m counts as 0)",
    "SM-L15 vo peaks mid-carry: PASS vo max=0.110 mid-carry over 2 line(s)",
    "SM-L15 pb>0 only at f<=0.30: PASS pb>0 only at f<=0.30 (max pb=3.2 over 3 line(s))",
    "SM-L15 verdict: PASS",
]
L16_A = "SM-L16 hold on:"
L16_B = "SM-L16 weapon stays on the hand until the hand is near the mount:"
L16_C = "SM-L16 lands on the mount:"


def stowhold(pathf=0.90, wd=0.000, dm=0.300, lf=0.20, hold=1, p=0.50, carry=0.50):
    """A HW41 STOWHOLD line in the DLL's print format (p and lf %.2f, pathF / wd / dm %.3f, carryW %.2f)."""
    return "STOWHOLD p=%.2f pathF=%.3f carryW=%.2f wd=%.3f dm=%.3f lf=%.2f hold=%d" % (p, pathf, carry, wd, dm, lf, hold)


# planted PASS window: hold on; wd 0 while the hand is past the landing (pathF 0.9 and 0.5); at the mount (pathF 0.01) wd == dm
STOW_PASS = [stowhold(pathf=0.90, wd=0.000, dm=0.300), stowhold(pathf=0.50, wd=0.000, dm=0.250),
             stowhold(pathf=0.20, wd=0.000, dm=0.200), stowhold(pathf=0.01, wd=0.050, dm=0.050)]
STOW_NEAR = stowhold(pathf=0.01, wd=0.050, dm=0.050)   # a line at the mount, so (c) is judged and the verdict is PASS or FAIL


class Hw41StowHoldTests(unittest.TestCase):
    """HW41 STOWHOLD: the parse, the summary line (default report, only when STOWHOLD lines exist) and --row SM-L16."""

    def verdict(self, bodies, rc, word):
        got_rc, out = run_log(bodies, ["--row", "SM-L16"])
        self.assertEqual(got_rc, rc, out)
        self.assertEqual(out[-1], "SM-L16 verdict: %s" % word, out)
        self.assertTrue(all(line.startswith("SM-L16 ") for line in out), out)
        return out

    def test_planted_pass_window(self):
        out = self.verdict(STOW_PASS, 0, "PASS")
        self.assertIn(L16_A + " PASS hold=1 on 4 of 4 line(s)", out)
        self.assertIn(L16_B + " PASS wd <= 0.005 m on all 2 judged line(s) (max wd 0.000 m at pathF 0.900)", out)
        self.assertIn(L16_C + " PASS |wd-dm| <= 0.020 m on all 1 line(s) with pathF <= 0.02 (max 0.000 m)", out)

    def test_planted_fail_window_wd_019_at_pathf_09(self):
        out = self.verdict([stowhold(pathf=0.90, wd=0.190, dm=0.300), STOW_NEAR], 1, "FAIL")
        self.assertIn(L16_A + " PASS hold=1 on 2 of 2 line(s)", out)
        self.assertIn(L16_B + " FAIL wd max 0.190 m at pathF 0.900 (1 of 1 judged line(s) above 0.005 m)", out)

    def test_wd_boundary_0005_passes_and_0006_fails(self):
        out = self.verdict([stowhold(pathf=0.90, wd=0.005, dm=0.300), STOW_NEAR], 0, "PASS")
        self.assertIn(L16_B + " PASS wd <= 0.005 m on all 1 judged line(s) (max wd 0.005 m at pathF 0.900)", out)
        out = self.verdict([stowhold(pathf=0.90, wd=0.006, dm=0.300), STOW_NEAR], 1, "FAIL")
        self.assertIn(L16_B + " FAIL wd max 0.006 m at pathF 0.900 (1 of 1 judged line(s) above 0.005 m)", out)

    def test_pathf_at_lf_plus_001_is_not_judged_and_lf_plus_0011_is_judged(self):
        out = self.verdict([stowhold(pathf=0.210, wd=0.100, dm=0.300, lf=0.20), STOW_NEAR], 3, "NO-DATA")
        self.assertIn(L16_B + " NO-DATA no hold=1 line with pathF > lf+0.01", out)
        out = self.verdict([stowhold(pathf=0.211, wd=0.100, dm=0.300, lf=0.20), STOW_NEAR], 1, "FAIL")
        self.assertIn(L16_B + " FAIL wd max 0.100 m at pathF 0.211 (1 of 1 judged line(s) above 0.005 m)", out)

    def test_lands_on_the_mount_judges_wd_against_dm(self):
        out = self.verdict([stowhold(pathf=0.90, wd=0.000), stowhold(pathf=0.01, wd=0.150, dm=0.170)], 0, "PASS")
        self.assertIn(L16_C + " PASS |wd-dm| <= 0.020 m on all 1 line(s) with pathF <= 0.02 (max 0.020 m)", out)
        out = self.verdict([stowhold(pathf=0.90, wd=0.000), stowhold(pathf=0.01, wd=0.150, dm=0.190)], 1, "FAIL")
        self.assertIn(L16_C + " FAIL |wd-dm| max 0.040 m at pathF 0.010 (wd 0.150 dm 0.190; 1 of 1 line(s) above 0.020 m)", out)

    def test_hold_off_is_no_data_never_fail(self):
        out = self.verdict([stowhold(pathf=0.90, wd=0.190, hold=0), stowhold(pathf=0.01, wd=0.200, dm=0.050, hold=0)], 3, "NO-DATA")
        self.assertIn(L16_A + " NO-DATA hold=0 on all 2 line(s): hold off or old build", out)
        self.assertIn(L16_B + " NO-DATA no hold=1 line with pathF > lf+0.01", out)
        self.assertIn(L16_C + " NO-DATA no hold=1 line with pathF <= 0.02", out)

    def test_empty_window_is_no_data(self):
        out = self.verdict([LOADED], 3, "NO-DATA")
        self.assertIn(L16_A + " NO-DATA no STOWHOLD line (old build, or no stow in window)", out)

    def test_unknown_row_message_lists_sm_l16(self):
        rc, out, err = run_err(["--row", "SM-L17", "/nonexistent.txt"])
        self.assertEqual(rc, 2)
        self.assertIn("SM-L16", err)

    def test_parse_reads_fields_and_tolerates_trailing_fields(self):
        recs, skipped, main = H.parse_lines([(1, stowhold(pathf=0.5, wd=0.012, dm=0.2) + " ext=1 foo=2.5"), (2, stowhold(hold=0))])
        self.assertEqual(skipped, [])
        self.assertEqual(recs, [])   # a window fact, never a record
        self.assertEqual(len(main["stowhold"]), 2)
        first = main["stowhold"][0]
        self.assertEqual((first["pathF"], first["wd"], first["dm"], first["lf"], first["hold"]), (0.5, 0.012, 0.2, 0.2, 1))

    def test_unparsable_stowhold_is_skipped_and_counted(self):
        rc, out = run_log(["STOWHOLD p=0.50 pathF=0.900"])
        self.assertEqual(rc, 0)
        self.assertEqual(out[0], "HOLSTER lines=1 parsed=0 skipped=1")
        self.assertFalse(any(line.startswith("STOWHOLD") for line in out), out)

    def test_summary_line_only_with_stowhold_lines(self):
        rc, out = run_log(STOW_PASS)
        self.assertEqual(rc, 0)
        self.assertIn("STOWHOLD n=4 hold=4/4 | wd max while pathF>lf: 0.0 cm | wd max overall: 5.0 cm", out)
        self.assertEqual(sum(1 for line in out if line.startswith("STOWHOLD ")), 1)

    def test_summary_fail_window_and_hold_off_and_not_past_landing(self):
        rc, out = run_log([stowhold(pathf=0.90, wd=0.190), STOW_NEAR])
        self.assertIn("STOWHOLD n=2 hold=2/2 | wd max while pathF>lf: 19.0 cm | wd max overall: 19.0 cm", out)
        rc, out = run_log([stowhold(pathf=0.90, wd=0.190, hold=0)])
        self.assertIn("STOWHOLD n=1 hold=0/1 | wd max while pathF>lf: 19.0 cm | wd max overall: 19.0 cm", out)
        rc, out = run_log([stowhold(pathf=0.10, wd=0.020)])
        self.assertIn("STOWHOLD n=1 hold=1/1 | wd max while pathF>lf: n/a | wd max overall: 2.0 cm", out)

    def test_old_style_window_report_and_row_are_unchanged(self):
        rc, out = run_log(OLD_STYLE_BODIES)
        self.assertEqual(rc, 0)
        self.assertEqual(out, OLD_STYLE_REPORT)
        self.assertFalse(any(line.startswith("STOWHOLD") for line in out), out)
        rc, out = run_log(OLD_STYLE_BODIES, ["--row", "SM-L15"])
        self.assertEqual(out, OLD_STYLE_ROW_L15)

    def test_mounts_json_stays_pure_json_with_stowhold_lines(self):
        rc, out = run_log(STOW_PASS + [mount_line(rel=rel_deg(0))], ["--mounts", "--json"])
        self.assertEqual(rc, 0)
        self.assertEqual(len(out), 1)
        self.assertEqual([d["bone"] for d in json.loads(out[0])], ["torso_g"])

    def test_worst_case_with_stowhold_stays_within_51(self):
        """The 47-line worst case plus the three HW38 lines and the HW41 STOWHOLD line: the real cap is 51."""
        bodies = worst_case()[:-1] + [slide38("start", gob=0x2A, left=1, dly=60), slide38("end"), reach38(f=0.20, vo=0.080, pb=3.2),
                                      stowhold(pathf=0.90, wd=0.000), CAP]
        rc, out = run_log(bodies)
        self.assertEqual(rc, 0)
        self.assertLessEqual(len(out), 51)
        self.assertEqual(sum(1 for line in out if line.startswith("STOWHOLD ")), 1)


if __name__ == "__main__":
    unittest.main()
