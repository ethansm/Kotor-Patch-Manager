# Session Handoff & Plan State: patch_manager_mods/38_k1_to_k2_asset_import.md

## 0. Next-Phase Direction
T34 harvest 0.2.0 (needs user 'played') then T35 surgical install (user GO, game closed): backup, build 3 kpatches, edit config copy, nwmove_check_config.py, copy DLLs, rollback proven

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/38_k1_to_k2_asset_import.md`
- **Waves:** .claude/waves/w1-38-T12/
- **Waves:** .claude/waves/w1-38-T17/
- **Waves:** .claude/waves/w1-38-trackA/
- **Waves:** .claude/waves/w10-38-audio/
- **Waves:** .claude/waves/w11-38-alienvo/
- **Branch:** `KOTOR-II` (6153858 fix(k1_port): recompute face plane distances, zero bones[16] padding, golden n_commm test)
- **Generated:** 2026-10-14 00:46:24 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
6153858 fix(k1_port): recompute face plane distances, zero bones[16] padding, golden n_commm test
 patch_manager_mods/k1_port/k1_port_model.py      |  56 +++++++++++
 patch_manager_mods/k1_port/k1_port_validate.py   |   2 +-
 patch_manager_mods/k1_port/test_k1_port_model.py | 123 +++++++++++++++++++++++
 3 files changed, 180 insertions(+), 1 deletion(-)
```
- **Dirty Files (155):**
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
  - ... +145 more (run `git status --short`)

## 2. Immediate Objective (T34)
**Harvest the live 0.2.0 play test (tag path, TextureVar/BodyVariation offsets, `+0x11AC/+0x11FC`, duplicate-on-reload)**

Status: TODO
Task: Harvest the live 0.2.0 play test (tag path, TextureVar/BodyVariation offsets, `+0x11AC/+0x11FC`, duplicate-on-reload)

