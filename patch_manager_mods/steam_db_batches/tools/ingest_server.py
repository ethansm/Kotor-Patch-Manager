#!/usr/bin/env python3
"""Load read-only Ghidra-server exports into the knowledge DB (DESIGN.md section 3).

Inputs, per program directory SERVER/<program>/:
  <name>_functions.tsv  addr size retn name namespace callconv prototype source thunk
  <name>_symbols.tsv    addr name namespace symbol_type source
  types.tsv (optional)  category type_name kind size ordinal field_offset field_name field_type
(<name> is Ghidra's program name and may differ from the folder name, so we glob.)

Usage:
  ingest_server.py [--programs P1,P2 | --all] [--apply] [--report OUT.md]

Without --apply every DB is opened read-only and only counts are printed.
With --apply the three server_* tables are created if missing and upserted, one
transaction per program. A second --apply changes 0 rows.

Environment (same names as tools/bdlib.py): BD_SERVER, BD_KDB, BD_BRANCHDB, BD_GOGDB.
"""
from __future__ import annotations

import argparse
import csv
import datetime as dt
import os
import re
import sqlite3
import sys
from dataclasses import dataclass, field
from pathlib import Path

STEAM_PROGRAM = "k2_win_steam_aspyr_swkotor2.exe"
GOG_PROGRAM = "k2_win_gog_aspyr_swkotor2.exe"

DDL = """
CREATE TABLE IF NOT EXISTS server_functions(
  program TEXT NOT NULL, addr TEXT NOT NULL,          -- '0x%08x'
  name TEXT, namespace TEXT, callconv TEXT, prototype TEXT,
  source TEXT,                                        -- USER_DEFINED / IMPORTED / ANALYSIS / DEFAULT
  trust TEXT,                                         -- 'trusted' (USER_DEFINED, IMPORTED) / 'auto' (ANALYSIS) / 'none' (DEFAULT)
  size INTEGER, retn INTEGER, thunk INTEGER,
  export_date TEXT, PRIMARY KEY(program, addr));
CREATE TABLE IF NOT EXISTS server_symbols(            -- non-function symbols, USER_DEFINED and IMPORTED only
  program TEXT NOT NULL, addr TEXT NOT NULL, name TEXT NOT NULL, namespace TEXT, symbol_type TEXT, source TEXT,
  export_date TEXT, PRIMARY KEY(program, addr, name, namespace));
CREATE TABLE IF NOT EXISTS server_types(
  program TEXT NOT NULL, category TEXT NOT NULL, type_name TEXT NOT NULL, kind TEXT, size INTEGER,
  ordinal INTEGER NOT NULL, field_offset INTEGER, field_name TEXT, field_type TEXT,
  export_date TEXT, PRIMARY KEY(program, category, type_name, ordinal));
"""

# table -> (column order, primary-key columns)
TABLES = {
    "server_functions": (
        ["program", "addr", "name", "namespace", "callconv", "prototype", "source", "trust",
         "size", "retn", "thunk", "export_date"],
        ["program", "addr"]),
    "server_symbols": (
        ["program", "addr", "name", "namespace", "symbol_type", "source", "export_date"],
        ["program", "addr", "name", "namespace"]),
    "server_types": (
        ["program", "category", "type_name", "kind", "size", "ordinal", "field_offset",
         "field_name", "field_type", "export_date"],
        ["program", "category", "type_name", "ordinal"]),
}

TRUST_BY_SOURCE = {"USER_DEFINED": "trusted", "IMPORTED": "trusted", "ANALYSIS": "auto", "DEFAULT": "none"}
SYMBOL_SOURCES = ("USER_DEFINED", "IMPORTED")


# --------------------------------------------------------------------------- paths

def env_paths() -> dict[str, Path]:
    """Resolved at call time so tests can patch the environment."""
    here = Path(__file__).resolve()
    kdb_default = here.parents[2] / "ghidra_knowledge" / "kotor2_ghidra_knowledge.db"
    games = Path.home() / "Games" / "KOTOR2_Mod_Workspace"
    adb = games / "tools" / "kpm-pr-steam-db" / "AddressDatabases"
    return {
        "server": Path(os.environ.get("BD_SERVER", games / "ghidra_dumps" / "upstream_server")),
        "kdb": Path(os.environ.get("BD_KDB", kdb_default)),
        "branchdb": Path(os.environ.get("BD_BRANCHDB", adb / "kotor2_steam_aspyr.db")),
        "gogdb": Path(os.environ.get("BD_GOGDB", adb / "kotor2_gog_aspyr.db")),
    }


