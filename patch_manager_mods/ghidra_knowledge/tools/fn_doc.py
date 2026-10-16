#!/usr/bin/env python3
"""Per-class function pages and drafting input from the Steam static dump (read-only, stdlib only).

Builds on static_facts (callers, callees, strings, globals, asm) and adds the Steam DB class
membership, knowledge-DB purposes and struct fields, REA ledger callsite rows and named
pseudo-C.  check-drafts validates drafter TSV rows against the dump's asm.

CLI:
  fn_doc.py page <Class> [--out DIR] [--split-bytes N]
  fn_doc.py draft-input <Class> [--n 25] [--out DIR]
  fn_doc.py check-drafts <tsv> [--out FILE] [--root DIR]
page and draft-input also take --root, --steam-db, --kdb, --named-root, --ledger-dir.
"""
from __future__ import annotations

import argparse
import os
import re
import sqlite3
import sys
from pathlib import Path
from urllib.parse import quote

import rea_ledger
import static_facts as sf

SPLIT_BYTES = 400 * 1024
PAGE_NAMED_LINES = 30
DRAFT_ASM_LINES = 70
DRAFT_NAMED_LINES = 40
DRAFT_LIST_CAP = 30  # per callee/string/global/field list in draft-input (keeps a chunk <~100 KB)
PURPOSE_MAX_WORDS = 20
BINARY_HASH = "6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef"
STEAM_DOCS_DEFAULT = sf.REPO / "patch_manager_mods" / "ghidra_knowledge" / "steam_docs"
NAMED_ROOT_DEFAULT = sf.DUMP_ROOT / "named_decomp"

_CLASS_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_ADDR_RE = re.compile(r"0[xX][0-9a-fA-F]+")
_ANCHOR_RE = re.compile(r"(0[xX][0-9a-fA-F]+): (\S.*)")
_FN_TOKEN_RE = re.compile(r"\b(?:FUN|LAB|DAT)_[0-9A-Za-z]+\b")
_HEX_TOKEN_RE = re.compile(r"0[xX][0-9a-fA-F]+")
_PTR_RE = re.compile(r"\b(?:dword|word|byte|qword) ptr ")
_HEX_NORM_RE = re.compile(r"0x[0-9a-f]+")

_DRAFT_HELP = """# Draft input: {cls} chunk {k} of {K}

Functions {lo}-{hi} of {total}, sorted by address.

## Instructions (read first)

Output one row per function in this file. No header, no code fences, no prose.
Each row is three tab-separated columns: `addr<TAB>purpose<TAB>anchor`

- addr: the function address from its heading: 0x followed by 8 hex digits (e.g. 0x007680a0).
- purpose: at most 20 words of plain English on what the function does. No bare addresses, no FUN_/LAB_/DAT_ names.
- anchor: one asm line copied exactly from that function's Asm block, in the form `0xADDR: TEXT`, that supports the purpose.
- Anchor only on lines that appear in that function's Asm block (the block may be truncated).

---

"""


def _fail(msg, code=1):
    sys.stderr.write("fn_doc: error: %s\n" % msg)
    return code


def _to_int(text):
    try:
        return int(str(text).strip(), 16)
    except ValueError:
        return None


def _parse_addr(text):
    """Strict form for drafter rows: 0x followed by hex digits."""
    t = str(text or "").strip()
    return int(t, 16) if _ADDR_RE.fullmatch(t) else None


def _hexa(x):
    return "0x%08x" % x


def _query(path, sql, params=()):
    p = Path(path)
    if not p.is_file():
        return []
    try:
        con = sqlite3.connect("file:%s?mode=ro" % quote(str(p)), uri=True)
    except sqlite3.Error:
        return []
    try:
        return con.execute(sql, params).fetchall()
    except sqlite3.Error:
        return []
    finally:
        con.close()


def _steam_path(args):
    return args.steam_db or os.environ.get("BD_BRANCHDB") or sf.STEAM_DB_DEFAULT


def _class_members(steam_path, cls):
    """[(addr, 'Class::fn')] for the class, sorted by address; first row wins per address."""
    out, seen = [], set()
    rows = _query(steam_path,
                  "SELECT address, function_name FROM functions WHERE class_name=? "
                  "ORDER BY address, function_name",
                  (cls,))
    for addr, fn in rows:
        try:
            a = int(addr)
        except (TypeError, ValueError):
            continue
        if a in seen or not fn:
            continue
        seen.add(a)
        out.append((a, "%s::%s" % (cls, fn)))
    return out


