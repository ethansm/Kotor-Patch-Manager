#!/usr/bin/env python3
"""Tests for k1_port_tex_tpc.py: TPC layout, mip maths, footprint rule, and RGB edits.

Synthetic parents are built in memory (dxt1 and dxt5, deterministic noise blocks). The footprint check computes the
expected changed-block set by brute force from mip-0 pixels, independent of the box-filter code. The real-file test
reads k1p_commm04.tpc from the staged override dir and SKIPs when it is absent. Under system python3 the file
re-executes itself with the repo venv, which has numpy.
"""
import contextlib
import io
import os
import shutil
import struct
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
VENV_PY = HERE.parents[1] / ".venv" / "bin" / "python"  # HERE = <repo>/patch_manager_mods/k1_port

try:
    import numpy as np
except ImportError:
    if VENV_PY.exists() and not os.environ.get("K1_TEX_REEXEC"):
        os.environ["K1_TEX_REEXEC"] = "1"
        os.execv(str(VENV_PY), [str(VENV_PY), str(Path(__file__).resolve())])
    print("SKIP: numpy not available (run with .venv/bin/python)")
    sys.exit(0)

sys.path.insert(0, str(HERE))
import k1_port_tex_tpc as tpc  # noqa: E402
from k1_port_tex_dxt import BLOCK_BYTES  # noqa: E402

REAL_TPC = Path("~/Games/KOTOR2_Mod_Workspace/k1_port_stage/override/k1p_commm04.tpc").expanduser()
PIXEL_TYPE = {"dxt1": 2, "dxt5": 4}
TESTS = []


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def make_header(w: int, h: int, fmt: str, mips: int, data_size: int) -> bytes:
    head = struct.pack("<IfHHBB", data_size, 1.0, w, h, PIXEL_TYPE[fmt], mips)
    return head + b"\0" * (128 - len(head))


