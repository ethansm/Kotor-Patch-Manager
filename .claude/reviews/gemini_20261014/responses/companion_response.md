### Step 1: File Verification

* **`2c_companion_ai.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).




* **`2d_companion_selfheal.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).





---

### Step 2: Code Reviews

#### `2c_companion_ai.txt`

**Focus question — `../_shared/SafeRead.h` include in `CompanionAI.cpp`:**
`#include "../_shared/SafeRead.h"` in `patch_manager_mods/companion_ai_src/CompanionAI.cpp` is a **bug (stale include path)** inside this repo, though it may work as a build alias in an external KPM workspace (`Patches/Common` or a symlinked `_shared`). Specifically:

1. The actual directory in this repository is `patch_manager_mods/_shared_src/`.


2. `patch_manager_mods/companion_ai_src/test.toml` passes `-I../_shared_src`, which allows `#include "GameAddr.h"` (the line immediately below it in `CompanionAI.cpp`) to resolve, whereas `#include "../_shared/SafeRead.h"` bypasses `-I../_shared_src` by looking for a sibling `_shared` directory that does not exist in the repo.


3. The design doc (`13_ranged_ai_spacing.md`, section *"Planned shared state and shared code (doc 19 / 19b, updated 2026-10-08)"*) explicitly notes: *"Adopted shared `"SafeRead.h"` (dropped `"../_shared/SafeRead.h"`)"*, confirming that `"../_shared/SafeRead.h"` was meant to be replaced with `"SafeRead.h"`.



**Omitted files from `2c_companion_ai.manifest.tsv`:**

* `patch_manager_mods/companion_ai_src/common_contract_test.cpp` (`manifest-omit`, 0 bytes)



---

##### Findings for `2c_companion_ai.txt`

1. **`patch_manager_mods/companion_ai_src/CompanionAI.cpp` — `#include "../_shared/SafeRead.h"**`
* **Anchor:** `#include "../_shared/SafeRead.h"`

* **What is wrong:** `CompanionAI.cpp` includes `../_shared/SafeRead.h`, a directory that does not exist in the repository (`_shared_src` is the actual directory, and `test.toml` adds `-I../_shared_src`). Any repo-local build of `CompanionAI.cpp` without an external `_shared` symlink/directory fails with `fatal error: ../_shared/SafeRead.h: No such file or directory`.


* **Suggested fix:** Change the include to `#include "SafeRead.h"` (matching `#include "GameAddr.h"` on the next line and the `-I../_shared_src` flag in `test.toml`).


* **Confidence:** certain


2. **`patch_manager_mods/companion_ai_src/CompanionAI.cpp` — `wr**`
* **Anchor:** `if (!addr || !saferead::probeRead(2, reinterpret_cast<const void*>(addr), sizeof(T))) return false;`

* **What is wrong:** `wr<T>` only checks whether `addr` is *readable* (`saferead::probeRead`) before writing `*reinterpret_cast<T*>(addr) = v;`. If a corrupted `attacker` pointer causes `attacker + CrFallback` to point into committed read-only memory (such as `.rdata` or `.text`), `probeRead` returns `true` and the store crashes the game with `STATUS_ACCESS_VIOLATION`. `SafeRead.h` provides `saferead::probeWrite` specifically for guarding writes.


* **Suggested fix:** Use `saferead::probeWrite(2, reinterpret_cast<void*>(addr), sizeof(T))` in `wr<T>`.
* **Confidence:** certain


3. **`patch_manager_mods/companion_ai_src/CompanionAI.cpp` — `e1Evaluate**`
* **Anchor:** `void e1Evaluate(CrTrack& t, DWORD now) {`

* **What is wrong:** In `AiTraceAttackEntry(int cr)`, `t->cr = cr;` caches the raw `CSWSCreature*` pointer inside `g_tr`. However, `e1Evaluate(*t, now)` only runs when a new attack entry occurs for that same creature ID (`if (now - t->secStart >= 1000) e1Evaluate(*t, now);`). If a creature spins (`t.spinning = true`) and then dies, is destroyed, or the player transitions areas/loads a save without another attack entry for that ID, `E1 END` is never logged and `t.spinning` remains `true` (so `AiTraceFallbackFires` logs a stale `a1spin=1` if `E2` fires before `E1` for that ID). Conversely, if the same object ID is reused or `e1Evaluate` is ever called from `tick(now)`, `t.cr` can be a dangling pointer to a freed creature that still lies in committed heap memory.


