#!/usr/bin/env python3
"""Stdlib-only output gate for K1-port TPC textures, independent of the writer that made them.

Compares a child TPC with its parent: geometry, header, length and trailer, the new-name rule,
the TXI sidecar rule, the DXT alpha / 1-bit alpha invariants and, with a mask, that blocks outside
the mask footprint are byte-identical. It parses both files itself and never imports the writer.

API:  gate(parent, child, child_name, child_dir_files, mask_pixels=None) -> list[dict]
      each dict is {"id": str, "ok": bool, "detail": str}; mask_pixels = read_mask(png_bytes)
CLI:  python3 k1_port_tex_gate.py check --parent P --child C [--mask FILE] [--json]
Exit: 0 = all checks pass, 1 = gate failures, 2 = bad arguments or unreadable/unparseable input.
"""
import argparse
import json
import os
import struct
import sys
import zlib
from dataclasses import dataclass
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))  # python -I drops the script dir from sys.path
from k1_port_textures import MAX_NEW_LEN, NEW_RE  # noqa: E402  stdlib only

HDR_LEN = 128
NAMED_HDR = range(8, 14)  # width, height, encoding, mip count: judged by dims / format / mips
ENC_FORMAT = {2: "DXT1", 4: "DXT5"}  # only when data_size != 0 (compressed)
BPB = {"DXT1": 8, "DXT5": 16}
DETAIL_CAP = 5
PNG_SIG = b"\x89PNG\r\n\x1a\n"
_THRESH = bytes(1 if v >= 128 else 0 for v in range(256))


@dataclass
class Level:
    k: int
    w: int
    h: int
    bw: int  # blocks across
    bh: int  # blocks down
    off: int  # file offset of the first block


@dataclass
class Geom:
    width: int
    height: int
    mips: int
    fmt: str
    bpb: int
    levels: list
    end: int  # file offset where the mip chain ends; trailer starts here


def _res(cid: str, ok: bool, detail: str) -> dict:
    return {"id": cid, "ok": bool(ok), "detail": detail}


def _format_of(raw: bytes) -> str:
    """Compressed when data_size != 0; encoding 2 = DXT1, 4 = DXT5 (uncompressed 2/4 = RGB/RGBA)."""
    data_size = struct.unpack_from("<I", raw, 0)[0]
    enc = raw[12]
    if data_size == 0:
        return f"uncompressed enc {enc}"
    return ENC_FORMAT.get(enc, f"unknown enc {enc}")


def _layout(width: int, height: int, mips: int, bpb: int) -> tuple[list, int]:
    levels, off = [], HDR_LEN
    for k in range(mips):
        w, h = max(1, width >> k), max(1, height >> k)
        bw, bh = (w + 3) // 4, (h + 3) // 4
        levels.append(Level(k, w, h, bw, bh, off))
        off += bw * bh * bpb
    return levels, off


def _parse_parent(data: bytes) -> Geom:
    if len(data) < HDR_LEN:
        raise ValueError(f"parent is {len(data)} bytes, shorter than the {HDR_LEN}-byte header")
    fmt = _format_of(data)
    if fmt not in BPB:
        raise ValueError(f"parent is not a DXT1 or DXT5 TPC ({fmt})")
    width, height = struct.unpack_from("<HH", data, 8)
    mips = data[13]
    if width < 1 or height < 1 or mips < 1:
        raise ValueError(f"parent header has width {width}, height {height}, mips {mips}")
    levels, end = _layout(width, height, mips, BPB[fmt])
    if end > len(data):
        raise ValueError(f"parent mip chain needs {end} bytes, file has {len(data)}")
    return Geom(width, height, mips, fmt, BPB[fmt], levels, end)


def _check_dims(g: Geom, child: bytes) -> dict:
    cw, ch = struct.unpack_from("<HH", child, 8)
    return _res("dims", (cw, ch) == (g.width, g.height), f"child {cw}x{ch} vs parent {g.width}x{g.height}")


def _check_mips(g: Geom, child: bytes) -> dict:
    cm = child[13]
    return _res("mips", cm == g.mips, f"child {cm} vs parent {g.mips} mip(s)")


def _check_format(g: Geom, child: bytes) -> dict:
    cf = _format_of(child)
    return _res("format", cf == g.fmt, f"child {cf} vs parent {g.fmt}")


def _check_header(parent: bytes, child: bytes) -> dict:
    diff = [i for i in range(HDR_LEN) if i not in NAMED_HDR and parent[i] != child[i]]
    if diff:
        return _res("header", False, f"header bytes differ at offsets {diff[:DETAIL_CAP]}")
    return _res("header", True, "data_size, alpha float and reserved bytes equal")


def _check_length(g: Geom, parent: bytes, child: bytes) -> dict:
    problems = []
    if len(child) != len(parent):
        problems.append(f"length child {len(child)} vs parent {len(parent)} B")
    if child[g.end:] != parent[g.end:]:
        problems.append(f"trailer differs ({len(parent) - g.end} B in parent)")
    if problems:
        return _res("length", False, "; ".join(problems))
    return _res("length", True, f"{len(child)} B, trailer {len(parent) - g.end} B equal")


