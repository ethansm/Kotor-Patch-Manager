# R3 enemy AI static research (2026-10-02)

Static only. Nothing in the game folder was written, launched or compiled; archives were copied to `/tmp/claude-1000/r3_scratch/mods/` and read there (own ERF/RIM reader plus the repo venv's pykotor `read_ncs` / `read_gff`, read-only). Scratch scripts are in `/tmp/claude-1000/r3_scratch/` (`reach.py`, `closure_run.py`, `allmod.py`, `bossflag2.py`, `styles.py`, `weapons.py`, `utcspells.py`, `slots.py`; outputs `*.json`/`*.pkl`).

Conventions: "verified" = quoted source or asm line (file:line or address). "Measured" = computed by the scratch scans (method in the appendix). Everything else is "plausible". Line numbers are those of `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/` (CR stripped) unless marked "vanilla" (`vanilla/` or `vanilla_all/`); the party patches shift vanilla lines by roughly +24 to +50.

## Summary + recommendation (routing vs recompile, with counts)

1. **The premise of ENEMY_AI_NOTES ("52 NPC scripts that embed the include must be recompiled or routed") does not hold for the steady state.** Enemies already run our `k_ai_master`.
   - Of 2,354 unique creature templates (2,085 in the 462 module archives plus 269 BIF-only), the combat-round slot (`ScriptEndRound`) holds `k_def_combend01` on 1,930 (82.0%) and `k_hen_combend01` on 118 (5.0%). Both are 2-line routers: `k_def_combend01.nss:13: ExecuteScript("k_ai_master", OBJECT_SELF, KOTOR_DEFAULT_EVENT_ON_COMBAT_ROUND_END);`. 303 (12.9%) have no script, 3 have a custom one. Heartbeat, attacked, damaged, perception and dialogue slots look the same (section "Inventory").
   - So an enemy gate flip is an edit to `k_inc_generic`/`k_inc_gensupport`/`k_ai_master` and one rebuild of `k_ai_master.ncs`. No per-NPC-script work.
2. **Only 18 of the 53 compiled scripts that embed include code contain the decision tree.** The compiler drops unused functions. Measured by NCS reachability (the `START DETERMINE COMBAT ROUND` string reachable from the entry point): `k_ai_master`, 13 `k_hen_<name>dlg`, `a_atkonend`, `a_force_combat`, `k_combat_rnd`, `k_def_rependd` = 18 (1.91 MB of BIF `.ncs`). The other 35 (433 KB) are spawn/userdef/action scripts that carry only helper functions: none of the 9 marker strings (DCR, shout, poison, weapon type, injury, knocked-out, Jedi Support, Boss, Kite) is reachable in any of them.
3. **Recommendation: neither a mass recompile (B) nor a mass route (A).** Use (C): gate flips inside `k_ai_master`'s include (already shipped), plus, if wanted, ONE extra router (`k_def_rependd`, 2.7 KB, vanilla source available). Numbers:

   | | A: route | B: recompile with patched include | C (recommended) |
   |---|---|---|---|
   | BIF scripts to touch for decision-tree parity | 1 more (16 already routed) | 18 (about +8 KB each, about 1.9-2.0 MB total) | 0 to 1 |
   | extra override bytes | +2.7 KB (16 routers today = 54,167 B measured) | about +2.0 MB | 0 to 2.7 KB |
   | module-local scripts embedding the tree | 51 names, 66 bodies, 120 archive copies in 34 areas, 5.3 MB: **0 sources** | same, 0 sources | not touched |
   | rollback | consistent in any combination | 17 files must roll back as a set | one file |

   `k_def_rependd` is an ALLY repair droid (it sets `STANDARD_FACTION_FRIENDLY_1` before its DCR, `k_def_rependd.nss:27-28`), so even it is not an enemy concern. The 51 module-local scripts (`a_*attack`, `a_*atk`, `tr_*`, spawn/userdef) are one-shot "turn hostile and start the fight" kick-offs: they run the embedded vanilla DCR for the FIRST decision only; every later round goes through `k_def_combend01 -> k_ai_master`.
4. **Enemy versions of the notes' list are mostly moot for stock content** (details below):
   - H1, M2, M3, M4: the enemy paths are dead code. No shipped script ever assigns NPC AI style 5 (Jedi Support) and no script ever sets `SW_FLAG_BOSS_AI`. Removing the party gate changes nothing.
   - M5: nothing to flip. The NPC branch of the poison check already works (it was never broken).
   - M7: nothing to flip for stock content: no creature template holds base items 104-107 in the right hand.
   - Real enemy-side differences exist only for **H2 (extension, not a gate flip)**, **E3** (two sites: `GN_RespondToShout` and `k_ai_master` case 1006) and **E11** (hostile `k_hen_*` creatures).
5. **E7 (`hold_enemies`)**: the existing hold hook can be reused unchanged (same window `0x6D83EA`); the design needs five guards beyond "`RangeWeaponEquipped` only" (not backoff, target is a party member, streak cap, enough per-creature slots, NPC-style exclusion) plus a script mirror, or enemies stall behind cover. See the section.
6. **Stage 0**: `log_all=1` alone is enough (`party_only=0` with `log_all=0` passes NOTHING, `CompanionAI.cpp:264`). Mid estimate 56 lines/s (about 0.4 MB/min) with 3 engaged enemies, 135 lines/s (1.0 MB/min) with 10, the 200 lines/s cap from about 20. Raise the cap for the session.

## Inventory table: script | includes k_inc_generic? | size | trigger | override/module copies | source available

Totals (measured): 52 raw `#include "k_inc_generic"` lines in the 638 vanilla sources (51 live; `k_oei_spawn.nss:8` is commented out but it includes `k_oei_hench_inc`, which includes it). Counting the include closure: 58 files = 5 include files (`k_inc_generic`, `k_inc_fakecombat`, `k_inc_zone`, `k_oei_hench_inc`, `k_inc_hawk`) + 53 compiled scripts. All 53 have a BIF `.ncs` and a `.nss` in `vanilla_all/`. The notes' "52" is the raw include count.

Columns: D = reaches `GN_DetermineCombatRound` in the BIF `.ncs` (reachability). OV = file in `$G/override/`. MOD = module archives (of 462) holding a same-named `.ncs`.

| script | include | .nss B / BIF .ncs B | D | trigger | OV | MOD | source |
|---|---|---|---|---|---|---|---|
| k_ai_master | direct | 43,140 / 121,637 | yes | every `k_def_*` (1001-1012) and `k_hen_*` (2001-2014) event, plus 3001-3003 | 131,658 (ours) | 0 | yes |
| k_hen_attondlg | direct | 4,338 / 106,407 | yes | OnDialogue slot of the companion templates (shouts via listen patterns, conversations) | 3,806 router | 0 | yes |
| k_hen_baodurdlg, g0t0dlg, hanharrdlg, hk47dlg, manddlg, miradlg, remotedlg, t3m4dlg | direct | 2.8-2.9 k / 106,033 | yes | same | 3,432 router each | 0 | yes |
| k_hen_discipdlg, hndmaiddlg, kreiadlg, visasdlg | direct | 3.1-3.6 k / 106.1-106.4 k | yes | same | 3,547-3,781 router | 0 | yes |
| a_atkonend | direct | 499 / 102,631 | yes | dialogue action (NPC attacks the speaker); `a_atkonend.nss:17 DelayCommand(0.3,GN_DetermineCombatRound())` | 2,786 router | 0 | yes |
| a_force_combat | direct | 174 / 102,556 | yes | dialogue/trigger action | 2,711 router | 0 | yes |
| k_combat_rnd | direct | 231 / 102,556 | yes | action script | 2,711 router | 0 | yes |
| k_def_rependd | direct | 924 / 103,325 | yes | OnEndDialogue of reactivated repair droids (becomes friendly first) | none | 0 | yes |
| 35 helper-only scripts (below) | direct or via | 0.2-11 k / 2.7-26 k | no | OnSpawn, OnUserDefined, dialogue actions, zone/galaxy-map scripts | see below | see below | yes |

The 35 helper-only scripts (all D = no, BIF sizes in bytes, trigger):
- OnSpawn: `k_def_spawn01` 25,462; `k_def_spn_t_{drd 25,636; empt 25,462; jedi 25,852; less 25,462; more 25,495; none 10,106; per 26,487}`; `k_def_ambient` 9,147; `k_def_ambmob` 9,147; `k_def_ambmobtrea` 25,459; `k_def_grenspn` 25,456 (sets NPC style 4); `k_def_repairsp` 4,618; `k_def_repairsptr` 21,104; `k_amb_prey_spawn` 9,147; `k_fmine_spawn` 10,308; `k_sithas_spawn01` 9,226; `k_hen_spawn01` 4,425; `k_oei_spawn` 16,950.
- OnUserDefined: `k_def_userdef01` 3,856; `k_def_repuser` 2,986; `k_amb_prey_ude` 3,864; `k_oei_userdef` 23,700.
- Actions/other: `a_master_atck` 3,126; `a_master_half` 3,552; `a_master_kill` 6,999; `a_master_setup` 6,505; `a_walkways` 7,523; `a_walkways2` 7,577; `k_act_com45` 4,831; `k_hen_leadchng` 3,160; `k_hen_retreat` 2,679; `k_sup_galaxymap` 10,117; `k_zon_catalog` 2,992; `k_zon_control` 4,954.
- Override copies of these (all from the 09-24 recompile/mods, none contains the tree): `k_def_spawn01` 27,281, `k_def_spn_t_{drd 27,455; empt 27,281; jedi 27,671; less 27,281; more 27,314; per 28,306}`, `k_def_ambmobtrea` 27,278, `k_def_grenspn` 27,275, `k_def_repairsptr` 22,923, `k_oei_userdef` 40,430 (Obscured Tomb), `k_sup_galaxymap` 11,217.
- Module copies of these names (none contains the tree): `k_def_ambmobtrea` (401DXN_s), `k_def_repairsp` (102PER_s + .mod), `k_def_repairsptr` (401DXN_s), `k_oei_userdef` (711kor.mod), `k_sithas_spawn01` (5 .mod). Plus `k_hen_combend01`/`k_hen_damage01` in `003ebo.mod` (1,360 / 914 B): they call `ExecuteScript("k_ai_master", ...)`, so they also reach our script.

**Module-local scripts that embed the decision tree** (not in vanilla; measured over 4,224 distinct module `.ncs` names): 51 names, 66 distinct bodies, 120 archive copies (61 `.mod` + 59 `.rim`) in 34 areas, 5.3 MB distinct (12.5 MB across copies). Source: none (the 462 archives hold 12 `.nss` in total, none of these). Types:
- dialogue/trigger kick-offs, 41 names: `a_902boss_cs a_atkonend_mech a_atrend1/2/3 a_bakkel_attack a_beast_train a_boma_attack a_bounty_attack a_camp_enemy a_circle_fight a_circlecut4 a_coward_stand a_dojo_fight a_drexl_act a_endgame a_exchange_h a_hand_fight a_kavar_atk a_make_hostile a_mala_act a_manatk_cs a_merc_attack a_revan_act a_riot_sc a_ritual_sc_2 a_royal_battle_1 a_scoutquest4 a_sentryatk a_tae_atk a_vaklu_attack a_zhugatk k_301_duroamb k_303_zattack k_506_kattack2 k_dialog_hk51cs tr_pc_foil tr_tukata_attack` (e.g. `a_merc_attack`: change faction, `ActionAttack`, embedded DCR).
- user-defined events, 8 names: `k_def_user_heal` (12 copies) `k_def_hkuserdef k_fab_drd_user k_fab_enheal_ud k_fab_healer_ud k_fab_prot_user k_mand_fade_ud k_fab_rependd`.
- spawn scripts, 4 names: `k_fab_sith_si` (7 copies), `k_fab_sithcs_si`, `k_sithspwn_151`, `k_sithspwn_152` (the Sith appear, turn hostile, run the embedded DCR).
- override (mod) copies with the tree: `m_sithspwn_152.ncs` 107,615 B and `m_spn_hssiss.ncs` 107,590 B.

**Direct callers of `GN_DetermineCombatRound` (non-comment, vanilla sources):** `k_ai_master` (22 call sites), `a_atkonend`, `a_force_combat`, `k_combat_rnd`, `k_def_rependd` (the 18 other grep hits, e.g. `k_def_damage01.nss`, are inside `/* */` blocks). The 13 `k_hen_<name>dlg` reach it through `GN_RespondToShout`. So the "16 other scripts" of the task statement correspond to the 13 dlg + 3 wrappers that are already routed; the only unrouted BIF caller is `k_def_rependd`.

**Creature-template slot census** (2,354 unique UTC: 2,085 from the module archives, 269 BIF-only; measured):

| slot | k_def_* router | k_hen_* router | empty | other |
|---|---|---|---|---|
| ScriptHeartbeat | 1,924 | 117 | 288 | 25 |
| ScriptOnNotice | 1,930 | 118 | 300 | 6 |
| ScriptEndRound | 1,930 | 118 | 303 | 3 |
| ScriptAttacked | 1,932 | 115 | 306 | 1 |
| ScriptDamaged | 1,925 | 114 | 304 | 11 |
| ScriptDialogue | 1,923 | 54 (+66 `k_hen_*dlg`) | 307 | 4 |

Verified routers (`vanilla_all/`): `k_def_heartbt01 / percept01 / combend01 / dialogue01 / attacked01 / damage01 / death01 / spellat01 / blocked01 / pathfail01 / endconv` and `k_hen_heartbt01 / percept01 / combend01 / dialogue01 / attacked01 / damage01 / blocked01` each contain one `ExecuteScript("k_ai_master", OBJECT_SELF, <event>)` (`k_def_attacked01.nss:13`, `k_def_damage01.nss:13`, `k_def_dialogue01.nss:13`, `k_def_heartbt01.nss:13`, `k_hen_combend01.nss:13`). `k_def_disturb01` and `k_hen_enter5m/exit5m` have none.

## Routing vs recompile analysis

**How the "byte-identical vanilla proof" works** (`build_party_ai_scripts.py`, docstring and `step_inputs` / `step_proofs`):
1. `step_inputs`: each source in `vanilla/` (+`vanilla_all/`) and each include of the recompile kit must equal the copy read straight from `chitin.key`/`scripts.bif` (own KEY/BIF reader); the BIF `.ncs` go to `vanilla_ncs/` with `MD5SUMS`.
2. `step_proofs`: `compile_ncs()` runs the kit's `compile.sh` (nwnnsscomp under Wine; success is judged by the output file only because nwnnsscomp exits 0 on errors) on the UNPATCHED source, then `open(out).read() == open(ref).read()` against the BIF `.ncs`. The 17 deliverables + 3 spares (`k_hen_leadchng`, `k_oei_endconv`, `k_oei_spawn`) all PASS (`build_party_ai_scripts.py` `OUTPUTS`/`SPARE`).
3. `step_baseline`: only the two Mod 8 insertions + kite v1 must reproduce the live `k_ai_master.ncs` md5 `94db8304`.
4. `step_patch`: each anchor must occur exactly once on the pristine text, spans must not overlap, no anchor inside another replacement (`lint_patches`).
5. The routers are compiled against the VANILLA includes (they contain no combat code); `k_ai_master` against the patched includes; then `check_ncs_arg_order.py --override-only` must report 0.

**Feasibility per option**
- Sources: 53/53 BIF scripts have vanilla sources (`vanilla_all/`), so proofs are possible for all of them. For the 51 module-local scripts: 0 sources, so neither routing nor recompiling can be proven. A decompile round trip would not be a byte-identical proof.
- Option B proof gap: a patched recompile can only be proven in two halves (vanilla proof + anchor lint); the combined output has no oracle. 17 outputs of about 110 KB each means 17 copies of one AI that must stay in lockstep (design doc section 5.2). Option A has one copy.
- Option A on `k_def_rependd`: same mechanism as the 3 wrappers (case 3001); its vanilla rebuild proof has NOT been run (it is not in `OUTPUTS`/`SPARE`). Expect PASS (same includes as `a_force_combat`; plausible), but it needs one proof run before any router.

**Shadowing risk (module copy vs override).** Same-named module copies exist for 5 closure names (listed above) and for none of: `k_ai_master`, the 17 DCR-bearing BIF names, the 16 router names, `k_def_rependd`, or any `k_def_*`/`k_hen_*` event script except the two 003ebo ones (which route to `k_ai_master` anyway). The engine's search order (override vs `.mod`/`.rim`) was NOT checked here (open question 1). It does not affect the recommendation: no name we override or would override has a module copy.

**Recommendation with numbers:** keep the 16 routers; do not recompile or route the 35 helper-only scripts (0 reachable tree, so no behaviour to change); leave the 51 module-local kick-offs vanilla (first decision of scripted fights only, no sources). If a router for `k_def_rependd` is ever wanted: +2.7 KB, one proof run, ally droids only.

## Enemy versions of H1/H2/M2/M3/M4/M5/M7/E3

### What the enemy dispatch actually reaches (shared by the rows below)

- Enemy DCR path: `GN_DetermineCombatRound` (`k_inc_generic.nss:268`) -> style dispatch lines 292-500. For a non-party creature `nNPC_AI` is whatever scripts set. Scanning all 612 BIF + 4,224 module script names for `SetNPCAIStyle` (action 707) with a literal style finds: 4 grenadier (`k_def_grenspn` and 7 module variants), 6 healer (`k_def_spawn_heal`, `k_fab_healer_si`), 7 skirmish (`k_def_spawn_skir`, Drexl), 8 turtle and 2 melee (`k_drexl_ud`), 15 monster-powers (`k_boss_spawn`, `k_def_amb_storm`), 13/14 party puppets (`give_puppet`, remotes). **Style 5 (`NPC_AISTYLE_JEDI_SUPPORT`) and 13 on an enemy are never assigned** (`vanilla_all/*.nss` grep: only `k_def_grenspn.nss:57` and `k_oei_hench_inc.nss:427` call `SetNPCAIStyle`). The UTC has no AI-style field (label census of 300 BIF UTCs). So enemies run style 0: `if(nNPC_AI == NPC_AISTYLE_DEFAULT_ATTACK || ...) GN_RunDefaultAIRoutine` (`k_inc_generic.nss:366-373`). Whether the engine's default for an unassigned NPC is 0 is plausible, not verified (Stage 0 prints `AI STYLE =` at `k_inc_generic.nss:286`).
- `GN_RunBossAIRoutine` runs only `if(GN_GetSpawnInCondition(SW_FLAG_BOSS_AI) == TRUE)` (`k_inc_generic.nss:341-343`). **`SW_FLAG_BOSS_AI` (=89, `vanilla k_inc_generic.nss:121`) is never set.** Evidence: `grep SW_FLAG_BOSS_AI|SW_FLAG_MALAK_AI_ON *.nss` over all 638 sources matches only `k_inc_generic.nss`; a compiled-script scan resolving the flag's global slot finds 0 `GN_SetSpawnInCondition(SW_FLAG_BOSS_AI, TRUE)` setters (the same scan finds 22 `SW_FLAG_AI_OFF` setters and 69 readers of the boss flag, which equals 18 BIF + 51 module scripts embedding the tree, so the method sees the flag); 0 literal `SetLocalBoolean(.., 89)`; 0 of 223 GIT creature lists have a `VarTable`. Residual: a setter that passes the flag through a variable would escape the scan (plausible, unlikely).
- Both the Jedi Support and the Boss routines are the only callers of `GN_FindAOETarget` (`k_inc_generic.nss:1079, 1179`), `GN_DetermineAttackTarget` (also `4333`, the party-only kite) and `GN_GetTargetedForcePower` (via `GN_GetBossCombatMove`, `k_inc_gensupport.nss:3078, 3110`).

Gate proposal (common to everything below, if any enemy fix is ever shipped): one function `GN_EnemyFixOn(int nBit)` in the include: `!IsObjectPartyMember(OBJECT_SELF)`, plus a compile-time mask `EAI_MASK` (default 0 = vanilla; zero runtime cost; rollback = swap `k_ai_master.ncs`), optionally OR-ed with `GetGlobalNumber("EAI_FIXES") & nBit` (a global the DLL or a console can set) and a difficulty clause `GetGameDifficulty() >= GAME_DIFFICULTY_NORMAL` (`nwscript.nss:4493`). Faction clause for "real enemies": `GetStandardFaction(OBJECT_SELF)` in {`STANDARD_FACTION_HOSTILE_1`=1, `HOSTILE_2`=3, `INSANE`=6} (`nwscript.nss:1020-1025`), because 1,260 of 2,354 templates are NEUTRAL by template and become hostile through `ChangeToStandardFaction` in the kick-off scripts (so a faction test misses scripted conversions; a `GetIsEnemy(GetFirstPC())` test does not).

### H1 Revitalize idle (`GN_CheckSeriesRevitalize`/`GN_CheckIfKnockedOut`)
- **Change when the gate is removed:** `k_inc_generic.nss:1047-1050` (`H1a`) and `2606-2615` (`H1b`) drop `IsObjectPartyMember`. An NPC Jedi Support would no longer idle while a PARTY member is down (`GN_CheckIfKnockedOut`, `:2424-2437`, loops `GetPartyMemberByIndex(0..2)`).
- **Enemy path:** only `GN_RunJediSupportAIRoutine` (`:1017`), reached for style 5 (`:380-387`) or 13 (`:457-465`). Nobody assigns those to an enemy (above). Even the power is impossible for enemies: `k_inc_force.nss:5016-5019 "Make sure the caster is in the party. If they're not we can't really do anything with this power since enemies don't get knocked unconscious, they die."` and 0 of 2,354 templates know Revitalize (spell 173) in `KnownList0`.
- **Balance impact:** none on stock content (verified dead). The correct enemy form, if a mod ever assigns style 5, is to make `GN_CheckIfKnockedOut` return FALSE for non-party callers (enemies must never idle for the player's knockouts).
- **Gate:** none needed.

### H2 injury check (`GN_CheckIfInjured`)
- **What differs:** the bug is in the PARTY branch only (`k_inc_generic.nss:2366-2407`, all three tests used `fDown00` in vanilla). The NPC branch is correct for itself: `:2410-2416` `fNPC < 0.5 -> return OBJECT_SELF`.
- **Enemy path (verified):** `GN_RunDefaultAIRoutine` (`:629`) begins with the poison check and then `oTarget = GN_CheckIfInjured(); if valid -> GN_TalentMasterRoutine(GEN_TALENT_HEALING, oTarget)` (`:642-646`, after the poison check at `:637-640`), so an enemy with a heal power already heals itself below 50%. And the heal powers heal friends too: `k_inc_force.nss:2324-2349` (Heal) and `:2389-2411` (Master Heal): for a non-party caster `oParty = OBJECT_SELF`, then `GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_FRIEND, OBJECT_SELF, nCnt, CREATURE_TYPE_RACIAL_TYPE, RACIAL_TYPE_HUMAN)`: self plus the 2 nearest HUMAN friends within 15 m (`GetDistanceBetween < 15.0`). So "an enemy healer never heals its allies" is wrong for humans near a wounded caster; it is true when the caster itself is above 50%.
- **Enemy version = extension, not a gate flip:** in the NPC branch also scan friends (`GetNearestCreature(REPUTATION_TYPE_FRIEND, ..., 1..3)`, within 15 m, not droid, `HP/Max < 0.5`) and return the first. The cast itself stays an area self-cast. About 30 lines, one new patch.
- **Reach (measured):** templates knowing Heal (28): 34 total, 23 with a hostile standard faction; Master Heal (134): 13 / 9; Cure (10): 9 / 0. Healer style 6 (`k_def_spawn_heal`, `k_fab_healer_si`) uses its own routine.
- **Balance:** more enemy healing in group fights with a healer caster (a few dozen templates); low-medium. Gate bit `EAI_H2`; faction/difficulty clause optional.

### M2 area power centred on an ally (`GN_FindAOETarget`)
- **Change:** `k_inc_gensupport.nss:2545-2552` (`if(!IsObjectPartyMember(OBJECT_SELF) || (GetIsEnemy(oCheck) && !GetIsDead(oCheck) && GetObjectSeen(oCheck)))`): remove the `!IsObjectPartyMember ||` term so non-party callers also require a live, seen enemy centre.
- **Enemy path:** `GN_RunBossAOEPowerRoutine` (`k_inc_generic.nss:1175-1200`, Boss AI only) and `GN_RunJediSupportAIRoutine` (`:1079`). Both unreachable on stock content (above). Related dead bug in the same function: `nDroid == TRUE;` at `k_inc_generic.nss:1148` and `1186` is a comparison, not an assignment.
- **Pool if it were reachable (measured):** hostile-faction templates knowing Storm 42, Wave 26, Insanity 28, Whirlwind 32.
- **Balance:** the fix makes boss AOE rarer but fewer rounds wasted on non-enemy centres; plausible net zero. **Verdict:** moot until a boss flag exists.

### M3 inverted debilitated-target search (`GN_DetermineAttackTarget`)
- **Change:** `k_inc_generic.nss:3527-3534` (`nPartyM3`): drop the party term so NPCs also skip debilitated enemies.
- **Enemy path:** `GN_DetermineAttackTarget` (`:3498`) has two callers: Jedi Support (`:1096`, dead) and the kite routine (`:4333`; `GN_RunKiteAIRoutine` sends non-party callers to the default routine at `:4299-4302`, so it never reaches it for enemies). The branch itself keys on `GetPartyAIStyle()`, the player's party style (`:3521 nPartyAI == PARTY_AISTYLE_AGGRESSIVE`). So it is unreachable for enemies on stock content.
- **What enemies really do** (default routine, `:726-760`): oTarget = last hostile target; the intruder if seen; otherwise the nearest seen enemy replaces the target when `GetCurrentHitPoints(oTarget) < GetCurrentHitPoints(oClose)` (`:748`), i.e. an enemy swaps from a wounded target to a healthier nearer one (anti-focus-fire), with no debilitation test at all. A debilitation-aware or focus-fire retarget is a NEW feature (see "Other candidates").
- **Balance / gate:** none for M3 itself.

### M4 shadowed `nDroid` (`GN_GetTargetedForcePower`)
- **Change:** `k_inc_gensupport.nss:3394-3410` (`M4a`): remove the `if(!IsObjectPartyMember(OBJECT_SELF)) nDroid = FALSE;` shim so NPCs use the real droid flag and `Droid Disable`.
- **Enemy path:** `GN_GetBossCombatMove` (`k_inc_gensupport.nss:3051`), called from Boss AI and Jedi Support only (`k_inc_generic.nss:1085-1109, 1150, 1188, 1265`). Dead on stock content. Enemy Force users in the default routine pick powers through the combo tables (`GN_GetComboMove` -> `GN_GetStandardJediCombo`, `k_inc_generic.nss:2931+`) and the droid filter `GN_CheckNonDroidForcePower` (defined `k_inc_generic.nss:3175`, called at `:901`), which is not shadowed.
- **Pool if reachable (measured):** hostile templates with Droid Disable (12): 3; with Choke 50, Lightning 50, Plague 2, Affliction 3. Party droids (HK-47, T3-M4, G0-T0) are the intended beneficiaries.
- **Balance:** enemies would stop wasting organic-only powers on droid party members: modest buff against droid party members, only if Boss/Jedi Support is ever enabled for an enemy. **Verdict:** moot.

### M5 poison cure never runs (`GN_CheckIfPoisoned`)
- **Enemy path (verified):** the broken loop is in the PARTY branch only. The NPC branch (`k_inc_generic.nss:2327-2337`, vanilla `:2256-2268` is the broken party loop; the NPC `else` follows it) walks `GetFirstEffect(OBJECT_SELF)` for `EFFECT_TYPE_POISON` and returns `OBJECT_SELF`; it works. `GN_RunDefaultAIRoutine` calls it first (`:640-643`). The NPC antidote branch exists too (`k_inc_generic.nss:2501 else if(!IsObjectPartyMember(OBJECT_SELF) && GetCreatureHasTalent(tSpe11_03) && oTarget == OBJECT_SELF)`).
- **Change when the gate is removed:** none. M5/M5b only touch the party loop and remove Cure from the party selection; the Cure-never-removes-poison fact (B1) is also true for enemies: Cure (9 templates, 0 hostile) would not cure them.
- **Verdict:** no enemy version needed. The notes' "M5: the poison cure never runs" is a party-only bug.

### M7 mining lasers count as melee (`GN_GetWeaponType*`)
- **Change:** `k_inc_gensupport.nss:2245-2257` (`M7b`) and `:2303-2310` (`M7a`): drop `IsObjectPartyMember` so base items 104/105 are ranged and 106/107 melee for every caller.
- **Enemy path (active):** `GN_GetWeaponType` feeds `GN_GetStandardNPCCombo` (`k_inc_gensupport.nss:665-690`), the series melee/ranged feat pickers (`:1389`, `:1419`), `GN_EquipAppropriateWeapon` (`:2338`, `:2373`) and `GN_GetBossCombatMove` (`:3091`).
- **Reach (measured, 1,282 right-hand weapons on 2,354 templates):** base items 104-107 appear on 0 templates (hostile subset also 0). Right-hand weapon classes by the script's own tables: ranged 683 (243 on hostile-faction templates), melee 473 (179), unresolved/other 126 (53; the item blueprints are not in the scanned BIF/modules/override; mostly stock `g_w_*` items by name). The notes' "Peragus mining-droid behaviour" has no template evidence (script-equipped weapons were not scanned).
- **Balance:** none on stock content. If it ever mattered: a mining-laser holder stops being "type 0" (neither melee nor ranged) in the weapon-swap and combo code.
- **Gate:** none needed; keep party-only.

### E3 run to 2 m of an unseen intruder (`GN_RespondToShout` and `k_ai_master` case 1006)
- **Change:** two sites, not one. (1) `GN_RespondToShout`: `k_inc_generic.nss:2093-2101`: the `if(IsObjectPartyMember(OBJECT_SELF)) GN_PartyApproachUnseen(oIntruder); else if(GetDistanceBetween(OBJECT_SELF, oIntruder) < 20.0 && !GetObjectSeen(oIntruder)) ActionMoveToObject(oIntruder, TRUE, 2.0);`. Enemy version = call `GN_PartyApproachUnseen` (`:4465-4480`: 4 m melee / 15 m otherwise, then `ActionDoCommand(GN_DetermineCombatRound(oIntruder))`) for non-party callers, minus the Ranged/Stationary/Kite "holds" branch. (2) the same pattern for a damager the creature cannot see in `k_ai_master.nss:316-334` (case 1006): `ActionMoveToObject(oTarget, TRUE, 2.0)` when `< 20.0 && !GetObjectSeen`; the party patch E11 only covers case 2006 (`:785`).
- **Why it is a run to 2 m for everyone** (comment above `GN_PartyApproachUnseen`, `:4460-4464`): the branch is reached only when `GetObjectSeen(oIntruder)` already failed, so `!GetObjectSeen` is always true and the 4/15 m else-branch is only used beyond 20 m.
- **Enemy reach:** every enemy with listen patterns (`GN_SetListeningPatterns` in the spawn scripts) that hears `GEN_I_WAS_ATTACKED` from a friend while the hostile actor is unseen, plus any enemy damaged by an unseen attacker. Rate unknown until Stage 0 (`Shout Clear 1900` lines for non-party objects).
- **Balance impact:** enemies stop sprinting to 2 m of unseen (stealthed or around-the-corner) party members; ranged enemies stop at 15 m, melee at 4 m, then re-plan. Plausible effect: stealth and ambush play gets safer for the player, ranged enemies hold range longer. Pair it with the perception throttle P1 (`p1=0` today) as one "stealth pass" so aggro timing and rush behaviour change together.
- **Gate:** bit `EAI_E3`; default OFF; test with `Shout Clear 1900` + `Party E3`-style print before/after.

### Other enemy-side candidates found while reading the paths (not in the lead's list; all verified in source)
1. **E11 for hostile hench creatures** (`k_ai_master.nss:785`, case 2006): HK-50/51, `n_trnremote`, the 262tel sisters, `p_kreia_evil001` use `k_hen_*` scripts (review S3). Currently party-gated.
2. **`GetNPCAIStyle(OBJECT_SELF) != SW_FLAG_BOSS_AI`** (`k_inc_generic.nss:711`, `:869`) compares an AI style (0-15) with a spawn-flag constant (89): the "bosses more aggressive" targeting and the 30 m drop-out exemption can never apply. Moot while the boss flag is unset.
3. **Target swap by HP** (`:748`) and no debilitation test in the default target choice (above).
4. **Local 9 collision**: the droid reposition loop counts with local 9 (`:807, 845, 890`) which is also `SW_NUMBER_HEALERAI_PERCENTAGE` (R-E result in doc 13).

## Mod 8 E7 hold_enemies design (parity, droid loop 734-831, script mirror, engine design input)

### Current gates (verified)
`RangedAI.cpp:179-197`: `party1 = party != 0; if (!party1 && !g_cfg.logAll) return 0;` then
`hold = g_cfg.hold && party1 && !isPC && stance in holdMask && ranged && los == 0 && inRange && myArea && myArea == tgtArea && ttype == ObjTypeCreature`. The hook is E-H1 at `0x006D83EA` (stolen bytes `8B 91 2C 05 00 00 89 95 8C FC FF FF`, consumed exit `0x006D83FF`, EAX-free window). Asm context (`decomp/6d/006d7390.asm`): `006d8366 CALL 0x0056e1e0 / MOV [EBP-0x164],EAX` (ranged), `006d8371 MOV [EBP-0x168],0` then `006d83da MOV [EBP-0x168],0x1` (back-off flag set when the too-close/geometry test passes), `006d83ea MOV EDX,[ECX+0x52c]` / `006d83f6 CMP ...,0` / `006d83ff PUSH 1; CALL 0x006dab20` (the end-attack block).

E7 as asked: drop `party1`, `isPC` (always 0 for NPCs), and the stance mask; keep `ranged` (`RangeWeaponEquipped`).

### Parity argument: why a hold is safe for any ranged creature, and where it is not
Safe (verified unless marked):
1. **The hook is reached only on the approach path.** `006d7390.c`: the whole approach block is entered only if `area differs || (max+0.1)^2 < dist^2 || (dist^2 < desired^2-c && !party) || los == 0 || geometry`; E-H1 sits after it (doc 13 review: "E-H1 is only reached on the approach path"). A hold therefore only REPLACES a walk the engine was about to queue; it never adds a move, an attack or a target change.
2. **Far targets are never held.** The hold needs `inRange` (`dist2 <= (max+0.1)^2`, `maxR > 1.0`). Enemies still close to weapon range exactly as in vanilla. The held state is "in weapon range, `ClearLineOfSight == 0`", where vanilla walks toward the target until the line clears or the creature reaches melee: the vanilla "ranged enemy walks into your face" case.
3. **`RangeWeaponEquipped` is data-driven.** `0x56E1E0` (`decomp/56/0056e1e0.c`): `FUN_006d0620(0x10)` (item in slot 0x10) then `*(char*)(FUN_006d5200(baseitem)+0x1a) != 0`. It reads the baseitems row flag of the right-hand weapon, not the script's hard-coded list, so M7's missing base items are irrelevant here, melee weapons never hold, and creatures whose only weapons are creature-weapon slots (beasts, many droids: slots 16384/32768/65536 in the UTC census) never hold (plausible mapping of the slot constants). 683 templates (243 hostile-faction) hold a ranged right-hand item.
4. **Same area and creature-target conditions** keep the door/placeable and area-transition corner cases vanilla (`myArea == tgtArea`, `ttype == 5`).
5. **Terminal behaviour is the engine's own.** The hold exits through the block the engine takes when `+0x52C != 0` (`EndAttackAction(1)`, returns "complete"). For a living enemy the 3 s fallback still re-runs the end-of-round script: `AIUpdate` fires slot 6 when `+0x520==1`, `+0x570<=0` and `vt+0x94()==0`; `vt+0x94 = 0x563460` returns `0` if party else `0x5477F0`, which returns `HP < 1` (`decomp/56/00563460.c`, `decomp/54/005477f0.c`), i.e. 0 for a living NPC. So a held enemy is re-evaluated at least every 3 s, and on `OnAttacked`/`OnDamaged` events when its queue is empty (`k_ai_master.nss:266-293`).

Not safe as written (each needs a guard, listed in the engine section):
- **Stalemate / turtle.** A blocked-LOS enemy now stands and re-issues the same attack every 3 s. A player behind a pillar is never approached: vanilla's walk-to-clear-LOS is exactly what resolves such blocks. Needs a streak cap or the script mirror.
- **Back-off overlap.** `[ebp-0x168]` = 1 (too close) also reaches the hook; vanilla then steps back. A blocked point-blank hold would suppress the step. Guard with `backoff == 0`.
- **Friendly/neutral ranged NPCs** (Mandalorian allies, arena NPC-vs-NPC fights, cutscene fights) are non-party creatures too. Guard: target must be a party member (`+0x11AC != 0` on the target) so NPC-vs-NPC stays vanilla.
- **Scripted movers.** NPC styles 7 (skirmish) and 8 (turtle) drive their own `ActionMoveToLocation` rounds; exclude styles other than 0 by default.
- **Does `ClearLineOfSight` treat creatures as blockers?** Unresolved (doc 13 M8-E3). If not, enemy `los==0` happens only for static geometry and E7 mostly matters at pillars/doors.
- **Track table.** `Track g_tr[8]` + `trackFor` collision reset (`RangedAI.cpp:138-146, 181-183`) cannot hold more than 8 distinct attackers; with `log_all=1` and many enemies each collision resets the throttle and the 6,000-line cap (`:63`, about 280 B per H1 line, about 1.7 MB) is burned in minutes.

### Droid reposition loop (`k_inc_generic.nss` vanilla 729-831 = src_party_ai 753-855; local 9)
What it is: the branch `else if (GetRacialType(OBJECT_SELF) == RACIAL_TYPE_DROID && !IsObjectPartyMember(OBJECT_SELF))` of `GN_RunDefaultAIRoutine`, taken only when there is no valid seen intruder and no seen `oClose` (vanilla 725-728). Comment at vanilla 731-739: "ranged-only users will try to chase down the characters ... HasLineOfSight ... works about 60%". Behaviour:
- `oClose` = nearest enemy WITHOUT the seen requirement (vanilla 748-752), dropped if `IsStealthed` (767).
- local 9 = number of repositions so far. While `<= 2`: pick `GetRandomDestination(OBJECT_SELF, 10)`, up to 20 tries for a point with `HasLineOfSight(vNewPos, vTargetPos)` (foot vectors, no objects, vanilla 793-808); if found and not already `ACTION_MOVETOPOINT`, `ActionMoveToLocation(.., TRUE)` and `SetLocalNumber(9, +1)` (821-822). Then `oClose = oTarget = OBJECT_INVALID`: the DCR returns FALSE this round.
- local 9 `> 2` (783): "Going straight there now": `ActionMoveToLocation(target position)`, local 9 NOT reset (it is reset only when a later DCR has a valid target, vanilla 849).
Interaction with the hold:
- The loop runs when the target is UNSEEN; the hold acts when the target is SEEN but the engine's `ClearLineOfSight == 0` (the two tests differ: perception sight vs the attack action's LOS with eye heights; the 60% remark is the same mismatch). In the seen-but-blocked window the loop is not entered, so for a droid with an equipped blaster (e.g. `c_drdastro001`: `w_blaste_03` in slot 16) the sequence with E7 is: hold (stand) -> perception drops the target -> loop repositions up to 3 times (each a script move that the hold does not touch) -> rush. That is a sensible fallback chain. Without E7, the engine walks it at the target throughout.
- Non-droid ranged humans have NO loop (the droid test, vanilla 729): hold would leave them standing until the target moves or the 3 s fallback re-issues the same attack. They need the mirror.
- The loop's local 9 is shared with `SW_NUMBER_HEALERAI_PERCENTAGE`; the mirror must use its own index.

### Script mirror for non-party ranged creatures
Goal: when the engine would have walked a blocked shooter forward and E7 holds it instead, give it a bounded way to get a clean line, then fall back to the vanilla approach.
- **Where:** `GN_RunDefaultAIRoutine`, just before the final `ActionAttack(oTarget)` (`k_inc_generic.nss:918-925`) for `!IsObjectPartyMember(OBJECT_SELF)` and `GN_EnemyFixOn(EAI_E7)`.
- **Conditions (all):** right-hand weapon type == 2 via `GN_GetWeaponTypeFromBaseItem` (or `GN_GetWeaponType`), `GetIsEnemy(oTarget)`, `GetObjectSeen(oTarget)`, `GetDistanceToObject(oTarget) <= 15`, `!HasLineOfSight(eye(me), eye(tgt), OBJECT_SELF, oTarget)` with both z+1.5 m eyes and both objects (doc 13 M8-SC2: matches the engine test within 0.01 m, unlike the droid loop's foot vectors).
- **Action:** up to 12 candidates `GetRandomDestination(OBJECT_SELF, 8)`; keep a candidate that has the clean line, is within 15 m of the target and at least 2.5 m from the nearest enemy; `ClearAllActions(); ActionMoveToLocation(loc, TRUE); ActionAttack(oTarget);` then counter +1. Same shape as the kite routine's step 3 and `GN_RunSkirmishAIRoutine` (`:1695`).
- **Counter:** its own local number: **30** (never used as a literal index by any of 612 BIF + 4,224 module scripts; 17 and 19 are free as literals too but mod120 For Mandalore uses 16-22 on summoned Mandalorians; named constants occupy 3-11, 20/21 are the kite, 29 walkways; engine range 0-0x1F, values 0-255). Cap `EAI_MAX_REPOS = 2`; when exhausted fall through to the vanilla `ActionAttack` and let the engine streak cap lapse (below), so the creature eventually approaches like vanilla.
- **Reset:** set to 0 whenever the shot is clear or the target changes, on every DCR (engine zeroes locals for party members at combat exit only, not for NPCs).
- **Optional E7c:** relax the vanilla droid condition to `(droid || ranged right-hand weapon)` so human shooters also get the existing unseen-target loop. A behaviour change on its own; keep separate.
- **Parity of the mirror:** with a clean line it is skipped (identical to vanilla); with a blocked line the worst case is two repositions, then vanilla. Script/engine mismatch (script says blocked, engine says clear) costs at most two unneeded steps.
- **Log-only variant first:** add one `GN_MyPrintString("... E7 mirror would fire ...")` and no action; Stage 0 then gives the frequency per enemy with zero behaviour change.

### Engine design input (for a later Opus pass; no asm here)
- Reuse the E-H1 window and consumed exit unchanged (no new hook, no new `check_windows` work). Change only the decision in `RangedAiHold`:
  `enemyHold = g_cfg.holdEnemies && !party && !isPC && styleAllowed(raw stance) && ranged && los == 0 && inRange && !backoff && sameArea && ttype == creature && targetIsParty && streak < cap`, with `backoff = rd<int>(ebp, F_BackoffFlag)`.
- New INI keys (hot reload, default 0 = vanilla): `hold_enemies`, `hold_enemy_styles` (default only style 0), `hold_enemy_streak` (max consecutive holds per attacker+target before the walk is allowed, suggested 2-3), `hold_enemy_reset_ms` (reset after the line clears or that long without a hold), `hold_enemy_min_hp` (optional: wounded enemies may come forward).
- Per-creature state: replace `Track g_tr[8]` by 64 slots with LRU eviction (the 8-slot collision reset defeats both the log throttle and any streak counter).
- Do not extend E-H2 (IsPC retarget) to enemies in the same step; it is a separate behaviour (enemies never retarget on a block).
- Stage 0 prerequisites: `ranged_ai.ini log_all=1` (log only; `RangedAI.cpp:180` continues past the party test and `hold` stays false for NPCs).

## Stage 0 log_all session plan + volume estimate

### Keys that exist (`companion_ai_src/companion_ai.ini`, `CompanionAI.cpp`)
- Trace: `trace`, `scripts`, `engine_points`, `trace_scripts` (default `k_ai_master,k_hen_*dlg,a_atkonend,a_force_combat,k_combat_rnd`), `shipbuild_scope` (`ai`/`all`/`off`), `prints_all_scripts`, `party_only`, `tags`, `log_all`, `engine_print`, `log_aur`, `dedupe_ms`, `max_lines_per_sec`, `max_file_mb`, `flush_ms`, `a1_burst_threshold`, `a1_rate_threshold`, `engine_verbose`, `summary_sec`, `p1_detail_lines`. Fixes (each own flag, 0 = vanilla): `a1`, `a1_stances`, `a1_party_only`, `a1_retarget`, `a1_retarget_stances`, `a1_rethink_ms`, `p1`, `p1_interval_ms`, `fm1`, `fm1_rethink_ms`, `fm4`, `a4`, `c1`, `c1_party_only`, `a3`, `log`.
- Object filter (`CompanionAI.cpp:263-265`): `objPasses = logAll || (partyOnly && creature && party) || tags.match`. Creature filter for the engine points (`:267-270`) is the same. **`log_all=1` alone passes everything; `party_only=0` with `log_all=0` and empty `tags` passes nothing** (not even the party). `E1`/`E1b` additionally skip non-party creatures unless `logAll` or `tags` (`:503`, `:519`).
- Throttling and caps (`CompanionAiLogic.h:303-339`, `LogGate::admit`): file cap `max_file_mb` (one marker, then writes stop, counters continue; hot-raising resumes); dedupe collapses only a line identical to the immediately preceding one (same object + script + text) within `dedupe_ms` into one `R ... repeated Nx` line; global per-second cap `max_lines_per_sec` for trace lines (`tline`), dropped lines counted and one marker per second; fix events, summaries and config (`eline`, force) are exempt from the per-second cap but not the file cap. E2 FALLBACK: first 20 lines per creature, then one per 10 s (`CompanionAI.cpp:543`). E1 SPIN/END and E3 aggregates are one line per event/summary. `S` summary every `summary_sec`.
- Where it goes: `<game dir>/ai_trace.txt`, truncated per session, buffered, flushed at most every `flush_ms` (events flush at once). `t=` is `GetTickCount()`, the same clock as `ranged_ai_log.txt`. Mod 8's own `ranged_ai.ini` has `log=1`/`log_all=0` and writes `ranged_ai_log.txt`.
- Capacity limits: `CrTrack g_tr[32]` (E1 tracks, non-party evicted first) and `ObjInfo g_obj[32]` (1 s cache): with more than 32 distinct enemies the object cache thrashes (a cost, not a loss).

### Recommended session INI (`$G/companion_ai.ini`, hot reload, no behaviour change)
```
log_all=1            ; party_only is then irrelevant; leave 1
max_lines_per_sec=600
max_file_mb=128
engine_points=1
engine_verbose=0     ; 1 only for a short fight with <= 10 creatures (adds one E1 RATE line per creature per second)
summary_sec=10
p1=0                 ; observe the E3 perception rate for real enemy counts; do not change behaviour
a1=1 c1=1 ...        ; unchanged (party-only fixes stay as installed)
trace_scripts=k_ai_master,k_hen_*dlg,a_atkonend,a_force_combat,k_combat_rnd,a_*attack*,a_*atk*,a_atrend*,a_merc_attack,k_def_user_heal,k_fab_sith*,k_sithspwn_*
```
(`trace_scripts` additions are optional: they flip `ShipBuild` for the 51 module-local kick-off scripts so their first-round prints appear; keep `prints_all_scripts=0` to avoid area-script noise.) For the E7 question run `ranged_ai.ini log_all=1` only in a SHORT fight with at most about 8 enemies (see the track-table caveat) or after the track table is enlarged.

### Volume estimate
Line sizes, from the format strings (design section 6): P/A line prefix about 63 B (`CompanionAI.cpp:474`, `:490`; `t=` 12, ` P ` 3, id 8, tag about 12, script 11, ` ev=` 8, ` d=` 4, ` | ` 3) + text. The 200 live debug prints in the three sources average 53.5 chars (median 52, max 84, measured), so about 125-140 B per line; I use **130 B**. E2 line about 117 B, E1 SPIN about 250 B, `S` line about 420 B, ranged-ai H1 line about 280 B.

Model: per engaged creature, DCRs per second r = 0.45 (0.33 = one per 3 s round end, 1.0 with attacked/damaged re-plans), prints per DCR L = 25 (15-40; design section 7 counts 15-40 and the heaviest functions). Heartbeats print nothing (`k_ai_master.nss` case 1001 has no print), so idle NPCs cost about 0. Creatures counted = enemies + 2 companions.

| engaged enemies | lines/s mid (low-high) | KB/s mid | MB/min mid | minutes to 64 MB (mid) |
|---|---|---|---|---|
| 0 (2 companions) | 22 (10-80) | 2.8 | 0.17 | 380 |
| 3 | 56 (25-200 cap) | 7.1 | 0.42 | 150 |
| 6 | 90 (40-200 cap) | 11.4 | 0.67 | 96 |
| 10 | 135 (59-200 cap) | 17.1 | 1.00 | 64 |
| 20 | 200 cap (109-200 cap) | 25.4 | 1.49 | 43 |

Per extra enemy: about 11 lines/s (5-40) = 1.4 KB/s = 86 KB/min while engaged. At the default 200 lines/s the cap engages from about 20 engaged enemies (earlier at the high rate); the dropped lines include party lines (the cap is global), hence `max_lines_per_sec=600`. At 600 lines/s the 128 MB cap lasts about 27 minutes of saturated combat; a typical session (20 fights of 40 s with 5 enemies) is about 60,000 lines, about 8 MB. Dedupe will remove almost nothing with several creatures interleaved. E2 adds at most 20 lines per enemy (about 2.3 KB); E3 aggregates one line per 10 s plus 20 detail lines per session; `A` lines can burst on path-blocked crowds (`k_ai_master.nss:483-484` posts two per OnPathBlocked, up to about 2 x FPS lines/s per blocked creature before dedupe), so watch the `S` line's `dropped=`/`deduped=` counters and set `log_aur=0` if they run away. These rates are a MODEL; the first real combat trace calibrates r and L (lines per `START DETERMINE COMBAT ROUND`).

### What the session should measure
1. **Routing/coverage:** counts of `ev=` per script for non-party objects; confirms enemies run `k_ai_master` (1001-1012) and how many hostile `k_hen_*` creatures run 2001-2014.
2. **NPC AI styles:** histogram of `AI STYLE = ` (`k_inc_generic.nss:286`) per non-party object. Expect style 0 almost only (plus 4/6/7/8/15 in specific fights). Any 5/13 would revive H1/M2/M3/M4.
3. **Dead-code checks (expect zero):** non-party lines containing `Boss AI Start`, `Boss AI: AOE Function Starting`, `Jedi Support AI: AOE Target`, `Starting: Determine Attack Target`, `Boss AI: Targeted Force Power Talent Chosen`, `Someone is knocked out.` (A line).
4. **DCR cadence:** DCRs per second per enemy and lines per DCR (calibrate the volume model); E2 FALLBACK counts per enemy (how often the 3 s fallback, not an event, triggers the round).
5. **E3:** count and time of `Shout Clear 1900` for non-party objects (the 2 m rush), and of `Determine Combat Round from On Damaged` (case 1006 site) with an unseen damager; note stealth/around-corner scenarios on a clock.
6. **H2:** non-party `I have Heal, Improved Heal, or Master Heal` + `Clear 2300` with the caster's own HP above/below 50%, and whether allies were healed (combat log).
7. **M7 / weapons:** non-party `Return No Weapon Type` (type-0 holders), `Switching Weapons`, `Equipping Melee Weapon` counts.
8. **E7 evidence (with `ranged_ai.ini log_all=1`, short fight):** per enemy H1 lines with `los=0 ranged=1 inRange=1 backoff=0` and their duration, whether `lastBlocked` equals `me` on consecutive calls (stuck position), and whether `los=0` ever occurs in open ground with an ally on the line (creatures as blockers, M8-E3). Companion-ai E1/E1b: spin counts for non-party attackers.
9. **P1:** E3 `calls/s`, `creatures=`, `elapsed>=4000` share with real enemy counts (the cost side of enabling the perception fix).
10. **Droid loop:** not printed in vanilla (the `AurPostString` calls are commented out). To measure it, add `GN_MyPrintString` lines in the droid branch in the next `k_ai_master` build (non-behavioural), or infer from `Default AI has failed to do an action` (`:928`) on droids.

Suggested protocol (user notes the clock time of each): (a) a fight with blaster soldiers near pillars/doors (hold relevance); (b) a droid-with-blaster fight (loop); (c) an approach in stealth or around a corner while allies shout (E3); (d) a Force-user enemy fight (styles, combos); (e) a fight with 8-10 enemies for the volume numbers.

## Open questions
1. **Resource search order, override vs a same-named script inside a `.mod`/`.rim`.** Not checked (no engine evidence read). It does not affect the recommendation (none of the names we override has a module copy), but decides whether the 51 module-local kick-offs could ever be replaced from `override/`.
2. **Does `ClearLineOfSight` (`0x52BB60`) count creatures as blockers?** Unresolved since doc 13 M8-E3. If not, E7's hold only matters at static geometry; Stage 0 `los=0` in open ground with allies in line answers it.
3. **Engine default NPC AI style** for a creature no script touched: assumed 0, not verified (`AI STYLE =` lines in Stage 0).
4. **Boss flag set through a variable** (not a literal) would escape the compiled-script scan; and mods (e.g. a "KotorII Improved AI" replacer) may set styles/flags. Stage 0 item 3 settles it for the installed content.
5. **Script-equipped weapons.** The M7 census used UTC `Equip_ItemList` only; weapons given by scripts (`CreateItemOnObject` + `ActionEquipItem`) were not scanned. 126 right-hand items could not be classified (blueprints absent from the scanned sources).
6. **`k_def_rependd` vanilla rebuild proof** has not been run (only needed if its router is wanted; ally droids only).
7. **Does the hold apply to talent/feat attacks** (`ActionUseTalentOnObject` goes through `AIActionCombat`/`0x6D9xxx`, not `AIActionPhysicalAttacks`)? If yes, Sniper-Shot-style enemy feats bypass E7; if no, E7 only affects plain attacks. Plausible: bypass. Check the E1/E1b counts for enemies in Stage 0.
8. **Is `+0x520 == 1` (combat flag) set for every engaged NPC**, so the 3 s fallback fires for held enemies? The E2 lines in Stage 0 show it.
9. **Faction census caveat:** hostile counts use template `FactionID in {1,3,6}` (657 templates); creatures converted by scripts are not in it. Use `GetIsEnemy(GetFirstPC())` in any real gate.
10. **Enemy heal extension (H2) value** depends on how many encounters pair a healer caster with wounded allies; unknown until Stage 0 item 6.

## Appendix: methods (so the lead can re-run)
- Include closure: strip comments, follow `#include "..."` in `vanilla_all/` (`closure.py`). 1 missing include (`k_inc_dan`, not used by the closure).
- "Contains the decision tree": `pykotor.resource.formats.ncs.read_ncs`, entry = instruction 0; follow fall-through, `JSR`/jump targets and the stored-action body after every `STORE_STATE` (`DelayCommand(.., GN_DetermineCombatRound())` compiles to a body skipped by a `JMP`; without this `a_atkonend` looked dead); report reachable `CONSTS` containing `START DETERMINE COMBAT ROUND`. Run over: 612 BIF `.ncs` (own `Bif` reader from `build_party_ai_scripts.py`), 4,224 module script names (462 archives copied to scratch), 288 override `.ncs`.
- Flag setters: initial-value block of the globals gives the slot of each flag; `CONSTI 1; CPTOPBP <slot>; JSR` = `GN_SetSpawnInCondition(flag)`. Validation: 69 readers of the boss flag = 18 + 51; 22 `SW_FLAG_AI_OFF` setters in plausible scripts.
- Styles: `ACTION 707 (SetNPCAIStyle)` with the preceding `CONSTI`. Local-number indices: `ACTION 681/682` with the preceding `CONSTI` (literal indices only).
- Templates: UTC `Script*` fields (GFF) from archives + BIF; spells from `ClassList/KnownList0`; weapons from `Equip_ItemList` struct id 16 + UTI `BaseItem` (modules, BIF, override).
