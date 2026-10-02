# AI7 Script <-> engine interface (KOTOR II Steam swkotor2.exe 6a522e71)

Pipeline position: the NCS interpreter reads a signed 16-bit command id and calls `CSWVirtualMachineCommands::ExecuteCommand 0x668FD0` -> handler from the command table (`malloc(0xDB4)`, built by `InitializeCommands 0x665F50`) -> engine function. In the other direction, engine events reach scripts through `CSWSCreature::EventHandler 0x56EFD0` (14 creature script slots at `+0x270 + 8n`, run through `CVirtualMachine::RunScript 0x6FD8D0`). This page covers the command table and cost classes, the commands our AI scripts depend on, the event id -> script slot table, `GetCurrentAction` (`0x67B100`) and what it reports during engine approaches (E4), talent selection internals (`GetCreatureTalentBest/Random`, `HasTalent`), and the medpac Force-class finding.

## Status
- Sources: batch `findings/AI7.md` (Sonnet) + `GATE_wave2.md` (Opus gate, 2026-10-02; section 1 AI7 verdicts, 1.5 medpac/Force class, 1.6 GetCurrentAction, section 2 reconciliation 1, 3, 4, 7, section 3 confidence, section 4 names, lessons 6-8, 12) + `LEAD_SPOTCHECKS.md` (AI7: item-spell gate lines, GetCurrentAction loop lines CONFIRMED; the Force-class set confirmed by decompile).
- Date: 2026-10-02. Static analysis only. The gate **re-derived the command table independently** (the lead noted AI6 had overwritten shared scratch files while AI7 ran): 774 `MOV [reg+off],imm` stores in `0x665F50`, each byte-compared with the exe (0 mismatches), index = off/4 joined to the 877 `nwscript.nss` prototypes. Handler self-checks: `0x67AE40` tests `0x133/0x134`, `0x67C430` tests `0xEB`.
- Gate-verified: command table, medpac gate (`0x6F2A30` = IsForceUserClass, classes.2da), GetCurrentAction groups, nCRMax dead, nExcludeId not applied to spells, slot 7 stores the pattern number. Cost magnitudes (GetNearestCreature) stay low.
- Conflicts resolved to the gate version are marked "batch said X; gate corrected".

