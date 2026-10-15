### Part 1: Holstered Weapons (`holster`)

1. **[holster] `patch_manager_mods/holster_src/HolsterLogic.h` — `hs::Fsm::onCombat` and `hs::Fsm::tick**`
* **Anchor:** `if (phase != PhHolstered) { phase = PhStowing; phaseStart = now; phaseLen = kDrawMs; }`

* **What is wrong and concrete input/state that triggers it:**
1. When combat ends (`combat == 0 && lastCombat != 0`), `onCombat` sets `stowPending = true; stowAt = now + kStowDelayMs;`. If combat was entered and exited rapidly within `< kDrawMs` (`350 ms`)—or if an in-combat weapon swap (`1000 ms`) started shortly before combat ended and the game was paused during the exit window so `tick()` wasn't called between `phaseStart + phaseLen` and `stowAt`—`tick(now)` at `now >= stowAt` transitions `phase` from `PhDrawing` (or `PhDrawn`) to `PhStowing` with `phaseStart = now; phaseLen = kDrawMs;`. If the weapon was only partly drawn when `PhStowing` began, `handFraction` jumps discontinuously to `1.0f` (`inHand()` at the hand) and slides over a full `kDrawMs` instead of reversing from its current fraction.
2. More importantly, across **pause/unpause, save/load, or area transitions**, `Fsm` uses raw `GetTickCount()` (`uint32_t now`) with no pause compensation or reset method (`reset()`). If combat ends and the player immediately opens a menu or pauses the game for `>= 1500 ms` (`kStowDelayMs`), `now - stowAt >= 0` is already true while the game simulation is paused (and `now - phaseStart >= phaseLen` finishes the `350 ms` stow slide while paused), so the weapon teleports to the hip before the post-combat exit animation even runs. Conversely, if a creature is in `PhDrawn` or `PhDrawing` when loading a save or transitioning to a new area where the creature's combat flag is already `0` (`lastCombat` was `1` in the old area/save, or if `onCombat` only fires on setter calls and not on area load), `Fsm` has no `reset()` on area load/save load, leaving the state machine stuck in `PhDrawn` out of combat until the creature enters and leaves combat again.


* **Suggested fix:** Add a `reset(int initialCombat = 0)` method to `Fsm` for area load / save load / creature creation (`phase = initialCombat ? PhDrawn : PhHolstered; stowPending = false; lastCombat = initialCombat;`), and advance `Fsm` timers using unpaused delta time (or freeze `phaseStart` / `stowAt` while paused) rather than raw wall-clock `GetTickCount()` across pauses.
* **Confidence:** certain


2. **[holster] `patch_manager_mods/holster_src/Holster.cpp` — `HolsterSwap` (and `patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml`)**
* **Anchor:** `# hook sits right after the prologue at 0x005DF926: MOV [EBP-0x6C],ECX / MOV EAX,[EBP-0x6C] (6 bytes, no relative branches; ECX is still `this`).`

* **What is wrong and concrete input/state that triggers it:** `HolsterSwap` hooks `0x005DF926` (`6` bytes after the function entry `0x005DF920`, because `cmdq-probe` hooks `0x005DF920`) and declares its parameter as `source = "ecx"`. Because `0x005DF920` is a `__thiscall` function, `ECX` holds `this` at `0x005DF920`, which executes `cmdq-probe`'s detour trampoline first before reaching `0x005DF926`. In x86 `__cdecl` / Win32 ABI, `ECX` is a volatile (caller-saved) register; if KPM's detour stub at `0x005DF920` (or any instruction in `0x005DF920..0x005DF925`) does not restore `ECX` across `CmdqProbeReequip` (or if `0x005DF920` is ever replaced/modified), `ECX` at `0x005DF926` could be clobbered before `MOV [EBP-0x6C], ECX` runs. (Note: `manifest.toml` also declares `conflicts = []`, not listing any mod that might hook `0x005DF920` with a longer span `> 6` bytes.)


* **Suggested fix:** Verify that KPM's detour trampoline (`pushad`/`popad` or explicit register save) preserves `ECX` across `0x005DF920` into `0x005DF926`, and document the `6`-byte span constraint on `0x005DF920` in `manifest.toml` / hook checks.
* **Confidence:** likely


