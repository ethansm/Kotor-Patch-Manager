#!/usr/bin/env python3
"""Stdlib-only raw structural validator for KOTOR binary MDL/MDX (no pykotor).

Checks: pointer bounds, the node child-array walk (cycles and duplicates), trimesh MDX
bitmap/offset/stride agreement, skin header arrays and counts, texture and model names,
and the MDX length against the mesh rows. The layout is the K2 vanilla layout measured on
the full K2 corpus (.claude/review/k1_import/K2v_layout_evidence.md). A finding means the
file departs from that layout or is structurally unsafe; it is not an engine verdict.

API:  validate(mdl_bytes, mdx_bytes=None, *, game="K2", require_lower_k1p=False) -> list[Finding]
CLI:  python3 k1_port_validate.py FILE.mdl [--mdx FILE.mdx] [--game K1|K2] [--k1p] [--json]
Exit: 0 = no findings, 1 = findings, 2 = bad arguments or unreadable file.

Offsets: stored pointers are relative to byte 12 of the MDL resource (file offset = stored + 12).
Finding.offset is always a MDL byte offset: the field that declares the problem.
"""
import argparse
import json
import struct
import sys
from dataclasses import asdict, dataclass
from pathlib import Path

sys.dont_write_bytecode = True

HDR = 12
NODE_HDR = 80
TM_SIZE = {"K1": 332, "K2": 340}
SKIN_HDR_MIN = 64
ROOT_PTR_OFF = 40
NAMES_HEAD_OFF = 0xB8
GEOM_NAME_OFF = 8
NODE_MESH, NODE_SKIN = 0x20, 0x40
TEX0_BIT, TANGENT_BIT = 0x02, 0x80
SKIN_ROW = 16  # weights (4 floats) and bone indices (4 floats)
# (bitmap bit, label, bytes per row, offset field inside the trimesh header)
BLOCKS = (
    (0x01, "vertex", 12, 260),
    (0x20, "normal", 12, 264),
    (0x40, "color", 4, 268),
    (0x02, "tex0", 8, 272),
    (0x04, "tex1", 8, 276),
    (0x08, "tex2", 8, 280),
    (0x10, "tex3", 8, 284),
    (0x80, "tangent", 36, 288),
)


@dataclass(frozen=True)
class Finding:
    code: str
    message: str
    offset: int


