#!/usr/bin/env python3
"""Per-address live-only facts from REA, appended to the REA ledger.

Per address, in order:
  1. Static indirect call sites from the dump (no REA): kind callsite. Candidates
     come from KDB class_methods at the vtable slot. Confidence is confirmed only
     when exactly one candidate's class equals the address's class in the Steam DB.
  2. Live body (analyze_function): kind function when the body is non-contiguous
     or larger than the static size.
  3. Live data refs (xrefs): kind xref for sources that are not static code call
     sites of the address.
  4. When steps 2-3 add nothing: kind xref marker "no live-only facts", keyed to
     the address so resume can skip it.

Rows for one address go out in one append, so a crash keeps finished addresses.
A dead REA session stops the run with exit 1; re-running the same command resumes.
A per-address REA error becomes an "error: ..." xref row and the run continues.

Usage:
  rea_facts.py run (--addrs FILE | --keys TSV [--keys TSV ...]) --topic <B>-s<k>
      [--max-sites 8] [--ledger-dir D] [--dump-root D] [--cache-dir D] [--lock-dir D]
      [--kdb DB] [--steam-db DB] [--no-live] [--cmd SHLEX] [--no-preflight]

Exit codes: 0 done, 1 REA session lost or unavailable (re-run to resume), 2 usage or topic collision.
"""

import argparse
import csv
import datetime as dt
import os
import re
import shlex
import sqlite3
import sys
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import rea_ledger as rl  # noqa: E402
import static_facts as sf  # noqa: E402
from rea_session import ReaError, ReaGuardError, ReaSession, guard, preflight  # noqa: E402

BINARY_HASH_LIKE = "6a522e71%"  # swkotor2.exe sha256 prefix used in the KDB
TOPIC_RE = re.compile(r"^[A-Za-z0-9]+-s\d+$")
ASM_LINE_RE = re.compile(r"^\s*0x([0-9a-fA-F]+):\s*(.*?)\s*$")
MAX_CANDIDATES = 6
DATA_REF_SAMPLE = 8
SESSION_LOST_MARKERS = ("connection lost", "stdout closed", "write to REA", "REA exited", "Broken pipe")
DEFAULT_KDB = TOOLS_DIR.parent / "kotor2_ghidra_knowledge.db"
DEFAULT_STEAM_DB = (Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db"
                    / "AddressDatabases/kotor2_steam_aspyr.db")


class UsageError(Exception):
    """Bad arguments, missing DB, or topic collision. Exit 2."""


class SessionLost(Exception):
    """REA went away mid-address. Stop the run, keep finished addresses. Exit 1."""


def _clean(text):
    return re.sub(r"[\t\r\n]+", " ", str(text)).strip()


def _to_int(value):
    if isinstance(value, int) and not isinstance(value, bool):
        return value
    return int(str(value).strip(), 16)


def _addr_text(value):
    try:
        return "0x%08x" % _to_int(value)
    except (TypeError, ValueError):
        return _clean(value)


def parse_addr(text):
    value = _to_int(text)
    if not 0 <= value <= 0xFFFFFFFF:
        raise ValueError("address out of range: %s" % text)
    return value


def load_addrs(path):
    """One hex address per line; blank and # lines skipped."""
    addrs = []
    try:
        lines = Path(path).read_text(encoding="utf-8").splitlines()
    except OSError as exc:
        raise UsageError("cannot read %s: %s" % (path, exc))
    for lineno, raw in enumerate(lines, 1):
        text = raw.strip()
        if not text or text.startswith("#"):
            continue
        token = text.split()[0]
        try:
            addrs.append(parse_addr(token))
        except ValueError:
            raise UsageError("%s:%d: not a hex address: %r" % (path, lineno, token))
    return addrs


