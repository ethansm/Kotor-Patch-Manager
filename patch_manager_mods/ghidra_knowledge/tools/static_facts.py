#!/usr/bin/env python3
"""Per-address static facts from the Ghidra static dump (read-only, stdlib only).

Reuses the asm parser and vtable-slot resolver from
patch_manager_mods/engine_ai_atlas/tools/atlas_common.py.  Names are attached only
to callers/callees; never to the queried address itself or to exclude_addrs.

CLI:
  static_facts.py addr <hex> [--json] [--root DIR]
  static_facts.py twins <hex> [--root DIR]
"""
from __future__ import annotations

import argparse
import bisect
import hashlib
import importlib.util
import json
import os
import re
import sqlite3
import sys
from collections import defaultdict
from pathlib import Path
from urllib.parse import quote

REPO = Path(__file__).resolve().parents[3]
DUMP_ROOT = Path.home() / "Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71"
STEAM_DB_DEFAULT = (Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db"
                    / "AddressDatabases/kotor2_steam_aspyr.db")
KDB_DEFAULT = REPO / "patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db"
_ATLAS_PATH = REPO / "patch_manager_mods/engine_ai_atlas/tools/atlas_common.py"


def _load_atlas():
    spec = importlib.util.spec_from_file_location("_atlas_common_static_facts", _ATLAS_PATH)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


_AC = _load_atlas()

_HEX_RE = re.compile(r"0x[0-9a-fA-F]+")
_DIRECT_RE = re.compile(r"^0x[0-9a-fA-F]+$")


def _hex(s):
    try:
        return int(s.strip(), 16)
    except (ValueError, AttributeError):
        return None


def _addr_val(x):
    if isinstance(x, int):
        return x
    if isinstance(x, str):
        return _hex(x)
    return None


def _big_to_hash(m):
    return "#" if int(m.group(0), 16) >= 0x10000 else m.group(0)


def _norm_insn(insn):
    """'CALL 0x0085d260' -> 'CALL #'; big hex immediates/addresses -> '#'; small displacements kept."""
    mnem, ops = _AC.split_insn(insn)
    if mnem in ("CALL", "JMP") and len(ops) == 1 and _DIRECT_RE.match(ops[0]):
        ops = ["#"]
    else:
        ops = [_HEX_RE.sub(_big_to_hash, o) for o in ops]
    return " ".join([mnem, ",".join(ops)]) if ops else mnem


def _callee_key(t):
    return (t[0], -1 if t[1] is None else t[1], t[2])


