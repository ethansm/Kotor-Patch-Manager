# Session Handoff & Plan State: .claude/research/texture_diversity/PLAN.md

## 0. Next-Phase Direction
texture_diversity_research helmet previews + dispatcher helmet allow-list + K1 item import research

## 1. Active State & Provenance
- **Master Plan:** `.claude/research/texture_diversity/PLAN.md`
- **Branch:** `KOTOR-II` (b35db90 research(k1 items): static K1-vs-K2 item inventory (810 K1 UTI, 586 K1-only, models all present in K2))
- **Generated:** 2026-10-13 01:56:19 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
b35db90 research(k1 items): static K1-vs-K2 item inventory (810 K1 UTI, 586 K1-only, models all present in K2)
 .claude/research/k1_items/SUMMARY.md         |   74 ++
 .claude/research/k1_items/baseitems_diff.tsv |  109 ++
 .claude/research/k1_items/baseitems_k1.tsv   |   93 ++
 .claude/research/k1_items/baseitems_k2.tsv   |  110 ++
 .claude/research/k1_items/items_k1.tsv       |  811 ++++++++++++++
 .claude/research/k1_items/items_k1_only.tsv  |  587 ++++++++++
 .claude/research/k1_items/items_k2.tsv       | 1528 ++++++++++++++++++++++++++
 scripts/k1_item_inventory.py                 |  654 +++++++++++
 8 files changed, 3966 insertions(+)
```
- **Dirty Files (1693):**
  - ` A .claude/active_plan.txt`
  - ` A .claude/fixes/batch_01/0001-T837a1ca36b.patch`
  - ` A .claude/fixes/batch_01/0002-Tf3fc96cfd2.patch`
  - ` A .claude/fixes/batch_01/0003-T3aed4377cc.patch`
  - ` A .claude/fixes/batch_01/0004-T30def762ad.patch`
  - ` A .claude/fixes/batch_01/0005-T26197ffe58.patch`
  - ` A .claude/fixes/batch_01/series.json`
  - ` A .claude/fixes/batch_02/0001-T39945544b1.patch`
  - ` A .claude/fixes/batch_02/0002-Td08ab70ec8.patch`
  - ` A .claude/fixes/batch_02/0003-Tbbca7ddcf8.patch`
  - ... +1683 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/research/texture_diversity/PLAN.md`.

## 5. Dead Ends & Verification (manual)
State (10-13): P1 DONE and audited. Commits 773df47 (engine), 04593ad (27 recipes), 8377a93 (Mandalorian helmet), 1e8fd76 (audit tool, avoid_rects, island-wide palette layers). 27/27 looks gate-ok in `~/Games/KOTOR2_Mod_Workspace/tex_looks/run1/` (`bake.json`, `tpc/`, `parents/`, `masks/<model>/`, `sheets/index.png`, `audit2/<atlas>.png`). Docs: `.claude/research/texture_diversity/p1/audit/SUMMARY.md`, `P1_INTERFACES.md`. Tools (all `patch_manager_mods/k1_port/`): `k1_port_tex_bake.py masks|bake|sheets`, `k1_port_tex_audit.py`, `k1_port_tex_overlay.py` (ops recolor_hsv, colorize, band, decal, wear, tint; layer keys cells, islands, avoid_cells, avoid_rects), recipes in `looks/*.json`.

NEXT (user request 10-13): render the baked looks on the 3D character offline so the user can judge stripes/decals in context. DRAFT `k1_port_tex_preview.py` is on disk, UNCOMMITTED and UNTESTED (mesh dump under k1port venv + numpy software rasteriser). Facts it relies on (verified): n_commm visible body = three skinned meshes `torso`, `ArmL`, `ArmR` (tex N_CommM04, vertices already model space, z up ~0..1.9, character faces +y); `*_g` bone nodes have texture NULL; heads are separate models (no head in preview); mesh node transforms are identity for those three.

