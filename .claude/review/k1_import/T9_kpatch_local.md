OUTPUT: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/review/k1_import/T9_kpatch_local.md
EVIDENCE: patch_manager_mods/BUILD_AND_INSTALL.md:54; scripts/_static_common.py:373-379; patch_manager_mods/npc_diversity_src/README.md:38
FINDING: .kpatch is a zip (manifest.toml, DLL, hooks.toml) with only detour/pointer/int/uint/simple hook params; no 2DA, GFF or file ops.
OPEN: Upstream KPM .kpatch spec is not in this repo; create-patch.py "extras" (BUILD_AND_INSTALL.md:54) are undefined here.
VERDICT: REFUTED

## Answers
1. No in-repo .kpatch spec. Packaging facts: scripts/_static_common.py:373-379 (zip members); patch_manager_mods/BUILD_AND_INSTALL.md:54 (create-patch.py packs *hooks.toml, binaries, extras); patch_manager_mods/03_framework_research.md:15 (KPatchCore parses .kpatch/manifest.toml).
2. 2DA: none. GFF/.utc: none. Hook schema only: [[hooks]] address, function, original_bytes, skip_original_bytes, replacement_bytes (patch_manager_mods/portrait_slots_src/kotor2-steam-aspyr.hooks.toml:5-9); [[hooks.parameters]] type = detour|pointer|int|uint|simple (same file :12).
3. Yes, but outside .kpatch: npc_diversity mod writes whole-file override/appearance.2da and heads.2da via scripts/generate_diversity_2da.py:53 (DEFAULT_OUT_DIR); patch_manager_mods/npc_diversity_src/README.md:38 and :52. Also scripts/02_merge_2da_and_tlk.py:233 write_2da into LIVE_OVERRIDE. BUILD_AND_INSTALL.md has 0 "2da" hits.
4. Binary add/replace: none in .kpatch. kpm_hotswap swaps only binaries/windows_x86.dll (BUILD_AND_INSTALL.md:54). Override assets ship out-of-band: scripts/add_equip_3dview_control.py:33 (OVERRIDE = GAME/"override"); patch_manager_mods/03_framework_research.md:76 (hand-copied partyselect_p.gui).
Caveat: patch_manager_mods/34_npc_diversity_dispatcher.md:267 says staged 2DAs are whole-file snapshots outside KPM ownership.