class Dump:
    """Lazy, read-only view of one Ghidra static dump directory."""

    def __init__(self, root=DUMP_ROOT, names=None):
        self.root = Path(root)
        self.names = dict(names) if names else {}
        self._cache = {}
        self._asm = {}

    # ---- TSV loading -------------------------------------------------------
    def _rows(self, rel):
        try:
            f = open(self.root / rel, encoding="utf-8", errors="replace")
        except OSError:
            return
        with f:
            next(f, None)  # header
            for line in f:
                line = line.rstrip("\r\n")
                if line:
                    yield line.split("\t")

    def _graph(self):
        if "graph" not in self._cache:
            callers = defaultdict(list)
            callees = defaultdict(list)
            indirect = defaultdict(list)
            for r in self._rows("callgraph.tsv"):
                if len(r) < 4:
                    continue
                caller, site, typ = _hex(r[0]), _hex(r[1]), r[3]
                if caller is None or site is None:
                    continue
                tgt = _hex(r[2])
                if typ in ("CALL", "TAILJMP") and tgt is not None:
                    callers[tgt].append((site, caller))
                if typ == "INDIRECT":
                    tgt = None
                    indirect[caller].append((site, r[4] if len(r) > 4 else ""))
                callees[caller].append((site, tgt, typ))
            self._cache["graph"] = (callers, callees, indirect)
        return self._cache["graph"]

    def _strings_idx(self):
        if "str" not in self._cache:
            idx = defaultdict(set)
            for r in self._rows("strings.tsv"):
                if len(r) < 2:
                    continue
                sa = _hex(r[0])
                if sa is None:
                    continue
                fns = r[3] if len(r) > 3 else ""
                for item in fns.split(","):
                    fa = _hex(item.split(":", 1)[0])
                    if fa is not None:
                        idx[fa].add((sa, r[1]))
            self._cache["str"] = idx
        return self._cache["str"]

    def _globals_idx(self):
        if "glob" not in self._cache:
            idx = defaultdict(set)
            for r in self._rows("globals.tsv"):
                if len(r) < 5:
                    continue
                ta, fa = _hex(r[1]), _hex(r[4])
                if ta is None or fa is None:
                    continue
                idx[fa].add((ta, r[3], r[6] if len(r) > 6 else ""))
            self._cache["glob"] = idx
        return self._cache["glob"]

    def _imports_idx(self):
        if "imp" not in self._cache:
            idx = defaultdict(set)
            for r in self._rows("imports.tsv"):
                if len(r) < 3:
                    continue
                fa = _hex(r[2])
                if fa is not None:
                    idx[fa].add((r[0], r[1]))
            self._cache["imp"] = idx
        return self._cache["imp"]

    def _funcs_idx(self):
        if "funcs" not in self._cache:
            sizes = {}
            for r in self._rows("inputs/steam_functions.tsv"):
                if len(r) < 3:
                    continue
                fa = _hex(r[0])
                try:
                    sizes[fa] = (int(r[1]), int(r[2]))
                except ValueError:
                    continue
            self._cache["funcs"] = (sizes, sorted(sizes))
        return self._cache["funcs"]

    def _asm_lines(self, a):
        if a not in self._asm:
            self._asm[a] = _AC.read_asm(self.root, a)
        return self._asm[a]

    # ---- queries -----------------------------------------------------------
    def callers(self, a):
        """(call_site, caller_fn) for CALL/TAILJMP edges into a, sorted."""
        return sorted(set(self._graph()[0].get(a, [])))

    def callees(self, a):
        """(site, target|None, type) for edges out of a, sorted by site. INDIRECT target is None."""
        return sorted(set(self._graph()[1].get(a, [])), key=_callee_key)

    def indirect_slots(self, a):
        """(site, slot|None, how) for each INDIRECT call site in a, resolved on a's asm."""
        sites = sorted(set(self._graph()[2].get(a, [])))
        if not sites:
            return []
        lines = self._asm_lines(a)
        if lines is None:
            return [(site, None, "no asm") for site, _insn in sites]
        index = {addr: i for i, (addr, _b, _t) in enumerate(lines)}
        out = []
        for site, insn in sites:
            slot, how = _AC.resolve_indirect(lines, site, insn, index)
            out.append((site, slot, how))
        return out

    def strings(self, a):
        """(string_addr, string) for strings referenced from a, sorted."""
        return sorted(self._strings_idx().get(a, ()))

    def globals_(self, a):
        """(target, reftype, insn) for global accesses made by a, sorted."""
        return sorted(self._globals_idx().get(a, ()))

    def imports(self, a):
        """(import, library) for imports called from a, sorted."""
        return sorted(self._imports_idx().get(a, ()))

    def size_ret(self, a):
        """(size, retn) from steam_functions.tsv, or None."""
        return self._funcs_idx()[0].get(a)

    def asm(self, a):
        """Instruction texts of a in order, or None when the decomp file is missing."""
        lines = self._asm_lines(a)
        return None if lines is None else [t for _addr, _b, t in lines]

    def _rel_texts(self, a):
        """[(addr, insn)] of a with direct branch targets inside a's body rewritten to 'L+<offset>'.

        Absolute local jump targets differ between byte-identical functions at different
        addresses; offsets from the function start do not.  None when the asm is missing.
        """
        lines = self._asm_lines(a)
        if not lines:
            return None
        sr = self.size_ret(a)
        end = a + sr[0] if sr is not None else lines[-1][0] + 1
        out = []
        for addr, _b, t in lines:
            mnem, ops = _AC.split_insn(t)
            if len(ops) == 1 and _DIRECT_RE.match(ops[0]) and (
                    mnem.startswith("J") or mnem.startswith("LOOP") or mnem == "CALL"):
                tgt = int(ops[0], 16)
                if a <= tgt < end:
                    t = "%s L+%d" % (mnem, tgt - a)
            out.append((addr, t))
        return out

    def shape_hash(self, a):
        rel = self._rel_texts(a)
        if not rel:
            return None
        text = "\n".join(_norm_insn(t) for _addr, t in rel)
        return hashlib.sha1(text.encode("utf-8")).hexdigest()

    def twins(self, a, window=0x400):
        """Other functions within +-window with the same size, ret and shape_hash; sorted."""
        sr = self.size_ret(a)
        if sr is None:
            return []
        h = self.shape_hash(a)
        if h is None:
            return []
        sizes, addrs = self._funcs_idx()
        lo = bisect.bisect_left(addrs, a - window)
        hi = bisect.bisect_right(addrs, a + window)
        out = []
        for c in addrs[lo:hi]:
            if c == a or sizes[c] != sr:
                continue
            if self.shape_hash(c) == h:
                out.append(c)
        return out

    def distinguishing(self, a, twins):
        """Per twin: '0x%08x: <insn>' of a's first line differing from the twin's ('' if identical).

        Local branch targets are compared relative to each function's start, so only real
        differences (call targets, string/global immediates, ...) are reported.
        """
        la = self._rel_texts(a) or []
        raw = {addr: t for addr, _b, t in (self._asm_lines(a) or [])}
        out = {}
        for t in twins:
            lt = self._rel_texts(t) or []
            val = ""
            for (addr_a, ins_a), (_addr_t, ins_t) in zip(la, lt):
                if ins_a != ins_t:
                    val = "0x%08x: %s" % (addr_a, raw[addr_a])
                    break
            out[t] = val
        return out


