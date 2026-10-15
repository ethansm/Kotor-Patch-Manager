# G2.3 skeptic input: holster_npc (8 fids)

## HN-T01 (P2): In hs::Fsm, does onSwap (clears stowPending without checking lastCombat==0) leave the weapon permanently PhDrawn after an out-of-combat swap during the stow delay, and does tick restart PhStowing at fraction 1.0 when a draw is still in progress?
- **HN-6h-05** v2=PARTIAL (G1 PARTIAL) | claimed medium test-gap | No tests for onCombat(2) after 1; onSwap during stow delay clears pending stow, weapon stays drawn | file patch_manager_mods/holster_src/holster_logic_test.cpp fn testFsmStowDelayAndCancel / testFsmSwapAndRollover
  - G1 evidence: patch_manager_mods/holster_src/holster_logic_test.cpp:140-170; patch_manager_mods/holster_src/HolsterLogic.h:189 | G1 note: Test gap true in range. Anchor 157 tests combat re-entry, not onSwap. Out-of-combat swap reachability outside range.
  - v2 evidence: patch_manager_mods/holster_src/Holster.cpp:184 | v2 notes: [det PARTIAL] d009: No Fsm hit in shipped code; onSwap flag at Holster.cpp:115 and :184 is separate, reachability unresolved

## HN-T02 (P1): Does the holster hook at 0x005DF926 (source=ecx) read a valid `this` given the cmdq-probe hook at 0x005DF920 and the KPM trampoline, i.e. do the hook toml and manifest declare/guarantee a non-overlapping 6-byte span and ECX preservation?
- **HN-6h-02** v2=PARTIAL (G1 PARTIAL) | claimed medium hook/ABI | ECX may be clobbered by cmdq-probe detour at 0x005DF920 before holster hook at 0x005DF926 | file patch_manager_mods/holster_src/Holster.cpp fn HolsterSwap (hook 0x005DF926)
  - G1 evidence: patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml:46-49; patch_manager_mods/holster_src/manifest.toml:8-9 | G1 note: Ordering and span adjacency true; ECX guarantee comment-only; clobber needs cmdq wrapper (outside range).
  - v2 evidence: patch_manager_mods/cmdq_probe_src/CmdqProbe.cpp:196-200; patch_manager_mods/08_level_up_override.md:223-224 | v2 notes: [gem2 PARTIAL] u009: Displaced bytes ECX-free; cdecl callee clobbers ECX; invocation and wrapper restore unverified

## HN-T03 (P2): Can applyBoneTest reach codepatch::writeChecked (memcpy of 5 bytes at the site) without a prior codepatch::committed(at,5) check, such that an unmapped site faults instead of returning cleanly?
- **HN-6h-03** v2=PARTIAL (G1 PARTIAL) | claimed high memory-safety | writeChecked memcpy reads patch site before committed() check; may fault | file patch_manager_mods/holster_src/HolsterBonePatch.h fn hs::patchNodeLiteral / applyBoneTest
  - G1 evidence: patch_manager_mods/_shared_src/CodePatch.h:89-91; patch_manager_mods/holster_src/HolsterBonePatch.h:18-21; patch_manager_mods/holster_src/Holster.cpp:234,241 | G1 note: No committed() gate before writeChecked memcpy; fault needs unmapped site, gated only by k2addr (outside range).
  - v2 evidence: patch_manager_mods/_shared_src/K2Addr.h:588; patch_manager_mods/_shared_src/K2Addr.h:597-600 | v2 notes: [gem2 PARTIAL] u010: Unset/empty env selects Steam with no exe check (confirmed); memcpy fault path unread

## HN-T04 (P3): Does HolsterCombat log old=0 edge=1 when creature+off is unreadable because rdU32 returns 0 on saferead failure instead of a sentinel?
- **HN-6h-04** v2=CONFIRMED (G1 UNSURE) | claimed low logic | Unreadable creature logs false old=0 edge=1 combat transition | file patch_manager_mods/holster_src/Holster.cpp fn HolsterCombat
  - G1 evidence: patch_manager_mods/holster_src/Holster.cpp:161-163; patch_manager_mods/_shared_src/SafeRead.h:115-119 | G1 note: Depends on rdU32 failure value (outside ranges). Sentinel alone still gives edge=1 when new=0.
  - v2 evidence: patch_manager_mods/holster_src/Holster.cpp:128 | v2 notes: [det CONFIRMED] d010: rdU32 initialises v to 0 and returns it unchanged on readAt failure: no sentinel

