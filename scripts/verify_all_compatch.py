"""
verify_all_compatch.py - Comprehensive End-to-End Verification of the Echoes of the Republic Compatch

Performs automated sanity and regression audits across:
1. dialog.tlk string bounds and key StrRefs
2. Texture dedup: no .tga/.dds + .tpc pair in override/ (the dedup_tga_tpc.py rule)
3. Module capsules (7 merged modules + 128 new standalone modules)
4. Companion dialogues (node counts)
5. Journal categories in global.jrl
6. Galaxy Map GUI controls in galaxymap_p.gui
7. Galaxy map scripts: travel stores every Echoes destination, M4-78 is off

Read-only. Checks the game at KOTOR2_ROOT (see _common.py). The expected numbers
below are those of the 154-mod Community Build plus Echoes; each check raises
AssertionError naming what differs, and the script then exits non-zero.
"""

import sys
from pathlib import Path
from pykotor.resource.formats.tlk import read_tlk
from pykotor.resource.formats.gff import read_gff
from pykotor.resource.formats.erf import read_erf
from pykotor.common.scriptdefs import TSL_FUNCTIONS
from pykotor.resource.formats.ncs import read_ncs
from pykotor.resource.type import ResourceType

_HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(_HERE.parent))  # dedup_tga_tpc.py in the repo layout
sys.path.insert(0, str(_HERE))  # _common.py, and dedup_tga_tpc.py in a flat copy
from _common import GAME_DIR, find_one_ci  # noqa: E402
from dedup_tga_tpc import dedup_override  # noqa: E402

LIVE_OVR = GAME_DIR / "override"
SHOW_GALAXY_MAP = next(i for i, f in enumerate(TSL_FUNCTIONS) if f.name == "ShowGalaxyMap")
LIVE_MODS = GAME_DIR / "Modules"

MIN_TLK_STRINGS = 136538
# strref: (what it is, text it must contain; "" = only has to exist)
KEY_STRREFS = {
    136375: ("PartySwap", ""),
    136514: ("Echoes (Coruscant)", "Coruscant"),
}
# merged module: (.git list, exact length after the merge)
MODULE_LIST_COUNTS = {
    "204tel.mod": ("Creature List", 17),
    "221tel.mod": ("WaypointList", 44),
    "302nar.mod": ("WaypointList", 50),
    # 100 since the 2026-09-24 capsule rebuild restored a mod waypoint edit (was 99)
    "303nar.mod": ("WaypointList", 100),
    "601dan.mod": ("TriggerList", 20),
    "701kor.mod": ("Placeable List", 30),
    "710kor.mod": ("Placeable List", 32),
}
MIN_MODULES = 200
# companion dialogue: (EntryList length, ReplyList length)
DIALOGUE_COUNTS = {
    "atton.dlg": (380, 414),
    "handmaiden.dlg": (437, 671),
    "kreia.dlg": (646, 755),
    "mandalore.dlg": (255, 256),
    "mira.dlg": (320, 306),
    "visasmarr.dlg": (289, 287),
}
JOURNAL_CATEGORIES = 217
GUI_CONTROLS = 25
# galaxymap_p.gui TAG: what it is
GUI_REQUIRED_TAGS = {
    "LBL_Planet_MalachorV": "Malachor V",
    "GUI_slehe_1": "Sleheyron button",
    "gui_yavin_1": "Yavin button",
}

# 003EBO_RETURN_DEST values k_sup_galaxymap.ncs must store: vanilla's minus M4-78 (7),
# plus Echoes' 0 (unkm41aa), 12 (954COR), 13 (yavstation), 14 (man_m26ad), 15 (tatm17ab).
# check_party_gui.ncs routes these when the party leaves the Ebon Hawk.
GALAXY_MAP_DESTINATIONS = {0, 1, 2, 3, 4, 5, 6, 8, 9, 12, 13, 14, 15}


def _find(directory, name):
    try:
        return find_one_ci(directory, name)
    except (FileNotFoundError, RuntimeError) as e:
        raise AssertionError(str(e)) from None


def test_tlk():
    print("Checking dialog.tlk...")
    tlk = read_tlk(_find(GAME_DIR, "dialog.tlk"))
    assert len(tlk) >= MIN_TLK_STRINGS, f"dialog.tlk: expected at least {MIN_TLK_STRINGS} strings, got {len(tlk)}"
    for strref, (what, text) in KEY_STRREFS.items():
        entry = tlk.get(strref)
        assert entry is not None, f"dialog.tlk: missing {what} StrRef {strref}"
        assert text in entry.text, f"dialog.tlk: unexpected {what} StrRef {strref} text: {entry.text!r}"
    print(f"  [PASS] dialog.tlk valid with {len(tlk)} strings.")


def test_textures():
    print("Checking texture collision dedup...")
    pairs = dedup_override(str(LIVE_OVR), dry_run=True)
    assert not pairs, (f"texture dedup: {len(pairs)} .tpc file(s) in override share a basename with a "
                       f".tga/.dds (run dedup_tga_tpc.py): {sorted(pairs)[:5]}")
    print("  [PASS] Zero .tga/.dds + .tpc duplicate pairs in override.")