# ---- names -----------------------------------------------------------------
def _ro_rows(path, sql):
    p = Path(path)
    if not p.is_file():
        return []
    try:
        con = sqlite3.connect("file:%s?mode=ro" % quote(str(p)), uri=True)
    except sqlite3.Error:
        return []
    try:
        return con.execute(sql).fetchall()
    except sqlite3.Error:
        return []
    finally:
        con.close()


def load_names(steam_db=None, kdb=None, exclude_sources=()):
    """address -> name. KDB 'high' rows first (minus excluded sources); Steam DB overrides on conflict."""
    names = {}
    prefixes = tuple(exclude_sources)
    kdb_path = kdb or KDB_DEFAULT
    for addr, name, source in _ro_rows(
            kdb_path, "SELECT steam_addr, name, source FROM name_map WHERE confidence = 'high'"):
        if not name:
            continue
        if prefixes and (source or "").startswith(prefixes):
            continue
        a = _addr_val(addr)
        if a is not None and a not in names:
            names[a] = name
    steam_path = steam_db or os.environ.get("BD_BRANCHDB") or STEAM_DB_DEFAULT
    for cls, fn, addr in _ro_rows(
            steam_path, "SELECT class_name, function_name, address FROM functions"):
        a = _addr_val(addr)
        if a is None or not fn:
            continue
        names[a] = "%s::%s" % (cls, fn) if cls else fn
    return names


# ---- facts / card ----------------------------------------------------------
def facts(dump, a, *, exclude_addrs=()):
    hidden = set(exclude_addrs)
    hidden.add(a)

    def nm(x):
        return None if (x is None or x in hidden) else dump.names.get(x)

    sr = dump.size_ret(a)
    callers = dump.callers(a)
    first_site = {}
    for site, fn in callers:
        first_site.setdefault(fn, site)  # callers is sorted by site: first = lowest site
    distinct = sorted((site, fn) for fn, site in first_site.items())
    tw = dump.twins(a)
    return {
        "addr": a,
        "size": sr[0] if sr is not None else None,
        "ret": sr[1] if sr is not None else None,
        "callers_n": len(first_site),
        "callers": [(site, fn, nm(fn)) for site, fn in distinct[:5]],  # distinct fns
        "callees": [(site, tgt, typ, nm(tgt)) for site, tgt, typ in dump.callees(a)],
        "indirect": dump.indirect_slots(a),
        "strings": dump.strings(a),
        "globals": dump.globals_(a),
        "imports": dump.imports(a),
        "twins": tw,
        "distinguishing": dump.distinguishing(a, tw),
        "asm_present": dump.asm(a) is not None,
    }


def _fmt_addr(x, name=None):
    return "0x%08x" % x + (" " + name if name else "")


