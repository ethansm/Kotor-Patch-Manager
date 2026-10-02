# Vanilla exe migration (2026-10-02 12:02)

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
