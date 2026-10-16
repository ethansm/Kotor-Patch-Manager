REVIEW npc-world + k1-spawn-probe | files read: 26 | findings: 7

### R2-NW-01 - Area change without AREA_LOAD does not clear seen tags or overflow state
- Class: reachable
- Severity: medium
- Confidence: 95
- Location: patch_manager_mods/npc_world_src/NwCore.h:205-209 :: nw::World::onCreatureAdded
- Anchor: `if (!resrefEquals(area_, ci.areaRes)) {   // first add of the load, or an area change without a load event: re-arm the gate`
- Trigger: `NW_OnAddToArea` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:235`) -> `g_world.onCreatureAdded(ci)` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:264`) -> `nw::World::onCreatureAdded` (`patch_manager_mods/npc_world_src/NwCore.h:205-209`) when a creature is added with a new `ci.areaRes` that differs from a non-empty `area_` without a preceding `NW_OnLoad` (such as the `0x531BE0 -> 0x556670 -> 0x5582F0` module copy path noted in `40_npc_behaviour_variety.md:96`), followed by 30 quiet ticks in `NW_OnTick` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:270`) -> `nw::World::onTick` (`patch_manager_mods/npc_world_src/NwCore.h:235`) -> consumer `K2NW_AREA_STABLE` callback -> `apiSpawn` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:197`) -> `nw::World::spawn` (`patch_manager_mods/npc_world_src/NwCore.h:253`) -> `nw::World::tagPresent` (`patch_manager_mods/npc_world_src/NwCore.h:265`).
- Impact: On an area transition that does not fire `NW_OnLoad`, `onCreatureAdded` resets the stability gate (`gateReset()`) and `spawnedThisVisit_ = 0`, but leaves `seenCount_` and `seenOverflow_` untouched. Every creature tag seen or spawned in the previous area continues to report `tagPresent(tag) == true` in the new area, causing `spawn()` to refuse any creature sharing a tag with the previous area (`SpawnResult::TagPresent`). Across multiple loadless area changes, `seenCount_` accumulates tags until `MaxSeenTags` (256) is reached and latches `seenOverflow_ = true`, blocking all tagged spawns until a full `NW_OnLoad` occurs.
- Fix: When `area_[0] != '\0'` inside the `!resrefEquals(area_, ci.areaRes)` branch of `onCreatureAdded`, reset `seenCount_ = 0` and `seenOverflow_ = false` before recording the new creature's tag, and assert `!w.tagPresent("a")` and `!w.tagPresent("t1")` after the area change in `nw_world_test.cpp:170-172`.
- Diff:
```diff
--- a/patch_manager_mods/npc_world_src/NwCore.h
+++ b/patch_manager_mods/npc_world_src/NwCore.h
@@ -203,6 +203,10 @@ public:
         if (ci.areaPtr && ci.areaRes && ci.areaRes[0]) {
             areaPtr_ = ci.areaPtr;
             if (!resrefEquals(area_, ci.areaRes)) {   // first add of the load, or an area change without a load event: re-arm the gate
+                if (area_[0]) {
+                    seenCount_ = 0;
+                    seenOverflow_ = false;
+                }
                 setArea(ci.areaRes);
                 gateReset();
                 spawnedThisVisit_ = 0;
```

