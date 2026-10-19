#!/usr/bin/env python3
"""Tests for k1_port_mdl_supernode.py. Synthetic MDL fixtures are built in-test (writes only go to a tempdir).
The golden tests read the K2 install and the local MDLOps stage directories read-only and SKIP when absent;
vanilla models are read from chitin.key/BIF with the stdlib, so they run under any python3.
Run: python3 -u test_k1_port_mdl_supernode.py   (cwd may be anything; exit 0 = all PASS or SKIP)
"""
import contextlib
import hashlib
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
CLI = HERE / "k1_port_mdl_supernode.py"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_mdl_supernode as sn  # noqa: E402
import k1_port_validate as kv  # noqa: E402

K2_INSTALL = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
STAGE = Path.home() / "Games/KOTOR2_Mod_Workspace/k1_port_stage"
RT_MDL = 2002
TESTS = []


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def raises(fn, *a, **kw):
    try:
        fn(*a, **kw)
    except sn.SupernodeError as e:
        return str(e)
    raise AssertionError("expected SupernodeError")


# ---- synthetic builder -----------------------------------------------------------------

class N:
    """Tree spec: name, children, optional explicit index (default: pre-order position), stored supernode."""

    def __init__(self, name, kids=(), idx=None, sn=0):
        self.name, self.kids, self.idx, self.sn = name, list(kids), idx, sn


def _preorder(n):
    yield n
    for k in n.kids:
        yield from _preorder(k)


def build(root, super_name=None, total=None, anims=(), names_extra=0):
    """Return MDL bytes. `anims` = list of N trees (anim nodes: index = idx, name unused).
    Header is 208 bytes; stored pointers are file offset - 12."""
    buf = bytearray(208)

    def alloc(n):
        while len(buf) % 4:
            buf.append(0)
        off = len(buf)
        buf.extend(bytes(n))
        return off

    geo = list(_preorder(root))
    for i, n in enumerate(geo):
        if n.idx is None:
            n.idx = i
    size = max(n.idx for n in geo) + 1 + names_extra
    table = {n.idx: n.name for n in geo}
    name_off = {}
    for i in range(size):
        s = table.get(i, f"filler{i}")
        o = alloc(len(s) + 1)
        buf[o:o + len(s)] = s.encode("latin1")
        name_off[i] = o
    arr = alloc(4 * size)
    for i in range(size):
        struct.pack_into("<I", buf, arr + 4 * i, name_off[i] - 12)
    struct.pack_into("<I", buf, 196, arr - 12)
    struct.pack_into("<II", buf, 200, size, size)

    def emit(n):
        for k in n.kids:
            if k.idx is None:
                k.idx = 0
        off = alloc(80)
        struct.pack_into("<HHH", buf, off, 1, n.sn, n.idx)
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
    if anims:
        heads = []
        for tree in anims:
            for n in _preorder(tree):
                if n.idx is None:
                    n.idx = 0
            h = alloc(136)
            ro = emit(tree)
            struct.pack_into("<I", buf, h + 40, ro - 12)
            heads.append(h)
        aa = alloc(4 * len(heads))
        for i, h in enumerate(heads):
            struct.pack_into("<I", buf, aa + 4 * i, h - 12)
        struct.pack_into("<I", buf, 100, aa - 12)
        struct.pack_into("<II", buf, 104, len(heads), len(heads))
    buf[148:180] = (super_name if super_name is not None else "NULL").encode("latin1").ljust(32, b"\0")
    struct.pack_into("<I", buf, 4, len(buf) - 12)
    return bytes(buf)


def sup_model():
    """Supermodel: root(0) -> a(1) -> a1(2), b(3); total 4. Stored supernodes are non-trivial (chain)."""
    return build(N("Root", [N("A", [N("A1", sn=7)], sn=5), N("B", sn=9)], sn=0), total=40)


def resolver_for(**models):
    calls = []

    def resolve(name):
        calls.append(name)
        if name not in models:
            raise FileNotFoundError(name)
        return models[name]

    resolve.calls = calls
    return resolve


