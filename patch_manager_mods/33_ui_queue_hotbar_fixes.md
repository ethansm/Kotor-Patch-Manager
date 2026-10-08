# 33 — UI, Queue, and Hotbar Polish Implementation Plan

Detailed engineering implementation plan for combat hotbar styling, companion popover text fix, and queue-hud dark teal UX overhaul with multi-item buffering.

---

## Plan Overview & Combined Workstreams

```
┌───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                       UI & QUEUE POLISH WORKSTREAMS                                               │
├────────────────────┬─────────────────────────────┬────────────────────────────────────────────────────────────────┤
│ Workstream         │ Scope / Affected Files      │ Key Deliverables                                               │
├────────────────────┼─────────────────────────────┼────────────────────────────────────────────────────────────────┤
│ 1. Combat Hotbar   │ `combat_hotbar_src/`        │ Red theme tint `{0.85, 0.15, 0.15}`, increased padding/gaps   │
│ 2. Popover Text    │ `companion_selfheal_src/`   │ Popover text renderer fix, Exile follower empty tag fix       │
│ 3. Queue HUD UX    │ `queue_hud_src/`            │ Combat queue geometry parity, dark teal theme, 38px icons      │
│ 4. Queue Buffer    │ `queue_hud_src/`            │ Multi-item out-of-combat queue buffer, follower wipe guard     │
└────────────────────┴─────────────────────────────┴────────────────────────────────────────────────────────────────┘
```

---

## Workstream 1: Combat Hotbar Red Styling & Padding (`combat-hotbar`)

### 1.1 Objective & Code Targets
- **Target Files:**
  - `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp`
  - `patch_manager_mods/combat_hotbar_src/CombatLogic.h`
  - `patch_manager_mods/combat_hotbar_src/combat_hotbar.ini`

### 1.2 Snippets & Logic
In `CombatHotbar.cpp:607`, replace the green/teal fallback tint with the native combat red tint:
```cpp
// Target container & button border tint: Red theme (matching native combat UI)
for (int g = 0; g < 3; ++g) {
    float tint[3] = { 0.85f, 0.15f, 0.15f }; float alp = 0.95f;
    const uintptr_t btnCtl = menu + GroupBase + (uintptr_t)g * GroupStride;
    tOk[g] = guikit::getBorderTint(btnCtl, tint[g]) && (tint[g][0] > 0.01f || tint[g][1] > 0.01f || tint[g][2] > 0.01f);
    aOk[g] = guikit::getAlpha(btnCtl, &alp) && alp >= 0.05f && alp <= 1.0f;
    // Force combat red theme for replace=1 button containers
    tint[g][0] = 0.85f; tint[g][1] = 0.15f; tint[g][2] = 0.15f;
```

Update spacing and padding defaults in `CombatLogic.h`:
```cpp
// Sizing defaults: increased padding and inter-button gaps
int pad = 6;         // was 4 (inside pill margin)
int groupGap = 8;    // was 4 (gap between group pills)
int sidePad = 10;    // was 6 (side pill padding)
int capPad = 6;      // was 4 (cap padding)
```

### 1.3 Verification
- Run tests: `python3 scripts/run_tests.py -k combat`
- Build check: `python3 scripts/build_check.py --mod combat_hotbar_src`

---

## Workstream 2: Companion Popover Text & Empty Tag (`companion-selfheal`)

### 2.1 Objective & Code Targets
- **Target Files:**
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp`
  - `patch_manager_mods/_shared_src/SelfHealUi.h`

### 2.2 Root Cause & Fix
1. **Invisible Popover Text:**
   `LBL_QUEUE1` in `mipc28x6_p.gui` has a null text renderer when created without template text. `guikit::setText` and `guikit::setTextColor` fail `textObjOk(ctl)` silently.
   Fix: In `buildWidgets`, initialize text controls with placeholder text `"-"` during template creation to force the engine to allocate `CSWGuiTextParams` and its renderer, or use `LBL_HOTBARPTITLE` / template label that already has an active text renderer.
   Ensure popover labels are appended strictly after `W_PopBg` and set text alpha explicitly.
2. **Exile Empty Tag Warning:**
   When player controls a companion, the Exile becomes member 1 or 2 with tag `""`.
   In `readCreatureTag`:
   ```cpp
   if (cr && (!str || len == 0)) {
       // Recognize the player character (empty tag)
       strncpy(out, "player", cap);
       return true;
   }
   ```

### 2.3 Verification
- Run tests: `python3 scripts/run_tests.py -k selfheal`
- Build check: `python3 scripts/build_check.py --mod companion_selfheal_src`

---

## Workstream 3: Queue HUD Layout, Dark Teal Styling & 38px Strips (`queue-hud`)

### 3.1 Objective & Code Targets
- **Target Files:**
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp`
  - `patch_manager_mods/queue_hud_src/QueueHudLogic.h`
  - `patch_manager_mods/queue_hud_src/queue_hud.ini`

