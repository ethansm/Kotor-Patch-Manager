# Phase 3 sweep: stale 3C-FD / LAA / 4AB72F references (read-only, 2026-10-03)

Authoritative state (vanilla_migration/README.md "RESUME HERE", re-checked on disk 10-03):
live `swkotor2.exe` = vanilla `6A522E71...` + KPM `4gb-patch` = `7751F7D8...`. The two exes differ by exactly ONE byte
(`cmp -l`: file offset 0x11E, 0x03 -> 0x23, PE Characteristics LAA bit). Consequences:
- `$G/swkotor2.exe.unpatched_vanilla` EXISTS, sha256 `6A522E71...FFEF` (== `backups/vanilla_clean/swkotor2.exe`, == `swkotor2.exe.backup.20261003_112944`). It is still a correct "vanilla" reference.
- Every "bytes identical in LAA exe and base exe" claim stays TRUE (only the PE header differs).
- No `*.toml` under the repo contains `4AB72FC1` any more, and `*_laa.db` files are gone from `$G` and from the address-DB worktree. Anything that tells you to add a `kotor2_steam_aspyr_laa` key / clone a `.db` is dead procedure.
- The live exe hash is now `7751F7D8`, but KPM matches `target_versions` / address DBs by the ORIGINAL hash recorded in `kpm_install_state.json` (6A522E), so mods keyed on `6A522E71` are the correct target; no mod should list `7751F7D8`.

Note on vanilla_migration/README.md itself (excluded from the table, one small internal inconsistency): "Status: INSTALLED ... 31 ids / 202 hooks" (12:14 entry) vs "RESUME HERE" 31 ids / 198 hooks (verified 10-03). The Status line is dated 12:14 and RESUME HERE is authoritative; consider appending "(then 198 after the sibling reinstall 10-03)". Not a contradiction about the exe.

## Summary table

Class: M = MISLEADING, H = HISTORY, F = FIXTURE / false positive / still-true property.