def open_ro(path: Path) -> sqlite3.Connection:
    return sqlite3.connect(f"file:{path}?mode=ro", uri=True)


# --------------------------------------------------------------------------- parsing helpers

def fmt_addr(text: str) -> str:
    return "0x%08x" % int(text.strip(), 16)


def to_int(text: str | None) -> int | None:
    if text is None or text.strip() == "":
        return None
    try:
        return int(text.strip(), 0)
    except ValueError:
        return None


def read_tsv(path: Path):
    """Yield dict rows; quoting is off because prototypes contain quote characters."""
    with open(path, encoding="utf-8", errors="replace", newline="") as fh:
        reader = csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE)
        for row in reader:
            yield {k: (v if v is not None else "") for k, v in row.items() if k is not None}


def mtime_date(path: Path) -> str:
    return dt.date.fromtimestamp(path.stat().st_mtime).isoformat()


@dataclass
class ProgramData:
    program: str
    folder: Path
    rows: dict[str, list[tuple]] = field(default_factory=dict)  # table -> deduped rows
    raw_counts: dict[str, int] = field(default_factory=dict)    # TSV data lines per file kind
    dup_counts: dict[str, int] = field(default_factory=dict)
    func_by_source: dict[str, int] = field(default_factory=dict)
    sym_by_source: dict[str, int] = field(default_factory=dict)
    has_types: bool = False

    def trusted_functions(self):
        """(addr, full_name, source) for trusted named functions."""
        for r in self.rows["server_functions"]:
            if r[7] == "trusted":
                yield r[1], full_name(r[3], r[2]), r[6]

    def trusted_symbols(self):
        for r in self.rows["server_symbols"]:
            yield r[1], full_name(r[3], r[2]), r[5]


def full_name(namespace: str | None, name: str | None) -> str:
    return f"{namespace}::{name}" if namespace else (name or "")


def _dedupe(rows: list[tuple], key_idx: list[int]) -> tuple[list[tuple], int]:
    seen: dict[tuple, tuple] = {}
    for r in rows:
        seen[tuple(r[i] for i in key_idx)] = r  # last occurrence wins
    return list(seen.values()), len(rows) - len(seen)


def load_program(folder: Path, functions: str = "trusted") -> ProgramData:
    """functions="trusted" keeps only USER_DEFINED/IMPORTED rows in server_functions (the KDB is git-tracked;
    the full exports stay in BD_SERVER and the pipeline reads them there); "all" keeps every row."""
    fn_files = sorted(folder.glob("*_functions.tsv"))
    sym_files = sorted(folder.glob("*_symbols.tsv"))
    if not fn_files:
        raise FileNotFoundError(f"{folder}: no *_functions.tsv")
    data = ProgramData(program=folder.name, folder=folder)
    prog = folder.name

    fn_path = fn_files[0]
    date = mtime_date(fn_path)
    funcs, n_raw = [], 0
    for r in read_tsv(fn_path):
        n_raw += 1
        src = r["source"].strip()
        data.func_by_source[src] = data.func_by_source.get(src, 0) + 1
        funcs.append((prog, fmt_addr(r["addr"]), r["name"], r["namespace"], r["callconv"], r["prototype"],
                      src, TRUST_BY_SOURCE.get(src, "none"), to_int(r["size"]), to_int(r["retn"]),
                      1 if r["thunk"].strip().lower() == "true" else 0, date))
    data.raw_counts["functions"] = n_raw
    if functions == "trusted":
        funcs = [f for f in funcs if f[7] == "trusted"]
    data.rows["server_functions"], data.dup_counts["functions"] = _dedupe(funcs, [0, 1])

    syms, n_raw = [], 0
    if sym_files:
        date = mtime_date(sym_files[0])
        for r in read_tsv(sym_files[0]):
            n_raw += 1
            src = r["source"].strip()
            data.sym_by_source[src] = data.sym_by_source.get(src, 0) + 1
            if src not in SYMBOL_SOURCES or r["symbol_type"].strip() == "Function":
                continue
            syms.append((prog, fmt_addr(r["addr"]), r["name"], r["namespace"], r["symbol_type"], src, date))
    data.raw_counts["symbols"] = n_raw
    data.rows["server_symbols"], data.dup_counts["symbols"] = _dedupe(syms, [0, 1, 2, 3])

    types_path = folder / "types.tsv"
    types, n_raw, ordinals = [], 0, {}
    data.has_types = types_path.exists()
    if data.has_types:
        date = mtime_date(types_path)
        for r in read_tsv(types_path):
            n_raw += 1
            tkey = (r["category"], r["type_name"])
            ordinal = to_int(r.get("ordinal"))
            if ordinal is None:  # no ordinal in the export: number the rows within the type
                ordinal = ordinals.get(tkey, 0)
            ordinals[tkey] = ordinal + 1
            types.append((prog, r["category"], r["type_name"], r["kind"], to_int(r["size"]), ordinal,
                          to_int(r["field_offset"]), r["field_name"], r["field_type"], date))
    data.raw_counts["types"] = n_raw
    data.rows["server_types"], data.dup_counts["types"] = _dedupe(types, [0, 1, 2, 5])
    return data


