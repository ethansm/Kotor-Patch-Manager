"""REA live-evidence section on `cards` (report-only): --rea, --rea-ledger-dir, --rea-topic-prefix, --out.

Fixtures come from tests/test_batch.py; the card setup mirrors CardLeakAuditTests in tests/test_batch_ledger_cards.py.
Run from tools/:  python3 -m unittest tests.test_rea_cards
"""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_batch import B1_SPEC, Base, batch, bd, run, write_candidates  # noqa: E402
from test_batch_ledger_cards import split_cards  # noqa: E402

LEDGER_COLS = ["addr", "kind", "name", "confidence", "evidence", "topic", "date"]
SECTION = "### Live evidence (REA, report-only)"
NAME = "CSWSFixture::Probe (test row)"


def ledger_row(addr, topic, name, confidence="confirmed", kind="function", evidence="procedure_assembly test"):
    return {"addr": "0x%08x" % addr, "kind": kind, "name": name, "confidence": confidence,
            "evidence": evidence, "topic": topic, "date": "2026-10-15"}


def write_ledger(ledger_dir, rows, fname="20261015_test.tsv"):
    ledger_dir.mkdir(parents=True, exist_ok=True)
    lines = ["\t".join(LEDGER_COLS)] + ["\t".join(r[c] for c in LEDGER_COLS) for r in rows]
    (ledger_dir / fname).write_text("\n".join(lines) + "\n", encoding="utf-8")


