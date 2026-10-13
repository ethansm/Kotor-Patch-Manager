# Engine recon P2: CSWSCreature::CanEquipItem (Steam vanilla)

Binary: /home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe.unpatched_vanilla
sha256 6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef (x86 PE), provider ghidra 12.1.4, read-only.
Notation: stats = creature+0x1198 (CSWSCreatureStats). item = param_1 of CanEquipItem. CanEquipItem(item, slot*, p3, p4, p5); spawn FUN_00573950 passes (1,1,1).
"dec" = batch_decompile result. "asm" = procedure_assembly / inspect_native_instruction result.
Inferred names are marked (inferred). Anything not confirmed is marked UNKNOWN or NOT FOUND.

## 0. Call order inside CanEquipItem 0x005bd6a0 (asm-verified)

| # | Check | Code | Refuse | Message |
|---|---|---|---|---|
| A | Gate A (level vs item value tier) | 0x5bd6aa-0x5bd756 | 0 | 0x62 (only if p5==1) |
| B | item+0x2c8 bit 6 (NonEquippable) | 0x5bd75b-0x5bd76e | 0 | silent |
| C | FUN_005bda40: baseitem EquipableSlots and proficiency | 0x5bd773-0x5bd78b | 0 | silent |
| 1 | 0x2b alignment, FUN_005d3480 | 0x5bd790-0x5bd7bd | 0 | 0xcf |
| 2 | 0x2c class, FUN_005d32c0 | 0x5bd7c2-0x5bd7ef | 0 | 0xd0 |
| 3 | 0x2d/0x3d race and gender, FUN_005d3650(item,1) | 0x5bd7f4-0x5bd823 | 0 | 0xd1 |
| 4 | 0x39 feat/skill, FUN_005d3a20 | 0x5bd828-0x5bd852 | 0 | 0x6b |
| 5 | 0x3c, FUN_005d3b60 | 0x5bd857-0x5bd884 | 0 | 0xe5 |
| 6 | 0x3e tag, FUN_005d3c90 | 0x5bd889-0x5bd8b6 | 0 | 0xe6 |
| 7 | 0x45 minimum stat, FUN_005d3ee0 | 0x5bd8bb-0x5bd8e8 | 0 | 0xe8 |
| 8 | baodur list, FUN_005d4190 | 0x5bd8ed-0x5bd91a | 0 | 0xeb |
| 9 | Slot branch: 0x10/0x20 -> FUN_005bd1d0 (weapon); 0x40000/0x80000 -> FUN_005bd3e0 (armour); else FUN_005bd610 (misc) | 0x5bd91f-0x5bd9a4 | 0 | misc: 0x77/0x7b |
| 10 | Epilogue: baseitem row+4 (EquipableSlots) AND slot must be nonzero | 0x5bd9f8-0x5bda2c | 0 | silent |

- Messages in checks 1-8 fire only when p3==1 and p5==1. Gate A also needs p4==0, so it never runs on the spawn path (spawn passes p4=1).
- Return values in the branches: weapon 1/2/3, armour 1/2/4, misc 1/2. Nonzero = allowed. 2 = slot occupied, as in the brief.
- Gate A's message-0x4c block (0x5bd6d3-0x5bd6da) is unreachable: the code always jumps past it (asm).

## 1. Item property records and the active test (supports P1)

- Count at item+0x28c (list B), base at item+0x294, 28-byte (0x1c) records. FUN_006042a0(i) = record i. Source: FUN_00601740 (UTI loader, dec).
- Record: +0 ushort PropertyName (type); +2 ushort Subtype; +4 byte CostTable; +6 ushort CostValue; +8 byte Param1; +9 byte Param1Value; +0x11 byte UpgradeType.
- Active test (dec, every restriction loop): record+0x11 == 0xFF, OR bit (record+0x11 & 0x1f) is set in item+0x2d4. item+0x2d4 is the UTI "Upgrades" field (FUN_00601740).
- List split: FUN_00603ee0 (dec) returns 1 only for types 0x0A, 0x25, 0x2E, 0x35, which go to list A (+0x288, base +0x290). Every restriction type 0x2b-0x45 is in list B, which is the only list the restriction loops scan.
- Map from number to itemprops 2DA row name: NOT FOUND in the binary. The code compares raw numbers.
- Semantics: "any" = any active matching record passes. "all" = every active matching record must pass.

## P1. Meaning of properties 0x2b, 0x2c, 0x2d, 0x3d, 0x39, 0x3c, 0x3e, 0x45

Gate: each check first runs FUN_00603f60(type,0), which returns 0 if the item has no such type, so the check passes. Feat 93 (0x5d) bypasses the 0x2b, 0x2c and 0x39 checks and the proficiency check (FUN_006b83f0(0x5d)). FUN_006b83f0 is a feat-list membership test (inferred from its use on baseitem proficiency IDs).