def diff_offsets(a, b):
    check(len(a) == len(b), "length changed")
    return [i for i in range(len(a)) if a[i] != b[i]]


def sup_chain_child():
    return N("root", [N("a", [N("a1"), N("a2", [N("deep")])]), N("c", [N("c1")]), N("b")])


def numbering(mdl, resolve):
    out, rep = sn.apply(mdl, resolve)
    t = sn.parse_tree(out)
    return {n.name_lower: n.sn for n in t.nodes}, t.total, out, rep


# ---- tests ----------------------------------------------------------------------------

@test
def test_parse_round_trip():
    mdl = build(N("Root", [N("A", [N("A1")]), N("B")]), super_name="Sup_Model", total=3,
                anims=[N("x", [N("y", idx=2)], idx=0)])
    t = sn.parse_tree(mdl)
    check([n.name_lower for n in t.nodes] == ["root", "a", "a1", "b"], [n.name_lower for n in t.nodes])
    check([n.idx for n in t.nodes] == [0, 1, 2, 3], "idx")
    check(t.total == 3 and t.super_name == "Sup_Model", f"{t.total} {t.super_name!r}")
    check(len(t.anims) == 1 and [n.idx for n in t.anims[0]] == [0, 2], "anims")
    check([c.name_lower for c in t.nodes[0].children] == ["a", "b"], "children")
    check(sn.parse_tree(build(N("r"))).super_name == "", "NULL supermodel gives empty name")
    check(sn.parse_tree(build(N("r"), super_name="")).super_name == "", "empty supermodel gives empty name")


@test
def test_parse_index_is_name_table_index_not_preorder():
    mdl = build(N("Root", [N("A", idx=3), N("B", idx=1)], idx=0), names_extra=0)
    t = sn.parse_tree(mdl)
    check([(n.name_lower, n.idx) for n in t.nodes] == [("root", 0), ("a", 3), ("b", 1)], "idx keys")


@test
def test_r0_root_and_r1_case_insensitive_match():
    child = build(N("ROOT", [N("a"), N("B")]), super_name="sup", total=3)
    res = numbering(child, resolver_for(sup=sup_model()))
    got, _, _, _ = res
    check(got["root"] == 0 and got["a"] == 5 and got["b"] == 9, got)


@test
def test_r2_allocation_order_dfs_preorder():
    child = build(N("root", [N("zzz", [N("zz1")]), N("a", [N("a1"), N("new1"), N("new2")]), N("yyy")]),
                  super_name="sup")
    got, _, _, _ = numbering(child, resolver_for(sup=sup_model()))
    # supermodel max supernode = 9 -> first fresh value 10; DFS pre-order: zzz, (zz1 below unmatched), a1 matched,
    # new1, new2, yyy
    check(got["zzz"] == 10, got)
    check(got["a"] == 5 and got["a1"] == 7, got)
    check(got["new1"] == 11 and got["new2"] == 12 and got["yyy"] == 13, got)


@test
def test_r3_below_unmatched_uses_index_plus_total_plus_one():
    child = build(N("root", [N("a", [N("a1"), N("a2", [N("deep")])]), N("c", [N("c1")]), N("b")]), super_name="sup")
    sup = sup_model()  # total 40
    out, _ = sn.assign(sn.parse_tree(child), sn.parse_tree(sup))
    t = sn.parse_tree(child)
    by = {n.name_lower: n.idx for n in t.nodes}
    check(out[by["a2"]] == 10 and out[by["c"]] == 11, out)   # a2 and c unmatched siblings: 10, 11 (c after a's subtree)
    check(out[by["deep"]] == by["deep"] + 40 + 1, out)
    check(out[by["c1"]] == by["c1"] + 40 + 1, out)
    check(out[by["a1"]] == 7 and out[by["b"]] == 9, out)


