#!/usr/bin/env python3
"""Tests for generate_diversity_slots (stdlib)."""
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import generate_diversity_slots as G  # noqa: E402


def slot(n, **fam):
    f = {"M": "no", "F": "no", "FatM": "no", "FatF": "no"}
    f.update(fam)
    return {"slot": n, "layout_ok": f}


def doc(extra=()):
    s = {}
    for n in range(1, 13):
        s["n_commm%02d" % n] = slot(n, M="yes")
        s["n_commf%02d" % n] = slot(n, F="yes")
    for k, v in extra:
        s[k] = v
    return {"slots": s}


class SlotsTests(unittest.TestCase):
    def test_allowlists_only_yes_and_sorted(self):
        d = doc([("n_commm34", slot(34, FatM="yes")), ("n_commf35", slot(35, FatF="unsure")),
                 ("n_commm20", slot(20))])
        al = G.allowlists(d)
        self.assertEqual(al["M"], list(range(1, 13)))
        self.assertEqual(al["FatM"], [34])
        self.assertEqual(al["FatF"], [])        # unsure is not allowed

    def test_render_golden_bits_and_determinism(self):
        t = G.render(doc([("n_commm34", slot(34, FatM="yes"))]))
        self.assertEqual(t, G.render(doc([("n_commm34", slot(34, FatM="yes"))])))
        self.assertIn("constexpr uint8_t kMaleSlots[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};", t)
        self.assertIn("constexpr size_t kFatMaleSlotsCount = 1;", t)
        self.assertIn("constexpr uint8_t kFatFemaleSlots[] = {0};", t)       # empty family: sentinel, count 0
        self.assertIn("constexpr size_t kFatFemaleSlotsCount = 0;", t)

    def test_vanilla_defaults_must_stay(self):
        d = doc()
        d["slots"]["n_commm06"] = slot(6)
        with self.assertRaises(SystemExit):
            G.render(d)

    def test_check_mode(self):
        with tempfile.TemporaryDirectory() as td:
            lab, out = Path(td) / "l.json", Path(td) / "o.h"
            lab.write_text(json.dumps(doc()))
            self.assertEqual(G.main(["--labels", str(lab), "--out", str(out), "--check"]), 1)      # missing
            self.assertEqual(G.main(["--labels", str(lab), "--out", str(out)]), 0)
            self.assertEqual(G.main(["--labels", str(lab), "--out", str(out), "--check"]), 0)
            out.write_text(out.read_text() + "x")
            self.assertEqual(G.main(["--labels", str(lab), "--out", str(out), "--check"]), 1)      # stale

    def test_committed_header_is_current(self):
        self.assertEqual(G.main(["--check"]), 0)


if __name__ == "__main__":
    unittest.main()
