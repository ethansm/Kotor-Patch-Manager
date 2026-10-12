# P5 behaviour + dialog recon (read-only, 2026-10-11)

Builds on R2_script_route.md and R4_utc_dialog.md. MW = ~/Games/KOTOR2_Mod_Workspace; vanilla scripts = MW/mod8_kite_script/vanilla_all; vanilla copy = MW/backups/vanilla_clean (pykotor read-only).

## 1. Behaviour recipes
Route
- Route exists only if WP_<tag>_02 exists (k_inc_walkways L183-186). Use at least 2 waypoints.
- Length = contiguous WP_<tag>_01.._NN, counted by GN_SetUpWayPoints (L550-594). Runs only inside GN_SetListeningPatterns (k_inc_generic L2172; walk code warns at L189). Do not use a_walkways.nss as spawn script.
- Tag = WP_ + GetTag(OBJECT_SELF). GetWaypointByTag is area-wide: use a unique tag.
- No WP_<tag>_02 and no POST_<tag>: no movement (guard L241).

(a) Stand still, facing a direction
- ScriptSpawn blank. Facing: SetFacing(float) nwscript L2093 (0=E, 90=N); SetFacingPoint L2787; GetFacing L2200. Precedent k_inc_utility L2279 (AssignCommand SetFacing(GetFacing(oWP0))).
- Simplest: facing in CreateObject location, Location(vec, fFacing) (k_sup_baodur L10).

(b) Walk route
- ScriptSpawn = k_def_spawn01 (L104 listening patterns, L113 GN_WalkWayPoints). No flag required.
- Default = ping-pong: reverses at last point and at index 1 (GN_GetWayPointDirection L512-536).
- Flags (SetLocalBoolean on self; active K2 values k_inc_walkways L34-55):
  - 99 WALK_ONCE: stops at last point (sets 106 DEACTIVATE, L226-229).
  - 100 WALK_CIRCULAR: last point jumps to 1 (L230-234).
  - 98 START_AT_NEAREST, 103 WALK_RANDOM, 104 WALK_RUN, 109 USE_WAYPOINT_ANIMATION (anim id in local 29).
  - Pause: 102 short (3 s), 107 long, 108 random. Faces waypoint only while pausing (L277-280).

(c) Idle animations (stationary)
- Flag 29 AMBIENT_ANIMATIONS, not 65. Path: k_def_heartbt01 -> k_ai_master event 1001 (L52-72) -> GN_PlayAmbientAnimation (k_inc_generic L3136).
- Heartbeat plays idle only if no WP_<tag>_01 exists (k_ai_master L59-61). Route and idle are exclusive. Vanilla spawn k_def_ambient sets flag 29 (L27) and calls GN_WalkWayPoints (L58).
- Flag 29 branch (k_inc_generic L3154-3228): friend within 5 m -> TALK_NORMAL (5); head turns 100/101; BORED 103; SCRATCH 102; LOOPING_PAUSE2 (1); PAUSE3 (24). No random walk.
- Flag 65 branch (L3230-3256) adds TAUNT 107, VICTORY1 108, ActionRandomWalk. Do not set 65 for stand-still NPCs.
- Sit: ANIMATION_LOOPING_SIT_CHAIR=36, SIT_AND_MEDITATE=35 (nwscript L1312-1318). No vanilla script uses them.

(d) Stay out of combat
- FactionID 5 = STANDARD_FACTION_NEUTRAL (nwscript L1024). Telos commoners and Wookiees use 5 (R4).
- GN_CheckShouldFlee (k_inc_generic L3337-3361) counts only HOSTILE_1/HOSTILE_2/INSANE neutrals as threats.
- Flag 87 AI_OFF (k_inc_generic L119) gates attacked reply (k_ai_master L271), perception, heartbeat (L55). It also kills idle anims: never combine with 29.
- Flag 85 COMMONER_BEHAVIOR (L117): flee when hostile near (L3152, L3258). Optional.
- UTC Min1HP, NoPermDeath, Plot (utc.py L97-101) are documented only; vanilla commoners all False.

