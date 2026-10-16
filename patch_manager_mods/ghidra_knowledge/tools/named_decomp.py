#!/usr/bin/env python3
"""Named pseudo-C for KOTOR II functions through one persistent REA session.

Flow: annotate names (REA's ephemeral copy only, annotate_native_function), then batch-decompile
the targets, then rewrite FUN_/DAT_ placeholders left by failed annotations. Writes
<out>/<bb>/0x%08x.c and <out>/manifest.tsv. Never calls set_address_name(s).

Usage: named_decomp.py run (--addrs FILE | --class C) [--batch 20] [--out DIR]
       [--names-from steamdb|file:TSV] [--cmd SHLEX] [--no-preflight]
       [--steam-db DB] [--kdb DB] [--dump-root DIR] [--exe PATH]
"""

import argparse
import datetime as dt
import hashlib
import os
import re
import shlex
import sqlite3
import sys
import time
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import rea_session as rs  # noqa: E402
import static_facts as sf  # noqa: E402

MANIFEST_NAME = "manifest.tsv"
MANIFEST_HEADER = "addr\tname\tstatus\tbytes\twall_s"
DEFAULT_BATCH = 20


def _sha256_file(path):
    digest = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _as_addr(value):
    """int or hex text (0x prefix optional) -> int, None when unparseable."""
    if isinstance(value, int):
        return value
    try:
        return int(str(value).strip(), 16)
    except ValueError:
        return None


def read_addr_file(path):
    addrs = set()
    for raw in Path(path).read_text(encoding="utf-8").splitlines():
        text = raw.strip()
        if not text or text.startswith("#"):
            continue
        addrs.add(int(text.split()[0], 16))  # ValueError -> main() returns 2
    return addrs


def class_addrs(steam_db, cls):
    con = sqlite3.connect("file:%s?mode=ro" % steam_db, uri=True)
    try:
        rows = con.execute("SELECT address FROM functions WHERE class_name = ?", (cls,)).fetchall()
    finally:
        con.close()
    addrs = set()
    for (addr,) in rows:
        value = _as_addr(addr)
        if value is not None:
            addrs.add(value)
    return addrs


def load_names_map(source, steam_db, kdb):
    if source == "steamdb":
        return sf.load_names(steam_db=steam_db, kdb=kdb)
    if source.startswith("file:"):
        names = {}
        for raw in Path(source[len("file:"):]).read_text(encoding="utf-8").splitlines():
            parts = raw.split("\t")
            if len(parts) < 2 or not parts[1].strip():
                continue
            names[int(parts[0].strip(), 16)] = parts[1].strip()
        return names
    raise ValueError("--names-from must be steamdb or file:TSV, got %r" % source)


def make_dump(dump_root, names):
    try:
        return sf.Dump(dump_root, names)
    except Exception:  # missing dump dir: no callgraph, so no callee annotations
        return None


def callee_targets(dump, addr):
    if dump is None:
        return []
    try:
        edges = dump.callees(addr)
    except Exception:
        return []
    targets = set()
    for _site, target, _kind in edges:
        if target is not None:
            value = _as_addr(target)
            if value is not None:
                targets.add(value)
    return sorted(targets)


def postpass(text, renames):
    """Replace FUN_/DAT_ placeholders for renamed addresses. renames: {addr: name}. Returns (text, count)."""
    total = 0
    for addr in sorted(renames):
        name = renames[addr]
        for hexed in sorted({"%08x" % addr, "%08X" % addr}):
            for prefix in ("FUN", "DAT"):
                pattern = re.compile(r"\b%s_%s\b" % (prefix, hexed))
                text, count = pattern.subn(lambda _m, new=name: new, text)
                total += count
    return text, total


def _session_died(exc):
    return "connection lost" in str(exc)


def _label(names, addr):
    """Name for header/manifest: one line, no tabs (the .c filename never depends on it)."""
    return "_".join((names.get(addr) or "FUN_%08x" % addr).split())


def _atomic_write(path, content):
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_bytes(content)
    os.replace(tmp, path)


def c_path(out, addr):
    return Path(out) / ("%08x" % addr)[2:4] / ("0x%08x.c" % addr)


