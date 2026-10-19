#!/usr/bin/env python3
"""Tests for k1_port_mdl_oracle.py. Synthetic MDL/MDX fixtures are built in-test (writes only go to a tempdir).
Calibration tests read the K2 install, the local MDLOps/pykotor stage dirs and the extracted K1 sources under
mdlops_t7 read-only, and SKIP when absent. The K1 install loader test needs pykotor: run it with the k1port venv
python (~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python); everything else runs on any python3.
Run: python3 -u test_k1_port_mdl_oracle.py   (exit 0 = all PASS or SKIP)
"""
import contextlib
import io
import json
import mmap
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_mdl_oracle.py"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_mdl_oracle as O  # noqa: E402
import k1_port_mdl_supernode as sn  # noqa: E402
import test_k1_port_mdl_supernode as T  # noqa: E402  (sibling: synthetic tree builder, K2Bif, stage paths)

WORK = Path.home() / "Games/KOTOR2_Mod_Workspace/mdlops_t7"
K1_INSTALL = Path.home() / ".local/share/Steam/steamapps/common/swkotor"
STAGE = T.STAGE
TESTS = []


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


# ---- synthetic builder: tree + trimesh/skin blocks + MDX -----------------------------------------------

TM = 340
SKIN_SIZE = 100


class M:
    """Node spec. type: 1 base, 0x21 mesh, 0x61 skin mesh, 0x121 dangly mesh. sn defaults to idx (no supermodel).
    faces: (nx, ny, nz, d, v1, v2, v3); verts: (x, y, z); bonemap: floats (one per node); bones16: 16 ints."""

    def __init__(self, name, kids=(), type=1, sn=None, idx=None, verts=(), faces=(), bonemap=None, bones16=None):
        self.name, self.kids, self.type, self.sn, self.idx = name, list(kids), type, sn, idx
        self.verts, self.faces, self.bonemap, self.bones16 = list(verts), list(faces), bonemap, bones16


def _pre(n):
    yield n
    for k in n.kids:
        yield from _pre(k)


