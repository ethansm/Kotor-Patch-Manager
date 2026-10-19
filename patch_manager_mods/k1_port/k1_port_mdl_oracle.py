#!/usr/bin/env python3
"""Oracle for a pykotor-converted K1 -> K2 model: judges the output against the K1 SOURCE model and the K2
supermodel chain, never against MDLOps output. The importer's `converter = "auto"` uses the verdict to decide
whether the pykotor build is trustworthy or must fall back to MDLOps. Core logic is stdlib on bytes; pykotor is
only touched inside the CLI's K1 loader (k1port venv python), and only when the K1 dir is a real install.

What it adds on top of k1_port_validate (which checks structure only, no content against a source):
  T2 supernode   output supernodes/total vs the chain numbering from k1_port_mdl_supernode.assign
                 sn_skin (FAIL, skinned node), sn_nonskin (WARN), sn_total (FAIL), sn_error (FAIL, chain unreadable)
                 NOTE: circular on a chain output by design (the same rule produced it). Independence comes from
                 the supernode module's vanilla golden tests (398/398 exact vs K2 vanilla), not from this check.
  T1 topology    K1 pre-order node names (after the rename map) == output names (topo_names, FAIL); child count per
                 pre-order position (topo_children, FAIL); node +4 index == pre-order position (topo_index, WARN:
                 vanilla K2 itself breaks it in 111 models because +4 is the name-table index)
  T7 plane       every face of every mesh: abs(d + n.v1) <= 1e-3*max(1, abs(d)) (plane, FAIL). validate does not
                 check face vertex indices or plane distances, so this is not a duplicate
  T4 bones16     skin slot table: used slots must name a node in [0, node count) and invert the bonemap
                 (bones16, WARN only: K2 vanilla itself has 617/1414 such)
  T8-lite dangly a dangly (0x100) node of the K1 source with no dangly node of the same renamed name in the
                 output = demoted (dangly_demoted, FAIL). Full dangly checks come later.
Not implemented yet (Tranche B/C): T5 pose + controller multiset, T6 triangles keyed (pos, uv0), T3 per-vertex skin
binding; see the TODO stubs at the bottom of the checks.

Rename map: `rename` = {k1_name_lower: k2_name_lower} applied to K1 node names before comparing. The importer
renames the model and, when the root node carried the model name, the root node (k1_port_model.rename_model);
that root rename is derived here from the two headers (model name at file offset 20) and an explicit `rename`
entry overrides it. There is no other node rename in the pykotor path.

API:  check_model(out_mdl, out_mdx, k1_mdl, k1_mdx, resolve, rename=None) -> list[OracleFinding]
      resolve(super_name_lower) -> supermodel MDL bytes (see k1_port_mdl_supernode.chain_resolver/dir_resolver)
CLI:  python3 k1_port_mdl_oracle.py --k1 DIR --k2 DIR --stage DIR --models K1NAME=K2NAME ... [--json]
      --k1: K1 install (needs the k1port venv python for pykotor) or a directory of extracted <name>.mdl/.mdx
      --k2: K2 install, read-only (supermodel chain: stage models dir, then override, then BIFs)
      --stage: dir holding models/<k2name>.mdl|.mdx, the converted outputs
Exit: 0 = no FAIL finding, 1 = any FAIL finding, 2 = error (bad arguments, missing or unreadable input).
"""
import argparse
import json
import struct
import sys
from dataclasses import asdict, dataclass
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))  # python -I drops the script dir from sys.path
import k1_port_mdl_supernode as sn  # noqa: E402  stdlib only

NODE_HDR = 80
TM_K2 = 340                       # K2 trimesh header size (K1 is 332)
NODE_MESH, NODE_SKIN, NODE_DANGLY = 0x20, 0x40, 0x100
OFF_MODEL_NAME = 20
FACE_SIZE = 32
PLANE_REL_TOL = 1e-3
MAX_PER_CODE = 25                 # findings per code before one summary finding takes over


class OracleError(Exception):
    pass


@dataclass
class OracleFinding:
    check: str
    severity: str                 # 'FAIL' | 'WARN' | 'INFO'
    message: str
    node: str | None = None


class _Out:
    """Finding collector with a per-code cap (the cap summary keeps the severity of the capped code)."""

    def __init__(self):
        self.items = []
        self._n = {}

    def add(self, check, severity, message, node=None):
        k = self._n.get(check, 0) + 1
        self._n[check] = k
        if k <= MAX_PER_CODE:
            self.items.append(OracleFinding(check, severity, message, node))
        elif k == MAX_PER_CODE + 1:
            self.items.append(OracleFinding(check, severity, f"{check}: further findings of this check not listed", None))