@test
def test_r4_total_with_and_without_supermodel():
    child = build(N("root", [N("a"), N("x")]), super_name="sup")
    _, total, _, rep = numbering(child, resolver_for(sup=sup_model()))
    check(total == 3 + 40 + 1 and rep["total_after"] == 44 and rep["total_before"] == 3, (total, rep))
    # supermodel total 0 -> own count only
    zero = build(N("Root", [N("A")]), total=0)
    _, total0, _, _ = numbering(child, resolver_for(sup=zero))
    check(total0 == 3, total0)
    # no supermodel: own count, supernodes = own index
    plain = build(N("r", [N("a", idx=2), N("b", idx=1)], idx=0), total=99)
    got, t, out, rep = numbering(plain, resolver_for())
    check(t == 3 and got == {"r": 0, "a": 2, "b": 1}, (t, got))
    check(rep["supermodel"] is None and rep["supermodel_sha256"] is None, rep)


@test
def test_anim_nodes_follow_geometry_map_and_unmapped_reported():
    anim1 = N("r", [N("m", idx=1), N("n", idx=77)], idx=0, sn=123)
    anim2 = N("r", [N("p", idx=2)], idx=0, sn=4)
    child = build(N("root", [N("a"), N("b")]), super_name="sup", anims=[anim1, anim2])
    out, rep = sn.apply(child, resolver_for(sup=sup_model()))
    t = sn.parse_tree(out)
    # geometry: root 0, a(idx1) -> 5, b(idx2) -> 9
    check([n.sn for n in t.anims[0]] == [0, 5, 0], [n.sn for n in t.anims[0]])  # idx 77 unmapped: stays 0 (stored)
    check([n.sn for n in t.anims[1]] == [0, 9], [n.sn for n in t.anims[1]])
    check(rep["anim_unmapped"] == 1, rep)
    # anim1 root 123->0 and m 0->5 change, n is unmapped; anim2 root 4->0 and p 0->9 change
    check(rep["anim_nodes_changed"] == 4, rep)


@test
def test_only_expected_offsets_differ_and_length_same():
    child = build(N("root", [N("a"), N("x", [N("y")])]), super_name="sup", anims=[N("r", [N("q", idx=1)], idx=0)])
    out, rep = sn.apply(child, resolver_for(sup=sup_model()))
    t = sn.parse_tree(child)
    allowed = {n.off + 2 for n in t.nodes} | {n.off + 2 for tr in t.anims for n in tr} | {56}
    d = diff_offsets(child, out)
    check(d and all(any(o in (a, a + 1) for a in allowed) for o in d), f"unexpected offsets {d}")
    check(len(out) == len(child), "length")
    check(rep["changed_offsets_sample"] and len(rep["changed_offsets_sample"]) <= 10, rep)
    check(all(o in allowed for o in rep["changed_offsets_sample"]), rep)


@test
def test_idempotent():
    child = build(sup_chain_child(), super_name="Sup", anims=[N("r", [N("q", idx=1)], idx=0)])
    r = resolver_for(sup=sup_model())
    once, _ = sn.apply(child, r)
    twice, rep2 = sn.apply(once, r)
    check(once == twice, "apply not idempotent")
    check(rep2["nodes_changed"] == 0 and rep2["anim_nodes_changed"] == 0 and rep2["changed_offsets_sample"] == [], rep2)


@test
def test_input_not_mutated_and_report_sha():
    sup = sup_model()
    child = build(N("root", [N("a")]), super_name="sup")
    snap = bytes(child)
    _, rep = sn.apply(child, resolver_for(sup=sup))
    check(child == snap, "input mutated")
    check(rep["supermodel"] == "sup" and rep["supermodel_sha256"] == hashlib.sha256(sup).hexdigest(), rep)
    ba = bytearray(child)
    out, _ = sn.apply(ba, resolver_for(sup=sup))
    check(isinstance(out, bytes) and bytes(ba) == snap, "bytearray input mutated")


@test
def test_resolver_called_only_for_real_supermodel_with_lower_name():
    r = resolver_for(sup_model_x=sup_model())
    sn.apply(build(N("r"), super_name="NULL"), r)
    sn.apply(build(N("r"), super_name="null"), r)
    sn.apply(build(N("r"), super_name=""), r)
    check(r.calls == [], r.calls)
    sn.apply(build(N("r"), super_name="Sup_Model_X"), r)
    check(r.calls == ["sup_model_x"], r.calls)


