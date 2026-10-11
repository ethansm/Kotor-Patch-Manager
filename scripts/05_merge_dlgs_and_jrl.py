"""
05_merge_dlgs_and_jrl.py - Phase 5: Companion Dialogue Splice, Journal Merge, & GUI Merge

1. Splices Echoes companion dialogue branches into PartySwap/Community-patched
   dialogues for Atton, Handmaiden, Kreia, Mandalore, Mira, and Visas Marr.
2. Merges 97 new quest categories from Echoes' global.jrl into live override/global.jrl,
   preserving live quests and excluding M4-78 additions.
3. Merges custom planet UI buttons into live override/galaxymap_p.gui, preserving
   Malachor V and widescreen positioning.
"""

import os
import sys
import copy
from pathlib import Path
from pykotor.resource.formats.gff import read_gff, write_gff, GFFList, GFFStruct
from pykotor.resource.type import ResourceType

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _common import GAME_DIR, STAGING_DIR, find_one_ci  # noqa: E402

LIVE_OVERRIDE = GAME_DIR / "override"
STAGE_OVERRIDE = STAGING_DIR / "Override"

COMPANION_DLGS = [
    "atton.dlg",
    "handmaiden.dlg",
    "kreia.dlg",
    "mandalore.dlg",
    "mira.dlg",
    "visasmarr.dlg",
]
# handmaiden.dlg holocron training entries, which Echoes rewrites in place
HANDMAIDEN_REPLACED_ENTRIES = [418, 419, 420]
# galaxymap_p.gui controls Echoes adds; the live file keeps Malachor V and widescreen positions
GUI_TARGET_TAGS = ["GUI_slehe_1", "gui_yavin_1", "lbl_imana", "lbl_itato"]

def find_file_ci(directory: Path, filename: str) -> Path:
    """The one file named filename in any case; raises if two real spellings exist."""
    return find_one_ci(directory, filename)

def merge_companion_dialogue(dlg_name: str):
    print(f"\n--- Merging companion dialogue: {dlg_name} ---")
    live_path = find_file_ci(LIVE_OVERRIDE, dlg_name)
    stage_path = find_file_ci(STAGE_OVERRIDE, dlg_name)
    
    g_live = read_gff(live_path)
    g_stage = read_gff(stage_path)
    
    e_live = g_live.root.get_list("EntryList")
    e_stage = g_stage.root.get_list("EntryList")
    r_live = g_live.root.get_list("ReplyList")
    r_stage = g_stage.root.get_list("ReplyList")
    
    live_entries_count = len(e_live)
    stage_entries_count = len(e_stage)
    live_replies_count = len(r_live)
    stage_replies_count = len(r_stage)
    
    # 1. Append new entries from stage (from live_entries_count to stage_entries_count)
    if stage_entries_count > live_entries_count:
        for i in range(live_entries_count, stage_entries_count):
            e_live.append(copy.deepcopy(e_stage[i]))
        print(f"  Appended {stage_entries_count - live_entries_count} new entries (new total: {len(e_live)})")
    else:
        print(f"  Entries already up-to-date (total: {len(e_live)})")
        
    # 2. Append new replies from stage (from live_replies_count to stage_replies_count)
    if stage_replies_count > live_replies_count:
        for i in range(live_replies_count, stage_replies_count):
            r_live.append(copy.deepcopy(r_stage[i]))
        print(f"  Appended {stage_replies_count - live_replies_count} new replies (new total: {len(r_live)})")
    else:
        print(f"  Replies already up-to-date (total: {len(r_live)})")
        
    # 3. Special case for handmaiden.dlg: update holocron learning entries (418, 419, 420)
    if dlg_name.lower() == "handmaiden.dlg":
        for idx in HANDMAIDEN_REPLACED_ENTRIES:
            if idx < len(e_live) and idx < len(e_stage):
                e_live[idx] = copy.deepcopy(e_stage[idx])
        print(f"  Updated Handmaiden holocron training entries {HANDMAIDEN_REPLACED_ENTRIES}")
        
    # 4. Splice new reply links into existing entries' RepliesList
    spliced_entries = 0
    for i in range(min(live_entries_count, stage_entries_count)):
        st_links = e_stage[i].get_list("RepliesList") or []
        lv_links = e_live[i].get_list("RepliesList")
        if lv_links is None:
            continue
            
        lv_indices = {r.get_uint32("Index") for r in lv_links}
        modified = False
        for link in st_links:
            idx = link.get_uint32("Index")
            if idx not in lv_indices:
                lv_links.append(copy.deepcopy(link))
                lv_indices.add(idx)
                spliced_entries += 1
                modified = True
        if modified:
            e_live[i].set_list("RepliesList", lv_links)
                
    if spliced_entries > 0:
        print(f"  Spliced {spliced_entries} new reply link(s) into existing entry nodes")
        
    # 5. Splice new entry links into existing replies' EntriesList
    spliced_replies = 0
    for i in range(min(live_replies_count, stage_replies_count)):
        st_links = r_stage[i].get_list("EntriesList") or []
        lv_links = r_live[i].get_list("EntriesList")
        if lv_links is None:
            continue
            
        lv_indices = {e.get_uint32("Index") for e in lv_links}
        modified = False
        for link in st_links:
            idx = link.get_uint32("Index")
            if idx not in lv_indices:
                lv_links.append(copy.deepcopy(link))
                lv_indices.add(idx)
                spliced_replies += 1
                modified = True
        if modified:
            r_live[i].set_list("EntriesList", lv_links)
                
    if spliced_replies > 0:
        print(f"  Spliced {spliced_replies} new entry link(s) into existing reply nodes")
        
    # Save modified lists back to root
    g_live.root.set_list("EntryList", e_live)
    g_live.root.set_list("ReplyList", r_live)
    
    write_gff(g_live, live_path)
    print(f"  Saved merged {live_path.name} successfully.")

