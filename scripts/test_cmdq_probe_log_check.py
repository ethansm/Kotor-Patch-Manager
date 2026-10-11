#!/usr/bin/env python3
"""Tests for scripts/cmdq_probe_log_check.py. Synthetic fixtures only.
Run: python3 scripts/test_cmdq_probe_log_check.py   (run_tests.py discovers it as py:scripts/test_cmdq_probe_log_check.py)"""
import contextlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import cmdq_probe_log_check as C  # noqa: E402

# Synthetic golden lines, one per grammar keyword, KpLog-style "<ms> <patch>/<chan> <payload>".
GOLDEN = {
    "PAUSE": "100 cmdq-probe/med PAUSE mask=3 val=-1",
    "CMD": "101 cmdq-probe/med CMD kind=item actor=1f id=2a tgt=0 real=1",
    "FIRE": "102 cmdq-probe/med FIRE kind=spell actor=0x1f",
    "ROUND": "103 cmdq-probe/med ROUND ev=start round=0x10 owner=ff",
    "CLR": "104 cmdq-probe/med CLR victim=0x3f caller=0xabc q=1,0x2,ff",
    "REEQ": "105 cmdq-probe/med REEQ actor=0x1f",
    "MED": "12345 cmdq-probe/med MED site=entry this=0x1234 arg=0x40000001 base=0x35 flag=0",
}
GOLDEN_FIELDS = {
    "PAUSE": {"mask": 3, "val": -1},
    "CMD": {"kind": "item", "actor": 0x1F, "id": 0x2A, "tgt": 0, "real": 1},
    "FIRE": {"kind": "spell", "actor": 0x1F},
    "ROUND": {"ev": "start", "round": 0x10, "owner": 0xFF},
    "CLR": {"victim": 0x3F, "caller": 0xABC, "q": [1, 2, 0xFF]},
    "REEQ": {"actor": 0x1F},
    "MED": {"site": "entry", "this": 0x1234, "arg": 0x40000001, "base": 0x35, "flag": 0},
}


def med(t, site="entry", base="0x35", flag=0):
    return "%d cmdq-probe/med MED site=%s this=0x1 arg=0x2 base=%s flag=%d" % (t, site, base, flag)


def fire(t, kind="item"):
    return "%d cmdq-probe/med FIRE kind=%s actor=0x1" % (t, kind)


def clr(t, caller="0xabc"):
    return "%d cmdq-probe/med CLR victim=0x3f caller=%s q=3f" % (t, caller)


def start(t):
    return "%d cmdq-probe/med ROUND ev=start round=0x1 owner=0x2" % t


def parse_one(line):
    recs, unknown, malformed = C.parse_lines([(1, line)])
    return recs, unknown, malformed


def run(argv):
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        rc = C.main(argv)
    return rc, out.getvalue()


def run_log(lines, extra=()):
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "cmdq-probe_log.txt"
        p.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")
        return run([str(p), *extra])


class ParseTests(unittest.TestCase):
    def test_each_keyword_parses_exact_fields(self):
        for kw, line in GOLDEN.items():
            with self.subTest(kw=kw):
                recs, unknown, malformed = parse_one(line)
                self.assertEqual((len(recs), unknown, malformed), (1, [], []))
                self.assertEqual(recs[0]["kw"], kw)
                self.assertEqual(recs[0]["f"], GOLDEN_FIELDS[kw])

    def test_hex_with_and_without_0x_parse_the_same(self):
        bare, _, _ = parse_one("1 cmdq-probe/med CMD kind=item actor=1f id=2a tgt=0 real=1")
        prefixed, _, _ = parse_one("1 cmdq-probe/med CMD kind=item actor=0x1f id=0x2a tgt=0x0 real=1")
        upper, _, _ = parse_one("1 cmdq-probe/med CMD kind=item actor=0X1F id=2A tgt=0 real=1")
        self.assertEqual(bare[0]["f"], prefixed[0]["f"])
        self.assertEqual(bare[0]["f"], upper[0]["f"])
        self.assertEqual(bare[0]["f"]["actor"], 31)

    def test_timestamp_forms(self):
        kp, _, _ = parse_one("T=500 f=- cmdq-probe/med REEQ actor=1")
        lvl, _, _ = parse_one("500 cmdq-probe/med WARN REEQ actor=1")
        bare, _, _ = parse_one("REEQ actor=1")
        self.assertEqual(kp[0]["t"], 500.0)
        self.assertEqual(lvl[0]["t"], 500.0)
        self.assertIsNone(bare[0]["t"])

    def test_blank_lines_are_ignored(self):
        recs, unknown, malformed = C.parse_lines([(1, ""), (2, "   "), (3, "100 cmdq-probe/med REEQ actor=1")])
        self.assertEqual((len(recs), unknown, malformed), (1, [], []))

    def test_malformed_and_unknown_are_counted_not_fatal(self):
        lines = [
            "100 cmdq-probe/med CMD kind=bow actor=1 id=1 tgt=0 real=1",   # bad kind
            "101 cmdq-probe/med CMD kind=item actor=1 id=1 tgt=0",         # missing real
            "102 cmdq-probe/med MED site=entry this=1 arg=2 base=3 flag=0 flag=1",  # duplicate key
            "103 cmdq-probe/med REEQ actor",                               # token without =
            "104 cmdq-probe/med PAUSE mask=1 val=0x",                      # invalid decimal
            "105 cmdq-probe/med FOO bar=1",                                # unknown keyword
            "106 cmdq-probe/med REEQ actor=0x1",                           # valid
        ]
        recs, unknown, malformed = C.parse_lines(list(enumerate(lines, 1)))
        self.assertEqual(len(recs), 1)
        self.assertEqual(unknown, [(6, "FOO")])
        self.assertEqual([m[0] for m in malformed], [1, 2, 3, 4, 5])
        self.assertIn("missing real", malformed[1][2])
        self.assertIn("duplicate flag", malformed[2][2])

    def test_empty_input(self):
        self.assertEqual(C.parse_lines([]), ([], [], []))
        s = C.summarize([])
        self.assertEqual((s["lines"], s["parsed"]), (0, 0))
        self.assertEqual(s["flags"], {"M1": 0, "M2": 0, "C1": 0})