Dead ends / traps:
- Haiku audit units given the PNG only via the brief's read list could not read it (budget tool counts PNG bytes as text tokens: ~300k est); fix = list the PNG in the TASK text as an explicit extra permitted read, not as an entry.
- `rm -f $VAR/*.x` is blocked by the safety check; use `"${D:?}"/*.x`.
- Annotation cell zones (w2/w4/w5) are eyeballed and often wrong; decide from island masks + audit images (see SUMMARY.md). Island `arms` holds sleeve/back-leg cloth as well as hands.
- Skin guard blocks tan cloth (n_commf01): use `keep_skin: false` there.
- Mask labels come from node-name rules; `head` island on the Mandalorian = helmet shell; recolour must include it.
- Do not edit existing P0 behaviour without a test; `splice_colour` now keeps a DXT1 parent block whose colour mode would flip (alpha invariant).
- Another session has ~140 dirty files in this worktree: commit explicit paths only.

Verification commands:
- `python3 scripts/run_tests.py -k k1_port_tex` (8 suites, all PASS, 0 skip)
- `.venv/bin/python -I patch_manager_mods/k1_port/k1_port_tex_bake.py bake --recipes patch_manager_mods/k1_port/looks --masks $R --out $R` (R=`~/Games/KOTOR2_Mod_Workspace/tex_looks/run1`; expect 27 `gate PASS`)
- `.venv/bin/python -I patch_manager_mods/k1_port/k1_port_tex_audit.py --bake $R --out $R/audit2`

Open for P2 (install, needs user go): dispatcher allowlist covers commoner M 1-16, F 1-17, fat 34/35 only (new NN >= 36 needs allowlist change or reused slot); Sith Commando and Mandalorian use racetex rows (own appearance rows needed); N_CommM10 pool parent is TGA (not bakeable); in-game V orientation still the user's T5 test; no ledger entries written.

Probe (10-13, after the draft was written, nothing committed): `k1_port_tex_preview.py mesh --models n_commm n_commf n_sithcommando n_mandalorian --out $R/mesh` works under the k1port venv (textured nodes: n_commm torso/ArmL/ArmR 1210 tris; n_commf torso/ArmL/ArmR 1148; n_sithcommando body/head/larm/rarm 1934; n_mandalorian Torso/RArm/RShldrPad/LShldrPad/LArm/Head 2225; 41-46 NULL-texture bone nodes skipped). `render --bake $R --meshes $R/mesh --out $R/preview_probe --only SC-copper` took 2.9 s and produced a correct 4-view parent-vs-look sheet (front, back, side, 3/4; character faces the camera, framing shared between parent and look). Finding from it: Sith Commando SHOULDER plates are still white in 3D on SC-copper (shoulder/upper-arm plates map to UV areas the recipe still skips): the 3D view exposes gaps the flat atlas hides. Outputs live in `~/Games/KOTOR2_Mod_Workspace/tex_looks/run1/{mesh,preview_probe}/` (outside repo). Still to do: tests (`test_k1_port_tex_preview.py`, synthetic quad mesh), per-family contact sheets over all 27 looks, a turntable GIF per chosen look, commit the tool, add the render command to the handoff/README.

