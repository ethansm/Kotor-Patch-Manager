#!/usr/bin/env python3
"""DXT1/DXT5 block codec for the K1-port texture pipeline (numpy only).

Why: pykotor's DXT encoder is lossy and wrong (pure red comes back as (49,0,54)), so the port
uses this block-level codec instead. It is the only module that knows block layouts.
All arrays are in STORED row order (row 0 = first block row in the file); nothing is flipped.
Palette and alpha interpolation use integer floor division, as GPU decoders do.
"""
import numpy as np

BLOCK_BYTES = {"dxt1": 8, "dxt5": 16}
_IDX16 = np.arange(16)
_VALUES = np.arange(256)


def _lut(bits: int) -> np.ndarray:
    """8-bit value -> code whose bit-replicated expansion is nearest to it."""
    codes = np.arange(1 << bits)
    expanded = _expand(codes, bits)
    return np.abs(expanded[None, :] - _VALUES[:, None]).argmin(axis=1)


def _expand(code: np.ndarray, bits: int) -> np.ndarray:
    """Bit-replicate a 5- or 6-bit code to 8 bits."""
    return ((code << (8 - bits)) | (code >> (2 * bits - 8))).astype(np.int32)


_LUT5 = _lut(5)
_LUT6 = _lut(6)


def _unpack565(p: np.ndarray) -> np.ndarray:
    """uint16 RGB565 codes (...,) -> expanded 8-bit RGB (...,3) int32."""
    p = p.astype(np.int32)
    return np.stack(
        [_expand((p >> 11) & 31, 5), _expand((p >> 5) & 63, 6), _expand(p & 31, 5)], axis=-1
    )


def _pack565(rgb: np.ndarray) -> np.ndarray:
    """RGB (...,3) in 0..255 -> nearest RGB565 codes (...,) int64."""
    q = np.clip(np.rint(rgb), 0, 255).astype(np.int64)
    return (_LUT5[q[..., 0]] << 11) | (_LUT6[q[..., 1]] << 5) | _LUT5[q[..., 2]]


def _grid(w: int, h: int) -> tuple[int, int]:
    """Block grid for a w x h image: returns (by, bx)."""
    return (h + 3) // 4, (w + 3) // 4


def _gather(table: np.ndarray, idx: np.ndarray) -> np.ndarray:
    """table (by,bx,K,...) indexed by idx (by,bx,16) -> (by,bx,16,...)."""
    by, bx = idx.shape[:2]
    return table[np.arange(by)[:, None, None], np.arange(bx)[None, :, None], idx]


def _tile(a: np.ndarray, by: int, bx: int, h: int, w: int) -> np.ndarray:
    """Per-block pixel arrays (by,bx,16,...) -> image (h,w,...) in stored order."""
    rest = a.shape[3:]
    a = a.reshape((by, bx, 4, 4) + rest).swapaxes(1, 2)
    return a.reshape((4 * by, 4 * bx) + rest)[:h, :w]


