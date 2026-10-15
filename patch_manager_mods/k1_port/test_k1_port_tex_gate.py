#!/usr/bin/env python3
"""Tests for k1_port_tex_gate.py. Synthetic in-memory TPCs and PNGs in tempdirs; stdlib only.

The CLI runs under `python -I` (no site, no script dir on sys.path). The real-file smoke test
SKIPs (counts as pass) when the K1-port stage file is absent.
"""
import json
import random
import struct
import subprocess
import sys
import tempfile
import time
import zlib
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import k1_port_tex_gate as G  # noqa: E402

CLI = HERE / "k1_port_tex_gate.py"
REAL_DIR = Path("~/Games/KOTOR2_Mod_Workspace/k1_port_stage/override").expanduser()
REAL_FILES = ("k1p_commm04.tpc", "k1p_llk_lenab01.tpc")  # DXT1 512 x 10 mips; DXT5 512 x 10 + trailer
ALL_IDS = ["dims", "mips", "format", "header", "length", "name", "txi_sidecar", "alpha"]
TESTS = []


class Skip(Exception):
    pass


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


# ---- synthetic TPC (layout written from the spec, not from the gate) ----

def make_tpc(w, h, enc, mips, seed=1, trailer=b""):
    bpb = 8 if enc == 2 else 16
    rng = random.Random(seed)
    body, first = b"", 0
    for k in range(mips):
        wk, hk = max(1, w >> k), max(1, h >> k)
        n = ((wk + 3) // 4) * ((hk + 3) // 4) * bpb
        if k == 0:
            first = n
        body += rng.randbytes(n)
    hdr = struct.pack("<IfHHBB", first, 1.0, w, h, enc, mips) + bytes(114)
    return hdr + body + trailer


def block_off(w, h, bpb, mip, bx, by):
    off = 128
    for k in range(mip):
        off += ((max(1, w >> k) + 3) // 4) * ((max(1, h >> k) + 3) // 4) * bpb
    bw = (max(1, w >> mip) + 3) // 4
    return off + (by * bw + bx) * bpb


def patch(data, off, new):
    b = bytearray(data)
    b[off:off + len(new)] = new
    return bytes(b)


# ---- synthetic PNG (writer with filter types 0-4 so the decoder is exercised) ----

def _chunk(ctype, body):
    return struct.pack(">I", len(body)) + ctype + body + struct.pack(">I", zlib.crc32(ctype + body))


def _paeth(a, b, c):
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    return b if pb <= pc else c


def _forward(ft, cur, prior, bpp):
    out = bytearray(len(cur))
    for i, x in enumerate(cur):
        a = cur[i - bpp] if i >= bpp else 0
        b = prior[i]
        c = prior[i - bpp] if i >= bpp else 0
        p = (0, a, b, (a + b) // 2, _paeth(a, b, c))[ft]
        out[i] = (x - p) & 0xFF
    return bytes(out)


def make_png(w, h, rows, color):
    bpp = 1 if color == 0 else 3
    raw = bytearray()
    prior = bytes(w * bpp)
    for y, row in enumerate(rows):
        raw.append(y % 5)
        raw += _forward(y % 5, row, prior, bpp)
        prior = row
    ihdr = struct.pack(">IIBBBBB", w, h, 8, color, 0, 0, 0)
    return (b"\x89PNG\r\n\x1a\n" + _chunk(b"IHDR", ihdr)
            + _chunk(b"IDAT", zlib.compress(bytes(raw))) + _chunk(b"IEND", b""))


def grey_png(w, h, fn):
    return make_png(w, h, [bytes(fn(x, y) for x in range(w)) for y in range(h)], 0)


def rgb_png(w, h, fn):
    rows = []
    for y in range(h):
        row = bytearray()
        for x in range(w):
            row += bytes(fn(x, y))
        rows.append(bytes(row))
    return make_png(w, h, rows, 2)


# ---- helpers ----

def run(parent, child, name="k1p_test.tpc", files=None, mask=None):
    return G.gate(parent, child, name, files if files is not None else [name], mask)


def failing(results):
    return [r["id"] for r in results if not r["ok"]]


def detail(results, cid):
    return next(r["detail"] for r in results if r["id"] == cid)


def expect_only(results, cid):
    bad = failing(results)
    check(bad == [cid], f"expected only {cid!r} failing, got {bad}: "
                        f"{[r['detail'] for r in results if not r['ok']]}")


def cli(*args):
    return subprocess.run([sys.executable, "-I", str(CLI), *args],
                          capture_output=True, text=True, timeout=300)


def dxt1_fixture():
    """DXT1 64x64 x 7 mips; mip-0 block (0,0) is 3-colour mode with pixel 0 transparent."""
    p = make_tpc(64, 64, 2, 7)
    return patch(p, 128, struct.pack("<HHI", 0x1000, 0x2000, 0x00000003))


# ---- gate tests ----

@test
def identical_copy_passes():
    p = make_tpc(256, 256, 4, 9, trailer=b"envmaptexture x\r\n")
    res = run(p, p)
    check([r["id"] for r in res] == ALL_IDS, f"ids {[r['id'] for r in res]}")
    check(not failing(res), f"failing {failing(res)}")


@test
def identical_copy_with_empty_mask_passes():
    p = make_tpc(64, 64, 4, 7)
    mask = G.read_mask(grey_png(64, 64, lambda x, y: 0))
    res = run(p, p, mask=mask)
    check([r["id"] for r in res] == ALL_IDS + ["mask_scope"], f"ids {[r['id'] for r in res]}")
    check(not failing(res), f"failing {failing(res)}")


@test
def changed_dims_fails_only_dims():
    p = make_tpc(256, 256, 4, 9)
    expect_only(run(p, patch(p, 8, struct.pack("<H", 128))), "dims")


@test
def dropped_last_mip_fails_only_mips():
    p = make_tpc(256, 256, 4, 9)
    expect_only(run(p, patch(p, 13, bytes([8]))), "mips")


@test
def dxt1_vs_dxt5_header_fails_only_format():
    p = make_tpc(256, 256, 2, 9)
    expect_only(run(p, patch(p, 12, bytes([4]))), "format")


@test
def changed_header_alpha_float_fails_only_header():
    p = make_tpc(256, 256, 4, 9)
    res = run(p, patch(p, 4, bytes([p[4] ^ 0x01])))  # byte 4 = alpha float, outside NAMED_HDR
    expect_only(res, "header")
    check("differ at offsets [4]" in detail(res, "header"), detail(res, "header"))


@test
def changed_reserved_header_byte_fails_only_header():
    p = make_tpc(64, 64, 2, 7)
    off = next(i for i in range(G.HDR_LEN) if i not in G.NAMED_HDR and i >= 14)  # first reserved byte
    res = run(p, patch(p, off, bytes([p[off] ^ 0xFF])))
    expect_only(res, "header")
    check(f"differ at offsets [{off}]" in detail(res, "header"), detail(res, "header"))


@test
def named_header_bytes_are_not_judged_by_header_check():
    p = make_tpc(64, 64, 2, 7)
    check(G._check_header(p, patch(p, 8, struct.pack("<H", 32)))["ok"], "NAMED_HDR byte flagged by the header check")


@test
def changed_trailer_fails_only_length():
    p = make_tpc(64, 64, 4, 7, trailer=b"envmaptexture x\r\n")
    res = run(p, p[:-1] + bytes([p[-1] ^ 0x01]))
    expect_only(res, "length")
    check("trailer differs" in detail(res, "length"), detail(res, "length"))


@test
def appended_bytes_fail_only_length():
    p = make_tpc(64, 64, 4, 7)
    res = run(p, p + b"\0")
    expect_only(res, "length")
    check("length child" in detail(res, "length"), detail(res, "length"))


@test
def one_dxt5_alpha_byte_fails_only_alpha():
    p = make_tpc(256, 256, 4, 9)
    off = block_off(256, 256, 16, 0, 2, 1) + 3
    expect_only(run(p, patch(p, off, bytes([p[off] ^ 0x10]))), "alpha")


@test
def sibling_txi_fails_only_txi():
    p = make_tpc(64, 64, 4, 7)
    expect_only(run(p, p, files=["k1p_test.tpc", "k1p_test.txi"]), "txi_sidecar")


@test
def name_32_chars_fails_only_name():
    p = make_tpc(64, 64, 4, 7)
    name = "k1p_" + "a" * 28 + ".tpc"
    check(len(Path(name).stem) == 32, "fixture length")
    expect_only(run(p, p, name=name, files=[name]), "name")


@test
def name_uppercase_fails_only_name():
    p = make_tpc(64, 64, 4, 7)
    expect_only(run(p, p, name="K1P_test.tpc", files=["K1P_test.tpc"]), "name")


@test
def block_outside_mask_fails_only_mask_scope():
    p = make_tpc(256, 256, 4, 9)
    mask = G.read_mask(grey_png(256, 256, lambda x, y: 255 if x < 8 and y < 8 else 0))
    off = block_off(256, 256, 16, 0, 10, 10) + 8  # colour half of a mip-0 block far from the mask
    res = run(p, patch(p, off, bytes([p[off] ^ 0xFF])), mask=mask)
    expect_only(res, "mask_scope")
    check("mip0 (10,10)" in detail(res, "mask_scope"), detail(res, "mask_scope"))


@test
def block_inside_mask_passes():
    p = make_tpc(256, 256, 4, 9)
    mask = G.read_mask(grey_png(256, 256, lambda x, y: 255 if x < 8 and y < 8 else 0))
    c = p
    for mip, bx, by in ((0, 0, 0), (2, 0, 0)):  # mip-2 block (0,0) footprint covers the mask
        off = block_off(256, 256, 16, mip, bx, by) + 8
        c = patch(c, off, bytes(b ^ 0x5A for b in c[off:off + 8]))
    res = run(p, c, mask=mask)
    check(not failing(res), f"failing {failing(res)}: {[r['detail'] for r in res if not r['ok']]}")


@test
def dxt1_mode_flip_fails_only_alpha():
    p = dxt1_fixture()
    res = run(p, patch(p, 128, struct.pack("<HH", 0x2000, 0x1000)))  # c0 > c1: 4-colour mode
    expect_only(res, "alpha")
    check("colour mode" in detail(res, "alpha"), detail(res, "alpha"))


@test
def dxt1_index3_change_fails_only_alpha():
    p = dxt1_fixture()
    res = run(p, patch(p, 132, struct.pack("<I", 0)))  # pixel 0 index 3 -> 0
    expect_only(res, "alpha")
    check("index-3" in detail(res, "alpha"), detail(res, "alpha"))


@test
def dxt1_colour_edit_keeping_alpha_passes():
    p = dxt1_fixture()
    res = run(p, patch(p, 128, struct.pack("<HH", 0x1100, 0x2100)))  # still c0<c1, same indices
    check(not failing(res), f"failing {failing(res)}")


@test
def mask_size_mismatch_raises():
    p = make_tpc(64, 64, 4, 7)
    mask = G.read_mask(grey_png(32, 32, lambda x, y: 255))
    try:
        G.gate(p, p, "k1p_test.tpc", ["k1p_test.tpc"], mask)
    except ValueError:
        return
    raise AssertionError("mask of the wrong size was accepted")


@test
def footprint_levels_match_bruteforce():
    W, H = 13, 7
    rnd = random.Random(7)
    base = bytes(1 if rnd.random() < 0.1 else 0 for _ in range(W * H))
    grid, gw, gh = base, W, H
    for k in range(5):
        if k:
            grid, gw, gh = G._or_reduce(grid, gw, gh)
        check((gw, gh) == (-(-W // (1 << k)), -(-H // (1 << k))), f"level {k} grid {gw}x{gh}")
        for y in range(gh):
            for x in range(gw):
                fy0, fy1 = y << k, min((y + 1) << k, H)
                fx0, fx1 = x << k, min((x + 1) << k, W)
                want = any(base[Y * W + X] for Y in range(fy0, fy1) for X in range(fx0, fx1))
                check((grid[y * gw + x] != 0) == want, f"level {k} pixel ({x},{y})")


@test
def png_decoder_all_filters_and_threshold():
    w, h = 13, 7
    vals = lambda x, y: (0, 127, 128, 255, 200, 50)[(x + 2 * y) % 6]  # covers the 127/128 edge
    want = bytes(1 if vals(x, y) >= 128 else 0 for y in range(h) for x in range(w))
    mw, mh, flat = G.read_mask(grey_png(w, h, vals))
    check((mw, mh) == (w, h), f"size {mw}x{mh}")
    check(flat == want, "grey mask decode mismatch")
    rgb = rgb_png(w, h, lambda x, y: (vals(x, y), 255 - vals(x, y), 0))
    check(G.read_mask(rgb)[2] == want, "rgb mask must use the red channel only")


@test
def mask_1024_completes_quickly():
    p = make_tpc(1024, 1024, 4, 11, seed=3)
    mask = G.read_mask(grey_png(1024, 1024, lambda x, y: 255 if (x - 512) ** 2 + (y - 512) ** 2 < 200 ** 2 else 0))
    off = block_off(1024, 1024, 16, 0, 0, 0) + 8  # mip-0 corner block is far outside the disk
    c = patch(p, off, bytes(b ^ 0xFF for b in p[off:off + 8]))
    t0 = time.monotonic()
    res = run(p, c, mask=mask)
    dt = time.monotonic() - t0
    expect_only(res, "mask_scope")
    check(dt < 20, f"gate took {dt:.1f}s on a 1024x1024 mask")


# ---- CLI tests ----

@test
def cli_exit_codes_and_json():
    with tempfile.TemporaryDirectory() as td:
        d = Path(td)
        p = make_tpc(64, 64, 4, 7, trailer=b"txi text\r\n")
        par = d / "parent.tpc"
        par.write_bytes(p)
        good = d / "k1p_good.tpc"
        good.write_bytes(p)
        off = block_off(64, 64, 16, 0, 1, 1) + 2
        bad = d / "k1p_bad.tpc"
        bad.write_bytes(patch(p, off, bytes([p[off] ^ 1])))
        side = d / "k1p_side.tpc"
        side.write_bytes(p)
        (d / "k1p_side.txi").write_text("sidecar\n")
        mask_ok = d / "mask.png"
        mask_ok.write_bytes(grey_png(64, 64, lambda x, y: 255 if x < 16 else 0))
        mask_bad = d / "mask_bad.png"
        mask_bad.write_bytes(b"not a png")
        mask_small = d / "mask_small.png"
        mask_small.write_bytes(grey_png(32, 32, lambda x, y: 0))

        r = cli("check", "--parent", str(par), "--child", str(good))
        check(r.returncode == 0 and "gate: PASS" in r.stdout, f"rc {r.returncode}: {r.stdout}{r.stderr}")

        r = cli("check", "--parent", str(par), "--child", str(bad))
        check(r.returncode == 1 and "FAIL alpha" in r.stdout, f"rc {r.returncode}: {r.stdout}{r.stderr}")

        r = cli("check", "--parent", str(par), "--child", str(bad), "--json")
        j = json.loads(r.stdout)
        check(r.returncode == 1 and j["ok"] is False
              and [c["id"] for c in j["checks"] if not c["ok"]] == ["alpha"], f"rc {r.returncode}: {j}")

        r = cli("check", "--parent", str(par), "--child", str(good), "--json")
        j = json.loads(r.stdout)
        check(r.returncode == 0 and j["ok"] is True and [c["id"] for c in j["checks"]] == ALL_IDS,
              f"rc {r.returncode}: {j}")

        r = cli("check", "--parent", str(par), "--child", str(side), "--json")
        j = json.loads(r.stdout)
        check(r.returncode == 1 and [c["id"] for c in j["checks"] if not c["ok"]] == ["txi_sidecar"],
              f"rc {r.returncode}: {j}")

        r = cli("check", "--parent", str(par), "--child", str(good), "--mask", str(mask_ok))
        check(r.returncode == 0 and "mask_scope" in r.stdout, f"rc {r.returncode}: {r.stdout}{r.stderr}")

        r = cli("check", "--parent", str(par), "--child", str(good), "--mask", str(mask_bad))
        check(r.returncode == 2, f"non-PNG mask rc {r.returncode}")

        r = cli("check", "--parent", str(par), "--child", str(good), "--mask", str(mask_small))
        check(r.returncode == 2, f"mismatched mask rc {r.returncode}")

        r = cli("check", "--parent", str(par), "--child", str(d / "missing.tpc"))
        check(r.returncode == 2, f"missing child rc {r.returncode}")

        r = cli("check", "--parent", str(par))
        check(r.returncode == 2, f"missing --child rc {r.returncode}")


@test
def real_stage_files_pass_against_themselves():
    present = [f for f in REAL_FILES if (REAL_DIR / f).exists()]
    if not present:
        raise Skip(f"no real files in {REAL_DIR}")
    for name in present:
        f = REAL_DIR / name
        r = cli("check", "--parent", str(f), "--child", str(f))
        check(r.returncode == 0, f"{name} rc {r.returncode}: {r.stdout}{r.stderr}")


def main() -> int:
    passed = 0
    for fn in TESTS:
        try:
            fn()
        except Skip as exc:
            print(f"SKIP {fn.__name__}: {exc}")
            passed += 1
        except Exception as exc:  # AssertionError or anything unexpected
            print(f"FAIL {fn.__name__}: {exc}")
        else:
            print(f"PASS {fn.__name__}")
            passed += 1
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
