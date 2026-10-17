#!/usr/bin/env python3
"""TPC container layer for the K1 texture port: parse the raw mip chain, decode mips, apply RGB edits, and
build a DXT1 TPC from an uncompressed TGA.

Works on raw bytes (128-byte header, back-to-back DXT mips, optional trailing TXI text) and uses the U1 DXT codec
for block work. Edits follow the shared footprint rule: a mip pixel is "in mask" when any mip-0 pixel under it is,
and only blocks touching that footprint are re-encoded. Odd mip dimensions fold the trailing row/column into the
last output pixel, so the box filter matches the container's floor-halved mip sizes.

TGA -> TPC (tga_to_tpc) stores rows bottom-up: the engine draws a TPC's stored row 0 at the bottom, like a TGA's
(scripts/generate_ultrawide_textures.py plan_output; found 09-26 as a mirrored "N" on the map tab). So decode_mip
of the output returns the image upside down, exactly as it does for a vanilla TPC.
CLI: python3 k1_port_tex_tpc.py tga-to-tpc IN.tga OUT.tpc [--force-opaque] [--txi FILE]
"""
import argparse
import struct
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
from k1_port_tex_dxt import BLOCK_BYTES, block_mask, decode, encode_rgb, splice_colour  # noqa: E402  stdlib + numpy

HEADER_SIZE = 128
PIXEL_TYPE_FMT = {2: "dxt1", 4: "dxt5"}  # compressed encodings only; uncompressed is not supported here
PIXEL_TYPE_DXT1 = 2
TGA_HEADER_SIZE = 18


def mip_dims(w: int, h: int, k: int) -> tuple[int, int]:
    return max(1, w >> k), max(1, h >> k)


