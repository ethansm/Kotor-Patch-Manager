# Holster C3: node class, transform space, event delivery (swkotor2.exe)

Binary: swkotor2.exe sha256 7751f7d8...dc04f, Ghidra 12.1.4 (provider ghidra).
Ledger README default build is 6a522e71. Every address here is for 7751f7d8.
Read first: holster_c1_postcopy.md; 01_inventory_3d_viewport.md:75-85.
Ledger grep for 0x5047B0 / 0x4587F0 (rea_findings + steam_functions.tsv): 0 matches.
Tags: VERIFIED = quoted tool output this run. GUESS = inferred, not shown.

## (a) N's class, callee, transform space
- VERIFIED: N = FindPart result (FUN_004587F0). Concrete class is Part. Ctor FUN_00456D00 (0x456d00) ends with Part::vftable 0x98B36C.
- VERIFIED: CAurPart::vftable 0x98B3EC is abstract. Slot +0xC = __purecall (0x91D3D1). N cannot be a bare CAurPart.
- VERIFIED: vtbl+0xC = FUN_00457510 (0x457510) for Part 0x98B36C, PartTriMesh 0x98B414, PartSkin 0x98B08C, PartAnimMesh 0x98B22C, PartDanglyMesh 0x98AFEC, PartLight 0x98B544 (read_bytes +0xC each).
- GUESS: Lightsaber, Reference, Camera, AABB, Emitter, Beam, Shadow subclasses not checked for override.
- VERIFIED node layout (ctor 0x456D00 + reads in 0x457510): name handle +0x04; vec3 +0x08; quat w-first +0x14,+0x18,+0x1C,+0x20; scale +0x24; children +0x30 (ptr), +0x34 (count); parent +0x3C; owner Gob +0x40.
- VERIFIED space of 0x457510: if [N+0x40]==0, returns N's own local pose (+0x08, +0x14..+0x20).
- VERIFIED: otherwise it walks N and its ancestors via +0x3C (scale, rotate, add translation via FUN_0044C8A0, compose quat). It then applies the owner Gob pose: [Gob+0x1E0] scale, [Gob+0xB0] quat, [Gob+0xA4] pos. Result is WORLD for owned nodes.
- VERIFIED caller 0x5047B0: `(**(code**)(**(int**)(in_ECX+0x18)+0xc))(piVar1+0x29, piVar1+0x2c)`. ECX = N; args = P+0xA4 (vec3), P+0xB0 (quat).
- GUESS: animated vs bind pose. 0x457510 reads only the live node fields, with no separate bind copy. Who writes +0x08/+0x14 per frame (animation) is not traced.
- NOT VERIFIED: callee stack cleanup of 0x457510 (2 stack args). Prior C1 assumed RET 8.

## (b) Delivery of 0x5047B0 and 0x5045E0
- VERIFIED: "UpdateDependentPosition" 0x9918B8 has one literal copy (search_strings) and 4 refs: 0x5045EA (strcmp in callback), 0x5047F6 (child re-send inside 0x5047B0), 0x504937 (child send in ctor 0x504810), 0x504A1C (registration).
- VERIFIED: ctor 0x504810 registers callback 0x5045E0 on the PARENT ([this+0x14]) via parent vtbl+0x28. Userdata = behavior. Float arg 0x461C3C00 = 9999.0 (hand-decoded). Only xref to 0x5045E0: 0x504A21.
- VERIFIED: callback 0x5045E0 passes only an exact "UpdateDependentPosition" match, then calls behavior vtbl+0x20 = 0x5047B0.
- VERIFIED: Gob vtbl+0x1B4 = dispatcher 0x4DD260 (Gob::vftable 0x98B5CC). It walks listeners at Gob+0x1BC (count +0x1C0, reverse order).
- VERIFIED record layout in 0x4DD260: name +0x00 (empty, '*', or stricmp match); callback +0x20; param +0x24; float gate +0x28; flag +0x2C.
- Gate FUN_004DD0E0 is from prior C1 Q3 (not re-run). It fires when DAT_00A049F4 == 0 or obj+0x58 < 9999.0.
- VERIFIED: procedure_callers(0x5047B0) = []. It runs only through listener callback 0x5045E0.
- Ghidra decompile of 0x4DD260 leaves loop guard local_18 unassigned. Not resolved.
- UNKNOWN: whether the parent-side event is sent per frame or on move. No literal sender found beyond the four refs above.

## (c) Name storage (hook can compare without an engine call)
- VERIFIED: FUN_00456B00 (cdecl(node,name), recursive over [node+0x30]/[node+0x34]) compares `__stricmp` (Ghidra label, callee 0x9370C7) with FUN_0044B350(H), H = [node+0x04]. Case-insensitive.
- VERIFIED: the name is NOT inline and NOT CExoString. Chain: H = [N+0x04]; index = [H+0x04]; owner = [H+0x08]; type t = [owner+0x4C] & 0x7F (FUN_00451E20).
- VERIFIED: t==2 gives table = [owner+0xB8]. t==5 gives table = [[owner+0x84]+0xB8] (FUN_00452830 / FUN_00452860 casts).
- VERIFIED: name = table[index], a char*. Other type values: not handled here (UNKNOWN).

## (d) FindPart signature
- VERIFIED: `undefined4 FUN_004587F0(char *name)`, thiscall (ECX = Gob). Returns node* in EAX. `RET 0x4`: callee pops the one stack arg (0x458830).
- VERIFIED body: `__stricmp("root", name)==0 ? [Gob+0x98] : FUN_00456B00([Gob+0x98], name)`.
- VERIFIED: FUN_00456B00 is cdecl. Its caller pops 8 (0x45882A ADD ESP,8).
- VERIFIED call site: 0x4616C4 `CALL EDX` (attach FUN_00461670). Next instruction 0x4616C6 is `MOV [EBP-0x14],EAX`. No caller cleanup, consistent with RET 4.
- VERIFIED slot: Gob::vftable +0x10C = 0x98B6D8 -> 0x4587F0 (read_bytes). Slot 0x98C388 also holds 0x4587F0. Its owner class is not identified.

## Open
1. Sender of the parent-side "UpdateDependentPosition" (per move or per frame): not found.
2. Who writes node +0x08/+0x14 each frame (anim vs bind): not traced.
3. Callee cleanup of 0x457510, and unchecked Part subclasses: not verified.
4. Ledger README default build (6a522e71) differs from this run's 7751f7d8. Consumers must pick the build.

Ledger: patch_manager_mods/ghidra_knowledge/rea_findings/20261015_holster-c3-nodeclass.tsv (30 rows).
