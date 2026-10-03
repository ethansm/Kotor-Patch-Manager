# Vanilla exe migration (2026-10-02 12:02)

## RESUME HERE (session context, 2026-10-03)
**Live (checked 10-03, sibling install 11:29:44):** exe `7751F7D8` = vanilla `6A522E` + `4gb-patch`; KPM state OriginalHash
`6A522E`. 31 ids / 198 hooks. Shader set: `4gb-patch`, `k2_aspyr_shader_fixes`, `K2_music_fix` ON; `colorshift_k2`,
saber-hilt, normal-map, `fog_reflection_fix_k2` OFF. Sibling ids also live: mouse-probe, k2-robustness-fixes,
k2-texture-memory-release.

**Follow-up done 10-03 ~12:00 (plan "branch workflow, launcher rebuild, stale-LAA sweep"):**
- **Branch workflow adopted.** `ethansm/local-mods` = what's live, pushed to the fork at `977f4f3` (WIP commits: BuffDurationHUD,
  LevelUpOverride, RobustnessFixes, ghidra scripts, MouseProbe, launcher NoLaunch). Held (sibling sessions mid-edit at
  11:46-11:49): ForcePowerHotbar + docs/RuntimeGuiControls.md, CharEffectsPanel; BuffDurationHUD dirty again (new .inc/.h).
  Excluded locally: `launch_kpm_ui.sh`, `registry_core_stage/`, `Patches/K2DialogueLetterboxFixProbe/` (duplicate id
  `k2-dialogue-letterbox-fix`). Inventory: `phase1_inventory.md`.
- **Commit-before-install rule** (doc 10 Standing rules + CLAUDE.md "KPM Install Rule"): `stage_and_verify_install.py stage`
  writes `<stage>/STAGE_INFO.txt` (branch/head/upstream/per-id clean|DIRTY), prints `WARN DIRTY`, exits 2 unless
  `--allow-dirty`; `verify` prints the recorded head.
- **Launcher rebuilt** (includes upstream #283; `KPATCH_NO_LAUNCH` now in source, local commit 977f4f3, not for upstream).
  `bin/NoLaunch` = new build: KPatchLauncher.dll `f9527abb`, KPatchCore.dll `8629dde6`, KotorPatcher.dll `36f10b7f` (copied
  from live, NOT built), binkw32 proxy `753ca1f1`. Rollback = `bin/NoLaunch_old_0928` (launcher `5bc31e49`, core `b236fcec`,
  patcher `2a706ac9`). dotnet test 427/427 before and after. Sandbox A/B (old vs new, first install + reinstall):
  byte-identical except KotorPatcher.dll; no new #283 state fields; reinstall restores the vanilla backup and re-patches.
- **RFC #180 registry patcher went live 11:51** (sibling registry session hot-copied `registry_core_stage/KotorPatcher.dll`,
  md5 `36f10b7f`, source `fa67a3b`). The OLD bin ships `2a706ac9`: installing with `NoLaunch_old_0928` reverts it.
- **Live reinstall with the new launcher 12:00:17** (stage `kotor-patch-manager/launcher_swap_stage_1003`, head 977f4f3,
  hotbar build 28 `6dd6d0e2` passed as EXTRA from `hotbar_stage17/`, because the newest tree kpatch is the uninstalled 29p
  probe). Result: every DLL/proxy/patcher md5 + exe sha `7751F7D8` identical to backup `backups/pre_launcher_swap_1003/`;
  OriginalHash `6A522E`; verify 0 mismatches; 31 ids/198 hooks; patch_config per-id identical (hotbar moved last; no shared
  hook addresses, so order is irrelevant).
- **Stale-LAA sweep** applied (commit b4c1612; `phase3_sweep.md`); memory kotor2-patch-manager.md LAA recipe marked obsolete.

**Open items (in order):**
1. User reported "very dark" after shader changes. `colorshift_k2` removed 10-02 14:58; user has NOT yet re-tested.
   If still dark: in-game Brightness (ini `Brightness=50`) first, else ask which areas; suspect Shader Fixes' +15
   brightness offset removal -> tweak in a branch (do not just drop the patch; it carries the saber/normal/fog fixes).
2. Held WIP commits (ForcePowerHotbar, CharEffectsPanel, BuffDurationHUD .inc/.h) once their sessions settle.
3. Upstream PRs: none (user, 10-03). `docs/prepatched-executables` branch stays on the fork.