3. **[holster] `patch_manager_mods/holster_src/Holster.cpp` — `applyBoneTest` (and `patch_manager_mods/holster_src/HolsterBonePatch.h` — `hs::patchNodeLiteral`)**
* **Anchor:** `const int rc = hs::patchNodeLiteral(s.at, s.orig, ptr, seen);`

* **What is wrong and concrete input/state that triggers it:** `hs::patchNodeLiteral` calls `codepatch::writeChecked(at, orig, repl, 5, seen)` without first calling `codepatch::committed(at, 5)`. Inside `codepatch::writeChecked`, `memcpy(seen, p, n)` reads `5` bytes directly from `at` (`0x00763AF9` / `0x00763E2B`) *before* `VirtualProtect` or any readability probe. Although `applyBoneTest()` checks `if (!k2addr::get())`, `k2addr::selectBySha("")` returns `&kSteam` whenever `KOTOR_VERSION_SHA` is unset or empty. If the DLL is loaded into a process where `KOTOR_VERSION_SHA` is unset and `0x00763AF9` is uncommitted or non-readable, `writeChecked` faults on `memcpy(seen, p, 5)` instead of refusing cleanly.


* **Suggested fix:** Check `if (!codepatch::committed(at, 5)) return -3;` inside `hs::patchNodeLiteral` (or inside `codepatch::writeChecked`) before `memcpy(seen, p, 5)`.


* **Confidence:** certain


4. **[holster] `patch_manager_mods/holster_src/Holster.cpp` — `HolsterCombat**`
* **Anchor:** `const uint32_t oldState = off ? rdU32(kSiteComb, c, off) : 0xFFFFFFFFu;`

* **What is wrong and concrete input/state that triggers it:** When `off` is non-zero (`0x138`) but `creature` (`c`) is `nullptr` or an unreadable pointer, `rdU32(kSiteComb, c, off)` fails `saferead::readAt` and returns `0`. `HolsterCombat` cannot distinguish an unreadable memory read from a valid `oldState == 0` (out of combat), so for an unreadable `creature` pointer with `newState != 0` it logs `old=0 new=1 edge=1` as if a real `0 -> 1` combat transition occurred.


* **Suggested fix:** Pass a `bool* ok` or default value (`0xFFFFFFFFu`) to `saferead::readAt(kSiteComb, c, off, &oldState)` so an unreadable `creature + 0x138` logs `old=-1 edge=0` instead of a false `old=0 edge=1` transition.


* **Confidence:** certain


5. **[holster] `patch_manager_mods/holster_src/holster_logic_test.cpp` — `testFsmStowDelayAndCancel` and `testFsmSwapAndRollover**`
* **Anchor:** `KT_CHECK(!g.stowPending && g.phase == PhDrawing);`

* **What is wrong and concrete input/state that triggers it:** `holster_logic_test.cpp` never tests two key transitions in `hs::Fsm`:
1. Calling `onCombat(2, now)` while already in combat (`lastCombat == 1`, non-zero to non-zero combat state update, which the engine does when combat state flags change between `1` and `2` at `this+0x138`): if `if (combat != 0 && lastCombat == 0)` in `onCombat` were written as `if (combat != 0)`, every non-zero combat state update during combat would restart `PhDrawing` from `0.f`, and `holster_logic_test.cpp` would still pass.


2. Calling `onSwap(now)` while `stowPending == true` (e.g., player swaps weapons during the `1500 ms` post-combat stow delay): `onSwap` clears `stowPending = false;` without resetting `lastCombat`, so the weapon remains permanently in `PhDrawn` after `kSwapMs` and never stows unless the creature enters and exits combat again! Neither the bug nor the `stowPending = false` assignment in `onSwap` is tested in `holster_logic_test.cpp`.




* **Suggested fix:** In `Fsm::onSwap`, only clear `stowPending` if `lastCombat != 0` (or re-arm `stowAt = now + kSwapMs` when `lastCombat == 0` so an out-of-combat weapon swap stows the newly swapped weapon after displaying it), and add tests in `holster_logic_test.cpp` for `onCombat(2, ...)` after `onCombat(1, ...)` and for `onSwap` when `lastCombat == 0` / `stowPending == true`.


* **Confidence:** certain


