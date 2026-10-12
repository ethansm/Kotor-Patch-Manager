"""Convert one KOTOR 1 model (MDL + MDX) into a KOTOR 2 binary pair with renamed textures and a new resref.

Run with the patched pykotor venv built by setup_venv.sh, NOT system python. Stock pykotor 2.3.12
cannot write K2 MDL files. The script has no shebang on purpose:

    ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python patch_manager_mods/k1_port/k1_port_model.py \
        --resref n_commm --tex-map N_CommM04=k1p_commm04 --new-resref k1p_commm --out-dir DIR

Source: --pack-dir DIR (reads <resref>.mdl and .mdx, file names matched case-insensitively), or else the
K1 install (override, then chitin, then modules). The install is only read, never written.
Output: <out-dir>/<NEW>.mdl and <NEW>.mdx, written by the forced-K2 MDLBinaryWriter. NEW is --new-resref
(default: k1p_ + the source resref, lowercased, cut to 16 chars). The internal model name becomes NEW, and the
root node too when it carried the model name (K1 NPC models: root N_CommM = model N_CommM). The supermodel
reference (S_Female02 etc.) is never renamed. The output is re-read and compared with the source. Texture refs
are compared after the rename map is applied.
K2 collision guard: NEW must not name an MDL or MDX in the K2 install (--k2-install, read only: override,
chitin, modules; about 30 s when nothing matches). A hit refuses the run unless --allow-k2-collision is given:
a K2 Override file with the K1 resref would replace the vanilla K2 model for every NPC using it. A missing
K2 install only warns (manifest k2_collision_checked false). --out-dir may not be inside either install.
UV1 is not synthesized by default: a textured mesh with no UV1 in the source stays UV-less.
--synth-zero-uv turns on pykotor's zero-UV fill (MDLOps-style). Skin unknown0 arrays are kept by the patched pykotor.
The compare also checks skin bone data (bonemap, qbones, tbones, bone_node_number, per-vertex bone indices
and weights), the MDX tangent block (per-mesh stride) and the internal model name. match is true only if every
compare flag is true. One tangent case is not a defect: a K1 mesh whose flagged tangent block is all zero rows is
written without the flag and the block (patched pykotor; no K1/K2 vanilla precedent for flagged-zero). The compare
counts those meshes as matches (manifest tangent_zero_cleared, plus an informational warning). --keep-zero-tangents
keeps the flag and block and applies the strict compare.

Provenance: --source-id ID must be registered in sources.json. Required with --pack-dir; with the K1 install it
defaults to vanilla_k1. Both written files are registered in the removal ledger (kind "model"): --ledger PATH,
else env K1PORT_LEDGER, else the library default. The ledger path obeys the texture tool's stage guard.

Prints ONE JSON manifest to stdout. Progress and pykotor chatter go to stderr.
Exit codes: 0 converted (read manifest "match"; a mismatch still writes the files),
1 runtime failure (files this run wrote are removed), 2 refused before anything was written
(bad arguments, texture-name collisions, existing output, missing source, wrong interpreter).
--help works under any interpreter because pykotor is imported only after argument parsing.
"""

import argparse
import contextlib
import hashlib
import json
import logging
import re
import struct
import sys
from pathlib import Path
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parent))  # python -I drops the script dir from sys.path
from k1_port_sources import Ledger, SourceError, get_source  # noqa: E402  stdlib only
from k1_port_textures import repo_roots, stage_refusal  # noqa: E402  stdlib only; one stage/ledger guard

DEFAULT_SOURCE_ID = "vanilla_k1"
PROVENANCE = ("source_id", "license_class", "requires_author_approval", "removal_group", "credits")
DEFAULT_K1_INSTALL = "~/.local/share/Steam/steamapps/common/swkotor"
DEFAULT_K2_INSTALL = "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
PYKOTOR_HINT = "~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python (built by patch_manager_mods/k1_port/setup_venv.sh)"
MAX_TEXTURE_NAME = 31  # MDL texture fields hold 32 bytes including the terminating NUL
RESREF_MAX = 16  # ResRef limit: the output file stem and the model's resref
RESREF_RE = re.compile(r"[A-Za-z0-9_]{1,16}")  # the source resref (--resref)
NEW_RESREF_RE = re.compile(r"[a-z0-9_]+")  # the output resref (--new-resref), lowercase only
NEW_RESREF_PREFIX = "k1p_"
TEXTURE_RE = re.compile(r"[A-Za-z0-9_\-]+")
DIRT_DEFAULTS = [False, 1, 1, False]  # dirt_enabled, dirt_texture, dirt_worldspace, hologram_donotdraw
NONE_SUPERMODELS = ("", "NULL")
POSE_DIGITS = 5  # qbones/tbones compare after rounding to this many decimals
POS_BYTES, NORMAL_BYTES, UV_BYTES = 12, 12, 8  # MDX block sizes per vertex (float3, float3, float2)
TANGENT_BYTES = 36  # MDX tangent block per vertex (3 x vec3); pykotor keeps only the flag, the block is extra
SKIN_BYTES = 32  # MDX bone block (4 indices) + weight block (4 weights), per vertex
SKIN_KEYS = ("bonemap", "qbones", "tbones", "bone_node_number", "vertex_bones")
EXIT_OK, EXIT_FAILED, EXIT_REJECTED = 0, 1, 2


