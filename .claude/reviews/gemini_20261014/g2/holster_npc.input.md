# G2 resolver input: batch holster_npc (10 open fids, 10 topics)

## HN-T01 (P2) — G1 unit u023, facts file recon/HN-T01.md
Question: In hs::Fsm, does onSwap (clears stowPending without checking lastCombat==0) leave the weapon permanently PhDrawn after an out-of-combat swap during the stow delay, and does tick restart PhStowing at fraction 1.0 when a draw is still in progress?
G1 windows (do NOT reuse): patch_manager_mods/holster_src/HolsterLogic.h:150-200;patch_manager_mods/holster_src/holster_logic_test.cpp:126-182
G1 OPEN: Can swap action 0x0E fire out of combat, and does any caller reset Fsm on pause, load or area change?
- HN-6h-05 [PARTIAL] No tests for onCombat(2) after 1; onSwap during stow delay clears pending stow, weapon stays drawn | file patch_manager_mods/holster_src/holster_logic_test.cpp | fn testFsmStowDelayAndCancel / testFsmSwapAndRollover | cat test-gap | G1 evidence patch_manager_mods/holster_src/holster_logic_test.cpp:140-170; patch_manager_mods/holster_src/HolsterLogic.h:189 | G1 note: Test gap true in range. Anchor 157 tests combat re-entry, not onSwap. Out-of-combat swap reachability outside range. | anchor now patch_manager_mods/holster_src/holster_logic_test.cpp:157 drift none

## HN-T02 (P1) — G1 unit u003, facts file recon/HN-T02.md
Question: Does the holster hook at 0x005DF926 (source=ecx) read a valid `this` given the cmdq-probe hook at 0x005DF920 and the KPM trampoline, i.e. do the hook toml and manifest declare/guarantee a non-overlapping 6-byte span and ECX preservation?
G1 windows (do NOT reuse): patch_manager_mods/holster_src/Holster.cpp:150-200;patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml;patch_manager_mods/holster_src/manifest.toml
G1 OPEN: Does the cmdq-probe wrapper preserve ECX across CmdqProbeReequip, and are its stolen bytes at 0x005DF920-0x005DF925 ECX-free? (cmdq-probe files, outside range)
- HN-6h-02 [PARTIAL] ECX may be clobbered by cmdq-probe detour at 0x005DF920 before holster hook at 0x005DF926 | file patch_manager_mods/holster_src/Holster.cpp | fn HolsterSwap (hook 0x005DF926) | cat hook/ABI | G1 evidence patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml:46-49; patch_manager_mods/holster_src/manifest.toml:8-9 | G1 note: Ordering and span adjacency true; ECX guarantee comment-only; clobber needs cmdq wrapper (outside range). | anchor now - drift noanchor

## HN-T03 (P2) — G1 unit u024, facts file recon/HN-T03.md
Question: Can applyBoneTest reach codepatch::writeChecked (memcpy of 5 bytes at the site) without a prior codepatch::committed(at,5) check, such that an unmapped site faults instead of returning cleanly?
G1 windows (do NOT reuse): patch_manager_mods/holster_src/HolsterBonePatch.h;patch_manager_mods/holster_src/Holster.cpp:225-262;patch_manager_mods/_shared_src/CodePatch.h:60-111
G1 OPEN: Does k2addr::get() guarantee 0x763AF9 and 0x763E2B are mapped committed code in the running exe?
- HN-6h-03 [PARTIAL] writeChecked memcpy reads patch site before committed() check; may fault | file patch_manager_mods/holster_src/HolsterBonePatch.h | fn hs::patchNodeLiteral / applyBoneTest | cat memory-safety | G1 evidence patch_manager_mods/_shared_src/CodePatch.h:89-91; patch_manager_mods/holster_src/HolsterBonePatch.h:18-21; patch_manager_mods/holster_src/Holster.cpp:234,241 | G1 note: No committed() gate before writeChecked memcpy; fault needs unmapped site, gated only by k2addr (outside range). | anchor now - drift noanchor

## HN-T04 (P3) — G1 unit u061, facts file recon/HN-T04.md
Question: Does HolsterCombat log old=0 edge=1 when creature+off is unreadable because rdU32 returns 0 on saferead failure instead of a sentinel?
G1 windows (do NOT reuse): patch_manager_mods/holster_src/Holster.cpp:150-182;patch_manager_mods/_shared_src/SafeRead.h:90-120
G1 OPEN: Does rdU32 zero-initialise its result when readAt fails? Its definition is outside both listed ranges.
- HN-6h-04 [UNSURE] Unreadable creature logs false old=0 edge=1 combat transition | file patch_manager_mods/holster_src/Holster.cpp | fn HolsterCombat | cat logic | G1 evidence patch_manager_mods/holster_src/Holster.cpp:161-163; patch_manager_mods/_shared_src/SafeRead.h:115-119 | G1 note: Depends on rdU32 failure value (outside ranges). Sentinel alone still gives edge=1 when new=0. | anchor now patch_manager_mods/holster_src/Holster.cpp:161 drift none

## HN-T06 (P1) — G1 unit u004, facts file recon/HN-T06.md
Question: In engineSpawn, after a failed LoadTemplate, does the cleanup deref vtbl[0] with no probe and call the scalar deleting dtor on a p whose vtable/registration state is not verified?
G1 windows (do NOT reuse): patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:120-175
G1 OPEN: Does kAddrCreatureCtor add an object-table entry that the flag-1 DtorFn removes? Outside lines 120-175.
- HN-6n-01 [PARTIAL] Failed LoadTemplate cleanup uses unprobed vtbl[0] dtor; may leave dangling object-table entry | file patch_manager_mods/npc_world_src/NpcWorldProvider.cpp | fn engineSpawn | cat memory-safety | G1 evidence patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:159-160 | G1 note: Unprobed vtbl read and vtbl[0] dtor call are real. Object-table dangling part is UNSURE. | anchor now patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:159 drift none

