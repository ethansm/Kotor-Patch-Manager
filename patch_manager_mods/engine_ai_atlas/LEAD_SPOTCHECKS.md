# Lead spot-checks on disk (2026-10-02)

## Phase 0 (mapper conflicts)
- `0x5644B0`: `005645e4 CALL 0x00540ca0` (AddActionToFront) -> AddMoveToPointActionToFront. Mapper right, DB wrong.
- `0x552A90`: `size=173 ret=0`, `00552aa5 PUSH 0x2000` / `CALL 0x00919723` -> Initialize; `0x552C90` `00552d89 RET 0x10` -> Put.
  Mapper wrong (GOG param sizes swapped), keep the R9 rows.
- `0x5FE4B0`: `ret=4`, callees include SetPartyMemberFlag `0x58A330`; mapper CSWPartyTable::SwitchPlayerCharacter vs DB
  CSWSCreature::MakePlayerCharacter: left to AI5.

## Wave 1
### AI4 (all three CONFIRMED)
- Items join AI level 0: `005ffe42 PUSH 0x0` ... `005ffe50 MOV ECX,EAX` / `005ffe52 CALL 0x0051e040` in the item ctor 0x5FFD60.
- Dispatch table: `005d5c15 CMP dword ptr [EBP + 0xfffffe4c],0x46`, `005d5c1c JA 0x005d688e`,
  `005d5c28 JMP dword ptr [EDX*0x4 + 0x5d7038]`; vanilla exe bytes at VA 0x5D7038 (file 0x1D6438) =
  `2f5c5d00 4d5f5d00 45605d00 9d605d00` as AI4 quotes.
- `+0x220` writer `0063c257 MOV dword ptr [EDX + 0x220],EAX` in 0x63C1E0 (other `+0x220` writers in the dump belong to
  other classes; the gate should confirm the CSWSObject provenance).

### AI3 (three checked; the sign bug needs the gate's semantic check)
- Spot distance term (0x578850): `00578fe0 FDIV double ptr [0x0098b968]`, `00578ffc FSUB double ptr [0x00987490]`,
  `00579002 CALL 0x0091c860` (ftol), `00579007 NEG EAX`, `0057900f SUB ECX,dword ptr [EBP + -0x10]` -> `[ebp-0x48] += ftol(d/k - c)`;
  listen (same function) `005787ab FDIV double ptr [0x0098b968]`, `005787b1 CALL 0x0091c860`, `005787b9 SUB EDX,EAX` -> subtracts.
  The two signs really differ in the asm. Whether `[ebp-0x48]` is "spot margin" (bug) or a DC (fine) is for the Opus gate.
- E3: `00552dd2 CMP dword ptr [EBP + -0x18],0x9` / `00552dd8 CMP dword ptr [EBP + -0x18],0xa` in 0x552D90: only types 9/10 tested. CONFIRMED.
- P1: `field_census --offset 0x3a0 --all` writers: creature ctor `00562366` only on the creature; the others are in
  0x5EED40 (CSWSPlaceable ctor) / 0x5F2FE0 / 0x5F4C00 (placeable range) or unrelated classes. CONFIRMED (matches lesson 230).

### AI2 (three CONFIRMED)
- Third slot-6 site: `0059e8c3 MOV EAX,0x6`, `0059e8e1 LEA ECX,[EAX + EDX*0x1 + 0x270]`, `0059e904 CALL 0x006fd8d0` in
  0x59E860; installed by `0059414c MOV dword ptr [ECX + 0x24],0x59e860` (in 0x593A30, remove-table fill).
- Deferred damage: callgraph callers of 0x6DCBE0 are only `005711a8` and `005711f1`, both in 0x571030.
- Leader-only pick: `0058f586 CMP dword ptr [EBP + -0x24],0x0`, `0058f58a JZ 0x0058f5d6`, `0058f5d1 CALL 0x006da4f0` in
  EndCombatRound. Round object: `0056223f PUSH 0xb2c...` in the creature ctor 0x561F30 (separate heap object at +0x10DC).

### AI1 (quoted lines exist as quoted; semantics for the gate)
- Partial path gate: `0055b3f8 MOV EDX,dword ptr [ECX]`, `0055b407 JNZ 0x0055b69a`, success commit
  `0055b628 MOV dword ptr [ECX + 0x8c],0x4`. The "fail -> SUCCESS(2) + straight 2-point path when PI+0 == 0" reading
  refines R1/lesson 275 (FM2): the gate must reconcile which return code 0x5BFC00 actually sees for an unreachable target.
- AI level 0 fails PlotPath: `0055b1b5 MOV EAX,dword ptr [EDX + 0x78]`, `0055b1c5 JNZ 0x0055b1d5`,
  `0055b1c7 MOV ECX,dword ptr [0x009944bc]` (=3, FAIL). Lines CONFIRMED.
- Bump abort: `005cbc18 MOV dword ptr [EAX + 0x268],ECX`, `005cbc1e CMP dword ptr [EBP + 0xffffff1c],0x5`,
  `005cbcb5 PUSH 0x9965f8` = "aborted walking, Maximum number of bumps happened" (strings.tsv, fn 0x5CB430). CONFIRMED.

### GATE_wave1 top 3 (CONFIRMED by the lead)
- 0x11 LOS block dead: `005bc4b8 XOR EDX,EDX` / `005bc4ba JZ 0x005bc5de` (always taken).
- FM2b fallback: approach move `005bc6a4 PUSH 0x0` ... `005bc6d1 CALL 0x005644b0`; probes skipped on -1:
  `0055b4fb CMP dword ptr [EBP + -0x20],-0x1` / `0055b4ff JZ 0x0055b61c`.
- Spot sign constants: doubles at 0x98B968 = 3.0, 0x987490 = 2.0 (PE section mapping on the vanilla exe).
