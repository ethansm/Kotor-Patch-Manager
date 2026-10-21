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
    """Every section over its cap, plus one unparsable COMB line. The report must stay at 46 lines (40 + 3 GOBREUSE/FLOURISH/SLIDE lines + 3 PARTYHOLD/MOVEHOLD/DELAY lines)."""
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

    def test_worst_case_is_capped_at_46_lines(self):
        rc, out = run_log(worst_case())
        self.assertEqual(rc, 0)
        self.assertEqual(len(out), 46)
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

    def test_place_view_caps_gobs_and_report_stays_within_46_lines(self):
        rc, out = run_log(worst_case()[:-1] + place_worst() + [CAP])
        self.assertEqual(rc, 0)
        self.assertLessEqual(len(out), 46)
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


if __name__ == "__main__":
    unittest.main()
