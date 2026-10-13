#!/usr/bin/env python3
"""npcvar_utc_geom.py - NPC-variety recon: pool equipment state and module area geometry.

Writes two TSVs under .claude/research/npc_variety/tables/:
  pool_armed.tsv     per pool_candidates.tsv resref: equipped + inventory items from the chitin UTC
  area_geometry.tsv  per module: GIT waypoint/creature counts and room WOK walkable face counts

Definitions follow .claude/research/npc_add/poc/W1c_walkmesh.md: a face is walkable when
pykotor's SurfaceMaterial.walkable() says so, and the walkmesh is the set of room WOKs named by
the module LYT (module archives hold no WOK). Module GIT counts come from <module>.mod when it
exists (its GIT carries the extra creatures), with the <module>.rim GIT counts kept in notes.

Run `.venv/bin/python scripts/npcvar_utc_geom.py` to regenerate both files. The pure helpers
below are imported by scripts/test_npcvar_utc_geom.py, which runs without pykotor; pykotor is
imported inside the functions that read the install.
"""
import logging
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))
from npcvar_tables import read_tsv, write_tsv  # noqa: E402

TABLE_DIR = REPO_ROOT / ".claude" / "research" / "npc_variety" / "tables"
POOL_IN = TABLE_DIR / "pool_candidates.tsv"
POOL_OUT = TABLE_DIR / "pool_armed.tsv"
AREA_OUT = TABLE_DIR / "area_geometry.tsv"
GAME_DIR = Path.home() / ".local" / "share" / "Steam" / "steamapps" / "common" / "Knights of the Old Republic II"

POOL_COLUMNS = ["resref", "n_items", "weapon_slot_filled", "item_resrefs"]
AREA_COLUMNS = ["module", "n_waypoints", "n_creatures", "n_wok", "walkable_faces", "notes"]
MODULES = ("302nar", "301nar", "682dan", "207tel", "203tel", "204tel")

# EquipmentSlot names (pykotor.common.misc) that count as a weapon. RIGHT_HAND_2 / LEFT_HAND_2
# are TSL-only and are deliberately not in this set (the brief lists the K1-era slots only).
WEAPON_SLOTS = frozenset({"RIGHT_HAND", "LEFT_HAND", "CLAW1", "CLAW2", "CLAW3"})


def weapon_slot_filled(equipment):
    """equipment: iterable of (slot_name, resref). True when a weapon slot holds a non-blank resref."""
    return any(slot in WEAPON_SLOTS and resref.strip() != "" for slot, resref in equipment)


def summarize_items(equipment, inventory):
    """equipment: [(slot_name, resref)] in slot order; inventory: [resref] in list order.

    Blank resrefs are skipped. Equipped items come first in item_resrefs, then inventory.
    """
    equipped = [resref.strip() for _, resref in equipment if resref.strip()]
    carried = [resref.strip() for resref in inventory if resref.strip()]
    items = equipped + carried
    return {
        "n_items": str(len(items)),
        "weapon_slot_filled": "true" if weapon_slot_filled(equipment) else "false",
        "item_resrefs": ",".join(items),
    }


def pool_row(resref, items):
    """One pool_armed.tsv row. items is None when the UTC is missing, else (equipment, inventory)."""
    if items is None:
        return {"resref": resref, "n_items": "NA", "weapon_slot_filled": "NA", "item_resrefs": ""}
    equipment, inventory = items
    return {"resref": resref, **summarize_items(equipment, inventory)}


def count_walkable(faces):
    """faces: objects with .material.walkable(). Returns (total faces, walkable faces)."""
    total = 0
    walkable = 0
    for face in faces:
        total += 1
        if face.material.walkable():
            walkable += 1
    return total, walkable


def unique_names(names):
    """Order-preserving, case-insensitive dedupe of non-blank names; the first spelling wins."""
    seen = set()
    out = []
    for name in names:
        clean = name.strip()
        key = clean.lower()
        if clean and key not in seen:
            seen.add(key)
            out.append(clean)
    return out


def area_row(module, git_mod, git_rim, rooms):
    """One area_geometry.tsv row.

    git_mod / git_rim: (n_waypoints, n_creatures) from <module>.mod / <module>.rim, or None.
    rooms: {"n_rooms", "n_wok", "faces", "walkable", "missing", "override"} from the module LYT,
    or None. "override" counts WOKs read from Override/ (loose files, not chitin).
    """
    notes = []
    primary = git_mod if git_mod is not None else git_rim
    if git_mod is not None:
        notes.append("GIT from .mod")
        if git_rim is not None and git_rim != git_mod:
            notes.append(f"rim GIT wp={git_rim[0]} cre={git_rim[1]}")
    elif git_rim is not None:
        notes.append("GIT from .rim (no .mod)")
    else:
        notes.append("no GIT found")

    if rooms is None:
        n_wok, walkable = "NA", "NA"
        notes.append("no LYT")
    else:
        n_wok, walkable = str(rooms["n_wok"]), str(rooms["walkable"])
        notes.append(f"LYT rooms={rooms['n_rooms']}")
        notes.append(f"faces total={rooms['faces']}")
        if rooms.get("override"):
            notes.append(f"WOK from Override={rooms['override']}")
        if rooms["missing"]:
            notes.append("missing WOK: " + ",".join(rooms["missing"]))

    n_waypoints, n_creatures = (str(primary[0]), str(primary[1])) if primary is not None else ("NA", "NA")
    return {
        "module": module,
        "n_waypoints": n_waypoints,
        "n_creatures": n_creatures,
        "n_wok": n_wok,
        "walkable_faces": walkable,
        "notes": "; ".join(notes),
    }


