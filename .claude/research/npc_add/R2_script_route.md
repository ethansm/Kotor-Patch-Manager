# R2 script route: script-spawned Wookiee on a waypoint route (KOTOR II)

Read-only recon. MW = ~/Games/KOTOR2_Mod_Workspace. Paths relative to the worktree unless absolute.

## 1. nwscript (K2/TSL)
- Canonical: MW/mod8_kite_script/vanilla/nwscript.nss, md5 b487ad80c934ed66cf37c46d2796273d, 6321 lines.
- Identical copies: MW/mod8_kite_script/vanilla_all/, MW/script_recompile_20260924/tools/vanilla/, MW/01_extracted/mod127_repair_hk47/tslpatchdata/ (TSL mod).
- K2 marker: GetPartyMemberByIndex at line 4805. No nwscript* file under .venv (pykotor ships none).
- Signatures, action number in brackets, line number after:
  - CreateObject(int nObjectType, string sTemplate, location lLocation, int bUseAppearAnimation=FALSE) -> object [243; 3301]. Waypoints are creatable (comment 3296). OBJECT_TYPE_CREATURE=1 (79), OBJECT_TYPE_WAYPOINT=32 (84).
  - ActionRandomWalk() [20; 2146]
  - ActionMoveToLocation(location lDestination, int bRun=FALSE) [21; 2154]
  - ActionForceMoveToLocation(location lDestination, int bRun=FALSE, float fTimeout=30.0f) [382; 3930]
  - ActionMoveToObject(object oMoveTo, int bRun=FALSE, float fRange=1.0f) [22; 2163]
  - ActionForceMoveToObject(object oMoveTo, int bRun=FALSE, float fRange=1.0f, float fTimeout=30.0f) [number not checked; 3933]
  - ActionWait(float fSeconds) [202; 3068]
  - ActionDoCommand(action aActionToDo) [294; 3530]
  - ActionStartConversation(object oObjectToConverse, string sDialogResRef="", int bPrivateConversation=FALSE, int nConversationType=CONVERSATION_TYPE_CINEMATIC, int bIgnoreStartRange=FALSE, ...) [204; 3100]
  - ActionJumpToLocation(location lLocation) [214; 3152]
  - SetFacing(float fDirection) [10; 2093]
  - GetWaypointByTag(string sWaypointTag) -> object [197; 3025]
  - DestroyObject(object oDestroy, float fDelay=0.0f, int bNoFade=FALSE, float fDelayUntilFade=0.0f, int nHideFeedback=0) [241; 3284]
  - SetPlotFlag(object oTarget, int nPlotFlag) [456; 4233]
  - SetCommandable(int bCommandable, object oTarget=OBJECT_SELF) [162; 2870]
  - GetLocalNumber(object oObject, int nIndex) -> int [681; 5158]. SetLocalNumber(object oObject, int nIndex, int nValue) [682; 5164]. Index range 12-28 per comment (5157-5163).
  - AssignCommand(object oActionSubject, action aActionToAssign) [6; 2072]
  - DelayCommand(float fSeconds, action aActionToDelay) [7; 2077]
- No SetTag or SetName in this nwscript. A script cannot rename a waypoint or creature, so the tag must come from the Override UTW/UTC template.

## 2. Default creature scripts (MW/mod8_kite_script/vanilla_all/ = vanilla K2 source)
- Heartbeat router: k_def_heartbt01.nss calls ExecuteScript("k_ai_master"). The heartbeat is about 6 s (review_ai_scripts_events.md E7).
- Walk logic lives in k_inc_walkways.nss, GN_WalkWayPoints() at line 101.
- Callers: OnSpawn k_def_spawn01.nss:113 ("TURN ON WALKPOINTS", unconditional). Also k_def_ambient:58, k_def_ambmob:58, k_def_spn_t_more:117, k_def_spn_t_none:113.
- Route selection is tag-based:
  - Tests WP_<tag>_02 (lines 183-186). If found, the move target is WP_<tag>_NN (NN = current index 01..end).
  - If not found, sWays="UNKNOWN" and the target is POST_<tag> (lines 203-205).
