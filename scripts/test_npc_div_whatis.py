#!/usr/bin/env python3
"""Tests for scripts/npc_div_whatis.py. Synthetic logs, labels and registry only (temp dir); never the real game log.
Run: python3 scripts/test_npc_div_whatis.py   (run_tests.py discovers it as py:scripts/test_npc_div_whatis.py)"""
import contextlib
import io
import json
import os
import struct
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import npc_div_whatis as W  # noqa: E402

BANNER = "BANNER v=1.0.0 kpl=1 sha=6A522E71 base=00400000 ts=5f3a1b2c"


def xyb_of(x, y):
    """xyb= token: the float32 bit patterns of x and y (what the mod logs)."""
    return "%08x,%08x" % (struct.unpack(">I", struct.pack(">f", x))[0], struct.unpack(">I", struct.pack(">f", y))[0])


def rmp(oid, x, y, before=805, after=806, tv=(1, 2), why="ok", xy=None):
    """A synthetic RMP line. xy= is printed from xy (may be rounded or wrong); xyb= always comes from (x, y)."""
    xy = xy or (x, y)
    tvtok = "" if tv is None else " tv=%d>%d" % tv
    return ("RMP this=12345678 pd=0a1b2c3d oid=%08x tp=00000010 tl=6 tag=Synthetic xy=%.1f,%.1f xyb=%s app=%d>%d%s "
            "h=06c232b2 why=%s" % (oid, xy[0], xy[1], xyb_of(x, y), before, after, tvtok, why))


def run_whatis(lines, args, labels=None, registry=None):
    """Write the synthetic log, labels and registry into a temp dir, run main(args + --log) and return (rc, stdout lines)."""
    with tempfile.TemporaryDirectory() as d:
        log = os.path.join(d, "npc-diversity_log.txt")
        lab, reg = Path(d, "texture_labels.json"), Path(d, "registry.jsonl")
        Path(log).write_text("\n".join(lines) + ("\n" if lines else ""))
        lab.write_text(json.dumps(labels if labels is not None else {"slots": {}}))
        reg.write_text("".join(json.dumps(r) + "\n" for r in (registry or [])))
        with mock.patch.object(W, "LABELS_JSON", lab), mock.patch.object(W, "REGISTRY", reg):
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                rc = W.main([*args, "--log", log])
    return rc, buf.getvalue().splitlines()


M_LABELS = {"slots": {
    "n_commm02_old": {"gender": "M", "slot": 2, "category": "commoner_clothing", "outfit": "rejected outfit",
                      "source": "n_commm02_old", "keep": False},
    "n_commm02": {"gender": "M", "slot": 2, "category": "commoner_clothing", "outfit": "tan wrap tunic",
                  "source": "n_commm02", "keep": True},
    "n_commm34": {"gender": "M", "slot": 34, "category": "armor", "outfit": "fat tunic", "source": "n_commm34",
                  "keep": True},
}}


