# P3: mechanism, install, save for a 15-creature Citadel PoC (recon, 10-11)

Read-only: repo docs, scripts, rg, sed -n, one ls. No game file or raw log read.
Target: 204TEL. Label "Citadel Station Residential - corteldc medical contact" (scripts/04_merge_modules.py:9). Treating 204TEL as "the medical center" is ASSUMED from that label.

## 1. Mechanism ranking (1 = best)
1. C hybrid: append 15 GIT Creature List structs (TemplateResRef, X/Y/Z, orientation) to 204tel.mod. Put the 15 UTCs in override/. Keep ScriptSpawn=k_def_spawn01 (vanilla) and 3 GIT WaypointList entries per creature (Tag WP_<tag>_01..03; R2 sec 2 tests WP_<tag>_02). No new NCS. Why: one capsule edit; the rest is loose override/ files; precedent mod115 (GIT append + UTC) and 04_merge_modules.py:45-60 (corteldc GIT append).
2. A: Override-only script spawn (R2 sec 3). Why: no capsule edit, but needs a host creature whose template is overridden game-wide, a compiled NCS (compile.sh, nwnnsscomp), and unknown save persistence for spawned objects.
3. B: same GIT append with UTCs inside the .mod. Why: same precedence risk as C; a bad UTC means restoring the whole capsule.

## 2. PROVEN vs ASSUMED
PROVEN (repo evidence):
- Live Modules/ is the source of truth. 04_merge_modules.py writes 204tel.mod in place (read_erf/write_erf, merge_module_204tel). replay/README.md:13-17.
- .mod GIT (78) and .rim GIT (74) differ. .mod-only: corteldc. .claude/review/npc_div/C2_telos_census.md:3,45.
- Override beats chitin (38_k1_to_k2_asset_import.md:16, claim 7). Override UTC read on a fresh visit (W4 Q4, K1 decomp + TSL source).
- Saves cache each VISITED module as a nested .sav with .git, .are, module.ifo (M0_save_layout.md:9-11).
ASSUMED / OPEN:
- Engine loads 204tel.mod, not 204tel.rim or _s.rim: ASSUMED. Only source is the docstring rule in scripts/npc_div_registry.py:80. C2:62, R1 OPEN 1, R3:267 all say unchecked.
- Override GIT/ARE/IFO honored: UNVERIFIED (R1 sec 2).
- Engine builds 204TEL creatures from the GIT list on a fresh visit: ASSUMED (no TSL area-loader decomp, W4 Q4).
- Case-variant spellings (e.g. 204TEL.mod vs 204tel.mod): OPEN (R1 OPEN 7). Write the exact file the engine loads.
- replay_capsule_edits.py not found in the worktree or workspace (find, maxdepth 6). replay/replay_log.jsonl exists.

## 3. Install and rollback outside KPM
KPM cannot ship override/ or modules/ (create-patch.py:852-870; BUILD_AND_INSTALL.md:54). A separate installer is needed.
- Best copy-as-installer: patch_manager_mods/k1_port/k1_port_probe.py (T12i). Subcommands plan (dry run default), install --yes, rollback --yes, status. Refuses when swkotor2 runs, refuses foreign files, checks --base-sha256, writes manifest.json and original/ under <game>/backups/k1p_probe_<stamp>/, plus a latest pointer. Gap: entry_dest() hard-codes override/ (lines 198-205). Modules/ needs a new branch. Never run live (T12h blocked on an unbuilt file, 38 doc:124). Tests: test_k1_port_probe.py.
- Rollback model: scripts/kpm_hotswap.py swap_batch and rollback (lines 235-330). Backups go to <game>/backups/<label>_swap_<ts>/ with a manifest. Undo on failure. Refuses when the game runs. "--rollback latest".
- Whole-tree net: scripts/install_echoes_compatch.py create_backup (lines 131-152). Tar of override/, Modules/, dialog.tlk before any change.
- scripts/snapshot_live.py covers override/ only (hardlinked, SHA256SUMS, game closed). Not Modules/.
- Manual precedent: <workspace>/backups/Modules_pre_swap_20260924/Modules (replay/README.md:185). No script named install_override exists. 06_deploy_assets.py copies into override/ with no backup.
Proposed PoC rollback: (a) copy the original 204tel.mod and 15 UTC hashes to backups/<label>_<ts>/ with a manifest; (b) write the .mod via temp file then rename; (c) append the edit to replay/replay_log.jsonl so a rebuild keeps it; (d) rollback restores the .mod by hash and deletes the 15 UTCs listed in the manifest. Game closed for all steps.

## 4. Savegame (6 lines)
1. A visited module is frozen in the save as a nested .sav (M0 sec 1).
2. Template and GIT edits never reach a visited area (W4 Q1, Q3, VERIFIED across 3 community sources).
3. The boundary is the first visit, not the save (W4 "Boundary").
4. Game2 (000003) holds nested 201tel, 202tel, 203tel only, so it looks pre-204TEL (M0 sec 1). Confirm no 204tel.sav, and copy it before use.
5. scripts/kotor_save_forge.py edits existing entries only (lines 418-421). It cannot create the no-204 state. A forged save needs a NEW removable slot and user consent (kotor2-forced-state-testing, step 3).
6. Standing NPCs sit in both AVAILNPC and the GIT (M0 sec 2). Engine preference for a GIT-added creature is OPEN.

## 5. Unknowns for the first probe (pick 3)
U1. Which file the engine reads for 204TEL (.mod vs .rim/_s.rim). Handle: corteldc exists in 204tel.mod GIT only (C2:45). User play check: enter 204TEL from Game2 and look for corteldc. Present means .mod wins. Absent means the GIT edit is a no-op.
U2. Do the 15 species rows resolve? Appearance rows and models must exist. override/appearance.2da is whole-file and owned by npc_diversity (34 doc:285), so new rows collide with it.
U3. Fresh-visit spawn of 15 creatures: object count and frame cost (R5 OPEN), and FactionID meaning (R5: unverified). Use faction 0 from the commoner precedent.

## 6. Final ranking
C hybrid first: one capsule edit, no new NCS. A second: no capsule edit, but game-wide template and unknown persistence. B last: same risk, whole-capsule rollback.
