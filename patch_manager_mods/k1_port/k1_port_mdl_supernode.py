#!/usr/bin/env python3
"""Rewrite the MDL node "supernode" u16 and the geometry node total of a pykotor-written K1->K2 model so it
carries K2 supermodel-chain numbering, as MDLOps does. Stdlib only, operates on the binary MDL.

Why: pykotor writes supernode = own index and total = own node count; the K2 engine expects the chain numbering
(a node that also exists in the supermodel inherits the supermodel node's number, others get fresh numbers
above the supermodel's range). The rules were recovered from MDLOpsM.pm get_super_nodes and verified exact on
398/398 vanilla K2 models with a supermodel (.claude/research/pykotor_writer/h6a_vanilla_sim.md):
  R0 the geometry root maps to the supermodel root.
  R1 a child whose lowercase name equals a child name of the matched supermodel parent inherits that
     supermodel node's supernode (first match in file order).
  R2 an unmatched child of a matched parent gets `max`, then max += 1 (max starts at largest supermodel
     supernode + 1; DFS pre-order).
  R3 every node below an unmatched node gets (its own index + supermodel total + 1).
  R4 geometry total (file offset 56) = own node count + (supermodel total + 1 if supermodel total else 0);
     animation nodes are not counted.
  R5 no supermodel (NULL or empty): supernode = own index, total = own node count.
Animation node headers take the supernode of the geometry node with the same index (+4 field); an animation
node whose index is not a geometry index is left alone and counted in the report.

Edits are only: u16 at (node header + 2) for geometry and animation nodes, and i32 at file offset 56. The input
is never mutated; the result has the same length and apply() is idempotent.

Offsets (stored pointers are relative to file byte 12; abs = value + 12): root node ptr at 52, node total at 56,
animation array ptr at 100, animation count at 104, supermodel name at 148 (32 bytes), name table ptr at 196,
name count at 200. Node header is 80 bytes: u16 type +0, u16 supernode +2, u16 index +4 (the name-table index),
children ptr +44, children count +48. Animation header (136 bytes) has its node-tree root ptr at +40.

CLI:  python3 k1_port_mdl_supernode.py check FILE (--super FILE | --super-dir DIR ...) [--json]
      python3 k1_port_mdl_supernode.py fix   FILE (--super FILE | --super-dir DIR ...) --out FILE [--in-place]
Exit codes: 0 clean/written, 1 mismatch (check only), 2 error. Importing the K2 install for the supermodel
(chain_resolver) needs the k1port venv python; everything else runs on any python3.
"""
import argparse
import hashlib
import json
import struct
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.dont_write_bytecode = True

NODE_SIZE = 80
MIN_NODE_ABS = 208          # model header end; nodes never start before it
OFF_ROOT, OFF_TOTAL = 52, 56
OFF_ANIM_PTR, OFF_ANIM_COUNT = 100, 104
OFF_SUPER = 148
OFF_NAME_PTR, OFF_NAME_COUNT = 196, 200
MAX_DEPTH = 2000


class SupernodeError(Exception):
    pass


@dataclass
class Node:
    off: int                 # absolute file offset of the 80-byte header
    type: int
    sn: int                  # stored supernode (+2)
    idx: int                 # stored index (+4)
    name_lower: str          # geometry nodes only ("" for animation nodes)
    children: list = field(default_factory=list)


@dataclass
class Tree:
    nodes: list              # geometry nodes, pre-order
    total: int               # stored i32 at offset 56
    super_name: str          # "" when none / NULL
    anims: list              # one pre-order node list per animation


# ---- parsing ----------------------------------------------------------------------------

def _u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def _u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def _need(mdl, off, n, what):
    if off < 0 or off + n > len(mdl):
        raise SupernodeError(f"{what}: {n} bytes at file offset {off} is outside the file (size {len(mdl)})")


