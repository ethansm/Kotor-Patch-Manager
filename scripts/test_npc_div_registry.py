#!/usr/bin/env python3
"""Tests for npc_div_registry pure logic (no game files, no pykotor)."""
import json
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import npc_div_registry as R  # noqa: E402


def row(**kw):
    base = dict(module="m", tag="t", name="", app=17, fam="M", conv="", plot=0, interruptable=1, x=0.0, y=0.0,
                name_n=0, conv_n=0, role="generic")
    base.update(kw)
    return base


class RegistryTests(unittest.TestCase):
    def test_tables_parsed_from_header(self):
        t = R.parse_tables()
        self.assertEqual(set(t), {"M", "F", "TM", "TF"})
        self.assertIn(17, t["M"])
        self.assertIn(14, t["F"])
        self.assertIn(416, t["TM"])
        self.assertIn(280, t["TF"])
        self.assertFalse(t["M"] & t["F"])
        self.assertEqual(R.family_of(17, t), "M")
        self.assertEqual(R.family_of(185, t), "F")
        self.assertEqual(R.family_of(72, t), "")

    def test_rule_role(self):
        self.assertEqual(R.rule_role(row(plot=1)), "plot")
        self.assertEqual(R.rule_role(row(interruptable=0)), "plot")
        self.assertEqual(R.rule_role(row(conv="unique_npc", conv_n=1)), "named")
        self.assertEqual(R.rule_role(row(conv="200comm", conv_n=40)), "generic")        # shared bark dialog
        self.assertEqual(R.rule_role(row(name="Traveler{M1}", name_n=1)), "generic")    # templated name
        self.assertEqual(R.rule_role(row(name="Zaalbar", name_n=1)), "named")
        self.assertEqual(R.rule_role(row(name="Citizen", name_n=30)), "generic")
        # plot beats a shared conversation
        self.assertEqual(R.rule_role(row(plot=1, conv="200comm", conv_n=40)), "plot")

    def test_rounded(self):
        self.assertEqual(R.rounded(12.34), 12.3)
        self.assertEqual(R.rounded(-0.06), -0.1)

    def test_ambiguity(self):
        keep = row(role="named", x=1.0, y=1.0)
        near_generic = row(role="generic", x=1.1, y=1.0)
        far_generic = row(role="generic", x=9.0, y=9.0)
        k, a = R.ambiguity([keep, far_generic], lambda r: r["role"] != "generic")
        self.assertEqual((k, a), (1, 0))
        k, a = R.ambiguity([keep, near_generic], lambda r: r["role"] != "generic")
        self.assertEqual((k, a), (1, 1))
        other_mod = row(module="other", role="generic", x=1.0, y=1.0)      # different module never collides
        self.assertEqual(R.ambiguity([keep, other_mod], lambda r: r["role"] != "generic"), (1, 0))

    def test_pairs_batching_and_dedupe(self):
        rows = [row(tag="a", name="A", module="m1"), row(tag="a", name="A", module="m2"),
                row(tag="b", name="B"), row(tag="c", name="C", fam=""), row(tag="d", name="D")]
        with tempfile.TemporaryDirectory() as td:
            reg = Path(td) / "reg.jsonl"
            reg.write_text("\n".join(json.dumps(r) for r in rows) + "\n")
            out = Path(td) / "roles"
            R.pairs(reg, out, 2)
            files = sorted(out.glob("pairs_*.json"))
            self.assertEqual(len(files), 2)                    # 3 distinct tabled pairs (a, b, d), batch of 2
            items = [i for f in files for i in json.loads(f.read_text())]
            self.assertEqual([i["tag"] for i in items], ["a", "b", "d"])
            self.assertEqual(items[0]["n"], 2)
            self.assertEqual(items[0]["mods"], ["m1", "m2"])

    def test_module_files_dedupes_case(self):
        with tempfile.TemporaryDirectory() as td:
            for n in ("201TEL.mod", "201tel.mod", "201tel_s.rim", "202TEL.rim", "x_dlg.erf"):
                (Path(td) / n).write_bytes(b"")
            f = R.module_files(td)
            self.assertEqual(sorted(f), ["201tel", "202tel"])
            self.assertIn("mod", f["201tel"])
            self.assertIn("s_rim", f["201tel"])
            self.assertIn("rim", f["202tel"])


if __name__ == "__main__":
    unittest.main()