@test
def test_missing_supermodel_raises_and_input_unchanged():
    child = build(N("r"), super_name="Gone")
    snap = bytes(child)
    msg = raises(sn.apply, child, resolver_for())
    check("Gone" in msg, msg)
    check(child == snap, "mutated")
    msg = raises(sn.apply, child, resolver_for(gone=b"not an mdl"))
    check("Gone" in msg, msg)


@test
def test_bad_pointer_and_cycle_and_bad_name_index():
    good = build(N("r", [N("a")]))
    t = sn.parse_tree(good)
    b = bytearray(good)
    struct.pack_into("<I", b, 52, 0x7FFFFF)
    raises(sn.parse_tree, bytes(b))
    b = bytearray(good)
    struct.pack_into("<I", b, t.nodes[0].off + 44, 0x7FFFFF)  # children array outside the file
    raises(sn.parse_tree, bytes(b))
    # cycle: make the only child point back at the root
    b = bytearray(good)
    cptr = struct.unpack_from("<I", b, t.nodes[0].off + 44)[0] + 12
    struct.pack_into("<I", b, cptr, t.nodes[0].off - 12)
    msg = raises(sn.parse_tree, bytes(b))
    check("twice" in msg, msg)
    # node leaving < 80 bytes
    b = bytearray(good)
    struct.pack_into("<I", b, cptr, len(good) - 40 - 12)
    raises(sn.parse_tree, bytes(b))
    # bad name index
    b = bytearray(good)
    struct.pack_into("<H", b, t.nodes[1].off + 4, 500)
    raises(sn.parse_tree, bytes(b))
    raises(sn.parse_tree, good[:100])
    # animation tree with a cycle
    g2 = build(N("r"), anims=[N("x", [N("y")])])
    t2 = sn.parse_tree(g2)
    ar = t2.anims[0][0]
    b = bytearray(g2)
    cp = struct.unpack_from("<I", b, ar.off + 44)[0] + 12
    struct.pack_into("<I", b, cp, ar.off - 12)
    raises(sn.parse_tree, bytes(b))


@test
def test_deep_tree_is_iterative_and_depth_limited():
    def chain(n):
        root = N("n0")
        cur = root
        for i in range(1, n):
            nxt = N(f"n{i}")
            cur.kids.append(nxt)
            cur = nxt
        return root

    old = sys.getrecursionlimit()
    sys.setrecursionlimit(20000)  # only the test's recursive builder needs this; the module is iterative
    try:
        mdl = build(chain(1500), super_name="sup")
        sup = build(chain(1500))
        deep = build(chain(2100))
    finally:
        sys.setrecursionlimit(old)
    out, rep = sn.apply(mdl, resolver_for(sup=sup))
    check(rep["total_after"] == 1500 + 1500 + 1 and sn.parse_tree(out).nodes[-1].sn == sn.parse_tree(sup).nodes[-1].sn,
          rep)
    raises(sn.parse_tree, deep)


@test
def test_u16_overflow_raises():
    sup = build(N("Root", [N("A")]), total=70000)
    child = build(N("root", [N("x", [N("y")])]), super_name="sup")  # y -> idx + 70000 + 1 > 65535
    msg = raises(sn.apply, child, resolver_for(sup=sup))
    check("u16" in msg, msg)


@test
def test_validator_findings_unchanged_on_built_model():
    mdl = build(N("root", [N("a")]), super_name="sup")
    before = kv.validate(mdl)
    out, _ = sn.apply(mdl, resolver_for(sup=sup_model()))
    after = kv.validate(out)
    check([f.code for f in after] == [f.code for f in before], f"{[f.code for f in before]} -> {[f.code for f in after]}")


