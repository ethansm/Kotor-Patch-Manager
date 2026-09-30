#!/usr/bin/env python3
"""Persist the Mods 6+8 0.2.0 build-phase findings (2026-09-30 evening) into ghidra_knowledge/kotor2_ghidra_knowledge.db.
Idempotent: lessons by topic; name_map rows (steam_addr unique) are added only where no other source named the address."""
import os, sqlite3
DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-09-30"
SRC = "mods68-0.2.0-asm"
LESSONS = [
    ("inventory", "0x5BD6A0 = can-equip-in-slot test (use it for 'can this character equip')",
     "thiscall(ECX=server creature; item, u32* slot, int feedback, int p5, int feedbackMsg) RET 0x14, result in AL. Push order at 0x8AC77A: "
     "0,0,0,&slot,item; ECX=[ebp-0x4c]=0x77D800 creature. *slot must be ONE slot bit: it dispatches 0x10/0x20 -> 0x5BD1D0, 0x40000/0x80000 -> 0x5BD3E0, "
     "else 0x5BD610 (slot occupied still returns 2 = ok), and ends by requiring baseitem+4 & *slot. It may rewrite *slot (0x20->0x10 etc.) and restores it; "
     "pass a fresh copy per bit. Feedback 0x584DA0 only when p4==1 && p6==1 (pass 0s = silent, pure query). Checks item+0x2C8 bit 6, feats (0x5BDA40), "
     "0x5D3480/0x5D32C0/0x5D3650/0x5D3A20/0x5D3B60/0x5D3C90/0x5D3EE0/0x5D4190. 0x8AC330 also applies a gender test first: baseitem+0xB4 1/2 vs "
     "*(u16*)(stats+0xE0) == *(u16*)0x99A8BC / 0x99A8B8. Mod 6 0.2.0 canEquip() = gender + any slot bit passing 0x5BD6A0."),
    ("stance-menu", "Party stance menu: row layout, click tail, tutorial arg",
     "AddStanceActions 0x77A9E0: common path fills row name (row+0), callback 0x77D890 (row+0xC..), row+0x1C, row+0x38=1; the jump table 0x77AE98 "
     "(aistate 9..13) case bodies set only row+8 = action id 0x410/.. and row+0x20 = CResRef icon (0x710EB0 thiscall(ECX=dst; out, str): lower-case, "
     "zero-padded 16 bytes, copied to out). Icons: 9 ib_aggressive, 11 ib_ranged, 12 ib_stationary, 13 ib_jedisupport/ib_grenadier. "
     "StanceActionClicked 0x77D890: [ebp-8] = stance message strref (0x1E33C/E/F, 0x1EC3E/0x1EC66) written to GUI+0x98 -> +0x15410 with a timer float; "
     "[ebp-4] byte = tutorial.2da row for 0x740A20 (-> 0x79E9D0: 0x7CF960 option flag + 0x898EB0 'Message_PC0' lookup + not-yet-shown), "
     "0x3D aggressive, 0x3F ranged, 0x40 stationary, 0x41/0x42 support, 0xFF none. Consumed-safe hook windows: 0x77AA5E (clamp), 0x77ACBC len 9, 0x77D8D4 len 6."),
    ("kpatch-framework", "Detour stolen bytes may END in RET; stolen CMP flags survive a consumed test",
     "wrapper_x86.cpp restores EFLAGS/regs and ESP exactly before the stolen bytes (the hook site is reached by JMP, no return address), so a window "
     "that is a function epilogue (MOV ESP,EBP; POP EBP; RET n) runs its RET and returns to the caller normally; the trailing JMP back is dead code. "
     "Used by Mod 6 H6d 0x8A8478. The consumed-exit test is PUSHFD/TEST EAX,EAX/.../POPFD, so a window ending in CMP before a Jcc keeps its flags on the "
     "resume path (Mod 8 S-H1 0x77AA5E). check_windows.py now rejects a RET that is not the last stolen instruction."),
    ("combat-ai", "Vanilla Ranged/Stationary companions never approach (0x6D876A/0x6D8A7E)",
     "AIActionPhysicalAttacks 0x6D7390 copies stats+0x146 to [ebp-0x174] at 0x6D850F (MOV [EBP-0x174],EAX, 6 bytes, EAX-free, only read by the "
     "CMPs at 0x6D876A/0x6D8773/0x6D8A7E/0x6D8A87). Stance 11 or 12 and attacker != controlled creature => skip the approach move (0x5644B0) and return "
     "via 0x6D8BE2, for every approach except the too-close/term-5 case. So 'a blocked ranged companion walks into melee' holds only for stances 9/10/13. "
     "A new stance (16) must be mapped to 11 there to inherit the rule (Mod 8 E-H3 skip-only hook). Found by the Mod 8 0.2.0 Opus review."),
    ("inventory", "Row-select 0x8A83D0 resets row colours only for New items",
     "Event handler thiscall(panel; row). If the row's item has the New bit (item+0x2C8 bit 7) it clears it, clears row+0x3AC bit 2 and calls 0x8B1CC0 "
     "(fastcall(row): SetItem 0x8B1250(row+0x1D0 id, 0x3AC bit1, bit2)), which rewrites the four colour triplets (teal) and drops a SetCanUse tint. "
     "SetItem never writes row+0x3B0. Row-enter 0x8A8100 does not touch colours. All paths of 0x8A83D0 end at the epilogue 0x8A8478 (Mod 6 H6d re-tints there)."),
]
NAMES = [
    ("0x005bd6a0", "CSWSCreature::CanEquipItemInSlot?", "can-equip test (see lesson)"),
    ("0x005bd1d0", "CanEquip_WeaponHands? (slots 0x10/0x20)", "sub-check of 0x5BD6A0; may rewrite *slot to 0x10"),
    ("0x005bd3e0", "CanEquip_WeaponHands2? (0x40000/0x80000)", "sub-check of 0x5BD6A0"),
    ("0x005bd610", "CanEquip_OtherSlot?", "0x5BDA40 then slot-occupied test; returns 2 when occupied (still ok)"),
    ("0x00740a20", "CGuiInGame::TriggerTutorial?", "(row byte, id, id, 0) -> 0x79E9D0; row = tutorial.2da"),
    ("0x008b1cc0", "CSWGuiInventoryItemEntry::RefreshItem?", "fastcall(row) -> SetItem(row+0x1D0, flags)"),
    ("0x008a83d0", "CSWGuiInGameInventory::OnItemSelected?", "clears New bit + refreshes the row"),
]
c = sqlite3.connect(DB); cur = c.cursor()
have = {r[0] for r in cur.execute("select topic from lessons")}
nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
for scope, topic, note in LESSONS:
    if topic in have: cur.execute("update lessons set note=? where topic=?", (note, topic)); continue
    cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); nxt += 1
for addr, name, ev in NAMES:
    row = cur.execute("select source from name_map where steam_addr=?", (addr,)).fetchone()
    if row and row[0] != SRC:
        print("name_map keeps existing", addr, row[0]); continue
    cur.execute("delete from name_map where steam_addr=?", (addr,))
    cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)", (addr, name, SRC, "medium", ev, DATE))
c.commit(); print(cur.execute("pragma integrity_check").fetchone(), "lessons:", cur.execute("select count(*) from lessons").fetchone()[0])