def discover_programs(server: Path) -> list[str]:
    if not server.is_dir():
        raise FileNotFoundError(f"BD_SERVER {server} is not a directory")
    return sorted(p.name for p in server.iterdir() if p.is_dir() and any(p.glob("*_functions.tsv")))


# --------------------------------------------------------------------------- plan / apply

@dataclass
class TablePlan:
    new: int = 0
    changed: int = 0
    unchanged: int = 0
    stale: int = 0  # in DB, not in the TSV (never deleted, informational)

    @property
    def changes(self) -> int:
        return self.new + self.changed


def table_exists(conn: sqlite3.Connection, table: str) -> bool:
    return conn.execute("SELECT 1 FROM sqlite_master WHERE type='table' AND name=?", (table,)).fetchone() is not None


def plan_table(conn: sqlite3.Connection, table: str, program: str, rows: list[tuple]) -> TablePlan:
    cols, keys = TABLES[table]
    kidx = [cols.index(k) for k in keys]
    existing: dict[tuple, tuple] = {}
    if table_exists(conn, table):
        for r in conn.execute(f"SELECT {','.join(cols)} FROM {table} WHERE program=?", (program,)):
            existing[tuple(r[i] for i in kidx)] = tuple(r)
    plan = TablePlan()
    seen = set()
    for r in rows:
        k = tuple(r[i] for i in kidx)
        seen.add(k)
        old = existing.get(k)
        if old is None:
            plan.new += 1
        elif old != r:
            plan.changed += 1
        else:
            plan.unchanged += 1
    plan.stale = len(set(existing) - seen)
    return plan


def plan_program(conn: sqlite3.Connection, data: ProgramData) -> dict[str, TablePlan]:
    return {t: plan_table(conn, t, data.program, data.rows[t]) for t in TABLES
            if t != "server_types" or data.has_types}


def upsert_sql(table: str) -> str:
    cols, keys = TABLES[table]
    rest = [c for c in cols if c not in keys]
    changed = " OR ".join(f"{table}.{c} IS NOT excluded.{c}" for c in rest)
    sets = ", ".join(f"{c}=excluded.{c}" for c in rest)
    return (f"INSERT INTO {table}({','.join(cols)}) VALUES({','.join('?' * len(cols))}) "
            f"ON CONFLICT({','.join(keys)}) DO UPDATE SET {sets} WHERE {changed}")


def apply_program(conn: sqlite3.Connection, data: ProgramData) -> dict[str, int]:
    """One transaction for the program; returns rows actually inserted/updated per table."""
    done: dict[str, int] = {}
    conn.execute("BEGIN")
    try:
        for table in TABLES:
            if table == "server_types" and not data.has_types:
                continue
            before = conn.total_changes
            conn.executemany(upsert_sql(table), data.rows[table])
            done[table] = conn.total_changes - before
        conn.execute("COMMIT")
    except BaseException:
        conn.execute("ROLLBACK")
        raise
    return done


# --------------------------------------------------------------------------- name normalisation

_DEFAULT_NAME = re.compile(r"^FUN_[0-9A-Fa-f]{6,8}$")
_DOUBLE_US = re.compile(r"^([A-Za-z][A-Za-z0-9_]*?)__([A-Za-z~][\w~]*)$")