def build(root, model_name="model", super_name=None, total=None):
    """(mdl, mdx) bytes. Header 208 bytes, stored pointers = file offset - 12, mesh nodes carry a K2 trimesh header."""
    buf, mdx = bytearray(208), bytearray()

    def alloc(n):
        while len(buf) % 4:
            buf.append(0)
        off = len(buf)
        buf.extend(bytes(n))
        return off

    geo = list(_pre(root))
    for i, n in enumerate(geo):
        if n.idx is None:
            n.idx = i
        if n.sn is None:
            n.sn = n.idx
    size = max(n.idx for n in geo) + 1
    table = {n.idx: n.name for n in geo}
    name_off = []
    for i in range(size):
        s = table.get(i, f"filler{i}")
        o = alloc(len(s) + 1)
        buf[o:o + len(s)] = s.encode("latin1")
        name_off.append(o)
    arr = alloc(4 * size)
    for i in range(size):
        struct.pack_into("<I", buf, arr + 4 * i, name_off[i] - 12)
    struct.pack_into("<I", buf, 196, arr - 12)
    struct.pack_into("<II", buf, 200, size, size)

    def emit(n):
        mesh, skin = bool(n.type & 0x20), bool(n.type & 0x40)
        off = alloc(80 + (TM if mesh else 0) + (SKIN_SIZE if mesh and skin else 0))
        struct.pack_into("<HHH", buf, off, n.type, n.sn, n.idx)
        if mesh:
            T0 = off + 80
            fo = alloc(32 * len(n.faces)) if n.faces else 0
            for i, (nx, ny, nz, d, a, b, c) in enumerate(n.faces):
                struct.pack_into("<4fI3H3H", buf, fo + 32 * i, nx, ny, nz, d, 0, 0, 0, 0, a, b, c)
            if n.faces:
                struct.pack_into("<I", buf, T0 + 8, fo - 12)
            struct.pack_into("<II", buf, T0 + 12, len(n.faces), len(n.faces))
            struct.pack_into("<i", buf, T0 + 252, 12)
            struct.pack_into("<I", buf, T0 + 256, 1)
            struct.pack_into("<i", buf, T0 + 260, 0)
            struct.pack_into("<H", buf, T0 + 304, len(n.verts))
            struct.pack_into("<I", buf, T0 + TM - 8, len(mdx))
            for v in n.verts:
                mdx.extend(struct.pack("<3f", *v))
            if skin:
                S = T0 + TM
                bm = n.bonemap if n.bonemap is not None else []
                bo = alloc(4 * len(bm)) if bm else 0
                for i, v in enumerate(bm):
                    struct.pack_into("<f", buf, bo + 4 * i, v)
                if bm:
                    struct.pack_into("<I", buf, S + 20, bo - 12)
                struct.pack_into("<I", buf, S + 24, len(bm))
                struct.pack_into("<16H", buf, S + 64, *(n.bones16 or [0] * 16))
        kids = [emit(k) for k in n.kids]
        if kids:
            ca = alloc(4 * len(kids))
            for i, ko in enumerate(kids):
                struct.pack_into("<I", buf, ca + 4 * i, ko - 12)
            struct.pack_into("<I", buf, off + 44, ca - 12)
            struct.pack_into("<II", buf, off + 48, len(kids), len(kids))
        return off

    root_off = emit(root)
    struct.pack_into("<I", buf, 52, root_off - 12)
    struct.pack_into("<i", buf, 56, len(geo) if total is None else total)
    buf[OFF_NAME:OFF_NAME + 32] = model_name.encode("latin1").ljust(32, b"\0")
    buf[148:180] = (super_name if super_name is not None else "NULL").encode("latin1").ljust(32, b"\0")
    struct.pack_into("<I", buf, 4, len(buf) - 12)
    struct.pack_into("<I", buf, 8, len(mdx))
    return bytes(buf), bytes(mdx)


OFF_NAME = 20
GOOD_VERTS = [(1, 2, 3), (4, 5, 6), (7, 8, 9)]
GOOD_FACE = (0, 0, 1, -3.0, 0, 1, 2)       # n.v1 = 3, d = -3


def sup_mdl():
    """Supermodel root(0) -> a(5) -> a1(7), b(9); total 40 (non-trivial chain numbers)."""
    return T.sup_model()


def resolver(**models):
    def resolve(name):
        if name not in models:
            raise FileNotFoundError(name)
        return models[name]
    return resolve


def tree4(a1_type=1, **kw):
    """root -> a -> a1 ; root -> b   (chain numbering against sup_mdl(): 0, 5, 7, 9, total 4 + 40 + 1)."""
    return M("root", [M("a", [M("a1", type=a1_type, sn=7, **kw)], sn=5), M("b", sn=9)], sn=0)


def chain_out(**kw):
    return build(tree4(**kw), super_name="sup", total=45)


def checks(findings, severity=None):
    return sorted(f.check for f in findings if severity is None or f.severity == severity)


def run_oracle(out, k1, rename=None, **models):
    return O.check_model(out[0], out[1], k1[0], k1[1], resolver(**models), rename)


# ---- clean pass ----------------------------------------------------------------------------------------

@test
def test_clean_model_has_no_findings():
    root = M("r", [M("m", type=0x21, verts=GOOD_VERTS, faces=[GOOD_FACE]), M("s", type=0x61, bonemap=[-1, -1, 0],
                                                                          bones16=[2] + [0] * 15)])
    m = build(root)
    check(run_oracle(m, m) == [], "unexpected findings on a clean no-supermodel model")


@test
def test_chain_numbered_output_passes_sn():
    out = chain_out()
    k1 = build(tree4())
    f = run_oracle(out, k1, sup=sup_mdl())
    check(f == [], [(x.check, x.message) for x in f])


