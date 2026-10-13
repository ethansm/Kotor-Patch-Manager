#!/usr/bin/env python3
"""Tests for k1_port_uvmask.py. Tempdirs only; nothing is written next to the repo.

The synthetic tests drive build_masks() with duck-typed nodes, so they need no game and no pykotor and run under
python3 or the repo venv alike. The real-game test runs the CLI under the patched pykotor venv (K1PORT_PY) and
SKIPs when that interpreter, the K2 install or the model is missing.
"""
import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_uvmask.py"
sys.path.insert(0, str(HERE))
import k1_port_uvmask as uv  # noqa: E402

PY = os.environ.get("K1PORT_PY") or str(Path("~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python").expanduser())
TESTS = []


class Skip(Exception):
    """A requirement is missing (the patched interpreter, the K2 install, the model). Reported as SKIP, a pass."""


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


class Vec:
    def __init__(self, x, y, z=0.0):
        self.x, self.y, self.z = x, y, z


class Face:
    def __init__(self, a, b, c):
        self.v1, self.v2, self.v3 = a, b, c


class Mesh:
    def __init__(self, uv, faces, pos):
        self.vertex_uv1, self.faces, self.vertex_positions = uv, faces, pos


class Node:
    def __init__(self, name, mesh):
        self.name, self.mesh = name, mesh


def mesh_from_tris(tris, centroids=None):
    """tris: 3-corner UV tuples. centroids: optional per-triangle (x, y, z) given to all three corners."""
    uv, pos, faces = [], [], []
    for i, tri in enumerate(tris):
        base = len(uv)
        c = centroids[i] if centroids else (0.0, 0.0, 0.0)
        for u, v in tri:
            uv.append(Vec(u, v))
            pos.append(Vec(*c))
        faces.append(Face(base, base + 1, base + 2))
    return Mesh(uv, faces, pos)


def square(u0, v0, u1, v1):
    return [((u0, v0), (u1, v0), (u1, v1)), ((u0, v0), (u1, v1), (u0, v1))]


def synthetic_nodes():
    """Torso (central, arm-side and leg faces), head, identical ArmL/ArmR, shoulder pad, unknown node, and skips."""
    torso_tris = square(0.0, 0.0, 0.2, 0.2) + square(0.4, 0.0, 0.5, 0.1) + square(0.6, 0.6, 0.7, 0.7)
    torso_c = [(0.0, 0.0, 1.0)] * 2 + [(0.3, 0.0, 1.0)] * 2 + [(0.0, 0.0, 0.5)] * 2
    return [
        Node("torso", mesh_from_tris(torso_tris, torso_c)),
        Node("head", mesh_from_tris(square(0.2, 0.8, 0.3, 0.9))),
        Node("ArmL", mesh_from_tris(square(0.8, 0.0, 0.9, 0.1))),
        Node("ArmR", mesh_from_tris(square(0.8, 0.0, 0.9, 0.1))),
        Node("Shoulder_L", mesh_from_tris(square(0.3, 0.3, 0.35, 0.35))),
        Node("Weapon R!", mesh_from_tris(square(0.9, 0.9, 1.0, 1.0))),
        Node("Bad", mesh_from_tris(square(1.1, 0.5, 1.2, 0.6))),
        Node("Empty", Mesh([Vec(0.1, 0.1)], [], [Vec(0.0, 0.0, 0.0)])),
        Node("NoUV", Mesh([], [Face(0, 1, 2)], [Vec(0.0, 0.0, 0.0)] * 3)),
        Node("Dummy", None),
    ]


