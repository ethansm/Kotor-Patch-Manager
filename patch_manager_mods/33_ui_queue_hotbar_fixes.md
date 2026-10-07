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

## Execution Order & Verification Steps

| Step | Action | Time Est | Validation Command |
|---|---|---|---|
| **1** | Apply `combat-hotbar` red theme & padding adjustments | 15 min | `python3 scripts/run_tests.py -k combat` |
| **2** | Fix `companion-selfheal` popover text init & Exile tag check | 20 min | `python3 scripts/run_tests.py -k selfheal` |
| **3** | Update `queue-hud` companion icon sizing to 38px | 10 min | `python3 scripts/run_tests.py -k queue_hud` |
| **4** | Implement dark teal geometry & wide Cancel button in `queue-hud` | 35 min | `python3 scripts/build_check.py --mod queue_hud_src` |
| **5** | Implement multi-item action buffer & wipe guard in `queue-hud` | 40 min | `python3 scripts/run_tests.py -k queue` |
| **6** | Pre-install gate & live test verification | 15 min | `bash scripts/pre_install_gate.sh --mod combat_hotbar_src --mod queue_hud_src` |