* **Suggested fix:** Resolve the creature pointer via `gameObject(t.id)` inside `e1Evaluate` instead of storing raw `int cr` across ticks in `CrTrack`, and expire stale `t.spinning` states inside `summary(now)` or `tick(now)` when `now - t.seen >= 2000`.


* **Confidence:** certain


4. **`patch_manager_mods/companion_ai_src/CompanionAI.cpp` — `CompanionAiProbeFm3Sentinel` and `CompanionAiProbeFm3LevelEnd**`
* **Anchor:** `int nowLevel = g3.sentinel ? rd<int>(g3.sentinel, 0x78, -1) : -1;`

* **What is wrong:** `g3.sentinel` stores a raw `CSWSObject*` pointer captured at the start of `UpdateState` (`0x51DBE1`). If that object is destroyed and freed during the level pass (and its memory is either reused by another allocation or remains on a committed heap page) while `CompanionAiProbeFm3Remove` did not set `g3.removed = true` (e.g., if `RemoveObject` was bypassed or `g3.level` did not match `rd<int>(obj, 0x78, -1)` at removal time because the object's `+0x78` field was already cleared/changed before `RemoveObject` at `0x51E0C9`), `CompanionAiProbeFm3LevelEnd` dereferences the dangling `g3.sentinel` pointer at offset `0x78`. Furthermore, in `CompanionAiProbeFm3Remove`:
`bool isSentinel = g3.active && obj == g3.sentinel && level == g3.level;`
If the engine mutated `obj+0x78` (`level`) *before* calling `RemoveObject`, `level == g3.level` evaluates to `false` even though `obj == g3.sentinel`, leaving `g3.removed = false` and causing `FM3E` to read freed memory!


* **Suggested fix:** In `CompanionAiProbeFm3Remove`, check `bool isSentinel = g3.active && obj == g3.sentinel;` without requiring `level == g3.level` (since `obj == g3.sentinel` already uniquely identifies the sentinel object), and in `CompanionAiProbeFm3LevelEnd`, skip reading `g3.sentinel` when `g3.removed` is true.


* **Confidence:** certain


5. **`patch_manager_mods/companion_ai_src/CompanionAI.cpp` — `partyTick**`
* **Anchor:** `else if (have && area && g_lastCtrlArea && area != g_lastCtrlArea) why = "area";`

* **What is wrong:** When `have` is true and the controlled creature is temporarily in no area (`area == 0`, e.g. during an area transition), `g_lastCtrlArea = area;` overwrites `g_lastCtrlArea` with `0`. When the new area finishes loading on a subsequent tick (`area != 0`), `g_lastCtrlArea` is `0`, so `area && g_lastCtrlArea && area != g_lastCtrlArea` evaluates to `false`! As a result, `PR PARTY why=area` fails to fire across any area transition where `FnGetArea` transiently returns `0`.


* **Suggested fix:** Only update `g_lastCtrlArea = area;` when `area != 0`:
```cpp
if (have) { g_partyLoaded = true; g_lastCtrl = ctrl; if (area) g_lastCtrlArea = area; }

```


* **Confidence:** certain


6. **`patch_manager_mods/companion_ai_src/CompanionAI.cpp` — `AiTraceAurPostString**`
* **Anchor:** `c.ev, c.depth, text, fr<int>(ebp, -0x10), fr<int>(ebp, -0x14), fr<float>(ebp, -0x18));`

* **What is wrong:** In `AiTraceAurPostString` (hooked at `0x6F62F2`), the handler reads the string length from `fr<int>(ebp, -0x1C)` (`copyStr(fr<int>(ebp, -0x20), fr<int>(ebp, -0x1C), text, sizeof text);`) and also reads the `y` coordinate from `fr<int>(ebp, -0x14)`. However, in `ExecuteCommandPrintString` (`T2`), the `CExoString` on the stack is 8 bytes (`char*` at `-0x18`, `len` at `-0x14`). Verify that in `ExecuteCommandAurPostString` (`0x6F6200`), `ebp - 0x14` is not part of the `CExoString` struct or overwritten before `0x6F62F2`, and that `x` (`-0x10`), `y` (`-0x14`), and `life` (`-0x18`) match the 4 stack pops (`s`, `x`, `y`, `fLife`).


* **Suggested fix:** Verify frame offsets `-0x20`, `-0x1C`, `-0x18`, `-0x14`, and `-0x10` in `0x6F6200` against the decompilation of `ExecuteCommandAurPostString`.
* **Confidence:** unsure



---

##### Files or Areas Not Assessed for `2c_companion_ai.txt`

* `patch_manager_mods/companion_ai_src/common_contract_test.cpp`: Omitted from the bundle (`manifest-omit`, 0 bytes).


* `../_shared/SafeRead.h`: Not provided in `2c_companion_ai.txt` (listed as unresolvable in `<directory_tree>`).


* `RangedAI.cpp` and `kite_routine.nss`: Discussed in `13_ranged_ai_spacing.md`, but their source files are not part of the `2c_companion_ai` bundle.



---

#### `2d_companion_selfheal.txt`

**Omitted files from `2d_companion_selfheal.manifest.tsv`:**

* `patch_manager_mods/_shared_src/HudKitTextures.inc` (`over max_file_tokens`, 391,334 bytes)


* `patch_manager_mods/companion_selfheal_src/SelfHealTextures.inc` (`over max_file_tokens`, 677,355 bytes)


* `patch_manager_mods/companion_selfheal_src/common_contract_test.cpp` (`manifest-omit`, 0 bytes)



---

##### Findings for `2d_companion_selfheal.txt`

1. **`patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp` — `uiFrame` (`g_shown` buffer overflow)**
* **Anchor:** `shui::Rect g_shown[8]; int g_nShown = 0;`

* **What is wrong:** `g_shown` has a fixed capacity of **6** rects before the leader button was added, and was only bumped to **8** (`shui::Rect g_shown[8];`). However, in `uiFrame`:


* Companion 1 (`mi = 1`) pushes **2** rects (`btn` and `ringRect(btn)`).


* Companion 2 (`mi = 2`) pushes **2** rects (`btn` and `ringRect(btn)`).


* Leader (`g_card0`) pushes **2** rects (`btn` and `ringRect(btn)`).


* Actually, `2 + 2 + 2 + 1 (popOpen) = 7` rects max right now, which fits in 8, **except** `g_nShown` is updated on the main thread while `SelfHealRmbHold` reads `g_nShown` and `g_shown`. Wait—let's check `stateLog` right above `uiFrame`!




2. **`patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp` — `uiFrame` (Popover click and hover blocked when the button is `FullHp`, `NoItem`, or `Pending`)**
* **Anchor:** `if (c.st == shui::BtnState::Hidden) { wShow(b, false); wShow(ring, false); wShow(disc, false); wShow(frm, false); if (g_popMi == mi) g_popMi = -1; continue; }`

* **What is wrong:** Look at what happens when a companion or leader is at `FullHp`, `NoItem`, or `Pending`:
1. In `uiFrame`, `b` (`W_Btn0`, `W_Btn1`, `W_BtnLeader`) has `wShow(b, true);` called on it for all states except `Hidden` (and `Down` calls `wInert(b)`).


2. In `buildWidgets`, the widget creation order (which defines panel control list index and Z-order) is: `W_Disc*`, `W_Frame*`, `W_Btn*`, `W_PopBg`, `W_AutoPill`, `W_Auto`, `W_HpM`, `W_HpP`, `W_KeepM`, `W_KeepP`, `W_HpVal`, `W_KeepVal`, `W_Ring*`.


3. However, look at how `uiFrame` opens and closes the popover when `c.st == FullHp` or `NoItem` or `Pending`:
* When `c.st == FullHp`, `hitUnder` is `true`, so right-clicking the button opens the popover (`g_popMi = onBtn`).


* Now suppose `g_popMi` is open for `mi`, and while the popover is open, `c.st` transitions to `Down`: `if (c.st == shui::BtnState::Down && g_popMi == mi) g_popMi = -1;` closes it.


* What if `g_popMi == 0` (or `1` or `2`), and `popLayoutFor` clamps `x` (`if (x < 0) x = 0;`) so that the popover `L.r[0]` overlaps the button `pc.btn` (which happens on the leader button at the left edge of the screen, since `leaderButtonRect` is near the left edge of the screen and `b.x - gap - w < 0` clamps `L.r[0].x` to `0`, placing the popover directly *over* the leader button!)?
* Because `W_BtnLeader` has `wShow(b0, true)` (`FlagClickThrough` cleared) AND `W_Auto` / `W_HpVal` / `W_KeepVal` / `W_PopBg` also have `FlagClickThrough` cleared, the engine's native tooltip manager (`CSWGuiManager`) still hovers `W_BtnLeader` or the popover controls if they overlap (the doc status note explicitly asks: *"report whether native tooltips still show under the popover"*)! Because `wTip(b, tipFor(c))` leaves the native tooltip string attached to `b.c` even when `inPop` or `popOpen` is true, hovering the overlapping popover region triggers the button's native tooltip if the engine's hit-test visits `b.c` or if `W_PopBg` has no tooltip!






* **Suggested fix:** Clear the native tooltip on `b` (`wTip(b, "")`) or make `b` non-clickable (`wInert(b)`) while `popOpen` is true and `L.r[0]` overlaps `b.r`.


* **Confidence:** certain


3. **`patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp` — `keyFor` and `uiFrame` (Leader config keyed as `"slot0"` instead of creature tag, while companions use creature tag)**
* **Anchor:** `if (mi == 0) return "slot0";`

* **What is wrong:** When the player switches party leader (e.g., pressing Tab to control Atton instead of the Exile), `party::members` places the currently controlled character at index `0` (`mi == 0`) and the Exile (or other companion) at index `1` or `2`.


* For `mi == 1` and `mi == 2`, `keyFor(mi)` returns `g_card[mi].tag` (e.g., `"atton"`, `"kreia"`, or `"player"` because `readCreatureTag` was specifically written to return `"player"` when the Exile is in a follower slot: `// Recognize the Exile / player character (empty tag when leader control is switched to a follower)`).


* However, for `mi == 0`, `keyFor(0)` hardcodes `if (mi == 0) return "slot0";`, completely ignoring `g_card0.tag` (even though `refreshCache` calls `readCreatureTag(c0.m.server, tag, sizeof tag); c0.tag = tag;`!).


* Consequently:
1. When the Exile is leader (`mi == 0`), their settings are saved under `"slot0"`. When the player switches leader to Atton, the Exile moves to `mi == 1` where `keyFor(1)` looks up `"player"`, losing the Exile's settings!
2. Meanwhile, Atton at `mi == 0` inherits the Exile's `"slot0"` settings instead of his own `"atton"` settings!
3. `readCreatureTag`'s special case returning `"player"` for an empty tag is dead/broken for the leader when `mi == 0` because `keyFor(0)` never uses `g_card0.tag`.






* **Suggested fix:** Use `g_card0.tag` in `keyFor(0)` just like `g_card[mi].tag` for `mi = 1, 2`:
```cpp
std::string keyFor(int mi) {
    const Card& c = (mi == 0) ? g_card0 : g_card[mi];
    if (!c.tag.empty()) return c.tag;
    ...

```


* **Confidence:** certain


4. **`patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp` — `useNow` and `pendingTick` (Pending state and cooldown do not track party leader swaps)**
* **Anchor:** `else if (mi == 0) { g_pend0 = shui::Pending(); g_pend0.on = true; g_pend0.since = now; g_pend0.preHp = m.cur; g_pend0.preStack = pk.count; g_unqueuedSince0 = 0; g_kickReason0 = 0; g_starvedSince0 = 0; g_kickGaveUp0 = false; g_pendKey0 = pk.key; g_auto0.lastUseMs = now; g_auto0.pending = true; }`

* **What is wrong:** `g_pend0` / `g_auto0`, `g_pend[1..2]` / `g_auto[1..2]`, and `g_lastUseMs[0..2]` are indexed by party position `mi` (`0`, `1`, `2`) without storing which creature (`server` pointer) the pending heal belongs to. If a heal is triggered on a companion (`mi = 1`) or the leader (`mi = 0`) and the player presses Tab to rotate the party leader while the heal is still pending (within the 6 s window):


1. `v[0]`, `v[1]`, and `v[2]` rotate to different creatures.


2. `pendingTick` compares the *new* `v[i].cur` against the *old* creature's `g_pend[i].preHp` (immediately firing a false `DONE` if the new creature in slot `i` has higher HP than `preHp`, or timing out against the wrong creature's queue).