def _split_last_scope(name: str) -> tuple[str, str]:
    """Split at the last '::' that is not inside <> or ()."""
    depth, cut, i = 0, -1, 0
    while i < len(name):
        ch = name[i]
        if ch in "<(":
            depth += 1
        elif ch in ">)":
            depth = max(0, depth - 1)
        elif ch == ":" and depth == 0 and name.startswith("::", i):
            cut = i
            i += 1
        i += 1
    return ("", name) if cut < 0 else (name[:cut], name[cut + 2:])


def normalize(name: str | None) -> str | None:
    """Canonical 'Class::Func' (or bare 'Func'); None for Ghidra default names.

    - strips thunk_ prefixes and a FUN_ prefix (a bare FUN_<addr> is unnamed -> None)
    - Class__Func (Aspyr/Ghidra style) -> Class::Func
    - Class::Class -> Class::Constructor, Class::~Class -> Class::Destructor (branch DB style)
    """
    if not name:
        return None
    name = name.strip()
    while name.startswith("thunk_"):
        name = name[len("thunk_"):]
    if _DEFAULT_NAME.match(name):
        return None
    if name.startswith("FUN_"):
        name = name[4:]
    cls, fn = _split_last_scope(name)
    if not cls:
        m = _DOUBLE_US.match(name)
        if m:
            cls, fn = m.group(1), m.group(2)
    if cls:
        base = cls.split("<", 1)[0].rsplit("::", 1)[-1]
        if fn == base or fn == cls.rsplit("::", 1)[-1]:
            fn = "Constructor"
        elif fn in ("~" + base, "~" + cls.rsplit("::", 1)[-1]):
            fn = "Destructor"
        return f"{cls}::{fn}"
    return fn


# --------------------------------------------------------------------------- report

def classify(server: str, others: list[str]) -> str:
    if not others:
        return "server-only"
    return "agree" if server in others else "conflict"


def load_name_map(kdb: Path) -> dict[str, tuple[str, str]] | None:
    conn = open_ro(kdb)
    try:
        if not table_exists(conn, "name_map"):
            return None
        out = {}
        for addr, name, conf in conn.execute("SELECT steam_addr, name, confidence FROM name_map"):
            try:
                out[fmt_addr(addr)] = (name, conf)
            except (ValueError, AttributeError):
                continue
        return out
    finally:
        conn.close()


def load_functions_db(path: Path) -> dict[str, list[str]] | None:
    """address -> list of 'Class::Func' (raw, un-normalised)."""
    if not path.exists():
        return None
    conn = open_ro(path)
    try:
        out: dict[str, list[str]] = {}
        for cls, fn, addr in conn.execute("SELECT class_name, function_name, address FROM functions"):
            out.setdefault("0x%08x" % addr, []).append(full_name(cls, fn))
        return out
    finally:
        conn.close()


@dataclass
class Compare:
    counts: dict[str, int] = field(default_factory=lambda: {"agree": 0, "conflict": 0, "server-only": 0})
    conflicts: list[tuple] = field(default_factory=list)      # (addr, server, other names, extra)
    server_only: list[tuple] = field(default_factory=list)


def compare_names(items, lookup) -> tuple[Compare, int]:
    """items: (addr, full_name, source); lookup(addr) -> (list[raw names], extra) ; returns (Compare, unnamed)."""
    cmp, unnamed = Compare(), 0
    for addr, name, source in sorted(items):
        norm = normalize(name)
        if norm is None:
            unnamed += 1
            continue
        raws, extra = lookup(addr)
        verdict = classify(norm, [n for n in (normalize(x) for x in raws) if n])
        cmp.counts[verdict] += 1
        if verdict == "conflict":
            cmp.conflicts.append((addr, name, raws, extra, source))
        elif verdict == "server-only":
            cmp.server_only.append((addr, name, source))
    return cmp, unnamed


def md_escape(text: object) -> str:
    return str(text).replace("|", "\\|").replace("\n", " ")


def _cmp_section(lines: list[str], title: str, cmp: Compare, unnamed: int, so_cap: int = 200) -> None:
    c = cmp.counts
    lines += [f"### {title}", "",
              f"agree {c['agree']} / conflict {c['conflict']} / server-only {c['server-only']} "
              f"(skipped {unnamed} default-named)", ""]
    if cmp.conflicts:
        lines += ["| addr | server | other | detail | server source |", "|---|---|---|---|---|"]
        for addr, name, raws, extra, source in cmp.conflicts:
            lines.append(f"| {addr} | {md_escape(name)} | {md_escape(', '.join(raws))} | {md_escape(extra)} | {source} |")
        lines.append("")
    if cmp.server_only:
        lines += [f"<details><summary>server-only (first {min(so_cap, len(cmp.server_only))} of "
                  f"{len(cmp.server_only)})</summary>", "", "| addr | server | source |", "|---|---|---|"]
        for addr, name, source in cmp.server_only[:so_cap]:
            lines.append(f"| {addr} | {md_escape(name)} | {source} |")
        lines += ["", "</details>", ""]


