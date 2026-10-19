#!/usr/bin/env python3
"""Tests for k1_port_model.py. Run from anywhere: python3 test_k1_port_model.py

The CLI needs the patched pykotor venv (K1PORT_PY, default ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python).
Fixtures are synthetic K1-layout models built by that interpreter in a temp dir, and the CLI always reads them
with --pack-dir, so the K1 install is never opened. The K2 collision guard gets a fake K2 dir (K2 markers, an
empty chitin.key, dummy Override files) unless a test points --k2-install at the real install, which is only
read and is SKIPped when absent. Without the venv the file prints SKIP and exits 0, after the pure compare
tests (hand-built summaries, no pykotor) have run.
"""

import contextlib
import copy
import hashlib
import importlib.util
import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_model.py"
K1PORT_PY = os.environ.get("K1PORT_PY") or os.path.expanduser(
    "~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python")
REAL_K2 = Path(os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"))


class Skip(Exception):
    """A test that cannot run on this machine. Reported as SKIP, never as FAIL."""

# Runs under the venv interpreter (python -I -c HELPER <mode> ...). Stdout ends with one JSON line.
HELPER = r'''
import json, logging, sys
from pathlib import Path
logging.disable(logging.CRITICAL)
from pykotor.common.misc import Game
from pykotor.resource.formats.mdl import MDL, MDLNode, MDLMesh, MDLFace, read_mdl
from pykotor.resource.formats.mdl.io_mdl import MDLBinaryWriter
from pykotor.resource.formats.mdl.mdl_data import MDLBoneVertex, MDLSkin, Vector2, Vector3
from pykotor.resource.formats.mdl.mdl_types import MDLNodeType

def make_mdl(spec):
    mdl = MDL()
    mdl.supermodel = spec.get("supermodel", "")
    mdl.name = spec.get("model_name", "")
    mdl.root.name = spec.get("root_name", "k1p_root")
    for i, m in enumerate(spec["meshes"]):
        node = MDLNode()
        node.name = m["name"]
        node.node_type = MDLNodeType.TRIMESH
        mesh = MDLMesh()
        mesh.texture_1 = m.get("texture_1", "")
        mesh.texture_2 = m.get("texture_2", "")
        dx = 10.0 * i
        mesh.vertex_positions = [Vector3(dx, 0, 0), Vector3(dx + 1, 0, 0), Vector3(dx + 1, 1, 0), Vector3(dx, 1, 0)]
        mesh.vertex_normals = [Vector3(0, 0, 1)] * 4
        mesh.vertex_uv1 = [] if m.get("no_uv") else [Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1)]
        faces = []
        for a, b, c in ((0, 1, 2), (0, 2, 3)):
            f = MDLFace()
            f.v1, f.v2, f.v3 = a, b, c
            faces.append(f)
        mesh.faces = faces
        if m.get("tangent") == "zero":  # flagged, every row all zero (the writer must drop the flag and block)
            mesh.tangent_space = True
            mesh.tangent_block = [bytes(36)] * 4
        elif m.get("tangent") == "nonzero":  # flagged, rows with data (kept by the writer)
            mesh.tangent_space = True
            mesh.tangent_block = [bytes([j + 1]) * 36 for j in range(4)]
        node.mesh = mesh
        if m.get("skin"):  # opt-in skin header: bone slots, bonemap, per-vertex bone indices and weights
            sk = m["skin"]
            node.node_type = MDLNodeType.SKIN
            node.skin = MDLSkin()
            node.skin.bone_indices = tuple(sk["bones16"])
            node.skin.bonemap = list(sk["bonemap"])
            for idx, wts in zip(sk["vertex_indices"], sk["vertex_weights"]):
                vb = MDLBoneVertex()
                vb.vertex_indices = tuple(float(v) for v in idx)
                vb.vertex_weights = tuple(float(v) for v in wts)
                node.skin.vertex_bones.append(vb)
        mdl.root.children.append(node)
    return mdl

def build(pack, base, spec):
    mdl = make_mdl(spec)
    mdl_path, mdx_path = Path(pack) / (base + ".mdl"), Path(pack) / (base + ".mdx")
    w = MDLBinaryWriter(mdl, mdl_path, mdx_path)
    w.force_game = Game.K1  # explicit K1 layout: no dirt fields, 8 bytes less per trimesh than K2
    w.synth_zero_uv = False  # real K1 files keep textured meshes without UV1 UV-less; so must the fixture
    w.keep_zero_tangents = True  # the fixture keeps a flagged all-zero block (llk_LenaH-like); the default drops it
    w.write()
    return {"k1_mdl_bytes": mdl_path.stat().st_size, "k1_mdx_bytes": mdx_path.stat().st_size,
            "meshes": len(spec["meshes"])}

def uvwarn(cli_dir, spec):
    # Checks the warning in memory on a UV-less source (the warning is about the source, not the file).
    sys.dont_write_bytecode = True  # -I ignores PYTHONDONTWRITEBYTECODE; keep the repo free of __pycache__
    sys.path.insert(0, cli_dir)
    import k1_port_model as K
    return {"warning": K.bare_uv_warning(K.summarize(make_mdl(spec)), bool(spec.get("synth", False)))}

def skinrt(cli_dir, pack, spec):
    # Production order: pad_skin_bones on the in-memory model, then the K1 writer, then read the files back.
    sys.dont_write_bytecode = True  # -I ignores PYTHONDONTWRITEBYTECODE; keep the repo free of __pycache__
    sys.path.insert(0, cli_dir)
    import k1_port_model as K
    mdl = make_mdl(spec)
    pad_changed = K.pad_skin_bones(mdl)
    mdl_path, mdx_path = Path(pack) / "skin_rt.mdl", Path(pack) / "skin_rt.mdx"
    w = MDLBinaryWriter(mdl, mdl_path, mdx_path)
    w.force_game = Game.K1
    w.synth_zero_uv = False
    w.keep_zero_tangents = True
    w.write()
    mdx = mdx_path.read_bytes()
    back = read_mdl(mdl_path.read_bytes(), source_ext=mdx, size_ext=len(mdx))
    skins = []
    def find(n):
        if n.skin is not None:
            skins.append(n.skin)
        for c in n.children:
            find(c)
    find(back.root)
    assert len(skins) == 1, f"expected 1 skin node after the round trip, got {len(skins)}"
    sk = skins[0]
    return {"pad_changed": pad_changed, "bones16": [int(b) for b in sk.bone_indices],
            "bonemap": [int(b) for b in sk.bonemap],
            "vertex_indices": [[float(v) for v in vb.vertex_indices] for vb in sk.vertex_bones],
            "vertex_weights": [[float(v) for v in vb.vertex_weights] for vb in sk.vertex_bones]}

def read(mdl_path, mdx_path):
    mdx = Path(mdx_path).read_bytes()
    mdl = read_mdl(Path(mdl_path).read_bytes(), source_ext=mdx, size_ext=len(mdx))
    nodes, meshes = [], []
    def walk(n):
        nodes.append(n.name)
        if n.mesh is not None:
            m = n.mesh
            meshes.append({"texture_1": m.texture_1 or "", "texture_2": m.texture_2 or "",
                           "dirt": [bool(m.dirt_enabled), int(m.dirt_texture), int(m.dirt_worldspace),
                                    bool(m.hologram_donotdraw)],
                           "vertices": len(m.vertex_positions), "faces": len(m.faces),
                           "uv1": len(m.vertex_uv1 or [])})
        for c in n.children:
            walk(c)
    walk(mdl.root)
    return {"model_name": mdl.name or "", "nodes": nodes, "meshes": meshes, "supermodel": mdl.supermodel}

mode = sys.argv[1]
if mode == "build":
    out = build(sys.argv[2], sys.argv[3], json.loads(sys.argv[4]))
elif mode == "read":
    out = read(sys.argv[2], sys.argv[3])
elif mode == "uvwarn":
    out = uvwarn(sys.argv[2], json.loads(sys.argv[3]))
elif mode == "skinrt":
    out = skinrt(sys.argv[2], sys.argv[3], json.loads(sys.argv[4]))
else:
    raise SystemExit("bad mode " + mode)
sys.stdout.write(json.dumps(out) + "\n")
'''


@contextlib.contextmanager
def workdir():
    root = Path(tempfile.mkdtemp(prefix="k1port_test_"))
    try:
        yield root
    finally:
        shutil.rmtree(root, ignore_errors=True)


SCRATCH_LEDGER = Path(tempfile.gettempdir()) / "k1port_model_test_unused_ledger.json"  # never the real ledger


def test_env(ledger: Path) -> dict:
    """Every subprocess gets K1PORT_LEDGER pointing into a test tempdir, never the real ledger."""
    env = dict(os.environ)
    env["K1PORT_LEDGER"] = str(ledger)
    return env


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def ledger_entries(root: Path) -> list:
    return json.loads((root / "ledger.json").read_text(encoding="utf-8"))["entries"]


def helper(*args):
    # cwd is a temp dir: pykotor creates logs/ in the CWD, which must never be the repo.
    with workdir() as cwd:
        r = subprocess.run([K1PORT_PY, "-I", "-c", HELPER, *args], capture_output=True, text=True, timeout=180,
                           env=test_env(SCRATCH_LEDGER), cwd=cwd)
    assert r.returncode == 0, f"helper {args[0]} failed rc={r.returncode}: {r.stderr[-800:]}"
    return json.loads(r.stdout.strip().splitlines()[-1])


def make_pack(root, meshes, base="k1p_test", supermodel="", model_name="", root_name="k1p_root"):
    pack = root / "pack"
    pack.mkdir(exist_ok=True)
    spec = {"meshes": meshes, "supermodel": supermodel, "model_name": model_name, "root_name": root_name}
    built = helper("build", str(pack), base, json.dumps(spec))
    return pack, built


def read_back(out_dir, resref):
    return helper("read", str(out_dir / f"{resref}.mdl"), str(out_dir / f"{resref}.mdx"))


def make_fake_k2(root):
    """A K2 install just complete enough for pykotor to detect K2: K2 marker files and a 0-entry chitin.key.
    Override is empty; a test adds files there."""
    k2 = root / "fake_k2"
    for d in ("streamvoice", "LocalVault", "miles", "data", "modules", "Override"):
        (k2 / d).mkdir(parents=True, exist_ok=True)
    for f in ("swkotor2.exe", "swkotor2.ini", "LocalVault/test.bic", "miles/binkawin.asi", "miles/mssds3d.flt",
              "miles/mssdolby.flt", "miles/mssogg.asi", "data/Dialogs.bif"):
        (k2 / f).write_bytes(b"x")
    (k2 / "chitin.key").write_bytes(b"KEY V1  " + struct.pack("<IIIIII", 0, 0, 64, 64, 2008, 1) + bytes(32))
    return k2


def run_cli(root, pack, resref, tex_map, out_dir=None, install=None, extra=(), source="vanilla_k1",
            new="k1p_test", k2=None):
    """Runs the CLI with --tex-map last (it takes several values). Never points at the real install.
    source=None omits --source-id. new=None omits --new-resref (the default name is then used). k2 defaults to
    a path that does not exist, so the K2 guard only warns. The ledger is root/ledger.json (K1PORT_LEDGER)."""
    cmd = [K1PORT_PY, str(CLI), "--install-k1", str(install or root / "no_install"), "--resref", resref,
           "--out-dir", str(out_dir or root / "out"), "--k2-install", str(k2 or root / "no_k2_install")]
    if new is not None:
        cmd += ["--new-resref", new]
    cmd += [*extra]
    if pack is not None:
        cmd += ["--pack-dir", str(pack)]
    if source is not None:
        cmd += ["--source-id", source]
    cmd += ["--tex-map", *tex_map]
    return subprocess.run(cmd, capture_output=True, text=True, timeout=300, env=test_env(root / "ledger.json"),
                          cwd=root)  # pykotor's logs/ land in the temp root, never the repo


def manifest_of(r):
    assert r.returncode == 0, f"CLI rc={r.returncode}: {r.stderr[-800:]}"
    return json.loads(r.stdout)


def assert_refused(r, root):
    assert r.returncode != 0, f"expected refusal, got rc=0; stdout={r.stdout[:300]}"
    assert not (root / "out").exists() or not any((root / "out").iterdir()), "refused run left files behind"


ONE_MESH = [{"name": "k1p_body", "texture_1": "N_TestTex", "texture_2": "keep_me"}]
TWO_MESH = [
    {"name": "k1p_body", "texture_1": "N_TestTex", "texture_2": "keep_me"},
    {"name": "k1p_head", "texture_1": "other_tex", "texture_2": "N_TestTex"},
]


def test_help_runs_under_system_python():
    r = subprocess.run([sys.executable, str(CLI), "--help"], capture_output=True, text=True, timeout=60,
                       env=test_env(SCRATCH_LEDGER))
    assert r.returncode == 0, r.stderr[-400:]
    assert "--tex-map" in r.stdout and "--pack-dir" in r.stdout and "--source-id" in r.stdout
    assert "--new-resref" in r.stdout and "--k2-install" in r.stdout and "--allow-k2-collision" in r.stdout


def test_one_mesh_rename_and_k2_size():
    with workdir() as root:
        pack, built = make_pack(root, ONE_MESH)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=K1P_Test"]))
        assert m["match"] is True, m.get("mismatch_detail")
        assert m["k1_mdl_bytes"] == built["k1_mdl_bytes"]
        assert m["k2_mdl_bytes"] == built["k1_mdl_bytes"] + 8, "one trimesh header should add 8 bytes"
        assert m["renames_applied"] == 1
        assert m["texture_map"] == {"n_testtex": "k1p_test"}
        back = read_back(root / "out", "k1p_test")
        assert back["meshes"][0]["texture_1"] == "k1p_test", back
        assert back["meshes"][0]["texture_2"] == "keep_me", "texture_2 without a map entry must stay"


def test_two_mesh_rename_hits_every_mesh_slot():
    with workdir() as root:
        pack, built = make_pack(root, TWO_MESH)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["N_TestTex=k1p_new"]))
        assert m["match"] is True, m.get("mismatch_detail")
        assert m["k2_mdl_bytes"] == built["k1_mdl_bytes"] + 16, "two trimesh headers should add 16 bytes"
        assert m["renames_applied"] == 2, m  # N_TestTex sits in mesh 1 texture_1 and mesh 2 texture_2
        back = read_back(root / "out", "k1p_test")
        assert [x["texture_1"] for x in back["meshes"]] == ["k1p_new", "other_tex"], back
        assert [x["texture_2"] for x in back["meshes"]] == ["keep_me", "k1p_new"], back


