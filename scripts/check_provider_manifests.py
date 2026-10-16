#!/usr/bin/env python3
"""Static check that provider/consumer manifests agree with the provide()/require() calls in their sources.

  python3 scripts/check_provider_manifests.py
  python3 scripts/check_provider_manifests.py --quiet
  python3 scripts/check_provider_manifests.py --root DIR [--shared DIR] [--core-header kpatch_api.h]

Scans <root>/*_src/manifest.toml (root default <repo>/patch_manager_mods; --shared default <root>/_shared_src; --core-header
default $KPM_DIR/Patches/Common/kpatch_api.h). Per mod the source set is its own *.cpp/*.h/*.inc (not *_test.cpp, not
testdata/ retired/ ...) plus the _shared_src headers reached transitively through #include "X.h". `#define NAME "str"` and
`#define NAME <int>` from that set and from the core header resolve the arguments of
  api->provide(NAME_OR_"literal", VERSION_OR_int, ...)    api->require(NAME_OR_"literal", VERSION_OR_int)
Comments are stripped first. Declarations of the functions themselves (typed parameters) are skipped. The core interface
kpatch.log is provided by the patcher and never needs a manifest entry.
  FAIL  provides / requires_interfaces entry that is not `name@<int>`; kind = "provider" with an empty provides
  FAIL  a `provides` entry with no matching provide() in the sources; a resolved provide() not listed in `provides`
  FAIL  a `requires_interfaces` entry with no matching require() in the sources
  FAIL  a consumer whose `requires` lacks the id of the manifest providing the interface; no manifest provides it
  FAIL  one hook address in the same hooks.toml file name of a provider and of one of its consumers
  WARN  a resolved require() of a non-core interface not listed in requires_interfaces; an argument that cannot be
        resolved (a core macro when the core header is missing, a variable, ...)
  PASS  one line per provider and per consumer without a FAIL
Exit 1 on any FAIL. Read-only.
"""
import argparse
import os
import re
import sys
import tomllib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _static_common as sc  # noqa: E402

PROVIDER_KIND = "provider"
CORE_IFACES = {("kpatch.log", 1)}
CORE_HEADER_REL = Path("Patches") / "Common" / "kpatch_api.h"
SRC_EXT = {".cpp", ".h", ".inc"}
ENTRY_RE = re.compile(r"^([A-Za-z0-9_.\-]+)@(\d+)$")
DEFINE_RE = re.compile(r"^[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)[ \t]+(.+?)[ \t]*$", re.M)
INCLUDE_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"([^"]+)"', re.M)
CALL_RE = re.compile(r"\b(provide|require)\s*\(")
STR_RE = re.compile(r'"((?:[^"\\]|\\.)*)"')
STRS_RE = re.compile(r'\s*(?:"(?:[^"\\]|\\.)*"\s*)+')
INT_RE = re.compile(r"^\(?\s*(?:\([A-Za-z_][\w\s]*\)\s*)?(0[xX][0-9a-fA-F]+|\d+)[uUlL]*\s*\)?$")
IDENT_RE = re.compile(r"^[A-Za-z_]\w*$")


def default_core_header():
    return sc.KPM_DIR / CORE_HEADER_REL


