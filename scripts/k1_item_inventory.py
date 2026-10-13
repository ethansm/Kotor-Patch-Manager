#!/usr/bin/env python3
"""Deterministic inventory of KotOR 1 item templates (UTI) and base items, flagged against KotOR 2.

Read-only over both Steam installs. Writes into --out:
  baseitems_k1.tsv / baseitems_k2.tsv  one row per baseitems.2da row (name resolved via the talk table)
  baseitems_diff.tsv                   one row per base-item label (case-insensitive) across both games
  items_k1.tsv / items_k2.tsv          one row per distinct UTI resref (primary copy chosen by precedence)
  items_k1_only.tsv                    K1 resrefs with no UTI of that resref in K2, with K2 look-alikes,
                                       model and texture checks against K2
  SUMMARY.md                           counts, rules used, parse-failure counts

Containers scanned per game: override, chitin (bifs), every module file (.rim, _s.rim, .mod; symlink
case-variants collapsed by realpath), and every .rim/.mod/.erf in the rims folder.
Primary copy of a resref: OVERRIDE > CHITIN > MODULE (sorted) > RIMS (sorted). All copies are listed.
Model and texture presence (MDL/MDX/TPC/TGA) come from override + chitin + modules only.

Run with the patched pykotor venv:
  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I scripts/k1_item_inventory.py --out .claude/research/k1_items
"""

from __future__ import annotations

import argparse
import collections
import os
import sys
from pathlib import Path
from types import SimpleNamespace
from typing import NamedTuple

REPO_ROOT = Path(__file__).resolve().parent.parent
K1_DIR_DEFAULT = Path.home() / ".local/share/Steam/steamapps/common/swkotor"
K2_DIR_DEFAULT = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OUT_DEFAULT = REPO_ROOT / ".claude/research/k1_items"
HELPER_DIR = REPO_ROOT / "patch_manager_mods" / "k1_port"
GROUP_ORDER = {"OVERRIDE": 0, "CHITIN": 1, "MODULE": 2, "RIMS": 3}
NO_MODEL_NAMES = {"", "i_null"}
TOP_LABELS = 20
# baseitems.2da stat columns; a column is written only when the game's baseitems.2da has it.
STAT_COLS = [
    "baseac", "dexbonus", "accheck", "armorcheckpen", "numdice", "dietoroll", "critthreat", "crithitmult",
    "basecost", "weapontype", "weaponsize", "weaponwield", "rangedweapon", "maxattackrange", "stacking",
    "maxprops", "minprops", "propcolumn", "armortype", "tenthlbs", "weaponmattype", "ammunitiontype",
    "itemtype", "itemmultiplier",
]
BASE_FIXED_COLS = ["row_id", "label", "name_strref", "name", "equipableslots", "slot_names", "itemclass",
                   "defaultmodel", "defaulticon", "modeltype", "bodyvar"]
ITEM_COLS = ["resref", "tag", "name", "baseitem_id", "baseitem_label", "modeltype", "itemclass",
             "defaultmodel", "model_variation", "texture_var", "body_variation", "cost", "upgrades",
             "upgrade_count", "property_count", "property_types", "icon_resref", "icon_rule", "icon_in_game",
             "model_resref", "model_derived", "primary_source", "containers", "copies"]
K1_ONLY_COLS = ["resref", "tag", "name", "baseitem_id", "baseitem_label", "baseitem_label_in_k2", "modeltype",
                "itemclass", "model_variation", "texture_var", "body_variation", "cost", "upgrade_count",
                "property_count", "property_types", "icon_resref", "icon_rule", "icon_in_k2", "in_k2_by_tag",
                "k2_tag_resrefs", "in_k2_by_name", "k2_name_resrefs", "model_resref", "model_derived",
                "model_in_k2", "model_loads", "mesh_nodes", "texture_resrefs", "texture_in_k2",
                "textures_all_missing_in_k2", "lightmap_resrefs", "containers"]
DIFF_COLS = ["label", "status", "k1_row_id", "k2_row_id", "k1_label", "k2_label", "k1_defaultmodel",
             "k2_defaultmodel", "k1_itemclass", "k2_itemclass", "k1_modeltype", "k2_modeltype", "k1_name",
             "k2_name"]