**Install rules:** siblings hot-swap live DLLs and the patcher without KPM (11:51 chareffects + KotorPatcher) and
reinstall often; right before any install: game closed, re-read live ids, `stage <dir>` (4gb-patch "hooks=DIFF" is
expected: static hook not in patch_config; a `dll=NO-MATCH` means pass the live build's kpatch as EXTRA), back up,
`KPATCH_NO_LAUNCH=1 bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches <stage> <ids>`, then `verify` + md5 vs backup.
Exe swaps need `swkotor2.exe.backup.*` + `kpm_install_state.json` moved aside first.

**Status: INSTALLED, awaiting in-game check.** 12:14: `colorshift_k2` + `K2_music_fix` added (31 ids / 202 hooks incl. a sibling's
`mouse-probe`, hand-installed 12:11 and carried over unchanged). LAA aliases removed from all manifests/hooks (KPM
`b5806b1`, mirrors here). Community note with the 3C-FD hash: KPM `docs/PrePatchedExecutables.md` (branch
`docs/prepatched-executables`, PR candidate for issues #49/#91). The live `swkotor2.exe` moved from the 3C-FD/LAA build
(`4AB72F…`) to vanilla Steam Aspyr (`6A522E…`), with 3C-FD's changes supplied by KPM patches instead.

## What changed
- Exe: `backups/vanilla_clean/swkotor2.exe` restored, then KPM's `4gb-patch` (static, 1 byte at 0x11E: 0x03 -> 0x23).
  Live hash now `7751F7D8…` (vanilla + LAA). `kpm_install_state.json` OriginalHash = `6A522E…`,
  OriginalVersion = `kotor_2 1.0.2 (aspyr+steam)_windows_steam_x86` (was "Unknown").
- Patch set (28 ids, 197 hooks; was 28 / 186): removed `k2-aspyr-saber-hilt-texture-fix` and `k2-aspyr-normal-map-fix`
  (both conflict with Shader Fixes, which includes them); added `4gb-patch` and `k2_aspyr_shader_fixes` 1.3.2
  (upstream 55725f7, includes #278 skinned-mesh fog and #280 AMD black surfaces). `fog_reflection_fix_k2` NOT
  installed: it hooks 0x990020/0x990918/0x990EC8, the same programs Shader Fixes replaces.
- All 26 other DLLs/hooks unchanged (md5 identical, per-id hook counts identical). Proxy/KotorPatcher unchanged.

## Pre-checks done
- 186/186 live hooks: original_bytes match the vanilla exe, none touch the 3C-FD-changed bytes.
- 14/14 new hooks (4gb + shader fixes) match vanilla; only overlaps were with the two removed patches.

## Not carried over from 3C-FD (yet)
- J0-o's in-place shader text edits (~5,120 bytes, VA ~0x989DBA..0x9F8B35) are replaced by KPM Shader Fixes, which is a
  different implementation: fog/lighting may look slightly different. Gap analysis: `j0o_gap_analysis.md`.
- 3-byte NOP at VA 0x70E130 (file 0x30D530; removes `fmul dword [ebp-0x28]`): NOT fog. It is in
  `CExoSoundSourceInternal::SetVolume` and drops the 0.2 dialogue ducking of music/ambient. Byte-identical to KPM's
  `K2_music_fix` (K2AspyrMusicFix), which is not installed.
- 0x989DB0 gamma pass colour grade (R/B x0.90, G x0.85) is byte-identical to KPM's `colorshift_k2`
  (K2AspyrColorCorrection), not installed. Check side by side first: Shader Fixes already removes the +15 brightness offset.
- J0-o's fog/vertex-program edits = an older `K2AspyrFogReflectionFix`; Shader Fixes supersedes them, nothing to port.
- Verified 10-02: all 5,124 3C-FD bytes fall inside 4GBPatch + FogReflectionFix + ColorCorrection + MusicFix hook ranges.
  Full analysis: `j0o_gap_analysis.md`.

## Gotcha (installer)
KPM's uninstall restores the newest `swkotor2.exe.backup.*` and keeps `OriginalHash` from `kpm_install_state.json`.
Swapping the exe requires moving both aside first (done; they are in the backup below), or KPM puts the old exe back.

## Install / rollback
- Stage: `~/Games/KOTOR2_Mod_Workspace/stage_vanilla_1002_1202/` (= stage_probe_1136 - saber/normal + 4GBPatch.kpatch +
  K2AspyrShaderFixes.kpatch zipped from Patches/*.toml). Command:
  `KPATCH_NO_LAUNCH=1 bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches <stage> <28 ids>` (NoLaunch build is
  pre-#283; rebuild still pending).
- Backup: `~/Games/KOTOR2_Mod_Workspace/backups_vanilla_migration_1002_1202/` (old exe `4AB72F`, old exe backups +
  json in `exe_backups/`, `kpm_install_state.json.moved`, patch_config.toml, patches/, proxy DLLs, DLL md5s).
- Rollback: game closed; move the current `$G/swkotor2.exe.backup.*` and `kpm_install_state.json` aside; copy back the
  backup's `swkotor2.exe`, `exe_backups/*` and `kpm_install_state.json.moved` -> `kpm_install_state.json`; reinstall
  the 28 old ids from `stage_probe_1136`.

## KPM repo
Fork `ethansm/Kotor-Patch-Manager` = `origin`, `LaneDibello/...` = `upstream` (push disabled). Local work committed on
`ethansm/local-mods` (dcc1cf6 LAA aliases, 2dcf48a Ghidra scripts, 583ba96 local patches), pushed. `master` tracks
`upstream/master`. Local excludes in `.git/info/exclude` (`*_live_patches/`, stages, laa.db, `*.kpatch.old`).
`_laa` aliases dropped in `b5806b1`; `kotor2_steam_aspyr_laa.db` (source + bin copies) moved to the backup's `laa_db/`.
New stages: build with `mods678_research/stage_and_verify_install.py` (now keys on the vanilla hash).

## 14:58 colorshift_k2 removed
User: "it's very dark now after the shader changes". Reinstalled the 12:50 live set (31 ids incl. sibling
k2-robustness-fixes + k2-texture-memory-release) without `colorshift_k2`; stage `stage_nocolor_1002_1457`,
backup `backups_nocolor_1002_1458`. If still too dark: compare with `k2_aspyr_shader_fixes` off (its +15 brightness
offset removal / gamma change is the next suspect), then in-game Brightness (swkotor2.ini Brightness=50).