| file:line | class | note |
|---|---|---|
| patch_manager_mods/README.md:97-102 | M (low) | "Existing exe-level patch already applied: Mod #4 + Mod #1 (3C-FD) modify swkotor2.exe PE headers" - 3C-FD gone; live LAA now comes from KPM 4gb-patch |
| patch_manager_mods/03_framework_research.md:113 | M | "Do not install Patch Manager's copy [4GBPatch]... already applied" - 4gb-patch IS now the installed LAA mechanism |
| patch_manager_mods/03_framework_research.md:23 | F | generic description of `static` hook type ("like the LAA bit") - true |
| patch_manager_mods/04_recommended_installs.md:50 | M | "## Do not install: Patch Manager's own 4GBPatch ... redundant with mod #4" - inverted: it is installed and required |
| patch_manager_mods/06_gog_to_steam_conversions.md:677 | M | port procedure step 8: "same LAA-hash two-part recipe" |
| patch_manager_mods/06_gog_to_steam_conversions.md:689-698 | M | "Open items": recipe "add kotor2_steam_aspyr_laa to manifest/hooks + clone .db" - requires LAA db alias |
| patch_manager_mods/06_gog_to_steam_conversions.md:273,330,362,393,408,472 | H | candidate write-ups dated 09-28 |
| patch_manager_mods/08_level_up_override.md:4-5 | M | header: "Target: Steam Aspyr LAA swkotor2.exe ... live exe 4AB72FC1... is the patched one" |
| patch_manager_mods/10_engine_optimization_research.md:224 | M | "Context (from the plan)": "live copy is the LAA variant 4AB72FC1..." |
| patch_manager_mods/09_load_hang_investigation.md:52 | M (low) | "Environment facts": "Game is the Steam Aspyr build (LAA variant, hash 4AB72FC1..., original 6A522E71...)" |
| patch_manager_mods/02_buff_duration_hud.md:1671 | M | "FINAL IMPLEMENTATION ... READ THIS FIRST": hooks target "Steam 6A522E71 and its LAA build 4AB72FC1" - shipped toml no longer lists 4AB72FC1 |
| patch_manager_mods/wrap_measure_fix_src/README.md:3 | M | "for Steam Aspyr and Steam Aspyr LAA exes" - toml targets only 6A522E71 (flagged in 20a F-row) |
| patch_manager_mods/issue_drafts/issue_4_steam_address_library.md:8 | M | unposted upstream draft advertises "LAA variant 4AB72FC1..." |
| patch_manager_mods/mods678_research/README.md:24 | M | tool table: "BOTH executables (LAA 4AB72FC1, base 6A522E71)" |
| patch_manager_mods/mods678_research/HANDOFF_2026-09-30.md:22 | M (low) | "identical LAA hook set" wording (script actually filters on the 6A522E71 hash) |
| patch_manager_mods/mods678_research/verify_windows_both_exes.py:3,28 | M (low, cosmetic) | docstring "LAA build, sha 4AB72FC1..." and output label `LAA:`; script WORKS (see below) |
| patch_manager_mods/kotor2-batch3-porting-plan.md:31,127,131,151,179 | M (low) | 09-28 plan still labelled "source of truth for resuming": states live hash 4ab72fc1, tells to put 4AB72FC1 in target_versions and `kotor2_steam_aspyr_laa` in manifest, cites `_laa.db`. Candidate A is built; plan is mostly history |
| patch_manager_mods/18_mouse_camera_investigation.md:60 | M (low) | "[V] exe is unmodified (CurrentHash == OriginalHash == 4AB72FC1...)" - true for 09-26, no longer; exe now has 4gb-patch |
| patch_manager_mods/K1_ENGINE_BUGS_IN_K2.md:8-9 | M (low) | public-facing; "Its LAA variant 4AB72FC1...5A5B" - hash tail is WRONG (real tail is `...36FB5A`, not `...5A5B`), variant is retired |
| memory/kotor2-patch-manager.md:265-284 | M | "Resolved (Phase 1a) - LAA-hash mismatch, two-part fix": says live exe is 4ab72fc1, "Repeat this exact two-part recipe for the next mod" (add `_laa` manifest key + clone `kotor2_steam_aspyr_laa.db`) |
| memory/kotor2-patch-manager.md:118-124 (hit 121) | M (low) | "LAA-hash question... resolved... mechanical step" - recipe obsolete |
| memory/kotor2-patch-manager.md:477 | M (low) | "already exists in `kotor2_steam_aspyr_laa.db`" - that db no longer exists; use `kotor2_steam_aspyr.db` |
| memory/kotor2-patch-manager.md:238,315 | F | `static` hook type, "e.g. LAA flag" - generic, now literally the 4gb-patch; true |
| memory/kotor2-patch-manager.md:703 | H | 09-29 scaffold note "Steam/LAA only" |
| memory/kotor2-patch-manager.md:1061 | H | 10-02 log: "Vanilla-exe migration ... deferred to post-soak" - superseded the same day (done 12:02); dated entry |
| memory/kotor2-engine-research.md:27 | H | 10-02 12:50 log: "adb symlink dir must skip the moved kotor2_steam_aspyr_laa.db" |
| memory/kotor2-ultrawide-gui.md:69 | F | "32-bit PE with LAA" - true property |
| memory/kotor2-vanilla-exe-migration.md:7 | F | the authoritative correct statement |
| memory/MEMORY.md, CLAUDE.md | none | zero hits; "kotor2-vanilla-exe" index line is correct |
| scripts/check_hook_bytes.py:17 | F | docstring: "extra target_versions such as the LAA exe are fine" - harmless; script uses VANILLA_EXE (vanilla_clean, 6A522E verified) |
| patch_manager_mods/steam_db_pr3/work/pe.py:2 | F (works) | `EXE = $G/swkotor2.exe.unpatched_vanilla`; file exists, sha 6A522E71; correct |
| patch_manager_mods/engine_ai_atlas/tools/tests/helpers.py:15 | F | `OTHER = "4ab72fc1..."` stays, per instruction (an arbitrary "other hash" for tests) |
| patch_manager_mods/10_equipment_3d_character.md:43,56; 08_level_up_override.md:570 | F | `3cfd` = companion/creature tag (t3m4/hk47/3cfd), false positive |
| patch_manager_mods/engine_ai_atlas/hook_catalogue.md:12,135,137 | H | catalogue data columns "live swkotor2.exe (LAA)" measured 10-02 pre-migration; values still valid (1-byte header diff) |
| everything else (see counts) | H | dated session / findings / status-log / pasted tool-output records |