# Helper import: reuse load_pykotor() / install_get() from the k1_port UV-mask tool (as npc_equip_scan does).
sys.path.insert(0, str(HELPER_DIR))
from k1_port_uvmask import install_get, load_pykotor  # noqa: E402


class Copy(NamedTuple):
    group: str
    container: str
    resname: str
    resource: object


def parse_int(text: object, default: int = 0) -> int:
    s = str(text).strip()
    if not s:
        return default
    try:
        return int(s, 0)
    except ValueError:
        try:
            return int(s)
        except ValueError:
            return default


def clean(value: object) -> str:
    return str(value).replace("\t", " ").replace("\r", " ").replace("\n", " ")


def yes_no(flag: bool) -> str:
    return "yes" if flag else "no"


def write_tsv(path: Path, header: list[str], rows: list[list[object]]) -> None:
    with path.open("w", encoding="utf-8", newline="") as fh:
        fh.write("\t".join(header) + "\n")
        for row in rows:
            fh.write("\t".join(clean(v) for v in row) + "\n")


def is_within(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
        return True
    except ValueError:
        return False


def popcount(value: int) -> int:
    return bin(value).count("1")


def presence_ext(pk, rtype: object) -> str | None:
    if rtype == pk.ResourceType.MDL:
        return "mdl"
    if rtype == pk.ResourceType.MDX:
        return "mdx"
    if rtype == pk.ResourceType.TPC:
        return "tpc"
    if rtype == pk.ResourceType.TGA:
        return "tga"
    return None


class Game:
    """Everything scanned from one install. Built once by load_game()."""

    def __init__(self, tag: str, inst: object) -> None:
        self.tag = tag
        self.inst = inst
        self.bi_headers: list[str] = []
        self.bi_source = ""
        self.base: dict[int, dict] = {}
        self.base_by_label: dict[str, dict] = {}
        self.base_rows: list[dict] = []
        self.base_dup_labels = 0
        self.base_blank_labels = 0
        self.uti: dict[str, list[Copy]] = collections.defaultdict(list)
        self.uti_copies = 0
        self.uti_copy_fail = 0
        self.uti_unparsed = 0
        self.presence: set[tuple[str, str]] = set()
        self.texpack: set[tuple[str, str]] = set()  # TPC/TGA names in TexturePacks/*.erf (icons only)
        self.items: dict[str, dict] = {}
        self.module_files = 0
        self.symlink_dupes = 0
        self.rims_files = 0
        self.rims_fail = 0
        self.name_resolve_fail = 0
        self.base_name_fail = 0

    def has_icon(self, icon: str) -> bool:
        key = icon.lower()
        return any(k in self.presence or k in self.texpack for k in (("tpc", key), ("tga", key)))


def load_baseitems(pk, ctx, g: Game) -> None:
    bi_res = install_get(pk, g.inst, "baseitems", pk.ResourceType.TwoDA)
    if bi_res is None:
        raise SystemExit(f"{g.tag}: baseitems.2da not found in override/chitin/modules")
    bi = ctx.read_2da(bi_res.data)
    headers = bi.get_headers()
    g.bi_headers = headers
    g.bi_source = str(bi_res.as_file_resource().filepath())
    tt = g.inst.talktable()
    for r in range(bi.get_height()):
        def cell(col: str) -> str:
            return str(bi.get_cell(r, col)) if col in headers else ""

        label = cell("label").strip()
        strref = parse_int(cell("name"), -1)
        name = ""
        if strref >= 0:
            try:
                name = str(tt.string(strref))
            except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
                g.base_name_fail += 1
        mask = parse_int(cell("equipableslots"))
        info = {
            "row_id": r, "label": label, "name_strref": strref if strref >= 0 else "", "name": name,
            "equipableslots": cell("equipableslots"), "slot_names": ctx.slot_names(mask),
            "itemclass": cell("itemclass"), "defaultmodel": cell("defaultmodel"),
            "defaulticon": cell("defaulticon"), "modeltype": cell("modeltype"), "bodyvar": cell("bodyvar"),
            "stats": [cell(c) for c in STAT_COLS if c in headers],
        }
        g.base[r] = info
        g.base_rows.append(info)
        if not label:
            g.base_blank_labels += 1
        elif label.lower() in g.base_by_label:
            g.base_dup_labels += 1
        else:
            g.base_by_label[label.lower()] = info


def scan_containers(pk, ctx, g: Game) -> None:
    inst = g.inst

    def add(group: str, container: str, res: object) -> None:
        name = res.resname()
        rtype = res.restype()
        if rtype == pk.ResourceType.UTI:
            g.uti[name.lower()].append(Copy(group, container, name, res))
            g.uti_copies += 1
        ext = presence_ext(pk, rtype)
        if ext and group != "RIMS":
            g.presence.add((ext, name.lower()))

    for pack in inst.texturepacks_list():
        for res in inst.texturepack_resources(pack):
            ext = presence_ext(pk, res.restype())
            if ext in ("tpc", "tga"):
                g.texpack.add((ext, res.resname().lower()))
    for res in inst.override_resources():
        add("OVERRIDE", "OVERRIDE", res)
    for res in sorted(inst.chitin_resources(), key=lambda x: (str(x.filepath()), x.resname().lower())):
        add("CHITIN", Path(str(res.filepath())).name, res)
    module_dir = str(inst.module_path())
    seen_real: set[str] = set()
    for name in sorted(inst.modules_list(), key=lambda n: (n.lower(), n)):
        real = os.path.realpath(os.path.join(module_dir, name))
        if real in seen_real:
            g.symlink_dupes += 1
            continue
        seen_real.add(real)
        g.module_files += 1
        container = os.path.basename(real)
        for res in inst.module_resources(name):
            add("MODULE", container, res)
    rims_dir = inst.rims_path()
    if rims_dir and Path(rims_dir).is_dir():
        for path in sorted(Path(rims_dir).iterdir(), key=lambda x: x.name.lower()):
            if path.suffix.lower() not in (".rim", ".mod", ".erf"):
                continue
            g.rims_files += 1
            try:
                resources = ctx.Capsule(path).resources()
            except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
                g.rims_fail += 1
                continue
            for res in resources:
                add("RIMS", path.name, res)


def parse_uti(ctx, g: Game, copy: Copy) -> dict:
    root = ctx.read_gff(copy.resource.data()).root
    name = ""
    loc = root.acquire("LocalizedName", None)
    if loc is not None:
        try:
            name = str(g.inst.string(loc))
        except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
            g.name_resolve_fail += 1
    props = root.get_list("PropertiesList") or []
    return {
        "tag": str(root.acquire("Tag", "")),
        "name": name,
        "base_id": parse_int(root.acquire("BaseItem", 0)),
        "model_variation": parse_int(root.acquire("ModelVariation", 0)),
        "texture_var": parse_int(root.acquire("TextureVar", 0)),
        "body_variation": parse_int(root.acquire("BodyVariation", 0)),
        "cost": parse_int(root.acquire("Cost", 0)),
        "upgrades": parse_int(root.acquire("Upgrades", 0)),
        "ptypes": [parse_int(p.acquire("PropertyName", 0)) for p in props],
    }


def derive_model(g: Game, info: dict, variation: int) -> tuple[str, str]:
    """Model rule: candidates itemclass_NNN (when ModelVariation > 0) then defaultmodel.

    rule  = first candidate that exists as an MDL in this game (override/chitin/modules).
    guess = nothing exists; falls back to defaultmodel (or itemclass_NNN when defaultmodel is I_Null).
    none  = no model name can be formed (I_Null defaultmodel and variation 0).
    """
    dm = info["defaultmodel"].strip()
    ic = info["itemclass"].strip()
    has_dm = dm.lower() not in NO_MODEL_NAMES
    variant = f"{ic}_{variation:03d}" if (variation > 0 and ic) else ""
    cands = [c for c in (variant, dm if has_dm else "") if c]
    for cand in cands:
        if ("mdl", cand.lower()) in g.presence:
            return cand, "rule"
    if has_dm:
        return dm, "guess"
    if variant:
        return variant, "guess"
    return "", "none"


def derive_icon(g: Game, info: dict, modeltype: str, model_variation: int, texture_var: int) -> tuple[str, str]:
    """Icon rule (verified against installed textures, not decompiled).

    Candidate = "i" + lowercase itemclass + "_" + 3-digit N, where N = TextureVar for armor (modeltype 1),
    ModelVariation otherwise (e.g. w_Lngswrd + 1 -> iw_lngswrd_001, I_Belt + 10 -> ii_belt_010,
    a_Class4 + TextureVar 7 -> ia_class4_007). The candidate is used when a TPC/TGA of that name exists
    (override/chitin/modules/TexturePacks). Otherwise the baseitems.2da defaulticon is used.
    """
    ic = info["itemclass"].strip().lower()
    n = texture_var if modeltype == "1" else model_variation
    if ic:
        cand = f"i{ic}_{n:03d}"
        if g.has_icon(cand):
            return cand, "itemclass_NNN"
    dflt = info["defaulticon"].strip()
    if dflt:
        return dflt, "defaulticon"
    return "", "none"


def build_items(pk, ctx, g: Game) -> None:
    for key in sorted(g.uti):
        copies = sorted(g.uti[key], key=lambda c: (GROUP_ORDER[c.group], c.container.lower(), c.resname))
        parsed: list[tuple[Copy, dict]] = []
        for c in copies:
            try:
                parsed.append((c, parse_uti(ctx, g, c)))
            except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
                g.uti_copy_fail += 1
        if not parsed:
            g.uti_unparsed += 1
            continue
        primary, p = parsed[0]
        info = g.base.get(p["base_id"])
        if info is None:
            info = {"label": f"BASEITEM_{p['base_id']}", "itemclass": "", "defaultmodel": "",
                    "defaulticon": "", "modeltype": ""}
        model, model_derived = derive_model(g, info, p["model_variation"])
        icon, icon_rule = derive_icon(g, info, info["modeltype"], p["model_variation"], p["texture_var"])
        icon_flag = "" if not icon else yes_no(g.has_icon(icon))
        g.items[key] = {
            "resref": primary.resname, "tag": p["tag"], "name": p["name"],
            "baseitem_id": p["base_id"], "baseitem_label": info["label"], "modeltype": info["modeltype"],
            "itemclass": info["itemclass"], "defaultmodel": info["defaultmodel"],
            "model_variation": p["model_variation"], "texture_var": p["texture_var"],
            "body_variation": p["body_variation"], "cost": p["cost"], "upgrades": p["upgrades"],
            "upgrade_count": popcount(p["upgrades"]), "property_count": len(p["ptypes"]),
            "property_types": ";".join(str(x) for x in p["ptypes"]),
            "icon_resref": icon, "icon_rule": icon_rule, "icon_in_game": icon_flag, "model_resref": model,
            "model_derived": model_derived, "primary_source": f"{primary.group}:{primary.container}",
            "containers": "; ".join(f"{c.group}:{c.container}" for c, _ in parsed), "copies": len(copies),
        }


def mdl_texture_names(root: object) -> tuple[list[str], list[str], int]:
    tex1: set[str] = set()
    tex2: set[str] = set()
    mesh_nodes = 0
    stack = [root]
    while stack:
        node = stack.pop()
        mesh = getattr(node, "mesh", None)
        if mesh is not None:
            mesh_nodes += 1
            for attr, bucket in (("texture_1", tex1), ("texture_2", tex2)):
                tex = str(getattr(mesh, attr, "") or "").strip().lower()
                if tex and tex != "null":
                    bucket.add(tex)
        stack.extend(list(getattr(node, "children", None) or []))
    return sorted(tex1), sorted(tex2), mesh_nodes


def read_model(pk, g1: Game, model: str) -> dict:
    """Read an MDL+MDX from K1 (same lookup order as dump_model in k1_port_tex_preview.py)."""
    rec: dict = {"status": "", "tex1": [], "tex2": [], "mesh_nodes": 0}
    if not model:
        rec["status"] = "no-model"
        return rec
    if ("mdl", model.lower()) not in g1.presence:
        rec["status"] = "no-mdl"
        return rec
    mdl_res = install_get(pk, g1.inst, model, pk.ResourceType.MDL)
    mdx_res = install_get(pk, g1.inst, model, pk.ResourceType.MDX)
    if mdl_res is None:
        rec["status"] = "no-mdl"
        return rec
    if mdx_res is None:
        rec["status"] = "no-mdx"
        return rec
    try:
        mdx = bytes(mdx_res.data)
        mdl = pk.read_mdl(bytes(mdl_res.data), source_ext=mdx, size_ext=len(mdx))
        tex1, tex2, nodes = mdl_texture_names(mdl.root)
    except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
        rec["status"] = "parse-fail"
        return rec
    rec.update(status="yes", tex1=tex1, tex2=tex2, mesh_nodes=nodes)
    return rec


def load_game(pk, ctx, tag: str, game_dir: Path) -> Game:
    g = Game(tag, pk.Installation(game_dir))
    load_baseitems(pk, ctx, g)
    scan_containers(pk, ctx, g)
    build_items(pk, ctx, g)
    return g


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="KotOR 1 item (UTI) and base item inventory vs KotOR 2 (static, read-only).")
    ap.add_argument("--k1", type=Path, default=K1_DIR_DEFAULT, help="KotOR 1 install dir (read-only)")
    ap.add_argument("--k2", type=Path, default=K2_DIR_DEFAULT, help="KotOR 2 install dir (read-only)")
    ap.add_argument("--out", type=Path, default=OUT_DEFAULT, help="output directory")
    args = ap.parse_args(argv)

    k1 = args.k1.expanduser().resolve()
    k2 = args.k2.expanduser().resolve()
    out = args.out.expanduser().resolve()
    for game in (k1, k2):
        if is_within(out, game):
            print(f"refusing to write into the game dir: {out}", file=sys.stderr)
            return 2
    out.mkdir(parents=True, exist_ok=True)

    pk = load_pykotor()
    from pykotor.common.misc import EquipmentSlot
    from pykotor.extract.capsule import Capsule
    from pykotor.resource.formats.gff.gff_auto import read_gff
    from pykotor.resource.formats.twoda.twoda_auto import read_2da

    slot_bits = sorted(((s.value, s.name) for s in EquipmentSlot if s.value), key=lambda t: t[0])

    def slot_names(mask: int) -> str:
        names, covered = [], 0
        for bit, name in slot_bits:
            if mask & bit:
                names.append(name)
                covered |= bit
        rest = mask & ~covered
        if rest:
            names.append(f"UNKNOWN_0x{rest:X}")
        return "|".join(names)

    ctx = SimpleNamespace(read_gff=read_gff, read_2da=read_2da, Capsule=Capsule, slot_names=slot_names)
    g1 = load_game(pk, ctx, "K1", k1)
    g2 = load_game(pk, ctx, "K2", k2)

    # 1. baseitems files and diff
    for g, fname in ((g1, "baseitems_k1.tsv"), (g2, "baseitems_k2.tsv")):
        stat_cols = [c for c in STAT_COLS if c in g.bi_headers]
        rows = []
        for info in g.base_rows:
            rows.append([info["row_id"], info["label"], info["name_strref"], info["name"],
                         info["equipableslots"], info["slot_names"], info["itemclass"], info["defaultmodel"],
                         info["defaulticon"], info["modeltype"], info["bodyvar"], *info["stats"]])
        write_tsv(out / fname, BASE_FIXED_COLS + stat_cols, rows)

    diff_status: collections.Counter = collections.Counter()
    diff_rows: list[list[object]] = []
    for key in sorted(set(g1.base_by_label) | set(g2.base_by_label)):
        a = g1.base_by_label.get(key)
        b = g2.base_by_label.get(key)
        if a and not b:
            status = "K1_ONLY"
        elif b and not a:
            status = "K2_ONLY"
        else:
            diffs = []
            if a["defaultmodel"].lower() != b["defaultmodel"].lower():
                diffs.append("DEFAULTMODEL")
            if a["itemclass"].lower() != b["itemclass"].lower():
                diffs.append("ITEMCLASS")
            status = "SAME" if not diffs else "DIFF_" + "+".join(diffs)
        diff_status[status] += 1
        label = (a or b)["label"]
        diff_rows.append([
            label, status,
            a["row_id"] if a else "", b["row_id"] if b else "",
            a["label"] if a else "", b["label"] if b else "",
            a["defaultmodel"] if a else "", b["defaultmodel"] if b else "",
            a["itemclass"] if a else "", b["itemclass"] if b else "",
            a["modeltype"] if a else "", b["modeltype"] if b else "",
            a["name"] if a else "", b["name"] if b else "",
        ])
    write_tsv(out / "baseitems_diff.tsv", DIFF_COLS, diff_rows)

    # 2. item files (one row per distinct resref)
    for g, fname in ((g1, "items_k1.tsv"), (g2, "items_k2.tsv")):
        rows = [[row[c] for c in ITEM_COLS] for _key, row in sorted(g.items.items())]
        write_tsv(out / fname, ITEM_COLS, rows)

    # 3. K1-only items with K2 look-alikes, model and texture checks
    k2_tag_index: dict[str, list[str]] = collections.defaultdict(list)
    k2_name_index: dict[str, list[str]] = collections.defaultdict(list)
    for row in g2.items.values():
        if row["tag"]:
            k2_tag_index[row["tag"].lower()].append(row["resref"])
        if row["name"]:
            k2_name_index[row["name"].lower()].append(row["resref"])

    model_cache: dict[str, dict] = {}
    k1_only_rows: list[dict] = []
    model_loads: collections.Counter = collections.Counter()
    tex_miss_in_pack = 0
    all_missing_count = 0
    no_texture_count = 0
    label_counter: collections.Counter = collections.Counter()
    tag_hits = name_hits = label_hits = 0
    derived_counter: collections.Counter = collections.Counter()
    for key in sorted(g1.items):
        if key in g2.items:
            continue
        row = g1.items[key]
        tag_k2 = sorted(set(k2_tag_index.get(row["tag"].lower(), []))) if row["tag"] else []
        name_k2 = sorted(set(k2_name_index.get(row["name"].lower(), []))) if row["name"] else []
        label_in_k2 = row["baseitem_label"].lower() in g2.base_by_label
        model = row["model_resref"]
        mkey = model.lower()
        if mkey not in model_cache:
            model_cache[mkey] = read_model(pk, g1, model)
        rec = model_cache[mkey]
        model_loads[rec["status"]] += 1
        model_in_k2 = "" if not model else yes_no(("mdl", mkey) in g2.presence)
        tex_checks = []
        for tex in rec["tex1"]:
            in_k2 = ("tpc", tex) in g2.presence or ("tga", tex) in g2.presence
            tex_checks.append((tex, in_k2))
            if not in_k2 and (("tpc", tex) in g2.texpack or ("tga", tex) in g2.texpack):
                tex_miss_in_pack += 1
        if tex_checks:
            all_missing = yes_no(not any(flag for _t, flag in tex_checks))
            if all_missing == "yes":
                all_missing_count += 1
        else:
            all_missing = ""
            no_texture_count += 1
        k1_only_rows.append({
            "resref": row["resref"], "tag": row["tag"], "name": row["name"],
            "baseitem_id": row["baseitem_id"], "baseitem_label": row["baseitem_label"],
            "baseitem_label_in_k2": yes_no(label_in_k2), "modeltype": row["modeltype"],
            "itemclass": row["itemclass"], "model_variation": row["model_variation"],
            "texture_var": row["texture_var"], "body_variation": row["body_variation"], "cost": row["cost"],
            "upgrade_count": row["upgrade_count"], "property_count": row["property_count"],
            "property_types": row["property_types"], "icon_resref": row["icon_resref"],
            "icon_rule": row["icon_rule"],
            "icon_in_k2": "" if not row["icon_resref"] else yes_no(g2.has_icon(row["icon_resref"])),
            "in_k2_by_tag": yes_no(bool(tag_k2)), "k2_tag_resrefs": ";".join(tag_k2),
            "in_k2_by_name": yes_no(bool(name_k2)), "k2_name_resrefs": ";".join(name_k2),
            "model_resref": model, "model_derived": row["model_derived"], "model_in_k2": model_in_k2,
            "model_loads": rec["status"], "mesh_nodes": rec["mesh_nodes"],
            "texture_resrefs": ";".join(rec["tex1"]),
            "texture_in_k2": ";".join(f"{t}={yes_no(f)}" for t, f in tex_checks),
            "textures_all_missing_in_k2": all_missing,
            "lightmap_resrefs": ";".join(rec["tex2"]), "containers": row["containers"],
        })
        label_counter[row["baseitem_label"]] += 1
        tag_hits += bool(tag_k2)
        name_hits += bool(name_k2)
        label_hits += label_in_k2
        derived_counter[row["model_derived"]] += 1

    write_tsv(out / "items_k1_only.tsv", K1_ONLY_COLS, [[r[c] for c in K1_ONLY_COLS] for r in k1_only_rows])

    # 4. SUMMARY.md
    lines: list[str] = []
    lines.append("# K1 item inventory vs K2 (static)")
    lines.append("")
    lines.append(f"K1 dir: `{k1}` (read-only). K2 dir: `{k2}` (read-only). Output: `{out}`.")
    lines.append("")
    lines.append("## Counts")
    lines.append("")
    lines.append(f"- K1 UTI occurrences (all containers): {g1.uti_copies}; distinct K1 UTI resrefs: {len(g1.items)}")
    lines.append(f"- K2 UTI occurrences (all containers): {g2.uti_copies}; distinct K2 UTI resrefs: {len(g2.items)}")
    lines.append(f"- K1-only UTI resrefs (no UTI with that resref in K2): {len(k1_only_rows)}")
    lines.append(f"- K1-only whose base item label also exists in K2: {label_hits}")
    lines.append(f"- K1-only with a tag match in K2 (non-empty tag): {tag_hits}; name match in K2 (non-empty name): {name_hits}")
    lines.append(f"- K1-only with a model that loads (MDL+MDX parsed): {model_loads['yes']} of {len(k1_only_rows)}; "
                 "status breakdown: " + ", ".join(f"{k or '(empty)'}={v}" for k, v in sorted(model_loads.items())))
    lines.append(f"- K1-only model_derived: " + ", ".join(f"{k}={derived_counter[k]}" for k in ("rule", "guess", "none")))
    lines.append(f"- K1-only with textures and ALL textures missing in K2 (TPC/TGA): {all_missing_count}; "
                 f"K1-only with no mesh texture_1 at all: {no_texture_count}")
    lines.append(f"- K1-only texture entries missing from K2 chitin/override/modules but present in K2 "
                 f"TexturePacks (not counted as missing above): {tex_miss_in_pack}")
    lines.append(f"- K1-only items whose chosen icon is absent in K2 (override/chitin/modules/TexturePacks): "
                 f"{sum(1 for r in k1_only_rows if r['icon_in_k2'] == 'no')}; "
                 "icon_rule counts: " + ", ".join(
                     f"{k}={sum(1 for r in k1_only_rows if r['icon_rule'] == k)}"
                     for k in ("itemclass_NNN", "defaulticon", "none")))
    lines.append("")
    lines.append(f"## K1-only per baseitem label (top {TOP_LABELS})")
    lines.append("")
    for label, n in sorted(label_counter.items(), key=lambda kv: (-kv[1], kv[0]))[:TOP_LABELS]:
        lines.append(f"- {label}: {n}")
    if not label_counter:
        lines.append("- none")
    lines.append("")
    lines.append("## Base item diff (case-insensitive label match)")
    lines.append("")
    for status in sorted(diff_status):
        lines.append(f"- {status}: {diff_status[status]}")
    lines.append("")
    lines.append("## Parse failures and resolution misses (counts only)")
    lines.append("")
    for g in (g1, g2):
        lines.append(f"- {g.tag} UTI copies that failed to parse: {g.uti_copy_fail}; "
                     f"distinct resrefs with no parseable copy: {g.uti_unparsed}")
        lines.append(f"- {g.tag} UTI names that failed to resolve via talk table: {g.name_resolve_fail}; "
                     f"baseitems names that failed: {g.base_name_fail}")
        lines.append(f"- {g.tag} rims files that failed to open: {g.rims_fail} of {g.rims_files}")
        lines.append(f"- {g.tag} blank baseitems labels (excluded from diff): {g.base_blank_labels}; "
                     f"duplicate baseitems labels (first row kept): {g.base_dup_labels}")
    lines.append(f"- K1-only MDL/MDX read failures (parse-fail): {model_loads['parse-fail']}; "
                 f"no-mdx: {model_loads['no-mdx']}; no-mdl: {model_loads['no-mdl']}")
    lines.append("")
    lines.append("## Rules and assumptions")
    lines.append("")
    lines.append("- Containers: override, chitin bifs, every module file in modules/ (.rim, _s.rim, .mod; "
                 f"K1 {g1.module_files} files, K2 {g2.module_files} files; symlink case-variants collapsed: "
                 f"K1 {g1.symlink_dupes}, K2 {g2.symlink_dupes}), and every .rim/.mod/.erf in rims/ "
                 f"(K1 {g1.rims_files}, K2 {g2.rims_files}).")
    lines.append("- One row per distinct resref (case-insensitive). Primary copy: OVERRIDE > CHITIN > MODULE > RIMS, "
                 "then container name. Every copy is listed in `containers`; `copies` counts them.")
    lines.append("- Matching is case-insensitive for resref, tag, name and baseitems label. Empty tags and "
                 "empty names never match.")
    lines.append("- Names: UTI LocalizedName and baseitems `name` strref, resolved through each install's own "
                 "talk table (dialog.tlk, default/masculine). Unresolved = empty string.")
    lines.append("- Baseitem stats: no `baseitemstats` 2DA exists in either install; `stat columns` means the "
                 "baseitems.2da stat columns (" + ", ".join(STAT_COLS) + "), each written only when present.")
    lines.append("- Icon rule (icon_rule): candidate = `i` + lowercase itemclass + `_` + 3-digit N, N = TextureVar for "
                 "armor (modeltype 1), ModelVariation otherwise. Used when that TPC/TGA exists (override, chitin, "
                 "modules, TexturePacks). Otherwise the baseitems.2da `defaulticon` is used (icon_rule=defaulticon). "
                 "Checked on the installed files: the candidate exists for most rows in both games (see counts "
                 "in the K1/K2 files); this is an empirical rule, not decompiled engine code. "
                 "textures.2da is absent in both games.")
    lines.append("- Model texture checks (texture_in_k2) use TPC/TGA from override/chitin/modules only, as specified. "
                 "TexturePacks are counted separately above.")
    lines.append("- Model rule (model_derived): candidate A = `itemclass_NNN` when ModelVariation NNN > 0 "
                 "(3-digit zero padded); candidate B = baseitems `defaultmodel` unless it is I_Null. The first "
                 "candidate that exists as an MDL in that game (override/chitin/modules) gives model_derived=rule. "
                 "If none exists: model_derived=guess and model_resref = defaultmodel (or A when defaultmodel is "
                 "I_Null); if no name can be formed: model_derived=none. Example confirmations in K1 chitin: "
                 "w_Qtrstaff_001 (weapon), i_belt_001 (belt, itemclass I_Belt), a_jedirobe_001 (armor, itemclass "
                 "a_JediRobe). Armor body-part models (modeltype 1) are not fully derivable from these fields.")
    lines.append("- Textures: MDL mesh `texture_1` of every mesh node (lowercased, `null` skipped) is the texture "
                 "list; `texture_2` (lightmap) is a separate column and is not checked against K2.")
    lines.append("- K2 presence for model and texture checks uses override + chitin + modules only (rims excluded). "
                 "TPC and TGA both count as a texture.")
    lines.append("- K1 MDL reads reuse the install_get / read_mdl(source_ext=MDX) call from "
                 "patch_manager_mods/k1_port/k1_port_tex_preview.py dump_model.")
    lines.append("- `model_loads`: yes = MDL+MDX parsed; no-model = no model name; no-mdl / no-mdx = missing; "
                 "parse-fail = read_mdl raised.")
    lines.append("- Output is sorted by resref (lowercase) and label, so reruns on the same installs are identical.")
    (out / "SUMMARY.md").write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"baseitems_k1.tsv rows={len(g1.base_rows)} baseitems_k2.tsv rows={len(g2.base_rows)} "
          f"baseitems_diff.tsv rows={len(diff_rows)}")
    print(f"items_k1.tsv rows={len(g1.items)} items_k2.tsv rows={len(g2.items)}")
    print(f"items_k1_only.tsv rows={len(k1_only_rows)}")
    print(f"SUMMARY.md written to {out / 'SUMMARY.md'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
