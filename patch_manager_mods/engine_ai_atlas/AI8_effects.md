# AI8 Effects and AoE as they gate AI (KOTOR II Steam swkotor2.exe 6a522e71)

Pipeline position: scripts create effects (`ExecuteCommandEffect* 0x672930` family, `ApplyEffectToObject 0x66D780`) -> `CSWSObject::ApplyEffect 0x544210` (sorted insert into the list at `+0x148/+0x14C`, apply-table handlers) -> `CSWSCreature::AIUpdate 0x569320` block 7 runs `UpdateEffectList 0x544AD0` (expiry, periodic) immediately before `RunActions 0x5D56B0`. Effects gate AI at four places only: the SetState id `+0xFF5` (precedence, per-state side effects, state scripts, slot 6 on end), the allowed-action mask `+0x1114`, the commandable flag `+0xEC`, and the `Add*Actions` / handler tests that read them. `AIUpdate` and `RunActions` themselves do not read `+0xFF5` or `+0xFF4`. This page also covers `GetHasSpellEffect` semantics (Battle Meditation, L2), AoE object triggers and cadence, and EFFOVF.

## Status
- Sources: batch `findings/AI8.md` (Sonnet) + `GATE_wave2.md` (Opus gate, 2026-10-02; section 1 AI8 verdicts, 1.7 `+0x1114` and `+0xEC`, 1.8 Battle Meditation, section 2 reconciliation 6, section 3 confidence, section 4 names, lessons 9-11) + `LEAD_SPOTCHECKS.md` (AI8: precedence lines, ClearAllActions skip lines, GetHasSpellEffect scan lines CONFIRMED).
- Date: 2026-10-02. Static analysis only. Gate-verified: SetState map and precedence (agrees with GATE_wave1), `+0xEC` gate on ClearAllActions, `+0x1114` bits 0x2/0x4/0x80/0x8, GetHasSpellEffect semantics and stamping, `0x57E280` identity, apply-table slots. Absence claims (states 0xB/0xC/0x11, `+0xFF5/+0xFF4` not gating AIUpdate) stay med; AoE cadence low; EFFOVF reach med.
- Conflicts resolved to the gate version are marked "batch said X; gate corrected".