## Function table
| addr | name | conf | purpose |
|---|---|---|---|
| 0x665F50 | CSWVirtualMachineCommands::InitializeCommands | high | `PUSH 0xDB4`, zero 0x36D slots, 774 handler stores (`00665f84 CMP [EBP-4],0x36D`) |
| 0x668FD0 | ExecuteCommand(id, argc) | high | `id < 0x36D && table[id] != 0` -> `handler(id,argc)` else -2002 (`00668fd9`, `00669008 CALL EAX`, `00669012`) |
| 0x6FD8D0 | CVirtualMachine::RunScript(CExoString*, objId, flag) | high | 63 callers = every engine script run |
| 0x56EFD0 | CSWSCreature::EventHandler(eventId, callerId, payload, day, ms) ret 20 | high | per-event dispatch incl. creature slots; first action of every event: `0056f011..0056f022 CALL 0x56B8A0` when `+0x38C == 0` |
| 0x683D40 | cmds 38, 226-229 GetNearestCreature / Object (+ToLocation, ByTag) | high | gate rename (DB GetNearestObject); x-sorted sweep of `area+0x194` |
| 0x67ACC0 | cmd 306 GetCreatureHasTalent | high | -> `0x6B8840(type,id)` (`0067ad56`) |
| 0x67AE40 | cmds 307/308 GetCreatureTalentRandom/Best | high | gate rename (DB GetCreatureTalent); -> `0x6B8BB0` |
| 0x6B8840 | CSWSCreatureStats::HasTalent(type, id) ret 8 | high | class-slot loop, FP cost, item scan (`006b89ce CALL 0x6F2A30`, `006b8a6c CALL 0x6C0330`, `006b8aa3` mask bit 8) |
| 0x6B8BB0 | CSWSCreatureStats::GetTalentBestOrRandom | high | coin flip spell group vs feat group (`006b8c2b CALL 0x91EB8B`, `006b8c30 AND EAX,0x80000001`) |
| 0x6BAEC0 / 0x6BA970 / 0x6BB3A0 / 0x6BC0E0 | PickKnownSpellTalent / IsSpellTalentCandidate / PickItemSpellTalent / PickFeatTalent | med / high / high / med | pickers; `006bb0ca`, `006bc23e`, `006bb707` compare crMax only with -1 |
| 0x6F2A30 | IsForceUserClass(classId) {3,4,5,11..16} | high | gate rename (batch IsForceClass, names SPECULATIVE); chain `006f2a34..006f2a68`; equals classes.2da rows with `forcedie` > 0 |
| 0x6C0330 / 0x6C06B0 | HasItemSpell / FindItemProvidingSpell | high | scan 20 equip slots then the shared repository for a property type 10 |
| 0x57D8B0 | CSWSCreature::UseTalentOnObject | med | item talent -> `0x57B4C0 UseItem`; spell without item -> `0x6C06B0`; feat -> AddAttackActions; skill -> UseSkill |
| 0x67B100 | cmd 522 GetCurrentAction | high | last reportable node of the head action group |
| 0x5D7190 / 0x5D7220 | ActionTypeIsReportable / ActionTypeToScriptAction | high | the type -> `ACTION_*` map |
| 0x68BBE0 / 0x669FC0 / 0x68B4B0 | cmd 576 IsObjectPartyMember / 730 ActionFollowLeader / 140 GetIsDead | high | `+0x11AC`; needs `+0x11AC != 0 && +0xEC == 1`; IsDead (vt+0x94) or (`+0x11AC` and HP <= 0); `0x68B4B0` gate rename (DB IsAIState) |
| 0x67C430 | cmds 235-237 GetIsEnemy/Friend/Neutral | high | gate rename (DB GetEmotions); reputation (AI6) |
| 0x67E670 / 0x5476C0 | cmd 304 GetHasSpellEffect / HasSpellEffectApplied | high | linear scan for `effect+0x1C == spell` (AI8) |
| 0x68CEE0 / 0x68CB50 | cmds 21/382 ActionMoveToLocation / ForceMoveToLocation, 22/383 ActionMoveToObject | high | gate rename (DB MoveToPoint); `0x68CEE0` has no `+0xEC` test (AI8) |
| 0x66DAF0 | cmd 37 ActionAttack | high | gate rename; `AddAttackActions(target,0,0,passive,...)` |
| 0x693860 | cmds 221/39 SpeakString / ActionSpeakString | high | volume map, shout dispatch (AI6) |
| 0x693020 / 0x678EB0 | cmds 131/132 SignalEvent / EventUserDefined | high | event 10, payload type 0xB (slot 11), delta 0 |
| 0x7E5C40 / 0x73FB90 | CSWParty::IsPartyMember / CClientExoApp::GetParty | high | engine's "active party" test for the OnNotice event only (AI5) |
| 0x599A10 | apply-table handler for effect 89 (slot-8 site) | low | gate: creates a new creature (`new(0x1220)`); batch "raise/resurrect" not supported |

## Key behaviours
1. **Command table facts (gate, CONFIRMED).** Index = store offset / 4 = `nwscript.nss` prototype order; 877 prototypes = 0x36D slots; **774 filled** (about 607 distinct handlers); the 103 empty slots are `SWMG_*` / `AurPostString` (idx 520/521, 582-668, 683-688, 717/718, 782, 789, 790, 792, 804, 825, batch list); idx 0 `Random` = `0x68F5D0`. Handler convention: thiscall, ECX = implementer (`[ECX+4]` OBJECT_SELF valid, `[ECX+8]` OBJECT_SELF id, `[ECX+0xC]` table), stack `(id, argc)`; arguments pop in nwscript parameter order. Return 0 ok, `0xFFFFF82F` (-2001) pop failed, `0xFFFFF830` (-2000) push failed, `0xFFFFF82E` (-2002) unknown/empty slot. The gate's 8 key commands: GetNearestCreature 38 `0x683D40`, GetCreatureHasTalent 306 `0x67ACC0`, GetCreatureTalentBest 308 / Random 307 `0x67AE40`, ActionAttack 37 `0x66DAF0`, GetCurrentAction 522 `0x67B100`, GetHasSpellEffect 304 `0x67E670`, IsObjectPartyMember 576 `0x68BBE0`, ActionFollowLeader 730 `0x669FC0`. Also verified: 9 ClearAllActions `0x66F330`, 21/382 `0x68CEE0`, 22/383 `0x68CB50`, 140 GetIsDead `0x68B4B0`, 162/163 `0x690F40/0x67AC20`, 195 `0x683740`, 208/209 `0x687E00/0x66D1B0`, 221/39 `0x693860`, 235-237 `0x67C430`, 309 `0x66BDC0`, 732 `0x696D80`. Lesson 12: DB names for command handlers from the GOG mapper were wrong 7 times; re-derive from the stores, do not reuse a pickle.
2. **Commands our AI scripts use** (index, handler, cost class, notes). Cost classes: C0 = O(1) field read + one hash lookup `0x53DFB0`; per-obj = scales with objects in the area (the X-sorted array holds ALL objects, not only creatures); per-eff = effects on the object (cap 1000, AI4); per-inv = inventory. Usage notes come from AI7/AI8/AI6 text (k_ai_master call sites as quoted there); "-" = engine-interface fact only.

