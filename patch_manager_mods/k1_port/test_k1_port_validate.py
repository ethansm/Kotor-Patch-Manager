#!/usr/bin/env python3
"""Tests for k1_port_validate.py. Synthetic MDL/MDX fixtures are built in-test (no files written except
the CLI test's tempdir). The real-model test reads the K2 install read-only and SKIPs when it is absent.
Run: python3 -u test_k1_port_validate.py   (cwd may be anything; exit 0 = all PASS or SKIP)
"""
import mmap
import struct
import subprocess
import json
import shutil
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_validate.py"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_validate as kv  # noqa: E402

K2_INSTALL = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
RT_MDL, RT_MDX = 2002, 3008
TESTS = []


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def codes(findings):
    return [f.code for f in findings]


def expect(findings, code):
    check(code in codes(findings), f"expected {code}, got {codes(findings)}")


class Model:
    """Writes a minimal MDL/MDX pair. alloc() returns file offsets; stored pointers are file offset - 12."""

    def __init__(self):
        self.buf = bytearray(12 + 0xC8)

    def alloc(self, n):
        while len(self.buf) % 4:
            self.buf.append(0)
        off = len(self.buf)
        self.buf += bytes(n)
        return off

    def u32(self, off, v):
        struct.pack_into("<I", self.buf, off, v & 0xFFFFFFFF)

    def i32(self, off, v):
        struct.pack_into("<i", self.buf, off, v)

    def u16(self, off, v):
        struct.pack_into("<H", self.buf, off, v)

    def name(self, off, text, size=32):
        self.buf[off:off + size] = text.encode("latin1").ljust(size, b"\0")[:size]


def build(skin=False, tm=340, names=("root", "mesh"), tex="tex01", model="c_test"):
    """One root node (dummy, children = [mesh]) and one trimesh or skin mesh with 3 rows.
    Returns (mdl, mdx, layout) where layout holds the file offsets the mutation tests need."""
    m = Model()
    m.name(12 + 8, model)
    name_fos = []
    for s in names:
        fo = m.alloc(len(s) + 1)
        m.buf[fo:fo + len(s)] = s.encode("latin1")
        name_fos.append(fo)
    name_arr = m.alloc(4 * len(names))
    for k, fo in enumerate(name_fos):
        m.u32(name_arr + 4 * k, fo - 12)
    m.u32(12 + 0xB8, name_arr - 12)
    m.u32(12 + 0xBC, len(names))
    m.u32(12 + 0xC0, len(names))

    root = m.alloc(80)
    root_kids = m.alloc(8)  # two slots so a duplicate-child layout can be built by mutation
    mesh = m.alloc(80 + tm + (100 if skin else 0))
    spare = m.alloc(4)  # child array for the mesh node (count 0 unless a test changes it)
    T = mesh + 80

    m.u16(root, 0x0001)
    m.u16(root + 4, 0)
    m.u32(root + 44, root_kids - 12)
    m.u32(root + 48, 1)
    m.u32(root + 52, 1)
    m.u32(root_kids, mesh - 12)
    m.u32(12 + 40, root - 12)

    m.u16(mesh, 0x0061 if skin else 0x0021)
    m.u16(mesh + 4, 1)
    m.u32(mesh + 44, spare - 12)
    m.u32(mesh + 48, 0)
    m.u32(mesh + 52, 0)

    face = m.alloc(32)
    m.u32(T + 8, face - 12)
    m.u32(T + 12, 1)
    m.u32(T + 16, 1)
    for head in (T + 176, T + 188, T + 200):
        arr4 = m.alloc(4)
        m.u32(head, arr4 - 12)
        m.u32(head + 4, 1)
        m.u32(head + 8, 1)
    m.name(T + 88, tex)
    m.name(T + 120, "")

    stride = 64 if skin else 32
    m.u32(T + 256, 0x23)  # vertex | normal | tex0
    m.i32(T + 252, stride)
    m.u16(T + 304, 3)
    m.i32(T + 260, 0)
    m.i32(T + 264, 12)
    m.i32(T + 268, -1)
    m.i32(T + 272, 24)
    m.i32(T + 276, -1)
    m.i32(T + 280, -1)
    m.i32(T + 284, -1)
    m.i32(T + 288, -1)
    m.u32(T + tm - 8, 0)  # MDX row base
    m.u32(T + tm - 4, 0)
    layout = {"root": root, "root_kids": root_kids, "mesh": mesh, "spare": spare, "T": T}

    if skin:
        S = T + tm
        n = 4
        bonemap = m.alloc(4 * n)
        qbones = m.alloc(16 * n)
        tbones = m.alloc(12 * n)
        unk0 = m.alloc(4 * n)
        m.u32(S + 12, 32)  # weights
        m.u32(S + 16, 48)  # bone indices
        m.u32(S + 20, bonemap - 12)
        m.u32(S + 24, n)
        m.u32(S + 28, qbones - 12)
        m.u32(S + 32, n)
        m.u32(S + 36, n)
        m.u32(S + 40, tbones - 12)
        m.u32(S + 44, n)
        m.u32(S + 48, n)
        m.u32(S + 52, unk0 - 12)
        m.u32(S + 56, n)
        m.u32(S + 60, n)
        layout["S"] = S

    mdx = bytes(3 * stride)
    m.u32(4, len(m.buf) - 12)
    m.u32(8, len(mdx))
    return bytes(m.buf), mdx, layout