## HN-T07 (P2) — G1 unit u025, facts file recon/HN-T07.md
Question: In World::onCreatureAdded, when the area resref changes or areaPtr changes with the same resref and no onLoad fires, are seenCount_/seenOverflow_ left uncleared and the stable gate not re-armed?
G1 windows (do NOT reuse): patch_manager_mods/npc_world_src/NwCore.h:150-200;patch_manager_mods/npc_world_src/NwCore.h:255-290;patch_manager_mods/npc_world_src/nw_world_test.cpp:150-190
G1 OPEN: Does spawn() refuse via seen_ (TagPresent) after a resref change without onLoad? spawn() is outside the read ranges.
- HN-6n-02 [PARTIAL] Area change without load event does not clear seen tags; spawns refused as TagPresent | file patch_manager_mods/npc_world_src/NwCore.h | fn nw::World::onCreatureAdded | cat logic | G1 evidence patch_manager_mods/npc_world_src/NwCore.h:176-180; NwCore.h:161-162 | G1 note: Seen-tag non-clear confirmed; TagPresent refusal unverified (spawn() unread); fix alters per-visit dedupe scope | anchor now patch_manager_mods/npc_world_src/NwCore.h:176 drift none

## HN-T08 (P2) — G1 unit u026, facts file recon/HN-T08.md
Question: If the engine tag read via [c+0x24]+0x18 in NW_OnAddToArea is empty or differs from the tag passed to Spawn, can tagPresent(row.tag) return false after save/reload and allow a duplicate spawn?
G1 windows (do NOT reuse): patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:222-262;patch_manager_mods/npc_world_src/NwCore.h:215-240;patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:320-340
G1 OPEN: Does onCreatureAdded (outside ranges) seed seen_ from ci.tag on reload? If so, empty engine tag can duplicate spawns.
- HN-6n-03 [UNSURE] Unreadable or mismatched engine tag at AddToArea causes duplicate spawns after save/reload | file patch_manager_mods/npc_world_src/NpcWorldProvider.cpp | fn NW_OnAddToArea | cat logic | G1 evidence patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:247; patch_manager_mods/npc_world_src/NwCore.h:224,231,236-240 | G1 note: In-range: engine tag never reaches seen_; reload seeding in onCreatureAdded unread | anchor now patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:247 drift none

## HN-T09 (P2) — G1 unit u027, facts file recon/HN-T09.md
Question: Does fmtRef16's hex branch write 52 bytes regardless of n, and does any caller pass a buffer smaller than 52 bytes?
G1 windows (do NOT reuse): patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:150-175;patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:295-312
G1 OPEN: Does the hex branch total exactly 52 bytes (line 176+), and do any fmtRef16 callers outside lines 295-312 pass under 52 bytes?
- HN-6n-05 [PARTIAL] Hex branch ignores n and writes 52 bytes unconditionally | file patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp | fn fmtRef16 | cat memory-safety | G1 evidence patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:171-175 | G1 note: Hex branch never uses n; anchor line 171 matches. Latent: callers pass 64B. | anchor now patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:171 drift none

## HN-T10 (P3) — G1 unit u062, facts file recon/HN-T10.md
Question: Is dumpEvent's budget/line accounting (fits(kEventLines), skipped-unreadable one-liners, CAP marker) outside ClientCore.h and unexercised by k1_client_core_test.cpp?
G1 windows (do NOT reuse): patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:195-256;patch_manager_mods/k1_spawn_probe_src/ClientCore.h;patch_manager_mods/k1_spawn_probe_src/k1_client_core_test.cpp
G1 OPEN: Does logf route every line through the K1SpawnProbe.cpp:70-74 gate, so the skipped one-liners are charged?
- HN-6n-06 [PARTIAL] dumpEvent budget accounting untested because logic lives outside ClientCore.h | file patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp | fn dumpEvent / dumpBlock | cat test-gap | G1 evidence patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:217; patch_manager_mods/k1_spawn_probe_src/ClientCore.h:38; patch_manager_mods/k1_spawn_probe_src/k1_client_core_test.cpp:154-172 | G1 note: Primitives unit-tested; dumpEvent composition, unreadable one-liners and CAP wiring untested. Extract pure step or add seam. | anchor now patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:217 drift none

## HN-T11 (P3) — G1 unit u063, facts file recon/HN-T11.md
Question: Does nw_registry_test.cpp lack a case where one dispatch callback unregisters an entry and registers a new one, so a slot-based token change would go undetected?
G1 windows (do NOT reuse): patch_manager_mods/npc_world_src/NwCore.h:70-125;patch_manager_mods/npc_world_src/nw_registry_test.cpp:55-100
G1 OPEN: Does any test outside the read ranges (nw_registry_test.cpp lines 1-54 or 101+, or the cbRegisters/cbKillsB bodies) unregister and register in one callback?
- HN-6n-07 [PARTIAL] No registry test for unregister plus register inside one dispatch callback | file patch_manager_mods/npc_world_src/NwCore.h | fn nw::Dispatcher::add | cat test-gap | G1 evidence patch_manager_mods/npc_world_src/NwCore.h:89-90; patch_manager_mods/npc_world_src/nw_registry_test.cpp:63-73 | G1 note: In-range tests never unregister and register in one callback. Tokens are monotonic, so no current defect. Add the test. | anchor now patch_manager_mods/npc_world_src/NwCore.h:90 drift none
