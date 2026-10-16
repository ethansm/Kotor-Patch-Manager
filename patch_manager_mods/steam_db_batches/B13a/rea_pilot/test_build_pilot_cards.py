"""Tests for build_pilot_cards.py. Run from this directory:
    python3 -m unittest test_build_pilot_cards -v
Only build() and pure helpers run here. The one main() call exits before writing anything."""
import contextlib
import importlib.util
import io
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("build_pilot_cards", HERE / "build_pilot_cards.py")
bpc = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(bpc)

ARMS = ["norea", "rea", "facts", "facts_rea"]


def card_ids(text):
    return [m.group(1) for m in bpc.CARD_RE.finditer(text)]


def split_cards(text):
    marks = list(bpc.CARD_RE.finditer(text))
    return {m.group(1): text[m.start():(marks[i + 1].start() if i + 1 < len(marks) else len(text))]
            for i, m in enumerate(marks)}


class PilotArms(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ids = bpc.pilot_ids()
        committed = bpc.load_blocks(bpc.B / "cards")
        cls.parts = {}
        for arm in ARMS:
            src = bpc.ARMS[arm][1]
            full = bpc.load_blocks(HERE / src) if src else {}
            cls.parts[arm] = bpc.build(arm, committed, full, cls.ids)

    def test_ids_and_order_identical_across_arms(self):
        seqs = {arm: [cid for p in parts for cid in card_ids(p)] for arm, parts in self.parts.items()}
        self.assertTrue(self.ids)
        self.assertEqual(len(set(self.ids)), len(self.ids))
        self.assertEqual(seqs["norea"], self.ids)
        for arm in ARMS:
            self.assertEqual(seqs[arm], seqs["norea"], arm)

    def test_each_arm_has_two_parts_with_unchanged_header(self):
        for arm in ARMS:
            self.assertEqual(len(self.parts[arm]), 2, arm)
            for k, text in enumerate(self.parts[arm], 1):
                self.assertTrue(text.startswith(bpc.HEADER % k), (arm, k))

    def test_arms_differ_only_by_appended_sections(self):
        for arm in ARMS[1:]:
            for base_part, arm_part in zip(self.parts["norea"], self.parts[arm]):
                base, extra = split_cards(base_part), split_cards(arm_part)
                self.assertEqual(list(extra), list(base), arm)
                for cid, card in base.items():
                    stem = card.rstrip("\n")
                    self.assertTrue(extra[cid].startswith(stem), (arm, cid))
                    self.assertTrue(extra[cid][len(stem):].startswith("\n\n### "), (arm, cid))

    def test_rea_and_norea_match_committed_r1_files(self):
        for arm, out_dir in (("rea", "cards_rea"), ("norea", "cards_norea")):
            for k in (1, 2):
                want = (HERE / out_dir / ("pilot_%d.md" % k)).read_bytes()
                self.assertEqual(self.parts[arm][k - 1].encode("utf-8"), want, "%s pilot_%d" % (arm, k))

    def test_facts_headers_once_per_card(self):
        for arm in ("facts", "facts_rea"):
            for part in self.parts[arm]:
                cards = split_cards(part)
                self.assertTrue(cards, arm)
                for cid, chunk in cards.items():
                    self.assertEqual(chunk.count(bpc.FACTS_HDR), 1, (arm, cid))
                    if arm == "facts":
                        self.assertEqual(chunk.count(bpc.REA_HDR), 0, (arm, cid))
                    else:
                        self.assertEqual(chunk.count(bpc.REA_HDR), 1, (arm, cid))
                        self.assertLess(chunk.index(bpc.FACTS_HDR), chunk.index(bpc.REA_HDR), (arm, cid))

    def test_main_missing_source_dir_exits_2_and_writes_nothing(self):
        with tempfile.TemporaryDirectory() as empty, mock.patch.object(bpc, "HERE", Path(empty)), \
                contextlib.redirect_stderr(io.StringIO()) as err:
            self.assertEqual(bpc.main(["--arm", "facts"]), 2)
            self.assertIn("cards_full_facts", err.getvalue())
            self.assertEqual(list(Path(empty).iterdir()), [])


if __name__ == "__main__":
    unittest.main()