def build_report(datas: dict[str, ProgramData], paths: dict[str, Path]) -> tuple[str, dict]:
    lines = ["# Server ingest report", "", f"Generated {dt.datetime.now():%Y-%m-%d %H:%M}.", ""]
    summary: dict = {}

    steam = datas.get(STEAM_PROGRAM)
    lines += ["## (a) Steam: trusted server names vs name_map and branch DB", ""]
    if steam is None:
        lines += [f"{STEAM_PROGRAM} not selected.", ""]
    else:
        nm = load_name_map(paths["kdb"])
        branch = load_functions_db(paths["branchdb"])
        if nm is None:
            lines += ["name_map table not found in the KDB: skipped.", ""]
        if branch is None:
            lines += [f"branch DB {paths['branchdb']} not found: skipped.", ""]
        sets = {"functions": list(steam.trusted_functions()), "symbols": list(steam.trusted_symbols())}
        for kind, items in sets.items():
            if nm is not None:
                cmp, unnamed = compare_names(items, lambda a: ([nm[a][0]], f"conf={nm[a][1]}") if a in nm else ([], ""))
                _cmp_section(lines, f"{kind} vs name_map ({len(items)} trusted)", cmp, unnamed)
                summary[f"steam_{kind}_name_map"] = dict(cmp.counts)
            if branch is not None:
                cmp, unnamed = compare_names(items, lambda a: (branch.get(a, []), ""))
                _cmp_section(lines, f"{kind} vs branch DB ({len(items)} trusted)", cmp, unnamed)
                summary[f"steam_{kind}_branch"] = dict(cmp.counts)

    gog = datas.get(GOG_PROGRAM)
    lines += ["## (b) GOG: trusted server names vs GOG address DB", ""]
    if gog is None:
        lines += [f"{GOG_PROGRAM} not selected.", ""]
    else:
        db = load_functions_db(paths["gogdb"])
        if db is None:
            lines += [f"GOG DB {paths['gogdb']} not found: skipped.", ""]
        else:
            cmp, unnamed = compare_names(list(gog.trusted_functions()), lambda a: (db.get(a, []), ""))
            trusted_addrs = {a for a, n, _ in gog.trusted_functions() if normalize(n)}
            server_fn_addrs = {r[1] for r in gog.rows["server_functions"]}
            db_only = [a for a in db if a not in trusted_addrs]
            db_only_fn = sum(1 for a in db_only if a in server_fn_addrs)
            c = cmp.counts
            lines += [f"agree {c['agree']} / disagree {c['conflict']} / server-only {c['server-only']} / "
                      f"db-only {len(db_only)} (of which {db_only_fn} are server functions with an untrusted "
                      f"name, {len(db_only) - db_only_fn} have no server function)", ""]
            summary["gog"] = {"agree": c["agree"], "disagree": c["conflict"],
                              "server-only": c["server-only"], "db-only": len(db_only)}
            lines += ["### GOG label suspects (disagree list)", ""]
            if cmp.conflicts:
                lines += ["| addr | server name | db name(s) | server source |", "|---|---|---|---|"]
                for addr, name, raws, _extra, source in cmp.conflicts:
                    lines.append(f"| {addr} | {md_escape(name)} | {md_escape(', '.join(raws))} | {source} |")
            else:
                lines.append("none")
            lines.append("")

    lines += ["## (c) Per-program counts", ""]
    lines += ["| program | functions | by source | symbols (all) | by source | trusted fn | trusted fn with namespace |",
              "|---|---|---|---|---|---|---|"]
    for prog, d in datas.items():
        trusted = [r for r in d.rows["server_functions"] if r[7] == "trusted"]
        with_ns = sum(1 for r in trusted if r[3])
        fsrc = ", ".join(f"{k} {v}" for k, v in sorted(d.func_by_source.items()))
        ssrc = ", ".join(f"{k} {v}" for k, v in sorted(d.sym_by_source.items()))
        tag = " (C++ class-method look)" if prog.startswith(("k2_linux", "k2_mac")) else ""
        lines.append(f"| {prog} | {d.raw_counts['functions']} | {fsrc} | {d.raw_counts['symbols']} | {ssrc} | "
                     f"{len(trusted)} | {with_ns}{tag} |")
    lines.append("")
    return "\n".join(lines), summary


