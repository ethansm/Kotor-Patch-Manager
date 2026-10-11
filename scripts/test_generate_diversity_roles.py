#!/usr/bin/env python3
"""Tests for generate_diversity_roles (stdlib)."""
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import generate_diversity_roles as G  # noqa: E402


def row(tag, name, x=0.0, y=0.0, fam="M", role="generic", plot=0, inter=1):
    return dict(tag=tag, name=name, x=x, y=y, fam=fam, role=role, plot=plot, interruptable=inter, module="m")


class RolesTests(unittest.TestCase):
    def test_haiku_verdict_beats_rule_role(self):
        rows = [row("Guard1", "Guard", role="generic"), row("Citizen", "Citizen", role="named")]
        v = {("Guard1", "Guard"): "uniformed", ("Citizen", "Citizen"): "generic"}
        tags, pos, counts = G.keep_sets(rows, v)
        self.assertEqual(tags, ["guard1"])
        self.assertEqual(counts["uniformed"], 1)
        self.assertEqual(counts["generic"], 1)

    def test_plot_always_keeps_and_unverdicted_follow_rule(self):
        rows = [row("P", "P", plot=1), row("N", "N", role="named"), row("G", "G", role="generic"),
                row("X", "X", inter=0)]
        tags, _, _ = G.keep_sets(rows, {})
        self.assertEqual(tags, ["n", "p", "x"])

    def test_non_table_and_unresolved_rows_ignored(self):
        rows = [row("A", "A", fam="", role="named"), {"unresolved": True, "module": "m", "template": "t"}]
        tags, pos, counts = G.keep_sets(rows, {})
        self.assertEqual((tags, pos, sum(counts.values())), ([], [], 0))

    def test_tag_keep_if_any_instance_qualifies(self):
        rows = [row("commoner", "Citizen"), row("commoner", "Boss", role="named")]
        v = {("commoner", "Citizen"): "generic", ("commoner", "Boss"): "named"}
        self.assertEqual(G.keep_sets(rows, v)[0], ["commoner"])

    def test_positions_rounded_sorted_unique(self):
        rows = [row("a", "a", 1.04, 2.06, role="named"), row("b", "b", 1.04, 2.06, role="named"),
                row("c", "c", -3.0, 0.0, role="named")]
        _, pos, _ = G.keep_sets(rows, {})
        self.assertEqual(pos, [(-30, 0), (10, 21)])

    def test_render_deterministic_and_escapes(self):
        t = G.render(['a"b', "z"], [(1, 2)])
        self.assertEqual(t, G.render(['a"b', "z"], [(1, 2)]))
        self.assertIn('"a\\"b",', t)
        self.assertIn("kKeepTagCount = 2;", t)
        self.assertIn("{1, 2},", t)
        self.assertIn("kKeepPosTol10 = 3;", t)

    def test_ambiguity_counts_generic_near_keep(self):
        rows = [row("k", "k", 1.0, 1.0, role="named"), row("g", "g", 1.1, 1.0), row("g2", "g2", 50.0, 50.0)]
        gen, hit = G.ambiguity(rows, {})
        self.assertEqual((gen, hit), (2, 1))

    def test_check_mode_and_committed_header(self):
        with tempfile.TemporaryDirectory() as td:
            reg, rd, out = Path(td) / "r.jsonl", Path(td) / "roles", Path(td) / "o.h"
            rd.mkdir()
            reg.write_text(json.dumps(row("k", "k", role="named")) + "\n")
            args = ["--registry", str(reg), "--roles-dir", str(rd), "--out", str(out)]
            self.assertEqual(G.main(args + ["--check"]), 1)
            self.assertEqual(G.main(args), 0)
            self.assertEqual(G.main(args + ["--check"]), 0)
        self.assertEqual(G.main(["--check"]), 0)


if __name__ == "__main__":
    unittest.main()