def test_dirt_fields_default_on_output():
    with workdir() as root:
        pack, _ = make_pack(root, TWO_MESH)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"]))
        assert m["compare"]["dirt_defaults"] is True, m
        back = read_back(root / "out", "k1p_test")
        for mesh in back["meshes"]:
            assert mesh["dirt"] == [False, 1, 1, False], mesh


def test_mdx_backed_vertex_counts():
    with workdir() as root:
        pack, _ = make_pack(root, TWO_MESH)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"]))
        assert m["mesh_count"] == 2 and m["node_count"] == 3, m  # root dummy + 2 meshes
        assert m["vertex_total"] == 8 and m["face_total"] == 4, m
        assert m["k2_mdx_bytes"] > 0 and m["k1_mdx_bytes"] > 0
        back = read_back(root / "out", "k1p_test")
        assert [x["vertices"] for x in back["meshes"]] == [4, 4], back
        assert [x["faces"] for x in back["meshes"]] == [2, 2], back


def test_texture_name_length_limit():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        ok_name = "k" * 31
        m = manifest_of(run_cli(root, pack, "k1p_test", [f"n_testtex={ok_name}"]))
        assert m["texture_map"] == {"n_testtex": ok_name}, m
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        r = run_cli(root, pack, "k1p_test", ["n_testtex=" + "k" * 32])
        assert_refused(r, root)
        assert "limit" in r.stderr, r.stderr[-300:]


def test_case_insensitive_new_collision_rejected():
    with workdir() as root:
        pack, _ = make_pack(root, TWO_MESH)
        r = run_cli(root, pack, "k1p_test", ["N_TestTex=Same_Name", "other_tex=SAME_NAME"])
        assert_refused(r, root)
        assert "target of both" in r.stderr, r.stderr[-300:]


def test_new_name_colliding_with_kept_texture_rejected():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        r = run_cli(root, pack, "k1p_test", ["N_TestTex=KEEP_ME"])
        assert_refused(r, root)
        assert "keeps" in r.stderr, r.stderr[-300:]


def test_existing_output_is_never_overwritten():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        out = root / "out"
        out.mkdir()
        sentinel = b"SENTINEL-do-not-overwrite"
        (out / "k1p_test.mdl").write_bytes(sentinel)
        r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], out_dir=out)
        assert r.returncode != 0, "existing .mdl must be refused"
        assert (out / "k1p_test.mdl").read_bytes() == sentinel, "existing output was modified"
        assert not (out / "k1p_test.mdx").exists(), "a refused run must not write the .mdx either"


def test_unknown_tex_map_source_rejected():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        r = run_cli(root, pack, "k1p_test", ["no_such_tex=k1p_x"])
        assert_refused(r, root)
        assert "does not occur" in r.stderr, r.stderr[-300:]


def test_pack_lookup_is_case_insensitive():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH, base="K1P_CI")
        m = manifest_of(run_cli(root, pack, "k1p_ci", ["n_testtex=k1p_new"], new="k1p_ci"))
        assert m["source"] == "pack" and m["match"] is True, m
        assert (root / "out" / "k1p_ci.mdl").is_file()


def test_mesh_without_uv1_is_warned():
    # In-memory check (the warning is about the source model, so no pack fixture is needed).
    bare = helper("uvwarn", str(HERE), json.dumps({"meshes": [
        {"name": "k1p_bare", "texture_1": "N_TestTex", "no_uv": True}, {"name": "k1p_ok", "texture_1": "x"},
        {"name": "k1p_plain", "no_uv": True}]}))  # no texture: never zero-filled, never warned
    assert bare["warning"] and "1 textured mesh(es)" in bare["warning"] and "k1p_bare" in bare["warning"], bare
    assert "k1p_ok" not in bare["warning"] and "k1p_plain" not in bare["warning"], bare
    assert bare["warning"].startswith("informational: "), bare
    assert "output keeps them UV-less" in bare["warning"] and "K2 vanilla n_commm" in bare["warning"], bare
    synth = helper("uvwarn", str(HERE), json.dumps({"meshes": [
        {"name": "k1p_bare", "texture_1": "N_TestTex", "no_uv": True}], "synth": True}))
    assert "--synth-zero-uv writes zero UVs" in synth["warning"], synth
    clean = helper("uvwarn", str(HERE), json.dumps({"meshes": ONE_MESH}))
    assert clean["warning"] is None, clean


# K1-5c-04: a skin node round-trips through pad_skin_bones, the K1 writer and read_mdl (no install needed).
SKIN_BONEMAP = [0, -1, 1, -1, 2]  # node index -> bone serial (-1 = no bone): 3 real bones at nodes 0, 2, 4
SKIN_BONES16_IN = [0, 2, 4] + [13] * 13  # slots 3..15 hold junk that pad_skin_bones must zero
SKIN_VERTEX_INDICES = [[0, 1, -1, -1], [1, 2, -1, -1], [2, -1, -1, -1], [0, 1, 2, -1]]
SKIN_VERTEX_WEIGHTS = [[0.75, 0.25, 0.0, 0.0], [0.5, 0.5, 0.0, 0.0], [1.0, 0.0, 0.0, 0.0], [0.5, 0.25, 0.25, 0.0]]


def test_skin_round_trip_keeps_bones16_bonemap_and_weights():
    skin = {"bones16": SKIN_BONES16_IN, "bonemap": SKIN_BONEMAP,
            "vertex_indices": SKIN_VERTEX_INDICES, "vertex_weights": SKIN_VERTEX_WEIGHTS}
    with workdir() as root:
        out = helper("skinrt", str(HERE), str(root), json.dumps({"meshes": [{"name": "k1p_body", "skin": skin}]}))
    assert out["pad_changed"] == 13, out  # slots 3..15 were nonzero junk
    assert out["bones16"] == [0, 2, 4] + [0] * 13, out  # bones[16]: real slots kept, unused slots zeroed
    assert out["bonemap"] == SKIN_BONEMAP, out  # -1 gaps survive signed
    assert out["vertex_indices"] == [[float(v) for v in r] for r in SKIN_VERTEX_INDICES], out
    assert out["vertex_weights"] == SKIN_VERTEX_WEIGHTS, out


