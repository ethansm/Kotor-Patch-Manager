#!/usr/bin/env python3
"""TPC container layer for the K1 texture port: parse the raw mip chain, decode mips, and apply RGB edits.

Works on raw bytes (128-byte header, back-to-back DXT mips, optional trailing TXI text) and uses the U1 DXT codec
for block work. Edits follow the shared footprint rule: a mip pixel is "in mask" when any mip-0 pixel under it is,
and only blocks touching that footprint are re-encoded. Odd mip dimensions fold the trailing row/column into the
last output pixel, so the box filter matches the container's floor-halved mip sizes.
"""
import struct
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
from k1_port_tex_dxt import BLOCK_BYTES, block_mask, decode, encode_rgb, splice_colour  # noqa: E402  stdlib + numpy

HEADER_SIZE = 128
PIXEL_TYPE_FMT = {2: "dxt1", 4: "dxt5"}  # compressed encodings only; uncompressed is not supported here


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
