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

### 4.3 QH4: Force Power Queue Buffering (`queue-hud`)
1. **Root Cause Analysis:**
   - `QueueHudPerformUseItem` only intercepts items (`PerformUseItem` 0x775600).
   - In non-combat, native Force power invocations call `CSWCCreature::PerformCastSpell` (`0x00775700`, sibling of `PerformUseItem` at `0x775600`).
   - The engine sends message `(6, 0x12)`, whose server handler `0x65D37F` calls `ActionManager(1)` -> `ClearAllActions(1)` on non-combat actors (`[creature+0x520] == 0`), instantly wiping any in-flight/queued action!
   - Gameplay log showed `ip_cure` -> `ip_knightspeed` -> `ip_mind` replacing each other at entries=1.
2. **Hook & Disassembly Specification:**
   - Target Address: `0x00775703` (6 bytes: `83 ec 0c 89 4d f4`, `sub esp, 0xc` / `mov [ebp-0xc], ecx`).
   - Consumed Exit: `0x0077574C` (`8b e5 5d c2 1c 00`, `mov esp, ebp` / `pop ebp` / `ret $0x1c`).
   - Signature: `__thiscall` on client creature (`ECX`), 7 stack parameters (28 bytes) = `spellId`, `arg2`, `targetId`, `multispell`, `metamagic`, `domain`, `posPtr`.
   - Dispatch Address: `0x00775700` (`PerformCastSpell`).
3. **Implementation Design & Edge Cases:**
   - Detour hook `QueueHudPerformCastSpell(int clientCreature, int ebp)`. Returns 0 if `g_combat != 0` (preserves vanilla in-combat queueing).
   - Unified `QueuedAction` buffer supporting both `ActionKind::Item` and `ActionKind::Spell`.
   - In `QueueHud.cpp:174`, expand `fakeAction` to support `T_CAST_SPELL` (0x0F) with spell ID at `fakeAction[14]` (+0x38), allowing `AddrIconResolver` (`0x0074E0B0`) to look up `spells.2da` icons without blanking.
   - Cancel integration: `doCancel(k)` clears the unified buffer and invokes `ServerClearAllActions`. Follower wipe guard re-injects if follower AI drops a spell before execution.

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
   - In `HotbarLogic.h:characterKeyFor`, the player character alternately registered as `0xFFFFFFFEu` (-2) and `0xFFFFFFFDu` (-3) or `0xFFFFFFFFu` (-1). Naive hex printing caused player profile splitting between `[Profile.objfffffffe]` and `[Profile.objfffffffd]`.
   - Fix: Initial anchor for player slot 0 / tags.
3. **AH3: Fast-Travel Sentinel Fluctuation (`objfffffff9`..`objfffffffa`) & Caching Trap:**
   - **Root Cause from Recent Log:** When fast-traveling between Telos modules, player creature was assigned `0xFFFFFFF9u` (-7) while party table was temporarily desynced (`partySlot = -1`, `isMember0 = false`). Because `HotbarLogic.h:88` only checked `0xFFFFFFFDu` and `0xFFFFFFFEu`, `isExile` returned false, falling through to `obj%08x` (`objfffffff9`). This selected an empty `[Profile.objfffffff9]` with 0/10 slots bound.
   - **Double-Check Discovery (Critical Caching Trap):** In `ActionHotbar.cpp:2032`, `same = (cr == lastCreature && oid == lastOid && g_partyCount == lastParty)` evaluated to `true` on every frame once `g_charKey` was computed as `"objfffffff9"`, locking the character key permanently for that module!
   - **Fix:**
     - In `HotbarLogic.h:characterKeyFor`, map the full Odyssey negative player sentinel range `0xFFFFFFF0u <= oid && oid <= 0xFFFFFFFEu` to canonical `"objfffffffd"` (Exile).
     - In `ActionHotbar.cpp:2032`, enforce `same = (!g_charKey.empty() && g_charKey.rfind("slot", 0) != 0 && cr == lastCreature && oid == lastOid && g_partyCount == lastParty)` so that uninitialized keys or slot fallbacks continue to re-evaluate until resolved to canonical key or named tag.
     - In `computeCharacterKey()`, enhance party slot matching via `(mem == c || (mem != 0 && creatureOid(mem) == oid))` to reliably identify companions even when pointer wrappers differ across module loads.

