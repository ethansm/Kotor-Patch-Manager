# holster C1: post-copy hook for UpdateDependentPosition (swkotor2.exe, Steam build)

Source: REA Ghidra provider, binary sha256 7751f7d8...dc04f. Tools: procedure_assembly, procedure_pseudo_code, read_bytes, xrefs, procedure_callers. All VERIFIED unless tagged.

## Premise corrections
- ECX in 0x5047B0 is NOT the Gob. It is the behavior object CAurBehaviorAttach (ctor 0x504810 writes vftable 0x9918D8 to [this]).
- Child Gob = [this+0x10] (P). Node N = [this+0x18]. Parent (name-lookup owner) = [this+0x14].
- 0x5047B0 does not contain the copy. It calls N->vtbl[+0xC](P+0xA4, P+0xB0). The callee does the write.
- vtbl 0x9918D8 slot +0x20 = 0x5047B0 (read_bytes 0x9918D8: bytes b0 47 50 00 at +0x20).

## Q1 0x5047B0 (95 bytes, 1 basic block, 0x5047B0-0x50480E)
Prologue (read_bytes): 55 8B EC 83 EC 0C 89 4D F4 8B 45 F4 8B 48 18
```
0x5047C5 MOV EAX,[EDX+0x10]     ; P = [this+0x10] = child Gob
0x5047C8 MOV [EBP-8],EAX
0x5047D4 PUSH ECX               ; arg2 = P+0xB0 (quat)
0x5047DE PUSH EDX               ; arg1 = P+0xA4 (vec3)
0x5047EF MOV EAX,[EDX+0xC]      ; N->vtbl[+0xC]
0x5047F2 CALL EAX               ; ECX=N; callee writes P+0xA4/P+0xB0
0x5047F4 PUSH 0                 ; <- first insn after copy
0x5047F6 PUSH 0x9918B8         ; "UpdateDependentPosition"
0x504809 CALL EAX               ; P->vtbl[+0x1B4]("UpdateDependentPosition",0)
```
- Post-copy point: 0x5047F4 (return address of CALL at 0x5047F2).
- Stolen bytes at 0x5047F4: `6A 00` (PUSH 0) + `68 B8 18 99 00` (PUSH 0x9918B8). 7 bytes, 2 insns, no relative branch or call.
- consumed_exit_address: 0x5047FB (`8B 4D F8`, MOV ECX,[EBP-8]).
- Gob slot: [EBP-8] = P (child Gob). Also [EBP-0xC] = behavior, [EBP-4] = N. EAX/ECX/EDX are clobbered by the callee; do not rely on them.
- Safe to clobber: EAX (callee return value unused), ECX, EDX, EFLAGS. Keep [EBP-4/8/0xC] intact. Stub must end with ESP = EBP-0xC.
- ESP at 0x5047F4 = EBP-0xC only if callee does RET 8. INFERRED: no ADD ESP after the call site, callee not decompiled.
- Local vs world: UNKNOWN. N's class is not identified (N = parent->vtbl[+0x10C](name)).

## Q2 attach 0x461670 and ctor 0x504810: separate code, same callee
- Callee N->vtbl[+0xC] is called from three sites: 0x4616F1 (attach), 0x504933 (ctor), 0x5047F2 (update).
- Attach 0x461670 (thiscall ECX=child Gob; args parent, nodeName, type; RET 0xC). Prologue (read_bytes): 55 8B EC 6A FF 68 AC A0 94 00 64 A1 00 00 00 00.
  - Name lookup at 0x4616C4: parent->vtbl[+0x10C](nodeName) returns N.
  - Copy at 0x4616F1 (CALL EDX). Post-copy 0x4616F3: `8B 45 10 89 45 C0` (6 bytes, 2 insns). Exit 0x4616F9.
  - Gob slot: [EBP-0x3C]. Frame SUB ESP,0x44.
