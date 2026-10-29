#!/usr/bin/env python3
"""Backfill function names that mod code knows but the knowledge DB / REA ledger do not.

Stdlib only.  Never writes to the knowledge DB (opened read-only).  The only writes are
ledger rows (`ledger --apply`) and the conflicts file next to the extract TSV.

CLI:
  ingest_names.py gap [--source modcode|notes|all] [--json]
  ingest_names.py extract --source modcode --out FILE
  ingest_names.py ledger --in FILE --topic ingest-modcode [--apply] [--date YYYY-MM-DD]
  ingest_names.py cards --in LEDGER_TSV --out-dir DIR [--chunk 12] [--decoy-every 4] [--seed N] [--decomp-lines 10]
                      [--decoy-pool batch|kdb]
  ingest_names.py twins --out FILE [--buckets FILE] [--order] [--gog-functions TSV] [--server DIR] [--extract-out F]
  ingest_names.py graphpair --buckets FILE --out FILE [--min-hits 2] [--margin 2] [--server DIR]
  ingest_names.py macnames --out FILE [--buckets FILE] [--targets TSV]... [--gog ADDR]... [--extract-out FILE]
  ingest_names.py score --key DIR/key.tsv --verdicts 'DIR/verdicts_*.tsv' --out promote.tsv
Common flags: --kdb DB  --ledger-dir DIR  --functions TSV  --root DIR  --dump-root DIR

extract TSV columns: addr kind name form src fn_start offset status kdb_name
  kind   = global (addr >= 0x980000) | function (addr is a function start) | callsite
  form   = qualified (Class::Method) | bare (name taken verbatim, never a guessed class)
  status = new | agrees | conflicts (vs name_map rows with confidence != 'wrong' and the ledger)
ledger writes `<YYYYMMDD>_<topic>.tsv` (confidence probable) for status=new function/global rows;
dry run by default.  KDB conflicts are never written anywhere.
cards writes blind Haiku verify cards (static facts + one candidate name, ~1 in 4 a same-kind decoy name)
as chunk_<k>.md plus key.tsv; score gates decoy rejection (>= 80%) and writes the promote TSV.
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
# The separator never spans "," or ";": in lists like "0x0040ee40 A::B, 0x00411b20" the name belongs to
# the address before it, not after.
SEP = r"[^\w,;]{1,6}"
NAME_THEN_ADDR_RX = re.compile(QNAME + SEP + r"(?:at|@|=)?\s*(0x00[4-9][0-9a-fA-F]{5})\b")
ADDR_THEN_NAME_RX = re.compile(r"\b(0x00[4-9][0-9a-fA-F]{5})" + SEP + QNAME)

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
    # ingest's own cards/extracts quote facts and decoy names; never read them back as notes
    skip = ("dump_excerpts", "/.claude/research/ingest/")
    return sorted(f for f in files if not any(s in f for s in skip) and os.path.isfile(f))


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


TABLE_SEP_RX = re.compile(r"^\s*\|?\s*:?-{2,}:?\s*(\|\s*:?-{2,}:?\s*)*\|?\s*$")
NAME_HEADER_RX = re.compile(r"name|function|symbol|method|func\b", re.I)
CELL_ADDR_RX = re.compile(r"0x00[4-9][0-9a-fA-F]{5}")
IDENT_RX = re.compile(r"^~?[A-Za-z_]\w*(?:::~?[A-Za-z_]\w*)*$")
DEFAULT_NAME_RX = re.compile(r"^(?:thunk_)?(?:FUN|DAT|LAB|PTR|SUB|UNK|switchD|caseD)_[0-9a-fA-F]+$")
# REA tool names and CRT/error words that notes put next to an address but that never name it
TOOL_NAME_RX = re.compile(r"^(?:(?:procedure|read|search|batch|analyze|resolve|find|inspect|open|close)_\w+"
                          r"|invalid_parameter)$")


def _cell_text(cell):
    t = cell.strip().strip("*_ ").strip("`").strip()
    return re.sub(r"\(\s*\)$", "", t)


def name_like(text):
    """True for Class::Method, CamelCase or snake_case identifiers (not prose, hex, numbers)."""
    if not IDENT_RX.match(text) or DEFAULT_NAME_RX.match(text) or TOOL_NAME_RX.match(text):
        return False
    if re.fullmatch(r"[0-9a-fA-F]+", text):
        return False
    if "::" in text or "_" in text.strip("_"):
        return True
    return bool(re.search(r"[a-z][A-Z]", text) or re.match(r"[A-Z][a-z]+[A-Z]", text) or
                re.match(r"[A-Z]{2,}[a-z]", text))


def _table_cells(line):
    s = line.strip()
    if not s.startswith("|"):
        return None
    s = s[1:]
    if s.endswith("|"):
        s = s[:-1]
    return s.split("|")


def table_pairs(cells, name_col):
    """(addr int, name) from one table row, or None.  name_col None -> first identifier-only cell."""
    texts = [_cell_text(c) for c in cells]
    addrs = [i for i, t in enumerate(texts) if CELL_ADDR_RX.fullmatch(t)]
    if len(addrs) != 1:
        return None
    ai = addrs[0]
    if name_col is not None:
        if name_col >= len(texts) or name_col == ai or not name_like(texts[name_col]):
            return None
        return int(texts[ai], 16), texts[name_col]
    for i, t in enumerate(texts):
        if i != ai and name_like(t):
            return int(texts[ai], 16), t
    return None


def scan_notes(path, relpath):
    """Yield (addr int, name, 'relpath:line') from one markdown note: pattern 3 anywhere + table rows."""
    text = _read(path)
    if text is None:
        return
    lines = text.splitlines()
    header = None  # name column index of the table being read
    for i, line in enumerate(lines):
        src = "%s:%d" % (relpath, i + 1)
        for m in NAME_THEN_ADDR_RX.finditer(line):
            yield int(m.group(2), 16), m.group(1), src
        for m in ADDR_THEN_NAME_RX.finditer(line):
            yield int(m.group(1), 16), m.group(2), src
        cells = _table_cells(line)
        if cells is None:
            header = None
            continue
        if TABLE_SEP_RX.match(line):
            continue
        nxt = lines[i + 1] if i + 1 < len(lines) else ""
        if TABLE_SEP_RX.match(nxt) and _table_cells(nxt) is not None:  # this row is a header
            header = None
            for ci, c in enumerate(cells):
                t = _cell_text(c)
                if NAME_HEADER_RX.search(t) and "addr" not in t.lower():
                    header = ci
                    break
            continue
        pair = table_pairs(cells, header)
        if pair:
            yield pair[0], pair[1], src


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


def extract(kdb_path=KDB_DEFAULT, ledger_dir=LEDGER_DEFAULT, functions=FUNCS_DEFAULT, root=REPO,
            source="modcode"):
    """List of row dicts (EXTRACT_COLS) deduped.  modcode: sorted by addr then src.
    notes: each row also has 'cites' (distinct note lines citing its addr+name); sorted by cites desc, addr, src."""
    kdb = load_kdb(kdb_path)
    ledger = load_ledger(ledger_dir)
    starts = load_starts(functions)
    notes = source == "notes"
    group, scanner = ("research_notes", scan_notes) if notes else ("mod_code", scan_file)
    seen = {}
    for f in group_files(root, group):
        rel = os.path.relpath(f, str(root))
        for a, name, src in scanner(f, rel):
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
    rows = [seen[k] for k in sorted(seen, key=lambda k: (k[0], k[2], k[1]))]
    if notes:
        cites = collections.Counter((r["addr"], r["name"]) for r in rows)
        for r in rows:
            r["cites"] = cites[(r["addr"], r["name"])]
        rows.sort(key=lambda r: (-r["cites"], r["addr"], r["src"], r["name"]))
    return rows


def top_new(rows, n):
    """Rows of the n most-cited new function/global addresses (best name per address; ties by addr, name)."""
    if n <= 0:
        return rows
    cites = collections.Counter((r["addr"], r["name"]) for r in rows)
    pairs = sorted({(r["addr"], r["name"]) for r in rows if r["status"] == "new"
                    and r["kind"] in ("function", "global")}, key=lambda p: (-cites[p], p[0], p[1]))
    keep, used = set(), set()
    for a, nm in pairs:
        if a not in used and len(keep) < n:
            used.add(a)
            keep.add((a, nm))
    return [r for r in rows if (r["addr"], r["name"]) in keep]


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
    if getattr(args, "out", None) and not args.apply:
        write_tsv(args.out, ledger_rows, rea_ledger.COLS)
        print(f"dry-run rows -> {args.out}", file=out)
    if args.apply:
        if ledger_rows:
            rea_ledger.append_rows(args.ledger_dir, target, ledger_rows)
        write_tsv(in_path.with_name("conflicts.tsv"), conflicts, ("addr", "chosen_name", "other_name", "src"))
        print(f"conflicts file: {in_path.with_name('conflicts.tsv')}", file=out)
    return 0


# ---- verify cards (I2) -------------------------------------------------------
KEY_COLS = ("card", "addr", "kind", "name", "is_decoy", "facts_addr", "src")
PROMOTE_COLS = ("addr", "kind", "name", "src", "verdict", "quote")
VERDICTS = ("consistent", "inconsistent", "unsure")
DECOY_MIN_DIST = 0x1000  # ScreenW/ScreenH-style neighbours would make a decoy look right
DECOY_PASS = 0.80
DECOY_SAME_QUAL = 20
FACT_LINE_MAX = 200
CHUNK_HEAD = """# Name check cards (blind)