def _u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def _u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def _i32(b, o):
    return struct.unpack_from("<i", b, o)[0]


def _model_name(mdl):
    return bytes(mdl[OFF_MODEL_NAME:OFF_MODEL_NAME + 32]).split(b"\0")[0].decode("latin1").lower()


def _rename_map(out_tree, k1_tree, out_mdl, k1_mdl, rename):
    """Root-node rename derived from both headers, then the explicit map (keys and values lowercased)."""
    m = {}
    if k1_tree.nodes and out_tree.nodes:
        k1_root, out_root = k1_tree.nodes[0].name_lower, out_tree.nodes[0].name_lower
        if k1_root == _model_name(k1_mdl) and out_root == _model_name(out_mdl):
            m[k1_root] = out_root
    for k, v in (rename or {}).items():
        m[k.lower()] = v.lower()
    return m


# ---- T2 supernode -------------------------------------------------------------------------

def _check_supernodes(out, tree, resolve):
    try:
        sup, _sha = sn._resolve_super(tree, resolve)
        mapping, total = sn.assign(tree, sup)
    except sn.SupernodeError as exc:
        out.add("sn_error", "FAIL", f"cannot compute the chain numbering: {exc}")
        return
    for pos, n in enumerate(tree.nodes):
        want = mapping[n.idx]
        if n.sn == want:
            continue
        skin = bool(n.type & NODE_SKIN)
        out.add("sn_skin" if skin else "sn_nonskin", "FAIL" if skin else "WARN",
                f"{'skinned' if skin else 'non-skin'} node #{pos} has supernode {n.sn}, chain numbering wants {want}",
                n.name_lower)
    if tree.total != total:
        out.add("sn_total", "FAIL", f"geometry node total is {tree.total}, chain numbering wants {total}")


# ---- T1 topology --------------------------------------------------------------------------

def _check_topology(out, o, k1, rename):
    k1_names = [rename.get(n.name_lower, n.name_lower) for n in k1.nodes]
    out_names = [n.name_lower for n in o.nodes]
    if k1_names != out_names:
        if len(k1_names) != len(out_names):
            out.add("topo_names", "FAIL", f"node count {len(out_names)} but the K1 source has {len(k1_names)}")
        bad = [i for i, (a, b) in enumerate(zip(k1_names, out_names)) if a != b]
        if bad:
            i = bad[0]
            out.add("topo_names", "FAIL",
                    f"{len(bad)} pre-order name differences; first at #{i}: K1 {k1_names[i]!r}, output {out_names[i]!r}",
                    out_names[i])
    k1_kids = [len(n.children) for n in k1.nodes]
    out_kids = [len(n.children) for n in o.nodes]
    bad = [i for i, (a, b) in enumerate(zip(k1_kids, out_kids)) if a != b]
    if bad:
        i = bad[0]
        out.add("topo_children", "FAIL",
                f"{len(bad)} child-count differences; first at #{i} ({out_names[i]}): K1 {k1_kids[i]}, output {out_kids[i]}",
                out_names[i])
    off = [i for i, n in enumerate(o.nodes) if n.idx != i]
    if off:
        i = off[0]
        out.add("topo_index", "WARN",
                f"{len(off)} nodes have +4 index != pre-order position (vanilla K2 does this in 111 models); "
                f"first: #{i} {out_names[i]} has index {o.nodes[i].idx}", out_names[i])


# ---- T7 plane -----------------------------------------------------------------------------

def _check_plane(out, mdl, mdx, tree):
    if mdx is None:
        out.add("plane", "INFO", "no MDX supplied, plane check skipped")
        return
    for n in tree.nodes:
        if not n.type & NODE_MESH:
            continue
        T = n.off + NODE_HDR
        try:
            _plane_mesh(out, mdl, mdx, n, T)
        except (struct.error, IndexError) as exc:
            out.add("plane", "FAIL", f"mesh data is unreadable ({exc})", n.name_lower)