def _callsite_rows(ledger_dir):
    try:
        rows = rea_ledger.read_rows(ledger_dir)
    except OSError:
        return []
    out = []
    for r in rows:
        a = _parse_addr(r.get("addr"))
        if a is not None and r.get("kind") == "callsite":
            out.append((a, r.get("name") or "", r.get("confidence") or ""))
    return sorted(out)


def _purposes(kdb_path):
    out = {}
    rows = _query(kdb_path, "SELECT address, purpose FROM functions WHERE binary_hash=? "
                  "ORDER BY address, purpose", (BINARY_HASH,))
    for addr, purpose in rows:
        a = _to_int(addr)
        if a is not None and purpose:
            out[a] = purpose
    return out


def _addr_list(text):
    return [a for a in (_to_int(t) for t in str(text or "").split()) if a is not None]


def _struct_fields(kdb_path):
    out = []
    rows = _query(kdb_path,
                  "SELECT class, offset, type, name, readers, writers FROM struct_fields")
    for cls, off, ftype, fname, readers, writers in rows:
        if isinstance(off, str):
            off = _to_int(off)
        if not isinstance(off, int):
            continue
        out.append((str(cls or ""), off, str(ftype or ""), str(fname or ""),
                    _addr_list(readers), _addr_list(writers)))
    return out


def _touched_fields(ctx, a, size):
    """(offset, class, name, type, 'R'|'W'|'RW') for fields with a reader/writer inside [a, a+size)."""
    lo, hi = a, a + (size or 0)
    out = []
    for cls, off, ftype, fname, readers, writers in ctx.fields:
        r = any(lo <= x < hi for x in readers)
        w = any(lo <= x < hi for x in writers)
        if r or w:
            out.append((off, cls, fname, ftype, "RW" if (r and w) else ("R" if r else "W")))
    return sorted(out)


def _named_lines(named_root, a):
    p = Path(named_root) / ("%08x" % a)[2:4] / ("0x%08x.c" % a)
    try:
        text = p.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return []
    return text.splitlines()


class _Ctx:
    """Everything the page and draft commands read, loaded once per run."""

    def __init__(self, root, steam_path, kdb_path, named_root, ledger_dir):
        self.dump = sf.Dump(root, names=sf.load_names(steam_db=steam_path, kdb=kdb_path))
        self.named_root = Path(named_root)
        self.callsites = _callsite_rows(ledger_dir)
        self.purposes = _purposes(kdb_path)
        self.fields = _struct_fields(kdb_path)


# ---- text blocks shared by page and draft-input ----------------------------
def _nm(name):
    return " " + name if name else ""


def _size_line(f):
    size = "?" if f["size"] is None else "%d B" % f["size"]
    ret = "?" if f["ret"] is None else str(f["ret"])
    return "- Size %s, RET %s" % (size, ret)


def _callers_lines(f):
    n = f["callers_n"]
    if not n:
        return ["- Callers (0 total): none"]
    more = ", first %d shown" % len(f["callers"]) if n > len(f["callers"]) else ""
    lines = ["- Callers (%d total%s):" % (n, more)]
    for site, fn, name in f["callers"]:
        lines.append("  - %s%s (site %s)" % (_hexa(fn), _nm(name), _hexa(site)))
    return lines


def _callee_label(typ):
    return {"INDIRECT": "indirect", "IMPORT": "import"}.get(typ, typ.lower())


def _capped(lines, cap, total, label):
    """Keep the heading plus the first cap items; note the omitted count."""
    if cap is None or total <= cap:
        return lines
    return lines[:1 + cap] + ["  - ... %d more %s omitted" % (total - cap, label)]


def _callees_lines(f, cap=None):
    items = f["callees"]
    if not items:
        return ["- Callees (0): none"]
    lines = ["- Callees (%d):" % len(items)]
    for site, tgt, typ, name in items:
        if tgt is None:
            lines.append("  - %s %s" % (_hexa(site), _callee_label(typ)))
        else:
            lines.append("  - %s %s %s%s" % (_hexa(site), typ, _hexa(tgt), _nm(name)))
    return _capped(lines, cap, len(items), "callees")


def _imports_line(f):
    if not f["imports"]:
        return "- Imports: none"
    return "- Imports: " + ", ".join("%s (%s)" % (imp, lib) for imp, lib in f["imports"])


def _strings_lines(f, cap=None):
    if not f["strings"]:
        return ["- Strings: none"]
    lines = ["- Strings (%d):" % len(f["strings"])]
    lines += ["  - %s \"%s\"" % (_hexa(sa), text) for sa, text in f["strings"]]
    return _capped(lines, cap, len(f["strings"]), "strings")