NO_UV_TEXTURED = [{"name": "k1p_bare", "texture_1": "N_TestTex", "no_uv": True}]


def test_textured_mesh_without_uv1_stays_uv_less_by_default():
    with workdir() as root:
        pack, built = make_pack(root, NO_UV_TEXTURED)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"]))
        assert m["synth_zero_uv"] is False, m
        assert m["compare"]["uv_checksum"] is True and m["match"] is True, m.get("mismatch_detail")
        assert m["k2_mdx_bytes"] == built["k1_mdx_bytes"], (m["k2_mdx_bytes"], built)
        back = read_back(root / "out", "k1p_test")
        assert back["meshes"][0]["uv1"] == 0, back
        assert any("output keeps them UV-less" in w for w in m["warnings"]), m["warnings"]


def test_synth_zero_uv_flag_adds_uv1_and_grows_mdx():
    with workdir() as root:
        pack, built = make_pack(root, NO_UV_TEXTURED)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], extra=["--synth-zero-uv"]))
        assert m["synth_zero_uv"] is True, m
        assert m["compare"]["uv_checksum"] is False and m["match"] is False, m
        assert m["k2_mdx_bytes"] == built["k1_mdx_bytes"] + 32, "4 vertices x TEX0 (8 bytes) must be added"
        back = read_back(root / "out", "k1p_test")
        assert back["meshes"][0]["uv1"] == 4, back
        assert any("--synth-zero-uv writes zero UVs" in w for w in m["warnings"]), m["warnings"]


def test_skin_unknown0_warning_is_gone():
    # The patched pykotor keeps the skin unknown0 array, so the tool no longer warns about dropping it.
    K = load_cli_module()
    assert not hasattr(K, "skin_unknown0_warning"), "the stale unknown0 warning must be removed"


def test_unavailable_supermodel_warns_and_exits_zero():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH, supermodel="k1p_nosuch")
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"]))
        assert m["supermodel"] == "k1p_nosuch" and m["supermodel_status"] == "not_found", m
        assert m["warnings"], "missing supermodel must be recorded in warnings"
        assert m["match"] is True, m


def load_cli_module():
    """The CLI loaded by path. Its top level imports no pykotor, so this runs under any interpreter."""
    sys.dont_write_bytecode = True
    spec = importlib.util.spec_from_file_location("k1_port_model_under_test", CLI)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


SKIN_POSE = {  # hand-built skin summary in the shape _skin_record() returns
    "bonemap": [-1, 2, 9],
    "qbones": [[-1.0, 0.0, 0.0, 0.0]],
    "tbones": [[0.0, 0.00003, 0.00939]],
    "bone_node_number": [],
    "vertex_bones": [[[0.0, 1.0, -1.0, -1.0], [0.9, 0.1, 0.0, 0.0]]],
}


def _rec(name="k1p_body", tangent_bytes=0, tangent=False, skin=None, zero_block=False):
    """Hand-built mesh record: 4 vertices with positions, normals and uv1 (32 bytes), plus skin and tangent blocks.
    zero_block: the stored tangent block has rows and they are all zero (tangent_block_zero)."""
    stride = 32 + (32 if skin is not None else 0) + tangent_bytes
    return {
        "name": name, "texture_1": "k1p_body_tex", "texture_2": "",
        "vertex_count": 4, "face_count": 2, "uv1_count": 4, "uv2_count": 0, "normal_count": 4,
        "pos_min": [0.0, 0.0, 0.0], "pos_max": [1.0, 1.0, 0.0],
        "uv1": "uv-digest", "uv2": "uv2-digest", "normals": "normal-digest", "faces": "face-digest",
        "dirt": [False, 1, 1, False], "tangent_space": tangent, "tangent_block_zero": zero_block,
        "mdx_stride": stride, "tangent_bytes": tangent_bytes, "skin": skin,
    }


def _summary(*meshes):
    nodes = [{"name": "k1p_root", "node_type": 1, "child_count": len(meshes), "is_mesh": False}]
    nodes += [{"name": m["name"], "node_type": 32, "child_count": 0, "is_mesh": True} for m in meshes]
    return {"nodes": nodes, "meshes": copy.deepcopy(list(meshes)), "animation_count": 0}


def test_identical_skin_and_tangent_summaries_all_true():
    K = load_cli_module()
    src = _summary(_rec(skin=copy.deepcopy(SKIN_POSE)), _rec("k1p_tan", tangent=True, tangent_bytes=36))
    results, detail = K.compare(src, copy.deepcopy(src), {})
    assert results["skin_bone_poses"] is True and results["tangent_data"] is True, results
    assert all(results.values()) and K.overall_match(results), results
    assert detail == {}, detail


def test_differing_bonemap_flags_skin_bone_poses():
    K = load_cli_module()
    src = _summary(_rec(skin=copy.deepcopy(SKIN_POSE)))
    zeroed = copy.deepcopy(SKIN_POSE)
    zeroed["bonemap"] = [0, 0, 0]  # the writer defect: re-read bonemap all zero
    results, detail = K.compare(src, _summary(_rec(skin=zeroed)), {})
    assert results["skin_bone_poses"] is False, results
    assert results["tangent_data"] is True and results["node_tree"] is True, results
    assert K.overall_match(results) is False, results
    assert "mesh 0 (k1p_body)" in detail["skin_bone_poses"] and "skin.bonemap" in detail["skin_bone_poses"], detail


def test_differing_tangent_count_flags_tangent_data():
    K = load_cli_module()
    src = _summary(_rec("k1p_tan", tangent=True, tangent_bytes=36))
    # The writer keeps the TANGENT_SPACE flag but drops the 36-byte block (stride 68 -> 32).
    results, detail = K.compare(src, _summary(_rec("k1p_tan", tangent=True, tangent_bytes=0)), {})
    assert results["tangent_data"] is False, results
    assert results["skin_bone_poses"] is True, results
    assert K.overall_match(results) is False, results
    assert "tangent bytes/vertex 36 -> 0" in detail["tangent_data"], detail


def test_tangent_bytes_isolates_the_tangent_block():
    K = load_cli_module()

    def rec(stride, uv1=4, skin=None):
        return {"vertex_count": 4, "normal_count": 4, "uv1_count": uv1, "uv2_count": 0, "skin": skin,
                "mdx_stride": stride}

    assert K.tangent_bytes(rec(68)) == 36, "pos+normal+uv1+tangent"
    assert K.tangent_bytes(rec(32)) == 0, "pos+normal+uv1 only"
    assert K.tangent_bytes(rec(24, uv1=0)) == 0, "UV-less mesh"
    assert K.tangent_bytes(rec(32, uv1=0)) == 8, "a stride of 32 without uv1 is an unexplained 8 bytes"
    # A synthesised uv1 (--synth-zero-uv) changes the stride but must not look like a tangent change.
    assert K.tangent_bytes(rec(24, uv1=0)) == K.tangent_bytes(rec(32, uv1=4)) == 0
    assert K.tangent_bytes(rec(64, skin={})) == 0, "skin adds 32 bytes"
    assert K.tangent_bytes(rec(100, skin={})) == 36, "skin plus tangent"


def test_zero_tangent_block_cleared_counts_as_match():
    K = load_cli_module()
    src = _summary(_rec("k1p_tan", tangent=True, tangent_bytes=36, zero_block=True))
    out = _summary(_rec("k1p_tan", tangent=False, tangent_bytes=0))
    results, detail = K.compare(src, out, {})
    assert results["tangent_data"] is True and K.overall_match(results) is True, results
    assert detail == {}, detail
    assert K.zero_tangent_cleared(src["meshes"], out["meshes"]) == [True]


def test_zero_tangent_cleared_is_counted_per_mesh_and_skipped_in_detail():
    K = load_cli_module()
    # a: zero block, dropped (accepted). b: non-zero rows, kept. c: no tangent at all.
    src = _summary(_rec("k1p_a", tangent=True, tangent_bytes=36, zero_block=True),
                   _rec("k1p_b", tangent=True, tangent_bytes=36), _rec("k1p_c"))
    out = _summary(_rec("k1p_a"), _rec("k1p_b", tangent=True, tangent_bytes=36), _rec("k1p_c"))
    results, _ = K.compare(src, out, {})
    assert results["tangent_data"] is True and K.overall_match(results) is True, results
    assert K.zero_tangent_cleared(src["meshes"], out["meshes"]) == [True, False, False]
    # Now b loses its non-zero rows: only b is reported, the accepted a is not named.
    out_bad = _summary(_rec("k1p_a"), _rec("k1p_b"), _rec("k1p_c"))
    results, detail = K.compare(src, out_bad, {})
    assert results["tangent_data"] is False and K.overall_match(results) is False, results
    assert "mesh 1 (k1p_b)" in detail["tangent_data"], detail
    assert "mesh 0" not in detail["tangent_data"], detail


def test_nonzero_tangent_block_dropped_still_fails():
    K = load_cli_module()
    src = _summary(_rec("k1p_tan", tangent=True, tangent_bytes=36))  # rows have data (zero_block False)
    out = _summary(_rec("k1p_tan", tangent=False, tangent_bytes=0))
    results, detail = K.compare(src, out, {})
    assert results["tangent_data"] is False and K.overall_match(results) is False, results
    assert K.zero_tangent_cleared(src["meshes"], out["meshes"]) == [False]
    assert "mesh 0 (k1p_tan): tangent flag True -> False" in detail["tangent_data"], detail


def test_zero_block_with_unexplained_bytes_still_fails():
    K = load_cli_module()
    src = _summary(_rec("k1p_tan", tangent=True, tangent_bytes=36, zero_block=True))
    out = _summary(_rec("k1p_tan", tangent=False, tangent_bytes=4))  # flag cleared, but 4 bytes left over
    results, detail = K.compare(src, out, {})
    assert results["tangent_data"] is False and K.overall_match(results) is False, results
    assert K.zero_tangent_cleared(src["meshes"], out["meshes"]) == [False]
    assert "tangent bytes/vertex 36 -> 4" in detail["tangent_data"], detail


