# HK1 T2: who re-adds the 0x3F / attack action after a combat-start ClearAllActions

Target: Steam `swkotor2.exe` (sha 6a522e71). Static only: `decomp/<bb>/<addr>.c`, `callgraph.tsv`, `callers_count.tsv`, plus names from `scripts/query_ghidra.py address` (steam-db) and repo docs. No REA, no game logs.

Dump root: `/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`

## 0. Facts and corrections

- `ClearAllActions` = `CSWSObject::ClearAllActions` 0x00541080. Direct CALL sites: **58 in 49 functions**. Matches the expected 58. No TAILJMP into it. No immediate-address references in the asm dumps.
- `AddAttackActions` = `CSWSCreature::AddAttackActions` 0x00568540. Direct CALL sites: **12 in 9 functions**. Matches the expected count.
- Action 0x3F = `CSWSCreature::AIActionCombat` (handler 0x006D9500, round container). It is not the attack chain itself.
- **Correction 1:** `AddAttackActions` adds 0x3F only in its `param_8 == 0` branch (decomp `56/00568540.c` lines 31-47: `FUN_00540ca0(0x3f,...)` gated by `FUN_0053f560(0x3f)==0`). **All 12 direct call sites pass `param_8 != 0`** (value 1, or the round type 1/0x0B). So AddAttackActions never re-adds 0x3F on a direct path.
- **Correction 2:** the `param_8 != 0` branch (the live path) adds action **0x0C** (`0x53F7F0` tail if `param_7==0`, `0x540CA0` front if `param_7!=0`). It clears the queue only when `param_6 != 0`. It then calls `CSWSCreature::BroadcastCombatStateToParty` 0x0057AC30, which starts combat.
- The 0x3F tail add lives in sibling `Add*Actions` functions (table C). Each one clears via `ActionManager(4)` 0x563C90 under a gate, then adds 0x3F if none is queued.

## A. ClearAllActions callers (49 functions, 58 sites)

Class: combat / AI / script / player-input / effect / party / client-UI / other. Temp: COLD = discrete event, WARM = per round or per queued node, HOT = per tick.

