# Phase 1 plan: Ghidra research + investigation of the KOTOR II level-up code (Steam Aspyr LAA)

## Context
Project: make KOTOR II level-ups follow a fixed per-character path (feats / Force powers / skills) via a Kotor Patch Manager
C++ DLL hooking `swkotor2.exe`. Phase 0 (rule tables, path files for Atton + Kreia, validator) is done and committed
(a4e0b46, c6bdd39). Everything needed to resume is in the repo: `research/companion_levelup/MASTER_PLAN.md` (all phases, status,
conventions), `PROJECT_CONTEXT.md` (sec 9 = Phase 0 facts), `paths/*.toml`, `check_paths.py`. This plan is Phase 1 only: a
READ-ONLY static investigation that ends with decompile-confirmed Steam addresses, calling conventions and struct offsets for the
level-up code, stored in the knowledge DB and in `patch_manager_mods/08_level_up_override.md`. No game changes, no installs.

The Steam binary has only ~48 named functions; the GOG address DB names the whole level-up cluster. The repo already has
`patch_manager_mods/ghidra_knowledge/gog_cluster_delta_map.py` (GOG name -> Steam address by per-run delta voting; run `--selftest`
first) and a 2702-row `name_map` table plus `steam_functions.tsv` (every Steam function start with `ret N`). So the first step is a
no-Ghidra mapping pass, not string-anchoring from scratch. The `name_map` has NO CSWSCreatureStats / CSWCLevelUpStats /
CSWPartyTable / level-up GUI rows yet (only `Global::ExecuteCommandShowLevelUpGUI` 0x00692c80, `CGuiInGame::SetLevelUpMode`
0x007bfa20, `CGuiInGame::ShowLevelUpGUI` 0x007cb7c0, `Global::LoadFeatGain` 0x006f0cd0 - all "medium").

## Step 0 - Write the docs first (so a context clear loses nothing)
1. `research/companion_levelup/PHASE1_PLAN.md` = this plan. MASTER_PLAN.md already links to it.
2. Skeleton `patch_manager_mods/08_level_up_override.md` (status, address table [GOG addr | Steam addr | evidence | calling conv |
   confidence], struct offsets, Q1-Q8 answers with evidence, hook-candidate shortlist, open items, exact commands run).
3. Add a `08_level_up_override.md` line to `patch_manager_mods/README.md`'s index. Commit (explicit paths only).

## Step 1 - Read first (cheap, no tools beyond python/sqlite)
- Knowledge DB `lessons` ids 29-31 (hook syntax/timing), 36 & 60 (caller-search technique), 78/88/132/160/161 (per-region GOG->Steam
  deltas; delta differs per class - CSWGuiInGameCharacter is +0x2B6700, CSWGuiMainInterface +0x22E880 only >=0x51cb20), 14/15/53
  (GOG DB head start; never trust vtable slot position). `ghidra_scripts` table for reusable scripts. Query with python3 (no sqlite3 CLI).
- `02_buff_duration_hud.md` (worked example), `07_force_power_hotbar.md` and `engine_research_findings/` (may already contain
  known-spell / known-feat accessors or creature-stats offsets), `05_build_plan.md` "Operational discovery", the hook syntax in
  `buff_duration_hud_src/kotor2-steam-aspyr.hooks.toml`.
- GOG DB (`AddressDatabases/kotor2_gog_aspyr.db`; tables functions[class_name,function_name,address,calling_convention,param_size_bytes],
  offsets[class_name,member_name,offset,notes], classes[size,vtable]): dump these classes fully to a scratch TSV: CSWSCreatureStats
  (129 fns), CSWCLevelUpStats (33), CSWPartyTable (80), CSWGuiInGameCharacter, CSWGuiLevelUpCharGen, CSWGuiPowersLevelUp,
  CSWGuiFeatsCharGen, CSWGuiSkillsCharGen, CSWGuiAbilitiesCharGen, CSWGuiInGameGameplay, CSWCMessage, CSWSMessage, CClientOptions,
  Global (GetRecommended*, ExecuteCommand LevelUpHenchman/GetPCLevellingUp/ShowLevelUpGUI), CGuiInGame. DB oddity: GetClass
  0x48f9b0 is listed under BOTH CSWSCreatureStats and CSWCLevelUpStats - treat as unknown.