---

## Workstream 6: Portrait Health Buttons & Action Tray UX (`companion-selfheal` & `queue-hud`)

### 6.1 Objective & Code Targets
- **Target Files:**
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp`
  - `patch_manager_mods/_shared_src/SelfHealUi.h`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp`
  - `patch_manager_mods/queue_hud_src/QueueHudLogic.h`

### 6.2 Implementations
1. **SH1: 58px Companion Health Buttons & Action Tray UX:**
   - Standardize companion health button dimensions to **58x58 px** (`autoIconPx`: `vit.h * 0.60f` clamped `[48, 72]`), matching companion queue buttons.
   - Update `shui::stripReserve(vit)` to reserve space for the 58px button + gap + ring overhang, ensuring `queue-hud` offsets companion queue icons correctly.
   - Render a unified companion action tray wrapper/pill enclosing both the 58px health button and queue icons, using a scaled version of the `lbl_miqueue_teal` style (sea-green `#118367` border, translucent black fill).
2. **SH2: Leader Health Button (Click/Auto Heal for Exile):**
   - Remove the `isLeaderVit` prohibition in `SelfHealUi.h:21` and `resolveState`.
   - Position a 58x58 px health button at top-left of the leader character portrait (`Slot_CardRect 0x54C`).
   - Allocate member 0 widget (`W_BtnLeader` / `mi = 0`), wire click to `useNow(ctx, now, 0, ...)`, and include member 0 in `tickAutoHeal()`, enabling click-to-heal and auto-heal for the Exile.
3. **SH3: Options Popover UX Blank Rendering Fix:**
   - **Double-Check Discovery (Template & Font Clipping Traps):**
     - `LBL_HOTBARPTITLE` does NOT exist in `mipc28x6_p.gui` (it was dynamically created by ActionHotbar from `LBL_QUEUE1`). `createLabelFromTemplate` requires templates present in the layout GFF.
     - `LBL_QUEUE1` is the valid template, but the engine forces the `b` font variant (`dialogfont10x10b`), which has a 16x16 / 24px cell height (V8 research). When the button was 16..20px, popover rows were cramped to 16..20px, vertically clipping out the text!
     - `setAlign(18)` centers text via `(rect.h - font_height) / 2`; when `rect.h < font_height`, this offset is negative, pushing text out of bounds.
   - **Fix:**
     - Expand popover row height in `shui::popoverLayout` to 40..58px (matching 58px button size), providing ample vertical line height for `dialogfont10x10b`.
     - Use `setRect` before setting text/font and align 17 (top-center) or ensure height >= 40px for positive centering.
     - Explicitly invalidate `wText` cache and set text color/alpha on popover show.
4. **QH5: Cancel Queue Button Color Parity:**
   - **Measured Values from `lbl_miqueue_teal.tga`:**
     - Tint (border + fill, one shared rgb field): dark green-teal `{ 0.033f, 0.233f, 0.182f }`, hover sea-green `{ 0.067f, 0.514f, 0.404f }`, alpha `0.85f`.
     - Reason: user play test 2026-10-11 (the black Cancel did not match the menu teal). Border and fill share one tint field, so the fill tint overwrote the border tint.
   - In `QueueHud.cpp:cwLookLeader`: reads `qh::cancelStyle(true)` (`QueueHudLogic.h`); same tint passed to `setBorderTint` and `setFill`.

---

## Execution Order & Phase Status