6. **[holster] `patch_manager_mods/_shared_src/SafeRead.h` — `saferead::readAt` (used by `Holster.cpp` and `common_contract_test.cpp`)**
* **Anchor:** `if (base == 0 || !probeRead(site, reinterpret_cast<const void*>(base + off), sizeof(T))) return false;`

* **What is wrong and concrete input/state that triggers it:** On 32-bit builds (`uintptr_t` is 32-bit unsigned), if `base` is a corrupted high pointer near `0xFFFFFFFF` (e.g., `0xFFFFFF00` or `0xFFFFFE00`, such as `-1` or a negative sentinel cast to a pointer) and `off` is a positive offset like `0x278` (`Creature_HandleR`) or `0x27C` (`Creature_HandleL`), `base + off` wraps around 32-bit unsigned arithmetic to a low address (or `>= 0x10000` if `base + off >= 0x10000`), bypassing `probeRead`'s `a + len < a` check.


* **Suggested fix:** Check for pointer-addition overflow in `saferead::readAt` before calling `probeRead`:
```cpp
uintptr_t addr = base + static_cast<intptr_t>(off);
if (base == 0 || (off > 0 && addr < base) || (off < 0 && addr > base)) return false;

```


* **Confidence:** certain



---

### Part 2: NPC World and Native Spawn (`npc`)

7. **[npc] `patch_manager_mods/npc_world_src/NpcWorldProvider.cpp` — `engineSpawn**`
* **Anchor:** `void** vtbl = *reinterpret_cast<void***>(p);`

* **What is wrong and concrete input/state that triggers it:** When `LoadTemplate` (`0x00572A70`) fails and returns `0`, `engineSpawn` cleans up the allocated `CSWSCreature` (`p`) by executing:
```cpp
void** vtbl = *reinterpret_cast<void***>(p);
reinterpret_cast<DtorFn>(vtbl[0])(p, 1);

```


There are two severe defects here:
1. **Wrong vtable slot / double-delete or crash:** Verify `CSWSCreature`'s vtable at `0x00561F30`! Even if `vtbl[0]` is the scalar deleting destructor (`~CSWSCreature(this, 1)`), `engineSpawn` dereferences `*reinterpret_cast<void***>(p)` and `vtbl[0]` directly with **no `saferead::probeRead` check** (violating the file's own rule: *"Every engine read goes through saferead"*). If `kAddrCreatureCtor` or `kAddrLoadTemplate` failed early or corrupted the vtable pointer at `*p`, dereferencing `vtbl[0]` crashes the game immediately.


2. **Dangling server object table / ID registration on failed `LoadTemplate`:** `kAddrCreatureCtor` (`0x00561F30`) is called with `id = kPlaceholderId` (`0x7f000000u`) and `0`. In KOTOR II's Aurora/Odyssey engine, if `CSWSCreature::CSWSCreature(this, id, ...)` or `LoadFromTemplate` registers `this` in the server's `CServerExoApp` object array (`CGameObjectArray`) or allocates a real object ID before failing partway through `LoadFromTemplate`, calling only the C++ scalar deleting destructor `vtbl[0](p, 1)` frees the heap block `p` (`0x1220` bytes) without removing its entry from the server's object table or area/module lookup lists, leaving a dangling pointer in the engine's object array. Conversely, `cres` (`alignas(8) char cres[32]`) is initialized via `kAddrResrefInit` (`0x00710840`), which for a 16-byte `CResRef` is plain POD, so `cres` has no destructor issue.




* **Suggested fix:** Probe `p` and `vtbl` with `saferead::readAt` before invoking `vtbl[0](p, 1)`, and verify in the `0x00561F30` / `0x00572A70` disassembly whether a failed `LoadFromTemplate` in the engine's own `CreateObject` path calls `vtbl[0](p, 1)` directly or also unregisters the object from the server object table.
* **Confidence:** likely


8. **[npc] `patch_manager_mods/npc_world_src/NwCore.h` — `nw::World::onCreatureAdded**`
* **Anchor:** `if (!resrefEquals(area_, ci.areaRes)) { // first add of the load, or an area change without a load event: re-arm the gate`

* **What is wrong and concrete input/state that triggers it:** Look at what happens when the area changes *without* a `LoadModuleInProgress` (`onLoad`) event (for example, a fast in-module area transition or a save reload path that does not hit `0x0055A460`):
```cpp
if (!resrefEquals(area_, ci.areaRes)) { // first add of the load, or an area change without a load event: re-arm the gate
    setArea(ci.areaRes);
    gateReset();
    spawnedThisVisit_ = 0;
}

```