def decode_grey_png(path):
    """(width, height, rows) for an 8-bit greyscale PNG written by write_png_grey. Checks CRCs and filter bytes."""
    data = Path(path).read_bytes()
    check(data[:8] == b"\x89PNG\r\n\x1a\n", f"{path}: not a PNG")
    pos, idat, w, h = 8, b"", None, None
    while pos < len(data):
        n, = struct.unpack(">I", data[pos:pos + 4])
        tag = data[pos + 4:pos + 8]
        body = data[pos + 4:pos + 8 + n]
        crc, = struct.unpack(">I", data[pos + 8 + n:pos + 12 + n])
        check(zlib.crc32(body) & 0xFFFFFFFF == crc, f"{path}: bad CRC in {tag!r}")
        payload = data[pos + 8:pos + 8 + n]
        if tag == b"IHDR":
            w, h, depth, ctype, _, _, _ = struct.unpack(">IIBBBBB", payload)
            check((depth, ctype) == (8, 0), f"{path}: want 8-bit greyscale, got depth {depth} type {ctype}")
        elif tag == b"IDAT":
            idat += payload
        pos += 12 + n
    raw = zlib.decompress(idat)
    check(len(raw) == h * (w + 1), f"{path}: raw size {len(raw)} != {h * (w + 1)}")
    rows = []
    for y in range(h):
        check(raw[y * (w + 1)] == 0, f"{path}: row {y} filter byte not 0")
        rows.append(raw[y * (w + 1) + 1:(y + 1) * (w + 1)])
    return w, h, rows


# ---------------------------------------------------------------- grouping rules (no pykotor)


@test
def test_label_rules(root):
    check(uv.resolve_label("ArmL") == ("arms", uv.ARM), f"ArmL -> {uv.resolve_label('ArmL')}")
    check(uv.resolve_label("RArm") == ("arms", uv.ARM), f"RArm -> {uv.resolve_label('RArm')}")
    check(uv.resolve_label("torso") == ("torso", uv.SPLIT), f"torso -> {uv.resolve_label('torso')}")
    check(uv.resolve_label("body") == ("body", uv.SPLIT), f"body -> {uv.resolve_label('body')}")
    check(uv.resolve_label("Head") == ("head", uv.WHOLE), f"Head -> {uv.resolve_label('Head')}")
    check(uv.resolve_label("Shoulder_R") == ("shoulders", uv.WHOLE), "shoulder pad rule")
    check(uv.resolve_label("Weapon R!") == ("weapon_r", uv.WHOLE), f"unknown -> {uv.resolve_label('Weapon R!')}")
    check(uv.sanitize("!!!") == "node", "empty sanitized name falls back to 'node'")
    check(uv.arm_side("ArmL") == "L" and uv.arm_side("LArm") == "L", "left arm names")
    check(uv.arm_side("RArm") == "R" and uv.arm_side("arm") is None, "right arm and bare arm names")


@test
def test_grouping_skips_and_labels(root):
    res = uv.build_masks(synthetic_nodes(), 64, 0)
    check(sorted(res["masks"]) == ["arms", "head", "legs", "shoulders", "torso", "torso_arm", "weapon_r"],
          f"labels: {sorted(res['masks'])}")
    skip = {(s["node"], s["reason"]) for s in res["skipped"]}
    check(skip == {("Bad", "out_of_range"), ("Empty", "no_faces"), ("NoUV", "no_uv")}, f"skipped: {skip}")
    check(res["labels"]["torso_arm"]["nodes"] == ["torso"] and res["labels"]["legs"]["nodes"] == ["torso"],
          "split labels name their source node")
    check(res["labels"]["arms"]["nodes"] == ["ArmL", "ArmR"], f"arm nodes: {res['labels']['arms']['nodes']}")
    check("larm" not in res["masks"] and "rarm" not in res["masks"],
          "identical L/R UV sets must not write larm/rarm")
    check(all(v["texels"] > 0 for v in res["labels"].values()), "every label covers some texels")


@test
def test_null_texture_node_is_skipped(root):
    mesh = mesh_from_tris(square(0.0, 0.0, 1.0, 1.0))
    mesh.texture_1 = "NULL"
    keep = mesh_from_tris(square(0.2, 0.2, 0.3, 0.3))
    keep.texture_1 = "N_Ma"
    res = uv.build_masks([Node("head_g", mesh), Node("Head", keep)], 32, 0)
    check(res["skipped"] == [{"node": "head_g", "reason": "no_texture"}], f"skipped: {res['skipped']}")
    check(res["labels"]["head"]["nodes"] == ["Head"], f"head nodes: {res['labels']['head']['nodes']}")