class Rejected(Exception):
    """A request refused before anything was written."""


class InstallUnavailable(Rejected):
    """The K1 install cannot be opened (missing, or not a K1 install)."""


# ---------------------------------------------------------------- pykotor access (imported lazily)

def load_pykotor() -> SimpleNamespace:
    # pykotor logs heavily at import and while loading an Installation; keep stderr usable.
    logging.disable(logging.CRITICAL)
    try:
        from pykotor.common.misc import Game
        from pykotor.extract.installation import Installation, SearchLocation
        from pykotor.resource.formats.mdl import io_mdl
        from pykotor.resource.formats.mdl.io_mdl import MDLBinaryWriter
        from pykotor.resource.formats.mdl.mdl_auto import read_mdl
        from pykotor.resource.type import ResourceType
    except ImportError as exc:
        raise Rejected(f"pykotor not importable ({exc}); run this script with {PYKOTOR_HINT}") from exc
    return SimpleNamespace(
        Game=Game, Installation=Installation, SearchLocation=SearchLocation,
        MDLBinaryWriter=MDLBinaryWriter, read_mdl=read_mdl, ResourceType=ResourceType, io_mdl=io_mdl,
    )


class Library:
    """Finds MDL/MDX pairs in the pack dir and/or the K1 install. The install is opened lazily."""

    def __init__(self, pk: SimpleNamespace, install: Path, pack_dir: Path | None):
        self.pk = pk
        self.install = install
        self.pack_dir = pack_dir
        self._inst = None

    def installation(self):
        if self._inst is None:
            if not self.install.is_dir():
                raise InstallUnavailable(f"K1 install not found: {self.install}")
            inst = self.pk.Installation(str(self.install))
            if inst.game() != self.pk.Game.K1:
                raise InstallUnavailable(f"{self.install} is not a K1 install (game={inst.game()})")
            self._inst = inst
        return self._inst

    def from_pack(self, resname: str):
        """(origin, path, mdl_bytes, mdx_bytes) from the pack dir, or None if the pair is absent."""
        mdl_path = find_ci(self.pack_dir, f"{resname}.mdl")
        mdx_path = find_ci(self.pack_dir, f"{resname}.mdx")
        if mdl_path is None or mdx_path is None:
            return None
        return ("pack", str(mdl_path), mdl_path.read_bytes(), mdx_path.read_bytes())

    def from_install(self, resname: str):
        """(origin, path, mdl_bytes, mdx_bytes) from the K1 install (override, chitin, modules), or None."""
        inst = self.installation()
        order = [self.pk.SearchLocation.OVERRIDE, self.pk.SearchLocation.CHITIN, self.pk.SearchLocation.MODULES]
        mdl = inst.resource(resname, self.pk.ResourceType.MDL, order)
        mdx = inst.resource(resname, self.pk.ResourceType.MDX, order)
        if mdl is None or mdx is None:
            return None
        return ("install", str(mdl.filepath), mdl.data, mdx.data)


def find_ci(directory: Path, filename: str) -> Path | None:
    """Exact file name first, then a case-insensitive match. None if absent."""
    exact = directory / filename
    if exact.is_file():
        return exact
    wanted = filename.lower()
    matches = sorted(p for p in directory.iterdir() if p.name.lower() == wanted and p.is_file())
    if len(matches) > 1:
        raise Rejected(f"{len(matches)} files in {directory} match {filename!r} case-insensitively")
    return matches[0] if matches else None


def k2_lookup(pk: SimpleNamespace, k2_path: Path, name: str) -> tuple[bool, list[str]]:
    """(checked, hits): hits are the paths of K2 MDL/MDX resources named name (override, chitin, modules).
    Read only. A missing install dir gives (False, []). A dir that is not a K2 install is refused."""
    if not k2_path.is_dir():
        return False, []
    try:
        inst = pk.Installation(str(k2_path))
        game = inst.game()
    except Exception as exc:  # noqa: BLE001 - pykotor raises several error types for a bad dir
        raise Rejected(f"cannot read the K2 install {k2_path}: {type(exc).__name__}: {exc}") from exc
    if game != pk.Game.K2:
        raise Rejected(f"--k2-install {k2_path} is not a K2 install (game={game})")
    order = [pk.SearchLocation.OVERRIDE, pk.SearchLocation.CHITIN, pk.SearchLocation.MODULES]
    hits = []
    for rtype in (pk.ResourceType.MDL, pk.ResourceType.MDX):
        found = inst.resource(name, rtype, order)
        if found is not None:
            hits.append(str(getattr(found, "filepath", "?")))
    return True, hits


# ---------------------------------------------------------------- argument rules (no pykotor needed)

def default_new_resref(source: str) -> str:
    """k1p_ + the source resref, lowercased and cut so the whole name fits a ResRef."""
    return (NEW_RESREF_PREFIX + source.lower())[:RESREF_MAX]