## Script verdicts against today's live exe

- `patch_manager_mods/mods678_research/verify_windows_both_exes.py`: loads `$G/swkotor2.exe` and `$G/swkotor2.exe.unpatched_vanilla`, compares each `check_windows_output_*.txt` window's original bytes in both, prints `LAA:True base:True`. Ran it read-only now: `ALL MATCH`. Correct logic; both files exist. Only the labels/docstring are stale (live is no longer "4AB72FC1"; the two exes now differ by the single PE-flag byte, so the comparison is vanilla vs 4gb-patched). Not hash-keyed.
- `patch_manager_mods/steam_db_pr3/work/pe.py`: reads `.unpatched_vanilla` (6A522E71, exists). Fine.
- `scripts/check_hook_bytes.py` / `_static_common.py`: use `backups/vanilla_clean/swkotor2.exe` (sha256 6A522E71, verified); `require_version` checks that hash. Fine.
- `patch_manager_mods/mods678_research/stage_and_verify_install.py` (no keyword hit; checked because HANDOFF says "LAA hook set"): `STEAM = "6A522E71..."`, keeps hooks whose `target_versions` contains 6A522E71 (or is empty); `verify` prints the live `target_version_sha` prefix. Correct today; no 4AB72F logic.
- `engine_ai_atlas/tools/tests/helpers.py`: fixture, leave.
- No script is keyed on hash 4AB72F for the live exe.

## Proposed edits (old -> new)

### patch_manager_mods/README.md (lines 97-102)
OLD:
```
3. **Existing exe-level patch already applied.** Mod #4 (4GB/LAA Patcher) and
   mod #1 (3C-FD Patcher, which also applies an LAA patch and explicitly
   **refuses to run on an already-patched executable** — see its install
   note) already modify `swkotor2.exe`'s PE headers. `KPatchLauncher.exe`
   injects at runtime rather than patching the binary on disk, so it should
   be independent of LAA — but this needs confirming, not assuming, per
```
NEW:
```
3. **Exe-level patch.** (Updated 2026-10-02: the live exe is vanilla `6A522E71`
   + KPM's own `4gb-patch` = `7751F7D8`; the old Mod #4 / Mod #1 (3C-FD) prepatched build
   `4AB72FC1` is retired.) The one byte `4gb-patch` flips is the PE LAA flag. `KPatchLauncher.exe`
   injects at runtime rather than patching the binary on disk, so it should
   be independent of LAA — but this needs confirming, not assuming, per
```

### patch_manager_mods/03_framework_research.md (line 111-113)
OLD:
```
- **`4GBPatch` (Patch Manager's own) directly overlaps with mod #4 (4GB
  Patcher, ntcore.com), already applied.** Do not install Patch Manager's
  copy — same static LAA-flag patch, redundant at best, corrupting at
  worst if both try to flip the same PE header bit independently.
```
NEW:
```
- **`4GBPatch` (Patch Manager's own) overlaps with mod #4 (4GB Patcher, ntcore.com).**
  (Superseded 2026-10-02: mod #4 / the 3C-FD prepatched exe was retired; the live exe is vanilla
  `6A522E71` + KPM `4gb-patch` = `7751F7D8`, so Patch Manager's copy IS the one installed.
  Never combine it with an externally LAA-patched exe: two independent writers of the same PE header bit.)
```