- **0x2b (43), FUN_005d3480, any.** Creature field: stats+0x18A (short), alignment (inferred). Subtype 0 = always passes. 1 = 40 < a < 60. 2 = a >= 60. 3 = a <= 40. 4 = a == 0. 5 = a == 100. KotOR term: alignment restriction (inferred).
- **0x2c (44), FUN_005d32c0, any.** Creature field: class-entry bytes at stats+0xAB + i*0x28 for i < stats+0x8D (class count). Subtype = class ID. Compared as ushort vs class byte. KotOR term: class restriction (inferred). The branch FUN_005d4240 is a stub returning 0 (0x5d4240, dec), so its early return never fires.
- **0x2d (45), FUN_005d3650, any.** Creature field: stats+0xE0 (short), race (inferred). Pass iff subtype == stats+0xE0. KotOR term: race restriction (inferred).
- **0x3d (61), FUN_005d3650, any.** Creature field: stats+0xEC (byte), gender (inferred). Pass iff subtype == stats+0xEC, and only when race != global word 0x99A8B8 (Ghidra shows 5). Race 5 is exempt. KotOR term: gender restriction (inferred).
- **0x39 (57), FUN_005d3a20, all.** Each active 0x39 subtype S must satisfy FUN_006b8440(S) != 0. FUN_006b8440 returns 0 if S is not in the table at FUN_006a20f0 or the creature does not hold feat S; otherwise a value 0..100 from the creature's feat/skill list. Receiver not verified. KotOR term: feat or skill requirement (inferred).
- **0x3c (60), FUN_005d3b60, all.** Calls creature vtable slot +0x88 (CSWSCreature::vftable at 0x994F5C, slot bytes 0x5632B0, read_bytes). FUN_005632B0 (dec) returns stats+0x64 (byte). Rule: subtype == stats+0x64, OR subtype is 0 or 1 and stats+0x64 == 2. KotOR meaning: UNKNOWN.
- **0x3e (62), FUN_005d3c90, any.** Tag restriction (inferred from string compare). Subtype 0: passes iff creature+0x1200 != 0 (dec; asm 0x5d3dd4-0x5d3de6). Subtype N: global 2DA row N, column literal "ExpectedTag" (0x996650, asm 0x5d3dec), must stricmp-equal creature+0x18 (the object tag, asm 0x5d3e3a-0x5d3e46: ECX = creature+0x18).
- **0x45 (69), FUN_005d3ee0, all.** Minimum stat. Jump table at 0x5d4170 (asm 0x5d4062-0x5d4075). Subtype -> stats byte: 0 -> +0xED (0x5d407c); 1 -> +0xEF (0x5d40a0); 2 -> +0xF1 (0x5d40c4); 3 -> +0xF1 (0x5d40e5, asm 0x5d40f4); 4 -> +0xF5 (0x5d4106); 5 -> +0xF7 (0x5d4127). Required value = CostValue (record+6) converted via FUN_006F0190(0x1a) and FUN_0071BCF0 (conversion NOT decoded). Refuse if stat < required. Ability names: UNKNOWN.
- **Probable source defect (not confirmed as a bug):** subtype 3 reads the same byte as subtype 2 (+0xF1). The other stat bytes are on a 2-byte stride (+0xED, +0xEF, +0xF1, [+0xF3], +0xF5, +0xF7), so the expected subtype-3 byte would be +0xF3. Intent: UNKNOWN.

## P2. The "baodur" list (0x5d4190, asm-verified)

- ECX for the call is creature+0x18: asm 0x5d4196-0x5d41a4 (MOV ECX,[EBP-0x24]; ADD ECX,0x18; CALL 0x734f60). The creature object is the caller's ECX (asm 0x5bd8f0-0x5bd8f4).
- Literal "baodur" is at 0x995164 (search_strings). FUN_00734f60 (dec) = __stricmp(param, *ECX) == 0. So it is a case-insensitive compare of the creature's tag (creature+0x18, inferred) with "baodur".
- The list is the baseitem IDs compared against item+0xC (asm 0x5d41ad-0x5d4223): 0x23, 0x24, 0x25, 0x61, 0x62, 0x63, 0x64, 0x66 = 35, 36, 37, 97, 98, 99, 100, 102.
- It lists base item type IDs, not resrefs or tags. The IDs are hard-coded (no 2DA read).
- Result: if the wearer's tag equals "baodur" (ci) AND the item is one of those base types -> refuse (msg 0xEB). Otherwise allow.
- Other literals, not referenced by these functions as far as checked: "BAODUR" (0x98DA4C), "P_BaoDur_Arm" (0x991264).

