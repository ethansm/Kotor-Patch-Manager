# Session Handoff & Plan State: patch_manager_mods/24_master_plan_status.md

## 0. Next-Phase Direction
All doc 24 phases DONE (H10 5673d94). Next: user GO for first live kpm_reinstall (dry-run first, game closed), or final verification/PR prep

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/24_master_plan_status.md`
- **Waves:** .claude/waves/w1-24-H1/
- **Waves:** .claude/waves/w2-24-H1/
- **Branch:** `KOTOR-II` (5673d94 feat(kpm_reinstall): snapshot, flat-stage KPatchLauncher run, peer DLL restore, rollback (doc 24 H10))
- **Generated:** 2026-10-14 00:01:56 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
5673d94 feat(kpm_reinstall): snapshot, flat-stage KPatchLauncher run, peer DLL restore, rollback (doc 24 H10)
 .claude/skills/kotor2-kpm-install/SKILL.md |   2 +-
 patch_manager_mods/BUILD_AND_INSTALL.md    |   2 +-
 scripts/kpm_reinstall.py                   | 285 +++++++++++++++++++++++++++++
 scripts/test_kpm_reinstall.py              | 271 +++++++++++++++++++++++++++
 4 files changed, 558 insertions(+), 2 deletions(-)
```
- **Dirty Files (149):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ... +139 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/24_master_plan_status.md`.

## 5. Dead Ends & Verification (manual)
- **Spec drift (doc 36 H1)**: `scripts/kpm_status.py:97` already globs depth 0 and 1; only the `**` change is needed. The doc's `kpm_status.py#L171` / `kpm_hotswap.py#L138` anchors are stale. Another session has small uncommitted edits in kpm_status.py (+7), kpm_hotswap.py (+-1), pre_install_gate.py (+3): `git diff` them first and commit only your own hunks.
- **Alias trap**: `canon()` keeps only the last path component, so `canon(str(rel))` adds no key; match the relative dir separately. Nested `probe/` manifests all get the parent name `probe` (collision hazard).
- **Wave artifacts**: cards in `.claude/waves/w2-24-H1/cards/` (a.md = kpm_status, b.md = kpm_hotswap), facts in `.claude/waves/w1-24-H1/`. Card b was Opus-patched after the semantic check (n=1 suggestion, action_hotbr and nested-probe tests added, no new ambiguity error).
- **Budget finding**: H1 recon/draft peaks were 62k-76k vs 47-53k estimated (agents explore beyond the listed entries). Hard ceiling 100k still held; the 60k target does not at the 45k overhead default. Decision pending with the user.
- **Verification**: `python3 scripts/run_tests.py test_kpm_status test_kpm_hotswap`
- **H1 landed (2ebd205)**: done inline (no waves). Test Base now patches `ks.MODS` too; `resolve()` reads `ks.repo_manifests()`.
- **H2 trap**: `kpm_hotswap.py:482-486` is the post-prepare check; pre-prepare REINSTALL is already HARD_REFUSE (`:48`, `:455`), so post-prepare REINSTALL only arises when the rebuild changes hooks. Keep rc 1 for non-SWAP/non-REINSTALL (DIRTY etc.) and for `--apply` with any REINSTALL. Rebuild failure (`:476`) stays rc 1. Spec's "mixed batch" test needs a faked `run_create_patch` that rewrites the kpatch hooks.
- **H2 verification**: `python3 scripts/run_tests.py test_kpm_hotswap`
- **H3 landed (8d236d9)**: inline. `step_default` tolerates exactly `{suite:check-exports}` (run_tests.py has no exclude flag); `step_exports` passes `--kpm-dir <scratch>` so only the `--kpatch` staged loop runs. Real-tree gate run (`--mod perf_lab_src/perflab --skip-default`) passed step 3 (`check_exports PASS=2 SKIP=1`) and then failed step 4 on `wine:perflab2/3` (attach-line count, 0.5.1 version line): untouched code, not investigated, NOT labelled pre-existing (AGENTS.md invariant): open item for the user.
- **H4 facts**: real KPM `Patches/` has 94 manifest ids, zero duplicates, and 3 non-mod dirs without manifest (`Common`, `_shared`, `build`), so the duplicate guard cannot break today's runs. A new mod reaches `prepare()` as verdict `MIRROR` with no `row["kpm"]` (`kpm_status.py:131-134`). `status()` order: NOT-LIVE before MIRROR, so a brand-new mod must already be in `patch_config.toml` live.
- **H4 verification**: `python3 scripts/run_tests.py test_kpm_status test_kpm_hotswap`
- **H3 open item RESOLVED (post-H4)**: the `wine:perflab2/3` step-4 failure was stale smokes, not mod bugs: `perf_lab_src/harness/smoke_{perflab2,perflab3,perflab5,hwmip}.cpp` pinned `perf-lab 0.5.1 attach`; perf-lab is 0.6.0. `smoke_perflab5.cpp` also required the `#METRICS` header to END with `module=`, but 0.6.0 appends ` state=<s> abreserve=<n>`; fixed with a `moduleOf()` helper. Gate: `pre_install_gate.py --mod perf_lab_src/perflab --skip-default` -> GATE PASS (50 passed, 33 skipped; md5 dd5f3f92...). Committed (see `git log -1 -- patch_manager_mods/perf_lab_src/harness`). Gate run used `--skip-default`: step 1 not re-run.
- **H5 recon (pre-plan)**: most of doc 36 H5 ALREADY exists: `AGENTS.md:6-12` (two-stage flow, "Verify After Fix & Gate Invariant", raw_addresses rule) and `BUILD_AND_INSTALL.md` section 5 (pre-mirror gate, prepare, MANDATORY full gate, apply). Real gaps: (a) project `CLAUDE.md` (what Claude Code reads) has NO verify-after-fix/"pre-existing" rule; (b) AGENTS.md lacks "never batch verification after multiple untested edits" and the "known-issues block" waiver wording; (c) harness middleware half (strip `cd $PWD &&`, 503 failover) lives OUTSIDE the repo in `~/.local/bin/oc-subagent` (failover on any non-zero exit already exists, lines 138-160; no `cd $PWD` stripping anywhere) and CLAUDE.md forbids oc-* backends in Claude Code, so treat as out of repo scope.
- **H9 landed (92bcb56)**: `kpm_ship.py` + 12 mocked tests; REINSTALL/NOT-LIVE targets refused there (H10's job). Live `--apply` never run.
- **H10 recon (pre-plan)**: `KPatchLauncher --patches <dir>` wants a FLAT dir of `.kpatch` files (see `patch_manager_mods/mods678_research/stage_and_verify_install.py` docstring), not `$K/Patches` (which holds per-mod source folders). All 43 live ids currently have a Patches/<X> kpatch (`ks.kpm_dirs`). Live config holds simple byte patches (`dll = ""`) next to DLL patches. Reuse `kpm_hotswap` snapshot style (`unique_dir`, `swap_manifest.json`, `CONFIG_FILES`, `is_game_running`) for the backup; backups go to `$G/backups/<id>_reinstall_<ts>/`.
