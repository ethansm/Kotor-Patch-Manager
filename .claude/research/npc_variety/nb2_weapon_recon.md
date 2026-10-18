# NB2 weapon recon: right-hand weapon read from a server creature (Steam swkotor2.exe 6a522e71)

Scope: read-only. Static dumps first (`ghidra_dumps/steam_6a522e71/decomp/<2hex>/<addr>.asm|.c`, `callgraph.tsv`, `strings.tsv`, `steam_docs/CSWSCreatureStats_validated.tsv`). One live REA session (provider ghidra, `backups/vanilla_clean/swkotor2.exe`, sha256 6a522e71...), used only for data xrefs and the vtable question, then closed. This agent has no Bash tool, so `query_ghidra.py`, `static_facts.py` and `agent_contract.py` were not run. Ledger: `patch_manager_mods/ghidra_knowledge/rea_findings/20261018_nb2-weapon-recon.tsv` (29 rows).

## Q1. How ECX reaches GetItemInSlot

Chain inside GetMeleeAttackBonus 0x006a7db0 (`this` = CSWSCreatureStats*, `RET 0x10`):
- `[this+0x24]` = owner object S (CSWSCreature). Asm 0x006a7ddf `MOV ECX,dword ptr [EAX + 0x24]`. The validated doc (`steam_docs/CSWSCreatureStats_validated.tsv:51`) also says +0x24 is the owning object.
- `[S+0x1150]` = CSWInventory*. Asm 0x006a7de8 `MOV EAX,dword ptr [EDX + 0x1150]`, then `CALL 0x006d0620` with ECX = that pointer.
- Right hand (mask 0x10): 0x006a7e00 to 0x006a7e1d: `MOV EDX,[ECX+0x24]` ... `MOV ECX,[EAX+0x1150]`; `PUSH 0x10` at 0x006a7e18; `CALL 0x006d0620` at 0x006a7e1d.
- Left/off hand (mask 0x20): 0x006a7dd9 to 0x006a7df6, same chain, `PUSH 0x20` at 0x006a7df1, `CALL` at 0x006a7df6.
- Mask select (asm 0x006a7dcd to 0x006a7dd7): 0x10 if param_1 == 0 or param_4 != 0, else 0x20.
- Directly from S: inventory = `[S+0x1150]`. The stats pointer is not needed for the read.
- Cross-check: `decomp/57/0057e3f0.c:12` tests `*(param_1 + 0x1150) != 0` before calling GetItemInSlot.

## Q2. Calling convention, return value, side effects

- 0x006d0620: thiscall, ECX = CSWInventory*, one stack arg (slot mask) at [EBP+8], `RET 0x4` (asm 0x006d0661 `C2 04 00`).
- Return EAX = item object pointer, or 0. It is not an object id: the id is read from the slot and consumed inside the call.
- Path: 0x006d0670(inv, mask) returns the slot address P or 0 (CALL at 0x006d0630). If P != 0, `MOV ECX,[EAX]` reads the object id (0x006d064d), then `CALL 0x0051c0e0` with ECX = `[[0x00a1b4a4]+8]` (0x006d063e to 0x006d0653).
- 0x0051c0e0: ECX = `[mgr+4]`, then `CALL 0x00537b00` (0x0051c0ee, 0x0051c0f1).
- 0x00537b00: table = `[holder+0x1005c]` (0x00537b13). `CALL 0x0053dfb0` (0x00537b27) is the hash lookup, read-only per ledger row 8 (`rea_findings/20261016_nb2-recon.tsv`). If found, out != 0 and `byte[out+8] == DAT_00994475` (6 = item, ledger row 22), it returns `vt+0x38` of the object (0x00537b5a `MOV EAX,[EDX+0x38]`, 0x00537b5d `CALL EAX`, no stack args in the asm). Otherwise 0. `RET 0x4` at 0x00537b66.
- Side effects in the four bodies (0x006d0620, 0x006d0670, 0x0051c0e0, 0x00537b00): only stack-frame writes. No allocation. The only non-trivial effect is the virtual call in 0x00537b00. Its target is not proven (see OPEN).
- The decompile of 0x00537b00 (`decomp/53/00537b00.c:14`) shows two args to the virtual; the asm pushes none. The asm was used.
- Candidate target: secondary vtable 0x009972cc (COL 0x009bc44c, offset 0x10). Its slot +0x38 (0x00997304) = 0x00600070, which is `return param_1 + -0x10` (`decomp/60/00600070.c:6`). This fits a table that holds a subobject pointer, but I could not prove that item objects use this vtable. The primary CSWSItem COL 0x009bc38c has one data xref, 0x009973ac, which is a registry record (COL, dtor 0x006000b0, "UTI "), not a vtable.

