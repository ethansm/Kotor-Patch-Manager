#!/usr/bin/env python3
"""Tests for pykotor_k2_dangly.patch (danglymesh 0x121 reader + writer fix on top of pykotor_k2_mdl.patch).

Spec: .claude/research/pykotor_writer/h4_findings.md. Target layout = vanilla K2 (716/716 dangly nodes):
dangly header, constraints (N floats, at cptr), positions (3N floats, at cptr + 4N), faces (at cptr + 16N);
cptr == end of the node headers, N == vertex count, no "-12" on any pointer.

Run: python3 -u test_k1_port_dangly.py   (or: python3 scripts/run_tests.py k1_port_dangly)
Needs pykotor with BOTH patches (patch_manager_mods/k1_port/setup_venv.sh). When the running interpreter has no
pykotor, the file re-execs itself under K1PORT_PY (default ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python).
Without a patched pykotor every pykotor test prints SKIP and the file exits 0. K1 / K2 installs are read-only
inputs (nothing from them is written into the repo); each test using them SKIPs when the install is absent.
"""
import hashlib
import json
import logging
import os
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
MDL_PATCH = HERE / "pykotor_k2_mdl.patch"
DANGLY_PATCH = HERE / "pykotor_k2_dangly.patch"
SETUP = HERE / "setup_venv.sh"
MARKER = "k1port-dangly-v1"
SKIP_MSG = "run patch_manager_mods/k1_port/setup_venv.sh"
K1PORT_PY = os.environ.get("K1PORT_PY") or os.path.expanduser(
    "~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python")
K1_INSTALL = Path.home() / ".local/share/Steam/steamapps/common/swkotor"
K2_INSTALL = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
K2_DANGLY_MODELS = ("pmhc06", "c_hmedoffh", "n_jorranh", "602danb")  # small vanilla K2 models with 0x121 nodes
TESTS = []

sys.dont_write_bytecode = True


class Skip(Exception):
    """A test that cannot run on this machine. Reported as SKIP, never as FAIL."""


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


# ------------------------------------------------------------------ raw MDL parser (no pykotor)

def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def f32(b, o):
    return struct.unpack_from("<f", b, o)[0]


TRI_K1, TRI_K2, SKIN_HDR, DANGLY_HDR = 332, 340, 100, 28
NODE_MESH, NODE_SKIN, NODE_DANGLY = 0x20, 0x40, 0x100


def raw_walk(b, k2=True):
    """Every geometry node as a dict, in tree order. All values are FILE offsets (stored pointer + 12)."""
    nodes = []

    def rec(off):
        t = u16(b, off)
        n = {"off": off, "type": t, "child_ptr": u32(b, off + 44) + 12, "child_n": u32(b, off + 48),
             "ctrl": u32(b, off + 56) + 12, "ctrl_n": u32(b, off + 60),
             "cdata": u32(b, off + 68) + 12, "cdata_n": u32(b, off + 72)}
        hdr = off + 80
        if t & NODE_MESH:
            tm = hdr
            n.update(tm=tm, faces=u32(b, tm + 8) + 12, face_n=u32(b, tm + 12),
                     ic=u32(b, tm + 176) + 12, io=u32(b, tm + 188) + 12, cnt=u32(b, tm + 200) + 12,
                     mdx_size=u32(b, tm + 252), bitmap=u32(b, tm + 256), vcount=u16(b, tm + 304),
                     verts=u32(b, tm + (TRI_K2 if k2 else TRI_K1) - 4) + 12)
            hdr += TRI_K2 if k2 else TRI_K1
            if t & NODE_SKIN:
                hdr += SKIN_HDR
        if t & NODE_DANGLY:
            n.update(dangly_hdr=hdr, cptr=u32(b, hdr) + 12, N=u32(b, hdr + 4), N2=u32(b, hdr + 8),
                     posptr=(u32(b, hdr + 24) + 12) if u32(b, hdr + 24) else 0, hdr_end=hdr + DANGLY_HDR)
        n["children"] = [u32(b, n["child_ptr"] + 4 * i) + 12 for i in range(n["child_n"])]
        nodes.append(n)
        for c in n["children"]:
            rec(c)

    rec(u32(b, 12 + 40) + 12)
    return nodes


