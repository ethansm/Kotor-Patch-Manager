# Session Handoff & Plan State: patch_manager_mods/34_npc_diversity_dispatcher.md

## 0. Next-Phase Direction
resume patch_manager_mods/34_npc_diversity_dispatcher.md
- next step is log analysis:  read area_hook_probe_log.txt with kotor2_logs.py. It should show whether H3 (saved load) fires instead of H2 (fresh)
- read general logs related to our npc diversity.  Bad textures are gone and the percentage math and exclusions looked good so far.
Leverage Haiku subagent swarm for planning and in the plan.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/34_npc_diversity_dispatcher.md`
- **Waves:** .claude/waves/w1-34-W6/
- **Waves:** .claude/waves/w2-34-npcadd/
- **Branch:** `KOTOR-II` (e94a142 docs(37): mark BG23 fixed)
- **Generated:** 2026-10-12 00:40:56 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
e94a142 docs(37): mark BG23 fixed
 patch_manager_mods/37_bug_board.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```
- **Dirty Files (114):**
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/perf_lab_src/harness/smoke_robustness.cpp`
  - ` M patch_manager_mods/perf_lab_src/harness/smokes.toml`
  - ` M scripts/kpm_hotswap.py`
  - ` M scripts/kpm_status.py`
  - ... +104 more (run `git status --short`)

## 2. Immediate Objective (**Phase 2**)
**Detour Hook Prototyping**

Status: REOPENED (10-11 REVIEW)
Task: Detour Hook Prototyping

## 2b. Upcoming Batches in Pipeline
- ****Phase 3**** : Fat Body & Alien Model Distribution
- ****Phase 4**** : Exclusion List & Savegame Compatibility
- ****Phase 5**** : Production Packaging

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/34_npc_diversity_dispatcher.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **Phase 2**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
### Wave 6 state (2026-10-11, session end)
LIVE: `0.1.3-probe` (repo `1c4c7a5`, KPM `177933f`, md5 `8c01ab2c`; rollback `python3 scripts/kpm_hotswap.py --rollback npc-diversity_swap_20261011_184033`). Gate 1 (user play of Telos, PTR dumps) NOT yet reported.
Landed commits: `1c4c7a5` probe, `8e392ab` labelling pipeline + registry + roles, `cc81e78` alien recon, `c9b8064` generators + DiversitySlots.h/DiversityRoles.h.
Generated, byte-stable (`--check`): `DiversitySlots.h` (M 1-16, F 1-17, FatM 34, FatF 35), `DiversityRoles.h` (112 keep tags, 158 keep positions, 1.1% ambiguity). Sources: `texture_labels.json`, `.claude/npc_div/registry.jsonl`, `.claude/npc_div/roles/out_*.json`.

### Dead ends / findings
- Pool slots 17-33 were HEAD textures, 28-30 robes, 31-33 armor, F33/F34/M35 other-mesh (Onderon/Sith) clothing: they belong to other body meshes (UV mismatch), NOT plain N_CommM/F. Never put them in the plain pool. User liked them: use via alt-body ROW remap (Onderon rows 472-478/502-506, Sith 291-302/42/43) in a later tier (`texture_labels.json` field `alt_body`).
- Haiku (86% vanilla recall) and Sonnet (76%) vision on flat atlases cannot separate clothing from robe/uniform; the UV-layout signature (vanilla min 0.28, decoy max 0.09, AUC 1.0) + one Sonnet vote (69/70 agree) is the decider. Vote B was skipped as redundant.
- F17 (uv 0.20) and M16 (uv 0.27) are USER-APPROVED TRIAL slots (plain FD/MD mesh): check them at Gate 2; if any look scrambled, drop via `ADJUDICATED` in `scripts/npc_div_merge_labels.py`.
- Conversation-set is NOT a named signal: generic commoners share bark conversations (`200comm`, `201travm`); registry rule uses conv_n>=4 as generic.
- Engine tag not readable (+0x1C/+0x20 are pointers); position fallback is the keep-default route unless Gate 1 PTR dumps show an ASCII tag.
- Alien-commoner recon (`.claude/npc_div/aliens/00_overview.md`) is a follow-on, not Wave 6.

### Verification commands
`python3 scripts/run_tests.py diversity generate_diversity_slots generate_diversity_roles npc_div_merge_labels npc_div_registry npc_div_texture_sheets npc_div_log_check`; `python3 scripts/generate_diversity_slots.py --check`; `python3 scripts/generate_diversity_roles.py --check --report`; `python3 scripts/build_check.py --mod npc_diversity_src`; `python3 scripts/check_hook_bytes.py --mod npc-diversity`; `python3 scripts/check_hook_spans.py`; `python3 scripts/kpm_status.py --only npc-diversity`.
- NPC-add R7 (10-12): R6's claim that 0x5582f0 never reaches 0x55a460 is REFUTED. Probe 0.1.0 shows H2 firing one tick after H3 on the same module, including a save load. Detail: `.claude/research/npc_add/recon_hook/R7_probe_log_result.md`.
- NPC-add R7: H1 latch (this+0x14), area+0x124 id (00000001 on both areas) and the area pointer (0c5d4420, reused) are dead keys. Key on area resref.
- Diversity rule 11 bug (oid 7f000000, tv 1 vs 102): fixed in source 0.1.5-probe (`NpcDiversityDispatcher.cpp:470` is now a 2-arg `PickTextureVar`). Not built or installed in this task.
- `log_triage` shows only the last 5 raw lines of a probe log. Lines 1-10 are hidden, so save-load vs door transition for 204tel at +435 s is unresolved. Probe 0.2.0 adds a POP-STABLE marker.
- NPC-add probe 0.2.0 (10-12): H2 fires for in-session area changes too (7/7 loads show H3, H2 one tick later, H4), and POP-STABLE fired on 7/7 loads at 33-34 ticks after load (verified). POP may miss one add (its count is 1 below the probe counter in 5/5 areas). Detail: `.claude/research/npc_add/recon_hook/R7_probe_log_result.md`.
- Diversity rule 11 fix (0.1.5-probe) verified live: 35 repeated oids stable (FAIL on 0.1.4-probe). Rule 16 still FAIL but compares across launches, so it is not a within-launch regression.
- Diversity rule 10 FAIL on the 0.1.5-probe launch (app=72, n=30, tv distinct=1): flagged, not yet investigated (E1 wave).

## Update 10-11 (Wave A+B landed, 0.1.4-probe built, NOT installed)
- Gate 1 NOT done: the last log is `0.1.2-probe`, no PTR lines. Tag route = default position-only keep (`DiversityKeep.h`); header comment says so. Re-run Task 6 only if the user plays 0.1.3 or 0.1.4 first (0.1.4 still logs PTR dumps).
- Landed: family allowlists (`FamilyOf`, `ShouldVary` 35%, `PickTextureVar(oid, app, currentTv)`), `DiversityKeep.h`, dispatcher gates (`why=keep`, `why=skip:vary`), STAT remap `vary= novary= keep=`, checker rules 12-16 (tv allowlist, changed share, keep-unchanged, stype tv writes, cross-launch map), `scripts/npc_div_whatis.py`, assets checker reads `DiversitySlots.h`.
- Known: checker R2 numerator counts every changed oid (incl. 805-808), so it can read a few % high.
- Next: Task 9 hot-swap `0.1.4-probe` (`kpm_status.py --only npc-diversity`, `kpm_hotswap.py --mod npc-diversity --prepare`, commit KPM, `--apply`), then ask the override question, then Gate 2 play.
- INSTALLED 10-11 20:24: `0.1.4-probe` LIVE (repo 9cf7b28, KPM 2062f71, DLL md5 2b697c78). Rollback: `python3 scripts/kpm_hotswap.py --rollback npc-diversity_swap_20261011_202423`. Awaiting Gate 2 play (also satisfies Gate 1: PTR dumps still logged). Open question to user: move the 35 rejected pool files out of the live override dir (reversible backup)?

## Update 10-11 late (NPC-add PoC, a SEPARATE track from diversity Wave 6; "wave 2" in the Next-Phase Direction = NPC-add Wave 2)
- Plan: `.claude/plans/npc_add_poc.md` (area DECIDED: 204TEL medical bed ward, Doctor Ganius `corteldc`). Recon: `.claude/research/npc_add/{R1..R5,00_summary.md}`, `poc/{P1..P6,W1a..W1d}`. Draft cards: `.claude/waves/w2-34-npcadd/cards/B1..B6.md` (untracked; `wave_cards.py verify` FAILs only on format: new files listed as Target Files, short anchor paths; content is fine).
- Decisions taken at the Opus gate: B3 = pure helper module (no .mod write); B2 owns all GIT/MOD edits and consumes B1 UTCs + B3 helper (so B1,B3,B4,B5,B6 first, then B2); orientation = unit vector toward target (do not use W1c radians); waypoint TemplateResRef `npcwp_<key>_NN` (<=16), tag `WP_npcadd_<slug>_NN`; creature tag = resref = `npcadd_<slug>`; B4 adds P6 A5/A6 checks; per-species soundsets/Gender deferred to batch 2.
- Dead ends: P1 walkmesh read failed because it looked up the module name (WOKs are keyed by room name, e.g. 204TELb, in chitin Models.bif); GIT model round trip is lossy (use GFF level); rea-recon overran budget (Q1-Q3 unresolved statically, so .mod precedence and spawn-script firing are play-test items).
- Still needed from the user: which save (Game1/5/8 or fresh) for Gate 2; GO for the live 204tel.mod write (Gate 1). Diversity Gate 2 play of 0.1.4-probe is still pending.

## NPC-add PoC Wave 2 (10-11, commit 84ca69a)
- Built offline, nothing installed: `patch_manager_mods/npc_add_src/out/{override/*.utc (18 mirror), modules/204tel.mod (sha c7d53b92, 21 creatures/51 waypoints/193 res)}`; doc `patch_manager_mods/35_npc_add_poc.md`.
- Tools: `scripts/npc_add_build_utc.py`, `npc_add_build_git.py`, `check_npc_add.py` (7 PASS/1 SKIP), `npc_add_install.py` (dry-run default; `--game`/`--game-dir`, `--apply`, `--rollback <stamp>`).
- Next: Gate 1 (user GO, game closed, ask which save) `python3 scripts/npc_add_install.py --apply`; Gate 2 play (Wookiee, Twi'lek F, Bith idle, Rodian walking R1). Open: .mod-vs-.rim precedence, k_def_spawn01 walking, hostility.
- Diversity track unchanged: `0.1.4-probe` LIVE 2b697c78, awaiting user Gate 2 play.
