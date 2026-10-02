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

## Wave 2
### AI5 (lines CONFIRMED; FM4 extension semantics for the gate)
- `+0x11AC` writers via `field_census --offset 0x11ac --all`: exactly `0056214c` (ctor, =0) and `0058a35e` (SetPartyMemberFlag);
  callgraph has 15 CALL rows to 0x58A330. So +0x11AC = "in the active party (selected followers + PC)": the 02:38 zeros
  (atton/t3m4/kreia standing on the Hawk) are expected, not a detection bug.
- FM6 consumer 0x5CA6A0: `005ca7c6 MOV ECX,dword ptr [EAX + 0x70]`, `005ca7e8 FCOMP double ptr [0x00996520]`, `005ca802 MOV dword ptr [ECX + 0x70],0x0`.
- FM4 extension: SaveCreature restore `0057515f PUSH 0x1` / `00575161 MOV EAX,[EBP-0x38]` / `0057516b CALL 0x0058a330`
  (lines exist; "non-party creature -> AI level 2 on every save" is for the gate).
### AI7 (lines CONFIRMED; two semantics for the gate)
- Item-spell gate: `006b89ce CALL 0x006f2a30`, `006b89d6 TEST EAX,EAX`, `006b89d8 JZ 0x006b8a13`, item scan `006b8a6c CALL 0x006c0330`.
  Whether 0x6F2A30 (ret 0, many callers) really means "Force class" decides the R4 medpac contradiction: gate must read it.
- GetCurrentAction loop: `0067b1bd CMP EDX,EAX` / `0067b1bf JNZ 0x0067b1fb` / `0067b1e7 MOV [EBP-0x8],EDX` / `0067b1f9 JMP 0x0067b1ac`;
  approach re-queued in the attack group: `006d871c MOVZX EDX,word ptr [ECX + 0x6c]` / `006d8729 CALL 0x00540ca0`.
### AI6 (shout abort CONFIRMED; transposition needs the gate)
- Shout loop 0x547100: `0054712f MOV dword ptr [EBP + -0x1c],0xffffffff`, `005471ac CALL 0x00522af0` (return unused),
  `005471b1 CMP dword ptr [EBP + -0x1c],0x0` / `005471b5 JL 0x005475e8`; `00547203 JNZ 0x005475e8`. No `0x005e13c0` call in the .asm (0 hits).
- AdjustReputation 0x5D0DF0: read `005d0e40 PUSH [EBP+8]` / `005d0e47 PUSH own(+0x78)` / `005d0e4b CALL 0x00664580`; write
  `005d0e8b PUSH val` / `005d0e8f PUSH [EBP+8]` / `005d0e94 PUSH own` / `005d0e97 CALL 0x006644d0`. Caller arg order is the SAME
  (own, p2) for both, so the transposition (if real) is inside 0x664580 vs 0x6644D0 index math: the gate must quote both.
- NOTE: AI6 overwrote shared scratchpad helper files early (pe.py, cmds.pkl, tbl.pkl, nws.nss) while AI7 was running:
  the gate must re-derive any AI7 claim that rests on exe table bytes (command table indices, constants) rather than the dump.
### AI8 (three CONFIRMED as lines)
- State precedence 0x59CBA0: `0059d957 CMP dword ptr [EBP + -0x1c],ECX`, `0059d95a JLE 0x0059db3b`, `0059d966 MOV byte ptr [EDX + 0xff5],AL`.
- ClearAllActions skipped while non-commandable: `005410e9 CMP dword ptr [EAX + 0xec],0x1` / `005410f0 JNZ 0x00541385`;
  state apply 0x59DE30: `0059dec7 MOV [EAX+0xec],0x1`, `0059ded6 CALL 0x00541080`, `0059def8 MOV [EAX+0xec],0x0`.
- GetHasSpellEffect scan 0x5476C0: `00547706 MOV EAX,dword ptr [EDX + 0x1c]` / `0054770f CMP ECX,dword ptr [EBP + 0x8]`.

### GATE_wave2 top claims (CONFIRMED by the lead)
- 0x6F2A30 (decompile): returns 1 for class ids 3, 4, 5, 0xB..0x10 only (Force classes per the gate) -> GetCreatureHasTalent(medpac)
  skips the item scan for Force-only creatures (`006b89ce CALL 0x006f2a30` / `006b89d8 JZ`): R4 medpac rule broken for Kreia/Visas.
- Reputation matrix: get 0x664580 `006645ef MOV ECX,[EBP+0xc]` / `006645f2 IMUL ECX,[EAX+0x1c]` (2nd arg x N);
  set 0x6644D0 `00664531 MOV ECX,[EBP+0x8]` / `00664534 IMUL ECX,[EAX+0x1c]` (1st arg x N). Same caller order -> transposed. BUG (low reach).

### REVIEW_synthesis top 2 (CONFIRMED by the lead)
- `+0x10E8` (IsPC) writers (field_census --all): ctor `00562d7d`=0, load `005768fe` (from stats), SwitchPlayerCharacter 0x5FE4B0
  (`005fe604`/`005fe951`=0, `005feae6`=1), `00628375`=0. The UI leader switch SetLeaderBySlot 0x7E5DF0 calls 0x5FE410 at
  `007e6311`, not 0x5FE4B0, so +0x10E8 stays on the Exile while the player controls a companion. "IsPC" != controlled.
- Controlled-leader id: SetLeaderBySlot `007e631d CALL 0x00632830`, which stores `006328d8 MOV dword ptr [EDX + 0x28],EAX`.