UTC script slots (Override-only template)
- ScriptSpawn: k_def_spawn01 (walker) | k_def_ambient (idle) | blank (stand).
- ScriptHeartbeat: k_def_heartbt01 (idle only).
- Conversation: DLG resref, or blank (vanilla n_telm02s is blank). Keep OnDialog k_def_dialogue01, OnEndDialog k_def_endconv.
- Use a unique resref: module .rim/.mod/_s.rim embed their own UTCs (npc_div_registry L59-80, L161). Same resref differs per module: 203tel n_telf02s has ScriptSpawn k_def_ambient; chitin copy has blank OnSpawn.

Waypoint (UTW) fields, pykotor utw.py L17-55
- Needed: Tag, TemplateResRef. Optional: LocalizedName.
- Not used by engine: LinkedTo, Description. Toolset only: Appearance, PaletteID, Comment.
- Position and facing come from the CreateObject location. nwscript L3298 says sTemplate is "the tag of the waypoint", so a UTW may be unnecessary (unverified).

## 2. Dialog
Vanilla Telos commoner DLGs (chitin Dialogs.bif, 30 DLGs in chitin)
- 200comm (M): 3 starters/3 entries, NumWords 56. 200comf (F): 3/3, NumWords 69. 200tsfm (TSF M): 2/2, NumWords 47.
- Pattern: starter active1 c_localn_eq (param 30); entry script1 a_localn_set (param 30); entry VO_ResRef set; Speaker empty; no replies; no camera fields.
- DLG: ConversationType 0 (Human), Skippable True, CameraModel and EndConversation empty.
- Smallest in chitin: 000ffield (1/1/0, NumWords 18), starter gated by c_npc_inprty. Not a commoner.
- WAVs present for 200comm (202/200COMM001.wav, GBL200COMM002/003) and 200tsfm (GBL200TSFM002). None found for 200ithr or 000ffield.

Authoring one-node DLG (pykotor .venv)
- Classes: pykotor.resource.generics.dlg exports DLG, DLGEntry, DLGLink. Writer: bytes_dlg(dlg, game=Game.K2) (io/gff.py L620) or write_dlg(dlg, target, game) (L592).
- Recipe: DLG(); DLGEntry with text (LocalizedString stringref) and vo_resref (ResRef); DLGLink(entry) appended to dlg.starters; no starter condition. Verified in memory (no disk write): bytes_dlg -> read_dlg round-trip OK.
- Silent line: VO not required. 14 vanilla chitin DLGs hold subtitle-only entries (VO_ResRef blank, valid strref), e.g. visasmarr 5, mira 3. Playback inferred.

## 3. Bark alternative (no DLG)
- void SpeakString(string, int nTalkVolume=TALKVOLUME_TALK) L3176 (action 221, immediate); void ActionSpeakString(string, int) L2276 (action 39); void ActionSpeakStringByStrRef(int nStrRef, int) L3279 (action 240). No non-action StrRef form.
- TALKVOLUME: TALK 0, WHISPER 1, SHOUT 2, SILENT_TALK 3, SILENT_SHOUT 4 (L35-39).
- Vanilla SpeakString calls are GEN_* listen tokens at SILENT volumes (k_ai_master L269). No vanilla visible bark or StrRef speech.
- Strref: scripts/append_ranged_kite_tlk.py appends entries at end of dialog.tlk (strrefs N, N+1); dry run default; --apply writes; game_running() guard (L117). Same TLK backs DLG text, so an appended strref should work with ActionSpeakStringByStrRef (inferred, untested).
- Bark needs a custom compiled script (R2 tooling) and a local counter (heartbeat about 6 s).
## PoC shape
1. Stand: CreateObject creature, blank ScriptSpawn, facing in Location.
2. Walk: ScriptSpawn k_def_spawn01, WP_<uniqueTag>_01.._03, default ping-pong, FactionID 5.
3. Idle: ScriptSpawn k_def_ambient + ScriptHeartbeat k_def_heartbt01, flag 29 only, no route.
4. Dialog (optional): one DLG, no VO, one TLK strref appended.

## OPEN
1. Do CreateObject creatures run ScriptSpawn and heartbeat? No vanilla route precedent.
2. Does GetWaypointByTag find CreateObject waypoints; is a UTW needed?
3. Override vs module-embedded UTC/DLG precedence (R2 OPEN); unique resrefs sidestep it.
4. Sit/chair anims on N_WookieM: unused in vanilla, untested.
5. Does attacking a neutral (faction 5) NPC make it hostile? English-substring DLG text?
