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
STATUS = "status: loaded=yes 0.1.0-dev | cap reached=yes max_lines=60000 | BONETEST=patched x1"


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


def fixture():
    """Every line kind, stamped and bare mixed. Gob 0x9000 turns 20 deg about z; its third sample is the same rotation
    with the sign flipped. Gob 0x9100 is constant except z."""
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
        CAP,
    ]
    return [stamped(b, i) if i % 3 == 0 else b for i, b in enumerate(bodies)]


def worst_case():
    """Every section over its cap, plus one unparsable COMB line. The report must stay at 40 lines."""
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
                         Counter({"COMB": 5, "ATTACH": 4, "SWAP": 3, "ANIMRES": 4, "ANIMSEND": 2, "GOB": 5}))
        self.assertEqual(main["loaded"], ["0.1.0-dev"])
        self.assertEqual(main["cap"], ["60000"])
        self.assertEqual(main["bonetest"], Counter({"patched": 1}))

    def test_stamp_is_optional(self):
        a = parse([comb(0x1F0, 0, 1, 1)])[0][0]
        b = parse([stamped(comb(0x1F0, 0, 1, 1))])[0][0]
        self.assertEqual({k: v for k, v in a.items() if k != "line"}, {k: v for k, v in b.items() if k != "line"})

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
        self.assertEqual(self.out[0], "HOLSTER lines=26 parsed=23 skipped=0")
        self.assertEqual(self.out[1], STATUS)

    def test_status_when_absent(self):
        rc, out = run_log(["hud built panel=1"])
        self.assertEqual(out[1], "status: loaded=no | cap reached=no | BONETEST=absent")

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
        self.assertIn("ANIMRES n=4 distinct=2 top10: 0x1f x3, 0x7 x1", self.out)
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

    def test_worst_case_is_capped_at_40_lines(self):
        rc, out = run_log(worst_case())
        self.assertEqual(rc, 0)
        self.assertEqual(len(out), 40)
        self.assertIn("COMB edges=6 creatures=6 (top 4 shown)", out)
        self.assertIn("ATTACH n=20 creatures=5 tags: 't'=20 (first 3 creatures shown)", out)
        self.assertTrue(any(s.startswith("ANIMRES n=12 distinct=12 top10:") and s.endswith("(+2 more ids)") for s in out))
        self.assertIn("GOB lines=55 gobs=10 (top 8 by samples shown)", out)
        self.assertTrue(out[-1].startswith("skipped e.g. L") and "COMB creature=zz" in out[-1])

    def test_missing_log_returns_2(self):
        with tempfile.TemporaryDirectory() as d:
            missing = os.path.join(d, "holster_log.txt")
            rc, out = run([missing])
        self.assertEqual((rc, out), (2, ["no log: %s" % missing]))


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


if __name__ == "__main__":
    unittest.main()