### patch_manager_mods/04_recommended_installs.md (lines 46-50)
OLD:
```
- **Patch Manager's own `4GBPatch`.** Directly redundant with mod #4 (4GB
  Patcher) already applied to this exe. Installing both risks two
  independent LAA-flag writes to the same PE header field.
```
NEW:
```
- **(Superseded 2026-10-02) Patch Manager's own `4GBPatch`.** It is now the installed LAA mechanism
  (`4gb-patch`, live exe `7751F7D8` = vanilla `6A522E71` + 1 byte at 0x11E). Do NOT combine it with
  mod #4 or the 3C-FD prepatched exe: two independent LAA-flag writes to the same PE header field.
```

### patch_manager_mods/06_gog_to_steam_conversions.md
Line 677-678 OLD:
```
8. Install on the live game directly (same LAA-hash two-part recipe as
   any other patch, `05_build_plan.md`'s Phase 1a section) and check the
```
NEW:
```
8. Install on the live game directly (KPM matches the original vanilla hash `6A522E71`; no LAA
   hash/db aliases since 2026-10-02, see `vanilla_migration/README.md`) and check the
```
Lines 689-698 OLD (whole bullet beginning `- **LAA-hash mismatch — RESOLVED as of Phase 1a/1b (2026-09-28), recipe` through `` `kotor2-steam-aspyr.hooks.toml` — no new unknowns here.``) NEW:
```
- **LAA-hash mismatch — OBSOLETE since 2026-10-02.** The live exe is now vanilla `6A522E71` +
  KPM `4gb-patch` (`7751F7D8`); KPM matches `target_versions` and the address DB by the original
  hash `6A522E71`. A new patch needs only `kotor2_steam_aspyr` in `manifest.toml` and
  `6A522E71...` in its `kotor2-steam-aspyr.hooks.toml` `target_versions`; do NOT add a
  `kotor2_steam_aspyr_laa` key, a `4AB72FC1` hash, or clone any `*_laa.db`. (The 09-28 two-part
  recipe is kept in `05_build_plan.md` Phase 1a as history.)
```

### patch_manager_mods/08_level_up_override.md (lines 4-5)
OLD:
```
`research/companion_levelup/MASTER_PLAN.md`. Target: Steam Aspyr LAA `swkotor2.exe` (sha256 6A522E71... = `.unpatched_vanilla`; the
live exe 4AB72FC1... is the patched one - bytes at every shortlisted site are identical in both). Ghidra image base 0x400000; file
```
NEW:
```
`research/companion_levelup/MASTER_PLAN.md`. Target: Steam Aspyr `swkotor2.exe` (sha256 6A522E71... = `.unpatched_vanilla`; since 2026-10-02 the live exe is that vanilla + KPM `4gb-patch` = 7751F7D8..., formerly the 3C-FD build 4AB72FC1... - bytes at every shortlisted site are identical in all). Ghidra image base 0x400000; file
```

### patch_manager_mods/10_engine_optimization_research.md (line 224)
OLD:
```
- Target: `swkotor2.exe` Steam/Aspyr, hash `6A522E71...`; live copy is the LAA variant `4AB72FC1...`, addresses identical. Goal:
```
NEW:
```
- Target: `swkotor2.exe` Steam/Aspyr, hash `6A522E71...`; live copy is that vanilla + KPM `4gb-patch` (`7751F7D8...`; was the 3C-FD/LAA build `4AB72FC1...` until 2026-10-02), addresses identical. Goal:
```
(the rest of line 224 unchanged)

### patch_manager_mods/09_load_hang_investigation.md (line 52, low)
OLD: `Game is the Steam Aspyr build (LAA variant, hash `4AB72FC1...`, original `6A522E71...`) using OpenGL`
NEW: `Game is the Steam Aspyr build (during this investigation the 3C-FD/LAA build `4AB72FC1...`; since 2026-10-02 vanilla `6A522E71...` + KPM `4gb-patch` = `7751F7D8...`) using OpenGL`