### R2-NW-02 - readStr turns non-printable bytes into '.', suppressing TAGSCAN and polluting dumpTag
- Class: reachable
- Severity: low
- Confidence: 90
- Location: patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:334-336 :: onCreatureAdded
- Anchor: `if (!npcHit && g_gate.wantNpc(true) && g_scan.allow(libTag[0] == 0, tagStr[0] == 0 && tag18[0] == 0, g_budget.capped())) {`
- Trigger: `NW_OnAddToArea` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:235`) -> `g_world.onCreatureAdded` (`patch_manager_mods/npc_world_src/NwCore.h:219`) -> `Dispatcher::dispatch` (`patch_manager_mods/npc_world_src/NwCore.h:115`) -> `onEvent` (`patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:361`) -> `onCreatureAdded` (`patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:291,297,334-336`) -> `readStr` (`patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:183-196`) when `[c+0x24]+0x18` (`tagStr`) or `c+0x18` (`tag18`) points to readable non-printable binary data starting with a non-NUL byte.
- Impact: Unlike `nw::readCreatureTag` (`NwCore.h:146,150`), which counts non-printable bytes and rejects candidate strings when `bad != 0`, `K1SpawnProbe.cpp`'s `readStr` replaces non-printable bytes with `'.'` and leaves `tagStr` / `tag18` non-empty (e.g. `"..."`). When `c+0x18` or `[c+0x24]+0x18` points to binary data on a creature: (1) `tagStr[0] == 0 && tag18[0] == 0` is `false`, so `g_scan.allow(...)` refuses to run the fallback `scanForTag` (`TAGSCAN`) in the exact case `TAGSCAN` was designed for; and (2) `dumpTag` at line 334 prioritizes `tagStr` (`[c+0x24]+0x18`) ahead of `tag18` (`c+0x18`) and `libTag`, so non-printable garbage at `[c+0x24]+0x18` is logged as `tag="..."` in `DUMP-HDR` even when `tag18` / `libTag` matched a valid harvest tag.
- Fix: Have `readStr` reject strings containing non-printable bytes (or track a `bad` counter like `readCStr` in `NpcWorldProvider.cpp:93-108`), and prefer `libTag` / `tag18` over the fallback `tagStr` when choosing `dumpTag`.
- Diff:
```diff
--- a/patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp
+++ b/patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp
@@ -189,7 +189,7 @@ bool readStr(unsigned site, uintptr_t p, char* out, int cap) {
         if (!saferead::probeRead(site, reinterpret_cast<const void*>(p + n), 1)) break;
         unsigned char ch = *reinterpret_cast<const unsigned char*>(p + n);
         if (ch == 0) break;
-        out[n++] = (ch >= 0x20 && ch < 0x7f) ? static_cast<char>(ch) : '.';
+        if (ch < 0x20 || ch >= 0x7f) { out[0] = 0; return false; }
+        out[n++] = static_cast<char>(ch);
     }
     out[n] = 0;
     return true;
@@ -331,7 +331,7 @@ void onCreatureAdded(const K2NwEvent* ev) {
 
     // Dump triggers. The NPC trigger is a harvest tag in either tag path. While the NPC dump is still wanted, the fallback scan runs
     // only when both tag strings came back empty (ScanBudget). DUMP-ANY fires once per distinct id, for the first three ids.
-    const char* dumpTag = tagStr[0] ? tagStr : (tag18[0] ? tag18 : libTag);
+    const char* dumpTag = libTag[0] ? libTag : (tag18[0] ? tag18 : tagStr);
     bool npcHit = k1spawn::IsHarvestTag(tagStr) || k1spawn::IsHarvestTag(tag18) || k1spawn::IsHarvestTag(libTag);
     if (!npcHit && g_gate.wantNpc(true) && g_scan.allow(libTag[0] == 0, tagStr[0] == 0 && tag18[0] == 0, g_budget.capped())) {
```

### R2-NW-03 - engineSpawn failed-LoadTemplate cleanup reads vtable and slot 0 without saferead (still present from 2026-10-14 review)
- Class: latent
- Severity: medium
- Confidence: 95
- Location: patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:167-168 :: engineSpawn
- Anchor: `void** vtbl = *reinterpret_cast<void***>(p);`
- Trigger: `NW_OnTick` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:270`) -> `nw::World::onTick` (`patch_manager_mods/npc_world_src/NwCore.h:235`) -> `onEvent` (`patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:362`) -> `onStable` (`patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:354`) -> `k1spawn::RunStable` (`patch_manager_mods/k1_spawn_probe_src/ClientCore.h:180`) -> `apiSpawn` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:200`) -> `nw::World::spawn` (`patch_manager_mods/npc_world_src/NwCore.h:256`) -> `engineSpawn` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:164-168`) when `kAddrLoadTemplate` returns 0 (for example, `k1p_juhani` in `ClientLogic.h:23` when `k1p_juhani.utc` is not installed in `Override/`) and `kAddrCreatureCtor` or `kAddrLoadTemplate` leaves `[p]` or `vtbl[0]` unreadable.
- Impact: Reported in the 2026-10-14 review and still unfixed in current code (`NpcWorldProvider.cpp:167-168`). With `k1p_juhani` added to `kRoster` in `30984d7` while `k1p_juhani.utc` remains staged-only (`38_k1_to_k2_asset_import.md:248`, `43_asset_import_and_npc_programme.md:27,83`), entering `204tel` takes the `!loaded` cleanup branch on every visit; lines 167-168 dereference `*reinterpret_cast<void***>(p)` and `vtbl[0]` without `saferead::readAt`, violating the file's invariant (`NpcWorldProvider.cpp:10`) and faulting if the vtable pointer is null or unreadable.
- Fix: Probe `[p]` and `vtbl[0]` with `saferead::readAt` before invoking the scalar deleting destructor.
- Diff:
```diff
--- a/patch_manager_mods/npc_world_src/NpcWorldProvider.cpp
+++ b/patch_manager_mods/npc_world_src/NpcWorldProvider.cpp
@@ -164,8 +164,9 @@ uint32_t engineSpawn(void*, void* area, const char* resref, float x, float y, fl
     const int loaded = reinterpret_cast<LoadFn>(kAddrLoadTemplate)(p, cres, 0) & 0xFF;
     if (!loaded) {
         KP_LOGV(chSpawn, KPLOG_WARN, "fail: load template %s; destroying", resref);
-        void** vtbl = *reinterpret_cast<void***>(p);
-        reinterpret_cast<DtorFn>(vtbl[0])(p, 1);
+        uintptr_t vtbl = 0, dtor = 0;
+        if (saferead::readAt(S_Creature, reinterpret_cast<uintptr_t>(p), 0, &vtbl) && saferead::readAt(S_Creature, vtbl, 0, &dtor) && dtor)
+            reinterpret_cast<DtorFn>(dtor)(p, 1);
         return 0;
     }
     const nw::Vec3 o = nw::orientationFromFacing(facing);
```