## P3. item+0x2c8 bit 6

- Source: UTI GFF boolean field "NonEquippable". FUN_00601740 (UTI loader, dec): item+0x2c8 = (item+0x2c8 & 0xFFFFFFBF) | (bool << 6), with the default being the current bit.
- Not from baseitems. Not a droid or plot flag. No "DroidOnly" flag found.
- Constructor FUN_005ffd60 (dec) also clears bit 6 at creation.
- Other bits set by the same loader: bit 3 Dropable, bit 4 Pickpocketable (FUN_00602be0 and FUN_00601740), bit 5 Stolen, bit 7 NewItem, bit 8 DELETING.
- Use: CanEquipItem gate B refuses silently (asm 0x5bd75e-0x5bd76e).

## P4. Gate A (message 0x62), asm 0x5bd6aa-0x5bd756

Refuse (return 0) iff all hold:
1. p3 == 1 (asm 0x5bd6aa).
2. stats+0x6C (dword) == 1. Meaning: UNKNOWN (asm 0x5bd6b7-0x5bd6cd).
3. p4 == 0 (asm 0x5bd6fb).
4. effective level < tier. Effective level = FUN_006A4E00(stats) (dec): sum of class levels from stats+0xAC + i*0x28, with a rules-multiplier branch when stats+0x6C == 0 and creature+0x250 != 0. Tier = FUN_00606C30(item) (receiver item, asm 0x5bd716; dec): 1 + first index i in the global tier table (rules+0x130+0x118, MAXSINGLEITEMVALUE rows) whose value >= FUN_00606A80(item). FUN_00606A80 (dec): item value from AddCost (item+0x2A0) times baseitem row float (+0xC4), min 1 unless item+0x10C == 1.
5. Global flag nonzero: [0xA1B4A4]+8 -> FUN_0051BD40 -> +0xD0 (asm 0x5bd725-0x5bd740). Meaning: UNKNOWN.

Message 0x62 is shown only if p5 == 1. Text of strref 0x62: UNKNOWN.

Not found: dead/stunned/in-conversation/in-combat checks, and "no free slot" refusals (slot occupancy only returns 2/3, which are allowed).

## P5. Head / hair and GoggleHook

- Chain: FUN_0076D200 (dec) -> FUN_0085A6C0 (flag bit 2) -> FUN_0085A9E0 (head build with item).
- FUN_0085A6C0 (dec): head type = FUN_0085C150(string from appearance-row column DAT_00A0FDE0, keyed by appearance id at +0x18). If type == 3, calls FUN_0085A9E0(3, headdata). Else sets the head variant byte and calls vtable +0x7C(0xFE). The item is not an input to this type decision.
- FUN_0085C150 (dec): returns 3 on a match against DAT_00991B40; 2 on 'F'/'L' tests; else error 0xFFFFFF0B.
- procedure_callers(0x85A9E0) = {0x85A6C0} only. So the GoggleHook attach runs only in the type-3 path.
- GoggleHook attach at 0x85B703 (inspect_native_instruction: PUSH 0x9A43E0). 0x9A43E0 = "GoggleHook" (list_strings). The address is inside FUN_0085A9E0 (dec, resolved from 0x85B700).
- Attach flow (dec, FUN_0085A9E0), runs only when the head objectId changed or in_ECX+0x40 is set:
  1. Old head item (FUN_0073F530(in_ECX+0x10)) is detached: vtable 0x50/0xCC/0x40 on its model.
  2. New head item = FUN_0073F530(headdata+0x10). If NULL: silent early return, in_ECX+0x10 is not updated.
  3. parent = vtable(in_ECX+0x3C)+0x98(0xFE,1), not null-checked.
  4. model = vtable(item+0x10)+0x98(0xFF,1). If model != 0: vtable(model)+0x50(parent, "GoggleHook", 0), then +0x18("default", 1.0, 0, 0).
  5. Then vtable(FUN_007ED830())+0xEC(headdata+0x10).
- Missing GoggleHook node: the call's return value is ignored and there is no check on the node. Behavior is inside vtable+0x50 of the model class (not decompiled; class not identified). UNKNOWN: silent skip, null parent, or crash.
- Hair hide/swap keyed on the equipped item: NOT FOUND in FUN_0076D200, FUN_0085A6C0, FUN_0085A9E0 (no baseitems column, no item model type, no "helmet hides hair" flag).
- Item inputs actually read in FUN_0085A9E0: the item's model via objectId; the head index byte (headdata+0x34) for 2DA model and ALTTEXTURE lookups (texture only).
- Hair hide inside the vtable callees (+0x7C, +0xAC): UNKNOWN.

