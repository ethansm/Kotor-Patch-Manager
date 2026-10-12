# AI texture pipeline for KOTOR 2 characters (recolor, accessories, logos)

Provenance: (uNN) = wave file in `.claude/waves/w5-38-texai/`, with verifier overrides applied (u11 over u01/u03, u12 over u02/u05, u13 over u04/u06, u14 over u07, u15 over u08). (orch) = orchestrator fact, treated as verified. UNVERIFIED = no source in the wave files. Combined by a Haiku agent, saved by the orchestrator. No repo code was changed.

## 1. Bottom line
1. Stage 1 is deterministic (HSV recolor, decal composite) inside UV-island masks. Stage 2 is AI: Qwen-Image-Edit, the only candidate with a sourced Apache-2.0 licence (u14 V1). AI output is always composited back through the mask.
2. First POC: recolor the torso island of a copy of `k1p_commm04.tpc` (512x512 DXT1, opaque, 10 mips, no TXI; u08 Q1, u15 V1). No AI model is needed.
3. No AI output may ship yet. No AI licence rule exists (u12 ADDED 1), and the gate passes a changed-content copy of a flagged asset (u12 ADDED 3).
4. Keep AI-derived files local-only, outside the repo, until `k1_port_prune.py gate` passes and the user signs off (u02 facts 6, 8, 10).
5. The k1p_commm atlas covers torso, arms and legs only; the head is a separate model (u08 Q2). The V-flip and the TextureVar-1 root cause are open. The in-game colour test is the user's.

## 2. Pipeline (numbered stages)

```
1. Extract   source TPC -> RGBA PNG, mip 0 only (read_tpc + _tpc_to_image,
             scripts/npc_div_texture_sheets.py:140-156) (u04, u13 VERIFIED)
             Orientation: sheets tool flips TPC (:144); pykotor does not (u15 C3).
             Use one config flag.
2. Mask      UV-island mask: label per triangle by mesh node and centroid;
             drop the 16 out-of-range nodes (u08 Q2, u15 rules 1-2)
3. Edit A    deterministic: HSV shift under mask; decal composite inside
             islands eroded ~8 px at 512 (u08 Q3, design only, not run)
4. Edit B    AI, stage 2 only: Qwen-Image-Edit on the crop. No mask input,
             so it can change any pixel (u14 V2)
5. Composite original pixels outside the mask restored; feather mask edge 2-4 px
             (u08 Q3). Seam benefit is UNVERIFIED (u14 item 12).
6. Encode    DXT1 TPC or TGA, source extension kept (u11 ADDED).
             Writers exist in the pykotor venv (u08 Q3); row-order round trip OK (u15 C3).
7. Gate      dims, mip count, format, alpha, TXI absent, name rule.
             NOT BUILT (section 4, gap 1)
8. Ledger    source, class, removal_group, sha256, credits (u12 CORRECTED 2)
             plus AI fields (section 6)
9. Stage     k1_port_ingest.py --texture OLD=NEW, output outside repo (u13 :477; u02 fact 6)
10. Install  k1_port_probe.py plan, then install --game-dir <GAME> --yes, game closed
             (u13 corrected steps 2-3)
11. In-game  user loads the 204tel save and judges the screenshot; rollback via
             k1_port_probe.py rollback (u06 sections 1, 3, 6)
```

## 3. Reuse list (already in repo)

All `k1_port` paths are under `patch_manager_mods/k1_port/`.

| Asset | Location | Use | Source |
|---|---|---|---|
| Source lookup, TPC first, TGA fallback | k1_port_textures.py:48, :150-173 | find input | u01, u11 |
| Read + dims check (w>0, h>0), format | k1_port_textures.py:274-288, :406 | input check | u11 CORRECTED |
| Name rule: max 31 chars, `[a-z0-9_]+` | k1_port_textures.py:46-47 | name gate | u01, u11 |
| All-or-nothing staging, exit codes 0/1/2 | k1_port_textures.py:14, :19-21 | stage safety | u11 |
| Licence classes, approval set | sources.json:3-10; k1_port_sources.py:35 | policy | u12 |
| Ledger add() fields | k1_port_sources.py:231-244 | ledger | u12 CORRECTED 2 |
| Gate: sha256 match, dry-run prune | k1_port_prune.py:243-279, :12 | gate | u12 |
| Probe: plan/install/rollback/status, --yes, game-running check | k1_port_probe.py:65, :681, :684, :363 | install | u13 |
| Ingest: --texture (required), --no-model, --skip-validate | k1_port_ingest.py:477, :487, :491 | stage | u13 |
| TPC/TGA to RGBA, contact sheets, masks | scripts/npc_div_texture_sheets.py:140-156, :111-125, :62-87 | preview, prescreen | u04, u13 |
| Engine name rule: texa + %02d, TPC before TGA, fallback 01 | patch_manager_mods/34_npc_diversity_dispatcher.md:47-66 | name rule | u12 ADDED 4 |
| Scratch probes: island masks, TPC row-order round trip | .claude/waves/w5-38-texai/scratch/u15_probe2.py, u15_tpc_roundtrip.py | prototype (not repo) | u15 |