| idx | command | handler | cost | notes |
|---|---|---|---|---|
| 38 | GetNearestCreature | 0x683D40 | per-obj | k_ai_master calls it with REPUTATION_ENEMY + PERCEPTION_SEEN at about 25 sites (`k_ai_master.nss:54, :238, :452, :525`). Candidate list `area+0x194`, outward walk (+1, -1, +2, ...), prune when X-distance^2 exceeds the nth-best 3D distance; with no match the whole array is swept; dead creatures excluded; `CREATURE_TYPE_IS_ALIVE` (`case 4: break;`) never matches |
| 226-229 | GetNearestCreatureToLocation / Object / ToLocation / ByTag | 0x683D40 | per-obj | same sweep, other filters |
| 128/129 | GetFirst/NextObjectInShape | 0x685960 | per-obj in x-slab | cursor in the area (`area+0x1A0`): nested First/Next loops corrupt each other |
| 306 | GetCreatureHasTalent | 0x67ACC0 | per class slot; item scan per-inv | medpac / L2 guards; false for Force-only classes with item spells (below) |
| 307 / 308 | GetCreatureTalentRandom / Best | 0x67AE40 | known spells x HasTalent | L2 buff pick `GetCreatureTalentBest(0x1408, 20)`; nCRMax dead |
| 309 | ActionUseTalentOnObject | 0x66BDC0 | per-inv for spells | item fields go straight to UseItem (`0x57B4C0`), no HasTalent |
| 37 | ActionAttack | 0x66DAF0 | C0 + queue | queues one 0xC (or 0x3F) node, group new (0xFFFF) |
| 9 | ClearAllActions | 0x66F330 | O(queue) | `0x541080(1)`; no-op unless `+0xEC == 1` (AI8) |
| 21 / 382 | ActionMoveToLocation / ForceMoveToLocation | 0x68CEE0 | C0 + queue | Mod 8 kite; one type-1 node; NOT `+0xEC` gated (gate, AI8) |
| 22 / 383 | ActionMoveToObject | 0x68CB50 | C0 + 3 nodes | chain [1, 0x11, 0x13] in ONE group; reports MOVETOPOINT |
| 730 | ActionFollowLeader | 0x669FC0 | C0 | no-op unless `+0x11AC != 0 && +0xEC == 1` |
| 522 | GetCurrentAction | 0x67B100 | O(group length) | E4; see key behaviour 4 |
| 304 | GetHasSpellEffect | 0x67E670 | per-eff | L2; AI8 |
| 163 / 162 | GetCommandable / SetCommandable | 0x67AC20 / 0x690F40 | C0 | `+0xEC`; k_ai_master tests it at 9 sites (AI8) |
| 576 | IsObjectPartyMember | 0x68BBE0 | C0 | raw `creature+0x11AC` |
| 140 | GetIsDead | 0x68B4B0 | C0 | never true via IsDead for `+0x11AC != 0`; HP <= 0 test instead |
| 235-237 | GetIsEnemy / Friend / Neutral | 0x67C430 | C0 + reputation | enemy `< 11`, friend `> 89`; script shout filter (AI6) |
| 289 / 290 | GetObjectSeen / GetObjectHeard | 0x686990 | O(visible list) | entry of the SOURCE, bit0 / bit1 |
| 221 | SpeakString | 0x693860 | AI6 | volumes {0,1,2,3,4} -> {1,3,2,0xD,0xE}; SILENT_TALK for pattern 15 |
| 195 / 254 | GetListenPatternNumber / GetLastSpeaker | 0x683740 / 0x682900 | C0 | `+0x1BC` pattern number / creature `+0x1A0` |
| 131 / 132 | SignalEvent / EventUserDefined | 0x693020 / 0x678EB0 | C0 + O(n) sorted insert | event 10, payload 0xB, delta 0 |
| 85-87 | GetFirst/NextEffect / RemoveEffect | 0x67BD70 / 0x68F7E0 | per-eff | cursor `obj+0x210`; remove = flag + event 0xE |
| 30 / 339 / 340 | GetItemPossessedBy / GetFirst/NextItemInInventory | 0x681050 / 0x680830 | per-inv / C0 | repository `0x58A2C0(1)` |
| 6 / 7 / 294 / 8 | AssignCommand / DelayCommand / ActionDoCommand / ExecuteScript | 0x66D9F0 / 0x670370 / 0x6699D0 / 0x679050 | O(n) insert, NCS copy | U1 |
| 54 | CancelCombat | 0x697090 | C0 | `0x586750` |
| 820 | HasLineOfSight | 0x69AC60 | one LOS ray | `0x52BB60` |
| 732 | GetIsDebilitated | 0x696D80 | C0 | `IsDebilitated(0)` (AI8) |