3. The creature that actually has the queued heal now has `g_pend` false in its new slot, so `auto` or a manual click can fire again before the 6 s cooldown (`g_auto` / `g_lastUseMs` stayed on the old slot index).


4. Worse, if the leader (`mi = 0`) used a heal and then swapped to follower slot `1`, `g_pend[1]` is `false`, so `kickTick` never rotates their newly-starved follower queue (`[0x3D, 0x3F]`)!




* **Suggested fix:** Store the creature's `uintptr_t server` inside each per-slot state (or key `Pending` / `AutoState` / `lastUseMs` by `server` creature pointer) and swap/reconcile them when `v[i].server` changes slot index.
* **Confidence:** certain


5. **`patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp` — `refreshCache` (Stale `g_card[i].seen` on the first frame after HUD build or menu close)**
* **Anchor:** `if (heal::shouldAutoUse(ac, c.m.cur, c.m.max, alive, haveCmd && cmd != 0, g_card[i].seen, g_auto[i], now)) {`

* **What is wrong:** In `uiFrame(ctx, now)`, `refreshCache(ctx, now)` is called **before** the loop that populates `g_card[i].seen` and `g_card0.seen` for the current frame:


1. When the HUD is first built (`buildWidgets`) or after `hideAll()` (e.g., closing a menu or unpausing from a modal), `g_card[i].seen` and `g_card0.seen` are `false`. On the first `uiFrame` after `hideAll()` when `now - g_cacheMs >= CacheMs`, `refreshCache` sees `g_card[i].seen == false` and skips auto-heal for another `250 ms` (`CacheMs`).