def mutable(mdl):
    return bytearray(mdl)


# ---- synthetic fixtures -------------------------------------------------------------

@test
def test_synthetic_mesh_is_clean():
    mdl, mdx, _ = build()
    check(kv.validate(mdl, mdx) == [], f"clean mesh: {kv.validate(mdl, mdx)}")
    check(kv.validate(mdl) == [], "clean mesh without MDX")


@test
def test_synthetic_skin_is_clean():
    mdl, mdx, _ = build(skin=True)
    check(kv.validate(mdl, mdx) == [], f"clean skin: {kv.validate(mdl, mdx)}")


@test
def test_synthetic_k1_trimesh_size_is_clean():
    mdl, mdx, _ = build(tm=332)
    check(kv.validate(mdl, mdx, game="K1") == [], f"clean K1 mesh: {kv.validate(mdl, mdx, game='K1')}")


@test
def test_bad_root_pointer():
    mdl, mdx, _ = build()
    b = mutable(mdl)
    struct.pack_into("<I", b, 12 + 40, 0x7FFFFF)
    expect(kv.validate(bytes(b), mdx), "BAD_POINTER")


@test
def test_child_array_cycle():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<I", b, L["spare"], L["root"] - 12)  # mesh's child points back at root
    struct.pack_into("<I", b, L["mesh"] + 48, 1)
    expect(kv.validate(bytes(b), mdx), "NODE_CYCLE")


@test
def test_child_node_referenced_twice():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<I", b, L["root_kids"] + 4, L["mesh"] - 12)
    struct.pack_into("<I", b, L["root"] + 48, 2)
    expect(kv.validate(bytes(b), mdx), "NODE_DUP")


@test
def test_tex0_bit_with_offset_minus_one():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<i", b, L["T"] + 272, -1)
    expect(kv.validate(bytes(b), mdx), "TEX0_BIT_MISMATCH")


@test
def test_tangent_bit_without_offset():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<I", b, L["T"] + 256, 0xA3)
    expect(kv.validate(bytes(b), mdx), "TANGENT_BIT_MISMATCH")


@test
def test_skin_count_mismatch():
    mdl, mdx, L = build(skin=True)
    b = mutable(mdl)
    struct.pack_into("<I", b, L["S"] + 60, 3)  # unknown0 count2 != bonemap count
    expect(kv.validate(bytes(b), mdx), "SKIN_COUNT_MISMATCH")