def _globals_lines(f, cap=None):
    if not f["globals"]:
        return ["- Globals: none"]
    lines = ["- Globals (%d):" % len(f["globals"])]
    lines += ["  - %s %s (%s)" % (_hexa(t), rt, insn) for t, rt, insn in f["globals"]]
    return _capped(lines, cap, len(f["globals"]), "globals")


def _fields_lines(ctx, a, size, cap=None):
    rows = _touched_fields(ctx, a, size)
    if not rows:
        return ["- Struct fields touched: none"]
    lines = ["- Struct fields touched (%d):" % len(rows)]
    for off, cls, fname, ftype, rw in rows:
        full = "%s::%s" % (cls, fname) if cls else fname
        lines.append("  - 0x%x %s (%s) %s" % (off, full, ftype, rw))
    return _capped(lines, cap, len(rows), "fields")


def _purpose_line(ctx, a):
    return "- KDB purpose: %s" % (ctx.purposes.get(a) or "(none)")


def _named_block(ctx, a, cap):
    lines = _named_lines(ctx.named_root, a)
    if not lines:
        return ["- Named pseudo-C: (no named decompile)"]
    shown = lines[:cap]
    label = "- Named pseudo-C (first %d of %d lines):" % (len(shown), len(lines))
    return [label, "```c"] + shown + ["```"]


def _page_section(ctx, a, name):
    f = sf.facts(ctx.dump, a)
    lines = ["## %s %s" % (_hexa(a), name), "", _size_line(f)]
    lines += _callers_lines(f)
    lines += _callees_lines(f)
    lines.append(_imports_line(f))
    lines += _indirect_lines(ctx, f, a)
    lines += _strings_lines(f)
    lines += _globals_lines(f)
    lines += _fields_lines(ctx, a, f["size"])
    lines.append(_purpose_line(ctx, a))
    lines += _named_block(ctx, a, PAGE_NAMED_LINES)
    return "\n".join(lines) + "\n\n"


def _indirect_lines(ctx, f, a):
    lines = []
    if f["indirect"]:
        lines.append("- Indirect slots (%d):" % len(f["indirect"]))
        for site, slot, how in f["indirect"]:
            slot_s = "?" if slot is None else "+0x%x" % slot
            lines.append("  - %s slot %s (%s)" % (_hexa(site), slot_s, how))
    else:
        lines.append("- Indirect slots: none")
    size = f["size"] or 0
    hits = [(x, name) for x, name, _conf in ctx.callsites if a <= x < a + size]
    if hits:
        lines.append("- Ledger callsites in range (%d):" % len(hits))
        lines += ["  - %s %s" % (_hexa(x), name) for x, name in hits]
    else:
        lines.append("- Ledger callsites in range: none")
    return lines


def _draft_block(ctx, a, name):
    f = sf.facts(ctx.dump, a)
    lines = ["## %s %s" % (_hexa(a), name), "", _size_line(f)]
    lines += _callers_lines(f)
    lines += _callees_lines(f, DRAFT_LIST_CAP)
    lines += _strings_lines(f, DRAFT_LIST_CAP)
    lines += _globals_lines(f, DRAFT_LIST_CAP)
    lines += _fields_lines(ctx, a, f["size"], DRAFT_LIST_CAP)
    lines.append(_purpose_line(ctx, a))
    asm = ctx.dump._asm_lines(a)
    if not asm:
        lines.append("- Asm: (no asm in dump; this function cannot be anchored)")
    else:
        shown = asm[:DRAFT_ASM_LINES]
        more = "; first %d shown" % len(shown) if len(asm) > len(shown) else ""
        lines.append("- Asm (%d lines%s):" % (len(asm), more))
        lines.append("```asm")
        lines += ["0x%08x: %s" % (addr, text) for addr, _b, text in shown]
        lines.append("```")
    lines += _named_block(ctx, a, DRAFT_NAMED_LINES)
    return "\n".join(lines) + "\n\n"


# ---- writing ---------------------------------------------------------------
def _write_text(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text.rstrip("\n") + "\n")


def _prune_numbered(out_dir, pattern, keep):
    """Delete files in out_dir whose 1-based index (first group of pattern) exceeds keep."""
    pat = re.compile(pattern)
    for p in out_dir.iterdir():
        m = pat.fullmatch(p.name)
        if m and int(m.group(1)) > keep:
            p.unlink()


def _page_header(cls, n, part=None):
    line = "%d functions in the Steam DB for %s." % (n, cls)
    if part:
        line += " Part %d of %d." % part
    return "# %s\n\n%s\n\n" % (cls, line)


