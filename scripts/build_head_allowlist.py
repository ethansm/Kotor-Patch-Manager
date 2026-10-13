#!/usr/bin/env python3
"""Build the head-slot allowlist for the npc_diversity patch (game-dependent, deterministic).

Writes one JSON document (patch_manager_mods/npc_diversity_src/head_items_source.json):
  baseitems    head-slot baseitems.2da rows (equipableslots bit 0x1): race rule, gender mask, required feats
  items        K2 UTI templates whose base item is a head-slot row: model, property restrictions
  appearances  appearance rows with >= 1 head-item template: observed races, genders, head items

Inputs:
  K2 install, read-only: baseitems.2da (override > chitin); primary UTI copy per resref, found with the
    container scan of scripts/k1_item_inventory.py (imported, not modified).
  .claude/research/k1_items/items_k2.tsv               resref, baseitem_id, model_resref
  .claude/research/npc_equipment/head_wearability.tsv  appearance rows, label, modeltype, has_goggle_hook
  .claude/research/npc_equipment/utc_equip.tsv         per-UTC race, gender, equip, head_item_resrefs

Run with the patched pykotor venv (the game is read here, never written):
  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I scripts/build_head_allowlist.py

Everything above load_game_modules() is stdlib-only, so the tests run without the venv.
"""

from __future__ import annotations

import argparse
import collections
import csv
import json
import sys
from pathlib import Path
from types import SimpleNamespace

SCRIPTS_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPTS_DIR.parent
K2_DIR_DEFAULT = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
HELPER_DIR = REPO_ROOT / "patch_manager_mods" / "k1_port"
ITEMS_TSV = REPO_ROOT / ".claude/research/k1_items/items_k2.tsv"
HEAD_WEAR_TSV = REPO_ROOT / ".claude/research/npc_equipment/head_wearability.tsv"
UTC_EQUIP_TSV = REPO_ROOT / ".claude/research/npc_equipment/utc_equip.tsv"
OUT_DEFAULT = REPO_ROOT / "patch_manager_mods/npc_diversity_src/head_items_source.json"

SCHEMA = 1
HEAD_SLOT_BIT = 0x1  # EquipmentSlot.HEAD; Equip_ItemList struct_id 1 is the head slot
HEAD_SLOT_ID = 1
DROID_RACE_RULE = 2  # droidorhuman 2: creature Race must equal 5
GENDER_MASK = 0  # no nonzero gender mask is known (baseitems.genderspecific is 0 on every row)
REQFEAT_COLS = ("reqfeat0", "reqfeat1", "reqfeat2", "reqfeat3", "reqfeat4")
# Property types that restrict who may equip an item: alignment, class, race, gender, feat/skill, 0x3c,
# tag, min stat.
RESTRICTION_TYPES = frozenset({0x2B, 0x2C, 0x2D, 0x3D, 0x39, 0x3C, 0x3E, 0x45})


# ----------------------------------------------------------------- pure functions (no game access)

def int_cell(text: object) -> int:
    """Strict integer cell: 0x-hex or decimal. Raises ValueError for anything else, including blank."""
    s = str(text).strip()
    try:
        return int(s, 0)
    except ValueError:
        return int(s, 10)


def gff_int(value: object) -> int:
    return value if isinstance(value, int) else int_cell(value)


def race_rule(cell: object) -> int:
    """droidorhuman cell -> race rule. A blank cell means no rule (0)."""
    return 0 if not str(cell).strip() else int_cell(cell)


def req_feats(cells: dict) -> list[int]:
    """reqfeat0..4 cells that are not blank, in column order. Every non-blank cell must be an integer."""
    return [int_cell(cells[c]) for c in REQFEAT_COLS if str(cells.get(c, "")).strip()]


def restrictions(props: list[tuple[int, int]]) -> list[dict]:
    """(PropertyName, Subtype) pairs filtered to RESTRICTION_TYPES, deduplicated and sorted."""
    keep = sorted({(gff_int(t), gff_int(s)) for t, s in props if gff_int(t) in RESTRICTION_TYPES})
    return [{"type": t, "subtype": s} for t, s in keep]


def uti_props(root: object) -> tuple[int, list[tuple[int, int]]]:
    """BaseItem and PropertiesList (PropertyName, Subtype) of one UTI GFF root.

    Duck-typed (acquire / get_list) so tests can pass a fake root.
    """
    props = root.get_list("PropertiesList") or []
    pairs = [(gff_int(p.acquire("PropertyName", 0)), gff_int(p.acquire("Subtype", 0))) for p in props]
    return gff_int(root.acquire("BaseItem", 0)), pairs