| # | caller | name (source) | sites | class | temp |
|---|---|---|---|---|---|
| 1 | 0x0051CB10 | CServerExoApp::ClearAllActions (steam-db B9) | 1 | other (app wrapper) | COLD |
| 2 | 0x005302E0 | CServerExoAppInternal::StorePlayerCharacters | 1 | other (save) | COLD |
| 3 | 0x00547BB0 | CSWSObject::AddDoorSaberAction | 2 | combat/AI (clear, then queue 0x47 or 0x3F) | COLD |
| 4 | 0x00563C90 | CSWSCreature::ActionManager(mode) | 2 | combat wrapper (player-object gate) | WARM |
| 5 | 0x00566410 | CSWSCreature::AddStanceActions | 1 | combat (stance) | COLD |
| 6 | 0x00568540 | CSWSCreature::AddAttackActions | 2 | combat attack (see table B) | COLD |
| 7 | 0x0057DA90 | CSWSCreature::AddUseTalentAtLocationActions | 2 | combat (use talent) | COLD |
| 8 | 0x0057DE70 | unnamed (only caller 0x65BE70; calls 0x563C90(8), 0x53F7F0(0x2A)) | 1 | combat/input | COLD |
| 9 | 0x00586750 | CSWSCreature::CancelCombat | 1 | combat (end) | COLD |
| 10 | 0x00586B70 | CSWSCreature::BumpFriends | 1 | combat (friend bump) | COLD |
| 11 | 0x005951A0 | CSWSEffectListHandler::OnApplyFactionModifier | 1 | effect | COLD |
| 12 | 0x005953E0 | CSWSEffectListHandler::OnRemoveFactionModifier | 1 | effect | COLD |
| 13 | 0x005959E0 | CSWSEffectListHandler::OnApplyDamage | 1 | combat effect (damage) | COLD (per hit) |
| 14 | 0x00597D90 | CSWSEffectListHandler::OnApplyDeath | 1 | effect | COLD |
| 15 | 0x00599050 | CSWSEffectListHandler::OnApplyResurrection | 1 | effect | COLD |
| 16 | 0x0059B520 | CSWSEffectListHandler::OnApplyEntangled | 1 | effect | COLD |
| 17 | 0x0059DE30 | CSWSEffectListHandler::OnApplySetStateInternal | 1 | effect (state) | COLD |
| 18 | 0x005A60B0 | CSWSEffectListHandler::OnApplyDisappearAppear | 1 | effect | COLD |
| 19 | 0x005A6280 | CSWSEffectListHandler::OnRemoveDisappearAppear | 1 | effect | COLD |
| 20 | 0x005A64C0 | CSWSEffectListHandler::OnApplyDisappear | 1 | effect | COLD |
| 21 | 0x005A6570 | CSWSEffectListHandler::OnApplyAppear | 1 | effect | COLD |
| 22 | 0x005D5550 | CSWSCreature::UnsummonMyself | 1 | other (summon) | COLD |
| 23 | 0x005D75B0 | CSWSObject::AIActionDialogObject (gog map, medium) | 2 | AI (RunActions dispatch) | COLD (per node) |
| 24 | 0x005DF920 | CSWSCreature::AIActionType47_ReequipResumeAttack | 2 | AI/combat: clear, re-equip, re-add attack | COLD (per 0x47 node) |
| 25 | 0x005F7F30 | CSWPartyTable::SaveMember | 1 | party | COLD |
| 26 | 0x005F8130 | CSWPartyTable::SaveMemberByObject | 1 | party | COLD |
| 27 | 0x005F8320 | CSWPartyTable::SavePuppet | 1 | party | COLD |
| 28 | 0x005F8520 | CSWPartyTable::SavePuppetByObject | 1 | party | COLD |
| 29 | 0x005FD480 | CSWPartyTable::SpawnNPC | 1 | party | COLD |
| 30 | 0x005FD810 | CSWPartyTable::SpawnPUP | 1 | party | COLD |
| 31 | 0x00656AF0 | unnamed (caller 0x65F8A0 HandlePlayerToServerMessage) | 1 | player input | COLD |
| 32 | 0x0065B270 | CSWSMessage::HandlePlayerToServerInputDriveControl | 1 | player input | COLD |
| 33 | 0x0065B490 | CSWSMessage::HandlePlayerToServerInputCancelGuiTimingEvent | 1 | player input | COLD |
| 34 | 0x0065BE70 | CSWSMessage::HandlePlayerToServerInputMessage (main player handler) | 1 | player input (combat) | COLD (per input) |
| 35 | 0x0065E670 | unnamed HandlePlayerToServerInput* (caller 0x65F8A0) | 2 | player input (combat) | COLD (per input) |
| 36 | 0x0066AD10 | Global::ExecuteCommandActionStartConversation | 2 | script | COLD |
| 37 | 0x0066DF70 | Global::ExecuteCommandBeginConversation | 2 | script | COLD |
| 38 | 0x0066F330 | Global::ExecuteCommandClearAllActions | 1 | script | COLD |
| 39 | 0x0068DB00 | Global::ExecuteCommandPlayAnimation | 1 | script | COLD |
| 40 | 0x00696E10 | Global::ExecuteCommandChangeByFaction | 1 | script | COLD |
| 41 | 0x006CF5D0 | DialogStartPacifyHostiles (engine-ai-atlas, medium) | 1 | dialog/combat (pacify) | COLD |
| 42 | 0x007445D0 | DoTargetAction (repo doc 31) | 1 | client UI | COLD |
| 43 | 0x007469F0 | unnamed client (no static caller) | 1 | client UI | COLD |
| 44 | 0x007511B0 | unnamed client (callers 0x752360, 0x8995A0) | 1 | client UI | COLD |
| 45 | 0x00751750 | CSWGuiMainInterface::HudInvokeGroup (repo doc 07) | 1 | client UI | COLD |
| 46 | 0x0077CF70 | unnamed client (dialog default-action callback, repo doc 18) | 1 | client UI | COLD |
| 47 | 0x0089FBD0 | CSWGuiPartySelection::AcceptParty | 1 | client UI | COLD |
| 48 | 0x008B0320 | CSWSCreature::EquipItem | 1 | equip | COLD |
| 49 | 0x008B05E0 | CSWSCreature::UnequipItem | 1 | equip | COLD |

Sites per function come from `callgraph.tsv` (callee 00541080). Sum = 58.

## B. AddAttackActions callers (9 functions, 12 sites)

Args decoded from the call text: `this, a2..a8, then stack extras`. `a6` = internal ClearAll (line 71). `a7` = 0 tail, 1 front. `a8` = branch selector (0 = 0x3F branch, never used).