### patch_manager_mods/02_buff_duration_hud.md (line 1671)
OLD:
```
**Hooks** (`kotor2-steam-aspyr.hooks.toml`, all detours, `original_bytes` verified vs the live exe; target hashes: Steam `6A522E71...` and its LAA build `4AB72FC1...`):
```
NEW:
```
**Hooks** (`kotor2-steam-aspyr.hooks.toml`, all detours, `original_bytes` verified vs the live exe; target hash: Steam `6A522E71...` only (the former LAA alias `4AB72FC1...` was dropped 2026-10-02)):
```

### patch_manager_mods/wrap_measure_fix_src/README.md (line 3)
OLD: `A tiny KPM "simple" byte patch for KOTOR II (Steam Aspyr and Steam Aspyr LAA exes).`
NEW: `A tiny KPM "simple" byte patch for KOTOR II (Steam Aspyr exe, hash 6A522E71; the LAA alias was dropped 2026-10-02, the live exe is vanilla + 4gb-patch).`

### patch_manager_mods/issue_drafts/issue_4_steam_address_library.md (line 8)
OLD: `(`swkotor2.exe`, SHA256 `6A522E...`, LAA variant `4AB72FC1...`)`
NEW: `(`swkotor2.exe`, SHA256 `6A522E...`, run with KPM's `4gb-patch` applied)`

### patch_manager_mods/mods678_research/README.md (line 24)
OLD: `compares every window's original bytes against BOTH executables (LAA `4AB72FC1`, base `6A522E71`); 2026-09-30 all 11 identical`
NEW: `compares every window's original bytes against BOTH executables (live `swkotor2.exe`: 2026-09-30 = LAA 3C-FD build `4AB72FC1`, since 2026-10-02 = vanilla + 4gb-patch `7751F7D8`; base = `.unpatched_vanilla` `6A522E71`); 2026-09-30 all 11 identical`

### patch_manager_mods/mods678_research/HANDOFF_2026-09-30.md (line 22)
OLD: `matched by live DLL md5 + identical LAA hook set`
NEW: `matched by live DLL md5 + identical hook set for the 6A522E71 target`

### patch_manager_mods/mods678_research/verify_windows_both_exes.py (lines 2-4 and 28)
OLD (docstring):
```
swkotor2.exe (LAA build, sha 4AB72FC1...) and swkotor2.exe.unpatched_vanilla (base build, sha 6A522E71...).
2026-09-30: all 11 windows identical in both. Re-run after any game/exe change."""
```
NEW:
```
swkotor2.exe (live: since 2026-10-02 vanilla + KPM 4gb-patch, sha 7751F7D8...; before that the 3C-FD/LAA build 4AB72FC1...)
and swkotor2.exe.unpatched_vanilla (base build, sha 6A522E71...). The two now differ only by the PE LAA flag byte at 0x11E.
2026-09-30: all 11 windows identical in both. Re-run after any game/exe change."""
```
OLD (line 28): `LAA:{res['swkotor2.exe']}  base:{res['swkotor2.exe.unpatched_vanilla']}`
NEW: `live:{res['swkotor2.exe']}  base:{res['swkotor2.exe.unpatched_vanilla']}`
(Callers/docs grep for the literal `LAA:True` in pasted outputs only; the pasted R1_engine_followups.md output blocks are history and need no change.)

### patch_manager_mods/kotor2-batch3-porting-plan.md (top banner, add after line 1; low)
NEW (insert):
```
> **2026-10-03 note:** the 2026-09-28 LAA-alias steps below (live hash `4ab72fc1...`, `4AB72FC1` in `target_versions`, `kotor2_steam_aspyr_laa` manifest key / `_laa.db`) are OBSOLETE. The live exe is vanilla `6A522E71` + KPM `4gb-patch` (`7751F7D8`); use only `kotor2_steam_aspyr` / `6A522E71` (see `vanilla_migration/README.md`).
```

