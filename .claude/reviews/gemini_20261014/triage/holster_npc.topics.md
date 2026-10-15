# holster_npc triage (bundle 6: 6h holster, 6n npc)

- HN-T01 P2: HN-6h-01,HN-6h-05 -- In hs::Fsm
- HN-T02 P1: HN-6h-02 -- Does the holster hook at 0x005DF926 (source=ecx) read a valid `this` given the cmdq-probe hook at 0x005DF920 a
- HN-T03 P2: HN-6h-03 -- Can applyBoneTest reach codepatch::writeChecked (memcpy of 5 bytes at the site) without a prior codepatch::com
- HN-T04 P3: HN-6h-04 -- Does HolsterCombat log old=0 edge=1 when creature+off is unreadable because rdU32 returns 0 on saferead failur
- HN-T05 P2: HN-6h-06 -- Can saferead::readAt with 32-bit uintptr_t base near 0xFFFFFFFF and positive off wrap base+off to an address t
- HN-T06 P1: HN-6n-01 -- In engineSpawn
- HN-T07 P2: HN-6n-02,HN-6n-04 -- In World::onCreatureAdded
- HN-T08 P2: HN-6n-03 -- If the engine tag read via [c+0x24]+0x18 in NW_OnAddToArea is empty or differs from the tag passed to Spawn
- HN-T09 P2: HN-6n-05 -- Does fmtRef16's hex branch write 52 bytes regardless of n
- HN-T10 P3: HN-6n-06 -- Is dumpEvent's budget/line accounting (fits(kEventLines)
- HN-T11 P3: HN-6n-07 -- Does nw_registry_test.cpp lack a case where one dispatch callback unregisters an entry and registers a new one

## Duplicates
- HN-6n-02 and HN-6n-04 share one read (World::onCreatureAdded area re-arm); folded into HN-T07, kept as distinct findings (04 is the same-resref/new-areaPtr variant).
- HN-6h-01 (Fsm stow/reset) and HN-6h-05 (Fsm test gap, onSwap clearing stowPending) folded into HN-T01. No pure duplicates dropped.
- Note: HN-6h-04 and HN-6h-06 both touch saferead (rdU32 failure semantics, readAt overflow); separate questions.

## Findings citing files outside the bundle manifest
- None. All cited files are in 6_holster.manifest.tsv. Gemini says it did not see kpatch_api.h, KpLog.h, windows.h stub (not in repo bundle).
- HN-6n-02 mentions design docs 38/40 only as omitted, no finding.

## Cross-batch candidates
- patch_manager_mods/_shared_src/SafeRead.h readAt wraparound (HN-6h-06) and readAt failure semantics (HN-6h-04): other batches using saferead may flag the same.
- patch_manager_mods/_shared_src/CodePatch.h writeChecked pre-committed() read (HN-6h-03): other mods (robustness_fixes, equip_icon_fix) use writeChecked; header comment says callers call committed() first.
- patch_manager_mods/npc_world_src/NwCore.h (HN-6n-02/04/07) and NpcWorldProvider.cpp (HN-6n-01/03): likely also reviewed in an NPC-diversity/npc-world batch.
- patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp (HN-6n-05/06): may overlap a K1 asset-import batch.

## Backlog (not assessed)
- kpatch_api.h, KpLog.h, host windows.h stub not provided (unresolvable includes).
- Design docs 38 and 40 omitted from bundle by size cap.
- No swkotor2.exe disassembly: CSWSCreature vtable slot 0 (0x00561F30/0x00572A70) and ECX preservation across cmdq-probe at 0x005DF920..0x005DF926 unverified; needs rea-recon or query_ghidra.py for HN-T02/HN-T06.