def test_keep_zero_mode_rejects_cleared_flag():
    K = load_cli_module()
    src = _summary(_rec("k1p_tan", tangent=True, tangent_bytes=36, zero_block=True))
    out = _summary(_rec("k1p_tan"))
    results, detail = K.compare(src, out, {}, allow_zero_tangent_drop=False)  # --keep-zero-tangents
    assert results["tangent_data"] is False and K.overall_match(results) is False, results
    assert "tangent flag True -> False" in detail["tangent_data"], detail
    # The same pair with the rule on is accepted, and keep mode accepts an output that kept flag and block.
    assert K.compare(src, out, {})[0]["tangent_data"] is True
    kept = _summary(_rec("k1p_tan", tangent=True, tangent_bytes=36, zero_block=True))
    assert K.compare(src, kept, {}, allow_zero_tangent_drop=False)[0]["tangent_data"] is True


def test_zero_tangent_block_cleared_end_to_end():
    with workdir() as root:
        meshes = [{"name": "k1p_tanz", "texture_1": "N_TestTex", "tangent": "zero"},
                  {"name": "k1p_tanv", "texture_1": "other_tex", "tangent": "nonzero"},
                  {"name": "k1p_plain", "texture_1": "other_tex"}]
        pack, _ = make_pack(root, meshes)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"]))
        assert m["match"] is True, m.get("mismatch_detail")
        assert m["tangent_zero_cleared"] == 1, m
        assert m["keep_zero_tangents"] is False, m
        assert any(w.startswith("informational: 1 meshes had all-zero tangent rows in K1; flag and block omitted")
                   for w in m["warnings"]), m["warnings"]
        keep = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], out_dir=root / "out_keep",
                                   extra=["--keep-zero-tangents"]))
        assert keep["match"] is True, keep.get("mismatch_detail")
        assert keep["tangent_zero_cleared"] == 0 and keep["keep_zero_tangents"] is True, keep
        assert not any("informational" in w for w in keep["warnings"]), keep["warnings"]
        # Keeping the block on the 4-vertex mesh: 4 x 36 bytes of rows, plus 36 for the MDLOps terminator row (one
        # stride, written per mesh), plus 12 bytes of 16-byte alignment padding (68-byte stride: 340 -> 352). 192 total.
        assert keep["k2_mdx_bytes"] - m["k2_mdx_bytes"] == 4 * 36 + 36 + 12, (keep["k2_mdx_bytes"], m["k2_mdx_bytes"])


def test_default_source_resolution():
    # Pure: registry lookup only, no pykotor. Default is vanilla_k1 with the install; --pack-dir needs an id.
    K = load_cli_module()
    assert K.resolve_source_id(None, pack_given=False) == "vanilla_k1"
    assert K.resolve_source_id("upscale_plus_v0.52", pack_given=True) == "upscale_plus_v0.52"
    for given, pack_given, needle in ((None, True, "needs --source-id"), ("no_such_source", False, "unregistered")):
        try:
            K.resolve_source_id(given, pack_given=pack_given)
        except K.Rejected as exc:
            assert needle in str(exc), str(exc)
        else:
            raise AssertionError(f"resolve_source_id({given!r}, {pack_given}) should be rejected")


def test_unknown_source_id_rejected_nothing_written():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], source="no_such_source")
        assert_refused(r, root)
        assert "unregistered source id" in r.stderr, r.stderr[-300:]
        assert not (root / "ledger.json").exists(), "refused run must not create the ledger"


def test_pack_without_source_id_rejected():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], source=None)
        assert_refused(r, root)
        assert "needs --source-id" in r.stderr, r.stderr[-300:]
        assert not (root / "ledger.json").exists(), "refused run must not create the ledger"


def test_vanilla_source_not_flagged_and_outputs_registered():
    with workdir() as root:
        pack, _ = make_pack(root, TWO_MESH)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], source="vanilla_k1"))
        assert m["source_id"] == "vanilla_k1" and m["requires_author_approval"] is False, m
        assert m["removal_group"] is None, m
        out = root / "out"
        entries = {e["file"]: e for e in ledger_entries(root)}
        assert len(entries) == 2, entries
        for path in (out / "k1p_test.mdl", out / "k1p_test.mdx"):
            e = entries[str(path)]
            assert Path(e["file"]).is_absolute(), e
            assert e["kind"] == "model" and e["source_id"] == "vanilla_k1", e
            assert e["removal_group"] is None and e["author_approved"] is False, e
            assert e["sha256"] == sha(path), "ledger sha256 must match the written file"
            assert e["resref"] == "k1p_test" and e["texture_map"] == {"n_testtex": "k1p_new"}, e


def test_permission_source_flagged_for_removal():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], source="upscale_plus_v0.52"))
        assert m["removal_group"] == "pending_author_approval" and m["requires_author_approval"] is True, m
        entries = ledger_entries(root)
        assert len(entries) == 2 and all(e["removal_group"] == "pending_author_approval" for e in entries), entries


def test_ledger_inside_repo_refused():
    probe = HERE / "k1port_model_ledger_refusal_probe.json"  # refused before any write; cleanup only if guard regresses
    try:
        with workdir() as root:
            pack, _ = make_pack(root, ONE_MESH)
            r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], extra=["--ledger", str(probe)])
            assert_refused(r, root)
            assert "inside the repo" in r.stderr, r.stderr[-300:]
            assert not probe.exists(), "guard let the CLI create a ledger inside the repo"
    finally:
        if probe.exists():
            probe.unlink()


def test_default_new_resref_naming():
    K = load_cli_module()
    assert K.default_new_resref("n_commm") == "k1p_n_commm"
    assert K.default_new_resref("N_CommKidM") == "k1p_n_commkidm", "the source resref is lowercased"
    assert K.default_new_resref("abcdefghijklmnopqrstuvwxyz") == "k1p_abcdefghijkl", "cut to the 16-char limit"
    for src in ("n_commm", "N_CommKidM", "abcdefghijklmnopqrstuvwxyz", "a" * 16):
        name = K.default_new_resref(src)
        assert len(name) <= 16, name
        K.check_new_resref(name)  # the default always passes the check


def test_new_resref_rules():
    K = load_cli_module()
    for good in ("k1p_commm", "a" * 16, "_0"):
        K.check_new_resref(good)
    for bad, needle in (("", "lowercase"), ("K1P_commm", "lowercase"), ("k1p-commm", "lowercase"),
                        ("k1p commm", "lowercase"), ("a" * 17, "limit is 16")):
        try:
            K.check_new_resref(bad)
        except K.Rejected as exc:
            assert needle in str(exc), str(exc)
        else:
            raise AssertionError(f"check_new_resref({bad!r}) should be rejected")


def test_root_rename_only_when_root_is_the_model():
    # Pure compare on hand-built summaries: a K1 root named like the model is renamed on purpose.
    K = load_cli_module()
    src = _summary(_rec(name="k1p_body"), _rec("k1p_head"))
    src["nodes"][0]["name"] = "N_CommM"
    src["model_name"] = "N_CommM"
    out = _summary(_rec(name="k1p_body"), _rec("k1p_head"))
    out["nodes"][0]["name"] = "k1p_commm"
    out["model_name"] = "k1p_commm"
    results, detail = K.compare(src, out, {}, model_name="k1p_commm", root_renamed=True)
    assert all(results.values()) and K.overall_match(results), (results, detail)
    results, _ = K.compare(src, out, {}, model_name="k1p_commm", root_renamed=False)
    assert results["node_tree"] is False and results["model_name"] is True, results
    child_renamed = copy.deepcopy(out)
    child_renamed["nodes"][1]["name"] = "k1p_xxx"
    results, _ = K.compare(src, child_renamed, {}, model_name="k1p_commm", root_renamed=True)
    assert results["node_tree"] is False, "only the root may carry the rename"
    stale_name = copy.deepcopy(out)
    stale_name["model_name"] = "N_CommM"
    results, _ = K.compare(src, stale_name, {}, model_name="k1p_commm", root_renamed=True)
    assert results["model_name"] is False and K.overall_match(results) is False, results


def test_default_new_resref_used_without_flag():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], new=None))
        assert m["new_resref"] == "k1p_k1p_test", m
        out = root / "out"
        assert sorted(p.name for p in out.iterdir()) == ["k1p_k1p_test.mdl", "k1p_k1p_test.mdx"], list(out.iterdir())
        assert read_back(out, "k1p_k1p_test")["model_name"] == "k1p_k1p_test"


def test_explicit_new_resref_sets_file_names_and_model_name():
    with workdir() as root:
        pack, _ = make_pack(root, TWO_MESH, model_name="K1P_Test")
        m = manifest_of(run_cli(root, pack, "k1p_test", ["N_TestTex=k1p_new"], new="k1p_commm"))
        assert m["match"] is True, m.get("mismatch_detail")
        assert m["resref"] == "k1p_test" and m["new_resref"] == "k1p_commm", m
        assert m["compare"]["model_name"] is True and m["compare"]["node_tree"] is True, m["compare"]
        out = root / "out"
        assert sorted(p.name for p in out.iterdir()) == ["k1p_commm.mdl", "k1p_commm.mdx"], list(out.iterdir())
        back = read_back(out, "k1p_commm")
        assert back["model_name"] == "k1p_commm", back
        assert back["nodes"][0] == "k1p_root", "the root is not the model name here, so it keeps its name"
        assert back["supermodel"] == "", back
        assert [x["texture_1"] for x in back["meshes"]] == ["k1p_new", "other_tex"], back
        assert all(e["resref"] == "k1p_commm" for e in ledger_entries(root)), ledger_entries(root)


def test_root_node_renamed_when_it_is_the_model_name():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH, model_name="K1P_Test", root_name="K1P_Test")
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], new="k1p_commm"))
        assert m["match"] is True, m.get("mismatch_detail")
        assert m["compare"]["node_tree"] is True and m["compare"]["model_name"] is True, m["compare"]
        back = read_back(root / "out", "k1p_commm")
        assert back["nodes"][0] == "k1p_commm" and back["model_name"] == "k1p_commm", back


def test_invalid_new_resref_refused_nothing_written():
    for bad, needle in (("K1P_Commm", "lowercase"), ("k1p-commm", "lowercase"), ("k1p_" + "x" * 13, "limit is 16")):
        with workdir() as root:
            pack, _ = make_pack(root, ONE_MESH)
            r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], new=bad)
            assert_refused(r, root)
            assert needle in r.stderr, r.stderr[-300:]