class M1Tests(unittest.TestCase):
    def test_refused_medpack_after_entry_is_flagged(self):
        s = C.summarize([med(1000, "entry"), med(2000, "canuse", flag=0)])
        self.assertEqual(s["flags"]["M1"], 1)
        hit = s["hits"]["M1"][0]
        self.assertEqual((hit["line"], hit["ref"]), (2, 1))
        self.assertIn("refused", hit["detail"])
        self.assertIn("entry L1", hit["detail"])

    def test_window_boundary(self):
        self.assertEqual(C.summarize([med(1000, "entry"), med(3000, "canuse", flag=0)])["flags"]["M1"], 1)
        self.assertEqual(C.summarize([med(1000, "entry"), med(3001, "canuse", flag=0)])["flags"]["M1"], 0)

    def test_accepted_canuse_is_not_flagged(self):
        self.assertEqual(C.summarize([med(1000, "entry"), med(1500, "canuse", flag=1)])["flags"]["M1"], 0)

    def test_different_base_is_not_flagged(self):
        self.assertEqual(C.summarize([med(1000, "entry", base="0x35"), med(1500, "canuse", base="0x36", flag=0)])
                         ["flags"]["M1"], 0)

    def test_handle_entry_with_base_zero_pairs_with_any_canuse_base(self):
        s = C.summarize([med(1000, "entry", base="0x0"), med(1500, "canuse", base="0x36", flag=0)])
        self.assertEqual(s["flags"]["M1"], 1)

    def test_canuse_without_earlier_entry_is_not_flagged(self):
        self.assertEqual(C.summarize([med(1500, "canuse", flag=0), med(1000, "entry")])["flags"]["M1"], 0)

    def test_untimed_lines_never_trigger(self):
        s = C.summarize(["MED site=entry this=0x1 arg=0x2 base=0x35 flag=0",
                         "MED site=canuse this=0x1 arg=0x2 base=0x35 flag=0"])
        self.assertEqual(s["flags"]["M1"], 0)


class M2Tests(unittest.TestCase):
    def test_entry_without_item_fire_is_flagged(self):
        s = C.summarize([med(1000), fire(2000, kind="spell")])
        self.assertEqual(s["flags"]["M2"], 1)
        self.assertEqual(s["hits"]["M2"][0]["line"], 1)
        self.assertIn("no FIRE kind=item", s["hits"]["M2"][0]["detail"])

    def test_item_fire_within_window_clears_entry(self):
        self.assertEqual(C.summarize([med(1000), fire(3000)])["flags"]["M2"], 0)

    def test_window_boundary(self):
        self.assertEqual(C.summarize([med(1000), fire(4000)])["flags"]["M2"], 0)
        self.assertEqual(C.summarize([med(1000), fire(4001)])["flags"]["M2"], 1)

    def test_fire_before_entry_does_not_clear(self):
        self.assertEqual(C.summarize([fire(500), med(1000)])["flags"]["M2"], 1)

    def test_canuse_lines_are_not_entries(self):
        self.assertEqual(C.summarize([med(1000, "canuse", flag=1)])["flags"]["M2"], 0)