def raw_dangly(b, n):
    N = n["N"]
    return ([f32(b, n["cptr"] + 4 * i) for i in range(N)],
            [struct.unpack_from("<3f", b, n["posptr"] + 12 * i) for i in range(N)])


def layout_problems(b, k2=True):
    """Pointer/layout problems of a written file (empty list = every pointer resolves in order, dangly = vanilla)."""
    bad, size = [], len(b)
    if u32(b, 4) not in (0, size - 12):
        bad.append(f"content length field {u32(b, 4)} != {size - 12}")
    for n in raw_walk(b, k2):
        for c in n["children"]:
            if not 12 <= c < size:
                bad.append(f"child ptr {c} outside the file")
        chain = [("faces", n.get("faces")), ("ic", n.get("ic")), ("verts", n.get("verts")), ("io", n.get("io")),
                 ("cnt", n.get("cnt")), ("ctrl", n["ctrl"] if n["ctrl_n"] else None)]
        last = n["off"]
        for nm, p in chain:
            if p is None:
                continue
            if not 12 <= p <= size:
                bad.append(f"{nm} ptr {p} outside the file")
            elif p < last:
                bad.append(f"{nm} ptr {p} before the previous region ({last})")
            else:
                last = p
        if n["ctrl_n"] and n["cdata_n"]:
            if n["cdata"] + 4 * n["cdata_n"] > size:
                bad.append("controller data runs past the file end")
            if n["ctrl"] + 16 * n["ctrl_n"] > n["cdata"]:
                bad.append("controllers overlap controller data")
        if n["child_n"] and n["child_ptr"] + 4 * n["child_n"] > (n["ctrl"] if n["ctrl_n"] else size):
            bad.append("children array overlaps the controllers")
        if "tm" in n:
            if n["faces"] + 32 * n["face_n"] > n["ic"]:
                bad.append("faces overlap indices_counts")
            if n["verts"] + 12 * n["vcount"] > n["io"]:
                bad.append("vertices overlap indices_offsets")
        if "dangly_hdr" in n and n["N"]:
            if n["cptr"] != n["hdr_end"]:
                bad.append(f"cptr {n['cptr']} != dangly header end {n['hdr_end']}")
            if n["posptr"] != n["cptr"] + 4 * n["N"]:
                bad.append("posptr != cptr + 4N")
            if n["faces"] != n["cptr"] + 16 * n["N"]:
                bad.append("faces != cptr + 16N")
            if n["N"] != n["vcount"] or n["N2"] != n["N"]:
                bad.append(f"N {n['N']}/{n['N2']} != vertex count {n['vcount']}")
    return bad


# ------------------------------------------------------------------ pykotor access

def load_pykotor():
    """Import pykotor and require the dangly patch (marker comment in io_mdl.py). Skip otherwise."""
    logging.disable(logging.CRITICAL)
    try:
        from pykotor.common.misc import Game
        from pykotor.resource.formats.mdl import io_mdl, read_mdl, MDLBinaryWriter
    except ImportError as exc:
        raise Skip(f"pykotor not importable ({exc}); {SKIP_MSG}")
    if MARKER not in Path(io_mdl.__file__).read_text(encoding="utf-8"):
        raise Skip(f"pykotor at {io_mdl.__file__} lacks {DANGLY_PATCH.name}; {SKIP_MSG}")
    return Game, read_mdl, MDLBinaryWriter, io_mdl