### patch_manager_mods/18_mouse_camera_investigation.md (line 60, low)
OLD: `Note [V]: the exe is unmodified (`CurrentHash == OriginalHash == 4AB72FC1...`);`
NEW: `Note [V] (as of 09-26): the exe was unmodified by us (`CurrentHash == OriginalHash == 4AB72FC1...`); since 10-02 the live exe is vanilla `6A522E71` + `4gb-patch` (`7751F7D8`, only the PE LAA flag differs);`

### patch_manager_mods/K1_ENGINE_BUGS_IN_K2.md (lines 8-9, low)
OLD:
```
sha256 `6A522E71631DCEE9...FFEF`. Its LAA variant
`4AB72FC1...5A5B` has the same code at every address listed here.
```
NEW:
```
sha256 `6A522E71631DCEE9...FFEF`. A large-address-aware build of it (e.g. via KPM's `4gb-patch`, or the old prepatched
`4AB72FC1...FB5A`) has the same code at every address listed here.
```
(the original tail `5A5B` is a typo for `FB5A`)

### memory/kotor2-patch-manager.md (note: memory files are edited by the user/Opus, not by this sweep)
Lines 265-267 OLD:
```
**Resolved (Phase 1a, 2026-09-28) — LAA-hash mismatch, two-part fix, both
required:** the live exe's SHA (`4ab72fc1...36fb5a` — pristine
```
NEW:
```
**OBSOLETE since 2026-10-02 (live exe = vanilla `6A522E71` + `4gb-patch` = `7751F7D8`; no LAA aliases/dbs; keep only for history). Was: Resolved (Phase 1a, 2026-09-28) — LAA-hash mismatch, two-part fix, both
required:** the live exe's SHA (`4ab72fc1...36fb5a` — pristine
```
and at line ~290 OLD `**Repeat this exact two-part recipe for the next mod** — don't stop after step 1,` NEW `**(Obsolete 2026-10-02: do NOT repeat for new mods; target 6A522E71 only.)** Historically: repeat this two-part recipe for the next mod — don't stop after step 1,`
Lines 120-124 OLD: `**The LAA-hash question that\n   doc used to list as a blocker is now resolved** — Phase 1a/1b proved\n   the two-part recipe (manifest/hooks hash entry + address-db clone)\n   works, so it's just a mechanical step now, not an open decision.`
NEW: `**The LAA-hash question that\n   doc used to list as a blocker is moot since 2026-10-02** (live = vanilla 6A522E71 + 4gb-patch;\n   no LAA hash entry or address-db clone is needed any more).`
Line 477 OLD: `already exists in `kotor2_steam_aspyr_laa.db` — checked` NEW: `already exists in `kotor2_steam_aspyr.db` (was `_laa.db`, removed 10-02) — checked`
Optional: append to line 1061 `(Done 10-02 12:02: vanilla + 4gb-patch live.)`

### Optional banners for HISTORY-only docs (no hit lines misleading, but a reader resuming from the top could follow them)
- `patch_manager_mods/05_build_plan.md` (13 hits, all in dated "Outcome (2026-09-28)" / "Phase 1a/1b" sections): add under the title `> 2026-10-03: the LAA-hash / kotor2_steam_aspyr_laa recipe below is historical; live exe is now vanilla 6A522E71 + 4gb-patch (7751F7D8), see vanilla_migration/README.md.`

## Skipped HISTORY / FIXTURE hit counts (matching lines per file; M-class lines above are included in the file's total where noted)

Total matching lines outside vanilla_migration/: 127 in .md (incl. 16 in memory files), 5 in .py (+1 pasted-output), 0 in CLAUDE.md, 0 in MEMORY.md.

