#!/usr/bin/env python3
"""Promote Haiku-verified ingest names (ingest_names.py score -> promote.tsv) into the KDB name_map.

INGEST plan phase I4.  Stdlib only.  Dry run by default; `--apply` writes in one transaction, runs
PRAGMA integrity_check and rolls back unless it returns ok.  Never overwrites: an address that already
has a name_map row (any confidence) is reported as `exists` (same name) or `conflict` (other name).

CLI:
  promote_names.py --promote promote.tsv --source ingest-modcode [--strict-key key.tsv
                   --strict-verdicts 'verdicts_[1-8].tsv'] [--confidence medium] [--date YYYY-MM-DD]
                   [--kdb DB] [--stage I2] [--apply]
--strict-*: keep only rows whose real (non-decoy) card was `consistent` in those verdict files too
(e.g. pass 1 of a retried wave), so a row promoted only by a retry is held back.
Exit codes: 0 success, 1 integrity or database failure, 2 bad input or arguments.
"""
from __future__ import annotations

import argparse
import datetime
import glob
import sqlite3
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ingest_names  # noqa: E402

KDB_DEFAULT = ingest_names.KDB_DEFAULT
EVIDENCE_MAX = 300


def strict_filter(rows, key_rows, verdicts):
    """Rows whose non-decoy card has verdict consistent in `verdicts` ({card: (verdict, quote)})."""
    ok = {(k["addr"], k["name"]) for k in key_rows
          if str(k["is_decoy"]) == "0" and verdicts.get(k["card"], ("",))[0] == "consistent"}
    return [r for r in rows if (r["addr"], r["name"]) in ok]


def plan(conn, rows, source, confidence, date, stage="I2"):
    """-> list of (action, row dict, existing name); action = insert | exists | conflict."""
    out = []
    for r in rows:
        addr = "0x%08x" % int(r["addr"], 16)
        cur = conn.execute("select name from name_map where steam_addr = ?", (addr,)).fetchone()
        if cur is None:
            ev = ("[rea] ingest %s Haiku+decoys: %s | src %s" % (stage, r.get("quote", ""), r.get("src", "")))
            out.append(("insert", {"steam_addr": addr, "name": r["name"], "source": source,
                                   "confidence": confidence, "evidence": ev[:EVIDENCE_MAX], "date": date}, None))
        else:
            out.append(("exists" if cur[0] == r["name"] else "conflict", {"steam_addr": addr, "name": r["name"]},
                        cur[0]))
    return out


def apply(conn, actions):
    """Insert planned rows in one transaction; -> integrity messages (['ok'] on success)."""
    try:
        with conn:
            conn.executemany(
                "insert into name_map (steam_addr, name, source, confidence, evidence, date) "
                "values (:steam_addr, :name, :source, :confidence, :evidence, :date)",
                [row for act, row, _ in actions if act == "insert"])
            msgs = [m[0] for m in conn.execute("PRAGMA integrity_check").fetchall()]
            if msgs != ["ok"]:
                raise sqlite3.DatabaseError("integrity_check: " + "; ".join(msgs[:5]))
    except sqlite3.DatabaseError as exc:
        return [str(exc)]
    return msgs


def main(argv=None, out=sys.stdout, err=sys.stderr):
    ap = argparse.ArgumentParser(prog="promote_names.py", description=__doc__.split("\n")[0])
    ap.add_argument("--promote", required=True)
    ap.add_argument("--source", required=True, help="name_map.source, e.g. ingest-modcode")
    ap.add_argument("--strict-key")
    ap.add_argument("--strict-verdicts")
    ap.add_argument("--confidence", default="medium")
    ap.add_argument("--date", default=None)
    ap.add_argument("--kdb", default=str(KDB_DEFAULT))
    ap.add_argument("--stage", default="I2", help="verify stage named in the evidence text (I2, I3, ...)")
    ap.add_argument("--apply", action="store_true")
    try:
        args = ap.parse_args(argv)
    except SystemExit as exc:
        return 0 if exc.code in (None, 0) else 2
    if bool(args.strict_key) != bool(args.strict_verdicts):
        print("promote_names: --strict-key and --strict-verdicts go together", file=err)
        return 2
    try:
        date = (datetime.date.fromisoformat(args.date) if args.date else datetime.date.today()).isoformat()
    except ValueError:
        print(f"promote_names: bad --date {args.date!r}", file=err)
        return 2
    rows = ingest_names.read_tsv(args.promote)
    n_in = len(rows)
    if args.strict_key:
        rows = strict_filter(rows, ingest_names.read_tsv(args.strict_key),
                             ingest_names.read_verdicts(sorted(glob.glob(args.strict_verdicts))))
    uri = "file:%s?mode=%s" % (args.kdb, "rw" if args.apply else "ro")
    try:
        conn = sqlite3.connect(uri, uri=True)
    except sqlite3.Error as exc:
        print(f"promote_names: {exc}", file=err)
        return 1
    try:
        actions = plan(conn, rows, args.source, args.confidence, date, args.stage)
        n = {a: sum(1 for x in actions if x[0] == a) for a in ("insert", "exists", "conflict")}
        verb = "inserted" if args.apply else "would-insert"
        print(f"{'apply' if args.apply else 'dry-run'}: in={n_in} kept={len(rows)} {verb}={n['insert']} "
              f"exists={n['exists']} conflict={n['conflict']}", file=out)
        for act, row, old in actions:
            if act == "conflict":
                print(f"  conflict {row['steam_addr']} {row['name']} (name_map has {old})", file=out)
        if args.apply:
            msgs = apply(conn, actions)
            if msgs != ["ok"]:
                print("integrity: FAILED (rolled back)", file=out)
                print(f"promote_names: {'; '.join(msgs)}", file=err)
                return 1
            print("integrity: ok", file=out)
    finally:
        conn.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
