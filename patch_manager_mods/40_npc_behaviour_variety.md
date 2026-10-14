# 40 NPC behaviour variety: extra walkers, scripted encounters, animation

## Status
- **State: V0, V1, V3, V4, V5, V6 and V7 done (2026-10-13); V7 = master plan in section 11 (build order NB1-NB5, doc 35 merged); V5 = scripted-encounter feasibility in section 10 (data-first route: Plot/Min1HP UTC templates plus faction pair 11/12 plus engine AI; fallback user-installed override `.ncs`; probes P11-P15; host-only, nothing installed); V6 = animation feasibility in section 9 (native `PlayAnimation` injector recommended, two generated tables, no DLL); V3 = pool and roster spec in section 8 (host-only, tables + 2 tested scripts, no DLL); V4 = visitSalt in doc 34 hash, host tests + build only, salt unwired, option `visit_variety` in npc-diversity.ini (default 0 = same look), nothing installed.** Next: V2b (live; needs user GO and a pre-visit save; the V2a probe is built as an `npc-world` client, section 13).
- Origin (user, 10-12/13): doc 34 only changes how generic commoners look. Goal: a section should not feel the same every pass. (a) randomize NPC animations/actions; (b) randomly add walking aliens, TSF agents, Czerka NPCs among plain commoners; (c) later, small random scripted events (example: TSF agents chasing and shooting at an outlaw across the station; runner and chasers invincible, hasted, combat disabled), possibly with K1 NPC audio/dialog.
- Evidence: Haiku wave `w9-npcvar` (12 recon units, all PARTIAL), raw findings copied to `.claude/research/npc_variety/u01..u12.md`, animation inventories `u03_anims.txt` and `u03b_target_rows.txt`. Web prior-art searches (10-13) summarized in section 5. Memory: `kotor2-npc-behaviour-variety-idea`.
- Related docs: 34 (appearance dispatcher), 35 (`35_npc_add_poc.md`, spawn routes U1-U11), 38 (K1 import, native spawn probe T25, licence policy section 8), `.claude/research/npc_add/` (R1-R6, poc/).

