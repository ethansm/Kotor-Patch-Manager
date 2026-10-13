# Plan: AI-assisted overlay customisation of NPC textures (draft 10-12, awaiting user go)

Status: PLAN ONLY. Nothing installed, no game file or repo code touched. Builds on `CATALOG.md` (research, sections 9-13) and `.claude/research/texture_ai_pipeline/report.md` (pipeline stages, gaps T1-T6, licence rule).

## 1. Principle
**Overlay, never repaint.** Every output keeps the parent's pixels, alpha, dimensions, mips and format outside a mask. Deterministic stage 1 does recolours, bands, emblems and wear. The local AI (stage 2, Qwen-Image-Edit, Apache-2.0) adds only what shapes cannot: ornament, weathering and blending. AI output is always composited back through the mask and gated. New looks ship as NEW variants (`<texa>NN`), never as edits of vanilla `01`.

## 2. What the assets give us (numbers)
- **123 usable add-on cards**: 81 commoner (KEEP), 30 uniform/armour/robe (KEEP-UNI), 12 species (KEEP-SPEC). 20 species cards stay locked (no body atlas yet); the rest of the 174 are merged, dropped or held.
- **184 atlases annotated**: 70 commoner (35 per sex, one shared layout each), 51 uniform/armour/robe, 63 gap and species. Families share one layout across their numbered variants, so one zone map plus one recipe fans out to N variants.
- **Reach (vanilla spawns, `plan/priority.tsv`)**: commoner pool ~305 spawns plus every patch-assigned generic NPC; Sith Commando 199; Sith trooper 124; Dark Jedi low/mid 168 (`n_darkjlowm01` alone 105); Mandalorian 61; Czerka bodies 95 (`n_czerkaoff01` 30 + `pmbama01/mc01` 65); Trandoshan 54; Rakata 76; Republic/TSF 73.
- **Engine delivery is already proven**: the body texture is `texa` + `%02d TextureVar`; any loose `<texa>NN.tpc/.tga` in `override/` loads, a missing one falls back to `01` (doc 34 sec 1.1). Vanilla uses ONE variant per uniform body, so every added look is new variety.

## 3. Customisation menu (what each layer does)
| Layer | Stage | Does | Cards / examples |
|---|---|---|---|
| L1 Palette | S1H | HSV shift under island mask | COLOR cards (Onderon blue-grey, Sith scarlet/silver, robe hues) |
| L2 Trim, stripes, rank bars | S1D | stencilled bands from our own shapes | TRIM cards, A3-02, A1-04, MAND-6 |
| L3 Emblems | S1D | own simplified shapes from the emblem grammar (`w3b/out`), no franchise art | Republic cog, shriek-hawk, Jaig eyes, medallion |
| L4 Wear and stains | S1W | procedural dirt gradient, fade, scuffs, oil | 15 wear cards; also usable on any atlas |
| L5 Harmonise (AI) | S2A | low-strength img2img on the decal crop so paint matches weave or plate grain | after L2/L3 |
| L6 Plate weathering (AI) | S2A | chips, scratches, soot on hard plate only | Mandalorian, Sith, TSF plates |
| L7 Ornament (AI) | S2A | embroidery, personal clan paint, tattoos on bare skin | MAND-3, SITHJ1-6, ALIEN2-3, ALIEN3-5 |
Rules: max 3 layers per look; no two faction emblems together; never touch face/hand/avoid cells; AI jobs run on 256-512 px crops, 3-5 seeds, human picks one.

## 4. Build recipes, ranked by reach
| # | Recipe family | Atlases (spawns) | Cards | New variants | Notes |
|---|---|---|---|---|---|
| R1 | Commoner POC set P1-P8 | 70 commoner atlases (generic pool) | 81 | ~8 looks x 35 | proven delivery; start here |
| R2 | Sith Commando | `n_sithcom01-03` (199) | SITHJ1-3 + L4/L6 | 04-09 | hard plate, alpha non-opaque: RGB only |
| R3 | Sith trooper / Captain | `n_sithsoldier01-03` (124), `n_sithcomm/f01` (49) | SITHJ1-2, A8-01/02/03 | 04-08 | officer sash on grey cloth |
| R4 | Mandalorian | `n_mandalorian01-03` (61) | 9 cards | 04-09 | highest card count; best AI showcase (MAND-3) |
| R5 | Czerka | `n_czerkaoff01` (30), `pmbama01/mc01` (65) | A3-02, A12-07, K137, ALIEN3-5 | 02-05 | only usable human tattoo field is `pmbama01` torso |
| R6 | Republic / TSF | `n_repsold01` (31), `n_tsfoff01` (26), `n_repoff01` (10) | REPUB-3/4, A1-03/04, A12-07 | 02-04 | REPUB-4 = yellow accents only (base already red) |
| R7 | Dark Jedi / Sith | `n_darkjlowm01` (105), `n_darkjmidm/f01` (63) | NONE assigned yet | 02-05 | **gap:** re-fit SITHJ1-1/4/5/8, SYM1 cards to these (zones in G1/H7); one Haiku agent |
| R8 | Species | Trandoshan (54), Twi'lek dancers, Rodian, Duros, Walrusman | 12 KEEP-SPEC | per family | confidence 2-3, re-check at full size |
Skipped as low value: Jedi robes (4 spawns), Ubese (1 card), Onderon until a Vaklu appearance row exists.