- Movement: ActionForceMoveToObject(oWay, run, personalSpace, fTimeOut) at about line 262. fTimeOut = max(30, dist/1.25).
- Self-loop: ActionDoCommand(GN_WalkWayPoints()) at about line 291. The creature re-queues itself after each hop.
- Direction is ping-pong, not a loop: GN_GetWayPointDirection (512-534) reverses at the end point and at index 1.
- Flag helpers: GN_SetSpawnInCondition = SetLocalBoolean (313-315). Active K2 values are in the block at lines 34-42: MOBILE 65, WALK_RUN 104, DEACTIVATE 106, PAUSE_SHORT 102, PAUSE_LONG 107. Lines 20-33 are a comment block with older values.
- Minimum for a walking NPC: OnSpawn = k_def_spawn01, plus WP_<tag>_02 and up. No flags are required.
- Most common OnSpawn in the corpus is k_def_spawn01 (R3_enemy_ai.md:50).

## 3. Hook points for our script
- Creature UTC scripts: ScriptSpawn, ScriptHeartbeat, ScriptEndRound, ScriptOnNotice (pykotor utc.py:477-497).
  - GIT creature entries hold only TemplateResRef and position (04_merge_modules.py:52; npc_div_registry.py:159). The GIT has no Script* fields for creatures.
  - Feasible without module archives: a new Override UTC, or an edit to an existing one. An edit to an existing template changes every instance of it game-wide.
- Placeable UTP OnUsed (utp.py:164): fires on interaction only. New placements need GIT (module archive), so not Override-only.
- Area ARE OnEnter, OnExit, OnHeartbeat (are.py:390-393). Module IFO Mod_OnModLoad, Mod_OnModStart, Mod_OnClientEntr (ifo.py:116-119).
  - ARE and IFO live in the module capsule. 04_merge_modules.py:313 writes ARE OnEnter into the module. Save writes ARE/IFO/GIT (10_engine_optimization_research.md:325).
  - Not feasible without touching module archives.
- Global k_sup_* scripts: not auto-run. They are invoked by name from other scripts (e.g. k_sup_galaxymap). No k_ot_* or startup script found in vanilla_all.
- Engine fires k_hen_enter5m / k_hen_exit5m by name (review_ai_scripts_events.md:303). Trigger source not traced.
- KPM DLL route (feasible, engine access already used):
  - KPatchInit(api) runs per module (portrait_slots_src/PortraitSlotsProvider.cpp:134; 19d_phase1_plan.md:117).
  - Engine globals and functions by absolute address (companion_ai_src/CompanionAI.cpp:63-72): G_VM 0x00A1B4A8, FnGetGameObject 0x0051C0A0, FnClearAllActions 0x00541080.
  - RunScript hook 0x6FD8D0 (CompanionAI.cpp:35, 1239; 63 callers, AI7.md:16).
  - ExecuteScript handler 0x679050; ActionMoveToLocation handler 0x68CEE0 (AI7.md:86, 68).
  - Command table built by InitializeCommands 0x665F50. Index = prototype order (AI7 line 40).
  - CreateObject engine address: not in repo.
- Override-only option A (most practical):
  1. Override the UTC of a creature that is already placed in the target Telos area, with ScriptSpawn or ScriptHeartbeat pointing at a new k_* script.
  2. That script calls CreateObject(OBJECT_TYPE_CREATURE, "<new utc>", loc), and CreateObject(OBJECT_TYPE_WAYPOINT, "<utw>", loc) for each WP_ point.
  3. Caveat: the template change is game-wide. Use a template that only appears in the target module.

## 4. Tooling (NSS to NCS)
- Working command: MW/script_recompile_20260924/tools/compile.sh "<ABSOLUTE out.ncs>" <script.nss> [include.nss ...] (compile.sh:1-3, 10-12).
  - Inside it runs `wine nwnnsscomp.exe -c script.nss -o script.ncs` under Proton, with the vanilla includes copied in.
  - Env: KOTOR2_PROTON_WINE, KOTOR2_NWNNSSCOMP (default: mod127 tslpatchdata/nwnnsscomp.exe).
  - nwnnsscomp exits 0 on errors, so check that the output file exists (build_party_ai_scripts.py:118).
- Raw invocation: scripts/03_compile_galaxy_map.py:62.
- Do not use pykotor compile_nss: it swaps engine-call arguments (build_party_ai_scripts.py:17).
- Verify arg order with `python3 scripts/check_ncs_arg_order.py --override-only` (HANDOFF_2026-09-30.md:62).

## OPEN
- Whether Override .are/.ifo are read in place of the module capsule. Not settled by the repo.
- Engine address of the CreateObject handler, needed for the DLL route (query_ghidra or rea-recon).
- Which existing Wookiee UTC uses ScriptSpawn=k_def_spawn01, and whether its template is unique to Telos. Not checked.
- Telos module name and its area UTC/GIT contents. Not checked.