## Step 2 - No-Ghidra GOG->Steam candidate map (Sonnet-delegable; Opus reviews)
Run `python3 gog_cluster_delta_map.py --selftest`, then with class regexes for every class above, `--out` a TSV per class into the
scratchpad. Output = HYPOTHESES (vote rows strong, drift rows weak, ret-N-0 rows weakest). Record per-run deltas. Prior hint to test:
CSWSCreatureStats cluster sits ~0x3C480 below GOG (SetSkillRank 0x6B81C0, HasFeat 0x6B83F0, RemoveFeat 0x6B85F0, AddKnownSpell 0x6B86A0
from the Steam DB) => predicted AddFeat 0x6B8280, AutoLevelup near 0x6BEDF0; the Steam DB's AddFeat 0x6F5B40 is suspect.
Deliverable: candidate table in `08_level_up_override.md` marked UNCONFIRMED. Nothing enters `name_map` until Step 3 confirms it.

## Step 3 - Ghidra anchor confirmation (headless, serialised, read-only)
Invocation (from `kotor2-batch3-porting-plan.md`): `$GHIDRA/support/analyzeHeadless ~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects
KOTOR2_SteamAspyr -process swkotor2.exe -noanalysis -readOnly -scriptPath .../kotor-patch-manager/tools/ghidra_scripts -postScript X.java`
(GHIDRA=~/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC). First `pgrep -af 'analyzeHeadless|Ghidra-Headless'`; a
LockException means another session holds the project. Batch many addresses per run. Reuse `DecompileArgs.java`, `DisasmArgs.java`,
`Callers.java`, `FindCallersAndDecompile.java`, `ListFunctionsInRange.java`, `DumpVtableGeneric.java`; write a new script only if none
fits and register it in the DB `ghidra_scripts` table.
Confirmation rule per function: decompile shows the expected shape (strings/2DA names, constants, callee pattern) - not just an address
match. Confirm ONE anchor per contiguous run first, then the rest of the run.
Order (dependency-driven):
 3a. Server creature stats core: SetSkillRank, HasFeat, AddFeat (decompile both 0x6B8280 and 0x6F5B40), AddKnownSpell, RemoveFeat,
     GetClassLevel, GetLevel, SetClass/SetClassLevel. Gives struct offsets (feat list, known spells, skill ranks, class array,
     starting_package).
 3b. Server level-up logic: CanLevelUp, ValidateLevelUp, LevelUp, GetExpNeededForLevelUp, CanChooseFeat, CanChooseSpell, HasFeatPrereq,
     HasSpellPrereq, FeatRequirementsMet, AutoLevelup. Read `AutoLevelup` fully: which 2DAs (packages, featpref/skillpref/spellpref),
     how it picks the package (class id vs starting_package byte at +140 - Q2), how it chooses Force powers when spellpref is empty.
 3c. Client pending object: CSWCLevelUpStats (AddDefaultPackageSkills/Feats/Spells, ClearPackage, CanChooseFeat/Spell,
     CalcNumberFeats, AddGrantedFeats, IncreaseSkillRank, AddKnownSpell) and Global::GetRecommendedFeat/Skill/Spell. Client-side
     auto picks sent, or recomputed server-side (Q1, Q3).
 3d. Flow entry points: CSWGuiInGameCharacter StartAutoLevelup/DoAutoLevelup/ShowLevelUpGUI; CSWGuiInGameGameplay::OnAutoLevelUp; the
     Recommend-button handlers; CSWGuiLevelUpCharGen::LevelUpCreature/CancelLevelUp; CSWPartyTable::AutoLevelUpNPCs and callers
     (XP award -> DistributeExperience); CClientOptions::SetAutoLevelUpNPCs (ini flag reader is FUN_007b88b0).
 3e. Messages: CSWCMessage::SendPlayerToServer_LevelUp, CSWSMessage::HandlePlayerToServerLevelUpMessage,
     HandleServerToPlayerLevelUpMessage, SendServerToPlayerLevelUp_Confirmation - what the server trusts from the client (Q3).
 3f. NWScript: Global::ExecuteCommand for LevelUpHenchman -> does it call AutoLevelup / LevelUp? (Q7).