@test
def test_dir_resolver_case_insensitive_and_origin():
    tmp = Path(tempfile.mkdtemp())
    try:
        a, b = tmp / "a", tmp / "b"
        a.mkdir()
        b.mkdir()
        (b / "Sup.MDL").write_bytes(sup_model())
        r = sn.dir_resolver([a, b])
        data = r("sup")
        check(data == sup_model() and r.origins["sup"] == f"dir:{b / 'Sup.MDL'}", r.origins)
        try:
            r("none")
            raise AssertionError("expected FileNotFoundError")
        except FileNotFoundError:
            pass
        out, _ = sn.apply(build(N("root", [N("a")]), super_name="SUP"), sn.dir_resolver([b]))
        check(sn.parse_tree(out).total == 2 + 40 + 1, "dir resolver apply")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


@test
def test_chain_resolver_dirs_override_then_install_error():
    tmp = Path(tempfile.mkdtemp())
    try:
        d, k2 = tmp / "d", tmp / "k2"
        (k2 / "override").mkdir(parents=True)
        d.mkdir()
        (d / "one.mdl").write_bytes(b"A")
        (k2 / "override" / "Two.mdl").write_bytes(b"B")
        r = sn.chain_resolver([d], k2)
        check(r("one") == b"A" and r.origins["one"].startswith("dir:"), r.origins)
        check(r("two") == b"B" and r.origins["two"] == "override", r.origins)
        try:
            r("three")  # not in dirs/override; install lookup needs pykotor and has no such model
            raise AssertionError("expected an error")
        except AssertionError:
            raise
        except Exception:  # noqa: BLE001  any lookup failure is fine; apply() wraps it
            pass
        msg = raises(sn.apply, build(N("r"), super_name="three"), r)
        check("three" in msg, msg)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


@test
def test_cli_exit_codes():
    tmp = Path(tempfile.mkdtemp())
    try:
        (tmp / "sup.mdl").write_bytes(sup_model())
        child = tmp / "child.mdl"
        child.write_bytes(build(N("root", [N("a"), N("x")]), super_name="sup"))

        def run(*argv):
            o, e = io.StringIO(), io.StringIO()
            with contextlib.redirect_stdout(o), contextlib.redirect_stderr(e):
                rc = sn.main([str(a) for a in argv])
            return rc, o.getvalue(), e.getvalue()

        rc, out, _ = run("check", child, "--super-dir", tmp)
        check(rc == 1 and "MISMATCH" in out, (rc, out))
        rc, out, _ = run("check", child, "--super", tmp / "sup.mdl", "--json")
        data = json.loads(out)
        check(rc == 1 and data["mismatches"] and data["total_want"] == 3 + 40 + 1, out)
        fixed = tmp / "fixed.mdl"
        rc, out, _ = run("fix", child, "--super-dir", tmp, "--out", fixed)
        check(rc == 0 and fixed.is_file() and json.loads(out)["total_after"] == 44, (rc, out))
        rc, out, _ = run("check", fixed, "--super-dir", tmp)
        check(rc == 0 and out.startswith("OK:"), (rc, out))
        # explicit --super is used whatever the header name is
        other = tmp / "other_name.mdl"
        other.write_bytes(sup_model())
        rc, _, _ = run("check", fixed, "--super", other)
        check(rc == 0, rc)
        # errors -> 2
        rc, _, err = run("check", tmp / "nope.mdl", "--super-dir", tmp)
        check(rc == 2, (rc, err))
        rc, _, err = run("check", child)  # supermodel needed but not given
        check(rc == 2 and "sup" in err, (rc, err))
        rc, _, err = run("check", child, "--super-dir", tmp / "empty")
        check(rc == 2, (rc, err))
        # fix refuses same path without --in-place, and steamapps paths
        rc, _, err = run("fix", child, "--super-dir", tmp, "--out", child)
        check(rc == 2 and "in-place" in err, (rc, err))
        steam = tmp / "steamapps" / "x"
        steam.mkdir(parents=True)
        rc, _, err = run("fix", child, "--super-dir", tmp, "--out", steam / "o.mdl")
        check(rc == 2 and not (steam / "o.mdl").exists(), (rc, err))
        before = child.read_bytes()
        rc, _, _ = run("fix", child, "--super-dir", tmp, "--out", child, "--in-place")
        check(rc == 0 and child.read_bytes() != before, rc)
        # no supermodel, no flags needed
        plain = tmp / "plain.mdl"
        plain.write_bytes(build(N("r", [N("a", sn=1)])))
        rc, _, _ = run("check", plain)
        check(rc == 0, rc)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