# Builds synthetic models. Executed in-process (after load_pykotor) and in the T5 subprocesses.
SYNTH_SRC = r'''
from pathlib import Path
from pykotor.common.misc import Game
from pykotor.resource.formats.mdl import MDL, MDLNode, MDLMesh, MDLFace, read_mdl, MDLBinaryWriter
from pykotor.resource.formats.mdl.mdl_data import MDLConstraint, MDLDangly, Vector2, Vector3
from pykotor.resource.formats.mdl.mdl_types import MDLNodeType

N_DANGLY = 6
CONSTRAINTS = [235.0, 235.0, 0.0, 0.0, 233.5, 12.25]          # exact in float32 and in the int(v*1e6) encoding
REST_OFFSET = (0.0, 0.0, 0.25)                                  # rest positions != vertex positions on purpose


def _mesh_fields(mesh, n, dx):
    mesh.texture_1 = "dgl_tex"
    mesh.vertex_positions = [Vector3(dx + i * 0.5, (i % 2) * 1.0, i * 0.125) for i in range(n)]
    mesh.vertex_normals = [Vector3(0, 0, 1)] * n
    mesh.vertex_uv1 = [Vector2(i / n, (i % 2) * 1.0) for i in range(n)]
    faces = []
    for a in range(n - 2):
        f = MDLFace()
        f.v1, f.v2, f.v3 = a, a + 1, a + 2
        faces.append(f)
    mesh.faces = faces


def make_synth(with_dangly=True, with_plain=True):
    """root dummy -> [plain trimesh] + [danglymesh with N_DANGLY vertices and constraints] + [trailing plain trimesh]."""
    mdl = MDL()
    mdl.name = "dgl_test"
    mdl.root.name = "dgl_test"
    if with_plain:
        node = MDLNode()
        node.name = "plain_a"
        node.node_type = MDLNodeType.TRIMESH
        node.mesh = MDLMesh()
        _mesh_fields(node.mesh, 5, 0.0)
        mdl.root.children.append(node)
    if with_dangly:
        node = MDLNode()
        node.name = "hair"
        node.node_type = MDLNodeType.DANGLYMESH
        d = MDLDangly()
        _mesh_fields(d, N_DANGLY, 10.0)
        d.displacement, d.tightness, d.period = 0.5, 20.0, 3.5
        for v in CONSTRAINTS:
            c = MDLConstraint()
            c.type = int(v * 1000000) if v != 0.0 else 0
            d.constraints.append(c)
        d.verts_original = [Vector3(p.x + REST_OFFSET[0], p.y + REST_OFFSET[1], p.z + REST_OFFSET[2])
                            for p in d.vertex_positions]
        d.verts = list(d.verts_original)
        node.dangly = d
        node.mesh = d
        mdl.root.children.append(node)
    if with_plain:
        node = MDLNode()
        node.name = "plain_b"
        node.node_type = MDLNodeType.TRIMESH
        node.mesh = MDLMesh()
        _mesh_fields(node.mesh, 4, 20.0)
        mdl.root.children.append(node)
    return mdl


def write_k2(mdl, out_dir, base):
    mdl_path, mdx_path = Path(out_dir) / (base + ".mdl"), Path(out_dir) / (base + ".mdx")
    w = MDLBinaryWriter(mdl, mdl_path, mdx_path)
    w.force_game = Game.K2
    w.write()
    return mdl_path.read_bytes(), mdx_path.read_bytes()


def walk_nodes(n):
    yield n
    for c in n.children:
        yield from walk_nodes(c)


def read_back(mdl_bytes, mdx_bytes):
    return read_mdl(mdl_bytes, source_ext=mdx_bytes, size_ext=len(mdx_bytes))
'''


def synth_ns():
    load_pykotor()
    ns = {}
    exec(compile(SYNTH_SRC, "<synth>", "exec"), ns)  # noqa: S102 - our own fixture source
    return ns


def cons_values(dangly):
    return [c.type / 1000000.0 for c in dangly.constraints]


def vec_tuples(vs):
    return [(v.x, v.y, v.z) for v in vs]


def f32r(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]


def vec_f32(vs):
    return [tuple(f32r(c) for c in v) for v in vec_tuples(vs)]


# ------------------------------------------------------------------ pure tests (no pykotor)

