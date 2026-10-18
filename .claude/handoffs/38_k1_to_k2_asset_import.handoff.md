# Session Handoff & Plan State: patch_manager_mods/38_k1_to_k2_asset_import.md

## 0. Next-Phase Direction
T27 TextureVar RE done; force-tv request queued with npc-diversity

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/38_k1_to_k2_asset_import.md`
- **Waves:** .claude/waves/WI-38-a1/
- **Waves:** .claude/waves/WI-38-a2/
- **Waves:** .claude/waves/WI-38-a3/
- **Waves:** .claude/waves/w1-38-T12/
- **Waves:** .claude/waves/w1-38-T17/
- **Branch:** `KOTOR-II` (1fbcd9f0 docs(holster): HW-UX3 build round handoff)
- **Generated:** 2026-10-18 13:59:52 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
1fbcd9f0 docs(holster): HW-UX3 build round handoff
 .claude/handoffs/holster_editor_ux_v2.handoff.md | 8 ++++++++
 1 file changed, 8 insertions(+)
```
- **Dirty Files (173):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ... +163 more (run `git status --short`)

## 2. Immediate Objective (T27)
**Stage 3 roster, persistence policy, TextureVar set; mechanism lives in doc 40 / `npc_world_src` (doc 35 was merged into doc 40)**

Status: PARTIAL 10-18: RE DONE (SEC 25), MECHANISM = CLIENT DESCRIPTOR BYTE VIA `REMAPNPCAPPEARANCE 0X76D200` (NPC-DIVERSITY OWNS THE DETOUR); ROSTER + PERSISTENCE DECIDED; TEXTUREVAR SET PENDING OWNER DECISION (FORCE-TV BRANCH REQUEST TO NPC-DIVERSITY, OR ARMOR-ITEM ROUTE)
Task: Stage 3 roster, persistence policy, TextureVar set; mechanism lives in doc 40 / `npc_world_src` (doc 35 was merged into doc 40)