def _plane_mesh(out, mdl, mdx, n, T):
    fcount = _u32(mdl, T + 12)
    if fcount == 0:
        return
    fbase = _u32(mdl, T + 8) + 12
    vc = _u16(mdl, T + 304)
    stride = _i32(mdl, T + 252)
    bitmap = _u32(mdl, T + 256)
    voff = _i32(mdl, T + 260)
    mdx_base = _u32(mdl, T + TM_K2 - 8)
    if not bitmap & 1 or voff < 0 or stride <= 0:
        out.add("plane", "FAIL", "mesh has faces but no readable MDX vertex block", n.name_lower)
        return
    if vc and mdx_base + (vc - 1) * stride + voff + 12 > len(mdx):
        out.add("plane", "FAIL", "MDX is too short for the vertex rows", n.name_lower)
        return
    if fbase + fcount * FACE_SIZE > len(mdl):
        out.add("plane", "FAIL", f"{fcount} faces run outside the file", n.name_lower)
        return
    bad_plane, bad_index, first = 0, 0, None
    for f in range(fcount):
        o = fbase + f * FACE_SIZE
        nx, ny, nz, d = struct.unpack_from("<4f", mdl, o)
        v1 = _u16(mdl, o + 26)
        if v1 >= vc:
            bad_index += 1
            first = first or f"face {f}: vertex index {v1} >= vertex count {vc}"
            continue
        x, y, z = struct.unpack_from("<3f", mdx, mdx_base + v1 * stride + voff)
        err = abs(d + nx * x + ny * y + nz * z)
        if not err <= PLANE_REL_TOL * max(1.0, abs(d)):   # also catches NaN
            bad_plane += 1
            first = first or f"face {f}: |d + n.v1| = {err:.5g}, d = {d:.5g}"
    if bad_plane or bad_index:
        out.add("plane", "FAIL",
                f"{bad_plane} of {fcount} faces off their plane, {bad_index} with an out-of-range vertex index; "
                f"first: {first}", n.name_lower)


# ---- T4 bones16 ---------------------------------------------------------------------------

def _check_bones16(out, mdl, tree):
    count = len(tree.nodes)
    for n in tree.nodes:
        if not (n.type & NODE_MESH and n.type & NODE_SKIN):
            continue
        S = n.off + NODE_HDR + TM_K2
        try:
            bm_n = _u32(mdl, S + 24)
            bm_at = _u32(mdl, S + 20) + 12
            bonemap = struct.unpack_from(f"<{bm_n}f", mdl, bm_at) if bm_n else ()
            b16 = struct.unpack_from("<16H", mdl, S + 64)
        except (struct.error, IndexError) as exc:
            out.add("bones16", "WARN", f"skin tables are unreadable ({exc})", n.name_lower)
            continue
        problems = []
        used = {}
        for node_idx, v in enumerate(bonemap):
            if v == -1:
                continue
            if v != v or v != int(v) or not 0 <= v < 16:
                problems.append(f"bonemap[{node_idx}] = {v!r} is not -1 or a slot in 0..15")
            else:
                used[int(v)] = node_idx
        for slot, node_idx in sorted(used.items()):
            if not 0 <= b16[slot] < count:
                problems.append(f"slot {slot} names node {b16[slot]}, outside [0, {count})")
            elif b16[slot] != node_idx:
                problems.append(f"slot {slot}: bonemap says node {node_idx}, bones16 says {b16[slot]}")
        if problems:
            out.add("bones16", "WARN", f"{len(problems)} skin slot problems; first: {problems[0]}", n.name_lower)


# ---- T8-lite dangly -----------------------------------------------------------------------

def _check_dangly(out, o, k1, rename):
    have = {n.name_lower for n in o.nodes if n.type & NODE_DANGLY}
    for n in k1.nodes:
        if n.type & NODE_DANGLY:
            name = rename.get(n.name_lower, n.name_lower)
            if name not in have:
                out.add("dangly_demoted", "FAIL",
                        f"K1 dangly node {n.name_lower!r} has no dangly node {name!r} in the output", name)


# ---- Tranche B/C (not implemented) --------------------------------------------------------

def _todo_pose_controllers(out, o, k1, rename):    # T5 pose + controller multiset
    pass


def _todo_triangles(out, o, k1, rename):           # T6 triangles keyed (pos, uv0)
    pass


def _todo_skin_binding(out, o, k1, rename):        # T3 per-vertex skin binding
    pass


# ---- API ----------------------------------------------------------------------------------

def check_model(out_mdl, out_mdx, k1_mdl, k1_mdx, resolve, rename=None):
    """All oracle findings for one converted model. No FAIL finding = trustworthy. `k1_mdx` is reserved for the
    Tranche B/C checks."""
    out_mdl, k1_mdl = bytes(out_mdl), bytes(k1_mdl)
    out_mdx = None if out_mdx is None else bytes(out_mdx)
    try:
        o = sn.parse_tree(out_mdl)
    except (sn.SupernodeError, struct.error) as exc:
        return [OracleFinding("parse", "FAIL", f"output model is unreadable: {exc}", None)]
    try:
        k1 = sn.parse_tree(k1_mdl)
    except (sn.SupernodeError, struct.error) as exc:
        return [OracleFinding("k1_parse", "FAIL", f"K1 source model is unreadable: {exc}", None)]
    rn = _rename_map(o, k1, out_mdl, k1_mdl, rename)
    res = _Out()
    _check_supernodes(res, o, resolve)
    _check_topology(res, o, k1, rn)
    _check_plane(res, out_mdl, out_mdx, o)
    _check_bones16(res, out_mdl, o)
    _check_dangly(res, o, k1, rn)
    _todo_pose_controllers(res, o, k1, rn)
    _todo_triangles(res, o, k1, rn)
    _todo_skin_binding(res, o, k1, rn)
    return res.items


