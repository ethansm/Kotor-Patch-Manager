#!/usr/bin/env python3
"""export_dump_inputs.py - re-export ROOT/inputs/{name_map,db_functions,candidates,lessons_recent}.tsv from the DB.

The DB is opened read-only.  Formats mimic the 2026-09-30 exports exactly (header line, TAB separated, no quoting,
NULL -> empty, trailing newline; embedded TAB/CR/LF in a text cell are flattened to a single space - the existing
files contain none):
  name_map.tsv       steam_addr name source confidence evidence          order by steam_addr
  db_functions.tsv   address symbol size purpose notes                  functions of the Steam binary
                     (binary_hash starts with 6a522e71), order by address
  candidates.tsv     id title subsystem hypothesis evidence status      DB insertion (rowid) order
  lessons_recent.tsv id scope topic note                                lessons with id >= --lessons-min-id
                     (default 150 = the threshold the 09-30 export used: 64 lines, ids 150..211; pass 140 for a wider window)

Before overwriting a file, the old one is moved to <name>.tsv.0930 only if that backup does not exist yet.
--dry-run writes to a temp dir instead and prints row counts + a diff summary against the current files.
"""
import argparse
import os
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import atlas_common as ac  # noqa: E402

BACKUP_SUFFIX = ".0930"


def clean(v):
    if v is None:
        return ""
    s = str(v)
    return s.replace("\r", "").replace("\t", " ").replace("\n", " ")


def build_exports(db, lessons_min_id, hash_prefix=ac.STEAM_HASH_PREFIX):
    """-> {filename: (header_list, rows_list_of_lists)}"""
    q = db.execute
    return {
        "name_map.tsv": (
            ["steam_addr", "name", "source", "confidence", "evidence"],
            q("select steam_addr, name, source, confidence, evidence from name_map order by steam_addr").fetchall()),
        "db_functions.tsv": (
            ["address", "symbol", "size", "purpose", "notes"],
            q("select address, symbol, size, purpose, notes from functions where binary_hash like ? order by address",
              (hash_prefix + "%",)).fetchall()),
        "candidates.tsv": (
            ["id", "title", "subsystem", "hypothesis", "evidence", "status"],
            q("select id, title, subsystem, hypothesis, evidence, status from candidates order by rowid").fetchall()),
        "lessons_recent.tsv": (
            ["id", "scope", "topic", "note"],
            q("select id, scope, topic, note from lessons where id >= ? order by id", (lessons_min_id,)).fetchall()),
    }


def render(header, rows):
    out = ["\t".join(header)]
    for r in rows:
        out.append("\t".join(clean(x) for x in r))
    return "\n".join(out) + "\n"


def read_lines(path):
    try:
        with open(path, encoding="utf-8", errors="replace", newline="") as f:
            data = f.read()
    except OSError:
        return None
    ls = data.split("\n")
    if ls and ls[-1] == "":
        ls.pop()
    return ls


def diff_summary(old_lines, new_lines):
    """Compare by first column (key). -> dict(added, removed, changed, same)."""
    def keyed(ls):
        d = {}
        for l in ls[1:]:
            d.setdefault(l.split("\t", 1)[0], l)
        return d
    o, n = keyed(old_lines), keyed(new_lines)
    added = [k for k in n if k not in o]
    removed = [k for k in o if k not in n]
    changed = [k for k in n if k in o and n[k] != o[k]]
    return {"added": added, "removed": removed, "changed": changed, "same": len(n) - len(added) - len(changed)}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ac.add_common_args(ap)
    ap.add_argument("--lessons-min-id", type=int, default=150)
    ap.add_argument("--dry-run", action="store_true", help="write to a temp dir; touch nothing under ROOT")
    args = ap.parse_args(argv)
    root = ac.expand_root(args.root)
    inputs = os.path.join(root, "inputs")
    db = ac.open_db_ro(args.db)
    if db is None:
        print("knowledge DB not found", file=sys.stderr)
        return 2
    exports = build_exports(db, args.lessons_min_id)
    if args.dry_run:
        outdir = tempfile.mkdtemp(prefix="dump_inputs_dry_")
    else:
        outdir = inputs
        os.makedirs(outdir, exist_ok=True)
    print("%s %s" % ("DRY RUN ->" if args.dry_run else "writing ->", outdir))
    for name, (header, rows) in exports.items():
        cur = os.path.join(inputs, name)
        old = read_lines(cur)
        text = render(header, rows)
        new_lines = text.rstrip("\n").split("\n")
        if old is not None and old[0] != new_lines[0]:
            print("WARNING %s: header differs from existing file: %r vs %r" % (name, old[0], new_lines[0]))
        target = os.path.join(outdir, name)
        note = ""
        if not args.dry_run and os.path.exists(cur):
            bak = cur + BACKUP_SUFFIX
            if not os.path.exists(bak):
                shutil.copy2(cur, bak)
                note = " (backup -> %s)" % os.path.basename(bak)
            else:
                note = " (backup %s already exists, kept)" % os.path.basename(bak)
        tmp = target + ".tmp"
        with open(tmp, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        os.replace(tmp, target)
        before = (len(old) - 1) if old is not None else 0
        print("%-20s rows before=%d after=%d%s" % (name, before, len(rows), note))
        if old is not None:
            d = diff_summary(old, new_lines)
            print("    vs existing: same=%d changed=%d added=%d removed=%d" % (
                d["same"], len(d["changed"]), len(d["added"]), len(d["removed"])))
            if d["removed"]:
                print("    removed keys (first 10): %s" % ", ".join(d["removed"][:10]))
    return 0


if __name__ == "__main__":
    import signal
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)  # quiet when piped into head
    sys.exit(main())
