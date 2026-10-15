"""`cards --facts` (report-only 'Static facts' section): flag wiring, section shape, order before REA, name blindness.

Fixtures and helpers come from tests/test_rea_cards.py (same Base / build_cards setup).
Run from tools/:  python3 -m unittest tests.test_facts_cards
"""
import hashlib
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_batch import B1_SPEC, Base, batch, bd, run, write_candidates  # noqa: E402
from test_batch_ledger_cards import split_cards  # noqa: E402
from test_rea_cards import ledger_row, write_ledger  # noqa: E402

STATIC = "### Static facts (script output, report-only)"
LIVE = "### Live evidence (REA, report-only)"
NAME_FMT = "CSWSFixture::Fn_%08x"


def static_block(ls):
    """Lines under the Static facts heading: up to the next blank line or '#' heading."""
    i = ls.index(STATIC) + 1
    out = []
    while i < len(ls) and ls[i] and not ls[i].startswith("#"):
        out.append(ls[i])
        i += 1
    return out


def without_static(ls):
    """The card lines with the Static facts section and the blank line before it removed."""
    i = ls.index(STATIC)
    j = i + 1 + len(static_block(ls))
    start = i - 1 if i > 0 and ls[i - 1] == "" else i
    return ls[:start] + ls[j:]


class FactsCardsTests(Base):
    def setUp(self):
        super().setUp()
        self.bdir = self.root / "batches" / "B1"
        # same fixture setup as ReaCardsTests: a content-free fingerprint for every Steam function
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

    # ------------------------------------------------------------------------------------------- (a) flag absent
    def test_a_without_facts_output_is_identical_and_has_no_static_section(self):
        _c1, _k1, raw1 = self.build_cards()
        _c2, _k2, raw2 = self.build_cards()
        digest1 = hashlib.sha256(b"".join(raw1[n] for n in sorted(raw1))).hexdigest()
        digest2 = hashlib.sha256(b"".join(raw2[n] for n in sorted(raw2))).hexdigest()
        self.assertTrue(raw1)
        self.assertEqual(digest1, digest2)
        self.assertEqual(raw1, raw2)
        blob = b"".join(raw1.values())
        self.assertNotIn(b"Static facts", blob)
        self.assertNotIn(b"- twins:", blob)
        # a --facts build in between must not change a later no-flag build (no state carried over)
        self.build_cards("--facts")
        _c3, _k3, raw3 = self.build_cards()
        self.assertEqual(raw1, raw3)

    # ------------------------------------------------------------------------------------------- (b) shape + only adds
    def test_b_facts_adds_one_section_per_card_real_and_decoy_alike(self):
        base_cards, _bk, _braw = self.build_cards()
        cards, key, _raw = self.build_cards("--facts")
        self.assertEqual(set(cards), set(base_cards))
        kinds = {k["kind"] for k in key.values()}
        self.assertIn("real", kinds)
        self.assertTrue(kinds - {"real"}, "fixture must have decoy cards too")
        for cid, ls in cards.items():
            self.assertEqual(ls.count(STATIC), 1, cid)
            self.assertEqual(sum(l.startswith("- twins:") for l in ls), 1, cid)
            block = static_block(ls)
            self.assertTrue(block, cid)
            self.assertTrue(all(l.startswith("- ") for l in block), (cid, block))
            self.assertEqual(without_static(ls), base_cards[cid], cid)

    # ------------------------------------------------------------------------------------------- (c) order vs REA
    def test_c_static_section_comes_before_live_evidence_on_every_card(self):
        _c, key, _raw = self.build_cards()
        addr = bd.parse_addr(key[self.real_ids(key)[0]]["steam_addr"])
        write_ledger(self.ledger, [ledger_row(addr, "B1-q1", "CSWSFixture::Probe (test row)")])
        cards, key, _raw = self.build_cards("--facts", "--rea", "--rea-ledger-dir", str(self.ledger))
        self.assertTrue(cards)
        for cid, ls in cards.items():
            self.assertEqual(ls.count(STATIC), 1, cid)
            self.assertEqual(ls.count(LIVE), 1, cid)
            self.assertLess(ls.index(STATIC), ls.index(LIVE), cid)
            self.assertEqual(sum(l.startswith("- twins:") for l in ls), 1, cid)

    # ------------------------------------------------------------------------------------------- (d) blindness
    def test_d_card_never_shows_its_own_steam_name_in_static_facts(self):
        # give every fixture Steam address a name; the card's own name must still stay out of its section
        names = {a: NAME_FMT % a for a in self.world.steam}
        sf = batch.load_static_facts()
        with mock.patch.object(sf, "load_names", side_effect=lambda **kw: dict(names)) as fake:
            cards, key, _raw = self.build_cards("--facts")
        fake.assert_called_once()
        self.assertEqual(fake.call_args.kwargs["exclude_sources"], ("B1",))
        for cid, ls in cards.items():
            own = NAME_FMT % bd.parse_addr(key[cid]["steam_addr"])
            self.assertNotIn(own, "\n".join(static_block(ls)), cid)

    # ------------------------------------------------------------------------------------------- (e) batch-sourced names
    def test_e_batch_sourced_name_map_rows_never_appear_but_other_sources_do(self):
        rows = [r.split("\t") for r in (self.world.dump / "callgraph.tsv").read_text().splitlines()[1:]]
        edges = {}
        for caller, _site, callee, *_ in rows:
            edges.setdefault(int(caller, 16), []).append(int(callee, 16))
        _c, key, _raw = self.build_cards()
        card = next((c for c in self.real_ids(key)
                     if len(set(edges.get(bd.parse_addr(key[c]["steam_addr"]), []))) >= 2), None)
        self.assertIsNotNone(card, "fixture needs a real card whose candidate has two callees")
        c1, c2 = sorted(set(edges[bd.parse_addr(key[card]["steam_addr"])]))[:2]
        con = sqlite3.connect(self.world.kdb)
        con.execute("insert into name_map values(?,?,?,?,?,?)",
                    (batch.hx8(c1), "Blind::BatchNamed", "B1-x", "high", "e", "2026-10-01"))
        con.execute("insert into name_map values(?,?,?,?,?,?)",
                    (batch.hx8(c2), "Visible::OtherNamed", "engine-ai-atlas wave9", "high", "e", "2026-10-01"))
        con.commit()
        con.close()
        cards, key, _raw = self.build_cards("--facts")
        text = "\n".join(static_block(cards[card]))
        self.assertNotIn("Blind::BatchNamed", text)
        self.assertIn("Visible::OtherNamed", text)
        self.assertNotIn("Blind::BatchNamed", "\n".join("\n".join(static_block(ls)) for ls in cards.values()))

    # ------------------------------------------------------------------------------------------- (f) env isolation / missing dump
    def test_f_facts_use_redirected_dbs_and_survive_a_dump_without_files(self):
        import os
        for var, path in (("BD_KDB", self.world.kdb), ("BD_BRANCHDB", self.world.branchdb)):
            self.assertEqual(os.environ[var], str(path))
            self.assertTrue(str(path).startswith(str(self.root)))
        seen = {}
        sf = batch.load_static_facts()
        orig_load = sf.load_names
        orig_dump = sf.Dump

        def spy_load(**kw):
            seen.update(kw)
            return orig_load(**kw)

        absent = self.tmp / "absent_dump"
        with mock.patch.object(sf, "load_names", side_effect=spy_load), \
                mock.patch.object(sf, "Dump", side_effect=lambda root, names=None: orig_dump(absent, names)):
            cards, key, _raw = self.build_cards("--facts")
        self.assertEqual(str(seen["kdb"]), str(self.world.kdb))
        self.assertEqual(str(seen["steam_db"]), str(self.world.branchdb))
        self.assertTrue(cards)
        for cid, ls in cards.items():
            block = static_block(ls)
            self.assertEqual(sum(l.startswith("- twins:") for l in block), 1, cid)
            self.assertIn("- twins: none", block, cid)
            self.assertIn("- asm: no decomp file in dump", block, cid)


if __name__ == "__main__":
    unittest.main()
