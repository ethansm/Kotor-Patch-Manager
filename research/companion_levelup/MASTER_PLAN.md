# MASTER PLAN: per-character level-up override (Ghidra mapping + Patch Manager C++ mod)

Status as of 2026-10-02: Phases 0/1/1b DONE (static). Phase 2 stage 0 DONE: probe `level-up-override` 0.1.0 (log + dry run) INSTALLED 09-30 21:38, live runs 1+2 read (doc 08 'Phase 2 live results' run 1/run 2: hook params, flow map, key=level+1, deltas, ability 6=none, Auto grants bypass pending, powers in spells-add all CONFIRMED; stats feat list is +0x00/+0x18). 0.1.1-probe built (feat dump fix), not installed. NEXT = Step 4 apply code (rewrite in place, default mode=log) then Step 5 user-gated Atton spike - see the plan file referenced in doc 08 'Session notes - Phase 2'.

Requirements update 2026-10-02 (user): rewrite only the AUTO level-up, never manual picks (0.2.1 does this); builds are player-chosen per playthrough; configuration via an in-game GUI (settings page or new main-menu page) - Phase 3 research item. Details: doc 08 Status.
Read order after a context clear: this file -> `PROJECT_CONTEXT.md` (sections 1-9) -> `PHASE1_PLAN.md` -> `README.md`.
Work tree: `/home/ethansm/orca/workspaces/local-setup/KOTOR-II` (git worktree, branch KOTOR-II). Commit only there; never push, never amend.
Commit trailer: `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`.

## Goal
User converts every convertible KOTOR II companion to Jedi with a fixed feat / Force power / skill path per character. The engine's
auto level-up cannot follow such a path: `packages.2da` has 8 rows (one per CLASS) pointing at class-wide ordered preference lists
(`PackFtJedis1`, `PackSkJedis1`, ...) and `spellpref2da` is empty for every row. Atton, Mira and the Exile share the JediSentinel
row, so data edits cannot differentiate them. Deliverable: a Kotor Patch Manager DLL mod (runtime hooks in `swkotor2.exe`) so
leveling follows a predetermined per-character path.

Test bed: in the user's saves (AUTOSAVE and Game9) Atton is Scoundrel 3 with 23,740 XP (several pending level-ups) in
Peragus/Harbinger modules; he cannot convert to Sentinel until later story progress, so first tests use his Scoundrel levels.
Kreia (Consular 6 in the save, no conversion) is the second subject. Verified 2026-09-29: AUTOSAVE `availnpc0` tag `atton`
Scoundrel 3; `availnpc6` tag `kreia` Consular 6; tags lowercase, no TemplateResRef on saved creatures.

Live binary: Steam Aspyr + LAA, sha256 `4ab72fc1ab082f427e008cda32fc5602d27b4e12fef48c4a1a2c6f7b2f36fb5a` = framework DB
`kotor2_steam_aspyr_laa.db` (only ~48 functions named). Ghidra project holds the PRISTINE binary (`6a522e71...`), code identical to
LAA per the knowledge DB `binaries` table. Nothing changes the game before Phase 3. Nothing is pushed.

## Phases
### Phase 0 - data + re-checked assumptions (DONE for atton/kreia)
Done: schema (`paths/<char>.toml`), `levelup_rules.py`, `check_paths.py`, `gen_paths.py`, atton + kreia paths, saves re-verified.
Remaining: Bao-Dur, Mira, Handmaiden, Disciple path files (need user build decisions: switch levels, melee/caster Disciple, blank "?"
picks in elkripper tables); Disciple live-feat discrepancy (Close Combat present, Finesse: Melee absent); effects of Alignment
Affects Force Powers / Classic Class Attack Bonus on each plan; Steam Cloud behavior check. None block Phase 1.

### Phase 1 - Ghidra mapping (read-only; persist EVERY finding to the knowledge DB before moving on)
Full detail: `PHASE1_PLAN.md`. Method model: `patch_manager_mods/02_buff_duration_hud.md`; knowledge-DB lessons (hook syntax/timing 29-31,
caller-search of non-virtual siblings 60 and 36; raw disassembly for hidden `this`; decompile-confirm every hook site; never trust
vtable position or name alone). Questions Q1-Q8 below. Deliverable: `patch_manager_mods/08_level_up_override.md` + knowledge-DB rows.

Questions Phase 1 must answer:
- Q1 Which flow does the user's use case take? User levels manually with the on-screen button (StartAutoLevelup/DoAutoLevelup and
  per-panel Recommend buttons, client side); the `Auto Level Up NPCs` ini option uses `CSWPartyTable::AutoLevelUpNPCs`. Find the
  shared routine both reach and hook there so BOTH flows follow the path.