### R2-NW-04 - Client callback lifetime on dynamic DLL unload and unguarded re-init / post-connect add()
- Class: latent
- Severity: medium
- Confidence: 90
- Location: patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:377-392 :: KPatchInit / DllMain
- Anchor: `if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hinst);`
- Trigger: (1) Dynamic unload (`FreeLibrary`) of `k1-spawn-probe.dll` triggers `DllMain` (`patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:389-392`) with `reason == DLL_PROCESS_DETACH` and `lpvReserved == nullptr`, followed by `NW_OnTick` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:274`) -> `nw::Dispatcher::dispatch` (`patch_manager_mods/npc_world_src/NwCore.h:115`). (2) Calling `KPatchInit` (`patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:377-385`) more than once, or calling `k2nw::Client::add` (`patch_manager_mods/_shared_src/NpcWorld.h:128-132`) after `connect()`.
- Impact: (1) `DllMain` never calls `g_nw.release()` on dynamic `DLL_PROCESS_DETACH` (`lpvReserved == nullptr`), leaving `onEvent` and `"k1-spawn-probe"` registered in `npc-world`'s `g_world.disp.e[]` pointing into unmapped memory; the next dispatched event jumps to an unmapped address and crashes. (2) Unlike `NpcWorldProvider.cpp:128` (`ensureInit`), `K1SpawnProbe.cpp`'s `KPatchInit` has no `g_inited` guard, and `k2nw::Client::add` (`NpcWorld.h:128-132`) accepts calls when `connected == true` by appending an entry with `token = 0` and returning `true` without calling `nw->Register`.
- Fix: Guard `KPatchInit` in `K1SpawnProbe.cpp` against repeat initialization, call `g_nw.release()` in `DllMain` when `reason == DLL_PROCESS_DETACH && !reserved`, and have `k2nw::Client::add` refuse or immediately register when `connected` is already true.
- Diff:
```diff
--- a/patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp
+++ b/patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp
@@ -42,6 +42,7 @@ enum : unsigned {
 // ---- state (all callbacks run on the engine's main thread) ------------------------------------------------------
 k2nw::Client g_nw;
 bool g_enabled = true;       // [spawn] enabled in k1_spawn_probe.ini
+bool g_inited = false;
 long g_seq = 0;              // ADDTOAREA events since attach (counts events after the cap too)
 long g_loadSeq = 0;
 char g_armedArea[17] = "";   // area resref "SPAWN armed" was last logged for (reset by AREA_LOAD)
@@ -375,6 +376,8 @@ extern "C" {
 
 __declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
+    if (g_inited) return;
+    g_inited = true;
     g_enabled = GetPrivateProfileIntA("spawn", "enabled", 1, kIniName) != 0;
     logf("k1-spawn-probe %s loaded (client of npc-world, spawn %s)", kVersion, g_enabled ? "enabled" : "disabled by ini");
     g_nw.init(api, "k1-spawn-probe");
@@ -386,7 +389,8 @@ __declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
 
 }  // extern "C"
 
-BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
+BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
     if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hinst);