# ---- T2 supernode --------------------------------------------------------------------------------------

@test
def test_sn_skin_mismatch_is_fail_nonskin_is_warn():
    wrong = M("root", [M("a", [M("a1", type=0x61, sn=2, bonemap=[], bones16=None)], sn=1), M("b", sn=3)], sn=0)
    f = run_oracle(build(wrong, super_name="sup", total=45), build(tree4()), sup=sup_mdl())
    check(checks(f, "FAIL") == ["sn_skin"], checks(f))
    check(checks(f, "WARN") == ["sn_nonskin", "sn_nonskin"], checks(f))
    check({x.node for x in f if x.check == "sn_skin"} == {"a1"}, f)


@test
def test_sn_total_mismatch_is_fail():
    f = run_oracle(build(tree4(), super_name="sup", total=4), build(tree4()), sup=sup_mdl())
    check(checks(f, "FAIL") == ["sn_total"], checks(f))
    check("45" in f[0].message and "4" in f[0].message, f[0].message)


@test
def test_sn_nonskin_only_mismatch_is_not_a_fail():
    out = M("root", [M("a", [M("a1", sn=7)], sn=5), M("b", sn=1)], sn=0)
    f = run_oracle(build(out, super_name="sup", total=45), build(tree4()), sup=sup_mdl())
    check(checks(f) == ["sn_nonskin"] and not O.has_fail(f), checks(f))


@test
def test_sn_missing_supermodel_is_fail_not_exception():
    f = run_oracle(chain_out(), build(tree4()))
    check(checks(f, "FAIL") == ["sn_error"], checks(f))


@test
def test_sn_no_supermodel_uses_own_index():
    root = M("r", [M("a", sn=9)])
    f = run_oracle(build(root, total=2), build(M("r", [M("a")])))
    check(checks(f) == ["sn_nonskin"], checks(f))


# ---- T1 topology + rename ------------------------------------------------------------------------------

@test
def test_topo_names_fail_on_different_name_and_on_count():
    k1 = build(M("r", [M("a"), M("b")]))
    f = run_oracle(build(M("r", [M("a"), M("c")])), k1)
    check(checks(f, "FAIL") == ["topo_names"] and "'b'" in f[0].message and "'c'" in f[0].message, f)
    f = run_oracle(build(M("r", [M("a")])), k1)
    check("topo_names" in checks(f, "FAIL"), f)


@test
def test_topo_children_fail_when_structure_differs():
    k1 = build(M("r", [M("a", [M("x")]), M("b")]))                 # pre-order r a x b
    f = run_oracle(build(M("r", [M("a"), M("x"), M("b")])), k1)      # same pre-order names, different tree
    check(checks(f, "FAIL") == ["topo_children"] and "#0" in f[0].message, checks(f))


@test
def test_topo_index_is_warn_only():
    out = build(M("r", [M("a", idx=2), M("b", idx=1)]))
    f = run_oracle(out, build(M("r", [M("a"), M("b")])))
    check(checks(f) == ["topo_index"] and f[0].severity == "WARN" and not O.has_fail(f), checks(f))


@test
def test_root_rename_is_derived_from_model_names():
    k1 = build(M("P_X", [M("child")]), model_name="P_X")
    out = build(M("k1p_x", [M("child")]), model_name="k1p_x")
    check(run_oracle(out, k1) == [], "root rename should be derived")
    out2 = build(M("k1p_x", [M("child")]), model_name="other")           # root no longer carries the model name
    check(checks(run_oracle(out2, k1), "FAIL") == ["topo_names"], "no derivation when the output root != model")


@test
def test_explicit_rename_map_applies_case_insensitively():
    k1 = build(M("r", [M("Hand_L")]))
    out = build(M("r", [M("lhand")]))
    check(checks(run_oracle(out, k1), "FAIL") == ["topo_names"], "needs the map")
    check(run_oracle(out, k1, rename={"HAND_L": "LHand"}) == [], "map keys and values are lowercased")