- Q2 How is the package chosen: class id vs creature `starting_package` (GOG offset CSWSCreatureStats+140, byte)? Every companion in
  the saves has StartingPackage=0; no live_utc template carries the field.
- Q3 Client/server split: client-computed pending picks are sent by `SendPlayerToServer_LevelUp`; server runs `ValidateLevelUp`. Does the
  server recompute/reject picks it did not produce? Safest interception point?
- Q4 Can picks be left unspent; can the level-up GUI still close? (Option A vs B)
- Q5 Runtime creature identification: key on Tag, case-insensitive; confirm the runtime Tag field is readable.
- Q6 Struct layouts: CSWCLevelUpStats pending feats/skills/powers/class/level; current class levels, known feats/powers.
- Q7 Does NWScript `LevelUpHenchman(oCreature, nClass, bReadyForLevelUp, nPackage)` share this code?
- Q8 Hook-address overlap with the 14 installed patches (notably `k2-finesse-melee-fix`, which touches feat logic).

### Phase 2 - design gate with a cheap spike
- Option A (only if Q2 shows a single package-selection decision point): hook "which package for this creature" so each character maps
  to its own NEW `packages.2da` row with its own `PackFt_*`/`PackSk_*`(+spell) tables. Caveats: existing companions all have
  StartingPackage=0 so a hook or save edit is still needed; `packages.2da` lives in the BIF (extract to Override first); preference
  lists cannot express "hold until level N" or "leave points unspent". 2DA tooling: NEW small pykotor script (read_2da/write_2da,
  patched pykotor per patch #5, back up first, validate with `scripts/verify_2da.py`). Do NOT reuse `scripts/02_merge_2da_and_tlk.py`.
- Option B (fallback / likely): own picker reading `paths/*.toml`, applying picks via confirmed AddFeat/AddKnownSpell/SetSkillRank (or
  the client CSWCLevelUpStats equivalents, per Q3).
- Option C (cheap probe): `LevelUpHenchman` with a custom package row via visible console (`k2-visible-console`) or a test script, only
  if Q7 says the code is shared.
- Spike: on a COPY of the save in a NEW slot, record what the vanilla button does for Atton's Scoundrel levels, then try C/A. Log in
  `08_level_up_override.md`. Gate decides A vs B.

**Phase 2 decision (2026-09-30, after Phase 1b static closure):** Option B via hook 1 (`LevelUp` 0x006B9870 entry, log-only first). Option A is dead
(package lists live in the shared class row); Option C is unnecessary (LevelUpHenchman shares AutoLevelup -> same sink); the "stored LvlStatList via
SetExperience" data-only route stays as an unverified fallback. Design details (rewrite rule, grants preserved, skill-delta/unspent math, hold semantics,
runtime table `research/companion_levelup/runtime/levelup_paths.ini` generated by `gen_runtime_table.py`) are in `patch_manager_mods/08_level_up_override.md`
sec. "Phase 1b results". Remaining gates need user consent: (Step 3) log-only probe `level_up_probe_src/` on a save COPY for the 4 flows; (Step 4) one-feat rewrite
spike with INI kill switch. Only then Phase 3 (full engine for Atton + Kreia).

### Phase 3 - C++ mod
New dir `patch_manager_mods/level_up_override_src/`. Copy the LAYOUT of `buff_duration_hud_src/` (`manifest.toml` with both Steam
hashes, `kotor2-steam-aspyr.hooks.toml`, `exports.def`, C++; note its `[metadata] target_versions` convention). Build per
`patch_manager_mods/05_build_plan.md` Phase 0 outcome: `g++-mingw-w64-i686`, framework `./build-mingw.sh`, `dotnet build`; copy into the
framework's `Patches/<Name>/` (uncommitted clone), run `python3 ../create-patch.py`, check every hook's `original_bytes` against the
live exe file offsets before install.
- Steam-LAA only. Key by creature Tag, case-insensitive. Only companions with a path are intercepted; Exile, unlisted NPCs and every
  other flow pass through untouched. Any lookup/validation failure => vanilla behavior + log. Kill-switch setting; strict path-file
  validation. Exhaustive logging from day one (memory feedback-log-thoroughly): per level-up log character, class, level, candidates,
  choice, reason; verbose flag; log beside the DLL. Guard against re-entrancy (GUI thread vs server tick); no allocations in hot paths.
- Exit: builds, installs, boots to main menu, and the log proves each hook fired with no behavior change.

### Phase 4 - live test loop (offer the user the choice before the first live write; they prefer deploy-and-debug-live given rollback)
- Saves are Steam Cloud saves `.../Knights of the Old Republic II/cloudsaves/62703561/NNNNNN - GameN/` (`SAVEGAME.sav` ERF,
  `PARTYTABLE.res`, `GLOBALVARS.res`). Cloud sync can overwrite a restored copy: back up the WHOLE slot folder OUTSIDE the game dir and
  test in a NEW slot.