| file | total hit lines | listed above as M | skipped (H/F) |
|---|---|---|---|
| memory/kotor2-engine-research.md | 1 | 0 | 1 |
| memory/kotor2-patch-manager.md | 12 | 4 groups (121, 265-284 [265,266,267,272,283,284], 477) = 8 lines | 4 (238, 315, 703, 1061) |
| memory/kotor2-ultrawide-gui.md | 1 | 0 | 1 |
| memory/kotor2-vanilla-exe-migration.md | 1 | 0 | 1 (correct text) |
| 01_inventory_3d_viewport.md | 1 | 0 | 1 |
| 02_buff_duration_hud.md | 5 | 1 (1671) | 4 (352, 491, 1621, 1638) |
| 03_framework_research.md | 2 | 1 (113) | 1 |
| 04_recommended_installs.md | 1 | 1 | 0 |
| 05_build_plan.md | 13 | 0 | 13 |
| 06_gog_to_steam_conversions.md | 9 | 2 groups (677, 689/692) = 3 lines | 6 |
| 07_force_power_hotbar.md | 1 | 0 | 1 |
| 08_level_up_override.md | 7 | 2 (4,5) | 5 (570 `3cfd` fixture, 712, 722, 725, 726) |
| 09_load_hang_investigation.md | 3 | 1 (52) | 2 (259, 280: `unpatched_vanilla`-free; matched on LAA wording of dated sections) |
| 10_engine_optimization_research.md | 12 | 1 (224) | 11 |
| 10_equipment_3d_character.md | 2 | 0 | 2 (`3cfd` false positives) |
| 13_ranged_ai_spacing.md | 1 | 0 | 1 |
| 15a_research_session_context_2026-10-01.md | 1 | 0 | 1 |
| 16_candidate_register.md | 1 | 0 | 1 |
| 17_dialogue_letterbox_and_wrap_fixes.md | 2 | 0 | 2 |
| 18_mouse_camera_investigation.md | 3 | 1 (60) | 2 (19, 170) |
| 20_TESTING_PLAN.md | 1 | 0 | 1 |
| 20a_patch_test_audit_2026-10-03.md | 2 | 0 | 2 (accurate audit findings) |
| 20d_phase1_session_2026-10-03.md | 1 | 0 | 1 |
| K1_ENGINE_BUGS_IN_K2.md | 2 | 2 | 0 |
| README.md (patch_manager_mods) | 3 | 3 (97, 98, 102) | 0 |
| engine_ai_atlas/PROBE_DESIGN.md | 1 | 0 | 1 |
| engine_ai_atlas/hook_catalogue.md | 3 | 0 | 3 |
| engine_research_findings/R1.md | 3 | 0 | 3 (LAA property true) |
| engine_research_findings/U5.md | 1 | 0 | 1 |
| engine_research_findings/V7.md | 2 | 0 | 2 |
| engine_research_findings/W7.md | 1 | 0 | 1 |
| ghidra_knowledge/README.md | 1 | 0 | 1 (schema text "Steam+LAA" binaries row; harmless) |
| issue_drafts/issue_4_steam_address_library.md | 1 | 1 | 0 |
| kotor2-batch3-porting-plan.md | 5 | 5 (31,127,131,151,179; low) | 0 |
| mods678_research/HANDOFF_2026-09-30.md | 1 | 1 | 0 |
| mods678_research/README.md | 1 | 1 | 0 |
| mods678_research/ai_research_2026-10-02/R1_engine_followups.md | 10 | 0 | 10 (pasted `LAA:True base:True` output) |
| mods678_research/ai_research_2026-10-02/R2_script_followups.md | 1 | 0 | 1 |
| mods678_research/ai_review_2026-10-01/build_reviews/review_dll_build.md | 1 | 0 | 1 |
| mods678_research/ai_review_2026-10-01/designs/design_ai_engine_fixes.md | 2 | 0 | 2 |
| mods678_research/ai_review_2026-10-01/designs/design_ai_trace.md | 3 | 0 | 3 |
| steam_db_pr3/20_address_db_session_context_2026-10-02.md | 1 | 0 | 1 |
| wrap_measure_fix_src/README.md | 1 | 1 | 0 |
| .py: check_hook_bytes.py:17, pe.py:2, helpers.py:15 | 3 | 0 | 3 (harmless/fixture) |
| .py: verify_windows_both_exes.py:3,28 (+17 functional reference) | 3 | 2 (cosmetic) | 1 (functional, correct) |
