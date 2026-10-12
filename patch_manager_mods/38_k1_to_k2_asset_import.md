# 38 — K1 → K2 NPC asset import: validated claims and pipeline

Status: research + tooling (10-11). Converter and texture stager built under `patch_manager_mods/k1_port/` (uncommitted, local-only). No install, no 2DA change, nothing tested in-game. Evidence (uncommitted): `.claude/review/k1_import/`.
Scope: validate a Gemini-web write-up on importing KotOR 1 NPC models, textures, 2DA rows, and voice into TSL, then pick an automated route.

## 1. Gemini claims — verdicts

| # | Claim | Verdict | Evidence |
|---|-------|---------|----------|
| 1 | K1 binary MDL must be converted for TSL | PARTIAL | TSL trimesh header is +8 bytes (340 vs 332). Raw drop-in never tested; no crash report found. T2, W2 |
| 2 | Facial bones and skeletal rigs differ | REFUTED | No head or neck bone-name difference in xoreos or PyKotor. T2 |
| 3 | `mdledit -ascii` and `mdledit -binary -k2` CLI | REFUTED | MDLEdit is GUI-only (master). `-k2` belongs to MDLOps. T1, T8 |
| 4 | Supermodel only needed during conversion | REFUTED | Supermodel is a 32-byte name the engine loads at runtime, recursively. T3 |
| 5 | TPC/TGA textures work unchanged | PARTIAL | TPC format identical across games. TGA identity unverified. T3 |
| 6 | Rename textures with a prefix, edit MDL strings in a hex editor | PARTIAL | Names cap at 31 chars + NUL, so prefixes must be short. Use ASCII round-trip, not hex. Untested. T3 |
| 7 | Override textures beat vanilla | VERIFIED | Override folder wins over chitin. T3 |
| 8 | Register models in `appearance.2da` and `heads.2da` | PARTIAL | `.utc` stores only `Appearance_Type`; head resolves via `normalhead` to `heads.2da`. Column lists community-sourced only. T4 |
| 9 | `.kpatch` appends rows to 2DA | REFUTED | In this repo `.kpatch` is a zip of manifest, DLL, hooks. No 2DA, GFF, or file ops. T9 |
| 10 | Edit `.utc` for Telos and existing saves update | PARTIAL | Fresh visits read the `.utc`. Saves hold full creature snapshots, so existing saves keep old data. TSL `Appearance_Type` serialization unverified. W4 |
| 11 | K1 voice `.wav` dropped into `StreamVoice` plays | REFUTED (as stated) | K1 voice is MP3-in-WAV in `streamwaves`. TSL needs a DLG subfolder. T5 |
| 12 | Type K1 `.wav` basename into a DLG ResRef field | REFUTED | DLG has no plain ResRef field (`Script` is a CResRef). Voice field is VO_ResRef. W1, T5 |
| 13 | `pykotor 2da2csv` and MDL read/write exist | VERIFIED | pykotor 2.3.12. T8 |
| 14 | `xoreos` ships unerf, unrim, gff2xml, xml2gff | VERIFIED | xoreos v0.0.6. T8 |
| 15 | Named mods (Ferc Kast heads, Malkior bridge, TSL Twi'lek pack, Seikan, N-DReW25) | PARTIAL | Ferc Kast VERIFIED. Malkior exact title REFUTED (closest: Texture-to-Game-Bridge). TSL Twi'lek VERIFIED. Seikan replaces one row, target game unclear. N-DReW25 has no faction rules. T6, T7, W5 |
| 16 | Effixian K1→K2 heads port ships converted files | PARTIAL | Ships TSLPatcher installers, not loose models. W5 |

Open license: no K1 NPC pack with an open license found. Leilukin (DeadlyStream #2179, "KotOR 1 Twi'lek Female NPC Diversity") allows use with credit. T10. #2242 is a different mod ("TSL Twi'lek Male NPC Diversity"); doc 34 lines 109 and 112 reuse that link for two other mods, so those two IDs stay unresolved. No conflict between #2179 and #2242.

## 2. pykotor K2 writer is broken (executed, 2.3.12)

- Control (K1 layout): a probe mesh writes 724 bytes.
- K2 probe (one dirt field set): `ValueError: _TrimeshHeader.write wrote 348 bytes, expected 340`.
- Cause: the K2 branch writes 18 bytes (10-byte dirt block + two `uint32` `k2_tail` fields). Engine delta is +8 (xoreos `skip(8)`, PyKotor's own `K2_SIZE = 340`, DeadlyStream "2 UInt32s"). The two extra fields are the 8-byte overshoot.
- Fix (scratch copy, diff at `.claude/review/k1_import/B2a_pk_fix.diff`): drop the two `k2_tail` writes and reads, keep the 10-byte dirt block. K2 minus K1 = 8 bytes for two node names (B2a, re-checked by hand).
- Reader on real TSL models (B2b): stock reader returns 0 vertices on the first mesh and denormal areas. Patched reader returns 24, 22, 68 vertices and plausible areas (p_bastilah, p_carthh, p_hk47). Stock misreads real K2 files.
- Writer fidelity (B3, B4): byte-identical round trip fails (writer drops unmodelled sections, output 696 to 6524 bytes shorter). Model-level round trip matches on all three models (node tree, vertex counts, textures, vertex extents). Not compared: normals, UVs, faces, controllers, walkmesh, dirt values (all inputs had default dirt).
- pykotor has no game parameter on binary write. K2 is chosen only when a mesh has non-default dirt or hologram values. Scratch copy adds a `force_game` hook (B3).
- Further writer defects found by the PoC (10-11, section 7; all stock 2.3.12 unless noted): (1) skin array pointers (bonemap, qbones, tbones) trail the arrays by 1532 B on N_CommM ArmL because `_calc_skin_offsets` ignores the writer's data order (T6d, fixed in the patch); (2) the writer synthesizes zero UV1 for textured meshes with none (11 on N_CommM), now opt-out (`synth_zero_uv`); (3) skin `unknown0` array is dropped (708 B on N_CommM), FIXED (T6h: reader stores it, writer emits it, K2 MDL = K1 + 8 per header exactly); (4) per-vertex tangent blocks dropped on 9 meshes of `llk_LenaH` (K1 rows are all zero): the patch keeps raw blocks, and the writer clears the 0x80 flag and omits the block when ALL rows are zero (T6f/T6i; `keep_zero_tangents` opts out); (5) skin MDX rows put bone indices at +0x20 and weights at +0x30, the reverse of K1 AND of all K2 vanilla skins: FIXED (T6h, weights first). The patch is now 19 hunks over `io_mdl.py` and `mdl_data.py`.
- Not proven: nothing here shows TSL loads the output. That needs an in-game test.
- Repro: `.claude/review/k1_import/probe_pykotor_k2_write.py`.

## 3. Candidate pipelines

1. **Option A — MDLOps `-k2` under Proton (BLOCKED).** `-k2` switch verified (A1). `mdlops.exe` not obtained: DeadlyStream returns 403, GitHub 404. Needs a user download. Binary-input behavior unknown. Needs Perl + Windows-only modules on Linux, or the compiled exe under Proton (A2: run recipe unverified).
2. **Option B — patched pykotor (BUILT, local).** `patch_manager_mods/k1_port/` holds the patch (6 hunks, 2 files), `setup_venv.sh`, `k1_port_model.py`, `k1_port_textures.py`, colocated tests (`run_tests.py k1_port`). Full K1→K2 path works on `n_commm` (match on nodes, UV, normals, faces, skin poses). `llk_LenaH` also matches. Still needed: skin `unknown0`, and an in-game test.
3. **Option C — manual MDLEdit GUI ASCII round trip.** Community-documented (Effix, 2025-09-01). Works, not automated.

2DA registration: `.kpatch` cannot do it (claim 9). Choose one:
- (a) TSLPatcher-style `changes.ini` (community standard, outside KPM).
- (b) Full `appearance.2da` override (conflicts with other appearance mods).
- (c) Runtime hook (the doc 34 approach), not row append.

Voice: skip for now. Drop-in path fails (claim 11). TLK vs VO_ResRef precedence unresolved (see open questions).

## 4. Open questions (ranked)

0. ANSWERED 10-11: K1 is installed (`swkotor`), so full K1→K2 conversion runs on real K1 models.
1. Does TSL load pykotor-patched output in-game? After K2v (10-11, `K2v_layout_evidence.md`, 1414 K2 and 968 K1 vanilla skins) the writer matches vanilla on skin row order (weights +32, bones +48), `unknown0` (preserved), and pointers. Residual risks: (a) UV-less textured meshes: KNOWN-TOLERATED (K2 `n_commm` has the same 11); (b) tangent flag cleared on all-zero rows (T6i), no vanilla precedent for flagged-zero so this is the conservative choice; (c) 59 vs 60 bonemap entries versus K2 `n_commm` (`freelookhook`); (d) supermodel `S_Female02/03` unreadable in pykotor but present in K2 (T2b); (e) K2 already ships `n_commm`, `n_commf`, `n_commkidm`, `n_commkidf`, `n_twilekf`, so converted models MUST be renamed (`k1p_*`, enforced by the tool); (f) real engine acceptance of the whole file is untested. Mitigations are planned in the plan file (MDLOps diff T7, loader recon, scratch-save probe). MDLOps cross-check needs user GO (T7).
2. Which wins, TLK sound or DLG VO_ResRef? Decompile `LoadDialog` (0x005a2ae0 per rakata) with `rea-recon`.
3. Does TSL serialize `Appearance_Type` into saves? (in-game save test or decompile)
4. Seikan 2.2 target game: K1 or TSL? Needs the archive readme (download, needs user OK).
5. Effixian contents: loose files or installer-only? Needs an archive listing (download, needs user OK).

## 5. Evidence

- Verdict files: `.claude/review/k1_import/T1..T10.md`, `W1..W5.md`.
- Wave 2 (Track B): `B2a_pk_fix.md` + `.diff`, `B2b_real_k2_read.md`, `B3_k2_roundtrip.md`, `B4_k2_semantic.md`.
- Tooling phase (10-11): `T2b_supermodels.md`, `T3b_pack_vs_vanilla.md`, `T6_poc_n_commm.md`, `T6a_uv_loss.md`, `T6d_skin_pointers.md`, `T6f_tangent_loss.md`, `T6h_skin_vanilla_layout.md`, `T6i_tangent_zero_rows.md`, `T6j_lena_ingest.md`; licences `L1_pack_licenses.md`; vanilla layout evidence `K2v_layout_evidence.md`; scripts in `scripts/` (still hardcode old scratch paths).
- Wave 1 (Track A): `A1_mdlops.md`, `A2_proton_run.md`.
- Probe: `.claude/review/k1_import/probe_pykotor_k2_write.py`.
- Scratch venv (outside repo): `pip install pykotor==2.3.12`.

## 6. K1 texture sources for the port

- Match key: the numbered file names the models reference (`N_CommF01`, `N_CommM04`, `P_CarthH01`). The 2DA stores base names. Local K1 read (K1c): 43 of 107 names found in the install (`.claude/review/k1_import/K1c_vanilla_names.md`).
- Conversion PoC (K1b): `p_carthh`, `p_bastilah`, `n_darthmalak` read from K1, written as K2, read back, MATCH on parsed content (node tree, vertex counts, textures, extents). Faces, UVs, normals, and controllers not yet compared.
- Candidate K1 texture packs. None states a license, so shipping any of them needs the author's permission:
  - HD Carth Onasi, DeadlyStream #1133: Carth skin and clothes. Direct match for the `p_carthh` PoC.
  - Redrob41 Character Textures, DeadlyStream #2659: "virtually all NPC textures," including aliens. K1 coverage 71 of 214 (Jan 2025), file names unverified.
  - Red11BY Ultimate Character Overhaul, Nexus #1282: AI upscale to 2048×2048 of every character. Nexus returns 403 to us, so contents are unverified.
  - Male NPC Clothing, DeadlyStream #2516: commoner clothing, renamed to `N_CommM08.tga` by the author. Shows the vanilla numbered naming.
  - Selphadur Redux, Nexus #1302: personal use only. Excluded from shipping.
- Neo Cities K1 build list: `kotor.neocities.org/modding/mod_builds/k1/spoiler-free` (about 190 mods). Source: `.claude/review/k1_import/R4_neo_cities.md`.
- Done (10-11): `Upscale+ Character Fixes - KotOR V0.52 (4x tpc).7z` matched. N_CommM, N_CommF, N_CommKidM, N_CzerkaOff keep vanilla topology and texture names; UV1 differs on 2-3 meshes per model (max 0.445 on N_CommKidM torso2); TPCs are 2048x2048 DXT5 vs vanilla 512x512 DXT1 (T3b). Readme: derivatives allowed, no redistribution without the author's permission, so everything stays local-only.
- Supermodels (T2b): `S_Female02`, `S_Female03`, `S_Male02` exist in K2 (chitin and Override). K1 animations missing in K2: 8 on S_Female02 and 1 on S_Male02 (f2a1..f4a3 combat family, K2 has suffixed variants); none on S_Female03. Low T-pose risk for commoners.
- PoC (T6): `n_commm` + pack `N_CommM04` as `k1p_commm04` converts with full match. Stage is `~/Games/KOTOR2_Mod_Workspace/k1_port_stage/` (outside the repo). Details in `T6_poc_n_commm.md`.

## 7. Phases (tooling + PoC, 10-11)

| Phase | Task | Status |
|-------|------|--------|
| T0 | Persist scratch evidence to `.claude/review/k1_import/scripts/` (scripts still hardcode scratch paths) | DONE |
| T1 | Patched pykotor patch + `setup_venv.sh` (`patch_manager_mods/k1_port/`) | DONE |
| T2 | Supermodel parity K1 vs K2 (`T2b_supermodels.md`) | DONE |
| T3 | Upscale+ pack models vs vanilla K1 (`T3b_pack_vs_vanilla.md`) | DONE |
| T4 | `k1_port_model.py` + tests | DONE |
| T5 | `k1_port_textures.py` + tests | DONE |
| T6 | PoC on `n_commm` (`T6_poc_n_commm.md`) | DONE |
| T7 | MDLOps cross-check under Proton (needs user GO) | DONE |
| T8 | 2DA integration into diversity patch (needs user decision) | DEFERRED |
| T9 | Licence registry, ledger, ingest driver, prune/gate (`sources.json`, `k1_port_sources/ingest/prune.py`, tests) | BUILT |
| T10 | Writer fidelity to vanilla K2 (skin order, unknown0, pointers, tangents) in the 19-hunk patch | BUILT |
| T11 | Ingest `n_commm` (vanilla, HD flagged) and `llk_LenaH/B` (credit) into the stage | BUILT |
| T12 | Risk mitigation: loader recon, MDLOps diff, scratch-save probe, converter hardening | BUILT |
| T13 | Commit `k1_port/` + doc 38 + evidence by explicit paths (needs user "commit") | DONE |
| T14 | Probe build: `k1_port_probe_build.py` makes stage `probe_T12h/override/{appearance.2da,c_ithorian003.utc}` (10-11 retarget: Ithorian Compound greeter in `204tel`, appearance 72 -> 812, literal name "K1 Probe Citizen"; test from `Game9`, 204tel unvisited; a save made inside 204tel would hold the old creature, so it cannot be used); probe tool gets save-slot backup; `plan` turns green | BUILT |
| T15 | Probe install (user GO, game closed; test NPC at x=-43.6 y=-27.5 in the Ithorian Compound, `npc_div_whatis.py -43.6 -27.5`): `k1_port_probe.py install --yes`, manifest verified | INSTALLED |
| T16 | Probe play test (user) + log check; KEEP or ROLLBACK; result into this doc and doc 34 item 10 | DONE |
| T17 | Spawn-hook RE (static, read-only, rea-recon): how `CreateObject`/`LoadFromTemplate`/`AddToArea` build and place a creature; safe call timing after area load from a save; how a save-loaded area avoids re-spawn; where the display name is read | DONE |
| T18 | Spawn-hook probe build (Route A2): KPM hook spawns the K1 probe NPC (UTC from override, literal name) once in `204tel` at the Ithorian Compound plant room, guarded against double spawn; heavy logging; host tests + build gate | BUILT |
| T19 | Spawn-hook install (user GO, hot-swap) + plant-room save play test (user) | INSTALLED |

## 8. Licence policy and tooling (10-11)

- Policy (user, 10-11): ingest packs that need the original author's approval FLAGGED for later removal; when several sources hold the same asset, prefer the one that needs no approval. Unknown licence counts as needing approval.
- Registry `patch_manager_mods/k1_port/sources.json` (metadata only): vanilla_k1, leilukin_k1_twilek_f_2179 (credit_required), upscale_plus_v0.52, effixians_k1_to_k2_heads, k1_heads_port_1_0_1 (permission_required), carth_onasi_ds1133, npc_clothing_m_ds2516, kotor2_to_kotor1_game_bridge, uco_redux_tpc_1282, characters_revisited_* (unknown). Evidence: `L1_pack_licenses.md`.
- Ledger `~/Games/KOTOR2_Mod_Workspace/k1_port_stage/ledger.json` (outside the repo) records every staged file: source, class, `removal_group` (`pending_author_approval` when flagged), sha256, credits.
- `k1_port_ingest.py` picks sources by that rule, stages approval-free assets under `override/` and flagged higher-quality variants under `_hd/override/` (same name, copy over on demand). `k1_port_prune.py list|prune|approve|gate|adopt|credits`: prune is dry-run by default; `gate PATH...` fails by sha256 match on flagged entries, run it before copying anything into the repo or a release.
- Staged so far (10 ledger rows, 1 flagged): `k1p_commm` (vanilla, 512 px; the 2048 Upscale+ variant flagged), `k1p_lenah`, `k1p_lenab` (Leilukin, credits Leilukin, SpaceAlex, DarthParametric).
- Known tooling gaps: variant-folder pick fixed in T12b (`--allow-variant-folders`; applies within one source only, cross-source ranking unchanged); `S_Female03`/`S_Female02` unreadable in pykotor until the `_get_node_order` fix (T12c, optional T12f); `scripts/*.py` evidence copies still hardcode scratch paths.

## 9. T12 results (10-11)

- **T12a** `k1_port_validate.py` (stdlib, raw layout checks): 0 findings on all 3237 vanilla K2 models; 0 on the 3 staged `k1p_*` models. T12g runs it inside ingest (finding = model rejected, `--skip-validate` for debugging).
- **T12b** ingest variant-folder penalty (`Optional*/`, `Upscale*/`) with `--allow-variant-folders`.
- **T12c** pykotor `_get_node_order` adds the 12-byte base twice (venv `io_mdl.py` L2460/L2470). Node tree is read correctly (49/49 sample), so converted output is NOT damaged; only K1 `s_female02/03` fail to load. Fix is 2-3 lines (`T12c_node_order.md`); T12f skipped until those models are needed as sources. Separate bug: K2 `505ond_fghtr` light-flare read.
- **T12d** `PERMISSION_REQUESTS.md`: drafts only, nothing sent. Open: does pushing converted Upscale+ textures into this repo count as redistribution (settle before sending).
- **T12e** engine loader questions 1-5 all UNSURE after two REA runs: the anchors are GOG addresses, Steam twins of CreateSkinNodeList/ExecuteSkinTransforms/HasTangentBases/CalculateTangentSpaceBasis/binary parsers were not found (Steam skinned vertex programs `0x00a050b8`, `0x00a08cb0` confirmed). The in-game probe is the empirical answer. Docs: `T12e_engine_loader.md`, `_v2.md`.
- **T12h** probe design (`T12h_probe_design.md`): append `appearance.2da` row 812 + `per_miner_002.utc` override (one NPC, `201tel`), test from the pre-Telos `Game9` save, back up AUTOSAVE, five-file rollback. Open: texture name derivation `k1p_commm` + 2-digit TextureVar.
- **T12i** `k1_port_probe.py plan|install --yes|rollback --yes|status` (validator + prune gate + game-not-running + foreign-file refusal + sha manifest). Real-stage `plan` is BLOCKED only on the unbuilt `probe_T12h/override/appearance.2da` + `per_miner_002.utc` (design doc section 3 build step).
- **T7a** MDLOps inputs exported (read-only) to `~/Games/KOTOR2_Mod_Workspace/mdlops_t7/inputs/` (n_commm, s_female02/01, s_male02/01); `T7a_inputs.md`. T7 itself waits for user GO.
- **T7** MDLOps cross-check done (user GO 10-11): see `.claude/review/k1_import/T7_mdlops_result.md`. Its K2 output validates clean with equal MDX strides; it re-welds vertices (41/44 meshes) and swaps 6 bonemap entries, so it is not a per-vertex reference. Engine acceptance still needs the in-game probe.

## 10. T17 spawn research and gate G1 (10-11, waves `.claude/waves/w2-38-T17/u01..u11`)

All 11 Haiku units PASS the contract check; all but u01/u02 are PARTIAL. Opus gate decisions:

- **Track S (script-only spawn, no DLL): GO.**
  - Compiler: `nwnnsscomp.exe` under Proton wine (`script_recompile_20260924/tools/compile.sh <out.ncs> <src.nss> [includes]`, vanilla sources in `tools/vanilla/` incl. `nwscript.nss`); compiled a 3-line script to a valid 23-byte `NCS V1.0B`; exit code is 0 even on errors, so success = non-empty `.ncs`.
  - API (vanilla `nwscript.nss`, re-checked by rg): `CreateObject(int, string, location, int=FALSE)`, `GetObjectByTag(string,int=0)`, `ExecuteScript`, `Location(vector,float)`, `Vector(x,y,z)`, `cos/sin`, `GetFirstPC`, `GetPosition`, `GetFacing`, `GetArea`. **K2 has no `GetLocalInt`/`SetLocalInt`, no `GetResRef`, no `GenerateNewLocation`** (use `GetLocalNumber`/`SetLocalNumber`, and gate on tag instead of area resref).
  - Original `k_def_heartbt01.nss` is a one-liner: `ExecuteScript("k_ai_master", OBJECT_SELF, KOTOR_DEFAULT_EVENT_ON_HEARTBEAT);` (includes `k_inc_switch`, `k_inc_debug`). So the wrapper ends with that same call; **no `k1p_hb_orig.ncs` copy is needed** (7 install files, not 8).
  - Conflicts (u02): neither `k_def_heartbt01` (in `Scripts.bif`) nor `k_204tel_enter` (in `Modules/204tel.mod`) exists in `override/` or in any installed mod: GO.
  - Saves (u04): the saved `204tel` is a nested archive holding only `204tel.git/.are/Module.ifo`, no scripts. Saved creatures store `ScriptHeartbeat = k_def_heartbt01` (ResRef) in 17 of 18 entries; the area OnEnter `k_204tel_enter` is in the saved `.are`. Script resolution is by name at run time, so an override `.ncs` runs inside an already-saved area. Unverified: whether a saved GIT field or the UTC template wins (irrelevant for the heartbeat route, both name `k_def_heartbt01`).
  - Trigger: wrapper over `k_def_heartbt01` gated on `GetTag(OBJECT_SELF) == "204_Ith_Rec"` (the Greeter) and `GetObjectByTag("k1p_probe_npc") == OBJECT_INVALID` (no local-number flag needed). Risk: the Greeter must be alive and heartbeating in the plant-room save.
- **Track H (native hook): NO-GO for now, research only.** R5 could not identify the ExecuteScript handler or script-run routine (stripped binary), so the hybrid hook has no entry point. Anchored Steam facts, all by REA (names are Ghidra FUN_, labels UNVERIFIED unless noted):
  - `0x0066f990` creature case: builds the object with `0x00561f30`, loads template `0x00572a70`, places via `0x00568bd0` (call at `0x0066fed8`); `0x00522bf0` is reached only from inside `0x00568bd0`; id from `0x0053dd20` (u07).
  - `0x00572a70` thiscall, 2 stack args, `ret 8`, prologue `55 8b ec 6a ff 68 1e 0e 95 00 64 a1`; `0x00568bd0` thiscall, 6 stack args (area*, x, y, z, int, int), `ret 0x18`, prologue `55 8b ec 6a ff 68 c3 0a 95 00 64 a1`; position/orientation setter candidates `0x00543f50` / `0x00543f10` (u08).
  - `0x005236b0` LoadArea `ret 4`, `this` in ECX; `0x00525f90` LoadCreatures `ret 16`; creatures are populated before LoadArea returns; only caller seen is `0x0055a585` (post-call hook point `0x0055a58a`); area resref offset unknown (u09).
  - `0x005632f0` GetFirstName reads stats+0x34; `0x00574500` SaveCreature writes FirstName and Tag via `0x006b3d10`; area save `0x00529dd0` calls it; whether a runtime-spawned creature enters the saved list is unverified (u10).
  - Hook skeleton (u05 + manual check): exports are `extern "C" __declspec(dllexport) void __cdecl Fn(void* ...)`, params mapped from `ecx` / `[esp+4]` / `ebp` in `kotor2-steam-aspyr.hooks.toml`; `test.toml` sections hold `cxxflags`. Toolchain: `i686-w64-mingw32-g++` present, x86_64 absent (u06).
  - Next RE if S fails: R5 rerun starting from the CSWVirtualMachineCommands vtable `0x009940d0` / InitCommands `0x00665f50`.

## 11. T19 play result: Track S spawn (user, 10-12 ~01:25, screenshot `~/Documents/Screenshot_2026-10-12_01-25-27.png`)

- **Spawn works with scripts only (no DLL).** From a `204tel` save, "K1 Probe Citizen" appeared via the `k_def_heartbt01` wrapper; the Greeter was still there; the clone has the Greeter's dialogue/text (UTC copy).
- **Persistence:** survives save + reload and leaving/re-entering the area (no duplicate reported). The spawned creature is therefore written into the saved area creature list (answers the R4 open question).
- **Name:** literal first name shows in the nameplate.
- **Head:** K1 head model renders correctly and its idle and conversation animations play.
- **Body: FAILS.** The `k1p_commm` body renders as a shredded cloud of white triangles (vertices exploding), not the pink-white/flat-grey texture failure of the plan's ladder, so this is geometry/skin, not the texture. Direction: skin bone map / bone weights / node order in the converted `k1p_commm.mdl/.mdx` (see T10 writer fidelity and T12c; Steam skin loader questions T12e are UNSURE).
- Spawn mechanism is now proven; the open work is the body model (T10), not placement.
- **Log swarm (Haiku x3, `.claude/waves/w4-38-T19/`)**: only `npc-diversity_log.txt` shows the NPC. Spawned creature `oid=0x8000026d` at (-21.1,-31.1): `RMP app=812>812 tv=1>1 why=nochg` (L1160, L1163, L1318), census `TEX app=812 cur=k1p_commm01` (L1161). **The engine asked for `k1p_commm01`, not the shipped `k1p_commm04`**, although the staged UTC has `TextureVar=4` (verified on the GFF; `BodyVariation=1`). So `CSWCCreature`'s textureVar (pDesc+1, `%s%02d`) is 1 for a runtime-created creature: the UTC TextureVar does not reach it (cause unresolved: CreateObject path vs LoadFromTemplate order, or the client desc default). A missing `k1p_commm01.tpc` explains the white shards; the exploded vertices may still be a separate skin problem. No other mod log, Proton log or crash address mentions the model. Options: (a) alias the texture as `k1p_commm01.tpc`, (b) have the dispatcher's `RemapNpcAppearance` write tv=4 for app 812 (npc-diversity session's files), (c) RE where the client desc textureVar is set for runtime spawns.
- **Option 1 applied (10-12 01:46, user pick):** ingested `N_CommM04` as `k1p_commm01.tpc` (`k1_port_ingest.py --resref n_commm --no-model --texture N_CommM04=k1p_commm01`, vanilla/BioWare, ledgered) and reinstalled as 7 files (`k1p_probe_20261012_0146`; previous Track S set rolled back first). Provenance answer: body `k1p_commm` + texture = K1 vanilla `N_CommM`/`N_CommM04`; head = K2 `normalhead=9` copied from appearance row 17 (not K1). Install file list: `probe_T12h/override/appearance.2da models/k1p_commm.mdl models/k1p_commm.mdx override/k1p_commm04.tpc override/k1p_commm01.tpc probe_spawn/override/k1p_probe_npc.utc probe_spawn/override/k_def_heartbt01.ncs`.
- **Alias test result (user, 10-12 14:04, `~/Documents/Screenshot_2026-10-12_14-04-13.png`):** body now textured (dark/brown/blue patches on the shards) but the geometry is still exploded. Texture bug confirmed and fixed; remaining fault is the body mesh (skin/vertex data or skeleton), head and name unaffected. Next discriminator: install the T7 MDLOps K2 output (`~/Games/KOTOR2_Mod_Workspace/mdlops_t7/mdl_run1/n_commm-ascii-k2-bin.mdl/.mdx`) as `k1p_commm.mdl/.mdx` and respawn (same appearance row); fine = our writer is at fault, same shards = K1 body vs K2 skeleton/engine issue.
- **A/B install (10-12 14:14, user GO):** `k1p_commm.mdl/.mdx` replaced by the T7 MDLOps K2 output (stage `probe_mdlops/models/`, 19 mixed-case name fields lowercased in place to pass the validator, adopted into the ledger as `vanilla_k1` model; everything else identical: manifest `k1p_probe_20261012_1414`, 7 files). Previous (pykotor-patched) model files remain in stage `models/`. Result pending: body fixed = our writer at fault; same shards = K1 skin vs K2 skeleton/loader.
- **A/B result (user, 10-12 14:31, `~/Documents/Screenshot_2026-10-12_14-31-21.png`): the MDLOps body renders correctly** (full textured body, robe, belt, gloves, head attached); one arm is held out at an angle (animation pose quirk). So the pykotor-patched writer output (`stage/models/k1p_commm.*`) is what explodes in game, not the K1 data or the spawn.
- **Static diff (ours vs MDLOps vs K2 vanilla `n_commm`, scratch `.../scratchpad/ab/`)**: parsed skin rows (weights/indices) for torso/arml/armr: 0 rows differ between ours and K2 vanilla; node positions/orientations/controller counts identical; trimesh headers equal except file-offset pointers; supermodel `S_Female02`, classification 4, anim scale 1.06 identical in all three. MDLOps differs from both in vertex welds (235/577 vs 228/587), bonemap swaps (0<->1, 9<->10 in torso/armr; likely the arm quirk) and face plane distances (ours writes 0 on face plane distance, MDLOps and K2 vanilla have real values: not a render issue but a writer defect). **The only structural difference between ours and both working models is the internal model name + root node name: `k1p_commm` (file offsets 0x14 and 0x1bc) vs `n_commm`.** Hypothesis: the engine keys skeleton/animation binding on the root/model name. Test staged: `probe_namefix/models/k1p_commm.mdl` = ours with only those 16 bytes changed to `n_commm` (ledgered); MDX unchanged. If it renders with a normal pose, fix = converter keeps the original internal root name (file resref still `k1p_*`), and fix face plane distance separately.
- **Name theory refuted (user, 10-12 ~14:50):** ours with internal model/root name `n_commm` still shards. **Root-cause candidate found by a name-aligned raw diff (ours vs K2 vanilla `n_commm`, all 44 trimesh + 3 skin headers):** everything is equal except file-offset pointers, face plane distance, and the skin header `bones[16]` int16 list (skin header = node+80+340 (K2 `TM_SIZE`); list at +0x40..+0x60). The first 14 entries are equal; **slots 14-15 hold K1 uninitialised junk in ours** (torso 48769, arml 16256, armr 18384/16148) where K2 vanilla and MDLOps have in-range node numbers (7/10, 0/0, 3/10). A K2 skinning loop over all 16 slots would index nodes out of range, which fits whole-mesh explosion (and MDLOps, which writes valid slots, renders). Writer fix to make in `pykotor_k2_mdl.patch`/`k1_port_model.py`: pad unused `bones[16]` slots like K2 (in-range; K2 vanilla pads with a valid node number or 0). Second, non-render defect: face plane distance written as 0. Stage test `probe_skinfix/models/k1p_commm.mdl` = ours with only those 3 x 32-byte lists replaced by K2 vanilla's (validator 0 findings, ledgered); texture/MDX/names unchanged.

## 12. Session pivot (user, 10-12 evening): native spawn RE, AI texture pipeline, skinfix test pending

- **Install state:** `probe_skinfix` model (ours + K2 vanilla `bones[16]` lists for torso/arml/armr; stage `probe_skinfix/models/`) installed 10-12 17:09, manifest `k1p_probe_20261012_1709`, 7 files. In-game result PENDING (user reports: normal body/arms, body OK with arm out, or still shards). If normal: fix the pykotor writer to pad unused `bones[16]` slots like K2 and the zero face plane distance, add tests, rebuild, record here.
- **AI texture pipeline research (done, 14 Haiku):** `.claude/research/texture_ai_pipeline/report.md` (commits 7c5ecad, cf19aee). Decisions: deterministic recolor/decal first, AI (Qwen-Image-Edit, Apache-2.0) second, always mask-composited, alpha preserved, gate demands dims/mips/format equal to the parent (resolution agnostic). **User decision 10-12: licence tags carry over to generated copies.** T3 DONE: `Ledger.add_derived` + `k1_port_prune.py derive` (inherits source_id/class/flag/credits/approval, parent_sha256 fail-closed, AI provenance fields, `local_only`). T1 (output gate), T2 (UV island mask tool) NOT started, wait for user go. Gemini Web feedback evaluated in report section 10 (4K claim UNVERIFIED, not an authorization).
- **Pivot decision (user, 10-12): keep the script spawn for texture-tooling tests; focus moves to reverse-engineering the engine to spawn characters natively.** Static, read-only RE only until a plan is approved; no live-file writes. Overlap: the npc-diversity session works on runtime injection too (doc 34, `area_hook_probe_src` POP-STABLE ~33 ticks on 7/7 loads); coordinate, do not duplicate hooks.
- **Native-spawn benefits over script (told to user):** no anchor NPC or shared `k_def_heartbt01` override, spawn at area load (no 6 s pop-in), direct control of appearance/TextureVar/name (runtime spawns get TextureVar 1), data-driven mass spawn, option to keep spawned NPCs out of saves. Costs: Steam-only addresses, crash risk, RE time.
- **RE wave `.claude/waves/w6-38-RE/` (launched 10-12, Haiku):** u01 TextureVar origin (LoadFromTemplate fields, SetAppearance 0x0076d200 callers), u02 AddToArea visibility chain + position/orientation setters, u03 area pointer/CResRef builders/simpler spawn wrapper, u04 save-list persistence (area+0x194, skip flags obj+0x11ac/0x11fc), u05 ExecuteScript via command table (index 8 = ExecuteScript, 243 = CreateObject sanity anchor; InitCommands 0x00665f50), u06 repo scout (KPM detour semantics, post-call hooks, hook overlap). Next: Opus gate, then plan mode for the next-phase implementation plan (Haiku-heavy).