def test_collision_with_fake_k2_refused_then_allowed():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        k2 = make_fake_k2(root)
        (k2 / "Override" / "k1p_collide.mdl").write_bytes(b"DUMMY")
        (k2 / "Override" / "k1p_collide.mdx").write_bytes(b"DUMMY")
        r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], new="k1p_collide", k2=k2)
        assert_refused(r, root)
        assert "already has a resource named 'k1p_collide'" in r.stderr, r.stderr[-300:]
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], new="k1p_collide", k2=k2,
                                extra=["--allow-k2-collision"]))
        assert m["k2_collision_checked"] is True and m["k2_collision"] is True, m
        assert any("K2 collision allowed" in w for w in m["warnings"]), m["warnings"]
        assert (root / "out" / "k1p_collide.mdl").is_file() and m["match"] is True, m.get("mismatch_detail")


def test_clean_fake_k2_passes_guard():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        k2 = make_fake_k2(root)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], new="k1p_commm", k2=k2))
        assert m["k2_collision_checked"] is True and m["k2_collision"] is False, m
        assert not any("K2" in w for w in m["warnings"]), m["warnings"]


def test_missing_k2_install_warns_and_continues():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        m = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], new="k1p_commm"))
        assert m["k2_collision_checked"] is False and m["k2_collision"] is False, m
        assert any("NOT checked" in w for w in m["warnings"]), m["warnings"]
        assert m["match"] is True, m.get("mismatch_detail")


def test_out_dir_inside_k2_install_refused():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        k2 = make_fake_k2(root)
        r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], out_dir=k2 / "Override", k2=k2, new="k1p_commm")
        assert r.returncode != 0 and "inside the K2 install" in r.stderr, r.stderr[-300:]
        assert not (k2 / "Override" / "k1p_commm.mdl").exists()


def test_real_k2_install_refuses_n_commm():
    if not (REAL_K2 / "chitin.key").is_file():
        raise Skip(f"no K2 install at {REAL_K2}")
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH)
        r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], new="n_commm", k2=REAL_K2)
        assert_refused(r, root)
        assert "already has a resource named 'n_commm'" in r.stderr, r.stderr[-300:]


def test_overlay_install_refused_without_allow_flag():
    with workdir() as root:
        ov = root / "overlay"
        ov.mkdir()
        (ov / "K1_HD_OVERLAY.json").write_text('{"source_id": "characters_revisited_main"}', encoding="utf-8")
        r = run_cli(root, None, "k1p_test", ["n_testtex=k1p_new"], install=ov)
        assert_refused(r, root)
        assert "K1 HD overlay" in r.stderr and "--allow-overlay-install" in r.stderr, r.stderr[-300:]
        r = run_cli(root, None, "k1p_test", ["n_testtex=k1p_new"], install=ov, extra=["--allow-overlay-install"])
        assert "K1 HD overlay" not in r.stderr, "flag did not lift the overlay refusal: " + r.stderr[-300:]


# ---------------------------------------------------------------- skin bones[16] padding and face planes

def _ns(**kw):
    import types
    return types.SimpleNamespace(**kw)


def _tree(*nodes):
    root = _ns(children=list(nodes), skin=None, mesh=None)
    return _ns(root=root)


def test_pad_skin_bones_zeroes_only_unused_slots():
    pk = _import_model()
    junk = [5, 6, 7, 8, 9, 48769, 16256, 18384, 1, 2, 3, 4, 7, 8, 9, 10]
    skin = _ns(bonemap=[0, 1, 2, 3, 4, -1, -1, -1], bone_indices=tuple(junk))
    mdl = _tree(_ns(children=[], skin=skin, mesh=None))
    changed = pk.pad_skin_bones(mdl)
    assert skin.bone_indices[:5] == tuple(junk[:5]), skin.bone_indices
    assert skin.bone_indices[5:] == (0,) * 11, skin.bone_indices
    assert changed == 11, changed
    assert pk.pad_skin_bones(mdl) == 0, "second run must be a no-op"


def test_pad_skin_bones_leaves_full_and_empty_skins_alone():
    pk = _import_model()
    full = _ns(bonemap=list(range(16)), bone_indices=tuple(range(100, 116)))
    empty = _ns(bonemap=[-1, -1], bone_indices=tuple(range(16)))
    mdl = _tree(_ns(children=[], skin=full, mesh=None), _ns(children=[], skin=empty, mesh=None))
    assert pk.pad_skin_bones(mdl) == 0
    assert full.bone_indices == tuple(range(100, 116)) and empty.bone_indices == tuple(range(16))


def test_fix_face_planes_recomputes_float_distance():
    pk = _import_model()
    v = lambda x, y, z: _ns(x=x, y=y, z=z)  # noqa: E731
    face = _ns(v1=1, normal=v(0.0, 0.0, 1.0), coefficient=0)
    flat = _ns(v1=0, normal=v(0.0, 0.0, 1.0), coefficient=0)
    mesh = _ns(faces=[face, flat], vertex_positions=[v(0, 0, 0), v(1, 2, 0.4)])
    mdl = _tree(_ns(children=[], skin=None, mesh=mesh))
    assert pk.fix_face_planes(mdl) == 1  # `flat` already has distance 0 = -dot(n, origin)
    assert abs(face.coefficient + 0.4) < 1e-9, face.coefficient  # the int() truncation to 0 is gone
    assert flat.coefficient == 0 or abs(flat.coefficient) < 1e-12


def test_demote_dangly_clears_only_dangly_nodes():
    pk = _import_model()
    cloth = _ns(children=[], skin=None, mesh=None, dangly=_ns(constraints=[1, 2]))
    skin = _ns(bonemap=[0], bone_indices=tuple(range(16)))
    body = _ns(children=[cloth], skin=skin, mesh=None, dangly=None)
    plain = _ns(children=[], skin=None, mesh=None)  # no dangly attribute at all
    mdl = _tree(body, plain)
    assert pk.demote_dangly(mdl) == 1
    assert cloth.dangly is None and body.skin is skin
    assert pk.demote_dangly(mdl) == 0, "second run must be a no-op"


# ---------------------------------------------------------------- supernode pass (chain numbering)

def _syn_mdl(tree, super_name="NULL", total=None):
    """Minimal binary MDL for k1_port_mdl_supernode: header 208 bytes, name table, 80-byte node headers.
    tree = (name, stored_supernode, [children]); node index = pre-order position. Stored pointers are abs - 12."""
    buf = bytearray(208)

    def alloc(n):
        while len(buf) % 4:
            buf.append(0)
        off = len(buf)
        buf.extend(bytes(n))
        return off

    flat = []

    def collect(t):
        flat.append(t)
        for c in t[2]:
            collect(c)

    collect(tree)
    offs = []
    for t in flat:
        o = alloc(len(t[0]) + 1)
        buf[o:o + len(t[0])] = t[0].encode("latin1")
        offs.append(o)
    arr = alloc(4 * len(flat))
    for i, o in enumerate(offs):
        struct.pack_into("<I", buf, arr + 4 * i, o - 12)
    struct.pack_into("<I", buf, 196, arr - 12)
    struct.pack_into("<II", buf, 200, len(flat), len(flat))
    counter = [0]

    def emit(t):
        idx = counter[0]
        counter[0] += 1
        off = alloc(80)
        struct.pack_into("<HHH", buf, off, 1, t[1], idx)
        kids = [emit(c) for c in t[2]]
        if kids:
            ca = alloc(4 * len(kids))
            for i, k in enumerate(kids):
                struct.pack_into("<I", buf, ca + 4 * i, k - 12)
            struct.pack_into("<I", buf, off + 44, ca - 12)
            struct.pack_into("<II", buf, off + 48, len(kids), len(kids))
        return off

    struct.pack_into("<I", buf, 52, emit(tree) - 12)
    struct.pack_into("<i", buf, 56, len(flat) if total is None else total)
    buf[148:180] = super_name.encode("latin1").ljust(32, b"\0")
    struct.pack_into("<I", buf, 4, len(buf) - 12)
    return bytes(buf)


# supermodel: root(0) -> a(5) -> a1(7), b(9); stored total 40
SYN_SUPER = _syn_mdl(("Root", 0, [("A", 5, [("A1", 7, [])]), ("B", 9, [])]), total=40)
# child as pykotor writes it: supernode = own index, total = own node count
SYN_CHILD = _syn_mdl(("root2", 0, [("a", 1, [("a1", 2, [])]), ("zed", 3, [("zkid", 4, [])])]), super_name="S_Syn")


def _fake_pk():
    return _ns(SearchLocation=_ns(OVERRIDE="OV", CHITIN="CH", MODULES="MO"), ResourceType=_ns(MDL="MDL"))


class _FakeInst:
    """Stands in for a pykotor Installation: resource(name, type, locations) -> object with .data, or None."""

    def __init__(self, table):
        self.table, self.calls = table, []

    def resource(self, name, rtype, where):
        self.calls.append((name, tuple(where)))
        for w in where:
            if (name, w) in self.table:
                return _ns(data=self.table[(name, w)])
        return None


def _numbering(mdl):
    import k1_port_mdl_supernode as ksn
    return [n.sn for n in ksn.parse_tree(mdl).nodes]


def test_supernode_legacy_is_a_byte_no_op():
    pk = _import_model()
    with workdir() as root:
        f = root / "m.mdl"
        f.write_bytes(SYN_CHILD)
        calls = []
        rec = pk.supernode_pass(f, "legacy", lambda name: calls.append(name))
        assert f.read_bytes() == SYN_CHILD and calls == [], "legacy must not touch the file or call the resolver"
        assert rec["mode"] == "legacy" and rec["origin"] is None and rec["total_after"] is None, rec


