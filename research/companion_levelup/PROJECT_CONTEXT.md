# PROJECT CONTEXT: per-character auto level-up override mod (read this first after a context clear)

Written 2026-09-29/30. Everything decided or learned in the originating conversation that is not derivable from code.

## 1. Goal
1. Finish a per-companion level-up guide (feats / Force powers / skills / class-switch timing) for the user's KOTOR II run.
   Assumption for ALL planning: every companion that can become a Jedi is converted.
2. Build a new Kotor Patch Manager (C++ DLL injection) mod that replaces the game's AUTO level-up with a predetermined,
   per-character path (so the guide can be applied automatically). Requires Ghidra mapping of the level-up code in
   `swkotor2.exe` plus C++ hook code.
3. Current test situation: the user has a save in which ATTON can level up several times (he is being held back at
   Scoundrel so that he can take Jedi/Sentinel levels later). This save is the primary live test bed. Do not modify or
   overwrite the user's saves without asking; copy first.

## 2. The player character (the Exile)
Light side female "Jack-of-all-Trades" Jedi Sentinel -> Watchman. Full guide snapshot:
`exile_playthrough_guide_snapshot.md` (source: github ethansm/hardware-tools, branch kotor2-modding-docs,
popos/docs/kotor2_playthrough_guide.md, commit a93f3232). Key decisions from it: CON 14->18 via attribute points at
levels 4/8/12/16 (needed for top D-Package implants); other attributes fixed at creation (STR10 DEX14 CON14 INT14 WIS14
CHA12); take Class Skill: Repair at level 1; Persuade maxed; Repair ~20; Computer Use 15-17; Awareness 15; powers:
Affect Mind + Haste taken, Force Valor next (guide relies on Master Valor), then Heal line, then Mind Trick line.
The guide has known inaccuracies vs StrategyWiki (see README.md finding list; duel levels 10/14/18 vs 14/18 only, and
Battle Precognition timing).

