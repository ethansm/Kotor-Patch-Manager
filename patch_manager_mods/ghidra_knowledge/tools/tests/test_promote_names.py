"""Tests for promote_names.py (INGEST I4: promote.tsv -> name_map).

Run from the tools dir: python3 -m unittest tests.test_promote_names -v
The KDB is a temp sqlite file per test; the real KDB is never opened.
"""
import io
import os
import sqlite3
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
import ingest_names as inn  # noqa: E402
import promote_names as pn  # noqa: E402

PROMOTE = [
    {"addr": "0x00401000", "kind": "function", "name": "NewFn", "src": "a.h:1", "verdict": "consistent",
     "quote": "q1"},
    {"addr": "0x00402000", "kind": "function", "name": "Same", "src": "a.h:2", "verdict": "consistent",
     "quote": "q2"},
    {"addr": "0x009a0000", "kind": "global", "name": "Clash", "src": "b.h:1", "verdict": "consistent",
     "quote": "q3"},
    {"addr": "0x009a2000", "kind": "global", "name": "RetryOnly", "src": "b.h:2", "verdict": "consistent",
     "quote": "q4"},
]
KEY = [
    {"card": "C1", "addr": "0x00401000", "name": "NewFn", "is_decoy": "0"},
    {"card": "C2", "addr": "0x00402000", "name": "Same", "is_decoy": "0"},
    {"card": "C3", "addr": "0x009a0000", "name": "Clash", "is_decoy": "0"},
    {"card": "C4", "addr": "0x009a2000", "name": "RetryOnly", "is_decoy": "0"},
    {"card": "C5", "addr": "0x009a2000", "name": "RetryOnly", "is_decoy": "1"},
]


class PromoteTest(unittest.TestCase):
    def setUp(self):
        self.td = tempfile.TemporaryDirectory()
        self.kdb = os.path.join(self.td.name, "k.db")
        con = sqlite3.connect(self.kdb)
        con.execute("CREATE TABLE name_map(steam_addr text primary key,name text,source text,"
                    "confidence text,evidence text,date text)")
        con.execute("insert into name_map values ('0x00402000','Same','x','high','e','2026-10-01')")
        con.execute("insert into name_map values ('0x009a0000','Other','x','high','e','2026-10-01')")
        con.commit()
        con.close()
        self.prom = os.path.join(self.td.name, "promote.tsv")
        inn.write_tsv(self.prom, PROMOTE, inn.PROMOTE_COLS)
        self.key = os.path.join(self.td.name, "key.tsv")
        inn.write_tsv(self.key, KEY, ("card", "addr", "name", "is_decoy"))
        with open(os.path.join(self.td.name, "verdicts_1.tsv"), "w") as fh:
            fh.write("card\tverdict\tquote\nC1\tconsistent\tq\nC2\tconsistent\tq\nC3\tconsistent\tq\n"
                     "C4\tunsure\tq\nC5\tconsistent\tq\n")

    def tearDown(self):
        self.td.cleanup()

    def rows(self):
        con = sqlite3.connect(self.kdb)
        try:
            return {r[0]: r for r in con.execute("select * from name_map")}
        finally:
            con.close()

    def run_main(self, *extra):
        buf = io.StringIO()
        rc = pn.main(["--promote", self.prom, "--source", "ingest-modcode", "--kdb", self.kdb,
                      "--date", "2026-10-27", *extra], out=buf, err=io.StringIO())
        return rc, buf.getvalue()

    def test_dry_run_writes_nothing(self):
        rc, out = self.run_main()
        self.assertEqual(rc, 0)
        self.assertIn("would-insert=2 exists=1 conflict=1", out)
        self.assertEqual(len(self.rows()), 2)

    def test_apply_inserts_new_never_overwrites(self):
        rc, out = self.run_main("--apply")
        self.assertEqual(rc, 0)
        self.assertIn("integrity: ok", out)
        rows = self.rows()
        self.assertEqual(rows["0x00401000"][1:4], ("NewFn", "ingest-modcode", "medium"))
        self.assertIn("src a.h:1", rows["0x00401000"][4])
        self.assertIn("ingest I2 Haiku", rows["0x00401000"][4])
        self.assertEqual(rows["0x009a0000"][1], "Other")
        self.assertIn("conflict 0x009a0000 Clash (name_map has Other)", out)

    def test_stage_named_in_evidence(self):
        self.run_main("--apply", "--stage", "I3")
        self.assertIn("ingest I3 Haiku", self.rows()["0x00401000"][4])

    def test_strict_drops_retry_only_and_ignores_decoy_card(self):
        v = os.path.join(self.td.name, "verdicts_[1-8].tsv")
        rc, out = self.run_main("--strict-key", self.key, "--strict-verdicts", v, "--apply")
        self.assertEqual(rc, 0)
        self.assertIn("in=4 kept=3", out)
        self.assertNotIn("0x009a2000", self.rows())

    def test_strict_flags_go_together(self):
        rc, _ = self.run_main("--strict-key", self.key)
        self.assertEqual(rc, 2)


if __name__ == "__main__":
    unittest.main()