@test
def test_patch_files_and_marker():
    for p in (MDL_PATCH, DANGLY_PATCH, SETUP):
        check(p.is_file(), f"missing {p}")
    text = DANGLY_PATCH.read_text(encoding="utf-8")
    check(MARKER in text, f"{DANGLY_PATCH.name} lacks the marker {MARKER}")
    files = [ln.split()[1] for ln in text.splitlines() if ln.startswith("+++ ")]
    check(files == ["b/pykotor/resource/formats/mdl/io_mdl.py"], f"unexpected files in the patch: {files}")


@test
def test_setup_venv_applies_both_patches():
    r = subprocess.run(["bash", "-n", str(SETUP)], capture_output=True, text=True)
    check(r.returncode == 0, f"bash -n failed: {r.stderr}")
    text = SETUP.read_text(encoding="utf-8")
    check("pykotor_k2_mdl.patch" in text and "pykotor_k2_dangly.patch" in text, "setup_venv.sh must name both patches")
    check(text.count("patch -p1 --dry-run -R") >= 2, "each patch needs its own 'already applied' detection")


# ------------------------------------------------------------------ pykotor tests

@test
def test_t1_layout_synthetic_vanilla_order():
    ns = synth_ns()
    with tempfile.TemporaryDirectory() as td:
        mdl_b, _mdx_b = ns["write_k2"](ns["make_synth"](), td, "dgl")
    problems = layout_problems(mdl_b)
    check(not problems, f"layout problems: {problems}")
    dn = [n for n in raw_walk(mdl_b) if n["type"] & NODE_DANGLY]
    check(len(dn) == 1, f"expected 1 dangly node, got {len(dn)}")
    n = dn[0]
    N = ns["N_DANGLY"]
    check(n["N"] == N and n["N2"] == N and n["vcount"] == N, f"N {n['N']}/{n['N2']} vcount {n['vcount']}")
    check(n["cptr"] == n["hdr_end"], "constraints must start right after the dangly header")
    check(n["posptr"] == n["cptr"] + 4 * N, "positions pointer must be cptr + 4N")
    check(n["faces"] == n["cptr"] + 16 * N, "faces pointer must be cptr + 16N")
    check(n["type"] == 0x121, f"node type {n['type']:#x}")


@test
def test_t3_round_trip_synthetic():
    ns = synth_ns()
    src = ns["make_synth"]()
    with tempfile.TemporaryDirectory() as td:
        mdl_b, mdx_b = ns["write_k2"](src, td, "dgl")
    # raw bytes
    n = [x for x in raw_walk(mdl_b) if x["type"] & NODE_DANGLY][0]
    cons, pos = raw_dangly(mdl_b, n)
    check(cons == [f32r(v) for v in ns["CONSTRAINTS"]], f"raw constraints {cons}")
    want_pos = vec_f32(src.root.children[1].dangly.verts_original)
    check([tuple(p) for p in pos] == want_pos, f"raw positions {pos}")
    check(want_pos != vec_f32(src.root.children[1].dangly.vertex_positions), "fixture must use rest != vertex positions")
    # patched reader
    back = ns["read_back"](mdl_b, mdx_b)
    hair = [x for x in ns["walk_nodes"](back.root) if x.dangly is not None]
    check(len(hair) == 1, "dangly node lost on read-back")
    d = hair[0].dangly
    check(cons_values(d) == ns["CONSTRAINTS"], f"read constraints {cons_values(d)}")
    check(vec_tuples(d.verts_original) == want_pos, f"read positions {vec_tuples(d.verts_original)}")
    check(vec_tuples(d.verts) == want_pos, "verts must mirror verts_original")
    check((d.displacement, d.tightness, d.period) == (0.5, 20.0, 3.5), "dangly floats changed")
    # second generation: rewrite what the reader returned, bytes must be stable
    with tempfile.TemporaryDirectory() as td:
        mdl2, _ = ns["write_k2"](back, td, "dgl2")
    n2 = [x for x in raw_walk(mdl2) if x["type"] & NODE_DANGLY][0]
    check(raw_dangly(mdl2, n2) == (cons, pos), "dangly blocks changed on the second generation")
    check(not layout_problems(mdl2), f"2nd generation layout: {layout_problems(mdl2)}")