class LoadReaRowsTests(unittest.TestCase):
    def test_filters_by_topic_prefix_skips_malformed_addr_and_sorts_per_address(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        ledger = Path(tmp.name) / "rea_findings"
        write_ledger(ledger, [
            ledger_row(0x510000, "B1-q1", "b", confidence="probable"),
            ledger_row(0x510000, "B1-q1", "a", confidence="confirmed"),
            ledger_row(0x510000, "other B1-q1", "foreign"),
            {**ledger_row(0x510000, "B1-q1", "bad"), "addr": "0xZZZZ"},
        ])
        rows = batch.load_rea_rows(ledger, "B1-q")
        self.assertEqual(list(rows), [0x510000])
        self.assertEqual([r["name"] for r in rows[0x510000]], ["a", "b"])


class ReaCardsTests(Base):
    def setUp(self):
        super().setUp()
        self.bdir = self.root / "batches" / "B1"
        # same fixture setup as CardLeakAuditTests: a content-free fingerprint for every Steam function
        for a, f in self.world.steam.items():
            if a not in self.world.steam_fp:
                self.world.fp("steam", a, f["retn"], size=f["size"])
        self.world.build()
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        ev = list(bd.iter_tsv(self.bdir / "evidence.tsv"))
        self.review = [e for e in ev if e["gate_class"] == "REVIEW"]
        self.assertGreaterEqual(len(self.review), 4)
        facts = [{"gog": e["gog"], "fact": "slot %d of the command table" % (10 + i)} for i, e in enumerate(self.review)]
        batch.write_tsv(self.bdir / "card_facts.tsv", ["gog", "fact"], facts)
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = Path(tmp.name)
        self.ledger = self.tmp / "rea_findings"

    def build_cards(self, *flags, out=None):
        """Run `cards B1 <flags>` -> (cards {id: lines}, key {id: row}, raw chunk bytes {file: bytes})."""
        rc, text = run("cards", "B1", *flags)
        self.assertEqual(rc, 0, text)
        cdir = Path(out) if out else self.bdir / "cards"
        chunks = sorted(cdir.glob("chunk_*.md"))
        cards = {}
        for p in chunks:
            cards.update(split_cards(p.read_text(encoding="utf-8")))
        key = {k["card_id"]: k for k in bd.iter_tsv(cdir / "key.tsv")}
        return cards, key, {p.name: p.read_bytes() for p in chunks}

    def real_ids(self, key):
        return sorted(c for c, k in key.items() if k["kind"] == "real")

    # ------------------------------------------------------------------------------------------- (a) flags absent
    def test_a_without_rea_chunks_are_byte_identical_and_have_no_live_section(self):
        _c1, key, raw1 = self.build_cards()
        _c2, _k2, raw2 = self.build_cards()
        self.assertTrue(raw1)
        self.assertEqual(raw1, raw2)
        blob = b"".join(raw1.values())
        self.assertNotIn(b"Live evidence", blob)
        self.assertNotIn(b"no REA rows", blob)
        # a ledger row at a card's address is ignored without --rea, even with --rea-ledger-dir present
        addr = bd.parse_addr(key[self.real_ids(key)[0]]["steam_addr"])
        write_ledger(self.ledger, [ledger_row(addr, "B1-q1", NAME)])
        _c3, _k3, raw3 = self.build_cards("--rea-ledger-dir", str(self.ledger))
        self.assertEqual(raw1, raw3)

    # ------------------------------------------------------------------------------------------- (b) row on its card
    def test_b_rea_row_shows_on_its_card_and_no_other_card_claims_it(self):
        _c, key, _raw = self.build_cards()
        target = self.real_ids(key)[0]
        addr = bd.parse_addr(key[target]["steam_addr"])
        write_ledger(self.ledger, [ledger_row(addr, "B1-q1", NAME)])
        cards, key, _raw = self.build_cards("--rea", "--rea-ledger-dir", str(self.ledger))
        holders = {c for c, ls in cards.items() if any(NAME in l for l in ls)}
        self.assertEqual(holders, {c for c in cards if bd.parse_addr(key[c]["steam_addr"]) == addr})
        self.assertIn(target, holders)
        self.assertIn("- function | %s | confirmed | procedure_assembly test" % NAME, cards[target])
        self.assertNotIn("- (no REA rows for 0x%08x)" % addr, cards[target])
        for c, ls in cards.items():
            self.assertEqual(ls.count(SECTION), 1, c)
            if c not in holders:
                self.assertIn("- (no REA rows for 0x%08x)" % bd.parse_addr(key[c]["steam_addr"]), ls)

    # ------------------------------------------------------------------------------------------- (c) foreign topic
    def test_c_row_with_foreign_topic_is_not_shown(self):
        _c, key, _raw = self.build_cards()
        addr = bd.parse_addr(key[self.real_ids(key)[0]]["steam_addr"])
        write_ledger(self.ledger, [ledger_row(addr, "GOG-cluster-names", NAME),
                                   ledger_row(addr, "old B1-q1", NAME + " again")])
        cards, key, _raw = self.build_cards("--rea", "--rea-ledger-dir", str(self.ledger))
        for c, ls in cards.items():
            self.assertNotIn(NAME, "\n".join(ls), c)
            self.assertIn("- (no REA rows for 0x%08x)" % bd.parse_addr(key[c]["steam_addr"]), ls, c)

    # ------------------------------------------------------------------------------------------- (d) --out
    def test_d_out_writes_elsewhere_and_leaves_bdir_cards_untouched(self):
        _c, _k, baseline = self.build_cards()
        cdir = self.bdir / "cards"
        before = {p.name: p.read_bytes() for p in cdir.iterdir() if p.is_file()}
        out = self.tmp / "elsewhere" / "cards"                   # does not exist yet: must be created
        _c2, _k2, raw = self.build_cards("--out", str(out), out=out)
        self.assertTrue((out / "key.tsv").is_file())
        self.assertEqual(raw, baseline)
        after = {p.name: p.read_bytes() for p in cdir.iterdir() if p.is_file()}
        self.assertEqual(before, after)

    # ------------------------------------------------------------------------------------------- (e) real + decoy
    def test_e_real_and_decoy_cards_both_get_the_section(self):
        _c, key, _raw = self.build_cards()
        real = self.real_ids(key)[0]
        real_addr = bd.parse_addr(key[real]["steam_addr"])
        decoy = next(c for c in sorted(key) if key[c]["kind"] != "real"
                     and bd.parse_addr(key[c]["steam_addr"]) != real_addr)
        decoy_addr = bd.parse_addr(key[decoy]["steam_addr"])
        DECOY_NAME = "CSWSFixture::Decoy (test row)"
        write_ledger(self.ledger, [ledger_row(real_addr, "B1-q1", NAME), ledger_row(decoy_addr, "B1-q1", DECOY_NAME)])
        cards, key, _raw = self.build_cards("--rea", "--rea-ledger-dir", str(self.ledger))
        self.assertEqual(set(cards), set(key))
        self.assertTrue(all(ls.count(SECTION) == 1 for ls in cards.values()))
        self.assertIn("- function | %s | confirmed | procedure_assembly test" % NAME, cards[real])
        self.assertIn("- function | %s | confirmed | procedure_assembly test" % DECOY_NAME, cards[decoy])

    # ------------------------------------------------------------------------------------------- cap at 8 rows
    def test_at_most_eight_rows_per_card_in_sorted_order(self):
        _c, key, _raw = self.build_cards()
        addr = bd.parse_addr(key[self.real_ids(key)[0]]["steam_addr"])
        write_ledger(self.ledger, [ledger_row(addr, "B1-q1", "N%02d" % i) for i in reversed(range(10))])
        cards, key, _raw = self.build_cards("--rea", "--rea-ledger-dir", str(self.ledger))
        shown = [l for l in cards[self.real_ids(key)[0]] if l.startswith("- function | N")]
        self.assertEqual(shown, ["- function | N%02d | confirmed | procedure_assembly test" % i for i in range(8)])


if __name__ == "__main__":
    unittest.main()