## Function table
| addr | name | conf | purpose |
|---|---|---|---|
| 0x59CBA0 | OnApplySetState (type 8) | high | immunity checks, type-0x17 mask -0x101, internal type-9 effect if the id beats `+0xFF5`; `0059d957 CMP [EBP-0x1C],ECX / 0059d95a JLE 0x59DB3B / 0059d966 MOV [EDX+0xFF5],AL` |
| 0x59DB50 | OnRemoveSetState (type 8) | high | `+0xEC = 1` (`0059db95`), `+0xFF5 = 0`, removes type-9 effects, re-elects the highest remaining state (`0059dd5c`; `0059dd65 MOV [EAX+0xEC],0` when another state remains) |
| 0x59DE30 | OnApplySetStateInternal (type 9) | high | `+0x390 = 0`, `+0x394 = 1` (forced heartbeat), for state != 9: `+0xEC = 1`, `ClearAllActions(1)` (`0059ded6`), `+0xEC = 0` (`0059def8`); per-state effects |
| 0x59E860 | OnRemoveSetStateInternal (type 9) | high | gate-confirmed: slot-6 RunScript for states 1, 3-10, 0xE-0x13 (`0059e8e1`, `0059e904 CALL 0x6FD8D0`), then `0x57A990`, then perception refresh for non-PC |
| 0x59C3B0 / 0x59C420 | OnApplySetAIState / OnRemoveSetAIState (type 0x17) | high | `+0x1114 &= int0` (`0059c3fc`, `0059c409`) / recompute (`0059c536`) |
| 0x59B520 / 0x59B8D0 / 0x59BE80 | OnApplyEntangled (0x12) / OnApplyKnockdown (0x14) / OnRemove | med / high / high | entangle mask -3; knockdown `+0xEC = 0` (`0059bc27`), restored `0059bf67`, spawns SetState 0xE |
| 0x5A7E70 / 0x5A8410 / 0x5A7820 | OnApplyForcePush / OnRemoveForcePush (0x3C) / OnApplyForceJump (0x67) | high / high / med | SetState 9 / 0xD; push recovery mask 0xFEA1 for 2.55 s |
| 0x597D90 | OnApplyDeath (0x13) | high | `+0x1114 = 0` (`005982d7`) |
| 0x6DAA00 | CSWSCreature::IsDebilitated(flag) | high | 0 only if state in {0, 1, 0x10} and not (party && HP < 1); flag = 1 also true while meditating (`+0x110C == 1`) |
| 0x696D80 / 0x67AC20 | cmd 732 GetIsDebilitated / cmd 163 GetCommandable | high | `IsDebilitated(0)` / `+0xEC` |
| 0x541080 | CSWSObject::ClearAllActions | high | body inside `if (param_1[0x3b] == 1)`: `005410e9 CMP [EAX+0xEC],1 / 005410f0 JNZ 0x541385` |
| 0x5E0E70 / 0x5E1340 | AIActionSetCommandable (0x2F) / AddDoCommandAction | high | `+0xEC = node value`; DoCommand queued only if commandable |
| 0x568540 | AddAttackActions | high | refuses unless commandable or state 1/0x10 (`00568695`, `005686aa..005686cf`) |
| 0x565310 / 0x564C21 | AddCastSpellActions / AddItemCastSpellActions | high | return 0 if `+0xEC == 0` (`00565713`) |
| 0x6D7390 | AIActionPhysicalAttacks | high | mask bits 0x80 and 0x4 gate every attack tick (`006d7754`, `006d777e`) |
| 0x6B8840 | HasTalent | med | spell talents need `mask & 8` (`006b8aa3`) |
| 0x56B8A0 | SpawnInHeartbeatPerception | high | heartbeat + state-script block (`0056bafb`, switch `0056bb87..0056bb9d`) |
| 0x57A990 | RecomputeAmbientAnimationState | high | `+0xFF5` -> ambient animation (14 compares `0x57AA3E..0x57ABEB`) |
| 0x57E280 | RemoveMindTrickAndScrambleEffects | high | gate rename (DB SetLastInstantEquip, low); `FUN_005e4e60(0) == 0x12 \|\| == 0x13` then `FUN_00544790`; closes the GATE_wave1 open item |
| 0x5E5F60 | CGameEffect::GetEffectTypeScriptConstant | med | type 8: state -> script constant (1 -> 0x18, 2 -> 0x19, 3 -> 0x23, 4 -> 0x1D, 5 -> 0x1B, 6 -> 0x1E, 0x10 -> 0x4F, 0x12 -> 0x50, 0x13 -> 0x51, others 0) |
| 0x5E5EA0 | CGameEffect::SetCreator | high | `+0x18 = creator`, `+0x1C = creator->vt[0xB8]()` |
| 0x563350 / 0x563370 | GetEffectSpellId / SetEffectSpellId (`+0xFF8`) | high | vtbl `+0xB8` / `+0xBC` |
| 0x67E670 / 0x5476C0 | cmd 304 GetHasSpellEffect / CSWSObject::GetHasSpellEffect | high | list scan `effect+0x1C == spell` (`00547706`, `0054770f`) |
| 0x67C250 | cmd 305 GetEffectSpellId | high | returns `effect+0x1C` |
| 0x544210 / 0x544AD0 / 0x544790 | ApplyEffect / UpdateEffectList / RemoveEffect(id) | high | sorted insert, no dedup / per-AIUpdate expiry / remove |
| 0x5E5510 | CGameEffect::LoadGameEffect | high | GFF -> effect; loop `005e58f3..005e5945` (EFFOVF) |
| 0x59F150 / 0x59F4F0 | OnApplyAreaOfEffect (0x1F) / OnRemove | high | creates the AoE object, strings 0/1/2 -> slots 2/0/3 |
| 0x62BFC0 / 0x62C360 / 0x62C630 | AoE ctor (AddObject level 0) / AIUpdate / EventHandler | high | `0062c13b CALL 0x51E040` with `PUSH 0`; `0062c440 CMP [EBP-4],0x1770` |
| 0x62E320 / 0x5C25E0 / 0x5C2930 / 0x54D180 | AoE UpdateSubAreas / UpdateSubareasOnMoveTo / diff + post enter/exit / CSWArea::InSubAreas | med / high | per walker step |

