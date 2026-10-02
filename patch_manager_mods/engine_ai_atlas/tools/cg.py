#!/usr/bin/env python3
"""cg.py - callgraph query over ROOT/callgraph.tsv with names from the knowledge DB.

Subcommands:
  callers ADDR                 call sites + caller functions calling ADDR
  callees ADDR                 callees of function ADDR in call-site order (INDIRECT sites get a slot guess)
  path A B [--depth 8] [--max 5] [--via-vtable] [--bridge A:B ...]
                               shortest call paths A -> B over CALL+TAILJMP edges
  subtree A [--depth N]        functions reachable from A (indented tree, each printed once)
  indirect --slot 0x70 [--in ADDR]
                               INDIRECT call sites whose vtable slot is 0x70 (+ class_methods candidates)

INDIRECT sites in callgraph.tsv are mostly 'CALL EAX' / 'CALL EDX'; the slot is recovered by scanning the
caller's .asm backwards for 'MOV EAX,dword ptr [EDX + 0x70]' (see atlas_common.resolve_indirect).
The class of the object is NOT known statically: class_methods matches are labelled "slot match, class unverified".

Edges used by BFS: CALL and TAILJMP always.  --via-vtable additionally bridges every INDIRECT site with a
resolved slot to every class_methods entry with vtable_index*4 == slot (all classes: spurious by nature).
--bridge A:B (repeatable) adds a manual edge fn A -> fn B (use it for vtable dispatches the DB cannot resolve).
"""
import argparse
import sys
import os
from collections import deque

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import atlas_common as ac  # noqa: E402


class Ctx:
    def __init__(self, args):
        self.root = ac.expand_root(args.root)
        self.db = ac.open_db_ro(args.db)
        self.g = ac.Graph(self.root)
        self.names = ac.Names(self.root, self.db, enabled=args.names)
        self.tsv = args.tsv
        self._asm = {}
        self.via_vtable = getattr(args, "via_vtable", False)
        self.bridges = {}
        for b in getattr(args, "bridge", None) or []:
            a, _, c = b.partition(":")
            self.bridges.setdefault(ac.parse_addr(a), []).append(ac.parse_addr(c))
        self._slot_cache = {}

    def asm(self, fn):
        if fn not in self._asm:
            lines = ac.read_asm(self.root, fn)
            self._asm[fn] = (lines, {a: i for i, (a, _b, _t) in enumerate(lines or [])})
        return self._asm[fn]

    def slot_of(self, fn, site, insn):
        key = (fn, site)
        if key not in self._slot_cache:
            lines, idx = self.asm(fn)
            if lines is None:
                self._slot_cache[key] = (None, "no asm for fn")
            else:
                self._slot_cache[key] = ac.resolve_indirect(lines, site, insn, idx)
        return self._slot_cache[key]

    def edges(self, fn):
        """-> list of (callee, kind, site, note) for BFS / subtree."""
        out = []
        for site, callee, typ, insn in self.g.by_caller.get(fn, []):
            if callee is not None and typ in ac.Graph.EDGE_TYPES:
                out.append((callee, typ, site, ""))
            elif typ == "INDIRECT" and self.via_vtable:
                slot, how = self.slot_of(fn, site, insn)
                if slot is not None:
                    for c, vi, a, s in ac.slot_candidates(self.db, slot):
                        out.append((a, "VTABLE", site, "slot 0x%x -> %s idx %d (class unverified)" % (slot, c, vi)))
        for tgt in self.bridges.get(fn, []):
            out.append((tgt, "BRIDGE", 0, "manual bridge"))
        return out

    def lab(self, a):
        return self.names.label(a)


def emit(ctx, header, rows, text_rows=None):
    """TSV mode prints header+rows; text mode prints text_rows (or tab rows)."""
    if ctx.tsv:
        print("\t".join(header))
        for r in rows:
            print("\t".join(str(x) for x in r))
    else:
        for t in (text_rows if text_rows is not None else ["\t".join(str(x) for x in r) for r in rows]):
            print(t)


def cmd_callers(ctx, args):
    a = ac.parse_addr(args.addr)
    rows = ctx.g.by_callee.get(a, [])
    trows, tsv = [], []
    for site, caller, typ in rows:
        tsv.append((ac.fmt8(a), ac.fmt8(site), ac.fmt8(caller), ctx.names.name(caller), typ))
        trows.append("  site %s  %-8s %s" % (ac.fmt8(site), typ, ctx.lab(caller)))
    if not ctx.tsv:
        print("callers of %s: %d call sites from %d functions" % (ctx.lab(a), len(rows), len({r[1] for r in rows})))
        print("  (direct CALL/TAILJMP only; vtable dispatch is invisible here - use `indirect --slot`)")
    emit(ctx, ["callee", "site", "caller", "caller_name", "type"], tsv, trows)


