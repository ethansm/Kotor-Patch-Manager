#!/usr/bin/env python3
"""REA findings ledger: read, lint, dedupe and append the TSV rows in rea_findings/.

A ledger is a directory of TSV files `<YYYYMMDD>_<topic>.tsv`. Line 1 is the header
(COLS); every other non-blank line is one finding. Stdlib only.

CLI: python3 rea_ledger.py lint [--dir D] [--strict]
"""
import argparse
import datetime
import os
import re
import sys
import tempfile
from collections import namedtuple
from pathlib import Path

COLS = ("addr", "kind", "name", "confidence", "evidence", "topic", "date")
KINDS = {"function", "global", "vtable", "string", "field", "callsite", "xref", "data", "table"}
CONF = {"confirmed", "probable", "refuted"}
ANCHOR_KINDS = {"function", "global", "vtable", "string", "field"}
DEFAULT_DIR = Path(__file__).resolve().parent.parent / "rea_findings"

HEADER = "\t".join(COLS)
Issue = namedtuple("Issue", "file line rule msg")

_ADDR_RE = re.compile(r"0x[0-9a-f]{1,8}")
_ADDR_ANY_RE = re.compile(r"0[xX][0-9a-fA-F]{1,8}")
_DATE_RE = re.compile(r"[0-9]{4}-[0-9]{2}-[0-9]{2}")
_ANCHOR_RE = re.compile(r"0x[0-9a-f]{6,8}: \S")
_TOPIC_RE = re.compile(r"[A-Za-z0-9_]+-q\d+")
_SUPERSEDES_RE = re.compile(r"supersedes:\s*([^\s:]+):(\d+)")


class LedgerError(Exception):
    """Ledger problem at file:line. str(err) is 'file:line: msg'; .rule names the lint rule if known."""

    def __init__(self, file, line, msg, rule=None):
        super().__init__(f"{file}:{line}: {msg}")
        self.file = file
        self.line = line
        self.msg = msg
        self.rule = rule


def _split(text):
    return [cell.strip() for cell in text.split("\t")]


def _is_header(text):
    return _split(text) == list(COLS)


def _load(path):
    """Return the file's lines without line endings; line N is lines[N - 1]."""
    text = Path(path).read_bytes().decode("utf-8", errors="replace")
    lines = text.split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    return [ln.rstrip("\r") for ln in lines]


def _data_lines(lines):
    """(line_no, raw) for each non-blank data line; the header on line 1 is skipped."""
    for no, raw in enumerate(lines[1:], start=2):
        if raw.strip():
            yield no, raw


def _ledger_files(dir):
    if not os.path.isdir(dir):
        raise FileNotFoundError(f"no such ledger dir: {dir}")
    return sorted((p for p in Path(dir).glob("*.tsv") if p.is_file()), key=lambda p: p.name)


def _addr_int(text):
    return int(text, 16) if _ADDR_ANY_RE.fullmatch(text) else None


def _valid_date(text):
    if not _DATE_RE.fullmatch(text):
        return False
    try:
        datetime.date.fromisoformat(text)
    except ValueError:
        return False
    return True


def read_rows(dir, strict=False):
    """All data rows of every *.tsv in dir, sorted by file name then line.

    Each dict has the COLS keys (cells stripped) plus "file" (basename) and "line" (1-based).
    Non-strict: files with a non-matching header and rows with != 7 cells or an empty addr are skipped.
    Strict: those raise LedgerError(file, line) instead.
    """
    rows = []
    for path in _ledger_files(dir):
        lines = _load(path)
        if not lines or not _is_header(lines[0]):
            if strict:
                raise LedgerError(path.name, 1, "L4 header is not the ledger header", rule="L4")
            continue
        for no, raw in _data_lines(lines):
            cells = _split(raw)
            if len(cells) != len(COLS):
                if strict:
                    raise LedgerError(path.name, no, f"L4 expected {len(COLS)} cells, got {len(cells)}", rule="L4")
                continue
            if not cells[0]:
                if strict:
                    raise LedgerError(path.name, no, "L5 addr is empty", rule="L5")
                continue
            row = dict(zip(COLS, cells))
            row["file"] = path.name
            row["line"] = no
            rows.append(row)
    return rows