### 3.2 Snippets & Logic
1. **Dark Teal Palette:**
   ```cpp
   // Dark teal palette matching custom GUI theme
   constexpr float kTealBg[3]     = { 0.05f, 0.25f, 0.28f }; // background
   constexpr float kTealBorder[3] = { 0.10f, 0.58f, 0.62f }; // border highlight
   constexpr float kTealDim[3]    = { 0.03f, 0.15f, 0.18f }; // waiting / dimmed
   ```
2. **Combat Layout Parity:**
   Replicate the native combat queue shape:
   - Left pointer chevron (`gui_q_arrow` or 2D polygon).
   - Rounded slot frame for slots 0..3.
   - Wide bottom "CANCEL" bar button positioned beneath Slot 0 (replacing the compact square Cancel label).
3. **Companion Strip Sizing:**
   In `QueueHudLogic.h`, revert auto icon size from 24px back to `vit.h * 0.40f` (~38px):
   ```cpp
   inline int autoIconPx(int vitH) {
       int px = (vitH * 40) / 100;
       return px < 32 ? 32 : (px > 48 ? 48 : px); // clamp 32..48 px
   }
   ```

---

## Workstream 4: Multi-Item Non-Combat Queue Buffer & Follower Wipe Guard (`queue-hud`)

### 4.1 Objective & Architecture
- In non-combat exploration, native `PerformUseItem` executes immediately. Queuing multiple items while paused causes drops.
- Follower AI (`AIActionFollowLeader` 0x3D) and heartbeat scripts periodically call `ClearAllActions` (`0x541080`), discarding items.

### 4.2 Implementation Design
1. **Queued Item Buffer (FIFO):**
   ```cpp
   struct QueuedItemAction {
       uintptr_t creature;
       uint32_t clientItem;
       uint32_t targetId;
       DWORD queueTime;
   };
   std::vector<QueuedItemAction> g_itemQueue[3]; // per party member
   ```
2. **Sequential Dispatcher:**
   - When player clicks an item out of combat while another action is active, buffer it.
   - On frame tick, when `creature->queueList` empties or creature animation ends, dispatch the next buffered item via `PerformUseItem`.
3. **Follower Wipe Protection:**
   - Track pending item count in `QueueHud`.
   - If `creature->queueList` drops an item action before HP changes or stack count decrements, re-inject the action to ensure it is not eaten by follower follow resets.

---

## Workstream 5: Action Hotbar Reset Prevention (`action-hotbar`)

### 5.1 Objective & Code Targets
- **Target Files:**
  - `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp`
  - `patch_manager_mods/action_hotbar_src/HotbarLogic.h`
  - `patch_manager_mods/action_hotbar_src/HotbarConfig.h`
  - `patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp`

### 5.2 Root Causes & Implementations
1. **AH1: Absent Snapshot Wiping Live INI:**
   - In `restoreFromSave`, when `pick.src == hb::SS_Absent`, `ActionHotbar.cpp` previously invoked `replaceLiveProfiles(nullptr, ...)` which erased all active profile bindings in `action_hotbar.ini`.
   - Fix: Disallowed `replaceLiveProfiles(nullptr)` on `SS_Absent`. It now logs `persave: %s: absent (live hotbar kept): source=%s` at `KPLOG_INFO` and returns immediately without overwriting the live INI.
2. **AH2: Transient Object ID Fluctuation (`objfffffffe` vs `objfffffffd`):**
   - In `HotbarLogic.h:characterKeyFor`, the player character alternately registered as `0xFFFFFFFEu` (-2) and `0xFFFFFFFDu` (-3) or `0xFFFFFFFFu` (-1). Naive hex printing caused player profile splitting between `[Profile.objfffffffe]` and `[Profile.objfffffffd]`. Companions used transient heap IDs (`obj8000xxxx`) that reset across module transitions.
   - Fix:
     - Player character recognition (`isExile`) anchors party slot 0, single-member party, `"player"`/`"exile"` tags, and IDs `0xFFFFFFFDu`/`0xFFFFFFFEu` strictly to canonical `"objfffffffd"`.
     - Companions are identified via creature tags (`CExoString` at server creature `+0x18`) yielding stable profile names (e.g. `[Profile.atton]`, `[Profile.kreia]`).
     - Fallback to party slot (`slot1`, `slot2`) if tags are missing, eliminating transient IDs.

---

## Execution Order & Phase Status

| Id | Batch | Status | Validation | Time Est |
|---|---|---|---|---|
| QH1 | [Queue HUD] Strip paused re-inject loop & disallow post-cancel resurrection | DONE | `python3 scripts/run_tests.py -k queue` | 15 min |
| QH2 | [Queue HUD] Exhaustive deterministic unit tests for queueing/cancels/wipes | DONE | `python3 scripts/run_tests.py -k queue` | 25 min |
| QH3 | [Queue HUD] Dark teal Cancel button styling parity (Leader & Companion) | DONE | `python3 scripts/build_check.py --mod queue_hud_src` | 20 min |
| AH1 | [Action Hotbar] Stop wiping live hotbar on absent snapshot (`SS_Absent`) | DONE | `python3 scripts/run_tests.py -k hotbar` | 20 min |
| AH2 | [Action Hotbar] Stabilize character profile keys using party slot / creature tags | DONE | `python3 scripts/run_tests.py -k hotbar` | 20 min |