@test
def test_positions_fall_back_to_vertex_positions():
    ns = synth_ns()
    src = ns["make_synth"]()
    d = src.root.children[1].dangly
    d.verts_original, d.verts = [], []
    with tempfile.TemporaryDirectory() as td:
        mdl_b, _ = ns["write_k2"](src, td, "dgl")
    n = [x for x in raw_walk(mdl_b) if x["type"] & NODE_DANGLY][0]
    _cons, pos = raw_dangly(mdl_b, n)
    check([tuple(p) for p in pos] == vec_f32(d.vertex_positions), "fallback must use the mesh vertex positions")


# T5 driver: runs in a subprocess under a chosen PYTHONPATH and prints one JSON line.
T5_DRIVER = r'''
import hashlib, json, logging, sys, tempfile
logging.disable(logging.CRITICAL)
import pykotor
''' + SYNTH_SRC + r'''
out = {"pykotor": pykotor.__file__}
with tempfile.TemporaryDirectory() as td:
    for key, kw in (("plain", dict(with_dangly=False)), ("dangly", dict(with_dangly=True))):
        m, x = write_k2(make_synth(**kw), td, key)
        out[key] = {"mdl": hashlib.md5(m).hexdigest(), "mdx": hashlib.md5(x).hexdigest(), "mdl_len": len(m),
                    "mdx_len": len(x)}
        Path(td, key + ".mdl").write_bytes(m)
        if key == "dangly":
            out["dangly_mdl_hex"] = m.hex()
    for name in sys.argv[1:]:   # optional extra files: K1 MDL/MDX pairs given as <mdl>:<mdx>
        mp, xp = name.split(":")
        mb, xb = Path(mp).read_bytes(), Path(xp).read_bytes()
        m, x = write_k2(read_mdl(mb, source_ext=xb, size_ext=len(xb)), td, "extra")
        out["extra:" + Path(mp).name] = {"mdl": hashlib.md5(m).hexdigest(), "mdx": hashlib.md5(x).hexdigest()}
print("JSON " + json.dumps(out))
'''


def run_driver(pythonpath, extra=()):
    env = dict(os.environ)
    env["PYTHONPATH"] = str(pythonpath)
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    r = subprocess.run([sys.executable, "-c", T5_DRIVER, *extra], capture_output=True, text=True, env=env, timeout=240)
    lines = [ln for ln in r.stdout.splitlines() if ln.startswith("JSON ")]
    check(r.returncode == 0 and lines, f"driver failed rc={r.returncode}: {r.stderr[-600:]}")
    return json.loads(lines[-1][5:])


def unpatched_dangly_tree(tmp):
    """A copy of the running pykotor with pykotor_k2_dangly.patch reversed (the pre-fix behaviour)."""
    load_pykotor()
    import pykotor
    pkg = Path(pykotor.__file__).resolve().parent
    if shutil.which("patch") is None:
        raise Skip("'patch' not installed")
    dst = Path(tmp) / "before"
    shutil.copytree(pkg, dst / "pykotor", ignore=shutil.ignore_patterns("__pycache__"))
    r = subprocess.run(["patch", "-p1", "-R", "--no-backup-if-mismatch", "-d", str(dst), "-i", str(DANGLY_PATCH)],
                       capture_output=True, text=True)
    check(r.returncode == 0, f"cannot reverse the dangly patch on a copy: {r.stdout[-400:]}{r.stderr[-400:]}")
    return dst