| caller | name | call site | a6 clear | a7 | a8 | class | temp | note |
|---|---|---|---|---|---|---|---|---|
| 0x005DF920 | AIActionType47_ReequipResumeAttack | 0x005DFBC8 | 0 | tail | 1 | AI | COLD (per 0x47 node) | clears 2x before (see section D) |
| 0x0066DAF0 | ExecuteCommandAttack | 0x0066DB89 | 0 | tail | 1 | script | COLD | no clear |
| 0x008B0320 | EquipItem | 0x008B04FC | 0 | front | 1 | equip | COLD | own ClearAll 0x008B0441 first |
| 0x008B05E0 | UnequipItem | 0x008B0736 | 0 | front | 1 | equip | COLD | own ClearAll 0x008B065D first |
| 0x0065BE70 | HandlePlayerToServerInputMessage | 0x0065C0EA | 0 | tail | 1 | player input | COLD | attack for the commanded actor, no clear |
| 0x0065BE70 | HandlePlayerToServerInputMessage | 0x0065C255 | **1** | front | 1 | player input | COLD | party loop: a6 clear, front attack, then StartCombat (re-adds 0x0C, not 0x3F) |
| 0x0065E670 | unnamed HandlePlayerToServerInput* | 0x0065E993 | 0 | front | 1 | player input | COLD | own ClearAll 0x0065E8CE first |
| 0x0065E670 | unnamed HandlePlayerToServerInput* | 0x0065F326 | 0 | front | 1 | player input | COLD | own ClearAll 0x0065F269 first |
| 0x00670D10 | ExecuteCommandDoDoorAction (case 2) | 0x00670F7F | **1** | tail | 1 | script | COLD | clear, tail attack, StartCombat |
| 0x0057D8B0 | AddUseTalentOnObjectActions | 0x0057DA58 | 0 | tail | 1 | combat (use talent) | COLD | called from 0x66BDC0 (script) and 0x65BE70 |
| 0x006D9500 | AIActionCombat (0x3F handler) | 0x006D9931 | **1** | front | 1 (= round type 1 or 0x0B) | AI | WARM (per attack round node) | clear, front attack, StartCombat, every expanded attack round |
| 0x006D9500 | AIActionCombat (0x3F handler) | 0x006D996B | **1** | front | 1 | AI | WARM | same, the other branch (`local_28 != 0`) |

Sites with a6=1 (internal combat-start clear immediately before StartCombat): 0x0065C255, 0x00670F7F, 0x006D9931, 0x006D996B.

## C. Functions that add 0x3F (direct AddAction(0x3F) or the GetAction check)

Each row clears (ActionManager(4) to ClearAllActions(1)) under a gate, then adds 0x3F if none is queued.

| fn | name | gate before the 0x3F add | direct callers |
|---|---|---|---|
| 0x005683D0 | CSWSCreature::AddCutsceneAttackActions | `+0x520 == 0` (not in combat) then ActionManager(4) | 0x0066DBA0 ExecuteCommandCutsceneAttack (only, script) |
| 0x00568480 | CSWSCreature::AddCutsceneMoveActions | `+0x520 == 0` then ActionManager(4) | 0x0066DC80 ExecuteCommandCutsceneMove (script) |
| 0x00566410 | CSWSCreature::AddStanceActions (p2==0, p7==0) | `+0x520 == 0` then ActionManager(4) | 0x0065BE70, 0x006D9500 (case 0x0D) |
| 0x00565310 | CSWSCreature::AddCastSpellActions (param_19==0) | `0x590110()==0` (no round pending) then ActionManager(4) | 0x57DA90, 0x656AF0, 0x65BE70, 0x670D10, 0x66E7E0, 0x57D8B0, 0x6D9500 |
| 0x005648D0 | CSWSCreature::AddItemCastSpellActions | `0x590110()!=0` (round pending) then ActionManager(4) (opposite polarity) | 0x57B4C0 UseItem, 0x006D9500 |
| 0x00547BB0 | CSWSObject::AddDoorSaberAction | `+0x520 != 1 && 0x590110()==0` then ActionManager(4). Then ClearAll + queue **0x47** (ReequipResumeAttack) when `0x521060()==0`. Otherwise round node + 0x3F if absent | 0x65BE70, 0x006D9500 (case 0x0E) |
| 0x00568540 | CSWSCreature::AddAttackActions (a8==0 branch) | `+0x520 == 0` then ActionManager(4), then 0x3F if absent | **none** (all 12 sites pass a8!=0) |

