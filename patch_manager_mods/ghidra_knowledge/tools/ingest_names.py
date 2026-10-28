#!/usr/bin/env python3
"""Backfill function names that mod code knows but the knowledge DB / REA ledger do not.

Stdlib only.  Never writes to the knowledge DB (opened read-only).  The only writes are
ledger rows (`ledger --apply`) and the conflicts file next to the extract TSV.

CLI:
  ingest_names.py gap [--source modcode|notes|all] [--json]
  ingest_names.py extract --source modcode --out FILE
  ingest_names.py ledger --in FILE --topic ingest-modcode [--apply] [--date YYYY-MM-DD]
Common flags: --kdb DB  --ledger-dir DIR  --functions TSV  --root DIR  --dump-root DIR

extract TSV columns: addr kind name form src fn_start offset status kdb_name
  kind   = global (addr >= 0x980000) | function (addr is a function start) | callsite
  form   = qualified (Class::Method) | bare (name taken verbatim, never a guessed class)
  status = new | agrees | conflicts (vs name_map rows with confidence != 'wrong' and the ledger)
ledger writes `<YYYYMMDD>_<topic>.tsv` (confidence probable) for status=new function/global rows;
dry run by default.  KDB conflicts are never written anywhere.
"""
from __future__ import annotations

import argparse
import bisect
import collections
import csv
import datetime
import glob
import json
import os
import re
import sqlite3
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import rea_ledger  # noqa: E402

REPO = Path(__file__).resolve().parents[3]
GK = REPO / "patch_manager_mods/ghidra_knowledge"
KDB_DEFAULT = GK / "kotor2_ghidra_knowledge.db"
LEDGER_DEFAULT = GK / "rea_findings"
FUNCS_DEFAULT = GK / "steam_functions.tsv"

GROUPS = {
    "research_notes": [
        ".claude/research/**/*.md",
        "patch_manager_mods/*research*/**/*.md",
        "patch_manager_mods/engine_research_findings/*.md",
        "patch_manager_mods/[0-9]*.md",
        ".claude/reviews/**/*.md",
    ],
    "mod_code": [
        "patch_manager_mods/*_src/**/*.toml",
        "patch_manager_mods/*_src/**/*.h",
        "patch_manager_mods/*_src/**/*.cpp",
        "patch_manager_mods/*_src/**/*.inc",
    ],
}
SOURCE_GROUPS = {"modcode": ["mod_code"], "notes": ["research_notes"],
                 "all": ["research_notes", "mod_code"]}

ADDR_RX = re.compile(r"\b0x00([4-9][0-9a-fA-F]{5})\b")
GLOBAL_MIN = 0x980000
GAP_SKIP = 0x980000  # gap skips addr > this

QNAME = r"(\w+(?:::~?\w+)+)"
HOOK_NAME_RX = re.compile(r"\b([A-Za-z_]\w*::~?[A-Za-z_]\w*)\b")
HOOK_ADDR_RX = re.compile(r"^\s*address\s*=\s*(0x00[4-9][0-9a-fA-F]{5})\b")
CONSTEXPR_LINE_RX = re.compile(r"^\s*(?:(?:static|inline)\s+)*constexpr\b")
CONSTEXPR_ITEM_RX = re.compile(r"\b([A-Za-z_]\w*)\s*=\s*(0x00[4-9][0-9a-fA-F]{5})\b")
NAME_THEN_ADDR_RX = re.compile(QNAME + r"\W{1,6}(?:at|@|=)?\s*(0x00[4-9][0-9a-fA-F]{5})\b")
ADDR_THEN_NAME_RX = re.compile(r"\b(0x00[4-9][0-9a-fA-F]{5})\W{1,6}" + QNAME)

EXTRACT_COLS = ("addr", "kind", "name", "form", "src", "fn_start", "offset", "status", "kdb_name")
EVIDENCE_MAX = 200


# ---- loading -----------------------------------------------------------------
def _hex(s):
    try:
        return int(str(s).strip(), 16)
    except (ValueError, TypeError):
        return None


def load_kdb(path):
    """{addr int: name} of name_map rows with confidence != 'wrong' (read-only open)."""
    uri = "file:" + str(path) + "?mode=ro"
    con = sqlite3.connect(uri, uri=True)
    try:
        out = {}
        for addr, name in con.execute("select steam_addr, name from name_map where confidence != 'wrong'"):
            a = _hex(addr)
            if a is not None:
                out[a] = name
        return out
    finally:
        con.close()