Notice what is **not** reset here: `seenCount_` and `seenOverflow_` are **never cleared** when `!resrefEquals(area_, ci.areaRes)`!


* Every creature tag seen in the *previous* area (e.g., `"MouseDroid"` or a commoner tag in area A, or `seenOverflow_ == true` if area A had `> 256` unique tags across transitions) remains in `seen_[]` in the new area B!
* Consequently, in area B, `tagPresent(tag)` returns `true` for any tag that existed in area A (or for *all* tags if `seenOverflow_` became `true` across multiple area transitions without `onLoad`), refusing valid spawns in area B with `SpawnResult::TagPresent`!


* Furthermore, `visitSalt_` is not updated on an area change without `onLoad`, and `nw_world_test.cpp` (Test 9: `"area change without a load event re-arms the gate and the spawn counter"`) only checks `w.stable()`, `w.area()`, and `w.spawnedCount()`, completely missing that `w.seenTags()` and `w.tagPresent("a")` from the previous area `204tel` were still present after transitioning to `261tel`!




* **Suggested fix:** Clear `seenCount_ = 0; seenOverflow_ = false;` inside the `if (!resrefEquals(area_, ci.areaRes))` block in `nw::World::onCreatureAdded`, and assert `KT_CHECK_EQ(w.seenTags(), 1)` / `KT_CHECK(!w.tagPresent("a"))` after switching to `"261tel"` in `nw_world_test.cpp` Test 9.


* **Confidence:** certain


9. **[npc] `patch_manager_mods/npc_world_src/NpcWorldProvider.cpp` — `NW_OnAddToArea` and `patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp` — `onCreatureAdded` (Duplicate spawns after save/reload or when tag chain is unreadable at `AddToArea` entry)**
* **Anchor:** `if (saferead::readAt(S_TagPtr, c, 0x24, &tagPtr) && tagPtr && saferead::readAt(S_TagPtr, tagPtr, 0x18, &charPtr))`

* **What is wrong and concrete input/state that triggers it:** The duplicate-spawn guard across save/reload relies entirely on `NW_OnAddToArea` reading the creature's tag at entry to `0x00568BD0` (`AddToArea`) via the pointer chain `*( *(creature + 0x24) + 0x18 )` and recording it in `seen_[]` via `rememberTag(ci.tag)`. There are two concrete failure modes that cause **duplicate spawns after save and reload**:


1. In `K1SpawnProbe.cpp`, `onCreatureAdded` explicitly includes a fallback `scanForTag(c, kCreatureDwords, &off)` and `scanForTag(tagPtr, kPtrDwords, &off)` with the comment: *"Fallback when the tag path (c+0x24 -> +0x18 -> char*) is wrong"* and *"While the NPC dump is still wanted, the fallback scan runs only when both tag strings came back empty"*. If `*( *(c + 0x24) + 0x18 )` is empty or lies at a different offset when the engine deserializes a saved creature from the `.sav` GIT during save load (or if `AddToArea` is called before the tag field is populated on save reload, or if saved creatures are restored via a different function than `0x00568BD0`), `NW_OnAddToArea` reads `tagStr = ""` (or never runs), so `rememberTag` records nothing! Then, 30 ticks later at `K2NW_AREA_STABLE`, `tagPresent("MouseDroid")` and `tagPresent("k1p_native_npc")` return `false`, and `RunStable` spawns a **second copy** on top of the saved creature.