@test
def test_t5_regression_non_dangly_bytes_identical():
    load_pykotor()
    import pykotor
    with tempfile.TemporaryDirectory() as td:
        before_tree = unpatched_dangly_tree(td)
        after_tree = Path(pykotor.__file__).resolve().parent.parent
        extra = []
        for name in ("p_missionh", "p_carthbb"):  # real K1 models without dangly nodes (K1 install optional)
            pair = k1_files(name, td)
            if pair:
                extra.append(f"{pair[0]}:{pair[1]}")
        a = run_driver(before_tree, extra)
        b = run_driver(after_tree, extra)
    check("before" in a["pykotor"], f"driver used the wrong tree: {a['pykotor']}")
    check(a["plain"] == b["plain"], f"non-dangly synthetic bytes changed: {a['plain']} vs {b['plain']}")
    for k in [k for k in a if k.startswith("extra:")]:
        check(a[k] == b[k], f"{k}: output changed by the dangly patch")
    # dangly node: MDX untouched; MDL grows by exactly 16N (positions 12N + nothing else; constraints moved)
    check(a["dangly"]["mdx"] == b["dangly"]["mdx"], "dangly MDX bytes changed")
    check(b["dangly"]["mdl_len"] - a["dangly"]["mdl_len"] == 12 * 6,
          f"expected the MDL to grow by the 12N positions block, got {b['dangly']['mdl_len'] - a['dangly']['mdl_len']}")
    n_before = [n for n in raw_walk(bytes.fromhex(a["dangly_mdl_hex"])) if n["type"] & NODE_DANGLY][0]
    n_after = [n for n in raw_walk(bytes.fromhex(b["dangly_mdl_hex"])) if n["type"] & NODE_DANGLY][0]
    check((n_before["mdx_size"], n_before["bitmap"]) == (n_after["mdx_size"], n_after["bitmap"]),
          "dangly MDX size / bitmap changed")
    check(not layout_problems(bytes.fromhex(b["dangly_mdl_hex"])), "patched layout has pointer problems")
    check(layout_problems(bytes.fromhex(a["dangly_mdl_hex"])), "sanity: the unpatched tree should show layout problems")
    print(f"  {len(extra)} real K1 models compared byte for byte; synthetic plain model identical")


@test
def test_t5_non_dangly_neighbour_pointers_after_dangly_node():
    # plain_a before and plain_b after the dangly node: children and controller pointers still resolve
    ns = synth_ns()
    with tempfile.TemporaryDirectory() as td:
        mdl_b, mdx_b = ns["write_k2"](ns["make_synth"](), td, "dgl")
    check(not layout_problems(mdl_b), f"{layout_problems(mdl_b)}")
    nodes = raw_walk(mdl_b)
    check(len(nodes) == 4, f"expected root + 3 meshes, got {len(nodes)}")
    root = nodes[0]
    check(len(root["children"]) == 3 and all(c in {n["off"] for n in nodes} for c in root["children"]),
          "root children pointers do not hit node starts")
    back = ns["read_back"](mdl_b, mdx_b)
    check([c.name for c in back.root.children] == ["plain_a", "hair", "plain_b"], "node order lost")
    for c in back.root.children:
        check(len(c.mesh.vertex_positions) == {"plain_a": 5, "hair": 6, "plain_b": 4}[c.name], f"{c.name} vertex count")


# ------------------------------------------------------------------ real data

def k1_files(name, tmp):
    """Writes <name>.mdl/.mdx from the K1 install into tmp and returns their paths, or None when absent."""
    pair = k1_bytes(name)
    if pair is None:
        return None
    mp, xp = Path(tmp) / f"{name}.mdl", Path(tmp) / f"{name}.mdx"
    mp.write_bytes(pair[0])
    xp.write_bytes(pair[1])
    return mp, xp


_INST = {}


def install_bytes(install, name):
    if not (install / "chitin.key").is_file():
        return None
    try:
        from pykotor.extract.installation import Installation
        from pykotor.resource.type import ResourceType
    except ImportError:
        return None
    inst = _INST.get(str(install))
    if inst is None:
        inst = _INST[str(install)] = Installation(str(install))
    mdl, mdx = inst.resource(name, ResourceType.MDL), inst.resource(name, ResourceType.MDX)
    if mdl is None or mdx is None:
        return None
    return bytes(mdl.data), bytes(mdx.data)


def k1_bytes(name):
    return install_bytes(K1_INSTALL, name)


