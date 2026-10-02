# Wave 2 briefs (AI5-AI8, supporting subsystems), 2026-10-02

## Common (every batch)
- Everything in the "Common" section of `WAVE1_BRIEFS.md` applies (contracts, tools, output rules, known name_map errors).
- ALSO read `REPO/patch_manager_mods/engine_ai_atlas/GATE_wave1.md` and the wave-1 findings that touch your subsystem
  (`ROOT/findings/AI1.md`..`AI4.md`). Gate-confirmed facts there are settled: cite them, do not re-derive.
  In particular use the wave-1 action-type table (AI4) and event-id facts (AI1/AI2/AI3) rather than rebuilding them.
- Output `ROOT/findings/AI<n>.md` (AI5..AI8), sections (a)-(g), <= ~600 lines, 5-line summary on top.
- Script sources (read-only): `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/` (our k_ai_master /
  k_inc_generic / k_inc_gensupport) and `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla/nwscript.nss` (command
  order = command index). R2/R3 reports in `REPO/patch_manager_mods/mods678_research/ai_research_2026-10-02/`.

## AI5 Follow, party and formation
Serves E4, E7, FM1, FM4, FM6, the party flag.
Seeds: `AIActionFollowLeader 0x5BF7F0`, `AIActionFollowOwner 0x5BF940`, `CheckMoveToFollowRadius 0x5BCDC0`,
`QuickWalk_FollowLeader 0x5C6340`, `0x5C7AD0`, `0x5CA000`, `0x5CAE00`, `0x5CAF30`, `0x5CB060`, `SetFormation 0x5D4B60`,
`CClientParty::RangeMonitor 0x7E6CE0` (the 5 m edge `0x7E77DA`-`0x7E78AE`; FM6 at `0x7E6546` slot+0x70),
`SetPartyMemberFlag 0x58A330`, `SaveCreature 0x574500` (FM4), `CFollowInfo ctor 0x5752C0`, the CSWPartyTable /
CSWParty / CSWPartyFollowDataArray rows in `ROOT/findings/AI0_map_*.tsv`, `0x5FE4B0` (mapper:
CSWPartyTable::SwitchPlayerCharacter, DB: CSWSCreature::MakePlayerCharacter: decide), and the mapper names `0x669020`
(actually CSWVirtualMachineCommands::RunScriptCallback per AI0) / `0x66B770` / `0x66B8A0`.
Questions:
1. The FollowInfo struct (field table).
2. **Every writer and reader of creature `+0x11AC`** (use `field_census.py --offset 0x11AC --all`, then filter by
   provenance) and exactly when it is 0 for a companion. Runtime fact: in a 02:38 launch on the Ebon Hawk, atton, t3m4,
   kreia and a droid showed `+0x11AC`=0 while the PC showed 1. Give its exact semantics vs "in the active party" (the
   2-3 selected followers), and the CORRECT party test a DLL should use (e.g. a CSWPartyTable lookup by object id, or a
   different flag), with the addresses a DLL would call/read.
3. The leader-switch path (UI and script): who changes the leader, what is reset (follow trail, FollowInfo, AI level).
4. The formation slots: how a follower's target position is computed.
5. FM6: who consumes the per-slot flag written at `0x7E6546` (slot+0x70)?

## AI6 Faction, reputation and hostility
Serves E9/L7 (shout 15), enemy AI, C1.
Seeds: `GetCreatureReputation 0x5D0D00`, `CSWSObject::GetReputation 0x5E13C0`, the CSWSFaction / CFactionManager
rows from AI0 (`AI0_map_CSWSFaction.tsv`, `AI0_map_CFactionManager.tsv`; 43 new vote rows, confirm the ones you use),
the GetIsEnemy / GetIsFriend / GetIsNeutral command handlers (find via `U1_map_vm.tsv` / the command table at
`InitCommands 0x665F50`), `ClearHostileActionsVersus 0x5867E0`, surrender (`ChangeToStandardFaction`,
`SurrenderToEnemies`), and `repute.2da` loading (`strings.tsv` "repute").
Questions:
1. The faction table layout (CFactionManager, CSWSFaction fields; how the 2DA matrix is stored).
2. Personal reputation overrides: where stored on the creature, precedence vs faction.
3. **The shout dispatch path**: `SpeakString` / `SilentShout` (talk volume 'silent shout' etc.) -> OnDialogue (slot 7)
   delivery to listeners: radius per volume, faction filter, the listener loop, the event id. This is the engine half of
   E9 (k_ai_master answering allied shout 15). Give what a listener receives (`GetListenPatternNumber`, speaker).
4. How a hostile/friendly flip propagates to perception (re-evaluation? event?) and to combat (ClearHostileActionsVersus,
   combat state), and its cost.

## AI7 Script <-> engine interface
Serves every script fix, L2, the medpac rule, the ai_trace.
Seeds: `CSWSCreature::EventHandler 0x56EFD0` (the full event id -> script slot table), `RunScript 0x6FD8D0`,
`ExecuteCommandExecuteScript 0x679050`, `InitCommands 0x665F50` (the command table), `U1_map_vm.tsv` and
`AI0_map_CSWVirtualMachineCommands.tsv`, and the handlers of every nwscript command our AI scripts call. Get the list by
grepping `src_party_ai/*.nss` for command names that appear in `vanilla/nwscript.nss`; at minimum: GetNearestCreature,
GetCreatureHasTalent (`0x6C0330`), GetCreatureTalentBest / GetCreatureTalentRandom, ActionUseTalentOnObject,
ActionAttack (`0x568915`), GetLastHostileTarget, GetIsInCombat, GetHasSpellEffect, GetCurrentAction, ClearAllActions,
ActionMoveToLocation, GetItemPossessedBy, GetNumStackedItems.
Questions:
1. Map each command handler: command index, address, args, engine call, side effects, and its cost (e.g. a
   GetNearestCreature area scan: O(n) per call?). One table.
2. The full event id -> script slot table from 0x56EFD0 (all slots 0-13), with the conditions that skip each.
3. `GetCurrentAction` -> action-type mapping (join with the AI4 action-type table).
4. Talent selection internals (TalentBest / TalentRandom: which talents, CR/level filters, item talents).

## AI8 Effects and AoE as they gate AI
Serves L2 buffs, enemy AI, stun/fear/confusion behaviour, EFFOVF.
Seeds: the CSWSEffectListHandler / CGameEffect / CSWSAreaOfEffectObject rows in AI0 (79 + 20 + 17 new vote rows:
confirm what you use), the 118-entry effect apply/remove dispatch tables (Buff HUD research: knowledge DB lessons
scope "Buff Duration HUD" and the classes rows `CSWSEffectListHandler` vft `0x992490`, `CGameEffectApplierRemover`
`0x9924A4`; `CGameEffect` +0x10/+0x14 expiry), the AI-gating checks (stun, paralysis, fear, confusion, sleep, horror,
droid stun, force push knockdown) inside `CSWSCreature::AIUpdate 0x569320` / `RunActions 0x5D56B0`.
Questions:
1. Which effects block or replace AI actions, and where (address of each gate, the creature field it reads).
2. `GetHasSpellEffect` semantics for Battle Meditation / Force Valor (the R2 L2 risk): does it match by spell id of the
   effect creator, and does a party-wide aura show on every member?
3. The AoE object AI triggers: enter / exit / heartbeat script slots and the cadence.
4. EFFOVF: effect stacking (no dedup) and the LoadGameEffect IntList bound: confirm, give reach.