def check_new_resref(name: str) -> None:
    """--new-resref: lowercase ASCII letters, digits and '_', at most RESREF_MAX (16) chars."""
    if len(name) > RESREF_MAX:
        raise Rejected(f"--new-resref {name!r} is {len(name)} chars; the ResRef limit is {RESREF_MAX}")
    if not NEW_RESREF_RE.fullmatch(name):
        raise Rejected(f"--new-resref {name!r} must be lowercase ASCII letters, digits and '_' only")

def parse_tex_map(items: list[str]) -> dict[str, tuple[str, str]]:
    """Map lower(OLD) -> (OLD as typed, NEW lowercased). Validates the NEW names."""
    mapping: dict[str, tuple[str, str]] = {}
    for item in items:
        old, sep, new = item.partition("=")
        if not sep or not old or not new:
            raise Rejected(f"--tex-map entry {item!r} must be OLD=NEW")
        if len(new) > MAX_TEXTURE_NAME:
            raise Rejected(f"NEW texture name {new!r} is {len(new)} chars; the limit is {MAX_TEXTURE_NAME}")
        if not TEXTURE_RE.fullmatch(new):
            raise Rejected(f"NEW texture name {new!r} may only contain ASCII letters, digits, '_' and '-'")
        key = old.lower()
        if key in mapping:
            raise Rejected(f"OLD texture name {old!r} is given twice (names are case-insensitive)")
        mapping[key] = (old, new.lower())
    return mapping


def check_rename_plan(tex_map: dict[str, tuple[str, str]], model_textures: set[str]) -> None:
    """Refuse maps that name a texture the model lacks, or that merge two textures into one name."""
    for key, (old, _new) in tex_map.items():
        if key not in model_textures:
            raise Rejected(f"--tex-map OLD {old!r} does not occur in any mesh texture_1/texture_2")
    kept = model_textures - set(tex_map)
    targets: dict[str, str] = {}
    for _key, (old, new) in tex_map.items():
        if new in kept:
            raise Rejected(f"NEW {new!r} (from {old!r}) collides with a texture the model keeps")
        if new in targets:
            raise Rejected(f"NEW {new!r} is the target of both {targets[new]!r} and {old!r} (case-insensitive)")
        targets[new] = old


# ---------------------------------------------------------------- model walk, summary, rename

def walk_nodes(node):
    yield node
    for child in node.children:
        yield from walk_nodes(child)


def _bounds(points, axes=("x", "y", "z")):
    if not points:
        return None, None
    lo = [round(min(getattr(p, a) for p in points), 4) for a in axes]
    hi = [round(max(getattr(p, a) for p in points), 4) for a in axes]
    return lo, hi


def _digest_vectors(items, axes) -> str:
    h = hashlib.sha256()
    h.update(struct.pack("<I", len(items)))
    fmt = "<%df" % len(axes)
    for v in items:
        h.update(struct.pack(fmt, *(float(getattr(v, a)) for a in axes)))
    return h.hexdigest()[:16]


def _digest_faces(faces) -> str:
    h = hashlib.sha256()
    h.update(struct.pack("<I", len(faces)))
    for f in faces:
        h.update(struct.pack("<5q", int(f.v1), int(f.v2), int(f.v3), int(f.material), int(f.smoothgroup)))
    return h.hexdigest()[:16]


def _mesh_record(mesh) -> dict:
    pos = list(mesh.vertex_positions or [])
    faces = list(mesh.faces or [])
    uv1 = list(mesh.vertex_uv1 or mesh.vertex_uvs or [])
    uv2 = list(mesh.vertex_uv2 or [])
    normals = list(mesh.vertex_normals or [])
    lo, hi = _bounds(pos)
    tangent_rows = getattr(mesh, "tangent_block", None) or []  # patched reader: one 36-byte row per vertex
    return {
        "texture_1": mesh.texture_1 or "",
        "texture_2": mesh.texture_2 or "",
        "vertex_count": len(pos),
        "face_count": len(faces),
        "uv1_count": len(uv1),
        "uv2_count": len(uv2),
        "normal_count": len(normals),
        "pos_min": lo,
        "pos_max": hi,
        "uv1": _digest_vectors(uv1, ("x", "y")),
        "uv2": _digest_vectors(uv2, ("x", "y")),
        "normals": _digest_vectors(normals, ("x", "y", "z")),
        "faces": _digest_faces(faces),
        "dirt": [bool(mesh.dirt_enabled), int(mesh.dirt_texture), int(mesh.dirt_worldspace),
                 bool(mesh.hologram_donotdraw)],
        "tangent_space": bool(mesh.tangent_space),
        # A stored tangent block with rows, all bytes zero (the writer drops such blocks; see zero_tangent_cleared).
        "tangent_block_zero": bool(tangent_rows) and all(b == 0 for row in tangent_rows for b in row),
    }


def _rounded(values, axes) -> list[list[float]]:
    """Float tuples rounded to POSE_DIGITS, so a re-read bone pose compares as a plain list."""
    return [[round(float(getattr(v, a)), POSE_DIGITS) for a in axes] for v in values]