3. **Event id -> script slot table (`0x56EFD0`, all 14 slots).** Event ids are consecutive dwords at `0x9923F4..0x992460`; event 10 `[0x99241C]` = `EVENT_SIGNAL_EVENT` with the payload word selecting the case. Slot offsets `0x270+8n` (SaveCreature `0x574500`; `index < 0xE` guard `0056fa5c`).

| slot | name | engine run site(s) | trigger | skip conditions |
|---|---|---|---|---|
| 0 | Heartbeat | `0x56B8A0` (`param_1+0x9C`) | AIUpdate every pass (`005694e3`) | IsDead; interval >= `+0x398` (3000..4199 ms) and (level != 0 or counter `+0x3D4 >= +0x398>>6`) |
| 1 | OnNotice | `00570640` non-party / `0057069a` party flag | event 10, payload type 1 (`0x576DC0`) | party-flag creatures run it only if `0x7E5C40(clientId)` (`005705e4`/`005705eb`) |
| 2 | SpellAt | `00570d08` | event 10, payload type 2 (poster not traced) | IsDead or (`+0x11AC` and HP < 1) |
| 3 | Attacked | `0056faa1` | event 0xF per hit (AI2) | IsDead (`0056f9fc`); party member HP <= 0 (`0056fa08..0056fa4c`) |
| 4 | Damaged | `0x5959E0` `005966d0` direct call | effect Damage apply | skipped for the party leader (`0x5FE3B0`, AI2) |
| 5 | Disturbed | `00570f55` | event 10, payload type 0x1B | none seen |
| 6 | EndRound | `0x58F50F` (EndCombatRound), `0x569A67` (AIUpdate 3 s fallback), `0x59E904` (SetState removal) | AI2 / GATE | AI2 conditions; third site ungated |
| 7 | Dialogue | `005709f0` | event 10, payload type 7 (shout, AI6) | stores speaker `+0x1A0` and the **pattern number** at `+0x1BC`; empty or "default" resref -> `k_hen_dialogue01`; no dead gate |
| 8 | Spawn | `0x56B8A0` first block (`+0x38C == 0`); `0x599A10` | first event / first AIUpdate | once per spawn; gate: `0x599A10` creates a new creature (low) |
| 9 | Rested | NONE found | - | med (absence: every `0x6FD8D0` caller in the creature classes uses other slots) |
| 10 | Death | `0x597D90` (`param_1+0xB0`) after `0x58E990(0)` | death handling | conditions in `0x597D90` not traced |
| 11 | UserDefined | `00570a72` | event 10, payload type 0xB (SignalEvent + EventUserDefined) | none |
| 12 | OnBlocked | `00570745` (`MOV EAX,0xC`) | event 10, payload type 0x1F (6 movement sites, AI1) | skipped for a player-controlled creature (`005706cf`/`005706e1`); sets `+0x37C` = blocker |
| 13 | EndDialogue | `0x58A500` (`param_1+0x2D8`, caller `0x6CF740`) | conversation end | none |

   Batch said slot 7 stores a "volume" at `+0x1BC`; gate corrected: pattern number. Events without a slot: 1 timed situation (`0x6FD930`), 4 RemoveFromArea, 5 ApplyEffect `0x544210`, 8/0x13 run the script named in the payload, 9 SetAnimation, 0xB death finalisation, 0xE RemoveEffectById, 0x12 `0x581070`-like (med/low names), 0x1B PlayRumble.
4. **GetCurrentAction mapping (gate-corrected).** `0x67B100`: result init `0xFFFF` (`0067b109`); head node `0x521060`, group `word [node+0x6C]` (`0067b1a4`); loop `0067b1bd CMP EDX,EAX / 0067b1bf JNZ` (stop at a new group), `0067b1ca CALL 0x5D7190` (reportable), `0067b1df CALL 0x5D7220`, `0067b1e7 MOV [EBP-8],EDX` (overwrites per reportable node), `0067b1f9 JMP`; empty list -> `0xFFFE` (`0067b1fd`). So the result is the **last reportable node of the head group**. Reportable set includes 1, 0xC, 0x3D (-> 0x26) and 0x3F (-> 0x27); it excludes 0x11, 0x13, 0x40. Groups:
   - **0xC approach -> 3 (ATTACKOBJECT):** `006d871c MOVZX EDX,word [ECX+0x6C]` / `006d8721 PUSH 0xC` / `006d8729 CALL 0x00540CA0`, so [move, 0x13, 0x11, 0xC] share one group.
   - **0x3F round approach -> 39 (0x27):** `006da0b5 MOVZX EAX,word [EDX+0x6C]` / `006da0ba PUSH 0x11` / `006da0c2 CALL 0x00540CA0`; batch said "an attack round reports ATTACKOBJECT"; gate corrected: 39.
   - **Follow -> 38 (0x26):** state 0 `0x5C7370` queues 0x40 + move in the 0x3D node's group (`FUN_005d5120(0x3d)`).
   - **MOVETOPOINT (0)** only for script move chains (ActionMoveToLocation, ActionMoveToObject [1, 0x11, 0x13]) and other script-built groups. QUEUEEMPTY 0xFFFE only for an empty list; 0xFFFF when nothing in the head group is reportable.

