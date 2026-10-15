### Step 1: File Verification

* **`2a_queue_hud_src.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).




* **`2b_queue_hud_tests.txt`**
* **Exact last line:** `</review_bundle>`

* **Ends with `</review_bundle>`:** Yes (not cut off).





---

### Step 2: Code Reviews

#### `2a_queue_hud_src.txt`

 Omitted files from `2a_queue_hud_src.manifest.tsv`:

* `patch_manager_mods/_shared_src/GuiKit.h` (`manifest-omit`, 0 bytes)


* `patch_manager_mods/_shared_src/HudKitTextures.inc` (`over max_file_tokens`, 391,334 bytes)


* `patch_manager_mods/_shared_src/K2Addr.h` (`manifest-omit`, 0 bytes)


* `patch_manager_mods/queue_hud_src/override/lbl_miqueue_teal.tga` (`binary`, 524,332 bytes)



---

##### Findings for `2a_queue_hud_src.txt`

1. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `tickItemQueues**`
* **Anchor:** `if (s.haveCur && (s.cur.subType == 9 || s.cur.subType == 10)) actionInQueue = true; // 9 = cast spell, 10 = use item`

* **What is wrong:** `actionInQueue` is computed from raw `s.haveCur` *before* `creal = qh::curReal(...)` filters out stale flagless item/spell round nodes (`subType == 9` or `10`, unpaused with `aa8 == 0 && ab8 == 0` for $\ge 1500\text{ ms}$). When a flagless item or spell node gets stuck in `s.cur` after $1500\text{ ms}$, `creal` becomes `false` and `STALE_CUR` is logged, but `actionInQueue` remains `true`. Because `actionInQueue` is passed into `makeEngineView` as `ev.actionPresent`, `activeVerdict` always hits Rule 1 (`if (ev.actionPresent) return AV_PRESENT;`). Consequently, `buf.active` never clears, `dispatchWhy` stays stuck on `DS_ACTIVE`, and the member's `ActionBuffer` deadlocks permanently despite `curReal` detecting the stale node.


* **Suggested fix:** Compute `creal` before `actionInQueue` and gate the `s.cur` check on `creal`:
```cpp
if (s.haveCur && !s.paused && s.aa8 == 0 && s.ab8 == 0) { if (buf.staleCurSince == 0) buf.staleCurSince = now; }
else buf.staleCurSince = 0;
const bool creal = qh::curReal(s.haveCur, s.haveCur ? (int)s.cur.subType : -1, s.aa8, s.ab8, s.paused, buf.staleCurSince, now);
if (creal && (s.cur.subType == 9 || s.cur.subType == 10)) actionInQueue = true;

```


* **Confidence:** certain


2. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `queueCountOf`, `QueueHudPerformUseItem`, `QueueHudPerformCastSpell`, and `tickItemQueues**`
* **Anchor:** `qh::DispatchWhy dsWhy = qh::dispatchWhy(buf, s.engineNe, creal, s.nrn);`

* **What is wrong:** `actq::read` counts every linked-list node in `cr+0x100` into `s.engineNe`, including a companion's permanent resting `0x3D` (`T_FOLLOW_LEADER`) action and any harmless leftover bare `0x3F` (`T_COMBAT`) with an empty round list (`ActQueue.h` notes: *"FollowLeader: a follower's permanent resting action"* and *"A bare 0x3F with an empty round list is a harmless leftover that stays behind 0x3D forever"*). Whereas `makeEngineView` ignores `0x3D` and bare `0x3F` when determining `ev.idle`, `queueCountOf` (used by `decideAction` in H9/H10) and step 3 of `tickItemQueues` (`dispatchWhy` and `shouldDispatchNext`) require `s.engineNe == 0`. Whenever a companion has `0x3D` in `cr+0x100` (or any party member has a leftover bare `0x3F`), `queueCountOf` returns $\ge 1$, forcing `decideAction` to buffer even the first command (`DECIDE_BUFFER`), and `tickItemQueues` then blocks forever at `DS_ENGINE_BUSY` (`WAIT why=engine_busy`) because `s.engineNe != 0`. Additionally, `queueCountOf` sets `*haveCur = ok && s.haveCur` instead of checking `qh::curReal`, so a post-combat ghost attack node (`subType == 1, aa8=ab8=0`) also forces `decideAction` to buffer.