def strip_comments(text):
    """Blank out // and /* */ comments (newlines kept, so line numbers stay valid); string literals are respected."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c == '"' or c == "'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join("\n" if ch == "\n" else " " for ch in text[i:j]))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def line_of(text, pos):
    return text.count("\n", 0, pos) + 1


def parse_int(s):
    m = INT_RE.match(s.strip())
    return int(m.group(1), 0) if m else None


def parse_str(s):
    """Concatenated string literal value, or None if `s` is not purely string literal(s)."""
    s = s.strip()
    if s and STRS_RE.fullmatch(s):
        return "".join(m.group(1) for m in STR_RE.finditer(s))
    return None


def collect_defines(text, table):
    """Add `#define NAME "str"` / `#define NAME <int>` / `#define NAME OTHER` (alias, resolved later) to table."""
    for m in DEFINE_RE.finditer(text):
        name, val = m.group(1), m.group(2).strip()
        s, i = parse_str(val), parse_int(val)
        if s is not None:
            table[name] = s
        elif i is not None:
            table[name] = i
        elif IDENT_RE.match(val):
            table[name] = ("alias", val)


def resolve(table, name, depth=0):
    v = table.get(name)
    if isinstance(v, tuple) and depth < 8:
        return resolve(table, v[1], depth + 1)
    return None if isinstance(v, tuple) else v


def split_args(text, open_pos):
    """Top-level comma-separated args of the call whose '(' is at open_pos, or None if it never closes."""
    depth, args, cur, i, n = 0, [], [], open_pos, len(text)
    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                j += 2 if text[j] == "\\" else 1
            cur.append(text[i:j + 1])
            i = j + 1
            continue
        if c in "([{":
            depth += 1
            if depth > 1:
                cur.append(c)
        elif c in ")]}":
            depth -= 1
            if depth == 0:
                args.append("".join(cur).strip())
                return args
            cur.append(c)
        elif c == "," and depth == 1:
            args.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
        i += 1
    return None


def looks_like_param(arg):
    """`const char* name`, `uint32_t`, `const void*` ...: a parameter declaration, not an expression."""
    bare = STR_RE.sub('""', arg)
    return bool(re.search(r"\w\s+[\w*&]|[*&]\s*$|\w\s*[*&]\s*\w", bare)) and parse_str(arg) is None


def mod_files(moddir):
    out = []
    for dirpath, dirnames, filenames in os.walk(moddir):
        dirnames[:] = sorted(d for d in dirnames if d not in sc.SKIP_DIRS)
        for f in sorted(filenames):
            if Path(f).suffix.lower() in SRC_EXT and not f.endswith("_test.cpp"):
                out.append(Path(dirpath) / f)
    return out


def shared_index(shared):
    idx = {}
    if shared and Path(shared).is_dir():
        for p in sorted(Path(shared).rglob("*")):
            if p.is_file() and p.suffix.lower() in SRC_EXT:
                idx.setdefault(p.name, p)
    return idx


def source_set(moddir, shared_idx):
    """[Path]: the mod's own sources plus the shared headers reached transitively through #include "..."."""
    files = mod_files(moddir)
    seen = set(files)
    queue = list(files)
    while queue:
        p = queue.pop()
        text = strip_comments(p.read_text(errors="replace"))
        for m in INCLUDE_RE.finditer(text):
            name = m.group(1)
            cand = p.parent / name
            if cand.is_file():
                tgt = cand.resolve()
                tgt = next((q for q in shared_idx.values() if q.resolve() == tgt), None)   # local include: only shared files join
            else:
                tgt = shared_idx.get(Path(name).name)      # -I style include ("PortraitSlots.h", "../_shared/X.h")
            if tgt is not None and tgt not in seen:
                seen.add(tgt)
                files.append(tgt)
                queue.append(tgt)
    return files


def scan_calls(files, table, rel):
    """[(kind, name|None, version|None, where, note)] for every provide()/require() call in `files`.

    name/version are None when unresolvable (note says why); declarations with typed parameters are skipped.
    """
    calls = []
    for p in files:
        text = strip_comments(p.read_text(errors="replace"))
        for m in CALL_RE.finditer(text):
            args = split_args(text, m.end() - 1)
            if not args or len(args) < 2:
                continue
            a1, a2 = args[0], args[1]
            if looks_like_param(a1) or looks_like_param(a2):
                continue                     # fn-pointer typedef / member declaration / definition
            where = f"{rel(p)}:{line_of(text, m.start())}"
            name, why = parse_str(a1), ""
            if name is None and IDENT_RE.match(a1):
                name = resolve(table, a1)
                if not isinstance(name, str):
                    name, why = None, f"{a1} is not a resolvable string macro"
            elif name is None:
                why = f"first argument `{a1}` is not a literal or macro"
            ver = parse_int(a2)
            if ver is None and IDENT_RE.match(a2):
                v = resolve(table, a2)
                ver = v if isinstance(v, int) else None
            if ver is None and not why:
                why = f"version `{a2}` is not an integer or a resolvable macro"
            if why:
                calls.append((m.group(1), None, None, where, why))
            else:
                calls.append((m.group(1), name, ver, where, ""))
    return calls