## Q3. Slot mask names

- Mask to offset (decomp `6d/006d0670.c`, lines 6 to 68, all in `CSWInventory`): 0x1:+4, 0x2:+8, 0x4:+0xC, 0x8:+0x10, 0x10:+0x14, 0x20:+0x18, 0x40:+0x1C, 0x80:+0x20, 0x100:+0x24, 0x200:+0x28, 0x400:+0x2C, 0x4000:+0x30, 0x8000:+0x34, 0x10000:+0x38, 0x20000:+0x3C, 0x40000:+0x40, 0x80000:+0x44. Masks 0x800, 0x1000, 0x2000 and others return 0.
- Bit index = script slot number: `decomp/57/0057e3f0.c:13-14` loops i = 0..15 and calls `FUN_006d0620(1 << i)`. `decomp/62/00622930.c:26` does the same from a script int.
- Names come from the repo scratch copy `.claude/scratch_npcvar/nwscript/nwscript.nss` (not from the binary, so probable): INVENTORY_SLOT_RIGHTWEAPON = 4 (line 44), INVENTORY_SLOT_LEFTWEAPON = 5 (line 45), RIGHTWEAPON2 = 18 (line 54), LEFTWEAPON2 = 19 (line 55).
- So: right hand = 0x10 (inv+0x14). Left/off hand = 0x20 (inv+0x18). Second weapon set: 0x40000 (inv+0x40) and 0x80000 (inv+0x44).

## Q4. Baseitem index and weapon flag

- item+0x0C = baseitem index (`0x006a7e2e MOV EAX,[EDX+0xc]`; `0x006d6e3c MOV ECX,[EAX+0xc]` in GetBaseItem 0x006d6e30).
- Baseitem table = `[[0x00a1b4d0]+0x34]` (`0x006d6e42`, `0x006d6e52`). The name of 0x00a1b4d0 is unknown.
- Record = `0x006d5200(tbl, idx)`: `[tbl+4] + idx*0xC8` when `0 <= idx < [tbl]` (0x006d521c, 0x006d5223), else 0. Record size 0xC8.
- Gate: `record+0x1A` byte != 0 makes GetMeleeAttackBonus return 0 at once (`0x006a7e52 MOVZX EDX,byte [ECX+0x1a]`, `0x006a7e63 JMP 0x006a836a`).
- Other record bytes are read but not named: +0x08 (type, tested for 2 or 4 at 0x006a82d8 to 0x006a82eb) and +0x09 (0x006a8060).
- Unarmed: right-hand item == 0 (0x006a7e25 to 0x006a7e29, `JZ 0x006a7e68`).
- Melee vs ranged: NOT separable from the dumps. +0x1A is the only gate and its meaning is unnamed. UNKNOWN.

## Q5. Client creature (CSWCCreature, vtable 0x0099EE14)

UNKNOWN. The ledger has the vtable and identity slots only (`rea_findings/20261016_nb2-recon.tsv` rows 13 to 21). I did not search the client code for a cached right-hand field, so this is not answered.

## Read recipe for the DLL (no virtual call)

1. S = 0x0051C0A0 result (ECX = manager, PUSH oid, RET 4). This does no type check, so check `[S+0x08] == 5` yourself.
2. inv = `[S+0x1150]`.
3. Right-hand oid = dword `[inv+0x14]`. Treat 0 and 0x7F000000 (DAT_00994484, ledger row 25, "invalid/none") as no weapon (probable, not confirmed for inventory slots).
4. To turn the oid into an item object without the vtable call, use the 0x0053dfb0 lookup and check `[obj+8] == 6` yourself. Its RET count was not checked.

## OPEN

Which vtable do the object-table entries use (item `vt+0x38`)? The primary CSWSItem vtable was not located. The candidate is 0x009972cc, slot 0x38 = 0x00600070 (pure this-0x10).

## Verdict

PARTIAL. Q1 to Q4 are confirmed by asm or decompile anchors. Q2's side-effect answer is incomplete because of the unproven vtable target. Q3's right-hand and left-hand names come from the nwscript source file, not the binary. Q5 is unknown.