## 4. What is missing (gaps)

1. **Output gate.** Repo checks only w>0 and h>0 (k1_port_textures.py:274-288, u11). No power-of-two, mip, alpha or TXI rule in the scripts (u03, u11 rg). TXI and DDS are not input types (u01 gaps 1-2). Needed: dims and mip count equal source (512x512, 10 mips; u08 Q1), format DXT1, no 1-bit alpha blocks (u08 Q1, Q3), no TXI sidecar, name rule.
2. **UV island mask tool.** Scratch code only (u08, u15). Needed: ArmL and ArmR nodes as one island, 39,847 texels, 15.2% (u15 C2; u08's 23.6% wrongly adds 8.4% of torso-node faces). Torso and legs from torso-node faces by centroid, legs z<0.80 (u08 Q2, not re-checked in u15). Exclude the 16 out-of-range nodes (max |u| 13.58; u15 C1, rule 2).
3. **AI provenance ledger fields.** Ledger row has no tool, model or licence field (u02 gap; u12 ADDED 1). See section 6.
4. **V-flip.** Pykotor reads TPC unflipped, top-first (u15 C3, A5a). Sheets tool flips TPC (npc_div_texture_sheets.py:144; u13 VERIFIED) and not TGA (:150). Engine convention open (u15 OPEN). Needed: one config flag, set only after the in-game colour test (u15 rule 4).
5. **Name wiring rule.** Engine name = texa + %02d of TextureVar. A missing file falls back to 01 silently (u12 ADDED 4). A runtime spawn gets TextureVar 1 although the UTC says 4 (orch). The working fix is a copy of the desired texture under the requested name (orch). The set needs both `k1p_commm04` and `k1p_commm01` (u06 section 3), but the ingest command in u13 step 1 makes only one; how the second file is made is UNVERIFIED. No validator checks k1p_ names: NEW_PREFIX is `npcadd_` (check_npc_add.py:60; u13). check_diversity_assets.py checks rows 802-811 and lists 812-815 as known-broken (check_diversity_assets.py:13-15, :81; u11). Appearance row 812 is a copy of row 17 (orch).

## 5. AI method options (ranked)

Hardware: RTX 5070 12 GB (orch). No timed run exists for any method (u07 OPEN, u14 OPEN 1). Ranked by fit, with licence as the gate.

| Rank | Method | Licence | VRAM / time evidence | Fit |
|---|---|---|---|---|
| 1 | Qwen-Image-Edit (20B), instruction edit | Apache-2.0, card metadata (u14 V1) | No VRAM or timing on card (u14 V2). Q4 GGUF file 11.9 GB; ComfyUI can spill to RAM (u14 ADDED 1, snippet). s/image UNVERIFIED | Only sourced Apache-2.0 option. No mask input, so composite is mandatory (u14 V2) |
| 2 | FLUX.1 Fill dev, native mask | FLUX dev non-commercial, gated (u14 V3) | No VRAM on card (u14 V4). 12 GB claim UNVERIFIED (u07). Stable Diffusion Art says 24 GB; conflicts (u14 ADDED 4) | Only native mask input; polarity not stated (u14 V4). Not shippable |
| 3 | FLUX.1 Kontext dev, instruction edit | FLUX dev non-commercial (u07; not re-checked in u14) | No mask; "minimal drift", not a region lock (u07). FP8 fits 12 GB, title-only source (u14 ADDED 6, UNVERIFIED) | Drift on untouched islands. Not shippable |
| 4 | SDXL inpainting 0.1, img2img at low denoise | CreativeML Open RAIL++-M; card says "research purposes only" (u14 V5). Commercial not cleared (inference, u14 CORRECTED 1) | Example strength 0.99, 15-30 steps (u14 V5). VRAM UNVERIFIED (u07). White = inpaint per fal docs, secondary (u14 ADDED 9) | Cheap, but licence blocks ship. POC only |
| 5 | Texture-projection tools (StableProjectorz, FRAMANENAI) | UNVERIFIED (u07) | UNVERIFIED | Not evaluated; sources are a bookmark page and a store page (u07) |

Not ranked: FLUX ControlNet Union Pro (no inpaint mode per search snippet; licence UNVERIFIED, u07). Kontext LoRA (overkill for one atlas, u07).

Arithmetic (inference, u14 ADDED 3): 20B at FP8 is about 20 GB of weights, so offload is needed on 12 GB. A Q4-class file of about 11.9 GB fits that picture.

Stage 1, deterministic, no model (so no model licence):

| Method | Evidence | Fit |
|---|---|---|
| S1 HSV shift under mask (recolor) | Design only, not run (u08 Q3). Decode mip 0 only (u04) | POC first |
| S2 Decal alpha composite inside eroded island | Design only (u08 Q3) | Accessories and logos; POC second |

## 6. Licence and provenance rule (DECIDED by the user 10-12: licence tags carry over to generated copies)

Existing rules: approval needed for permission_required, personal_use_only and unknown (k1_port_sources.py:35; u12 CORRECTED 3). Upscale+ derivatives allowed if free and credited (sources.json:49; u12 ADDED 2). Local-only, and gate before copy (u02 facts 6, 8, 10). No AI rule exists (u12 ADDED 1).

PROPOSAL, for user decision (T4):
1. An AI-modified copy inherits its source's class and removal_group. A source that needs approval makes the copy need approval too (u02 INFERENCE, not in policy).
2. The model licence also applies. A non-commercial or research-only model makes the output not shippable without user approval. Qwen-Image-Edit (Apache-2.0) is the only candidate that avoids that step (u14 V1).
3. Ledger gains: ai_tool, model_id, model_licence, parent_sha256, mask_sha256, seed, ai_pixel_fraction. parent_sha256 links the row to its source row.
4. Gate change: a row whose parent_sha256 is flagged blocks even when its own sha256 differs. Today a content change only prints a WARNING (k1_port_prune.py:276-278; u12 ADDED 3).
5. Credits: derivatives of credit_required or Upscale+ sources keep their credits (sources.json:49; u12 ADDED 2). All AI outputs stay local-only until the gate passes and the user signs off.

## 7. In-game test loop (corrected flags)

Run from repo root. `<GAME>` = Steam KOTOR II install dir. Game must be closed for install and rollback (u13 :363). Roll back the previous set before a new install (u06 :162-163).

1. Ingest (agent): `python3 patch_manager_mods/k1_port/k1_port_ingest.py --resref <r> --new-resref k1p_<name> --no-model --texture <K1_TEX>=k1p_<name>01` (u13 corrected step 1). Without `--new-resref`, the default is `k1p_<resref>`, the wrong set (u13 :59, :74). The `k1p_<name>04` file is also needed; its command is UNVERIFIED.
2. Preview (agent): `python3 patch_manager_mods/k1_port/k1_port_probe.py plan --game-dir <GAME>` (u13 corrected step 2). Without `--game-dir`, override/ is not checked (u13 :366).
3. Install (agent, game closed): `python3 patch_manager_mods/k1_port/k1_port_probe.py install --game-dir <GAME> --yes` (u13 corrected step 3; :681, :684).
4. USER: load the 204tel save and wait for the heartbeat spawn (u06 step 4; doc38:144, :155).
5. Census (agent, after user play): `python3 scripts/kotor2_logs.py check <GAME>/npc-diversity_log.txt --new`. Confirm `TEX app=812 cur=k1p_<name>01` (emitter `patch_manager_mods/npc_diversity_src/NpcDivLog.h:263`; orch).
6. USER: judge the screenshot in `~/Documents/Screenshot_<date>_<time>.png` (u06 :153). Symptom ladder: pink-white or grey = texture missing; white shards = k1p_commm01.tpc absent; textured shards = texture bound, geometry broken; full textured body = pass (u06 section 1, UNVERIFIED per u13).
7. Bad: `python3 patch_manager_mods/k1_port/k1_port_probe.py rollback --game-dir <GAME> --yes` (u13 corrected step 6). Good: keep manifest `k1p_probe_<YYYYMMDD_HHMM>` (u06 :162). Rollback does not restore saves; restore by hand (u13 :553).

Note: the install tool takes `--files` for a custom file set (see doc 38 §11 command pattern); the default file set is the original probe set.

## 8. Risks and open questions (max 6)

1. **AI derivative licence has no rule.** Resolve: user accepts, changes or rejects section 6. Owner: user.
2. **Gate passes a changed-content copy of a flagged asset** (k1_port_prune.py:276-278; u12 ADDED 3). Resolve: parent_sha256 gate rule (T3). Owner: agent, after section 6 is decided.
3. **TextureVar-1 root cause unknown; only the alias works** (u12 OPEN 2; orch). Resolve: find which code path sets pDesc+1 for runtime creatures; doc34 Strategy B gate is open (doc34:276; u12 ADDED 6). Owner: agent (read-only rea-recon on swkotor2.exe).
4. **V convention unknown** (u15 OPEN). Resolve: three-colour mask test on a copy of k1p_commm04 (T5). Owner: user (play test).
5. **No measured VRAM or s/image for any AI model on the 5070; FLUX VRAM sources conflict** (u07 OPEN; u14 OPEN 1; u14 ADDED 4). Resolve: timed local Qwen-Image-Edit Q4 run on a 512 crop (T6). Owner: agent; user only if gated FLUX is tested.
6. **512 px quality and seams for AI edits untested** (u14: "Verdict for 512: UNVERIFIED"). Resolve: AI edit on the torso crop, composite back, judge the seam in the colour test. Owner: agent (crop); user (in-game look).

## 9. Proposed work items T-series (max 6)

- T1 (agent): output gate script: dims, mip count, DXT1, alpha, TXI absent, name rule; tests colocated with code (gap 1).
- T2 (agent): move island-mask code from scratch/u15_probe2.py into `k1_port/` with tests; arms = ArmL+ArmR nodes only (gap 2).
- T3 (agent) DONE 10-12: `Ledger.add_derived` + `k1_port_prune.py derive` (copy inherits source_id, class, flag, credits, approval; parent_sha256 fail-closed; AI fields tool/model_id/model_licence/seed/mask_sha256/ai_pixel_fraction; local_only). Gate needs no change: the copy is a ledger row, so the sha256 gate blocks it. Was: add AI fields to ledger (k1_port_sources.py:231-244) and parent_sha256 gate rule (k1_port_prune.py:243-279), after T4.
- T4 (user) DONE 10-12: accepted. Was: accept, change or reject the section 6 licence proposal.
- T5 (user): in-game V-flip test: torso, legs and arm masks painted in three colours on a copy of k1p_commm04.
- T6 (agent): POC: torso recolor on a copy of k1p_commm04; stage the 04 and 01 pair; gate; install only after user go (game closed); then the Qwen timing run.

## 10. External review (Gemini Web, 10-12) and our verdict

| Gemini point | Verdict |
|---|---|
| Loose TPC/TGA in override/ needs no 2DA rebuild | Consistent with our probes (alias texture worked). No change. |
| 4K community mod (Sept 2026) may put 4096 textures in the source dirs | UNVERIFIED, no source given. Design is resolution-agnostic anyway: T1 gate demands output dims, mip count and format EQUAL TO THE PARENT (not a fixed 512); erosion/feather scale as width/512 (8 px at 512, 64 px at 4096). Gemini option A. Option B (reject non-512) not needed. |
| Gate lives in k1_port_textures.py | Wrong file: that is the stager. T1 is a new gate script that reuses its readers. |
| Preserve alpha; AI touches RGB only | Agreed. T1/T2 rule: output alpha identical to parent alpha; composite RGB only. Our current atlas is opaque DXT1, so this protects other textures (specular/env in alpha). |
| "Cleared to implement T1-T3" | Not an authorization. Only T3 was approved (user, 10-12); T1/T2 wait for the user. |

## 11. Follow-up (10-12 night): add-on catalog and lore research

`.claude/research/texture_diversity/CATALOG.md` (46 Haiku agents, 5 waves). 81 usable add-on cards for the 35 commoner atlases (60 deterministic S1D decals, 15 procedural wear, 6 recolours; 1 tattoo), 35 cards need uniform/armour bases, resolved lore table `w4/out/facts.md`, variant system draft `w4/out/system.md`. Fandom HTML is bot-blocked; `api.php` works (`tools/harvest.py`). Next unlock: annotate uniform/armour atlases (wave-2 method), then T1/T2 (gate, island masks) on user go.
