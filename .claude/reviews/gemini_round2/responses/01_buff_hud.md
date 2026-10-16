REVIEW buff-duration-hud | files read: 19 | findings: 15

### R2-BH-01 - `iconForEffectIconEffect` reads hardcoded Steam offsets instead of `k2addr::off`
- Class: reachable
- Severity: medium
- Confidence: 92
- Location: patch_manager_mods/_shared_src/EffectIcons.h:31-32 :: k2fx::iconForEffectIconEffect
- Anchor: `if (!rd(eff, Effect_Type, &type) || type != EffType_Icon) return nullptr;`
- Trigger: `RetargetBuffIcon` (`BuffDurationHUD.cpp:208`) or `drawColumnCore` (`BuffDurationHUD.cpp:244`) or `ForceArrowCounters` (`BuffDurationHUD.cpp:410`) -> `k2fx::collectBuffs(..., debuff = true)` (`EffectIcons.h:40`) -> `k2fx::iconForEffectIconEffect(eff, &item)` (`EffectIcons.h:51`) -> `rd(eff, Effect_Type, &type)` / `rd(eff, Effect_IntCount, &n)` / `rd(eff, Effect_Ints, &ip)` (`EffectIcons.h:31-32`), when `k2addr::get()` has `Effect_Type == 0`, `Effect_IntCount == 0`, or `Effect_Ints == 0`.
- Impact: Unlike `k2fx::effectRatio` (`Effects.h:97`) and `k2fx::scanShieldPools` (`Effects.h:116-118`), which query `k2addr::off(&K2AddrTable::Effect_*)` and return early when an offset is `0` (unknown), `iconForEffectIconEffect` uses the hardcoded `constexpr` constants `Effect_Type`, `Effect_IntCount`, and `Effect_Ints` directly, bypassing the `K2AddrTable` zero-means-unknown contract (`Effects.h:14-15`).
- Fix: Read `Effect_Type`, `Effect_IntCount`, and `Effect_Ints` via `k2addr::off(&K2AddrTable::...)` and bail if any is `0`.
- Diff:
```diff
--- a/patch_manager_mods/_shared_src/EffectIcons.h
+++ b/patch_manager_mods/_shared_src/EffectIcons.h
@@ -28,6 +28,8 @@ inline const char* iconForEffectIconEffect(int eff, bool* item) {
     *item = false;
+    const int oType = k2addr::off(&K2AddrTable::Effect_Type), oIntCount = k2addr::off(&K2AddrTable::Effect_IntCount), oInts = k2addr::off(&K2AddrTable::Effect_Ints);
+    if (!oType || !oIntCount || !oInts) return nullptr;
     unsigned short type = 0; int n = 0, ip = 0, row = 0;
-    if (!rd(eff, Effect_Type, &type) || type != EffType_Icon) return nullptr;
-    if (!rd(eff, Effect_IntCount, &n) || n <= 0 || !rd(eff, Effect_Ints, &ip) || !ip || !rd(ip, 0, &row)) return nullptr;
+    if (!rd(eff, oType, &type) || type != EffType_Icon) return nullptr;
+    if (!rd(eff, oIntCount, &n) || n <= 0 || !rd(eff, oInts, &ip) || !ip || !rd(ip, 0, &row)) return nullptr;
     if (row <= 0 || row >= kEffectIconRowCount || kEffectIconRows[row].good != 0 || !kEffectIconRows[row].resref) return nullptr;
```