def parse_entry(s):
    m = ENTRY_RE.match(s) if isinstance(s, str) else None
    return (m.group(1), int(m.group(2))) if m else None


def hook_addresses(path):
    try:
        doc = tomllib.loads(Path(path).read_text(encoding="utf-8"))
    except (tomllib.TOMLDecodeError, OSError, UnicodeDecodeError):
        return set()
    out = set()
    for h in doc.get("hooks", []):
        try:
            out.add(sc._to_int(h["address"]))
        except (KeyError, ValueError):
            pass
    return out


class Mod:
    def __init__(self, d, doc):
        pt = doc.get("patch", {})
        self.dir, self.id = d, pt.get("id") or d.name
        self.kind = pt.get("kind", "patch")
        self.provides_raw = list(pt.get("provides", []))
        self.requires = list(pt.get("requires", []))
        self.req_ifaces_raw = list(pt.get("requires_interfaces", []))
        self.fails = 0
        self.provides, self.req_ifaces = {}, {}   # {(name, ver): raw}


def run(root, shared=None, core_header=None, reporter=None):
    root = Path(root)
    rep = reporter or sc.Reporter()
    shared = Path(shared) if shared else root / "_shared_src"
    core_header = Path(core_header) if core_header else default_core_header()
    sidx = shared_index(shared)

    core_table = {}
    core_ok = core_header.is_file()
    if core_ok:
        collect_defines(strip_comments(core_header.read_text(errors="replace")), core_table)
    else:
        rep.warn("manifest", f"core header {core_header} not found: core macros (KPATCH_LOG_IFACE ...) stay unresolved")

    def rel(p):
        try:
            return str(Path(p).resolve().relative_to(root.resolve().parent))
        except ValueError:
            return str(p)

    mods = []
    for mp in sorted(root.glob("*_src/manifest.toml")):
        try:
            mods.append(Mod(mp.parent, tomllib.loads(mp.read_text(encoding="utf-8"))))
        except (tomllib.TOMLDecodeError, UnicodeDecodeError) as e:
            rep.fail("manifest", f"{rel(mp)}: unreadable manifest ({e})")

    def fail(mod, check, msg):
        mod.fails += 1
        rep.fail(check, f"{mod.id}: {msg}")

    # ---- manifest entries
    for m in mods:
        for field, raw, dest in (("provides", m.provides_raw, m.provides), ("requires_interfaces", m.req_ifaces_raw, m.req_ifaces)):
            for s in raw:
                e = parse_entry(s)
                if e is None:
                    fail(m, "manifest", f"malformed {field} entry {s!r} (expected name@<int>, e.g. k2.portraitslots@1)")
                else:
                    dest[e] = s
        if m.kind not in ("patch", PROVIDER_KIND):
            fail(m, "manifest", f'kind = "{m.kind}" is not "patch" or "provider"')
        if m.kind == PROVIDER_KIND and not m.provides_raw:
            fail(m, "manifest", 'kind = "provider" with an empty provides')

    provider_of = {}                         # (name, ver) -> Mod
    for m in mods:
        for e in m.provides:
            provider_of.setdefault(e, m)

    # ---- source calls vs manifest
    for m in mods:
        files = source_set(m.dir, sidx)
        table = dict(core_table)
        for p in files:
            collect_defines(strip_comments(p.read_text(errors="replace")), table)
        calls = scan_calls(files, table, rel)
        seen_prov, seen_req = {}, {}
        for kind, name, ver, where, why in calls:
            if name is None:
                rep.warn("source", f"{m.id}: {where} {kind}() skipped: {why}"
                         + ("" if core_ok or "KPATCH_" not in why else " (core header missing)"))
                continue
            key = (name, ver)
            if kind == "provide":
                seen_prov.setdefault(key, where)
            elif key not in CORE_IFACES:
                seen_req.setdefault(key, where)
        for key, where in seen_prov.items():
            if key not in m.provides:
                fail(m, "provides", f"{where} provide({key[0]!r}, {key[1]}) is not listed in provides (add \"{key[0]}@{key[1]}\")")
        for key in m.provides:
            if key not in seen_prov:
                fail(m, "provides", f"provides \"{m.provides[key]}\" but no provide({key[0]!r}, {key[1]}) call in the sources")
        for key in m.req_ifaces:
            if key not in seen_req:
                fail(m, "requires", f"requires_interfaces \"{m.req_ifaces[key]}\" but no require({key[0]!r}, {key[1]}) call in the sources")
        for key, where in seen_req.items():
            if key not in m.req_ifaces and key not in m.provides:
                rep.warn("requires", f"{m.id}: {where} require({key[0]!r}, {key[1]}) is not listed in requires_interfaces")

    # ---- consumer wiring
    consumers = {}                           # provider id -> [consumer Mod]
    for m in mods:
        for key, raw in m.req_ifaces.items():
            if key in CORE_IFACES:
                continue
            prov = provider_of.get(key)
            if prov is None:
                fail(m, "requires", f"requires_interfaces \"{raw}\" but no manifest provides it")
            elif prov is not m:
                if prov.id not in m.requires:
                    fail(m, "requires", f"requires_interfaces \"{raw}\" is provided by {prov.id} but requires = {m.requires} lacks it")
                lst = consumers.setdefault(prov.id, [])
                if m not in lst:
                    lst.append(m)
        for rid in m.requires:               # a plain `requires` on a provider also makes this mod its consumer
            lib = next((x for x in mods if x.id == rid and x.kind == PROVIDER_KIND), None)
            if lib is not None and lib is not m:
                lst = consumers.setdefault(lib.id, [])
                if m not in lst:
                    lst.append(m)

    # ---- duplicate hook addresses between a provider and its consumers
    for lib in mods:
        for c in consumers.get(lib.id, []):
            for hp in sorted(lib.dir.glob("*.hooks.toml")):
                cp = c.dir / hp.name
                if not cp.is_file():
                    continue
                dup = hook_addresses(hp) & hook_addresses(cp)
                for a in sorted(dup):
                    fail(c, "hooks", f"hook 0x{a:08X} is in both {lib.id} and {c.id} ({hp.name}); the provider owns it")
                    lib.fails += 1

    # ---- PASS lines
    for m in mods:
        if m.kind == PROVIDER_KIND and m.fails == 0 and m.provides:
            cs = ", ".join(c.id for c in consumers.get(m.id, [])) or "none"
            rep.passed("provider", f"{m.id} provides {', '.join(m.provides.values())} (consumers: {cs})")
    for m in mods:
        if m.kind != PROVIDER_KIND and m.req_ifaces and m.fails == 0:
            rep.passed("consumer", f"{m.id} requires {', '.join(m.req_ifaces.values())} and lists {', '.join(m.requires)}")
    return rep


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--root", default=str(sc.REPO / "patch_manager_mods"), help="dir holding the *_src mods (default: <repo>/patch_manager_mods)")
    ap.add_argument("--shared", help="shared headers dir (default <root>/_shared_src)")
    ap.add_argument("--core-header", help="kpatch_api.h (default $KPM_DIR/Patches/Common/kpatch_api.h)")
    ap.add_argument("--quiet", action="store_true", help="hide PASS and INFO lines")
    a = ap.parse_args(argv)
    rep = sc.Reporter(quiet=a.quiet)
    run(a.root, a.shared, a.core_header, rep)
    return rep.finish("check_provider_manifests")


if __name__ == "__main__":
    sys.exit(main())
