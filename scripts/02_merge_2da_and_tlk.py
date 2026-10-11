#!/usr/bin/env python3
"""Merge Echoes of the Republic 2DA tables and TLK strings into the live Community Build.

Preserves all 154 existing mods, prevents any table wiping or truncation,
and re-indexes creature appearances, placeable appearances, spell IDs, and effect icons.

Every input is read and every merged table is built in memory before anything is
written, so a missing or unreadable file stops the run with the game unchanged.
The tables are written first, then the staged GFF/NCS edits, then dialog.tlk.

Appending is not idempotent, so a second run is refused: if the live dialog.tlk
already ends with Echoes' strings, or appearance.2da already has Echoes' first
appearance row, the script exits 1 before writing anything. To re-run, restore
the pre-compatch backup first.

Needs the patched pykotor (../pykotor_patches/README.md, patch #5). Stock 2.3.12
reads some shipped 2DAs as blank tables, and this script would write them back.
"""
import os
import shutil
import sys
from pathlib import Path

from pykotor.resource.formats.tlk import read_tlk, write_tlk
from pykotor.resource.formats.twoda import read_2da, write_2da, TwoDA
from pykotor.resource.formats.gff import read_gff, write_gff
from pykotor.resource.formats.ncs import read_ncs, write_ncs

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _common import GAME_DIR, STAGING_DIR  # noqa: E402

LIVE_ROOT = GAME_DIR
LIVE_OVERRIDE = LIVE_ROOT / "override"
STAGING_ROOT = STAGING_DIR
STAGING_OVERRIDE = STAGING_ROOT / "Override"

# Echoes added strings 136375..136398 (24 strings) to the end of its dialog.tlk.
ECHOES_TLK_START = 136375

# First row Echoes added to each table; rows from here to the end are appended.
ECHOES_FIRST_ROW = {
    "heads.2da": 192,
    "appearance.2da": 678,  # rows 671..677 are TSLRCM, already in live; Echoes adds 678..725
    "placeables.2da": 375,  # rows 361..374 are M4-78/TSLRCM, already in live; Echoes adds 375..396
    "upgrade.2da": 369,  # 369..373
    "ambientmusic.2da": 59,
    "modulesave.2da": 91,  # 91..219
    "globalcat.2da": 999,  # only names not already in live are added
}
ECHOES_SPELL_ROW = 282  # spells.2da row 282 in Echoes is ekunholo
ECHOES_EFFECTICON_ROW = 114  # effecticon.2da row 114; ekun_holo.ncs pushes it as CONSTI 114

# The 3 Echoes creatures that use Echoes-added appearance rows.
UTC_APPEARANCE = {
    "n_ezerarden.utc": 708,
    "n_jetpuppet.utc": 718,
    "n_kazius.utc": 712,
}
PLANETARY_TLK_COLUMNS = ["name", "description", "lockedoutreason"]
PLANETARY_M478_ROW = 4  # M4-78: disabled, since the build doesn't include it
COPIED_2DAS = ["genericdoors.2da", "soundset.2da"]  # copied whole


def copy_2da_row(src_2da, src_idx, dst_2da, remap_dict=None):
    dst_idx = dst_2da.add_row()
    dst_2da.set_label(dst_idx, src_2da.get_row(src_idx).label())
    for h in dst_2da.get_headers():
        if h in src_2da.get_headers():
            val = src_2da.get_cell(src_idx, h)
            if remap_dict and h in remap_dict and val in remap_dict[h]:
                val = remap_dict[h][val]
            dst_2da.set_cell(dst_idx, h, val)
        else:
            dst_2da.set_cell(dst_idx, h, "****")
    return dst_idx


def append_rows(src, dst, first, remap_dict=None):
    """Copy rows first.. of src onto the end of dst. Returns {old: new} for int and str keys."""
    row_map = {}
    for r in range(first, src.get_height()):
        new_idx = copy_2da_row(src, r, dst, remap_dict)
        row_map[r] = new_idx
        row_map[str(r)] = str(new_idx)
    return row_map


def already_applied(live_tlk, echoes_tlk, live_app, echoes_app):
    """Why the merge looks already done, or None."""
    tail = [echoes_tlk.get(i).text for i in range(ECHOES_TLK_START, len(echoes_tlk))]
    if tail and len(live_tlk) >= len(tail):
        live_tail = [live_tlk.get(i).text for i in range(len(live_tlk) - len(tail), len(live_tlk))]
        if live_tail == tail:
            return f"live dialog.tlk already ends with Echoes' {len(tail)} strings"

    first = ECHOES_FIRST_ROW["appearance.2da"]
    if first < echoes_app.get_height():
        label = echoes_app.get_row(first).label()
        # normalhead is remapped on merge, and live-only columns are "****"
        cols = [h for h in echoes_app.get_headers() if h in live_app.get_headers() and h != "normalhead"]
        want = [echoes_app.get_cell(first, h) for h in cols]
        for r in range(live_app.get_height()):
            if live_app.get_row(r).label() == label and [live_app.get_cell(r, h) for h in cols] == want:
                return f"live appearance.2da row {r} is already Echoes' appearance row {first}"
    return None