* **Suggested fix:** Count only non-resting / active queue entries (excluding `0x3D` and bare `0x3F` when `nrn == 0 && !creal`) for `engineQueueLen` in `queueCountOf` and `tickItemQueues`, and use `qh::curReal` in `queueCountOf`.
* **Confidence:** certain


3. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `tickItemQueues` and ` patch_manager_mods/queue_hud_src/QueueHudLogic.h` — `shouldReInject` / `reInjectWhy**`
* **Anchor:** `const qh::ReInjectWhy riWhy = qh::reInjectWhy(buf, actionInQueue, hasFollowLeader, s.paused, now);`

* **What is wrong:** In `QueueHudLogic.h`, `shouldReInject` and `reInjectWhy` ignore `paused` and `now` (`(void)paused; (void)now;`) and return `true` / `RI_OK` immediately whenever `buf.active && !actionInQueue && hasFollowLeader && reInjectCount < 2`. In `tickItemQueues`, `if (riWhy == qh::RI_OK)` runs *before* the `else if (s.round != 0)` branch that implements `kVanishDebounceMs` ($150\text{ ms}$), `kMinActiveMs` ($500\text{ ms}$), `kReInjectSpacingMs` ($250\text{ ms}$), `g_cfg.pauseEatenWait`, and `g_cfg.pauseReinject`. Consequently, for a companion with `0x3D` (`hasFollowLeader == true`), if an action is wiped while paused (or takes 1 frame to appear in the server queue), `riWhy == RI_OK` fires on 2 consecutive frames with $0\text{ ms}$ debounce/spacing, ignores `pause_eaten_wait=1` and `pause_reinject=0`, exhausts `reInjectCount` (`kMaxReInjectCount = 2`) in 2 frames while still paused, and does not call `buf.markDispatched(now)` (leaving `goneSinceTick` unreset).


* **Suggested fix:** Gate `shouldReInject` / `reInjectWhy` on `!paused` (when `pauseEatenWait` is active) and require `now - buf.activeStartTick >= kVanishDebounceMs`, and call `buf.markDispatched(now)` on re-injection in `tickItemQueues`.


* **Confidence:** certain


4. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `handoffTick**`
* **Anchor:** `if (s.e[i].type == qh::T_USE_ITEM || s.e[i].type == qh::T_CAST_SPELL) midFlight = true;`

* **What is wrong:** `handoffTick` checks whether a previous hand-off command is still in flight by scanning only `s.e[i].type == T_USE_ITEM (0x2E) || T_CAST_SPELL (0x0F)`. However, `PerformUseItem` and `PerformCastSpell` in KOTOR II frequently enqueue a `0x3F` (`T_COMBAT`) entry backed by a round node with `subType == 9` (spell) or `10` (item) in `s.rn` or `s.cur` (which is why `engineHoldsAction` checks `s.rn[i].subType == 9 || 10` and `s.haveCur && (s.cur.subType == 9 || 10)`). Because `midFlight` ignores `s.rn` and `s.cur`:


1. If the active command at the combat edge was already executing (`inEngine == true`) and either `handoff_resend_held=0` or `heldHeadAction` returns `HELD_DROP`, `h.sent` is still `0` when the first buffered command is popped. `clearBeforeHandoff(g_cfg.handoffClearFirst, 0)` then calls `ServerClearAllActions`, wiping the in-flight active command!


2. When multiple buffered commands are handed off, the second command does not see the first command's `0x3F` + round node (`subType 9/10`) as `midFlight`.




* **Suggested fix:** Use `engineHoldsAction(s)` for `midFlight` (or check `s.rn` and `creal` for `subType == 9 || 10`), and do not call `ServerClearAllActions` before the first buffered command if the active command was kept in the engine (`inEngine`) at the combat edge.


* **Confidence:** certain


5. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `drawAll`, `paintLeader`, `drawStrips`, and `onFrameBegin**`
* **Anchor:** `const bool hardCombat = g_combat != 0 && !g_softCombat; // soft combat (ally-only targets) draws the strip like out of combat`

