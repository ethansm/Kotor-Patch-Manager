#!/usr/bin/env python3
"""Persist validated function purposes from a TSV into the functions table of kotor2_ghidra_knowledge.db.

Usage:
  persist_purposes.py PURPOSES.tsv [--kdb PATH] [--source NAME] [--date YYYY-MM-DD] [--apply]

Input TSV: header row with required columns addr and purpose; optional symbol, size, decision.
If a decision column exists only rows with decision keep or fix are used. Other columns are ignored.

Dry-run is the default (read-only connection, writes nothing). --apply writes in one transaction and
rolls back if PRAGMA integrity_check does not return ok.

Ownership: rows written here carry notes 'persist_purposes <source>'. A row with a non-empty purpose and
no persist_purposes token in its notes is human-owned and is never touched (reported as protected).

Exit codes: 0 success, 1 integrity or database failure, 2 bad input or arguments.
"""
import argparse
import csv
import datetime as dt
import sqlite3
import sys
from collections import Counter, namedtuple
from pathlib import Path

BINARY_HASH = "6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef"
DEFAULT_KDB = Path(__file__).resolve().parents[1] / "kotor2_ghidra_knowledge.db"
OWNER_TOKEN = "persist_purposes"
KEEP_DECISIONS = {"keep", "fix"}

Row = namedtuple("Row", "addr purpose symbol size")
Action = namedtuple("Action", "kind addr sql params")


class InputError(Exception):
    """Bad TSV, bad arguments or missing database. Exit code 2."""


def is_blank(value):
    return value is None or str(value).strip() == ""


def normalise_addr(raw):
    text = raw.strip().lower()
    if text.startswith("0x"):
        text = text[2:]
    if not text or len(text) > 8 or any(c not in "0123456789abcdef" for c in text):
        raise InputError(f"bad address {raw!r}")
    return "%08x" % int(text, 16)


def read_tsv(path):
    try:
        fh = open(path, newline="", encoding="utf-8-sig")
    except OSError as exc:
        raise InputError(f"cannot read {path}: {exc.strerror}") from exc
    with fh:
        reader = csv.reader(fh, delimiter="\t")
        try:
            header = [h.strip() for h in next(reader)]
        except StopIteration:
            raise InputError(f"{path}: empty file") from None
        missing = [c for c in ("addr", "purpose") if c not in header]
        if missing:
            raise InputError(f"{path}: missing column(s) {', '.join(missing)}")
        index = {name: i for i, name in enumerate(header)}
        has_decision = "decision" in index

        def cell(fields, name):
            i = index.get(name)
            return fields[i].strip() if i is not None and i < len(fields) else ""

        rows, seen = [], {}
        for fields in reader:
            if not any(f.strip() for f in fields):
                continue
            line = reader.line_num
            if has_decision and cell(fields, "decision").lower() not in KEEP_DECISIONS:
                continue
            try:
                addr = normalise_addr(cell(fields, "addr"))
            except InputError as exc:
                raise InputError(f"{path}:{line}: {exc}") from None
            if addr in seen:
                raise InputError(f"{path}:{line}: duplicate addr 0x{addr} (first at line {seen[addr]})")
            seen[addr] = line
            purpose = cell(fields, "purpose")
            if not purpose:
                continue
            size_text = cell(fields, "size")
            try:
                size = int(size_text) if size_text else None
            except ValueError:
                raise InputError(f"{path}:{line}: size {size_text!r} is not an integer") from None
            rows.append(Row(addr, purpose, cell(fields, "symbol") or None, size))
    return rows


