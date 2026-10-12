# R1: Add creatures to vanilla Telos areas via module data

Read-only recon. Repo docs and scripts only, plus `ls` of the install `modules/` and `Override/`. No game file was read.
Question: can a Wookiee (or any creature) be placed into a vanilla Telos area by editing module data (.git Creature List, .utc, waypoints)?

## 1. Module files and Citadel mapping
- Install `modules/` (case variants all present on Linux): `201TEL.mod|.rim|_s.rim`, `201tel_dlg.erf`, and the same for 202-204 (ls only).
- In scope: 201TEL-204TEL. 78 GIT creature instances (21/20/20/17). `.mod` GIT = 78, `.rim` GIT = 74 (`.claude/review/npc_div/C2_telos_census.md:3,9,45`).
- Citadel label: `scripts/04_merge_modules.py:2-9` calls 204TEL "Citadel Station Residential" and 221TEL "Citadel Station Czerka / Grenn".
- Save AREANAME "Citadel Station - Residential 082 East", LASTMODULE 203TEL (`research/companion_levelup/phase6_evidence/M0_save_layout.md:43`).
- C2 calls 201-204 "Telos Station". Working answer: Citadel Station is the 2xxTEL set, 201-204 plus 221 at least. Exact list OPEN.

## 2. Resource precedence (Override vs module capsule)
- Override beats chitin: VERIFIED (`patch_manager_mods/38_k1_to_k2_asset_import.md:16`, claim 7).
- TPC: engine formats `N_CommM%02d` and queries `CExoResMan::Exists`; an Override TPC loads by name (`patch_manager_mods/34_npc_diversity_dispatcher.md:69`).
- MDL/MDX: Override-only Twi'lek dancer meshes are used by the live game (`.claude/review/npc_div/D1_assets.md:74`; `34_...md:288`).
- 2DA: whole-file `override/appearance.2da` is live (`34_...md:285`; `patch_manager_mods/README.md:118`).
- NCS: 10 `.ncs` files in `override/` (`~/Games/KOTOR2_Mod_Workspace/replay/README.md:112`). `scripts/check_ncs_arg_order.py:28,108-114` scans both `override/` and `Modules/*.mod`.
- UTC: Override template read on a fresh visit VERIFIED (`.claude/review/k1_import/W4_save_persistence.md` Q4). Module capsules also carry UTCs: 216 `.mod` files hold 3603 UTC (`.claude/review/npc_div/C1_logic_tests.md:12`).
- DLG: repo edits an Override `.dlg` (`patch_manager_mods/15e_phase4_session_context_2026-10-03.md:34`, "incl. override kreia.dlg"). Override vs `<mod>_dlg.erf` precedence: no repo proof.
- GIT / ARE / IFO: no repo evidence of Override loading. Every repo writer targets the module capsule (`04_merge_modules.py:36-68`) or a save's nested `.sav` (`scripts/kotor_save_forge.py:372-472`). Treat Override GIT/ARE/IFO as UNVERIFIED.
- Engine: Override dir scan builds the resource key table at 0x723220 (`engine_research_findings/R8.md:53`; `R9.md:161`). Once per process or per module load is OPEN (`R8.md:164`).

## 3. Tooling in repo or workspace
- pykotor 2.3.12 in `.venv/lib/python3.12/site-packages/pykotor/`: `read_erf`, `write_erf`, `bytes_erf`, `read_gff`, `write_gff`, `bytes_gff`, `GFFStruct`, `GFFList`, `ResourceType` (`04_merge_modules.py:22-25`; `kotor_save_forge.py:28-31`). `read_rim` is used by C2.
- Precedent for "add creature to GIT list + ship its UTC in the same .mod": `scripts/04_merge_modules.py:36-68` (`merge_module_204tel`). It deep-copies a creature into `Creature List` (dedupe by TemplateResRef), injects missing resources, then `write_erf`. It writes to the live `Modules/` with no staging step inside the function.
- `scripts/kotor_save_forge.py:418-421` edits existing Creature List entries by Tag in the nested GIT. It never adds an entry and never edits an existing save slot (header lines 4-12).
- pykotor patch #6 (edits to existing capsule resources silently reverted): `hardware-tools-docs/popos/scripts/kotor2/pykotor_patches/README.md:234-262`. The venv copy copies existing resources first and sets the patched one last (`.venv/.../pykotor/extract/capsule.py:274-279`). Read from source only; no round-trip run.
- No `scripts/*gff*` file. `patch_manager_mods/k1_port/` has ingest/prune/model/texture tools and no GIT writer.
- GIT entry fields (TemplateResRef, position, UseTemplates) and the overlay rule: W4 Q4 cites `git.html`, rakata K1 docs. Not checked against TSL in repo.

## 4. Savegame interaction
- A save holds one nested SAV per VISITED module, each an ERF of exactly `<mod>.git`, `<mod>.are`, `module.ifo` (`M0_save_layout.md:9-11`). Sample save: 003ebo, 201tel, 202tel, 203tel.
- Saves hold full creature snapshots; a template edit never reaches a visited area (W4 Q1/Q3, community x3; `38_...md:19` claim 10, PARTIAL).
- Community: a fresh game or a pre-visit save is needed (W4 Q3a/b). Boundary is per first visit, not per save (W4 "Boundary").
- Standing NPCs: GIT and AVAILNPC both hold them; engine preference unproven (`26_FORCED_STATE_TESTING.md:28`; `M0_save_layout.md` section 6).
- Repo's own probe design uses a pre-Telos save with no `201tel` nested `.sav` (`.claude/review/k1_import/T12h_probe_design.md:119`).
- Consequence: a module GIT edit reaches new games and saves made before a module's first visit. Visited modules keep their old creature list.

## 5. Test handle
- `corteldc` (app 193) sits in `204tel.mod` GIT and not in `204tel.rim` GIT (`C2_telos_census.md:45`). Appended by `04_merge_modules.py:45-60`. If it spawns on a fresh 204TEL visit, `.mod` is the engine source. Needs a user play check.

## OPEN
1. Load order when both `.mod` and `.rim` exist (and `_s.rim`) (`C2_telos_census.md:46,61-62`).
2. Override GIT/ARE/IFO: no evidence either way.
3. DLG: Override `.dlg` vs `<mod>_dlg.erf` on conflict.
4. Does module load replace an area creature from AVAILNPC (`M0_save_layout.md:63`)?
5. TSL area-loader call site for GIT (none decompiled; W4).
6. Exact module set of "Citadel Station".
7. Case-variant duplicates (`201TEL.mod` vs `201tel.mod`): which one the engine picks.