## 1. What the research settled (facts, with source)
1. **Spawn route for per-visit variety = the patch-only native hook.** GIT edit is fixed per install (u04). Script `CreateObject`: spawn-script firing unresolved (u04, u07). Hook H2 `0x0055A460` is the only documented route that fires on every load (u04). Native spawn reaches `AddToArea 0x568BD0` (verified, u05). Override scripts cannot be shipped through KPM (u11, R5:42).
2. **Animation set is fixed by the model.** Every target body (N_CommM/F, Czerka officer 18, TSF 479/485, Rodian 39, Bith 12, Duros 24, Gand 569, Sullustan 601, Twi'lek F 47) resolves to the same humanoid chain: 7 idles (pause1, pause2, pause3, standstill, neutral, good, evil) and 14 flourishes (all one-shot, stationary). `cpause1`, `cpause2`, `powerup` exist nowhere. Droids differ: P_HK47 has 0 flourishes, P_T3M4/P_G0T0 4 (u03, u03b). Flourish length varies by chain (read it from the model).
3. **Vanilla behaviour model.** A creature either idles or walks: the heartbeat idles only when no `WP_<tag>_01` exists. Ambient flag 29 plays fixed IDs (LOOPING_PAUSE2=1, TALK_NORMAL=5, SCRATCH=102, BORED=103, head turns 100/101, PAUSE3=24). Flag 109 USE_WAYPOINT_ANIMATION reads the animation id from local int 29, the only per-waypoint animation hook found (u01). `k_def_spawn01` walks `WP_<tag>_NN` ping-pong; a 2-point route reverses every hop, so use 3+ points (u02). Flags: MOBILE 65, WALK_RUN 104, DEACTIVATE 106, PAUSE_SHORT 102, PAUSE_LONG 107; their meaning is inferred from names only (u02).
4. **Pool.** Neutral (faction 5), conversation-blank chitin templates: aliens n_rodian, n_gand, n_bith, n_duros, n_sullustan01, n_gran, n_quarren01, n_weequay01, n_devaronian01, n_twilekf/m, n_wookiem/f, n_tuskenf01, c_ithorian, c_jawa; TSF n_tsfoff (app 479), n_tsfcmdr (485), n_repsold, n_repoff, n_repsold_bh01; Czerka n_czerkaoff (app 18, the only chitin one). Gaps: Rakata, Selkath, Gammorean, Tusken male, Toydarian (all faction 1 or module-only). Zabrak, Cathar, Mon Cal, Aqualish: no rows (u06).
5. **Areas.** Deduped strict-commoner template counts: 302nar 19, 301nar 11, 207tel 12, 203tel 8, 204tel 5, 682dan 15 (loose), 205tel 0. Raw TSV counts are inflated about 3.5x (dedupe on lowercased module stem + resref). Only 204tel has waypoint and walkmesh data (48 waypoints, 344/519 walkable faces in 204TELb) (u08).
6. **Randomness.** Neither doc 34 nor the code has a per-visit input. Code: `HashCreatureId(oid, appearance)` (`DiversityLogic.h:87`); doc 34 text hashes area + tag + position (u09 read the doc; reconcile). Plan: `visitSalt` parameter, default 0 for backward compatibility, set once at area entry, cache per (oid, visit), force nonzero, log the seed, new salts start at 7 (salts 1-6 used) (u09).

## 2. Requirements (consolidated)
- Spawn only from the neutral chitin pool above; set FactionID 5 explicitly; Plot 0; never reuse a vanilla resref or tag; unique tag prefix per spawn, unique waypoint tags (waypoint lookup is area-wide); keep OnDialog `k_def_dialogue01`, blank conversation.
- Never combine flag 87 (AI off) with flag 29 (kills idle). Set flag 65 only on walkers. Do not queue `ActionPlayAnimation` on a walker (the walk step may clear it); use flag 109 or play flourishes only while idle.
- Filter animation pools by the resolved model list (lowercase compare); skip on zero or unknown; schedule the next action after the flourish length.
- Pick candidate areas by dedupe rule, exclude hostile-heavy (kor, ond), ship-like (ebo), cutscene or tiny areas, 205tel, and any area without waypoints/walkmesh data.
- Exclude armed templates until their UTC ItemList is parsed (k2_utc.tsv has no inventory column).
- Install after TSLRCM; conflicts to test: Doodmane444 Citadel mods, N. Drew K2 NPC Overhaul, Leilukin Twi'lek Male NPC Diversity, Kreia's robes patch (replaces `k_204tel_enter`). Never replace `k_*_enter` scripts; append only.
- Existing guards stay: exclusion before any hash draw, companions/named NPCs untouched, `npc-diversity_debug.txt` REMAP debug line for every change.

## 3. Edge cases that must be designed in from the start
- **Persistence/duplicates:** do hook-spawned creatures persist in the save (flags +0x11AC / +0x11FC at `0x52A600`; which value to write is undecoded)? Without a guard, reload duplicates; without despawn (`RemoveFromArea` not identified), visits accumulate. Tag guard must be self-tracked (no GetObjectByTag in the binary; tag length offset unconfirmed).
- **Save-load path:** `0x5582F0` callees exclude `0x55A460`; whether a save-load triggers the per-load reset is unproven.
- **OID stability across loads** unknown: it decides whether a look is stable per creature or re-rolls on reload (u09, u12).
- **Timing:** replace the 30-tick wait with the POP-STABLE wait; cancel pending spawns on area exit (key on area resref); defer during cutscenes/dialog; snap positions to GIT-verified walkmesh points (native path has no verified snap).
- **Safety unknowns:** does attacking a faction-5 spawn make it hostile (doc 35 U4); does `k_def_dialogue01` tolerate a blank conversation; wardrobe scripts (`jc_wardrobe209`) may re-apply looks; quest scripts using `GetNearestObjectByTag` / counters; TSF/Czerka are hostile at some plot moments; companions' and crime reactions.
- **Capacity:** no cap, frame-cost or memory data exists. Measure at 1/6/18 extra creatures; RMP log stops after call 300 (use a dedicated counter).
- **Detour conflicts:** `area-hook-probe` and `k1-spawn-probe` hook the same addresses (`0x568BD0`, `0x55A460`, `0x533830`); only one may be live. `261tel` has an unexplained branch in `AddToArea`.
- **Testing:** forged saves cannot add NPCs; a pre-visit slot is needed (no slot proven pre-Telos); fresh-launch play tests; D0 pause on live fix runs; verify on a scratch copy of the shared tree.

## 4. Scripted encounters (new, idea level)
- Example: TSF agents chase and shoot at an outlaw across the station; all parties invincible, hasted, combat interaction disabled.
- **Not feasible by appearance patching alone:** needs script execution (NWScript) or native action calls. A native hook cannot just run an NSS file; open question: can the patch trigger a script (`ExecuteScript`-style call) or must it set up objects and let a vanilla/Override script run? Override scripts are not KPM-shippable.
- **Building blocks to verify:** `EffectHaste`, plot/immortal flags (`SetPlotFlag`, `SetImmortal`), `SetCommandable`, faction choice so nobody engages the player, `ActionForceMoveToLocation` along waypoints, fake shooting (`ActionCastFakeSpellAtObject`-style or an attack animation), the DeadlyStream report that `CutsceneAttack` did not work for a modder who tried two NPCs fighting, and the K1 workaround of placeables as stand-ins. Armed templates would show weapons; blasters need LOS/projectile handling (unknown).
- **Audio/dialog:** barks need sound resources plus either `ActionSpeakStringByStrRef`/sound-object calls or a one-node DLG. K1 voice reuse needs a licence check under doc 38 section 8 (ledger/ingest/prune/gate), format conversion (SithCodec/WAV, Holocron Toolset), and LIP files (K2 lacks some, per the K1 Ported Alien VO author). Subtitle-only is the safe fallback (blank VO_ResRef).
- **Encounter hygiene:** area whitelist, once-per-visit cooldown, never during plot sequences, no spawn near the player on load, abort on combat/dialog start, cleanup on exit.

## 5. Web prior art (10-13)
- No KOTOR 1 or 2 mod found that adds random ambient events, chase scenes or idle-animation swaps. TSLRCM adds scripted, not random, Citadel behaviour (Grenn remarks); the ambient-NPC mods are Doodmane444's Citadel mods and N. Drew's K2 NPC Overhaul (appearance/dialogue, not events). NPC Diversity Pack [K1] scatters commoner variants (placement and appearance only). Related: Random Enemy Enhancement and Reinforcement Mod (K1 combat, WIP).
- Reverse-direction audio port exists: AshtonVoid's "K1 Ported Alien VO Replacements" (K2 species voices into K1; notes LIP gap; credits KotOR Tool, Holocron Toolset, DLG Editor, SithCodec, HoloPatcher). No K1-to-K2 NPC voice port guide found.
- NWScript notes (DeadlyStream/NWN lexicon): scripted animation ids in TSL need +10000 over the `animations.2da` row; waypoint-walk AI can override a scripted animation (rename waypoint tags); praying NPCs stand when clicked unless OnConversation handles it; the Calo Nord speeder cutscene uses placeables as stunt stand-ins; module scripts are binary-only (DeNCS to decompile).
- Searches were snippet-level only; absence of precedent is not proof.

## 6. Phases
| ID | Phase | State |
|---|---|---|
| V0 | Research wave w9-npcvar (12 Haiku units), web prior art, this doc | DONE |
| V1 | Close static gaps with Haiku recon (no game): vanilla script animation calls and flag meanings, UTC ItemList armed status for the pool, waypoint/walkmesh counts for candidate areas, quest-script tag scan, `+0x11AC` persistence decode and save-load path (REA) | DONE (section 7) |
| NB0 | Shared library `npc-world` (owns H1/H2/H3, provides `k2.npcworld@1`, section 12); moves the hooks and native spawn out of `k1_spawn_probe_src`; V2a and NB1-NB4 become its clients | PARTIAL (section 12.6, host build 1e58d6e: `npc_world_src` 0.1.0-dev, 4 test suites + build gate PASS; live check = old T26 play check through the library, needs REINSTALL and user GO) |
| V2 | Live probe design and build (needs user GO): OID stability across loads, hook-spawn persistence in saves, save-load reset, object count and frame cost | PARTIAL (V2a host build finished, section 13: `npc_world_probe_src` 0.1.0-probe, stages t26/pool/capacity/persist/frame; V2b live TODO, needs user GO; P5, P6, P8-P11, P13 and a real Remove need service growth or RE first) |
| V3 | Pool and roster spec: group pools, area eligibility table, spawn budget, per-species walk speeds | DONE (section 8) |
| V4 | Visit-salt change in doc 34 code (host tests, build only) and doc 34 section 3.2.2 update | DONE (host tests + build; salt unwired, V4b = area-entry source after V2) |
| V5 | Scripted-encounter feasibility: can the patch run or trigger scripts, effect/flag building blocks, K1 audio licence and format path (audio/voice side researched 2026-10-13: see doc 41) | DONE (section 10; data-first route, override-`.ncs` fallback, P11-P15 added to V2) |
| V6 | Animation randomization feasibility: script (flag 109 / local 29) versus native (`Gob::PlayAnimation 004e3270`) | DONE (section 9; native route recommended, P8-P10 added to V2) |
| V7 | Opus gate: choose build order, write the master plan, retire or merge the doc 35 routes | DONE (section 11; build order NB1-NB5, doc 35 merged) |
| NB1 | Visit salt wiring (V4b): `visit_variety` source = area-entry latch | TODO |
| NB2 | Animation injector (`anim_variety`) | TODO |
| NB3 | Extra walkers (`walker_variety`) | TODO |
| NB4 | Scripted encounter, route (a) (`encounter_variety`) | TODO |
| NB5 | Encounter fallback, override `.ncs` (only if P12/P14 fail) | TODO (conditional) |

## 7. V1 results (2026-10-13; static, no game, nothing installed)
Evidence: wave `w10-40-V1` (copies in `.claude/research/npc_variety/v1/u01..u06.md`), tables `.claude/research/npc_variety/tables/*.tsv`, tools `scripts/dump_game_scripts.py`, `scripts/npcvar_tables.py`, `scripts/npcvar_utc_geom.py`. Scratch (untracked): `.claude/scratch_npcvar/nss` (13 core scripts), `nss_all` (638 sources), `nwscript/nwscript.nss`.

### 7.1 Ground truth changed: real NSS sources exist
`Scripts.bif` ships the original `.nss` (all `k_inc_*`, 638 scripts). `dump_game_scripts.py` reads them as-is; the 192 NCS-only scripts cannot be decompiled (this venv's pykotor lacks the `dencs` package). Earlier "no `.nss` on disk" is wrong; everything below cites source lines.

### 7.2 Answers to the OPENs
| OPEN | Answer | Evidence |
|---|---|---|
| Animation ids vanilla plays | Ambient pause = random d8 per waypoint (`GN_PlayWalkWaysAnimation`): FIREFORGET_PAUSE_BORED 103, SCRATCH_HEAD 102, LOOPING_PAUSE2 1 (3 s or 5 s), LOOPING_PAUSE3 24 (male 20.4 s, female 13.3 s), HEAD_TURN_LEFT/RIGHT 100/101. Friend-nearby: TALK_NORMAL 5, TALK_LAUGHING 8, TALK_FORCEFUL 7. Mobile: PAUSE2 2 s, TAUNT 107, VICTORY1 108, else ActionRandomWalk. Also SALUTE 104, BOW 105, GREETING 106 exist. | `k_inc_walkways.nss:608-659`, `k_inc_generic.nss:3162-3254`, `nwscript.nss:1274-1342` |
| Ids vs `animations.2da` | `ANIMATION_*` value = `animations.2da` row (LOOPING 0-46, FIREFORGET 100+). Row-to-name check against the 2da is still unverified. | `nwscript.nss` |
| Per-waypoint animation | Flag 109 `SW_FLAG_USE_WAYPOINT_ANIMATION` + `SetLocalNumber(self, 29, animId)` -> `ActionPlayAnimation(GetLocalNumber(self,29), 1.0, nRand)` queued behind the move; else `ActionWait`. Local 29 and flag 29 `AMBIENT_ANIMATIONS` share a key (author warns). | `k_inc_walkways.nss:275-291`, `k_def_spawn01.nss:52-58` |
| What clears queued anims | The walkways path never clears; `k_inc_generic` clears (`ClearAllActions`) before friend-nearby and mobile anims (3162, 3234); move helpers use `AssignCommand(o, ClearAllActions())` (walkways 387/440/493). | as listed |
| Flag meanings (live set) | Stored as `SetLocalBoolean(self, flagNumber)`. 29 AMBIENT_ANIMATIONS, 65 AMBIENT_ANIMATIONS_MOBILE, 98 START_AT_NEAREST, 99 WALK_ONCE, 100 CIRCULAR, 101 PATH, 102 PAUSE_SHORT (1-3 s), 103 WALK_RANDOM, 104 RUN, 105 DIRECTION, 106 DEACTIVATE (set by the walker itself after WALK_ONCE), 107 PAUSE_LONG (4-6 s per spawn01; ambient.nss says 6-12, unresolved), 108 PAUSE_RANDOM, 109 USE_WAYPOINT_ANIMATION. The "duplicate" numbers (30, 34-39, 41-42, 46-47, 73) sit in a commented block. Generic AI flags: 85 COMMONER_BEHAVIOR, 86 SPECTATOR_STATE, 87 AI_OFF (the 60-62 set is in a commented block). | `k_inc_walkways.nss:15-52`, `k_inc_generic.nss:59-119` |
| UTC VarTable preset of those locals | OPEN (engine side): needs proof that a UTC VarTable Int/bool entry lands in the store `GetLocalBoolean` reads. Probe item P1. `k_def_spawn01` calls `GN_WalkWayPoints()` unconditionally and sets no walk flags (documented default: repeat forever, waypoints named by creature tag). | `k_def_spawn01.nss:31-65,113` |
| Blank conversation | `k_def_dialogue01` forwards to `k_ai_master` event 1004, which calls `BeginConversation()` with no resref check (ActionStartConversation only when the COMPUTER flag is set). Engine result with a blank resref: OPEN, probe P2. | `k_ai_master.nss:181-262` |
| Attack on a faction-5 creature | Scripts never change faction/reputation. 1005 always shouts GEN_I_WAS_ATTACKED, then (commandable, not AI_OFF, idle) calls `GN_DetermineCombatRound`, which attacks only the nearest REPUTATION_TYPE_ENEMY or the given intruder, and does nothing at all if COMMONER_BEHAVIOR, SPECTATOR_STATE or AI_OFF is set. 1006 approaches only a `GetIsEnemy` damager. No flee branch. So a neutral stays neutral in script; whether the engine flips faction/reputation on attack is OPEN (probe P3, doc 35 U4). Death shouts GEN_I_AM_DEAD, nothing else. | `k_ai_master.nss:266-380`, `k_inc_generic.nss:261-330` |
| Combat-off for scripted actors | Set flag 87 AI_OFF (or 85 COMMONER_BEHAVIOR) on the actor: `GN_DetermineCombatRound` is skipped. Fits "chase scene, combat off". | `k_inc_generic.nss:281-285` |

### 7.3 Scripted-encounter building blocks (K2 routine table, ids = list index)
DelayCommand 7, ExecuteScript 8, ActionAttack 37, PlaySound 46, SetFacingPoint 143, SetCommandable 162, ChangeFaction 173, ApplyEffectToObject 220 (nDurationType, eEffect, oTarget, fDuration), ActionSpeakStringByStrRef 240 (nStrRef, nTalkVolume), DestroyObject 241, CreateObject 243 (nObjectType, sTemplate, lLocation, bUseAppearAnimation), EffectHaste 270, SetIsDestroyable 323, ActionForceMoveToLocation 382 (lDestination, bRun, fTimeout), SoundObjectPlay 413, SetPlotFlag 456, ActionCastFakeSpellAtObject 501, CutsceneAttack 503. `SetImmortal` does not exist in K2: use `SetPlotFlag(o, TRUE)` (456) or `SetMinOneHP` (716, params oObject, nMinOneHP). Source: `u04.md`.

### 7.4 Tables (pool, armed, geometry)
- `pool_candidates.tsv`: 71 chitin faction-5 blank-conversation templates (22 alien, 5 tsf, 1 czerka, 43 other; the "other" group contains `g_*`/`c_*` creatures, filter them out in V3 by humanoid body).
- `pool_armed.tsv`: 28 of 71 have a weapon slot filled; 16 have `CLAW3` (natural weapon), so true armed status needs a weapon-slot-only filter in V3. No pool UTC uses the TSL-only second-hand slots.
- `area_commoners.tsv` matches u08 for the 6 target modules (strict commoner counts 302nar 19, 301nar 11, 207tel 12, 203tel 8, 204tel 5, 682dan 15).
- `area_geometry.tsv` (waypoints / creatures / walkable faces): 302nar 50/69/739, 301nar 135/41/578, 682dan 21/2/559, 207tel 17/32/196, 203tel 93/20/371, 204tel 48/17/344. The `.mod` GIT is the live one (302nar rim GIT has 24 waypoints vs 50 in the `.mod`). 301nar and 203tel have the most waypoints to piggyback on for script-free walking; 207tel is geometry-poor (196 walkable faces).

### 7.5 Native questions (REA + local DB)
- VERIFIED: `SaveCreatures` (0x0052A600) writes a creature only when `[obj+0x11AC]==0` and `[obj+0x11FC]==0`; either nonzero = skipped on save (field meanings OPEN). The save-resume path in 0x00533830 calls `LoadModuleInProgress` (0x0055A460) directly; the copy path (0x531BE0 -> 0x556670 -> 0x5582F0) does not.
- OPEN: RemoveFromArea/destroy function and convention; whether runtime-created creatures join the `SaveCreatures` list (CreateObject 0x0066F990 passes area+position to 0x00568BD0, not the new creature; area save 0x00529DD0 builds its list from area+0x194/+0x198); native run-script-by-resref and apply-effect entry points. The Steam address of `ExecuteCommandExecuteScript` is 0x00679050 (the local DB maps it to GOG 0x0078e630); `RunScript` is known only at GOG 0x00640af0/0x00645549 (needs Steam mapping). `Gob::PlayAnimation` 0x004E3270 is `(char* name, float, uint flags, float)`, thiscall; its only direct caller is 0x0045A190 (clear with "NULL"); the VM `ActionPlayAnimation` handler and default-idle setter (candidate FUN_00918420, "pause1" at 0x0099F35C) are unconfirmed. REA cost: E1 154k, E2 149k tokens (both blocked once on path/session, rerun). Stop REA here; V2 probes cover the rest empirically.
- Audio (u05): repo policy says vanilla K1 files stay local, never committed or redistributed; unknown licence counts as needing approval; no audio ingest path exists. A subtitle-only bark = `ActionSpeakStringByStrRef(strref, vol)` with a strref appended via `scripts/append_ranged_kite_tlk.py`, script compiled with `compile.sh`, shipped as an override `.ncs` (pattern proven for `k_def_heartbt01`, T19). Appended-strref playback untested. `.wav/.lip/.ssf` formats and paths are OPEN.

### 7.6 Revised V2 probe list (needs user GO + pre-visit save)
P1 VarTable-preset walker with a stock `k_def_spawn01` OnSpawn (no script change): does it walk the area's `WP_<tag>_NN` waypoints. P2 blank-conversation template: click it. P3 shoot a spawned faction-5 template: does it turn hostile. P4 hook-spawned creature across save/load (OID, persistence, `+0x11AC/+0x11FC` values). P5 AI_OFF (87) on a spawned actor plus `SetPlotFlag`/`EffectHaste`/`ActionForceMoveToLocation` from a script. P6 `ActionSpeakStringByStrRef` with an appended strref. P7 RemoveFromArea on a spawned creature when the area exits. P8-P10 (animation, added by V6): see section 9.5. P11-P15 (scripted encounter, added by V5): see section 10.4.

### 7.7 Recon caveats
Haiku units u01-u03, u05, u06 all returned PARTIAL (they stay inside the read ranges); I closed the gaps from source myself (flag blocks, ANIMATION constants, `GN_DetermineCombatRound`). u04 failed on a bad entry range; I answered it directly. T3 peaked at 115k tokens (over the 100k ceiling).

## 8. V3 results (2026-10-13; host-only, no game, nothing installed)
Tables (generated, read-only inputs are the V1 tables): `.claude/research/npc_variety/tables/roster_pool.tsv` (`scripts/npcvar_roster_pool.py`, test `scripts/test_npcvar_roster_pool.py`) and `roster_areas.tsv` (`scripts/npcvar_roster_areas.py`, test `scripts/test_npcvar_roster_areas.py`). Values are design defaults; V2 measures capacity and may change the cap.

### 8.1 Group pools
Eligible template = resref starts with `n_` AND group in {alien, tsf, czerka} AND `weapon_slot_filled` false (`CLAW3`-only creatures count as unarmed; `n_items` alone is wrong). This drops `c_*`/`g_*` creatures, named NPCs and group "other" without a hand filter. Result: 23 of 71.
| Group | n | Templates |
|---|---|---|
| alien | 17 | `n_bith n_devaronian01 n_duros n_gand n_gran n_quarren01 n_rodian n_sullustan01 n_trandobrn n_trandogrn n_tuskenf01 n_twilekf n_twilekm n_walrusman n_weequay01 n_wookief n_wookiem` |
| tsf | 5 | `n_repoff n_repsold n_repsold_bh01 n_tsfcmdr n_tsfoff` |
| czerka | 1 | `n_czerkaoff` |

Region weights (design default, tunable): `telos` alien 50 / tsf 30 / czerka 20; `nar_shaddaa` alien 70 / czerka 30 / tsf 0 (TSF is Telos security). Pick order per spawn: group by weight, then template uniformly inside the group, using the salted `HashCreatureId` family (new `Mix` salts start at 7).

### 8.2 Area table
Eligible iff strict commoners >= 5 AND waypoints >= 40 AND walkable faces >= 300 (`hostile` is not a gate: it counts creature templates, not area danger).
| Module | strict | waypoints | faces | Eligible | Budget |
|---|---|---|---|---|---|
| 302nar | 19 | 50 | 739 | yes | 4 |
| 301nar | 11 | 135 | 578 | yes | 2 |
| 203tel | 8 | 93 | 371 | yes | 2 |
| 204tel | 5 | 48 | 344 | yes | 1 |
| 207tel | 12 | 17 | 196 | no (`waypoints<40+faces<300`) | 0 |
| 682dan | 0 | 21 | 559 | no (`strict<5+waypoints<40`) | 0 |

Total 9 extras maximum across the four areas, one area per visit. `205tel` and hostile, ship and cutscene areas stay excluded per section 2. Note: u08's "682dan strict 15" is the `loose` count; the strict column is 0.

### 8.3 Spawn budget
`budget = min(cap, max(1, strict // 4))` for eligible areas, else 0. `cap = 4` is a placeholder until V2 measures 1, 6 and 18 extra creatures. Extras are counted per visit and never persisted. Gated by ini switch `walker_variety=0|1` (default 0), to be added together with the hook (not yet present). Each later feature gets its own default-off switch, same pattern as `visit_variety` (doc 34 section 3.2).

### 8.4 Walk speed
Use the template's own appearance `walkdist` unchanged. Speed class: slow < 1.5, normal 1.5..2.2, fast > 2.2. All 23 eligible are normal (1.6..2.0, TSF/Czerka/most aliens 1.813) except `n_tuskenf01` 1.2 = slow (accepted: reads as an old or limping Tusken); none are fast. Never set run for walkers (flag 104 off).

### 8.5 Open items carried forward
- V2: capacity cap, persistence and save-load behaviour, P1 VarTable-walker proof (do the extras actually walk `WP_<tag>_NN`).
- V7: TSF and Czerka hostility at plot moments (TSF walkers in the wrong storyline state), build order.
- Resolved: armed status = weapon slot only (section 7.4 caveat closed).

## 9. V6 results (2026-10-13; host-only, no game running, nothing installed)
Tables: `.claude/research/npc_variety/tables/anim_pool.tsv` (`scripts/npcvar_anim_pool.py`, 13 target models, 272 rows, 7 ambient flourish names) and `anim_ids.tsv` (`scripts/npcvar_anim_ids.py`, 58 rows, ids 0-46 and 100-110). Both have colocated tests (`run_tests.py npcvar` = 6 suites PASS).

### 9.1 T1 pool facts
- Every one of the 13 models has all 7 idles (`pause1 pause2 pause3 standstill neutral good evil`) and all 7 `ambient` flourishes (`pausesh scanning shrug rolleyes touchheart nodyes greeting`; lengths 1.3-5.1 s). `military` = `salute` only (1.6 s; 1.7 s on the female and Twi'lek bodies). Everything else (`victory taunt bow point equip itemequip`) is `excluded`.
- Chain depth: `N_Bith` 3 (297 anims), `N_CzerkaOff` 5, `PFBCM` 6 (460). Flourishes are therefore the same set for all targets, so one list per class is enough (no per-model table needed in the DLL).

### 9.2 T2 result: `ANIMATION_*` is NOT an `animations.2da` row id
All 11 V1 claims fail when the script constant is used as a row index: 1 -> `walkinj`, 24 -> `meditate`, 100-108 -> `c2p2 c2p3 c2p4 c2p5 c2d1 c2d2 c2d3 c2d4 c2d5` (rows 100+ are weapon-stance animations, listed as "stance-like" in `u03_anims.txt`). The 2da has 571 rows (0-570) and its row names do line up with the constants by NAME (`pause2` row 7, `salute` row 16, `bow` 19, `greeting` 31, `taunt` 33, `victory` 17), so the engine must translate script constants to 2da rows through an internal table we have not read. Consequences:
- Which name a script ID really plays (V1 claims for 102 scratch head, 103 bored, 104 salute, ...) is UNVERIFIED; the V1 sentence "104 = salute" rests on the constant name only. Do not rely on it for the script route.
- The native route is unaffected: it passes the animation NAME string (`pausesh`, `salute`, ...) which the model chain resolves by itself.
- Id 42 has no nwscript constant (DIVE_ROLL line is commented out).

### 9.3 Verdict: script versus native
| | Script (flag 109 + local 29) | Native (`Gob::PlayAnimation 0x004E3270`) |
|---|---|---|
| Reach | Only creatures running the walkways include, at waypoint pauses; widens the existing d8 pause set | Any creature, including stationary commoners |
| Shipping | Needs an override `.ncs` (KPM cannot ship it; user-installed copy only) and P1 (VarTable preset reaching `GetLocalNumber`) | Part of the existing `npc-diversity` DLL, no script |
| ID mapping | Needs the unread constant-to-row translation (9.2) | Uses names, already proven in Mod 10 (flourish recipe: speed +1.0, flags 0, timer ends 60 ms before wrap, re-issue idle `(name,1.0,2,0)`) |
| Hazards | Queue clears (section 3), flag 87 + 29 conflict | Needs creature -> Gob and an "idle" test (open), idle state machine may overwrite |

### 9.4 Recommendation
Native injector inside `npc-diversity`, behind ini switch `anim_variety=0|1` (default 0, own key like `visit_variety`). Classes from `anim_pool.tsv`: `ambient` for all, `military` only for TSF bodies; minimum 20 s between plays per creature; only when the creature has no action and is not in conversation or combat; pick via the salted `HashCreatureId` family (next free `Mix` salt: 11 or higher, V3 starts at 7). The script route stays the fallback only if P1 passes AND P8 fails.

### 9.5 Probes added to the V2 list (7.6)
P8 `PlayAnimation` on an idle world commoner: obtain the Gob from the creature object, check whether the idle state machine resumes or overwrites afterwards. P9 flourish during a walk (expect snap or overwrite; restrict to idle only if so). P10 cost of the existence probe `vt[0x1c](name,&len)` per creature per tick (returns -1.0 when missing).

### 9.6 Open (not claimed)
Creature object -> Gob field, and how to read "creature is idle" natively (V2 P8, or one `rea-recon` run; skipped here as P8 answers it empirically). Script constant -> animation row translation (only needed if the script route is revived).

## 10. V5 results (2026-10-13; host-only, no game running, nothing installed)
Evidence files: `.claude/research/npc_variety/tables/script_primitives.tsv` (from `scripts/npcvar_script_scan.py`), `.claude/research/npc_variety/tables/faction_matrix.tsv` (from `scripts/npcvar_faction_matrix.py`), `.claude/research/npc_variety/v5/t3_native_script_entry.md`, `.claude/research/npc_variety/v5/t4_vanilla_patterns.md`.

### 10.1 What the evidence says
- **Vanilla corpus has no chase/shoot script.** In 637 scanned scripts (120 use a tracked primitive; module scripts are not in the corpus) `CutsceneAttack` has 0 live calls (one hit, a comment at `k_inc_fakecombat.nss:49`; the "2 scripts" in the V5 plan was wrong), `ActionCastFakeSpellAtObject` 0, `EffectHaste` 0, `ChangeFaction` 0, `ActionForceMoveToLocation` 2 (`a_move_party`, `k_inc_utility:1181`), `SetPlotFlag` 1 (a door lock, `k_inc_utility:2118`). The only entry script of the three examined is `a_move_party` and it moves party slots 0-2 only (`t4`, E2). `FAI_*` fake combat and `UT_PlotMoveLocation` are include-only; no other corpus script uses them (`rg`).
- **Vanilla fake attack is a real attack:** `FAI_PerformFakeAttack` = `EffectAssuredHit` + real `ActionAttack`, with `SetMinOneHP(target, TRUE)` as the damage floor and the target on a chosen standard faction (`k_inc_fakecombat.nss:21-26,53-54`). So "shooting that cannot kill" is a Min1HP/Plot target plus a real attack, not a cutscene effect.
- **Template fields already carry invincibility.** Our UTC tooling confirms `Plot`, `Min1HP`, `NoPermDeath`, `Interruptable`, `WalkRate` as UTC fields (`scripts/npc_add_build_utc.py:6`); pool templates currently use Plot 0 / Min1HP 0. A template with Plot=1 needs no script (UNVERIFIED in game: P12).
- **Native script entry is not usable yet.** `0x006FD8D0` takes (CExoString-like*, objid, flag); calling convention, ECX, retn size and string layout are all unproven, and no safe call site exists (`t3` sections 1, 2, 4). A native call would also only reach chitin scripts, and none does the choreography.
- **KPM cannot ship a new `.ncs`.** The proven workaround is a user-installed override `.ncs` (T19 `k_def_heartbt01`, play-verified spawn without DLL, doc 38 lines 150 and 164); it ships as override files installed by the user (`t3` section 3).

### 10.2 Faction conclusion (`faction_matrix.tsv`, 21 factions, rule hostile = repute < 50; values are only 0/50/100 except Player->SELF_LOATHING 9)
- **Candidate chaser/outlaw pair: Predator (11) and Prey (12).** 11->12 and 12->11 are 0 (hostile); both are 50 (neutral) to the player (row 0), to commoners (5) and to themselves. Other mutually hostile pairs that are not hostile to the player: Gizka_1/Gizka_2 (16, 17), (2, 4), (4, 8), (2, 15).
- **Caveat:** engine AI for predator/prey creatures (animals) may behave differently from humanoids (flee, no ranged weapon use): P14.
- No OUTLAW/TSF standard faction exists in `nwscript`; vanilla TSF/Czerka factions are non-standard ids and are hostile at some plot moments (section 3), so avoid them.

### 10.3 Verdict: three routes
| | (a) Data-first: UTC templates (Plot/Min1HP, WalkRate) + faction pair 11/12 + engine AI | (b) Native-driven (DLL calls RunScript or action entries) | (c) Hybrid with a user-installed override `.ncs` (T19 pattern) |
|---|---|---|---|
| Shipping | Data only, if KPM can ship the new UTC (UNVERIFIED; doc 38 shipped it as override files under user GO) | DLL only | User-installed override files, not KPM-shippable |
| Chase | Engine AI hostility chase (UNVERIFIED, P14); no waypoint path | Needs action-queue entries, unproven | `ActionForceMoveToLocation` along waypoints, vanilla `UT_PlotMoveLocation` pattern (`t4` E3) |
| Invincible + hasted | Plot/Min1HP via template (P12); haste only through a WalkRate row (P12) | Unknown struct offsets | `SetPlotFlag`, `EffectHaste` (routines 456, 270) |
| Fake shooting | Real fight between armed templates, damage floor from Plot | Unknown | `FAI_PerformFakeAttack` pattern: real `ActionAttack` + assured hit on a Min1HP target; `ActionCastFakeSpellAtObject` never used in vanilla (P13) |
| Control | Weakest (spawn, then AI decides) | Strongest if it works | High, proven by T19, per-event logic |
| Status | Most promising; unknowns are P12/P14 | NO-GO until P11 (convention unproven; only reaches chitin scripts anyway) | Reliable fallback |

### 10.4 Recommendation and probes
- **First route: (a).** The DLL spawns two of our own templates (an armed chaser, an outlaw) at area waypoints with unique tags, factions 11 and 12, Plot=1, a fast WalkRate row, with engine AI doing the chase. **Fallback: (c)**, one user-installed override script on the spawned creature's heartbeat/OnSpawn doing waypoint moves and the fake attack. **(b) only if P11 passes**, and then mainly to call the (c) script without relying on the heartbeat.
- **Encounter hygiene vs route:** area whitelist, once per visit, no spawn near the player on load, and cleanup on exit are enforced by the DLL in every route. Abort on combat or dialog start: the DLL checks player state and removes the creatures (P15). Route (a) cannot stop a running AI fight mid-way except by removing the creatures.
- **Probes added to the V2 list (7.6), each yes/no:** P11 call RunScript natively with a chitin resref on a live creature (convention from one REA run or a safe trial in a tick hook). P12 a Plot=1 template survives real hits, and a fast WalkRate row (or `EffectHaste`+`SetPlotFlag` via script in route c) hastes a spawned actor, then a chase over `ActionForceMoveToLocation` or engine AI. P13 `ActionCastFakeSpellAtObject` gives visible fake shooting with no damage (route c only). P14 faction pair 11/12 (then 16/17 as the second choice): the two chase/fight each other and nobody engages the player or commoners. P15 cleanup via `DestroyObject` or area-exit removal leaves no stray creature (extends P7). P5 and P6 stay as listed.

### 10.5 Audio
Voice and barks for the event are owned by doc 41 (sections 3 and 6: architecture, test ladder E1-E8); this phase adds no audio work. Subtitle-only barks (blank VO_ResRef, `ActionSpeakStringByStrRef` or a DLG node) remain the fallback and are independent of the route chosen.

### 10.6 Open (not claimed)
Calling convention of `0x006FD8D0` (one `rea-recon` run, T6 skipped; P11 answers it empirically). Whether KPM ships a new `.utc` or only chitin-pool resrefs. Behaviour of predator/prey AI. Module scripts (not in the corpus) may still contain `FAI_*` users; regenerate with `scripts/dump_game_scripts.py --out <dir> --all` if that matters.

## 11. V7 results (2026-10-13; docs only, nothing installed)
Opus gate. Origin order was a (animation), b (walkers), c (events); built in value x risk order below. One probe DLL, one detour set per address (`0x568BD0/0x55A460/0x533830`): extend `k1_spawn_probe_src`, never a second DLL.

### 11.1 Build order
| Phase | Goal | Gate probes | ini switch | Size | Rollback |
|---|---|---|---|---|---|
| V2a | Combined probe DLL: P1-P15 + capacity as ini-selected stages (host tests + build gate only) | none (host) | stage key in probe ini | ~1-2 days | not installed |
| V2b | Run the stages live; results fill 11.4 | needs user GO | probe ini | ~1 play session | remove probe mod, restore `npc-diversity` |
| NB1 | Visit salt wiring: salt from area-entry latch (H2 + POP-STABLE ~33 ticks, 7/7 loads) | none beyond proven latch | `visit_variety` | ~0.5 day | switch 0 |
| NB2 | Animation: native `PlayAnimation 0x004E3270` injector, new Mix salts 11+ | P8, P9, P10 | `anim_variety` | ~1-2 days | switch 0 |
| NB3 | Extra walkers: hook spawn from pool, cap 4 until capacity measured | P1, P2, P3, P4, P7, capacity | `walker_variety` | ~2-3 days | switch 0 |
| NB4 | Scripted encounter, route (a) of section 10 (Plot/Min1HP templates + faction 11/12 + engine AI) | P12, P14, P15 | `encounter_variety` | ~2 days | switch 0 |
| NB5 | Encounter fallback, route (c): user-installed override `.ncs` (T19 pattern), not KPM-shippable; only if P12 or P14 fails, user decides then | P5, P13 | `encounter_variety` | ~2 days | delete override |
| Audio | Doc 41 V-C / E1-E8 and kvoice, own track; joins NB4 only after the E-ladder passes, else subtitle-only | E1-E8 | doc 41 | per doc 41 | per doc 41 |

New Mix salts: V3 salts start at 7 (1-6 used); animation uses 11+.

### 11.2 Doc 35 disposition (MERGE, nothing deleted)
| Route | Fate | Reason | Its U-questions |
|---|---|---|---|
| `.mod` GIT edit ("C hybrid", builders `npc_add_build_git.py`, `npc_add_build_utc.py`, `npc_add_waypoints.py`, `npc_add_install.py`, `check_npc_add.py`, wave 2 build 84ca69a) | Retired as primary; kept as fallback toolbox (UTC and waypoint builders reused for NB3/NB4 templates) | Fixed per install and per save-visit (no per-visit variety), writes the user's `Modules/204tel.mod`, needs U1/U2/U8 answers, gives every unvisited save the NPCs | U1, U2, U3, U8 RETIRED (reopen only if P4 shows hook spawns do not survive) |
| Route A (override UTC `k_def_spawn01` CreateObject) | Retired | Not KPM-shippable (R5), spawn-script firing unresolved | same |
| Patch-only H2 hook (U11) | Adopted = doc 40 spawn route (NB3) | Per-visit variety, KPM-shippable, latch proven | U11 -> P1/P4/P7 |

Other mappings: U4 -> P3; U5 -> V2b pre-visit save; U6 -> packaging item in NB3 (loose Override files, Twi'lek F 47 / Trandoshan 464); U7 -> UTC soundset check in NB3 (soundset 35); U9 -> NB3 Tier 2 model path; U10 -> capacity stage. Doc 35 Gate 1 (live `.mod` write) stays NOT GIVEN and is now moot.

### 11.3 Probe to phase map
- NB3: P1 (VarTable walker with stock `k_def_spawn01`), P2 (blank-conversation click), P3 (shoot a faction-5 spawn), P4 (hook-spawn across save/load: OID, `+0x11AC/+0x11FC`), P7 (RemoveFromArea on area exit), capacity 1/6/18.
- NB2: P8-P10 (section 9.5).
- NB4: P12, P14, P15 (section 10.4).
- NB5 or later: P5 (AI_OFF + plot + haste from script), P6 (`ActionSpeakStringByStrRef` strref), P11, P13.
- P1/P4 also decide whether doc 35 U1-U3 reopen.

### 11.4 V2b result slots (fill after the live run)
| Probe | Result (true/false) | Evidence |
|---|---|---|
| P1 | | |
| P2 | | |
| P3 | | |
| P4 | | |
| P5 | | |
| P6 | | |
| P7 | | |
| P8 | | |
| P9 | | |
| P10 | | |
| P11 | | |
| P12 | | |
| P13 | | |
| P14 | | |
| P15 | | |
| Capacity 1/6/18 | | |

### 11.5 Live-step gates
- User GO (never inferred).
- Game closed for any install or hot-swap.
- D0 pause lifted: OPEN, the repo has no record it is lifted.
- Pre-visit save slot named by the user (none proven; candidates Game1/5/8).
- Play tests belong to the user, not the agent.

### 11.6 Residual risks
- TSF/Czerka hostile at plot moments: spawned creatures are forced to faction 5 and areas are whitelisted; covered by P3.
- Mod conflicts: Doodmane444, Drew, Leilukin, Kreia robes (section 2).
- `261tel` AddToArea branch differs from `204tel`.
- One detour set per address: probe and `npc-diversity` hooks cannot be live together on `0x568BD0/0x55A460/0x533830` (`area-hook-probe` was removed in T26 for this reason).

## 12. Shared library `npc-world` (decision 2026-10-13, design only, nothing built)
User decision: spawn new NPCs AND change existing ones, through one common library patch that `npc-diversity` and the new features all use. Same pattern as `save-events` (`kind = "library"`, `provides`, KPatchInit registry, C ABI table, header-only client in `_shared_src`, `requires` or soft-connect in consumers).

### 12.1 Why a library
- KPM rejects two patches that detour one address. `k1-spawn-probe` (and the retired `area-hook-probe`) own `0x00568BD0` AddToArea, `0x0055A460` LoadModuleInProgress and `0x00533830` MainLoop. NB1 (salt), NB2, NB3, NB4 all need the same three events, so exactly one patch may own them.
- `npc-diversity` keeps `0x0076D200` SetAppearance and `0x0085E601` texture (no overlap, stays where it is). Spawned walkers pass through SetAppearance on the client side, so commoner rows are remapped automatically; no extra call needed.
- (`save-events` only mentions 0x00533830 in a comment; its hook sites are different. Re-check with `rg` of all `*.hooks.toml` before NB0 install.)

### 12.2 Provider: `npc-world` (new `npc_world_src/`, id `npc-world`, `provides = ["k2.npcworld@1"]`, header `_shared_src/NpcWorld.h`)
Owns: H1 MainLoop `0x533830`, H2 LoadModuleInProgress `0x55A460`, H3 AddToArea `0x568BD0`. Code moved from `k1_spawn_probe_src` (area tracking, tag-seen list, 30-tick POP-STABLE gate `SpawnLogic.h`, native spawn sequence with step logging, `g_spawning` re-entry guard). `conflicts = ["k1-spawn-probe", "area-hook-probe"]`.

Events (mask, one K2NW_* bit per event; callbacks cheap, engine thread, 5 ms WARN as in save-events):
| Event | When | Payload |
|---|---|---|
| `K2NW_AREA_LOAD` | H2 fires | load seq |
| `K2NW_CREATURE_ADDED` | H3 fires (not for the library's own spawns unless `origin` flag says so) | creature ptr, OID, tag, area resref, pos, `origin` (engine/lib) |
| `K2NW_AREA_STABLE` | 30 quiet ticks after H3 settles, once per load | area resref, area ptr, `visit_seq`, `visit_salt` (hash of load seq + wall clock bits; replaces the unwired `g_visitSalt`) |
| `K2NW_TICK` | every H1 (opt-in, expensive; most consumers use AREA_STABLE) | tick counter |

Services (table functions, all safe no-ops on a bad arg):
| Service | Use |
|---|---|
| `Spawn(resref, tag, x, y, z, facing)` -> OID or 0 | native new/ctor/CResRef/LoadFromTemplate/orient/AddToArea; skips if tag already seen; counts against a global cap (default 4, ini, hard max from capacity probe) |
| `TagPresent(tag)` | dedupe against save-loaded creatures |
| `CurrentArea(buf)` / `VisitSalt()` / `VisitSeq()` | context for pickers |
| `ForEachCreature(cb, user)` | enumerate area creatures from the AddToArea-seen list (existing NPC changes: animation, remap, despawn) |
| `PlayAnimation(creature, anim, speed)` | NB2, wraps `0x004E3270` (added in NB2, struct grows) |
| `Remove(oid)` | P7 outcome (RemoveFromArea), added when verified |

### 12.3 Consumers
| Consumer | Uses | Notes |
|---|---|---|
| `npc-diversity` (existing, 0.1.x) | `AREA_STABLE` salt via soft client (no `requires`, no-op without provider; `visit_variety` stays off by default) | NB1. Stays soft so a missing library cannot break the live appearance remap. |
| `npc-walkers` (new, `walker_variety`) | `AREA_STABLE`, `Spawn`, `TagPresent` | NB3. Pool/roster from V3 tables (section 8). |
| `npc-anim` (new or inside walkers, `anim_variety`) | `ForEachCreature`, `PlayAnimation` | NB2. |
| `npc-encounter` (`encounter_variety`) | `AREA_STABLE`, `Spawn` x N | NB4. |
| `k1-spawn-probe` -> `npc-world-probe` | everything, ini-selected stages P1-P15 + capacity | V2a. Becomes a client, no hooks of its own. |

### 12.4 Phase changes
- New **NB0** (before V2a): build `npc-world` by moving the hooks and spawn code out of `k1_spawn_probe_src`; host tests (colocated: `npcworld_registry_test.cpp`, reuse `spawn_logic_test.cpp`), `common_contract_test.cpp` pinning `NpcWorld.h`, `python3 scripts/compat_check.py` over `_shared_src/NpcWorld.h`, version `0.1.0-dev`. Size ~1.5 days.
- NB0 live check = the pending T26 play check done through the library (mouse droid + `k1p_native_npc` in `204tel` appear once, no duplicate after save/load) before any new feature. It needs REINSTALL (hooks owner changes: `k1-spawn-probe` out, `npc-world` in), so the gates in 11.5 apply, with the user running the launcher.
- V2a becomes: `npc-world-probe` client with the stage list, not a second hook owner. V2b unchanged.
- Order is now: NB0 -> V2a -> V2b -> NB1 -> NB2 -> NB3 -> NB4 -> NB5 (conditional). Rollback of NB0: reinstall `k1-spawn-probe` 0.2.0-probe.

### 12.5 Risks
- Single point of failure: a bug in `npc-world` hooks hits every consumer. Mitigation: log-before-step, SafeRead everywhere, `enabled=0` ini in the library disables events and spawns but keeps hooks passive.
- Hook moves are REINSTALL, not SWAP: any later address addition costs a launcher reinstall. Put all three addresses plus the future `PlayAnimation` call (a call, not a detour) in at NB0.
- Registry needs a KotorPatcher with `KPatchInit` (already true for `save-events`).
- Spawned OIDs differ per load (P4 decides whether persisted state could ever key on them); consumers must key on tag, not OID.

### 12.6 NB0 host build results (2026-10-13; nothing installed)
Files: `_shared_src/NpcWorld.h` (interface + `k2nw::Client`), `npc_world_src/` (`NwCore.h` pure core, `NpcWorldProvider.cpp` engine side, `manifest.toml` 0.1.0-dev, `exports.def`, `kotor2-steam-aspyr.hooks.toml`, `test.toml`). Tests (run `python3 scripts/run_tests.py npc_world_src nw_registry_test nw_world_test npcworld_client_test`): `nw_registry_test` (5079 checks), `nw_world_test` (126), `npcworld_client_test` (86), `common_contract_test` (pins SafeRead offsets, `gameaddr::New`, KpLog). `python3 scripts/build_check.py --mod npc_world_src` PASS (4 exports, 3 detours). The header was copied into the KPM clone with `check_shared_headers.py --sync` (only NpcWorld.h drifted); that sync is outside the repo and must be repeated on any other machine.

Changes against the 12.2 sketch (decided while building):
- **Salt is chosen at `AREA_LOAD`** (H2), not at `AREA_STABLE`, and `VisitSalt()` is valid from then on: `npc-diversity`'s SetAppearance runs during the load, before the 30-tick latch. NB1 therefore reads `VisitSalt()` directly; no callback needed.
- **`ForEachCreature` and `PlayAnimation` and `Remove` are NOT in v1.** A raw pointer list of seen creatures can dangle once the engine frees an object, so enumeration waits for NB2 (it needs a pointer-validity rule); the table grows through `struct_size`. v1 services: `Register/Unregister`, `VisitSalt`, `VisitSeq`, `CurrentArea`, `TagPresent`, `Spawn`, `SpawnedCount`, `SpawnCap`.
- `Spawn` refuses before the current visit is stable, on a duplicate tag, over the cap (`npc_world.ini [world] spawn_cap`, default 4, max 32), and from inside its own re-entrant add callback. Library-spawned adds are delivered as `CREATURE_ADDED` with `origin = LIB` and never reset the stable gate.
- Seen-tag table: 256 tags, case-insensitive on 32 chars; on overflow `TagPresent` answers true (unknown counts as present, so nobody double-spawns).
- Ini: `npc_world.ini [world] enabled=1|0` (0 = hooks stay passive: no events, salt or spawns), `spawn_cap`, `verbose` (per-AddToArea DEBUG lines). Log file `npc-world_log.txt`.

Open items before NB0 can be called DONE:
1. **Live check** (needs user GO, game closed, REINSTALL because the hook owner changes: remove `k1-spawn-probe`, add `npc-world`): `c_drdmouse01` and `k1p_native_npc` must appear once in `204tel` and not duplicate after save/load. The roster moves to the V2a probe client, so for this check use a tiny throw-away client or run the V2a stage `T26-equivalent`.
2. **Allowlist review** (human/Opus): `tests/static/raw_addresses.toml` needs a block for `patch_manager_mods/npc_world_src/NpcWorldProvider.cpp` with `0x00533830, 0x00543F10, 0x0055A460, 0x00561F30, 0x00568BD0, 0x00572A70, 0x00710840` (same set as `K1SpawnProbe.cpp` plus the two hook sites already allowlisted for the area probe; `query_ghidra.py address` verifies all but `0x00710840`, which returns no row). The file has uncommitted peer edits, so it was not touched; `suite:check-raw-addresses` fails on these lines until then.
3. `suite:check-hook-spans` still FAILs on `area-hook-probe` vs `k1-spawn-probe` (same three sites; neither manifest declares the conflict). Not caused by NB0 (`npc-world` declares both conflicts and is not flagged); it resolves when the probe's hooks are removed in V2a.

## 13. V2a results (2026-10-13; host-only, nothing installed)
`patch_manager_mods/npc_world_probe_src/` = id `npc-world-probe` 0.1.0-probe, a DLL-only client of `npc-world` (`requires = ["npc-world"]`, `requires_interfaces = ["k2.npcworld@1"]`, conflicts `k1-spawn-probe` and `area-hook-probe`). It owns no detour (hooks toml is `[metadata]` only; KPM loads a hookless patch as DLL_ONLY and still runs its `KPatchInit`), so the three-site overlap with the old probes disappears once they are removed. `k1_spawn_probe_src/` is untouched (peer-owned, untracked): rollback = reinstall `k1-spawn-probe` 0.2.0-probe and remove `npc-world` + `npc-world-probe`.

Files: `ProbeCore.h` (pure stage logic, `nwp::Probe` against an `nwp::Env`), `NpcWorldProbe.cpp` (DLL glue: decodes `K2NwEvent`, reads creature `+0x11AC/+0x11FC/+0x90` through saferead, KpLog sink, ini), `manifest.toml`, `exports.def` (`KPatchInit`), hooks toml, `test.toml`, `npc_world_probe.ini.sample`. Log: kpatch.log patch `npc-world-probe` (channels core/stage/persist/frame/spawn), fallback `npc-world-probe_log.txt`. A DLL-only client has no hook to retry the provider lookup from, so `npc-world` must load first (the `requires` ordering); if it is missing the probe logs one WARN and stays inert.

Tests: `python3 scripts/run_tests.py nwprobe_core_test nwprobe_e2e_test npc_world_probe_src` (core: stage list, row parsing, config keys, capacity grid, frame stats, every stage against a fake Env, log-line length; e2e: the real `nw::World` behind the real C table and `k2nw::Client`, visits 1-3 with restored / not restored copies, capacity 6 against cap 4 and 18 against cap 18, frame windows, no provider; `common_contract_test` pins the three creature offsets, the KpLog 232-byte cut and the inert client). `python3 scripts/build_check.py --mod npc_world_probe_src` PASS (1 export, 0 detours). Three mutations of the core (persist verdict, ge33 threshold, realtag skip) were caught.

### 13.1 Stage to probe map
| Stage | Ini | Covers | Evidence line |
|---|---|---|---|
| `t26` | default | the NB0 live check (old T26 roster in `204tel`) | `SPAWNED`, `SKIP stage=t26`, `STABLE ... restored=` |
| `persist` | default | P4 (OID, `+0x11AC/+0x11FC`, area id of a lib add), P7/P15 observational (engine-restored copy of a watched tag, previous-visit verdict `none-persisted`, `partial` or `all-persisted`) | `LIBADD`, `RESTORED`, `PERSIST-CHECK` |
| `capacity` | `capacity_n` 1/6/18 (+ `npc_world.ini [world] spawn_cap >= N`) | capacity count; `capacity_realtag` stops a doubling when copies were restored | `CAPACITY requested= spawned= cap=`, WARN on cap |
| `frame` | `frame_window` 120 | frame cost: tick-interval avg/max/>=33 ms/>=50 ms, pre vs post windows (budget 4 / 12 per visit), load stalls above 2 s ignored | `FRAME phase=pre` or `post` |
| `pool` | `row1..row16` | P1 / P2 / P3 / P12 / P14 templates: the DLL only spawns; waypoint walking, clicks and hits are the user's play observations | `SPAWNED`, `SKIP stage=pool` |

### 13.2 Not covered by V2a (needs more than the v1 library service)
- P8-P10 animation (`Gob::PlayAnimation`, creature-to-Gob, existence probe cost), P5, P6, P11, P13 (native script, effect and speak entry points unresolved, section 7.5): no library service and no verified convention. They become V2c after NB2 grows the table (`PlayAnimation`, `ForEachCreature`) or an RE run resolves the conventions.
- A real `Remove`: P7/P15 are observed only as "did the engine restore the spawn on the next visit"; whether leaving the area destroys it is read off the `PERSIST-CHECK` verdict, not forced.
- P1 needs a pool template whose `OnSpawn` is stock `k_def_spawn01` and whose tag matches the `WP_<tag>_NN` waypoints of the chosen area; picking that pair is V2b prep (tables in `.claude/research/npc_variety/tables/`).

### 13.3 V2b run list (needs user GO, game closed for the install, pre-visit save named by the user)
1. Run A (NB0 live check, default ini): REINSTALL = remove `k1-spawn-probe`, add `npc-world` + `npc-world-probe`. `c_drdmouse01` and `k1p_native_npc` appear once in `204tel`; save; reload the save; `PERSIST-CHECK` verdict and no duplicate.
2. Run B (capacity, k1 disabled via its ini so it does not use the cap): `stages=capacity,persist,frame`, `capacity_n` 1, then 6, then 18, with `spawn_cap=N+2` in `npc_world.ini` (20 for N=18); read `FRAME` pre vs post and the user's frame feel.
3. Run C (pool rows): P1-P3 and P12/P14 templates once V2b prep picks them.

### 13.4 Open
- `suite:check-raw-addresses`: the `npc_world_src` provider block was added by the doc 38 session (10-13, uncommitted in `tests/static/raw_addresses.toml`; all addresses verified via `query_ghidra.py` except `0x00710840`, REA-recon in doc 38 sec 13/14). The only remaining FAIL is `voice_probe_src` (not ours). The probe has no raw addresses.
- `suite:check-hook-spans` PASSES (10-13): `k1-spawn-probe` 0.3.0-probe (doc 38 session, untracked source) is a hookless client of `npc-world`, so `k1_spawn_probe_src` no longer shares the three sites. `conflicts` of `npc-world` and `npc-world-probe` now list only `area-hook-probe`. The live `k1-spawn-probe` 0.2.0 install still owns the sites until the V2b REINSTALL (its play test is harvested first); both clients may run together (dual-run matrix is doc 38 sec 17: load order npc-world -> k1 -> npc-world-probe, k1 spawns first and shares the default `spawn_cap` 4; Run 1 `stages=t26,persist,frame`, where `t26` is refused by `TagPresent`; Run 2 sets `[spawn] enabled=0` in k1's ini and `spawn_cap=N+2`, because `stages=persist,capacity,frame` spawns nothing without `capacity_n`). ONE atomic install of npc-world + npc-world-probe + k1 0.3.0 (task T35): a hand-edited config must not leave k1 0.2.0 hooks and npc-world hooks on the same addresses, since the second VerifyBytes aborts every later block.
- Capacity grid defaults (`-39.0,-63.1` + `0.9` steps, 6 columns) are unverified floor points in `204tel`; a cell off the walkmesh is logged and visible, not prevented. Adjust `capacity_x/y` after Run A shows where the mouse droid stands.
- **Install gate (peer, 10-13):** `k1-spawn-probe` 0.2.0-probe (md5 19cb16df, 3 hooks) is LIVE since 10-13 00:14 and its play test is not harvested yet (tag path, TextureVar/BodyVariation offsets, `+0x11AC/+0x11FC`, duplicate-on-reload). `npc-world` and `npc-world-probe` cannot be installed until it is removed (KPM address conflict), and must not replace it before that play is harvested. Likely path: retire the old probe in favour of `npc-world-probe` stage `t26` after the harvest; the user decides. `native-creature-spawn-re` holds `k1_spawn_probe_src` unchanged until then.
- Open (doc 38 T27): a body/TextureVar variation at AddToArea needs no library change for a first version: `K2NW_CREATURE_ADDED` runs before the engine's AddToArea code, so a client may write its own creature field in the callback (engine adds only, keyed on tag). A v2 table entry is justified only if two clients need the same write. The creature field itself is unverified (doc 38 RE item). NB2 appends `PlayAnimation`/`ForEachCreature` after `SpawnCap`; clients check `struct_size`.
