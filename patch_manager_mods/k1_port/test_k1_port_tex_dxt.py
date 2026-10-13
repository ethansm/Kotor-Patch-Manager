#!/usr/bin/env python3
"""Tests for k1_port_tex_dxt.py: DXT1/DXT5 codec, colour splice and block mask.

Synthetic data only, except the real-file checks (staged k1p_commm04.tpc, pykotor). Numpy is required;
if the system python3 lacks it, the file re-execs under the repo .venv. Missing inputs SKIP (count as pass).
"""
import os
import struct
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
try:
    import numpy as np
except ImportError:
    VENV_PY = REPO / ".venv" / "bin" / "python"
    if VENV_PY.exists() and not os.environ.get("K1_TEX_REEXEC"):
        os.execve(str(VENV_PY), [str(VENV_PY), str(Path(__file__).resolve())],
                  dict(os.environ, K1_TEX_REEXEC="1"))
    print("SKIP test_k1_port_tex_dxt: numpy not available")
    sys.exit(0)

sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
from k1_port_tex_dxt import BLOCK_BYTES, block_mask, decode, encode_rgb, splice_colour  # noqa: E402

TPC = Path("~/Games/KOTOR2_Mod_Workspace/k1_port_stage/override/k1p_commm04.tpc").expanduser()
TESTS = []


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def mae(a, b) -> float:
    return float(np.abs(a.astype(np.int32) - b.astype(np.int32)).mean())


def endpoints(enc: np.ndarray):
    c0 = enc[..., 0].astype(np.int32) | (enc[..., 1].astype(np.int32) << 8)
    c1 = enc[..., 2].astype(np.int32) | (enc[..., 3].astype(np.int32) << 8)
    return c0, c1


def pack_dxt1(c0: int, c1: int, idx16) -> bytes:
    """Colour half: c0, c1 (u16 LE) then 16 2-bit indices, row-major, pixel 0 in the low bits."""
    word = sum(int(v) << (2 * i) for i, v in enumerate(idx16))
    return struct.pack("<HHI", c0, c1, word)


def pack_alpha(a0: int, a1: int, idx16) -> bytes:
    """DXT5 alpha half: a0, a1 then 16 3-bit indices (48 bits LE), pixel 0 in the low bits."""
    word = sum(int(v) << (3 * i) for i, v in enumerate(idx16))
    return bytes([a0, a1]) + word.to_bytes(6, "little")