# ---- T7 plane ------------------------------------------------------------------------------------------

def mesh_out(face, verts=GOOD_VERTS, mdx_keep=True):
    root = M("r", [M("m", type=0x21, verts=verts, faces=[face])])
    mdl, mdx = build(root)
    return (mdl, mdx if mdx_keep else None), build(M("r", [M("m", type=0x21)]))


@test
def test_plane_good_and_tolerance_scales_with_d():
    out, k1 = mesh_out(GOOD_FACE)
    check(run_oracle(out, k1) == [], "good plane")
    big = (0, 0, 1, -1000.0, 0, 1, 2)                 # n.v1 = 3 -> error 997, far off
    check(checks(run_oracle(*mesh_out(big)), "FAIL") == ["plane"], "far off plane must fail")
    near = (0, 0, 1, -3.0 - 0.0009, 0, 1, 2)          # error 0.0009 <= 1e-3 * max(1, |d|)
    check(run_oracle(*mesh_out(near)) == [], "within tolerance")
    off = (0, 0, 1, -3.0 - 0.01, 0, 1, 2)             # error 0.01 > 1e-3 * 3.01
    check(checks(run_oracle(*mesh_out(off)), "FAIL") == ["plane"], "just outside tolerance")
    huge_d = (0, 0, 1, -3.0 - 0.5, 0, 1, 2)
    big_verts = [(1, 2, 3.5 + 0.5 - 0.0), (4, 5, 6), (7, 8, 9)]   # n.v1 = 4.0, d = -3.5 -> error 0.5
    check(checks(run_oracle(*mesh_out(huge_d, big_verts)), "FAIL") == ["plane"], "error 0.5 on |d|=3.5 fails")
    d1000 = (0, 0, 1, -1000.0, 0, 1, 2)
    v1000 = [(0, 0, 999.5), (4, 5, 6), (7, 8, 9)]     # error 0.5 <= 1e-3 * 1000
    check(run_oracle(*mesh_out(d1000, v1000)) == [], "relative tolerance at large d")


@test
def test_plane_bad_vertex_index_and_nan_fail():
    f = run_oracle(*mesh_out((0, 0, 1, -3.0, 7, 1, 2)))
    check(checks(f, "FAIL") == ["plane"] and "vertex index 7" in f[0].message, f)
    f = run_oracle(*mesh_out((0, 0, 1, float("nan"), 0, 1, 2)))
    check(checks(f, "FAIL") == ["plane"], "NaN plane must fail")


@test
def test_plane_without_mdx_is_info_only():
    f = run_oracle(*mesh_out(GOOD_FACE, mdx_keep=False))
    check([(x.check, x.severity) for x in f] == [("plane", "INFO")], f)


@test
def test_plane_short_mdx_fails():
    (mdl, mdx), k1 = mesh_out(GOOD_FACE)
    f = O.check_model(mdl, mdx[:20], k1[0], k1[1], resolver())
    check(checks(f, "FAIL") == ["plane"] and "too short" in f[0].message, f)


# ---- T4 bones16 ----------------------------------------------------------------------------------------

def skin_out(bonemap, bones16):
    root = M("r", [M("a"), M("b"), M("s", type=0x61, bonemap=bonemap, bones16=bones16)])
    return build(root), build(M("r", [M("a"), M("b"), M("s", type=0x61, bonemap=[], bones16=None)]))