### R2-BH-02 - `collectBuffs` reads hardcoded `Effect_SpellId` instead of `k2addr::off(&K2AddrTable::Effect_SpellId)`
- Class: reachable
- Severity: medium
- Confidence: 92
- Location: patch_manager_mods/_shared_src/EffectIcons.h:47 :: k2fx::collectBuffs
- Anchor: `if (!rd(arr, i * 4, &eff) || !rd(eff, Effect_SpellId, &spell)) continue;`
- Trigger: `RetargetBuffIcon` (`BuffDurationHUD.cpp:208`), `drawColumnCore` (`BuffDurationHUD.cpp:244-245`), or `ForceArrowCounters` (`BuffDurationHUD.cpp:409-410`) -> `k2fx::collectBuffs(...)` (`EffectIcons.h:40`) -> `rd(eff, Effect_SpellId, &spell)` (`EffectIcons.h:47`), when `K2AddrTable::Effect_SpellId` is `0` while `Creature_EffectArray` and `Creature_EffectCount` are non-zero.
- Impact: `collectBuffs` ignores `K2AddrTable::Effect_SpellId` and reads offset `0x1c` unconditionally, violating the `Effects.h:14-15` contract ("every helper reads the K2Addr.h table selected for the running exe (k2addr::get()) and does nothing / returns its neutral value when the table is null or a field it needs is 0").
- Fix: Query `const int oSpell = k2addr::off(&K2AddrTable::Effect_SpellId);` at the top of `collectBuffs`, return `0` if `!oSpell`, and pass `oSpell` to `rd(eff, oSpell, &spell)`.
- Diff:
```diff
--- a/patch_manager_mods/_shared_src/EffectIcons.h
+++ b/patch_manager_mods/_shared_src/EffectIcons.h
@@ -41,7 +41,8 @@ inline int collectBuffs(uintptr_t creature, int day, int ms, bool haveTime, Buff
     int arr = 0, count = 0;
-    if (!effectListNonEmpty(creature, &arr, &count)) return 0;
+    const int oSpell = k2addr::off(&K2AddrTable::Effect_SpellId);
+    if (!oSpell || !effectListNonEmpty(creature, &arr, &count)) return 0;
     int seen[MaxEffects]; int nSeen = 0; int n = 0;
     const char* iconSeen[MaxEffects]; int nIcon = 0;   // icons added from type-0x43 rows (debuff only)
     for (int i = 0; i < count && n < maxOut; ++i) {
         int eff, spell;
-        if (!rd(arr, i * 4, &eff) || !rd(eff, Effect_SpellId, &spell)) continue;
+        if (!rd(arr, i * 4, &eff) || !rd(eff, oSpell, &spell)) continue;
         bool item = false;
```