def gradient(h: int, w: int) -> np.ndarray:
    y, x = np.mgrid[0:h, 0:w]
    return np.stack([x * 255 // max(w - 1, 1), y * 255 // max(h - 1, 1), (x + y) * 255 // max(h + w - 2, 1)],
                    axis=-1).astype(np.uint8)


@test
def flat_colours_round_trip():
    for colour in ((255, 0, 0), (0, 0, 255)):
        enc = encode_rgb(np.full((16, 16, 3), colour, np.uint8))
        check(enc.shape == (4, 4, 8), f"{colour}: encode shape {enc.shape}")
        c0, c1 = endpoints(enc)
        check(bool((c0 > c1).all()), f"{colour}: c0 <= c1 in some block")
        out = decode(enc.tobytes(), 16, 16, "dxt1")
        err = int(np.abs(out[..., :3].astype(np.int32) - np.array(colour)).max())
        check(err <= 8, f"{colour}: max channel error {err} > 8")
        check(bool((out[..., 3] == 255).all()), f"{colour}: alpha not opaque")


@test
def gradient_and_noise_round_trip():
    grad = gradient(256, 256)
    out = decode(encode_rgb(grad).tobytes(), 256, 256, "dxt1")
    m = mae(out[..., :3], grad)
    check(m < 8, f"gradient MAE {m:.2f} >= 8")
    noise = np.random.default_rng(7).integers(0, 256, (256, 256, 3), dtype=np.uint8)
    out_n = decode(encode_rgb(noise).tobytes(), 256, 256, "dxt1")
    check(out_n.shape == (256, 256, 4), f"noise decode shape {out_n.shape}")
    print(f"INFO random-noise 256x256 MAE {mae(out_n[..., :3], noise):.2f} (no threshold)", flush=True)


@test
def large_image_round_trip():
    grad = gradient(1024, 1024)
    t0 = time.time()
    out = decode(encode_rgb(grad).tobytes(), 1024, 1024, "dxt1")
    dt = time.time() - t0
    check(out.shape == (1024, 1024, 4), f"shape {out.shape}")
    m = mae(out[..., :3], grad)
    check(m < 8, f"1024x1024 gradient MAE {m:.2f} >= 8")
    print(f"INFO 1024x1024 encode+decode {dt:.2f}s MAE {m:.2f}", flush=True)


@test
def splice_keeps_alpha_and_unchanged_blocks():
    rng = np.random.default_rng(3)
    parent = rng.integers(0, 256, 4 * 16, dtype=np.uint8)  # 2x2 DXT5 blocks
    new = rng.integers(0, 256, (2, 2, 8), dtype=np.uint8)
    changed = np.array([[True, False], [False, True]])
    out = splice_colour(parent.tobytes(), 8, 8, "dxt5", new, changed)
    check(len(out) == len(parent), "DXT5 splice changed length")
    P = parent.reshape(2, 2, 16)
    O = np.frombuffer(out, dtype=np.uint8).reshape(2, 2, 16)
    check(np.array_equal(O[..., :8], P[..., :8]), "DXT5 alpha half changed")
    for by in range(2):
        for bx in range(2):
            if changed[by, bx]:
                check(np.array_equal(O[by, bx, 8:], new[by, bx]), f"changed block {by},{bx} colour not spliced")
            else:
                check(np.array_equal(O[by, bx], P[by, bx]), f"unchanged block {by},{bx} not byte-identical")
    parent1 = parent[:32].copy()
    parent1.reshape(2, 2, 8)[..., 1] = 0xFF  # c0 > c1 everywhere: all 4-colour, so no mode flip
    parent1.reshape(2, 2, 8)[..., 3] = 0x00
    new = new.copy()
    new[..., 1], new[..., 3] = 0xFF, 0x00
    out1 = splice_colour(parent1.tobytes(), 8, 8, "dxt1", new, changed)
    O1 = np.frombuffer(out1, dtype=np.uint8).reshape(2, 2, 8)
    P1 = parent1.reshape(2, 2, 8)
    check(np.array_equal(O1[0, 0], new[0, 0]) and np.array_equal(O1[1, 1], new[1, 1]), "DXT1 colour not spliced")
    check(np.array_equal(O1[0, 1], P1[0, 1]) and np.array_equal(O1[1, 0], P1[1, 0]), "DXT1 unchanged block changed")


@test
def splice_dxt1_keeps_parent_block_when_colour_mode_would_flip():
    four = bytes([0x83, 0x49, 0x12, 0x30, 0, 0, 0, 0])  # c0 > c1 : 4-colour mode
    three = bytes([0x83, 0x49, 0xA3, 0x51, 0xFE, 0xFF, 0xFF, 0xFF])  # c0 <= c1 : 3-colour mode (transparent index 3)
    parent = four + three + four + three
    new = np.zeros((1, 4, 8), dtype=np.uint8)
    new[0, 0] = np.frombuffer(four, dtype=np.uint8) ^ 1  # 4-colour over 4-colour: spliced
    new[0, 1] = np.frombuffer(four, dtype=np.uint8) ^ 1  # 4-colour over 3-colour: parent kept
    new[0, 2] = np.frombuffer(three, dtype=np.uint8)  # 3-colour over 4-colour: parent kept
    new[0, 3] = np.frombuffer(three, dtype=np.uint8) ^ 1  # 3-colour over 3-colour: spliced
    out = splice_colour(parent, 16, 4, "dxt1", new, np.ones((1, 4), dtype=bool))
    O = np.frombuffer(out, dtype=np.uint8).reshape(4, 8)
    P = np.frombuffer(parent, dtype=np.uint8).reshape(4, 8)
    check(np.array_equal(O[0], new[0, 0]) and np.array_equal(O[3], new[0, 3]), "same-mode block not spliced")
    check(np.array_equal(O[1], P[1]) and np.array_equal(O[2], P[2]), "mode-flipping block was not kept")


@test
def splice_large_random_dxt5():
    rng = np.random.default_rng(11)
    by = bx = 256
    parent = rng.integers(0, 256, by * bx * 16, dtype=np.uint8)
    changed = rng.random((by, bx)) < 0.3
    new = rng.integers(0, 256, (by, bx, 8), dtype=np.uint8)
    out = splice_colour(parent.tobytes(), 1024, 1024, "dxt5", new, changed)
    P = parent.reshape(by, bx, 16)
    O = np.frombuffer(out, dtype=np.uint8).reshape(by, bx, 16)
    check(np.array_equal(O[..., :8], P[..., :8]), "alpha half changed on 1024x1024")
    check(np.array_equal(O[~changed], P[~changed]), "unchanged blocks not byte-identical")
    check(np.array_equal(O[changed][:, 8:], new[changed]), "changed colour halves not spliced")


@test
def hand_built_blocks_match_reference():
    # DXT1 4-colour: red/blue endpoints; index = column, so the palette is read left to right.
    out = decode(pack_dxt1(0xF800, 0x001F, [c for _ in range(4) for c in range(4)]), 4, 4, "dxt1")
    pal = [(255, 0, 0, 255), (0, 0, 255, 255), (170, 0, 85, 255), (85, 0, 170, 255)]
    exp = np.array([[pal[c] for c in range(4)] for _ in range(4)], np.uint8)
    check(np.array_equal(out, exp), f"DXT1 4-colour mismatch:\n{out[0]}")
    # DXT1 3-colour (c0 <= c1): black/white, mid grey = (0+255)//2 = 127, index 3 transparent.
    idx = [3, 2, 1, 0] + [0] * 12
    out3 = decode(pack_dxt1(0x0000, 0xFFFF, idx), 4, 4, "dxt1")
    check(tuple(out3[0, 0]) == (0, 0, 0, 0), f"transparent index: {out3[0, 0]}")
    check(tuple(out3[0, 1]) == (127, 127, 127, 255), f"mid colour: {out3[0, 1]}")
    check(tuple(out3[0, 2]) == (255, 255, 255, 255), f"white: {out3[0, 2]}")
    check(tuple(out3[0, 3]) == (0, 0, 0, 255), f"black: {out3[0, 3]}")
    check(tuple(out3[1, 0]) == (0, 0, 0, 255), f"index 0 black: {out3[1, 0]}")
    # DXT5 8-value alpha (a0 > a1): values from a0=200, a1=100, index = i & 7.
    tab8 = [200, 100, 185, 171, 157, 142, 128, 114]
    red = pack_dxt1(0xF800, 0x001F, [0] * 16)
    out8 = decode(pack_alpha(200, 100, [i & 7 for i in range(16)]) + red, 4, 4, "dxt5")
    check(list(out8[..., 3].ravel()) == [tab8[i & 7] for i in range(16)], f"8-alpha: {out8[..., 3]}")
    check(tuple(out8[0, 0, :3]) == (255, 0, 0), f"DXT5 colour: {out8[0, 0]}")
    # DXT5 6-value alpha (a0 <= a1): 0 and 255 are the last two entries.
    tab6 = [50, 200, 80, 110, 140, 170, 0, 255]
    out6 = decode(pack_alpha(50, 200, [i & 7 for i in range(16)]) + red, 4, 4, "dxt5")
    check(list(out6[..., 3].ravel()) == [tab6[i & 7] for i in range(16)], f"6-alpha: {out6[..., 3]}")
    # DXT5 colour half with c0 <= c1 is still 4-colour: index 3 = (0 + 2*255)//3 = 170 grey, opaque.
    out_c = decode(pack_alpha(0, 0, [0] * 16) + pack_dxt1(0x0000, 0xFFFF, [3] * 16), 4, 4, "dxt5")
    check(tuple(out_c[0, 0]) == (170, 170, 170, 0), f"DXT5 colour not 4-colour: {out_c[0, 0]}")


@test
def non_multiple_of_four_shapes():
    y, x = np.mgrid[0:10, 0:6]  # h=10, w=6 -> 3x2 blocks; gentle slope so 4 colours fit a block
    rgb = np.stack([100 + 2 * x, 120 + 3 * y, 140 + x + y], axis=-1).astype(np.uint8)
    enc = encode_rgb(rgb)
    check(enc.shape == (3, 2, 8), f"encode shape {enc.shape}")
    out = decode(enc.tobytes(), 6, 10, "dxt1")
    check(out.shape == (10, 6, 4), f"dxt1 decode shape {out.shape}")
    check(mae(out[..., :3], rgb) < 8, "6x10 gradient MAE >= 8")
    raw5 = np.random.default_rng(5).integers(0, 256, 3 * 2 * 16, dtype=np.uint8).tobytes()
    check(decode(raw5, 6, 10, "dxt5").shape == (10, 6, 4), "dxt5 6x10 shape")
    one = encode_rgb(np.array([[[10, 20, 30]]], np.uint8))
    check(one.shape == (1, 1, 8), f"1x1 encode shape {one.shape}")
    check(decode(one.tobytes(), 1, 1, "dxt1").shape == (1, 1, 4), "1x1 decode shape")
    m = np.zeros((10, 6), dtype=bool)
    m[9, 5] = True
    bm = block_mask(m)
    check(bm.shape == (3, 2) and bool(bm[2, 1]) and int(bm.sum()) == 1, f"block_mask {bm}")


def read_k1p_mip0():
    if not TPC.exists():
        raise Skip(f"{TPC} missing")
    raw = TPC.read_bytes()
    data_size, _alpha, w, h, pixel_type, _mips = struct.unpack("<IfHHBB", raw[:14])
    check(pixel_type == 2, f"expected DXT1 pixel_type 2, got {pixel_type}")
    check(data_size == ((w + 3) // 4) * ((h + 3) // 4) * BLOCK_BYTES["dxt1"], "mip0 data_size mismatch")
    return w, h, raw[128:128 + data_size]


@test
def real_k1p_commm04_mip0_round_trip():
    w, h, mip0 = read_k1p_mip0()
    rgba = decode(mip0, w, h, "dxt1")
    check(rgba.shape == (h, w, 4), f"mip0 shape {rgba.shape}")
    again = decode(encode_rgb(rgba[..., :3]).tobytes(), w, h, "dxt1")
    m = mae(again[..., :3], rgba[..., :3])
    check(m < 8, f"decode->encode->decode MAE {m:.2f} >= 8")
    print(f"INFO k1p_commm04 mip0 decode->encode->decode MAE {m:.2f}", flush=True)


@test
def real_k1p_commm04_vs_pykotor():
    w, h, mip0 = read_k1p_mip0()
    try:
        from pykotor.resource.formats.tpc import TPCTextureFormat, read_tpc
    except ImportError as exc:
        raise Skip(f"pykotor not importable: {exc}")
    rgba = decode(mip0, w, h, "dxt1")
    tpc = read_tpc(str(TPC))
    tpc.convert(TPCTextureFormat.RGBA)
    ref = np.frombuffer(bytes(tpc.get(0, 0).data), dtype=np.uint8)
    check(ref.size == w * h * 4, f"pykotor RGBA size {ref.size} != {w * h * 4}")
    ref = ref.reshape(h, w, 4)
    check(ref.shape == rgba.shape, f"shape mismatch {ref.shape} vs {rgba.shape}")
    print(f"INFO k1p_commm04 mip0 mean abs diff vs pykotor RGBA {mae(rgba, ref):.2f} (no threshold)", flush=True)


def main() -> int:
    passed = 0
    for fn in TESTS:
        name = fn.__name__
        try:
            fn()
        except Skip as exc:
            print(f"SKIP {name}: {exc}", flush=True)
            passed += 1
        except Exception as exc:  # report every failure, keep running
            print(f"FAIL {name}: {exc}", flush=True)
        else:
            print(f"PASS {name}", flush=True)
            passed += 1
    print(f"{passed}/{len(TESTS)} PASS", flush=True)
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