def test_modules():
    print("Checking module capsules...")
    for mod, (lst_name, exp_len) in MODULE_LIST_COUNTS.items():
        erf = read_erf(_find(LIVE_MODS, mod))
        git_name = mod[:mod.index(".")].lower()
        git = read_gff(erf.get(git_name, ResourceType.GIT))
        cur_len = len(git.root.get_list(lst_name))
        assert cur_len == exp_len, f"Module {mod} {lst_name} count {cur_len} != expected {exp_len}"

    all_mods = [f for f in LIVE_MODS.iterdir() if f.suffix.lower() == ".mod"]
    assert len(all_mods) >= MIN_MODULES, f"Modules/: expected >= {MIN_MODULES} modules, found {len(all_mods)}"
    print(f"  [PASS] All {len(MODULE_LIST_COUNTS)} merged modules and {len(all_mods)} total module capsules verified.")


def test_dialogues():
    print("Checking companion dialogues...")
    for d, (exp_e, exp_r) in DIALOGUE_COUNTS.items():
        g = read_gff(_find(LIVE_OVR, d))
        e = len(g.root.get_list("EntryList"))
        r = len(g.root.get_list("ReplyList"))
        assert e == exp_e, f"{d} EntryList length {e} != {exp_e}"
        assert r == exp_r, f"{d} ReplyList length {r} != {exp_r}"
    print(f"  [PASS] All {len(DIALOGUE_COUNTS)} companion dialogues verified with exact node counts.")


def test_journal_and_gui():
    print("Checking global.jrl and galaxymap_p.gui...")
    jrl = read_gff(_find(LIVE_OVR, "global.jrl"))
    cats = jrl.root.get_list("Categories")
    assert len(cats) == JOURNAL_CATEGORIES, f"global.jrl categories {len(cats)} != {JOURNAL_CATEGORIES}"

    gui = read_gff(_find(LIVE_OVR, "galaxymap_p.gui"))
    ctrls = [c.get_string("TAG") for c in gui.root.get_list("CONTROLS")]
    assert len(ctrls) == GUI_CONTROLS, f"galaxymap_p.gui controls {len(ctrls)} != {GUI_CONTROLS}"
    for tag, what in GUI_REQUIRED_TAGS.items():
        assert tag in ctrls, f"galaxymap_p.gui: missing {what} ({tag})"
    print("  [PASS] global.jrl and galaxymap_p.gui verified.")


def stored_destinations(ncs):
    """The CONSTI n values pushed right before the "003EBO_RETURN_DEST" name, i.e.
    the n of each SetGlobalNumber("003EBO_RETURN_DEST", n)."""
    ins = ncs.instructions
    return {ins[i].args[0] for i in range(len(ins) - 1)
            if ins[i].ins_type.name == "CONSTI" and ins[i + 1].ins_type.name == "CONSTS"
            and ins[i + 1].args == ["003EBO_RETURN_DEST"]}


def test_galaxy_map_scripts():
    print("Checking galaxy map scripts...")
    dests = stored_destinations(read_ncs(_find(LIVE_OVR, "k_sup_galaxymap.ncs")))
    assert dests == GALAXY_MAP_DESTINATIONS, (
        f"k_sup_galaxymap.ncs stores destinations {sorted(dests)}, expected {sorted(GALAXY_MAP_DESTINATIONS)}: "
        f"missing {sorted(GALAXY_MAP_DESTINATIONS - dests)}, unexpected {sorted(dests - GALAXY_MAP_DESTINATIONS)} "
        f"(phase 6 deploy_galaxy_map)")
    consts = [i.args[0] for i in read_ncs(_find(LIVE_OVR, "a_galaxymap.ncs")).instructions if i.ins_type.name == "CONSTS"]
    assert "000_M478_Found" not in consts, "a_galaxymap.ncs still makes M4-78 depend on 000_M478_Found (phase 3)"
    # a_galaxy_map.ncs is the prologue's (001EBO) map script, not a copy of the travel script:
    # it uses no globals and opens the map. The travel script there leaves the prologue map shut.
    prologue = read_ncs(_find(LIVE_OVR, "a_galaxy_map.ncs")).instructions
    globals_used = [i.args[0] for i in prologue if i.ins_type.name == "CONSTS"]
    assert not globals_used, (
        f"a_galaxy_map.ncs uses {globals_used[:3]}: it is the travel script, not Echoes' prologue script, "
        "so the prologue galaxy map never opens (restore Echoes' Override/a_galaxy_map.ncs)")
    assert any(i.ins_type.name.startswith("ACTION") and i.args[0] == SHOW_GALAXY_MAP for i in prologue), \
        "a_galaxy_map.ncs never calls ShowGalaxyMap (prologue galaxy map)"
    print("  [PASS] Galaxy map travel stores every Echoes destination; M4-78 off; prologue map script intact.")


def main():
    print("=" * 60)
    print("Echoes of the Republic Compatch: End-to-End Verification")
    print(f"Game: {GAME_DIR}")
    print("=" * 60)

    test_tlk()
    test_textures()
    test_modules()
    test_dialogues()
    test_journal_and_gui()
    test_galaxy_map_scripts()

    print("\n" + "=" * 60)
    print("ALL AUDIT CHECKS PASSED: 100% HEALTHY COMPATCH")
    print("=" * 60)

if __name__ == "__main__":
    main()