def _group(sections, overhead, limit):
    groups, cur, size = [], [], overhead
    for sec in sections:
        n = len(sec.encode("utf-8"))
        if cur and size + n > limit:
            groups.append(cur)
            cur, size = [], overhead
        cur.append(sec)
        size += n
    if cur:
        groups.append(cur)
    return groups


def _write_page(out_dir, cls, n, sections, split_bytes):
    whole_text = _page_header(cls, n) + "".join(sections)
    whole = out_dir / (cls + ".md")
    numbered = r"%s\.(\d+)\.md" % re.escape(cls)
    if len(whole_text.encode("utf-8")) <= split_bytes:
        _write_text(whole, whole_text)
        _prune_numbered(out_dir, numbered, 0)
        return [whole]
    overhead = len(_page_header(cls, n, (99, 99)).encode("utf-8"))
    groups = _group(sections, overhead, split_bytes)
    paths = []
    for k, group in enumerate(groups, 1):
        p = out_dir / ("%s.%d.md" % (cls, k))
        _write_text(p, _page_header(cls, n, (k, len(groups))) + "".join(group))
        paths.append(p)
    if whole.exists():
        whole.unlink()
    _prune_numbered(out_dir, numbered, len(groups))
    return paths


_DRAFT_ITEM_RE = r"%s_(\d+)\.md"


def _write_drafts(ctx, cls, members, n, out_dir):
    work = out_dir / "_work"
    work.mkdir(parents=True, exist_ok=True)
    chunks = [members[i:i + n] for i in range(0, len(members), n)]
    paths = []
    for k, chunk in enumerate(chunks, 1):
        lo = (k - 1) * n + 1
        hi = lo + len(chunk) - 1
        head = _DRAFT_HELP.format(cls=cls, k=k, K=len(chunks), lo=lo, hi=hi, total=len(members))
        body = head + "".join(_draft_block(ctx, a, name) for a, name in chunk)
        p = work / ("%s_%d.md" % (cls, k))
        _write_text(p, body)
        paths.append(p)
    _prune_numbered(work, _DRAFT_ITEM_RE % re.escape(cls), len(chunks))
    return paths


# ---- check-drafts ----------------------------------------------------------
def _norm_text(s):
    """Lowercase, drop 'dword ptr'-style width words, int-normalise 0x tokens, collapse spaces."""
    t = _PTR_RE.sub("", s.lower())
    t = _HEX_NORM_RE.sub(lambda m: "0x%x" % int(m.group(0), 16), t)
    return " ".join(t.split())


def _alpha_words(text):
    t = _FN_TOKEN_RE.sub(" ", text)
    t = _HEX_TOKEN_RE.sub(" ", t)
    return len(re.findall(r"[A-Za-z]+", t))


def _check_row(dump, cells, seen=None):
    """-> (addr, 'pass'|'fail', reason) for one drafter row (first failing rule wins)."""
    if len(cells) != 3:
        first = cells[0].strip() if cells else ""
        return (first or "-", "fail",
                "bad row shape: expected 3 tab-separated cells, got %d" % len(cells))
    addr_s, purpose, anchor = (c.strip() for c in cells)
    a = _parse_addr(addr_s)
    if a is None:
        return (addr_s or "-", "fail", "bad row shape: addr is not 0x<hex>")
    shown = _hexa(a)
    if seen is not None and a in seen:
        return (shown, "fail", "duplicate addr (an earlier row for it already passed)")
    asm = dump._asm_lines(a)
    if not asm:
        reason = "no asm: decomp file missing" if dump.size_ret(a) is not None \
            else "unknown addr: not in dump"
        return (shown, "fail", reason)
    if not purpose:
        return (shown, "fail", "purpose empty")
    words = len(purpose.split())
    if words > PURPOSE_MAX_WORDS:
        return (shown, "fail", "purpose has %d words (max %d)" % (words, PURPOSE_MAX_WORDS))
    if _alpha_words(purpose) < 3:
        return (shown, "fail", "purpose is address-only (fewer than 3 alphabetic words)")
    m = _ANCHOR_RE.fullmatch(anchor)
    if not m:
        return (shown, "fail", "anchor not of form 0xADDR: TEXT")
    ea = _parse_addr(m.group(1))
    by_addr = {addr: text for addr, _b, text in asm}
    if ea not in by_addr:
        return (shown, "fail", "anchor addr %s not in function asm" % m.group(1))
    if _norm_text(m.group(2)) != _norm_text(by_addr[ea]):
        return (shown, "fail", "anchor text mismatch at %s" % _hexa(ea))
    if seen is not None:
        seen.add(a)
    return (shown, "pass", "ok")


