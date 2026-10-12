#!/usr/bin/env python3
"""npc-diversity what-is: which RMP line sits at a world position, and what the mod did to it. Python 3, stdlib only; read-only.

Usage: npc_div_whatis.py <x> <y> [--log PATH] [--tol 1.0]
  Default log: the npc-diversity log in the KOTOR II game dir (same default as npc_div_log_check).
  Looks at the LAST launch's RMP lines and matches on xyb (float32 bits, exact), not the rounded xy= field.
  Prints only parsed fields (oid, app, tv, why, slot label, registry role). Raw log lines are never echoed.
  Exit 0 = a line was found within --tol, 1 = none within --tol (or no usable RMP line), 2 = log missing.
"""
from __future__ import annotations

import argparse
import json
import math
import re
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import npc_div_log_check as N  # noqa: E402  (parse_lines, _of, family_of, DEFAULT_LOG; the checker is not modified)
import log_triage as LT  # noqa: E402  (read_lines only)

LABELS_JSON = HERE.parent / "patch_manager_mods/npc_diversity_src/texture_labels.json"
REGISTRY = HERE.parent / ".claude/npc_div/registry.jsonl"
ROLE_RADIUS = 0.3                                   # metres (2D) for a registry position match
FAMILY_GENDER = {"FatM": "M", "FatF": "F"}          # label gender for the fat families (M/F otherwise)
XYB_RE = re.compile(r"([0-9a-fA-F]{8}),([0-9a-fA-F]{8})")


def bits_to_float(hex8):
    """float32 from its 8-hex-digit IEEE bit pattern (xyb= '41480000' -> 12.5)."""
    return struct.unpack(">f", struct.pack(">I", int(hex8, 16)))[0]


def rmp_positions(sess):
    """([(x, y, rec)] decoded from xyb for the last launch's RMP lines, count of RMP lines with a bad xyb)."""
    out, bad = [], 0
    for r in N._of(sess, "RMP"):
        m = XYB_RE.fullmatch(r["f"]["xyb"])
        if not m:
            bad += 1
            continue
        out.append((bits_to_float(m.group(1)), bits_to_float(m.group(2)), r))
    return out, bad


def load_labels():
    """texture_labels.json parsed; None when missing or unreadable."""
    try:
        return json.loads(LABELS_JSON.read_text())
    except (OSError, ValueError):
        return None


def load_registry():
    """Registry rows that carry a numeric x and y; None when the file is missing. Malformed lines are skipped."""
    try:
        text = REGISTRY.read_text()
    except OSError:
        return None
    rows = []
    for line in text.splitlines():
        try:
            row = json.loads(line)
        except ValueError:
            continue
        if isinstance(row, dict) and isinstance(row.get("x"), (int, float)) and isinstance(row.get("y"), (int, float)):
            rows.append(row)
    return rows


def slot_lines(v, labels):
    """Slot label lines for one RMP line: the tv after-value is the slot, the final app's family picks the gender."""
    if v["tv_after"] is None:
        return ["slot: n/a (line has no tv=)"]
    fam = N.family_of(v["after"])
    if fam is None:
        return ["slot: not judged (final app %d is not a known body family)" % v["after"]]
    gender = FAMILY_GENDER.get(fam, fam)
    slot = v["tv_after"]
    if labels is None:
        return ["slot %s %d: texture_labels.json missing" % (gender, slot)]
    hits = [e for e in labels.get("slots", {}).values() if e.get("gender") == gender and e.get("slot") == slot]
    if not hits:
        return ["slot %s %d: no label" % (gender, slot)]
    hits.sort(key=lambda e: not e.get("keep"))           # a kept label wins over a rejected one on the same slot
    e = hits[0]
    return ["slot %s %d  category=%s keep=%s" % (gender, slot, e.get("category"), str(bool(e.get("keep"))).lower()),
            "  outfit: %s" % e.get("outfit"),
            "  source: %s" % e.get("source")]


def role_line(px, py, registry):
    """Role of the nearest registry row within ROLE_RADIUS (2D) of the RMP position."""
    if registry is None:
        return "role: registry missing"
    hits = sorted(((math.hypot(row["x"] - px, row["y"] - py), row) for row in registry), key=lambda t: t[0])
    hits = [h for h in hits if h[0] <= ROLE_RADIUS]
    if not hits:
        return "role: none in registry within %.1f m" % ROLE_RADIUS
    d, row = hits[0]
    text = "role: %s  tag=%s module=%s  d=%.2f m" % (row.get("role"), row.get("tag"), row.get("module"), d)
    if len(hits) > 1:
        text += "  (+%d more within %.1f m)" % (len(hits) - 1, ROLE_RADIUS)
    return text


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("x", type=float)
    ap.add_argument("y", type=float)
    ap.add_argument("--log", default=None)
    ap.add_argument("--tol", type=float, default=1.0, help="match radius in metres (default 1.0)")
    a = ap.parse_args(argv)
    if a.tol < 0:
        ap.error("--tol must be >= 0")
    path = Path(a.log).expanduser() if a.log else N.DEFAULT_LOG
    if not path.is_file():
        print("no log: %s" % path)
        return 2
    recs, _ = N.parse_lines(list(enumerate(LT.read_lines(path), start=1)))
    starts = [i for i, r in enumerate(recs) if r["kind"] == "BANNER:v"]
    sess = recs[starts[-1]:] if starts else recs
    pts, bad_xyb = rmp_positions(sess)
    if not pts:
        print("no RMP line with a usable xyb in the last launch (RMP lines=%d, bad xyb=%d)"
              % (len(N._of(sess, "RMP")), bad_xyb))
        return 1
    scored = sorted(((math.hypot(px - a.x, py - a.y), px, py, r) for px, py, r in pts), key=lambda t: t[0])
    within = [s for s in scored if s[0] <= a.tol]
    if not within:
        print("no RMP line within %.2f m of (%.2f, %.2f); nearest is %.2f m away" % (a.tol, a.x, a.y, scored[0][0]))
        return 1
    d, px, py, r = within[0]
    v = r["v"]
    print("oid=%08x at (%.2f, %.2f) d=%.2f m; RMP lines within tol: %d%s" % (
        v["oid"], px, py, d, len(within), "" if not bad_xyb else "; bad xyb skipped: %d" % bad_xyb))
    tv = "tv %d>%d" % (v["tv_before"], v["tv_after"]) if v["tv_after"] is not None else "tv not logged (pre-0.1.2 line)"
    print("app %d>%d  %s  why=%s" % (v["before"], v["after"], tv, r["f"]["why"]))
    for line in slot_lines(v, load_labels()):
        print(line)
    print(role_line(px, py, load_registry()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
