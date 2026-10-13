# 40 NPC behaviour variety: extra walkers, scripted encounters, animation

## Status
- **State: research phase V0 done (2026-10-13); nothing built, nothing installed.** Next: V1 (static gap closing, no game).
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
| V1 | Close static gaps with Haiku recon (no game): vanilla script animation calls and flag meanings, UTC ItemList armed status for the pool, waypoint/walkmesh counts for candidate areas, quest-script tag scan, `+0x11AC` persistence decode and save-load path (REA) | TODO |
| V2 | Live probe design and build (needs user GO): OID stability across loads, hook-spawn persistence in saves, save-load reset, object count and frame cost | TODO |
| V3 | Pool and roster spec: group pools, area eligibility table, spawn budget, per-species walk speeds | TODO |
| V4 | Visit-salt change in doc 34 code (host tests, build only) and doc 34 section 3.2.2 update | TODO |
| V5 | Scripted-encounter feasibility: can the patch run or trigger scripts, effect/flag building blocks, K1 audio licence and format path | TODO |
| V6 | Animation randomization feasibility: script (flag 109 / local 29) versus native (`Gob::PlayAnimation 004e3270`) | TODO |
| V7 | Opus gate: choose build order, write the master plan, retire or merge the doc 35 routes | TODO |