def _colour(halves: np.ndarray, allow_3: bool) -> tuple[np.ndarray, np.ndarray]:
    """Colour halves (by,bx,8) -> RGB (by,bx,16,3) uint8 and DXT1 transparent mask (by,bx,16).

    allow_3 enables DXT1 3-colour mode (c0 <= c1, index 3 transparent). DXT5 colour is always 4-colour.
    """
    c0 = halves[..., 0].astype(np.int32) | (halves[..., 1].astype(np.int32) << 8)
    c1 = halves[..., 2].astype(np.int32) | (halves[..., 3].astype(np.int32) << 8)
    word = (halves[..., 4:8].astype(np.int64) << (8 * np.arange(4))).sum(axis=-1)
    idx = (word[..., None] >> (2 * _IDX16)) & 3
    e0, e1 = _unpack565(c0), _unpack565(c1)
    pal = np.stack([e0, e1, (2 * e0 + e1) // 3, (e0 + 2 * e1) // 3], axis=2)
    three = (c0 <= c1) if allow_3 else np.zeros(c0.shape, dtype=bool)
    pal[:, :, 2] = np.where(three[..., None], (e0 + e1) // 2, pal[:, :, 2])
    pal[:, :, 3] = np.where(three[..., None], 0, pal[:, :, 3])
    return _gather(pal, idx).astype(np.uint8), three[..., None] & (idx == 3)


def _alpha(half: np.ndarray) -> np.ndarray:
    """DXT5 alpha half (by,bx,8) -> alpha (by,bx,16) uint8 (8-alpha mode if a0 > a1, else 6-alpha)."""
    a0 = half[..., 0].astype(np.int64)
    a1 = half[..., 1].astype(np.int64)
    word = (half[..., 2:8].astype(np.int64) << (8 * np.arange(6))).sum(axis=-1)
    idx = (word[..., None] >> (3 * _IDX16)) & 7
    zero, full = np.zeros_like(a0), np.full_like(a0, 255)
    t8 = [a0, a1, (6 * a0 + a1) // 7, (5 * a0 + 2 * a1) // 7, (4 * a0 + 3 * a1) // 7,
          (3 * a0 + 4 * a1) // 7, (2 * a0 + 5 * a1) // 7, (a0 + 6 * a1) // 7]
    t6 = [a0, a1, (4 * a0 + a1) // 5, (3 * a0 + 2 * a1) // 5, (2 * a0 + 3 * a1) // 5,
          (a0 + 4 * a1) // 5, zero, full]
    tab = np.where((a0 > a1)[..., None], np.stack(t8, axis=-1), np.stack(t6, axis=-1))
    return _gather(tab, idx).astype(np.uint8)


def decode(data: bytes, w: int, h: int, fmt: str) -> np.ndarray:
    """Block data of one mip level -> (h,w,4) uint8 RGBA. Padded edge blocks are cropped.

    DXT1: index 3 of a c0 <= c1 block is transparent (alpha 0, else 255).
    DXT5: alpha from the 8-byte alpha half (6- or 8-value interpolation); colour is 4-colour only.
    """
    if fmt not in BLOCK_BYTES:
        raise ValueError(f"unknown DXT format {fmt!r}")
    by, bx = _grid(w, h)
    nb = BLOCK_BYTES[fmt]
    need = by * bx * nb
    if len(data) < need:
        raise ValueError(f"{fmt} {w}x{h} needs {need} bytes, got {len(data)}")
    blocks = np.frombuffer(data, dtype=np.uint8, count=need).reshape(by, bx, nb)
    if fmt == "dxt1":
        rgb, transparent = _colour(blocks[..., :8], allow_3=True)
        alpha = np.where(transparent, 0, 255).astype(np.uint8)
    else:
        rgb, _ = _colour(blocks[..., 8:], allow_3=False)
        alpha = _alpha(blocks[..., :8])
    return _tile(np.concatenate([rgb, alpha[..., None]], axis=-1), by, bx, h, w)


def _fit(pix: np.ndarray, lo: np.ndarray, hi: np.ndarray) -> tuple[np.ndarray, ...]:
    """Quantise endpoints to RGB565, force c0 > c1, choose nearest palette index per pixel.

    pix (by,bx,16,3) int32; lo/hi (by,bx,3) float. Returns c0, c1, idx (by,bx,16), err (by,bx).
    """
    p0, p1 = _pack565(lo), _pack565(hi)
    top, eq = np.maximum(p0, p1), p0 == p1
    c0 = np.where(eq & (top < 0xFFFF), top + 1, top)
    c1 = np.where(eq & (top == 0xFFFF), 0xFFFE, np.minimum(p0, p1))
    e0, e1 = _unpack565(c0), _unpack565(c1)
    pal = np.stack([e0, e1, (2 * e0 + e1) // 3, (e0 + 2 * e1) // 3], axis=2)
    dist = np.stack([((pix - pal[:, :, k, None, :]) ** 2).sum(axis=-1) for k in range(4)], axis=-1)
    return c0, c1, dist.argmin(axis=-1), dist.min(axis=-1).sum(axis=-1)


def _pick(a: tuple, b: tuple) -> tuple:
    """Per block, keep whichever fit has the lower error."""
    m = b[3] < a[3]
    return (np.where(m, b[0], a[0]), np.where(m, b[1], a[1]),
            np.where(m[..., None], b[2], a[2]), np.where(m, b[3], a[3]))


def _refine(pf: np.ndarray, idx: np.ndarray, e0: np.ndarray, e1: np.ndarray):
    """Least-squares endpoints for the assigned pixels; falls back to e0/e1 where singular."""
    t = np.array([0.0, 1.0, 1.0 / 3.0, 2.0 / 3.0])  # palette k = (1-t_k)*c0 + t_k*c1
    a = 1.0 - t
    oh = (idx[..., None] == np.arange(4)).astype(np.float64)
    n = oh.sum(axis=2)
    s = np.einsum("bxpk,bxpc->bxkc", oh, pf)
    sxx, sxy, syy = (n * a * a).sum(-1), (n * a * t).sum(-1), (n * t * t).sum(-1)
    rx, ry = np.einsum("bxkc,k->bxc", s, a), np.einsum("bxkc,k->bxc", s, t)
    det = sxx * syy - sxy * sxy
    ok = (det > 1e-6)[..., None]
    d = np.where(ok, det[..., None], 1.0)
    c0 = (syy[..., None] * rx - sxy[..., None] * ry) / d
    c1 = (sxx[..., None] * ry - sxy[..., None] * rx) / d
    return np.clip(np.where(ok, c0, e0), 0, 255), np.clip(np.where(ok, c1, e1), 0, 255)


def _pack(best: tuple) -> np.ndarray:
    """Fit result -> (by,bx,8) uint8 DXT1 colour halves (c0 LE, c1 LE, 32-bit index word LE)."""
    c0, c1, idx, _ = best
    word = (idx.astype(np.int64) << (2 * _IDX16)).sum(axis=-1)
    cols = [c0 & 255, c0 >> 8, c1 & 255, c1 >> 8,
            word & 255, (word >> 8) & 255, (word >> 16) & 255, word >> 24]
    return np.stack(cols, axis=-1).astype(np.uint8)


def encode_rgb(rgb: np.ndarray) -> np.ndarray:
    """(h,w,3) uint8 -> (by,bx,8) uint8 DXT1 colour halves, always 4-colour mode (c0 > c1).

    Partial edge blocks are padded by edge replication. Endpoints start from a principal-axis
    fit and a bounding-box fit (lower error wins per block), then one least-squares refinement.
    """
    rgb = np.asarray(rgb)
    if rgb.ndim != 3 or rgb.shape[2] != 3:
        raise ValueError("encode_rgb expects an (h,w,3) array")
    h, w = rgb.shape[:2]
    by, bx = _grid(w, h)
    pad = np.pad(rgb, ((0, 4 * by - h), (0, 4 * bx - w), (0, 0)), mode="edge")
    pix = pad.reshape(by, 4, bx, 4, 3).swapaxes(1, 2).reshape(by, bx, 16, 3).astype(np.int32)
    pf = pix.astype(np.float64)
    mean = pf.mean(axis=2)
    cen = pf - mean[:, :, None, :]
    cov = np.einsum("bxpi,bxpj->bxij", cen, cen) / 16.0
    axis = np.linalg.eigh(cov)[1][..., :, -1]
    t = np.einsum("bxpi,bxi->bxp", cen, axis)
    pca = _fit(pix, mean + t.min(axis=-1)[..., None] * axis, mean + t.max(axis=-1)[..., None] * axis)
    box = _fit(pix, pf.min(axis=2), pf.max(axis=2))
    best = _pick(pca, box)
    e0 = _unpack565(best[0]).astype(np.float64)
    e1 = _unpack565(best[1]).astype(np.float64)
    r0, r1 = _refine(pf, best[2], e0, e1)
    return _pack(_pick(best, _fit(pix, r0, r1)))


def block_mask(mask: np.ndarray) -> np.ndarray:
    """(h,w) bool -> (by,bx) bool, True if any pixel of the 4x4 block is set."""
    mask = np.asarray(mask, dtype=bool)
    h, w = mask.shape
    by, bx = _grid(w, h)
    pad = np.zeros((4 * by, 4 * bx), dtype=bool)
    pad[:h, :w] = mask
    return pad.reshape(by, 4, bx, 4).any(axis=(1, 3))


def _three_colour_half(new: np.ndarray, parent: np.ndarray) -> np.ndarray:
    """(k,8) new DXT1 colour halves + (k,8) parent blocks -> (k,8) 3-colour halves for the same pixels.

    Endpoints are the new block's two 565 words, low then high. Each parent-opaque pixel takes the nearest of
    c0, c1, (c0+c1)/2 on its decoded new colour; parent index-3 (transparent) pixels stay 3.
    """
    shift = 2 * np.arange(16, dtype=np.int64)
    n0 = new[:, 0].astype(np.int64) | (new[:, 1].astype(np.int64) << 8)
    n1 = new[:, 2].astype(np.int64) | (new[:, 3].astype(np.int64) << 8)
    nword = sum(new[:, 4 + i].astype(np.int64) << (8 * i) for i in range(4))
    pword = sum(parent[:, 4 + i].astype(np.int64) << (8 * i) for i in range(4))
    nidx = (nword[:, None] >> shift) & 3
    opaque = ((pword[:, None] >> shift) & 3) != 3
    a, b = _unpack565(n0).astype(np.float64), _unpack565(n1).astype(np.float64)
    four = np.stack([a, b, (2 * a + b) // 3, (a + 2 * b) // 3], axis=1)  # integer floor, as the decoder
    three = np.stack([a, b, (a + b) // 2, np.zeros_like(a)], axis=1)  # index 3 of a 3-colour block decodes black
    pal = np.where((n0 > n1)[:, None, None], four, three)
    rgb = np.take_along_axis(pal, np.repeat(nidx[:, :, None], 3, axis=2), axis=1)
    lo, hi = np.minimum(n0, n1), np.maximum(n0, n1)
    e0, e1 = _unpack565(lo).astype(np.float64), _unpack565(hi).astype(np.float64)
    ends = np.stack([e0, e1, (e0 + e1) // 2], axis=1)
    near = ((rgb[:, :, None, :] - ends[:, None, :, :]) ** 2).sum(-1).argmin(-1)
    return _pack((lo, hi, np.where(opaque, near, 3), None))


def splice_colour(parent: bytes, w: int, h: int, fmt: str,
                  new_colour: np.ndarray, changed: np.ndarray) -> bytes:
    """Replace the colour half of changed blocks in one raw mip level; everything else is kept.

    DXT1: all 8 bytes of a changed block are replaced. A changed block with a 3-colour parent (c0<=c1, 1-bit alpha
    mode) is re-encoded in 3-colour mode with its transparent (index 3) pixels kept, so alpha never changes; a
    4-colour parent with a 3-colour new block keeps the parent's bytes. DXT5: bytes 8..16 are
    replaced and the alpha half (bytes 0..8) is always kept from the parent. Output length equals parent length.
    """
    nb = BLOCK_BYTES[fmt]
    by, bx = _grid(w, h)
    n = by * bx * nb
    if len(parent) < n:
        raise ValueError(f"{fmt} {w}x{h} needs {n} parent bytes, got {len(parent)}")
    new = np.asarray(new_colour, dtype=np.uint8)
    ch = np.asarray(changed, dtype=bool)
    if new.shape != (by, bx, 8) or ch.shape != (by, bx):
        raise ValueError(f"expected new_colour {(by, bx, 8)} and changed {(by, bx)}")
    blocks = np.frombuffer(parent, dtype=np.uint8, count=n).reshape(by, bx, nb).copy()
    off = 0 if fmt == "dxt1" else 8
    if fmt == "dxt1":
        p0 = blocks[..., 0].astype(np.uint16) | (blocks[..., 1].astype(np.uint16) << 8)
        p1 = blocks[..., 2].astype(np.uint16) | (blocks[..., 3].astype(np.uint16) << 8)
        n0 = new[..., 0].astype(np.uint16) | (new[..., 1].astype(np.uint16) << 8)
        n1 = new[..., 2].astype(np.uint16) | (new[..., 3].astype(np.uint16) << 8)
        ch = ch & ~((p0 > p1) & (n0 <= n1))  # 4-colour parent, 3-colour new: keep parent (encode_rgb never emits this)
        conv = ch & (p0 <= p1)  # 3-colour parent: re-encode, parent index-3 pixels stay transparent
        if conv.any():
            new = new.copy()  # np.asarray may alias the caller's array
            new[conv] = _three_colour_half(new[conv], blocks[conv])
    blocks[..., off:off + 8] = np.where(ch[..., None], new, blocks[..., off:off + 8])
    return blocks.tobytes() + bytes(parent[n:])
