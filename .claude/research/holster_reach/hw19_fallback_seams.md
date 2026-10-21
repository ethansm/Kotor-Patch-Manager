# HW19 fallback seams (swkotor2.exe Steam 6a522e71, static only, no live REA)

Sources: raw PE bytes (sha256 matches target), decomp/48/00480e40.asm, decomp/45/00451e60.asm, decomp/48/00481d50.asm, cg.py callers. Drafts are NOT applied.

## Corrections to HW18 (CONFIRMED)
- Gob::Animate 0x4E1D40 (thiscall, float arg, RET 4) is vtable slot +0x00 of 0x98B5E0. Slot +0x14 of 0x98B5E0 and 0x98C290 is 0x4DD140 (RET 0x14), not Animate.
- PE value scan: 0x4E1D40 sits in one data slot only (0x98B5E0+0); no JMP thunk to it; only direct CALL is 0x481073.
- So no image vtable sends 0x451E60's `CALL [vt+0x14]` to Gob::Animate. HW18 "AnimateExternal calls Gob::Animate via vt+0x14" is REFUTED.
- 0x98C290 slot +0 = 0x481D50 = `SUB ECX,4` / `JMP 0x480E40` (CONFIRMED). No ctor stores either vtable pointer (owner UNSURE).

## (a) 0x481078 = after `CALL 0x4E1D40` at 0x481073 (in 0x480E40)
- 0x481066 51 PUSH ECX; 0x481067 D9 45 08 FLD [EBP+8]; 0x48106A D9 1C 24 FSTP [ESP] (float dt overwrites pushed slot)
- 0x48106D 8B 4D F4 MOV ECX,[EBP-0xC]; 0x481070 83 C1 04 ADD ECX,4
- 0x481073 E8 C8 0C 06 00 CALL 0x4E1D40 (CONFIRMED: 0x481078+0x60CC8=0x4E1D40)
- 0x481078 8B E5 MOV ESP,EBP; 0x48107A 5D POP EBP; 0x48107B C2 04 00 RET 4 (next 0x48107E)
- Stolen span: 0x481078..0x48107D = `8B E5 5D C2 04 00` (6 B, whole insns, contains RET 4, no relative branch or call). Five bytes is impossible: it would split RET imm16.
- Gob: `[EBP-0xC]` = G (ECX at 0x480E46, thunk-adjusted). ECX = G+4 is clobbered after return.
- Nearest alternative: re-point rel32 at 0x481074 to a stub that calls 0x4E1D40. Not a plain detour.

Draft (NOT applied):
```
[[hooks]]
address = 0x00481078
type = "detour"
function = "HwPostAnimateGob"   # placeholder handler name
original_bytes = [0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0x00]
skip_original_bytes = false
exclude_from_restore = []

[[hooks.parameters]]
source = "[ebp-0xc]"
type = "pointer"
```

## (b) 0x451E60 (AnimateExternal): this = [EBP-4], CONFIRMED copy of ECX; ECX clobbered by callee
- Site 1, counter path: 0x451EA8 `FF D0` CALL EAX; return 0x451EAA. Stolen 0x451EAA..0x451EB2 = `8B 4D FC 8B 91 94 00 00 00` (9 B; MOV ECX,[EBP-4]; MOV EDX,[ECX+0x94]). Resume 0x451EB3.
- Site 2, no-counter path: 0x451ED9 `FF D0`; return 0x451EDB. Stolen 0x451EDB..0x451EE0 = `8B E5 5D C2 04 00` (6 B, contains RET 4, same shape as (a)).

Draft 1 (NOT applied; Draft 2 = same shape at 0x00451EDB with original_bytes 8B E5 5D C2 04 00):
```
[[hooks]]
address = 0x00451EAA
type = "detour"
function = "HwPostAnimateExtA"   # placeholder
original_bytes = [0x8B, 0x4D, 0xFC, 0x8B, 0x91, 0x94, 0x00, 0x00, 0x00]
skip_original_bytes = false
exclude_from_restore = []

[[hooks.parameters]]
source = "[ebp-0x4]"
type = "pointer"
```

## Callers (cg.py; CONFIRMED): 0x480E40 sole caller is thunk 0x481D50 (vt 0x98C290 slot 0)
- 0x451E60 creature path: CSWCCreature::SetAppearance 0x76D200 (medium) -> 0x85E650 (5 sites), obj = [this+0x3C]->vt+0x98(0xFF,1).
- Other: 0x8EDB50 (via 0x8E6830), 0x413650/0x413A80 (client UI), 0x7A7C60 CSWCArea::SetWeather.

## Verdict per question
- Party/NPC body Gobs: 0x451E60 reaches them via SetAppearance -> 0x85E650 (obj class UNSURE). 0x480E40 is UNSURE: only reachable via vt 0x98C290 slot 0 of unlocated owner.
- Hook candidates: 0x481078 (6 B) and 0x451EAA (9 B) are clean. 0x451EDB (6 B) is clean if RET in stolen bytes is accepted (no existing entry does this).
