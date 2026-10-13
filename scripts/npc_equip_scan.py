#!/usr/bin/env python3
"""Static scan: which creature templates (UTC) spawn with equipped items, and which base item types are headgear.

Read-only over the game directory. Writes four files into --out:
  baseitems_equip.tsv  one row per baseitems.2da row (is_headgear = HEAD bit set)
  appearance_rows.tsv  one row per appearance.2da row (head/helmet columns as named in the file)
  utc_equip.tsv        one row per UTC occurrence (module container, override, chitin)
  SUMMARY.md           counts, top headgear types, appearance-label tables, notes

Run with the patched pykotor venv:
  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I scripts/npc_equip_scan.py --out .claude/research/npc_equipment

Equipment slot bits come from pykotor.common.misc.EquipmentSlot (the UTC Equip_ItemList struct_id
is the slot bit, field EquippedRes; baseitems.2da equipableslots uses the same bits):
  HEAD=0x1 ARMOR=0x2 GAUNTLET=0x8 RIGHT_HAND=0x10 LEFT_HAND=0x20 RIGHT_ARM=0x80 LEFT_ARM=0x100
  IMPLANT=0x200 BELT=0x400 CLAW1=0x4000 CLAW2=0x8000 CLAW3=0x10000 HIDE=0x20000
  TSL only: RIGHT_HAND_2=0x40000 LEFT_HAND_2=0x80000
"""

from __future__ import annotations

import argparse
import collections
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
GAME_DIR_DEFAULT = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OUT_DEFAULT = REPO_ROOT / ".claude/research/npc_equipment"
HELPER_DIR = REPO_ROOT / "patch_manager_mods" / "k1_port"
MOD_SUFFIX_RE = re.compile(r"(_s|_dlg)?\.(mod|rim|erf)$")
APPEARANCE_HEAD_COLS = [
    "normalhead", "backuphead", "headtexve", "headtexe", "headtexg", "headtexvg",
    "helmet_scale_m", "helmet_scale_f", "headbone", "headtrack", "head_arc_h", "head_arc_v",
]
BASEITEM_COLS = ["equipableslots", "itemclass", "defaultmodel", "modeltype", "partenvmap",
                 "bodyvar", "itemtype"]
LOW_COUNT_FLOOR = 5  # appearance labels with >= this many templates and 0 equipped items are listed