## 2b. Upcoming Batches in Pipeline
- **T8** [Testing & Contracts] : 2DA integration into diversity patch (needs user decision)
- **T9** [Testing & Contracts] : Licence registry, ledger, ingest driver, prune/gate (`sources.json`, `k1_port_sources/ingest/prune.py`, tests)
- **T10** [Testing & Contracts] : Writer fidelity to vanilla K2 (skin order, unknown0, pointers, tangents) in the 19-hunk patch

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/38_k1_to_k2_asset_import.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate T34` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

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

### 10-12 night: diversity swarm w8-38-DIVA + Characters Revisited (CR) discovery
- Proposal: `.claude/waves/w8-38-DIVA/PROPOSAL.md` (11 Haiku units u01-u11; inventories `.claude/waves/w8-38-DIV/inv/`, tools `.claude/waves/w8-38-DIV/tools/`). Findings: K2 install is a superset of K1 vanilla NPC content; K1 "new" rows (N_CommMD/FD/FC, N_TwilekFA-FC) reuse textures K2 already has; k1p_commm mesh port adds nothing; all K1 species models exist in K2 (unused by K2 rows: n_trandoshan, l_twilekf).
- **CR main pack was missed by our registry** (only its Creatures and Updates addons are in sources.json). Archives: `~/Downloads/Characters Revisited-1639-1-0-1715980455.rar` (3.3 GB, unrar only: no 7z/bsdtar), `...Creatures_Addon...rar`, `2_Characters Revisited_Updates...rar`. File lists + sizes: `.claude/waves/w9-38-CR/cr_files.tsv`, classified: `cr_classified.tsv` (`classify.py`, run with `.venv/bin/python`). TGAs are 2048x2048 RGBA (16.7 MB each): main pack 9.3 GB uncompressed, so extract in shards and delete TGAs after previews.
- Main pack TGA families: head_player 151, body_player 111, species 70, portraits 169 (po_, not game textures), faction_npc 63, commoner/head 38, items 90, creature/droid 32 (+40 addon), party 29, other 63; 20 MDL (n_calonord, n_darthmalak, n_dodonna, n_repsold_f, comm_a_f2/comm_a_m/comm_w_f heads, pfhb03/04, pfhc01, pmhc01, uniques). Nearly every name already exists in K1/K2: CR overwrites rather than adds.
- Peer session owns `.claude/research/texture_diversity/` (add-on catalog, procedural/AI decals; 32 cards need species bodies). Coordinate; do not edit. CR species skins feed those species-locked cards.
- Licence: Laast, no licence text seen (R2 P5); treat as needs-approval, local-only, flag in sources.json.
- **CR analysis DONE (10-12/13), report `.claude/research/k1_cr/REPORT.md`.** `sources.json` has `characters_revisited_main` (flagged); unsent Laast draft in `PERMISSION_REQUESTS.md`. Tools `k1_port_cr_extract.py` + `k1_port_cr_sheets.py` (+tests, `run_tests.py k1_port_cr`). Extracted to `~/Games/KOTOR2_Mod_Workspace/k1_port_extract/characters_revisited/` (10 shards, 1,117 manifest rows, ~9 GB, outside repo). Data in `.claude/waves/w9-38-CR/` (`cr_vs_base_*.tsv`, `cr_models.tsv`, `cr_catalog.tsv`, `cr_candidates.tsv`, sheets); recon outputs `.claude/waves/w9-38-CRA/` (gitignored).
- Findings: CR = upscaled K1 originals, not new designs (K2 re-did many same-named textures, so "REDESIGN vs K2" = K1 look vs K2 look). Only 13 CR TGA names exist in neither game (n_commf33, n_smuggler A/B/C 01-04). All species names exist in K2: replacement only, no extra species slots. All 21 CR MDLs overwrite same-resref K2 models. Heads pfhb03/04, pfhc01, pmhc01 ship a K1-layout MDL (self-consistent set; K2 rows 112-117,211-212,30,121-123,26,31,166-168,316); other CR heads have no MDL, so same-name textures mis-map on K2 meshes (layout corr 0.15-0.66). CR head textures have black eye sockets. True CR-original look: n_twilekfb01 (layout corr 0.84, safe). Next: Track A (`/plan-handoff 38:T-trackA`): renamed head-model set + smuggler A/B/C slots; or Tier A `kAlienTable` probe. Wave cost: 40 Haiku agents $0.90; 32/38 recon units exceeded 60k ctx (use <=4 sheets per unit).
- **Track A evidence (10-13, wave `.claude/waves/w1-38-trackA/`, u01-u04 facts):** "Track A" = DIVA PROPOSAL Track A (zero-import rows), NOT the CR head set. The six Track A textures (n_commmd01, n_commfd01, n_commfc01, n_twilekfa01/fb01/fc01) are pixel-identical in K1 and K2 and already in the K2 pack; CR copies are hi-res (n_twilekfb01 CR adds a green jacket: do not overwrite). K2 already has the K1 rows (ids 47,270,366,325,426,339-341,345,346 same ids). TRAPS: (1) PROPOSAL says "rows 816+" but the header declares only 10 rows 802-811 (`DiversityRows.h:9-18`, APP_COUNT=812); the live `override/appearance.2da` has 813 rows and row 812 = doc 38's `k1p_commm` probe row, so new rows start at 813 unless the probe is rolled back; (2) `DiversityRows.h`/`DiversitySlots.h` are generated ("DO NOT EDIT"): edit `scripts/generate_diversity_2da.py` spec list (`DIVERSITY_APPEARANCES`, lines 272-371, row = `{label, base, overrides}`; id = append position; app_first/app_count are caller parameters, caller at ~595+ not read), regenerate rebuilds the table so row 812 would be dropped; (3) tests: `diversity_tables_test.cpp:40-46` loops `app=802..815` (extend), `diversity_logic_test.cpp:79-80` hard-codes 805-811 variant ids, goldens :156/:158 depend on family allowlists; run `python3 scripts/run_tests.py diversity_tables_test` / `diversity_logic_test`; (4) doc 34 Phase 3 play test pending; install needs user GO.

### 10-13/14 probe->client refactor (T29-T36; session refactor-spawn-probe-npc-world)
- Built, uncommitted: `patch_manager_mods/k1_spawn_probe_src/` (K1SpawnProbe.cpp 0.3.0-probe, ClientLogic.h, spawn_logic_test.cpp; SpawnLogic.h deleted). `raw_addresses.toml`: NpcWorldProvider.cpp block replaces the k1 block (only our hunk). Peer `plan-npc-variety-build-order` dropped `k1-spawn-probe` from both manifests' conflicts (fe12c4e) and will use ini stages for the dual run.
- Reviews: `.claude/waves/w12-38-nwmove/{R1,R2,R3,OPUS_REVIEW}.md`. Opus verdict: no BLOCKER; HIGH-1 ini matrix (stages=persist,capacity,frame with no capacity_n spawns nothing), HIGH-2 the ONLY copy of 0.2.0 source is `$K/Patches/K1SpawnProbe/K1SpawnProbe.cpp` (kpatch holds no source): `cp -a` it before mirroring, HIGH-3 surgical edit must be one atomic step (new k1 DLL + old config aborts every later block; npc-world hooks added without deleting k1's three aborts too).
- Gate: GATE FAIL 272/275, only `check-raw-addresses` (voice_probe_src/VoiceProbe.cpp:5 0x0068EA70) and `kvoice/test_kvoice_lip.py` fail; both are the voice session's, independently confirmed. Do not call them pre-existing in a commit message without that evidence.
- Do NOT: install anything or touch `$G` without an explicit user GO and the game closed; `check_shared_headers.py --sync`; edit `npc_world_src/` or `npc_world_probe_src/`; skip the R3 step-6 offline dry run if the full-reinstall route is chosen; run the full gate repeatedly (140 s, shared tree).
- Verify: `python3 scripts/run_tests.py -k spawn_logic`; `python3 scripts/build_check.py --mod k1_spawn_probe_src --quiet`; peers `-k nw_`, `-k npcworld`, `-k nwprobe`.