def _name_reader(mdl):
    ptr, cnt = _u32(mdl, OFF_NAME_PTR) + 12, _u32(mdl, OFF_NAME_COUNT)
    cache = {}

    def name(idx):
        if idx in cache:
            return cache[idx]
        if idx >= cnt:
            raise SupernodeError(f"node name index {idx} is >= name count {cnt}")
        _need(mdl, ptr + 4 * idx, 4, f"name table entry {idx}")
        so = _u32(mdl, ptr + 4 * idx) + 12
        if so < 0 or so >= len(mdl):
            raise SupernodeError(f"name table entry {idx} points at {so}, outside the file")
        end = mdl.find(b"\0", so)
        if end < 0:
            raise SupernodeError(f"name table entry {idx} is not NUL-terminated")
        cache[idx] = mdl[so:end].decode("latin1").lower()
        return cache[idx]

    return name


def _walk(mdl, root_abs, name_of, what):
    """Pre-order walk of one node tree by pointers. Raises on cycles, revisits and bad pointers."""
    out = []
    seen = set()
    stack = [(root_abs, None, 1)]
    while stack:
        a, parent, depth = stack.pop()
        if depth > MAX_DEPTH:
            raise SupernodeError(f"{what}: tree deeper than {MAX_DEPTH}")
        if a < MIN_NODE_ABS or a + NODE_SIZE > len(mdl):
            raise SupernodeError(f"{what}: node pointer {a} is outside the file or leaves < {NODE_SIZE} bytes")
        if a in seen:
            raise SupernodeError(f"{what}: node at {a} is reached twice (cycle or shared node)")
        seen.add(a)
        idx = _u16(mdl, a + 4)
        node = Node(a, _u16(mdl, a), _u16(mdl, a + 2), idx, name_of(idx) if name_of else "")
        out.append(node)
        if parent is not None:
            parent.children.append(node)
        cnt, cptr = _u32(mdl, a + 48), _u32(mdl, a + 44) + 12
        if cnt:
            _need(mdl, cptr, 4 * cnt, f"{what}: children array of node at {a}")
            kids = [_u32(mdl, cptr + 4 * k) + 12 for k in range(cnt)]
            for k in reversed(kids):
                stack.append((k, node, depth + 1))
    return out


def parse_tree(mdl):
    """Parse the geometry tree, animation trees and header fields needed for supernode numbering."""
    if not isinstance(mdl, (bytes, bytearray)):
        raise SupernodeError("mdl must be bytes")
    _need(mdl, 0, 208, "model header")
    name_of = _name_reader(mdl)
    raw = bytes(mdl[OFF_SUPER:OFF_SUPER + 32])
    super_name = raw.split(b"\0")[0].decode("latin1").strip()
    if super_name.lower() in ("", "null"):
        super_name = ""
    nodes = _walk(mdl, _u32(mdl, OFF_ROOT) + 12, name_of, "geometry tree")
    total = struct.unpack_from("<i", mdl, OFF_TOTAL)[0]
    acnt, aptr = _u32(mdl, OFF_ANIM_COUNT), _u32(mdl, OFF_ANIM_PTR) + 12
    anims = []
    if acnt:
        _need(mdl, aptr, 4 * acnt, "animation array")
        for i in range(acnt):
            ah = _u32(mdl, aptr + 4 * i) + 12
            _need(mdl, ah, 136, f"animation header {i}")
            anims.append(_walk(mdl, _u32(mdl, ah + 40) + 12, None, f"animation {i} tree"))
    return Tree(nodes, total, super_name, anims)


# ---- numbering --------------------------------------------------------------------------

def assign(child, sup):
    """Return ({index: supernode}, total) for `child` Tree against supermodel Tree `sup` (None = no supermodel)."""
    out = {}
    if sup is None or not sup.nodes:
        for n in child.nodes:
            out[n.idx] = n.idx
        return out, len(child.nodes)
    stotal = sup.total
    counter = [max(n.sn for n in sup.nodes) + 1]
    # state: a supermodel Node (matched), -1 (unmatched child of a matched parent), -2 (below an unmatched node)
    stack = [(child.nodes[0], sup.nodes[0])]
    while stack:
        node, state = stack.pop()
        if state == -1:
            out[node.idx] = counter[0]
            counter[0] += 1
        elif state == -2:
            out[node.idx] = node.idx + stotal + 1
        else:
            out[node.idx] = state.sn
        kids = []
        if isinstance(state, int):
            kids = [(c, -2) for c in node.children]
        else:
            first = {}
            for sc in state.children:
                first.setdefault(sc.name_lower, sc)
            kids = [(c, first.get(c.name_lower, -1)) for c in node.children]
        stack.extend(reversed(kids))
    total = len(child.nodes) + (stotal + 1 if stotal else 0)
    return out, total


