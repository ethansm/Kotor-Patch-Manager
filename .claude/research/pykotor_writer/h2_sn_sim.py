#!/usr/bin/env python3
"""h2 prototype: port of MDLOpsM.pm get_super_nodes (bead-v supernode numbering) and a check against files.

Read-only. Loads MDL files with the k1port pykotor reader (monkeypatched to keep node_id/padding0), then:
  sim(child, supermodel) -> {node_id: supernode}, total
and compares against the child's own supernode values where that file is the MDLOps/K2 output.
Usage: python3 -I h2_sn_sim.py CHILD.mdl SUPER.mdl [SUPER_SN_OVERRIDE_NOT_USED]
"""
import struct
import sys
sys.path.insert(0, "/home/ethansm/Games/KOTOR2_Mod_Workspace/venvs/k1port/lib/python3.12/site-packages")
from pykotor.resource.formats.mdl import io_mdl as IO  # noqa: E402
from pykotor.resource.formats.mdl.mdl_auto import read_mdl  # noqa: E402

_nh = IO._NodeHeader.read
REC = []


def _hook(self, reader):
    REC.append(self)
    return _nh(self, reader)


IO._NodeHeader.read = _hook


def load(path):
    REC.clear()
    mdl = read_mdl(path)
    nodes = list(mdl.all_nodes())
    geo = REC[: len(nodes)]
    sn = {h.node_id: h.padding0 for h in geo}
    with open(path, "rb") as f:
        total = struct.unpack_from("<i", f.read(12 + 80), 12 + 44)[0]
    by_id = {n.node_id: n for n in nodes}
    return mdl, by_id, sn, total


def supernodes(child, supermodel):
    """supermodel = (mdl, by_id, sn, total) as returned by load(). Returns ({node_id: sn}, total)."""
    _, sby, ssn, stotal = supermodel
    _, cby, _, _ = child
    croot = cby[0]
    sroot = sby[0]
    # supermodel child index lists by node_id, in file order (= MDLOps childindexes order)
    schild = {nid: [c.node_id for c in n.children] for nid, n in sby.items()}
    smax = max(ssn.values()) if ssn else 0
    st = {"max": smax + 1}
    out = {}
    own = len(cby)
    nodenum_of = {id(n): n.node_id for n in cby.values()}

    def rec(node, superpart):
        if superpart < 0:
            if superpart == -1:
                out[node.node_id] = st["max"]
                st["max"] += 1
            else:
                out[node.node_id] = node.node_id + stotal + 1
            for ch in node.children:
                rec(ch, -2)
            return
        out[node.node_id] = ssn[superpart]
        for ch in node.children:
            name = ch.name.lower()
            match = -1
            for sc in schild.get(superpart, []):
                if sby[sc].name.lower() == name:
                    match = sc
                    break
            rec(ch, match)

    rec(croot, sroot.node_id)  # root matched to supermodel node 0 (MDLOps: get_super_nodes(.., 0, 0))
    total = own + (stotal + 1 if stotal else 0)
    return out, total


def compare(label, child, supermodel):
    sim, total = supernodes(child, supermodel)
    _, cby, csn, ctotal = child
    diff = [(cby[k].name, csn.get(k), sim[k]) for k in sorted(sim) if csn.get(k) != sim[k]]
    print(f"{label}: nodes={len(sim)} match={len(sim) - len(diff)} mismatch={len(diff)} "
          f"total file={ctotal} predicted={total}")
    for d in diff[:8]:
        print(f"   mismatch {d[0]}: file sn={d[1]} predicted={d[2]}")
    return sim


if __name__ == "__main__":
    child = load(sys.argv[1])
    sup = load(sys.argv[2])
    compare("child vs supermodel", child, sup)