* **What is wrong:** `drawLeader` computes `hardCombat = g_combat != 0 && !g_softCombat` so the green leader queue stays visible during soft combat (when `g_combat != 0` due to an ally-targeted item/power with no hostiles). However, four other gates in the same frame test raw `g_combat != 0` instead of `hardCombat`:


1. `drawAll` checks `qh::shouldShow(g_cfg.enabled, g_combat != 0, ..., g_cfg.showInCombat)`. If the user sets `show_in_combat=0` in `queue_hud.ini`, `drawAll` returns early before calling `drawLeader`, making `drawLeader`'s `!g_softCombat` check unreachable.


2. `paintLeader` checks `qh::cancelVisible(..., g_combat != 0, ...)`, hiding `g_want[0]` (the leader Cancel Queue button) during soft combat.


3. `drawStrips` checks `if (g_cfg.cancel && g_combat == 0)` for companion cancel buttons.


4. `onFrameBegin` checks `if (!g_cfg.cancel || inMenu || g_combat != 0) { cwHideAll(...); }`, hiding and disabling all Cancel Queue buttons during soft combat.
Meanwhile, `softBannerTick` hides the vanilla red Cancel Combat bar (`BTN_CLEARALL`, `BTN_CLEARONE`, `BTN_CLEARONE2`) during soft combat (`hide_redbar_soft=1`), leaving the player with neither the vanilla cancel bar nor the mod's Cancel Queue button.




* **Suggested fix:** Use `const bool hardCombat = g_combat != 0 && !g_softCombat;` consistently in `drawAll`, `paintLeader`, `drawStrips`, `onFrameBegin`, and the hooks/tick gates (or leave the vanilla cancel bar visible if `g_cw` is hidden).


* **Confidence:** certain


6. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `QueueHudPerformUseItem`, `QueueHudPerformCastSpell`, and `handoffTick**`
* **Anchor:** `static uintptr_t bufOwner(const qh::ActionBuffer& b) { return b.active ? b.activeAction.creature : b.queue[0].creature; }`

* **What is wrong:** Party slot rotation (`IDENT_SWAP`) is only reconciled inside `tickItemQueues` when `!inMenu && g_combat == 0`. This creates two bugs across party rotation or save/load:


1. If the player rotates party leader (Tab) while paused and clicks an item/spell for the new leader before `onFrameBegin` runs `tickItemQueues`, `QueueHudPerformUseItem` / `QueueHudPerformCastSpell` indexes `g_itemQueue[memberK]` before the buffers are swapped. The new leader's command is pushed into the old leader's busy `g_itemQueue[0]`, and on the next tick `tickItemQueues` swaps `g_itemQueue[0]` (determined solely by `bufOwner(b)` = `activeAction.creature`) to the old leader's new companion slot, carrying the new leader's queued command with it.


2. If combat begins on the same frame as a party rotation (or if a save is loaded while in combat), `tickItemQueues` takes the `if (inMenu || g_combat != 0)` early return and calls `handoffTick` *without* running the identity pre-pass. `handoffTick` then calls `ServerClearAllActions` on slot `k`'s current creature `srv`, but dispatches `a.creature` (which may belong to another slot or be a freed client creature pointer from before a save load that still passes `probeRead(..., 4)`).




* **Suggested fix:** Run the identity reconciliation helper before indexing `g_itemQueue[memberK]` in `QueueHudPerformUseItem`, `QueueHudPerformCastSpell`, and `handoffTick`, and verify `a.creature == cli` (the live client creature pointer from `party::member(tbl, k)`) before calling `dispatchAction`.


* **Confidence:** certain


7. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `tickItemQueues**`
* **Anchor:** `const bool combatNow = g_combat != 0 && !g_combatUnreadable;`

* **What is wrong:** When `g_combatUnreadable` is transiently `true` for one frame while in combat, `combatNow` evaluates to `false` and the loop `for (int k = 0; k < 3; ++k) { edgeFlags[k] = combatNow && !g_prevCombat[k]; g_prevCombat[k] = combatNow; }` overwrites `g_prevCombat[k]` with `false`. On the next frame when the HUD flags are readable again, `combatNow && !g_prevCombat[k]` fires a false combat rising edge in the middle of combat, re-triggering `combatEdgeTrace` and `handoffTick` edge logic (`endHandoff(k, qh::DR_COMBAT_CLEAR)` drops any waiting `headHeld` command!).