## 5. Modules to build (order)
- **B0 T1 output gate** (parent-relative: dims, mips, format, alpha identical, no TXI, name rule). Blocks everything.
- **B1 T2 island masks** for pilot atlases `n_commm05`, `n_sithcom01`, `n_mandalorian01`. Source: MDL UV islands where they work; fallback = polygon masks from the annotation cells, verified by overlay images. Replaces the 4x4 estimate.
- **B2 decal library**: ~12 emblems drawn as our own SVG/raster shapes (licence: ours); Commons manifest only if share-alike is accepted.
- **B3 `overlay.py`**: recipe JSON -> composite -> gate. Recipe = `{family, atlas, layers:[{kind, card, cell/mask, colour, scale, blend, seed}]}`.
- **B4 bake CLI + review sheets**: reuse the contact-sheet tool; output outside the repo, local-only.
- **B5 AI runner**: Qwen-Image-Edit Q4 on the 12 GB RTX 5070, crop in/out, auto-QA (pixels outside mask identical, alpha identical, dE drift cap), seed log into the ledger (T3 `derive` exists).
- **B6 name allocator + stager**: next free `<texa>NN` per family, ingest, plan/install/rollback via the existing `k1_port_probe.py` loop.
- **B7 dispatcher extension** (engine side, KPM C++): doc 34 chooses `TextureVar` for generic commoner families. Whether it covers uniform/species families is **unverified**; if not, uniform looks need a new work item (per-family variant tables) or new appearance rows. Plan it after the POC, user go.

## 6. Phases (each a bounded session)
| Phase | Owner | Work | Exit |
|---|---|---|---|
| P0 | agent | B0 + B1 on 3 pilot atlases, tests colocated | gate passes on unchanged parents; overlay images show masks |
| P1 | agent | B2 + B3 + B4; bake 8 commoner looks + Sith Commando + Mandalorian looks, stage 1 only | local review sheets |
| P2 | user | look at sheets (about 5 min), pick winners; game-closed install of 2-3 files, screenshot (T5) | go/no-go on style |
| P3 | agent | Qwen timing run (T6), L5/L6/L7 on 3 pilot recipes, compare with stage-1 only | s/image and quality known |
| P4 | Haiku waves | one agent per family writes recipe JSON (1 deliverable each); bake; gates | ~100-500 gated variants |
| P5 | agent + user | B7 dispatcher scoping and RE, then install and census (`kotor2_logs.py`) | variants seen in game |

## 7. Decisions needed from the user (max 5)
1. **Go for P0** (B0 + B1)? No game or repo-code change; adds scripts and tests only.
2. **Named-faction emblems** (Czerka, TSF, Exchange have no text source): use our own labelled "inspired" shapes now, or wait for in-game ground truth? Recommend own shapes.
3. **Tattoos**: only one human field exists (`pmbama01` torso). Accept thin coverage or drop the slot?
4. **Dispatcher extension** for uniform families: scope it after P2 as a new work item?
5. **Qwen setup** (about 12 GB download, ComfyUI or diffusers): go/no-go before P3.

## 8. Risks and unverified items
- Cell maps are eyeballed (confidence 2-4); T2 must confirm every one before a bake.
- `TextureVar` 0 read as 01 is inferred (no `*00` file exists).
- HD (2048 px) Twi'lek dancer textures come from the installed mod pack and cost VRAM (doc 34 H7 note); do not add more large variants for them.
- Qwen 20B Q4 on 12 GB: no timing or quality evidence at 512 px; stage 1 alone must already look good.
- Licence tags carry to derived files (user decision 10-12); everything stays local-only until the prune gate passes.
- Vaklu shares the Onderon models; a Vaklu look needs a new appearance row or variant, not a recolour of the shared atlas.
