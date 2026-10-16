# Holster C5: weapon identity at the 0x504EF5 hook (swkotor2.exe 7751f7d8, REA ghidra)

Tags: VERIFIED = tool output this run (tool + addr). GUESS = inferred. UNKNOWN = not resolved.
Builds on c3/c4 (not re-derived): patch_manager_mods/ghidra_knowledge/rea_findings/20261015_holster-c3-nodeclass.tsv, 20261016_holster-c4-weapon-attach.tsv
Ledger: patch_manager_mods/ghidra_knowledge/rea_findings/20261016_holster-c5-weapon-identity.tsv

## (a) Child Gob P: is there a model resref?
1. VERIFIED Gob ctor FUN_00458B70 (entry 0x458b70; procedure_pseudo_code 0x458bd6): `*in_ECX = Gob::vftable`; param copied into Gob+0x64 (`FUN_0091cff0(in_ECX+0x19, local_58)`).
2. VERIFIED Gob+0x64 is an INLINE char name, not a pointer. FUN_00458AE0 (0x458ae0) binary-searches the Gob registry with `__stricmp(*(entry)+100, param_1)`.
3. VERIFIED model link is Gob+0x84. FUN_00462320 (0x462320, model creator; xref 0x462484): `local_24 = FUN_0048e160(param_1)` (resref lookup), then `local_7c[0x21] = local_24`.
4. VERIFIED model name use: `param_2 = (char *)(local_24 + 8)` becomes the default Gob name. So model+0x08 is an inline char* name. GUESS: it holds the resref.
5. VERIFIED root node: `local_20[0x26] = FUN_00456cd0(model+0x28, Gob)` (Gob+0x98 = root). Then `*(root+0x40) = Gob`, the owner back-pointer (C3 node layout).
6. VERIFIED other model type (not weapon): FUN_00464500 (Scene) sets `in_ECX[0x67]` (Gob+0x19C) from `FUN_004528c0("c_FocusGobDummyModel%d")`.
7. Answer: P does not hold a resref string itself. Chain: P+0x84 -> model -> +0x08 name (inline char*, GUESS resref). Weapon-specific use: UNKNOWN.

## (b) 0x7637E0 weapon attach
1. VERIFIED signature: `FUN_007637e0(undefined4 param_1, char *param_2, int *param_3)`. param_3 = creature, param_2 = event string ("AppearanceForceUpdate" / "Loading"). No weapon item object is passed in.
2. VERIFIED slot records: `FUN_0076e610(0x10)` gives a handle, `FUN_0073f530` resolves it (wrapper of FUN_0078bd80). Creature+0x278 = right handle (`local_2c[0x9e]`), +0x27C = left (`[0x9f]`).
3. VERIFIED rhand asm (offsets are authoritative; Ghidra local_N labels differ by 4):
   - 0x763B1B `8B 45 C0` MOV EAX,[EBP-0x40] (slot record). 0x763B1E `8B 48 14` MOV ECX,[EAX+0x14] (attach handle).
   - 0x763B21 `89 8D 60 FF FF FF` MOV [EBP-0xA0],ECX (handle saved).
   - 0x763B27 `51` PUSH ECX as a reserve; 0x763B28 `D9 45 C4` FLD [EBP-0x3C] (scale); 0x763B2B `D9 1c 24` FSTP [ESP]; 0x763B2E `6A 01` PUSH 1. GUESS: the reserve is for the float.
   - 0x763B10 `E8 1B 9D 08 00` CALL FUN_007ED830 (singleton); 0x763B15 `89 85 3C FF FF FF` stores EAX at [EBP-0xC4].
   - Args past 0x763B30 (the "rhand" string and vtbl+0x7C call) are not decoded. Arg order: GUESS.
4. VERIFIED slot record fields in pseudo: +0x10 = current weapon object (`OnClose(local_44+0x10)` on replace; `vtbl+0x98(0xff,1)` on it); +0x14 = attach handle; +0x164 written as `local_2c[1]` (creature object id).
5. Base-item id source: VERIFIED `FUN_0071B670(*(u16 *)(creature[0x89] + 0x18), local_58, &scale)` with literal "WEAPONSCALE". Scale defaults to 1.0 on failure. GUESS: creature+0x224 is the weapon item object. UNKNOWN.
6. Newly attached Gob: UNKNOWN. Candidates: replace path `local_30 = (*(slot+0x10))->vtbl[0x98](0xff,1)`; new path `local_1c = creature->vtbl[0x98](0xf7,1)->vtbl[0x70]()`. The attach itself runs inside singleton vtbl+0x7C, not decoded.
7. Hook candidate H-A: stolen bytes `8B 45 C0 8B 48 14` at 0x763B1B (6 bytes, two MOVs, no relative branch). Resume at 0x763B21. Put a 5-byte JMP at 0x763B1B plus 1 NOP. The stub replays both MOVs. At the hook, EAX = slot record, ECX = attach handle. It gives the slot, not the new Gob. Weapon identity from here needs creature+0x224+0x18 (GUESS).

## (c) Back-pointers
1. VERIFIED node owner: `*(root+0x40) = Gob` in FUN_00462320 (node +0x40 = owner Gob, C3). Root points to the Gob, not to an item.
2. PROBABLE: slot+0x164 = creature object id (value, not pointer) in the replace path (0x7637E0 `*(local_44+0x164) = local_2c[1]`). Identity of local_44 is GUESS.
3. UNKNOWN: no item pointer written into Gob by 0x458B70, 0x462320, or 0x7637E0.

## Open
1. FUN_0048E2A0 (numeric branch `param_3 != 0` of FUN_00462320) and its callers. This may be the weapon path (item or appearance id).
2. What singleton vtbl+0x7C (from FUN_007ED830) does with the handle, and where P is set.
3. Model+0x08 content (resref vs tag). FUN_0048E160 not decoded.
4. Creature vtbl+0x98 ids 0xf7 / 0xfb / 0xff, and creature+0x224 writer.
5. Gob+0x164 writer identity; FUN_0076E610 slot creation.

## Corrections
- None to c3/c4. c4 "vtbl+0x7c(weapon+0x14, rhand, 1, scale)" is confirmed by asm 0x763B1E.