@test
def test_bones16_clean_and_warn_paths():
    ok = skin_out([-1, 0, 1, -1], [1, 2] + [0] * 14)
    check(run_oracle(*ok) == [], "consistent skin tables")
    far = skin_out([-1, 0, 1, -1], [1, 40] + [0] * 14)                # slot 1 names node 40 (>= 4 nodes)
    f = run_oracle(*far)
    check(checks(f) == ["bones16"] and f[0].severity == "WARN" and "node 40" in f[0].message, f)
    inv = skin_out([-1, 0, 1, -1], [2, 1] + [0] * 14)                 # not the inverse of the bonemap
    check(checks(run_oracle(*inv)) == ["bones16"], "inverse mismatch")
    badval = skin_out([-1, 16.0, 0.5, -1], [0] * 16)
    check(checks(run_oracle(*badval)) == ["bones16"], "bonemap value outside -1 or 0..15")
    junk_unused = skin_out([-1, -1, -1, -1], [999] * 16)              # unused slots may hold junk (K2 vanilla does)
    check(run_oracle(*junk_unused) == [], "unused slots are not checked")
    check(not O.has_fail(run_oracle(*far)), "bones16 never fails")


# ---- T8-lite dangly ------------------------------------------------------------------------------------

@test
def test_dangly_demoted_fails_and_kept_passes():
    k1 = build(M("r", [M("cloth", type=0x121), M("b")]))
    demoted = build(M("r", [M("cloth", type=0x21), M("b")]))
    f = run_oracle(demoted, k1)
    check(checks(f, "FAIL") == ["dangly_demoted"] and f[0].node == "cloth", f)
    check(run_oracle(build(M("r", [M("cloth", type=0x121), M("b")])), k1) == [], "kept dangly passes")
    ren = build(M("r", [M("cloth2", type=0x121), M("b")]))
    check(run_oracle(ren, k1, rename={"cloth": "cloth2"}) == [], "renamed dangly counts as kept")
    check("dangly_demoted" in checks(run_oracle(ren, k1), "FAIL"), "unmapped rename = demoted")


# ---- parse failures, caps ------------------------------------------------------------------------------

@test
def test_unreadable_models_are_findings():
    good = build(M("r", [M("a")]))
    f = O.check_model(b"\0" * 100, None, good[0], good[1], resolver())
    check([(x.check, x.severity) for x in f] == [("parse", "FAIL")], f)
    f = O.check_model(good[0], good[1], b"\0" * 100, None, resolver())
    check([(x.check, x.severity) for x in f] == [("k1_parse", "FAIL")], f)


@test
def test_per_code_cap_keeps_a_summary():
    kids = lambda sn_: [M(f"n{i}", sn=sn_) for i in range(40)]            # noqa: E731
    f = run_oracle(build(M("r", kids(900))), build(M("r", kids(0))))
    mine = [x for x in f if x.check == "sn_nonskin"]
    check(len(mine) == O.MAX_PER_CODE + 1 and "not listed" in mine[-1].message, len(mine))


# ---- CLI -----------------------------------------------------------------------------------------------

def cli(argv):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = O.main(argv)
    return rc, out.getvalue(), err.getvalue()


def cli_dirs(tmp, out_pair, k1_pair, k1_name="p_x", k2_name="k1p_x"):
    k1d, k2d, stage = tmp / "k1", tmp / "k2", tmp / "stage"
    for d in (k1d, k2d, stage / "models"):
        d.mkdir(parents=True)
    (k1d / f"{k1_name.upper()}.mdl").write_bytes(k1_pair[0])         # case-insensitive lookup
    (k1d / f"{k1_name}.mdx").write_bytes(k1_pair[1])
    if out_pair is not None:
        (stage / "models" / f"{k2_name}.mdl").write_bytes(out_pair[0])
        (stage / "models" / f"{k2_name}.mdx").write_bytes(out_pair[1])
    return ["--k1", str(k1d), "--k2", str(k2d), "--stage", str(stage), "--models", f"{k1_name}={k2_name}"]


def tmpdir():
    return Path(tempfile.mkdtemp(prefix="oracle_test_"))


