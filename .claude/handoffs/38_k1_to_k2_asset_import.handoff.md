# Session Handoff & Plan State: patch_manager_mods/38_k1_to_k2_asset_import.md

## 0. Next-Phase Direction
38 remaining: build probe appearance.2da+utc, user plays probe (install needs GO), T8 2DA, T13 commit k1_port; T7 done

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/38_k1_to_k2_asset_import.md`
- **Waves:** .claude/waves/w1-38-T12/
- **Branch:** `KOTOR-II` (78b6624 docs(research): NPC-add feasibility recon R1-R5 + summary)
- **Generated:** 2026-10-11 20:53:44 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
78b6624 docs(research): NPC-add feasibility recon R1-R5 + summary
 .claude/research/npc_add/00_summary.md         |  8 +++
 .claude/research/npc_add/R1_module_route.md    | 50 ++++++++++++++++
 .claude/research/npc_add/R2_script_route.md    | 79 ++++++++++++++++++++++++++
 .claude/research/npc_add/R3_engine_route.md    | 59 +++++++++++++++++++
 .claude/research/npc_add/R4_utc_dialog.md      | 54 ++++++++++++++++++
 .claude/research/npc_add/R5_prior_art_risks.md | 54 ++++++++++++++++++
 6 files changed, 304 insertions(+)
```
- **Dirty Files (124):**
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/34_npc_diversity_dispatcher.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/activation_fix_src/ActivationFix.cpp`
  - ` M patch_manager_mods/equip_icon_fix_src/EquipIconFix.cpp`
  - ` M patch_manager_mods/perf_lab_src/harness/smoke_activationfix.cpp`
  - ` M patch_manager_mods/perf_lab_src/harness/smoke_equipiconfix.cpp`
  - ... +114 more (run `git status --short`)

## 2. Immediate Objective (T8)
**2DA integration into diversity patch (needs user decision)**

Status: DEFERRED
Task: 2DA integration into diversity patch (needs user decision)

## 2b. Upcoming Batches in Pipeline
- **T9** [Testing & Contracts] : Licence registry, ledger, ingest driver, prune/gate (`sources.json`, `k1_port_sources/ingest/prune.py`, tests)
- **T10** [Testing & Contracts] : Writer fidelity to vanilla K2 (skin order, unknown0, pointers, tangents) in the 19-hunk patch
- **T11** [Testing & Contracts] : Ingest `n_commm` (vanilla, HD flagged) and `llk_LenaH/B` (credit) into the stage

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/38_k1_to_k2_asset_import.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate T8` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
### Dead ends & lessons (10-11)
- Stock pykotor skin pointers (`_calc_skin_offsets`) are wrong even for K1 writes; tool `match=true` hid it until skin bonemap/qbones/tbones and tangents were added to the compare. Never trust a converter match without skin and tangent flags.
- Byte-identical round trip is not the test; semantic compare plus raw-parse of pointers is. K2 vanilla evidence (`K2v_layout_evidence.md`) is the oracle, not K1.
- `agent_contract.py check --scope ...` reports OUT_OF_SCOPE/NOT_DIRTY for other sessions' files in this shared worktree; judge by the worker's own paths, not the FAIL count. Argument order: reply file first, then `--scope`.
- `python -I` omits the script dir from sys.path: every k1_port script inserts its own dir. py7zr: a second extract in one process raised CrcError once (files fine).
- `setup_venv.sh` skips hunks on an already-patched venv: `rm -rf` the venv before applying a changed patch.
- Pack candidate choice takes the shallowest path ("Optional - Upscaled Textures" beats `tslpatchdata`); fix before extracting optional folders.
- K2 already ships n_commm/n_commf/n_commkidm/n_commkidf/n_twilekf: converted models must be renamed `k1p_*` (tool enforces, collision check ~27 s on a miss).
- Do NOT: run `mdlops.exe` without user GO (T7); touch compatdata/208580; extract archives into the repo; commit pack assets; restore doc 33 handoff (user: another session owns it); run `prune --apply`/`approve` on the real ledger without user say-so.

### Verification commands
- `python3 scripts/run_tests.py k1_port` (5 suites, ~40 s).
- Fresh conversion check: `K1PORT_LEDGER=<scratch>/l.json ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_model.py --install-k1 ~/.local/share/Steam/steamapps/common/swkotor --resref n_commm --tex-map N_CommM04=k1p_commm04 --new-resref k1p_commm --out-dir <scratch>/out` -> match=true, K2 mdl 165675 B.
- Ledger/gate: `python3 patch_manager_mods/k1_port/k1_port_prune.py list`; `... gate <file>` (exit 1 on flagged sha).
- Repo asset leak check: `git status --porcelain | rg '^\?\? .*\.(tpc|tga|mdl|mdx|7z|rar)$'` -> empty.
