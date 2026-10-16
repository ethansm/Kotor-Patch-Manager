# Holster C7-F: Gob to client creature chain (static dump, Steam 6a522e71)

Tags: VERIFIED = quoted asm/decomp line + address. UNKNOWN = not resolved.
Source: ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/ (static only, no REA).
Addresses agree with holster_c4 (0x504EF3 slot +0xC, 0x7637E0 attach path).

## 1. CSWCAnimBase (creature+0x68) and body Gob
- VERIFIED 0x0085FBB0 (anim vtbl+0x08, `FUN_0085fbb0`): `if (param_2 == -1) uVar1 = *(param_1 + 0xb8)`; asm `0085fbc9 MOV EAX,dword ptr [ECX + 0xb8]`.
- VERIFIED anim is a WRAPPER, not the Gob. Body Gob = anim+0xB8.
- VERIFIED +0xB8 written only in LoadModel 0x0085FBE0 (part -1 branch: `param_1[0x2e] = FUN_00462320(...)`) and zeroed in ctor 0x0085F7E0 (`param_1[0x2e] = 0`).
- VERIFIED CSWCAnimBaseTW (vtbl 0x9A4BDC, ctor 0x86BFD0) slot +0x08 = 0x86C210: part -4 -> +0x17C, part -3 -> +0x178, else base (so -1 -> +0xB8).
- Creature accessor VERIFIED 0x007ED830 `MOV EAX,dword ptr [ECX + 0x68]; RET`.

## 2. Back-pointer Gob -> creature
- VERIFIED Gob vtbl+0x74 = 0x4597D0: `MOV dword ptr [EAX + 0x1d8],ECX` (asm 0x4597DD); decomp `*(param_1+0x1d8) = param_2`.
- VERIFIED Gob vtbl+0x70 = 0x4597B0 getter: `MOV EAX,dword ptr [EAX + 0x1d8]`.
- VERIFIED setter call in CSWCCreature::LoadModel 0x00769F60 (string "CSWCCreature::LoadModel(): Failed..."): `0x76A4E8 CALL [anim+8]` gives Gob into [EBP-0x10]; `0x76A4F6 PUSH ECX` where ECX=[EBP-0x1B4]=this (prologue 0x769F8B `MOV [EBP-0x1B4],ECX`); `0x76A502 CALL EDX` (vtbl+0x74). Block runs for part -1..-4.
- VERIFIED Gob ctor 0x458B70 zeroes +0x1D8 (asm 0x459370). Copy helper 0x461B30 (Gob vtbl+0x80) copies +0x1D8/+0x1DC from another Gob.
- VERIFIED no clear-to-zero writer for Gob+0x1D8 besides the ctor (field_census --writers, 28 hits; Gob-class ones are 0x459370, 0x4597D0, 0x461B30). Stale pointers are possible after creature unload.
- Other owner setters, same pattern:
  - CSWCObject::AcquireAnimationBase 0x007F2910: this=[EBP-0xC]; `0x7F29B7 CALL EDX` (vtbl+0x74 with this); also `vtbl+0x7C(byte this+8)` (see 3).
  - 0x007FD5B0 `0x7FD69C CALL EAX`: passes `[EBP-0x1C]-0x10`, i.e. this-0x10. NOT uniform with the creature path.
  - CSWCPlaceable::LoadModel 0x008419B0 `0x841A8F CALL EDX`: arg not decoded.
- Gob+0x1DC is NOT an object id: 0x459810 (vtbl+0x7C) stores it. Creature path passes `MOVZX EAX,byte ptr [0x00994474]` (asm 0x76A504); AcquireAnimationBase passes byte this+8.
- Gob object id: UNKNOWN (no Gob-level id field found).

## 3. Cheapest chain (parent Gob -> creature)
    parent   = *(u32*)(beh + 0x14)          ; given
    creature = *(u32*)(parent + 0x1D8)      ; CSWCCreature*, creature path only
    anim     = *(u32*)(creature + 0x68)     ; CSWCAnimBase
    check    = *(u32*)(anim + 0xB8) == parent  ; base and TW classes; guards stale/adjusted ptr
Two derefs, no table. Use the check before trusting creature; fall back to a table keyed by the body Gob filled in 0x769F60 or the combat hook if it fails.

## 4. Open / risk
- UNKNOWN: whether beh+0x14 is the body Gob (anim+0xB8). The attach call `vtbl+0x7C(handle,"rhand",1,scale)` from 0x7637E0 goes to anim vtbl+0x7C = 0x72DFA0 for base and TW classes. That body is `xor eax,eax` (decomp `return 0`, ret=16), i.e. a no-op. So the creature's anim class (or its real attach method) is not identified.
- UNKNOWN: anim classes at 0x86AEA0 (case 1), 0x8695D0 (case 2), 0x867F00 (case 3). Their ctor vtable stores (0x9A4BCC, 0x9A4A8C, 0x9A497C) do not decode as plain vtables in the dump. Do not assume +0xB8 for them.
- Owner-pointer convention differs by path (0x7FD5B0 uses this-0x10). Only the 0x769F60 creature path is verified raw.
- Object id on the creature: not resolved here.