def _skin_record(skin) -> dict | None:
    """Bone data of one skin node as pykotor reads it back. None for a non-skin mesh."""
    if skin is None:
        return None
    return {
        "bonemap": [int(b) for b in skin.bonemap],
        "qbones": _rounded(skin.qbones, ("x", "y", "z", "w")),
        "tbones": _rounded(skin.tbones, ("x", "y", "z")),
        "bone_node_number": [int(n) for n in skin.bone_node_number],
        # Per vertex: 4 bone indices and 4 weights, exact (no rounding).
        "vertex_bones": [[[float(i) for i in vb.vertex_indices], [float(w) for w in vb.vertex_weights]]
                         for vb in skin.vertex_bones],
    }


def tangent_bytes(rec: dict) -> int:
    """MDX bytes per vertex that are not explained by the blocks pykotor models (pos, normal, uv1, uv2, skin).

    pykotor keeps only the TANGENT_SPACE flag; the tangent block itself is not modelled. So the block size is
    the per-vertex MDX stride minus the modelled blocks: 36 bytes for K1 tangent meshes, 0 when absent.
    """
    base = ((POS_BYTES if rec["vertex_count"] else 0) + (NORMAL_BYTES if rec["normal_count"] else 0)
            + (UV_BYTES if rec["uv1_count"] else 0) + (UV_BYTES if rec["uv2_count"] else 0)
            + (SKIN_BYTES if rec["skin"] is not None else 0))
    return rec["mdx_stride"] - base


def zero_tangent_cleared(sm: list[dict], om: list[dict]) -> list[bool]:
    """Per mesh pair: True when the source mesh was tangent-flagged with a stored block of all-zero rows, and the
    output mesh has the flag cleared and no tangent bytes (the writer omitted the flag and the block). This is the
    one tangent difference the compare accepts. Any other difference (non-zero rows dropped, flag lost with data,
    a stride change not explained by the 36-byte block) is not covered and still fails tangent_data."""
    if len(sm) != len(om):
        return [False] * len(sm)
    return [
        bool(a.get("tangent_block_zero", False)) and bool(a["tangent_space"]) and a["tangent_bytes"] == TANGENT_BYTES
        and not b["tangent_space"] and b["tangent_bytes"] == 0
        for a, b in zip(sm, om)
    ]


def binary_mdx_strides(pk: SimpleNamespace, mdl_bytes: bytes, mdx_bytes: bytes) -> list[int]:
    """Per-mesh MDX bytes per vertex (trimesh header mdx_data_size), in the same pre-order as walk_nodes.

    pykotor's MDL object does not keep this value, so the binary node headers are read directly with
    pykotor's own binary classes (io_mdl private names; the k1port venv pins the version). Reads only.
    """
    io = pk.io_mdl
    probe = io.MDLBinaryReader(mdl_bytes, source_ext=mdx_bytes, size_ext=len(mdx_bytes))
    probe.load()  # sets probe.game (K1 or K2) from the geometry header, as the normal read does
    reader = io.MDLBinaryReader(mdl_bytes, source_ext=mdx_bytes, size_ext=len(mdx_bytes))
    reader.game = probe.game
    header = io._ModelHeader().read(reader._reader)
    strides: list[int] = []

    def visit(offset: int) -> None:  # same traversal as MDLBinaryReader._load_node
        reader._reader.seek(offset)
        node = io._Node().read(reader._reader, reader.game)
        if node.trimesh is not None:
            strides.append(int(node.trimesh.mdx_data_size))
        for child in node.children_offsets:
            if child not in (0, 0xFFFFFFFF):
                visit(child)

    visit(header.geometry.root_node_offset)
    return strides


def summarize(mdl, mdx_strides: list[int] | None = None) -> dict:
    """Model summary. mdx_strides (from binary_mdx_strides) adds the tangent fields to each mesh record."""
    nodes, meshes = [], []
    for node in walk_nodes(mdl.root):
        nodes.append({
            "name": node.name,
            "node_type": int(node.node_type),
            "child_count": len(node.children),
            "is_mesh": node.mesh is not None,
        })
        if node.mesh is not None:
            rec = _mesh_record(node.mesh)
            rec["name"] = node.name
            rec["skin"] = _skin_record(node.skin)
            meshes.append(rec)
    if mdx_strides is not None:
        if len(mdx_strides) != len(meshes):
            raise RuntimeError(f"binary walk found {len(mdx_strides)} trimesh headers, "
                               f"the model object has {len(meshes)} meshes")
        for rec, stride in zip(meshes, mdx_strides):
            rec["mdx_stride"] = stride
            rec["tangent_bytes"] = tangent_bytes(rec)
    return {"model_name": mdl.name or "", "nodes": nodes, "meshes": meshes, "animation_count": len(mdl.anims)}


def _has_texture(name: str) -> bool:
    return bool(name) and name.strip() != "" and name.upper() != "NULL"


