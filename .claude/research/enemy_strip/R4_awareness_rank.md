# R4: Awareness skill rank from a server creature (Steam swkotor2.exe)

Dumps: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/` (short: `D/`).
Ledger names are from `scripts/query_ghidra.py address` (name_map rows).

## Chain (verified)

- `0x0068805b-0x68807a` (`D/68/00687fe0.asm`): `[obj]` → `CALL [EAX+0x30]` → creature in `[EBP-0x14]`. Ledger: `Global::ExecuteCommandGetSkillRank` (0x687fe0).
- `0x688085-0x688091` (`D/68/00687fe0.asm`): `MOV EDX,[ECX+0x1198]` → stats; `CALL 0x006b7bb0` at `0x6880c2`.
- Ledger: `CSWSCreatureStats::GetSkillRank` = 0x006b7bb0 (steam-db-B0, high).
- Calling convention: `__thiscall`, `RET 0xC` (asm `006b8198`). ECX = stats. Stack, pushed in order p4, p3, skill, so skill at `[EBP+8]`, p3 at `[EBP+0xC]`, p4 at `[EBP+0x10]`. Caller side is `PUSH 0; PUSH 0; PUSH skill; MOV ECX,stats; CALL` (`0x6880ba-0x6880c2`). Return is AL, sign-extended by the caller (`MOVSX` at `0x6880a2`, `0x6880c7`).

## Q1: base or total?

- **Array pointer, not inline (CORRECTS the brief).** `stats+0x174` is a POINTER to the int8 array. Asm `006b7be6 MOV EAX,[EDX+0x174]`, then `006b7bec MOVSX ECX,byte [EAX+ECX]`. Decomp `D/6b/006b7bb0.c:18`.
- **Base storage (CONFIRMED).** Ledger: `CSWSCreatureStats::SetSkillRank` = 0x006b81c0. `D/6b/006b81c0.c:6-7`: `*(char*)(param_2 + *(int*)(param_1+0x174)) = param_3`, guarded by `param_2 < DAT_00a1b4d0[+0x123]`. Its raw value is the base rank.
- **0x006b7bb0 has a mode flag p4.**
  - `p4 != 0`: the whole bonus block is skipped (`asm 006b7bf7 JNZ 0x006b8153`; `D/6b/006b7bb0.c:19`, block ends at `:224`). Returns base, clamped.
  - `p4 == 0`: base + bonuses. Sources, in order:
    - skill-2 untrained case via `CSWSCreatureStats::HasSpell` (0x6bd9c0) with args (0, 0x9c, 0), `.c:20`;
    - untrained gate on skill-table flag `row+0x1c` bit 1, `.c:23-25`;
    - `CSWSCreature::GetTotalEffectBonus` (0x582010) with (5, p3, ..., skill, 0xff), `.c:28`;
    - ability term from `row+0x18` vs `DAT_009960f8..0c`, adding bytes at +0xEE/+0xF2/+0xF4/+0xF6/+0xF8, `.c:30-56`;
    - `row+0x1c` bit 2 adds bytes +0x178 + 0x179, `.c:58-60`;
    - per-skill switch of `HasFeat` (0x6b83f0) checks, `.c:62-223`.
  - Clamp: max 0x7F; below -0x7E returns 0x81 (-127), `.c:225-241`.
- **No separate total getter.** Total is the `p4==0` mode of the same function.
  - nwscript `GetSkillRank(int nSkill, object oTarget)`, prototype index 315, is `p4=0` (total). Source: `01_extracted/mod127_repair_hk47/tslpatchdata/nwscript.nss:3631`.
  - nwscript `GetSkillRankBase`, index 870 = 0x366, is `p4=1` (base). Source: same file, `:6283`. Handler gate: `0x68807c CMP [EBP+8],0x366`.
  - Index = count of prototype lines before the declaration. Total 877 matches RE_AGENT_GUIDE "877 slots".
- **Callers with skill 2 or 3, all p4=0 (total).**
  - `CSWSCreature::DoListenDetection` 0x577fa0: skill 2 at `D/57/00577fa0.c:145,153`; skill 3 at `:160,166`.
  - `CSWSCreature::DoSpotDetection` 0x578850: skill 2 at `D/57/00578850.c:83,91`; skill 3 at `:99,106`.
  - `CSWSCreature::UpdateMineCheck` 0x56c310: skill 3 at `D/56/0056c310.c:141,159,198,230`.
  - `CSWSCreature::MaxAttackRange` 0x571570: skill 3 at `D/57/00571570.c:38`.
- **Listen call subject (asm).** `0x5784a7-0x5784d4`: ECX = `[[EBP+8]+0x1198]`. p3 = result of vtable `+0x10` on `[EBP-0x184]`. So the decomp text's "param_1" is not the ECX subject in the caller. The subject side (param_1 vs param_2) is UNPROVEN for Spot and Mine.

## Q2: Awareness row = 3

- Script constants, `01_extracted/mod127_repair_hk47/tslpatchdata/nwscript.nss:1043-1052`: COMPUTER_USE 0, DEMOLITIONS 1, STEALTH 2, AWARENESS 3, PERSUADE 4, REPAIR 5, SECURITY 6, TREAT_INJURY 7, MAX_SKILLS 8. Same values in 3+ other mod copies.
- Engine passes nSkill straight through as the array index (`0x6880ba` pushes skill, `0x006b7bb0` indexes `stats+0x174 + skill`). So the script value is the engine row index. CONFIRMED given the constants.
- `skills.2da` itself is NOT in `01_extracted` or `02_processed`, and `find ~/Games -iname skills.2da` returned nothing. So the 2DA row order is not read directly.

## Q3: Count 8, signed

- Signed: CONFIRMED. `0x006b7bec MOVSX`; caller `0x6880a2 MOVSX`.
- Count is runtime: byte at `DAT_00a1b4d0+0x123`, written at `D/6a/006a17a0.c:33` from the "Skills" 2DA `local_120[9]` (field `+0x24`, meaning UNPROVEN). Bounds check at `D/6b/006b7bb0.c:17`.
- Value 8: nwscript `SKILL_MAX_SKILLS = 8` (`nwscript.nss:1052`) only. Runtime 2DA row count is UNPROVEN.

## Q4: `0x1198` object type

- **Server (CONFIRMED by several anchors):**
  - `D/5a/005a8560.c:15-22`: ledger `CSWSEffectListHandler::OnApplyDamageForcePoints` reads `[obj+0x1198]+0x130/0x132`.
  - `D/74/0074ee90.asm:0074ef24 CALL 0x0077d800` (MemberServerCreature), then `0074f1ab MOV ECX,[EAX+0x1198]`. Client UI `CSWGuiInGameGameplay::UpdatePartyPortraits` reads 0x1198 only on the server creature. Ledger for 0x77d800: "creature+0x1198 = stats", conf medium.
  - `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:63`: "server creature: stats ptr".
- **Level-up override** (`patch_manager_mods/level_up_override_src/LevelUpOverride.cpp`):
  - `:2097` `Sv_Stats = 0x1198`, `Cl_Server = 0xF8`.
  - `:2121` client panel → `+0xF8` → server.
  - `:2130` `readAt(server, Sv_Stats)`.
  - `:1741` `Cr_Stats = 0x1198` ("creature -> CSWSCreatureStats*").
- **K2Addr.h:** field `:134` says only "creature +stats object". Value `0x1198` in `makeSteam` (`:315`, block starts `:207`) and `makeGog` (`:502`, block starts `:403`). No object type stated.
- **Client `CSWCCreature` at 0x1198:** NOT checked. Absence is UNPROVEN.
- Shield bar `ShieldBar.cpp:389-393` reads `creature+Creature_Stats`. Caller at `:573` not traced.

## Summary

- Base array (set by 0x6b81c0), read at `stats+0x174` as a pointer.
- Total = `p4==0`. Base = `p4!=0` (GetSkillRankBase, idx 870).
- Awareness = row 3 (script constant; 2DA not extracted).
- Count 8 only as a script constant.
- Signed int8.
- 0x1198 = CSWSCreatureStats*, server creature.

## OPEN

1. Extract `skills.2da` from the chitin BIF to confirm row count (8) and row order (Awareness = row 3).
2. Decode `0x006b7bb0` `p3` (vtable +0x10 result) and `0x582010` type 5. Name them.
3. Confirm the stealth subject side in DoSpot and Listen (param_1 vs param_2).
4. Check whether `CSWCCreature` (client) has a stats pointer at 0x1198. Not yet checked.