2. Even when `engineSpawn` spawns a creature from a UTC template whose in-file `Tag` field differs from the `tag` argument passed to `Spawn(resref, tag, ...)` (or if the UTC's `Tag` is empty/different in case/truncation), during the initial visit `World::spawn` explicitly calls `rememberTag(tag)` with the caller's `tag` argument (`"MouseDroid"`, `"k1p_native_npc"`), whereas on **save and reload**, `NW_OnAddToArea` reads the creature's *engine* tag from the saved UTC data (`ci.tag`). If the UTC template's embedded `Tag` in `k1p_native_npc.utc` or `c_drdmouse01.utc` does not match `row.tag` in `kRoster` (`"MouseDroid"` / `"k1p_native_npc"`), `NW_OnAddToArea` on reload records the UTC's tag instead of `row.tag`, so `env.tagPresent(row.tag)` returns `false` on reload and spawns a duplicate every save/load cycle!




* **Suggested fix:** Verify in `engineSpawn` (or in `NW_OnAddToArea` when `spawning_ == true`) whether `ci.tag` read from `[c+0x24]+0x18` actually matches the `tag` passed to `Spawn()`, log a `WARN` if `ci.tag` is empty or mismatches `tag` during `origin == K2NW_ORIGIN_LIB` (which immediately diagnoses whether `[c+0x24]+0x18` works and matches `row.tag`), and if `ci.tag` differs from `tag`, overwrite the creature's tag in engine memory at spawn time so save serialization saves `row.tag`.
* **Confidence:** certain


10. **[npc] `patch_manager_mods/npc_world_src/NwCore.h` — `nw::World::onCreatureAdded` (Area pointer updated even when `areaRes` is unreadable or empty, or when `areaPtr` changes within the same `areaRes`)**
* **Anchor:** `if (ci.areaPtr && ci.areaRes && ci.areaRes[0]) {`

* **What is wrong and concrete input/state that triggers it:** Suppose the player is in `"204tel"` (`areaPtr_ = 0x10000000`, `area_ = "204tel"`, `stableFired_ = true`) and reloads a save of the **same area** `"204tel"` (or transitions out and back in) through a code path that does not fire `NW_OnLoad` (`0x0055A460`), or where the engine allocates a **new** `CSWSArea*` (`ci.areaPtr = 0x20000000`) that still has the same resref `"204tel"`. Look at `onCreatureAdded`:


```cpp
if (ci.areaPtr && ci.areaRes && ci.areaRes[0]) {
    areaPtr_ = ci.areaPtr;
    if (!resrefEquals(area_, ci.areaRes)) { // first add of the load, or an area change without a load event: re-arm the gate
        setArea(ci.areaRes);
        gateReset();
        spawnedThisVisit_ = 0;
    }
}

```


Because `resrefEquals("204tel", "204tel")` is `true` even when `ci.areaPtr != areaPtr_` (a newly constructed `CSWSArea*` instance for the same module!), `onCreatureAdded` updates `areaPtr_ = ci.areaPtr`, **does NOT re-arm the gate** (`stableFired_` stays `true`!), and **does NOT clear `seenCount_**`!
In fact, `nw_world_test.cpp` Test 9 explicitly has a line:


```cpp
add(w, "c", g_areaA, "261TEL"); // same resref in another case: same area, no re-arm

```


where `areaPtr` changed from `g_areaB` to `g_areaA`, yet `w.stable()` stayed `true`! If a new `CSWSArea*` object is created for the same area resref without an `onLoad` event, `K2NW_AREA_STABLE` never fires for the new area instance, or if a consumer calls `Spawn()` while the new area is still in the middle of loading its creatures, `stableFired_` is already `true` from the old area instance!


* **Suggested fix:** Also re-arm the gate (and clear `seenCount_`, `seenOverflow_`, `spawnedThisVisit_`) when `areaPtr_ != nullptr && ci.areaPtr != areaPtr_` (a different `CSWSArea*` pointer means a new area object was constructed).


* **Confidence:** certain


11. **[npc] `patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp` — `fmtRef16**`
* **Anchor:** `char* o = out;`

* **What is wrong and concrete input/state that triggers it:** `fmtRef16(char* out, size_t n, unsigned site, uintptr_t p)` takes the buffer capacity `n`, uses `snprintf(out, n, ...)` in the unreadable and printable branches, and then in the non-printable hex branch **completely ignores `n**`:
```cpp
char* o = out;
memcpy(o, "hex:", 4);
o += 4;
for (int i = 0; i < 16; ++i) {
    *o++ = kHex[rf[i] >> 4];
    *o++ = kHex[rf[i] & 0x0f];
    *o++ = (i < 15) ? ' ' : '\0';
}

```


This unconditionally writes `4 + 16 * 3 = 52` bytes into `out` regardless of `n`. If any caller passes a buffer with `n < 52`, `fmtRef16` overflows `out`.


* **Suggested fix:** Check `if (n < 52) { if (n > 0) out[0] = 0; return; }` at the start of the hex fallback in `fmtRef16` (or use `SafeRead.h`'s existing `saferead::readResRef16`).


* **Confidence:** certain


12. **[npc] `patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp` — `dumpEvent` and `dumpBlock**`
* **Anchor:** `if (!g_budget.fits(kEventLines)) return false;`

* **What is wrong and concrete input/state that triggers it:** `dumpEvent` checks `if (!g_budget.fits(kEventLines))` (`25` lines) against the total cap (`3000`) and then calls `logf("DUMP-HDR ...")` and `dumpBlock(...)` (`16` lines for creature, `4` lines for `DUMP24`, `4` lines for `DUMPST`). However, when `c`, `tagPtr`, or `st` is unreadable, `dumpEvent` calls `logf("%s skipped (unreadable)", pfx)` (`1` line) instead of `dumpBlock` (`16` or `4` lines). Conversely, if `g_budget` has `< 25` lines remaining (e.g., `20` lines left before `3000`), `dumpEvent` returns `false` without charging anything, so `chargeLine` is never called and the `"CAP reached at n=<seq>"` marker is never emitted if only dumps/scans are triggered near the cap. More importantly, `dumpEvent` is completely in `K1SpawnProbe.cpp` rather than `ClientCore.h`, so `k1_client_core_test.cpp` never tests `dumpEvent`, `dumpBlock`, `scanForTag`, or `fmtRef16`.


* **Suggested fix:** Move the line-count calculation or `dumpEvent` orchestration into `ClientCore.h` so `k1_client_core_test.cpp` can test partial/unreadable dumps and budget interactions.


* **Confidence:** likely


13. **[npc] `patch_manager_mods/npc_world_src/NwCore.h` — `nw::Dispatcher::add` (and `nw_registry_test.cpp`)**
* **Anchor:** `return nextToken++;`

* **What is wrong and concrete input/state that triggers it:** During a callback dispatch (`Dispatcher::dispatch`), `snap` holds a copy of the entries at the start of `dispatch`, and for each entry checks `if (!(snap[i].mask & ev.kind) || !live(snap[i].token)) continue;`. In `nw_registry_test.cpp` Section 3 (`"snapshot rules"`), `cbRegisters` registers a new consumer `cbC` inside a `K2NW_TICK` callback, and the test asserts that `cbC` is not called until the next event. However, if callback 0 **unregisters** an entry (reducing `n` from `2` to `1`) and then **registers** a new entry during the same `dispatch()`, the new entry is placed at `e[1]` with a new `nextToken`, so `live(snap[1].token)` returns `false` (good). What if `nextToken` were ever not strictly monotonic (e.g. if someone changed `nextToken` to reuse slot index `n + 1`)? Then `live(snap[1].token)` would return `true` for the newly registered consumer and skip/call the wrong entry! Because `nw_registry_test.cpp` never tests unregistering *and* registering within the same callback dispatch, changing token assignment to slot-based tokens (`return n;`) would not be caught by the snapshot test.


* **Suggested fix:** Add a test in `nw_registry_test.cpp` where callback A removes callback B (`g_tokB`) and immediately calls `d.add(...)` in the same callback, verifying that neither removed B nor the newly added consumer is called during that dispatch.


* **Confidence:** certain



---

### Files or Areas Not Assessed

* **Omitted / unresolvable headers (`kpatch_api.h`, `KpLog.h`, and host `windows.h` stub):** Listed in `<directory_tree>` under *"Includes not resolvable inside the repo (files not provided)"*. Because `kpatch_api.h` and `KpLog.h` were not included in `<documents>`, their internal implementation details could not be directly inspected (only their usage and the contracts pinned in `FakeKPatchApi.h` and `common_contract_test.cpp`).


* **Design docs 38 and 40 (`38_...md`, `40_npc_behaviour_variety.md`):** Explicitly omitted from the bundle per `<instructions>` due to the size cap.


* **Live `swkotor2.exe` disassembly at hook/call sites (`0x00561F30`, `0x00572A70`, `0x005DF920`, `0x007637E0`):** The binary executable is not in the bundle, so engine vtable layouts (such as `CSWSCreature` vtable slot `0` in `engineSpawn`) and caller register preservation across `cmdq-probe` at `0x005DF920..0x005DF926` could only be assessed from the provided source and comments.