- Ctor 0x504810 (thiscall ECX=behavior; args Gob, parent, N; RET 0xC).
  - Copy at 0x504933 (CALL EDX), P = [this+0x10].
  - Post-copy 0x504935: `6A 00 68 B8 18 99 00` (7 bytes). Exit 0x50493C.
  - Slots: [EBP-0x18] = P, [EBP-0x14] = N, [EBP-0x38] = this.
- Attach-time and ctor-time writes skip 0x5047B0. A hook only at 0x5047F4 misses the initial pose. Add hooks at 0x4616F3 and 0x504935 if the first frame matters.

## Q3 FUN_004DD0E0: REFUTED as a recursion guard
- 0x5047B0 has no call to 0x4DD0E0. Its only CALL instructions are 0x5047F2 and 0x504809 (indirect).
- Decompile (VERIFIED): `undefined1 FUN_004dd0e0(float, char)`, ECX = object. If DAT_00A049F4 == 0 return 1. Else if arg1 <= obj[+0x58], return (arg2 && obj->vtbl[+0x1C8]()). Else return 1.
- No side effects. Only resolved caller: 0x4DD260.

## Q4 name-based re-parent candidates
- C1 0x461670 (VERIFIED). Attaches a CAurBehaviorAttach that copies node N to child Gob. Name lookup via parent->vtbl[+0x10C]. Type 0..3 selects ctor 0x504810 / 0x504F60 / 0x5053C0 / 0x505650. Caller wrapper 0x4819E0 (VERIFIED, forwards args). No detach function found.
- C2 0x7637E0 (VERIFIED decompile, the hooked function). Args: (undef, event name, creature*). Events "AppearanceForceUpdate" and "Loading". Not a tag. Uses creature+0x9E (rhand id) and +0x9F (lhand id). Calls FUN_007ED830 (= singleton+0x68) then vtbl[+0x7C](weapon+0x14, "rhand"|"lhand", 1|0, scale). Target of vtbl+0x7C: GUESS (name-based attach). Prologue (read_bytes): 55 8B EC 6A FF 68 38 49 96 00 64 A1 00 00 00 00.
- C3 0x86A430 (VERIFIED decompile; "rhand" xref 0x86A61C). Dummy-hand setup: FUN_00734010("dummyright"/"dummyleft"), then obj->vtbl[+0x50](ptr, "rhand"|"lhand", 0). vtbl+0x50 target: GUESS. Prologue (read_bytes): 55 8B EC 6A FF 68 68 53 96 00 64 A1 00 00 00 00.
- "rhand" xrefs: 0x763AF9 (in C2), 0x86A61C (in C3), 0x82A181 (in FUN_00829850, effect name), 0x870799, 0x8735B5, 0x873929 (not examined).

## Q5 call frequency
- Direct callers of 0x5047B0: none resolved (procedure_callers returned []).
- Path: callback FUN_0x5045E0 compares the name "UpdateDependentPosition" (__stricmp) and calls behavior->vtbl[+0x20]() = 0x5047B0. The callback is registered in ctor 0x504810 at 0x504A21, via parent->vtbl[+0x28](0x5045E0, name, this, 0x461C3C00, 0).
- No dirty-flag check in 0x5047B0 or 0x5045E0. The copy runs once per delivered "UpdateDependentPosition" event.
- Event sender: UNKNOWN. String 0x9918B8 xrefs: 0x5045EA (compare), 0x5047F6 and 0x504937 (child-side sends via vtbl+0x1B4), 0x504A1C (registration). The parent-side dispatch is not identified. Measure per-frame hits at runtime.

## Hook notes
- Each update overwrites P+0xA4/+0xB0 from N before the hook, so multiply the fresh copy and do not accumulate.
- The 5-byte patch at 0x5047F4 must stay inside [0x5047F4, 0x5047FB).

## Open
- Class of N and its vtbl[+0xC]: local or world transform (UNKNOWN). Resolve via parent->vtbl[+0x10C] implementation.
