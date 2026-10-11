"""
04_merge_modules.py - Phase 4: Surgical Merge of 7 Colliding Base-Game Modules

Merges Echoes of the Republic's newly added creatures, waypoints, triggers,
doors, and quests into the live Community Build module capsules without
overwriting community fixes (such as Lootra, Mira, and Korriban patches).

Target Capsules:
1. 204TEL.mod (Citadel Station Residential - corteldc medical contact)
2. 221TEL.MOD (Citadel Station Czerka / Grenn - Captain Tarkus spawn)
3. 302NAR.mod (Refugee Quad - IG-12 assassin door, Talon gang)
4. 303nar.mod (Docks - Darth Talon spawn waypoint & hook)
5. 601DAN.mod (Khoonda Plains - Dantooine Grove transition trigger & return waypoint)
6. 701kor.mod (Valley of the Dark Lords - Tomb entrances, Visas quest hook)
7. 710KOR.MOD (Shyrack Caves - Ancient Sith Temple transition placeable & return waypoint)
"""

import os
import sys
import copy
from pathlib import Path
from pykotor.resource.formats.erf import read_erf, write_erf, ERF, ERFType
from pykotor.resource.formats.gff import read_gff, write_gff, GFFStruct
from pykotor.resource.type import ResourceType
from pykotor.common.misc import ResRef

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _common import GAME_DIR, STAGING_DIR, find_one_ci  # noqa: E402

LIVE_MOD_DIR = GAME_DIR / "Modules"
STAGE_MOD_DIR = STAGING_DIR / "Modules"

def find_file(directory: Path, filename: str) -> Path:
    """The one file named filename in any case; raises if two real spellings exist."""
    return find_one_ci(directory, filename)

def merge_module_204tel():
    print("\n--- Merging 204TEL.mod ---")
    live_path = find_file(LIVE_MOD_DIR, "204TEL.mod")
    stage_path = find_file(STAGE_MOD_DIR, "204tel.mod")
    
    live_erf = read_erf(live_path)
    stage_erf = read_erf(stage_path)
    
    # 1. Update 204tel.git: append corteldc creature
    git_res = live_erf.get("204tel", ResourceType.GIT)
    git_gff = read_gff(git_res)
    stage_git = read_gff(stage_erf.get("204tel", ResourceType.GIT))
    
    creatures = git_gff.root.get_list("Creature List")
    stage_creatures = stage_git.root.get_list("Creature List")
    existing_creatures = {str(c.get_resref("TemplateResRef")) for c in creatures}
    
    corteldc = None
    for c in stage_creatures:
        if str(c.get_resref("TemplateResRef")) == "corteldc":
            corteldc = c
            break
    if corteldc and "corteldc" not in existing_creatures:
        creatures.append(copy.deepcopy(corteldc))
        git_gff.root.set_list("Creature List", creatures)
        print(f"  Appended creature 'corteldc' to Creature List (new total: {len(creatures)})")
    else:
        print(f"  Creature 'corteldc' already present in Creature List")
    
    git_bytes = bytearray()
    write_gff(git_gff, git_bytes)
    live_erf.set_data("204tel", ResourceType.GIT, bytes(git_bytes))
    
    # 2. Inject stage-only files
    for r in stage_erf:
        resref_lower = r.resref.lower()
        if not live_erf.has(resref_lower, r.restype):
            live_erf.set_data(r.resref, r.restype, r.data)
            print(f"  Injected new resource: {r.resref}.{r.restype.extension}")
            
    write_erf(live_erf, live_path)
    print(f"  Saved merged {live_path.name} successfully.")

def merge_module_221tel():
    print("\n--- Merging 221TEL.MOD ---")
    live_path = find_file(LIVE_MOD_DIR, "221TEL.MOD")
    stage_path = find_file(STAGE_MOD_DIR, "221tel.mod")
    
    live_erf = read_erf(live_path)
    stage_erf = read_erf(stage_path)
    
    # 1. Update 221tel.git: append tark_spwn1 waypoint
    git_res = live_erf.get("221tel", ResourceType.GIT)
    git_gff = read_gff(git_res)
    stage_git = read_gff(stage_erf.get("221tel", ResourceType.GIT))
    
    wps = git_gff.root.get_list("WaypointList")
    stage_wps = stage_git.root.get_list("WaypointList")
    existing_wps = {w.get_string("Tag") for w in wps}
    
    for w in stage_wps:
        if w.get_string("Tag") == "tark_spwn1" and "tark_spwn1" not in existing_wps:
            wps.append(copy.deepcopy(w))
            git_gff.root.set_list("WaypointList", wps)
            print(f"  Appended waypoint 'tark_spwn1' to WaypointList (new total: {len(wps)})")
            break
            
    git_bytes = bytearray()
    write_gff(git_gff, git_bytes)
    live_erf.set_data("221tel", ResourceType.GIT, bytes(git_bytes))
    
    # 2. Update grenn.dlg: hook spwnteltark1 into ReplyList[40]
    dlg_res = live_erf.get("grenn", ResourceType.DLG)
    dlg_gff = read_gff(dlg_res)
    reply_list = dlg_gff.root.get_list("ReplyList")
    reply_list[40].set_resref("Script", ResRef("spwnteltark1"))
    dlg_gff.root.set_list("ReplyList", reply_list)
    print(f"  Hooked 'spwnteltark1' script into grenn.dlg ReplyList[40]")
    
    dlg_bytes = bytearray()
    write_gff(dlg_gff, dlg_bytes)
    live_erf.set_data("grenn", ResourceType.DLG, bytes(dlg_bytes))
    
    # 3. Inject stage-only files
    for r in stage_erf:
        if not live_erf.has(r.resref.lower(), r.restype):
            live_erf.set_data(r.resref, r.restype, r.data)
            print(f"  Injected new resource: {r.resref}.{r.restype.extension}")
            
    write_erf(live_erf, live_path)
    print(f"  Saved merged {live_path.name} successfully.")