+    else if (reason == DLL_PROCESS_DETACH && !reserved) g_nw.release();
     return TRUE;
 }
```

### R2-NW-05 - readCreatureTag and DllMem::u32 pre-add offsets to base, bypassing saferead::readAt null-base and wraparound guards
- Class: latent
- Severity: low
- Confidence: 92
- Location: patch_manager_mods/npc_world_src/NwCore.h:142-153 :: nw::readCreatureTag
- Anchor: `if (m.u32(c + 0x18, &p) && p && m.cstr(p, out, TagChars + 1, &bad) && bad == 0 && out[0]) return TagVia18;`
- Trigger: `NW_OnAddToArea` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:235`) -> `nw::readCreatureTag` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:254`) -> `nw::readCreatureTag` (`patch_manager_mods/npc_world_src/NwCore.h:146,150`) -> `DllMem::u32` (`patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:112`) -> `saferead::readAt` (`patch_manager_mods/_shared_src/SafeRead.h:115-121`) when `creature` (`c`) is `0`.
- Impact: Before `22a3d7a`, `NW_OnAddToArea` called `saferead::readAt(S_TagPtr, c, 0x24, &tagPtr)`, where `SafeRead.h:118` rejected `base == 0` and `base + off < base` before calling `probeRead`. `22a3d7a` moved the `+ 0x18` and `+ 0x24` offset additions into `nw::readCreatureTag` (`c + 0x18`, `c + 0x24`, `holder + 0x18`) without checking `if (!c) return TagNone;`, and `DllMem::u32` passes `off = 0` (`saferead::readAt(S_TagPtr, addr, 0, v)`). When `c == 0`, `readCreatureTag` no longer short-circuits on a null creature pointer: in the DLL it invokes `probeRead` at `0x18` and `0x24` (recording two failed probes in `saferead::g_ring` when enabled), and in host `MemReader` tests it reads addresses `0x18` and `0x24`.
- Fix: Return `TagNone` immediately in `nw::readCreatureTag` when `c == 0` (or pass `(base, off)` to `MemReader::u32` so `saferead::readAt` receives the base and offset separately).
- Diff:
```diff
--- a/patch_manager_mods/npc_world_src/NwCore.h
+++ b/patch_manager_mods/npc_world_src/NwCore.h
@@ -141,6 +141,7 @@ enum TagVia { TagNone = 0, TagVia18 = 1, TagVia24 = 2 };
 // out must hold TagChars+1 bytes. Returns which path produced the tag; out = "" when none did.
 inline TagVia readCreatureTag(MemReader& m, uintptr_t c, char* out) {
     out[0] = 0;
+    if (!c) return TagNone;
     unsigned p = 0;
     int bad = 0;
     if (m.u32(c + 0x18, &p) && p && m.cstr(p, out, TagChars + 1, &bad) && bad == 0 && out[0]) return TagVia18;
```

### R2-NW-06 - nw_tag_test.cpp does not test non-printable fallback rejection or clearing out after a rejected primary read
- Class: test-gap
- Severity: low
- Confidence: 98
- Location: patch_manager_mods/npc_world_src/nw_tag_test.cpp:48-55 :: main
- Anchor: `{ FakeMem m; m.dw[C + 0x18] = 0x10; m.str[0x10] = std::string("\x01\x02\xff", 3); m.dw[C + 0x24] = 0x1000; m.dw[0x1018] = 0x2000; m.str[0x2000] = "B";`
- Trigger: Host test execution of `main` (`patch_manager_mods/npc_world_src/nw_tag_test.cpp:36-56`) calling `tagOf` (`patch_manager_mods/npc_world_src/nw_tag_test.cpp:29-34`) -> `nw::readCreatureTag` (`patch_manager_mods/npc_world_src/NwCore.h:142-153`).
- Impact: Case 4 (`nw_tag_test.cpp:49`) tests non-printable bytes at `c + 0x18` only when the `[c + 0x24] + 0x18` fallback succeeds and overwrites `out` with `"B"`, while case 5 (`nw_tag_test.cpp:52-55`) tests failure cases only when `m.cstr` writes nothing (`""`, `0`, `0x99`, or empty `FakeMem`). If `out[0] = 0;` at `NwCore.h:147` and `NwCore.h:151` were both removed (leaving garbage in `out` when `readCreatureTag` returns `TagNone`), or if `&& bad == 0 && out[0]` were removed from the fallback check at `NwCore.h:150` (accepting non-printable or empty strings via `TagVia24`), `nw_tag_test.cpp` would still pass.
- Fix: Add assertions in `nw_tag_test.cpp` where `c + 0x18` points to non-printable bytes with no fallback (verifying `tagOf` returns `""` and `TagNone`), and where `[c + 0x24] + 0x18` points to non-printable bytes or `""` (verifying `tagOf` returns `""` and `TagNone`).
- Diff:
```diff
--- a/patch_manager_mods/npc_world_src/nw_tag_test.cpp
+++ b/patch_manager_mods/npc_world_src/nw_tag_test.cpp
@@ -50,6 +50,9 @@ int main() {
       KT_CHECK(tagOf(m, C, &via) == "B"); KT_CHECK_EQ(int(via), int(nw::TagVia24)); }
     // 5. empty string, null pointer, unreadable pointer, nothing at all -> "" and TagNone
+    { FakeMem m; m.dw[C + 0x18] = 0x10; m.str[0x10] = std::string("\x01\x02", 2); KT_CHECK(tagOf(m, C, &via) == ""); KT_CHECK_EQ(int(via), int(nw::TagNone)); }
+    { FakeMem m; m.dw[C + 0x24] = 0x1000; m.dw[0x1018] = 0x2000; m.str[0x2000] = std::string("\x01\x02", 2); KT_CHECK(tagOf(m, C, &via) == ""); KT_CHECK_EQ(int(via), int(nw::TagNone)); }
+    { FakeMem m; m.dw[C + 0x24] = 0x1000; m.dw[0x1018] = 0x2000; m.str[0x2000] = ""; KT_CHECK(tagOf(m, C, &via) == ""); KT_CHECK_EQ(int(via), int(nw::TagNone)); }
     { FakeMem m; m.dw[C + 0x18] = 0x10; m.str[0x10] = ""; KT_CHECK(tagOf(m, C, &via) == ""); KT_CHECK_EQ(int(via), int(nw::TagNone)); }
```

### R2-NW-07 - common_contract_test.cpp tests the 0.1.0 ADD log format without via= and omits the c+0x18 tag read
- Class: test-gap
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/npc_world_src/common_contract_test.cpp:93-94 :: main
- Anchor: `KP_LOG(chAdd, "ADD c=%08lx id=%08x tag=\"%s\" area=%s pos=%.1f,%.1f,%.1f origin=%s", 0xFFFFFFFFul, 0xFFFFFFFFu,`
- Trigger: Host test execution of `main` (`patch_manager_mods/npc_world_src/common_contract_test.cpp:45-65,92-100`).
- Impact: Commit `22a3d7a` updated `NW_OnAddToArea` (`NpcWorldProvider.cpp:254,261-262`) to read the primary creature tag at `c + 0x18` and added `via=%s` (up to 9 extra bytes for `via=none `) to the `ADD` log format string. `common_contract_test.cpp` was not updated: lines 93-94 still test the shorter 0.1.0 `ADD` format string without `via=%s` against the 232-byte `KpLog` truncation threshold, and lines 45-65 only test the fallback `[c+0x24] -> +0x18` tag chain rather than the primary `c + 0x18` read.
- Fix: Update `common_contract_test.cpp:93-94` to include `via=%s` (`"none"`) matching `NpcWorldProvider.cpp:261-262`, and add a `readAt` check for `c + 0x18` in section 1 (`common_contract_test.cpp:45-65`).
- Diff:
```diff
--- a/patch_manager_mods/npc_world_src/common_contract_test.cpp
+++ b/patch_manager_mods/npc_world_src/common_contract_test.cpp
@@ -90,8 +90,8 @@ int main() {
         HEQ("KpLog_Init with kpatch.log service = 1", KpLog_Init(fakekp::api()), 1);
         HCHECK("KpLog_HasCore is true with the service", KpLog_HasCore());
         // the longest line the provider writes: ADD with a 32-char tag, 16-char area and large coordinates
-        KP_LOG(chAdd, "ADD c=%08lx id=%08x tag=\"%s\" area=%s pos=%.1f,%.1f,%.1f origin=%s", 0xFFFFFFFFul, 0xFFFFFFFFu,
-               std::string(32, 't').c_str(), std::string(16, 'a').c_str(), -99999.9f, -99999.9f, -99999.9f, "engine");
+        KP_LOG(chAdd, "ADD c=%08lx id=%08x tag=\"%s\" via=%s area=%s pos=%.1f,%.1f,%.1f origin=%s", 0xFFFFFFFFul, 0xFFFFFFFFu,
+               std::string(32, 't').c_str(), "none", std::string(16, 'a').c_str(), -99999.9f, -99999.9f, -99999.9f, "engine");
         HCHECK("KP_LOG reaches the core service", !fakekp::state().logLines.empty());
         const std::string line = fakekp::state().logLines.empty() ? std::string() : fakekp::state().logLines.back();
```

## Fix check
- 22a3d7a: Fixed save-reload duplicate spawn by reading `c+0x18` first, but left R2-NW-05, R2-NW-06, and R2-NW-07.
- f55b8c1: f55b8c1 looks correct.
- 78275a0: Added `tag18` logging and dump-trigger checks, but left R2-NW-02.
- 30984d7: Added the `k1p_juhani` roster row and harvest tag; when `k1p_juhani.utc` is not installed in `Override/`, `LoadFromTemplate` fails on every `204tel` visit and hits R2-NW-03.

## Retracted
- `K1SpawnProbe.cpp:323` (`if (fromLib) return;` skipping `wantNpc` dumps for `K2NW_ORIGIN_LIB` adds): dropped because `K1SpawnProbe.cpp:256` explicitly documents that library spawns are logged but never dumped (the dump targets engine-added or save-restored creatures).
- `NpcWorldProvider.cpp:248-250` (`ci.x/y/z` reading `0, 0, 0` on `K2NW_ORIGIN_LIB` `K2NW_CREATURE_ADDED` events because `0x00568BD0` writes `+0x94..+0x9c` after the detour runs): dropped because `kotor2-steam-aspyr.hooks.toml:9` and `NpcWorld.h:68` explicitly specify that `x, y, z` are entry values at `AddToArea`.
- `NpcWorldProvider.cpp:180` (`return id ? id : kPlaceholderId;` returning non-zero `0x7f000000` when `p+4` reads as 0): dropped because `AddToArea` has already succeeded at line 176, so returning a non-zero sentinel ensures `nw::World::spawn` counts the spawned creature toward `spawnedThisVisit_` and records its tag.
- `NpcWorldProvider.cpp:258` and `K1SpawnProbe.cpp:312-313` (`a + 0x110` pointer arithmetic before `probeRead`): dropped because `0x110 < 0x10000`, so any 32-bit wraparound lands below `0x10000` and is rejected by `saferead::probeRead` (`SafeRead.h:101`).

## Not assessed
- Exact engine binary addresses (`0x00533830`, `0x00543F10`, `0x0055A460`, `0x00561F30`, `0x00568BD0`, `0x00572A70`, `0x00710840`, `0x00919723`), struct size `0x1220`, and whether `c+0x18` holds a `char*` tag pointer across all non-creature / special creature subclasses at `AddToArea` entry: requires live play / reverse engineering beyond source review.