| node type | handler | value | constant |
|---|---|---|---|
| 0x01 MoveToPoint | 0x5BFC00 | 0 | ACTION_MOVETOPOINT |
| 0x0C PhysicalAttacks | 0x6D7390 | 3 | ACTION_ATTACKOBJECT |
| 0x0F CastSpell | 0x5B96F0 | 4 | ACTION_CASTSPELL |
| 0x1E Wait | 0x5D9AA0 | 0x24 | ACTION_WAIT |
| 0x2E ItemCastSpell | 0x5B8000 | 0x13 | ACTION_ITEMCASTSPELL |
| 0x3D FollowLeader | 0x5BF7F0 | 0x26 (38) | ACTION_FOLLOWLEADER |
| 0x3F Combat | 0x6D9500 | 0x27 (39) | none (number only) |
| 0x46 FollowOwner | 0x5BF940 | 0x2B | ACTION_FOLLOWOWNER |
| 0x47 re-equip + resume attack | 0x5DF920 | 0x2C | none |
(full list in the batch Q3: pickups, doors 5/6, mines 8-0xC, locks 0xD-0xF, stubs 0x10-0x12, counterspell 0x1F, heal 0x21, follow 0x23, 0x43-0x45 = 0x28-0x2A.)
   **E4 consequence:** `GetCurrentAction() == ACTION_MOVETOPOINT` holds only for script moves. An engine approach (FM2 / FM2b loops, the `0x11` re-approach) and a follow walk never report 0 (3, 39 and 38 instead). **Design correction (REVIEW_synthesis 2026-10-02):** widening the key to {3, 39} is wrong (3 = a ranged attacker standing in range, 38 = an idle follower). E4 = {0 (script moves)} plus {38 with leader distance > follow range, same 2 m cell for N heartbeats}; engine approaches (3/39) go to the DLL FM2 fail-fast on the `0x11` re-approach count (GATE_wave1 1.1). Probe AI7-P7ACT expects 3 / 39 / 38, not 0, during approach/follow.