def has_fail(findings):
    return any(f.severity == "FAIL" for f in findings)


# ---- CLI ----------------------------------------------------------------------------------

def _find_ci(directory, filename):
    d = Path(directory).expanduser()
    if not d.is_dir():
        return None
    want = filename.lower()
    return next((f for f in sorted(d.iterdir()) if f.name.lower() == want and f.is_file()), None)


def load_k1(k1_dir, name):
    """(mdl, mdx) bytes of a K1 model. A directory without chitin.key is read as extracted <name>.mdl/.mdx files;
    a real install goes through pykotor (k1port venv python), read only."""
    d = Path(k1_dir).expanduser()
    if not d.is_dir():
        raise OracleError(f"K1 dir not found: {d}")
    if not (d / "chitin.key").is_file():
        mdl, mdx = _find_ci(d, name + ".mdl"), _find_ci(d, name + ".mdx")
        if mdl is None or mdx is None:
            raise OracleError(f"{name}.mdl/.mdx not found in {d} (no chitin.key, read as an extracted-files dir)")
        return mdl.read_bytes(), mdx.read_bytes()
    try:
        import k1_port_model as K
        pk = K.load_pykotor()
        hit = K.Library(pk, d, None).from_install(name)
    except Exception as exc:  # noqa: BLE001  pykotor and install problems all mean "cannot load K1 source"
        raise OracleError(f"K1 model {name}: pykotor lookup failed ({type(exc).__name__}: {exc}); "
                          "run with the k1port venv python") from exc
    if hit is None:
        raise OracleError(f"K1 model {name} not found in {d}")
    return hit[2], hit[3]


def _parse_models(items):
    out = []
    for item in items:
        a, sep, b = item.partition("=")
        if not sep or not a or not b:
            raise OracleError(f"bad --models entry {item!r}: expected K1NAME=K2NAME")
        out.append((a.lower(), b.lower()))
    return out


def _run(args):
    stage_models = Path(args.stage).expanduser() / "models"
    resolve = sn.chain_resolver([stage_models], args.k2)
    report, rc = {}, 0
    for k1_name, k2_name in _parse_models(args.models):
        mdl, mdx = _find_ci(stage_models, k2_name + ".mdl"), _find_ci(stage_models, k2_name + ".mdx")
        if mdl is None or mdx is None:
            raise OracleError(f"converted output {k2_name}.mdl/.mdx not found in {stage_models}")
        k1_mdl, k1_mdx = load_k1(args.k1, k1_name)
        found = check_model(mdl.read_bytes(), mdx.read_bytes(), k1_mdl, k1_mdx, resolve, {k1_name: k2_name})
        report[k2_name] = found
        if has_fail(found):
            rc = 1
    return report, rc


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--k1", required=True, help="K1 install (k1port venv python) or dir of extracted <name>.mdl/.mdx")
    ap.add_argument("--k2", required=True, help="K2 install, read only (supermodel chain)")
    ap.add_argument("--stage", required=True, help="stage dir holding models/<k2name>.mdl|.mdx")
    ap.add_argument("--models", nargs="+", required=True, metavar="K1NAME=K2NAME")
    ap.add_argument("--json", action="store_true")
    try:
        args = ap.parse_args(argv)
    except SystemExit as exc:
        return 2 if exc.code not in (0, None) else 0
    try:
        report, rc = _run(args)
    except (OracleError, sn.SupernodeError, OSError) as exc:
        if args.json:
            print(json.dumps({"status": "error", "error": str(exc), "findings": []}, indent=2))
        else:
            print(f"error: {exc}", file=sys.stderr)
        return 2
    if args.json:
        flat = [{"model": m, **asdict(f)} for m, fs in report.items() for f in fs]
        models = {m: ("fail" if has_fail(fs) else "pass") for m, fs in report.items()}
        print(json.dumps({"status": "fail" if rc else "pass", "models": models, "findings": flat}, indent=2))
    else:
        for m, fs in report.items():
            for f in fs:
                print(f"{f.severity} {m} {f.check}{' [' + f.node + ']' if f.node else ''}: {f.message}")
            print(f"{'FAIL' if has_fail(fs) else 'PASS'} {m}: {sum(f.severity == 'FAIL' for f in fs)} FAIL, "
                  f"{sum(f.severity == 'WARN' for f in fs)} WARN")
    return rc


if __name__ == "__main__":
    sys.exit(main())