def test_supernode_chain_renumbers_with_a_supermodel_dir():
    pk = _import_model()
    with workdir() as root:
        sup = root / "sup"
        sup.mkdir()
        (sup / "S_SYN.MDL").write_bytes(SYN_SUPER)  # case-insensitive file name
        f = root / "m.mdl"
        f.write_bytes(SYN_CHILD)
        res = pk.ChainResolver(_fake_pk(), [sup], None)
        rec = pk.supernode_pass(f, "chain", res)
        out = f.read_bytes()
        # root->0, a->5, a1->7 (matched), zed unmatched -> max(9)+1 = 10, zkid below it -> idx 4 + 40 + 1 = 45
        assert _numbering(out) == [0, 5, 7, 10, 45], _numbering(out)
        assert rec["mode"] == "chain" and rec["supermodel"] == "S_Syn", rec
        assert rec["origin"] == f"dir:{sup / 'S_SYN.MDL'}", rec
        assert rec["supermodel_sha256"] == hashlib.sha256(SYN_SUPER).hexdigest(), rec
        assert rec["nodes_changed"] == 4 and rec["total_before"] == 5 and rec["total_after"] == 5 + 41, rec
        assert len(out) == len(SYN_CHILD)
        diff = {i for i, (a, b) in enumerate(zip(SYN_CHILD, out)) if a != b}
        import k1_port_mdl_supernode as ksn
        allowed = {n.off + k for n in ksn.parse_tree(SYN_CHILD).nodes for k in (2, 3)} | {56, 57}
        assert diff and diff <= allowed, sorted(diff - allowed)
        assert not ksn.mismatches(out, res)["mismatches"]


def test_supernode_dir_beats_override_beats_install():
    pk = _import_model()
    other = _syn_mdl(("Root", 0, [("A", 1, [])]), total=2)  # a different supermodel, so a wrong pick shows in the numbers
    with workdir() as root:
        sup = root / "sup"
        sup.mkdir()
        (sup / "s_syn.mdl").write_bytes(SYN_SUPER)
        inst = _FakeInst({("s_syn", "OV"): other, ("s_syn", "CH"): other})
        res = pk.ChainResolver(_fake_pk(), [sup], inst)
        assert res("S_Syn") == SYN_SUPER and res.origins["s_syn"].startswith("dir:") and inst.calls == []
        res2 = pk.ChainResolver(_fake_pk(), [], inst)
        assert res2("s_syn") == other and res2.origins["s_syn"] == "override"
        assert inst.calls == [("s_syn", ("OV",))], "the install must not be searched past a hit"
        inst3 = _FakeInst({("s_syn", "MO"): other})
        res3 = pk.ChainResolver(_fake_pk(), [], inst3)
        assert res3("s_syn") == other and res3.origins["s_syn"] == "install"
        assert [c[1] for c in inst3.calls] == [("OV",), ("CH", "MO")], inst3.calls


def test_supernode_missing_supermodel_raises_and_leaves_file_alone():
    pk = _import_model()
    import k1_port_mdl_supernode as ksn
    with workdir() as root:
        f = root / "m.mdl"
        f.write_bytes(SYN_CHILD)
        for res in (pk.ChainResolver(_fake_pk(), [root], None), pk.ChainResolver(_fake_pk(), [], _FakeInst({}))):
            try:
                pk.supernode_pass(f, "chain", res)
            except ksn.SupernodeError as exc:
                assert "s_syn" in str(exc).lower() and "--supermodel-dir" in str(exc), exc
            else:
                raise AssertionError("missing supermodel must raise SupernodeError")
            assert f.read_bytes() == SYN_CHILD


def test_supernode_chain_without_a_supermodel_is_the_identity():
    pk = _import_model()
    plain = _syn_mdl(("root", 0, [("a", 1, [])]), super_name="NULL")
    with workdir() as root:
        f = root / "m.mdl"
        f.write_bytes(plain)
        rec = pk.supernode_pass(f, "chain", pk.ChainResolver(_fake_pk(), [], None))
        assert f.read_bytes() == plain and rec["supermodel"] is None and rec["origin"] is None, rec


def test_supernode_arg_rules():
    pk = _import_model()
    with workdir() as root:
        a = lambda **kw: _ns(**kw)  # noqa: E731
        assert pk.check_supernode_args(a()) == ("legacy", [])
        assert pk.check_supernode_args(a(supernode="chain", supermodel_dir=[str(root)])) == ("chain", [root])
        for bad in (a(supernode="chain", supermodel_dir=[str(root / "nope")]),
                    a(supernode="legacy", supermodel_dir=[str(root)]),
                    a(supernode="bogus")):
            try:
                pk.check_supernode_args(bad)
            except pk.Rejected:
                continue
            raise AssertionError(f"expected Rejected for {bad}")


def test_keep_dangly_skips_the_demotion():
    pk = _import_model()
    cloth = _ns(children=[], skin=None, mesh=None, dangly=_ns(constraints=[1]))
    mdl = _tree(cloth)
    assert pk.count_dangly(mdl) == 1
    assert pk.demote_unless_kept(mdl, True) == 0 and cloth.dangly is not None, "--keep-dangly must not demote"
    assert pk.demote_unless_kept(mdl, False) == 1 and cloth.dangly is None


def test_supernode_and_keep_dangly_flags_are_parsed_with_legacy_defaults():
    pk = _import_model()
    p = pk.build_parser()
    base = ["--resref", "x", "--tex-map", "a=b", "--out-dir", "o"]
    d = p.parse_args(base)
    assert d.supernode == "legacy" and d.supermodel_dir == [] and d.keep_dangly is False
    c = p.parse_args(base + ["--supernode", "chain", "--supermodel-dir", "d1", "--supermodel-dir", "d2", "--keep-dangly"])
    assert c.supernode == "chain" and c.supermodel_dir == ["d1", "d2"] and c.keep_dangly is True


def _import_model():
    spec = importlib.util.spec_from_file_location("k1_port_model_mod", CLI)
    mod = importlib.util.module_from_spec(spec)
    sys.modules["k1_port_model_mod"] = mod
    spec.loader.exec_module(mod)
    return mod


def _raw_skin_and_planes(mdl, tm):
    """DFS over the node tree of a K2-layout MDL. Returns (name table, {lowercase node name: (bones16 or None,
    [plane distances])}). bones16 values index the name table."""
    nc = struct.unpack_from("<I", mdl, 12 + 0xC0)[0]
    ap = struct.unpack_from("<I", mdl, 12 + 0xB8)[0]
    tab = [mdl[12 + struct.unpack_from("<I", mdl, 12 + ap + 4 * i)[0]:].split(b"\0")[0].decode("latin1").lower()
           for i in range(nc)]
    out = {}

    def walk(off):
        typ, idx = struct.unpack_from("<HH", mdl, off)[0], struct.unpack_from("<H", mdl, off + 4)[0]
        bones, planes = None, []
        if typ & 0x20:
            T = off + 80
            fp, fc = struct.unpack_from("<II", mdl, T + 8)
            planes = [struct.unpack_from("<f", mdl, 12 + fp + 32 * i + 12)[0] for i in range(fc)]
            if typ & 0x40:
                bones = struct.unpack_from("<16H", mdl, T + tm + 64)
        out[tab[idx]] = (bones, planes)
        cp, cc = struct.unpack_from("<II", mdl, off + 44)
        for k in range(cc):
            walk(12 + struct.unpack_from("<I", mdl, 12 + cp + 4 * k)[0])

    walk(12 + struct.unpack_from("<I", mdl, 12 + 40)[0])
    return tab, out


def test_golden_n_commm_matches_k2_vanilla_skin_and_planes():
    """Convert the real K1 n_commm and compare against K2 vanilla n_commm, node by node by name (K2 has one extra
    node, freelookhook, so indices shift): real bones[16] slots name the same nodes, padding slots are in range,
    and every plane distance is within 1e-3 of vanilla."""
    k1 = Path(os.path.expanduser(os.environ.get("K1_INSTALL", "~/.local/share/Steam/steamapps/common/swkotor")))
    if not (k1 / "chitin.key").is_file() or not (REAL_K2 / "chitin.key").is_file():
        raise Skip("K1 or K2 install not found")
    vspec = importlib.util.spec_from_file_location("tv_golden", HERE / "test_k1_port_validate.py")
    tv = importlib.util.module_from_spec(vspec)
    sys.modules["tv_golden"] = tv
    vspec.loader.exec_module(tv)
    van = next(((m, x) for n, m, x in tv._vanilla_k2_pairs() if n == "n_commm"), None)
    assert van is not None, "n_commm missing from K2 chitin"
    with workdir() as root:
        r = subprocess.run([K1PORT_PY, str(CLI), "--install-k1", str(k1), "--resref", "n_commm", "--out-dir",
                            str(root / "out"), "--k2-install", str(REAL_K2), "--new-resref", "k1p_commm",
                            "--source-id", "vanilla_k1", "--tex-map", "N_CommM04=k1p_commm04"],
                           capture_output=True, text=True, timeout=300, env=test_env(root / "ledger.json"), cwd=root)
        man = manifest_of(r)
        assert man["face_planes_recomputed"] > 0 and man["skin_mesh_count"] == 3, man
        ours = (root / "out" / "k1p_commm.mdl").read_bytes()
    tab_o, nodes_o = _raw_skin_and_planes(ours, 340)
    tab_v, nodes_v = _raw_skin_and_planes(van[0], 340)
    extra_v = set(nodes_v) - set(nodes_o) - {"n_commm"}
    assert extra_v == {"freelookhook"}, extra_v
    skins = 0
    for name, (bo, po) in nodes_o.items():
        if name == "k1p_commm":
            continue
        bv, pv = nodes_v[name]
        assert len(po) == len(pv), name
        assert all(abs(a - b) < 1e-3 for a, b in zip(po, pv)), f"{name}: plane distance differs from K2 vanilla"
        if bo is not None:
            skins += 1
            assert all(x < len(tab_o) for x in bo), f"{name}: bones[16] slot out of range: {bo}"
            assert [tab_o[x] for x in bo[:14]] == [tab_v[x] for x in bv[:14]], f"{name}: real bone slots differ"
    assert skins == 3, skins


def test_supermodel_map_rewrites_header_name():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH, supermodel="P_OldBB")
        r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], extra=["--supermodel-map", "p_oldbb=k1p_newbb"])
        man = manifest_of(r)
        assert man["supermodel_mapped_to"] == "k1p_newbb", man
        assert read_back(root / "out", "k1p_test")["supermodel"] == "k1p_newbb"