* **Suggested fix:** Do not update `g_prevCombat` when `g_combatUnreadable` is true:
```cpp
bool edgeFlags[3] = {false, false, false};
if (!g_combatUnreadable) {
    const bool combatNow = g_combat != 0;
    for (int k = 0; k < 3; ++k) { edgeFlags[k] = combatNow && !g_prevCombat[k]; g_prevCombat[k] = combatNow; }
}

```


* **Confidence:** certain


8. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `cwLookCompanion**`
* **Anchor:** `if (w.fill == want || w.fill == 2) return;`

* **What is wrong:** In `cwLookCompanion`, if `tex::image(...)` returns `nullptr` on the first frame (for instance, before the texture is available or if `guikit::setFill` fails once), `w.fill` is set to `2` (text fallback `"X"`). Because the early return checks `w.fill == 2`, `cwLookCompanion` permanently latches into fallback mode for the lifetime of the HUD panel and never retries `tex::image` even after `tex::heal()` restores the texture.


* **Suggested fix:** Allow retrying `tex::image` when `w.fill == 2` if `tex::image(want ? tex::CancelHi : tex::Cancel)` is now non-null, or only latch text fallback after `embtex` has given up (`MaxHeals`).


* **Confidence:** certain


9. **`patch_manager_mods/queue_hud_src/QueueHud.cpp` — `drawChevron` (and `queue_hud.ini` vs `QueueHudLogic.h` defaults)**
* **Anchor:** `void drawChevron(int x, int y, int w, int h, const float* rgb, float alpha) {`

* **What is wrong:** `drawChevron` is defined inside the anonymous namespace in `QueueHud.cpp` (paired with `qh::chevronRect` in `QueueHudLogic.h`), but `paintLeader` never calls it. Aside from the missing chevron indicator next to Slot 0, defining an unreferenced function in an anonymous namespace triggers `-Wunused-function`, which fails builds whenever `-Wall -Werror` is enabled. Additionally, `queue_hud.ini` documents `;kit = 0` as the default (`kit = 0 (default) skins the queue in dark teal`), whereas `qh::Config` in `QueueHudLogic.h` initializes `bool kit = true;`.


* **Suggested fix:** Either call `drawChevron` in `paintLeader` using `qh::chevronRect(q0)` or remove the dead function, and align `Config::kit` with `queue_hud.ini`.


* **Confidence:** certain


10. **`patch_manager_mods/_shared_src/SafeRead.h` — `saferead::readAt**`
* **Anchor:** `if (base == 0 || !probeRead(site, reinterpret_cast<const void*>(base + off), sizeof(T))) return false;`

* **What is wrong:** On 32-bit builds, if `base` is a corrupted high value (e.g. `0xFFFFF000`) and `off` is a large positive offset such as `PauseInternal` (`0x10078`), `base + off` wraps around 32-bit `uintptr_t` to `0x0000F078` (or $\ge \text{0x10000}$ for `base >= 0xFFFFFEF8`), bypassing `probeRead`'s `a < 0x10000` and `a + len < a` checks and probing an unrelated low memory address.


* **Suggested fix:** Check for pointer-addition overflow in `readAt` before calling `probeRead`:
```cpp
uintptr_t addr = base + static_cast<intptr_t>(off);
if (base == 0 || (off > 0 && addr < base) || (off < 0 && addr > base)) return false;

```


* **Confidence:** certain


11. **`patch_manager_mods/_shared_src/EmbedTex.h` — `embtex::heal**`
* **Anchor:** `if (s.heals == MaxHeals) EMBTEX_LOG("tex %s: giving up after %d failed refills (built=%d missing=%d pixels=%p); art missing, labels still work",`

* **What is wrong:** In `embtex::heal()`, `++s.heals` is incremented *before* `detail::fill` is called. On the 20th refill (`s.heals == MaxHeals`), even when `detail::fill` succeeds and queues the texture for upload on the next engine texture tick, `if (s.heals == MaxHeals)` immediately logs `"giving up after 20 failed refills ... art missing"` in the same call.