def head_baseitems(rows: dict[int, dict]) -> dict[int, dict]:
    """Rows of baseitems.2da (row index -> column -> text) whose equipableslots has the HEAD bit."""
    out: dict[int, dict] = {}
    for rid in sorted(rows):
        cells = rows[rid]
        mask_text = str(cells.get("equipableslots", "")).strip()
        mask = int_cell(mask_text) if mask_text else 0
        if not mask & HEAD_SLOT_BIT:
            continue
        rule = race_rule(cells.get("droidorhuman", ""))
        out[rid] = {
            "label": str(cells.get("label", "")),
            "equipableslots": f"0x{mask:05X}",
            "race_rule": rule,
            "gender_mask": GENDER_MASK,
            "req_feats": req_feats(cells),
            "droid": rule == DROID_RACE_RULE,
        }
    return out


def items_from_rows(item_rows: list[dict], head_ids: set[int],
                    uti: dict[str, tuple[int, list[tuple[int, int]]]]) -> tuple[dict, list[str]]:
    """K2 items whose baseitem is a head row. uti maps resref (lower) -> (BaseItem, property pairs)."""
    items: dict[str, dict] = {}
    problems: list[str] = []
    for r in item_rows:
        bid = int_cell(r["baseitem_id"])
        if bid not in head_ids:
            continue
        key = r["resref"].lower()
        if key in items:
            problems.append(f"{key}: duplicate resref in items TSV")
            continue
        if key not in uti:
            problems.append(f"{key}: no parseable primary UTI copy")
            continue
        base, pairs = uti[key]
        if base != bid:
            problems.append(f"{key}: primary UTI BaseItem {base} != items TSV baseitem_id {bid}")
            continue
        items[key] = {"baseitem": bid, "model": r["model_resref"].lower(), "restrictions": restrictions(pairs)}
    return items, problems


def aggregate_appearances(wear_rows: list[dict], utc_rows: list[dict]) -> dict[str, dict]:
    """One entry per head_wearability row with n_utc_with_head_item > 0, aggregated from utc_equip rows.

    n_utc counts distinct template resrefs, as head_wearability does. Raises ValueError when the
    recomputed counts disagree with head_wearability.
    """
    acc: dict[str, dict] = collections.defaultdict(lambda: {
        "resrefs": set(), "head_resrefs": set(), "races": set(), "genders": set(), "head_items": set()})
    for u in utc_rows:
        a = acc[str(int_cell(u["appearance_type"]))]
        resref = u["resref"].lower()
        a["resrefs"].add(resref)
        if u["has_head_item"].strip() == "1":
            a["head_resrefs"].add(resref)
        a["races"].add(int_cell(u["race"]))
        a["genders"].add(int_cell(u["gender"]))
        for entry in u["equip"].split(";"):
            slot, _, item = entry.partition(":")
            if item.strip() and slot.strip() and int_cell(slot) == HEAD_SLOT_ID:
                a["head_items"].add(item.strip().lower())
        for item in u["head_item_resrefs"].split(";"):
            if item.strip():
                a["head_items"].add(item.strip().lower())

    out: dict[str, dict] = {}
    for w in wear_rows:
        n_head = int_cell(w["n_utc_with_head_item"])
        if n_head <= 0:
            continue
        key = str(int_cell(w["row"]))
        if key not in acc:
            raise ValueError(f"appearance row {key}: head_wearability has head items but utc_equip has no rows")
        a = acc[key]
        if len(a["resrefs"]) != int_cell(w["n_utc"]) or len(a["head_resrefs"]) != n_head:
            raise ValueError(
                f"appearance row {key}: recomputed n_utc={len(a['resrefs'])} "
                f"n_head={len(a['head_resrefs'])} vs head_wearability n_utc={w['n_utc']} "
                f"n_utc_with_head_item={n_head}")
        out[key] = {
            "label": w["label"],
            "modeltype": w["modeltype"],
            "has_goggle_hook": w["has_goggle_hook"],
            "races": sorted(a["races"]),
            "genders": sorted(a["genders"]),
            "n_utc": len(a["resrefs"]),
            "head_items": sorted(a["head_items"]),
        }
    return out


def dump_json(doc: dict) -> str:
    """Deterministic text: sorted keys, fixed indent, no timestamps."""
    return json.dumps(doc, indent=2, sort_keys=True, ensure_ascii=True) + "\n"


def read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open(encoding="utf-8", newline="") as fh:
        reader = csv.reader(fh, delimiter="\t", quoting=csv.QUOTE_NONE)
        header = next(reader)
        rows: list[dict[str, str]] = []
        for line_no, cells in enumerate(reader, start=2):
            if not cells:
                continue
            if len(cells) != len(header):
                raise ValueError(f"{path}:{line_no}: {len(cells)} cells, header has {len(header)}")
            rows.append(dict(zip(header, cells)))
    return rows


# ----------------------------------------------------------------- game access (patched pykotor venv only)