def _row_issues(row):
    """Per-row rules L1, L2, L5 for a split row that carries "file" and "line"."""
    f, n = row["file"], row["line"]
    out = []
    if row["kind"] not in KINDS:
        out.append(Issue(f, n, "L1", f"kind {row['kind']!r} not in KINDS"))
    if row["kind"] in ANCHOR_KINDS and not _ANCHOR_RE.search(row["evidence"]):
        out.append(Issue(f, n, "L2", f"{row['kind']} evidence lacks a '0x<6-8 hex>: <quote>' anchor"))
    if not _ADDR_RE.fullmatch(row["addr"]):
        out.append(Issue(f, n, "L5", f"addr {row['addr']!r} is not lowercase 0x hex"))
    if not _valid_date(row["date"]):
        out.append(Issue(f, n, "L5", f"date {row['date']!r} is not YYYY-MM-DD"))
    if row["confidence"] not in CONF:
        out.append(Issue(f, n, "L5", f"confidence {row['confidence']!r} not in CONF"))
    return out


def _cross_issues(rows):
    """Cross-row rules L6 (confirmed vs refuted conflicts) and L8 (function needs xref)."""
    out = []
    groups = {}
    for r in rows:
        a = _addr_int(r["addr"])
        if a is not None:
            groups.setdefault((a, r["kind"]), []).append(r)
    for grp in groups.values():
        confirmed = [r for r in grp if r["confidence"] == "confirmed" and "supersedes:" not in r["evidence"]]
        if not confirmed:
            continue
        for r in grp:
            if r["confidence"] != "refuted" or "supersedes:" in r["evidence"]:
                continue
            c = confirmed[0]
            out.append(Issue(r["file"], r["line"], "L6",
                             f"refuted row conflicts with confirmed row {c['file']}:{c['line']}; "
                             "add 'supersedes: <file>:<line>' to one of them"))
    xrefs = {(r["file"], _addr_int(r["addr"])) for r in rows if r["kind"] == "xref"}
    for r in rows:
        if r["kind"] != "function" or not _TOPIC_RE.fullmatch(r["topic"]):
            continue
        a = _addr_int(r["addr"])
        if a is not None and (r["file"], a) not in xrefs:
            out.append(Issue(r["file"], r["line"], "L8",
                             f"function {r['addr']} (topic {r['topic']}) has no xref row with the same addr in this file"))
    return out


def lint(dir):
    """Run rules L1 L2 L4 L5 L6 L8 over every *.tsv in dir; returns Issues sorted by (file, line, rule)."""
    issues = []
    checked = []
    for path in _ledger_files(dir):
        name = path.name
        lines = _load(path)
        if not lines or not _is_header(lines[0]):
            issues.append(Issue(name, 1, "L4", "header is not the ledger header; rows not checked"))
            continue
        for no, raw in _data_lines(lines):
            cells = _split(raw)
            if len(cells) != len(COLS):
                issues.append(Issue(name, no, "L4", f"expected {len(COLS)} cells, got {len(cells)}"))
                continue
            row = dict(zip(COLS, cells), file=name, line=no)
            issues.extend(_row_issues(row))
            checked.append(row)
    issues.extend(_cross_issues(checked))
    return sorted(issues, key=lambda i: (i.file, i.line, i.rule))