@test
def test_skin_pointer_out_of_file():
    mdl, mdx, L = build(skin=True)
    b = mutable(mdl)
    struct.pack_into("<I", b, L["S"] + 20, 0x7FFFFF)  # bonemap pointer
    expect(kv.validate(bytes(b), mdx), "BAD_POINTER")


@test
def test_skin_row_order_reversed():
    mdl, mdx, L = build(skin=True)
    b = mutable(mdl)
    struct.pack_into("<I", b, L["S"] + 12, 48)
    struct.pack_into("<I", b, L["S"] + 16, 32)
    got = kv.validate(bytes(b), mdx)
    expect(got, "SKIN_ROW_ORDER")
    check(codes(got) == ["SKIN_ROW_ORDER"], f"row swap should be the only finding: {codes(got)}")


@test
def test_stride_mismatch():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<i", b, L["T"] + 252, 36)
    expect(kv.validate(bytes(b), mdx), "STRIDE_MISMATCH")


@test
def test_mdx_too_short():
    mdl, mdx, _ = build()
    expect(kv.validate(mdl, mdx[:-1]), "MDX_TOO_SHORT")


@test
def test_block_overlap_and_range():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<i", b, L["T"] + 272, 20)  # tex0 [20,28) overlaps normal [12,24)
    expect(kv.validate(bytes(b), mdx), "BLOCK_OVERLAP")
    b = mutable(mdl)
    struct.pack_into("<i", b, L["T"] + 272, 28)  # tex0 [28,36) past stride 32
    expect(kv.validate(bytes(b), mdx), "BLOCK_RANGE")


@test
def test_present_block_with_negative_offset():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<i", b, L["T"] + 260, -1)  # vertex bit set, offset gone
    expect(kv.validate(bytes(b), mdx), "BLOCK_OFFSET")


@test
def test_texture_name_too_long():
    mdl, mdx, _ = build(tex="A" * 32)
    expect(kv.validate(mdl, mdx), "TEX_NAME_TOO_LONG")
    mdl31, mdx31, _ = build(tex="A" * 31)
    check(kv.validate(mdl31, mdx31) == [], "31-char texture name is allowed")


@test
def test_texture_name_bad_chars():
    mdl, mdx, L = build(tex="ti\x01x")
    expect(kv.validate(mdl, mdx), "TEX_NAME_BAD_CHARS")


@test
def test_k1p_requires_lowercase_names():
    mdl, mdx, _ = build(tex="Tex01")
    check(kv.validate(mdl, mdx) == [], "uppercase is fine without --k1p")
    expect(kv.validate(mdl, mdx, require_lower_k1p=True), "NAME_NOT_LOWERCASE")
    mdl, mdx, _ = build(tex="NULL")
    check(kv.validate(mdl, mdx, require_lower_k1p=True) == [], "NULL sentinel is exempt")


@test
def test_bad_name_index():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<H", b, L["mesh"] + 4, 9)
    expect(kv.validate(bytes(b), mdx), "BAD_NAME_INDEX")


@test
def test_truncated_and_size_fields():
    mdl, mdx, _ = build()
    expect(kv.validate(mdl[:40], mdx), "TRUNCATED")
    b = mutable(mdl)
    struct.pack_into("<I", b, 4, 5)
    expect(kv.validate(bytes(b), mdx), "MDL_SIZE_MISMATCH")
    expect(kv.validate(mdl, mdx[:-1]), "MDX_SIZE_MISMATCH")


@test
def test_face_and_name_table_pointers():
    mdl, mdx, L = build()
    b = mutable(mdl)
    struct.pack_into("<I", b, L["T"] + 12, 0x7FFFFFFF)  # face count runs past end
    expect(kv.validate(bytes(b), mdx), "BAD_POINTER")
    b = mutable(mdl)
    struct.pack_into("<I", b, 12 + 0xB8, 0x7FFFFF)  # name table pointer
    expect(kv.validate(bytes(b), mdx), "BAD_POINTER")


# ---- CLI ----------------------------------------------------------------------------