### R2-BH-03 - `VisSlot` per-column state (`drawn`, `tracked`, `rc`) is not cleared on `visReject` or zero-buff reset
- Class: reachable
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp:138-151 :: visColumn
- Anchor: `const int total = v.drawn[0] + v.drawn[1], trk = v.tracked[0] + v.tracked[1];`
- Trigger: Frame N: creature has both a buff (`drawn[0]=1, tracked[0]=1`) and a debuff (`drawn[1]=1, tracked[1]=1`); `DrawBuffColumn` (`BuffDurationHUD.cpp:324`) -> `drawColumnCore` (`BuffDurationHUD.cpp:268`) -> `visColumn` sets `v.drawn={1,1}`. Frame N+1: all debuffs expire (`rawDebuff=0, nDebuff=0`), while 1 buff remains (`rawBuff=1, nBuff=1`). `ForceArrowCounters` (`BuffDurationHUD.cpp:390`) leaves `Slot_ArrowDebuff` at `0`, so the engine only calls `PortraitIconRowDraw` for the buff row (`whichRow != 0`). `drawColumnCore` (`BuffDurationHUD.cpp:268`) -> `visColumn(slotPtr, false, 1, 1, vrp)` updates `v.drawn[0]=1, v.tracked[0]=1`, then sums `v.drawn[0] + v.drawn[1]` using the stale `v.drawn[1]=1` and `v.rc[1]` from Frame N. Similarly, if `visReject` (`BuffDurationHUD.cpp:129`) or the zero-buff reset (`BuffDurationHUD.cpp:415`) runs, `v.drawn` and `v.tracked` are not zeroed before the next single-row `visColumn` call.
- Impact: `VIS slot%d` diagnostic log lines report stale `count` and bounding `rect` values (merging the expired column's old rectangle and icon count) whenever one of the two rows drops to 0 arrows while the other row continues drawing.
- Fix: Clear `v.drawn`, `v.tracked`, and `v.rc` inside `visSet` whenever `cat != VisDrawn`, and in `ForceArrowCounters` clear `v.drawn[0]/tracked[0]` when `rawBuff == 0 && nBuff == 0` and `v.drawn[1]/tracked[1]` when `rawDebuff == 0 && nDebuff == 0`.

### R2-BH-04 - Fixed-size slot tables (`g_vis`, `g_arrowRaw`, `colGate`, `g_missSeen`) never evict stale pointers across area loads or save loads
- Class: reachable
- Severity: low
- Confidence: 90
- Location: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp:102-108 :: visIndex
- Anchor: `for (int i = 0; i < 4; ++i) if (g_vis[i].slotPtr == 0) { g_vis[i] = {}; g_vis[i].slotPtr = slotPtr; return i; }`
- Trigger: During normal play across area transitions, module reloads, or `CSWGuiInGameMenu` / `CSWGuiMainInterface` recreation:
  1. `ForceArrowCounters` (`BuffDurationHUD.cpp:391`) or `drawColumnCore` (`BuffDurationHUD.cpp:268`) -> `visIndex(slotPtr)` (`BuffDurationHUD.cpp:103`): `g_vis[4]` has only 4 entries (`K2PS_MAX_SLOTS`). The 4 HUD slots from the first `CSWGuiMainInterface` instance fill all 4 slots; if `CSWGuiMainInterface` is recreated at a different heap address, `visIndex` returns `-1` permanently for all new slots.
  2. `ForceArrowCounters` (`BuffDurationHUD.cpp:400`) -> `arrowRawFor(slotPtr)` (`BuffHudLogic.h:69`): `g_arrowRaw[8]` fills after 2 HUD instances (4 slots each), returning `nullptr` and suppressing `ARROWS`/`ICONCACHE` change logs thereafter.
  3. `drawColumnCore` (`BuffDurationHUD.cpp:255`) -> `colGate.shouldLog(slotPtr, ...)` (`BuffHudLogic.h:170`): `ColumnLogGate` holds 16 `(slot, debuff)` pairs (4 HUD slots + 2 menu companion controls = 12 keys per UI instance); on the second UI recreation, new keys exceed 16 and never log `COLUMN#` changes.
  4. `ForceArrowCounters` (`BuffDurationHUD.cpp:411`) -> `logUntrackedDebuff` (`BuffDurationHUD.cpp:355`): `g_missSeen[16]` fills after 16 distinct creature pointers across areas and stops logging `DBGMISS`.
- Impact: Diagnostic change-detection tables silently stop logging for new HUD/menu/creature instances after UI or area reloads in a long session (or `g_vis` on the very first HUD address change).
- Fix: Evict the oldest entry (round-robin or LRU cursor) when a table is full instead of returning `-1` / `nullptr` / `false`.

### R2-BH-05 - `onFrameBegin` skips `buffhud_off.txt` polling if only non-0 mod-30 frames reach the callback
- Class: reachable
- Severity: low
- Confidence: 82
- Location: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp:461 :: onFrameBegin
- Anchor: `if (g_frame % 30 == 0) {`
- Trigger: `onFrameBegin` (`BuffDurationHUD.cpp:458`) checks `g_frame % 30 == 0`, where `g_frame` is `int(c->frame)` from the `portrait-slots` provider. If `K2PS_FRAME_BEGIN` is not invoked on an exact multiple of 30 (for example, while in a state where the leader slot gate is skipped on that specific tick), or before the first multiple of 30 after late connection, `g_noBuff` is not updated.
- Impact: If `buffhud_off.txt` is present at game startup and the first rendered frame has `c->frame == 1`, frames `1..29` draw the buff HUD before `g_frame == 30` polls `buffhud_off.txt`.
- Fix: Track `static int lastPollFrame = -30;` and poll when `lastPollFrame < 0 || g_frame - lastPollFrame >= 30 || g_frame < lastPollFrame`.
- Diff:
```diff
--- a/patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp
+++ b/patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp
@@ -460,3 +460,5 @@ static void KPATCH_CALL onFrameBegin(const K2PsContext* c, void*) {
     flushConnectNote();
-    if (g_frame % 30 == 0) {
+    static int lastPoll = -30;
+    if (lastPoll < 0 || g_frame - lastPoll >= 30 || g_frame < lastPoll) {
+        lastPoll = g_frame;
         g_noBuff = fileExists("buffhud_off.txt");
```

### R2-BH-06 - `drawColumnCore` passes `border + T->Border_Tint` to `gui2d::drawImage` without probing readability of the 3 tint floats
- Class: latent
- Severity: medium
- Confidence: 94
- Location: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp:276 :: drawColumnCore
- Anchor: `const float* color = border ? reinterpret_cast<const float*>(border + T->Border_Tint) : whiteTint;`
- Trigger: `DrawBuffColumn` (`BuffDurationHUD.cpp:315`) -> `drawColumnCore` (`BuffDurationHUD.cpp:236`) -> `border = iconWidget + T->Label_Border` (`BuffDurationHUD.cpp:273`) -> `color = reinterpret_cast<const float*>(border + T->Border_Tint)` (`BuffDurationHUD.cpp:276`) -> `draw2` (`BuffDurationHUD.cpp:280`) -> `gui2d::drawImage` (`BuffDurationHUD.cpp:281`). When `RetargetBuffIcon` did not run for this widget (or failed its `BorderVtable` check at `BuffDurationHUD.cpp:216`, which only logs and returns without aborting the row draw), `DrawBuffColumn` still runs at row exit. In `drawColumnCore`, `readAt(border, T->Border_Rot, &rotBits)` and `readAt(border, T->Border_Alpha, &baseAlpha)` have their return values ignored, and `color` is formed as a raw pointer `border + T->Border_Tint` (`+0x28..+0x34`) without checking `readAt` or `BorderVtable`. When the engine's `ImageVt_Draw` dereferences `color[0..2]`, an invalid `iconWidget` pointer faults.
- Impact: Potential access violation inside `ImageVt_Draw` if `iconWidget` is corrupt or not a valid `CSWGuiLabel` with a readable `CSWGuiBorder` at `+0x60`.
- Fix: Read the 3 tint floats into a local `float tint[3] = { 1.0f, 1.0f, 1.0f };` via `readAt` (and optionally verify `BorderVtable` as `RetargetBuffIcon` does) instead of passing a raw interior pointer into engine memory.
- Diff:
```diff
--- a/patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp
+++ b/patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp
@@ -274,4 +274,4 @@ static void drawColumnCore(int slotPtr, int creature, int iconWidget, int isLarg
-    static const float whiteTint[3] = { 1.0f, 1.0f, 1.0f };
+    float color[3] = { 1.0f, 1.0f, 1.0f };
     int rotBits = 0; if (border) readAt(border, T->Border_Rot, &rotBits);
-    const float* color = border ? reinterpret_cast<const float*>(border + T->Border_Tint) : whiteTint;
+    if (border) readAt(border, T->Border_Tint, &color);
     float baseAlpha = 1.0f; if (border) readAt(border, T->Border_Alpha, &baseAlpha);
```

### R2-BH-07 - Direct memory writes to `border` and `slotPtr` bypass `saferead::probeWrite`
- Class: latent
- Severity: medium
- Confidence: 85
- Location: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp:221-222 :: RetargetBuffIcon
- Anchor: `*reinterpret_cast<int*>(border + T->Border_Width) = 0;`
- Trigger:
  1. `RetargetBuffIcon` (`BuffDurationHUD.cpp:221-222`) and `RestoreBuffIcon` (`BuffDurationHUD.cpp:228-229`) write directly to `*reinterpret_cast<int*>(border + T->Border_Width)` and `Border_Height` after only `readAt` (which checks `readableProtect`, including `PAGE_READONLY`).
  2. `ForceArrowCounters` (`BuffDurationHUD.cpp:419-420`) writes directly to `*reinterpret_cast<unsigned char*>(slotPtr + T->Slot_ArrowBuff)` and `Slot_ArrowDebuff` after only `readAt`.
  Furthermore, `RestoreBuffIcon` (`BuffDurationHUD.cpp:228-229`) writes to `g_saved.border` in a later hook call (`RestoreBuffIcon` at `0x00745a97`, or `RetargetBuffIcon` / `DrawBuffColumn` after `saferead::beginScope()`) without re-probing `g_saved.border`.
- Impact: If a bogus `slotPtr` or `iconWidget` points into a read-only page (such as `.rdata` where a constant matches `BorderVtable` or `Slot_Flags` bit 1), `readAt` succeeds and the unprobed store raises an access violation.
- Fix: Probe writable memory with `saferead::probeWrite` before writing to `border` or `slotPtr` (and in `RestoreBuffIcon`).

### R2-BH-08 - `remainingRatio` and `brightPx` do not guard against `NaN` / `Inf` float durations
- Class: latent
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/_shared_src/Effects.h:88-92 :: k2fx::remainingRatio
- Anchor: `float ratio = float(remaining / (double(duration) * 1000.0));`
- Trigger: `RetargetBuffIcon` / `drawColumnCore` / `ForceArrowCounters` -> `k2fx::collectBuffs` (`EffectIcons.h:61`) -> `k2fx::effectRatio` (`Effects.h:98-100`): if a corrupt `CGameEffect` in memory has `duration = +INFINITY` (or `duration` very small / subnormal where `remaining` and `duration` produce `NaN`, or `brightPx` in `BuffHudLogic.h:137` is called directly with `ratio = NAN`):
  - In `remainingRatio` (`Effects.h:90-91`), `if (ratio > 1.0f)` and `if (ratio < minRatio)` are both `false` when `ratio` is `NaN`, returning `NaN`.
  - In `bh::brightPx` (`BuffHudLogic.h:138-139`), `int(size * NAN + 0.5f)` is undefined behaviour in C++ (on x86 SSE `cvttss2si` yields `INT_MIN = -2147483648`), and for large negative `ratio` (e.g. `-1e10f`), `int(size * ratio + 0.5f)` also overflows `int` UB.
- Impact: Undefined float-to-int conversion in `brightPx` or unclamped `NaN` in `Buff::ratio` (unlike `gui2d::vfillClip` at `Gui2D.h:144`, which explicitly writes `if (!(frac > 0.0f)) frac = 0.0f; // also NaN`).
- Fix: In `remainingRatio`, replace `if (ratio < minRatio)` with `if (!(ratio >= minRatio)) ratio = minRatio;`. In `bh::brightPx`, guard `if (!(ratio > 0.0f)) return 0; if (ratio >= 1.0f) return size;` before multiplying and casting to `int`.
- Diff:
```diff
--- a/patch_manager_mods/buff_duration_hud_src/BuffHudLogic.h
+++ b/patch_manager_mods/buff_duration_hud_src/BuffHudLogic.h
@@ -137,4 +137,6 @@ inline IconPos iconPos(const ColumnLayout& l, int i, bool debuff, int cardW) {
 inline int brightPx(int size, float ratio) {
+    if (!(ratio > 0.0f) || size <= 0) return 0;
+    if (ratio >= 1.0f) return size;
     int bright = int(size * ratio + 0.5f);
     if (bright > size) bright = size;
     return bright;
```

### R2-BH-09 - `logUntrackedDebuff` reads hardcoded `Effect_*` offsets instead of `k2addr::off`
- Class: latent
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp:350 :: logUntrackedDebuff
- Anchor: `if (readAt(arr, i * 4, &eff) && eff && readAt(eff, k2fx::Effect_SpellId, &spell)) sig = sig * 31 + spell;`
- Trigger: `onPreEngineRead` (`BuffDurationHUD.cpp:453`) -> `ForceArrowCounters` (`BuffDurationHUD.cpp:411`) -> `logUntrackedDebuff` (`BuffDurationHUD.cpp:344`): lines 350, 367-369, and 379 read `k2fx::Effect_SpellId`, `k2fx::Effect_Type`, `k2fx::Effect_Subtype`, `k2fx::Effect_IntCount`, `k2fx::Effect_Ints`, `k2fx::Effect_Duration`, and `k2fx::Effect_Creator` directly instead of checking `T->Effect_*` from `k2addr::get()`.
- Impact: If an exe table has `arrowFieldsOk(T)` true and `Creature_EffectArray`/`Creature_EffectCount` set, but any of the `Effect_*` offsets in `K2AddrTable` is `0` (unknown) or differs from Steam, `logUntrackedDebuff` reads hardcoded Steam offsets instead of skipping the diagnostic.
- Fix: Gate `logUntrackedDebuff` on the needed `T->Effect_*` fields being non-zero and use `T->Effect_*`.

### R2-BH-10 - `common_contract_test.cpp` does not test `EffectIconRowTable.inc` or `iconForEffectIconEffect` (type-`0x43` debuffs)
- Class: test-gap
- Severity: medium
- Confidence: 98
- Location: patch_manager_mods/buff_duration_hud_src/common_contract_test.cpp:574-577 :: effects_contract
- Anchor: `for (int i = 0; i < 287; ++i) {`
- Trigger: `common_contract_test.cpp` (`effects_contract`, lines 496-584) tests `collectBuffs` with spell IDs from `kEffectIcons` and `kDebuffIcons`, and validates resref length (`<= 16`), lowercase, and 256-entry pool capacity across `kEffectIcons` and `kDebuffIcons` only. It never tests `kEffectIconRows` (`EffectIconRowTable.inc`, 120 rows) or `iconForEffectIconEffect` (`EffectIcons.h:28-37`), nor the `viaIcon` deduplication path in `collectBuffs` (`EffectIcons.h:50-60`) when a creature has a type-`0x43` (`EffType_Icon`) effect with `spell == -1` (such as Slow) or when both a spell-mapped debuff and a type-`0x43` effect resolve to the same icon resref.
- Impact: Regressions in `iconForEffectIconEffect`, `EffectIconRowTable.inc` (e.g., an uppercase or `> 16`-char resref, or pool overflow when combined with `kEffectIconRows` entries like `"ii_trapkit_001"` at row 63 which is not in `kEffectIcons`/`kDebuffIcons`), or `viaIcon` deduplication in `collectBuffs` will not be caught by `common_contract_test`.
- Fix: Add contract checks in `effects_contract()` covering: (1) `collectBuffs` on a fake creature with a type-`0x43` effect (`Effect_SpellId = -1`, `Effect_Type = 0x43`, `ints[0] = 28` -> `"ip_slow"`), (2) deduplication between two type-`0x43` effects and between a spell debuff and a type-`0x43` effect with the same icon, (3) `good != 0` and out-of-range row rejection, and (4) including `kEffectIconRows` (`good == 0`) in the `distinct` resref set, `maxLen <= 16`, and lowercase checks.

### R2-BH-11 - `testColumnLayout` and `testMenuGates` contain tautological `l.size >= 0 && l.size >= 1` assertions
- Class: test-gap
- Severity: low
- Confidence: 100
- Location: patch_manager_mods/buff_duration_hud_src/buff_hud_logic_test.cpp:277 :: testColumnLayout
- Anchor: `bool ok = l.gap >= 1 && l.size >= 0 && l.size >= 1 && l.cols >= 1 && l.cols <= bh::MaxColumns && l.perCol >= 1`
- Trigger: `testColumnLayout` (`buff_hud_logic_test.cpp:277`) and `testMenuGates` (`buff_hud_logic_test.cpp:451`: `KT_CHECK(l.size >= 0 && l.size >= 1 && l.perCol >= 1); // w of a few px can give size 0 (see testColumnLayout)`).
- Impact: `l.size >= 0` is redundant with `l.size >= 1` (left over from when F22 allowed `size == 0` and the test was updated in place), and the comment on line 451 is stale and contradicts the assertion on the same line.
- Fix: Remove the redundant `l.size >= 0 &&` and update the comment at line 451.
- Diff:
```diff
--- a/patch_manager_mods/buff_duration_hud_src/buff_hud_logic_test.cpp
+++ b/patch_manager_mods/buff_duration_hud_src/buff_hud_logic_test.cpp
@@ -277,2 +277,2 @@ static void testColumnLayout() {
-        bool ok = l.gap >= 1 && l.size >= 0 && l.size >= 1 && l.cols >= 1 && l.cols <= bh::MaxColumns && l.perCol >= 1
+        bool ok = l.gap >= 1 && l.size >= 1 && l.cols >= 1 && l.cols <= bh::MaxColumns && l.perCol >= 1
                && l.fit == l.perCol * l.cols && l.drawN >= 0 && l.drawN <= n && l.drawN <= l.fit && (l.drawN == n || l.drawN == l.fit);
@@ -451,2 +451,2 @@ static void testMenuGates() {
         bh::ColumnLayout l = bh::columnLayout(4, 2, 0, w, h, 2.0f);
-        KT_CHECK(l.size >= 0 && l.size >= 1 && l.perCol >= 1);   // w of a few px can give size 0 (see testColumnLayout)
+        KT_CHECK(l.size >= 1 && l.perCol >= 1);
     }
```

### R2-BH-12 - `02_buff_duration_hud.md` states force-power icons get a 1px boundary line, but code only draws it for item abilities (`disc != nullptr`)
- Class: doc
- Severity: low
- Confidence: 98
- Location: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp:303 :: drawColumnCore
- Anchor: `if (disc && bright < size) {                                                     // 1px boundary line (round item icons only)`
- Trigger: `02_buff_duration_hud.md:1671` ("FINAL IMPLEMENTATION") states: "with a 1 px dark-teal line at the boundary (on item icons the line is clipped to the round disc: width = the circle's chord at that height, hidden outside it; force-power icons keep a full-width line)." In `BuffDurationHUD.cpp:303`, the boundary line is gated by `if (disc && bright < size)`, where `disc` is non-null only when `buffs[i].item` is true (`BuffDurationHUD.cpp:292`), so force-power icons (`buffs[i].item == false`) never get a boundary line.
- Impact: A maintainer reading `02_buff_duration_hud.md:1671` is misled into thinking force-power icons draw a full-width 1px boundary line (or that the missing force-power line in `drawColumnCore` is an accidental regression).
- Fix: Update `02_buff_duration_hud.md:1671` to match the code (only round item-ability discs draw the 1px boundary line), or if a full-width line on force-power icons was intended, add `else if (!buffs[i].item && bright < size)` in `drawColumnCore`.

### R2-BH-13 - `EffectIconTable.inc` maps droid shield row 115 (`DROID_ITEM_ENERGY_SHIELD_6`) to `ii_drdshld_005` instead of `ii_drdshld_008`
- Class: doc
- Severity: low
- Confidence: 80
- Location: patch_manager_mods/_shared_src/EffectIconTable.inc:120 :: kEffectIcons
- Anchor: `/*115*/ "ii_drdshld_005",`
- Trigger: `k2fx::collectBuffs` (`EffectIcons.h:49`) -> `k2fx::iconForSpell(115, false, &item)` (`EffectIcons.h:22`) -> `kEffectIcons[115]` (`EffectIconTable.inc:120`).
- Impact: Spells 112 (`DROID_ITEM_ENERGY_SHIELD_3`, line 117) and 115 (`DROID_ITEM_ENERGY_SHIELD_6`, line 120) both return `"ii_drdshld_005"`. While `02_buff_duration_hud.md:1690` and `:1704` note that `114/115` use best-guess icons, if a droid has both shield effects or a player activates row 115, it displays the same icon as row 112 (and if `collectBuffs` ever deduplicated by icon or a type-`0x43` effect was present, they would collide).
- Fix: Verify against the game's `d_shield_06.uti` / `ii_drdshld_*` textures whether a distinct variation texture exists, or document the intentional reuse in `EffectIconTable.inc`'s generator script.

### R2-BH-14 - `EffectIcons.h` header comment omits `EffectIconRowTable.inc` and says only two tables are generated
- Class: doc
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/_shared_src/EffectIcons.h:2-3 :: k2fx
- Anchor: `// Header-only. The two tables are GENERATED (scripts/gen_buff_hud_icon_table.py, scripts/gen_debuff_icon_table.py); they were`
- Trigger: Reading `EffectIcons.h:2-4` or `21_portrait_slots_and_split.md:41` (`EffectIcons.h + EffectIconTable.inc / DebuffIconTable.inc: iconForSpell, collectBuffs`).
- Impact: `EffectIcons.h` actually includes three generated `.inc` tables (`EffectIconTable.inc`, `DebuffIconTable.inc`, and `EffectIconRowTable.inc` at line 9) and also exports `iconForEffectIconEffect` (`EffectIcons.h:28`). In addition, line 4 says `Include as "../_shared/EffectIcons.h"`, which contradicts `02_buff_duration_hud.md:1747` (`New code includes "SafeRead.h", not "../_shared/SafeRead.h"`).
- Fix: Update the top comment in `EffectIcons.h` (and `Effects.h:3`, `Gui2D.h:2-3`, `GuiKit.h:5`) to reference `"EffectIcons.h"` and mention `EffectIconRowTable.inc` / `iconForEffectIconEffect`.

### R2-BH-15 - `BuffDurationHUD.cpp` top comment and `manifest.toml` version mismatch (`v2.0.0` / `2.0.1` in docs vs `0.2.3` in manifest)
- Class: doc
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp:7 :: top comment
- Anchor: `// v2.0.0 (doc 21 W4): the shield bar is its own patch (shield-bar) and the shared portrait hooks (UpdatePortraits slot table, the`
- Trigger: Compare `manifest.toml:4` (`version = "0.2.3"`) and `manifest.toml:6` (`since 2.0.0`) with `BuffDurationHUD.cpp:7` (`v2.0.0`) and `21_portrait_slots_and_split.md:23-24` (`Buff 2.0.1`).
- Impact: The version numbering in `manifest.toml` (`0.2.3`) disagrees with `manifest.toml:6` (`since 2.0.0`), `BuffDurationHUD.cpp:7` (`v2.0.0`), and `21_portrait_slots_and_split.md` (`2.0.0` / `2.0.1`), making it unclear in logs and staging reports which release is installed.
- Fix: Align the version references in `BuffDurationHUD.cpp` and `manifest.toml` description with `manifest.toml`'s `0.2.3` (or bump `manifest.toml` if `2.0.x` was intended).

## Retracted
- Untracked debuff arrows left at 0x0 size when `RetargetBuffIcon` returns early on `index >= n`: dropped because `RetargetBuffIcon` unconditionally calls `RestoreBuffIcon()` at line 198 before any early return, and `DrawBuffColumn` calls `RestoreBuffIcon()` at line 319 after the loop.
- `columnLayout` division by zero when `size + gap == 0` or `perCol == 0`: dropped because `cardW` and `cardH` are clamped to `>= 1` at lines 101-102, `gap` is clamped to `>= 1` at line 104, `size` is clamped to `>= 1` at line 119, and `perCol` is clamped to `>= 1` at lines 111 and 120.
- `ColumnLogGate` log spam from alternating buff and debuff column calls on the same slot: dropped because `ColumnLogGate::shouldLog` keys on both `(slotPtr, debuffFlag)` (`BuffHudLogic.h:172`).
- `Gui2D` clip stack imbalance in `drawColumnCore` or `onMenuOverlay`: dropped because every `gui2d::pushClip` call (`BuffDurationHUD.cpp:298`, `307`, `487`) is paired with `gui2d::popClip()` inside the `if (ok)` branch with no early return or `continue` in between, and `CapGuard` (`BuffDurationHUD.cpp:272`) restores GL capabilities on scope exit.
- `bh::appendf` buffer overrun in `ForceArrowCounters` (`ICONCACHE` loop, `BuffDurationHUD.cpp:438`): dropped because `strfmt::appendf` (`StrFmt.h:11-20`) clamps `len` to `[0, cap - 1]` and always NUL-terminates.
- Same icon drawn twice when a debuff has both a mapped `Effect_SpellId` and a sibling type-`0x43` `EffType_Icon` effect: dropped because `collectBuffs` (`EffectIcons.h:54-58`) checks `out[0..n-1].resref` when `viaIcon` is true (catching earlier spell-mapped or icon-mapped entries) and checks `iconSeen[0..nIcon-1]` when `viaIcon` is false (catching earlier icon-mapped entries).

## Not assessed
- `KpLog.h` and `kpatch_api.h`: located in the external KPM repository (`$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common`), outside this workspace and out of the listed `_shared_src` scope.
- Provider implementation of `k2.portraitslots` (`portrait_slots_src/`): out of scope for this patch review; judged only through `PortraitSlots.h` and `BuffDurationHUD.cpp`'s consumer calls.
- Live engine values of `slot+0xddc`/`+0xddd` above 1 when `bumpArrowCounters` raises `0 -> 1` while the engine later updates the slot in the same frame: `ForceArrowCounters` runs in `K2PS_PRE_ENGINE_READ` immediately before `FUN_00745770` reads the counters; whether any other engine path reads the bumped `1` after `FUN_00745770` returns needs reverse engineering (though `UpdatePortraits` overwrites both bytes on the next frame).