def merge_module_302nar():
    print("\n--- Merging 302NAR.mod ---")
    live_path = find_file(LIVE_MOD_DIR, "302NAR.mod")
    stage_path = find_file(STAGE_MOD_DIR, "302nar.mod")
    
    live_erf = read_erf(live_path)
    stage_erf = read_erf(stage_path)
    
    # 1. Update 302nar.git
    git_res = live_erf.get("302nar", ResourceType.GIT)
    git_gff = read_gff(git_res)
    stage_git = read_gff(stage_erf.get("302nar", ResourceType.GIT))
    
    # Append creatures: cordrgrecip, bloodfromstone1
    creatures = git_gff.root.get_list("Creature List")
    stage_creatures = stage_git.root.get_list("Creature List")
    existing_creatures = {str(c.get_resref("TemplateResRef")) for c in creatures}
    for c in stage_creatures:
        res = str(c.get_resref("TemplateResRef"))
        if res in ["cordrgrecip", "bloodfromstone1"] and res not in existing_creatures:
            creatures.append(copy.deepcopy(c))
            print(f"  Appended creature '{res}' to Creature List")
    git_gff.root.set_list("Creature List", creatures)
            
    # Update Door 19 to door_corig12
    doors = git_gff.root.get_list("Door List")
    for d in doors:
        x = round(d.get_single("X", 0.0), 1)
        y = round(d.get_single("Y", 0.0), 1)
        if x == 19.2 and y == 170.2:
            d.set_resref("TemplateResRef", ResRef("door_corig12"))
            d.set_string("Tag", "door_corig12")
            print(f"  Updated door at ({x}, {y}) to 'door_corig12'")
            break
    git_gff.root.set_list("Door List", doors)
            
    # Append 10 waypoints
    wps = git_gff.root.get_list("WaypointList")
    stage_wps = stage_git.root.get_list("WaypointList")
    existing_wp_tags = {w.get_string("Tag") for w in wps}
    for w in stage_wps:
        tag = w.get_string("Tag")
        if tag and tag not in existing_wp_tags:
            wps.append(copy.deepcopy(w))
            print(f"  Appended waypoint '{tag}' to WaypointList")
    git_gff.root.set_list("WaypointList", wps)
            
    git_bytes = bytearray()
    write_gff(git_gff, git_bytes)
    live_erf.set_data("302nar", ResourceType.GIT, bytes(git_bytes))
    
    # 2. Inject stage-only files
    injected_count = 0
    for r in stage_erf:
        if not live_erf.has(r.resref.lower(), r.restype):
            live_erf.set_data(r.resref, r.restype, r.data)
            injected_count += 1
    print(f"  Injected {injected_count} new resources into {live_path.name}")
    
    write_erf(live_erf, live_path)
    print(f"  Saved merged {live_path.name} successfully.")