@test
def test_split_nesting_and_head_overlap(root):
    res = uv.build_masks(synthetic_nodes(), 64, 0)
    ov = res["overlaps"]
    check(ov["head&torso"] == 0, f"torso and head must not overlap: {ov['head&torso']}")
    check(ov["torso&torso_arm"] == res["labels"]["torso_arm"]["texels"], "torso_arm is nested in torso")
    check(ov["legs&torso"] == res["labels"]["legs"]["texels"], "legs is nested in torso")
    check(ov["arms&torso"] == 0, "arms and torso are disjoint here")


@test
def test_arm_sides_written_when_uv_differs(root):
    nodes = [Node("ArmL", mesh_from_tris(square(0.8, 0.0, 0.9, 0.1))),
             Node("ArmR", mesh_from_tris(square(0.8, 0.1, 0.9, 0.2)))]
    res = uv.build_masks(nodes, 64, 0)
    check("larm" in res["masks"] and "rarm" in res["masks"], f"larm/rarm missing: {sorted(res['masks'])}")
    check(res["labels"]["arms"]["texels"] == res["labels"]["larm"]["texels"] + res["labels"]["rarm"]["texels"],
          "arms is the union of larm and rarm (disjoint here)")
    check(res["overlaps"]["larm&rarm"] == 0, "larm and rarm do not overlap here")


@test
def test_coverage_is_exact_for_a_square(root):
    nodes = [Node("Weapon", mesh_from_tris(square(0.0, 0.0, 0.5, 0.5)))]
    res = uv.build_masks(nodes, 64, 0)
    check(res["labels"]["weapon"]["texels"] == 1024, f"texels {res['labels']['weapon']['texels']} != 32*32 = 1024")
    check(abs(res["labels"]["weapon"]["coverage"] - 0.25) < 1e-9, f"coverage {res['labels']['weapon']['coverage']}")


@test
def test_flip_is_exact_row_reverse(root):
    plain = uv.build_masks(synthetic_nodes(), 64, 0, size_h=40)
    flipped = uv.build_masks(synthetic_nodes(), 64, 1, size_h=40)
    check(plain["width"] == 64 and plain["height"] == 40, "non-square dims")
    check(sorted(plain["masks"]) == sorted(flipped["masks"]), "same labels either way")
    W, H = 64, 40
    for label, g0 in plain["masks"].items():
        g1 = flipped["masks"][label]
        for y in range(H):
            check(g1[y * W:(y + 1) * W] == g0[(H - 1 - y) * W:(H - y) * W],
                  f"{label}: flipped row {y} is not plain row {H - 1 - y}")
        check(g0.count(255) == g1.count(255), f"{label}: texel count changed under flip")


@test
def test_flip_validation(root):
    try:
        uv.build_masks(synthetic_nodes(), 64, 2)
    except uv.UsageError:
        return
    raise AssertionError("flip=2 must be refused")


@test
def test_png_roundtrip_and_masks_json(root):
    res = uv.build_masks(synthetic_nodes(), 64, 0, size_h=40)
    meta = {"model": "synth", "atlas": "synth_atlas", "size": 64, "flip": 0,
            "source": {"mdl_sha256": "00", "mdx_sha256": "11"}}
    out = root / "out"
    doc = uv.write_outputs(out, res, meta)
    check(set(doc) == {"model", "atlas", "size", "size_h", "flip", "source", "labels", "skipped", "overlaps"},
          f"masks.json keys: {sorted(doc)}")
    W, H = 64, 40
    for label, grid in res["masks"].items():
        w, h, rows = decode_grey_png(out / f"{label}.png")
        check((w, h) == (W, H), f"{label}.png is {w}x{h}")
        for y in range(H):
            check(rows[y] == bytes(grid[y * W:(y + 1) * W]), f"{label}.png row {y} differs from the mask")
        info = doc["labels"][label]
        check(set(info) == {"coverage", "nodes", "texels"}, f"label entry keys: {sorted(info)}")
        check(abs(info["coverage"] - round(info["texels"] / (W * H), 6)) < 1e-9, f"{label} coverage formula")
    check(set(p.name for p in out.glob("*.png")) == {f"{k}.png" for k in res["masks"]}, "one PNG per label")
    check((out / "masks.json").is_file(), "masks.json written")


