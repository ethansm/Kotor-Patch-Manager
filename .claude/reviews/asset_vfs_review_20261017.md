# asset-vfs 0.1.0-dev review (2026-10-17, doc 38 session)

Scope: `patch_manager_mods/asset_vfs_src/*` (Gemini/Antigravity build), `_shared_src/{VfsRegistry,TwoDaSplicer,CharacterProvider}.h`, `k1_port/k1_port_vfs_pack.py`. Uncommitted at review time.

## Verified OK (static, against Steam exe 6A522E71 and ghidra_dumps/steam_6a522e71)
- All 4 hook `original_bytes` match the exe: 0x0071244E, 0x00713E83, 0x00729FD0, 0x0070CB76.
- Exits: 0x00712462 and 0x00713FA3 are real epilogues (`mov esp,ebp; pop ebp; ret 0x10` / `ret 8`); 0x0070D939 is a native label (JNZ from 0x0070D6B3) with the same EH state (ebp-4 = 0) as the hook site.
- Demand 0x711C20 dispatches `packedId >> 30 == 1` to ServiceFromImage 0x713E80, so prefix 0x40FC0000 routes correctly.
- FreeResourceData 0x7120C0 skips the free when `packedId >> 30 == 1`, so HeapAlloc buffers are safe.
- CRes layout: +0x08 packedId, +0x0C flags, +0x10 data, +0x14 key-entry back-pointer (SetResObject 0x712B60), +0x18 size, +0x1C set to 1 by SetResObject. Key entry: +0x10 pRes, +0x14 packedId, +0x1A resType. Table +0x1C read by Exists 0x711ED0 and GetResObject 0x712970 (4 suppresses the debug-string path).
- GetKeyEntry 0x712360: 5 table walks, the miss tail writes *out1 = *out2 = 0 at 0x71244E. VFS is lowest priority (vanilla and Override shadow it).
- CRes2DA::OnResourceServiced 0x729FD0 is fastcall(ecx = CRes2DA), parsed flag at +0x28, data +0x10, size +0x18, header `2DA V2.b`. Hooking the entry before parse is right.
- No overlap with perf-lab (0x71222D, 0x7120C0, 0x712F30, 0x71306C, 0x7130B0), robustness 0x712F55, or k2-mp3-playback-fix (0x70CDCB, 0x70CF34).
- Host suites re-run: vfs_registry, twoda_splicer, character_provider, asset_vfs_core: 4/4 PASS.

## Findings (most important first)
1. **No logging / no observer mode.** Only 2 KP_LOG calls (init, Miles callbacks). Doc 41 §9 item 5 requires an observe-only mode (passthrough + log name, restype, served/passed) before serving. With 0 files the inotify watcher is blind, so a VFS-T1 failure is undiagnosable. Channels chRes/ch2Da/chChar are declared but unused.
2. **No consumer.** Nothing calls RegisterAssets / Register2DaCells / RegisterCharacter; no Juhani bundle mod or generated `juhani_vfs_bundle.h` exists. VFS-T1 cannot run. Summary's "ready for live staging" overstated.
3. **OpenStream hook covers one branch only.** 0x70CB76 sits in the `[0xA10768] != 0` branch; the else branch (0x70D668 -> 0x70D6B3 -> 0x70D939) opens without passing the hook. 0xA10768 is not in the Ghidra DB.
4. **Audio bypass of k2-mp3-playback-fix.** Matched streams jump past the kind-8 region where that fix hooks. Container slicing (`inspectAudioContainer`) must fully replace it; untested. `_AIL_set_file_callbacks` is process-global (replaces any existing callbacks; fallback CreateFileA). MilesCursorTable has no lock (Miles stream thread vs main thread).
5. **ServiceFromImage hides failures.** Provider returns 1 always; native returns the OnResourceServiced vtbl[4] result.
6. **Doc 44 stale.** Still lists Demand/Exists/0x70CAA0 detours and Demand==DemandResMan confusion; real build uses 4 mid-function hooks; no roadmap row for the provider mod.
7. **Packer refuses .tga.** CR textures are TGA. TPC conversion must force alpha 255 on head textures and emit `clamp 1` TXI (doc 38 §24); pykotor DXT encode is lossy (memory kotor2-pykotor-dxt-bugs).
8. **Hard `requires = ["npc-world"]`** only for Spawn. Soft dependency would decouple. Minor.
9. Minor: 2DA hook runs FreeData+Malloc on every unparsed 2DA even when the splicer has no cells for it? (buildSpliced returns 0 -> early return, OK). Mid-function hook sites are not in the address DB; verified via asm dumps only.