@test
def test_cli_subprocess():
    tmp = Path(tempfile.mkdtemp())
    try:
        (tmp / "sup.mdl").write_bytes(sup_model())
        child = tmp / "child.mdl"
        child.write_bytes(build(N("root", [N("a")]), super_name="sup"))
        r = subprocess.run([sys.executable, "-I", str(CLI), "check", str(child), "--super-dir", str(tmp)],
                           capture_output=True, text=True)
        check(r.returncode == 1, f"rc {r.returncode}: {r.stdout} {r.stderr}")
        r = subprocess.run([sys.executable, "-I", str(CLI), "check", str(tmp / "missing.mdl")],
                           capture_output=True, text=True)
        check(r.returncode == 2, f"rc {r.returncode}")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


# ---- real models (read-only, optional) ------------------------------------------------

def _ci_path(root, rel):
    cur = Path(root)
    for part in rel.split("/"):
        hit = [c for c in cur.iterdir() if c.name.lower() == part.lower()] if cur.is_dir() else []
        if not hit:
            raise FileNotFoundError(f"{root}/{rel}")
        cur = hit[0]
    return cur


class K2Bif:
    """Reads vanilla MDLs from chitin.key/BIF (stdlib). Callable as a supermodel resolver."""

    def __init__(self):
        key = (K2_INSTALL / "chitin.key").read_bytes()
        check(key[:4] == b"KEY ", "chitin.key signature")
        bif_n, key_n, bif_off, key_off = struct.unpack_from("<IIII", key, 8)
        self.bifs = []
        for i in range(bif_n):
            _size, fo, fs, _drv = struct.unpack_from("<IIHH", key, bif_off + 12 * i)
            rel = key[fo:fo + fs].split(b"\0")[0].decode("latin1").replace("\\", "/")
            self.bifs.append(_ci_path(K2_INSTALL, rel))
        self.res = {}
        for i in range(key_n):
            o = key_off + 22 * i
            resref = key[o:o + 16].split(b"\0")[0].decode("latin1").lower()
            rtype, rid = struct.unpack_from("<HI", key, o + 16)
            self.res[(resref, rtype)] = (rid >> 20, rid & 0xFFFFF)

    def __call__(self, name):
        bi, ri = self.res[(name.lower(), RT_MDL)]
        with open(self.bifs[bi], "rb") as f, mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as mm:
            n, _, voff = struct.unpack_from("<III", mm, 8)
            for k in range(n):
                rid2, off, size, _typ = struct.unpack_from("<IIII", mm, voff + 16 * k)
                if rid2 & 0xFFFFF == ri:
                    return bytes(mm[off:off + size])
        raise KeyError(name)


def _bif_or_skip():
    if not (K2_INSTALL / "chitin.key").is_file():
        raise Skip(f"K2 install not found at {K2_INSTALL}")
    return K2Bif()


def _scramble_like_pykotor(vanilla):
    """supernode := own index for geometry+anim nodes, total := own geometry count (what pykotor writes)."""
    t = sn.parse_tree(vanilla)
    b = bytearray(vanilla)
    for n in list(t.nodes) + [x for tr in t.anims for x in tr]:
        struct.pack_into("<H", b, n.off + 2, n.idx)
    struct.pack_into("<i", b, 56, len(t.nodes))
    return bytes(b), t


def _golden(name):
    bif = _bif_or_skip()
    vanilla = bif(name)
    scrambled, t = _scramble_like_pykotor(vanilla)
    check(scrambled != vanilla, f"{name}: scramble was a no-op")
    check(t.super_name, f"{name}: expected a supermodel in the header")
    out, rep = sn.apply(scrambled, bif)
    check(rep["supermodel"].lower() == t.super_name.lower(), rep)
    if out != vanilla:
        d = diff_offsets(out, vanilla)
        raise AssertionError(f"{name}: {len(d)} bytes differ from vanilla, first offsets {d[:8]}; report {rep}")
    return t, rep