def read_manifest(path):
    rows = {}
    if not path.is_file():
        return rows
    for line in path.read_text(encoding="utf-8").splitlines()[1:]:
        cols = line.split("\t")
        if len(cols) != 5:
            continue
        addr = _as_addr(cols[0])
        if addr is not None:
            rows[addr] = tuple(cols[1:])  # name, status, bytes, wall_s
    return rows


def write_manifest(out, rows):
    out.mkdir(parents=True, exist_ok=True)
    lines = [MANIFEST_HEADER]
    for addr in sorted(rows):
        lines.append("\t".join(["0x%08x" % addr] + list(rows[addr])))
    _atomic_write(out / MANIFEST_NAME, ("\n".join(lines) + "\n").encode("utf-8"))


def summary(n, ok, err, skipped, annotated, failed):
    return ("named_decomp: targets %d, ok %d, error %d, skipped %d (resume), annotated %d, annotate_failed %d"
            % (n, ok, err, skipped, annotated, failed))


def run(args, session_overrides):
    out = Path(args.out)
    steam_db = args.steam_db or os.environ.get("BD_BRANCHDB") or str(sf.STEAM_DB_DEFAULT)
    kdb = args.kdb or str(sf.KDB_DEFAULT)
    dump_root = args.dump_root or str(sf.DUMP_ROOT)
    if args.batch < 1:
        print("named_decomp: --batch must be >= 1", file=sys.stderr)
        return 2

    names = load_names_map(args.names_from, steam_db, kdb)
    targets = sorted(read_addr_file(args.addrs) if args.addrs else class_addrs(steam_db, args.cls))
    if not targets:
        print("named_decomp: no targets selected", file=sys.stderr)
    prior = read_manifest(out / MANIFEST_NAME)
    skipped = {t for t in targets
               if prior.get(t, ("", ""))[1] == "ok" and c_path(out, t).is_file()}
    remaining = [t for t in targets if t not in skipped]
    if not remaining:
        print(summary(len(targets), 0, 0, len(skipped), 0, 0))
        return 0

    run_stamp = dt.datetime.now(dt.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    session_id = "session-" + run_stamp
    kwargs = {"read_only": False, "exe": args.exe}
    if args.cmd:
        kwargs["cmd"] = shlex.split(args.cmd)
    kwargs.update(session_overrides)
    exe = kwargs["exe"]

    try:
        sha_before = _sha256_file(exe)
    except OSError as exc:
        print("named_decomp: cannot hash exe: %s" % exc, file=sys.stderr)
        return 2
    if not args.no_preflight:
        try:
            rs.guard()
            rs.preflight(exe, rs.EXE_SHA256)
        except rs.ReaError as exc:
            print("named_decomp: REA unavailable: %s" % exc, file=sys.stderr)
            return 2

    dump = make_dump(dump_root, names)
    callees = {t: callee_targets(dump, t) for t in remaining}
    annotate_set = set()
    for t in remaining:
        if t in names:
            annotate_set.add(t)
        annotate_set.update(c for c in callees[t] if c in names)

    # Default open_timeout_s (900) is kept on purpose: the live cold start is ~5 min.
    session = rs.ReaSession(**kwargs)
    merged = dict(prior)
    rows_new = {}
    annotated_ok, failed = set(), set()
    code = 0
    try:
        try:
            session.start()
            session.open()
        except rs.ReaError as exc:
            print("named_decomp: REA start/open failed: %s" % exc, file=sys.stderr)
            return 1

        # Every annotate call happens before the first batch_decompile.
        for addr in sorted(annotate_set):
            try:
                session.call("annotate_native_function", {"procedure": "0x%08x" % addr, "name": names[addr]})
                annotated_ok.add(addr)
            except rs.ReaError as exc:
                if _session_died(exc):
                    print("named_decomp: REA session died while annotating: %s" % exc, file=sys.stderr)
                    return 1
                failed.add(addr)
                print("named_decomp: annotate 0x%08x failed: %s" % (addr, exc), file=sys.stderr)
        names_applied = {t: sum(1 for a in {t} | set(callees[t]) if a in annotated_ok) for t in remaining}
        renames = {a: names[a] for a in failed if a in names}
        all_renames = {a: names[a] for a in annotate_set}
        proc = getattr(session, "_proc", None)
        reconnected = False

        died = False
        for i in range(0, len(remaining), args.batch):
            chunk = remaining[i:i + args.batch]
            if died:
                for addr in chunk:
                    rows_new[addr] = (_label(names, addr), "error", "0", "0.00")
                continue
            t0 = time.monotonic()
            try:
                env = session.call("batch_decompile", {"addresses": ["0x%08x" % a for a in chunk]},
                                   timeout_s=max(session.call_timeout_s, 15 * len(chunk)))
            except rs.ReaError as exc:
                print("named_decomp: batch_decompile chunk failed: %s" % exc, file=sys.stderr)
                env = None
                code = 1  # re-run resumes the missing rows
                died = _session_died(exc)
            per = (time.monotonic() - t0) / len(chunk)
            if getattr(session, "_proc", None) is not proc:
                # The session was transparently restarted: its annotations are gone, so every name
                # from here on is restored by the post-pass instead.
                reconnected = True
                proc = getattr(session, "_proc", None)
            by_addr = {}
            ev = session_id
            if isinstance(env, dict):
                payload = env.get("normalized_result")
                items = payload.get("items") if isinstance(payload, dict) else None
                for item in items or []:
                    key = _as_addr(item.get("address")) if isinstance(item, dict) else None
                    if key is not None:
                        by_addr[key] = item
                ev = env.get("evidence_id") or env.get("id") or session_id
            for addr in chunk:
                item = by_addr.get(addr)
                text = item.get("pseudocode") if item and item.get("status") == "ok" else None
                label = _label(names, addr)
                if not isinstance(text, str):
                    rows_new[addr] = (label, "error", "0", "%.2f" % per)
                    continue
                body, replaced = postpass(text, all_renames if reconnected else renames)
                names_value = "postpass" if replaced else str(names_applied.get(addr, 0))
                header = "// NAMED 0x%08x %s evidence=%s names=%s exe=%s" % (
                    addr, label, ev, names_value, sha_before[:12])
                content = (header + "\n" + body).encode("utf-8")
                if not content.endswith(b"\n"):
                    content += b"\n"
                _atomic_write(c_path(out, addr), content)
                rows_new[addr] = (label, "ok", str(len(content)), "%.2f" % per)
            merged.update(rows_new)
            write_manifest(out, merged)  # progress survives a later crash; a re-run resumes
    finally:
        session.close()
        if rows_new:
            for addr in remaining:
                rows_new.setdefault(addr, (_label(names, addr), "error", "0", "0.00"))
            merged.update(rows_new)
            write_manifest(out, merged)

    sha_after = _sha256_file(exe)
    if sha_after != sha_before:
        print("named_decomp: exe sha changed during run (%s -> %s)" % (sha_before[:12], sha_after[:12]),
              file=sys.stderr)
        code = 3

    ok_n = sum(1 for row in rows_new.values() if row[1] == "ok")
    print(summary(len(targets), ok_n, len(rows_new) - ok_n, len(skipped), len(annotated_ok), len(failed)))
    return code


def parse_args(argv):
    parser = argparse.ArgumentParser(prog="named_decomp.py", description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    run_p = sub.add_parser("run", help="annotate names and batch-decompile targets into named .c files")
    src = run_p.add_mutually_exclusive_group(required=True)
    src.add_argument("--addrs", metavar="FILE", help="one hex address per line")
    src.add_argument("--class", dest="cls", metavar="C", help="all functions.address rows with class_name C")
    run_p.add_argument("--batch", type=int, default=DEFAULT_BATCH)
    run_p.add_argument("--out", default=str(rs.DUMP_ROOT / "named_decomp"))
    run_p.add_argument("--names-from", default="steamdb", help="steamdb (default) or file:TSV")
    run_p.add_argument("--cmd", default=None, help="REA MCP command line (shell-quoted)")
    run_p.add_argument("--no-preflight", action="store_true")
    run_p.add_argument("--steam-db", default=None)
    run_p.add_argument("--kdb", default=None)
    run_p.add_argument("--dump-root", default=None)
    run_p.add_argument("--exe", default=str(rs.EXE))
    return parser.parse_args(argv)


def main(argv=None, *, session_overrides=None):
    args = parse_args(argv)
    try:
        return run(args, dict(session_overrides or {}))
    except (OSError, ValueError, sqlite3.Error, rs.ReaError) as exc:
        print("named_decomp: %s" % exc, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