def load_ledger(ledger_dir):
    """{addr int: [names in file order]} over every *.tsv in the ledger dir with an addr column."""
    if not os.path.isdir(ledger_dir):
        raise FileNotFoundError(f"no such ledger dir: {ledger_dir}")
    out = collections.defaultdict(list)
    for f in sorted(glob.glob(os.path.join(str(ledger_dir), "*.tsv"))):
        with open(f, encoding="utf-8", errors="replace", newline="") as fh:
            for row in csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE):
                a = _hex(row.get("addr"))
                if a is not None:
                    out[a].append((row.get("name") or "").strip())
    return dict(out)


def load_starts(functions_tsv):
    starts = set()
    with open(functions_tsv, encoding="utf-8", errors="replace", newline="") as fh:
        for row in csv.reader(fh, delimiter="\t", quoting=csv.QUOTE_NONE):
            if row:
                a = _hex(row[0])
                if a is not None:
                    starts.add(a)
    return sorted(starts)


def fn_of(starts, a):
    i = bisect.bisect_right(starts, a) - 1
    return starts[i] if i >= 0 else None


def group_files(root, group):
    root = str(root).rstrip("/") + "/"
    files = set()
    for p in GROUPS[group]:
        files.update(glob.glob(root + p, recursive=True))
    return sorted(f for f in files if "dump_excerpts" not in f and os.path.isfile(f))


def _read(path):
    try:
        with open(path, encoding="utf-8", errors="ignore") as fh:
            return fh.read()
    except OSError:
        return None


# ---- gap ---------------------------------------------------------------------
def _src_label(root, f):
    rel = os.path.relpath(f, root).split("/")
    if rel[0] == "patch_manager_mods":
        return rel[1] if len(rel) > 1 else rel[0]
    return rel[2] if len(rel) > 2 else rel[0]


def gap(source="all", kdb_path=KDB_DEFAULT, ledger_dir=LEDGER_DEFAULT, functions=FUNCS_DEFAULT,
        root=REPO, top_n=12):
    """{group: {files, cited, unknown, top: [{fn, cites, in}]}} for the chosen source."""
    known = set(load_kdb(kdb_path)) | set(load_ledger(ledger_dir))
    starts = load_starts(functions)
    result = {}
    for g in SOURCE_GROUPS[source]:
        files = group_files(root, g)
        cites = collections.Counter()
        where = collections.defaultdict(set)
        for f in files:
            text = _read(f)
            if text is None:
                continue
            for m in ADDR_RX.finditer(text):
                a = int(m.group(1), 16)
                if a > GAP_SKIP:
                    continue
                fn = fn_of(starts, a)
                if fn is None:
                    continue
                cites[fn] += 1
                where[fn].add(_src_label(str(root), f))
        unknown = [fn for fn in cites if fn not in known]
        top = sorted(unknown, key=lambda fn: (-cites[fn], fn))[:top_n]
        result[g] = {
            "files": len(files), "cited": len(cites), "unknown": len(unknown),
            "top": [{"fn": "0x%08x" % fn, "cites": cites[fn], "in": sorted(where[fn])[:4]} for fn in top],
        }
    return result


# ---- extract -----------------------------------------------------------------
def comment_text(line, is_toml):
    """The comment portion of a source line, or None."""
    if is_toml:
        i = line.find("#")
        return line[i + 1:] if i >= 0 else None
    i = line.find("//")
    if i >= 0:
        return line[i + 2:]
    i = line.find("/*")
    if i >= 0:
        return line[i + 2:]
    if line.lstrip().startswith("*"):
        return line.lstrip()[1:]
    return None


def scan_file(path, relpath):
    """Yield (addr int, name, 'relpath:line') candidates from one mod-code file."""
    text = _read(path)
    if text is None:
        return
    lines = text.splitlines()
    is_toml = path.endswith(".toml")
    is_hooks = path.endswith(".hooks.toml")
    for i, line in enumerate(lines):
        src = "%s:%d" % (relpath, i + 1)
        if is_hooks:
            m = HOOK_ADDR_RX.match(line)
            if m:
                for j in range(i - 1, max(i - 7, -1), -1):
                    prev = lines[j]
                    if HOOK_ADDR_RX.match(prev):
                        break
                    if not prev.lstrip().startswith("#"):
                        continue
                    nm = HOOK_NAME_RX.search(prev)
                    if nm:
                        yield int(m.group(1), 16), nm.group(1), src
                        break
        elif CONSTEXPR_LINE_RX.match(line):
            code = line.split("//", 1)[0]
            for m in CONSTEXPR_ITEM_RX.finditer(code):
                yield int(m.group(2), 16), m.group(1), src
        com = comment_text(line, is_toml)
        if com:
            for m in NAME_THEN_ADDR_RX.finditer(com):
                yield int(m.group(2), 16), m.group(1), src
            for m in ADDR_THEN_NAME_RX.finditer(com):
                yield int(m.group(1), 16), m.group(2), src