def bare_uv_warning(summary: dict, synth: bool = False) -> str | None:
    """Textured meshes with vertices but no UV1 in the source. Only these can get zero UVs (synth_zero_uv)."""
    bare = [m["name"] for m in summary["meshes"]
            if m["vertex_count"] and not m["uv1_count"] and _has_texture(m["texture_1"])]
    if not bare:
        return None
    shown = ", ".join(bare[:5]) + (" ..." if len(bare) > 5 else "")
    head = f"informational: {len(bare)} textured mesh(es) have no UV1 ({shown}); "
    if synth:
        return head + "--synth-zero-uv writes zero UVs for them, so uv_checksum will not match"
    return head + "output keeps them UV-less (not a defect: K2 vanilla n_commm has 11 such meshes too)"


def rename_model(mdl, new_name: str) -> bool:
    """Sets the internal model name to new_name. The root node is renamed too when it carries the model name
    (K1 NPC models: root 'N_CommM' and model 'N_CommM'). Returns True when the root was renamed."""
    root_name = mdl.root.name or ""
    root_is_model = bool(mdl.name) and root_name.lower() == mdl.name.lower()
    mdl.name = new_name
    if root_is_model:
        mdl.root.name = new_name
    return root_is_model


def model_texture_names(summary: dict) -> set[str]:
    return {t.lower() for m in summary["meshes"] for t in (m["texture_1"], m["texture_2"]) if t}


def apply_rename(mdl, tex_map: dict[str, tuple[str, str]]) -> int:
    renamed = 0
    for node in walk_nodes(mdl.root):
        mesh = node.mesh
        if mesh is None:
            continue
        for attr in ("texture_1", "texture_2"):
            value = getattr(mesh, attr) or ""
            hit = tex_map.get(value.lower()) if value else None
            if hit is not None:
                setattr(mesh, attr, hit[1])
                renamed += 1
    return renamed


def _first_diff(expected: list, actual: list) -> str:
    if len(expected) != len(actual):
        return f"length {len(expected)} -> {len(actual)}"
    for i, (a, b) in enumerate(zip(expected, actual)):
        if a != b:
            return f"index {i}: {str(a)[:80]} -> {str(b)[:80]}"
    return "differs"


def _first_skin_diff(sm: list[dict], om: list[dict]) -> str:
    if len(sm) != len(om):
        return f"mesh count {len(sm)} -> {len(om)}"
    for i, (a, b) in enumerate(zip(sm, om)):
        sa, sb = a["skin"], b["skin"]
        if sa == sb:
            continue
        label = f"mesh {i} ({a['name']})"
        if sa is None or sb is None:
            return f"{label}: skin {'absent' if sa is None else 'present'} -> {'absent' if sb is None else 'present'}"
        for key in SKIN_KEYS:
            if sa[key] != sb[key]:
                return f"{label} skin.{key}: {_first_diff(sa[key], sb[key])}"
    return "differs"


def _first_tangent_diff(sm: list[dict], om: list[dict], cleared: list[bool]) -> str:
    if len(sm) != len(om):
        return f"mesh count {len(sm)} -> {len(om)}"
    for i, (a, b) in enumerate(zip(sm, om)):
        if cleared[i]:  # the accepted zero-block drop is not a difference to report
            continue
        if (a["tangent_space"], a["tangent_bytes"]) != (b["tangent_space"], b["tangent_bytes"]):
            return (f"mesh {i} ({a['name']}): tangent flag {a['tangent_space']} -> {b['tangent_space']}, "
                    f"tangent bytes/vertex {a['tangent_bytes']} -> {b['tangent_bytes']} "
                    f"(mdx stride {a['mdx_stride']} -> {b['mdx_stride']})")
    return "differs"


def overall_match(results: dict) -> bool:
    """A model matches only if every compare flag holds (a false flag can never be hidden by the others)."""
    return bool(results) and all(results.values())