@test
def test_cli_exit_codes_and_json():
    mdl, mdx, L = build()
    bad = mutable(mdl)
    struct.pack_into("<i", bad, L["T"] + 272, -1)
    tmp = Path(tempfile.mkdtemp(prefix="k1port_validate_test_"))
    try:
        good_mdl, bad_mdl, good_mdx = tmp / "good.mdl", tmp / "bad.mdl", tmp / "good.mdx"
        good_mdl.write_bytes(mdl)
        bad_mdl.write_bytes(bytes(bad))
        good_mdx.write_bytes(mdx)

        def run(*args):
            return subprocess.run([sys.executable, "-I", str(CLI), *map(str, args)],
                                  capture_output=True, text=True)

        r = run(good_mdl, "--mdx", good_mdx)
        check(r.returncode == 0, f"clean CLI rc {r.returncode}: {r.stdout}{r.stderr}")
        r = run(bad_mdl, "--mdx", good_mdx)
        check(r.returncode == 1 and "TEX0_BIT_MISMATCH" in r.stdout, f"bad CLI rc {r.returncode}: {r.stdout}")
        r = run(bad_mdl, "--json")
        data = json.loads(r.stdout)
        check(r.returncode == 1 and data["count"] >= 1 and data["findings"][0]["code"], f"json: {r.stdout}")
        r = run(tmp / "missing.mdl")
        check(r.returncode == 2, f"missing file rc {r.returncode}")
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


def _vanilla_k2_pairs():
    """Yield (name, mdl_bytes, mdx_bytes) for every MDL in the K2 install, via chitin.key/BIF."""
    key = (K2_INSTALL / "chitin.key").read_bytes()
    check(key[:4] == b"KEY ", "chitin.key signature")
    bif_n, key_n, bif_off, key_off = struct.unpack_from("<IIII", key, 8)
    bifs = []
    for i in range(bif_n):
        _size, fo, fs, _drv = struct.unpack_from("<IIHH", key, bif_off + 12 * i)
        rel = key[fo:fo + fs].split(b"\0")[0].decode("latin1").replace("\\", "/")
        bifs.append(_ci_path(K2_INSTALL, rel))
    res = {}
    for i in range(key_n):
        o = key_off + 22 * i
        resref = key[o:o + 16].split(b"\0")[0].decode("latin1").lower()
        rtype, rid = struct.unpack_from("<HI", key, o + 16)
        res[(resref, rtype)] = (rid >> 20, rid & 0xFFFFF)
    opened = {}

    def get(resref, rtype):
        bi, ri = res[(resref, rtype)]
        if bi not in opened:
            with open(bifs[bi], "rb") as f:
                mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
            n, _, voff = struct.unpack_from("<III", mm, 8)
            ent = {}
            for k in range(n):
                rid2, off, size, _typ = struct.unpack_from("<IIII", mm, voff + 16 * k)
                ent[rid2 & 0xFFFFF] = (off, size)
            opened[bi] = (mm, ent)
        mm, ent = opened[bi]
        off, size = ent[ri]
        return bytes(mm[off:off + size])

    names = sorted(r for (r, t) in res if t == RT_MDL)
    for name in names:
        if (name, RT_MDX) in res:
            yield name, get(name, RT_MDL), get(name, RT_MDX)
    for mm, _ent in opened.values():
        mm.close()


@test
def test_vanilla_k2_models_have_zero_findings():
    if not (K2_INSTALL / "chitin.key").is_file():
        raise Skip(f"K2 install not found at {K2_INSTALL}")
    checked, bad = 0, []
    for name, mdl, mdx in _vanilla_k2_pairs():
        checked += 1
        got = kv.validate(mdl, mdx)
        if got:
            bad.append((name, [(f.code, f.offset) for f in got][:3]))
    check(checked >= 200, f"only {checked} vanilla models checked")
    check(not bad, f"vanilla false positives in {len(bad)}/{checked}: {bad[:5]}")
    print(f"  checked {checked} vanilla K2 models, 0 findings")


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