# ---- commands --------------------------------------------------------------
def _cmd_page(args):
    if not _CLASS_RE.fullmatch(args.cls):
        return _fail("bad class name: %r" % args.cls)
    if args.split_bytes < 1:
        return _fail("--split-bytes must be >= 1")
    steam = _steam_path(args)
    members = _class_members(steam, args.cls)
    if not members:
        return _fail("no functions for class %s in Steam DB %s" % (args.cls, steam))
    ctx = _Ctx(args.root, steam, args.kdb or sf.KDB_DEFAULT, args.named_root, args.ledger_dir)
    sections = [_page_section(ctx, a, name) for a, name in members]
    for p in _write_page(Path(args.out).resolve(), args.cls, len(members), sections,
                         args.split_bytes):
        print(p)
    return 0


def _cmd_draft(args):
    if not _CLASS_RE.fullmatch(args.cls):
        return _fail("bad class name: %r" % args.cls)
    if args.n < 1:
        return _fail("--n must be >= 1")
    steam = _steam_path(args)
    members = _class_members(steam, args.cls)
    if not members:
        return _fail("no functions for class %s in Steam DB %s" % (args.cls, steam))
    ctx = _Ctx(args.root, steam, args.kdb or sf.KDB_DEFAULT, args.named_root, args.ledger_dir)
    for p in _write_drafts(ctx, args.cls, members, args.n, Path(args.out).resolve()):
        print(p)
    return 0


def _cmd_check(args):
    try:
        with open(args.tsv, encoding="utf-8", errors="replace") as fh:
            lines = fh.read().split("\n")
    except OSError as e:
        return _fail("cannot read %s: %s" % (args.tsv, e), 2)
    dump = sf.Dump(args.root)
    results, seen = [], set()
    for line in lines:
        line = line.rstrip("\r")
        if not line.strip() or line.split("\t")[0].strip().lower() == "addr":
            continue
        results.append(_check_row(dump, line.split("\t"), seen))
    text = "".join("%s\t%s\t%s\n" % r for r in results)
    if args.out:
        out = Path(args.out)
        out.parent.mkdir(parents=True, exist_ok=True)
        with open(out, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
    else:
        sys.stdout.write(text)
    npass = sum(1 for r in results if r[1] == "pass")
    sys.stderr.write("check-drafts: %d rows, %d pass, %d fail\n"
                     % (len(results), npass, len(results) - npass))
    return 0


def _cli(argv=None):
    ap = argparse.ArgumentParser(
        description="Per-class function pages and drafting input from the Steam static dump.")
    sub = ap.add_subparsers(dest="cmd", required=True)

    def add_paths(p):
        p.add_argument("--root", default=str(sf.DUMP_ROOT), help="static dump root")
        p.add_argument("--steam-db", default=None,
                       help="Steam address DB (default: $BD_BRANCHDB, then the kpm-pr-steam-db copy)")
        p.add_argument("--kdb", default=None, help="knowledge DB (default: static_facts KDB_DEFAULT)")
        p.add_argument("--named-root", default=str(NAMED_ROOT_DEFAULT),
                       help="named pseudo-C root (<bb>/0x<addr>.c)")
        p.add_argument("--ledger-dir", default=str(rea_ledger.DEFAULT_DIR),
                       help="REA findings ledger directory")

    p_page = sub.add_parser("page", help="write <Class>.md (or <Class>.<n>.md parts)")
    p_page.add_argument("cls", metavar="Class")
    p_page.add_argument("--out", default=str(STEAM_DOCS_DEFAULT))
    p_page.add_argument("--split-bytes", type=int, default=SPLIT_BYTES)
    add_paths(p_page)

    p_draft = sub.add_parser("draft-input", help="write drafter chunks to <out>/_work/")
    p_draft.add_argument("cls", metavar="Class")
    p_draft.add_argument("--n", type=int, default=25, help="functions per chunk")
    p_draft.add_argument("--out", default=str(STEAM_DOCS_DEFAULT))
    add_paths(p_draft)

    p_chk = sub.add_parser("check-drafts", help="validate drafter TSV rows against the dump asm")
    p_chk.add_argument("tsv")
    p_chk.add_argument("--out", default=None, help="write results here (default: stdout)")
    p_chk.add_argument("--root", default=str(sf.DUMP_ROOT), help="static dump root")

    args = ap.parse_args(argv)
    if args.cmd == "page":
        return _cmd_page(args)
    if args.cmd == "draft-input":
        return _cmd_draft(args)
    return _cmd_check(args)


if __name__ == "__main__":
    sys.exit(_cli())
