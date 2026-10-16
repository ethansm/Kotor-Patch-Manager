# Holster C6: unarmed anim selection (swkotor2.exe 7751f7d8, REA ghidra)

Tags: VERIFIED = REA tool output this run (tool + addr). GUESS = inferred, not shown. UNKNOWN = not resolved.
Ledger: patch_manager_mods/ghidra_knowledge/rea_findings/20261016_holster-c6-unarmed-anim.tsv (21 rows)
Builds on c4/c5 (not re-derived). Brief's known facts re-checked: 0x863650 thiscall, u16 id, reads of +0xC5/+0xC6.

## (a) Where the anim row is chosen
1. VERIFIED 0x863650 = `ushort FUN_00863650(ushort)`, 4574 B, 5 body ranges. procedure_callees: none. procedure_callers: none resolved (indirect call site, UNKNOWN).
2. VERIFIED 7 data refs to 0x863650 (xrefs): 0x9a462c, 0x9a4954, 0x9a4a74, 0x9a4bb4, 0x9a4cbc, 0x9a982c, 0x9aa11c. Not in the CSWCCreature vtable (0x99ee14) by distance. Owner table: UNKNOWN.
3. VERIFIED wield read: `MOV CL,[EAX+0xC5]` at 0x863670 (read_bytes: `8A 88 C5 00 00 00`), spilled to [EBP-1] at 0x863676.
4. VERIFIED gate read: `MOV DL,[ECX+0xC6]` at 0x863664 (`8A 91 C6 00 00 00`). Then `CMP ECX,1` at 0x863689 and `JNE 0x864439` at 0x86368c (gate != 1 takes the constant table).
5. VERIFIED no g<N> string and no g-prefix table inside 0x863650. It returns a u16 row index. Mapping to "g5r1"-style names: UNKNOWN (caller not read).
6. VERIFIED gate==1 table for id 0x2711/0x27a4: wield 1->0x5c, 2->0x85, 3->0xae, 4->0xd7, 5->0xdf, 6->0xed, 7->0xf5, 8->0xf9 or 0x217 (iVar2 = +0xD4), 9->0xf5 (pseudo, ledger row 11).
7. VERIFIED wield 0 has no case, so id 0x2711 passes through unchanged. For 0x2712/0x2713 wield 0 gives 0; for 0x2714 wield 0 gives 2.
8. VERIFIED ids 0x2710/0x27a5 map to 6 and 0x2715/0x2717 map to 0x55 regardless of wield. So an idle on 0x2710 is wield-independent. The holding idle is probably 0x2711: GUESS.

## (b) Writer of creature+0xC5 and the unarmed value
1. VERIFIED WeaponWield source: baseitems loader FUN_006d5240 (rows of 200 B, row base in_ECX[1]). Column name pushed at 0x6d57ef (`68 98 8D 99 00`, string 0x998d98). Stored `MOV [ECX+8],DL` at 0x6d5856: baseitem row+0x08 u8.
2. VERIFIED FUN_0069dea0 (writes CSWRules::vftable) calls FUN_006d5240.
3. UNKNOWN writer of creature+0xC5. No single GetWeaponWieldType found. Candidate probe: code that does `imul r,r,0xC8` then reads [r+8], then stores to creature+0xC5.
4. VERIFIED the CSWCCreature ctor FUN_007645f0 (vtable write 0x764635) does NOT set byte +0xC5. Its `*(undefined2*)(in_ECX+0xc5)=0xffff` is a dword-scaled write at byte +0x314. REFUTED claim, see ledger.
5. VERIFIED ctor default anim id u16 10000 at +0x48 (`in_ECX[0x12]`, ctor pseudo), also +0x58 inside the model block.
6. GUESS unarmed value = 0 (no switch case; id passes through). Not checked against baseitems rows. Passthrough of raw 0x2711 is a risk (see c).

## (c) Hook candidate (not built, recon only)
1. Site: 0x863670, `8A 88 C5 00 00 00` (MOV CL,[EAX+0xC5]). One whole instruction, no relative branch, 6 bytes. A 5-byte JMP plus 1 NOP fits exactly. Resume at 0x863676.
2. Slots at 0x863670: EAX = creature (this), [EBP-0xC] = u16 id (copied at 0x86365d), [EBP-0x34] = creature. CL is the only register result.
3. Stub sketch (GUESS until built): replay `MOV CL,[EAX+0xC5]`; if [EBP-0xC] is 0x2711 or 0x27a4 and holstered(EAX), then `XOR CL,CL`; save/restore EFLAGS and EDX around any call; JMP 0x863676.
4. Risk: wield 0 leaves raw 0x2711 (not a valid row). The caller's handling of raw 0x2711 is UNKNOWN and could pick a wrong anim. Read the caller first.
5. Risk: combat. If combat ready/idle also uses 0x2711, the holstered flag must clear on combat entry. Attacks on 0x2712-0x2714 and 0x27ad/0x27ae are not touched by the id filter.
6. Risk: the 0x283e path re-reads [ECX+0xC5] directly (gate branch), so this hook does not cover it.
7. Save state: the stub changes only the local CL. creature+0xC5 and +0x48/+0x58 are not written, so no save-state change is expected (GUESS).
8. Holstered flag lookup is not in the exe. Keep the per-creature set in the mod DLL.

## (d) Server counterpart
1. VERIFIED CSWSCreature::vftable 0x994f5c, xrefs 0x561f86 and 0x5634f4 (find_xrefs_to_name). 0x561f86 lies in FUN_00561f30 (0x561f30-0x563169, containment only, body not decompiled).
2. UNKNOWN whether the server keeps a wield byte or anim id that the client resolver mirrors. A client-only +0xC5 change would leave the server state unchanged. Divergence depends on whether the server sends anim ids (UNKNOWN).

## Corrections
- REFUTED: ctor 0x7645f0 initialises creature+0xC5 (ledger row, kind xref).

## Open (next probes)
1. Owner of the 7 data refs to 0x863650 (0x9a462c, 0x9a4954, 0x9a4a74, 0x9a4bb4, 0x9a4cbc, 0x9a982c, 0x9aa11c). This gives the caller and what it does with the id.
2. Writer of creature+0xC5 (row+8 reader plus creature store). Start near the equip path that stores creature+0x224.
3. Decompile FUN_00561f30 for the server counterpart.
4. What the caller does with raw 0x2711 and how the row maps to a g<N> name.
