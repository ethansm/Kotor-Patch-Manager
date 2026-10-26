# Session Handoff & Plan State: patch_manager_mods/50_k2_render_performance.md

## 0. Next-Phase Direction
Execute **PF2a**: Redundant per-bin state hoist (set texenv/material/client state once per bucket)

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/50_k2_render_performance.md`
- **Waves:** .claude/waves/w1-50-PF2/
- **Waves:** .claude/waves/w1-50-PV/
- **Branch:** `KOTOR-II` (a715f5fc grass 0.1.8-dev: variety counters survive the PERF gap reset (load-time PV3 calls were wiped); PERF2 seeds=)
- **Generated:** 2026-10-26 01:28:06 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
a715f5fc grass 0.1.8-dev: variety counters survive the PERF gap reset (load-time PV3 calls were wiped); PERF2 seeds=
 .../K2GrassDrawDistance.cpp                           |  8 +++++---
 .../K2GrassDrawDistanceLogic.h                        | 19 ++++++++++++++++---
 .../grass_draw_distance_test.cpp                      |  9 +++++++++
 .../k2_grass_draw_distance_src/manifest.toml          |  2 +-
 4 files changed, 31 insertions(+), 7 deletions(-)
```
- **Dirty Files (185):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M .claude/settings.json`
  - ` M CLAUDE.md`
  - ... +175 more (run `git status --short`)

## 2. Immediate Objective (**PF2a**)
**Redundant per-bin state hoist (set texenv/material/client state once per bucket)**

Status: TODO
Task: Redundant per-bin state hoist (set texenv/material/client state once per bucket)

## 2b. Upcoming Batches in Pipeline
- ****PF3**** : Defaults from A/B logs; log tightening (`perf_log` default off, verbose gating)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/50_k2_render_performance.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **PF2a**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- `kpm_hotswap.py --prepare` refuses NOT-LIVE (new) ids. Build a new mod's kpatch with the module API instead:
  `python3 -c "import sys,datetime;from pathlib import Path;sys.path.insert(0,'scripts');import kpm_hotswap as h;print(h.prepare('<id>',{'verdict':'MIRROR'},Path('patch_manager_mods/<x>_src').resolve(),Path('~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager').expanduser(),datetime.datetime.now().strftime('%Y%m%d_%H%M%S')))"`
  then re-run the gate, then `kpm_reinstall.py --mod <ids> --dry-run`.
- Grass and fog changes that add hooks or new ids need a FULL reinstall (`kpm_reinstall.py`, user GO, game closed); hot-swap only for DLL-only changes.
- Gate default tier fails on peer texmem 6ee4936c: use `--skip-default`.
- `git commit -- <path>` fails if a pathspec is untracked (handoffs are untracked): commit tracked paths only.
- Culling at the fog end via the camera far plane is rejected (skyboxes are geometry).
- Fog has no per-frame cost worth cutting; the fog mod only moves it.
- Verification: `python3 scripts/run_tests.py k2_grass_draw_distance`, `run_tests.py fog_distance`, `run_tests.py k2_shadow_distance`; `scripts/pre_install_gate.sh --mod <x>_src --skip-default`; `python3 scripts/check_hook_spans.py`; `python3 scripts/run_tests.py -k raw_address`.
- PF1d (10-25 18:16): a peer session hot-swapped action-hotbar in the same second our `kpm_reinstall.py` started (backup `action-hotbar_swap_20261025_181615`) and mod-config at 18:15. The result was consistent (both LIVE = their kpatch), but `kpm_reinstall.py` does not lock out a concurrent `kpm_hotswap.py --apply`. Re-run `kpm_status.py` right before and after any install.
- New mods need a `[<id>]` section in the game-dir `kplog.ini` or they write no log (added fog + shadow at PF1d; backup `backups/batch_reinstall_20261025_181615/kplog.ini.pre`).
- `plan_manager.py mark-done` banner treats BUILT rows as unfinished, so it suggested resuming PF1a after PF1c; use INSTALLED/DONE or ignore the banner.
- Wave 2 batching decision lives in doc 50 section 4 (W2-1 = PF2d+PF2b+PF2c+PF2a probe in grass 0.1.5-dev; W2-2 = PF2a hoist + PF3).
- W2-1 evidence (10-25): `.claude/waves/w1-50-PF2/u01_pf2c_facts.md` (glDrawArrays CALLs 0x00436A95 / 0x00436B86, count at game [esp+8]; KPM source `esp+8` unbracketed = writable ADDRESS, bare `esp` refused; `scripts/check_hook_sources.py` FAILs unbracketed esp+N unless allowlisted like activation_fix line 29) and `u02_pf2a_pf2d_facts.md` (path decode from F=[0x00A32DF8] + caches; exe never uses GL query objects, so PF2d loads its own entry points).
- R4 correction: 0x0046661B is the OUTER bucket loop init, the inner bin loop is 0x0046664B. The 0.1.4 Begin/End hooks bracket the whole grass pass (timing still valid); only the comment "bin loop" is wrong.
- PF2c: never lower the bin count field (bin+0x0C) or the helper's arg6: it is also the normals/UV block offset (count*0x30). Thin only the pushed glDrawArrays count.