@test
def test_cli_exit_codes_and_json():
    tmp = tmpdir()
    try:
        k1 = build(M("p_x", [M("a")]), model_name="p_x")
        good = build(M("k1p_x", [M("a")]), model_name="k1p_x")
        argv = cli_dirs(tmp, good, k1)
        rc, out, _ = cli(argv)
        check(rc == 0 and "PASS k1p_x" in out, (rc, out))
        rc, out, _ = cli(argv + ["--json"])
        doc = json.loads(out)
        check(rc == 0 and doc["status"] == "pass" and doc["findings"] == [], doc)
        bad = build(M("k1p_x", [M("zzz")]), model_name="k1p_x")
        (tmp / "stage/models/k1p_x.mdl").write_bytes(bad[0])
        rc, out, _ = cli(argv)
        check(rc == 1 and "FAIL topo_names" not in out and "topo_names" in out, (rc, out))
        rc, out, _ = cli(argv + ["--json"])
        doc = json.loads(out)
        check(rc == 1 and doc["status"] == "fail" and doc["models"] == {"k1p_x": "fail"}
              and doc["findings"][0]["check"] == "topo_names" and doc["findings"][0]["model"] == "k1p_x", doc)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


@test
def test_cli_error_exit_codes():
    tmp = tmpdir()
    try:
        k1 = build(M("p_x", [M("a")]), model_name="p_x")
        argv = cli_dirs(tmp, None, k1)
        rc, _, err = cli(argv)
        check(rc == 2 and "not found" in err, (rc, err))                           # output missing
        rc, out, _ = cli(argv + ["--json"])
        check(rc == 2 and json.loads(out)["status"] == "error", out)
        rc, _, err = cli(argv[:-1] + ["nonsense"])
        check(rc == 2 and "K1NAME=K2NAME" in err, (rc, err))
        good = build(M("k1p_x", [M("a")]), model_name="k1p_x")
        argv = cli_dirs(tmpdir(), good, k1)
        argv[1] = str(tmp / "no_such_k1")
        rc, _, err = cli(argv)
        check(rc == 2 and "K1 dir not found" in err, (rc, err))
        check(cli(["--k1", "x"])[0] == 2, "missing required args")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


@test
def test_cli_k1_dir_without_model_is_error():
    tmp = tmpdir()
    try:
        k1 = build(M("p_x", [M("a")]), model_name="p_x")
        good = build(M("k1p_x", [M("a")]), model_name="k1p_x")
        argv = cli_dirs(tmp, good, k1)
        argv[-1] = "p_missing=k1p_x"
        rc, _, err = cli(argv)
        check(rc == 2 and "p_missing" in err, (rc, err))
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


@test
def test_cli_subprocess_no_args_is_2():
    r = subprocess.run([sys.executable, "-I", str(CLI)], capture_output=True, text=True)
    check(r.returncode == 2, f"rc {r.returncode}")


@test
def test_cli_resolves_supermodel_from_stage_models():
    tmp = tmpdir()
    try:
        out = build(tree4(), super_name="sup", total=45, model_name="k1p_x")
        k1 = build(tree4(), model_name="p_x")
        argv = cli_dirs(tmp, out, k1)
        rc, text, _ = cli(argv)
        check(rc == 1 and "sn_error" in text, (rc, text))                          # supermodel not available yet
        (tmp / "stage/models/sup.mdl").write_bytes(sup_mdl())
        rc, text, _ = cli(argv)
        check(rc == 0 and "PASS k1p_x" in text, (rc, text))
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


# ---- calibration against real models (read-only, optional) ---------------------------------------------

class Bif(T.K2Bif):
    """K2 BIF reader with MDX access (resource type 3008)."""

    def mdx(self, name):
        bi, ri = self.res[(name.lower(), 3008)]
        with open(self.bifs[bi], "rb") as f, mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as mm:
            n, _, voff = struct.unpack_from("<III", mm, 8)
            for k in range(n):
                rid, off, size, _typ = struct.unpack_from("<IIII", mm, voff + 16 * k)
                if rid & 0xFFFFF == ri:
                    return bytes(mm[off:off + size])
        raise KeyError(name)


