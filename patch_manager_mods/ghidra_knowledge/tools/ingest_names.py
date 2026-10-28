#!/usr/bin/env python3
"""Backfill function names that mod code knows but the knowledge DB / REA ledger do not.

Stdlib only.  Never writes to the knowledge DB (opened read-only).  The only writes are
ledger rows (`ledger --apply`) and the conflicts file next to the extract TSV.

CLI:
  ingest_names.py gap [--source modcode|notes|all] [--json]
  ingest_names.py extract --source modcode --out FILE
  ingest_names.py ledger --in FILE --topic ingest-modcode [--apply] [--date YYYY-MM-DD]
  ingest_names.py cards --in LEDGER_TSV --out-dir DIR [--chunk 12] [--decoy-every 4] [--seed N]
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


def function_fact_lines(dump):
    """Callable addr -> fact lines from static_facts (each trimmed to FACT_LINE_MAX)."""
    from static_facts import facts, card_lines

    def get(a):
        return [ln[:FACT_LINE_MAX] for ln in card_lines(facts(dump, a))]
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


def pick_decoys(rows, n, rng):
    """n (facts_row, name_row) pairs: same kind, different name, addresses >= DECOY_MIN_DIST apart."""
    by_kind = collections.defaultdict(list)
    for r in rows:
        by_kind[r["kind"]].append(r)
    sources = rng.sample(rows, min(n, len(rows)))
    out = []
    for src in sources:
        a = int(src["addr"], 16)
        pool = [r for r in by_kind[src["kind"]]
                if r["name"] != src["name"] and abs(int(r["addr"], 16) - a) >= DECOY_MIN_DIST]
        if pool:
            out.append((src, rng.choice(pool)))
    return out


def build_cards(rows, fn_facts, glob_facts, decoy_every=4, seed=0):
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
    items += [(src, nm, True) for src, nm in pick_decoys(rows, n_decoys, rng)]
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
    cards, key = build_cards(rows, function_fact_lines(dump), global_fact_lines(root / "decomp", names),
                             args.decoy_every, args.seed)
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