def plan_actions(conn, rows, marker, date):
    actions = []
    for row in rows:
        found = conn.execute(
            "SELECT symbol, size, purpose, notes FROM functions WHERE binary_hash=? AND address=?",
            (BINARY_HASH, row.addr),
        ).fetchone()
        if found is None:
            actions.append(Action(
                "insert", row.addr,
                "INSERT INTO functions (binary_hash, address, symbol, size, purpose, notes, discovered_date)"
                " VALUES (?,?,?,?,?,?,?)",
                (BINARY_HASH, row.addr, row.symbol, row.size, row.purpose, marker, date),
            ))
            continue
        symbol, size, purpose, notes = found
        if is_blank(purpose):
            new_symbol = symbol if not is_blank(symbol) else (row.symbol or symbol)
            new_size = size if size is not None else row.size
            if is_blank(notes):
                new_notes = marker
            elif marker in notes:
                new_notes = notes
            else:
                new_notes = f"{notes} ; {marker}"
            actions.append(Action(
                "update", row.addr,
                "UPDATE functions SET purpose=?, symbol=?, size=?, notes=? WHERE binary_hash=? AND address=?",
                (row.purpose, new_symbol, new_size, new_notes, BINARY_HASH, row.addr),
            ))
        elif OWNER_TOKEN in (notes or ""):
            if purpose == row.purpose:
                actions.append(Action("same", row.addr, None, None))
            else:
                actions.append(Action(
                    "update", row.addr,
                    "UPDATE functions SET purpose=? WHERE binary_hash=? AND address=?",
                    (row.purpose, BINARY_HASH, row.addr),
                ))
        else:
            actions.append(Action("protected", row.addr, None, None))
    return actions


def integrity_check(conn):
    return [r[0] for r in conn.execute("PRAGMA integrity_check").fetchall()]


def run(kdb, rows, apply, marker, date):
    """Return (actions, integrity messages, committed)."""
    if apply:
        conn = sqlite3.connect(str(kdb), isolation_level=None, timeout=30)
    else:
        conn = sqlite3.connect(kdb.resolve().as_uri() + "?mode=ro", uri=True)
    committed = False
    try:
        if apply:
            conn.execute("BEGIN IMMEDIATE")
            try:
                actions = plan_actions(conn, rows, marker, date)
                for action in actions:
                    if action.sql:
                        conn.execute(action.sql, action.params)
                messages = integrity_check(conn)
            except BaseException:
                conn.execute("ROLLBACK")
                raise
            if messages == ["ok"]:
                conn.execute("COMMIT")
                committed = True
            else:
                conn.execute("ROLLBACK")
        else:
            actions = plan_actions(conn, rows, marker, date)
            messages = integrity_check(conn)
    finally:
        conn.close()
    return actions, messages, committed


def main(argv=None):
    parser = argparse.ArgumentParser(description="Persist validated function purposes into the KDB functions table.")
    parser.add_argument("tsv", help="validated purposes TSV (header must include addr and purpose)")
    parser.add_argument("--kdb", default=str(DEFAULT_KDB), help="knowledge database path (default: %(default)s)")
    parser.add_argument("--apply", action="store_true", help="write changes (default is dry-run)")
    parser.add_argument("--source", help="name in the notes marker (default: TSV basename)")
    parser.add_argument("--date", help="discovered_date for inserted rows, YYYY-MM-DD (default: today)")
    args = parser.parse_args(argv)

    try:
        date = dt.date.fromisoformat(args.date).isoformat() if args.date else dt.date.today().isoformat()
    except ValueError:
        print(f"persist_purposes: error: bad --date {args.date!r}", file=sys.stderr)
        return 2
    kdb = Path(args.kdb)
    try:
        if not kdb.is_file():
            raise InputError(f"database not found: {kdb}")
        marker = f"{OWNER_TOKEN} {args.source or Path(args.tsv).name}"
        rows = read_tsv(Path(args.tsv))
        actions, messages, committed = run(kdb, rows, args.apply, marker, date)
    except InputError as exc:
        print(f"persist_purposes: error: {exc}", file=sys.stderr)
        return 2
    except sqlite3.Error as exc:
        print(f"persist_purposes: database error: {exc}", file=sys.stderr)
        return 1

    counts = Counter(a.kind for a in actions)
    for action in actions:
        print(f"{action.kind} 0x{action.addr}")
    if not args.apply:
        mode = "dry-run"
    else:
        mode = "applied" if committed else "rolled back"
    print(
        f"persist_purposes: {len(actions)} rows, insert {counts['insert']}, update {counts['update']}, "
        f"same {counts['same']}, protected {counts['protected']} ({mode})"
    )
    if messages == ["ok"]:
        print("integrity: ok")
        return 0
    print("integrity: FAILED", file=sys.stdout)
    print(f"persist_purposes: integrity_check: {'; '.join(messages[:5])}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