def write_back(Game, read_mdl, MDLBinaryWriter, mdl_b, mdx_b, td, base):
    mdl = read_mdl(mdl_b, source_ext=mdx_b, size_ext=len(mdx_b))
    mp, xp = Path(td) / (base + ".mdl"), Path(td) / (base + ".mdx")
    w = MDLBinaryWriter(mdl, mp, xp)
    w.force_game = Game.K2
    w.write()
    return mdl, mp.read_bytes(), xp.read_bytes()


def walk_mdl(n):
    yield n
    for c in n.children:
        yield from walk_mdl(c)


@test
def test_t2_t3_k1_carth_head_reader_and_round_trip():
    Game, read_mdl, Writer, _ = load_pykotor()
    pair = k1_bytes("p_carthh")
    if pair is None:
        raise Skip(f"p_carthh not found in the K1 install at {K1_INSTALL}")
    mdl_b, mdx_b = pair
    raw = [n for n in raw_walk(mdl_b, k2=False) if n["type"] & NODE_DANGLY]
    check(len(raw) == 1 and raw[0]["N"] == 40, f"expected one K1 dangly node with N=40, got {[n['N'] for n in raw]}")
    rc, rp = raw_dangly(mdl_b, raw[0])
    # T2 reader value oracle: raw K1 bytes at the stored pointer == what the reader returns
    mdl = read_mdl(mdl_b, source_ext=mdx_b, size_ext=len(mdx_b))
    hair = [n for n in walk_mdl(mdl.root) if n.dangly is not None]
    check(len(hair) == 1 and hair[0].name == "hair", "reader lost the hair node")
    d = hair[0].dangly
    check(cons_values(d) == [f32r(v) for v in rc] or cons_values(d) == rc, f"reader constraints {cons_values(d)[:6]} vs raw {rc[:6]}")
    check(vec_tuples(d.verts_original) == [tuple(p) for p in rp], "reader positions differ from the raw K1 positions")
    check(rc[:6] == [235.0, 235.0, 0.0, 0.0, 233.0, 233.0], f"unexpected K1 constraints {rc[:6]}")
    # T3 round trip K1 -> K2 writer -> raw parse
    with tempfile.TemporaryDirectory() as td:
        _m, out_mdl, out_mdx = write_back(Game, read_mdl, Writer, mdl_b, mdx_b, td, "carthh_k2")
    check(not layout_problems(out_mdl), f"K2 output layout: {layout_problems(out_mdl)}")
    n = [x for x in raw_walk(out_mdl) if x["type"] & NODE_DANGLY]
    check(len(n) == 1 and n[0]["N"] == 40, "dangly node lost or resized by the K2 writer")
    wc, wp = raw_dangly(out_mdl, n[0])
    check(wc == rc, f"written constraints differ from K1 (first diffs {[(i, a, b) for i, (a, b) in enumerate(zip(wc, rc)) if a != b][:4]})")
    check(wp == rp, "written positions differ from K1 index for index")
    check(n[0]["bitmap"] == 0x23 and n[0]["mdx_size"] == 32, f"dangly MDX layout changed: {n[0]['bitmap']:#x}/{n[0]['mdx_size']}")


@test
def test_k1_mission_head_dangly_report():
    Game, read_mdl, Writer, _ = load_pykotor()
    pair = k1_bytes("p_missionh")
    if pair is None:
        raise Skip(f"p_missionh not found in the K1 install at {K1_INSTALL}")
    mdl_b, mdx_b = pair
    raw = [n for n in raw_walk(mdl_b, k2=False) if n["type"] & NODE_DANGLY]
    print(f"  p_missionh dangly nodes in the K1 file: {len(raw)} (N={[n['N'] for n in raw]})")
    with tempfile.TemporaryDirectory() as td:
        _m, out_mdl, _x = write_back(Game, read_mdl, Writer, mdl_b, mdx_b, td, "missionh_k2")
    check(not layout_problems(out_mdl), f"K2 output layout: {layout_problems(out_mdl)}")
    out = [n for n in raw_walk(out_mdl) if n["type"] & NODE_DANGLY]
    check(len(out) == len(raw), "dangly node count changed")
    for a, b in zip(raw, out):
        check(raw_dangly(mdl_b, a) == raw_dangly(out_mdl, b), "dangly blocks differ after the K2 write")