def merge_module_303nar():
    print("\n--- Merging 303nar.mod ---")
    live_path = find_file(LIVE_MOD_DIR, "303nar.mod")
    stage_path = find_file(STAGE_MOD_DIR, "303nar.mod")
    
    live_erf = read_erf(live_path)
    stage_erf = read_erf(stage_path)
    
    # 1. Update 303nar.git: append spwn_talon waypoint
    git_res = live_erf.get("303nar", ResourceType.GIT)
    git_gff = read_gff(git_res)
    stage_git = read_gff(stage_erf.get("303nar", ResourceType.GIT))
    
    wps = git_gff.root.get_list("WaypointList")
    stage_wps = stage_git.root.get_list("WaypointList")
    existing_wps = {w.get_string("Tag") for w in wps}
    for w in stage_wps:
        if w.get_string("Tag") == "spwn_talon" and "spwn_talon" not in existing_wps:
            wps.append(copy.deepcopy(w))
            git_gff.root.set_list("WaypointList", wps)
            print(f"  Appended waypoint 'spwn_talon' to WaypointList")
            break
            
    git_bytes = bytearray()
    write_gff(git_gff, git_bytes)
    live_erf.set_data("303nar", ResourceType.GIT, bytes(git_bytes))
    
    # 2. Update k_303nar_enter.ncs with stage's Talon spawn logic
    stage_enter = stage_erf.get("k_303nar_enter", ResourceType.NCS)
    live_erf.set_data("k_303nar_enter", ResourceType.NCS, stage_enter)
    print(f"  Updated k_303nar_enter.ncs with Talon spawn logic")
    
    # 3. Inject 4 new stage files (talon files)
    for r in stage_erf:
        if not live_erf.has(r.resref.lower(), r.restype):
            live_erf.set_data(r.resref, r.restype, r.data)
            print(f"  Injected new resource: {r.resref}.{r.restype.extension}")
            
    write_erf(live_erf, live_path)
    print(f"  Saved merged {live_path.name} successfully (preserved all 13 community fix files).")

def merge_module_601dan():
    print("\n--- Merging 601DAN.mod ---")
    live_path = find_file(LIVE_MOD_DIR, "601DAN.mod")
    stage_path = find_file(STAGE_MOD_DIR, "601dan.mod")
    
    live_erf = read_erf(live_path)
    stage_erf = read_erf(stage_path)
    
    # 1. Update 601dan.git: add AreaTransition trigger and from_680 waypoint
    git_res = live_erf.get("601dan", ResourceType.GIT)
    git_gff = read_gff(git_res)
    stage_git = read_gff(stage_erf.get("601dan", ResourceType.GIT))
    
    triggers = git_gff.root.get_list("TriggerList")
    stage_triggers = stage_git.root.get_list("TriggerList")
    existing_trigs = {str(t.get_resref("LinkedToModule")) for t in triggers}
    for t in stage_triggers:
        if str(t.get_resref("LinkedToModule")) == "680dan" and "680dan" not in existing_trigs:
            triggers.append(copy.deepcopy(t))
            git_gff.root.set_list("TriggerList", triggers)
            print(f"  Appended Grove transition trigger to TriggerList (target: 680dan)")
            break
            
    wps = git_gff.root.get_list("WaypointList")
    stage_wps = stage_git.root.get_list("WaypointList")
    existing_wps = {w.get_string("Tag") for w in wps}
    for w in stage_wps:
        if w.get_string("Tag") == "from_680" and "from_680" not in existing_wps:
            wps.append(copy.deepcopy(w))
            git_gff.root.set_list("WaypointList", wps)
            print(f"  Appended return waypoint 'from_680' to WaypointList")
            break
            
    git_bytes = bytearray()
    write_gff(git_gff, git_bytes)
    live_erf.set_data("601dan", ResourceType.GIT, bytes(git_bytes))
    
    write_erf(live_erf, live_path)
    print(f"  Saved merged {live_path.name} successfully.")

