# Session Handoff & Plan State: patch_manager_mods/38_k1_to_k2_asset_import.md

## 0. Next-Phase Direction
Execute T14: probe build (k1_port_probe_build.py + save backup in probe tool), then T15 install on user GO, T16 user play test. T7 done, T13 committed 5cba5c3

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/38_k1_to_k2_asset_import.md`
- **Waves:** .claude/waves/w1-38-T12/
- **Branch:** `KOTOR-II` (5cba5c3 feat(k1-port): K1->K2 model/texture converter, licence ledger, raw validator, probe installer (doc 38 T0-T12))
- **Generated:** 2026-10-11 20:55:41 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
5cba5c3 feat(k1-port): K1->K2 model/texture converter, licence ledger, raw validator, probe installer (doc 38 T0-T12)
 .../handoffs/38_k1_to_k2_asset_import.handoff.md   |  68 ++
 .claude/review/k1_import/A1_mdlops.md              |  18 +
 .claude/review/k1_import/A2_proton_run.md          |  31 +
 .claude/review/k1_import/B2a_pk_fix.diff           |  28 +
 .claude/review/k1_import/B2a_pk_fix.md             |  17 +
 .claude/review/k1_import/B2b_real_k2_read.md       |   7 +
 .claude/review/k1_import/B3_k2_roundtrip.md        |  18 +
 .claude/review/k1_import/B4_k2_semantic.md         |   4 +
 .claude/review/k1_import/K1a_extract.md            |  30 +
 .claude/review/k1_import/K1b_k1_to_k2.md           |  26 +
 .claude/review/k1_import/K1c_vanilla_names.md      |  31 +
 .claude/review/k1_import/K2v_layout_evidence.md    |  70 ++
 .claude/review/k1_import/L1_pack_licenses.md       |  73 ++
 .claude/review/k1_import/N1_npcdiv_overlap.md      |  25 +
 .claude/review/k1_import/P1_texmap.md              |  15 +
 .claude/review/k1_import/R1_k1_tex_deadlystream.md |  33 +
 .claude/review/k1_import/R2_k1_tex_other_hosts.md  |  33 +
 .claude/review/k1_import/R3_k1_npc_tex_names.md    |  11 +
 .claude/review/k1_import/R4_neo_cities.md          |  34 +
 .claude/review/k1_import/T10_k1_sources.md         |  21 +
 .claude/review/k1_import/T12c_node_order.md        |  94 +++
 .claude/review/k1_import/T12e_engine_loader.md     |  59 ++
 .claude/review/k1_import/T12e_engine_loader_v2.md  |   5 +
 .claude/review/k1_import/T12h_probe_design.md      | 137 ++++
 .claude/review/k1_import/T1_mdledit_cli.md         |   4 +
 .claude/review/k1_import/T2_model_format.md        |  35 +
 .claude/review/k1_import/T2b_supermodels.md        |  19 +
 .claude/review/k1_import/T3_supermodel_textures.md |  16 +
 .claude/review/k1_import/T3b_pack_vs_vanilla.md    |  37 +
 .claude/review/k1_import/T4_2da_schema.md          |  39 +
 .claude/review/k1_import/T5_voice_resref.md        |  23 +
 .claude/review/k1_import/T6_mods_a.md              |  21 +
 .claude/review/k1_import/T6_poc_n_commm.md         |  67 ++
 .claude/review/k1_import/T6a_uv_loss.md            |  50 ++
 .claude/review/k1_import/T6d_skin_pointers.md      |  59 ++
 .claude/review/k1_import/T6f_tangent_loss.md       |  58 ++
 .../review/k1_import/T6h_skin_vanilla_layout.md    |  47 ++
 .claude/review/k1_import/T6i_tangent_zero_rows.md  |  38 +
 .claude/review/k1_import/T6j_lena_ingest.md        |  50 ++
 .claude/review/k1_import/T7_mdlops_result.md       |  18 +
 .claude/review/k1_import/T7_mods_b.md              |  29 +
 .claude/review/k1_import/T7a_inputs.md             |  31 +
 .claude/review/k1_import/T8_headless_tooling.md    |  39 +
 .claude/review/k1_import/T9_kpatch_local.md        |  12 +
 .claude/review/k1_import/W1_voice_priority.md      |  30 +
 .claude/review/k1_import/W2_raw_k1_failures.md     |  20 +
 .claude/review/k1_import/W3_pykotor_convert.md     |  16 +
 .claude/review/k1_import/W4_save_persistence.md    |  27 +
 .claude/review/k1_import/W5_effixian_seikan.md     |  23 +
 .claude/review/k1_import/download_list.md          |  59 ++
 .claude/review/k1_import/probe_pykotor_k2_write.py |  37 +
 .../review/k1_import/scripts/T7_mdlops_compare.py  |  83 ++
 .../review/k1_import/scripts/T7_mdlops_pass1.sh    |   8 +
 .../review/k1_import/scripts/T7_mdlops_pass2.sh    |   8 +
 .../review/k1_import/scripts/T7_mdlops_strides.py  |  11 +
 .claude/review/k1_import/scripts/delta_check.py    |  21 +
 .claude/review/k1_import/scripts/extract_k1.py     |  33 +
 .claude/review/k1_import/scripts/k1b_run.py        | 114 +++
 .claude/review/k1_import/scripts/roundtrip.py      |  56 ++
 .claude/review/k1_import/scripts/semantic_check.py | 165 ++++
 patch_manager_mods/34_npc_diversity_dispatcher.md  |   2 +-
 patch_manager_mods/38_k1_to_k2_asset_import.md     | 126 +++
 patch_manager_mods/k1_port/PERMISSION_REQUESTS.md  |  71 ++
 patch_manager_mods/k1_port/README.md               |  72 ++
 patch_manager_mods/k1_port/k1_port_ingest.py       | 582 ++++++++++++++
 patch_manager_mods/k1_port/k1_port_model.py        | 789 +++++++++++++++++++
 patch_manager_mods/k1_port/k1_port_probe.py        | 594 ++++++++++++++
 patch_manager_mods/k1_port/k1_port_prune.py        | 386 +++++++++
 patch_manager_mods/k1_port/k1_port_sources.py      | 348 ++++++++
 patch_manager_mods/k1_port/k1_port_textures.py     | 462 +++++++++++
 patch_manager_mods/k1_port/k1_port_validate.py     | 325 ++++++++
 patch_manager_mods/k1_port/pykotor_k2_mdl.patch    | 257 ++++++
 patch_manager_mods/k1_port/setup_venv.sh           |  35 +
 patch_manager_mods/k1_port/sources.json            | 152 ++++
 patch_manager_mods/k1_port/test_k1_port_ingest.py  | 530 +++++++++++++
 patch_manager_mods/k1_port/test_k1_port_model.py   | 871 +++++++++++++++++++++
 patch_manager_mods/k1_port/test_k1_port_probe.py   | 305 ++++++++
 patch_manager_mods/k1_port/test_k1_port_prune.py   | 471 +++++++++++
 patch_manager_mods/k1_port/test_k1_port_sources.py | 395 ++++++++++
 .../k1_port/test_k1_port_textures.py               | 451 +++++++++++
 .../k1_port/test_k1_port_validate.py               | 463 +++++++++++
 81 files changed, 9977 insertions(+), 1 deletion(-)
```
- **Dirty Files (117):**
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/38_k1_to_k2_asset_import.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/activation_fix_src/ActivationFix.cpp`
  - ` M patch_manager_mods/equip_icon_fix_src/EquipIconFix.cpp`
  - ` M patch_manager_mods/perf_lab_src/harness/smoke_activationfix.cpp`
  - ` M patch_manager_mods/perf_lab_src/harness/smoke_equipiconfix.cpp`
  - ... +107 more (run `git status --short`)

## 2. Immediate Objective (T14)
**Probe build: `k1_port_probe_build.py` makes stage `probe_T12h/override/{appearance.2da,per_miner_002.utc}`; probe tool gets save-slot backup; `plan` turns green**

Status: TODO
Task: Probe build: `k1_port_probe_build.py` makes stage `probe_T12h/override/{appearance.2da,per_miner_002.utc}`; probe tool gets save-slot backup; `plan` turns green

## 2b. Upcoming Batches in Pipeline
- **T8** [Testing & Contracts] : 2DA integration into diversity patch (needs user decision)
- **T9** [Testing & Contracts] : Licence registry, ledger, ingest driver, prune/gate (`sources.json`, `k1_port_sources/ingest/prune.py`, tests)
- **T10** [Testing & Contracts] : Writer fidelity to vanilla K2 (skin order, unknown0, pointers, tangents) in the 19-hunk patch

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/38_k1_to_k2_asset_import.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate T14` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

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
- (10-11 late) MDLOps (T7) is NOT a per-vertex reference: it re-welds vertices and swaps 6 bonemap entries; use validator + K1 original as truth. Run dir `~/Games/KOTOR2_Mod_Workspace/mdlops_t7/mdl_run1/`.
- (10-11 late) rea-recon with GOG anchors lands on unrelated Steam functions (2 tries, all Qs UNSURE); do not retry without Steam addresses. The probe is the engine test.
- Commit 5cba5c3 holds k1_port/ + doc 38/34 note + evidence; doc 38 rows T14-T16 and this handoff are uncommitted. `k1_port_probe.py plan` is blocked only on the unbuilt `probe_T12h/override/{appearance.2da,per_miner_002.utc}`; tool has no save-slot backup yet (design section 6 step 2).
- Verify: `python3 scripts/run_tests.py k1_port` (7 suites); `python3 patch_manager_mods/k1_port/k1_port_probe.py plan --game-dir "$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II"`.
