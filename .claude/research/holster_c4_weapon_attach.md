# Holster C4: weapon attach chain and per-frame composer (swkotor2.exe 7751f7d8, REA ghidra)

Tags: VERIFIED = quoted tool output this run (tool + address). GUESS = inferred, not shown.
Build 7751f7d8 (see holster_c3_nodeclass.md). Ledger rows: patch_manager_mods/ghidra_knowledge/rea_findings/20261016_holster-c4-weapon-attach.tsv

## (a) Attach chain
1. VERIFIED 0x7637E0 (procedure_pseudo_code): for "rhand" it calls `FUN_007ed830()`, then `vtbl+0x7c(weapon+0x14, "rhand", 1, scale)`. The "rhand" push is 0x763AF9 (`68 d8 ed 99 00`, literal 0x99EDD8). The singleton call is 0x763B10 with `MOV ECX,[EBP-0x28]` at 0x763B0D (read_bytes 0x763AE0).
2. VERIFIED FUN_007ed830 = `MOV EAX,[ECX+0x68]; RET` (procedure_assembly 0x7ed830, 0x7ed83a). The object at [EBP-0x28]+0x68 is UNKNOWN. Its vtbl+0x7C body is UNKNOWN (not decompiled). xrefs 0x7ed830 show about 100 call sites.
3. VERIFIED Gob attach-by-name = FUN_00461670 = Gob vtable 0x98B5CC slot +0x50 (read_bytes 0x98B61C: `70 16 46 00`, xrefs 0x461670 gives 0x98b61c and 0x4819f9).
4. VERIFIED wrapper 0x4819E0 sets ECX = this+4 (the child Gob) and calls 0x461670 (procedure_assembly 0x4819e0). Its caller is thunk 0x481D00 (procedure_callers 0x4819e0). The thunk's only data xref is 0x98C2CC (xrefs 0x481d00). Which code calls that slot with which type: UNKNOWN.
5. VERIFIED 0x461670 layout: thiscall ECX=child (saved [EBP-0x3C]); args parent [EBP+8], nodeName [EBP+0xC], type [EBP+0x10]; RET 0xC. Name lookup `parent->vtbl+0x10C` returns N, stored in [EBP-0x14].
6. VERIFIED attach copy at 0x4616F1 `CALL EDX`: N->vtbl+0xC(child+0xA4 vec3, child+0xB0 quat). Pushed at 0x4616D3-0x4616E5.
7. VERIFIED type switch (0x461670 switch on [EBP+0x10]): 0 -> ctor 0x504810 (CAurBehaviorAttach), 1 -> 0x504F60 (Translate), 2 -> 0x5053C0 (Rotate), 3 -> 0x505650 (NoOrientation). Ctor results stored at [EBP-0x44], [EBP-0x48], [EBP-0x4C], [EBP-0x50] respectively.
8. VERIFIED the behavior is stored on the CHILD: Gob vtbl+0x58 = 0x461600 writes `*(in_ECX+0x1b4) = behavior` (procedure_pseudo_code 0x461600; read_bytes 0x98B624 = `00 16 46 00`). It also calls `0x461AA0` with parent+0x94, which sets child+0x94 and sends "UpdateScenes" (procedure_pseudo_code 0x461aa0).
9. VERIFIED behavior layout (asm 0x504D40): P = behavior+0x10 (child Gob), parent = behavior+0x14, N = behavior+0x18. The writer of +0x14 (FUN_00504280) is not decoded: GUESS.
10. REFUTED: the parent Gob is NOT at gob+0x1B4. 0x461530 (GetAttachedParent) returns `[this+0x1B4]`, which is the behavior (asm 0x46156a). The 01_inventory wording "returns *(Gob+0x1b4)" hides this.

