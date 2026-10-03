"""cmdtable.py: command-table decode (B15) and the card-facts / withheld-sources card rules."""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import batch  # noqa: E402
import cmdtable  # noqa: E402

STORES = "fn\tinsn\tbase\tdisp\ttarget\n" + "".join("%s\n" % r for r in [
    "0x00700000\t0x00700010\tECX\t0x0\t0x00500000",      # slot 0
    "0x00700000\t0x00700020\tEAX\t0x4\t0x00500040",      # slot 1
    "0x00700000\t0x00700030\tEDX\t0x8\t0x00500040",      # slot 2 (shared handler)
    "0x00700000\t0x00700040\tECX\t0xc\t0x00500080",      # slot 3, overwritten below
    "0x00700000\t0x00700050\tECX\t0xc\t0x005000c0",
    "0x00700000\t0x00700060\tECX\t0x6\t0x00500100",      # unaligned: ignored
    "0x00700000\t0x00700070\tECX\t0x100\t0x00500100",    # beyond the slot count: ignored
    "0x00710000\t0x00710010\tECX\t0x10\t0x00500140",     # slot 4, second init function
    "0x00720000\t0x00720010\tECX\t0x14\t0x00500180",     # another table function: not listed, ignored
])


class DecodeTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.path = Path(self.tmp.name) / "cmdstores.tsv"
        self.path.write_text(STORES)

    def tearDown(self):
        self.tmp.cleanup()

    def test_slots_last_store_wins_and_filters(self):
        table, over = cmdtable.decode(self.path, [0x700000, 0x710000], slots=6)
        self.assertEqual(table, {0: 0x500000, 1: 0x500040, 2: 0x500040, 3: 0x5000C0, 4: 0x500140})
        self.assertEqual(over, [(3, 0x500080, 0x5000C0, 0x700050)])

    def test_slot_sets_group_shared_handlers(self):
        table, _ = cmdtable.decode(self.path, [0x700000, 0x710000], slots=6)
        sets = cmdtable.slot_sets(table)
        self.assertEqual(sets[0x500040], frozenset({1, 2}))
        self.assertEqual(cmdtable.fmt_slots(sets[0x500040]), "1,2")

    def test_mac_stride_8(self):
        # gcc pointer-to-member slots are 8 bytes: 0x0 -> 0, 0x8 -> 1; 0x4 / 0xc are not slot starts
        table, _ = cmdtable.decode(self.path, [0x700000, 0x710000], slots=6, stride=8)
        self.assertEqual(table, {0: 0x500000, 1: 0x500040, 2: 0x500140})

    def test_missing_init_function_is_an_error(self):
        with self.assertRaises(SystemExit):
            cmdtable.decode(self.path, [0x700000, 0x7FFFFF], slots=6)

    def test_nss_functions_in_declaration_order(self):
        nss = Path(self.tmp.name) / "nwscript.nss"
        nss.write_text("// int Fake(int x);\nint    NUM = 3;\nint Random(int nMaxInteger);\n/* void Gone();\n*/\n"
                       "void PrintString(string sString);\neffect EffectHeal(int n);\n")
        self.assertEqual(cmdtable.nss_functions(nss), ["Random", "PrintString", "EffectHeal"])


class CardFactTests(unittest.TestCase):
    def test_card_facts_load_by_gog_address(self):
        with tempfile.TemporaryDirectory() as d:
            batch.write_tsv(Path(d) / "card_facts.tsv", ["gog", "fact"],
                            [{"gog": "0x0077eef0", "fact": "slot 700 ActionBarkString"}])
            self.assertEqual(batch.load_card_facts(Path(d)), {0x77EEF0: ["slot 700 ActionBarkString"]})
            self.assertEqual(batch.load_card_facts(Path(d) / "missing"), {})


if __name__ == "__main__":
    unittest.main()