def load_keys(path):
    """TSV with a steam_addr column. No other column is read (blindness to kind/gog/card_id)."""
    addrs = []
    try:
        handle = open(path, newline="", encoding="utf-8")
    except OSError as exc:
        raise UsageError("cannot read %s: %s" % (path, exc))
    with handle:
        reader = csv.DictReader(handle, delimiter="\t")
        if not reader.fieldnames or "steam_addr" not in reader.fieldnames:
            raise UsageError("%s: no steam_addr column" % path)
        for lineno, row in enumerate(reader, 2):
            text = (row.get("steam_addr") or "").strip()
            if not text:
                continue
            try:
                addrs.append(parse_addr(text))
            except ValueError:
                raise UsageError("%s:%d: bad steam_addr %r" % (path, lineno, text))
    return addrs


def open_ro(path, label):
    db = Path(path)
    if not db.is_file():
        raise UsageError("%s DB not found: %s (set the env var or pass the option)" % (label, db))
    return sqlite3.connect(db.resolve().as_uri() + "?mode=ro", uri=True)


def candidates(kdb, slot):
    """Sorted (class, symbol, address) for vtable slot; slot = vtable_index * 4."""
    if slot % 4:
        return []
    rows = kdb.execute(
        "SELECT class_name, symbol, address FROM class_methods "
        "WHERE binary_hash LIKE ? AND vtable_index = ?",
        (BINARY_HASH_LIKE, slot // 4)).fetchall()
    return sorted((c or "?", s or "?", _addr_text(a)) for c, s, a in rows)


def steam_classes(steam, a):
    rows = steam.execute("SELECT DISTINCT class_name FROM functions WHERE address = ?", (a,)).fetchall()
    return {r[0] for r in rows if r[0]}


def is_session_lost(exc):
    if isinstance(exc, ReaGuardError):
        return True
    text = str(exc)
    return any(marker in text for marker in SESSION_LOST_MARKERS)


def _key(row):
    return (_to_int(row["addr"]), row["kind"], row["name"], row["evidence"])


class Facts:
    """One run: static facts, live facts through an optional session, ledger writes."""

    def __init__(self, *, topic, ledger_dir, fname, existing, dump, kdb, steam, max_sites, session):
        self.topic = topic
        self.ledger_dir = ledger_dir
        self.fname = fname
        self.date = dt.date.today().isoformat()
        self.dump = dump
        self.kdb = kdb
        self.steam = steam
        self.max_sites = max_sites
        self.session = session
        self.keys = {_key(r) for r in existing}
        self.done = {_to_int(r["addr"]) for r in existing if r["kind"] != "callsite"}
        self.written = 0

    def row(self, addr, kind, name, confidence, evidence):
        return {"addr": "0x%08x" % addr, "kind": kind, "name": _clean(name),
                "confidence": confidence, "evidence": _clean(evidence),
                "topic": self.topic, "date": self.date}

    # Step 1: static indirect call sites.
    def static_rows(self, a):
        rows = []
        classes = steam_classes(self.steam, a)
        sites = sorted(self.dump.indirect_slots(a), key=lambda t: t[0])[: self.max_sites]
        for site, slot, how in sites:
            evidence = "static_facts 0x%08x: %s" % (site, how)
            if slot is None:
                name, confidence = "slot unknown (%s)" % how, "probable"
            else:
                cands = candidates(self.kdb, slot)
                shown = ", ".join("%s::%s@%s" % c for c in cands[:MAX_CANDIDATES]) or "none"
                same = sum(1 for cls, _sym, _addr in cands if cls in classes)
                name = "slot 0x%x candidates: %s" % (slot, shown)
                confidence = "confirmed" if same == 1 else "probable"
            rows.append(self.row(site, "callsite", name, confidence, evidence))
        return rows

    # Step 2: live body ranges.
    def body_rows(self, a):
        res = self.session.call("analyze_function", {"procedure": "0x%08x" % a}).get("normalized_result") or {}
        body = (res.get("procedure") or {}).get("body") or {}
        ranges = body.get("ranges") or []
        if not ranges:
            return []
        static = self.dump.size_ret(a)
        static_size = static[0] if static else None
        span = body.get("span_bytes")
        grew = static_size is not None and isinstance(span, int) and span > static_size
        if not (body.get("non_contiguous") or grew):
            return []
        spans = sorted((_to_int(r["start"]), _to_int(r["end"])) for r in ranges)
        name = "body ranges %s total %s span %s" % (
            ";".join("0x%x-0x%x" % s for s in spans), body.get("total_bytes"), span)
        evidence = "analyze_function 0x%08x: %s" % (a, self.last_ret(a))
        return [self.row(a, "function", name, "confirmed", evidence)]

    def last_ret(self, a):
        text = self.session.call("procedure_assembly", {"procedure": "0x%08x" % a}).get("normalized_result")
        lines = text.splitlines() if isinstance(text, str) else [str(x) for x in (text or [])]
        found = None
        for line in lines:
            match = ASM_LINE_RE.match(line)
            if match and match.group(2).upper().startswith("RET"):
                found = "0x%08x: %s" % (int(match.group(1), 16), match.group(2))
        return found or "no RET line in procedure_assembly"

    # Step 3: live data refs (xrefs that are not static code call sites).
    def xref_rows(self, a):
        res = self.session.call("xrefs", {"address": "0x%08x" % a}).get("normalized_result") or []
        sources = sorted({_to_int(s) for s in res})
        calls = {site for site, _caller in self.dump.callers(a)}
        data = [s for s in sources if s not in calls]
        if not data:
            return []
        code = len(sources) - len(data)
        name = "data refs %d: %s (first 8)" % (
            len(data), ",".join("0x%x" % s for s in data[:DATA_REF_SAMPLE]))
        evidence = "xrefs 0x%08x: %d sources, %d code call sites" % (a, len(sources), code)
        return [self.row(a, "xref", name, "confirmed", evidence)]

    def batch(self, a):
        rows = self.static_rows(a)
        if self.session is None:
            return rows
        live, step = [], "body"
        try:
            live += self.body_rows(a)
            step = "xrefs"
            live += self.xref_rows(a)
        except ReaError as exc:
            if is_session_lost(exc):
                raise SessionLost(_clean(exc)) from exc
            error = self.row(a, "xref", "error: %s" % _clean(exc)[:80], "probable",
                             "rea_facts 0x%08x: %s failed" % (a, step))
            return rows + live + [error]
        if not live:
            live.append(self.row(a, "xref", "no live-only facts", "probable",
                                 "rea_facts 0x%08x: checked" % a))
        return rows + live

    def flush(self, batch):
        fresh, seen = [], set()
        for row in batch:
            key = _key(row)
            if key in self.keys or key in seen:
                continue
            seen.add(key)
            fresh.append(row)
        if fresh:
            self.ledger_dir.mkdir(parents=True, exist_ok=True)
            rl.append_rows(self.ledger_dir, self.fname, fresh)
            self.keys |= seen
            self.written += len(fresh)
        return fresh

    def run(self, addrs):
        """Return (skipped, lost). lost is (addr, message) when the session died."""
        skipped, lost = 0, None
        for a in addrs:
            if self.session is not None and a in self.done:
                skipped += 1
                continue
            try:
                batch = self.batch(a)
            except SessionLost as exc:
                lost = (a, str(exc))
                break
            fresh = self.flush(batch)
            if batch and not fresh:
                skipped += 1
        return skipped, lost


def _report(facts, addrs, skipped, lost):
    print("addresses %d, rows %d, skipped %d (resume)" % (len(addrs), facts.written, skipped))
    if lost:
        a, message = lost
        print("rea_facts: REA session lost at 0x%08x (%s); finished addresses are kept. "
              "Re-run the same command to resume." % (a, message), file=sys.stderr)
        return 1
    return 0


def _session_kwargs(args, overrides):
    kwargs = {"read_only": True}
    if args.cmd:
        kwargs["cmd"] = shlex.split(args.cmd)
    if args.cache_dir:
        kwargs["cache_dir"] = Path(args.cache_dir)
    if args.lock_dir:
        kwargs["lock_dir"] = Path(args.lock_dir)
    kwargs.update(overrides)
    return kwargs


def _execute(args, overrides):
    if not TOPIC_RE.match(args.topic):
        raise UsageError("topic must look like <B>-s<k>, got %r" % args.topic)
    if args.max_sites < 1:
        raise UsageError("--max-sites must be >= 1")
    if args.addrs:
        addrs = load_addrs(args.addrs)
    else:
        addrs = [a for path in args.keys for a in load_keys(path)]
    addrs = sorted(set(addrs))
    if not addrs:
        raise UsageError("no addresses to process")

    ledger_dir = Path(args.ledger_dir)
    fname = "%s_%s.tsv" % (dt.date.today().strftime("%Y%m%d"), args.topic)
    everything = rl.read_rows(ledger_dir) if ledger_dir.is_dir() else []
    clash = sorted({Path(r["file"]).name for r in everything
                    if r["topic"] == args.topic and Path(r["file"]).name != fname})
    if clash:
        raise UsageError("topic %s is already used in %s; pick a new topic or resume that file"
                         % (args.topic, ", ".join(clash)))
    existing = [r for r in everything if Path(r["file"]).name == fname]

    dump_root = Path(args.dump_root)
    if not dump_root.is_dir():
        raise UsageError("dump root not found: %s" % dump_root)
    dump = sf.Dump(root=dump_root)
    kdb = open_ro(args.kdb, "KDB")
    try:
        steam = open_ro(args.steam_db, "Steam")
        try:
            def facts_for(session):
                return Facts(topic=args.topic, ledger_dir=ledger_dir, fname=fname, existing=existing,
                             dump=dump, kdb=kdb, steam=steam, max_sites=args.max_sites, session=session)

            if args.no_live:
                facts = facts_for(None)
                return _report(facts, addrs, *facts.run(addrs))

            kwargs = _session_kwargs(args, overrides)
            try:
                guard()
                if not args.no_preflight:
                    preflight(**{k: kwargs[k] for k in ("exe",) if k in kwargs})
                with ReaSession(**kwargs) as session:
                    facts = facts_for(session)
                    return _report(facts, addrs, *facts.run(addrs))
            except ReaError as exc:
                print("rea_facts: REA unavailable: %s" % exc, file=sys.stderr)
                return 1
        finally:
            steam.close()
    finally:
        kdb.close()


def parse_args(argv):
    parser = argparse.ArgumentParser(prog="rea_facts.py", description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    run = sub.add_parser("run", help="write live-only facts for addresses into the REA ledger")
    source = run.add_mutually_exclusive_group(required=True)
    source.add_argument("--addrs", metavar="FILE", help="one hex address per line")
    source.add_argument("--keys", metavar="TSV", action="append",
                        help="TSV with a steam_addr column (repeatable)")
    run.add_argument("--topic", required=True, help="<B>-s<k>, e.g. B13a-s1")
    run.add_argument("--max-sites", type=int, default=8)
    run.add_argument("--ledger-dir", default=str(rl.DEFAULT_DIR))
    run.add_argument("--dump-root", default=str(sf.DUMP_ROOT))
    run.add_argument("--cache-dir", help="REA call cache dir (default: rea_session default)")
    run.add_argument("--lock-dir", help="REA session lock dir (default: rea_session default)")
    run.add_argument("--kdb", default=os.environ.get("BD_KDB") or str(DEFAULT_KDB))
    run.add_argument("--steam-db", default=os.environ.get("BD_BRANCHDB") or str(DEFAULT_STEAM_DB))
    run.add_argument("--no-live", action="store_true", help="static callsite rows only; no REA")
    run.add_argument("--cmd", metavar="SHLEX", help="REA MCP command (default: rea_session default)")
    run.add_argument("--no-preflight", action="store_true")
    return parser.parse_args(argv)


def main(argv=None, *, session_overrides=None):
    args = parse_args(argv)
    try:
        return _execute(args, session_overrides or {})
    except UsageError as exc:
        print("rea_facts: %s" % exc, file=sys.stderr)
        return 2
    except rl.LedgerError as exc:
        print("rea_facts: ledger write failed: %s" % exc, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