## Key behaviours
1. **`+0xFF5` / `+0xFF4` do not gate AIUpdate or RunActions** (census over `--offset 0xFF5 --all`: 32 hits in 10 functions, none in `0x569320` / `0x5D56B0`; absence, med). `+0xFF5` readers: `AddAttackActions`, `SpawnInHeartbeatPerception`, `DoPerceptionUpdateOnCreature`, `RecomputeAmbientAnimationState`, knockdown apply, `IsDebilitated`, client `OnObjectSelected`. Writers: ctor `0x5625AD`, `0x59D966`, `0x59DBA2`, `0x59DD5C`. `+0xFF4` (u8 sight bits: 1 see-invisible, 2 ultravision, 4 true seeing, 8 darkness, 0x10 blind; high for 1/2/4, med for 8/0x10) is read only by perception (`0x578850`, `0x585480`, `0x585420`). Block-7 order: `UpdateEffectList` (expiry) then `RunActions`, so a state that expires removes its mask, sets `+0xEC = 1` and runs slot 6 before the same tick's RunActions.
2. **SetState id map** (type 8 effect, `int0` = state; creators via `0x672930` jump table `00672a12 JMP [EDX*4+0x67308C]` and standalone handlers):

| state | meaning | script effect (cmd id) | creator | `GetEffectType` | mask via type 0x17 | notes |
|---|---|---|---|---|---|---|
| 1 | confused | EffectConfused (157) | 0x672930 (0x672BD1) | 24 | none | StateScripts row 3 each heartbeat; `+0xEC = 0` re-asserted |
| 2 | frightened | EffectFrightened (158) | 0x672930 (0x672C19) | 25 | none | type 0x1B penalty; StateScripts row 2; no slot 6 on removal |
| 3 | droid stun | EffectDroidStun (391) | 0x6774E0 | 35 | -3 | no immunity check |
| 4 | stunned | EffectStunned (161), CutSceneStunned (756) | 0x672930 (0x672C3D) | 29 | -3 | |
| 5 | paralyzed | EffectParalyze (148) | 0x672930 (0x672B89) | 27 | -15 (0xFFF1: clears 2, 4, 8) | |
| 6 | sleep | EffectSleep (154) | 0x672930 (0x672BAD) | 30 | -351 (0xFEA1: clears 2, 4, 8, 0x10, 0x40, 0x100) | |
| 7 | choke | EffectChoke (159) | 0x6781F0 | 0 (script cannot see it) | -3 | |
| 8 | horrified | EffectHorrified (471) | 0x677D90 / 0x672F14 | 0 | -3 | |
| 9 | force-push stagger | (engine) ForcePushed | 0x5A7E70 | 0 | -3 | skips the ClearAllActions block (`0059debe CMP [EBP-0x10],9 / JZ 0x59DF09`); recovery 2.55 s mask 0xFEA1 |
| 0xA | whirlwind | EffectWhirlWind (703) | 0x677A50 | 0 | -3 | |
| 0xD | force jump | EffectForceJump (153) via type 0x67 | 0x5A7820 | 0 | none | no slot 6 on removal |
| 0xE | knockdown | EffectKnockdown (134) via type 0x14 | 0x59B8D0 | 0 | -3 | |
| 0xF | crush | EffectCrush (781) | 0x672830 | 0 | -3 | |
| 0x10 | droid confused | EffectDroidConfused (809) | 0x672930 (0x672BF5) | 79 | none | like 1 |
| 0x12 | mind trick | EffectMindTrick (848) | 0x672930 (0x672C61) | 80 | -3 | removed by `0x57E280` on damage / enemy within 3.16 m |
| 0x13 | droid scramble | EffectDroidScramble (852) | 0x672930 (0x672C85) | 81 | -3 | same as 0x12 |
| 0xB, 0xC, 0x11 | no creator found | - | - | 0 | 0x11: -3; 0xB, 0xC none | med (absence); StateScripts row 1 exists for 0xB |

   `GetEffectType` returns script type 0 for states 7, 8, 9, 0xA and 0xF: scripts cannot see them. Every type-8 effect also clears mask bit 0x100 (`FUN_005e4e80(0,0xfffffeff)`).