Other 0x47 enqueuer: `Global::ExecuteCommandActionSwitchWeapons` 0x0069B980 (script, no clear, no caller in dump).

## D. Named candidate and chain

**Candidate: `CSWSCreature::AIActionType47_ReequipResumeAttack` 0x005DF920** (AddAttackActions call at 0x005DFBC8, args `a6=0, a7=0, a8=1`).

Chain (static):
1. Trigger: `CSWSObject::AddDoorSaberAction` 0x547BB0 runs `ActionManager(4)` when not in combat and no round is pending (0x547BB0 decomp lines 21-22). It then calls `ClearAllActions(1)` (0x547CB4 / 0x547D03) and queues **action 0x47** (decomp lines 26-27: `FUN_00541080(1)` then `FUN_0053f7f0(0x47,...)`).
2. `CSWSObject::RunActions` 0x005D56B0 dispatches node 0x47 to 0x005DF920.
3. 0x005DF920 decomp line 15: `ClearAllActions(1)`, unconditional, at entry. A second `ClearAllActions(1)` follows, then `CSWSCombatRound::RemoveAllActions` 0x590430, which empties the round list. ClearAll itself does not touch the round list (doc 29 4.4), so this wipes queued round nodes (player cast/use) too.
4. Re-equip (0x5676D0 / 0x566650 on items `+0x14`).
5. If `+0x520 != 0` and `+0x11E8 == 1` and target `+0xB20 != 0x7F000000`, call AddAttackActions(target, tail, a8=1) at 0x005DFBC8. This re-adds **action 0x0C** and calls `StartCombat` 0x57AC30.

So the attack is re-added after the clear, but as 0x0C, not 0x3F. The 0x3F re-add is in table C.

Alternates, ranked below the candidate:
- `CSWSMessage::HandlePlayerToServerInputMessage` 0x0065BE70 @ 0x0065C255 (party loop): clear + front attack + StartCombat in one call. Re-adds 0x0C.
- `AIActionCombat` 0x006D9500 @ 0x006D9931/0x006D996B: clears the whole queue every expanded attack round (WARM). Re-adds 0x0C, front.
- `AddCutsceneAttackActions` 0x005683D0: the only attack-named function that re-adds **0x3F** right after a `+0x520==0` clear. Reached only from `ExecuteCommandCutsceneAttack` 0x0066DBA0 (script, dispatch unresolved).

## E. Indirect and unresolved gaps

- **Script command table (no static caller):** `ExecuteCommand*` handlers have empty `callers=` in the headers, so they are reached through a function-pointer table. Affected here: 0x66DAF0 Attack, 0x66DBA0 CutsceneAttack, 0x66DC80 CutsceneMove, 0x670D10 DoDoorAction, 0x66DF70 BeginConversation, 0x66F330 ClearAllActions, 0x66AD10, 0x68DB00, 0x696E10, 0x66BDC0 UseTalent, 0x69B980 SwitchWeapons, 0x699E80 SetFakeCombatState.
- **Effect handlers:** `OnApply*` / `OnRemove*` (0x5959E0 ... 0x5A6570, 0x5951A0, 0x5953E0) have no static caller. They are dispatched through the effect-type table.
- **Virtual / vtbl calls inside callers (INDIRECT rows in callgraph):** 0x65BE70 has 16, 0x65E670 has 11, 0x6D9500 has 7, 0x8B0320 has 3, 0x670D10 has 2. Also 0x5DF920, 0x565310, 0x5648D0, 0x547BB0 have 1 each. A vtbl target could be ClearAll or AddAttack, but none resolved to 0x541080 or 0x568540.
- **Unnamed HandlePlayerToServerInput\* (0x656AF0, 0x65E670):** names not in the DB. Classified from behavior only.

## F. Verdict

PARTIAL. The premise that AddAttackActions re-adds 0x3F is not supported on any direct path. The 0x3F re-add is real in sibling functions (table C). The attack re-add after a clear is real via 0x5DF920 (action 0x0C).

Open question: does the 0x47 node from `AddDoorSaberAction` 0x547BB0 (gate `0x521060`, unnamed) get queued in live combat start, or only from weapon-switch scripts (0x69B980)?