def test_supermodel_map_unmatched_warns_and_bad_entries_refused():
    with workdir() as root:
        pack, _ = make_pack(root, ONE_MESH, supermodel="P_OldBB")
        r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], extra=["--supermodel-map", "p_other=k1p_x"])
        man = manifest_of(r)
        assert man["supermodel_mapped_to"] is None
        assert any("not in the map" in w for w in man["warnings"]), man["warnings"]
    for bad in ("nonsense", "P_OldBB=Has Space", "a=" + "b" * 17):
        with workdir() as root:
            pack, _ = make_pack(root, ONE_MESH, supermodel="P_OldBB")
            r = run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], extra=["--supermodel-map", bad])
            assert_refused(r, root)


# ---------------------------------------------------------------- supernode chain through the CLI

def _load_validate():
    spec = importlib.util.spec_from_file_location("kv_model_test", HERE / "k1_port_validate.py")
    mod = importlib.util.module_from_spec(spec)
    sys.modules["kv_model_test"] = mod
    spec.loader.exec_module(mod)
    return mod


def _put_supermodel(root, name="s_syn", data=None, sub="sup"):
    d = root / sub
    d.mkdir(exist_ok=True)
    (d / f"{name}.mdl").write_bytes(data if data is not None else SYN_SUPER_PK)
    return d


# supermodel for the pykotor-written pack model (root k1p_root -> k1p_body, k1p_head): body matches, head does not
SYN_SUPER_PK = _syn_mdl(("anyroot", 0, [("k1p_body", 3, []), ("other", 5, [])]), total=9)


def _convert(root, out_name, extra=(), supermodel="S_Syn", k2=None, meshes=TWO_MESH):
    pack, _ = make_pack(root, meshes, supermodel=supermodel)
    return run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], out_dir=root / out_name, extra=list(extra), k2=k2)


def _diff_offsets(a, b):
    return {i for i, (x, y) in enumerate(zip(a, b)) if x != y}


def test_supernode_default_is_legacy_and_bytes_match_an_explicit_legacy_run():
    import k1_port_mdl_supernode as ksn
    with workdir() as root:
        m1 = manifest_of(_convert(root, "o1"))
        assert m1["supernode"]["mode"] == "legacy" and m1["supernode"]["origin"] is None, m1["supernode"]
        assert m1["keep_dangly"] is False
        (root / "ledger.json").unlink()
        pack = root / "pack"
        m2 = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], out_dir=root / "o2",
                                 extra=["--supernode", "legacy"]))
        assert (root / "o1" / "k1p_test.mdl").read_bytes() == (root / "o2" / "k1p_test.mdl").read_bytes()
        # pykotor numbering: supernode == own index, total == own node count
        tree = ksn.parse_tree((root / "o1" / "k1p_test.mdl").read_bytes())
        assert [n.sn for n in tree.nodes] == [n.idx for n in tree.nodes] and tree.total == len(tree.nodes)
        assert m2["supernode"]["mode"] == "legacy"


def test_supernode_chain_differs_only_at_supernode_fields_and_total():
    import k1_port_mdl_supernode as ksn
    with workdir() as root:
        sup = _put_supermodel(root)
        legacy = manifest_of(_convert(root, "leg"))
        (root / "ledger.json").unlink()
        pack = root / "pack"
        chain = manifest_of(run_cli(root, pack, "k1p_test", ["n_testtex=k1p_new"], out_dir=root / "chn",
                                    extra=["--supernode", "chain", "--supermodel-dir", str(sup)]))
        a = (root / "leg" / "k1p_test.mdl").read_bytes()
        b = (root / "chn" / "k1p_test.mdl").read_bytes()
        assert len(a) == len(b) and a != b
        allowed = {n.off + k for n in ksn.parse_tree(a).nodes for k in (2, 3)} | {56, 57, 58, 59}
        diff = _diff_offsets(a, b)
        assert diff <= allowed, f"bytes changed outside node+2 / total: {sorted(diff - allowed)[:10]}"
        assert (root / "leg" / "k1p_test.mdx").read_bytes() == (root / "chn" / "k1p_test.mdx").read_bytes(), "MDX changed"
        tree = ksn.parse_tree(b)
        # root->supermodel root (0), body matches (3), head is unmatched -> max(5)+1 = 6; total 3 + (9 + 1)
        assert [n.sn for n in tree.nodes] == [0, 3, 6], [n.sn for n in tree.nodes]
        assert tree.total == 13, tree.total
        rec = chain["supernode"]
        assert rec["mode"] == "chain" and rec["supermodel"].lower() == "s_syn", rec
        assert rec["origin"] == f"dir:{sup / 's_syn.mdl'}", rec
        assert rec["supermodel_sha256"] == hashlib.sha256(SYN_SUPER_PK).hexdigest(), rec
        assert rec["total_before"] == 3 and rec["total_after"] == 13 and rec["nodes_changed"] == 2, rec
        assert legacy["k2_mdl_bytes"] == chain["k2_mdl_bytes"]
        assert chain["match"] is True, chain.get("mismatch_detail")
        # the ledger and the manifest see the FINAL bytes
        row = next(e for e in ledger_entries(root) if e["file"].endswith("chn/k1p_test.mdl"))
        assert row["sha256"] == hashlib.sha256(b).hexdigest(), "ledger sha is not the chain-numbered file"
        kv = _load_validate()
        found = kv.validate(b, (root / "chn" / "k1p_test.mdx").read_bytes(), game="K2", require_lower_k1p=True)
        base = kv.validate(a, (root / "leg" / "k1p_test.mdx").read_bytes(), game="K2", require_lower_k1p=True)
        assert [f.code for f in found] == [f.code for f in base], (found, base)


def test_supernode_chain_missing_supermodel_exits_1_and_writes_nothing():
    with workdir() as root:
        k2 = make_fake_k2(root)
        r = _convert(root, "out", extra=["--supernode", "chain"], supermodel="s_nowhere", k2=k2)
        assert r.returncode == 1, f"rc={r.returncode} stderr={r.stderr[-400:]}"
        assert "s_nowhere" in r.stderr and "SupernodeError" in r.stderr, r.stderr[-400:]
        out = root / "out"
        assert not out.exists() or not any(out.iterdir()), "failed run left output files"
        assert not any(e["file"].startswith(str(out)) for e in ledger_entries(root)) if (root / "ledger.json").exists() else True


def test_supernode_chain_bad_arguments_refused_before_writing():
    with workdir() as root:
        r = _convert(root, "out", extra=["--supernode", "chain", "--supermodel-dir", str(root / "nope")])
        assert r.returncode == 2 and "not a directory" in r.stderr, (r.returncode, r.stderr[-300:])
        assert not (root / "out").exists()
        r = _convert(root, "out", extra=["--supermodel-dir", str(root)])
        assert r.returncode == 2 and "--supernode chain" in r.stderr, (r.returncode, r.stderr[-300:])


def test_supernode_chain_override_supermodel_warns_and_dir_wins():
    with workdir() as root:
        k2 = make_fake_k2(root)
        (k2 / "Override" / "s_syn.mdl").write_bytes(SYN_SUPER_PK)
        m = manifest_of(_convert(root, "ov", extra=["--supernode", "chain"], k2=k2))
        rec = m["supernode"]
        assert rec["origin"] == "override", rec
        r = _convert(root, "ov2", extra=["--supernode", "chain"], k2=k2)  # only to read stderr of a fresh run
        assert "WARNING" in r.stderr and "Override" in r.stderr, r.stderr[-400:]
        assert any("Override" in w for w in m["warnings"]), m["warnings"]
        assert rec["supermodel_sha256"] == hashlib.sha256(SYN_SUPER_PK).hexdigest()
        # a --supermodel-dir copy wins over Override, and then no override warning is printed
        other = _syn_mdl(("anyroot", 0, [("k1p_body", 1, [])]), total=4)
        sup = _put_supermodel(root, data=other, sub="sup2")
        (root / "ledger.json").unlink()
        shutil.rmtree(root / "ov"), shutil.rmtree(root / "ov2")
        r = _convert(root, "ov3", extra=["--supernode", "chain", "--supermodel-dir", str(sup)], k2=k2)
        m3 = manifest_of(r)
        assert m3["supernode"]["origin"].startswith("dir:") and "WARNING" not in r.stderr, (m3["supernode"], r.stderr[-300:])
        assert m3["supernode"]["supermodel_sha256"] == hashlib.sha256(other).hexdigest()


def test_keep_dangly_flag_is_recorded_and_default_demotes():
    with workdir() as root:
        m = manifest_of(_convert(root, "kd", extra=["--keep-dangly"], supermodel=""))
        assert m["keep_dangly"] is True and m["dangly_demoted"] == 0, m


def test_golden_n_commm_chain_supernode_matches_s_female02():
    """Real K1 n_commm converted twice (legacy, chain) against the real K2 install: the chain output carries
    the K2 supermodel-chain numbering of vanilla s_female02 (k1_port_mdl_supernode.mismatches is empty), differs
    from legacy only at node+2 / total, and the MDX is byte-identical."""
    import k1_port_mdl_supernode as ksn
    k1 = Path(os.path.expanduser(os.environ.get("K1_INSTALL", "~/.local/share/Steam/steamapps/common/swkotor")))
    if not (k1 / "chitin.key").is_file() or not (REAL_K2 / "chitin.key").is_file():
        raise Skip("K1 or K2 install not found")
    vspec = importlib.util.spec_from_file_location("tv_golden_sn", HERE / "test_k1_port_validate.py")
    tv = importlib.util.module_from_spec(vspec)
    sys.modules["tv_golden_sn"] = tv
    vspec.loader.exec_module(tv)
    sup_bytes = next((m for n, m, x in tv._vanilla_k2_pairs() if n == "s_female02"), None)
    assert sup_bytes is not None, "s_female02 missing from K2 chitin"

    def conv(root, out):
        r = subprocess.run([K1PORT_PY, str(CLI), "--install-k1", str(k1), "--resref", "n_commm", "--out-dir",
                            str(root / out), "--k2-install", str(REAL_K2), "--new-resref", "k1p_commm",
                            "--source-id", "vanilla_k1", *CONV_EXTRA, "--tex-map", "N_CommM04=k1p_commm04"],
                           capture_output=True, text=True, timeout=600, env=test_env(root / "ledger.json"), cwd=root)
        return manifest_of(r)

    with workdir() as root:
        CONV_EXTRA.clear()
        legacy = conv(root, "leg")
        (root / "ledger.json").unlink()
        CONV_EXTRA[:] = ["--supernode", "chain"]
        try:
            chain = conv(root, "chn")
        finally:
            CONV_EXTRA.clear()
        a, b = (root / "leg" / "k1p_commm.mdl").read_bytes(), (root / "chn" / "k1p_commm.mdl").read_bytes()
        rec = chain["supernode"]
        assert rec["supermodel"].lower() == "s_female02" and rec["origin"] in ("install", "override"), rec
        if rec["origin"] == "override":  # the player's Override holds its own s_female02 (the converter warned): use it
            ov = next(f for f in (REAL_K2 / "override").iterdir() if f.name.lower() == "s_female02.mdl")
            sup_bytes = ov.read_bytes()
            assert any("Override" in w for w in chain["warnings"]), chain["warnings"]
        assert rec["supermodel_sha256"] == hashlib.sha256(sup_bytes).hexdigest(), rec
        res = ksn.mismatches(b, lambda name: sup_bytes)
        assert not res["mismatches"] and res["total_ok"], res["mismatches"][:5]
        allowed = {n.off + k for n in ksn.parse_tree(a).nodes for k in (2, 3)} | {56, 57, 58, 59}
        assert _diff_offsets(a, b) <= allowed
        assert ksn.mismatches(a, lambda name: sup_bytes)["mismatches"], "legacy output unexpectedly already has chain numbering"
        assert (root / "leg" / "k1p_commm.mdx").read_bytes() == (root / "chn" / "k1p_commm.mdx").read_bytes()
        assert legacy["supernode"]["mode"] == "legacy" and chain["match"] is True


