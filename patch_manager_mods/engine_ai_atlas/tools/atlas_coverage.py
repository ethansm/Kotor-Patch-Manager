#!/usr/bin/env python3
"""atlas_coverage.py - per-class naming coverage of the engine-AI atlas.

Columns:
  mapped             name_map rows whose name starts with 'Class::'
  high               of those, confidence == 'high'
  verified_in_atlas  of those, source starts with 'engine-ai-atlas'
  open               rows of ROOT/findings/AI0_map_*.tsv (mapper output, no header:
                     steam_addr gog_addr Class::Name vote|drift+0xNN retn=N) for that class whose steam_addr has
                     NO name_map row yet (0 when no mapper files exist)
  struct_fields      rows per class in the DB table `struct_fields` (column only appears when that table exists)
Output: TSV by default, --md for a markdown table.  --map-glob changes the mapper file pattern.
"""
import argparse
import glob
import os
import sqlite3
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import atlas_common as ac  # noqa: E402

DEFAULT_CLASSES = ["CServerAIMaster", "CPathfind", "CPathfindInformation", "CSWSFaction", "CFactionManager",
                   "CSWSCombatRound", "CSWSCreature", "CSWSObject", "CSWSArea", "CSWSAreaOfEffectObject",
                   "CSWSEffectListHandler", "CGameEffect", "CVirtualMachine", "CSWVirtualMachineCommands"]


def load_map_rows(root, pattern):
    """-> list of (steam_addr:int, class, name) from the mapper TSVs."""
    rows = []
    for p in sorted(glob.glob(os.path.join(root, "findings", pattern))):
        with open(p, encoding="utf-8", errors="replace") as f:
            for line in f:
                parts = line.rstrip("\n").split("\t")
                if len(parts) < 3 or not parts[2] or "::" not in parts[2]:
                    continue
                try:
                    rows.append((ac.parse_addr(parts[0]), parts[2].split("::", 1)[0], parts[2]))
                except ValueError:
                    continue
    return rows


def compute(db, root, classes, map_glob="AI0_map_*.tsv"):
    nm = {}
    for a, n, src, conf in db.execute("select steam_addr, name, source, confidence from name_map"):
        try:
            nm[ac.parse_addr(a)] = (n or "", src or "", conf or "")
        except (ValueError, TypeError):
            pass
    has_sf = db.execute("select 1 from sqlite_master where type='table' and name='struct_fields'").fetchone()
    sf = {}
    if has_sf:
        try:
            for c, n in db.execute("select class, count(*) from struct_fields group by class"):
                sf[c] = n
        except sqlite3.Error:
            has_sf = None
    maprows = load_map_rows(root, map_glob)
    open_by = {}
    for a, c, _n in maprows:
        if a not in nm:
            open_by.setdefault(c, set()).add(a)
    out = []
    for c in classes:
        mine = [v for v in nm.values() if v[0].startswith(c + "::")]
        out.append({
            "class": c,
            "mapped": len(mine),
            "high": sum(1 for v in mine if v[2] == "high"),
            "verified_in_atlas": sum(1 for v in mine if v[1].startswith("engine-ai-atlas")),
            "open": len(open_by.get(c, ())),
            **({"struct_fields": sf.get(c, 0)} if has_sf else {}),
        })
    return out, bool(has_sf), len(maprows)


def render(rows, md):
    cols = list(rows[0].keys()) if rows else ["class"]
    tot = {c: sum(r[c] for r in rows) for c in cols[1:]}
    allrows = [[str(r[c]) for c in cols] for r in rows] + [["TOTAL"] + [str(tot[c]) for c in cols[1:]]]
    if md:
        lines = ["| " + " | ".join(cols) + " |", "|" + "|".join(["---"] + ["---:"] * (len(cols) - 1)) + "|"]
        lines += ["| " + " | ".join(r) + " |" for r in allrows]
    else:
        lines = ["\t".join(cols)] + ["\t".join(r) for r in allrows]
    return "\n".join(lines)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ac.add_common_args(ap)
    ap.add_argument("--classes", default=None, help="comma-separated class list (default: the AI atlas classes)")
    ap.add_argument("--md", action="store_true", help="markdown table (default TSV)")
    ap.add_argument("--map-glob", default="AI0_map_*.tsv")
    args = ap.parse_args(argv)
    db = ac.open_db_ro(args.db)
    if db is None:
        print("knowledge DB not found", file=sys.stderr)
        return 2
    classes = [c.strip() for c in args.classes.split(",") if c.strip()] if args.classes else DEFAULT_CLASSES
    rows, _has_sf, nmap = compute(db, ac.expand_root(args.root), classes, args.map_glob)
    print(render(rows, args.md))
    if nmap == 0:
        print("(no mapper rows found for %s: 'open' is 0)" % args.map_glob, file=sys.stderr)
    return 0


if __name__ == "__main__":
    import signal
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)  # quiet when piped into head
    sys.exit(main())