def append_rows(dir, file, rows):
    """Append rows (dicts with COLS keys) to dir/file atomically; returns the number written.

    Every row is validated (L1, L2, L4, L5) before anything is written. Existing bytes are kept
    verbatim: the new content goes to a temp file in dir, is fsynced, then os.replace'd over the target.
    """
    if (not isinstance(file, str) or not file or os.path.basename(file) != file
            or "\\" in file or ".." in file or not file.endswith(".tsv")):
        raise ValueError(f"file must be a bare <name>.tsv basename, got {file!r}")
    if not os.path.isdir(dir):
        raise FileNotFoundError(f"no such ledger dir: {dir}")
    rows = list(rows)
    if not rows:
        return 0
    path = os.path.join(dir, file)
    existing = Path(path).read_bytes() if os.path.exists(path) else b""
    if existing:
        first = existing.split(b"\n", 1)[0].decode("utf-8", errors="replace").rstrip("\r")
        if not _is_header(first):
            raise LedgerError(file, 1, "L4 existing header is not the ledger header; refusing to append", rule="L4")
        ends_nl = existing.endswith(b"\n")
        prefix = existing if ends_nl else existing + b"\n"
        next_line = existing.count(b"\n") + (0 if ends_nl else 1) + 1
    else:
        prefix = (HEADER + "\n").encode("utf-8")
        next_line = 2

    new_lines = []
    for i, row in enumerate(rows):
        no = next_line + i
        cells = []
        for col in COLS:
            if col not in row:
                raise LedgerError(file, no, f"L4 row is missing column {col!r}", rule="L4")
            value = row[col]
            if not isinstance(value, str):
                raise LedgerError(file, no, f"L4 column {col!r} is not a string", rule="L4")
            if "\t" in value or "\n" in value or "\r" in value:
                raise LedgerError(file, no, f"L4 column {col!r} contains a tab or newline", rule="L4")
            cells.append(value.strip())
        issues = _row_issues(dict(zip(COLS, cells), file=file, line=no))
        if issues:
            raise LedgerError(file, no, f"{issues[0].rule} {issues[0].msg}", rule=issues[0].rule)
        new_lines.append("\t".join(cells) + "\n")

    payload = prefix + "".join(new_lines).encode("utf-8")
    mode = os.stat(path).st_mode & 0o777 if os.path.exists(path) else 0o644
    tmp_path = None
    try:
        with tempfile.NamedTemporaryFile(dir=dir, prefix="." + file + ".", suffix=".tmp", delete=False) as tmp:
            tmp_path = tmp.name
            tmp.write(payload)
            tmp.flush()
            os.fsync(tmp.fileno())
        os.chmod(tmp_path, mode)
        os.replace(tmp_path, path)
    except BaseException:
        if tmp_path is not None:
            try:
                os.unlink(tmp_path)
            except OSError:
                pass
        raise
    return len(new_lines)


def dedupe(rows):
    """Collapse rows by (addr int, kind, name) and flag superseded rows.

    Per key the last row is kept. If that last row is refuted, every earlier confirmed or
    probable row of the key is kept too with superseded=True. A row whose evidence contains
    `supersedes: <file>:<line>` marks the row at that file:line superseded=True.
    Returns copies with a bool "superseded" key, in (file, line) order.
    """
    ordered = sorted(rows, key=lambda r: (r.get("file", ""), r.get("line", 0)))
    marked = set()
    for r in ordered:
        for tfile, tline in _SUPERSEDES_RE.findall(r.get("evidence", "")):
            marked.add((tfile, int(tline)))
    groups = {}
    for r in ordered:
        a = _addr_int(r["addr"])
        key = (a if a is not None else r["addr"], r["kind"], r["name"])
        groups.setdefault(key, []).append(r)
    picked = []
    for grp in groups.values():
        last = grp[-1]
        picked.append((last, False))
        if last["confidence"] == "refuted":
            picked.extend((r, True) for r in grp[:-1] if r["confidence"] in ("confirmed", "probable"))
    out = []
    for r, flag in picked:
        d = dict(r)
        d["superseded"] = bool(flag or (r.get("file", ""), r.get("line", 0)) in marked)
        out.append(d)
    return sorted(out, key=lambda r: (r.get("file", ""), r.get("line", 0)))


def main(argv=None):
    parser = argparse.ArgumentParser(prog="rea_ledger.py", description="REA findings ledger tools.")
    sub = parser.add_subparsers(dest="cmd", required=True)
    lint_p = sub.add_parser("lint", help="check ledger rules L1 L2 L4 L5 L6 L8")
    lint_p.add_argument("--dir", default=str(DEFAULT_DIR), help="ledger dir (default: rea_findings/)")
    lint_p.add_argument("--strict", action="store_true", help="also fail if read_rows(strict=True) raises")
    try:
        args = parser.parse_args(argv)
    except SystemExit as exc:
        return 0 if exc.code in (None, 0) else 2
    if not os.path.isdir(args.dir):
        print(f"rea_ledger: no such ledger dir: {args.dir}", file=sys.stderr)
        return 2
    issues = lint(args.dir)
    for i in issues:
        print(f"{i.file}:{i.line} {i.rule} {i.msg}")
    print(f"{len(issues)} issues")
    rc = 1 if issues else 0
    if args.strict:
        try:
            read_rows(args.dir, strict=True)
        except LedgerError as exc:
            print(f"strict: {exc}")
            rc = 1
    return rc


if __name__ == "__main__":
    sys.exit(main())