def _check_name(child_name: str) -> dict:
    stem = Path(child_name).stem
    problems = []
    if not NEW_RE.fullmatch(stem):
        problems.append("must match [a-z0-9_]+")
    if len(stem) > MAX_NEW_LEN:
        problems.append(f"{len(stem)} chars, max {MAX_NEW_LEN}")
    if problems:
        return _res("name", False, f"{stem!r}: " + "; ".join(problems))
    return _res("name", True, f"{stem!r} ok")


def _check_txi(child_name: str, child_dir_files: list[str]) -> dict:
    stem = Path(child_name).stem.lower()
    hits = sorted(f for f in child_dir_files
                  if Path(f).stem.lower() == stem and Path(f).suffix.lower() == ".txi")
    if hits:
        return _res("txi_sidecar", False, "sidecar next to child: " + ", ".join(hits))
    return _res("txi_sidecar", True, "no .txi sidecar next to child")


def _t3_set(block: bytes) -> int:
    """Bit 2i is set iff pixel i (row-major) has index 3: both index bits of the pair are set."""
    word = struct.unpack_from("<I", block, 4)[0]
    return word & (word >> 1) & 0x55555555


def _dxt1_block_diff(pb: bytes, cb: bytes):
    """Why a DXT1 block's 1-bit alpha behaviour changed, or None: colour mode (c0<=c1) or index-3 set."""
    p0, p1 = struct.unpack_from("<HH", pb, 0)
    c0, c1 = struct.unpack_from("<HH", cb, 0)
    if (p0 <= p1) != (c0 <= c1):
        return "colour mode (c0<=c1) changed"
    if p0 <= p1 and _t3_set(pb) != _t3_set(cb):
        return "index-3 (transparent) pixels changed"
    return None


def _check_alpha(g: Geom, parent: bytes, child: bytes) -> dict:
    bad, n = [], 0
    for lv in g.levels:
        for by in range(lv.bh):
            for bx in range(lv.bw):
                o = lv.off + (by * lv.bw + bx) * g.bpb
                pb, cb = parent[o:o + g.bpb], child[o:o + g.bpb]
                if len(cb) < g.bpb:
                    why = "child block truncated"
                elif g.fmt == "DXT5":
                    why = "alpha half changed" if cb[:8] != pb[:8] else None
                else:
                    why = _dxt1_block_diff(pb, cb)
                if why:
                    n += 1
                    if len(bad) < DETAIL_CAP:
                        bad.append(f"mip{lv.k} ({bx},{by}) {why}")
    if n:
        return _res("alpha", False, f"{n} block(s) changed alpha behaviour; first: " + "; ".join(bad))
    return _res("alpha", True, f"{g.fmt} alpha behaviour equal to parent on every block")


def _bor(a: bytes, b: bytes) -> bytes:
    return (int.from_bytes(a, "big") | int.from_bytes(b, "big")).to_bytes(len(a), "big")


def _or_reduce(grid: bytes, w: int, h: int) -> tuple[bytes, int, int]:
    """One footprint level: each output pixel is the OR of its 2x2 inputs (edge pixels clipped)."""
    w2, h2 = (w + 1) // 2, (h + 1) // 2
    out = bytearray()
    for y2 in range(h2):
        y0 = 2 * y2
        r0 = grid[y0 * w:(y0 + 1) * w]
        r1 = grid[(y0 + 1) * w:(y0 + 2) * w] if y0 + 1 < h else bytes(w)
        if w % 2:
            r0, r1 = r0 + b"\0", r1 + b"\0"
        row = _bor(r0, r1)
        out += _bor(row[0::2], row[1::2])
    return bytes(out), w2, h2


def _check_mask(g: Geom, parent: bytes, child: bytes, mask_pixels) -> dict:
    mw, mh, flat = mask_pixels
    if (mw, mh) != (g.width, g.height):
        raise ValueError(f"mask is {mw}x{mh}, mip 0 of the parent is {g.width}x{g.height}")
    grid, gw, gh = bytes(flat), mw, mh
    bad, n = [], 0
    for lv in g.levels:
        if lv.k:
            grid, gw, gh = _or_reduce(grid, gw, gh)
        for by in range(lv.bh):
            for bx in range(lv.bw):
                x0, y0 = 4 * bx, 4 * by
                x1, y1 = min(x0 + 4, lv.w), min(y0 + 4, lv.h)
                if any(any(grid[y * gw + x0:y * gw + x1]) for y in range(y0, y1)):
                    continue  # touches the mask footprint at this level: free to change
                o = lv.off + (by * lv.bw + bx) * g.bpb
                if child[o:o + g.bpb] != parent[o:o + g.bpb]:
                    n += 1
                    if len(bad) < DETAIL_CAP:
                        bad.append(f"mip{lv.k} ({bx},{by})")
    if n:
        return _res("mask_scope", False, f"{n} block(s) outside the mask changed; first: " + ", ".join(bad))
    return _res("mask_scope", True, "every block outside the mask footprint matches parent")