Each finding is written to the knowledge DB (functions, class_methods where vtable, stack_offsets, lessons, name_map rows tagged
`level-up-P1-decompile-confirmed`) BEFORE moving to the next sub-step.

## Step 4 - Answer the questions with evidence (write into 08_level_up_override.md)
- Q1 shared routine both flows reach + best single hook point (or two if none shared). Call graph from GUI button / ini auto-level /
  NWScript down to the pick code.
- Q2 package selection: exact instruction/expression mapping creature -> package row. Note `starting_package` offset on Steam.
- Q3 server trust: does HandlePlayerToServerLevelUp re-run ValidateLevelUp/CanChooseFeat per pick; can the server accept arbitrary
  valid picks (=> Option B on the client is enough) or does it recompute (=> hook server AutoLevelup).
- Q4 unspent picks: what state blocks the GUI from closing / LevelUp() from succeeding when slots remain.
- Q5 runtime creature identity: Tag (CSWSCreatureStats::GetTag/SetTag 0x6f2fb0/0x6f2fd0 on GOG) and client-side equivalent; how to
  reach from the hook's `this`. Confirm case handling (saved tags lowercase, e.g. `atton`).
- Q6 struct layouts: pending feats/skills/powers/class/level in CSWCLevelUpStats; known feats / powers / skill ranks / class list in
  CSWSCreatureStats; container types and element sizes; how `this` reaches them.
- Q7 LevelUpHenchman shares code? yes/no + evidence.
- Q8 hook overlap: every hook address in the 14 installed patches (`batch3_live_patches/`, `buff_duration_hud_src/*.hooks.toml`, live
  `patches/`) vs the shortlist `[addr, addr+len)`, especially `k2-finesse-melee-fix`.
Also record: calling conventions (`__thiscall` + ECX `this`, stack args, `ret N`), hook-site prologue bytes (`original_bytes`, whole
instruction boundary >=5 bytes for a detour), and whether the function is reentrant / called per pick or once per level.

## Step 5 - Cheap dynamic cross-check ONLY if static analysis leaves a Q3/Q4 doubt
A logging-only probe is Phase 2/3 scope and needs user consent (offer first). Static-only exit is the default. If needed, reuse
`_shared_src/SafeRead.h` and the buff-HUD logging pattern (frame-stamped, exhaustive).

## Step 6 - Hook shortlist + Phase 2 handoff
Ranked shortlist (max 3) of hook sites with: Steam address, `original_bytes` (read from the exe file via PE section VA->file offset
math, never assumed), convention, parameters to capture, readable/writable state, risk notes. State recommended A-vs-B design and the
Phase 2 spike plan. Update MASTER_PLAN.md status, PROJECT_CONTEXT.md, README index, memory pointer; commit explicit paths only
(trailer `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`), never push/amend.

## Delegation (per user preference)
Sonnet subagents: Step 1 GOG DB dumps, Step 2 delta-map runs, Ghidra script execution batches, doc/DB row entry from a supplied
findings list. Opus: choose decompile targets, read/judge every decompile, resolve Q1-Q8, decide hook sites, write knowledge-DB rows,
commit. Never run two Ghidra jobs at once, never overlap a background job with a coding phase (verify, commit, then launch).

## Exit criteria (Phase 1 done)
For Q1-Q3, Q5, Q6: Steam addresses + calling conventions + offsets, each decompile-confirmed and stored in the knowledge DB; Q4, Q7,
Q8 answered with evidence; `08_level_up_override.md` complete; hook shortlist with verified `original_bytes`; all committed locally.

## Risks / fallbacks
- Delta map drift on a class: fall back to caller-search from confirmed siblings (lessons 60, 36) and string anchors on 2DA names
  ("packages", "featgain", "classpowergain", "cls_spgn_jedi" - `LoadFeatGain` 0x6f0cd0 is a ready anchor).
- Client and server both compute picks or the server recomputes: may need two hooks; keep both in the shortlist.
- Steam code differs from GOG (Aspyr edits): every GOG structural claim is a hypothesis until decompiled.
- Ghidra project lock contention with other sessions: serialise, `-readOnly`.
- Nothing here touches the live game, saves or installed patches.