def classify(starts, a):
    """(kind, fn_start text, offset text)."""
    if a >= GLOBAL_MIN:
        return "global", "", ""
    fn = fn_of(starts, a)
    if fn == a:
        return "function", "0x%08x" % fn, "0x0"
    if fn is None:
        return "callsite", "", ""
    return "callsite", "0x%08x" % fn, "0x%x" % (a - fn)


def status_of(a, name, kdb, ledger):
    """(status, kdb_name) of one candidate against name_map and the ledger."""
    k = kdb.get(a)
    if k is not None:
        if name == k or ("::" not in name and k.endswith("::" + name)):
            return "agrees", k
        return "conflicts", k
    names = ledger.get(a)
    if names:
        if name in names:
            return "agrees", "rea:" + name
        return "conflicts", "rea:" + names[0]
    return "new", ""


def extract(kdb_path=KDB_DEFAULT, ledger_dir=LEDGER_DEFAULT, functions=FUNCS_DEFAULT, root=REPO):
    """List of row dicts (EXTRACT_COLS) from mod code, deduped and sorted by addr then src."""
    kdb = load_kdb(kdb_path)
    ledger = load_ledger(ledger_dir)
    starts = load_starts(functions)
    seen = {}
    for f in group_files(root, "mod_code"):
        rel = os.path.relpath(f, str(root))
        for a, name, src in scan_file(f, rel):
            key = (a, name, src)
            if key in seen:
                continue
            kind, fn_start, offset = classify(starts, a)
            status, kdb_name = status_of(a, name, kdb, ledger)
            seen[key] = {
                "addr": "0x%08x" % a, "kind": kind, "name": name,
                "form": "qualified" if "::" in name else "bare", "src": src,
                "fn_start": fn_start, "offset": offset, "status": status, "kdb_name": kdb_name,
            }
    return [seen[k] for k in sorted(seen, key=lambda k: (k[0], k[2], k[1]))]


def write_tsv(path, rows, cols):
    with open(path, "w", encoding="utf-8", newline="") as fh:
        fh.write("\t".join(cols) + "\n")
        for r in rows:
            fh.write("\t".join(str(r[c]) for c in cols) + "\n")


def read_tsv(path):
    with open(path, encoding="utf-8", newline="") as fh:
        return list(csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE))


# ---- ledger ------------------------------------------------------------------
def _src_line_text(root, src):
    path, _, line = src.rpartition(":")
    text = _read(os.path.join(str(root), path))
    if text is None or not line.isdigit():
        return ""
    lines = text.splitlines()
    n = int(line)
    return lines[n - 1].strip() if 1 <= n <= len(lines) else ""


def _clean(text):
    return re.sub(r"[\t\r\n]+", " ", text).strip()


def build_ledger_rows(rows, dump, root, topic, date):
    """(ledger rows, conflict rows, skipped list) from extract rows (status=new, function|global)."""
    by_addr = collections.OrderedDict()
    for r in rows:
        if r["status"] == "new" and r["kind"] in ("function", "global"):
            by_addr.setdefault(r["addr"], []).append(r)
    out, conflicts, skipped = [], [], []
    for addr, grp in by_addr.items():
        counts = collections.Counter(r["name"] for r in grp)
        best = max(counts.values())
        first = next(r for r in grp if counts[r["name"]] == best)
        chosen = first["name"]
        chosen_src = first["src"]
        for r in grp:
            if r["name"] != chosen:
                conflicts.append({"addr": addr, "chosen_name": chosen, "other_name": r["name"], "src": r["src"]})
        a = int(addr, 16)
        asm = dump.asm(a)
        insn = _clean(asm[0]) if asm else ""
        if insn:
            quote = insn
        elif first["kind"] == "global":
            quote = _clean(_src_line_text(root, chosen_src))
        else:
            quote = ""
        if not quote:
            skipped.append({"addr": addr, "name": chosen, "reason": "no asm" if first["kind"] == "function"
                            else "no source text"})
            continue
        ev = ("0x%08x: %s | src %s" % (a, quote, chosen_src))[:EVIDENCE_MAX].rstrip()
        out.append({"addr": addr, "kind": first["kind"], "name": chosen, "confidence": "probable",
                    "evidence": ev, "topic": topic, "date": date})
    return out, conflicts, skipped