# ---- apply ------------------------------------------------------------------------------

def _resolve_super(tree, resolve):
    """(supermodel Tree or None, sha256 or None)."""
    if not tree.super_name:
        return None, None
    name = tree.super_name.lower()
    try:
        data = resolve(name)
    except Exception as exc:  # noqa: BLE001  never fall back silently
        raise SupernodeError(f"supermodel {tree.super_name} could not be resolved: {type(exc).__name__}: {exc}") from exc
    if not isinstance(data, (bytes, bytearray)) or not data:
        raise SupernodeError(f"supermodel {tree.super_name}: resolver returned no bytes")
    try:
        sup = parse_tree(bytes(data))
    except SupernodeError as exc:
        raise SupernodeError(f"supermodel {tree.super_name} is not a readable MDL: {exc}") from exc
    return sup, hashlib.sha256(data).hexdigest()


def _plan(mdl, resolve):
    tree = parse_tree(mdl)
    sup, sha = _resolve_super(tree, resolve)
    mapping, total = assign(tree, sup)
    for idx, v in mapping.items():
        if not 0 <= v <= 0xFFFF:
            raise SupernodeError(f"node index {idx}: supernode {v} does not fit a u16")
    return tree, sup, sha, mapping, total


def apply(mdl, resolve):
    """Return (new_bytes, report). `resolve(super_name_lower) -> bytes` is called only for a real supermodel."""
    tree, _sup, sha, mapping, total = _plan(mdl, resolve)
    out = bytearray(mdl)
    changed = []
    nodes_changed = 0
    for n in tree.nodes:
        want = mapping[n.idx]
        if n.sn != want:
            struct.pack_into("<H", out, n.off + 2, want)
            nodes_changed += 1
            changed.append(n.off + 2)
    anim_changed = unmapped = 0
    for tr in tree.anims:
        for n in tr:
            if n.idx not in mapping:
                unmapped += 1
                continue
            want = mapping[n.idx]
            if n.sn != want:
                struct.pack_into("<H", out, n.off + 2, want)
                anim_changed += 1
                changed.append(n.off + 2)
    if tree.total != total:
        struct.pack_into("<i", out, OFF_TOTAL, total)
        changed.append(OFF_TOTAL)
    report = {
        "supermodel": tree.super_name or None,
        "supermodel_sha256": sha,
        "nodes_changed": nodes_changed,
        "anim_nodes_changed": anim_changed,
        "anim_unmapped": unmapped,
        "total_before": tree.total,
        "total_after": total,
        "changed_offsets_sample": changed[:10],
    }
    return bytes(out), report


def mismatches(mdl, resolve):
    """Per-node differences between the file's stored geometry numbering and `assign`. Returns a dict."""
    tree, _sup, sha, mapping, total = _plan(mdl, resolve)
    bad = [{"offset": n.off + 2, "index": n.idx, "name": n.name_lower, "have": n.sn, "want": mapping[n.idx]}
           for n in tree.nodes if n.sn != mapping[n.idx]]
    anim_bad = sum(1 for tr in tree.anims for n in tr if n.idx in mapping and n.sn != mapping[n.idx])
    return {"supermodel": tree.super_name or None, "supermodel_sha256": sha, "nodes": len(tree.nodes),
            "mismatches": bad, "total_have": tree.total, "total_want": total,
            "total_ok": tree.total == total, "anim_nodes_differing": anim_bad}


# ---- resolvers --------------------------------------------------------------------------

def _find_ci(directory, filename):
    d = Path(directory).expanduser()
    if not d.is_dir():
        return None
    want = filename.lower()
    for f in sorted(d.iterdir()):
        if f.name.lower() == want and f.is_file():
            return f
    return None