- `KPatchLauncher` removes ALL patches before installing (05_build_plan.md "Operational discovery"); no "uninstall one mod". Live install
  has 14 patches incl. verbose `buff-duration-hud`. Stage the FULL set (13 files from `batch3_live_patches/` + BuffDurationHUD + new
  patch), run the documented CLI, rely on the automatic `backups/patch_manager_*_pre_install_*` for rollback. Record exact commands in
  `08_level_up_override.md`. Game exe backups exist beside `swkotor2.exe` (`swkotor2.exe.backup.20260929_190855`, `.unpatched_vanilla`).
- Tests: (1) Atton pending Scoundrel levels (button flow, then Auto Level Up NPCs flow), compare every pick with `paths/atton.toml`;
  (2) Kreia Consular 6; (3) Jedi-conversion levels only when story allows or by documented save edit; Handmaiden's duels gate her switch
  (Exile levels 14/18); (4) automated verification: parse `SAVEGAME.sav` with pykotor, read `availnpcN.utc` (active party may be in the
  nested module `.sav`, read both), diff class levels/feats/powers/skills vs spec (extend `dump_companion_utc.py`); (5) negatives: Exile
  and unlisted NPC exactly vanilla, kill switch off => vanilla, malformed path file => vanilla + log, cancelling the screen leaves no
  partial state, server rejection path handled.
- Exit: Atton and Kreia full pending runs match spec with zero deviations; all negatives pass.

### Phase 5 - persist and commit
Update `08_level_up_override.md`, knowledge DB, `research/companion_levelup/README.md`/`PROJECT_CONTEXT.md`, memory pointers; commit to the
KOTOR-II worktree only.

## Critical files
- Research: `research/companion_levelup/{MASTER_PLAN.md,PHASE1_PLAN.md,PROJECT_CONTEXT.md,README.md,live_companions.json,dump_companion_utc.py,levelup_rules.py,check_paths.py,gen_paths.py,paths/}`
- Patch Manager docs/example: `patch_manager_mods/{README.md,02_buff_duration_hud.md,05_build_plan.md,03_framework_research.md}`, `buff_duration_hud_src/`
- Ghidra: `patch_manager_mods/ghidra_knowledge/` (kotor2_ghidra_knowledge.db + README), project `~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects/KOTOR2_SteamAspyr`,
  scripts `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts/`
- Address DBs: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/AddressDatabases/{kotor2_gog_aspyr.db,kotor2_steam_aspyr_laa.db}`
- Game: `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/` (Override is a symlink to `override`)

## Risks
Steam mapping cost/errors; client picks rejected by server validation (Q3); unspent picks / GUI cannot close (Q4); preference-list
semantics too weak (-> B); existing saves store companion state so template edits do not apply retroactively (runtime hook required);
hook overlap with the 14 installed patches (Q8); Steam Cloud overwriting test saves; Proton/Wine behavior; concurrent hook calls;
companion attribute gains stay engine-fixed and are out of scope unless Phase 1 shows the UI exposes them.

## Session notes / operating conventions (from the originating sessions)
- Working agreements: Sonnet subagents do routine work (Ghidra script runs, table dumps, doc drafting); Opus designs, reviews, gates, does
  live writes and commits. Never overlap a background job with a coding phase: verify, commit, then launch. Persist every Ghidra finding
  to the knowledge DB before moving on. With a safe rollback the user prefers deploying now and debugging live, but offer the choice.
- Tooling quirks: use `../../.venv/bin/python` (pykotor) from `research/companion_levelup/` and append `2>/dev/null` (pykotor prints huge
  DEBUG/traceback noise to stderr). No `sqlite3` CLI: use python3's sqlite3. `tomllib` is available (py3.12).
- Live 2DA tables are binary 2DA V2.b; read them via pykotor `Installation(GAME).resource(name, ResourceType.TwoDA)`. Base-game tables
  (classes, skills, featgain, classpowergain, exptable, packages, cls_spgn_jedi, cls_st_*) come from the BIFs, not Override.
- Rule-table facts learned in Phase 0 are in `PROJECT_CONTEXT.md` section 9.
- Proposed (not user-approved) items: skill-point allocations in atton/kreia paths; Atton Sentinel picks 6/7/9 (Precise Shot I-III) are
  placeholders from elkripper.
- Pending user decisions: builds for Bao-Dur/Mira/Handmaiden/Disciple; whether Phase 1 goes to a Sonnet subagent under Opus review.