def cmd_callees(ctx, args):
    a = ac.parse_addr(args.addr)
    rows = ctx.g.by_caller.get(a, [])
    tsv, trows = [], []
    for site, callee, typ, insn in rows:
        extra, slot_s, cands = "", "", []
        if typ == "INDIRECT":
            slot, how = ctx.slot_of(a, site, insn)
            if slot is not None:
                slot_s = "0x%x" % slot
                cands = ac.slot_candidates(ctx.db, slot)
                extra = "slot 0x%x (%s)" % (slot, how)
            else:
                extra = "slot ? (%s)" % how
        tgt = ctx.lab(callee) if callee is not None else "-"
        tsv.append((ac.fmt8(a), ac.fmt8(site), ac.fmt8(callee) if callee is not None else "-", typ, insn, slot_s))
        line = "  %s  %-8s %s" % (ac.fmt8(site), typ, tgt if callee is not None else "%s  %s" % (insn, extra))
        trows.append(line)
        for c, vi, ca, s in cands:
            trows.append("      slot match, class unverified: %s idx %d -> %s" % (c, vi, ctx.lab(ca)))
    if not ctx.tsv:
        print("callees of %s: %d sites" % (ctx.lab(a), len(rows)))
    emit(ctx, ["caller", "site", "callee", "type", "insn", "slot"], tsv, trows)


def bfs_paths(ctx, start, goal, depth, maxpaths):
    """All shortest paths (<= maxpaths printed) from start to goal; returns (paths, found_depth).

    path = list of (fn, edge_info_into_fn) ; edge_info = (kind, site, note).
    """
    dist = {start: 0}
    preds = {}
    q = deque([start])
    found = None
    while q:
        u = q.popleft()
        du = dist[u]
        if found is not None and du >= found:
            break
        if du >= depth:
            continue
        for v, kind, site, note in ctx.edges(u):
            if v not in dist:
                dist[v] = du + 1
                preds[v] = [(u, (kind, site, note))]
                q.append(v)
                if v == goal:
                    found = du + 1
            elif dist[v] == du + 1:
                if all(p[0] != u or p[1][1] != site for p in preds[v]):
                    preds[v].append((u, (kind, site, note)))
    if goal not in dist:
        return [], None
    paths = []

    def rec(v, acc):
        if len(paths) >= maxpaths:
            return
        if v == start:
            paths.append(list(reversed(acc)))
            return
        for u, info in preds[v]:
            rec(u, acc + [(v, info)])
            if len(paths) >= maxpaths:
                return

    rec(goal, [])
    return paths, dist[goal]


def cmd_path(ctx, args):
    a, b = ac.parse_addr(args.a), ac.parse_addr(args.b)
    paths, d = bfs_paths(ctx, a, b, args.depth, args.max)
    mode = "direct CALL+TAILJMP" + (" + vtable bridge" if ctx.via_vtable else "") + (
        " + manual bridges" if ctx.bridges else "")
    if not paths:
        if ctx.tsv:
            print("path\thop\tfn\tname\tkind\tsite\tnote")
        else:
            print("no path %s -> %s within depth %d (%s)" % (ctx.lab(a), ctx.lab(b), args.depth, mode))
        return 1
    tsv, trows = [], []
    trows.append("%d shortest path(s) of %d hops %s -> %s  [%s]" % (len(paths), d, ctx.lab(a), ctx.lab(b), mode))
    for pi, p in enumerate(paths, 1):
        trows.append("path %d:" % pi)
        trows.append("  %s" % ctx.lab(a))
        tsv.append((pi, 0, ac.fmt8(a), ctx.names.name(a), "", "", ""))
        for hop, (fn, (kind, site, note)) in enumerate(p, 1):
            trows.append("   -> [%s @%s%s] %s" % (kind, ac.fmt8(site) if site else "-", (" " + note) if note else "",
                                                  ctx.lab(fn)))
            tsv.append((pi, hop, ac.fmt8(fn), ctx.names.name(fn), kind, ac.fmt8(site) if site else "", note))
    emit(ctx, ["path", "hop", "fn", "name", "kind", "site", "note"], tsv, trows)
    return 0