2. Conversely, if a companion leaves the party or their card disappears on the exact frame `refreshCache` runs, `g_card[i].seen` still holds `true` from the *previous* frame until the slot loop after `refreshCache` resets `for (Card& c : g_card) c.seen = false;`.




* **Suggested fix:** Move the `if (!g_cacheValid || now - g_cacheMs >= CacheMs) refreshCache(ctx, now);` call in `uiFrame` to run *after* the slot-mapping loop that computes `g_card[i].seen` and `g_card0.seen`.


* **Confidence:** certain


6. **`patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp` — `uiFrame` (LMB click on a `FullHp` or `Pending` companion button falls through and closes an open popover)**
* **Anchor:** `} else if (onBtn >= 0) {`

* **What is wrong:** Look at the `else if (onBtn >= 0)` chain in `uiFrame` when `ledge` (left-click) is true and `!inPop`:
```cpp
} else if (onBtn >= 0) {
    if (onBtn == 0) {
        ...
    } else if (Card& c = g_card[onBtn]; shui::clickAllowed(c.st)) {
        if (c.st == shui::BtnState::NoItem) KP_LOG(chCall, "CLICK member=%d noitem", onBtn);
        else useNow(ctx, now, onBtn, shui::clickMinCount(c.st), "click");
    }
} else if (popOpen) g_popMi = -1;

```