def bif_or_skip():
    if not (T.K2_INSTALL / "chitin.key").is_file():
        raise Skip(f"K2 install not found at {T.K2_INSTALL}")
    try:
        return Bif()
    except (OSError, FileNotFoundError) as exc:
        raise Skip(f"K2 install unreadable: {exc}")


class StageResolver:
    """Stage models dir first (converted outputs chain onto each other), then the vanilla install."""

    def __init__(self, d, bif):
        self.d, self.bif = d, bif

    def __call__(self, name):
        hit = sn._find_ci(self.d, name + ".mdl")
        return hit.read_bytes() if hit is not None else self.bif(name)


def real_case(models_dir, run, k1_name, k2_name, bif):
    d = Path(models_dir)
    pair = [d / f"{k2_name}.mdl", d / f"{k2_name}.mdx"]
    k1p = WORK / run / "work" / f"{k1_name}.mdl"
    if not all(p.is_file() for p in pair) or not k1p.is_file():
        raise Skip(f"missing {pair[0]} or {k1p}")
    return O.check_model(pair[0].read_bytes(), pair[1].read_bytes(), k1p.read_bytes(), None,
                         StageResolver(d, bif), {k1_name: k2_name})


def desc(f):
    return [(x.severity, x.check, x.node, x.message[:90]) for x in f if x.severity != "INFO"][:6]


@test
def test_vanilla_k2_models_pass_t2_t1():
    bif = bif_or_skip()
    for name in ("p_bastilabb", "s_female03"):
        mdl, mdx = bif(name), bif.mdx(name)
        f = O.check_model(mdl, mdx, mdl, mdx, bif)
        check(not O.has_fail(f), f"{name}: {desc(f)}")
        check(not [x for x in f if x.check.startswith(("sn_", "topo_names", "topo_children", "plane"))],
              f"{name}: {desc(f)}")
        print(f"  {name}: {len(f)} non-fail findings {sorted({x.check for x in f})}")


@test
def test_vanilla_scrambled_numbering_fails_sn_skin():
    bif = bif_or_skip()
    mdl, mdx = bif("p_bastilabb"), bif.mdx("p_bastilabb")
    scrambled, _t = T._scramble_like_pykotor(mdl)
    f = O.check_model(scrambled, mdx, mdl, mdx, bif)
    check({"sn_skin", "sn_total"} <= set(checks(f, "FAIL")), desc(f))


@test
def test_vanilla_plane_mutation_fails():
    bif = bif_or_skip()
    mdl, mdx = bif("p_bastilabb"), bif.mdx("p_bastilabb")
    tree = sn.parse_tree(mdl)
    node = next(n for n in tree.nodes if n.type & 0x20 and struct.unpack_from("<I", mdl, n.off + 80 + 12)[0] > 0)
    b = bytearray(mdl)
    face0 = struct.unpack_from("<I", mdl, node.off + 80 + 8)[0] + 12
    d = struct.unpack_from("<f", mdl, face0 + 12)[0]
    struct.pack_into("<f", b, face0 + 12, d + 5.0)
    f = O.check_model(bytes(b), mdx, mdl, mdx, bif)
    check(checks(f, "FAIL") == ["plane"], desc(f))
    check([x.node for x in f if x.check == "plane"] == [node.name_lower], desc(f))


MDLOPS_GOOD = (
    ("char_carth_mdlops", "carth_run1", ("p_carthba", "p_carthbb", "p_carthh")),
    ("probe_mdlops_juhani3", "juhani_run2", ("p_juhaniba", "p_juhanibb", "p_juhanih")),
)