def parse(b: bytes) -> dict:
    """Read the header and locate every mip. Raises ValueError for anything that is not a sized DXT chain."""
    b = bytes(b)
    if len(b) < HEADER_SIZE:
        raise ValueError(f"TPC is {len(b)} bytes, shorter than the {HEADER_SIZE}-byte header")
    data_size = struct.unpack_from("<I", b, 0)[0]
    w, h = struct.unpack_from("<HH", b, 8)
    pixel_type, mips = b[12], b[13]
    if data_size == 0:
        raise ValueError("uncompressed TPC (data_size 0) is not supported")
    fmt = PIXEL_TYPE_FMT.get(pixel_type)
    if fmt is None:
        raise ValueError(f"unsupported compressed TPC pixel_type {pixel_type}")
    if w < 1 or h < 1 or mips < 1:
        raise ValueError(f"bad TPC dimensions or mip count: {w}x{h}, {mips} mips")
    offsets, sizes = [], []
    off = HEADER_SIZE
    for k in range(mips):
        mw, mh = mip_dims(w, h, k)
        size = ((mw + 3) // 4) * ((mh + 3) // 4) * BLOCK_BYTES[fmt]
        offsets.append(off)
        sizes.append(size)
        off += size
    if off > len(b):
        raise ValueError(f"TPC truncated: mip chain needs {off} bytes, file has {len(b)}")
    if sizes[0] != data_size:
        raise ValueError(f"data_size {data_size} does not match mip 0 size {sizes[0]}")
    return {
        "w": w,
        "h": h,
        "fmt": fmt,
        "mips": mips,
        "offsets": offsets,
        "sizes": sizes,
        "trailer": b[off:],
        "header": b[:HEADER_SIZE],
    }


def decode_mip(b: bytes, info: dict, k: int) -> np.ndarray:
    """RGBA (h, w, 4) uint8 for mip k, in stored row order."""
    off, size = info["offsets"][k], info["sizes"][k]
    mw, mh = mip_dims(info["w"], info["h"], k)
    return decode(bytes(b[off:off + size]), mw, mh, info["fmt"])


def _axis_starts(n: int) -> np.ndarray:
    """Start index of each output pixel along one axis; the last output pixel absorbs any odd trailing index."""
    m = max(1, n >> 1)
    return np.arange(0, 2 * m, 2)


def _box_sum(arr: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    """Sum each 2x2 (edge-folded) footprint of an (h, w[, c]) array; also return the footprint pixel counts."""
    h, w = arr.shape[:2]
    sy, sx = _axis_starts(h), _axis_starts(w)
    summed = np.add.reduceat(np.add.reduceat(arr, sy, axis=0), sx, axis=1)
    ly = np.diff(np.append(sy, h))
    lx = np.diff(np.append(sx, w))
    return summed, np.outer(ly, lx)


def downsample_rgb(rgb: np.ndarray) -> np.ndarray:
    """2x2 box filter on (h, w, 3) uint8, rounding half up. Odd and 1-px dimensions fold into the last pixel."""
    summed, count = _box_sum(np.asarray(rgb, dtype=np.int64))
    count = count[:, :, None]
    return ((2 * summed + count) // (2 * count)).astype(np.uint8)


def downsample_mask(mask: np.ndarray) -> np.ndarray:
    """Footprint rule: an output pixel is in mask if any pixel of its 2x2 (edge-folded) footprint is."""
    summed, _ = _box_sum(np.asarray(mask, dtype=np.int64))
    return summed > 0


def apply_rgb_edit(parent: bytes, rgb: np.ndarray, mask: np.ndarray) -> bytes:
    """Return a same-length TPC with the masked mip-0 pixels set to rgb, propagated down the mip chain.

    rgb is the desired mip-0 colour (h, w, 3) uint8; mask is (h, w) bool. Only masked pixels count. Blocks whose
    footprint has no masked pixel at that level stay byte-identical, and DXT5 alpha halves are never touched.
    """
    b = bytes(parent)
    info = parse(b)
    w, h, fmt = info["w"], info["h"], info["fmt"]
    rgb = np.asarray(rgb)
    mask = np.asarray(mask, dtype=bool)
    if rgb.dtype != np.uint8 or rgb.shape != (h, w, 3):
        raise ValueError(f"rgb must be uint8 ({h}, {w}, 3), got {rgb.dtype} {rgb.shape}")
    if mask.shape != (h, w):
        raise ValueError(f"mask must be ({h}, {w}), got {mask.shape}")
    if not mask.any():
        return b

    composite = np.where(mask[:, :, None], rgb, decode_mip(b, info, 0)[:, :, :3])
    cur_mask = mask
    parts = []
    for k in range(info["mips"]):
        if k > 0:
            composite = downsample_rgb(composite)  # box-filter the previous composite
            cur_mask = downsample_mask(cur_mask)
            parent_rgb = decode_mip(b, info, k)[:, :, :3]
            composite = np.where(cur_mask[:, :, None], composite, parent_rgb)
        mw, mh = mip_dims(w, h, k)
        off, size = info["offsets"][k], info["sizes"][k]
        colour = encode_rgb(composite)
        changed = block_mask(cur_mask)
        parts.append(splice_colour(b[off:off + size], mw, mh, fmt, colour, changed))
    out = info["header"] + b"".join(parts) + info["trailer"]
    if len(out) != len(b):
        raise AssertionError(f"edit changed TPC length: {len(b)} -> {len(out)}")
    return out


def read_tga(tga: bytes) -> np.ndarray:
    """Uncompressed true-colour TGA (type 2, 24 or 32 bpp) -> (h, w, 4) uint8 RGBA, rows top-down.

    Descriptor bit 5 set = top-left origin (rows stored top first); clear = bottom-left, so rows are flipped here.
    Bit 4 (right-to-left) mirrors the columns. 24-bit files get alpha 255. The ID field is skipped.
    """
    b = bytes(tga)
    if len(b) < TGA_HEADER_SIZE:
        raise ValueError(f"TGA is {len(b)} bytes, shorter than the {TGA_HEADER_SIZE}-byte header")
    id_len, cmap_type, img_type = b[0], b[1], b[2]
    w, h = struct.unpack_from("<HH", b, 12)
    depth, desc = b[16], b[17]
    if img_type != 2:
        raise ValueError(f"TGA image type {img_type} not supported; only uncompressed true-colour (2)")
    if cmap_type != 0:
        raise ValueError(f"TGA colour map (type {cmap_type}) not supported")
    if depth not in (24, 32):
        raise ValueError(f"TGA pixel depth {depth} not supported; 24 or 32 only")
    if w < 1 or h < 1:
        raise ValueError(f"bad TGA dimensions {w}x{h}")
    bpp = depth // 8
    off = TGA_HEADER_SIZE + id_len
    need = w * h * bpp
    if off + need > len(b):
        raise ValueError(f"TGA truncated: pixel data needs {need} bytes at offset {off}, file has {len(b)}")
    px = np.frombuffer(b, dtype=np.uint8, count=need, offset=off).reshape(h, w, bpp)
    rgba = np.empty((h, w, 4), dtype=np.uint8)
    rgba[..., 0], rgba[..., 1], rgba[..., 2] = px[..., 2], px[..., 1], px[..., 0]  # file is BGR(A)
    rgba[..., 3] = px[..., 3] if bpp == 4 else 255
    if not desc & 0x20:  # bottom-left origin: the first stored row is the bottom row of the image
        rgba = rgba[::-1]
    if desc & 0x10:  # right-to-left
        rgba = rgba[:, ::-1]
    return np.ascontiguousarray(rgba)


def tga_to_tpc(tga_bytes: bytes, force_opaque: bool = False, txi=None) -> bytes:
    """Uncompressed TGA -> DXT1 TPC with a full mip chain down to 1x1 and an optional raw TXI trailer.

    force_opaque sets alpha to 255 first. Any alpha below 255 left after that raises ValueError: DXT5 is not
    encoded here. txi is bytes (or str, encoded as ASCII) appended verbatim after the mip data.
    """
    rgba = read_tga(tga_bytes)
    if force_opaque:
        rgba[..., 3] = 255
    if (rgba[..., 3] != 255).any():
        raise ValueError("non-opaque TGA needs DXT5; not supported")
    rgba = rgba[::-1]  # stored row 0 = bottom image row (see module docstring)
    h, w = rgba.shape[:2]
    if w > 0xFFFF or h > 0xFFFF:
        raise ValueError(f"TGA {w}x{h} exceeds the TPC 16-bit dimension limit")
    mips = max(w, h).bit_length()  # floor-halved chain down to 1x1
    level = rgba[:, :, :3]
    body = []
    for k in range(mips):
        if k:
            level = downsample_rgb(level)
        body.append(encode_rgb(level).tobytes())
    data = b"".join(body)
    header = struct.pack("<IfHHBB", len(body[0]), 1.0, w, h, PIXEL_TYPE_DXT1, mips)
    tail = b"" if txi is None else (txi.encode("ascii") if isinstance(txi, str) else bytes(txi))
    return header + b"\0" * (HEADER_SIZE - len(header)) + data + tail


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="K1 port TPC tools")
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("tga-to-tpc", help="uncompressed TGA -> DXT1 TPC with a full mip chain")
    p.add_argument("tga", type=Path, help="input uncompressed 24/32-bit TGA")
    p.add_argument("tpc", type=Path, help="output TPC path")
    p.add_argument("--force-opaque", action="store_true", help="set alpha to 255 before encoding")
    p.add_argument("--txi", type=Path, help="TXI text file appended as the TPC trailer")
    args = ap.parse_args(argv)
    txi = args.txi.read_bytes() if args.txi else None
    try:
        data = tga_to_tpc(args.tga.read_bytes(), force_opaque=args.force_opaque, txi=txi)
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    args.tpc.write_bytes(data)
    print(f"wrote {args.tpc} ({len(data)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