def compare(src: dict, out: dict, tex_map: dict[str, tuple[str, str]], model_name: str | None = None,
            root_renamed: bool = False, allow_zero_tangent_drop: bool = True) -> tuple[dict, dict]:
    """model_name: the internal name the output must carry (None: the source's name). root_renamed: rename_model()
    renamed the root node to model_name on purpose, so the node-tree check expects that one name there.
    allow_zero_tangent_drop: a flagged all-zero source tangent block that the output dropped (zero_tangent_cleared)
    counts as a match. False is the strict flag-and-bytes compare (--keep-zero-tangents)."""
    def mapped(name: str) -> str:
        hit = tex_map.get(name.lower()) if name else None
        return hit[1] if hit is not None else name

    sm, om = src["meshes"], out["meshes"]
    cleared = zero_tangent_cleared(sm, om) if allow_zero_tangent_drop else [False] * len(sm)
    tangent_expected = [[False, 0] if c else [m["tangent_space"], m["tangent_bytes"]] for m, c in zip(sm, cleared)]
    expected_nodes = src["nodes"]
    if root_renamed and expected_nodes:
        expected_nodes = [dict(expected_nodes[0], name=model_name)] + expected_nodes[1:]
    expected_name = model_name if model_name is not None else src.get("model_name")
    fields = {
        "model_name": ([expected_name], [out.get("model_name")]),
        "node_tree": (expected_nodes, out["nodes"]),
        "animation_count": ([src["animation_count"]], [out["animation_count"]]),
        "vertex_face_counts": ([[m["vertex_count"], m["face_count"]] for m in sm],
                               [[m["vertex_count"], m["face_count"]] for m in om]),
        "mesh_extents": ([[m["pos_min"], m["pos_max"]] for m in sm], [[m["pos_min"], m["pos_max"]] for m in om]),
        "uv_checksum": ([[m["uv1"], m["uv2"]] for m in sm], [[m["uv1"], m["uv2"]] for m in om]),
        "normals_checksum": ([m["normals"] for m in sm], [m["normals"] for m in om]),
        "faces_checksum": ([m["faces"] for m in sm], [m["faces"] for m in om]),
        "textures": ([[mapped(m["texture_1"]), mapped(m["texture_2"])] for m in sm],
                     [[m["texture_1"], m["texture_2"]] for m in om]),
        "dirt_defaults": ([list(DIRT_DEFAULTS) for _ in om], [m["dirt"] for m in om]),
        "skin_bone_poses": ([m["skin"] for m in sm], [m["skin"] for m in om]),
        "tangent_data": (tangent_expected, [[m["tangent_space"], m["tangent_bytes"]] for m in om]),
    }
    results, detail = {}, {}
    for name, (expected, actual) in fields.items():
        results[name] = expected == actual
        if not results[name]:
            detail[name] = _first_diff(expected, actual)
    # The generic diff prints whole skin dicts or flag pairs; these say which mesh and which field differ.
    if "skin_bone_poses" in detail:
        detail["skin_bone_poses"] = _first_skin_diff(sm, om)
    if "tangent_data" in detail:
        detail["tangent_data"] = _first_tangent_diff(sm, om, cleared)
    return results, detail


# ---------------------------------------------------------------- supermodel

def load_supermodel(lib: Library, pk: SimpleNamespace, name: str, warnings: list[str]) -> tuple[str, str | None]:
    """Returns (status, origin). Status: none | read | not_found | unreadable. Never raises."""
    if name.strip().upper() in NONE_SUPERMODELS:
        return "none", None
    try:
        found = (lib.from_pack(name) if lib.pack_dir is not None else None) or lib.from_install(name)
    except InstallUnavailable as exc:
        warnings.append(f"supermodel {name!r} not looked up: {exc}")
        return "not_found", None
    except (OSError, Rejected) as exc:
        warnings.append(f"supermodel {name!r} lookup failed: {exc}")
        return "not_found", None
    if found is None:
        warnings.append(f"supermodel {name!r} not found in the pack dir or the K1 install")
        return "not_found", None
    origin, _path, mdl_bytes, mdx_bytes = found
    try:
        pk.read_mdl(mdl_bytes, source_ext=mdx_bytes, size_ext=len(mdx_bytes))
    except Exception as exc:  # noqa: BLE001 - any parse failure is a warning, not a stop
        warnings.append(f"supermodel {name!r} ({origin}) unreadable: {type(exc).__name__}: {str(exc)[:160]}")
        return "unreadable", origin
    return "read", origin


# ---------------------------------------------------------------- conversion