def load_game_modules() -> SimpleNamespace:
    """Import the pykotor helpers. Lazy, so the pure functions above stay game-free."""
    for p in (str(HELPER_DIR), str(SCRIPTS_DIR)):
        if p not in sys.path:
            sys.path.insert(0, p)
    from k1_port_uvmask import install_get, load_pykotor
    import k1_item_inventory as K1I
    from pykotor.extract.capsule import Capsule
    from pykotor.resource.formats.gff.gff_auto import read_gff
    from pykotor.resource.formats.twoda.twoda_auto import read_2da
    return SimpleNamespace(pk=load_pykotor(), install_get=install_get, K1I=K1I, Capsule=Capsule,
                           read_gff=read_gff, read_2da=read_2da)


def read_k2_baseitems(gm: SimpleNamespace, inst: object) -> dict[int, dict[str, str]]:
    res = gm.install_get(gm.pk, inst, "baseitems", gm.pk.ResourceType.TwoDA)
    if res is None:
        raise SystemExit("K2: baseitems.2da not found in override/chitin")
    bi = gm.read_2da(res.data)
    headers = bi.get_headers()
    return {r: {c: str(bi.get_cell(r, c)) for c in headers} for r in range(bi.get_height())}


def read_primary_uti(gm: SimpleNamespace, inst: object, wanted: set[str]) -> dict[str, tuple[int, list]]:
    """Primary copy per resref, chosen as k1_item_inventory.build_items does (first copy that parses)."""
    K1I = gm.K1I
    ctx = SimpleNamespace(read_gff=gm.read_gff, read_2da=gm.read_2da, Capsule=gm.Capsule,
                          slot_names=lambda mask: "")
    g = K1I.Game("K2", inst)
    K1I.scan_containers(gm.pk, ctx, g)
    out: dict[str, tuple[int, list]] = {}
    for key in sorted(wanted):
        copies = sorted(g.uti.get(key, []),
                        key=lambda c: (K1I.GROUP_ORDER[c.group], c.container.lower(), c.resname))
        for c in copies:
            try:
                out[key] = uti_props(gm.read_gff(c.resource.data()).root)
            except Exception:  # noqa: BLE001 - a copy that fails to parse is skipped, as in k1_item_inventory
                continue
            break
    return out


# ----------------------------------------------------------------- CLI

def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Build the npc_diversity head-slot allowlist (KotOR 2, read-only).")
    ap.add_argument("--k2", type=Path, default=K2_DIR_DEFAULT, help="KotOR 2 install dir (read-only)")
    ap.add_argument("--items", type=Path, default=ITEMS_TSV, help="items_k2.tsv")
    ap.add_argument("--head-wear", type=Path, default=HEAD_WEAR_TSV, help="head_wearability.tsv")
    ap.add_argument("--utc-equip", type=Path, default=UTC_EQUIP_TSV, help="utc_equip.tsv")
    ap.add_argument("--out", type=Path, default=OUT_DEFAULT, help="output JSON path")
    args = ap.parse_args(argv)

    k2 = args.k2.expanduser().resolve()
    out = args.out.expanduser().resolve()
    if out == k2 or k2 in out.parents:
        print(f"refusing to write into the game dir: {out}", file=sys.stderr)
        return 2

    item_rows = read_tsv(args.items.expanduser().resolve())
    wear_rows = read_tsv(args.head_wear.expanduser().resolve())
    utc_rows = read_tsv(args.utc_equip.expanduser().resolve())

    gm = load_game_modules()
    inst = gm.pk.Installation(k2)
    bi_rows = read_k2_baseitems(gm, inst)
    heads = head_baseitems(bi_rows)
    head_ids = set(heads)
    wanted = {r["resref"].lower() for r in item_rows if int_cell(r["baseitem_id"]) in head_ids}
    uti = read_primary_uti(gm, inst, wanted)
    items, problems = items_from_rows(item_rows, head_ids, uti)
    if problems:
        for p in problems:
            print(f"PROBLEM {p}", file=sys.stderr)
        print(f"{len(problems)} problem(s); nothing written", file=sys.stderr)
        return 1
    apps = aggregate_appearances(wear_rows, utc_rows)

    doc = {
        "schema": SCHEMA,
        "baseitems": {str(k): v for k, v in heads.items()},
        "items": items,
        "appearances": apps,
    }
    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("w", encoding="utf-8", newline="\n") as fh:
        fh.write(dump_json(doc))

    n_droid = sum(1 for v in heads.values() if v["droid"])
    n_restricted = sum(1 for v in items.values() if v["restrictions"])
    n_not_6 = sum(1 for v in apps.values() if v["races"] != [6])
    gender_set = [k for k in heads if bi_rows[k].get("genderspecific", "").strip() not in ("", "0")]
    print(f"head-slot baseitem rows: {len(heads)} (ids {','.join(str(k) for k in sorted(heads))}; "
          f"droid={n_droid})")
    print(f"head-slot items: {len(items)} (with property restrictions: {n_restricted})")
    print(f"appearances: {len(apps)} (races != [6]: {n_not_6})")
    print(f"head rows with nonzero genderspecific: {len(gender_set)}")
    print(f"wrote {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