class _Resolver:
    def __init__(self, dirs, k2_dir=None):
        self.dirs = [Path(d).expanduser() for d in dirs]
        self.k2_dir = Path(k2_dir).expanduser() if k2_dir is not None else None
        self.origins = {}

    def __call__(self, name):
        name = name.lower()
        for d in self.dirs:
            hit = _find_ci(d, name + ".mdl")
            if hit is not None:
                self.origins[name] = f"dir:{hit}"
                return hit.read_bytes()
        if self.k2_dir is None:
            raise FileNotFoundError(f"{name}.mdl not found in {[str(d) for d in self.dirs]}")
        hit = _find_ci(self.k2_dir / "override", name + ".mdl")
        if hit is not None:
            self.origins[name] = "override"
            return hit.read_bytes()
        import k1_port_mdlops  # lazy: keeps this module stdlib-only at import, no import cycle
        data = k1_port_mdlops.k2_model_bytes(self.k2_dir, name)[0]
        self.origins[name] = "install"
        return data


def dir_resolver(dirs):
    """Resolver looking up <name>.mdl case-insensitively in `dirs` in order. `.origins[name]` = 'dir:<path>'."""
    return _Resolver(dirs)


def chain_resolver(dirs, k2_dir, pk=None):
    """dirs, then <k2_dir>/override, then the K2 install via k1_port_mdlops.k2_model_bytes (needs the k1port
    venv python). `.origins[name]` = 'dir:<path>' | 'override' | 'install'. `pk` is accepted for call-site
    compatibility and unused (k2_model_bytes loads pykotor itself)."""
    return _Resolver(dirs, k2_dir)


# ---- CLI --------------------------------------------------------------------------------

def _cli_resolver(args, super_name):
    if args.super:
        sp = Path(args.super)
        if not super_name:
            print(f"note: {args.file} has no supermodel; ignoring --super", file=sys.stderr)
        return lambda _name: sp.read_bytes()
    if args.super_dir:
        return dir_resolver(args.super_dir)
    if super_name:
        raise SupernodeError(f"{args.file} names supermodel {super_name}; pass --super FILE or --super-dir DIR")
    return lambda _name: b""


def _run(args):
    mdl = Path(args.file).read_bytes()
    resolve = _cli_resolver(args, parse_tree(mdl).super_name)
    if args.cmd == "check":
        res = mismatches(mdl, resolve)
        bad = bool(res["mismatches"]) or not res["total_ok"]
        if args.json:
            print(json.dumps(res, indent=2))
        else:
            for m in res["mismatches"]:
                print(f"MISMATCH node {m['index']} ({m['name']}) at {m['offset']}: have {m['have']} want {m['want']}")
            if not res["total_ok"]:
                print(f"MISMATCH total: have {res['total_have']} want {res['total_want']}")
            print(f"{'MISMATCH' if bad else 'OK'}: {len(res['mismatches'])}/{res['nodes']} nodes differ, "
                  f"total {res['total_have']} (want {res['total_want']}), supermodel {res['supermodel']}")
        return 1 if bad else 0
    src, out = Path(args.file).resolve(), Path(args.out).resolve()
    if "steamapps" in str(out).lower():
        raise SupernodeError(f"refusing to write into a game install path: {out}")
    if src == out and not args.in_place:
        raise SupernodeError("--out is the input file; pass --in-place to overwrite it")
    new, report = apply(mdl, resolve)
    out.write_bytes(new)
    print(json.dumps(report, indent=2))
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    for cmd in ("check", "fix"):
        p = sub.add_parser(cmd)
        p.add_argument("file")
        p.add_argument("--super", help="explicit supermodel MDL (used whatever the header name is)")
        p.add_argument("--super-dir", action="append", help="directory holding <supermodel>.mdl (repeatable)")
        if cmd == "check":
            p.add_argument("--json", action="store_true")
        else:
            p.add_argument("--out", required=True)
            p.add_argument("--in-place", action="store_true")
    args = ap.parse_args(argv)
    try:
        return _run(args)
    except (SupernodeError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