class _Checker:
    def __init__(self, mdl, mdx, tm, lower):
        self.mdl = mdl
        self.mdx = mdx
        self.tm = tm
        self.lower = lower
        self.out = []
        self.names = []
        self.names_ok = False
        self.max_end = None  # (mdx byte end, trimesh offset) of the mesh that ends last

    def add(self, code, msg, off):
        self.out.append(Finding(code, msg, off))

    def fits(self, off, n):
        return off >= HDR and n >= 0 and off + n <= len(self.mdl)

    def u32(self, off):
        return struct.unpack_from("<I", self.mdl, off)[0]

    def i32(self, off):
        return struct.unpack_from("<i", self.mdl, off)[0]

    def u16(self, off):
        return struct.unpack_from("<H", self.mdl, off)[0]

    def array(self, ptr_off, cnt_off, elem, label):
        """Count-prefixed array: pointer at ptr_off, count at cnt_off. Returns (file_off, nbytes)
        for a non-empty array that lies inside the file, else None (BAD_POINTER is recorded)."""
        if not self.fits(ptr_off, 4) or not self.fits(cnt_off, 4):
            self.add("BAD_POINTER", f"{label} array header lies outside the file", ptr_off)
            return None
        count = self.u32(cnt_off)
        if count == 0:
            return None
        fo = HDR + self.u32(ptr_off)
        nbytes = count * elem
        if not self.fits(fo, nbytes):
            self.add("BAD_POINTER", f"{label} array ({count} x {elem} B) runs outside the file", ptr_off)
            return None
        return fo, nbytes

    def name(self, off, size, prefix, label):
        """Fixed-size NUL-terminated name field (32 bytes: at most 31 chars)."""
        if not self.fits(off, size):
            self.add("BAD_POINTER", f"{label} field lies outside the file", off)
            return
        raw = self.mdl[off:off + size]
        z = raw.find(b"\0")
        if z < 0:
            self.add(f"{prefix}_TOO_LONG", f"{label} has no NUL within {size} bytes", off)
            return
        text = raw[:z]
        if any(not 0x20 <= c <= 0x7E for c in text):
            self.add(f"{prefix}_BAD_CHARS", f"{label} has non-printable bytes: {text!r}", off)
            return
        # "NULL" is the vanilla no-texture sentinel, so it is exempt from the lowercase rule.
        if self.lower and text and text != text.lower() and text != b"NULL":
            self.add("NAME_NOT_LOWERCASE", f"{label} {text.decode('ascii')!r} must be lowercase (--k1p)", off)

    def read_names(self):
        head = HDR + NAMES_HEAD_OFF
        if not self.fits(head, 8):
            self.add("BAD_POINTER", "name table header lies outside the file", head)
            return
        if self.u32(head + 4) == 0:
            self.names_ok = True
            return
        arr = self.array(head, head + 4, 4, "name table")
        if arr is None:
            return
        base, nbytes = arr
        for k in range(nbytes // 4):
            slot = base + 4 * k
            e = HDR + self.u32(slot)
            end = self.mdl.find(b"\0", e) if self.fits(e, 1) else -1
            if end < 0:
                self.add("BAD_POINTER", f"name {k} string lies outside the file or is unterminated", slot)
                self.names.append(None)
            else:
                self.names.append(self.mdl[e:end].decode("latin1"))
        self.names_ok = True

    def walk(self):
        root_slot = HDR + ROOT_PTR_OFF
        if not self.fits(root_slot, 4):
            self.add("BAD_POINTER", "root node pointer lies outside the file", root_slot)
            return
        todo = [("enter", self.u32(root_slot))]
        path, seen = set(), set()
        while todo:
            kind, val = todo.pop()
            if kind == "exit":
                path.discard(val)
                continue
            fo = HDR + val
            if fo in path:
                self.add("NODE_CYCLE", "child array points back to an ancestor node", fo)
                continue
            if fo in seen:
                self.add("NODE_DUP", "node is referenced by more than one child array", fo)
                continue
            if not self.fits(fo, NODE_HDR):
                self.add("BAD_POINTER", "node header lies outside the file", fo)
                continue
            seen.add(fo)
            path.add(fo)
            todo.append(("exit", fo))
            self.node(fo)
            arr = self.array(fo + 44, fo + 48, 4, "child")
            if arr is not None:
                base, nbytes = arr
                kids = [self.u32(base + 4 * k) for k in range(nbytes // 4)]
                todo.extend(("enter", c) for c in reversed(kids))

    def node(self, fo):
        typ = self.u16(fo)
        nid = self.u16(fo + 4)
        if self.names_ok and nid >= len(self.names):
            self.add("BAD_NAME_INDEX", f"node name index {nid} >= {len(self.names)} names", fo + 4)
        if typ & NODE_MESH:
            self.trimesh(fo, typ)

    def trimesh(self, fo, typ):
        T = fo + NODE_HDR
        tm = self.tm
        if not self.fits(T, tm):
            self.add("BAD_POINTER", "trimesh header lies outside the file", T)
            return
        self.array(T + 8, T + 12, 32, "face")
        for head in (T + 176, T + 188, T + 200):
            self.array(head, head + 4, 4, "index")
        self.name(T + 88, 32, "TEX_NAME", "texture name")
        self.name(T + 120, 32, "TEX_NAME", "tex1 name")

        bitmap = self.u32(T + 256)
        stride = self.i32(T + 252)
        vc = self.u16(T + 304)
        mdx_base = self.u32(T + tm - 8)
        offs = {label: self.i32(T + pos) for _bit, label, _size, pos in BLOCKS}

        if bool(bitmap & TEX0_BIT) != (offs["tex0"] >= 0):
            self.add("TEX0_BIT_MISMATCH",
                     f"bitmap 0x{bitmap:02x} tex0 bit {'set' if bitmap & TEX0_BIT else 'clear'} "
                     f"but tex0 offset is {offs['tex0']}", T + 256)
        if bool(bitmap & TANGENT_BIT) != (offs["tangent"] >= 0):
            self.add("TANGENT_BIT_MISMATCH",
                     f"bitmap 0x{bitmap:02x} tangent bit {'set' if bitmap & TANGENT_BIT else 'clear'} "
                     f"but tangent offset is {offs['tangent']}", T + 256)

        blocks, expected = [], 0
        for bit, label, size, _pos in BLOCKS:
            if not bitmap & bit:
                continue
            expected += size
            o = offs[label]
            if o < 0:
                if label not in ("tex0", "tangent"):  # already reported as a bit mismatch
                    self.add("BLOCK_OFFSET", f"bitmap has the {label} bit but its offset is {o}", T + 256)
                continue
            blocks.append((o, size, label))

        if typ & NODE_SKIN:
            S = T + tm
            skin_blocks = self.skin(S)
            expected += 2 * SKIN_ROW
            blocks.extend(skin_blocks)

        if vc > 0:
            if stride != expected:
                self.add("STRIDE_MISMATCH",
                         f"MDX stride {stride} != {expected} from bitmap 0x{bitmap:02x}"
                         f"{' + skin rows' if typ & NODE_SKIN else ''}", T + 252)
            for o, size, label in blocks:
                if o < 0 or o + size > stride:
                    self.add("BLOCK_RANGE", f"{label} block [{o}, {o + size}) is outside stride {stride}", T + 252)
            ordered = sorted(blocks)
            for a, b in zip(ordered, ordered[1:]):
                if a[0] + a[1] > b[0]:
                    self.add("BLOCK_OVERLAP", f"{a[2]} block and {b[2]} block overlap in the MDX row", T + 256)
            if stride >= 0:
                end = mdx_base + vc * stride
                if self.max_end is None or end > self.max_end[0]:
                    self.max_end = (end, T)

    def skin(self, S):
        """Skin header at S (trimesh start + tm). Returns the weight and bone-index row blocks."""
        if not self.fits(S, SKIN_HDR_MIN):
            self.add("BAD_POINTER", "skin header lies outside the file", S)
            return []
        n = self.u32(S + 24)
        counts = {"qbones": S + 32, "qbones count2": S + 36, "tbones": S + 44,
                  "tbones count2": S + 48, "unk0": S + 56, "unk0 count2": S + 60}
        bad = {k: self.u32(o) for k, o in counts.items() if self.u32(o) != n}
        if bad:
            self.add("SKIN_COUNT_MISMATCH", f"bonemap count {n} but {bad}", S + 24)

        arrays = []
        for label, ptr_off, cnt_off, elem in (("bonemap", S + 20, S + 24, 4), ("qbones", S + 28, S + 32, 16),
                                               ("tbones", S + 40, S + 44, 12), ("unk0", S + 52, S + 56, 4)):
            arr = self.array(ptr_off, cnt_off, elem, label)
            if arr is not None:
                arrays.append((arr[0], arr[1], label, ptr_off))
        for prev, cur in zip(arrays, arrays[1:]):
            if cur[0] < prev[0] + prev[1]:
                self.add("SKIN_ARRAY_ORDER",
                         f"{cur[2]} starts at {cur[0]} before {prev[2]} ends at {prev[0] + prev[1]}", cur[3])

        w, b = self.u32(S + 12), self.u32(S + 16)
        if w > b:
            self.add("SKIN_ROW_ORDER", f"weights offset {w} follows bone-index offset {b} (vanilla: weights first)",
                     S + 12)
        return [(w, SKIN_ROW, "weights"), (b, SKIN_ROW, "bone-indices")]

    def check_mdx_length(self):
        if self.mdx is not None and self.max_end is not None and self.max_end[0] > len(self.mdx):
            self.add("MDX_TOO_SHORT",
                     f"last mesh rows end at MDX byte {self.max_end[0]} but the MDX has {len(self.mdx)} bytes",
                     self.max_end[1])


def validate(mdl_bytes, mdx_bytes=None, *, game="K2", require_lower_k1p=False):
    """Return every structural finding for one model. An empty list means no findings."""
    if game not in TM_SIZE:
        raise ValueError(f"unknown game {game!r} (expected K1 or K2)")
    mdl = bytes(mdl_bytes)
    mdx = None if mdx_bytes is None else bytes(mdx_bytes)
    chk = _Checker(mdl, mdx, TM_SIZE[game], require_lower_k1p)
    if len(mdl) < HDR + 0xC4:
        chk.add("TRUNCATED", f"MDL is {len(mdl)} bytes; the model header needs {HDR + 0xC4}", 0)
        return chk.out
    mdl_size = chk.u32(4)
    if mdl_size != len(mdl) - HDR:
        chk.add("MDL_SIZE_MISMATCH", f"header says {mdl_size} MDL bytes, resource has {len(mdl) - HDR}", 4)
    if mdx is not None and chk.u32(8) != len(mdx):
        chk.add("MDX_SIZE_MISMATCH", f"header says {chk.u32(8)} MDX bytes, MDX has {len(mdx)}", 8)
    chk.name(HDR + GEOM_NAME_OFF, 32, "MODEL_NAME", "model resref")
    chk.read_names()
    chk.walk()
    chk.check_mdx_length()
    return chk.out


def main(argv=None):
    ap = argparse.ArgumentParser(description="Structural validator for KOTOR MDL/MDX (stdlib only).")
    ap.add_argument("mdl", help="path to the .mdl file (resource bytes, 12-byte header included)")
    ap.add_argument("--mdx", help="path to the matching .mdx file (enables MDX length checks)")
    ap.add_argument("--game", choices=sorted(TM_SIZE), default="K2", help="trimesh header size (default K2)")
    ap.add_argument("--k1p", action="store_true", help="require lowercase texture names and model resref")
    ap.add_argument("--json", action="store_true", help="print findings as JSON")
    a = ap.parse_args(argv)
    try:
        mdl = Path(a.mdl).read_bytes()
        mdx = Path(a.mdx).read_bytes() if a.mdx else None
    except OSError as e:
        print(f"error: {e}", file=sys.stderr)
        return 2
    findings = validate(mdl, mdx, game=a.game, require_lower_k1p=a.k1p)
    if a.json:
        print(json.dumps({"mdl": a.mdl, "mdx": a.mdx, "count": len(findings),
                          "findings": [asdict(f) for f in findings]}, indent=2))
    else:
        for f in findings:
            print(f"{f.code} @0x{f.offset:x}: {f.message}")
        print(f"{len(findings)} finding(s)")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