@test
def test_mdlops_k2_numbered_outputs_pass():
    bif = bif_or_skip()
    done = 0
    for sub, run, names in MDLOPS_GOOD:
        for k1 in names:
            f = real_case(STAGE / sub / "models", run, k1, "k1p_" + k1[2:], bif)
            check(not O.has_fail(f), f"{sub}/{k1}: {desc(f)}")
            check(not [x for x in f if x.check in ("topo_names", "topo_children", "plane", "dangly_demoted")],
                  f"{sub}/{k1}: {desc(f)}")
            done += 1
            print(f"  {sub}/k1p_{k1[2:]}: PASS, non-fail findings {sorted({x.check for x in f})}")
    check(done == 6, done)


@test
def test_mdlops_k1_numbered_output_fails_t2():
    bif = bif_or_skip()
    f = real_case(STAGE / "probe_mdlops_juhani2/models", "juhani_run2", "p_juhanibb", "k1p_juhanibb", bif)
    check({"sn_skin", "sn_total"} <= set(checks(f, "FAIL")), desc(f))
    check(not [x for x in f if x.check in ("topo_names", "topo_children", "plane")], desc(f))


@test
def test_pykotor_legacy_numbered_carth_fails_t2():
    bif = bif_or_skip()
    for k1 in ("p_carthbb", "p_carthba"):
        f = real_case(STAGE / "models", "carth_run1", k1, "k1p_" + k1[2:], bif)
        check({"sn_skin", "sn_total"} <= set(checks(f, "FAIL")), f"{k1}: {desc(f)}")
        check(not [x for x in f if x.check in ("topo_names", "topo_children", "plane", "dangly_demoted")],
              f"{k1}: {desc(f)}")


@test
def test_cli_real_models_exit_codes():
    bif_or_skip()
    work = WORK / "carth_run1" / "work"
    if not (work / "p_carthbb.mdl").is_file() or not (STAGE / "char_carth_mdlops/models/k1p_carthbb.mdl").is_file():
        raise Skip("Carth MDLOps fixtures absent")
    common = ["--k1", str(work), "--k2", str(T.K2_INSTALL), "--models"]
    names = ["p_carthba=k1p_carthba", "p_carthbb=k1p_carthbb", "p_carthh=k1p_carthh"]
    rc, out, err = cli(common[:4] + ["--stage", str(STAGE / "char_carth_mdlops")] + common[4:] + names)
    check(rc == 0, (rc, out[-400:], err[-300:]))
    rc, out, err = cli(common[:4] + ["--stage", str(STAGE)] + common[4:] + names[:2] + ["--json"])
    doc = json.loads(out)
    check(rc == 1 and doc["models"] == {"k1p_carthba": "fail", "k1p_carthbb": "fail"}, (rc, doc["models"]))


@test
def test_load_k1_from_install_matches_extracted_copy():
    work = WORK / "carth_run1" / "work" / "p_carthbb.mdl"
    if not (K1_INSTALL / "chitin.key").is_file() or not work.is_file():
        raise Skip("K1 install or extracted copy absent")
    try:
        import k1_port_model as K
        K.load_pykotor()
    except Exception as exc:  # noqa: BLE001  pykotor lives in the k1port venv
        raise Skip(f"pykotor not importable under this python ({type(exc).__name__}); run with the k1port venv python")
    mdl, mdx = O.load_k1(K1_INSTALL, "p_carthbb")
    check(mdl == work.read_bytes() and mdx == work.with_suffix(".mdx").read_bytes(), "install bytes != extracted copy")
    try:
        O.load_k1(K1_INSTALL, "p_no_such_model_xyz")
        raise AssertionError("expected OracleError")
    except O.OracleError as exc:
        check("not found" in str(exc), str(exc))


def main() -> int:
    passed, skipped, failed = 0, 0, 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}")
            passed += 1
        except (Skip, T.Skip) as e:
            print(f"SKIP {fn.__name__}: {e}")
            skipped += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
            failed += 1
    print(f"{passed}/{len(TESTS)} PASS ({skipped} SKIP, {failed} FAIL)")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