# --------------------------------------------------------------------------- CLI

def expected_counts(data: ProgramData) -> dict[str, int]:
    return {t: len(data.rows[t]) for t in TABLES if t != "server_types" or data.has_types}


def db_counts(conn: sqlite3.Connection, program: str) -> dict[str, int | None]:
    return {t: (conn.execute(f"SELECT count(*) FROM {t} WHERE program=?", (program,)).fetchone()[0]
                if table_exists(conn, t) else None) for t in TABLES}


def main(argv: list[str] | None = None, out=None) -> int:
    out = out or sys.stdout

    def say(*a):
        print(*a, file=out)

    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sel = ap.add_mutually_exclusive_group()
    sel.add_argument("--programs", help="comma-separated program folder names")
    sel.add_argument("--all", action="store_true", help="every program folder under BD_SERVER")
    ap.add_argument("--apply", action="store_true", help="write the KDB (lead only)")
    ap.add_argument("--functions", choices=("trusted", "all"), default="trusted",
                    help="server_functions rows to load (default: trusted names only)")
    ap.add_argument("--report", metavar="OUT.md", help="write the diff report here")
    args = ap.parse_args(argv)
    if not (args.programs or args.all):
        ap.error("give --programs P1,P2 or --all")

    paths = env_paths()
    names = discover_programs(paths["server"]) if args.all else [p.strip() for p in args.programs.split(",") if p.strip()]
    datas = {n: load_program(paths["server"] / n, args.functions) for n in names}
    kdb = paths["kdb"]
    if not kdb.exists():
        ap.error(f"KDB {kdb} does not exist")

    say(f"mode: {'APPLY' if args.apply else 'dry run (read-only)'}   KDB: {kdb}")
    conn = sqlite3.connect(kdb, isolation_level=None) if args.apply else open_ro(kdb)
    try:
        if args.apply:
            conn.executescript(DDL)
        for prog, data in datas.items():
            for t, n in data.dup_counts.items():
                if n:
                    say(f"  note: {prog} {t}: {n} duplicate key rows collapsed")
            plan = plan_program(conn, data)
            say(f"{prog}: TSV functions {data.raw_counts['functions']}, symbols {data.raw_counts['symbols']} "
                f"(kept {len(data.rows['server_symbols'])}), types {data.raw_counts['types']}")
            for t, p in plan.items():
                say(f"  {t:17s} insert {p.new:6d}  update {p.changed:6d}  unchanged {p.unchanged:6d}"
                    f"  (stale in DB, kept: {p.stale})")
            if args.apply:
                done = apply_program(conn, data)
                say("  applied: " + ", ".join(f"{t} {n}" for t, n in done.items()))

        # gate checks
        say("\nGATES")
        all_ok = True
        for prog, data in datas.items():
            exp, have = expected_counts(data), db_counts(conn, prog)
            for t, n in exp.items():
                ok = have[t] == n
                state = "PASS" if ok else ("FAIL" if args.apply else "NOT-APPLIED")
                all_ok &= ok or not args.apply
                say(f"  rowcount {prog} {t}: tsv {n} db {have[t]} -> {state}")
        remaining = sum(p.changes for d in datas.values() for p in plan_program(conn, d).values())
        if args.apply:
            say(f"  idempotency: re-plan after apply = {remaining} changes -> {'PASS' if remaining == 0 else 'FAIL'}")
            all_ok &= remaining == 0
        else:
            say(f"  idempotency: pending changes = {remaining} (0 once applied)")
        integrity = conn.execute("PRAGMA integrity_check").fetchall()
        say(f"  integrity_check: {', '.join(r[0] for r in integrity)}")
        all_ok &= integrity == [("ok",)]
    finally:
        conn.close()

    if args.report:
        text, summary = build_report(datas, paths)
        Path(args.report).write_text(text + "\n", encoding="utf-8")
        say(f"\nreport written: {args.report}")
        for k, v in summary.items():
            say(f"  {k}: {v}")
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