5. **Talent selection internals.**
   1. Group choice (`0x6B8BB0`): `rand() & 1`: 0 = spell group (`0x6BAEC0` known spells, else `0x6BAE00` class-granted Force powers via `0x6F2A30` -> `0x6BCFA0`, else `0x6BB3A0` item spells), 1 = feat group (`0x6BC0E0`); a group that finds nothing is marked done and the other is tried. Best is not "the best of the category": a 50/50 pick between best spell and best feat when both match.
   2. **`nCRMax` is dead (CONFIRMED).** Only compares: `006bb0ca CMP [EBP+0xC],-1` (`0x6BAEC0`), `006bc23e` (`0x6BC0E0`), `006bb707`/`006bbc46` (`0x6BB3A0`), `006bd4f7` (`0x6BCFA0`); forwarded as arg 4 of `0x6BA970` (`006bb6c5`, `006bd4d4`), which never reads `[EBP+0x14]`. So `GetCreatureTalentBest(cat, 20)` == `(cat, 1)` == `(cat, 99)`; -1 turns Best into Random.
   3. Ranking: spells by spells.2da `MaxCR` (`row+0x134`), category `row+0x130`; feats by `feat+0x14` / `+0x10`. Highest MaxCR wins, ties `rand() % n`; Random collects every match.
   4. Category match `0x6BDD60`: 4 nibbles, wanted nibble 0xF = wildcard (`0xF8FF`); spell rows also need `0x6BDE30` (2DA column "Exclusion" non-zero excludes; `nInclusion` has no effect, SPECULATIVE intent).
   5. **`nExcludeId` not applied to spells (CONFIRMED):** `006bb07e PUSH -1` (arg 8) / `006bb080 MOV EDX,[EBP+0x24]` (arg 7); `006ba97c CMP EAX,[EBP+0x24]` compares with the constant -1. Feats do apply it.
   6. **Exclusion-type semantics (gate-corrected).** Batch said "(0,-1) drops the spell group, (1,-1) drops the feat group"; gate corrected: (0,-1) still runs the spell group once (`006b8c59 CMP [EBP+0x30],-1` / `006b8c5f MOV [EBP-0x10],1` marks the spell group done, then the pickers still run with exclusion -1, `006b8ca4 CALL 0x006baec0`); (1,-1) marks the SPELL group done (`006b8d29 CMP [EBP+0x30],-1` / `006b8d2f MOV [EBP-0x10],1`), not the feat group (`[EBP-0x14]`). Our scripts never pass exclusions (grep: 0 uses).
   7. Per-candidate test `0x6BA970`: spell != excl, then `HasTalent(0, spell)` BEFORE the category test (cost = #known spells x HasTalent), then area and `0x6BDD60`.
   8. Item talents: equip-slot pass `006bb6db CALL 0x006BA970` (HasTalent inside); the repository pass in `0x6BB3A0` uses only `FUN_006bdd60(row+0x130, cat)`: no HasTalent, no Exclusion, no class test. An item counts with a property of type 10 (active, usable-class bit), spell = property subtype.
   9. Use: `ActionUseTalentOnObject` -> `0x57D8B0`; requires `creature+0xEC != 0`.
6. **Medpac Force-class finding (AI-MEDPAC-FORCECLASS, gate 1.5).** `HasTalent 0x6B8840` (type 0) loops the class slots: **Force class** slot -> only `0x6BD9C0 && 0x6B8F40` (known spell and cost payable); **non-Force class** slot -> innate -> known -> `006b8a6c CALL 0x6C0330` (item scan); true if any slot succeeds, AND mask bit 8 (`+0x1114`).
   - `0x6F2A30` asm: `006f2a34 CMP [EBP+0x8],0x3` ... `006f2a64 CMP [EBP+0x8],0xC`, then 1 for {3,4,5,0x10,0xF,0xE,0xD,0xB,0xC}. classes.2da rows (workspace copies; row ids vanilla): 0 Soldier, 1 Scout, 2 Scoundrel, **3 JediGuardian, 4 JediConsular, 5 JediSentinel**, 6 CombatDroid, 7 ExpertDroid, 8 Minion, 9 TechSpecialist, 10 BountyHunter (cut), **11 JediWeaponmaster, 12 JediMaster, 13 JediWatchman, 14 SithMarauder, 15 SithLord, 16 SithAssassin**. The Force set is exactly the rows with `forcedie` > 0. Other callers fit (GetMaxForcePoints `0x57EDD6`, AddGrantedPowers `0x847ACE`, AutoLevelup).
   - Companion class lists (UTC): Kreia [(4,3)], Visas [(5,6)], Handmaiden [(0,...)], Disciple [(0,...)], Atton [(2,...)], Bao-Dur [(9,...)], Mira [(1,...)], Hanharr [(1,...)], HK-47 [(6,...)], T3-M4 [(7,...)], G0-T0 [(7,...)]. Level-ups only add Jedi or prestige classes, so a non-Force base slot persists.
   - Category 0x1408 matches only `ITEM_ABILITY_MED_PACK` 64, `ADVANCED_MED_PACK` 65, `LIFE_SUPPORT_KIT` 66, `RECOVERY_STIM` 129 (spells.2da). Force Heal is 0x2441.
   - **Result:** `GetCreatureTalentBest(0x1408, 20)` returns a medpac talent for anyone with stock (repository pass has no class test), but `GetCreatureHasTalent(tSpe11_05)` is FALSE for **Kreia (class 4 only), Visas (5 only) and the Exile (always: every K2 PC class is in {3,4,5,11..16}; REVIEW_synthesis E5)** even with medpacs. The R4 rule (and vanilla NPC medpac use) never fires for them; failure is safe (no idle loop) but they never use medpacs. Works for everyone else, droids included (repair kits). Batch/R4 said `GetCreatureHasTalent(tSpe11_05)` implies stock >= 1 for any creature (lesson 280); gate: the implication holds, the converse fails for Force-only class lists; lesson 280 and R4 row 4 need a correction line.
   - **Fix idea (script-only, low risk):** for party members use `GetIsTalentValid(tSpe11_05)` + `GN_PartyMedpacStock() >= 1` instead of GetCreatureHasTalent; `ActionUseTalentOnObject` with the item fields goes straight to UseItem (`0x57D8B0` -> `0x57B4C0`) and needs no HasTalent.
7. **Party facts at the script boundary.** IsObjectPartyMember returns raw `+0x11AC`; ActionFollowLeader needs `+0x11AC != 0 && +0xEC == 1`; GetIsDead = IsDead (never true for `+0x11AC != 0`) OR (`+0x11AC != 0` and HP <= 0). The engine's own "active party" test `0x7E5C40` applies to the OnNotice event only; for DLLs `+0x11AC` and `0x7E5C40` agree except during transients (AI5).
8. **Cost laws.** GetNearestCreature / GetFirstObjectInShape / GetFirstEffect keep cursors in the target or area; nested loops on one target clash. `ExecuteScript` runs a full NCS (memcpy per run, U1). A typical area has 100-600 objects (magnitude SPECULATIVE).

## Struct fields
| class | offset | type | name | conf | evidence |
|---|---|---|---|---|---|
| CSWVirtualMachineCommands | 0x04 / 0x08 / 0x0C | int / objid / ptr | OBJECT_SELF valid / OBJECT_SELF / handler table | high | `00668fe5 MOV ECX,[EAX+0xC]` |
| CSWSCreature | 0x270+8n (n = 0..13) | CExoString | script slot n | high | `0057464f` |
| CSWSCreature | 0xEC | int | commandable | high | `0066a001`, `00691004` |
| CSWSCreature | 0x11AC | int | party flag (raw value of IsObjectPartyMember) | high | `0068bc2e` |
| CSWSCreature | 0x1114 | u16 | allowed-action mask (init 0xFFFF; bit 8 for HasTalent) | high | `005625a0`, `006b8aa3` (gate raised from med) |
| CSWSCreature | 0x520 / 0x548 / 0x550 / 0x568 / 0x580 | int / id | in combat / attack target / attempted attack target / attempted spell target / last hostile target | high | `006dadb5`, `0057e3dd`, `00565ecb`, `0058f618` |
| CSWSCreature | 0x1A0 / 0x1BC (CSWSObject) | id / int | last speaker / last pattern number | high | AI6 |
| CSWSCreatureStats | 0x8D / 0xAB+i*0x28 | byte / byte | class count / class id of slot i | high | `006b89b9` |
| CSWSCreatureStats | 0x130 / 0x132 / 0x134 | i16 / i16 / ptr | Force points current / extra / known-spell vector (12 B entries) | med / high | `0x6B8F40`, `0x6BAEC0` |
| CSWRules | 0x104 / 0x108 / 0x12C | table | spells (rows 0x1A8, `+0x130` Category, `+0x134` MaxCR) / feats (rows 0x50) / skills | high / high / med | `006c1699`, `0069ed97` |
| CSWSObject | 0x100 / 0x148 / 0x14C / 0x210 | list / ptr / int / u16 | action list / effect array / count / GetFirst/NextEffect cursor | high | `0067b17a`, `0x5476C0` |
| CGameEffect | 0x1C / 0x20 | int | creator spell id / active flag | high | `0x5476C0`, `0x68F7E0` |
| CSWSArea | 0x194 / 0x198 / 0x1A0 | id* / int / int | X-sorted object ids / count / GetFirst/NextObjectInShape cursor | high | `0x522AF0`, `0x685960` |

## Bugs and candidates
| item | verdict | user impact | fix idea | mod item |
|---|---|---|---|---|
| AI-MEDPAC-FORCECLASS: GetCreatureHasTalent(item spell) FALSE for Force-only classes | CONFIRMED | Kreia, Visas and the Exile (always) never use medpacs via the R4 rule (and the vanilla NPC rule) | script: party branch uses `GetIsTalentValid(tSpe11_05)` + stock >= 1; update lesson 280 and R4 row 4 | medpac, L2 |
| E4 keyed on ACTION_MOVETOPOINT misses engine approaches | CONFIRMED (gate-corrected) | stuck-move detection never sees FM2/FM2b loops or follow walks | corrected (REVIEW_synthesis): E4 = {0} + {38 with leader distance > follow range, no progress}; 3/39 are left to the DLL FM2 fail-fast on the `0x11` re-approach count | E4, FM2b |
| AI-TALENTEXCL: TalentBest exclusion flags set the wrong group | CONFIRMED, no current use | none for our scripts (no exclusion args) | document only; do not rely on nExcludeType/Id | none |
| `nCRMax` dead; coin flip spell vs feat; per-known-spell HasTalent | CONFIRMED | `GetCreatureTalentBest(cat, 20)` is not "best of category"; script cost scales with known spells | do not rely on the CR cap | L2, A1, enemy-AI, perf |
| Slot 9 (Rested) never run | med (absence) | none | probe P7-RS | none |
| Nested GetFirstObjectInShape / GetFirstEffect loops corrupt cursors | CONFIRMED | scripts with a call inside the loop that loops again break | avoid nesting | L2, EFFOVF-adjacent |
| DB name_map corrections (0x67C430, 0x683740, 0x68B4B0, 0x683D40, 0x68CEE0, 0x67AE40) | CORRECTED | none (naming) | GATE_wave2 section 4 | none |

## Hook sites
Gate-checked windows (section 5; boundary OK, bytes = exe, no relative branch inside, no inbound target inside, jump tables scanned):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI7-EV | 0x56EFF9 | 6 | `89 8D 18 FD FF FF` | HOT (every event) | ECX = creature, `[EBP+8]` event id, `[EBP+0x10]` payload (word = sub-type); stolen writes memory only; keep the handler to a few compares |
| AI7-P7ACT | 0x67B204 | 10 | `8B 4D F8 51 8B 0D A8 B4 A1 00` | warm | `[EBP-8]` = result on every path (init 0xFFFF); window start is the target of `0x67B14C/0x67B161/0x67B183/0x67B1FB` (allowed); expect 3 / 39 / 38 during approach/follow |

Batch-only (not gate-checked; the batch scanned inbound targets; perf-lab already hooks `0x668FD0` and `0x7022A0`, do not stack a second detour):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI7-RS | 0x6FD8D0 | 6 | `55 8B EC 83 EC 08` | hot-ish | script runs per (creature, slot); `[esp]` return address identifies the engine site |
| AI7-CMD-OUT | 0x66900A | 6 | `89 45 FC 8B 45 FC` | hot | ExecuteCommand exit; EAX = result; EAX preserved |
| AI7-GNC | 0x68433C | 6 | `8B 95 58 FF FF FF` | hot | candidate id `[EBP-0xA8]`; stolen load writes EDX (live): restore it |
| AI7-GNC-IN | 0x683D40 | 10 | `55 8B EC 6A FF 68 F1 A4 95 00` | warm | GetNearestCreature entry |
| AI7-TB | 0x6B8BB0 | 6 | `55 8B EC 83 EC 20` | warm | Best/Random: category, crMax; 585 B from `0x6B8DF9` (finesse fix) |
| AI7-HAS | 0x6B8840 | 6 | `55 8B EC 83 EC 4C` | warm | HasTalent calls by type and return address |
| AI7-INV | 0x6C0330 | 9 | `55 8B EC 81 EC 94 00 00 00` | warm | item-scan calls and QPC time |

## Open items
- Poster of event-10 payload types 2 (SpellAt) and 0x1B (Disturbed), and of events 8/0x13 (script named in the payload): not traced.
- spells.2da `Exclusion` / `MaxCR` values (2DA live override) needed to predict Best's winner; `0x6B8F40` / `0x6C4120` internals (cost-reduction effect `DAT_009960d0`, direction not read).
- `0x5D53A0` (GetUserActionsPending) list vs a different queue; whether `+0x17C` (GetLastHostileActor) is a CSWSObject or CSWSCreature field.
- Module-embedded NCS scan for exclusion args to TalentBest.
- Medpac script fix for Force-only members (decide and ship); update lesson 280 and R4 row 4.
- E4 design (user): E4 = {0} + {38 far from the leader, no progress}; engine approaches (3/39) via the DLL FM2 fail-fast (REVIEW_synthesis section 4; {0,3,39} is not a valid key).

## Mod relevance
| item | served by |
|---|---|
| medpac | key behaviour 6, AI-MEDPAC-FORCECLASS, the class list and fix idea |
| E4, FM2, FM2b | key behaviour 4: engine approach reports 3 / 39, follow reports 38, MOVETOPOINT only for script moves; E1/E3 stand-off moves report MOVETOPOINT; exit5m / E7 `ACTION_FOLLOWLEADER` test |
| L2, L1, L3, A1, enemy-AI, perf | key behaviour 5: nCRMax dead, coin flip, HasTalent per known spell, no nExcludeId for spells |
| party-flag, FM1, FM4, E4 | key behaviour 7: IsObjectPartyMember = `+0x11AC`, ActionFollowLeader no-op rule |
| P1, P2, M8, enemy-E7, FM5, C1, ai_trace | key behaviour 3: event table; slot 6 (M8), slot 1 (`hold_enemies`), slot 12 skipped for the player (FM5), slot 7 / shout (C1, AI6); hooks AI7-RS / AI7-EV |
| perf, enemy-AI, E9 | GetNearestCreature sweep cost scales with area object count |
| Mod 8 (kite) | `ActionMoveToLocation 0x68CEE0` not `+0xEC` gated (AI8) |
| perf, ai_trace | command table, ExecuteCommand error codes |