## 6. UPDATE 10-13 (supersedes the NEXT/draft lines above): 3D preview LANDED, equipment + K1 item research started
- Landed: 04154c4 (preview tool `dossier`/`render`, 11 tests, 3D-driven recipe fixes), 43f5805 (docs), 8a28e4f + 988d9d8 (equipment research), b35db90 (K1 item inventory). Haiku vision pilot works (see texture_diversity_research.handoff.md "3D preview"). R = `~/Games/KOTOR2_Mod_Workspace/tex_looks/run1`.
- Equipment facts (all in `.claude/research/npc_equipment/`): CanEquipItem (Steam 0x005bd6a0) has NO appearance/model block; spawn path 0x00573950 runs Equip_ItemList through it; checks = UTI NonEquippable bit, baseitems EquipableSlots + proficiency, item properties (alignment/class/race/gender/feat/tag/min stat), baseitems row +0xB4 race rule and +0xB8 gender mask, Bao-Dur base-item list (35,36,37,97-100,102). GoggleHook attach (0x0085B703) runs only for head type 3 (appearance column via FUN_0085C150); other head types swap the head variant; 28 head-item appearances have no GoggleHook. `appearance.2da` has helmet_scale_m / helmet_scale_f. Mask base item 44 -> I_Mask_NNN models are small goggle/visor meshes in head-local coords (probe: i_mask_001 = Infragoggles, i_mask_008 = AuralAmp); full helmets such as a_helmet_04 need their own model/variant path (NOT yet understood).
- K1 inventory facts (`.claude/research/k1_items/SUMMARY.md`): 810 K1 UTI vs 1527 K2; 586 K1-only resrefs but EVERY K1-only item whose model loads (382) already has that model resref in K2; 107 have no derivable model, 97 no model. So K1-only items are mostly new templates (stats/names), not new meshes. Real novelty must come from a resource-level model/texture diff, not UTI-driven.
- User request (10-13): (a) helmet previews on the 54 head-item appearances, (b) dispatcher helmet allow-list from baseitems race/gender rules, (c) include K1-item swarm next steps. Plan written in plan mode after this handoff.
- Dead ends/traps added: rea-recon cannot overwrite an existing file (no Read tool): give it a NEW output filename; it needs the exe path in the brief (`swkotor2.exe.unpatched_vanilla`) and STEAM addresses, GOG addresses in the old knowledge map were wrong for CanEquipWeapon/2 (real branches 0x5bd1d0/0x5bd3e0/0x5bd610); GoggleHook-on-head is NOT necessary for a helmet to show; Haiku scan helpers reuse `scripts/npc_equip_scan.py` pattern.

## 7. UPDATE 10-13 (later): helmet previews + head-item allow-list LANDED, K1 asset diff produced
- Landed: 6c36ef5 (preview tool: `mesh --textures` dumps hooks + raw textures incl. TexturePacks, `render_scene` multi-part z-buffer, `helmets` CLI, 9 test files green), f2629e4 (`DiversityHeadItems.h` + `head_items_source.json` + generators/tests + engine_recon_p3/p4 + `gender` column in utc_equip.tsv). R = `~/Games/KOTOR2_Mod_Workspace/tex_looks/run1`; helmet sheets `$R/helmets_final/<row>_helmets.png` + `wall_helmets_i_mask_007.png`; mesh dumps `$R/helm/mesh` (64 models, tex/).
- Helmet facts: goggles/visors/hoods sit correctly on type-B heads at head `headhook` + `GoggleHook` (placement verified by eye on 38 B rows; 25 OK, 13 NO_NODE drawn bare = engine drops the item). F/L rows (16): recon p3 CONFIRMED the GoggleHook path (FUN_0085a9e0) runs only for classifier value 3 and no F/L head-item attach was found (FUN_0085b940 not decompiled): drawn bare, labelled ATTACH_UNKNOWN. Tiling-UV nodes are kept (`uv_wrap`) so i_mask_013 etc. render. Wall frame = bare head + pad 0.30; F/L frame = top 0.28 m of the body.
- Allow-list facts: baseitems `droidorhuman` is the race rule: Mask (44) = rule 1 = creature race 6 only; droid head items (rows 69-72) = rule 2 = race 5 only; gender mask 0 for all head rows (+0xB8 column name still UNKNOWN); no reqfeats. 58 head items (41 Mask + 17 baseitem 72; 33 with restrictions). Invariants: all 70 vanilla appearance/item pairs and 203 UTC rows obey the model (0 violations). Rows with races != [6] (9: 17, 56, 66, 236-238, 270, 555, 565): the dispatcher must still check the CREATOR's race (row 17 and 270 have race-2 and race-6 templates).
- K1 asset diff (`.claude/research/k1_items/asset_diff_*.tsv`, `ASSET_DIFF_FINDINGS.md`): no item mesh only in K1; 61 `w_*` weapon MDLs have different node counts (same resref, converter refuses collisions); 15 K1-only `p_*` companion models; MDL hashes all differ (use node count). C2 as planned (12 units) is oversized: propose ~4 Haiku units (61 w_*, 15 p_*, 4 icons) after the user's go on spend.
- Not done / gated: Haiku vision pass over helmet sheets (user must approve rubric first); C2 triage swarm (needs user spend go); dispatcher C++ use of `DiversityHeadItems.h` (design + go); F/L attach (decompile FUN_0085b940); +0xB8 column name.
- Resume: `python3 scripts/plan_manager.py resume .claude/research/texture_diversity/PLAN.md`.