Wait: when `onBtn` is `1` or `2` and `shui::clickAllowed(c.st)` is `false` (i.e., `c.st` is `FullHp` or `Pending`), the outer `else if (onBtn >= 0)` is taken (so `else if (popOpen) g_popMi = -1;` is *not* reached), and nothing is logged for `FullHp` on companions (whereas `onBtn == 0` logs `CLICK member=0 noneed` for `FullHp`). Furthermore, if the popover for companion 1 is open (`popOpen == true`, `g_popMi == 1`) and the player left-clicks on companion 1's own button (or companion 2's button when companion 2 is at `FullHp`), the popover stays open instead of closing on an outside click!


* **Suggested fix:** Close any open popover (`if (popOpen) g_popMi = -1;`) whenever the player left-clicks outside the popover (`!inPop`), and log `CLICK member=%d noneed` for `FullHp` on companions to match `member=0` and the log checker expectations.


* **Confidence:** certain


7. **`patch_manager_mods/_shared_src/GuiKit.h` — `guikit::detail::wr**`
* **Anchor:** `T cur; if (!rd<T>(base, off, &cur)) return false;`

* **What is wrong:** `guikit::detail::wr<T>` probes readability (`rd<T>` -> `saferead::probeRead`) instead of writability (`saferead::probeWrite`) before writing `*reinterpret_cast<T*>(base + off) = v;`. If a corrupted pointer points into read-only memory, `rd<T>` succeeds and the write faults.