class C1Tests(unittest.TestCase):
    def test_clr_just_before_combat_start_is_flagged_with_caller(self):
        s = C.summarize([clr(1000, caller="0xdead"), start(1080)])
        self.assertEqual(s["flags"]["C1"], 1)
        hit = s["hits"]["C1"][0]
        self.assertEqual((hit["line"], hit["ref"]), (2, 1))
        self.assertIn("caller=0xdead", hit["detail"])

    def test_window_boundary(self):
        self.assertEqual(C.summarize([clr(1000), start(1100)])["flags"]["C1"], 1)
        self.assertEqual(C.summarize([clr(1000), start(1101)])["flags"]["C1"], 0)

    def test_round_end_is_not_a_start(self):
        self.assertEqual(C.summarize([clr(1000), "1050 cmdq-probe/med ROUND ev=end round=0x1 owner=0x2"])
                         ["flags"]["C1"], 0)

    def test_clr_after_round_start_is_not_flagged(self):
        self.assertEqual(C.summarize([start(1080), clr(1000)])["flags"]["C1"], 0)


class CapTests(unittest.TestCase):
    def _big_log(self):
        lines, t = [], 0
        for _ in range(100):
            t += 10000
            lines.append(med(t, "entry"))                      # M2 hit (no item fire)
            lines.append(med(t + 10, "canuse", flag=0))        # M1 hit (refused after entry)
            lines.append(clr(t + 20))                          # C1 hit with the next ROUND start
            lines.append(start(t + 50))
        lines += ["%d cmdq-probe/med CMD kind=bow actor=1 id=1 tgt=0 real=1" % (t + i) for i in range(50)]
        lines += ["%d cmdq-probe/med FOO x=%d" % (t + i, i) for i in range(50)]
        return lines

    def test_default_view_is_capped_at_60_lines(self):
        rc, out = run_log(self._big_log())
        self.assertEqual(rc, 0)
        rows = out.splitlines()
        self.assertLessEqual(len(rows), 60)
        self.assertIn("M2 item-never-fired 100", rows)
        self.assertIn("  ... +88 more", rows)
        self.assertIn("MALFORMED 50", rows)
        self.assertIn("UNKNOWN 50", rows)

    def test_all_lifts_the_cap(self):
        rc, out = run_log(self._big_log(), extra=["--all"])
        self.assertEqual(rc, 0)
        rows = out.splitlines()
        self.assertGreater(len(rows), 60)
        # 100 hits each for M1, M2, C1, plus 50 MALFORMED and 50 UNKNOWN samples, all listed.
        self.assertEqual(sum(1 for r in rows if r.startswith("  L")), 300 + 50 + 50)
        self.assertNotIn("  ... +88 more", rows)


class CliTests(unittest.TestCase):
    def test_missing_log_exits_2(self):
        rc, out = run(["/nonexistent/dir/cmdq-probe_log.txt"])
        self.assertEqual(rc, 2)
        self.assertIn("no log:", out)

    def test_empty_log_exits_0(self):
        rc, out = run_log([])
        self.assertEqual(rc, 0)
        self.assertIn("CMDQ-PROBE lines=0 parsed=0 unknown=0 malformed=0", out)
        self.assertIn("FLAGS M1=0 M2=0 C1=0", out)

    def test_channels_and_flags_lines(self):
        rc, out = run_log([GOLDEN["PAUSE"], GOLDEN["MED"], "bogus line here", med(1000, "entry")])
        self.assertEqual(rc, 0)
        self.assertIn("CHANNELS PAUSE=1 CMD=0 FIRE=0 ROUND=0 CLR=0 REEQ=0 MED=2", out)
        self.assertIn("UNKNOWN 1", out)
        self.assertIn("M2 item-never-fired 2", out)

    def test_malformed_lines_do_not_stop_flags(self):
        rc, out = run_log(["100 cmdq-probe/med CMD kind=bow", clr(1000), start(1050)])
        self.assertEqual(rc, 0)
        self.assertIn("malformed=1", out)
        self.assertIn("C1 combat-start-wipe 1", out)

    def test_json_output(self):
        rc, out = run_log([med(1000, "entry"), med(2000, "canuse", flag=0)], extra=["--json"])
        self.assertEqual(rc, 0)
        doc = json.loads(out)
        self.assertEqual(doc["flags"], {"M1": 1, "M2": 1, "C1": 0})
        self.assertEqual(doc["hits"]["M1"][0]["ref"], 1)
        self.assertEqual(doc["channels"]["MED"], 2)
        self.assertEqual(doc["malformed"], 0)


if __name__ == "__main__":
    unittest.main()