## 8. UPDATE 10-13 (latest): C2 + C3 DONE, TGA render bug fixed, CR textures chosen as the K1 base
- Landed: ad80b7e (C2 triage, 80 assets, 4 Haiku units, `k1_items/TRIAGE_SUMMARY.md`), 2b1de52 (`compare` CLI: K1 vs K2 sheets; `mesh` skips unparsable models), d606ab0 + 96b6721 (`C3_FINDINGS.md`; **renderer fix: bottom-origin TGAs were sampled upside down**, affected 7 helmet textures and all 40 K2 weapon textures; test `test_load_texture_tga_origin_uses_stored_row_order`, suite 22/22). Fixed renders: `$R/helmets_v3/`, `$R/c3/sheets_v2/`, `$R/c3/sheets_p_hd2/` (R = `~/Games/KOTOR2_Mod_Workspace/tex_looks/run1`).
- Findings: K2 reworked every weapon mesh (3-7x tris) and its textures are far better, so ALL K1 weapons dropped (user 10-13); sabers not comparable (K2 saber dumps have 0 textured nodes). The 15 K1-only `p_*` companion models (Bastila, Canderous, Carth, Jolee, Juhani, T3-M4) are the only real K1-only assets; with CR 2048 textures they render cleanly and the user approved them as bases.
- K1 facts: the K1 install is vanilla (OVERRIDE holds 0 files). Upscaled K1 textures exist only in the Characters Revisited (CR) extract `~/Games/KOTOR2_Mod_Workspace/k1_port_extract/characters_revisited/{party,head_player,body_player,species,...}` (2048 px TGA, same UV layout as vanilla, licence unknown/approval required, local-only). Upscale+ V0.52 covers N_Comm* models only. No upscaled K1 weapon/item textures anywhere on disk.
- User decision (10-13): install the CR textures over the K1 assets so all K1 experimentation uses them instead of vanilla low-res. OPEN in the plan: where (K1 game Override = a write into the K1 dir, which the earlier constraint forbade; or a local overlay dir that our tools, `mesh --textures` and the converter, search first). Then the next phases (doc 38 `p_*` import, texture diversity P1, dispatcher consumption).
- Not done / gated: Haiku vision pass over helmet sheets (rubric approval); dispatcher C++ use of `DiversityHeadItems.h`; F/L head-item attach; `asset_diff_all.tsv` uncommitted.
- Resume: `python3 scripts/plan_manager.py resume .claude/research/texture_diversity/PLAN.md`.

## 9. UPDATE 10-13 (latest): CR base overlay + ledger safety + 15 p_* imported (rows HE/HF DONE)
- Overlay `~/Games/KOTOR2_Mod_Workspace/k1_hd_install` (418 same-layout CR stems; `scripts/k1_hd_overlay.py build|check`); layout verifier `scripts/cr_k1_layout_check.py`; findings `.claude/research/k1_items/CR_BASE_FINDINGS.md`.
- Ledger safety: `--allow-overlay-install` (refused otherwise), `--prefer-source`, `updates/` tie-break; `gate` BLOCKS CR shas (also renamed copies).
- 15 `k1p_*` p_* models + 16 flagged CR textures in the stage (`scripts/k1_import_companions.py`, dry run default, `--apply`). Not installed in K2.
- Open: P2 user look at `R/sheets/index.png` (3 files game-closed install needs user go); `cr_stem_to_model.tsv` heads wall not rendered; `asset_diff_all.tsv` (6 MB) left untracked on purpose; CR permission drafts unsent.