* **Suggested fix:** Check the give-up condition on the next tick when `s.heals == MaxHeals` and the texture is still unhealthy (incrementing `s.heals` to `MaxHeals + 1` once logged so it logs only once).


* **Confidence:** certain



##### Files or Areas Not Assessed for `2a_queue_hud_src.txt`

* `patch_manager_mods/_shared_src/GuiKit.h` and `patch_manager_mods/_shared_src/K2Addr.h`: Omitted from the bundle (`manifest-omit`, 0 bytes).


* `patch_manager_mods/_shared_src/HudKitTextures.inc` and `patch_manager_mods/queue_hud_src/override/lbl_miqueue_teal.tga`: Omitted (`over max_file_tokens` / `binary`).


* `kpatch_api.h` and `KpLog.h`: External headers not provided in the bundle.



---

#### `2b_queue_hud_tests.txt`

**Omitted files from `2b_queue_hud_tests.manifest.tsv`:**

* None (all 7 files listed in `2b_queue_hud_tests.manifest.tsv` are present in `2b_queue_hud_tests.txt`).



---

##### Findings for `2b_queue_hud_tests.txt`

1. **`patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp` — `testDecideBranches` (and all other `decideAction` / `decideItemAction` tests)**
* **Anchor:** `KT_CHECK_EQ((int)decideAction(idle, false, 0), (int)DECIDE_DISPATCH_NOW);   // idle + nothing queued in engine`

* **What is wrong:** `qh::decideAction` and `qh::decideItemAction` take a 4th parameter `bool haveCur = false` (`inline ActionDecision decideAction(const ActionBuffer& buf, bool inCombat, int engineQueueLen, bool haveCur = false)`), which forces `DECIDE_BUFFER` when `engineQueueLen == 0` but a 3000 ms round node is still running in `cur`. Not a single call to `decideAction` or `decideItemAction` anywhere in `queue_hud_logic_test.cpp` ever passes `haveCur = true`. If `!haveCur` in `decideAction` were deleted or inverted, or if `decideItemAction` failed to forward `haveCur`, the test suite could not fail.


* **Suggested fix:** Add assertions in `testDecideBranches` testing `haveCur = true` on an otherwise idle buffer:
```cpp
KT_CHECK_EQ((int)decideAction(idle, false, 0, true), (int)DECIDE_BUFFER);
KT_CHECK_EQ((int)decideItemAction(idle, false, 0, true), (int)DECIDE_BUFFER);
KT_CHECK_EQ((int)decideAction(three, false, 0, true), (int)DECIDE_DROP_FULL);
KT_CHECK_EQ((int)decideAction(idle, true, 0, true), (int)DECIDE_DISPATCH_NOW);

```


* **Confidence:** certain


2. **`patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp` — `testFollowerWipeProtection**`
* **Anchor:** `if (nowDone - buf.activeStartTick > 500) {`

* **What is wrong:** In section C of `testFollowerWipeProtection` ("Natural item completion"), the test calls `shouldReInject(buf, false, false, false, nowEarly)` (which takes `buf` by `const ActionBuffer&` and ignores `now` via `(void)now;`) and then asserts `KT_CHECK(buf.active); // still active until 500ms elapsed`. Immediately after, the test itself executes `if (nowDone - buf.activeStartTick > 500) { buf.active = false; }` and asserts `KT_CHECK(!buf.active);`. Both `KT_CHECK` assertions test only the test's own local variable mutations, not any function in `QueueHudLogic.h`, and cannot fail regardless of how `QueueHudLogic.h` is modified.


* **Suggested fix:** Replace the manual `if (nowDone - buf.activeStartTick > 500)` block with calls to `buf.noteView(false, ...)` and `qh::activeVerdict(buf, Ev(false), ...)` at $t \le 500\text{ ms}$ (`AV_PRESENT`) and $t > 500\text{ ms}$ (`AV_UNSEEN` / `AV_EXECUTED`), which is where `kMinActiveMs = 500` actually lives in `QueueHudLogic.h`.