class WhatisTests(unittest.TestCase):
    def test_match_uses_xyb_not_the_rounded_xy_field(self):
        # xy= says (5.0, 5.0) but the float bits are (50.0, 50.0): a query at (5, 5) must not match.
        lines = [BANNER, rmp(0x10, 50.0, 50.0, xy=(5.0, 5.0))]
        rc, out = run_whatis(lines, ["5", "5"])
        self.assertEqual(rc, 1, out)
        self.assertEqual(out, ["no RMP line within 1.00 m of (5.00, 5.00); nearest is 63.64 m away"])
        rc, out = run_whatis(lines, ["50.2", "50"])
        self.assertEqual(rc, 0, out)
        self.assertEqual(out[0], "oid=00000010 at (50.00, 50.00) d=0.20 m; RMP lines within tol: 1")

    def test_nearest_line_wins_and_tol_limits_the_match(self):
        lines = [BANNER, rmp(0x11, 20.0, 0.0), rmp(0x12, 20.6, 0.0)]
        rc, out = run_whatis(lines, ["20.5", "0"])
        self.assertEqual(rc, 0, out)
        self.assertEqual(out[0], "oid=00000012 at (20.60, 0.00) d=0.10 m; RMP lines within tol: 2")
        rc, out = run_whatis(lines, ["20.5", "0", "--tol", "0.05"])
        self.assertEqual(rc, 1, out)

    def test_negative_coordinates_are_positional_arguments(self):
        rc, out = run_whatis([BANNER, rmp(0x13, -3.5, 12.5)], ["-3.5", "12.5"])
        self.assertEqual(rc, 0, out)
        self.assertTrue(out[0].startswith("oid=00000013 at (-3.50, 12.50)"), out)

    def test_fields_and_kept_slot_label(self):
        lines = [BANNER, rmp(0x20, 30.0, 30.0, before=805, after=806, tv=(1, 2), why="ok")]
        rc, out = run_whatis(lines, ["30", "30"], labels=M_LABELS)
        self.assertEqual(rc, 0, out)
        self.assertEqual(out[1:], [
            "app 805>806  tv 1>2  why=ok",
            "slot M 2  category=commoner_clothing keep=true",
            "  outfit: tan wrap tunic",
            "  source: n_commm02",
            "role: none in registry within 0.3 m",
        ])

    def test_fat_final_app_takes_the_fat_slot_of_its_gender(self):
        rc, out = run_whatis([BANNER, rmp(0x21, 30.0, 30.0, before=17, after=802, tv=(1, 34))], ["30", "30"],
                             labels=M_LABELS)
        self.assertEqual(rc, 0, out)
        self.assertEqual(out[2], "slot M 34  category=armor keep=true")

    def test_pre_012_line_reports_no_tv_and_no_slot(self):
        rc, out = run_whatis([BANNER, rmp(0x22, 30.0, 30.0, tv=None)], ["30", "30"], labels=M_LABELS)
        self.assertEqual(rc, 0, out)
        self.assertEqual(out[1], "app 805>806  tv not logged (pre-0.1.2 line)  why=ok")
        self.assertEqual(out[2], "slot: n/a (line has no tv=)")

    def test_registry_role_within_radius_and_none_outside(self):
        registry = [{"x": 30.1, "y": 30.0, "role": "named", "tag": "HK50", "module": "001ebo"},
                    {"x": 30.5, "y": 30.0, "role": "generic", "tag": "", "module": "001ebo"},
                    {"module": "007ebo", "template": "p_g0t0", "unresolved": True}]
        rc, out = run_whatis([BANNER, rmp(0x23, 30.0, 30.0)], ["30", "30"], registry=registry)
        self.assertEqual(rc, 0, out)
        self.assertEqual(out[-1], "role: named  tag=HK50 module=001ebo  d=0.10 m")
        rc, out = run_whatis([BANNER, rmp(0x24, 30.0, 30.0)], ["30", "30"],
                             registry=[{"x": 30.5, "y": 30.0, "role": "generic", "tag": "", "module": "001ebo"}])
        self.assertEqual(out[-1], "role: none in registry within 0.3 m")

    def test_only_the_last_launch_is_searched(self):
        lines = [BANNER, rmp(0x30, 40.0, 40.0), BANNER, rmp(0x31, 60.0, 60.0)]
        rc, out = run_whatis(lines, ["40", "40"])
        self.assertEqual(rc, 1, out)
        rc, out = run_whatis(lines, ["60", "60"])
        self.assertEqual(rc, 0, out)
        self.assertTrue(out[0].startswith("oid=00000031 "), out)

    def test_log_without_rmp_lines_returns_1(self):
        rc, out = run_whatis([BANNER], ["0", "0"])
        self.assertEqual(rc, 1, out)
        self.assertEqual(out, ["no RMP line with a usable xyb in the last launch (RMP lines=0, bad xyb=0)"])

    def test_missing_log_returns_2(self):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = W.main(["0", "0", "--log", "/nonexistent/npc-diversity_log.txt"])
        self.assertEqual(rc, 2)
        self.assertTrue(buf.getvalue().startswith("no log: "), buf.getvalue())

    def test_negative_tol_is_rejected(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as cm:
            W.main(["0", "0", "--tol", "-1", "--log", "/nonexistent"])
        self.assertEqual(cm.exception.code, 2)

    def test_real_label_and_registry_files_load(self):
        labels = W.load_labels()
        self.assertIsNotNone(labels)
        self.assertIn("slots", labels)
        registry = W.load_registry()
        self.assertTrue(registry)
        self.assertTrue(all(isinstance(r["x"], (int, float)) for r in registry))


if __name__ == "__main__":
    unittest.main()