def gate(parent: bytes, child: bytes, child_name: str, child_dir_files: list[str],
         mask_pixels=None) -> list[dict]:
    """Run every check and return all results (no early stop). Raises ValueError on unparseable input.

    mask_pixels: None, or (width, height, flat) from read_mask(); flat holds one byte per mip-0 pixel
    in stored row order, nonzero = in mask. The mask_scope check runs only when it is given.
    """
    g = _parse_parent(parent)
    if len(child) < HDR_LEN:
        raise ValueError(f"child is {len(child)} bytes, shorter than the {HDR_LEN}-byte header")
    results = [
        _check_dims(g, child),
        _check_mips(g, child),
        _check_format(g, child),
        _check_header(parent, child),
        _check_length(g, parent, child),
        _check_name(child_name),
        _check_txi(child_name, child_dir_files),
        _check_alpha(g, parent, child),
    ]
    if mask_pixels is not None:
        results.append(_check_mask(g, parent, child, mask_pixels))
    return results


def _paeth(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    return b if pb <= pc else c


def _unfilter(ft: int, raw: bytes, prior: bytes, bpp: int) -> bytes:
    if ft == 0:
        return raw
    if ft == 2:
        return bytes((x + p) & 0xFF for x, p in zip(raw, prior))
    out = bytearray(len(raw))
    for i, x in enumerate(raw):
        a = out[i - bpp] if i >= bpp else 0
        b = prior[i]
        if ft == 1:
            p = a
        elif ft == 3:
            p = (a + b) >> 1
        else:
            p = _paeth(a, b, prior[i - bpp] if i >= bpp else 0)
        out[i] = (x + p) & 0xFF
    return bytes(out)


def read_mask(png: bytes) -> tuple[int, int, bytes]:
    """Decode an 8-bit grey or RGB non-interlaced PNG (zlib + struct only) to (width, height, flat).

    flat has one byte per pixel in stored row order: 1 = in mask (grey, or red channel, >= 128), else 0.
    """
    if png[:8] != PNG_SIG:
        raise ValueError("mask is not a PNG file")
    pos, ihdr, idat = 8, None, []
    while True:
        if pos + 12 > len(png):
            raise ValueError("PNG ends before IEND")
        length, ctype = struct.unpack_from(">I4s", png, pos)
        end = pos + 12 + length
        if end > len(png):
            raise ValueError(f"PNG chunk {ctype!r} runs past end of file")
        body = png[pos + 8:pos + 8 + length]
        if zlib.crc32(ctype + body) != struct.unpack_from(">I", png, end - 4)[0]:
            raise ValueError(f"PNG CRC error in {ctype!r} chunk")
        if ctype == b"IHDR":
            ihdr = body
        elif ctype == b"IDAT":
            idat.append(body)
        elif ctype == b"IEND":
            break
        pos = end
    if ihdr is None or len(ihdr) != 13 or not idat:
        raise ValueError("PNG has no IHDR or IDAT chunk")
    w, h, depth, color, comp, filt, interlace = struct.unpack(">IIBBBBB", ihdr)
    if depth != 8 or color not in (0, 2) or comp or filt or interlace or w < 1 or h < 1:
        raise ValueError(f"mask PNG must be 8-bit grey or RGB, non-interlaced "
                         f"(depth {depth}, colour type {color}, interlace {interlace})")
    bpp = 1 if color == 0 else 3
    stride = w * bpp
    raw = zlib.decompress(b"".join(idat))
    if len(raw) < h * (stride + 1):
        raise ValueError("PNG image data is truncated")
    rows, prior = [], bytes(stride)
    for y in range(h):
        base = y * (stride + 1)
        ft = raw[base]
        if ft > 4:
            raise ValueError(f"PNG row {y} has filter type {ft}")
        prior = _unfilter(ft, raw[base + 1:base + 1 + stride], prior, bpp)
        rows.append(prior)
    full = b"".join(rows)
    red = full if bpp == 1 else full[0::3]
    return w, h, red.translate(_THRESH)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Gate a K1-port TPC child against its parent.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    ck = sub.add_parser("check", help="run every gate check on one parent/child pair")
    ck.add_argument("--parent", required=True, type=Path, help="parent TPC (the reference)")
    ck.add_argument("--child", required=True, type=Path, help="child TPC to judge")
    ck.add_argument("--mask", type=Path, help="8-bit PNG mask at mip-0 size; enables mask_scope")
    ck.add_argument("--json", action="store_true", help="print one JSON object instead of text")
    args = ap.parse_args(argv)
    try:
        parent = args.parent.read_bytes()
        child = args.child.read_bytes()
        names = os.listdir(args.child.parent)
        mask = read_mask(args.mask.read_bytes()) if args.mask else None
        checks = gate(parent, child, args.child.name, names, mask)
    except (OSError, ValueError, struct.error, zlib.error) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    ok = all(c["ok"] for c in checks)
    if args.json:
        print(json.dumps({"ok": ok, "checks": checks}))
    else:
        for c in checks:
            print(f"{'PASS' if c['ok'] else 'FAIL'} {c['id']}: {c['detail']}")
        failed = sum(1 for c in checks if not c["ok"])
        print(f"gate: {'PASS' if ok else 'FAIL'} ({failed} failed of {len(checks)})")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