* **Confidence:** certain


3. **`patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp` — `testWhyParityMatrix**`
* **Anchor:** `int reHits[7] = { 0 };`

* **What is wrong:** `ReInjectWhy` has **8** enumerators (`RI_OK` = 0 through `RI_EXECUTED` = 7), and `DispatchWhy` has **7** enumerators (`DS_OK` = 0 through `DS_FLAG_BUSY` = 6). In `testWhyParityMatrix`, `reHits` is sized `int reHits[7]` and `dsHits` is sized `int dsHits[6]`, omitting `RI_EXECUTED` and `DS_FLAG_BUSY` (and starting `rc` at `1` instead of `0`). If `b.sawExecuting` or `flagHold` is varied in the matrix (or if `reInjectWhy` / `dispatchWhy` is mutated to return `RI_EXECUTED` or `DS_FLAG_BUSY` for one of the matrix inputs), `reHits[why]++` / `dsHits[why]++` writes out-of-bounds onto the stack (undefined behavior).


* **Suggested fix:** Size the arrays to `int reHits[8] = { 0 };` and `int dsHits[7] = { 0 };`, include `sawExecuting` (`0..1`) and `rc` (`0..2`) in the `reInjectWhy` loop, and include `flagHold` (`0..1`) in the `dispatchWhy` loop.


* **Confidence:** certain


4. **`patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp` — `testIniLine` and `testConfigDefaultsAndStyle**`
* **Anchor:** `if (detail::keyEq(kb, ke, "waiting_dim_rgb")) return detail::parseRgb(vb, ve, c.waitingBg);`

* **What is wrong:**
1. `parseIniLine` supports the key `"waiting_dim_rgb"` to configure `c.waitingBg`, but no test in `queue_hud_logic_test.cpp` ever passes `"waiting_dim_rgb=..."` to `parseIniLine` or `parseIni`. If the `"waiting_dim_rgb"` branch in `parseIniLine` were deleted or wrote into `c.bg`, no test would fail.


2. In `testConfigDefaultsAndStyle`, when `slotStyle(c, i, true)` is called for `i = 1..3` or with `c.waitingDim = false`, the test checks `s.iconA`, `s.bgA`, and `s.borderA`, but never checks that `s.bg[0..2]` remains `c.bg[0..2]` instead of `c.waitingBg[0..2]`.




* **Suggested fix:** Add valid and invalid `"waiting_dim_rgb"` cases to `testIniLine` and `testIniBad`, and assert `s.bg[0] == c.bg[0] && s.bg[1] == c.bg[1] && s.bg[2] == c.bg[2]` for `slotIndex > 0` and for `c.waitingDim == false` in `testConfigDefaultsAndStyle`.


* **Confidence:** certain


5. **`patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp` — `testChevronRect` and `testStripCancel**`
* **Anchor:** `if (w < 8) w = 8;`

* **What is wrong:** `chevronRect(Rect q0)` clamps `w` to a minimum of `8` and `h` to a minimum of `10` (`if (w < 8) w = 8; if (h < 10) h = 10;`), and rejects `q0.w <= 0 || q0.h <= 0`. `testChevronRect()` only tests `q0` sizes `40x40` (`w = 18, h = 26`), `80x80` (`w = 36, h = 52`), and `0x0`. Because no small `q0` (e.g., `10x10`) or `q0 = { 432, 521, 40, 0 }` is tested, removing or mutating the `w < 8`, `h < 10`, or `q0.h <= 0` guards in `chevronRect` (as well as `icons[0].h <= 0` in `stripCancelRect` and `q0.h <= 0` in `combatBgRect`) cannot fail any test.


* **Suggested fix:** Add small-extent (`Rect{ 100, 100, 10, 10 }` $\to$ `w == 8, h == 10`) and zero-height (`w > 0, h == 0`) checks to `testChevronRect` and `testStripCancel`.


* **Confidence:** certain



##### Files or Areas Not Assessed for `2b_queue_hud_tests.txt`

* All files in `2b_queue_hud_tests.manifest.tsv` were provided and assessed. Note that `test.toml` references a `common_contract_test` target whose source file is not part of this bundle.