def cmd_subtree(ctx, args):
    a = ac.parse_addr(args.addr)
    seen = {a}
    count = [0]
    tsv, trows = [], []
    hub = args.skip_hubs
    fanin = {}
    if hub:
        for fn_, n_ in _fan_in_fns(ctx.root).items():
            fanin[fn_] = n_

    def walk(fn, level):
        if level >= args.depth:
            return
        ind = "  " * (level + 1)
        indirect = 0
        for callee, kind, site, note in ctx.edges(fn):
            if hub and fanin.get(callee, 0) >= hub:
                continue
            if count[0] >= args.max_nodes:
                return
            if callee in seen:
                tsv.append((level + 1, ac.fmt8(callee), ctx.names.name(callee), kind, ac.fmt8(fn), "seen"))
                trows.append("%s%s  (seen)" % (ind, ctx.lab(callee)))
                continue
            seen.add(callee)
            count[0] += 1
            tsv.append((level + 1, ac.fmt8(callee), ctx.names.name(callee), kind, ac.fmt8(fn), ""))
            trows.append("%s%s%s" % (ind, ctx.lab(callee), "" if kind == "CALL" else "  <%s>" % kind))
            walk(callee, level + 1)
        for site, c, typ, insn in ctx.g.by_caller.get(fn, []):
            if typ == "INDIRECT":
                indirect += 1
        if indirect and not ctx.via_vtable and level < args.depth:
            trows.append("%s(+%d indirect sites in %s)" % (ind, indirect, ac.fmt8(fn)))

    trows.append(ctx.lab(a))
    tsv.append((0, ac.fmt8(a), ctx.names.name(a), "", "", ""))
    walk(a, 0)
    if count[0] >= args.max_nodes:
        trows.append("... truncated at --max-nodes %d" % args.max_nodes)
    if not ctx.tsv:
        trows.append("%d distinct functions below %s (depth %d)" % (count[0], ac.fmt8(a), args.depth))
    emit(ctx, ["depth", "fn", "name", "kind", "parent", "note"], tsv, trows)


def _fan_in_fns(root):
    out = {}
    p = os.path.join(root, "callers_count.tsv")
    try:
        with open(p, encoding="utf-8", errors="replace") as f:
            next(f, None)
            for line in f:
                parts = line.rstrip("\n").split("\t")
                if len(parts) >= 4:
                    try:
                        out[int(parts[0], 16)] = int(parts[3])
                    except ValueError:
                        pass
    except OSError:
        pass
    return out


def cmd_indirect(ctx, args):
    slot = ac.parse_addr(args.slot)
    fns = [ac.parse_addr(args.in_fn)] if args.in_fn else sorted(ctx.g.by_caller)
    tsv, trows = [], []
    n = 0
    for fn in fns:
        for site, callee, typ, insn in ctx.g.by_caller.get(fn, []):
            if typ != "INDIRECT":
                continue
            s, how = ctx.slot_of(fn, site, insn)
            if s == slot:
                n += 1
                tsv.append((ac.fmt8(fn), ctx.names.name(fn), ac.fmt8(site), insn, how))
                trows.append("  %s  %s  %s   <- %s" % (ac.fmt8(site), ctx.lab(fn), insn, how))
    if not ctx.tsv:
        print("INDIRECT sites with vtable slot 0x%x%s: %d" % (slot, " in %s" % ac.fmt8(fns[0]) if args.in_fn else "", n))
    emit(ctx, ["fn", "fn_name", "site", "insn", "slot_evidence"], tsv, trows)
    cands = ac.slot_candidates(ctx.db, slot)
    if not ctx.tsv:
        print("class_methods with vtable_index*4 == 0x%x (slot match, class unverified): %d" % (slot, len(cands)))
        for c, vi, a, s in cands:
            print("  %s idx %d -> %s" % (c, vi, ctx.lab(a)))


def build_parser():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ac.add_common_args(ap)
    ap.add_argument("--names", action=argparse.BooleanOptionalAction, default=True,
                    help="join names from DB/callers_count (default on; --no-names to disable)")
    ap.add_argument("--tsv", action="store_true", help="machine-readable output")
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("callers"); p.add_argument("addr")
    p = sub.add_parser("callees"); p.add_argument("addr")
    p = sub.add_parser("path"); p.add_argument("a"); p.add_argument("b")
    p.add_argument("--depth", type=int, default=8); p.add_argument("--max", type=int, default=5)
    p.add_argument("--via-vtable", action="store_true", help="bridge INDIRECT [reg+0xNN] sites to class_methods slot matches")
    p.add_argument("--bridge", action="append", metavar="A:B", help="manual edge fn A -> fn B (repeatable)")
    p = sub.add_parser("subtree"); p.add_argument("addr")
    p.add_argument("--depth", type=int, default=3)
    p.add_argument("--max-nodes", type=int, default=2000)
    p.add_argument("--skip-hubs", type=int, default=0, metavar="N", help="skip callees with fan_in_fns >= N")
    p.add_argument("--via-vtable", action="store_true"); p.add_argument("--bridge", action="append", metavar="A:B")
    p = sub.add_parser("indirect"); p.add_argument("--slot", required=True)
    p.add_argument("--in", dest="in_fn", default=None, help="limit to one caller function")
    return ap


def main(argv=None):
    args = build_parser().parse_args(argv)
    ctx = Ctx(args)
    fn = {"callers": cmd_callers, "callees": cmd_callees, "path": cmd_path,
          "subtree": cmd_subtree, "indirect": cmd_indirect}[args.cmd]
    return fn(ctx, args) or 0


if __name__ == "__main__":
    import signal
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)  # quiet when piped into head
    sys.exit(main())