| Id | Batch | Status | Validation | Time Est |
|---|---|---|---|---|
| QH1 | [Queue HUD] Strip paused re-inject loop & disallow post-cancel resurrection | DONE | `python3 scripts/run_tests.py -k queue` | 15 min |
| QH2 | [Queue HUD] Exhaustive deterministic unit tests for queueing/cancels/wipes | DONE | `python3 scripts/run_tests.py -k queue` | 25 min |
| QH3 | [Queue HUD] Dark teal Cancel button styling parity (Leader & Companion) | DONE | `python3 scripts/build_check.py --mod queue_hud_src` | 20 min |
| AH1 | [Action Hotbar] Stop wiping live hotbar on absent snapshot (`SS_Absent`) | DONE | `python3 scripts/run_tests.py -k hotbar` | 20 min |
| AH2 | [Action Hotbar] Stabilize character profile keys using party slot / creature tags | DONE | `python3 scripts/run_tests.py -k hotbar` | 20 min |
| AH3 | [Action Hotbar] Fix fast-travel profile wipe via negative sentinel range `0xFFFFFFF0`..`0xFFFFFFFE` | DONE | `python3 scripts/run_tests.py -k hotbar` | 15 min |
| QH4 | [Queue HUD] Buffer non-combat Force powers & items in unified FIFO (`haveCur` idle gate, `fakeAction` icon resolution) | PLANNED | `python3 scripts/run_tests.py -k queue` | 30 min |
| QH5 | [Queue HUD] Exact color parity for Cancel button matching `lbl_miqueue_teal` palette | DONE | `python3 scripts/build_check.py --mod queue_hud_src` | 15 min |
| QL1 | [Queue HUD] Non-combat queue fix: pause restore (active-command verdict), ghost `cur`, identity guard, combat hand-off (doc 30 "Queue-fix QL1") | BUILT | `python3 scripts/run_tests.py queue_hud` | 20 min |
| QL2 | [Queue HUD] Strip flicker (active-cmd slot + 400 ms hold), hand-off clears engine queue first (`handoff_clear_first`), ROUND/HANDOFF_CLEAR trace; medpack = recon only (doc 30 "Queue-fix QL2") | BUILT | `python3 scripts/run_tests.py queue_hud` | 25 min |
| QL3 | [Queue HUD] Lost combat-trigger action: engine-held head re-sent when combat start wipes it within 300 ms (`heldHeadAction`, `handoff_resend_held`), log `HANDOFF_HELD`, analyzer flag I (doc 30 "QL3") | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 30 min |
| QL4 | [Queue HUD] Leader Cancel button dark green-teal (`qh::cancelStyle`): border and fill share one tint field, the black fill had overwritten the border | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 10 min |
| QL5 | [Queue HUD] Click-to-cancel icons in the non-combat queue via the engine queue panel: RECON DONE, no implementation (doc 30 "QL5 investigation") | RECON | none | next plan |
| QL6a | [Queue HUD] Pause rule: an eaten command waits while paused (no re-inject, no budget), one re-inject with a fresh budget on unpause (`activeVerdict` rule 4b, `pause_eaten_wait`) | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 15 min |
| QL6b | [Queue HUD] Dispatch gate waits for engine aa8/ab8 flags (`FlagGate`, `DS_FLAG_BUSY`, 2 s cap, `dispatch_wait_flags`) | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 15 min |
| QL6c | [Queue HUD] Held-head rule replaces the 300 ms clock (`heldHeadAction(held, sawExec, paused, unpausedHeldMs)`), `HANDOFF_OUT` 8 s ground truth | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 20 min |
| QL6d | [Queue HUD] Trace: round watch at every combat edge, `CLICK`, `CMBT_FIELDS`, analyzer flag J (`trace_clicks`) | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 20 min |
| QL7 | [Queue HUD] Per-item cancel on the leader strip and companion strips (`ActionBuffer::removeAt`, `StripHit`, `cancelOne`), after P1 play data | PLANNED | `python3 scripts/run_tests.py queue_hud` | 40 min |
| QL8 | [Queue HUD] Soft-combat banner: hide `LBL_CMBTMODEMSG` + backing AND the red bar (BG2/BG3/0x6724 + 3 Clear buttons) only when no hostile (`softCombat`: party target, no non-party target; clear own visible bits, restore if a hostile appears, never 0x6E54; `hide_banner_soft`/`hide_redbar_soft`; leader strip drawn as out of combat; `BANNER` trace), after P1 data + G2 | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 35 min |
| QL9 | [Queue HUD] Buffered force powers never fire: dispatch_now spells fire (cmds 9-11, 14-16), buffered replays do not (cmds 4, 13; cmdq-probe sees no server cast). Suspects: replay passes a non-null zeroed `pos` where the engine got NULL; target `0x00dbee74`. Add SPELL_ARGS trace + ini A/B (`spell_replay_pos`), DLL-only | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 45 min |
| QL10 | [Queue HUD] Combat-click ordering (user choice 10-12: old queue first, then the new attack/power). Today `handoff_clear_first` wipes the engine's new 3f attack and re-sends only the old command (L316 cmd 8, L380 cmd 9 in `.claude/handoffs/ql8/logs`). Needs recon: how to re-issue the attack/power after the hand-off (target = CMBT_FIELDS t550) and what a held head (L501) does. Findings: `.claude/handoffs/ql8/findings.md`. **Built (10-15/16)**: `handoff_keep_new` (no clear while paused, `910b4aa`), `ROUND_ROT` round-list reorder so the new attack runs last (`ce1423c`), Cancel after the combat edge drops the held head AND the buffer (`115d9c8`, `253b380`), `handoff_detach_held` holds an enemy power out of the round list until the hand-off ends (`a770a0a`, DLL 4fcc4a91, rollback `queue-hud_swap_20261016_000435`). Verified in play logs `.claude/handoffs/ql10/logs3` and `logs5`: items then attack (subs 10,10,9,1), shield then stim then power, one Cancel ends combat. The detached power is drawn by QL11 (`b312aac`). Unpaused combat start also uses keep/detach/reorder (`handoff_keep_unpaused`, `5d84d14`, DLL 619c892b, rollback `queue-hud_swap_20261016_010953`): VERIFIED 10-16 in `.claude/handoffs/ql10/logs10` (`HANDOFF_KEEP` + `ROUND_ROT ok=1` + `ROUND_DETACH` at edges with `paused=0 combat=1`; user: "worked"). Open (unexplained, not blocking): one-frame item-icon blip at the first send; stim on a team member starting combat | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 60 min |
| QL11 | [Queue HUD] Show the round node that `handoff_detach_held` holds out of the list as a dimmed extra slot on the next unused vanilla slot control in hard combat (`drawDetachedSlots`, leader only). Built `b312aac`, not installed; play check: the held Force power icon appears next to the shield icon while the shield runs, then moves into the engine queue | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 25 min |
| QL12 | [Queue HUD] Change-gated `MODE m=<k> combat soft menu paused hostile party strip cancel banner why` line per member (`qh::ModeState`, `modeChanged` strcmp compare, `fmtMode`, `trace_mode`/`mode_cap`), DLL-only, read-only | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 45 min |
| QL13 | [Queue HUD] Log gaps: `noteHidden` compares reasons with strcmp; `VIS cancel*` leaves the 64 cap when `trace_mode=1`; the three `CANCEL` emitters share `CANCEL m=<k> act=click/done/refused n=<n>`; startup line prints `handoff_keep_unpaused` | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 15 min |
| QL14 | [Queue HUD analyzer] `queue_hud_log_check.py`: MODE/CANCEL parsed (no longer SKIPPED), `MODES` per-member timeline (12 rows), flags L (soft->hard with no banner restore in 500 ms), M (strip shown in hard combat without show_in_combat), N (Cancel shown in hard combat) | INSTALLED | `python3 scripts/run_tests.py queue_hud_log_check` | 25 min |
| QL15 | [Queue HUD] Play check MODE1 (PLAY_TEST_QUEUE): medpack out of combat, stim on a member with no enemies (soft), hostile appears (soft->hard + BANNER restore), fight to the end, menu open/close; read with `queue_hud_log_check.py` MODES + flags L/M/N | PLANNED | live | 10 min |
| QL16 | [Combat logging, cross-mod] Shared `_shared_src/CombatTrace.h` change-gated `CSTATE combat=a>b paused=a>b menu=a>b` edge line in combat-hotbar, action-hotbar, companion-selfheal, companion-ai (others if trivial); `[c= p= m=]` context on VIS lines; `scripts/combat_timeline.py` (`kotor2_logs.py check combat`) per-combat merged timeline + cross-mod disagreements; play check CST1 | INSTALLED | `python3 scripts/run_tests.py -k combat_trace` | 90 min |
| QL17 | [Queue HUD] 10-18 play bugs (wave `.claude/waves/w3-33-QL17/`, logs `.claude/handoffs/ql12/logs/`): (B1) buffered exile actions ran on Atton after a paused leader switch: client Perform* wrappers act on the controlled creature, so buffers whose owner is not party slot 0 are HELD (`DS_NOT_CTRL`/`RI_NOT_CTRL`, `HANDOFF_HOLD`) until that member leads again; (B2) leader Cancel = `BTN_CLEARALL` clone (thin border like Cancel Combat, bright teal), label fallback; (B3) strip head icon 150% (`strip_head_pct`) and not icon-dimmed while waiting (`waiting_icon_dim=0`); 0.4.3-dev; play check QH17 | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 60 min |
| QL18 | [Queue HUD] `loadIni` reads only 4095 bytes (`QueueHud.cpp` ~L117): read the whole file (size-checked) so keys past byte 4095 apply; ship the QL17 keys near the top of `queue_hud.ini`. Found 10-18: the game-dir `queue_hud.ini` (Oct 6, all keys commented) had no QL17 values, so `strip_head_pct=150`/`waiting_icon_dim=0` were NOT live until appended by hand (backup `queue_hud.ini.bak_ql17_20261018`) | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 20 min |
| QL19 | [Queue HUD] Leader Cancel Queue BUTTON border colour: 0.4.3 used the cyan companion tint (rendered 33,171,179) but the queue tray border is green-teal (18,150,122, sampled from the 10-18 22:50 screenshot); new `qh::cancelButtonStyle()` tint (0.055,0.51,0.42), hover (0.09,0.66,0.54); with QL18 reader fix = 0.4.4-dev. NOT installed (user: no install yet) | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 20 min |
| QL20 | [Queue HUD] 10-23 play bug (logs `.claude/handoffs/ql20/logs/`): queued companion Force powers stayed queued until that companion led again, then all ran in order. Two causes: (1) QL17 B1 HOLD of buffers owned by members 1-2 (`DS_NOT_CTRL`, cmd 20/21); (2) a power the engine took directly (`dispatch_now`) while the member led starves behind its 0x3D FollowLeader after the switch (cmd 17 ran 53 s late; doc 29 PQ2 1.5). Fix: `dispatchAction` sends a member-1/2 Force power on an object target through `AddUseTalentOnObjectActions` 0x57D8B0 on that member's server creature (`serverCast`, `SRV_CAST` line; arg2 = target client id, 2nd stack arg of 0x775700), and `kickStarved` rotates a starved 0x3F to the head under the selfheal CS12 R8 gates (`SRV_KICK`/`SRV_KICK_SKIP`). Items and location targets stay held; ini `companion_server_cast` (default 1); 0.4.5-dev LIVE 10-23 15:44 (KPM 8f48ddb, rollback queue-hud_swap_20261023_154447); play check QH20 | INSTALLED | `python3 scripts/run_tests.py queue_hud` | 60 min |
| PB1 | [cmdq-probe] No log: `[cmdq-probe] level=info` missing from game `kplog.ini` (added) + `kotor2_logs.py check cmdq-probe` route | BUILT | `python3 scripts/run_tests.py kotor2_logs` | 10 min |
| HK1 | [cmdq-probe 0.1.0] Log-only probe mod, 11 windows (pause/cmd/fire/round/clr/reequip/med), log checker `scripts/cmdq_probe_log_check.py`; CanUseItem window is 0x5D32AE (read eax), not 0x5D32AB; AddAttackActions re-adds 0x0C not 0x3F (`.claude/handoffs/hk1/`). NOT installed: needs full reinstall + GO | INSTALLED | `python3 scripts/run_tests.py cmdq_probe_log` | 45 min |
| SH1 | [SelfHeal] 58px companion health button & unified action tray wrapper | DONE | `python3 scripts/run_tests.py -k selfheal` | 25 min |
| SH2 | [SelfHeal] Leader 58px health button at top-left of leader portrait | DONE | `python3 scripts/run_tests.py -k selfheal` | 25 min |
| SH3 | [SelfHeal] Popover options text fix: `LBL_HOTBARPTITLE` template & 58px row layout | DONE | `python3 scripts/run_tests.py -k selfheal` | 20 min |