## HN-T06 (P1): In engineSpawn, after a failed LoadTemplate, does the cleanup deref vtbl[0] with no probe and call the scalar deleting dtor on a p whose vtable/registration state is not verified?
- **HN-6n-01** v2=UNSURE (G1 PARTIAL) | claimed high memory-safety | Failed LoadTemplate cleanup uses unprobed vtbl[0] dtor; may leave dangling object-table entry | file patch_manager_mods/npc_world_src/NpcWorldProvider.cpp fn engineSpawn
  - G1 evidence: patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:159-160 | G1 note: Unprobed vtbl read and vtbl[0] dtor call are real. Object-table dangling part is UNSURE.
  - v2 evidence: 0x994F5C vtbl[0]=0x563490; 0x572AC0 JZ new-fail | v2 notes: [re UNSURE] re.md HN-T06: ctor stores vtbl 0x994F5C, dtor frees; no global registration seen; new() failure skips ctor

## HN-T07 (P2): In World::onCreatureAdded, when the area resref changes or areaPtr changes with the same resref and no onLoad fires, are seenCount_/seenOverflow_ left uncleared and the stable gate not re-armed?
- **HN-6n-02** v2=UNSURE (G1 PARTIAL) | claimed high logic | Area change without load event does not clear seen tags; spawns refused as TagPresent | file patch_manager_mods/npc_world_src/NwCore.h fn nw::World::onCreatureAdded
  - G1 evidence: patch_manager_mods/npc_world_src/NwCore.h:176-180; NwCore.h:161-162 | G1 note: Seen-tag non-clear confirmed; TagPresent refusal unverified (spawn() unread); fix alters per-visit dedupe scope
  - v2 evidence: patch_manager_mods/npc_world_src/NwCore.h:224; patch_manager_mods/npc_world_src/NwCore.h:236-241 | v2 notes: [gem2 UNSURE] u011: Spawn-side TagPresent check verified; area-change seen clear is out of range (OPEN)

## HN-T08 (P2): If the engine tag read via [c+0x24]+0x18 in NW_OnAddToArea is empty or differs from the tag passed to Spawn, can tagPresent(row.tag) return false after save/reload and allow a duplicate spawn?
- **HN-6n-03** v2=PARTIAL (G1 UNSURE) | claimed high logic | Unreadable or mismatched engine tag at AddToArea causes duplicate spawns after save/reload | file patch_manager_mods/npc_world_src/NpcWorldProvider.cpp fn NW_OnAddToArea
  - G1 evidence: patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:247; patch_manager_mods/npc_world_src/NwCore.h:224,231,236-240 | G1 note: In-range: engine tag never reaches seen_; reload seeding in onCreatureAdded unread
  - v2 evidence: patch_manager_mods/npc_world_src/NwCore.h:183; patch_manager_mods/npc_world_src/nw_world_test.cpp:122,126 | v2 notes: [gem2 PARTIAL] u012: Empty ci.tag skips seen_ seed (true); mismatch stored, not skipped; dup outcome OPEN. Fix unstated: no.

## HN-T11 (P3): Does nw_registry_test.cpp lack a case where one dispatch callback unregisters an entry and registers a new one, so a slot-based token change would go undetected?
- **HN-6n-07** v2=CONFIRMED (G1 PARTIAL) | claimed low test-gap | No registry test for unregister plus register inside one dispatch callback | file patch_manager_mods/npc_world_src/NwCore.h fn nw::Dispatcher::add
  - G1 evidence: patch_manager_mods/npc_world_src/NwCore.h:89-90; patch_manager_mods/npc_world_src/nw_registry_test.cpp:63-73 | G1 note: In-range tests never unregister and register in one callback. Tokens are monotonic, so no current defect. Add the test.
  - v2 evidence: patch_manager_mods/npc_world_src/nw_registry_test.cpp:22 | v2 notes: [det CONFIRMED] d014: cbRegisters only adds and cbKillsB only removes; no callback does both, so add-and-remove in one callback is untested