def open_install(game_dir):
    from pykotor.extract.installation import Installation

    return Installation(game_dir)


def load_utc_items(inst, resref):
    """(equipment [(slot, resref)], inventory [resref]) from the chitin UTC, or None when missing."""
    from pykotor.extract.installation import SearchLocation
    from pykotor.resource.generics.utc import read_utc
    from pykotor.resource.type import ResourceType

    res = inst.resource(resref, ResourceType.UTC, order=[SearchLocation.CHITIN])
    if res is None:
        return None
    utc = read_utc(res.data)
    equipment = [
        (slot.name, str(item.resref))
        for slot, item in sorted(utc.equipment.items(), key=lambda kv: kv[0].value)
    ]
    inventory = [str(item.resref) for item in utc.inventory]
    return equipment, inventory


def find_module_file(modules_dir, module, ext):
    for name in (module.lower() + ext, module.upper() + ext.upper()):
        path = modules_dir / name
        if path.is_file():
            return path
    return None


def module_git_counts(game_dir, module):
    """{"mod": (n_waypoints, n_creatures) | None, "rim": (...) | None} from Modules/<module>.mod/.rim."""
    from pykotor.resource.formats.erf import read_erf
    from pykotor.resource.formats.rim import read_rim
    from pykotor.resource.generics.git import read_git
    from pykotor.resource.type import ResourceType

    modules_dir = game_dir / "Modules"
    out = {"mod": None, "rim": None}
    for key, ext, reader in (("mod", ".mod", read_erf), ("rim", ".rim", read_rim)):
        path = find_module_file(modules_dir, module, ext)
        if path is None:
            continue
        raw = reader(path).get(module, ResourceType.GIT)
        if raw is None:
            continue
        git = read_git(raw)
        out[key] = (len(git.waypoints), len(git.creatures))
    return out


def room_wok_stats(inst, module):
    """Walkable stats over the room WOKs named by the module LYT, or None when there is no LYT.

    Uses the installation's default lookup (Override, then Modules, then chitin), the order the
    game resolves them in. 682dan keeps its LYT in Modules/682dan.mod and its WOKs in Override/.
    """
    from pykotor.resource.formats.bwm import read_bwm
    from pykotor.resource.formats.lyt import read_lyt
    from pykotor.resource.type import ResourceType

    lyt_res = inst.resource(module, ResourceType.LYT)
    if lyt_res is None:
        return None
    rooms = unique_names(room.model for room in read_lyt(lyt_res.data).rooms)
    stats = {"n_rooms": len(rooms), "n_wok": 0, "faces": 0, "walkable": 0, "missing": [], "override": 0}
    for room in rooms:
        wok = inst.resource(room, ResourceType.WOK)
        if wok is None:
            stats["missing"].append(room)
            continue
        total, walkable = count_walkable(read_bwm(wok.data).faces)
        stats["n_wok"] += 1
        stats["faces"] += total
        stats["walkable"] += walkable
        if Path(str(wok.filepath)).parent.name.lower() == "override":
            stats["override"] += 1
    return stats


def pool_armed_rows(inst, pool_rows):
    return [pool_row(r["resref"], load_utc_items(inst, r["resref"])) for r in pool_rows]


def area_rows(inst, game_dir):
    rows = []
    for module in MODULES:
        git = module_git_counts(game_dir, module)
        rooms = room_wok_stats(inst, module)
        rows.append(area_row(module, git["mod"], git["rim"], rooms))
    return rows


def main():
    logging.disable(logging.CRITICAL)
    if not GAME_DIR.is_dir():
        print(f"game dir not found: {GAME_DIR}", file=sys.stderr)
        return 1
    inst = open_install(GAME_DIR)
    armed = pool_armed_rows(inst, read_tsv(POOL_IN))
    area = area_rows(inst, GAME_DIR)
    write_tsv(POOL_OUT, POOL_COLUMNS, armed)
    write_tsv(AREA_OUT, AREA_COLUMNS, area)
    print(f"pool_armed: {len(armed)} rows -> {POOL_OUT}")
    print(f"area_geometry: {len(area)} rows -> {AREA_OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