## (b) Per-frame follow
1. VERIFIED all four attach-behavior vtables have slot +0x20 = 0x5047B0 (read_bytes 0x9918D8, 0x991914, 0x99194C, 0x991980; xrefs 0x5047b0 lists them). 0x5047B0 is the event path (listener 0x5045E0 calls vtbl+0x20). It is not the per-frame composer.
2. VERIFIED slot +0x08 is the per-frame composer. Base 0x504D40 (read_bytes 0x9918D8: `40 4d 50 00`). Translate 0x505020 calls the base first, then sets pos from behavior+0x3C..+0x44 rotated by the P quat (+0x48 flag). Rotate 0x5054B0 calls the base first, then sets P quat = FUN_00515620(behavior+0x3C..) times P quat. NoOrientation 0x505690 is not read.
3. VERIFIED base refresh (asm 0x504D40): if `P->GetAttachedParent(-1) == this` (0x504E06-0x504E11) and parent != 0 (0x504E3A), it calls `N->vtbl+0xC(P+0xA4, P+0xB0)` at 0x504EF3 (`CALL EAX`), then `MOV [EBP-0x3C],1` at 0x504EF5.
4. VERIFIED child pose is overwritten from N on every refresh. So the LOCAL-offset hypothesis is REFUTED for this path: the write after the attach copy only lasts until the next refresh. The copy source is world for owned nodes (C3, 0x457510): GUESS for the render side (0x4b3c90 uses +A4/+B0 directly).
5. UNKNOWN: who calls behavior vtbl+0x08 each frame (indirect call; a Gob update is suspected). 0x5047B0 does not call vtbl+0x08, so it is not the sword path. This matches the live "OmenRef01 only" result.

## (c) Hook points
- P1 (per refresh, type 0 and via 0x504D40 in Translate/Rotate): site 0x504EF5. Stolen bytes `C7 45 C4 01 00 00 00` (MOV [EBP-0x3C],1, 7 bytes, no relative branch). Next insn 0x504EFC. Use a 5-byte JMP plus 2 NOP. The stub replays the stolen MOV, then runs the hook, then JMPs to 0x504EFC.
  Slots at the hook: [EBP-0x1C] = P (child Gob), [EBP-0x44] = behavior, [EBP-0x14] = parent, [EBP-0x48] = P. ECX/EDX/EAX are clobbered. Keep ESP and EBP unchanged on exit.
  Caveat: Translate writes pos and Rotate writes quat after the base returns, so P1 is overwritten for those types. Hook the end of 0x505020 / 0x5054B0 instead (exact bytes UNKNOWN).
- P2 (attach-time only, first frame): site 0x4616F3. Stolen bytes `8B 45 10 89 45 C0` (MOV EAX,[EBP+0x10]; MOV [EBP-0x40],EAX, 6 bytes, 2 insns). Exit 0x4616F9. Slots: [EBP-0x3C] = child Gob, [EBP-0x18] = parent, [EBP-0x14] = N, [EBP+0x10] = type. A 5-byte JMP needs the stub to replay both insns.
- Types 1/2 have offset fields: behavior+0x3C/+0x40/+0x44 (vec3, zeroed by ctors 0x504F60 and 0x5053C0 at 0x504F60 `in_ECX[0xf..0x11]=0`), +0x48 flag. Setting them after the ctor returns is a candidate. Which type the sword uses is UNKNOWN (GUESS).

## (d) Why 0x5047B0 fires for OmenRef01 only
GUESS: 0x5047B0 runs only via listener 0x5045E0 on the parent, when "UpdateDependentPosition" reaches the parent's dispatcher. The only literal senders are child-side (0x5047F6, 0x504937), so only a behavior whose parent is itself a child fires. OmenRef01 is presumably such a nested attach. Not verified.

## Corrections to earlier notes
- 01_inventory line 80: GetAttachedParent returns the behavior at +0x1B4, not the parent Gob.
- 42_holstered_weapons line 47: the per-frame re-copy is vtbl+0x08 (0x504EF3). 0x5047B0 is event-driven.
- C1 hook 0x5047F4 is not the sword's per-frame site.

## Open
1. Class and vtbl+0x7C of [[EBP-0x28]+0x68] at 0x763B10, and which type it passes to 0x461670.
2. Caller of behavior vtbl+0x08 per frame.
3. NoOrientation 0x505690 body and the exact end sites in 0x505020 / 0x5054B0.
4. Writer of behavior+0x14 (FUN_00504280) and who sets behavior+0x3C..+0x48.