@test
def test_vanilla_k2_layout_and_round_trip():
    Game, read_mdl, Writer, _ = load_pykotor()
    if not (K2_INSTALL / "chitin.key").is_file():
        raise Skip(f"K2 install not found at {K2_INSTALL}")
    done = 0
    for name in K2_DANGLY_MODELS:
        pair = install_bytes(K2_INSTALL, name)
        if pair is None:
            continue
        mdl_b, mdx_b = pair
        van = [n for n in raw_walk(mdl_b) if n["type"] & NODE_DANGLY]
        if not van:
            continue
        for n in van:  # vanilla layout is the target: cptr == header end, posptr == cptr+4N, faces == cptr+16N
            check(n["cptr"] == n["hdr_end"] and n["posptr"] == n["cptr"] + 4 * n["N"]
                  and n["faces"] == n["cptr"] + 16 * n["N"] and n["N"] == n["vcount"], f"{name}: vanilla layout assumption broke")
        with tempfile.TemporaryDirectory() as td:
            mdl, out_mdl, _x = write_back(Game, read_mdl, Writer, mdl_b, mdx_b, td, name)
        check(not layout_problems(out_mdl), f"{name}: written layout {layout_problems(out_mdl)[:3]}")
        out = [n for n in raw_walk(out_mdl) if n["type"] & NODE_DANGLY]
        check(len(out) == len(van), f"{name}: dangly nodes {len(van)} -> {len(out)}")
        for a, b in zip(van, out):
            ca, pa = raw_dangly(mdl_b, a)
            cb, pb = raw_dangly(out_mdl, b)
            check(pa == pb, f"{name}: positions differ after read+write")
            # constraints go through the int(v*1e6)/1e6 encoding, so allow a 1e-6 absolute error
            check(all(abs(x - y) <= 1e-6 for x, y in zip(ca, cb)) and len(ca) == len(cb),
                  f"{name}: constraints differ beyond 1e-6")
            check(a["mdx_size"] == b["mdx_size"] and a["bitmap"] == b["bitmap"], f"{name}: MDX layout changed")
            ha = mdl_b[a["dangly_hdr"] + 4:a["dangly_hdr"] + 24]
            hb = out_mdl[b["dangly_hdr"] + 4:b["dangly_hdr"] + 24]
            check(ha == hb, f"{name}: dangly header counts/floats changed")
        done += 1
    check(done >= 3, f"only {done} vanilla K2 dangly models available (need 3)")
    print(f"  {done} vanilla K2 models: layout, positions and constraints match after read+write")


def reexec_if_needed():
    """Run under the pykotor venv when the current interpreter has no pykotor (e.g. via scripts/run_tests.py)."""
    if os.environ.get("K1PORT_DANGLY_REEXEC") == "1":
        return
    try:
        import pykotor  # noqa: F401
        return
    except ImportError:
        pass
    if Path(K1PORT_PY).is_file() and os.path.abspath(K1PORT_PY) != os.path.abspath(sys.executable):
        env = dict(os.environ, K1PORT_DANGLY_REEXEC="1", PYTHONDONTWRITEBYTECODE="1")
        os.execve(K1PORT_PY, [K1PORT_PY, "-u", str(Path(__file__).resolve()), *sys.argv[1:]], env)


def main() -> int:
    reexec_if_needed()
    passed = skipped = failed = 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}", flush=True)
            passed += 1
        except Skip as e:
            print(f"SKIP {fn.__name__}: {e}", flush=True)
            skipped += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {str(e)[:600]}", flush=True)
            failed += 1
    print(f"{passed}/{len(TESTS)} PASS ({skipped} SKIP, {failed} FAIL)", flush=True)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