def _is_within(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
        return True
    except ValueError:
        return False


def resolve_source_id(given: str | None, pack_given: bool) -> str:
    """--source-id: required with --pack-dir; with the K1 install it defaults to vanilla_k1. Must be registered."""
    if given is None:
        if pack_given:
            raise Rejected("--pack-dir needs --source-id (a registry id from sources.json; see k1_port_sources.py show)")
        given = DEFAULT_SOURCE_ID
    try:
        get_source(given)
    except SourceError as exc:
        raise Rejected(str(exc)) from exc
    return given


def open_ledger(path_arg: Path | None) -> Ledger:
    """The removal ledger (--ledger, else env K1PORT_LEDGER, else the library default). Refused inside the repo,
    under steamapps/common or compatdata: the texture tool's --stage-dir guard."""
    try:
        led = Ledger(path_arg)
    except (SourceError, OSError) as exc:
        raise Rejected(f"cannot open ledger: {exc}") from exc
    where = led.path.absolute()
    reason = stage_refusal(where, repo_roots())
    if reason:
        raise Rejected(f"--ledger {where} refused: {reason}")
    return led


def register_outputs(led: Ledger, source_id: str, resref: str, tex_map: dict, out_mdl: Path, out_mdx: Path,
                     mdl_bytes: bytes, mdx_bytes: bytes) -> tuple[dict, dict]:
    """Adds both written files to the ledger (memory only; the caller saves)."""
    texture_map = {old: new for old, new in tex_map.values()}
    mdl_entry = led.add(str(out_mdl), "model", source_id, sha256=hashlib.sha256(mdl_bytes).hexdigest(),
                        resref=resref, texture_map=texture_map)
    mdx_entry = led.add(str(out_mdx), "model", source_id, sha256=hashlib.sha256(mdx_bytes).hexdigest(),
                        resref=resref, texture_map=texture_map)
    return mdl_entry, mdx_entry


def convert(args: argparse.Namespace) -> dict:
    resref = args.resref
    if not RESREF_RE.fullmatch(resref):
        raise Rejected(f"--resref {resref!r} must be 1-16 ASCII letters, digits or '_'")
    new_resref = args.new_resref if args.new_resref is not None else default_new_resref(resref)
    check_new_resref(new_resref)
    tex_map = parse_tex_map(args.tex_map)

    install = Path(args.install_k1).expanduser()
    k2_install = Path(args.k2_install).expanduser()
    pack_dir = Path(args.pack_dir).expanduser() if args.pack_dir else None
    if pack_dir is not None and not pack_dir.is_dir():
        raise Rejected(f"--pack-dir is not a directory: {pack_dir}")
    source_id = resolve_source_id(args.source_id, pack_dir is not None)
    led = open_ledger(args.ledger)
    out_dir = Path(args.out_dir).expanduser().resolve()
    if out_dir.exists() and not out_dir.is_dir():
        raise Rejected(f"--out-dir exists and is not a directory: {out_dir}")
    if _is_within(out_dir, install.resolve()):
        raise Rejected(f"--out-dir {out_dir} is inside the K1 install; the install is read-only")
    if k2_install.is_dir() and _is_within(out_dir, k2_install.resolve()):
        raise Rejected(f"--out-dir {out_dir} is inside the K2 install; the install is read-only")
    out_mdl, out_mdx = out_dir / f"{new_resref}.mdl", out_dir / f"{new_resref}.mdx"
    existing = [p.name for p in (out_mdl, out_mdx) if p.exists()]
    if existing:
        raise Rejected(f"refusing to overwrite existing output in {out_dir}: {', '.join(existing)}")

    pk = load_pykotor()
    lib = Library(pk, install, pack_dir)
    if pack_dir is not None:
        found = lib.from_pack(resref)
        if found is None:
            raise Rejected(f"{resref}.mdl and {resref}.mdx not both found in {pack_dir}")
    else:
        found = lib.from_install(resref)
        if found is None:
            raise Rejected(f"{resref}.mdl and {resref}.mdx not found in the K1 install {install}")
    origin, source_path, mdl_bytes, mdx_bytes = found

    warnings: list[str] = []
    mdl = pk.read_mdl(mdl_bytes, source_ext=mdx_bytes, size_ext=len(mdx_bytes))
    src = summarize(mdl, binary_mdx_strides(pk, mdl_bytes, mdx_bytes))  # before the rename, so compare applies the map
    check_rename_plan(tex_map, model_texture_names(src))
    synth = bool(args.synth_zero_uv)
    uv_warning = bare_uv_warning(src, synth)
    if uv_warning:
        warnings.append(uv_warning)
    skin_count = sum(1 for node in walk_nodes(mdl.root) if node.skin is not None)
    sm_status, sm_origin = load_supermodel(lib, pk, mdl.supermodel or "", warnings)

    # Collision guard, before anything is written: a K2 resource with the output name would be replaced in Override.
    k2_checked, k2_hits = k2_lookup(pk, k2_install, new_resref)
    if not k2_checked:
        warnings.append(f"K2 install not found at {k2_install}: {new_resref!r} NOT checked against K2 resources")
    k2_collision = bool(k2_hits)
    if k2_collision:
        where = "; ".join(k2_hits)
        if not args.allow_k2_collision:
            raise Rejected(f"K2 install already has a resource named {new_resref!r} ({where}); pick another "
                           "--new-resref, or pass --allow-k2-collision to replace it on purpose")
        warnings.append(f"K2 collision allowed by --allow-k2-collision: {new_resref!r} also exists in K2 ({where})")

    renamed = apply_rename(mdl, tex_map)
    root_renamed = rename_model(mdl, new_resref)
    created_dir = not out_dir.exists()
    written: list[Path] = []
    ledger_touched = False
    try:
        out_dir.mkdir(parents=True, exist_ok=True)
        written += [out_mdl, out_mdx]
        writer = pk.MDLBinaryWriter(mdl, out_mdl, out_mdx)
        writer.force_game = pk.Game.K2
        writer.synth_zero_uv = synth
        writer.keep_zero_tangents = bool(args.keep_zero_tangents)
        writer.write()
        k2_mdl_bytes, k2_mdx_bytes = out_mdl.read_bytes(), out_mdx.read_bytes()
        back = pk.read_mdl(k2_mdl_bytes, source_ext=k2_mdx_bytes, size_ext=len(k2_mdx_bytes))
        out = summarize(back, binary_mdx_strides(pk, k2_mdl_bytes, k2_mdx_bytes))
        ledger_touched = True
        mdl_entry, mdx_entry = register_outputs(led, source_id, new_resref, tex_map, out_mdl, out_mdx,
                                                k2_mdl_bytes, k2_mdx_bytes)
        led.save()
    except BaseException:
        for p in written:
            with contextlib.suppress(OSError):
                p.unlink()
        if ledger_touched:
            for p in written:
                led.remove(p)
            with contextlib.suppress(OSError):
                led.save()  # a stale entry for a missing file is harmless; this just keeps the ledger honest
        if created_dir:
            with contextlib.suppress(OSError):
                out_dir.rmdir()
        raise

    results, detail = compare(src, out, tex_map, model_name=new_resref, root_renamed=root_renamed,
                              allow_zero_tangent_drop=not args.keep_zero_tangents)
    zero_cleared = 0 if args.keep_zero_tangents else sum(zero_tangent_cleared(src["meshes"], out["meshes"]))
    if zero_cleared:
        warnings.append(f"informational: {zero_cleared} meshes had all-zero tangent rows in K1; flag and block "
                        "omitted (no K1/K2 vanilla precedent for flagged-zero)")
    manifest = {
        "resref": resref,
        "new_resref": new_resref,
        "source": origin,
        "source_path": source_path,
        "supermodel": None if sm_status == "none" else mdl.supermodel,
        "supermodel_status": sm_status,
        "supermodel_origin": sm_origin,
        "texture_map": {old: new for old, new in tex_map.values()},
        "renames_applied": renamed,
        "node_count": len(src["nodes"]),
        "mesh_count": len(src["meshes"]),
        "skin_mesh_count": skin_count,
        "k2_collision_checked": k2_checked,
        "k2_collision": k2_collision,
        "synth_zero_uv": synth,
        "keep_zero_tangents": bool(args.keep_zero_tangents),
        "tangent_zero_cleared": zero_cleared,
        "vertex_total": sum(m["vertex_count"] for m in src["meshes"]),
        "face_total": sum(m["face_count"] for m in src["meshes"]),
        "k1_mdl_bytes": len(mdl_bytes),
        "k1_mdx_bytes": len(mdx_bytes),
        "k2_mdl_bytes": len(k2_mdl_bytes),
        "k2_mdx_bytes": len(k2_mdx_bytes),
        "out_dir": str(out_dir),
        "source_id": source_id,
        "license_class": mdl_entry["license_class"],
        "requires_author_approval": mdl_entry["requires_author_approval"],
        "removal_group": mdl_entry["removal_group"],
        "credits": mdl_entry["credits"],
        "ledger": str(led.path.absolute()),
        "match": overall_match(results),
        "compare": results,
        "warnings": warnings,
    }
    if detail:
        manifest["mismatch_detail"] = detail
    return manifest


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="Convert one KOTOR 1 model to a KOTOR 2 MDL/MDX pair with renamed textures. "
                    "Run with the patched pykotor venv (see module docstring).",
    )
    p.add_argument("--install-k1", default=DEFAULT_K1_INSTALL,
                   help=f"K1 install root, read-only (default {DEFAULT_K1_INSTALL})")
    p.add_argument("--resref", required=True, help="model resref, e.g. n_commm")
    p.add_argument("--pack-dir", help="read <resref>.mdl/.mdx from this dir instead of the K1 install "
                                      "(then --source-id is required)")
    p.add_argument("--source-id", default=None,
                   help=f"registry id from sources.json (default {DEFAULT_SOURCE_ID} with the K1 install; "
                        "required with --pack-dir)")
    p.add_argument("--ledger", type=Path, default=None,
                   help="removal ledger JSON (default: env K1PORT_LEDGER, else the library default)")
    p.add_argument("--tex-map", required=True, nargs="+", metavar="OLD=NEW",
                   help="texture renames, matched case-insensitively; NEW is written lowercase, max 31 chars")
    p.add_argument("--new-resref", default=None, metavar="NAME",
                   help="output resref: file stem and internal model name; lowercase letters, digits, '_', "
                        f"max {RESREF_MAX} chars (default: {NEW_RESREF_PREFIX} + source resref, cut to {RESREF_MAX})")
    p.add_argument("--k2-install", default=DEFAULT_K2_INSTALL,
                   help=f"K2 install, read-only; NEW must not name a K2 MDL/MDX (default {DEFAULT_K2_INSTALL})")
    p.add_argument("--allow-k2-collision", action="store_true",
                   help="write even though the K2 install has NEW (the output would replace that K2 model)")
    p.add_argument("--out-dir", required=True,
                   help="output dir; existing <new-resref>.mdl/.mdx are never overwritten")
    p.add_argument("--synth-zero-uv", action="store_true",
                   help="fill UV1 with zeros for textured meshes that have none (pykotor/MDLOps behaviour; "
                        "default off keeps them UV-less, matching the source)")
    p.add_argument("--keep-zero-tangents", action="store_true",
                   help="keep a flagged all-zero tangent block (flag and rows written as-is) and apply the strict "
                        "tangent compare; default drops such blocks and counts them in tangent_zero_cleared")
    return p


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    real_stdout = sys.stdout
    sys.stdout = sys.stderr  # nothing but the manifest may reach stdout
    try:
        manifest = convert(args)
    except Rejected as exc:
        print(f"error: rejected, nothing written: {exc}", file=sys.stderr, flush=True)
        return EXIT_REJECTED
    except Exception as exc:  # noqa: BLE001
        print(f"error: {type(exc).__name__}: {exc}", file=sys.stderr, flush=True)
        return EXIT_FAILED
    finally:
        sys.stdout = real_stdout
    print(json.dumps(manifest, indent=2), flush=True)
    return EXIT_OK


if __name__ == "__main__":
    sys.exit(main())