def run_ledger(args, dump, out=sys.stdout, err=sys.stderr):
    try:
        date = datetime.date.fromisoformat(args.date) if args.date else datetime.date.today()
    except ValueError:
        print(f"ingest_names: bad --date {args.date!r}, want YYYY-MM-DD", file=err)
        return 2
    in_path = Path(args.in_path)
    rows = read_tsv(in_path)
    target = "%s_%s.tsv" % (date.strftime("%Y%m%d"), args.topic)
    target_path = os.path.join(args.ledger_dir, target)
    if args.apply and os.path.exists(target_path):
        print(f"ingest_names: {target_path} already exists; refusing --apply to avoid double appends "
              "(pick another --topic/--date or remove the file)", file=err)
        return 2
    ledger_rows, conflicts, skipped = build_ledger_rows(rows, dump, args.root, args.topic, date.isoformat())
    verb = "written" if args.apply else "would-write"
    print(f"{'apply' if args.apply else 'dry-run'}: {verb}={len(ledger_rows)} skipped={len(skipped)} "
          f"conflicts={len(conflicts)} target={target}", file=out)
    for s in skipped[:10]:
        print(f"  skipped {s['addr']} {s['name']} ({s['reason']})", file=out)
    if args.apply:
        if ledger_rows:
            rea_ledger.append_rows(args.ledger_dir, target, ledger_rows)
        write_tsv(in_path.with_name("conflicts.tsv"), conflicts, ("addr", "chosen_name", "other_name", "src"))
        print(f"conflicts file: {in_path.with_name('conflicts.tsv')}", file=out)
    return 0


# ---- CLI ---------------------------------------------------------------------
def build_parser():
    p = argparse.ArgumentParser(prog="ingest_names.py", description=__doc__.split("\n")[0])
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--kdb", default=str(KDB_DEFAULT))
    common.add_argument("--ledger-dir", default=str(LEDGER_DEFAULT))
    common.add_argument("--functions", default=str(FUNCS_DEFAULT))
    common.add_argument("--root", default=str(REPO))
    common.add_argument("--dump-root", default=None)
    sub = p.add_subparsers(dest="cmd", required=True)
    g = sub.add_parser("gap", parents=[common], help="count unnamed/unledgered cited functions")
    g.add_argument("--source", choices=sorted(SOURCE_GROUPS), default="all")
    g.add_argument("--json", action="store_true")
    e = sub.add_parser("extract", parents=[common], help="name/address pairs from mod code")
    e.add_argument("--source", choices=["modcode"], default="modcode")
    e.add_argument("--out", required=True)
    le = sub.add_parser("ledger", parents=[common], help="turn extract rows into ledger rows")
    le.add_argument("--in", dest="in_path", required=True)
    le.add_argument("--topic", required=True)
    le.add_argument("--apply", action="store_true")
    le.add_argument("--date", default=None)
    return p


def _make_dump(dump_root):
    from static_facts import Dump, DUMP_ROOT
    return Dump(root=dump_root or DUMP_ROOT)


def main(argv=None, dump=None, out=None, err=None):
    out = out or sys.stdout
    err = err or sys.stderr
    try:
        args = build_parser().parse_args(argv)
    except SystemExit as exc:
        return 0 if exc.code in (None, 0) else 2
    try:
        if args.cmd == "gap":
            res = gap(args.source, args.kdb, args.ledger_dir, args.functions, args.root)
            if args.json:
                print(json.dumps(res, indent=2), file=out)
            else:
                for g, d in res.items():
                    print(f"{g}: files={d['files']} functions cited={d['cited']} "
                          f"unnamed/unledgered={d['unknown']}", file=out)
                    for t in d["top"]:
                        print(f"  {t['fn']} cites={t['cites']} in={t['in']}", file=out)
            return 0
        if args.cmd == "extract":
            rows = extract(args.kdb, args.ledger_dir, args.functions, args.root)
            write_tsv(args.out, rows, EXTRACT_COLS)
            c = collections.Counter(r["status"] for r in rows)
            print(f"extract: rows={len(rows)} new={c['new']} agrees={c['agrees']} "
                  f"conflicts={c['conflicts']} -> {args.out}", file=out)
            return 0
        if args.cmd == "ledger":
            return run_ledger(args, dump if dump is not None else _make_dump(args.dump_root), out, err)
    except (OSError, sqlite3.Error, rea_ledger.LedgerError) as exc:
        print(f"ingest_names: {exc}", file=err)
        return 2
    return 2


if __name__ == "__main__":
    sys.exit(main())