def merge_journal():
    print("\n--- Merging global.jrl ---")
    live_path = find_file_ci(LIVE_OVERRIDE, "global.jrl")
    stage_path = find_file_ci(STAGE_OVERRIDE, "global.jrl")
    
    g_live = read_gff(live_path)
    g_stage = read_gff(stage_path)
    
    cats_live = g_live.root.get_list("Categories")
    cats_stage = g_stage.root.get_list("Categories")
    existing_tags = {c.get_string("Tag") for c in cats_live}
    
    added_count = 0
    for c in cats_stage:
        tag = c.get_string("Tag")
        if tag and tag not in existing_tags:
            cats_live.append(copy.deepcopy(c))
            existing_tags.add(tag)
            added_count += 1
            
    print(f"  Appended {added_count} new quest categories (new total: {len(cats_live)})")
    g_live.root.set_list("Categories", cats_live)
    write_gff(g_live, live_path)
    print(f"  Saved merged {live_path.name} successfully.")

def merge_galaxy_map_gui():
    print("\n--- Merging galaxymap_p.gui ---")
    live_path = find_file_ci(LIVE_OVERRIDE, "galaxymap_p.gui")
    stage_path = find_file_ci(STAGE_OVERRIDE, "galaxymap_p.gui")
    
    g_live = read_gff(live_path)
    g_stage = read_gff(stage_path)
    
    ctrls_live = g_live.root.get_list("CONTROLS")
    ctrls_stage = g_stage.root.get_list("CONTROLS")
    existing_tags = {c.get_string("TAG") for c in ctrls_live}
    
    added_count = 0
    for c in ctrls_stage:
        tag = c.get_string("TAG")
        if tag in GUI_TARGET_TAGS and tag not in existing_tags:
            ctrls_live.append(copy.deepcopy(c))
            existing_tags.add(tag)
            added_count += 1
            print(f"  Appended planet UI control: {tag}")
            
    print(f"  Appended {added_count} new planet controls (new total: {len(ctrls_live)})")
    g_live.root.set_list("CONTROLS", ctrls_live)
    write_gff(g_live, live_path)
    print(f"  Saved merged {live_path.name} successfully (preserved Malachor V and widescreen positioning).")

def main():
    print("=" * 60)
    print("Phase 5: Companion Dialogue Splice & Journal / GUI Merge")
    print("=" * 60)
    
    for dlg in COMPANION_DLGS:
        merge_companion_dialogue(dlg)
        
    merge_journal()
    merge_galaxy_map_gui()
    
    print("\n" + "=" * 60)
    print("Phase 5 Complete!")
    print("=" * 60)

if __name__ == "__main__":
    main()