@test
def test_golden_p_bastilabb_equals_vanilla():
    t, rep = _golden("p_bastilabb")
    print(f"  p_bastilabb: supermodel {rep['supermodel']}, {len(t.nodes)} nodes, {rep['nodes_changed']} changed, "
          f"total {rep['total_before']} -> {rep['total_after']}, byte-equal to vanilla")


@test
def test_golden_s_female03_equals_vanilla():
    t, rep = _golden("s_female03")
    check(len(t.anims) > 0 and rep["anim_nodes_changed"] > 0, f"expected animation edits: {rep}")
    print(f"  s_female03: supermodel {rep['supermodel']}, {len(t.nodes)} nodes, {len(t.anims)} anims, "
          f"{rep['anim_nodes_changed']} anim nodes changed, byte-equal to vanilla")


@test
def test_chain_resolver_install_matches_bif():
    bif = _bif_or_skip()
    try:
        import k1_port_mdlops  # noqa: F401
        import k1_port_model as K
        K.load_pykotor()
    except Exception as exc:  # noqa: BLE001  pykotor lives in the k1port venv
        raise Skip(f"pykotor not importable under this python ({type(exc).__name__}); run with the k1port venv python")
    name = next((n for n in ("c_rancor", "p_hk47", "n_darthmalak", "s_male01")
                 if (n, RT_MDL) in bif.res and sn._find_ci(K2_INSTALL / "override", n + ".mdl") is None), None)
    if name is None:
        raise Skip("override/ shadows every candidate vanilla model")
    r = sn.chain_resolver([], K2_INSTALL)
    check(r(name) == bif(name) and r.origins[name] == "install", r.origins)


class _StageResolver:
    """Same directory first (MDLOps outputs chain onto each other), then the vanilla install."""

    def __init__(self, d, bif):
        self.dir, self.bif, self.origins = d, bif, {}

    def __call__(self, name):
        hit = sn._find_ci(self.dir, name + ".mdl")
        if hit is not None:
            self.origins[name] = "dir"
            return hit.read_bytes()
        self.origins[name] = "install"
        return self.bif(name)


def _mdlops_golden(subdir, expect_min):
    d = STAGE / subdir / "models"
    files = sorted(d.glob("*.mdl")) if d.is_dir() else []
    if not files:
        raise Skip(f"no MDLOps outputs in {d}")
    bif = _bif_or_skip()
    res = _StageResolver(d, bif)
    done = []
    for f in files:
        data = f.read_bytes()
        t = sn.parse_tree(data)
        if t.super_name:
            try:
                res(t.super_name.lower())
            except (KeyError, OSError):
                continue  # supermodel not available here
        mm = sn.mismatches(data, res)
        check(not mm["mismatches"] and mm["total_ok"],
              f"{f.name}: MDLOps numbering != rule: {mm['mismatches'][:5]} total {mm['total_have']} vs {mm['total_want']}")
        done.append(f.name)
    if len(done) < expect_min:
        raise Skip(f"only {len(done)} of {len(files)} models in {d} had their supermodel available")
    print(f"  {subdir}: {len(done)} MDLOps models reproduce their own supernodes+total: {done}")


@test
def test_mdlops_carth_numbering_matches_rule():
    _mdlops_golden("char_carth_mdlops", 1)


@test
def test_mdlops_juhani_numbering_matches_rule():
    _mdlops_golden("probe_mdlops_juhani3", 1)


def main() -> int:
    passed, skipped, failed = 0, 0, 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}")
            passed += 1
        except Skip as e:
            print(f"SKIP {fn.__name__}: {e}")
            skipped += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
            failed += 1
    print(f"{passed}/{len(TESTS)} PASS ({skipped} SKIP, {failed} FAIL)")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