CONV_EXTRA: list = []


PURE_TESTS = [
    ("identical_skin_and_tangent_summaries_all_true", test_identical_skin_and_tangent_summaries_all_true),
    ("differing_bonemap_flags_skin_bone_poses", test_differing_bonemap_flags_skin_bone_poses),
    ("differing_tangent_count_flags_tangent_data", test_differing_tangent_count_flags_tangent_data),
    ("tangent_bytes_isolates_the_tangent_block", test_tangent_bytes_isolates_the_tangent_block),
    ("zero_tangent_block_cleared_counts_as_match", test_zero_tangent_block_cleared_counts_as_match),
    ("zero_tangent_cleared_is_counted_per_mesh_and_skipped_in_detail",
     test_zero_tangent_cleared_is_counted_per_mesh_and_skipped_in_detail),
    ("nonzero_tangent_block_dropped_still_fails", test_nonzero_tangent_block_dropped_still_fails),
    ("zero_block_with_unexplained_bytes_still_fails", test_zero_block_with_unexplained_bytes_still_fails),
    ("keep_zero_mode_rejects_cleared_flag", test_keep_zero_mode_rejects_cleared_flag),
    ("default_source_resolution", test_default_source_resolution),
    ("default_new_resref_naming", test_default_new_resref_naming),
    ("new_resref_rules", test_new_resref_rules),
    ("root_rename_only_when_root_is_the_model", test_root_rename_only_when_root_is_the_model),
    ("skin_unknown0_warning_is_gone", test_skin_unknown0_warning_is_gone),
    ("pad_skin_bones_zeroes_only_unused_slots", test_pad_skin_bones_zeroes_only_unused_slots),
    ("pad_skin_bones_leaves_full_and_empty_skins_alone", test_pad_skin_bones_leaves_full_and_empty_skins_alone),
    ("fix_face_planes_recomputes_float_distance", test_fix_face_planes_recomputes_float_distance),
    ("demote_dangly_clears_only_dangly_nodes", test_demote_dangly_clears_only_dangly_nodes),
    ("supernode_legacy_is_a_byte_no_op", test_supernode_legacy_is_a_byte_no_op),
    ("supernode_chain_renumbers_with_a_supermodel_dir", test_supernode_chain_renumbers_with_a_supermodel_dir),
    ("supernode_dir_beats_override_beats_install", test_supernode_dir_beats_override_beats_install),
    ("supernode_missing_supermodel_raises_and_leaves_file_alone",
     test_supernode_missing_supermodel_raises_and_leaves_file_alone),
    ("supernode_chain_without_a_supermodel_is_the_identity", test_supernode_chain_without_a_supermodel_is_the_identity),
    ("supernode_arg_rules", test_supernode_arg_rules),
    ("keep_dangly_skips_the_demotion", test_keep_dangly_skips_the_demotion),
    ("supernode_and_keep_dangly_flags_are_parsed_with_legacy_defaults",
     test_supernode_and_keep_dangly_flags_are_parsed_with_legacy_defaults),
]


TESTS = [
    ("help_runs_under_system_python", test_help_runs_under_system_python),
    ("one_mesh_rename_and_k2_size", test_one_mesh_rename_and_k2_size),
    ("two_mesh_rename_hits_every_mesh_slot", test_two_mesh_rename_hits_every_mesh_slot),
    ("dirt_fields_default_on_output", test_dirt_fields_default_on_output),
    ("mdx_backed_vertex_counts", test_mdx_backed_vertex_counts),
    ("texture_name_length_limit", test_texture_name_length_limit),
    ("case_insensitive_new_collision_rejected", test_case_insensitive_new_collision_rejected),
    ("new_name_colliding_with_kept_texture_rejected", test_new_name_colliding_with_kept_texture_rejected),
    ("existing_output_is_never_overwritten", test_existing_output_is_never_overwritten),
    ("unknown_tex_map_source_rejected", test_unknown_tex_map_source_rejected),
    ("pack_lookup_is_case_insensitive", test_pack_lookup_is_case_insensitive),
    ("mesh_without_uv1_is_warned", test_mesh_without_uv1_is_warned),
    ("textured_mesh_without_uv1_stays_uv_less_by_default", test_textured_mesh_without_uv1_stays_uv_less_by_default),
    ("synth_zero_uv_flag_adds_uv1_and_grows_mdx", test_synth_zero_uv_flag_adds_uv1_and_grows_mdx),
    ("unavailable_supermodel_warns_and_exits_zero", test_unavailable_supermodel_warns_and_exits_zero),
    ("zero_tangent_block_cleared_end_to_end", test_zero_tangent_block_cleared_end_to_end),
    ("default_new_resref_used_without_flag", test_default_new_resref_used_without_flag),
    ("explicit_new_resref_sets_file_names_and_model_name", test_explicit_new_resref_sets_file_names_and_model_name),
    ("root_node_renamed_when_it_is_the_model_name", test_root_node_renamed_when_it_is_the_model_name),
    ("invalid_new_resref_refused_nothing_written", test_invalid_new_resref_refused_nothing_written),
    ("collision_with_fake_k2_refused_then_allowed", test_collision_with_fake_k2_refused_then_allowed),
    ("clean_fake_k2_passes_guard", test_clean_fake_k2_passes_guard),
    ("missing_k2_install_warns_and_continues", test_missing_k2_install_warns_and_continues),
    ("out_dir_inside_k2_install_refused", test_out_dir_inside_k2_install_refused),
    ("real_k2_install_refuses_n_commm", test_real_k2_install_refuses_n_commm),
    ("overlay_install_refused_without_allow_flag", test_overlay_install_refused_without_allow_flag),
    ("unknown_source_id_rejected_nothing_written", test_unknown_source_id_rejected_nothing_written),
    ("pack_without_source_id_rejected", test_pack_without_source_id_rejected),
    ("vanilla_source_not_flagged_and_outputs_registered", test_vanilla_source_not_flagged_and_outputs_registered),
    ("permission_source_flagged_for_removal", test_permission_source_flagged_for_removal),
    ("ledger_inside_repo_refused", test_ledger_inside_repo_refused),
    ("supermodel_map_rewrites_header_name", test_supermodel_map_rewrites_header_name),
    ("supermodel_map_unmatched_warns_and_bad_entries_refused", test_supermodel_map_unmatched_warns_and_bad_entries_refused),
    ("golden_n_commm_matches_k2_vanilla_skin_and_planes", test_golden_n_commm_matches_k2_vanilla_skin_and_planes),
    ("skin_round_trip_keeps_bones16_bonemap_and_weights", test_skin_round_trip_keeps_bones16_bonemap_and_weights),
    ("supernode_default_is_legacy_and_bytes_match_an_explicit_legacy_run",
     test_supernode_default_is_legacy_and_bytes_match_an_explicit_legacy_run),
    ("supernode_chain_differs_only_at_supernode_fields_and_total",
     test_supernode_chain_differs_only_at_supernode_fields_and_total),
    ("supernode_chain_missing_supermodel_exits_1_and_writes_nothing",
     test_supernode_chain_missing_supermodel_exits_1_and_writes_nothing),
    ("supernode_chain_bad_arguments_refused_before_writing", test_supernode_chain_bad_arguments_refused_before_writing),
    ("supernode_chain_override_supermodel_warns_and_dir_wins", test_supernode_chain_override_supermodel_warns_and_dir_wins),
    ("keep_dangly_flag_is_recorded_and_default_demotes", test_keep_dangly_flag_is_recorded_and_default_demotes),
    ("golden_n_commm_chain_supernode_matches_s_female02", test_golden_n_commm_chain_supernode_matches_s_female02),
]


def main() -> int:
    passed = failed = skipped = 0

    def run(name, fn):
        nonlocal passed, failed, skipped
        try:
            fn()
        except Skip as exc:  # a missing optional input (e.g. the real K2 install): not a failure
            skipped += 1
            print(f"SKIP {name}: {exc}", flush=True)
        except Exception as exc:  # noqa: BLE001 - every failure is reported, none aborts the run
            failed += 1
            print(f"FAIL {name}: {type(exc).__name__}: {str(exc)[:600]}", flush=True)
        else:
            passed += 1
            print(f"PASS {name}", flush=True)

    run(*TESTS[0])  # --help needs no venv
    for name, fn in PURE_TESTS:  # compare logic on hand-built summaries: no venv needed
        run(name, fn)
    if not Path(K1PORT_PY).is_file():
        print(f"SKIP k1_port_model: interpreter not found: {K1PORT_PY} "
              f"(build it with patch_manager_mods/k1_port/setup_venv.sh or set K1PORT_PY)", flush=True)
        return 1 if failed else 0
    for name, fn in TESTS[1:]:
        run(name, fn)
    total = passed + failed
    extra = f", {skipped} SKIP" if skipped else ""
    print(f"{passed}/{total} PASS{extra}", flush=True)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
