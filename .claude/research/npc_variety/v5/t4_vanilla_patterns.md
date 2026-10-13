# T4: vanilla K2 fake-combat, move, no-damage patterns

Sources: `.claude/scratch_npcvar/nss_all/` (windows only). Only `a_move_party.nss` defines `main()` (:13): the sole entry script (ExecuteScript-able). The two `k_inc_*` files are include-only.

E1 (a: fake shoot/cast) `.claude/scratch_npcvar/nss_all/k_inc_fakecombat.nss:42-78` DoFakeAttack + DoFakeTalent (live code):
```
42 void DoFakeAttack(object oTarget,int bLethal)
44   if(bLethal) { SetMinOneHP(oTarget,FALSE);
47     ApplyEffectToObject(DURATION_TYPE_INSTANT,EffectDamage(GetCurrentHitPoints(oTarget)-1),
53   ApplyEffectToObject(DURATION_TYPE_TEMPORARY,EffectAssuredHit(),OBJECT_SELF,3.0);
54   ActionAttack(oTarget);
77   ApplyEffectToObject(DURATION_TYPE_TEMPORARY,EffectAssuredHit(),OBJECT_SELF,3.0);
78   ActionUseTalentOnObject(t,oTarget);
```
- Reads: oTarget, bLethal, `talent t`, OBJECT_SELF (attacker, via :65/:88). No locals, no faction ids.
- :49 CutsceneAttack is comment-only. The `//else //{ //}` guard (:51-55) is commented out, so :53-54 run for lethal AND non-lethal. Live attack = assured hit + real ActionAttack.
- Reuse: NOT tag-only (include-only, needs new .ncs).

E2 (b: move) `a_move_party.nss:40-48`:
```
40 DelayCommand(IntToFloat(iDelay + (iInterval * i)), AssignCommand(oPC, ClearAllActions()));
...
43 if (iRun == 2) DelayCommand(..., AssignCommand(oPC, ActionJumpToLocation(GetLocation(oWP))));
47 else DelayCommand(..., AssignCommand(oPC, ActionForceMoveToLocation(GetLocation(oWP), iRun)));
```
- Reads: tags `sWaypoint + IntToString(i)` (default `wp_pc` gives `wp_pc1..3`; :24, :35); party slots `GetPartyMemberByIndex(i-1)` (:31); iRun 0 walk / 1 run / 2 jump. Side effect: `SetPartyLeader(NPC_PLAYER)` (:26).
- Reuse: tag-only ONLY for party slots 0-2 (ExecuteScript + params + waypoints by tag). Spawned non-party NPC: needs new .ncs.

E3 (b: move) `k_inc_utility.nss:1179-1184` UT_PlotMoveLocation:
```
1179 void UT_PlotMoveLocation(location lTarget,int nRun = FALSE)
1180 {
1181   ActionForceMoveToLocation(lTarget,nRun);
1182   ActionDoCommand(SetCommandable(TRUE));
1183   SetCommandable(FALSE);
```
- Reads: lTarget, nRun (default FALSE), OBJECT_SELF (mover). SetCommandable(FALSE) applies at call time; TRUE is queued after the move, so the NPC is uninterruptible while walking. Reuse: NOT tag-only (include-only).

E4 (c: no damage) `k_inc_fakecombat.nss:21-40` FAI_EnableFakeMode / FAI_DisableFakeMode:
```
21 SetCommandable(TRUE,oTarget);
22 AssignCommand(oTarget,ClearAllActions());
23 SetLocalBoolean(oTarget,SW_FLAG_AI_OFF,TRUE);
25 ChangeToStandardFaction(oTarget,iFaction);
26 SetMinOneHP(oTarget,TRUE);
```
- Reads: local bool `SW_FLAG_AI_OFF` (defined outside these 3 files, unchecked); `iFaction` (param; no hardcoded id in the 3 files). Damage floor = :26. Disable (:36-39) restores only what the caller passes (no saved faction).
- GAP: FAI_PerformFakeAttack (:58-66) never calls EnableFakeMode. Non-lethal fake attack with MinOneHP FALSE can kill via ActionAttack. Caller must enable first.

E5 (c: PlotFlag) `k_inc_utility.nss:2111-2118` UT_LockDoorDuplicates:
```
2111 object oDoor = GetNearestObjectByTag(sTag);
2112 while(GetIsObjectValid(oDoor) && GetObjectType(oDoor) == OBJECT_TYPE_DOOR)
2116   AssignCommand(oDoor, ActionCloseDoor(oDoor));
2117   AssignCommand(oDoor, ActionLockObject(oDoor));
2118   AssignCommand(oDoor, SetPlotFlag(oDoor, TRUE));
```
- Door-only; PlotFlag on creatures is not shown in these files. Reuse: NOT tag-only (include-only).

Verdict: tag-only reuse only for the party move (E2, party slots 0-2); all else needs a new .ncs. Open: whether an existing entry script already calls FAI_* or UT_PlotMoveLocation (not searched; 3-file budget).