# Helper import: reuse load_pykotor() / install_get() from the k1_port UV-mask tool.
sys.path.insert(0, str(HELPER_DIR))
from k1_port_uvmask import install_get, load_pykotor  # noqa: E402


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


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Scan UTC equipment and headgear base items (static, read-only).")
    ap.add_argument("--game", type=Path, default=GAME_DIR_DEFAULT, help="KOTOR II install dir (read-only)")
    ap.add_argument("--out", type=Path, default=OUT_DEFAULT, help="output directory")
    args = ap.parse_args(argv)

    game = args.game.expanduser().resolve()
    out = args.out.expanduser().resolve()
    if is_within(out, game):
        print(f"refusing to write into the game dir: {out}", file=sys.stderr)
        return 2
    out.mkdir(parents=True, exist_ok=True)

    pk = load_pykotor()
    from pykotor.common.misc import EquipmentSlot
    from pykotor.resource.formats.gff.gff_auto import read_gff
    from pykotor.resource.formats.twoda.twoda_auto import read_2da

    slot_bits = sorted(((s.value, s.name) for s in EquipmentSlot if s.value), key=lambda t: t[0])
    head_bit = EquipmentSlot.HEAD.value

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

    inst = pk.Installation(game)

    # 1. baseitems.2da and appearance.2da (override wins over chitin, as the game loads them)
    bi_res = install_get(pk, inst, "baseitems", pk.ResourceType.TwoDA)
    ap_res = install_get(pk, inst, "appearance", pk.ResourceType.TwoDA)
    if bi_res is None or ap_res is None:
        print("baseitems.2da or appearance.2da not found in override/chitin", file=sys.stderr)
        return 1
    bi_path = bi_res.as_file_resource().filepath()
    ap_path = ap_res.as_file_resource().filepath()
    bi = read_2da(bi_res.data)
    ap_t = read_2da(ap_res.data)

    bi_headers = bi.get_headers()
    base_info: dict[int, dict] = {}
    base_rows: list[list[object]] = []
    for r in range(bi.get_height()):
        row_id = bi.get_label(r)
        mask = parse_int(bi.get_cell(r, "equipableslots")) if "equipableslots" in bi_headers else 0
        is_head = int(bool(mask & head_bit))
        label = bi.get_cell(r, "label") if "label" in bi_headers else ""
        cols = {c: (bi.get_cell(r, c) if c in bi_headers else "") for c in BASEITEM_COLS}
        base_info[r] = {"label": label, "mask": mask, "is_headgear": is_head, "itemclass": cols["itemclass"]}
        base_rows.append([
            row_id, label, cols["equipableslots"], slot_names(mask), is_head, cols["itemclass"],
            cols["defaultmodel"], cols["modeltype"], cols["partenvmap"], cols["bodyvar"], cols["itemtype"],
            bi.get_cell(r, "name") if "name" in bi_headers else "",
        ])
    write_tsv(
        out / "baseitems_equip.tsv",
        ["row_id", "label", "equipableslots", "slot_names", "is_headgear", "itemclass", "defaultmodel",
         "modeltype", "partenvmap", "bodyvar", "itemtype", "name_strref"],
        base_rows,
    )

    app_headers = ap_t.get_headers()
    app_label: dict[int, str] = {}
    app_rows: list[list[object]] = []
    for r in range(ap_t.get_height()):
        row_id = ap_t.get_label(r)
        label = ap_t.get_cell(r, "label") if "label" in app_headers else ""
        app_label[r] = label if label else f"row{r}"
        race = ap_t.get_cell(r, "race") if "race" in app_headers else ""
        modeltype = ap_t.get_cell(r, "modeltype") if "modeltype" in app_headers else ""
        head_vals = [(ap_t.get_cell(r, c) if c in app_headers else "") for c in APPEARANCE_HEAD_COLS]
        app_rows.append([row_id, label, app_label[r], race, modeltype, *head_vals])
    write_tsv(
        out / "appearance_rows.tsv",
        ["row", "label", "appearance_label", "race", "modeltype", *APPEARANCE_HEAD_COLS],
        app_rows,
    )
    missing_head_cols = [c for c in APPEARANCE_HEAD_COLS if c not in app_headers]
    missing_bi_cols = [c for c in BASEITEM_COLS if c not in bi_headers]

    # 2. Collect containers: module files (symlink case-variants collapsed by realpath), override, chitin.
    module_dir = str(inst.module_path())
    seen_real: set[str] = set()
    module_utcs: list[tuple[str, str, object]] = []
    module_utis: dict[str, dict[str, object]] = collections.defaultdict(dict)
    module_files = 0
    symlink_dupes = 0
    for name in sorted(inst.modules_list(), key=lambda n: (n.lower(), n)):
        real = os.path.realpath(os.path.join(module_dir, name))
        if real in seen_real:
            symlink_dupes += 1
            continue
        seen_real.add(real)
        module_files += 1
        container = os.path.basename(real)
        stem = MOD_SUFFIX_RE.sub("", container.lower())
        for r in inst.module_resources(name):
            rt = r.restype()
            if rt == pk.ResourceType.UTC:
                module_utcs.append((stem, container, r))
            elif rt == pk.ResourceType.UTI:
                module_utis[stem].setdefault(r.resname().lower(), r)

    def index_pool(resources: list) -> tuple[list[tuple[str, str, object]], dict[str, object]]:
        utcs, utis = [], {}
        for r in sorted(resources, key=lambda x: str(x.filepath())):
            rt = r.restype()
            container = Path(str(r.filepath())).name
            if rt == pk.ResourceType.UTC:
                utcs.append((container, r))
            elif rt == pk.ResourceType.UTI:
                utis.setdefault(r.resname().lower(), r)
        return utcs, utis

    ov_utcs, ov_utis = index_pool(inst.override_resources())
    ch_utcs, ch_utis = index_pool(inst.chitin_resources())

    all_utcs: list[tuple[str, str, object]] = list(module_utcs)
    all_utcs += [("OVERRIDE", container, r) for container, r in ov_utcs]
    all_utcs += [("CHITIN", container, r) for container, r in ch_utcs]

    uti_cache: dict[int, int | None] = {}

    def uti_base_item(r: object) -> int | None:
        key = id(r)
        if key not in uti_cache:
            try:
                root = read_gff(r.data()).root
                uti_cache[key] = parse_int(root.acquire("BaseItem", 0))
            except Exception:  # noqa: BLE001 - count-only failure, reported in SUMMARY
                uti_cache[key] = None
                uti_parse_fail[0] += 1
        return uti_cache[key]

    uti_parse_fail = [0]

    any_module_utis: dict[str, object] = {}
    for stem_key in sorted(module_utis):
        for name_key, uti_res in module_utis[stem_key].items():
            any_module_utis.setdefault(name_key, uti_res)

    def find_uti(stem: str, name_lower: str):
        pools = (
            ("MODULE", module_utis.get(stem, {})),
            ("OVERRIDE", ov_utis),
            ("CHITIN", ch_utis),
            ("ANY_MODULE", any_module_utis),
        )
        for src, pool in pools:
            if name_lower in pool:
                return pool[name_lower], src
        return None, "UNRESOLVED"

    # 3. Parse every UTC.
    utc_rows: list[dict] = []
    utc_parse_fail = 0
    blank_equip_entries = 0
    unknown_slot_entries = 0
    known_slot_ids = {s.value for s in EquipmentSlot}
    resolve_src: collections.Counter = collections.Counter()
    for stem, container, r in all_utcs:
        try:
            root = read_gff(r.data()).root
            appearance = parse_int(root.acquire("Appearance_Type", 0))
            race = parse_int(root.acquire("Race", 0))
            tag = str(root.acquire("Tag", ""))
            equip_list = root.get_list("Equip_ItemList") or []
            entries = []
            for s in equip_list:
                slot = int(s.struct_id)
                item = str(s.acquire("EquippedRes", "")).strip()
                if not item:
                    blank_equip_entries += 1
                    continue
                if slot not in known_slot_ids:
                    unknown_slot_entries += 1
                uti, src = find_uti(stem, item.lower())
                resolve_src[src] += 1
                base_id = uti_base_item(uti) if uti is not None else None
                if base_id is None:
                    base_label, item_head = "UNRESOLVED", False
                else:
                    info = base_info.get(base_id)
                    base_label = info["label"] if info else f"BASEITEM_{base_id}"
                    item_head = bool(info and info["is_headgear"])
                entries.append((slot, item, base_label, item_head))
        except Exception:  # noqa: BLE001 - count-only failure, reported in SUMMARY
            utc_parse_fail += 1
            continue

        head_via = set()
        for slot, _item, _label, item_head in entries:
            if slot == head_bit:
                head_via.add("slot")
            if item_head:
                head_via.add("baseitem")
        utc_rows.append({
            "module": stem,
            "container": container,
            "resref": r.resname(),
            "tag": tag,
            "appearance": appearance,
            "appearance_label": app_label.get(appearance, f"UNKNOWN_row{appearance}"),
            "race": race,
            "entries": entries,
            "has_head": bool(head_via),
            "head_via": "+".join(sorted(head_via)),
        })

    utc_rows.sort(key=lambda d: (d["module"], d["container"], d["resref"].lower(), d["tag"]))

    equip_rows: list[list[object]] = []
    head_item_counter: collections.Counter = collections.Counter()
    head_item_utc: collections.Counter = collections.Counter()
    head_slot_unresolved = 0
    slot_base_counter: collections.Counter = collections.Counter()
    for d in utc_rows:
        entries = d["entries"]
        head_refs = sorted({item for slot, item, _l, ih in entries if slot == head_bit or ih})
        for slot, item, label, item_head in entries:
            if item_head:
                head_item_counter[label] += 1
            if slot == head_bit and label == "UNRESOLVED":
                head_slot_unresolved += 1
            slot_base_counter[(slot, label)] += 1
        for label in sorted({label for _s, _i, label, ih in entries if ih}):
            head_item_utc[label] += 1
        equip_rows.append([
            d["module"], d["container"], d["resref"], d["tag"], d["appearance"], d["appearance_label"],
            d["race"], len(entries),
            int(d["has_head"]), d["head_via"],
            ";".join(f"{slot}:{item}" for slot, item, _l, _h in entries),
            ";".join(f"{slot}:{label}" for slot, _i, label, _h in entries),
            ";".join(head_refs),
        ])
    write_tsv(
        out / "utc_equip.tsv",
        ["module", "container", "resref", "tag", "appearance_type", "appearance_label", "race",
         "equip_count", "has_head_item", "head_via", "equip", "equip_baseitems", "head_item_resrefs"],
        equip_rows,
    )

    # 4. SUMMARY.md
    total = len(utc_rows)
    with_any = sum(1 for d in utc_rows if d["entries"])
    with_head = sum(1 for d in utc_rows if d["has_head"])
    uniq_total = len({d["resref"].lower() for d in utc_rows})
    uniq_with_head = len({d["resref"].lower() for d in utc_rows if d["has_head"]})
    per_app_total: collections.Counter = collections.Counter()
    per_app_equipped: collections.Counter = collections.Counter()
    per_app_head: collections.Counter = collections.Counter()
    for d in utc_rows:
        per_app_total[d["appearance_label"]] += 1
        if d["entries"]:
            per_app_equipped[d["appearance_label"]] += 1
        if d["has_head"]:
            per_app_head[d["appearance_label"]] += 1
    headgear_types = [(base_info[r]["label"], r) for r in sorted(base_info) if base_info[r]["is_headgear"]]

    lines: list[str] = []
    lines.append("# NPC equipment scan (static)")
    lines.append("")
    lines.append(f"Game dir: `{game}` (read-only). Module containers scanned: {module_files} "
                 f"(symlink case-variants collapsed: {symlink_dupes}). Override UTC: {len(ov_utcs)}, "
                 f"chitin UTC: {len(ch_utcs)}.")
    lines.append("")
    lines.append("## Counts")
    lines.append("")
    lines.append(f"- UTC total (occurrences): {total} (unique resref: {uniq_total})")
    lines.append(f"- UTC with any equipped item: {with_any}")
    lines.append(f"- UTC with a head item: {with_head} (unique resref: {uniq_with_head})")
    lines.append(f"- Equipped entries with a headgear base item: {sum(head_item_counter.values())}")
    lines.append(f"- UTC that failed to parse: {utc_parse_fail}")
    lines.append(f"- UTI that failed to parse (base item unknown): {uti_parse_fail[0]}")
    lines.append(f"- Equipped entries that are HEAD slot with unresolved UTI: {head_slot_unresolved}")
    lines.append(f"- Blank Equip_ItemList entries skipped: {blank_equip_entries}")
    lines.append(f"- Equipped entries by UTI source: " + ", ".join(
        f"{src}={resolve_src[src]}" for src in ("MODULE", "OVERRIDE", "CHITIN", "ANY_MODULE", "UNRESOLVED")))
    lines.append(f"- Equipped entries whose slot id is outside the EquipmentSlot values (0 = INVALID is inside): "
                 f"{unknown_slot_entries}")
    lines.append("")
    lines.append("## Slot id vs resolved base item (bit check; top 3 labels per slot)")
    lines.append("")
    by_slot: dict[int, list[tuple[str, int]]] = collections.defaultdict(list)
    for (slot, label), n in slot_base_counter.items():
        by_slot[slot].append((label, n))
    for slot in sorted(by_slot):
        total_slot = sum(n for _l, n in by_slot[slot])
        top_labels = sorted(by_slot[slot], key=lambda kv: (-kv[1], kv[0]))[:3]
        shown = ", ".join(f"{label} {n}" for label, n in top_labels)
        lines.append(f"- slot {slot} ({slot_names(slot) or 'not a slot bit'}): {total_slot} entries; {shown}")
    lines.append("")
    lines.append("## Top 10 headgear base items (equipped entries; UTC rows in brackets)")
    lines.append("")
    top = sorted(head_item_counter.items(), key=lambda kv: (-kv[1], kv[0]))[:10]
    if top:
        for label, n in top:
            lines.append(f"- {label}: {n} [{head_item_utc[label]}]")
    else:
        lines.append("- none")
    lines.append("")
    lines.append("## Appearance labels with >=1 head item (head UTC rows / total UTC rows)")
    lines.append("")
    head_apps = sorted(per_app_head.items(), key=lambda kv: (-kv[1], kv[0]))
    if head_apps:
        for label, n in head_apps:
            lines.append(f"- {label}: {n} / {per_app_total[label]}")
    else:
        lines.append("- none")
    lines.append("")
    lines.append(f"## Appearance labels with 0 equipped items across >={LOW_COUNT_FLOOR} UTC rows "
                 "(label only, no conclusion)")
    lines.append("")
    zero_apps = sorted(
        ((label, n) for label, n in per_app_total.items() if n >= LOW_COUNT_FLOOR and per_app_equipped[label] == 0),
        key=lambda kv: (-kv[1], kv[0]),
    )
    if zero_apps:
        for label, n in zero_apps:
            lines.append(f"- {label}: {n} UTC rows")
    else:
        lines.append("- none")
    lines.append("")
    lines.append("## Headgear base item types (baseitems.2da rows with HEAD bit set)")
    lines.append("")
    lines.append(f"Count: {len(headgear_types)}")
    lines.append("")
    for label, r in headgear_types:
        lines.append(f"- {r}: {label} (equipableslots {bi.get_cell(r, 'equipableslots')})")
    lines.append("")
    lines.append("## Notes")
    lines.append("")
    lines.append(f"- baseitems.2da source: `{bi_path}` (override wins over chitin).")
    lines.append(f"- appearance.2da source: `{ap_path}`.")
    lines.append("- Slot bits (pykotor EquipmentSlot, same bits in Equip_ItemList struct_id and "
                 "baseitems equipableslots): " + ", ".join(f"{n}=0x{b:X}" for b, n in slot_bits) + ".")
    lines.append("- is_headgear = equipableslots has the HEAD bit (0x1). Cross-check: the slot bit check section "
                 "shows which base items the game actually equips under each slot id.")
    lines.append("- UTI lookup order: same module stem, then override, then chitin, then any module (first "
                 "match by sorted container). UNRESOLVED = no UTI found in any of these.")
    lines.append("- Race is the raw UTC Race id (racialtypes), not labelled here. Appearance label = "
                 "appearance.2da label, or rowN when the label cell is blank.")
    lines.append(f"- appearance.2da head/bodyvariant columns absent from the file: "
                 f"{', '.join(missing_head_cols) or 'none'}.")
    lines.append(f"- baseitems.2da optional columns absent from the file: {', '.join(missing_bi_cols) or 'none'}.")
    lines.append("- Counts are per UTC occurrence (a template in two containers counts twice). "
                 "Unique resref counts are given separately.")
    lines.append("- Appearance-label 'cannot equip' categories are label-only observations; "
                 "no conclusion is drawn from data.")
    (out / "SUMMARY.md").write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"baseitems_equip.tsv rows={len(base_rows)} headgear={len(headgear_types)}")
    print(f"appearance_rows.tsv rows={len(app_rows)}")
    print(f"utc_equip.tsv rows={len(equip_rows)} with_any={with_any} with_head={with_head} "
          f"parse_fail={utc_parse_fail}")
    print(f"SUMMARY.md written to {out / 'SUMMARY.md'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
