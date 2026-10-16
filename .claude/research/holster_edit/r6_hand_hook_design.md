# R6: hand hook design at the 0x7637E0 attach call (Steam dump steam_6a522e71, static only)

Read-only. Source: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/76/007637e0.asm` (+ .c), `86/0086aea0`, `86/008695d0`, `86/00867f00`, `86/0086bfd0`, `85/0085f7e0`, `76/00769f60`, `72/0072dfa0`, `swkotor2_vanilla_6a522e71.exe` (PE read of vtables). Asm frame slots are authoritative; Ghidra `local_N` labels in the .c differ by 4 for several slots (e.g. .c `local_40` = asm `[EBP-0x3C]`).

## OUTPUT

1. Hook sites (exact 5-byte windows, two whole instructions each, no jump lands inside them):
   - Right: `0x763B49`, stolen bytes `8B 42 7C FF D0` (`MOV EAX,[EDX+0x7C]`; `CALL EAX`). Resume `0x763B4E` (`C7 45 FC FF FF FF FF`).
   - Left: `0x763E7B`, stolen bytes `8B 42 7C FF D0`. Resume `0x763E80`.
2. State at the site (before the stolen instructions):
   - `[ESP]=W`, `[ESP+4]=CExoString*` ("rhand" at `[EBP-0x5C]`, "lhand" at `[EBP-0x6C]`), `[ESP+8]=flag` (R 1, L 0), `[ESP+0xC]=scale` float.
   - `ECX=body` (`[EBP-0xC4]` R / `[EBP-0xD8]` L), `EDX=[body]` (vptr).
   - `[EBP-0x2C]` = weapon Gob for that hand (non-null, checked at `0x763A28` R / `0x763D55` L).
   - `[EBP-0x40]` (R) / `[EBP-0x30]` (L) = item object. `W = [item+0x14]`.
3. Stub: record (hand, `[EBP-0x2C]`, W, item), save/restore all registers and flags, replay `MOV EAX,[EDX+0x7C]` and `CALL EAX` itself (the callee is RET 0x10, so the args pop), then `JMP` to the resume address. Keep ECX/EDX for the replayed call.
4. Hand needs no timing invariant: the flag and the site identify the hand, and `[EBP-0x2C]` identifies the Gob at the same instant.

## EVIDENCE

- Args: `0x763AF9` PUSH "rhand" (0x99EDD8); `0x763B21` `MOV [EBP-0xA0],ECX` (W saved); `0x763B27` PUSH ECX (reserve); `0x763B2B` FSTP `[ESP]` (scale); `0x763B2E` PUSH 1; `0x763B30-0x763B3A` LEA EDX `[EBP-0x5C]`, PUSH EDX, PUSH EAX; `0x763B41` MOV EDX,[ECX]; `0x763B49` MOV EAX,[EDX+0x7C]; `0x763B4C` CALL EAX; no `ESP` cleanup after the call, so the target must be RET 0x10.
- Left matches: `0x763E2B` PUSH "lhand" (0x99EDD0); `0x763E50` `MOV ECX,[EAX+0x14]` with EAX=`[EBP-0x30]`; `0x763E60` PUSH 0; `0x763E7B` MOV EAX,[EDX+0x7C]; `0x763E7E` CALL EAX.
- Weapon Gob: `0x7639E3-0x7639FE` PUSH 1; PUSH 0xFF; ECX=`item+0x10`; `CALL [EAX+0x98]` on `[item+0x10]`; `MOV [EBP-0x2C],EAX`. Left: `0x763D23-0x763D2B`, same shape, `MOV [EBP-0x2C],EAX`.
- Slot ids: `FUN_0076E610` case 0x10 = creature+0x238 (R), 0x20 = +0x23C (L). `FUN_0073F530` is a one-call wrapper of `FUN_0078BD80` (object lookup, not decoded). Empty = 0x7F000000 (literal at the empty path).
- Body: `FUN_007ED830` = `MOV EAX,[ECX+0x68]; RET` (`0x7ED830`). `CSWCCreature::LoadModel` `0x769F60` builds creature+0x68 through a type switch (jump table `0x76A9C0`). Allocations: 0xFC (ctor `0x85F7E0`), 0x180 (ctor `0x86BFD0`, TW), 0x1D0 (`0x86AEA0`), 0x1DC (`0x8695D0`), 0x22C (`0x867F00`).
- RTTI-verified vftables (COL at vt-4, type name read from the exe):
  - `0x9A454C` = CSWCAnimBase (COL `0x9BE698`). Slot +0x7C = `0x72DFA0`.
  - `0x9A4BDC` = CSWCAnimBaseTW (COL `0x9BE998`). Slot +0x7C = `0x72DFA0`.
  - `0x72DFA0` = `XOR EAX,EAX; RET 0x10` (decomp: `return 0`, ret=16). A no-op that ignores W, name, flag and scale.
- Not verified vftables: the 0x1D0/0x1DC/0x22C ctors store `0x9A4BCC` / `0x9A4A8C` / `0x9A497C`. Vt-4 holds `0x86D510` / `0x86AD70` / `0xFFFFFE80`, not a COL. Their +0x7C targets are `0x8612A0` / `0x863620` / `0x861150`, each RET 4 and one-argument: 0x8612A0 calls body-Gob vtbl+0x134/+0x130 by its arg; 0x863620 returns `anim+0xB8 != 0`; 0x861150 calls `anim+0xB8` vtbl+0x60 with a vector. None is a 4-arg attach.
- `0x86AEA0` writes `[this]=0x9A4BCC`, then calls the TW ctor on `this+0x50` (`0x86AEE4`). That ctor writes only its own subobject, so the primary vptr stays `0x9A4BCC`.
- Holster: `holster_src/Holster.cpp:716-721` classifies the weapon by `[Gob+0x84]` model name, not by hand. `Holster.cpp:586-591` `kWorldKnowsHand = false` (from r1).

## FINDING

1. r1 open Q1 is answered, and the premise fails: vtbl+0x7C does not attach or create a weapon node in any RTTI-verified anim class. Both verified classes map it to a no-op. `0x7637E0` records the hand and item, but it does not attach the Gob there.
2. The three unverified classes do not implement a 4-arg attach either, so the in-world attach is not in vtbl+0x7C of any decoded class. The real attach path is not located.
3. Q2: W is `[item+0x14]`, the item's object handle (same domain as the slot id; empty = NoItemId 0x7F000000). It is not an object created from the item. The weapon Gob is `[EBP-0x2C]`, the result of `item+0x10` vtbl+0x98(0xFF,1).
4. Q3: the exact bytes and slot layout are in OUTPUT 1-2. Hooks at `0x763B49` / `0x763E7B` see (hand, Gob, W) directly.

## OPEN

1. Confirm that the HolsterPlace `g` equals `[EBP-0x2C]` (the item+0x10 result) in one live session. A probe must log both at the same time.
2. Confirm the live body class for the player and companions (type byte from the `0x76A9D8` table), and whether the 0x1D0/0x1DC/0x22C vptrs are live.
3. Find the path that attaches the item Gob to the hand node (probably Gob AttachByName `0x461670` via vtbl+0x50, not decoded). Candidate callers using "rhand": `0x86A430`, `0x86F530`, `0x829850`, `0x873360`, not decoded.
4. Build mismatch: the C4/C5 notes cite build 7751f7d8, this dump is 6a522e71. Check which build `hooks.toml` targets before writing offsets.
5. `W == item+0x14 == slot handle` is INFERRED (sentinel-compatible handle, used as a lookup key). `FUN_0078BD80` is not decoded.

## VERDICT

NEEDS-PROBE-BUILD. The two hook windows are exact and safe, and they capture (hand, weapon Gob, W) at one instant. Whether the Gob is the one HolsterPlace sees, and what attaches it to the hand, needs a live probe.
