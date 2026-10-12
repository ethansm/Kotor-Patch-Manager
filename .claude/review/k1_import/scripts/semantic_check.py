"""B4: model-level K2 fidelity check.

Reads B3 inputs (read-only), summarizes the parsed model, writes it with the
forced-K2 writer into B4, reads that back, and diffs the node summaries.
"""

import json
import sys
from pathlib import Path

SCRATCH = Path("/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2c3f74c1-88fa-49a4-8101-3e63d340c7c0/scratchpad")
B3 = SCRATCH / "B3"
B4 = SCRATCH / "B4"
NAMES = ("p_bastilah", "p_carthh", "p_hk47")
sys.path.insert(0, str(B3 / "pk_force"))

import pykotor  # noqa: E402
from pykotor.common.misc import Game  # noqa: E402
from pykotor.resource.formats.mdl.io_mdl import MDLBinaryWriter  # noqa: E402
from pykotor.resource.formats.mdl.mdl_auto import read_mdl  # noqa: E402

FIELDS = (
    "name",
    "node_type",
    "child_count",
    "trimesh",
    "vertex_count",
    "face_count",
    "textures",
    "dirt_enabled",
    "dirt_texture",
    "dirt_worldspace",
    "hologram_donotdraw",
    "hide_in_hologram",
    "pos_min",
    "pos_max",
)


def _bounds(verts):
    if not verts:
        return None, None
    lo = [round(min(getattr(v, a) for v in verts), 4) for a in ("x", "y", "z")]
    hi = [round(max(getattr(v, a) for v in verts), 4) for a in ("x", "y", "z")]
    return lo, hi


def summarize(mdl):
    nodes = []

    def walk(node):
        rec = {
            "index": len(nodes),
            "name": node.name,
            "node_type": int(node.node_type),
            "child_count": len(node.children),
            "trimesh": node.mesh is not None,
        }
        m = node.mesh
        if m is not None:
            lo, hi = _bounds(m.vertex_positions)
            rec.update(
                vertex_count=len(m.vertex_positions),
                face_count=len(m.faces),
                textures=sorted({t for t in (m.texture_1, m.texture_2) if t}),
                dirt_enabled=bool(m.dirt_enabled),
                dirt_texture=int(m.dirt_texture),
                dirt_worldspace=int(m.dirt_worldspace),
                hologram_donotdraw=bool(m.hologram_donotdraw),
                hide_in_hologram=bool(m.hide_in_hologram),
                pos_min=lo,
                pos_max=hi,
            )
        nodes.append(rec)
        for child in node.children:
            walk(child)

    walk(mdl.root)
    anims = getattr(mdl, "animations", None)
    return {"animation_count": len(anims) if anims is not None else None, "nodes": nodes}


def compare(orig_nodes, k2_nodes):
    field_hits = {}
    first = None
    for i in range(min(len(orig_nodes), len(k2_nodes))):
        a, b = orig_nodes[i], k2_nodes[i]
        for f in FIELDS:
            va, vb = a.get(f), b.get(f)
            if va != vb:
                field_hits[f] = field_hits.get(f, 0) + 1
                if first is None:
                    first = (i, a.get("name"), f, va, vb)
    return field_hits, first


def fmt_val(v):
    s = repr(v)
    return s if len(s) <= 80 else s[:77] + "..."


def make_row(name, orig, k2):
    on, kn = orig["nodes"], k2["nodes"]
    hits, first = compare(on, kn)
    anim = f"anims {orig['animation_count']}->{k2['animation_count']}"
    if not hits and len(on) == len(kn):
        return f"{name}: MATCH ({len(on)} nodes, {anim})", True
    parts = [f"{f} ({n} nodes)" for f, n in sorted(hits.items())]
    if len(on) != len(kn):
        parts.append(f"node count {len(on)}->{len(kn)}")
    fields = ", ".join(parts)
    if first:
        i, nm, f, va, vb = first
        fi = f"first: idx {i} '{nm}' {f}: {fmt_val(va)} -> {fmt_val(vb)}"
    else:
        fi = "first: none (node count only)"
    return f"{name}: MISMATCH fields=[{fields}]; {fi}; {anim}", False


def main():
    print(f"pykotor={pykotor.__file__}", flush=True)
    B4.mkdir(parents=True, exist_ok=True)
    rows = []
    matches = 0
    for name in NAMES:
        print(f"== {name}", flush=True)
        try:
            in_mdl = (B3 / f"in_{name}.mdl").read_bytes()
            in_mdx = (B3 / f"in_{name}.mdx").read_bytes()
            mdl = read_mdl(in_mdl, source_ext=in_mdx, size_ext=len(in_mdx))
            orig = summarize(mdl)
            (B4 / f"orig_{name}.json").write_text(json.dumps(orig, indent=1))

            out_mdl_path = B4 / f"out_{name}.mdl"
            out_mdx_path = B4 / f"out_{name}.mdx"
            writer = MDLBinaryWriter(mdl, out_mdl_path, out_mdx_path)
            writer.force_game = Game.K2
            writer.write()
            print(f"  wrote {out_mdl_path.stat().st_size} mdl / {out_mdx_path.stat().st_size} mdx", flush=True)
        except Exception as exc:  # noqa: BLE001
            rows.append(f"{name}: MISMATCH write/parse failed: {type(exc).__name__}: {exc}")
            continue

        try:
            out_mdl = out_mdl_path.read_bytes()
            out_mdx = out_mdx_path.read_bytes()
            k2 = summarize(read_mdl(out_mdl, source_ext=out_mdx, size_ext=len(out_mdx)))
            (B4 / f"k2_{name}.json").write_text(json.dumps(k2, indent=1))
        except Exception as exc:  # noqa: BLE001
            rows.append(f"{name}: MISMATCH read-back failed: {type(exc).__name__}: {exc}")
            continue

        row, ok = make_row(name, orig, k2)
        rows.append(row)
        matches += int(ok)

    for r in rows:
        print(r, flush=True)
    total = len(NAMES)
    verdict = "all MATCH" if matches == total else f"{total - matches} of {total} MISMATCH"
    print(f"VERDICT: {verdict}", flush=True)


if __name__ == "__main__":
    main()
