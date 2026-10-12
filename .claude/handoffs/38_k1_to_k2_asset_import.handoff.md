# Session Handoff & Plan State: patch_manager_mods/38_k1_to_k2_asset_import.md

## 0. Next-Phase Direction
38 body-mesh fault: spawn POC works (Track S installed, 7 files, 204tel, persists); body renders textured but exploded. Next: A/B the T7 MDLOps K2 output as k1p_commm (static prep, needs user GO + game closed to install), then decide writer fix vs engine RE (skin loader, Steam twins unfound). T8 2DA only if body renders.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/38_k1_to_k2_asset_import.md`
- **Waves:** .claude/waves/w1-38-T12/
- **Waves:** .claude/waves/w1-38-T17/
- **Waves:** .claude/waves/w2-38-T17/
- **Waves:** .claude/waves/w3-38-T18/
- **Waves:** .claude/waves/w4-38-T19/
- **Branch:** `KOTOR-II` (163de0f docs(38): T19 Track S spawn result, texture alias, log swarm findings)
- **Generated:** 2026-10-12 14:05:26 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
163de0f docs(38): T19 Track S spawn result, texture alias, log swarm findings
 patch_manager_mods/38_k1_to_k2_asset_import.md | 16 ++++++++++++++--
 1 file changed, 14 insertions(+), 2 deletions(-)
```
- **Dirty Files (121):**
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/perf_lab_src/harness/smoke_robustness.cpp`
  - ` M patch_manager_mods/perf_lab_src/harness/smokes.toml`
  - ... +111 more (run `git status --short`)

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
- (10-11 late) MDLOps (T7) is NOT a per-vertex reference: it re-welds vertices and swaps 6 bonemap entries; use validator + K1 original as truth. Run dir `~/Games/KOTOR2_Mod_Workspace/mdlops_t7/mdl_run1/`.
- (10-11 late) rea-recon with GOG anchors lands on unrelated Steam functions (2 tries, all Qs UNSURE); do not retry without Steam addresses. The probe is the engine test.
- Commit 5cba5c3 holds k1_port/ + doc 38/34 note + evidence; doc 38 rows T14-T16 and this handoff are uncommitted. `k1_port_probe.py plan` is blocked only on the unbuilt `probe_T12h/override/{appearance.2da,per_miner_002.utc}`; tool has no save-slot backup yet (design section 6 step 2).
- Verify: `python3 scripts/run_tests.py k1_port` (7 suites); `python3 patch_manager_mods/k1_port/k1_port_probe.py plan --game-dir "$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II"`.

- 10-11 spawn-hook session: a save made INSIDE an already-visited area (plant room, `204tel`) stores its creature instances, so a UTC override does not show there (expected, unverified); Route B therefore replays from Game9 (`106per`, 204tel unvisited). None of the 11 cloudsaves slots is on Telos surface/204tel except Game2 (`204tel`, visited).
- Dispatcher (doc 34) only remaps appearance of existing creatures: no spawn, no name hook. Route A2 = new hook calling the `CreateObject` machinery. Steam addrs known: ExecuteCommandCreateObject 0x0066f990, CSWSArea::LoadCreatures 0x00525f90, CSWSCreature::LoadCreature 0x00574180. GOG: LoadFromTemplate(CSWSCreature*,CResRef*,int) 0x0068b5a0, AddToArea 0x00681800.
- Route B target: `c_ithorian003` ("Greeter{Ithorian}", tag 204_Ith_Rec, x=-43.6 y=-27.5, app 72) in 204tel.mod; stage `probe_T12h/override/c_ithorian003.utc`; plan READY.
- Verify: `python3 scripts/run_tests.py k1_port` (8 suites); build: `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_probe_build.py --game-dir <game> --json`.