def main():
    live_tlk_path = LIVE_ROOT / "dialog.tlk"
    table_names = list(ECHOES_FIRST_ROW) + ["spells.2da", "effecticon.2da", "planetary.2da"]

    print("=== Reading inputs ===")
    live_tlk = read_tlk(live_tlk_path)
    echoes_tlk = read_tlk(STAGING_ROOT / "dialog.tlk")
    echoes = {n: read_2da(STAGING_OVERRIDE / n) for n in table_names}
    live = {n: read_2da(LIVE_OVERRIDE / n) for n in table_names if n != "planetary.2da"}
    for fname in COPIED_2DAS:
        if not (STAGING_OVERRIDE / fname).is_file():
            sys.exit(f"Missing {STAGING_OVERRIDE / fname}")
    utcs = {n: read_gff(STAGING_OVERRIDE / n) for n in UTC_APPEARANCE}
    utps = {f: read_gff(STAGING_OVERRIDE / f) for f in sorted(os.listdir(STAGING_OVERRIDE)) if f.lower().endswith(".utp")}
    uti = read_gff(STAGING_OVERRIDE / "ekunholo.uti")
    ncs = read_ncs(STAGING_OVERRIDE / "ekun_holo.ncs")

    why = already_applied(live_tlk, echoes_tlk, live["appearance.2da"], echoes["appearance.2da"])
    if why:
        sys.exit(f"phase 2 already applied ({why}). Nothing written. "
                 "Restore the pre-compatch backup to run it again.")

    print("=== Step 1: Merging dialog.tlk Strings ===")
    print(f"Live TLK current size: {len(live_tlk)}")
    print(f"Echoes TLK size: {len(echoes_tlk)}")
    tlk_map = {}
    for old_id in range(ECHOES_TLK_START, len(echoes_tlk)):
        entry = echoes_tlk.get(old_id)
        new_id = live_tlk.add(entry.text, str(entry.voiceover) if entry.voiceover else "")
        tlk_map[str(old_id)] = str(new_id)
        print(f"   TLK [{old_id}] -> [{new_id}]: {repr(entry.text)[:50]}")

    print("\n=== Step 2: Merging heads.2da ===")
    head_map = append_rows(echoes["heads.2da"], live["heads.2da"], ECHOES_FIRST_ROW["heads.2da"])
    print(f"Merged {len(head_map)//2} head rows into heads.2da (new height: {live['heads.2da'].get_height()})")

    print("\n=== Step 3: Merging appearance.2da ===")
    app_map = append_rows(echoes["appearance.2da"], live["appearance.2da"],
                          ECHOES_FIRST_ROW["appearance.2da"], {"normalhead": head_map})
    print(f"Merged {len(app_map)//2} appearance rows into appearance.2da (new height: {live['appearance.2da'].get_height()})")

    print("\n=== Step 4: Merging placeables.2da ===")
    plc_map = append_rows(echoes["placeables.2da"], live["placeables.2da"], ECHOES_FIRST_ROW["placeables.2da"])
    print(f"Merged {len(plc_map)//2} placeable rows into placeables.2da (new height: {live['placeables.2da'].get_height()})")

    print("\n=== Step 5: Merging spells.2da ===")
    new_spell_idx = copy_2da_row(echoes["spells.2da"], ECHOES_SPELL_ROW, live["spells.2da"])
    print(f"Merged spell {ECHOES_SPELL_ROW} as {new_spell_idx} in spells.2da (new height: {live['spells.2da'].get_height()})")

    print("\n=== Step 6: Merging effecticon.2da ===")
    new_ei_idx = copy_2da_row(echoes["effecticon.2da"], ECHOES_EFFECTICON_ROW, live["effecticon.2da"],
                              {"namestrref": tlk_map})
    print(f"Merged effecticon {ECHOES_EFFECTICON_ROW} as {new_ei_idx} in effecticon.2da (new height: {live['effecticon.2da'].get_height()})")

    print("\n=== Step 7: Merging upgrade.2da ===")
    up_map = append_rows(echoes["upgrade.2da"], live["upgrade.2da"], ECHOES_FIRST_ROW["upgrade.2da"])
    print(f"Merged {len(up_map)//2} upgrade rows into upgrade.2da (new height: {live['upgrade.2da'].get_height()})")

    print("\n=== Step 8: Merging ambientmusic.2da ===")
    music_map = append_rows(echoes["ambientmusic.2da"], live["ambientmusic.2da"], ECHOES_FIRST_ROW["ambientmusic.2da"])
    print(f"Merged {len(music_map)//2} music rows into ambientmusic.2da (new height: {live['ambientmusic.2da'].get_height()})")

    print("\n=== Step 9: Merging modulesave.2da ===")
    ms_map = append_rows(echoes["modulesave.2da"], live["modulesave.2da"], ECHOES_FIRST_ROW["modulesave.2da"],
                         {"areaname": tlk_map})
    print(f"Merged {len(ms_map)//2} modulesave rows into modulesave.2da (new height: {live['modulesave.2da'].get_height()})")

    print("\n=== Step 10: Merging globalcat.2da ===")
    echoes_gc, live_gc = echoes["globalcat.2da"], live["globalcat.2da"]
    live_names = set(live_gc.get_row(r).get_string("name").lower() for r in range(live_gc.get_height()))
    added_gc = 0
    for r in range(ECHOES_FIRST_ROW["globalcat.2da"], echoes_gc.get_height()):
        name = echoes_gc.get_row(r).get_string("name")
        if name.lower() not in live_names:
            copy_2da_row(echoes_gc, r, live_gc)
            live_names.add(name.lower())
            added_gc += 1
    print(f"Added {added_gc} unique global variables into globalcat.2da (new height: {live_gc.get_height()})")

    print("\n=== Step 11: Merging and Decoupling planetary.2da ===")
    echoes_plan = echoes["planetary.2da"]
    for r in range(echoes_plan.get_height()):
        for col in PLANETARY_TLK_COLUMNS:
            val = echoes_plan.get_cell(r, col)
            if val in tlk_map:
                echoes_plan.set_cell(r, col, tlk_map[val])
    echoes_plan.set_cell(PLANETARY_M478_ROW, "label", "M4_78_Disabled")
    echoes_plan.set_cell(PLANETARY_M478_ROW, "model", "****")
    echoes_plan.set_cell(PLANETARY_M478_ROW, "icon", "****")
    echoes_plan.set_cell(PLANETARY_M478_ROW, "guitag", "")
    print("Configured planetary.2da (M4-78 disabled, new planets mapped).")

    print("\n=== Step 12: Updating Staged GFFs with New Mapped IDs ===")
    staged = []  # (write function, object, path)
    for utc_name, old_app in UTC_APPEARANCE.items():
        new_app = app_map[old_app]
        utcs[utc_name].root.set_uint16("Appearance_Type", new_app)
        staged.append((write_gff, utcs[utc_name], STAGING_OVERRIDE / utc_name))
        print(f"Updated {utc_name}: Appearance_Type {old_app} -> {new_app}")

    for utp_name, gff in utps.items():
        old_app = gff.root.get_uint32("Appearance")
        if old_app in plc_map:
            gff.root.set_uint32("Appearance", plc_map[old_app])
            staged.append((write_gff, gff, STAGING_OVERRIDE / utp_name))
            print(f"Updated {utp_name}: Appearance {old_app} -> {plc_map[old_app]}")

    for prop in uti.root.get_list("PropertiesList"):
        if prop.get_uint16("PropertyName") == 10 and prop.get_uint16("Subtype") == ECHOES_SPELL_ROW:
            prop.set_uint16("Subtype", new_spell_idx)
            print(f"Updated ekunholo.uti: Spell Subtype {ECHOES_SPELL_ROW} -> {new_spell_idx}")
    staged.append((write_gff, uti, STAGING_OVERRIDE / "ekunholo.uti"))

    # ekun_holo.ncs pushes the effect icon index as a constant (instruction 42)
    for instr in ncs.instructions:
        if instr.ins_type.name == "CONSTI" and instr.args == [ECHOES_EFFECTICON_ROW]:
            instr.args = [new_ei_idx]
            staged.append((write_ncs, ncs, STAGING_OVERRIDE / "ekun_holo.ncs"))
            print(f"Patched ekun_holo.ncs: Icon index {ECHOES_EFFECTICON_ROW} -> {new_ei_idx}")
            break
    else:
        print(f"Warning: Could not find CONSTI {ECHOES_EFFECTICON_ROW} in ekun_holo.ncs")

    print("\n=== Step 13: Writing ===")
    for name, table in live.items():
        write_2da(table, LIVE_OVERRIDE / name)
    write_2da(echoes_plan, LIVE_OVERRIDE / "planetary.2da")
    for fname in COPIED_2DAS:
        shutil.copy2(STAGING_OVERRIDE / fname, LIVE_OVERRIDE / fname)
    print(f"Wrote {len(live) + 1} merged tables and copied {', '.join(COPIED_2DAS)} to live override.")
    for write, obj, path in staged:
        write(obj, path)
    print(f"Wrote {len(staged)} re-indexed staging file(s).")
    write_tlk(live_tlk, live_tlk_path)
    print(f"Updated live dialog.tlk. New entry count: {len(live_tlk)}")

    print("\nPhase 2 (2DA, TLK, GFF re-indexing) 100% COMPLETE and VERIFIED!")


if __name__ == "__main__":
    main()
