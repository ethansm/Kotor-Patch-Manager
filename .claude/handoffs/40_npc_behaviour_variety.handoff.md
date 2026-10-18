# Session Handoff & Plan State: patch_manager_mods/40_npc_behaviour_variety.md

## 0. Next-Phase Direction
NB2-B: Run B design from 15.6-15.8 (flags 9 vs SetAnimation), swap 0.2.6-probe, armed-area play

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/40_npc_behaviour_variety.md`
- **Waves:** .claude/waves/w10-40-V1/
- **Branch:** `KOTOR-II` (3b9c239e holster-editor 0.4.3-dev: button caption rect fix + CAPTION probe; offline editor mockup)
- **Generated:** 2026-10-18 16:28:26 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
3b9c239e holster-editor 0.4.3-dev: button caption rect fix + CAPTION probe; offline editor mockup
 .../holster_editor_src/HolsterEditor.cpp           | 117 ++++++-
 patch_manager_mods/holster_editor_src/HseView.h    |  14 +
 .../holster_editor_src/holster_editor.ini          |   6 +-
 .../holster_editor_src/hse_view_test.cpp           |  17 +
 .../holster_editor_src/manifest.toml               |   4 +-
 .../holster_editor_src/tools/hse_layout_dump.cpp   |  51 +++
 research/holstered_weapons/ux_v3/F3_R1_inset.txt   | 173 ++++++++++
 research/holstered_weapons/ux_v3/F3_R2_border.txt  |  48 +++
 .../holstered_weapons/ux_v3/F3_R3_mockup_reuse.txt |  57 ++++
 .../holstered_weapons/ux_v3/F3_R4_lineheight.txt   |  61 ++++
 .../holstered_weapons/ux_v3/F3_RV1_ideal3840.txt   |  32 ++
 .../holstered_weapons/ux_v3/F3_RV4_otherres.txt    |  28 ++
 .../holstered_weapons/ux_v3/F3_V1_calibration.txt  |  44 +++
 .../holstered_weapons/ux_v3/F3_V2_fixcheck.txt     |  57 ++++
 research/holstered_weapons/ux_v3/F3_V3_gates.txt   |   7 +
 scripts/holster_editor_mockup.py                   | 350 +++++++++++++++++++++
 scripts/test_holster_editor_mockup.py              | 143 +++++++++
 17 files changed, 1201 insertions(+), 8 deletions(-)
```
- **Dirty Files (187):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` A .claude/research/npc_variety/nb2_anim_driver_recon.md`
  - ` A .claude/research/npc_variety/nb2_play_semantics_recon.md`
  - ... +177 more (run `git status --short`)

## 2. Immediate Objective (NB2)
**Animation injector (`anim_variety`)**

Status: PARTIAL (HOST + BUILD, SECTION 15; CHAIN UNVERIFIED LIVE: FIRST RUN IS THE `ANIM_PROBE` LOG, THEN `ANIM_VARIETY`)
Task: Animation injector (`anim_variety`)