def merge_module_701kor():
    print("\n--- Merging 701kor.mod ---")
    live_path = find_file(LIVE_MOD_DIR, "701kor.mod")
    stage_path = find_file(STAGE_MOD_DIR, "701KOR.mod")
    
    live_erf = read_erf(live_path)
    stage_erf = read_erf(stage_path)
    
    # 1. Update 701kor.git: add invisokorpath placeable and 5 waypoints
    git_res = live_erf.get("701kor", ResourceType.GIT)
    git_gff = read_gff(git_res)
    stage_git = read_gff(stage_erf.get("701kor", ResourceType.GIT))
    
    plcs = git_gff.root.get_list("Placeable List")
    stage_plcs = stage_git.root.get_list("Placeable List")
    existing_plcs = {str(p.get_resref("TemplateResRef")) for p in plcs}
    for p in stage_plcs:
        res = str(p.get_resref("TemplateResRef"))
        if res == "invisokorpath" and "invisokorpath" not in existing_plcs:
            plcs.append(copy.deepcopy(p))
            git_gff.root.set_list("Placeable List", plcs)
            print(f"  Appended placeable 'invisokorpath' to Placeable List")
            break
            
    wps = git_gff.root.get_list("WaypointList")
    stage_wps = stage_git.root.get_list("WaypointList")
    existing_wp_tags = {w.get_string("Tag") for w in wps}
    for w in stage_wps:
        tag = w.get_string("Tag")
        if tag in ["markatomb_1", "ajuntomb_1", "tulaktomb_1", "nagatomb_1", "FROM_745KOR"] and tag not in existing_wp_tags:
            wps.append(copy.deepcopy(w))
            print(f"  Appended waypoint '{tag}' to WaypointList")
    git_gff.root.set_list("WaypointList", wps)
            
    git_bytes = bytearray()
    write_gff(git_gff, git_bytes)
    live_erf.set_data("701kor", ResourceType.GIT, bytes(git_bytes))
    
    # 2. Update 701kor.are: OnEnter -> new_701area_ent
    are_res = live_erf.get("701kor", ResourceType.ARE)
    are_gff = read_gff(are_res)
    are_gff.root.set_resref("OnEnter", ResRef("new_701area_ent"))
    are_bytes = bytearray()
    write_gff(are_gff, are_bytes)
    live_erf.set_data("701kor", ResourceType.ARE, bytes(are_bytes))
    print(f"  Updated 701kor.are OnEnter hook to 'new_701area_ent'")
    
    # 3. Update kreia_kr.dlg: EntryList[14] Quest kor_purebld1
    dlg_res = live_erf.get("kreia_kr", ResourceType.DLG)
    dlg_gff = read_gff(dlg_res)
    entry_list = dlg_gff.root.get_list("EntryList")
    entry_list[14].set_string("Quest", "kor_purebld1")
    entry_list[14].set_uint32("QuestEntry", 10)
    dlg_gff.root.set_list("EntryList", entry_list)
    dlg_bytes = bytearray()
    write_gff(dlg_gff, dlg_bytes)
    live_erf.set_data("kreia_kr", ResourceType.DLG, bytes(dlg_bytes))
    print(f"  Updated kreia_kr.dlg EntryList[14] quest to 'kor_purebld1' (10)")
    
    # 4. Inject 20 stage-only files
    injected_count = 0
    for r in stage_erf:
        if not live_erf.has(r.resref.lower(), r.restype):
            live_erf.set_data(r.resref, r.restype, r.data)
            injected_count += 1
    print(f"  Injected {injected_count} new resources into {live_path.name}")
    
    write_erf(live_erf, live_path)
    print(f"  Saved merged {live_path.name} successfully.")

def merge_module_710kor():
    print("\n--- Merging 710KOR.MOD ---")
    live_path = find_file(LIVE_MOD_DIR, "710KOR.MOD")
    stage_path = find_file(STAGE_MOD_DIR, "710kor.mod")
    
    live_erf = read_erf(live_path)
    stage_erf = read_erf(stage_path)
    
    # 1. Update 710kor.git: add kortempclick placeable and from_770 waypoint
    git_res = live_erf.get("710kor", ResourceType.GIT)
    git_gff = read_gff(git_res)
    stage_git = read_gff(stage_erf.get("710kor", ResourceType.GIT))
    
    plcs = git_gff.root.get_list("Placeable List")
    stage_plcs = stage_git.root.get_list("Placeable List")
    existing_plcs = {str(p.get_resref("TemplateResRef")) for p in plcs}
    for p in stage_plcs:
        res = str(p.get_resref("TemplateResRef"))
        if res == "kortempclick" and "kortempclick" not in existing_plcs:
            plcs.append(copy.deepcopy(p))
            git_gff.root.set_list("Placeable List", plcs)
            print(f"  Appended placeable 'kortempclick' to Placeable List")
            break
            
    wps = git_gff.root.get_list("WaypointList")
    stage_wps = stage_git.root.get_list("WaypointList")
    existing_wps = {w.get_string("Tag") for w in wps}
    for w in stage_wps:
        tag = w.get_string("Tag")
        if tag == "from_770" and "from_770" not in existing_wps:
            wps.append(copy.deepcopy(w))
            git_gff.root.set_list("WaypointList", wps)
            print(f"  Appended return waypoint 'from_770' to WaypointList")
            break
            
    git_bytes = bytearray()
    write_gff(git_gff, git_bytes)
    live_erf.set_data("710kor", ResourceType.GIT, bytes(git_bytes))
    
    # 2. Inject 3 stage-only files
    for r in stage_erf:
        if not live_erf.has(r.resref.lower(), r.restype):
            live_erf.set_data(r.resref, r.restype, r.data)
            print(f"  Injected new resource: {r.resref}.{r.restype.extension}")
            
    write_erf(live_erf, live_path)
    print(f"  Saved merged {live_path.name} successfully.")

def main():
    print("=" * 60)
    print("Phase 4: Merging 7 Colliding Base-Game Modules")
    print("=" * 60)
    
    merge_module_204tel()
    merge_module_221tel()
    merge_module_302nar()
    merge_module_303nar()
    merge_module_601dan()
    merge_module_701kor()
    merge_module_710kor()
    
    print("\n" + "=" * 60)
    print("Phase 4 Module Merges Complete!")
    print("=" * 60)

if __name__ == "__main__":
    main()
