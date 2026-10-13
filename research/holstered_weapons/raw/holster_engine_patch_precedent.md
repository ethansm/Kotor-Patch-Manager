# Holster / attach-node engine precedent (web recon, 2026-10-13)
Scope: public web only. Repo untouched. Goal: redirect weapon attach node (hand to hip) at runtime via native DLL/hook.

## Injection and hook precedent
- https://github.com/LaneDibello/Kotor-Patch-Manager : KPM runtime DLL injection (KotorPatcher.dll), binkw32.dll proxy under Wine/Proton, hook types simple/replace/detour/static, SQLite address DB. One summarized fetch; README and docs/ paths 404 on direct fetch.
- https://deadlystream.com/topic/11948-kotor-1-gog-reverse-engineering/ : "Lane" (Oct 2025) shares Ghidra project, SARIF, XML, .h for K1 swkotor.exe; ~99.9% funcs labeled, data types "VERY incomplete"; names CSWCObject and CSWCCreature; links the KPM repo.
- https://github.fr/lcdr/mod_loader : minimal dinput8 proxy DLL loader, per-mod folders; generic, not KOTOR-specific (search snippet only, not fetched).
- https://github.com/TsudaKageyu/minhook/wiki : MinHook x86/x64 hooking by function pointer (detours + trampolines), BSD; usable on in-process game code, no Linux cross-build docs found.
- https://packages.msys2.org/base/mingw-w64-MinHook : MSYS2 ships MinHook 1.3.3-2 for mingw32 (i686); upstream last release 1.3.3 (Jan 2017).

## Engine class and function names (reimplementations, not binary symbols)
- https://www.xoreos.org/doxygen/dc/dd5/classEngines_1_1KotOR_1_1Creature-members.html : xoreos K1 Creature has changeWeapon(EquipmentSlot), equipItem(tag, slot), getEquipedItem(slot), _equipment, _model, _headModel; no attach/hand/node names.
- https://www.xoreos.org/doxygen/dc/db7/classEngines_1_1KotOR2_1_1Creature.html : xoreos K2 Creature exposes no weapon/equip/attach members; only model/PartModels, loadBody/loadHead.
- https://synchro.codeberg.page/rakata/internals/mdl_deep_dive.html : rakata MDL notes: node type 0x0001 = Dummy/base; nodes resolved by name_index into name table; no hand/holster names; cites Ghidra addresses (e.g. FindModel 0x00464110).
- https://github.com/seedhartha/reone : reone (clean-room, non-commercial) runs KotOR/TSL; fetched layout shows include/, src/, test/ only, no creature-equip path visible.

## Negative results (no public hit)
- CSWCCreature, CSWSCreature, CSWCItem, GetNodeByName, rhand/lhand, holster/rhip/lhip: no source found with these names or a weapon-to-hand mapping.
- TSL (swkotor2.exe) Ghidra symbol share: none found; only the K1 share above.
- swkotor2 4GB / LAA patch thread on DeadlyStream: none found.

## Build and test tooling
- https://www.mikrocontroller.net/topic/431877 : forum reply names the 32-bit cross toolchain i686-w64-mingw32, installable via apt gcc-mingw-w64-i686 (unverified on this box).
- https://docs.rs/crate/patchlaa/0.2.0 : CLI that sets the Large Address Aware flag on PE32 exes (only relevant if a memory-limit question arises).
- https://deadlystream.com/files/file/2734-fog-fix-more-c3-fd-patcher : existing swkotor exe-side patcher (fog and reflections, K1 and K2); precedent for shipped binary patching.
- https://list.winehq.org/pipermail/wine-bugs/2018-June/493184.html : Wine bug 45109, winedbg debugging of 32-bit processes; relevant to testing a hook under Wine.

## Gaps
- No public source names the function that attaches a weapon to rhand/lhand, or any hip/holster node.
- KPM docs (Roadmap2026, NATIVE_LINUX) and README were not read directly (404 on fetch).
- "Lane" = LaneDibello is inferred from the KPM link in the thread, not confirmed.