## 2b. Upcoming Batches in Pipeline
- **NB0** : Shared library `npc-world` (owns H1/H2/H3, provides `k2.npcworld@1`, section 12); moves the hooks and native spawn out of `k1_spawn_probe_src`; V2a and NB1-NB4 become its clients
- **NB1** : Visit salt wiring (V4b): `visit_variety` source = area-entry latch
- **NB3** : Extra walkers (`walker_variety`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/40_npc_behaviour_variety.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate NB2` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
### Dead ends and lessons (10-13)
- V7 traps: phase IDs must be unique across plans, so build phases are `NB1..NB5` (not `B1`/`W7`; W7 is a file name in `engine_research_findings/`). `mark-done` appends to `engine_research_findings/V7.md`: peer-owned ledger, leave it uncommitted.
- Wave `w9-npcvar` (12 Haiku units, Opus-gated): all PARTIAL because most units only had narrow docs in range. Raw output: `.claude/research/npc_variety/u01..u12.md`; consolidated: doc 40 sections 1-3. Units that ran big awk over `k2_utc.tsv` peaked 85-128k tokens (u06 128k, over the 100k ceiling): give catalog-scanning units a precomputed small TSV instead.
- `k2_utc.tsv` counts are inflated about 3.5x by case variants and `_s.rim` copies; always dedupe on lowercased module stem + resref.
- u02/u07 wrote to the wrong path (brief Task said `out/uNN.md`, Write Scope said `uNN.md`): keep the path identical in both.
- No `.nss` sources are on disk. Script behaviour comes only from `.claude/research/npc_add/R2_script_route.md` and `poc/P5_behaviour_dialog.md`. Decompiling needs DeNCS or pykotor on the game's scripts.bif; not yet tried.
- Do NOT `EnterWorktree`/install anything for this track before V2; V2 needs user GO and a pre-visit save slot (none proven; candidates Game1, Game5, Game8).
- u03 default run covered party rows only; `scripts/list_creature_anims.py --rows 190,185,18,479,485,39,12,569,24,601,47` gives the target bodies (result in `u03b_target_rows.txt`).
- Verification commands: `.venv/bin/python scripts/list_creature_anims.py --rows N,N --out <file>`; `python3 scripts/wave_tools.py check <wave>`; `python3 scripts/check_diversity_assets.py`; `python3 scripts/run_tests.py diversity`.
- Live state at hand-off: npc-diversity 0.1.6-probe LIVE (md5 7aaf936d), `npc-diversity_debug.txt` present in the game dir (debug REMAP lines ON); user play test of rows 812-817 still pending.

### V1 lessons and V4 facts (10-13, after V1 commit 3ec0cc1)
- `Scripts.bif` ships real `.nss`: `scripts/dump_game_scripts.py --out <dir> [--names a,b|--all]` (the older "no .nss on disk / decompile only" note above is WRONG). pykotor `decompile_ncs` fails here (no `dencs` package); do not retry it.
- Unit ids in `wave_tools.py add` must be `uNN` (not `rN`); max 3 entries per unit. Haiku units stay inside their line ranges: give them the whole function, or fix the gaps inline.
- `rea-recon`: pass the full path `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe`; never run two REA delegations at once (the shared session drops, "No app is open"); try `scripts/query_ghidra.py symbol|address` first (it names SaveCreatures 0x52A600, LoadModuleInProgress 0x55A460, ExecuteCommandExecuteScript 0x679050). Each REA run cost about 150k tokens.
- V4 facts: `HashCreatureId` is called in ONE place (`NpcDiversityDispatcher.cpp:493`), plus inside `ShouldVary` (`DiversityLogic.h:149`) and `PickTextureVar` (`:159`); the texture detour `OverrideNpcTexture` is log-only. Goldens for salt 0 live in `diversity_logic_test.cpp:128,438-449`. Verify: `python3 scripts/run_tests.py diversity` (12 suites).

### V4 lessons (10-13, commits c1346b6 + 4f71065)
- V4 landed: `visitSalt` in `HashCreatureId`/`ShouldVary`/`PickTextureVar`, `MakeVisitSalt`, `ActiveVisitSalt(visitVariety, salt)`; dispatcher reads `npc-diversity.ini [NpcDiversity] visit_variety=0|1` (default 0 = same look every visit) in `readConfig()` at first hook call; `g_visitSalt` stays 0 (V4b = area-entry source after V2). Any later random feature (walkers, encounters, animation) should get its own ini switch, default off, same pattern.
- `kpm_status` shows npc-diversity DIRTY (source differs from live 7aaf936d) until a reinstall; expected, nothing was deployed.
- Shared doc 34 held a peer hunk: stage only your hunk with `git diff -U0 -- <doc> > p; head -N p > mine; git apply --cached --unidiff-zero mine`, then `git commit` (no pathspec) as a separate commit; a pathspec commit ignores the staged hunk.
- `agent_contract.py check` takes `reply` as a positional arg (file path or `-`), `--scope` globs must come before it.

### V3 lessons (10-13, commit b51865a)
- V3 landed: `scripts/npcvar_roster_pool.py` + `npcvar_roster_areas.py` (tests colocated) write `tables/roster_pool.tsv` (23 eligible of 71) and `roster_areas.tsv` (4 eligible: 302nar 4, 301nar 2, 203tel 2, 204tel 1); spec in doc 40 section 8. `eligible` column is `true`/`false` in both tables.
- 682dan strict is 0 in `area_commoners.tsv` (the u08 "15" is `loose`); reason is `strict<5+waypoints<40`.
- `agent_contract.py check` FAILs on peer-dirty files outside the Write Scope (baseline has ~1700 dirty paths): judge only the claimed files. `mark-done` appends to `patch_manager_mods/engine_research_findings/V3.md` (leave uncommitted, peer-owned ledger).
- Haiku workers (~60k peak each, $0.015) did T1/T2 in one pass; they write absolute paths in replies, normalise to repo-relative before `agent_contract.py check`.
- Next: V6 animation feasibility. Script route facts are in doc 40 section 7.2; real `.nss` sources are in `.claude/scratch_npcvar/nss_all` (untracked; regenerate with `scripts/dump_game_scripts.py --out <dir> --all`).