def _join(items, n, more=False):
    shown = items[:n]
    text = ", ".join(shown)
    if more or len(items) > n:
        text += ", ..."
    return text


def card_lines(f, max_lines=10):
    lines = []
    if f.get("size") is not None:
        lines.append("- size %d B, RET %d" % (f["size"], f["ret"]))
    else:
        lines.append("- size unknown (not in steam_functions)")
    if not f.get("asm_present"):
        lines.append("- asm: no decomp file in dump")

    if f.get("callers_n"):
        seen, names = [], {}
        for _site, fn, name in f["callers"]:
            if fn not in names:
                names[fn] = name
                seen.append(fn)
        items = [_fmt_addr(fn, names[fn]) for fn in seen]
        lines.append("- callers %d: %s" % (f["callers_n"],
                                           _join(items, 5, more=f["callers_n"] > len(seen))))

    callee_items, seen_c = [], set()
    for _site, tgt, typ, name in f.get("callees", []):
        if tgt is None or (tgt, typ) in seen_c:
            continue
        seen_c.add((tgt, typ))
        callee_items.append("%s (%s)" % (_fmt_addr(tgt, name), typ))
    if callee_items:
        lines.append("- callees: " + _join(callee_items, 6))

    if f.get("indirect"):
        items = []
        for site, slot, _how in f["indirect"]:
            if slot is None:
                items.append("0x%08x unresolved" % site)
            else:
                items.append("0x%08x slot 0x%x" % (site, slot))
        lines.append("- indirect: " + _join(items, 5))

    if f.get("strings"):
        items = ['0x%08x "%s"' % (sa, s) for sa, s in f["strings"]]
        lines.append("- strings: " + _join(items, 3))

    if f.get("globals"):
        items = ["0x%08x %s" % (t, rt) for t, rt, _ins in f["globals"]]
        lines.append("- globals: " + _join(items, 3))

    if f.get("imports"):
        items = ["%s (%s)" % (imp, lib) for imp, lib in f["imports"]]
        lines.append("- imports: " + _join(items, 3))

    # The twins line is always emitted ("- twins: none" when empty) and is never truncated: the cap drops
    # other lines first, so a missing twins line can never be mistaken for "no twins".
    tw = f.get("twins") or []
    if not tw:
        twins_line = "- twins: none"
    else:
        head = _join(["0x%08x" % t for t in tw], 5)
        vals = []
        for t in tw:
            v = f.get("distinguishing", {}).get(t, "")
            if v and v not in vals:
                vals.append(v)
        if vals:
            head += " (distinguishing: %s)" % "; ".join(vals)
        twins_line = "- twins: " + head

    if max_lines <= 0:
        return []
    room = max_lines - 1  # lines left for everything except the twins line
    if len(lines) > room:
        keep = max(room - 1, 0)
        dropped = len(lines) - keep
        lines = lines[:keep] + (["- ... (%d more)" % dropped] if room >= 1 else [])
    return lines + [twins_line]


# ---- CLI -------------------------------------------------------------------
def _cli(argv=None):
    ap = argparse.ArgumentParser(description="Per-address static facts from the Ghidra static dump.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    p_addr = sub.add_parser("addr", help="facts card for one address")
    p_addr.add_argument("hex")
    p_addr.add_argument("--json", action="store_true")
    p_addr.add_argument("--root", default=str(DUMP_ROOT))
    p_tw = sub.add_parser("twins", help="twins of one address")
    p_tw.add_argument("hex")
    p_tw.add_argument("--root", default=str(DUMP_ROOT))
    args = ap.parse_args(argv)
    a = _hex(args.hex)
    if a is None:
        ap.error("bad hex address: %s" % args.hex)
    dump = Dump(args.root, names=load_names())
    if args.cmd == "addr":
        f = facts(dump, a)
        if args.json:
            print(json.dumps(f, indent=2, default=str))
        else:
            print("0x%08x" % a)
            for line in card_lines(f):
                print(line)
    else:
        tw = dump.twins(a)
        dist = dump.distinguishing(a, tw)
        if not tw:
            print("none")
        for t in tw:
            print("0x%08x  %s" % (t, dist.get(t, "")))
    return 0


if __name__ == "__main__":
    sys.exit(_cli())
