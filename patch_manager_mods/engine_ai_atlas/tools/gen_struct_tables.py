#!/usr/bin/env python3
"""gen_struct_tables.py - markdown struct-field tables from the knowledge DB (read-only, stdlib only).

Reads the DB table `struct_fields` (opened with mode=ro) and prints, per class, a markdown table
`offset | size | type | name | conf` ordered by offset, keeping only confidence high/medium; the number of omitted
`low` rows is stated in each class heading.

  python3 gen_struct_tables.py                      # default key classes, one field per table row
  python3 gen_struct_tables.py --pack 2             # two fields per table row (README layout)
  python3 gen_struct_tables.py --classes CServerAIMaster,CSWSFaction
  python3 gen_struct_tables.py --pack 2 --splice ../README.md
      rewrites the text between the markers
      `<!-- struct_fields:begin -->` and `<!-- struct_fields:end -->` in that file (the only write this tool does;
      the DB is never written).
"""
import argparse
import os
import re
import sqlite3
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_DB = os.path.normpath(os.path.join(HERE, "..", "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db"))
DEFAULT_CLASSES = ["CSWSCreature", "CSWSCombatRound", "CPathfindInformation", "CAction", "CSWSObject",
                   "CFollowInfo", "CSWParty", "CSWPartySlot"]
KEEP = ("high", "medium")
BEGIN = "<!-- struct_fields:begin -->"
END = "<!-- struct_fields:end -->"
ABBR = {"high": "h", "medium": "m", "low": "l"}


def open_ro(path):
    return sqlite3.connect("file:%s?mode=ro" % os.path.abspath(path), uri=True)


def cell(s, maxlen):
    s = "" if s is None else str(s)
    s = s.replace("|", "\\|").replace("\n", " ").strip()
    if maxlen and len(s) > maxlen:
        s = s[: maxlen - 3].rstrip() + "..."
    return s


def fetch(db, cls):
    rows = db.execute(
        "select offset, size, type, name, confidence from struct_fields where class=? order by offset", (cls,)
    ).fetchall()
    kept = [r for r in rows if (r[4] or "").lower() in KEEP]
    low = len(rows) - len(kept)
    return kept, low


def render_class(cls, kept, low, pack, maxname, maxtype):
    out = ["#### %s (%d rows high/medium; %d low omitted)" % (cls, len(kept), low)]
    if not kept:
        out.append("(no high/medium rows)")
        return out
    one = ["offset", "size", "type", "name", "c"]
    head = "| " + " | ".join(one * pack) + " |"
    sep = "|" + "|".join(["---", "---:", "---", "---", "---"] * pack) + "|"
    out += [head, sep]
    fmt = []
    for off, size, typ, name, conf in kept:
        fmt.append([
            "0x%X" % off,
            "" if size is None else str(size),
            cell(typ, maxtype),
            cell(name, maxname),
            ABBR.get((conf or "").lower(), "?"),
        ])
    for i in range(0, len(fmt), pack):
        chunk = fmt[i:i + pack]
        while len(chunk) < pack:
            chunk.append([""] * 5)
        out.append("| " + " | ".join(" | ".join(c) for c in chunk) + " |")
    return out


def generate(db, classes, pack, maxname, maxtype):
    out = ["Source: DB table `struct_fields`, confidence high/medium only (c: h = high, m = medium); offsets are "
           "relative to the class base (a sub-object keeps its own class). Regenerate with "
           "`python3 tools/gen_struct_tables.py --pack %d --splice README.md`." % pack, ""]
    for c in classes:
        kept, low = fetch(db, c)
        out += render_class(c, kept, low, pack, maxname, maxtype)
        out.append("")
    return "\n".join(out).rstrip() + "\n"


def splice(path, body):
    with open(path, encoding="utf-8") as f:
        text = f.read()
    pat = re.compile(re.escape(BEGIN) + r".*?" + re.escape(END), re.S)
    if not pat.search(text):
        raise SystemExit("markers not found in %s" % path)
    new = pat.sub(lambda m: BEGIN + "\n" + body + END, text, count=1)
    with open(path, "w", encoding="utf-8") as f:
        f.write(new)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--db", default=DEFAULT_DB)
    ap.add_argument("--classes", default=",".join(DEFAULT_CLASSES))
    ap.add_argument("--pack", type=int, default=1, help="fields per table row (1..3)")
    ap.add_argument("--maxname", type=int, default=0, help="truncate names (0 = no limit; 44 when --pack > 1)")
    ap.add_argument("--maxtype", type=int, default=0, help="truncate types (0 = no limit; 16 when --pack > 1)")
    ap.add_argument("--splice", default=None, help="rewrite the marked block in this markdown file")
    args = ap.parse_args(argv)
    if not os.path.exists(args.db):
        print("knowledge DB not found: %s" % args.db, file=sys.stderr)
        return 2
    pack = max(1, min(3, args.pack))
    maxname = args.maxname or (44 if pack > 1 else 0)
    maxtype = args.maxtype or (16 if pack > 1 else 0)
    db = open_ro(args.db)
    classes = [c.strip() for c in args.classes.split(",") if c.strip()]
    body = generate(db, classes, pack, maxname, maxtype)
    if args.splice:
        splice(args.splice, body)
        print("spliced %d lines into %s" % (body.count("\n"), args.splice))
    else:
        sys.stdout.write(body)
    return 0


if __name__ == "__main__":
    signal = __import__("signal")
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)
    sys.exit(main())