Each card gives static facts about one address in swkotor2.exe and ONE candidate name for it.
Judge only from the facts on the card. Do not read any other file.

- consistent: the facts fit the name (what it calls, reads, compares, strings, how it is used).
- inconsistent: the facts contradict the name (e.g. "Vtable" but the address is a float divisor,
  "Iat" but it is never CALLed through, a GUI name on an audio routine).
- unsure: the facts say too little either way.

Naming noise to ignore: prefixes Addr/k/G_/g_, suffixes Global/Val/Flag. Iat = import address table
slot (CALL dword ptr [addr]); Vtable/Vtbl = vtable pointer stored into an object (MOV [reg],addr).

Write one TSV line per card: `card<TAB>verdict<TAB>quote` where quote is the one fact line
(copied verbatim, may be shortened) that decided it. Header line: `card\tverdict\tquote`.
"""


def _src_of(evidence):
    i = evidence.find("| src ")
    return evidence[i + 6:].strip() if i >= 0 else ""


PLACEHOLDER_RX = re.compile(r"\b(FUN|DAT|PTR|LAB)_([0-9a-fA-F]{8})\b")
DECL_RX = re.compile(r"^[\w\s\*\[\]]+;$")  # bare local declarations such as `undefined4 local_30;`
EXCERPT_LINE_MAX = 160


def decomp_excerpt(c_text, a, names, n=10):
    """Signature + up to n-1 body lines of dump pseudo-C, placeholders renamed from `names`.

    The function's own placeholder becomes `this_fn` (the card stays blind).  Lines that call a named
    function or hold a string literal come first, then other calls / vtable slots, then the rest; the
    picked lines keep their source order."""
    def rename(m):
        v = int(m.group(2), 16)
        if v == a:
            return "this_fn"
        nm = names.get(v)
        return nm if nm else m.group(0)

    sig, body = None, []
    for raw in c_text.splitlines():
        line = " ".join(raw.split())
        if not line or line.startswith("//") or line in ("{", "}"):
            continue
        if sig is None:
            sig = line
            continue
        if DECL_RX.match(line):
            continue
        body.append(line)
    if sig is None:
        return []

    def rank(line):
        named = re.search(r"\b[A-Za-z_]\w*::~?\w+\s*\(", line) or '"' in line
        return 0 if named else (1 if "(" in line else 2)

    renamed = [PLACEHOLDER_RX.sub(rename, ln) for ln in body]
    picked = sorted(sorted(range(len(renamed)), key=lambda i: (rank(renamed[i]), i))[:max(n - 1, 0)])
    out = ["- decomp: " + PLACEHOLDER_RX.sub(rename, sig)]
    out += ["    " + renamed[i] for i in picked]
    return [ln[:EXCERPT_LINE_MAX] for ln in out]


def function_fact_lines(dump, decomp_lines=0, names=None):
    """Callable addr -> fact lines from static_facts (each trimmed to FACT_LINE_MAX), plus an optional
    `decomp_lines`-line pseudo-C excerpt from the dump (decomp/<bb>/<addr>.c)."""
    from static_facts import facts, card_lines

    def get(a):
        lines = [ln[:FACT_LINE_MAX] for ln in card_lines(facts(dump, a))]
        if decomp_lines > 0:
            p = Path(dump.root) / "decomp" / ("%02x" % ((a >> 16) & 0xFF)) / ("%08x.c" % a)
            try:
                text = p.read_text(encoding="utf-8", errors="replace")
            except OSError:
                return lines + ["- decomp: none in dump"]
            lines += decomp_excerpt(text, a, names if names is not None else dump.names, decomp_lines)
        return lines
    return get


def global_fact_lines(decomp_root, names, max_hits=3):
    """Callable addr -> up to max_hits reference lines from the dump decomp (.c first, then .asm)."""
    import subprocess

    def get(a):
        h6 = "%06x" % (a & 0xFFFFFF)
        rx = r"(?i)(_00%s\b|0x(00)?%s\b)" % (h6, h6)
        res = subprocess.run(["rg", "-n", "--no-heading", "-e", rx, str(decomp_root)],
                             capture_output=True, text=True)
        hits = sorted(res.stdout.splitlines())
        fns = {Path(h.split(":", 1)[0]).stem for h in hits}
        lines = ["- referenced from %d function(s)" % len(fns)]
        picked, seen = [], set()
        for ext in (".c", ".asm"):
            for h in hits:
                f, ln, text = h.split(":", 2)
                fn = Path(f).stem
                if not f.endswith(ext) or fn in seen:
                    continue
                seen.add(fn)
                nm = names.get(int(fn, 16))
                picked.append("- in 0x%s%s: %s" % (fn, " (%s)" % nm if nm else "", " ".join(text.split())))
                if len(picked) >= max_hits:
                    break
            if len(picked) >= max_hits:
                break
        if not hits:
            lines = ["- no references in the decomp dump"]
        return [ln[:FACT_LINE_MAX] for ln in lines + picked]
    return get


def _qual(name):
    return name.rpartition("::")[0]


def kdb_decoy_pool(kdb_path):
    """name_map function rows (addr < GLOBAL_MIN) as decoy-name rows: {addr kind name evidence}."""
    return [{"addr": "0x%08x" % a, "kind": "function", "name": n, "evidence": "| src kdb name_map"}
            for a, n in sorted(load_kdb(kdb_path).items()) if a < GLOBAL_MIN and n]


def pick_decoys(rows, n, rng, pool=None):
    """n (facts_row, name_row) pairs: same kind, different name, addresses >= DECOY_MIN_DIST apart.

    `pool` (e.g. kdb_decoy_pool) supplies the decoy names instead of `rows`; names with the same
    qualifier as the real one (Global::, CExoFile::) are preferred when there are >= DECOY_SAME_QUAL.
    """
    by_kind = collections.defaultdict(list)
    for r in (rows if pool is None else pool):
        by_kind[r["kind"]].append(r)
    sources = rng.sample(rows, min(n, len(rows)))
    out = []
    for src in sources:
        a = int(src["addr"], 16)
        pool_ = [r for r in by_kind[src["kind"]]
                 if r["name"] != src["name"] and abs(int(r["addr"], 16) - a) >= DECOY_MIN_DIST]
        same = [r for r in pool_ if _qual(r["name"]) == _qual(src["name"])]
        cand = same if len(same) >= DECOY_SAME_QUAL else pool_
        if cand:
            out.append((src, rng.choice(cand)))
    return out


def build_cards(rows, fn_facts, glob_facts, decoy_every=4, seed=0, decoy_pool=None):
    """-> (cards, key): cards = [{card, kind, name, facts}], key rows per KEY_COLS.  Shuffled, seeded."""
    import random
    rng = random.Random(seed)
    facts_cache = {}

    def facts_of(r):
        if r["addr"] not in facts_cache:
            get = fn_facts if r["kind"] == "function" else glob_facts
            facts_cache[r["addr"]] = get(int(r["addr"], 16))
        return facts_cache[r["addr"]]

    items = [(r, r, False) for r in rows]
    n_decoys = len(rows) // decoy_every if decoy_every > 0 else 0
    items += [(src, nm, True) for src, nm in pick_decoys(rows, n_decoys, rng, decoy_pool)]
    rng.shuffle(items)
    width = max(2, len(str(len(items))))
    cards, key = [], []
    for i, (src, nm, decoy) in enumerate(items, 1):
        cid = "C%0*d" % (width, i)
        cards.append({"card": cid, "addr": src["addr"], "kind": src["kind"], "name": nm["name"],
                      "facts": facts_of(src)})
        key.append({"card": cid, "addr": nm["addr"], "kind": nm["kind"], "name": nm["name"],
                    "is_decoy": int(decoy), "facts_addr": src["addr"], "src": _src_of(nm["evidence"])})
    return cards, key


def card_text(c):
    return "## %s\nkind: %s\naddress: %s\ncandidate name: `%s`\nfacts:\n%s\n" % (
        c["card"], c["kind"], c["addr"], c["name"], "\n".join(c["facts"]))


def write_cards(cards, key, out_dir, chunk=12):
    os.makedirs(out_dir, exist_ok=True)
    paths = []
    for k in range(0, len(cards), chunk):
        p = os.path.join(out_dir, "chunk_%d.md" % (k // chunk + 1))
        with open(p, "w", encoding="utf-8") as fh:
            fh.write(CHUNK_HEAD + "\n" + "\n".join(card_text(c) for c in cards[k:k + chunk]))
        paths.append(p)
    write_tsv(os.path.join(out_dir, "key.tsv"), key, KEY_COLS)
    return paths


def read_verdicts(paths):
    """card -> (verdict, quote); unknown verdict words become 'unsure'."""
    out = {}
    for p in paths:
        with open(p, encoding="utf-8") as fh:
            for line in fh:
                parts = line.rstrip("\r\n").split("\t")
                if len(parts) < 2 or parts[0].strip().lower() == "card":
                    continue
                v = parts[1].strip().lower()
                out[parts[0].strip()] = (v if v in VERDICTS else "unsure",
                                         parts[2].strip() if len(parts) > 2 else "")
    return out


def score(key, verdicts):
    """-> dict with decoy rejection (total + per kind), PASS flag, promote rows, missing cards."""
    per = collections.defaultdict(lambda: [0, 0])
    promote, missing, misses = [], [], []
    for k in key:
        v = verdicts.get(k["card"])
        if v is None:
            missing.append(k["card"])
            v = ("unsure", "")
        if str(k["is_decoy"]) == "1":
            per[k["kind"]][1] += 1
            if v[0] == "inconsistent":
                per[k["kind"]][0] += 1
            else:
                misses.append({**k, "verdict": v[0], "quote": v[1]})
        elif v[0] == "consistent":
            promote.append({"addr": k["addr"], "kind": k["kind"], "name": k["name"], "src": k["src"],
                            "verdict": v[0], "quote": v[1]})
    rej = sum(p[0] for p in per.values())
    tot = sum(p[1] for p in per.values())
    ratio = rej / tot if tot else 0.0
    return {"rejected": rej, "decoys": tot, "ratio": ratio, "pass": tot > 0 and ratio >= DECOY_PASS,
            "per_kind": {kd: tuple(p) for kd, p in sorted(per.items())},
            "promote": promote, "missing": missing, "misses": misses}


def run_cards(args, dump, out=sys.stdout):
    from static_facts import load_names
    rows = read_tsv(args.in_path)
    names = load_names()
    if not dump.names:
        dump.names = dict(names)
    root = Path(args.dump_root) if args.dump_root else dump.root
    cards, key = build_cards(rows, function_fact_lines(dump, args.decomp_lines, names), global_fact_lines(root / "decomp", names),
                             args.decoy_every, args.seed,
                             kdb_decoy_pool(args.kdb) if args.decoy_pool == "kdb" else None)
    paths = write_cards(cards, key, args.out_dir, args.chunk)
    nd = sum(k["is_decoy"] for k in key)
    print(f"cards: rows={len(rows)} decoys={nd} cards={len(cards)} chunks={len(paths)} -> {args.out_dir}",
          file=out)
    return 0


def run_score(args, out=sys.stdout):
    key = read_tsv(args.key)
    res = score(key, read_verdicts(sorted(glob.glob(args.verdicts))))
    split = " ".join("%s=%d/%d" % (kd, r, t) for kd, (r, t) in res["per_kind"].items())
    print("decoy rejection %d/%d = %.0f%% (%s) -> %s" % (
        res["rejected"], res["decoys"], 100 * res["ratio"], split, "PASS" if res["pass"] else "FAIL"),
        file=out)
    if res["missing"]:
        print("missing verdicts: " + " ".join(res["missing"]), file=out)
    for m in res["misses"]:
        print("  miss %s %s on facts %s: %s (%s)" % (m["card"], m["name"], m["facts_addr"], m["verdict"],
                                                    m["quote"][:100]), file=out)
    if not res["pass"]:
        print("no promote file written", file=out)
        return 1
    write_tsv(args.out, res["promote"], PROMOTE_COLS)
    print(f"promote: {len(res['promote'])} rows -> {args.out}", file=out)
    return 0


# ---- twins -------------------------------------------------------------------
TWIN_COLS = ("gog", "steam", "source", "name", "cites")
BUCKET_COLS = ("steam", "bucket", "gog_addrs", "steam_peers", "name", "cites", "in_gog_functions", "order_gog")
SERVER_DEFAULT = "~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/"
FP_STEAM = "k2_win_steam_aspyr_swkotor2.exe"
FP_GOG = "k2_win_gog_aspyr_swkotor2.exe"


def _fp_key(r):
    """Structural fingerprint of one fp.tsv row; identical code in two builds gives the same key."""
    callees, strs = r["callees"], r["str_refs"]
    return (r["size"], r["retn"], r["n_insn"], r["disp"], r["imm"],
            len(callees.split(",")) if callees else 0, len(strs.split(",")) if strs else 0)


def _load_fp(path):
    """({addr int: row}, {key: [addrs sorted]}) for one fp.tsv."""
    by_addr, groups = {}, collections.defaultdict(list)
    for r in read_tsv(path):
        a = int(r["addr"], 16)
        by_addr[a] = r
        groups[_fp_key(r)].append(a)
    for v in groups.values():
        v.sort()
    return by_addr, groups


GOG_EVIDENCE_RX = re.compile(r"\bGOG (0x[0-9a-fA-F]{6,8})\b")


def load_kdb_pairs(path):
    """[(steam int, gog int)] from name_map rows (confidence != 'wrong') whose evidence names the GOG addr."""
    con = sqlite3.connect("file:" + str(path) + "?mode=ro", uri=True)
    try:
        out = []
        for addr, ev in con.execute("select steam_addr, evidence from name_map where confidence != 'wrong'"):
            m = GOG_EVIDENCE_RX.search(ev or "")
            a = _hex(addr)
            if m and a is not None:
                out.append((a, int(m.group(1), 16)))
        return out
    finally:
        con.close()


def load_gog_functions(path):
    """Set of GOG addrs (int) listed in DUMP/inputs/gog_functions.tsv."""
    out = set()
    for r in read_tsv(path):
        a = _hex(r.get("gog_addr"))
        if a is not None:
            out.add(a)
    return out


def _gname(r):
    return "%s::%s" % (r["namespace"] or "Global", r["name"]) if r["name"] and not r["name"].startswith("FUN_") else ""


def order_pairs(s_addrs, g_addrs, anchors):
    """k-th Steam <-> k-th GOG pairs of one ambiguous key group, or [] when the order is not proven.

    Needs equal group sizes; the pairs plus every trusted anchor (steam, gog) whose Steam addr lies inside
    the group's Steam span, plus the nearest anchor on each side, must form one strictly increasing run.
    `anchors` is sorted by Steam addr.
    """
    if len(s_addrs) != len(g_addrs) or len(s_addrs) < 2:
        return []
    pairs = list(zip(s_addrs, g_addrs))
    steams = [a for a, _g in anchors]
    lo = max(bisect.bisect_left(steams, s_addrs[0]) - 1, 0)
    hi = bisect.bisect_right(steams, s_addrs[-1]) + 1
    run = sorted(set(pairs) | set(anchors[lo:hi]))
    for (s0, g0), (s1, g1) in zip(run, run[1:]):
        if s1 <= s0 or g1 <= g0:
            return []
    return pairs


def twins_full(server, source="all", kdb_path=KDB_DEFAULT, ledger_dir=LEDGER_DEFAULT, functions=FUNCS_DEFAULT,
               root=REPO, order=False, gog_functions=None):
    """(rows, counts, buckets) for every gap function.

    rows: unique named GOG twins (source fptwin), plus with `order` ambiguous-group pairs proven by address
    order (source fptwin_order).  counts keys: gap named unnamed ambiguous no_twin no_fp (sum to gap) + order.
    buckets: one BUCKET_COLS row per gap function.  `gog_functions` (path) fills in_gog_functions.
    """
    steam, steam_groups = _load_fp(os.path.join(server, FP_STEAM, "fp.tsv"))
    gog, gog_groups = _load_fp(os.path.join(server, FP_GOG, "fp.tsv"))
    in_gog = load_gog_functions(gog_functions) if gog_functions else None
    anchors = None
    # a function can sit in several source groups: dedupe by address, sum its cites across groups
    cites = collections.Counter()
    for d in gap(source, kdb_path, ledger_dir, functions, root, top_n=100000).values():
        for t in d["top"]:
            cites[int(t["fn"], 16)] += t["cites"]
    counts = collections.Counter(gap=len(cites), order=0)
    rows, buckets = [], []

    def flag(g):
        return "" if in_gog is None else ("yes" if g in in_gog else "no")

    for a in sorted(cites):
        b = {"steam": "0x%08x" % a, "bucket": "no_fp", "gog_addrs": "", "steam_peers": "", "name": "",
             "cites": cites[a], "in_gog_functions": "", "order_gog": ""}
        buckets.append(b)
        s = steam.get(a)
        if s is None:
            counts["no_fp"] += 1
            continue
        k = _fp_key(s)
        gs, ss = gog_groups.get(k, []), steam_groups[k]
        b["gog_addrs"] = ",".join("0x%08x" % g for g in gs)
        b["steam_peers"] = ",".join("0x%08x" % x for x in ss if x != a)
        if not gs:
            b["bucket"] = "no_twin"
            counts["no_twin"] += 1
        elif len(ss) > 1 or len(gs) > 1:
            b["bucket"] = "ambiguous"
            b["name"] = ",".join(_gname(gog[g]) or "-" for g in gs)
            if order:
                if anchors is None:
                    anchors = sorted(set(load_kdb_pairs(kdb_path)) |
                                     {(v[0], gog_groups[kk][0]) for kk, v in steam_groups.items()
                                      if len(v) == 1 and len(gog_groups.get(kk, ())) == 1})
                g = dict(order_pairs(ss, gs, anchors)).get(a)
                if g is not None:
                    b["order_gog"] = "0x%08x" % g
                    b["in_gog_functions"] = flag(g)
                    name = _gname(gog[g])
                    if name:
                        counts["order"] += 1
                        rows.append({"gog": "0x%08x" % g, "steam": "0x%08x" % a, "source": "fptwin_order",
                                     "name": name, "cites": cites[a]})
            counts["ambiguous"] += 1
        else:
            g = gs[0]
            name = _gname(gog[g])
            b["in_gog_functions"] = flag(g)
            if not name:
                b["bucket"] = "unnamed"
                counts["unnamed"] += 1
                continue
            b["bucket"], b["name"] = "named", name
            counts["named"] += 1
            rows.append({"gog": "0x%08x" % g, "steam": "0x%08x" % a, "source": "fptwin",
                         "name": name, "cites": cites[a]})
    return rows, counts, buckets


def twin_extract_rows(rows, buckets):
    """fptwin rows whose GOG fn is absent from gog_functions.tsv (in_gog_functions=no) as extract rows.

    batch.py cannot see these GOG fns, so they go through ledger + cards instead.  FID_conflict names
    (one FLIRT signature, several library fns) are skipped.
    """
    absent = {b["steam"] for b in buckets if b["bucket"] == "named" and b["in_gog_functions"] == "no"}
    return [{"addr": r["steam"], "kind": "function", "name": r["name"], "form": "qualified",
             "src": "fptwin GOG %s" % r["gog"], "fn_start": r["steam"], "offset": "0", "status": "new",
             "kdb_name": ""}
            for r in rows if r["source"] == "fptwin" and r["steam"] in absent and "FID_conflict" not in r["name"]]


def twins(server, source="all", kdb_path=KDB_DEFAULT, ledger_dir=LEDGER_DEFAULT, functions=FUNCS_DEFAULT,
          root=REPO, order=False):
    """Gap functions whose structural twin in the GOG exe carries a real name: (rows, counts)."""
    rows, counts, _b = twins_full(server, source, kdb_path, ledger_dir, functions, root, order)
    return rows, counts


# ---- graphpair (I9a) ---------------------------------------------------------
GRAPH_COLS = ("gog", "steam", "source", "name", "score", "margin", "fp_equal", "bucket")
GRAPH_SKIP = ("named", "unnamed", "no_fp")  # a unique twin already decides these rows


def _callee_set(r):
    return {a for a in (_hex(x) for x in r["callees"].split(",")) if a is not None} if r["callees"] else set()


def _caller_index(by_addr):
    out = collections.defaultdict(set)
    for a, r in by_addr.items():
        for c in _callee_set(r):
            out[c].add(a)
    return out


def trusted_map(steam_groups, gog_groups, kdb_path):
    """({gog: steam}, {steam: gog}): KDB pairs plus unique fingerprint twins; an addr claimed twice is dropped."""
    pairs = set(load_kdb_pairs(kdb_path)) if kdb_path and os.path.exists(kdb_path) else set()
    pairs |= {(v[0], gog_groups[k][0]) for k, v in steam_groups.items()
              if len(v) == 1 and len(gog_groups.get(k, ())) == 1}
    gs = collections.Counter(g for _s, g in pairs)
    ss = collections.Counter(s for s, _g in pairs)
    g2s = {g: s for s, g in pairs if gs[g] == 1 and ss[s] == 1}
    return g2s, {s: g for g, s in g2s.items()}


def graphpair(server, buckets, kdb_path=KDB_DEFAULT, min_hits=2, margin=2):
    """(rows, counts) GOG twins for gap rows decided by call-graph neighbours mapped through trusted pairs.

    Candidates: the row's `gog_addrs` when set, else the GOG callees of each mapped Steam caller and the GOG
    callers of each mapped Steam callee.  GOG fns already in the trusted map are dropped.  score = mapped
    callee hits + mapped caller hits.  A row ships when best >= min_hits and best - 2nd >= margin, or when the
    best is fp-equal with best >= 1 and no rival scores.  A GOG fn won by two Steam rows ships for neither.
    """
    steam, steam_groups = _load_fp(os.path.join(server, FP_STEAM, "fp.tsv"))
    gog, gog_groups = _load_fp(os.path.join(server, FP_GOG, "fp.tsv"))
    g2s, s2g = trusted_map(steam_groups, gog_groups, kdb_path)
    s_callers, g_callers = _caller_index(steam), _caller_index(gog)
    counts = collections.Counter()
    won = []
    for b in read_tsv(buckets):
        a = _hex(b["steam"])
        bucket = b.get("bucket", "")
        if bucket in GRAPH_SKIP:
            continue
        counts["in_" + bucket] += 1
        s = steam.get(a)
        if s is None:
            counts["no_fp"] += 1
            continue
        s_out, s_in = _callee_set(s), s_callers.get(a, set())
        cands = {g for g in (_hex(x) for x in (b.get("gog_addrs") or "").split(",")) if g is not None}
        if not cands:
            for c in s_in:
                if c in s2g and s2g[c] in gog:
                    cands |= _callee_set(gog[s2g[c]])
            for x in s_out:
                if x in s2g:
                    cands |= g_callers.get(s2g[x], set())
        cands = [g for g in cands if g in gog and g not in g2s]
        if not cands:
            counts["no_cand"] += 1
            continue
        key = _fp_key(s)
        scored = sorted(((sum(g2s.get(c) in s_out for c in _callee_set(gog[g])) +
                          sum(g2s.get(c) in s_in for c in g_callers.get(g, ())),
                          _fp_key(gog[g]) == key, g) for g in cands), key=lambda t: (-t[0], -t[1], t[2]))
        best, fp_eq, g = scored[0]
        second = scored[1][0] if len(scored) > 1 else 0
        if not ((best >= min_hits and best - second >= margin) or (fp_eq and best >= 1 and second == 0)):
            counts["undecided"] += 1
            continue
        won.append({"gog": "0x%08x" % g, "steam": "0x%08x" % a, "source": "fpgraph", "name": _gname(gog[g]),
                    "score": best, "margin": best - second, "fp_equal": "yes" if fp_eq else "no", "bucket": bucket})
    claims = collections.Counter(r["gog"] for r in won)
    rows = [r for r in won if claims[r["gog"]] == 1]
    counts["conflict"] = len(won) - len(rows)
    for r in rows:
        counts["win_" + r["bucket"]] += 1
        counts["named"] += bool(r["name"])
    return rows, counts


# ---- macnames (I8b)----------------------------------------------------------
MAC_COLS = ("gog", "steam", "gog_name", "mac_name", "mac_addr", "fit", "margin", "strings", "keep", "classes", "top3")
FP_MAC = "k2_mac_KOTOR2_x86"
MAC_NEIGHBOURS = 2  # named GOG fns on each side whose classes join the candidate set
BDLIB_DIR = REPO / "patch_manager_mods" / "steam_db_batches" / "tools"
MAC_FIT_MIN, MAC_MARGIN_MIN = 0.5, 0.15


def _bdlib():
    if str(BDLIB_DIR) not in sys.path:
        sys.path.insert(0, str(BDLIB_DIR))
    import bdlib
    return bdlib


def mac_targets(buckets=None, targets=(), gogs=()):
    """[(gog int, steam str, skip_named bool)] in input order, deduped by GOG addr.

    buckets (twins --buckets TSV): unnamed rows + every ambiguous row paired by --order; skip_named is set for
    these, so an order pair whose GOG fn already has a name is left to B18.  targets: TSVs with a gog|gog_addr
    column and optional steam|steam_counterpart (e.g. label suspects: named, but the name is doubted).
    """
    out = collections.OrderedDict()
    if buckets:
        for r in read_tsv(buckets):
            g = r["gog_addrs"] if r["bucket"] == "unnamed" else r.get("order_gog", "")
            if _hex(g) is not None:
                out.setdefault(_hex(g), (r["steam"], True))
    for t in targets:
        for r in read_tsv(t):
            g = _hex(r.get("gog") or r.get("gog_addr"))
            if g is not None:
                out.setdefault(g, ((r.get("steam") or r.get("steam_counterpart") or "").strip(), False))
    for g in gogs:
        if _hex(g) is not None:
            out.setdefault(_hex(g), ("", False))
    return [(g, st, sk) for g, (st, sk) in out.items()]


def macnames(server, targets):
    """MAC_COLS rows: best Mac x86 symbol per target GOG fn by bdlib mac_fit (fit = 0.75 value overlap +
    0.25 size closeness).  keep = yes when fit >= 0.5 and the margin over the best other Mac name >= 0.15.
    strings = string texts the GOG fn and that Mac fn both reference.  Targets flagged skip_named are dropped
    when their GOG fn already carries a real name."""
    bd = _bdlib()
    corpus = object.__new__(bd.Corpus)          # mac_fit needs only gog_fp (+ resolve_gog, _values)
    gdir, mdir = os.path.join(server, FP_GOG), os.path.join(server, FP_MAC)
    corpus.gog_fp = bd.load_fp(Path(gdir) / "fp.tsv")
    mac_fp = bd.load_fp(Path(mdir) / "fp.tsv")
    gtext = bd.load_server_strings(Path(gdir) / "strings.tsv")
    mtext = bd.load_server_strings(Path(mdir) / "strings.tsv")
    macs = [m for m in mac_fp.values() if m.name and not m.name.startswith("FUN_")]
    mac_ns = collections.defaultdict(list)
    mac_by_text = collections.defaultdict(set)
    for m in macs:
        mac_ns[m.namespace or ""].append(m)
        for a in m.str_refs:
            if mtext.get(a):
                mac_by_text[mtext[a]].add(m.addr)
    named = sorted(a for a, f in corpus.gog_fp.items()
                   if f.name and not f.name.startswith("FUN_") and f.namespace and f.namespace != "Global")
    rows = []
    for g, steam, skip_named in targets:
        gfp = corpus.gog_fp.get(g)
        gname = ("%s::%s" % (gfp.namespace or "Global", gfp.name)) if gfp and gfp.name else ""
        if skip_named and gname and not gfp.name.startswith("FUN_"):
            continue
        x = bd.GogFn(g, "", "", "", None, "")
        # candidates: Mac fns of the target's class, of the classes of its nearest named GOG neighbours, or
        # sharing a string text with it.  A search over every Mac fn is noise (value overlap fits ~everything).
        i = bisect.bisect_left(named, g)
        classes = {corpus.gog_fp[a].namespace for a in named[max(0, i - MAC_NEIGHBOURS):i + MAC_NEIGHBOURS]}
        if gfp and gfp.namespace and gfp.namespace != "Global":
            classes.add(gfp.namespace)
        texts = {gtext.get(a, "") for a in (gfp.str_refs if gfp else ())} - {""}
        by_text = set().union(*(mac_by_text.get(t, set()) for t in texts)) if texts else set()
        cands = [m for c in sorted(classes) for m in mac_ns.get(c, [])] + [mac_fp[a] for a in sorted(by_text)]
        best = {}
        for m in cands:
            f = corpus.mac_fit(x, m)
            if f is None:
                continue
            n = "%s::%s" % (m.namespace or "Global", m.name)
            if n not in best or f > best[n][0]:
                best[n] = (f, m)
        ranked = sorted(best.items(), key=lambda kv: (-kv[1][0], kv[0]))
        r = {"gog": "0x%08x" % g, "steam": steam, "gog_name": gname, "mac_name": "", "mac_addr": "",
             "fit": "", "margin": "", "strings": "", "keep": "no", "classes": ",".join(sorted(classes)),
             "top3": ",".join("%s:%.2f" % (n, f) for n, (f, _m) in ranked[:3])}
        if ranked:
            n, (f, m) = ranked[0]
            margin = f - ranked[1][1][0] if len(ranked) > 1 else f
            shared = sorted({gtext.get(a, "") for a in (gfp.str_refs if gfp else ())} &
                            {mtext.get(a, "") for a in m.str_refs} - {""})
            r.update(mac_name=n, mac_addr="0x%08x" % m.addr, fit="%.2f" % f, margin="%.2f" % margin,
                     strings=" | ".join(_clean(t)[:40] for t in shared[:3]),
                     keep="yes" if f >= MAC_FIT_MIN and margin >= MAC_MARGIN_MIN else "no")
        rows.append(r)
    return rows


def mac_extract_rows(rows):
    """keep=yes rows with a Steam addr as extract rows (status new), ready for `ledger --in`."""
    return [{"addr": r["steam"], "kind": "function", "name": r["mac_name"], "form": "qualified",
             "src": "mac %s fit=%s margin=%s via GOG %s" % (r["mac_addr"], r["fit"], r["margin"], r["gog"]),
             "fn_start": r["steam"], "offset": "0", "status": "new", "kdb_name": ""}
            for r in rows if r["keep"] == "yes" and _hex(r["steam"]) is not None]


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
    e.add_argument("--source", choices=["modcode", "notes"], default="modcode")
    e.add_argument("--top", type=int, default=0,
                   help="keep only the N most-cited new function/global addresses (0 = all)")
    e.add_argument("--out", required=True)
    le = sub.add_parser("ledger", parents=[common], help="turn extract rows into ledger rows")
    le.add_argument("--in", dest="in_path", required=True)
    le.add_argument("--topic", required=True)
    le.add_argument("--apply", action="store_true")
    le.add_argument("--date", default=None)
    le.add_argument("--out", default=None, help="dry run only: write the would-write ledger rows to this TSV")
    c = sub.add_parser("cards", parents=[common], help="blind Haiku verify cards with decoys")
    c.add_argument("--in", dest="in_path", required=True)
    c.add_argument("--out-dir", required=True)
    c.add_argument("--chunk", type=int, default=12)
    c.add_argument("--decoy-every", type=int, default=4)
    c.add_argument("--seed", type=int, default=0)
    c.add_argument("--decoy-pool", choices=["batch", "kdb"], default="batch",
                   help="decoy names from the input rows (batch) or from name_map functions (kdb)")
    c.add_argument("--decomp-lines", type=int, default=0, help="add an N-line named pseudo-C excerpt to function cards")
    t = sub.add_parser("twins", parents=[common], help="names for gap functions from unique GOG fingerprint twins")
    t.add_argument("--source", choices=sorted(SOURCE_GROUPS), default="all")
    t.add_argument("--server", default=None, help="upstream server dump dir (default: $BD_SERVER or %s)" % SERVER_DEFAULT)
    t.add_argument("--out", required=True)
    t.add_argument("--buckets", default=None, help="also write every gap fn with its bucket to this TSV")
    t.add_argument("--order", action="store_true",
                   help="pair equal-size ambiguous key groups by address order (source fptwin_order)")
    t.add_argument("--extract-out", default=None,
                   help="named twins absent from gog_functions.tsv as extract TSV for `ledger --in` (CRT lane)")
    t.add_argument("--gog-functions", default=None,
                   help="GOG functions TSV for the in_gog_functions column (default: DUMP/inputs/gog_functions.tsv "
                        "when it exists)")
    gp = sub.add_parser("graphpair", parents=[common],
                        help="GOG twins for ambiguous/no_twin gap fns from mapped callers/callees")
    gp.add_argument("--buckets", required=True, help="twins --buckets TSV (or any steam/bucket/gog_addrs TSV)")
    gp.add_argument("--out", required=True)
    gp.add_argument("--server", default=None)
    gp.add_argument("--min-hits", type=int, default=2)
    gp.add_argument("--margin", type=int, default=2)
    mn = sub.add_parser("macnames", help="Mac x86 name hypotheses for GOG fns by value/size fit (bdlib mac_fit)")
    mn.add_argument("--buckets", default=None, help="twins --buckets TSV: unnamed + order-paired rows")
    mn.add_argument("--targets", action="append", default=[], help="TSV with gog|gog_addr [+ steam|steam_counterpart]")
    mn.add_argument("--gog", action="append", default=[], help="extra GOG addr (repeatable)")
    mn.add_argument("--server", default=None)
    mn.add_argument("--out", required=True)
    mn.add_argument("--extract-out", default=None, help="keep=yes rows as extract TSV for `ledger --in`")
    s = sub.add_parser("score", help="gate decoy rejection, write promote TSV")
    s.add_argument("--key", required=True)
    s.add_argument("--verdicts", required=True, help="glob of verdicts_*.tsv")
    s.add_argument("--out", required=True)
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
            rows = extract(args.kdb, args.ledger_dir, args.functions, args.root, args.source)
            rows = top_new(rows, args.top)
            write_tsv(args.out, rows, EXTRACT_COLS + (("cites",) if args.source == "notes" else ()))
            c = collections.Counter(r["status"] for r in rows)
            print(f"extract: rows={len(rows)} new={c['new']} agrees={c['agrees']} "
                  f"conflicts={c['conflicts']} -> {args.out}", file=out)
            return 0
        if args.cmd == "twins":
            server = os.path.expanduser(args.server or os.environ.get("BD_SERVER") or SERVER_DEFAULT)
            gf = args.gog_functions
            if gf is None and args.buckets:
                from static_facts import DUMP_ROOT
                cand = os.path.join(args.dump_root or str(DUMP_ROOT), "inputs", "gog_functions.tsv")
                gf = cand if os.path.exists(cand) else None
            rows, c, buckets = twins_full(server, args.source, args.kdb, args.ledger_dir, args.functions, args.root,
                                          args.order, gf)
            write_tsv(args.out, rows, TWIN_COLS)
            extra = f" order={c['order']}" if args.order else ""
            print(f"twins: gap={c['gap']} named={c['named']} unnamed={c['unnamed']} ambiguous={c['ambiguous']} "
                  f"no_twin={c['no_twin']} no_fp={c['no_fp']}{extra} -> {args.out}", file=out)
            if args.buckets:
                write_tsv(args.buckets, buckets, BUCKET_COLS)
                print(f"twins: buckets={len(buckets)} -> {args.buckets}", file=out)
            if args.extract_out:
                ex = twin_extract_rows(rows, buckets)
                write_tsv(args.extract_out, ex, EXTRACT_COLS)
                print(f"twins: extract rows={len(ex)} -> {args.extract_out}", file=out)
            return 0
        if args.cmd == "graphpair":
            server = os.path.expanduser(args.server or os.environ.get("BD_SERVER") or SERVER_DEFAULT)
            rows, c = graphpair(server, args.buckets, args.kdb, args.min_hits, args.margin)
            write_tsv(args.out, rows, GRAPH_COLS)
            per = " ".join(f"{k[3:]}={c['win_' + k[3:]]}/{v}" for k, v in sorted(c.items()) if k.startswith("in_"))
            print(f"graphpair: rows={len(rows)} named={c['named']} {per} undecided={c['undecided']} "
                  f"no_cand={c['no_cand']} no_fp={c['no_fp']} conflict={c['conflict']} -> {args.out}", file=out)
            return 0
        if args.cmd == "macnames":
            server = os.path.expanduser(args.server or os.environ.get("BD_SERVER") or SERVER_DEFAULT)
            rows = macnames(server, mac_targets(args.buckets, args.targets, args.gog))
            write_tsv(args.out, rows, MAC_COLS)
            keep = sum(r["keep"] == "yes" for r in rows)
            print(f"macnames: targets={len(rows)} keep={keep} -> {args.out}", file=out)
            if args.extract_out:
                ex = mac_extract_rows(rows)
                write_tsv(args.extract_out, ex, EXTRACT_COLS)
                print(f"macnames: extract rows={len(ex)} -> {args.extract_out}", file=out)
            return 0
        if args.cmd == "ledger":
            return run_ledger(args, dump if dump is not None else _make_dump(args.dump_root), out, err)
        if args.cmd == "cards":
            return run_cards(args, dump if dump is not None else _make_dump(args.dump_root), out)
        if args.cmd == "score":
            return run_score(args, out)
    except (OSError, sqlite3.Error, rea_ledger.LedgerError) as exc:
        print(f"ingest_names: {exc}", file=err)
        return 2
    return 2


if __name__ == "__main__":
    sys.exit(main())
