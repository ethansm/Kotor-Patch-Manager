# HN-T03
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6h-03 | PARTIAL | patch_manager_mods/_shared_src/CodePatch.h:89-91; patch_manager_mods/holster_src/HolsterBonePatch.h:18-21; patch_manager_mods/holster_src/Holster.cpp:234,241 | yes | No committed() gate before writeChecked memcpy; fault needs unmapped site, gated only by k2addr (outside range). |
- patch_manager_mods/holster_src/Holster.cpp:241 holds the anchor text verbatim; HolsterBonePatch.h does not. Finding is misattributed, not invented.
- patch_manager_mods/holster_src/Holster.cpp:233: applyBoneTest returns at once while g_boneTest is off (default OFF per :231).
- patch_manager_mods/holster_src/Holster.cpp:234: refuses when k2addr::get() is null; the only guard before the write path. Table contents are outside range.
- patch_manager_mods/holster_src/Holster.cpp:237: sites are rhand (always on) and lhand (on only with g_boneLeft).
- patch_manager_mods/holster_src/HolsterBonePatch.h:12-13: fixed push-imm32 sites at 0x763AF9 and 0x763E2B.
- patch_manager_mods/holster_src/HolsterBonePatch.h:18-21: patchNodeLiteral builds the 5-byte repl and calls writeChecked directly; no committed() call.
- patch_manager_mods/_shared_src/CodePatch.h:70-74: committed() uses VirtualQuery, which returns false for free regions without faulting; it does not test readability. A committed() pre-check before writeChecked is therefore a sound fix.
- patch_manager_mods/_shared_src/CodePatch.h:89-91: writeChecked runs memcpy(seen, p, n) before any check; no committed() call inside the function.
- patch_manager_mods/_shared_src/CodePatch.h:87-88: comment says seen is captured before the compare on purpose (evidence capture). The read-first is design, not an oversight.
- patch_manager_mods/_shared_src/CodePatch.h:94: VirtualProtect failure (-2) is reached only after the read at :91, so it cannot prevent a fault there.