3. **Precedence.** `0059d957 CMP [EBP-0x1C],ECX / 0059d95a JLE 0x59DB3B`: a new id must be strictly greater than the current `+0xFF5`; the type-8 effect stays in the list either way; `OnRemoveSetState` re-elects the highest remaining id. Numeric order = precedence: 0x13/0x12 > 0x11 ... > 0xE > 9 > 8 > 7 > 6 > 5 > 4 > 3 > 2 > 1. The internal type-9 effect has duration type 4 (never expired by `UpdateEffectList`); it dies only via `0x544790` from `0x59CBA0` (replacement) or `0x59DB50` (parent removed). Apply order in `0x59DE30`: stamps, then (state != 9) `+0xEC = 1`, `ClearAllActions(1)`, `0x590430` (free the round's action list `+0xB00`), `+0xEC = 0`, then per-state effects, then `0x57A990`.
4. **`+0xEC` commandable (gate-corrected).** ctor `0053eb05` = 1. `ClearAllActions` is a **full no-op unless `+0xEC == 1`** (the whole body: queue clear, `+0x11B0 := 1`, `+0x54C`, PI reset, `0x5908F0`, party slot -1). **Gated** (refuse work when 0): `AddAttackActions` (`0x568695`, except states 1/0x10), `AddCastSpellActions` `0x565713`, `AddItemCastSpellActions`, Equip/Unequip/Stance entries, `AddDoCommandAction 0x5E134C` (drops the queued command), and script handlers with an explicit test: ActionFollowLeader (`0066a001`/`0066a00e JNZ`), MoveToObject (`0068cccc`/`0068ccd9`), MoveAwayFromObject (`0068ca9d`), Wait (`00695fa3`), JumpToObject/Point, MoveAwayFromLocation, RandomWalk, conversations, OpenDoor, PlayAnimation, SpeakString, CastSpell. **NOT gated:** ActionMoveToLocation (cmd 21) and ActionForceMoveToLocation (382) = `0x68CEE0` -> `0x564770` -> `0x53F7F0`; the census over all three shows no `+0xEC` read, so the **move node IS queued** on a non-commandable creature. Batch said "script `Action*` commands are dropped when `+0xEC` is 0" and "`+0xEC == 0` drops the AI's ActionMoveToLocation" (Mod 8 kite); gate corrected: partly refuted. Whether the queued move then walks under a SetState mask is open (type 1 has no general bit-2 gate).
5. **Live writers of `+0xEC := 0` that reach party members (gate):** SetState apply `0059def8` for every state except 9; the heartbeat re-assert for states 1/0x10 (`0056bb0d`, `0056bc3d`); knockdown apply `0059bc27` (restored `0059bf67`); OnRemoveSetState re-elect `0059dd65`; script SetCommandable / action 0x2F (`00691004`, `005e0f1b`); LoadObjectState `0053fccc`; effect-table slot 15 `0x59ACC0` (`0059ade0`, conditional, type unnamed). Restored by OnRemoveSetState `0059db95`, ClearAllEffects `0054a5fb`, `0x599137`. Code that must clear actions forces `+0xEC = 1` temporarily: SetState apply `0059dec7`, follow state 0 `005c77c4`/`005c7a18`, SpawnNPC `005fd549`/`005fd563`. Batch said the OnApplyDamage writer `0x596A7A` sets `+0xEC := 0`; gate corrected: **dead code** (`005969cb XOR EDX,EDX` / `005969cd JZ 0x00596b74`, every branch into `0x5969D3..0x596B74` comes from inside that block); remove it from the writer list.
6. **`+0x1114` allowed-action mask** (u16, init 0xFFFF `005625a0`, death 0, type-0x17 effects AND it, removal recomputes). Gate-confirmed bits: **0x2 = movement**: `AIActionCheckMoveToPoint 0x5BC796` -> 3, `AIActionFollowLeader 0x5BF89E` -> 3, `AIActionFollowOwner 0x5BF9E3` -> 3, `AIActionMoveToPoint 0x5C1745` only in the object-type case 6/8 at `0x5C172F` ("I have no legs! I can't feel my legs!!!!" `005c1783 PUSH 0x99649C`, returns 3), CounterSpell; it is NOT a general MoveToPoint gate. **0x4 + 0x80 = attack** (`0x6D7754` for 0x80, `0x6D777E` for 0x4; both needed, else EndAttackAction `006d77ac` and return 3). **0x8 = spells** (`HasTalent` type 0, `006b8aa3`). 0x40 med (awake flag mirrored to the client). 0x01/0x10/0x20/0x100: no server reader (low). Stun-likes (mask -3) keep 4 and 0x80, so an in-progress `0xC` is not refused by the mask; what stops them is `ClearAllActions(1)` at apply, `+0xEC == 0` and bit 2 for movement. Paralysis and sleep clear 4 and 8 (no melee, no spells).
7. **`IsDebilitated`** is true for stun, paralysis, sleep, choke, horror, push, whirlwind, knockdown, crush, droid stun, mind trick, scramble, fear; false for confusion (1, 0x10); HP < 1 party members count as debilitated. Callers include `ResolveEngagedState 0x59128C`, `GetArmorClass`, `OnApplyDamage`, the script command.
8. **Heartbeat state block** (`0x56B8A0`): state 1/0x10 -> `+0xEC = 0` (`0056bb0d`); slot-0 OnHeartbeat; `switch(+0xFF5)` (table `0x56C070`): states 1 and 0x10 -> StateScripts.2da row 3, state 2 -> row 2, state 0xB -> row 1 (script names not in the exe); the state script runs with `+0xEC == 0` and sets it 0 again (`0056bc3d`). Only 1, 2, 0xB run state scripts, so our `k_ai_master` is bypassed for confusion and fear; the state script must call `SetCommandable(TRUE)` itself.
9. **State end -> slot 6** (`0x59E860`): states 1, 3-10, 0xE-0x13 (not 2, 0xB, 0xC, 0xD), on replacement as well as expiry: the engine mechanism that re-arms `k_ai_master` EndRound after a stun.
10. **GetHasSpellEffect semantics (gate: CONFIRMED).** `0x67E670` -> `0x5476C0`: `if (param_2 != -1) for (i < +0x14C) if (effect->+0x1C == spell) return 1` (`00547706 MOV EAX,[EDX+0x1C]`, `0054770f CMP ECX,[EBP+8]`); O(E), no side effect, no creator or duration filter, shows on every member that received a copy. `+0x1C` = `CGameEffect::SetCreator 0x5E5EA0` = creator `+0xFF8` (an AOE creator is replaced by `+0x288`), which `EventHandler` sets around the spell script (event 0x13: `0056fc5e MOV EDX,[EAX+0xBC]` / `0056fc64 CALL EDX` set spell id, `0056fce2 CALL 0x6FD8D0`, `0056fce7 PUSH -1` / `0056fcfd CALL EAX` reset). Effects built outside a spell script (plain AI scripts, heartbeat) or later via `DelayCommand` carry -1 and never match; the BM tree does not use DelayCommand. Saved as GFF `SpellId` and copied by `CopyEffect` (survives saves). `OnApplyLink 0x5A1680` applies the children separately, each stamped with the cast id.
11. **Battle Meditation note (L2, gate 1.8).** `k_inc_force.nss` 3972-4027: the link is built once (stamped with the cast id), then `nCnt` 0..2 over `GetPartyMemberByIndex` (client CSWParty, max 3 slots, the caster included) and `GetRacialType != RACIAL_TYPE_DROID`, then Remove/Better checks, then Apply. Droids never report it, so a family test on a droid target is always FALSE (recast risk only if L2 tests a droid); non-selected companions never get it. Batch said "does not show on the 4th+ member"; gate: moot, the active party has at most 3. R2 L2a tests the caster itself (`GN_PartyBuffActive` on OBJECT_SELF, a Jedi): correct; **L2 risk LOW**. The family test `GetHasSpellEffect(FORCE_POWER_BATTLE_MEDITATION_PC / _IMPROVED_ / _MASTER_)` is right per member (removal scripts match by id too: `Sp_RemoveRelatedPowers`, `Sp_BetterRelatedPowerExists`, k_inc_force 6123-6133).
12. **AoE triggers and cadence (low).** AoE script table at `+0x2A0`, 8 B per slot: 0 heartbeat, 1 user-defined, 2 OnObjEnter, 3 OnObjExit; `OnApplyAreaOfEffect` maps strings 0/1/2 -> slots 2/0/3 (`0059f2b4`, `0059f359`, `0059f409`). Registered at AI level 0 (`0062c13b CALL 0x51E040`, `PUSH 0`), so `0x62C360` runs once per level-0 scheduler pass. Heartbeat: `if (5999 < ms) { if (+0x2C4 != 0) RunScript(+0x2A0, id, 1); stamp = now }` (`0062c440 CMP [EBP-4],0x1770`): the first pass only primes the stamp (first script >= 6 s later), afterwards 6000 ms rounded up to the level-0 per-object update interval (AI4 AIQ model, SPECULATIVE). Follow: host position copied if `+0x28C != INVALID && byte +0x270 == 0`, host gone -> event 0xB to itself. Duration: `+0x2CC == 1` decrements `+0x2C8` by dt. **Enter/exit** are event 10, payload word 0xC (enter, sets `+0x290`, runs `+0x2B0`) / 0xD (exit, sets `+0x294`, runs `+0x2B8`), delta 0, posted by `0x5C2930` from every walker step (`0x5C25E0` -> `InSubAreas 0x54D180`, O(sub-area objects in the area) per step of every moving creature; callers `005c39ee`, `005c4c36`, `005c6121`, `005ced49`, jumps `0x5C2770`) and by `0x62E320` (AoE creation `0x62C800`, `JumpToPoint 0x62EB70`, follow aura). Script-visible: `GetEnteringObject` (id 25 `0x67C6F0`), `GetExitingObject` (id 26 `0x682C30`, case 0xB returns `+0x294`).
13. **EFFOVF (CONFIRMED, reach med).** `ApplyEffect 0x544210` has no dedup or cap (sorted insert; only early exit is the handler verdict `FUN_0051efd0 == DAT_00995f18`); permanent / instant-type effects (`flags & 7 != 1`) are never expired by `0x544AD0`; `ApplyEffectToObject 0x66D780` has no count guard. Reach: any script that re-applies a long/permanent effect every heartbeat or round (observed mitigation: `Sp_RemoveRelatedPowers` before application). `LoadGameEffect 0x5E5510`: `005e56fc CALL 0x5E4DD0` (`new(n*4)`), loop `005e58f3..005e5945` bounded by the GFF IntList length, not `NumIntegers` (`+0x30`): heap overflow only on a corrupt or externally edited save (callers `0x5205F0`, `LoadEffectList 0x5406E0`, `LoadGameDefinedStructure 0x669370`). Queue guards from AI4: effect count `+0x14C` > 500 logs 0xA2, > 1000 calls `0x544A10`.

## Struct fields
| class | offset | type | name | conf | evidence |
|---|---|---|---|---|---|
| CSWSObject | 0xEC | int | commandable (ctor 1) | high | `005410e9`, `00565713`; writers in key behaviour 5 |
| CSWSObject | 0x148 / 0x14C | ptr / int | effect list / count (sorted by type) | high | `005476e7 CMP EDX,[ECX+0x14C]` |
| CSWSCreature | 0xFF4 | u8 | sight bits (1 see-inv, 2 ultra, 4 true seeing, 8 dark, 0x10 blind) | high (1,2,4) / med | `005a500a`, `005a520b`, `005a563e` |
| CSWSCreature | 0xFF5 | u8 | current SetState id (highest wins) | high | `0059d957` |
| CSWSCreature | 0xFF8 | int | effect spell id (-1 outside a spell script; stamped into new effects) | high | `0056337d` |
| CSWSCreature | 0x1114 | u16 | allowed-action mask | high | `0059c3fc`, writers `0x5625A0`, `0x59C409`, `0x59C536`, `0x5982D7`, `0x576C4D`, `0x59912D`, `0x6BA601` |
| CSWSCreature | 0x390 / 0x394 / 0x398 / 0x3D4 | int | heartbeat stamp (day, ms; SetState sets (0,1)) / interval 3000 + rand % 1200 / call counter | high / med | `0059dea0`, `0059dead`, `0x56B8A0` |
| CSWSCreature | 0x110C | u8 | stance (0xFF none, 0 total defense, 1 meditate) | high | `005d2f2d` |
| CSWSCreature | 0x106C / 0x1070 / 0x170 | int | spell cast in progress / spell-slot payload / spell id | med | `0056fc94` |
| CGameEffect | 0x08 / 0x0A | u16 | type / flags (low 3 bits duration type) | high | `005e5f60`, `0x544AD0` |
| CGameEffect | 0x18 / 0x1C / 0x24 | id / int / int | creator id / spell id (-1) / caster `+0x1070` at apply | high / high / med | `005e5088`, `0066d91b` |
| CGameEffect | 0x28 / 0x2C / 0x30 / 0x34 | ptr / ptr / int / int* | link children / NumIntegers / IntList (unchecked index) | high | `0x5A1680`, `0x5E4E80` |
| CSWSAreaOfEffectObject | 0x270 / 0x28C / 0x290 / 0x294 | u8 / id / id / id | shape byte (0 = follows host) / follow target / LastEntered / LastLeft | med / high | `0062c4c3`, `0062c6a2` |
| CSWSAreaOfEffectObject | 0x2A0..0x2BF / 0x2C0 / 0x2C4 / 0x2C8 / 0x2CC | slots / int / int / uint / u8 | script slots [heartbeat, user, enter, exit] / heartbeat stamp (day, ms; 0 = unprimed) / remaining duration ms / mode (1 timed, 2 permanent) | high | `0062c44c`, `0062c118` |
| CSWSArea | 0x1A4 / 0x1A8 | id* / int | sub-area object list (triggers, AoEs) | med | `0x54D180` |

## Bugs and candidates
| item | verdict | user impact | fix idea | mod item |
|---|---|---|---|---|
| EFFOVF: no dedup/cap on ApplyEffect; IntList loop unbounded vs NumIntegers | CONFIRMED (reach med) | effect list growth for re-applied permanent effects; heap overflow only on corrupt saves | counters via perf-lab `PerfLabEffectApply` (H10) and H11; script-side remove-before-apply | EFFOVF |
| ClearAllActions no-op and most Action* refused while non-commandable | CONFIRMED (gate) | stunned/confused creatures drop scripted actions and ignore ClearAllActions; ActionMoveToLocation still queues | use `GetCommandable` guards (k_ai_master 9 sites); probe whether a queued move walks under a SetState mask (RunActions result window `0x5D6A60`) | enemy-AI, Mod 8 |
| `+0xEC := 0` writer `0x596A7A` | CORRECTED: dead code | none | remove from the writer list | none |
| State scripts bypass k_ai_master for confusion/fear | CONFIRMED | confused/frightened creatures run StateScripts.2da rows 3/2 each heartbeat | none; note | enemy-AI |
| GetEffectType returns 0 for states 7, 8, 9, 0xA, 0xF | CONFIRMED | scripts cannot see choke, horror, push, whirlwind, crush | none | enemy-AI |
| InSubAreas O(sub-areas) per walker step; each AoE adds a level-0 object | CONFIRMED structure, magnitude SPECULATIVE | cost grows with AoE count | none yet | AIQ, perf |
| Battle Meditation never reported by droids; non-selected companions never receive it | CONFIRMED | a droid-target family test is always FALSE (recast risk only if L2 tests a droid) | L2 tests the caster only | L2 |

## Hook sites
Gate-checked windows (section 5; boundary OK, bytes = exe, no relative branch inside, no inbound target inside):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI8-H1 | 0x59D966 | 6 | `88 82 F5 0F 00 00` | cold | EDX = creature, AL = new state, `[EDX+0xFF5]` = old; stolen reads AL: wrapper must preserve EAX |
| AI8-H3 | 0x59E8F4 | 10 | `6A 01 8B 55 F8 52 8B 45 F4 50` | cold | slot-6 on state end; start is the target of the `0x59E8EB` JMP (allowed); `[EBP-4]` creature, `[EBP-8]` id, `[EBP-0xC]` slot ptr; handler must not touch ESP |
| AI8-H9 | 0x67E70C | 10 | `8B 4D F0 51 8B 0D A8 B4 A1 00` | warm | GetHasSpellEffect: spell `[EBP-8]`, result `[EBP-0x10]`; on the two early paths `[EBP-0xC]` is the unresolved object (null-check before `+0x148`) |
| AI8-H10 | 0x544210 | 9 | `55 8B EC 81 EC 9C 00 00 00` | warm | ApplyEffect entry; **collides with live perf-lab `PerfLabEffectApply` (same address and length): extend it, never ship both** |

Batch-only (not gate-checked; distances to the sibling list > 1.2 MB):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI8-H2 | 0x59DEA0 | 10 | `C7 81 90 03 00 00 00 00 00 00` | cold | state apply: forced heartbeat stamp; state `[EBP-0x10]` |
| AI8-H4 | 0x56BB5B | 10 | `C6 45 D3 00 8B 8D 2C FF FF FF` | warm | state-script dispatch in the heartbeat |
| AI8-H5 | 0x62C47B | 13 | `6A 01 8B 4D 9C 8B 51 04 52 8B 45 BC 50` | cold (per 6 s per AoE) | AoE heartbeat script; start is a JMP target (`0x62C472`) |
| AI8-H6 / H7 | 0x62C6D9 / 0x62C750 | 13 / 13 | `6A 01 8B 4D CC 8B 51 04 52 8B 45 E0 50` / `6A 01 8B 55 CC 8B 42 04 50 8B 4D D4 51` | warm | AoE enter / exit scripts |
| AI8-H8 | 0x6D77AC | 8 | `6A 01 8B 8D 64 FB FF FF` | warm | attack refusal tail; 5 inbound branches land exactly on the start (allowed) |
| AI8-H11 | 0x5E5936 | 10 | `8B 55 B4 52 8B 8D 68 FF FF FF` | cold | IntList store: index `[EBP-0x4C]` vs count `[effect+0x30]` |

## Open items
- StateScripts.2da rows 1-3 `SCRIPTNAME` values and the other 33 rows (not in the exe; need the game 2DA); vanilla confusion/fear AI is whatever those scripts do.
- States 0xB, 0xC, 0x11 have no creator in the exe (absence; a computed state from a data table would be missed); immunity feats 98-100/200 not read in `feat.2da`; type 0x1B meaning (`0x599E40`) inferred.
- Whether a move queued while non-commandable (`0x68CEE0`) walks under a SetState mask: probe type-1 results during stun (window `0x5D6A60`).
- Identity of effect-table slot 15 (`0x59ACC0`, conditional `+0xEC := 0`) and of effect type 89 (AI5).
- Mask bits 0x01, 0x10, 0x20, 0x100: no server reader found; client readers other than bit 2 not decoded.
- Runtime probes proposed by the batch (observation-only):

| id | question | hook | expected signal |
|---|---|---|---|
| P8-1 | live state-id histogram, replacement rate | AI8-H1 `0x59D966`: old `[EDX+0xFF5]`, new AL, creature id | which states occur on party/enemy; replacements per fight |
| P8-2 | does the forced heartbeat fire on the next AIUpdate | AI8-H2 `0x59DEA0` + AI8-H4 `0x56BB5B` timestamps | ms between apply and state-script dispatch; rows 3/2/1 per state |
| P8-3 | slot 6 on state end vs the EndRound script firing | AI8-H3 `0x59E8F4`: creature, state | count per state; compare with the k_ai_master EndRound log |
| P8-4 | mask refusal of attacks | AI8-H8 `0x6D77AC`: `[creature+0x1114]`, dead flag, HP, target == self | share of refusals from bits 0x80/0x04 vs dead |
| P8-5 | GetHasSpellEffect truth table for L2 | AI8-H9 `0x67E70C`: spell, object id, result; dump the target's `+0x148` spell ids | BM true for 3 non-droid members, false for droids |
| P8-6 | AoE cadence | AI8-H5/H6/H7: AoE id, slot name, ms | heartbeat period vs 6000 ms; enter/exit per walker step |
| P8-7 | effect stacking | `0x544210` entry (reuse perf-lab `PerfLabEffectApply`): adds minus removes per (object, type); max E per creature | growth of E over a session (EFFOVF) |
| P8-8 | IntList overflow guard | AI8-H11 `0x5E5936`: `[EBP-0x4C]` vs `[effect+0x30]` | count > NumIntegers only on a corrupt save |
- Whether AoE enter/exit fires for non-creature objects moved by scripts; per-pass interval of level-0 objects in a busy area (AoE heartbeat jitter, low); `0x6FDAB0` VM structure pop/copy (copy-per-call semantics for a shared link effect applied to 3 members).

## Mod relevance
| item | served by |
|---|---|
| L2 | key behaviours 10-11: family test correct per member, droids never report BM, max 3 active, L2 risk LOW |
| L1 / L3 | `HasTalent` hides Force powers while mask bit 8 is clear; confusion/stun do not clear bit 8 but `+0xEC == 0` makes `AddCastSpellActions` refuse |
| enemy-AI | key behaviours 2-8: state statues, state scripts rows 3/2, `GetCommandable` guards |
| A1 / a1_retarget_strict / E2 | slot 6 fires on end/replacement of states 1, 3-10, 0xE-0x13 (restarts DetermineCombatRound); fear never does |
| medpac (R4) | `GetIsDebilitated` true for every SetState except confusion; HP < 1 party counts as debilitated |
| Mod 8 (kite) | `+0x1114` bit 2 fails move actions only in specific handlers; ActionMoveToLocation still queues while `+0xEC == 0` (gate-corrected) |
| party-flag (`+0x11AC`) | `IsDebilitated` / handlers use `+0x11AC != 0 && HP < 1` as "downed party member"; nothing new about the flag |
| EFFOVF | key behaviour 13; hooks H10 / H11 |
| AIQ / perf | AoE objects are level-0 AI objects; `InSubAreas` per walker step; `UpdateEffectList` O(E) per AIUpdate |
| E9, P1, P2, FM* | none (state-end perception refresh after `0x59E860` is the only perception link) |