## P6. Race / gender / appearance / model-type hard blocks

Found:
- Baseitem row (FUN_006D5200 table, 200-byte rows; receiver item via FUN_006D6E30, asm 0x5d369d):
  - row+0xB4 byte: 1 -> stats+0xE0 must equal global word 0x99A8BC (Ghidra 6); 2 -> must equal global word 0x99A8B8 (Ghidra 5); other nonzero -> refuse (asm 0x5d36ab-0x5d372a).
  - Skipped if the item has property 10 with subtype 0x81 (FUN_006042E0 search, asm 0x5d3676-0x5d3696).
  - row+0xB8 dword: refuse if bit (1 << stats+0xEC) is set (gender mask; asm 0x5d377f-0x5d37c9).
  - row+4 (EquipableSlots) == 0 -> refuse (FUN_005BDA40 dec); row+4 & slot == 0 -> refuse (epilogue).
  - row+0x7C list (2-byte IDs), count row+0x80: each ID must be held (FUN_006B83F0), unless feat 93.
- Race and gender compares via 0x2d and 0x3d (P1).
- 0x3c compares stats+0x64, which may be an appearance or body type. UNKNOWN.

Not found:
- Appearance-type or model-type compare in CanEquipItem or its three branches. Weapon branch: byte at object+0x8 in {3,5,6} or {1,2,4} (FUN_006D5090/FUN_006D50E0, receiver not verified), row+0x1A. Armour branch: slot masks. Misc branch: occupancy only.
- Appearance-type or model-type compare in spawn FUN_00573950. It reads only ObjectId, EquippedRes/InventoryRes (via FUN_00601610, resref resolve), Dropable and Pickpocketable, then calls CanEquipItem(1,1,1).

## Spawn path (context)

FUN_00573950 (dec) reads GFF Equip_ItemList, then ItemList. For each entry it builds the item (FUN_005FFD60), loads it (FUN_00602BE0 when p5==0, or the EquippedRes path), then calls CanEquipItem(item, &slot, 1, 1, 1). On refusal with slot==2 it sets creature+0x11E0 = 1 and moves the item via FUN_00605090. On success it calls FUN_0056D6F0(slot, item, 1, 1, 0).

## OPEN (unconfirmed)

1. Meaning of 0x3c and stats+0x64 (KotOR name).
2. Gate A: stats+0x6C meaning and the global flag at [0xA1B4A4]+8 chain (+0xD0).
3. Behavior when GoggleHook is missing: vtable+0x50 of the head model class (not identified).
4. Text of message strefs 0x62, 0xCF-0xEB.
5. Feat 93 (0x5D) name; meaning of property 10 subtype 0x81.
6. 0x45 subtype-3 byte (+0xF1 vs expected +0xF3): intent and runtime effect.
7. Receivers (ECX) for FUN_006B83F0, FUN_006B8440, FUN_006D5090/50E0 (dec does not show ECX).
8. Map of property numbers to itemprops 2DA row names: not in binary.

## Evidence (tool calls)

- open_binary (ghidra): sha256 6a522e71...
- batch_decompile: 0x5bd6a0, 0x5bd1d0, 0x5bd3e0, 0x5bd610, 0x573950; 0x5d3480, 0x5d32c0, 0x5d3650, 0x5d3a20, 0x5d3b60, 0x5d3c90, 0x5d3ee0, 0x5d4190, 0x5bda40; 0x603f60, 0x6b83f0, 0x6b8440, 0x6d6e30, 0x6042a0, 0x6d0620, 0x6d5200, 0x6d5090, 0x6d50e0, 0x6d4fc0; 0x6a4e00, 0x606c30, 0x51bd40, 0x584da0, 0x734f60, 0x5d4240, 0x6042e0, 0x6c0aa0, 0x6a20f0, 0x51c0e0; 0x76d200, 0x85a6c0, 0x85a9e0, 0x85b700, 0x606a80, 0x6ad2f0, 0x734010, 0x71b1d0; 0x601610, 0x602be0, 0x5ffd60; 0x601740; 0x85c150; 0x5632b0; 0x603ee0, 0x604260.
- procedure_assembly: 0x5bd6a0, 0x5d4190, 0x5d3c90, 0x5d3ee0, 0x5d3650.
- inspect_native_instruction: 0x85b703.
- search_strings: "Dropable" -> 0x995354; "baodur" -> 0x995164 (and 0x98DA4C, 0x991264).
- xrefs: 0x995354. list_strings: 0x9A43E0 = "GoggleHook". find_xrefs_to_name: CSWSCreature::vftable = 0x994F5C. read_bytes: 0x994FE4 = b0 32 56 00 (slot +0x88 = 0x5632B0). procedure_callers: 0x85A9E0.
