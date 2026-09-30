#!/usr/bin/env python3
"""Persist the Mods 6-8 batch-3 findings (2026-09-30, full-program dump analysis, no new Ghidra run) into
ghidra_knowledge/kotor2_ghidra_knowledge.db: functions, name_map (high) and lessons.
Idempotent (upserts; lessons de-duplicated by topic). Evidence: dump_excerpts/<addr>.{c,asm} in this folder."""
import os, sqlite3, sys

HERE = os.path.dirname(os.path.abspath(__file__))
DB = os.path.join(HERE, "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
HASH = "6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef"
DATE = "2026-09-30"
SRC = "mods678-batch3-dump"
EVID = "decompile+asm from full dump (dump_excerpts/), Opus read 2026-09-30"

# (addr, symbol, purpose, notes)
FUNCS = [
    # ---- Mod 6 inventory
    ("008a7ba0", "CSWGuiInGameInventory::PopulateItemListBox", "fastcall(panel). frame: [ebp-0x84]=panel [ebp-0x40]=current creature (0x77d800) [ebp-0x38]=client party obj (0x7e5d80) [ebp-0x3c]=row counter [ebp-0x2c]=list [ebp-0x18]=item [ebp-0x20]=reselect idx",
     "equipped loop 8a7c85..8a7db7, backpack loop 8a7dc5..8a7e71, finalise CALL 41fc30 @8a7e8c. Splice windows validated by check_windows.py"),
    ("008a75f0", "CSWGuiInGameInventory::CreateItemEntry", "thiscall(panel; creatureObj, int* counter, list*, item, slotFlag, equipped) RET 0x18",
     "row = ((int**)*(panel+0x24b0))[counter] before call; ends *counter += 1; calls SetItem(item+0x14, equipped, item+0x2c8>>7&1)"),
    ("008a8df0", "CSWGuiInGameInventory::CheckFilter", "thiscall(panel; item, creature, slot) RET 0xC; mode byte at GuiInGame+0xfc1",
     "mode 5 = 0x58ae90(creature)->0x77b670(item,slot,0xff) && 0x5d2f80(creature)(item,0,1). decompile hid the ECX values"),
    ("008a8b30", "CSWGuiInGameInventory::SetNextFilter", "thiscall(panel, mode 0..11); writes mode, highlights tab for 0,6..11, panel+0x24d0|=4", "no tab highlight for modes 1-5"),
    ("008a8cf0", "CSWGuiInGameInventory::OnFilterTabClicked", "thiscall(panel, ctrl) RET 4; mode=ctrl[0x17] (ctrl+0x5c) written to GuiInGame+0xfc1; clears 7 tab highlights, highlights clicked, panel+0x24d0|=4",
     "shared by all 7 BTN_* tabs (ctor 8a6170 binds event 0x27). epilogue for consumed-exit = 0x8a8de2 (MOV ESP,EBP;POP EBP;RET 4)"),
    ("005d2f80", "CSWSCreature::CanUseItem", "thiscall(creature; item, unused, checkEquip) RET 0xC; false if this[0x43a]==1 (IsPC) && ... ", "param_3 unused; CheckFilter mode 5 passes (item,0,1)"),
    ("0077b670", "GuiInGame::IsItemUsableByCurrent", "thiscall(client party obj; item, slot, mask=0xff) RET 0xC; 0 => row click shows NotUseable/CantEquip popup",
     "uses 0x77d800 current creature and 0x7aae00; called by CreateItemEntry and CheckFilter mode 5"),
    ("0077d800", "ClientParty::GetCurrentCreature", "thiscall(partyObj) -> server creature the inventory is showing", "Populate: 73fb90(ECX=[a1b4a4]+4)->7e5d80->77d800"),
    ("0058ae90", "Creature::GetServerObjectVt2c", "thiscall(client creature) -> 0x54a840 cached server obj +0x264 then vt+0x2c", "used to make ECX for 77b670 in CheckFilter mode 5"),
    ("00605090", "CItemRepository::AddItem", "thiscall(repo; item**, x, y, bool newFlag) RET 0x10; newFlag sets item+0x2c8 bit7 and repo+4++",
     "callers with newFlag=1 = acquire paths"),
    ("00605e80", "CItemRepository::RemoveItem", "thiscall(repo, item): scans id array (repo+0xc, count +0x10), clears bit7 and repo+4--", ""),
    ("00605f40", "CItemRepository::ClearNewItems", "thiscall(repo, {ids*,count}*): clears bit7 on each, repo+4-=; only caller 8a7520 (OnPanelRemoved)", "closing the inventory clears all New flags"),
    ("00615d60", "CSWSItem::AddUpgrade", "thiscall(item, upgradeItem, slot): appends upgrade's 0x1c-byte property entries (item+0x294, count +0x28c) tagged +0x12=slot+1", "does NOT touch +0x2f4; installed upgrade id item+0x2f4+slot*4 (-1 empty) is written by 0x8cdb00, which never fills slot 1 of a type-1 item"),
    ("00615e90", "CSWSItem::RemoveUpgrade", "thiscall(item, upgradeItem, slot): drops property entries with +0x12==slot+1", ""),
    ("006076d0", "CSWSItem::GetUpgradeType", "thiscall(ECX=item; one UNUSED stack arg) RET 4 (decompile label fastcall was wrong; asm 8c9cf3 pushes [ebp+0x10]): 0 if item+0x2f0<=0; by baseitem+0xac class: 0..2->3, 3..5->2, 0x27..0x29->1, 0x1f..0x26->4", "GUI slot count via 0x8cb230: type1=6 type2=3 type3=3 type4=2"),
    ("008cb230", "CSWGuiUpgrade::GetSlotCount", "fastcall(panel): switch(panel+0x3d2c) 1->6 2->3 3->3 4->2", ""),
    ("008b1d00", "CSWGuiInGameItemEntry::SetCanUse", "thiscall(entry, byte mode) RET 4: mode 0/1 teal, 2/3 = (0.74,0.11,0.0) red-orange at entry+0x98/0x1fc/0x110/0x274; entry+0x3b0=mode",
     "only vanilla caller is 8ac330 (other panel); inventory never calls it"),
    ("008b1250", "CSWGuiInGameItemEntry::SetItem", "thiscall(entry; itemId, equipped, newFlag) RET 0xC: entry+0x1d0=id, [0x3ac] bit1=equipped bit2=new", ""),
    ("007fdfa0", "CSWCItem::GetName", "thiscall(clientItem; CExoString* out, bool identified) RET 8; name cstring at +0x18c", "name sort deferred (v1 sorts by base item id)"),
    ("0073f750", "ClientApp::GetGuiInGame", "fastcall(clientApp=[a1b4a4]+4) -> *(this+4)+0x40 (mode byte at +0xfc1)", ""),
    ("00601740", "CSWSItem::LoadFromGff", "reads BaseItem, UpgradeSlot0..5 (item+0x2f4..0x308), NewItem (bit7), DELETING (bit8)", "confirms New flag persists in saves"),
    ("00602dd0", "CSWSItem::SaveToGff", "writes NewItem from item+0x2c8>>7", ""),
    # ---- Mod 7 containers
    ("00742a00", "CSWGuiTargetActionMenu::UpdateNameLabel", "thiscall(menu, obj) RET 4: Empty/Nothing suffix for HUD *target* (menu target), NOT hover", "chain: obj->vt[0x4c]() ->0x842d00 -> sp; sp+0x37c; sp->vt[0x48]() -> 0x5f67b0(0)"),
    ("00743540", "CSWGuiTargetActionMenu::Update", "thiscall(menu; a, b, targetObj) RET 0xC -> UpdateNameLabel", "sole caller 74cfe0"),
    ("0074cfe0", "CSWGuiMainInterface::UpdateTargetMenu", "fastcall(hud): target = 0x73f4d0(HUD+0x68) -> vt[0xc]; runs from Draw 74b3f0 and 7b3ca0", "HUD+0x68 = selected target id"),
    ("00796b50", "SetHudTarget", "cdecl(obj*, x) -> 7ce710 -> 74cfb0 writes HUD+0x68", "non-zero setter only 798d70 (click/select); others clear (0,0)"),
    ("00798d70", "OnObjectSelected", "thiscall(this; obj, a, b) RET 0x10: sets HUD target then selection highlight", "callers 79b700, 79a4c0"),
    ("007aa460", "CSWCArea::HiliteAreaObjects", "thiscall(area, on): loop area+0x210 ids (count +0x214); obj=0x73f4d0(id)->vt[0xc]; type byte obj[2]; case 9 colour (0.3,1,1) at [ebp-0x64]; vt+0xa4(&rgb,2,0.0f,1)/vt+0xa8(2)",
     "only call in exe = 0x797eae with on=0 (E8/E9/data-ref scan) => on=1 unreachable in vanilla"),
    ("007b3a00", "CClientExoAppInternal::HandleMouseOver", "thiscall(app; hoveredId, pos*, x) RET 0xC: builds actions 7b40f0; cursor 0x2d (select) unless hovered==HUD target, then ActionIconToMouseCursor(list[0].id)", "app+0x4b0 hovered id, +0x4cc action list (0x3c-byte rows: +8 id,+0xc cb,+0x1c target,+0x20 icon)"),
    ("007b3870", "CClientExoAppInternal::SetMouseCursor", "thiscall(app; id, a, b) RET 0xC; callers 7b3a00, 788d90", "prologue 55 8B EC 51 89 4D FC (7 bytes)"),
    ("007b3ee0", "ActionIconToMouseCursor", "cdecl(actionId, multi) -> cursor id", "only caller 7b3a00"),
    ("007b40f0", "CClientExoAppInternal::BuildHoverActions", "fastcall(app): fills action list; placeable: HasInventory -> 'i_openplace' text 0x16d Open / 0xbc36 Search via UseSearch", ""),
    ("007ac470", "CClientExoAppInternal::Update", "thiscall(app, float dt) RET 4, 15.8 KB, per-frame; called by MainLoop 781be0", "contains live hook 0x7af198 (post-combat-movement-fix, 31 bytes)"),
    ("00842d00", "CSWCPlaceable::GetServerPlaceable", "thiscall(cplaceable) -> server placeable", ""),
    ("005f67b0", "CSWSPlaceable::GetItemCount", "thiscall(sp; onlyFlagged) RET 4: 0 if !HasInventory(+0x37c); walks repo sp+0x3c4", ""),
    ("0073f4d0", "ClientApp::GetClientObject", "thiscall(clientApp=[a1b4a4]+4; id) RET 4 -> game object (vt[0xc] gives CSWCObject)", ""),
    ("0051c160", "ServerApp::GetPlaceableById", "thiscall(serverApp=[a1b4a4]+8; id) RET 4 -> 0x537d30 (type byte 9)", "object types: 4 area 5 creature 6 item 9 placeable 10 door"),
    # ---- Mod 8 ranged AI
    ("006d7390", "CSWSCreature::AIActionPhysicalAttacks", "thiscall(creature, action) RET 4. locals: los=[ebp-0xd0] (ClearLineOfSight result, 0=blocked) dist2=[ebp-0xbc] max=[ebp-0x2c] desired=[ebp-0x4c] targetObj=[ebp-0x50] tgtArea=[ebp-0x38]",
     "hold splice 0x6d83ea (this[0x14b] test), IsPC retarget gate 0x6d7c73; returns 1 in-progress, 2 failed, 3 complete"),
    ("006dab20", "CSWSCreature::EndAttackAction", "thiscall(creature, 1): clears target, vt+0x7c(10000) ..; returned with DAT_009944bc(3=complete) or b8(2)", ""),
    ("006dac70", "CSWSCreature::InitializeNewAttackTarget", "thiscall(creature; targetId, action): sets this[0x14b]=1 (+0x52c)", ""),
    ("00571570", "CSWSCreature::MaxAttackRange", "ranged: baseitem+0x34 (0->30m) + feat bonus; melee: desired+0.5", ""),
    ("005716a0", "CSWSCreature::DesiredAttackRange", "weapon-independent: 1.6 + both radii (creature targets)", "so ranged enemies only back off inside melee distance"),
    ("0056e1e0", "CSWSCreature::RangeWeaponEquipped", "thiscall(creature) -> right-hand weapon baseitem+0x1a != 0", ""),
    ("0052bb60", "CSWSArea::ClearLineOfSight", "thiscall(area; src*, dst*, out*, ..., srcObj, dstObj...) -> 1 if clear", "src/dst objects added to an ignore list; mesh list area+0x254 (0x4c stride, count +0x250)"),
    ("0052d130", "CSWSArea::LosObjectBoxTest", "AABB-filtered per-object segment test over an id list (param_1[0x14], count [0x12])", "whether creatures are in the list is unresolved statically -> Stage-0 instrumentation"),
    ("0057c990", "CSWSCreature::GetNearestVisibleEnemyInRange", "(range, ...) -> id or 0x7f000000", ""),
    ("005644b0", "CSWSCreature::AddMoveToPointAction", "17 stack args (68 bytes)", ""),
    ("00517980", "Vec3::Equal", "cdecl(a*, b*) nonzero when equal (stuck check this+0x558 vs this+0x94)", ""),
    ("0068bbe0", "Global::ExecuteCommandIsObjectPartyMember", "returns creature+0x11ac", "=> this[0x46b] is the engine 'is party member' flag (PC included)"),
    ("0058a330", "CSWSCreature::SetPartyMemberFlag", "thiscall(creature; value, apply) RET 8: writes +0x11ac; state 4 alloc +0x500 / state 2 free", "called by creature/party spawn paths"),
    ("005fe4b0", "CSWSCreature::MakePlayerCharacter", "GAMEINPROGRESS:PC path: sets stats+0x6c=1, this[0x43a]=1, 0x58a330(1,1); only caller 0x694400 = ExecuteCommandSwitchPlayerCreature", "creature+0x10e8 caches stats+0x6c (IsPC), also updated at runtime"),
    ("0077a9e0", "CSWGuiTargetActionMenu::AddStanceActions", "thiscall(menu, list*) RET 4: one row per aiscripts.2da row (skips aistate 10); CLAMPS stats+0x146 to {9,0xb,0xc,0xd} else writes 9",
     "row: +8 id (0x410/412/413/414), +0xc=0x77d890, +0x20 icon resref; jump table 0x77ae98 over state-9; splices 0x77aa5e (clamp) and 0x77acbc (row fill)"),
    ("0077d890", "StanceActionClicked", "thiscall(menu; actionId, creatureCtx) RET 8: switch 0x410..0x414 writes stats+0x146 = 9/0xb/0xc/0xd, floaty strref 0x1e33c.., sound id", "splice 0x77d8d4 for id 0x415 -> 0x10"),
    ("0074da40", "CSWGuiMainInterface::UpdateStanceAction", "fastcall(hud): maps stats+0x146 9..0xd -> 0x410..0x414 (default -1)", "optional 4th hook to show 16 as current"),
    ("00696580", "Global::ExecuteCommandSetNPCAIStyle", "writes ushort to stats+0x146 with NO range check", ""),
    ("00696190", "Global::ExecuteCommandGetNPCAIStyle", "reads ushort stats+0x146", ""),
    ("0069ac60", "Global::ExecuteCommandHasLineOfSight", "args (vSrc, vDst, oSrc, oDst) as in OEI k_trp_power.nss; passes vectors as given (no +1.5 eye height)", ""),
]

# name_map upgrade. 'high' = address is a real function entry AND its behaviour/calling convention was read from
# decompile+asm; 'medium' = the label itself is interpretive (behaviour known, name is our guess).
MEDIUM = {"0058ae90", "0077d800", "00796b50", "0058a330", "0052d130", "00517980", "0074cfe0", "007b40f0", "006dab20",
          "0077b670", "0073f750", "0073f4d0", "0051c160", "00798d70", "0074da40", "0077d890", "0077a9e0", "008cb230",
          "008a8cf0", "007ac470"}
NAME_UP = [(a, n, "medium" if a in MEDIUM else "high") for a, n, _, _ in FUNCS]

LESSONS = [
    ("general Ghidra methodology", "Full-program dump first (mods678 batch 3)",
     "The Phase-2 dump (lesson 166, ghidra_dumps/steam_6a522e71/decomp/<bb>/<addr>.c/.asm + callgraph.tsv + strings.tsv) answered ~90% of the Mods 6-8 items with grep/sed and needed ZERO new Ghidra runs. Use copy_dump_excerpts.sh ADDR... to copy cited functions into mods678_research/dump_excerpts/ so evidence lives in the repo. Scan the PE bytes with python (E8/E9 rel32 and 4-byte data refs) to prove a function has no other callers, e.g. HiliteAreaObjects(on=1) is never called."),
    ("kotor-patch-manager framework", "consumed_exit_address lets a detour skip code",
     "The installed KotorPatcher.dll supports hooks.toml consumed_exit_address (wrapper_x86.cpp): the wrapper runs the stolen original bytes FIRST (unless skip_original_bytes=true), then PUSHFD; TEST EAX,EAX; if the handler returned non-zero it POPFD+JMPs to consumed_exit_address, else falls through. Handler return must be kept with exclude_from_restore=[\"eax\"]. Consequences: (1) with skip=false the consumed target must be reachable with the original bytes already applied; (2) to return early from an entry hook, jump to the function's own epilogue (MOV ESP,EBP;POP EBP;RET n) not a bare RET; (3) windows that are never re-run (skip=true) may contain relative CALLs. No live hook uses it yet (checked patch_config.toml)."),
    ("Mod 6 inventory", "Sort/filter splice design (definitive)",
     "Backpack rows: buffer at 0x8A7E22 (79-byte skip window incl. reselect logic + CALL CreateItemEntry) then flush sorted before the finalise call at 0x8A7E76 (22-byte window). Tab handler 0x8A8CF0 shared by all tabs (mode = ctrl[0x17]); re-click = compare with GuiInGame+0xfc1. LBL_FILTER (panel+0x16b8) is the static caption 'Filter' (strref 48548), never rewritten by code. SetCanUse(entry,2) tints (0.74,0.11,0); SetItem rewrites all row colours every call, so never call SetCanUse(row,0). New flag persists ('NewItem' GFF field); ClearNewItems on panel close clears only items hovered/entered (panel+0x24bc list). Installed upgrades: item+0x2f4+slot*4, 0xFFFFFFFF = empty; slots by GetUpgradeType (1:6, 2:3, 3:3, 4:2)."),
    ("Mod 7 containers", "KOTOR2 is a two-step interface; Empty label is selection-only; highlight ON is dead code",
     "HandleMouseOver 0x7b3a00 shows cursor 0x2d (select) unless the hovered id equals the HUD target (HUD+0x68), and only then the action cursor. UpdateNameLabel (Empty/Nothing) runs only for the HUD target set by OnObjectSelected 0x798d70 => not shown on plain hover. HiliteAreaObjects has one call site (0x797eae, on=0) so the highlight-everything key does not exist in this exe; the DLL can supply the trigger by calling the engine function with on=1. Hooks: SetMouseCursor 0x7b3870 (cursor), 0x7aa747 (case 9 colour)."),
    ("Mod 8 ranged AI", "Party/PC flags on the creature; hold splice",
     "creature+0x11ac (this[0x46b]) = IsObjectPartyMember (script command returns it) and disables the 'too close' re-path; creature+0x10e8 (this[0x43a]) = IsPC (cached stats+0x6c) and enables the LOS-blocked retarget (0x57c990). this[0x14b] (+0x52c) != 0 ends the attack action (returns 3=complete) instead of approaching: hook the test at 0x6d83ea with consumed exit 0x6d83ff to hold a ranged party member that is in range but has no LOS. Return codes: 1 in progress, 2 failed, 3 complete."),
    ("Mod 8 ranged AI", "A new AI stance needs 3 engine hooks (+1 cosmetic)",
     "aiscripts.2da (label,name_strref,aistate) rows are turned into menu rows by 0x77a9e0, which CLAMPS stats+0x146 to {9,0xb,0xc,0xd} (resets to 9), fills id/icon by a jump table over state-9 (JA for >13 leaves the row empty), and 0x77d890 maps action ids 0x410..0x414 to the stats write. SetNPCAIStyle (0x696580) does not range-check. New stance 16 = data row + hooks at 0x77aa5e (clamp), 0x77acbc (row), 0x77d8d4 (click); optional 0x74db87 for the HUD current-stance marker. Icons are Override/ib_*.tga 128x128."),
    ("general Ghidra methodology", "Validate every splice window mechanically",
     "mods678_research/check_windows.py checks window length on instruction boundaries, relative branches inside a re-executed window, external branch targets into the window interior, and overlap with live hooks (live_hooks_2026-09-30.tsv) and prints TOML-ready original_bytes. Run it before writing any hooks.toml."),
    ("general Ghidra methodology", "Decompile hides implicit ECX again: CheckFilter mode 5",
     "The pretty-printed CheckFilter shows FUN_0058ae90(...) result ignored; raw asm shows ECX=creature for 0x58ae90, its result becomes ECX of 0x77b670(item,slot,0xff), and ECX=creature for 0x5d2f80(item,0,1). Always read the asm before copying a call recipe."),
]


def main():
    con = sqlite3.connect(DB)
    cur = con.cursor()
    for scope, topic, note in LESSONS:
        cur.execute('update lessons set note=? where topic=? and date=?', (note, topic, DATE))
    fcols = [r[1] for r in cur.execute("pragma table_info(functions)")]
    for addr, sym, purpose, notes in FUNCS:
        cur.execute("insert or replace into functions (binary_hash,address,symbol,size,purpose,notes,discovered_date) values (?,?,?,?,?,?,?)",
                    (HASH, addr, sym, None, purpose, ("mods678 batch3: " + notes).strip(), DATE))
    for addr, name, conf in NAME_UP:
        key = "0x" + addr
        cur.execute("insert or replace into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)",
                    (key, name, SRC, conf, EVID, DATE))
    have = {r[0] for r in cur.execute("select topic from lessons")}
    nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
    added = 0
    for scope, topic, note in LESSONS:
        if topic in have:
            continue
        cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE))
        nxt += 1
        added += 1
    con.commit()
    print("functions upserted:", len(FUNCS), " name_map upgraded:", len(NAME_UP), " lessons added:", added)
    print("counts:", [cur.execute(f"select count(*) from {t}").fetchone()[0] for t in ("functions", "name_map", "lessons")])


if __name__ == "__main__":
    main()