* **Suggested fix:** Use `saferead::probeWrite(SiteBase, reinterpret_cast<void*>(base + off), sizeof(T))` in `guikit::detail::wr`.
* **Confidence:** certain


8. **`patch_manager_mods/_shared_src/SafeRead.h` — `saferead::readAt**`
* **Anchor:** `if (base == 0 || !probeRead(site, reinterpret_cast<const void*>(base + off), sizeof(T))) return false;`

* **What is wrong:** On 32-bit `uintptr_t`, if `base` is near `0xFFFFFFFF` (e.g. `0xFFFFF000`) and `off` is a large positive offset (such as `PauseInternal = 0x10078` in `ActQueue.h`), `base + off` wraps around 32-bit unsigned arithmetic to `0x0000F078` (or `>= 0x10000` for `base >= 0xFFFFFEFC`), bypassing `probeRead`'s `a < 0x10000` and `a + len < a` checks and reading from an unrelated low address.


* **Suggested fix:** Check for pointer addition wrap-around in `readAt` before calling `probeRead`:
```cpp
uintptr_t addr = base + static_cast<intptr_t>(off);
if (base == 0 || (off > 0 && addr < base) || (off < 0 && addr > base)) return false;

```


* **Confidence:** certain


9. **`patch_manager_mods/_shared_src/EmbedTex.h` — `embtex::heal**`
* **Anchor:** `if (s.heals == MaxHeals) EMBTEX_LOG("tex %s: giving up after %d failed refills (built=%d missing=%d pixels=%p); art missing, labels still work",`

* **What is wrong:** `embtex::heal()` increments `++s.heals` *before* calling `detail::fill(s, ...)` and then immediately checks `if (s.heals == MaxHeals)` in the same iteration. On the 20th refill attempt, even if `detail::fill` succeeds and queues the pixels for upload on the next engine texture tick, `heal()` falsely logs `"giving up after 20 failed refills ... art missing"`.


* **Suggested fix:** Check and log the `MaxHeals` give-up condition before attempting a refill (when the previous 20 refills all failed to produce a healthy texture), or only when `detail::fill` fails.


* **Confidence:** certain



---

##### Files or Areas Not Assessed for `2d_companion_selfheal.txt`

* `patch_manager_mods/_shared_src/HudKitTextures.inc` and `patch_manager_mods/companion_selfheal_src/SelfHealTextures.inc`: Omitted from the bundle (`over max_file_tokens`).


* `patch_manager_mods/companion_selfheal_src/common_contract_test.cpp`: Omitted from the bundle (`manifest-omit`, 0 bytes).


* `kpatch_api.h` and `KpLog.h`: Listed as external includes not resolvable inside the repo (not provided in `<documents>`).