def make_parent(w: int, h: int, fmt: str, mips: int, seed: int = 7, trailer: bytes = b"") -> bytes:
    sizes = [((max(1, w >> k) + 3) // 4) * ((max(1, h >> k) + 3) // 4) * BLOCK_BYTES[fmt] for k in range(mips)]
    body = np.random.default_rng(seed).integers(0, 256, size=sum(sizes), dtype=np.uint8).tobytes()
    return make_header(w, h, fmt, mips, sizes[0]) + body + trailer


def mip_slices(data: bytes, info: dict) -> list:
    return [data[o:o + s] for o, s in zip(info["offsets"], info["sizes"])]


def blocks_of(data: bytes, mw: int, mh: int, bb: int) -> list:
    """Per-block byte strings in stored order (row-major, row 0 first)."""
    count = ((mw + 3) // 4) * ((mh + 3) // 4)
    return [data[i * bb:(i + 1) * bb] for i in range(count)]


def spans(n: int, k: int) -> list:
    """Mip-0 index span (lo, hi) under each mip-k pixel along one axis, folding odd trailing indices."""
    cur = [(i, i + 1) for i in range(n)]
    for _ in range(k):
        m = max(1, len(cur) >> 1)
        nxt = []
        for x in range(m):
            lo = cur[2 * x][0]
            hi = cur[2 * x + 1][1] if x < m - 1 else cur[-1][1]
            nxt.append((lo, hi))
        cur = nxt
    return cur


def expected_mask(mask0: np.ndarray, k: int) -> np.ndarray:
    """Brute force: a mip-k pixel is in mask if any mip-0 pixel under its footprint is."""
    sy, sx = spans(mask0.shape[0], k), spans(mask0.shape[1], k)
    out = np.zeros((len(sy), len(sx)), dtype=bool)
    for j, (y0, y1) in enumerate(sy):
        for i, (x0, x1) in enumerate(sx):
            out[j, i] = bool(mask0[y0:y1, x0:x1].any())
    return out


def expected_blocks(mask_k: np.ndarray, mw: int, mh: int) -> list:
    """A 4x4 block may change only if one of its clipped mip-k pixels is in mask."""
    bxn, byn = (mw + 3) // 4, (mh + 3) // 4
    return [bool(mask_k[4 * by:4 * by + 4, 4 * bx:4 * bx + 4].any()) for by in range(byn) for bx in range(bxn)]


def check_footprint(parent: bytes, out: bytes, mask0: np.ndarray) -> list:
    """Assert only footprint blocks changed at every mip (and DXT5 alpha halves never did); return changed counts."""
    info = tpc.parse(parent)
    w, h, fmt = info["w"], info["h"], info["fmt"]
    bb = BLOCK_BYTES[fmt]
    p_mips, o_mips = mip_slices(parent, info), mip_slices(out, info)
    counts = []
    for k in range(info["mips"]):
        mw, mh = tpc.mip_dims(w, h, k)
        exp = expected_blocks(expected_mask(mask0, k), mw, mh)
        pb, ob = blocks_of(p_mips[k], mw, mh, bb), blocks_of(o_mips[k], mw, mh, bb)
        check(len(pb) == len(exp), f"mip {k}: block count {len(pb)} != expected {len(exp)}")
        changed = 0
        for i, (p, o) in enumerate(zip(pb, ob)):
            if not exp[i]:
                check(p == o, f"mip {k} block {i} is outside the mask footprint but changed")
            elif p != o:
                changed += 1
            if fmt == "dxt5":
                check(p[:8] == o[:8], f"mip {k} block {i}: DXT5 alpha half changed")
        counts.append(changed)
    return counts


@test
def test_parse_layout(root: Path):
    trailer = b"[TXI]\r\nfake_txi_text\r\n"
    for fmt, scale in (("dxt1", 1), ("dxt5", 2)):
        b = make_parent(64, 64, fmt, 7, trailer=trailer)
        info = tpc.parse(b)
        check(info["w"] == 64 and info["h"] == 64 and info["fmt"] == fmt, f"{fmt}: header fields wrong {info}")
        check(info["mips"] == 7, f"{fmt}: mips {info['mips']} != 7")
        want = [s * scale for s in (2048, 512, 128, 32, 8, 8, 8)]
        check(info["sizes"] == want, f"{fmt}: sizes {info['sizes']} != {want}")
        check(info["offsets"][0] == 128, f"{fmt}: first mip offset {info['offsets'][0]} != 128")
        for k in range(1, 7):
            check(info["offsets"][k] == info["offsets"][k - 1] + info["sizes"][k - 1], f"{fmt}: offset gap at {k}")
        check(sum(info["sizes"]) == len(b) - 128 - len(trailer), f"{fmt}: mip bytes do not sum to body length")
        check(info["trailer"] == trailer, f"{fmt}: trailer not preserved")
        check(info["header"] == b[:128], f"{fmt}: header slice wrong")


@test
def test_parse_rejects_bad_input(root: Path):
    good = make_parent(64, 64, "dxt1", 7)
    cases = {
        "short": good[:100],
        "uncompressed": struct.pack("<IfHHBB", 0, 1.0, 64, 64, 2, 7) + b"\0" * 114 + good[128:],
        "bad pixel_type": struct.pack("<IfHHBB", 2048, 1.0, 64, 64, 12, 7) + b"\0" * 114 + good[128:],
        "truncated": good[:-5],
        "data_size mismatch": make_header(64, 64, "dxt1", 7, 999) + good[128:],
        "zero mips": make_header(64, 64, "dxt1", 0, 2048) + good[128:],
    }
    for name, data in cases.items():
        try:
            tpc.parse(data)
        except ValueError:
            continue
        raise AssertionError(f"parse accepted bad input: {name}")


@test
def test_mip_dims(root: Path):
    want = [(60, 28), (30, 14), (15, 7), (7, 3), (3, 1), (1, 1), (1, 1)]
    got = [tpc.mip_dims(60, 28, k) for k in range(7)]
    check(got == want, f"mip_dims(60,28) {got} != {want}")
    check(tpc.mip_dims(1, 1, 3) == (1, 1), "1x1 mip must stay 1x1")


@test
def test_downsample_rgb_rules(root: Path):
    two = np.zeros((2, 2, 3), dtype=np.uint8)
    two[0, 0, 0] = 1
    two[0, 1, 0] = 1  # R sum 2 of 4 -> 0.5 rounds half up to 1
    check(tpc.downsample_rgb(two)[0, 0, 0] == 1, "half must round up")
    two[0, 1, 0] = 0  # sum 1 of 4 -> 0.25 -> 0
    check(tpc.downsample_rgb(two)[0, 0, 0] == 0, "quarter must round down")
    one = np.array([[[7, 8, 9]]], dtype=np.uint8)
    check(np.array_equal(tpc.downsample_rgb(one), one), "1x1 must be identity")
    three = np.zeros((3, 3, 3), dtype=np.uint8)
    three[0, 0, 0] = 5  # 5 of 9 -> 0.56 -> 1, all 3x3 folded into one pixel
    out3 = tpc.downsample_rgb(three)
    check(out3.shape == (1, 1, 3) and out3[0, 0, 0] == 1, f"3x3 fold wrong: {out3.shape} {out3[0, 0, 0]}")
    five = np.zeros((1, 5, 3), dtype=np.uint8)
    five[0, 4, 0] = 9  # last output covers cols 2..4: 9/3 = 3
    out5 = tpc.downsample_rgb(five)
    check(out5.shape == (1, 2, 3), f"5-wide shape {out5.shape} != (1, 2, 3)")
    check(out5[0, 1, 0] == 3 and out5[0, 0, 0] == 0, f"5-wide fold values {out5[0, :, 0]}")


@test
def test_downsample_mask_rules(root: Path):
    m = np.zeros((2, 2), dtype=bool)
    m[1, 0] = True
    check(tpc.downsample_mask(m).tolist() == [[True]], "any-of-2x2 must be True")
    row = np.array([[False, False, True]])
    check(tpc.downsample_mask(row).tolist() == [[True]], "edge-folded trailing pixel must count")
    check(not tpc.downsample_mask(np.zeros((28, 60), dtype=bool)).any(), "empty mask must stay empty")
    check(tpc.downsample_mask(np.zeros((28, 60), dtype=bool)).shape == (14, 30), "60x28 mask must halve to 14x30")


@test
def test_empty_mask_identity(root: Path):
    for fmt in ("dxt1", "dxt5"):
        b = make_parent(64, 64, fmt, 7, trailer=b"TXI")
        out = tpc.apply_rgb_edit(b, np.zeros((64, 64, 3), dtype=np.uint8), np.zeros((64, 64), dtype=bool))
        check(out == b, f"{fmt}: empty mask must return the parent byte-for-byte")


@test
def test_masked_edit_footprint(root: Path):
    for fmt in ("dxt1", "dxt5"):
        b = make_parent(64, 64, fmt, 7, trailer=b"TXI")
        mask = np.zeros((64, 64), dtype=bool)
        mask[5:21, 37:45] = True
        rgb = np.zeros((64, 64, 3), dtype=np.uint8)
        rgb[:, :] = (255, 0, 0)
        out = tpc.apply_rgb_edit(b, rgb, mask)
        check(len(out) == len(b), f"{fmt}: length changed {len(b)} -> {len(out)}")
        check(out[:128] == b[:128], f"{fmt}: header changed")
        check(out[-3:] == b"TXI", f"{fmt}: trailer changed")
        counts = check_footprint(b, out, mask)
        check(counts[0] > 0, f"{fmt}: mip 0 has no changed block inside the mask")


@test
def test_non_pow2_60x28(root: Path):
    for fmt in ("dxt1", "dxt5"):
        b = make_parent(60, 28, fmt, 7, seed=11)
        mask = np.zeros((28, 60), dtype=bool)
        mask[20:28, 55:60] = True  # touches the last row and column, the odd-fold edge
        rgb = np.zeros((28, 60, 3), dtype=np.uint8)
        rgb[:, :] = (0, 0, 255)
        out = tpc.apply_rgb_edit(b, rgb, mask)
        check(len(out) == len(b), f"{fmt} 60x28: length changed")
        counts = check_footprint(b, out, mask)
        check(counts[0] > 0, f"{fmt} 60x28: no mip-0 block changed")


@test
def test_red_tint_decode(root: Path):
    for fmt in ("dxt1", "dxt5"):
        b = make_parent(64, 64, fmt, 7, seed=3)
        mask = np.zeros((64, 64), dtype=bool)
        mask[8:24, 16:32] = True  # block aligned, so every pixel outside the mask sits in an untouched block
        info = tpc.parse(b)
        before = tpc.decode_mip(b, info, 0)
        rgb = before[:, :, :3].copy()
        rgb[mask] = (255, 0, 0)
        out = tpc.apply_rgb_edit(b, rgb, mask)
        after = tpc.decode_mip(out, tpc.parse(out), 0)
        inside = after[mask][:, :3].astype(int)
        red_shift = inside[:, 0] - (inside[:, 1] + inside[:, 2]) / 2
        check(red_shift.mean() > 120, f"{fmt}: masked pixels not red-shifted, mean shift {red_shift.mean():.1f}")
        check(np.array_equal(after[~mask], before[~mask]), f"{fmt}: pixels outside the mask changed")
        # the mean alone hides skipped blocks (a random DXT1 parent is ~half 3-colour): every masked block must turn red
        for by in range(8, 24, 4):
            for bx in range(16, 32, 4):
                blk = after[by:by + 4, bx:bx + 4, :3].astype(int)
                best = (blk[..., 0] - (blk[..., 1] + blk[..., 2]) / 2).max()
                check(best > 120, f"{fmt}: masked block at pixel ({bx},{by}) was not edited, best red shift {best:.1f}")


@test
def test_red_tint_every_masked_pixel_on_four_colour_parent(root: Path):
    for fmt in ("dxt1", "dxt5"):
        raw = bytearray(make_parent(64, 64, fmt, 7, seed=3))
        info = tpc.parse(bytes(raw))
        bb, col = BLOCK_BYTES[fmt], 0 if fmt == "dxt1" else 8
        for off, size in zip(info["offsets"], info["sizes"]):  # every level 4-colour only: c0 = 0xFFFF > c1 = 0
            for o in range(off + col, off + size, bb):
                raw[o:o + 4] = b"\xff\xff\x00\x00"
        b = bytes(raw)
        mask = np.zeros((64, 64), dtype=bool)
        mask[8:24, 16:32] = True  # block aligned
        before = tpc.decode_mip(b, info, 0)
        rgb = before[:, :, :3].copy()
        rgb[mask] = (255, 0, 0)
        out = tpc.apply_rgb_edit(b, rgb, mask)
        after = tpc.decode_mip(out, tpc.parse(out), 0)
        px = after[mask][:, :3].astype(int)
        shift = px[:, 0] - (px[:, 1] + px[:, 2]) / 2
        check(shift.min() > 120, f"{fmt}: a masked pixel is not red, min shift {shift.min():.1f}")
        check(np.array_equal(after[~mask], before[~mask]), f"{fmt}: pixels outside the mask changed")


@test
def test_edit_on_3colour_parent_lands_on_opaque_pixels(root: Path):
    raw = bytearray(make_parent(64, 64, "dxt1", 7, seed=3))
    info = tpc.parse(bytes(raw))
    off, size = info["offsets"][0], info["sizes"][0]
    blk = np.frombuffer(bytes(raw[off:off + size]), dtype=np.uint8).reshape(-1, 8).copy()
    blk[:, 0:4] = (0x00, 0x00, 0xFF, 0xFF)  # c0 <= c1 everywhere: every mip-0 block is 3-colour
    b = bytes(raw[:off]) + blk.tobytes() + bytes(raw[off + size:])

    def index_map(data):  # (64, 64) DXT1 index per mip-0 pixel
        d = np.frombuffer(data[off:off + size], dtype=np.uint8).reshape(16, 16, 8).astype(np.int64)
        word = d[..., 4] | (d[..., 5] << 8) | (d[..., 6] << 16) | (d[..., 7] << 24)
        return ((word[..., None] >> (2 * np.arange(16))) & 3).reshape(16, 16, 4, 4).swapaxes(1, 2).reshape(64, 64)

    mask = np.zeros((64, 64), dtype=bool)
    mask[8:24, 16:32] = True  # block aligned
    before = tpc.decode_mip(b, info, 0)
    rgb = before[:, :, :3].copy()
    rgb[mask] = (255, 0, 0)
    out = tpc.apply_rgb_edit(b, rgb, mask)
    idx_in, idx_out = index_map(b), index_map(out)
    check(np.array_equal(idx_out == 3, idx_in == 3), "transparent (index 3) pixels changed")
    after = tpc.decode_mip(out, tpc.parse(out), 0)
    ob = np.frombuffer(out[off:off + size], dtype=np.uint8).reshape(-1, 8)
    c0, c1 = ob[:, 0] | (ob[:, 1].astype(int) << 8), ob[:, 2] | (ob[:, 3].astype(int) << 8)
    check(bool((c0 <= c1).all()), f"{int((c0 > c1).sum())} mip-0 block(s) lost 3-colour mode (c0 > c1)")
    check(np.array_equal(after[..., 3], before[..., 3]), "decoded mip-0 alpha changed")
    px = after[mask & (idx_in != 3)].astype(int)
    shift = px[:, 0] - (px[:, 1] + px[:, 2]) / 2
    check(shift.min() > 120, f"opaque masked pixels not red-shifted, min shift {shift.min():.1f}")
    check(np.array_equal(after[~mask], before[~mask]), "pixels outside the mask changed")


@test
def test_bad_edit_shapes_rejected(root: Path):
    b = make_parent(64, 64, "dxt1", 7)
    ok_mask = np.zeros((64, 64), dtype=bool)
    ok_rgb = np.zeros((64, 64, 3), dtype=np.uint8)
    for rgb, mask in ((np.zeros((32, 64, 3), dtype=np.uint8), ok_mask), (ok_rgb, np.zeros((64, 32), dtype=bool))):
        try:
            tpc.apply_rgb_edit(b, rgb, mask)
        except ValueError:
            continue
        raise AssertionError("apply_rgb_edit accepted a mis-shaped input")


@test
def test_red_tint_on_three_colour_parent(root: Path):
    # every mip-0 block 3-colour (c0 == c1 == 0, opaque index 0); checkerboard of fully transparent blocks
    b = bytearray(make_parent(64, 64, "dxt1", 7, seed=3))
    off = tpc.parse(bytes(b))["offsets"][0]
    m0 = np.frombuffer(bytes(b), dtype=np.uint8, count=16 * 16 * 8, offset=off).reshape(16, 16, 8).copy()
    m0[..., :4] = 0
    m0[..., 4:] = 0
    odd = (np.add.outer(np.arange(16), np.arange(16)) % 2) == 1
    m0[odd, 4:] = 0xFF
    b[off:off + m0.size] = m0.tobytes()
    parent = bytes(b)
    before = tpc.decode_mip(parent, tpc.parse(parent), 0)
    mask = np.zeros((64, 64), dtype=bool)
    mask[8:24, 16:32] = True  # block aligned: block rows 2..5, block cols 4..7
    rgb = before[:, :, :3].copy()
    rgb[mask] = (255, 0, 0)
    out = tpc.apply_rgb_edit(parent, rgb, mask)
    O = np.frombuffer(out, dtype=np.uint8, count=m0.size, offset=off).reshape(16, 16, 8)
    check(np.all(O[2:6, 4:8][odd[2:6, 4:8]][:, 4:] == 0xFF), "transparent masked blocks lost index 3")
    c0, c1 = O[..., 0].astype(int) | (O[..., 1].astype(int) << 8), O[..., 2].astype(int) | (O[..., 3].astype(int) << 8)
    check(bool((c0 <= c1).all()), f"{int((c0 > c1).sum())} mip-0 block(s) lost 3-colour mode (c0 > c1)")
    after = tpc.decode_mip(out, tpc.parse(out), 0)
    check(np.array_equal(after[..., 3], before[..., 3]), "decoded mip-0 alpha changed")
    opaque = mask & ~np.kron(odd, np.ones((4, 4), dtype=bool))
    inside = after[opaque][:, :3].astype(int)
    red_shift = inside[:, 0] - (inside[:, 1] + inside[:, 2]) / 2
    check(red_shift.min() > 120, f"opaque masked pixel not red, min shift {red_shift.min():.1f}")
    check(np.array_equal(after[~mask], before[~mask]), "pixels outside the mask changed")


@test
def test_real_file_commm04(root: Path):
    if not REAL_TPC.exists():
        raise Skip(f"{REAL_TPC} not found")
    data = REAL_TPC.read_bytes()
    info = tpc.parse(data)
    check((info["w"], info["h"], info["fmt"], info["mips"]) == (512, 512, "dxt1", 10), f"header {info}")
    check(info["trailer"] == b"", f"unexpected trailer of {len(info['trailer'])} bytes")
    check(info["sizes"][0] == 131072 and info["sizes"][-3:] == [8, 8, 8], f"sizes {info['sizes']}")
    out = tpc.apply_rgb_edit(data, np.zeros((512, 512, 3), dtype=np.uint8), np.zeros((512, 512), dtype=bool))
    check(out == data, "empty-mask edit of k1p_commm04 is not byte-identical")


def make_tga(rgba: np.ndarray, bpp: int = 32, top_left: bool = False, img_type: int = 2, id_field: bytes = b"",
             cmap_type: int = 0, depth: int = 0) -> bytes:
    """Uncompressed TGA for tests: rgba (h, w, 4) top-down, stored bottom-first unless top_left."""
    h, w = rgba.shape[:2]
    rows = rgba if top_left else rgba[::-1]
    order = [2, 1, 0, 3] if bpp == 32 else [2, 1, 0]
    desc = (0x20 if top_left else 0) | (8 if bpp == 32 else 0)
    head = struct.pack("<BBBHHBHHHHBB", len(id_field), cmap_type, img_type, 0, 0, 0, 0, 0, w, h,
                       depth or bpp, desc)
    return head + id_field + np.ascontiguousarray(rows[..., order]).tobytes()


def flat_rgba(h: int, w: int, rgb: tuple, alpha: int = 255) -> np.ndarray:
    out = np.zeros((h, w, 4), dtype=np.uint8)
    out[..., :3] = rgb
    out[..., 3] = alpha
    return out


def expect_value_error(fn, text: str) -> None:
    try:
        fn()
    except ValueError as exc:
        check(text in str(exc), f"ValueError {exc!s} does not mention {text!r}")
        return
    raise AssertionError(f"expected ValueError mentioning {text!r}")


@test
def test_tga_to_tpc_header_and_mips(root: Path):
    out = tpc.tga_to_tpc(make_tga(flat_rgba(32, 64, (120, 60, 200)), top_left=True), txi=b"clamp 1\n")  # 64 wide, 32 tall
    head = struct.unpack_from("<IfHHBB", out, 0)
    check(head == (1024, 1.0, 64, 32, 2, 7), f"header {head} != (1024, 1.0, 64, 32, 2, 7)")
    check(out[14:128] == b"\0" * 114, "header padding is not zero")
    info = tpc.parse(out)
    check(info["sizes"] == [1024, 256, 64, 16, 8, 8, 8], f"sizes {info['sizes']}")
    check(info["trailer"] == b"clamp 1\n", f"TXI trailer {info['trailer']!r}")
    check(len(out) == 128 + sum(info["sizes"]) + len(b"clamp 1\n"), "TPC length is not header + mips + trailer")


@test
def test_tga_to_tpc_non_pow2_chain(root: Path):
    out = tpc.tga_to_tpc(make_tga(flat_rgba(28, 60, (255, 255, 255)), bpp=24, top_left=True))
    info = tpc.parse(out)
    dims = [tpc.mip_dims(60, 28, k) for k in range(info["mips"])]
    check(info["mips"] == 6 and dims[-1] == (1, 1), f"60x28 chain {dims}")
    want = [((mw + 3) // 4) * ((mh + 3) // 4) * 8 for mw, mh in dims]
    check(info["sizes"] == want, f"60x28 sizes {info['sizes']} != {want}")
    check(len(out) == 128 + sum(want), "60x28 TPC length wrong")


@test
def test_tga_origin_bottom_left_and_top_left_agree(root: Path):
    rgba = flat_rgba(64, 64, (0, 0, 0))
    rgba[:32, :, :3] = (255, 0, 0)  # top half red
    rgba[32:, :, :3] = (0, 0, 255)  # bottom half blue
    bl, tl = make_tga(rgba, top_left=False), make_tga(rgba, top_left=True)
    check(bl[18:22] == b"\xff\x00\x00\xff", "bottom-left file should store the blue bottom row first")
    check(tl[18:22] == b"\x00\x00\xff\xff", "top-left file should store the red top row first")
    check(np.array_equal(tpc.read_tga(bl), rgba), "bottom-left TGA not flipped back to top-down")
    check(np.array_equal(tpc.read_tga(tl), rgba), "top-left TGA read wrong")
    out_bl, out_tl = tpc.tga_to_tpc(bl), tpc.tga_to_tpc(tl)
    check(out_bl == out_tl, "the two origins produce different TPCs")
    mip0 = tpc.decode_mip(out_bl, tpc.parse(out_bl), 0)
    # TPC stored row 0 is the image's bottom row (engine draws it at the bottom), so decode order is flipped.
    check(mip0[0, 0, 2] > 200 and mip0[0, 0, 0] < 60, f"stored row 0 not the blue bottom row: {mip0[0, 0]}")
    check(mip0[63, 63, 0] > 200 and mip0[63, 63, 2] < 60, f"last stored row not the red top row: {mip0[63, 63]}")


@test
def test_tga_right_to_left_mirrors_columns(root: Path):
    rgba = flat_rgba(2, 3, (0, 0, 0))
    rgba[:, 0, 0] = 255  # left column red
    rgba[:, 2, 1] = 255  # right column green
    stored = rgba[:, ::-1]  # right-to-left storage: first stored pixel is the right column
    head = struct.pack("<BBBHHBHHHHBB", 0, 0, 2, 0, 0, 0, 0, 0, 3, 2, 32, 0x20 | 0x10 | 0x08)
    check(np.array_equal(tpc.read_tga(head + np.ascontiguousarray(stored[..., [2, 1, 0, 3]]).tobytes()), rgba),
          "right-to-left TGA not mirrored")


@test
def test_tga_rejects_unsupported(root: Path):
    rgba = flat_rgba(8, 8, (255, 255, 255))
    good = make_tga(rgba)
    cases = {
        "RLE (type 10)": make_tga(rgba, img_type=10),
        "colour-mapped (type 1)": make_tga(rgba, img_type=1),
        "colour map present": make_tga(rgba, cmap_type=1),
        "16 bpp": make_tga(rgba, depth=16),
        "truncated": good[:-7],
        "short header": good[:10],
    }
    for name, data in cases.items():
        try:
            tpc.tga_to_tpc(data)
        except ValueError:
            continue
        raise AssertionError(f"tga_to_tpc accepted {name}")


@test
def test_opaque_rules_and_encoding(root: Path):
    rgba = flat_rgba(16, 16, (10, 200, 90))
    out = tpc.tga_to_tpc(make_tga(rgba))
    check(out[12] == 2, f"pixel_type {out[12]} != 2 (DXT1)")
    check(bool((tpc.decode_mip(out, tpc.parse(out), 0)[..., 3] == 255).all()), "opaque DXT1 has non-opaque pixels")
    semi = rgba.copy()
    semi[0, 0, 3] = 254
    expect_value_error(lambda: tpc.tga_to_tpc(make_tga(semi)), "non-opaque TGA needs DXT5")
    forced = tpc.tga_to_tpc(make_tga(semi), force_opaque=True)
    check(forced[12] == 2, "force_opaque output is not DXT1")
    check(bool((tpc.decode_mip(forced, tpc.parse(forced), 0)[..., 3] == 255).all()), "force_opaque left alpha != 255")
    alpha0 = tpc.tga_to_tpc(make_tga(flat_rgba(16, 16, (10, 200, 90), alpha=0)), force_opaque=True)
    check(alpha0 == tpc.tga_to_tpc(make_tga(rgba), force_opaque=False), "force_opaque on alpha 0 differs from opaque")
    check(tpc.tga_to_tpc(make_tga(flat_rgba(16, 16, (10, 200, 90)), bpp=24)) == out,
          "24-bit TGA (implicit alpha 255) should encode like an opaque 32-bit one")


@test
def test_flat_colour_round_trip(root: Path):
    rgba = flat_rgba(64, 64, (200, 30, 60))
    out = tpc.tga_to_tpc(make_tga(rgba))
    info = tpc.parse(out)
    for k in (0, info["mips"] - 1):
        px = tpc.decode_mip(out, info, k)[..., :3].astype(int)
        err = np.abs(px - [200, 30, 60]).max()
        check(err <= 8, f"mip {k} flat colour off by {err}")


@test
def test_txi_trailer(root: Path):
    tga = make_tga(flat_rgba(16, 16, (1, 2, 3)))
    plain = tpc.tga_to_tpc(tga)
    check(tpc.parse(plain)["trailer"] == b"", "unexpected trailer without txi")
    with_txi = tpc.tga_to_tpc(tga, txi=b"clamp 1\n")
    check(tpc.parse(with_txi)["trailer"] == b"clamp 1\n", "TXI trailer missing or altered")
    check(with_txi[:len(plain)] == plain, "TXI changed the mip data")
    check(tpc.parse(tpc.tga_to_tpc(tga, txi="clamp 1\n"))["trailer"] == b"clamp 1\n", "str txi not encoded")


@test
def test_pykotor_reads_tga_tpc(root: Path):
    try:
        from pykotor.resource.formats.tpc import TPCTextureFormat, read_tpc
    except ImportError as exc:
        raise Skip(f"pykotor not importable: {exc}")
    rgba = flat_rgba(64, 32, (0, 0, 0))
    rgba[:32, :, :3] = (255, 0, 0)
    rgba[32:, :, :3] = (0, 0, 255)
    out = tpc.tga_to_tpc(make_tga(rgba), txi=b"clamp 1\n")
    info = tpc.parse(out)
    t = read_tpc(out)
    mips = t.layers[0].mipmaps
    check([(m.width, m.height) for m in mips] == [tpc.mip_dims(32, 64, k) for k in range(info["mips"])],
          f"pykotor mip dims {[(m.width, m.height) for m in mips]}")
    t.convert(TPCTextureFormat.RGBA)
    ref = np.frombuffer(bytes(t.get(0, 0).data), dtype=np.uint8).reshape(64, 32, 4)
    ours = tpc.decode_mip(out, info, 0)
    err = np.abs(ref[..., :3].astype(int) - ours[..., :3].astype(int)).max()
    check(err <= 8, f"pykotor mip0 differs from decode_mip by {err}")
    check(bool(ref[0, 0, 2] > 200 and ref[63, 0, 0] > 200), "pykotor stored row 0 is not the bottom (blue) row")


@test
def test_cli_writes_tpc(root: Path):
    tga_path, txi_path, out_path = root / "in.tga", root / "in.txi", root / "out.tpc"
    tga_path.write_bytes(make_tga(flat_rgba(32, 32, (9, 9, 9), alpha=0)))
    txi_path.write_bytes(b"clamp 1\n")
    quiet = io.StringIO()
    with contextlib.redirect_stderr(quiet), contextlib.redirect_stdout(quiet):
        check(tpc.main(["tga-to-tpc", str(tga_path), str(out_path)]) == 1, "non-opaque CLI run without flag must fail")
        check(not out_path.exists(), "failed CLI run wrote a file")
        check(tpc.main(["tga-to-tpc", str(tga_path), str(out_path), "--force-opaque", "--txi", str(txi_path)]) == 0,
              "CLI run with --force-opaque failed")
    check(tpc.parse(out_path.read_bytes())["trailer"] == b"clamp 1\n", "CLI output TXI trailer wrong")
    check(out_path.read_bytes() == tpc.tga_to_tpc(tga_path.read_bytes(), force_opaque=True, txi=b"clamp 1\n"),
          "CLI output differs from tga_to_tpc")


def main() -> int:
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_tpc_test_"))
        try:
            fn(root)
            print(f"PASS {fn.__name__}")
            passed += 1
        except Skip as e:
            print(f"SKIP {fn.__name__}: {e}")
            passed += 1  # a missing requirement is not a failure
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
        finally:
            shutil.rmtree(root, ignore_errors=True)
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
