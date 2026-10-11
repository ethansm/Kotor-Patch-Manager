# A1 exe recon: which exe the Steam DB and ghidra_knowledge key to

1. Exe path used: `~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe` (resolves to `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe`). Path convention also set in `tests/suites.toml:62` (`KOTOR2_VANILLA_EXE` default).
2. sha256: `6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef` (prefix `6A522E71`, matches the brief).
3. Size: 6588416 bytes.
4. Keys to this exe:
   - Steam DB `kotor2_steam_aspyr.db` (`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/AddressDatabases/`; the worktree has no copy): `game_version.sha256_hash` = 6A522E71..., version_string `steam aspyr`, platform Windows. Counts: 0 classes, 48 functions, 14 global_pointers, 21 offsets.
   - `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db`, `binaries` row `6a522e71...` label `kotor2_steam_aspyr`, program `swkotor2.exe`, note "Pristine Steam Aspyr build, no LAA flag".
   - KPM manifests and hooks target 6A522E71 (e.g. `patch_manager_mods/npc_diversity_src/manifest.toml:14`).
   - Game dir `G` as written in `patch_manager_mods/BUILD_AND_INSTALL.md:9`: `~/.steam/steam/steamapps/common/Knights of the Old Republic II`. Not listed or walked.
5. Copy status: DONE. Copied to the scratchpad, not `/tmp/claude-1000/rea_probe/` (the environment rule puts scratch files in the scratchpad). Copy path: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/ed54f7d8-0ac3-4af3-8e27-7b2a5ece43fe/scratchpad/rea_probe/swkotor2.exe`. Copy sha256 = `6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef` (matches the source, 6588416 bytes).

## Discrepancies found (not resolved)
- Knowledge DB row `4ab72fc1...` (label `kotor2_steam_aspyr_laa`) says "Live install target = pristine + LAA flag flip only". This conflicts with `patch_manager_mods/04_recommended_installs.md:49` (live = `7751F7D8` = vanilla + 4gb-patch) and `patch_manager_mods/vanilla_migration/j0o_gap_analysis.md:4` (`4AB72F` = 3C-FD J0-o exe). No knowledge row exists for `7751F7D8`.
- The live exe was not hashed or touched (per brief). Its `7751F7D8` value comes from the docs only.