@test
def test_dims_and_tpc_header(root):
    check(uv.resolve_dims(1024, None, (1024, 1024)) == (1024, 1024), "square atlas")
    try:
        uv.resolve_dims(1024, None, (2048, 1024))
    except uv.UsageError:
        pass
    else:
        raise AssertionError("non-square atlas without --size-h must be refused")
    check(uv.resolve_dims(1024, 512, (2048, 1024)) == (1024, 512), "--size-h overrides the aspect")
    header = struct.pack("<IfHHBB", 1048576, 1.0, 1024, 512, 4, 11) + bytes(114)
    check(uv.tpc_dims(header) == (1024, 512), "TPC width and height at offsets 8 and 10")
    try:
        uv.tpc_dims(b"\x00" * 64)
    except uv.InputError:
        return
    raise AssertionError("short TPC must be refused")


@test
def test_cli_refuses_bad_arguments(root):
    game = root / "game"
    game.mkdir()
    cases = [
        ["--flip", "2", "--out", str(root / "o1")],
        ["--out", str(game / "inside")],
        ["--model", "n_commm", "--mdl", "x.mdl", "--mdx", "x.mdx", "--out", str(root / "o2")],
    ]
    for extra in cases:
        cmd = [sys.executable, str(CLI), "build", "--atlas", "n_commm05", "--game", str(game)] + extra
        if "--model" not in extra and "--mdl" not in extra:
            cmd += ["--model", "n_commm"]
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        check(r.returncode == 2, f"{extra}: exit {r.returncode}, stderr {r.stderr.strip()!r}")
    check(not (root / "o1").exists(), "a refused run must not create the output directory")


# ---------------------------------------------------------------- real game (patched pykotor)


@test
def test_real_game_n_commm_anchors(root):
    if not Path(PY).exists():
        raise Skip(f"patched pykotor python not found at {PY} (set K1PORT_PY, see setup_venv.sh)")
    game = uv.DEFAULT_GAME
    if not game.is_dir():
        raise Skip(f"K2 install not found at {game}")
    out = root / "nc"
    cmd = [PY, str(CLI), "build", "--model", "n_commm", "--atlas", "n_commm05", "--size", "1024", "--flip", "0",
           "--out", str(out), "--game", str(game)]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=900)
    if r.returncode == 1 and ("not found" in r.stderr or "not importable" in r.stderr):
        raise Skip(f"n_commm not readable: {r.stderr.strip()}")
    check(r.returncode == 0, f"CLI exit {r.returncode}: {r.stderr.strip()}")
    doc = json.loads((out / "masks.json").read_text(encoding="utf-8"))
    check(doc["size"] == 1024 and doc["size_h"] == 1024 and doc["flip"] == 0, "dims and flip recorded")
    torso = doc["labels"]["torso"]["coverage"]
    arms = doc["labels"]["arms"]["coverage"]
    check(abs(torso - 0.545) <= 0.015, f"torso coverage {torso:.4f} is not within 1.5 pp of 0.545")
    check(abs(arms - 0.152) <= 0.015, f"arms coverage {arms:.4f} is not within 1.5 pp of 0.152")
    w, h, _ = decode_grey_png(out / "torso.png")
    check((w, h) == (1024, 1024), f"torso.png is {w}x{h}")


def main() -> int:
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_uvmask_test_"))
        try:
            fn(root)
            print(f"PASS {fn.__name__}")
            passed += 1
        except Skip as e:
            print(f"SKIP {fn.__name__}: {e}")
            passed += 1  # a missing requirement is not a failure
        except Exception as e:  # noqa: BLE001 - report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
        finally:
            shutil.rmtree(root, ignore_errors=True)
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