## 3. Companions in scope (all converted to Jedi)
| Companion | Now | Jedi class | Notes |
|---|---|---|---|
| Atton | Scoundrel 3 at recruit | Sentinel | elkripper plan: convert ~lvl 20 |
| Bao-Dur | Tech Specialist 6 | Guardian | only late-switch build documented |
| Mira | Scout 6 | Sentinel | best switch lvl 17-18; needs Awareness 11 on Exile |
| Handmaiden | Soldier 6 | Guardian | 3-duel gate (Exile lvl 14 / 18); PartySwap gives both her and Disciple |
| Disciple | Soldier 6 | Consular | can convert immediately; Consular = worst attack bonus (mod #10) |
Visas is already a Sentinel (no conversion). Kreia already a Consular. Droids/Hanharr/Mandalore/G0-T0 not convertible.
Exact live stats/feats: `live_companions.json`. Source pages and analysis: `README.md`, `text/`, `raw/`.

## 4. Installed mods that change the rules (154-mod r/kotor Full build + extras)
Thematic KOTOR 2 Companions (#8), Classic Class Attack Bonus (#10: Guardian 1.0, Sentinel 0.75, Consular 0.5 attack/level),
Alignment Affects Force Powers + Treat Injury Affects Force Healing (#142), Improved LS Enlightenment (#139),
Bao-Dur Can Wear Heavy Armor (#108), JC's Crystal Attunement (#109), PartySwap (#97) + Extended Enclave (#98) + compatch (#99),
Choose Mira or Hanharr (#95), Companion Starting Armors (#138), TSLRCM (+ tweak pack #6). Detail: README.md.

## 5. How level-up works (what is known)
- Companion ATTRIBUTE increases are automatic/fixed per companion (from build tables); feats, Force powers and skill
  points are chosen by the player when leveling manually. NOT yet confirmed in-engine.
- swkotor2.ini has `Auto Level Up NPCs=0` (live install). The engine's own auto level-up (used when that option is on, and
  by the level-up screen's auto button) is driven by `packages.2da` (8 rows, ONE PER CLASS: Soldier, Scout, Scoundrel,
  JediGuardian, JediConsular, JediSentinel, CombatDroid, ExpertDroid) -> `featpref2da` / `skillpref2da` tables such as
  PackFtJedis1 (ordered feat-index priority list, 60 rows) and PackSkJedis1 (ordered skill-index list, 8 rows).
  `spellpref2da` is EMPTY for every row: no Force-power preference table exists in data -> power auto-selection logic
  must live in engine code (unknown). Because the tables are per class, Exile/Atton/Mira all share JediSentinel, so a
  pure 2DA edit cannot give per-character behavior. That is the reason for an engine hook.
- Handmaiden/Bao-Dur `Unarmed Specialist` progression is per CLASS level (granted at class levels 2,6,10,14,...).

## 6. Engine / tooling knowledge that already exists in this repo
- Live binary: Steam Aspyr + LAA, sha256 `4ab72fc1ab082f427e008cda32fc5602d27b4e12fef48c4a1a2c6f7b2f36fb5a`
  (= framework DB `kotor2_steam_aspyr_laa.db`). Only ~48 Steam functions are named there; the GOG DB
  (`kotor2_gog_aspyr.db`, 10,425 functions) has real names. GOG-name -> Steam mapping must be re-derived by Ghidra
  (see patch_manager_mods/ghidra_knowledge lessons; never trust vtable position or name alone).
- GOG-DB leads relevant to leveling (addresses decimal from DB; GOG binary, NOT the live one):
  CClientOptions::SetAutoLevelUpNPCs (4598880), CGuiInGame::SetLevelUpMode (5046448), CGuiInGame::ShowLevelUpGUI (5095104),
  class CSWCLevelUpStats (GetClass 4782512, GetClassLevel 4782576, AddKnownSpell 4783952, RemoveKnownSpell 4784048),
  CSWCCreatureStats (SetClass 4782640, SetClassLevel 4782688, GetFeat 4783184, GetLevel 4781008, HasClassSkillFeat 4782944,
  GetSkillRank 4784464, GetNumberKnownSpells 4784336, SetDEX/CON/WIS/CHABase 4786752..., ClearFeats 4780240, ClearSkills 4780064),
  CSWCCreatureStats_ClassInfo (AddKnownSpell 7210208, HasKnownSpell 4800752). Server side: CSWSCreatureStats (offsets table:
  STR/DEX/CON/INT/WIS/CHA base at 237..247, ClassCount 141, starting_package 140); Steam LAA already names
  CSWSCreatureStats::AddFeat 7297856, AddKnownSpell 7046816, HasFeat 7046128, SetSkillRank 7045568, GetClass 8670784.
  Ini reader FUN_007b88b0 (Steam pristine `6a522e...`) reads Game Options incl. 'Auto Level Up NPCs' (ruled out for a
  different task, but it is the ini-parse site).
- Ghidra knowledge DB: `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db` + README (schema, lessons; no sqlite3
  CLI - use python). Read the `lessons` table BEFORE writing Ghidra scripts (findBytes is a regex; use find(Address,byte[]);
  RTTI names need Symbol.getName(true); exact byte match != unique hook site).
- Existing Patch Manager mods/docs: `patch_manager_mods/` (README index, 05_build_plan.md, 02_buff_duration_hud.md is the
  best worked example of a full Ghidra->hook->live-test cycle, 07_force_power_hotbar.md). Source of a working mod:
  `patch_manager_mods/buff_duration_hud_src/` (manifest.toml, *.hooks.toml, exports.def, C++). Framework clone:
  `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/` (uncommitted by policy). Proton/Wine run; live game dir:
  `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/` (has swkotor2.exe backups + kpm_install_state.json).
- Related memories: kotor2-patch-manager, kotor2-ghidra-knowledge-db, feedback-log-thoroughly (add exhaustive logging + memory
  dumps first when a hook misbehaves), feedback-phase-sequencing, feedback-sonnet-delegation, feedback-deploy-debug-live.

## 7. Working agreements (from memory / this conversation)
- Only commit to the local-setup/KOTOR-II worktree; nothing is committed yet for this research; do not push.
- Never overlap a background job with a coding phase. Verify, commit, then launch.
- Sonnet subagents for routine work; Opus designs/reviews/gates/live writes/commits.
- With a safe rollback the user prefers deploying to the live game and debugging live, but still offer the choice.
- Persist every Ghidra finding into the knowledge DB before moving on.
- WebFetch/curl need a browser User-Agent for StrategyWiki/elkripper.

## 8. Open questions (also tracked in README.md)
Bao-Dur early-switch plan; Atton higher-level feat picks; effect of TSLRCM 1.8/Extended Enclave/PartySwap on conversion
gates; Disciple live feat discrepancy (Close Combat present, Finesse: Melee absent); whether companion attributes are truly
non-selectable in the level-up UI; where the engine chooses Force powers on auto level-up.

## 9. Phase 0 status (2026-09-29) - level-up override plan
DONE: `levelup_rules.py` (rule tables from the live install), `check_paths.py` (validator), `gen_paths.py` (path generator),
`paths/atton.toml` (Scoundrel 4-19, Sentinel 1-9) and `paths/kreia.toml` (Consular 4-30): both pass with 0 errors; negative test
(bad prereqs / slot overflow / unselectable feat) is caught. Run: `../../.venv/bin/python check_paths.py 2>/dev/null`.
NOT DONE (need the user's build decisions, they do not block Phase 1): Bao-Dur, Mira, Handmaiden, Disciple path files
(schema = the two existing files; elkripper pages in `text/` have the level tables; many picks there are blank "?").
Re-verified on disk: saves at `.../cloudsaves/62703561/NNNNNN - GameN/` (AUTOSAVE = 000001); AUTOSAVE `availnpc0` = tag `atton`
Scoundrel 3, `availnpc6` = `kreia` Consular 6 (tags lowercase); ini `Auto Level Up NPCs=0`; exe sha256 = LAA hash.
Rule facts learned from the 2DAs (these corrected the plan's assumptions):
- feat.2da `<cls>_list`: 0/1 = selectable, 3 = class-granted (see `<cls>_granted` = class level), 4 = unavailable. Feat slots per class level:
  featgain.2da row (level-1), columns `<cls>_reg` + `<cls>_bon`. Scoundrel slots at class levels 1,2,5,8,11,...; Sentinel 1,3,6,7,9,12,13,...
- Force power slots: classpowergain.2da row (level-1), column jcn/jsn/jgd. spells.2da guardian/consular/sentinel = minimum CHARACTER
  level (99 / -1 = unavailable); `prerequisites` = spell ids joined by '_' (e.g. Master Heal '10_28').
- Skill points = classes.2da skillpointbase + INT mod (min 1); class skills = skills.2da `<cls>_class`. Scoundrel class skills here also
  include Persuade (elkripper omitted it). Max rank = char level + 3 (assumed; verify in game); cross-class cost 2 (assumed).
- Display names collide (Precise Shot I = ids 136 (unusable XXX) and 240 (real)); always use ids + `<cls>_list` check.
- Class ids: Soldier 0, Scout 1, Scoundrel 2, JediGuardian 3, JediConsular 4, JediSentinel 5, TechSpecialist 9.
Skill-point allocations in the two paths are PROPOSALS (greedy by priority), not user-approved builds.