## 2b. Upcoming Batches in Pipeline
- **T8** [Testing & Contracts] : 2DA integration into diversity patch (needs user decision)
- **T9** [Testing & Contracts] : Licence registry, ledger, ingest driver, prune/gate (`sources.json`, `k1_port_sources/ingest/prune.py`, tests)
- **T10** [Testing & Contracts] : Writer fidelity to vanilla K2 (skin order, unknown0, pointers, tangents) in the 19-hunk patch

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/38_k1_to_k2_asset_import.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate T27` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

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

## 6. 10-16 Juhani session (self-contained; read this instead of the transcript)
### Outcome
Juhani renders correctly in K2 `204tel` (Cathar head with markings, CR textures, body good; left arm held out = MDLOps quirk). Doc 38 section 18 holds the full record (commit 054005a); doc 38 section 17 has the peer's Run 1b (no duplicates after save/reload, npc-world 0.1.1-dev).

### Live state (game dir `$K2` = `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II`)
- Probe install ACTIVE: manifest `$K2/backups/k1p_probe_20261016_0025/manifest.json`, 9 files in `override/`: `appearance.2da` (3665911e, row 818 appended), `heads.2da` (c5191ec0, row 249), `k1p_juhani.utc` (e15ecacc), MDLOps models `k1p_juhaniba` (5a0db3b4/7e69c046), `k1p_juhanibb` (eafe6d44/02dcc2b9), `k1p_juhanih` (cc804f80/4aa24bae). Base shas for rollback: appearance `44427a35...488b91`, heads `e0f8381c...fb9d`. Saves backed up in the manifest dir (do NOT save in 204tel while installed).
- 3 CR textures hand-copied (private test, NOT in the probe manifest): `override/k1p_juhaniba01.tga`, `k1p_juhanibb01.tga`, `k1p_juhanih01.tga`; list+shas in `~/Games/KOTOR2_Mod_Workspace/k1_port_stage/char_juhani_test/CR_MANUAL_COPY.txt`. Probe `rollback` does NOT delete them: `rm --` each by explicit path with `"${K2:?}"` first.
- Rollback = game closed; `rm` the 3 CR files; `python3 patch_manager_mods/k1_port/k1_port_probe.py rollback --game-dir "$K2" --yes`; verify base shas; `override/` keeps `k1p_native_npc.utc` (peer's, leave).
- DLL: k1-spawn-probe 0.3.2-probe LIVE (roster rows: `c_drdmouse01` pet at -39.0,-63.1; `k1p_native_npc` -21.1,-31.1; `k1p_juhani` -37.5,-63.1; all `204tel`, spawn_cap 4). Source commit 30984d7, KPM commit 1fd8d3a, hot-swap backup `k1-spawn-probe_swap_20261015_145104` (rollback: `kpm_hotswap.py --rollback k1-spawn-probe_swap_20261015_145104`). npc-world 0.1.1-dev and npc-world-probe untouched.
- Commits this session: 30984d7 (roster row + tests rows=3), fa66d8d (`demote_dangly` in `k1_port_model.py` + test), 054005a (doc 38 s18, PLAN.md HH note, texture_diversity handoff line). Not committed: nothing of mine (the w13-juhani agent outputs are written by Haiku agents; commit them explicitly by path after review).

### Key findings (details doc 38 s18)
1. pykotor cannot write danglymesh (0x121): placeholder constraints, no positions array. `demote_dangly` makes hair/cloth static plain trimesh in the pykotor path; MDLOps keeps valid dangly headers (positions ptr = constraints ptr + 4 x count, like K2 vanilla).
2. Displayed body is `modela` (modeltype B, no armor) = `k1p_juhaniba`; `bb` is only its supermodel + head chain.
3. pykotor-writer `k1p_juhaniba` shreds in game although EVERY static check equals K1 (bonemap, bones16, qbones/tbones, vertex weights, faces, positions, normals, UVs, node order; skeleton rest poses equal K2 `S_Female03`/`p_bastilabb`). Ruled out: bone count (K2 has 203 skins >= 16 bones), 2-level supermodel chains (K2 chains 5 deep), `supernode` values (arbitrary in vanilla), texture alpha, name theory. Cause OPEN. The K1 probe citizen (fixed pykotor `k1p_commm`) renders fine, so the writer is right for that model: do not assume it is broken everywhere.
4. MDLOps recipe works (doc 38 s18): work dir `~/Games/KOTOR2_Mod_Workspace/mdlops_t7/juhani_run/` (`pass1.sh`, `pass1_more.sh`, `pass2.sh`, `pass2_more.sh`, `work/`, `pass2/`); Proton prefix reused from `mdl_run1/prefix`; ASCII rename via word-boundary regex (a plain double replace produced `k1k1p_`). Stage copies: `~/Games/KOTOR2_Mod_Workspace/k1_port_stage/probe_mdlops_juhani/models/` (adopted into the ledger as `vanilla_k1`).
5. Wiring facts: modeltype B no armor -> body `modela`, texture `texa` + TextureVar (runtime spawns get TextureVar 1, so `k1p_juhaniba01`); head from heads.2da row 249 (`k1p_juhanih`), head texture `k1p_juhanih01`. Appearance 818/heads 249 are the live row counts +1 at build time (`--new-row` refuses a grown table).
6. Playtest history: (a) Ithorian-UTC override rig (rolled back); (b) hook spawn with pykotor models: shards + head artifacts; (c) +dangly demote: body still shards; (d) MDLOps body A: body good, head bad; (e) MDLOps ba+bb+h: all good, arm out.
7. User report: an Ithorian once spawned on the mouse droid and separated on click (likely a vanilla 204tel creature; check the GIT, doc 38 s17 O1).

### Research in flight (Haiku 5.5 recon wave `.claude/waves/w13-juhani/`, launched 10-16, read-only, 5 agents)
- u01 `u01_dangly_verify.md`: is MDLOps hair/cloth sway data intact vs K1 and K2 vanilla.
- u02 `u02_dangly_writer_design.md`: smallest pykotor-writer fix to keep dangly (design only).
- u03 `u03_arm_pose.md`: cause of the held-out left arm (bonemap/bones16 vs animation).
- u04 `u04_animations.md`: animation names K1 vs K2 chain vs our models; missing ones.
- u05 `u05_audio_needs.md`: audio inventory + minimal test plan for Juhani (ssf, DLG `k_hjuh_dialog`, VO, lip).
Contract: `python3 scripts/agent_contract.py check --scope '.claude/waves/w13-juhani/*' -` after the agents return (snapshot taken at launch). Results are NOT yet merged into docs: on return, summarize each in doc 38 new section 19 (research), then update this handoff and the memory file `kotor2-k1-asset-import.md`.

### Audio facts already known (doc 41 s1-2, doc 38 claims 11/12)
K1 VO wavs are MP3 data in a WAV container in `streamwaves`; K2 StreamVoice uses a 58-byte header + MPEG-1 L3 32 kHz mono 48 kbps and a per-dialog subfolder; dropping K1 wavs in `StreamVoice` does not play (claim 11 refuted). Open: does K2 read voice from `Override` (E1/E2), TLK SoundResRef vs DLG VO_ResRef precedence, `.lip` strictness (E3). K1 Juhani: `k_hjuh_dialog` 1,272 entries+replies, 850 VO refs, 533 wavs, soundset 29 wavs; current UTC soundset is generic female 57. Dialog scripts/plot flags/TLK strrefs are K1-specific (inline text needed). Private use of K1 audio OK, public NO (doc 41 s5).

### Open items after the research lands
a) restore dangly sway (MDLOps route already keeps it: confirm in game; pykotor route needs the writer fix), b) left-arm pose, c) why pykotor shreds `p_juhaniba`, d) Ithorian-on-droid overlap, e) audio ladder E1-E3 then a 3-line Juhani DLG, f) Carth (same rig, chained tables 819/250), g) T27 roster/persistence policy (npc-diversity/doc 40), h) CR author permission (user sends), i) rollback of this rig when testing ends, j) decide whether the converter should default to the MDLOps route for skinned K1 bodies.

### Resume
`python3 scripts/plan_manager.py resume patch_manager_mods/38_k1_to_k2_asset_import.md` (never bare). Next dev action: read the five `w13-juhani/u0*.md` findings, then plan mode.

### 10-16 research wave RESULTS (merged into doc 38 section 19, commit follows 7f8580f; raw reports are local-only in gitignored `.claude/waves/w13-juhani/u01..u05_*.md`)
u01 dangly CONFIRMED (MDLOps keeps sway data); u02 pykotor dangly writer fix feasible (~30 lines: reader seeks constraints at ptr+12, skips positions; writer omits positions, zero positions ptr); u03 left arm PARTIAL (lead: my rename lowercased node names such as `lbicepL_g`; test one build keeping K1 casing); u04 animations CONFIRMED (idle/walk/run covered; `cpause1/2`, `cwalk`, `crun` missing in the K2 chain, also in vanilla); u05 audio PARTIAL (UTC Conversation is `204ithr` and soundset 57; fix to `k_hjuh_dialog`/new row; minimal plan in `u05_audio_needs.md` s4; tests E3, E4, E2d first).
