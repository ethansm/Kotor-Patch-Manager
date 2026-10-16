REVIEW queue-hud | files read: 25 | findings: 11

### R2-QH-01 - Clicking Cancel during soft combat does not clear `g_handoff[k]`, immediately re-dispatching the cancelled head
- Class: reachable
- Severity: medium
- Confidence: 98
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:808-810 :: doCancel
- Anchor: `g_itemQueue[k].cancel();`
- Trigger: `onFrameBegin` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1604) -> `tickItemQueues` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1648) -> `handoffTick` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1252) on a soft-combat rising edge (`g_combat != 0 && g_softCombat`) moves `buf.activeAction` into `g_handoff[k].head` (`h.pending = true`, `h.headValid = true`, `h.headHeld = true` or waiting on `s.nrn` / `midFlight` at patch_manager_mods/queue_hud_src/QueueHud.cpp:1054-1072). While still in soft combat (where commit `528c59d` keeps the Cancel control visible and active via `const bool hardCombat = g_combat != 0 && !g_softCombat;` at patch_manager_mods/queue_hud_src/QueueHud.cpp:1621), the player clicks Cancel -> `onFrameBegin` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1641) -> `doCancel(on)` (patch_manager_mods/queue_hud_src/QueueHud.cpp:785-811) -> `g_itemQueue[k].cancel()` and `ServerClearAllActions(server, id)` (patch_manager_mods/queue_hud_src/QueueHud.cpp:808-809) -> `tickItemQueues` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1648) -> `handoffTick` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1077-1138).
- Impact: `doCancel` clears `g_itemQueue[k]` and the engine action queue, but leaves `g_handoff[k].pending` and `g_handoff[k].headValid` set. Seven lines later in the same `onFrameBegin` call, `tickItemQueues` sees `g_combat != 0` and runs `handoffTick`. Because `ServerClearAllActions` just removed the action from the engine, `engineHoldsAction(s)` is now `false`; when `h.sawExec` is `false` (e.g. while paused) or `h.headHeld` was already `false`, `handoffTick` pops `h.head` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1109) and calls `dispatchAction(a, k)` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1138), immediately re-injecting the command the player just cancelled.
- Fix: Call `endHandoff(k, qh::DR_CANCEL);` inside `doCancel(int k)` when cancelling `g_itemQueue[k]`.
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/QueueHud.cpp
+++ b/patch_manager_mods/queue_hud_src/QueueHud.cpp
@@ -806,5 +806,6 @@ void doCancel(int k) {
     }
     g_lastOurs[k].tag = "cancel"; g_lastOurs[k].tick = GetTickCount();
+    endHandoff(k, qh::DR_CANCEL);
     g_itemQueue[k].cancel();
     reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ServerClearAllActions)(server, id);
     g_cpend[k].on = true; g_cpend[k].n = n;
```

### R2-QH-02 - `actionInQueue` still uses raw `s.haveCur` before `creal`, keeping `ev.actionPresent` true forever on stale `subType` 9/10 nodes
- Class: reachable
- Severity: medium
- Confidence: 97
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:1342-1355 :: tickItemQueues
- Anchor: `if (s.haveCur && (s.cur.subType == 9 || s.cur.subType == 10)) actionInQueue = true; // 9 = cast spell, 10 = use item`
- Trigger: Reported in the 2026-10-14 review and still present in current code. `onFrameBegin` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1648) -> `tickItemQueues` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1342-1355) when `buf.active` is `true` and the creature's round has a flagless current round node (`s.haveCur == true`, `s.cur.subType == 9` or `10`, `s.aa8 == 0`, `s.ab8 == 0`, `s.paused == false`) that sits unpaused for `>= qh::kStaleCurMs` (1500 ms).
- Impact: After 1500 ms unpaused, `creal` (`qh::curReal`, patch_manager_mods/queue_hud_src/QueueHud.cpp:1350) flips to `false` and `STALE_CUR` is logged (patch_manager_mods/queue_hud_src/QueueHud.cpp:1363), but `actionInQueue` was already set to `true` at line 1342 from raw `s.haveCur` and passed into `qh::makeEngineView(..., actionInQueue, ...)` at line 1353 (`ev.actionPresent = actionInQueue`), `qh::reInjectWhy` at line 1372, and `buf.noteView(actionInQueue, now)` at line 1407. Because rule 1 of `qh::activeVerdict` (patch_manager_mods/queue_hud_src/QueueHudLogic.h:1280) returns `AV_PRESENT` whenever `ev.actionPresent` is `true`, `buf.active` never resolves or re-injects and any buffered commands in `buf.queue[]` stay permanently blocked on `DS_ACTIVE`.
- Fix: Compute `buf.staleCurSince` and `creal` before `actionInQueue`, and gate the `s.cur` check in `actionInQueue` on `creal` rather than raw `s.haveCur`.
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/QueueHud.cpp
+++ b/patch_manager_mods/queue_hud_src/QueueHud.cpp
@@ -1336,15 +1336,15 @@ void tickItemQueues(DWORD now, bool inMenu) {
         //    Known limit (BG23): an unrelated engine-initiated cast can look like our eaten action (AV_PRESENT) until it ends.
+        // Stale-cur bookkeeping: a flagless item/spell round node that sits unpaused with aa8/ab8 == 0 goes stale.
+        if (s.haveCur && !s.paused && s.aa8 == 0 && s.ab8 == 0) { if (buf.staleCurSince == 0) buf.staleCurSince = now; }
+        else buf.staleCurSince = 0;
+        const bool creal = qh::curReal(s.haveCur, s.haveCur ? (int)s.cur.subType : -1, s.aa8, s.ab8, s.paused, buf.staleCurSince, now);
+
         bool actionInQueue = false;
         bool hasFollowLeader = false;
         for (int i = 0; i < s.engineNe; ++i) {
             if (s.e[i].type == qh::T_USE_ITEM || s.e[i].type == qh::T_CAST_SPELL) actionInQueue = true;
             if (s.e[i].type == 0x3D) hasFollowLeader = true; // AIActionFollowLeader
         }
-        if (s.haveCur && (s.cur.subType == 9 || s.cur.subType == 10)) actionInQueue = true; // 9 = cast spell, 10 = use item
+        if (creal && (s.cur.subType == 9 || s.cur.subType == 10)) actionInQueue = true; // 9 = cast spell, 10 = use item
         for (int i = 0; i < s.nrn; ++i) {
             if (s.rn[i].subType == 9 || s.rn[i].subType == 10) actionInQueue = true;
         }
-
-        // Stale-cur bookkeeping: a flagless item/spell round node that sits unpaused with aa8/ab8 == 0 goes stale.
-        if (s.haveCur && !s.paused && s.aa8 == 0 && s.ab8 == 0) { if (buf.staleCurSince == 0) buf.staleCurSince = now; }
-        else buf.staleCurSince = 0;
-        const bool creal = qh::curReal(s.haveCur, s.haveCur ? (int)s.cur.subType : -1, s.aa8, s.ab8, s.paused, buf.staleCurSince, now);
```

### R2-QH-03 - `queueCountOf` passes raw `s.haveCur` instead of `curReal` to `decideAction`, buffering commands behind ghost `cur` nodes
- Class: reachable
- Severity: medium
- Confidence: 96
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:763-772 :: queueCountOf
- Anchor: `if (haveCur) *haveCur = ok && s.haveCur;`
- Trigger: Out of combat, a party member has a leftover ghost `[round+0xB18]` pointer (`s.haveCur == true` with `s.cur.subType == 1` and `s.aa8 == 0 && s.ab8 == 0` after combat, or a stale flagless `subType` 9/10 node with `now - staleCurSince >= 1500`). The player uses an item or casts a Force power -> H9 `QueueHudPerformUseItem` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1862-1865) or H10 `QueueHudPerformCastSpell` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1961-1964) -> `queueCountOf(memberK, &haveCur, &gateQ)` (patch_manager_mods/queue_hud_src/QueueHud.cpp:763-772) -> `qh::decideAction(g_itemQueue[memberK], g_combat != 0, gateQ > 0 ? gateQ : 0, haveCur)` (patch_manager_mods/queue_hud_src/QueueHudLogic.h:737-742).
- Impact: Commit `c7b205b`/`910b4aa` added the `haveCur` parameter to `decideAction` to prevent dispatching into a running round node, but `queueCountOf` populates `*haveCur` with raw `s.haveCur` instead of `qh::curReal(...)` (which `tickItemQueues` uses at line 1350). Whenever a creature has a leftover ghost attack node (`subType == 1`, `aa8 = ab8 = 0`) or stale `cur`, `decideAction` sees `haveCur == true` on an otherwise idle creature (`gateQ == 0`, `buf` idle) and returns `DECIDE_BUFFER` (consuming the engine call with `return 1`) instead of `DECIDE_DISPATCH_NOW`. If triggered while `inMenu` is true (`tickItemQueues` returns early at line 1251), the command does not dispatch until the menu closes; on the HUD it forces an unnecessary buffer-and-replay cycle.
- Fix: Compute `*haveCur` in `queueCountOf` using `qh::curReal` (with `g_itemQueue[k].staleCurSince` when `0 <= k < 3`).
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/QueueHud.cpp
+++ b/patch_manager_mods/queue_hud_src/QueueHud.cpp
@@ -767,5 +767,8 @@ int queueCountOf(int k, bool* haveCur = nullptr, int* gateNe = nullptr) {
     Snapshot s;
     const bool ok = gatherFor(m ? party::serverCreature(m) : 0, s);
-    if (haveCur) *haveCur = ok && s.haveCur;
+    const uint32_t staleSince = (k >= 0 && k < 3) ? g_itemQueue[k].staleCurSince : 0u;
+    if (haveCur) {
+        *haveCur = ok && qh::curReal(s.haveCur, s.haveCur ? static_cast<int>(s.cur.subType) : -1, s.aa8, s.ab8, s.paused, staleSince, GetTickCount());
+    }
     if (gateNe) *gateNe = ok ? s.gateNe : -1;
     return ok ? s.engineNe : -1;
```

### R2-QH-04 - Combat ending while `g_handoff[k].pending` is true drops `g_handoff[k].head` without executing it or restoring `buf.active`
- Class: reachable
- Severity: medium
- Confidence: 93
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:1306-1307 :: tickItemQueues
- Anchor: `if (g_handoff[k].pending) endHandoff(k, qh::DR_COMBAT_CLEAR);`
- Trigger: Out of combat, party member `k` has an active command (`buf.active == true`). Combat starts (`g_combat == 1`, e.g. a brief soft-combat pulse from an ally-targeted action or a short combat) -> `onFrameBegin` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1648) -> `tickItemQueues` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1252) -> `handoffTick` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1050-1073) moves `buf.activeAction` into `g_handoff[k].head` (`h.headValid = true`, `h.pending = true`) and sets `buf.active = false` (line 1069). While `h.head` is still waiting in `g_handoff[k]` (`h.headHeld` waiting in `HELD_WAIT` at line 1095, `s.nrn + 1 > qh::kMaxRound` at line 1091, or `midFlight` at line 1107), combat ends (`g_combat == 0`) -> next `tickItemQueues` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1306-1307) calls `endHandoff(k, qh::DR_COMBAT_CLEAR)` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1020-1028).
- Impact: `endHandoff` logs `DROP ... why=combat_clear` and zeroes `g_handoff[k]` while `buf.active` remains `false`. If the active command was wiped by the combat transition or eaten while paused, it is permanently lost and never re-injected out of combat, while later commands in `buf.queue[]` still dispatch.
- Fix: When combat ends while `g_handoff[k].pending` is true, if `g_handoff[k].headValid && !g_itemQueue[k].active`, restore `g_itemQueue[k].activeAction = g_handoff[k].head; g_itemQueue[k].active = true; g_itemQueue[k].markDispatched(now);` before clearing `g_handoff[k]`.
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/QueueHud.cpp
+++ b/patch_manager_mods/queue_hud_src/QueueHud.cpp
@@ -1305,4 +1305,11 @@ void tickItemQueues(DWORD now, bool inMenu) {
     // Combat ended before a hand-off drained: the held head is dropped; the still-buffered commands run through the sequential dispatch below.
-    for (int k = 0; k < 3; ++k)
-        if (g_handoff[k].pending) endHandoff(k, qh::DR_COMBAT_CLEAR);
+    for (int k = 0; k < 3; ++k) {
+        if (!g_handoff[k].pending) continue;
+        if (g_handoff[k].headValid && !g_itemQueue[k].active) {
+            g_itemQueue[k].activeAction = g_handoff[k].head;
+            g_itemQueue[k].active = true;
+            g_itemQueue[k].markDispatched(now);
+            g_handoff[k].headValid = false;
+        }
+        endHandoff(k, qh::DR_COMBAT_CLEAR);
+    }
```

### R2-QH-05 - Party rotation before `tickItemQueues`'s identity pre-pass causes `handoffTick`, `doCancel`, and `QueueHudPerform*` hooks to index `g_itemQueue` by the wrong slot
- Class: reachable
- Severity: medium
- Confidence: 92
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:1251-1254 :: tickItemQueues
- Anchor: `if (combatNow) { combatEdgeTrace(now, edgeFlags); handoffTick(now, edgeFlags); roundWatchTick(now); }   // unreadable HUD flags: no hand-off and no drop, the buffers wait`
- Trigger: Party member 0 (Creature A) has a busy buffer in `g_itemQueue[0]` (`bufOwner(g_itemQueue[0]) == A`), and the player rotates party leader with Tab (Creature B is now in slot 0, Creature A in slot 1):
  1. **Combat hand-off path**: Combat starts on the same frame or while `g_combat != 0` -> `onFrameBegin` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1648) -> `tickItemQueues` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1251-1253) enters `if (inMenu || g_combat != 0)`, calls `handoffTick(now, edgeFlags)`, and returns before the identity pre-pass at line 1269 -> `handoffTick` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1044-1050, 1080-1138) reads `srv` for slot 0 (Creature B), evaluates `engineHoldsAction(s)` on B, and (when `handoff_keep_new=0`) calls `ServerClearAllActions` on B before dispatching `g_itemQueue[0]`'s actions (whose `a.creature` is A).
  2. **Cancel path**: `onFrameBegin` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1641) calls `doCancel(on)` before `tickItemQueues` (line 1648). If party order changed since the previous `tickItemQueues` (e.g. while `inMenu` or in combat), `doCancel(on)` cancels the unswapped `g_itemQueue[on]` (Creature A's buffer) while calling `ServerClearAllActions` on `party::member(tbl, on)` (Creature B).
  3. **Hook capture path**: If `QueueHudPerformUseItem` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1844-1881) or `QueueHudPerformCastSpell` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1937-1980) runs for Creature B (`memberK = 0`) before `tickItemQueues` runs its identity pre-pass, B's action is pushed into `g_itemQueue[0]` while `g_itemQueue[0].activeAction.creature` is still A; on the next `tickItemQueues`, `bufOwner(g_itemQueue[0])` (patch_manager_mods/queue_hud_src/QueueHud.cpp:856) returns A, so the pre-pass at line 1275 swaps the entire `g_itemQueue[0]` (carrying B's newly pushed command) into A's slot (`g_itemQueue[1]`). Similarly, on an `IDENT_SWAP`, `g_flagGate[k]` (line 940) and `g_handoff[k]` (line 950) are not swapped with `g_itemQueue[k]`.
- Impact: Rotating party order around combat entry, Cancel clicks, or hook captures can cross-contaminate two party members' `ActionBuffer` queues, cancel the wrong member's buffered commands, or run hand-off checks and `ServerClearAllActions` against the wrong party member.
- Fix: Extract the slot-reconciliation loop into a helper that swaps `g_itemQueue`, `g_lastOurs`, `g_handoff`, and `g_flagGate`, and call it at the top of `tickItemQueues` (before the `inMenu || g_combat != 0` branch), at the top of `doCancel`, and in `QueueHudPerformUseItem` / `QueueHudPerformCastSpell` before indexing `g_itemQueue[memberK]`.

### R2-QH-06 - `softBannerTick` resets `g_softCombat = false` before checking `hideBannerSoft`/`hideRedbarSoft` and `g_combatUnreadable`
- Class: reachable
- Severity: medium
- Confidence: 95
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:1684-1691 :: softBannerTick
- Anchor: `g_softCombat = false;`
- Trigger: `onHudFrame` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1791-1793) -> `QueueHudCapture(ctx->hud)` -> `softBannerTick(ctx->hud)` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1684-1691), under either of two conditions during soft combat:
  1. The user sets `hide_banner_soft=0` and `hide_redbar_soft=0` in `queue_hud.ini`, hitting `if (!g_cfg.hideBannerSoft && !g_cfg.hideRedbarSoft) return;` at line 1686 after `g_softCombat = false;` at line 1685.
  2. `QueueHudCapture` hits a transient unreadable HUD flags word (`g_combatUnreadable = true`, `g_combat = 1` at lines 1803-1804), and `softBannerTick` overwrites `g_softCombat = false;` at line 1685 before hitting `if (g_combatUnreadable) return; // no verdict, no writes; the state is kept` at line 1691.
- Impact: When both banner/redbar hide INI switches are disabled, `g_softCombat` is permanently `false`, disabling the soft-combat Leader Queue HUD (`drawLeader`, line 562) and soft-combat Cancel buttons (`paintLeader` line 525, `drawStrips` line 726, `onFrameBegin` line 1621) added in `528c59d`. When `g_combatUnreadable` is true for one frame during soft combat, `g_softCombat` is cleared to `false` instead of being kept, so `hardCombat` (`g_combat != 0 && !g_softCombat`) becomes `true` and `onFrameBegin` (line 1623) calls `cwHideAll("combat")`, hiding the Cancel buttons and resetting their hover/fill state.
- Fix: Check `if (g_combatUnreadable) return;` before modifying `g_softCombat`, and compute `g_softCombat` whenever `g_combat != 0` before returning early on `!g_cfg.hideBannerSoft && !g_cfg.hideRedbarSoft`.
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/QueueHud.cpp
+++ b/patch_manager_mods/queue_hud_src/QueueHud.cpp
@@ -1684,9 +1684,8 @@ void bannerTrace(const char* act, bool soft, int nWr) {
 void softBannerTick(uintptr_t hud) {
-    g_softCombat = false;
-    if (!g_cfg.hideBannerSoft && !g_cfg.hideRedbarSoft) return;
     if (g_bannerHidden && hud != g_bannerHud) {   // HUD rebuilt (load): the old controls are gone
         bannerTrace("forget", false, 0);
         g_bannerHidden = false; g_bannerMask = 0; g_bannerLastAct = -1; g_bannerResetLogged = false;
     }
     if (g_combatUnreadable) return;               // no verdict, no writes; the state is kept
+    g_softCombat = false;
     if (g_combat == 0) {
@@ -1719,3 +1718,4 @@ void softBannerTick(uintptr_t hud) {
     g_softCombat = soft;
+    if (!g_cfg.hideBannerSoft && !g_cfg.hideRedbarSoft) return;
     const qh::BannerAct act = qh::bannerAct(g_bannerHidden, true, soft);
```

### R2-QH-07 - `drawAll`, `drawLeader`, and `drawStrips` still gate on raw `g_combat != 0` instead of `hardCombat`
- Class: reachable
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:742-745 :: drawAll
- Anchor: `if (!qh::shouldShow(g_cfg.enabled, g_combat != 0, ctx->inMenu != 0, age, 1, g_cfg.showInCombat)) {`
- Trigger: `onHudDraw` (patch_manager_mods/queue_hud_src/QueueHud.cpp:752-757) -> `drawAll` (patch_manager_mods/queue_hud_src/QueueHud.cpp:742) -> `drawLeader` (patch_manager_mods/queue_hud_src/QueueHud.cpp:562-566) and `drawStrips` (patch_manager_mods/queue_hud_src/QueueHud.cpp:680) while in soft combat (`g_combat != 0 && g_softCombat`):
  1. In `drawAll` (line 742), `qh::shouldShow` is passed raw `g_combat != 0` instead of `hardCombat` (`g_combat != 0 && !g_softCombat`).
  2. In `drawLeader` (line 563-565), `qh::shouldShow(g_cfg.enabled, hardCombat, ctx->inMenu != 0, age, n)` checks `hardCombat`, but when it returns `false` because `n == 0` in soft combat, line 565 calls `noteHidden(g_combat != 0 ? "combat" : "empty queue");` using raw `g_combat != 0`.
  3. In `drawStrips` (line 680), `qh::panelHold(hs.tick, now, inMenu, g_combat != 0 && !g_cfg.showInCombat)` passes raw `g_combat != 0` instead of `hardCombat`.
- Impact: If the user sets `show_in_combat=0` in `queue_hud.ini` (documented as hiding strips in real combat), `drawAll` returns at line 742 during soft combat too, hiding the leader panel, companion strips, and Cancel buttons during soft combat. Under default `show_in_combat=1`, when the leader's queue becomes empty during soft combat, `drawLeader` logs `hidden (combat)` instead of `hidden (empty queue)`.
- Fix: Use `const bool hardCombat = g_combat != 0 && !g_softCombat;` in `drawLeader` line 565, `drawStrips` line 680, and `drawAll` line 742-743.
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/QueueHud.cpp
+++ b/patch_manager_mods/queue_hud_src/QueueHud.cpp
@@ -563,5 +563,5 @@ void drawLeader(uintptr_t hud, DWORD now, DWORD age, const K2PsContext* ctx) {
     if (!qh::shouldShow(g_cfg.enabled, hardCombat, ctx->inMenu != 0, age, n)) {
         if (n == 0 && held.nd > 0 && qh::panelHold(held.tick, now, ctx->inMenu != 0, hardCombat)) { paintLeader(hud, held.d, held.nd, age, ctx); return; }
-        noteHidden(g_combat != 0 ? "combat" : "empty queue");
+        noteHidden(hardCombat ? "combat" : "empty queue");
         return;
     }
@@ -678,5 +678,6 @@ void drawStrips(uintptr_t hud, DWORD now, bool inMenu) {
         HeldStrip& hs = g_heldStrip[k];
         if (hs.creature != srv[k]) { hs.d = StripDraw(); hs.tick = 0; hs.creature = srv[k]; }   // a Tab switch never shows the old strip
-        if (n == 0 && hs.d.have && qh::panelHold(hs.tick, now, inMenu, g_combat != 0 && !g_cfg.showInCombat)) {
+        const bool hardCombat = g_combat != 0 && !g_softCombat;
+        if (n == 0 && hs.d.have && qh::panelHold(hs.tick, now, inMenu, hardCombat && !g_cfg.showInCombat)) {
             d[k] = hs.d;   // hold repaint: Cancel below and the draw loop use the held rects; hs.tick is not refreshed
             memcpy(types[k], hs.types, sizeof types[k]);
@@ -740,6 +741,7 @@ void drawAll(const K2PsContext* ctx) {
     uint32_t probe = 0;
     if (!hud || !rd(SiteHud, hud, HudSlotBase, &probe)) { if (hud) noteHidden("hud unreadable"); if (g_cfg.companions) stripsHidden(); return; }
-    if (!qh::shouldShow(g_cfg.enabled, g_combat != 0, ctx->inMenu != 0, age, 1, g_cfg.showInCombat)) {
-        noteHidden(g_combat ? "combat" : ctx->inMenu ? "menu" : "stale capture");
+    const bool hardCombat = g_combat != 0 && !g_softCombat;
+    if (!qh::shouldShow(g_cfg.enabled, hardCombat, ctx->inMenu != 0, age, 1, g_cfg.showInCombat)) {
+        noteHidden(hardCombat ? "combat" : ctx->inMenu ? "menu" : "stale capture");
         if (g_cfg.companions) stripsHidden();
         return;
```

### R2-QH-08 - `cwLookCompanion` still latches `w.fill == 2` on text fallback and never clears `"X"` text if texture fill later succeeds
- Class: latent
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:415-426 :: cwLookCompanion
- Anchor: `if (w.fill == want || w.fill == 2) return;`
- Trigger: Reported in the 2026-10-14 review and still present in current code. `onHudDraw` (patch_manager_mods/queue_hud_src/QueueHud.cpp:756) -> `cwApply` (patch_manager_mods/queue_hud_src/QueueHud.cpp:444) -> `cwLook` (patch_manager_mods/queue_hud_src/QueueHud.cpp:430) -> `cwLookCompanion` (patch_manager_mods/queue_hud_src/QueueHud.cpp:415-426) when `tex::image(want ? tex::CancelHi : tex::Cancel)` returns `nullptr` or `guikit::setFill` fails on one frame while the companion Cancel is shown.
- Impact: Line 423 sets the label text to `"X"` and line 424 sets `w.fill = 2`. Because line 419 checks `if (w.fill == want || w.fill == 2) return;`, the control stays latched to the text fallback for as long as it remains visible even after `embtex::heal()` rebuilds the texture. Moreover, if the control hides (`cwShow` resets `w.fill = -1` at line 389) and is later shown again when `guikit::setFill` succeeds at line 422, `guikit::setText(w.c, "")` is never called, leaving the `"X"` text drawn over the `kit_cancel` texture.
- Fix: Remove `|| w.fill == 2` at line 419 (or retry when `img` is available) and clear the text with `guikit::setText(w.c, "");` when `guikit::setFill` succeeds.
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/QueueHud.cpp
+++ b/patch_manager_mods/queue_hud_src/QueueHud.cpp
@@ -418,6 +418,7 @@ void cwLookCompanion(CW& w) {
     const int want = w.hover ? 1 : 0;
-    if (w.fill == want || w.fill == 2) return;
+    if (w.fill == want) return;
     void* img = tex::image(want ? tex::CancelHi : tex::Cancel);
     const float* tint = w.hover ? tealHover : tealBorder;
-    if (img && guikit::setFill(w.c, img, tint)) { w.fill = want; return; }
+    if (img && guikit::setFill(w.c, img, tint)) { if (w.fill == 2) guikit::setText(w.c, ""); w.fill = want; return; }
+    if (w.fill == 2) return;
     guikit::setFont(w.c, "dialogfont10x10"); guikit::setAlign(w.c, 18); guikit::setText(w.c, "X");
```

### R2-QH-09 - `edgeTick` logs false `EDGE what=combat` transitions and updates `pCombat` on unreadable HUD frames
- Class: latent
- Severity: low
- Confidence: 92
- Location: patch_manager_mods/queue_hud_src/QueueHud.cpp:1592-1600 :: edgeTick
- Anchor: `if (combat != pCombat) { traceEdge("combat", pCombat, combat, g_combatUnreadable ? "unreadable" : "hud"); pCombat = combat; }`
- Trigger: `QueueHudCapture` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1802-1804) encounters a transient unreadable HUD flags word (`flagsOk == false`), setting `g_combat = 1` and `g_combatUnreadable = true` -> `onFrameBegin` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1646) -> `edgeTick(inMenu)` (patch_manager_mods/queue_hud_src/QueueHud.cpp:1592-1600).
- Impact: While `ab456aa` fixed `tickItemQueues` (line 1250) via `qh::combatEdgeStep` so unreadable frames leave `g_prevCombat` untouched, `edgeTick` still treats `g_combat = 1` on an unreadable frame as a real combat transition, logging `EDGE what=combat old=0 new=1 src=unreadable`, overwriting `pCombat = true`, and then logging a second false transition `EDGE what=combat old=1 new=0 src=hud` on the next readable non-combat frame.
- Fix: Skip updating `pCombat` (and defer `init` for `pCombat` or preserve `pCombat`) when `g_combatUnreadable` is true.
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/QueueHud.cpp
+++ b/patch_manager_mods/queue_hud_src/QueueHud.cpp
@@ -1598,3 +1598,3 @@ void edgeTick(bool inMenu) {
-    if (!init) { init = true; pCombat = combat; pPaused = paused; pMenu = inMenu; return; }
-    if (combat != pCombat) { traceEdge("combat", pCombat, combat, g_combatUnreadable ? "unreadable" : "hud"); pCombat = combat; }
+    if (!init) { if (g_combatUnreadable) return; init = true; pCombat = combat; pPaused = paused; pMenu = inMenu; return; }
+    if (!g_combatUnreadable && combat != pCombat) { traceEdge("combat", pCombat, combat, "hud"); pCombat = combat; }
     if (paused != pPaused) { traceEdge("paused", pPaused, paused, "hud"); pPaused = paused; }
```

### R2-QH-10 - `testWhyParityMatrix` still sizes `reHits[7]` and `dsHits[6]` one short of `ReInjectWhy` (8) and `DispatchWhy` (7), never testing `RI_EXECUTED` or `DS_FLAG_BUSY`
- Class: test-gap
- Severity: low
- Confidence: 100
- Location: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1458-1499 :: testWhyParityMatrix
- Anchor: `int reHits[7] = { 0 };`
- Trigger: Reported in the 2026-10-14 review and still present in current code. `testWhyParityMatrix` (patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1458-1499, called from `main` at line 2390) tests `ReInjectWhy` (`RI_OK = 0` .. `RI_EXECUTED = 7`, 8 values in patch_manager_mods/queue_hud_src/QueueHudLogic.h:790) and `DispatchWhy` (`DS_OK = 0` .. `DS_FLAG_BUSY = 6`, 7 values in patch_manager_mods/queue_hud_src/QueueHudLogic.h:791).
- Impact: `reHits` has size 7 and `dsHits` has size 6, while the loops never vary `b.sawExecuting` or `flagHold`. Contrary to the assertions `// every ReInjectWhy is reachable in the matrix` (line 1482) and `// every DispatchWhy is reachable in the matrix` (line 1498), `RI_EXECUTED` (7) and `DS_FLAG_BUSY` (6) are never exercised in `testWhyParityMatrix`, and reaching either value would write past the end of `reHits` / `dsHits`.
- Fix: Resize `reHits[8]` and `dsHits[7]`, vary `b.sawExecuting` in the `ReInjectWhy` loop and `flagHold` in the `DispatchWhy` loop, and check all 8 and 7 bins.
- Diff:
```diff
--- a/patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp
+++ b/patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp
@@ -1460,5 +1460,6 @@ void testWhyParityMatrix() {
-    int reHits[7] = { 0 };
+    int reHits[8] = { 0 };
     for (int active = 0; active < 2; ++active)
     for (int pend = 0; pend < 2; ++pend)
     for (int bcan = 0; bcan < 2; ++bcan)
     for (int rc = 1; rc <= 3; ++rc)
+    for (int exec = 0; exec < 2; ++exec)
     for (int inq = 0; inq < 2; ++inq)
@@ -1475,18 +1476,20 @@ void testWhyParityMatrix() {
         b.activeAction = makeSpellAction(0x1000, 0x55, 0, 1, 0, 0, 0, pos, 1);
         b.activeAction.reInjectCount = rc;
+        b.sawExecuting = exec != 0;
         const uint32_t now = nowSet ? 999u : 0u;
         const bool want = shouldReInject(b, inq != 0, lead != 0, paused != 0, now, pcan != 0);
         const ReInjectWhy why = reInjectWhy(b, inq != 0, lead != 0, paused != 0, now, pcan != 0);
         KT_CHECK_EQ(why == RI_OK, want);
         reHits[why]++;
     }
-    for (int r = 0; r < 7; ++r) KT_CHECK(reHits[r] > 0);   // every ReInjectWhy is reachable in the matrix
+    for (int r = 0; r < 8; ++r) KT_CHECK(reHits[r] > 0);   // every ReInjectWhy is reachable in the matrix
 
-    int dsHits[6] = { 0 };
+    int dsHits[7] = { 0 };
     for (int active = 0; active < 2; ++active)
     for (int isEmpty = 0; isEmpty < 2; ++isEmpty)
     for (int eng = 0; eng < 3; ++eng)
     for (int cur = 0; cur < 2; ++cur)
-    for (int nrn = -1; nrn <= 1; ++nrn) {
+    for (int nrn = -1; nrn <= 1; ++nrn)
+    for (int fh = 0; fh < 2; ++fh) {
         ActionBuffer b;
         if (!isEmpty) KT_CHECK(b.push(makeSpellAction(0x1000, 0x55, 0, 1, 0, 0, 0, pos, 1)));
         b.active = active != 0;
         const bool want = shouldDispatchNext(b, eng, cur != 0, nrn);
-        const DispatchWhy why = dispatchWhy(b, eng, cur != 0, nrn);
-        KT_CHECK_EQ(why == DS_OK, want);
+        const DispatchWhy why = dispatchWhy(b, eng, cur != 0, nrn, fh != 0);
+        KT_CHECK_EQ(why == DS_OK, want && !fh);
         dsHits[why]++;
     }
-    for (int r = 0; r < 6; ++r) KT_CHECK(dsHits[r] > 0);   // every DispatchWhy is reachable in the matrix
+    for (int r = 0; r < 7; ++r) KT_CHECK(dsHits[r] > 0);   // every DispatchWhy is reachable in the matrix
 }
```

### R2-QH-11 - `30_queue_hud.md` and `queue_hud.ini` misstate `handoff_keep_new` default behaviour, `heldHeadAction` timeout, and left-strip auto icon size
- Class: doc
- Severity: low
- Confidence: 96
- Location: patch_manager_mods/30_queue_hud.md:152-170 :: Design
- Anchor: `- **QL3 (2026-10-11)**: play test 10-11 13:07 log: cmd 8 (sonic grenade, the combat trigger) was engine-held at the edge, so `engineHoldsAction` skipped the re-send and the engine wiped it 70 ms later (HANDOFF L302, no CLEAR/ONE). Our clear killed no trigger (0 of 3). Fix: an engine-held head waits for the engine to release it and is re-sent if that happens within 300 ms of the edge (`qh::heldHeadAction`), else dropped; ini `handoff_resend_held` (default 1, 0 = old behaviour). Attack clicks are still never hooked: untracked, needs cmdq-probe `CLR caller=` data.`
- Trigger: A maintainer reads `patch_manager_mods/30_queue_hud.md` (lines 5, 19-20, 152, 170) or `patch_manager_mods/queue_hud_src/queue_hud.ini` (line 67) alongside `patch_manager_mods/queue_hud_src/QueueHudLogic.h`:
  1. `30_queue_hud.md:152` states that at hand-off, the first dispatch is preceded by `ServerClearAllActions` by default (`handoff_clear_first` default 1), without mentioning `handoff_keep_new` (`bool handoffKeepNew = true;` at `QueueHudLogic.h:197` and `handoff_keep_new=1` at `queue_hud.ini:48`), which makes `qh::clearBeforeHandoff(g_cfg.handoffClearFirst, h.sent, g_cfg.handoffKeepNew)` (`QueueHudLogic.h:141`) return `false` by default and skips `ServerClearAllActions`.
  2. `30_queue_hud.md:170` states `qh::heldHeadAction` re-sends an engine-held head only if released "within 300 ms of the edge (`qh::heldHeadAction`), else dropped", whereas `QueueHudLogic.h:146-155` replaced the 300 ms clock with `sawExec` and a `kHeldStuckMs = 3000` unpaused held-time cap.
  3. `queue_hud.ini:67` (and `30_queue_hud.md:5, 19-20`) states `strip_icon_px = 0` on `left` sizes icons to "the self-heal button side, 16..24 px" (or "40% of vit.h (38 px)"), whereas `QueueHudLogic.h:230-246` (`autoIconPx`) computes 60% of `vit.h` clamped to `[48, 72]` (58 px on the live 96 px bar).
- Impact: The documentation misleads maintainers about whether `ServerClearAllActions` runs at combat hand-off under default settings, how `heldHeadAction` settles held commands, and how companion strip icons are sized.
- Fix: Update `30_queue_hud.md` (lines 5, 19-20, 152, 170) and `queue_hud.ini:67` to reflect `handoff_keep_new=1` overriding `handoff_clear_first=1`, the `sawExec` + 3000 ms unpaused stuck rule in `heldHeadAction`, and the 60% `[48, 72]` auto icon size in `autoIconPx`.

## Fix check
- `a2ecc13` looks correct
- `ab456aa` R2-QH-06, R2-QH-09
- `528c59d` R2-QH-01, R2-QH-06, R2-QH-07
- `c7b205b` / `910b4aa` R2-QH-03, R2-QH-11
- `2d2dab0` R2-QH-10

## Retracted
- `QueueHudPerformUseItem` / `dispatchAction` replaying `a.pos` (zero vector) instead of `nullptr` when original `[ebp+0x14]` `posPtr` was `0` (`QueueHud.cpp:929, 1871`): dropped because `PerformUseItem` (`0x00775600`) has no evidence of distinguishing a null position pointer from `{0,0,0}`, unlike `PerformCastSpell`.
- Reentrancy between `QueueHudCapture` (`onHudFrame`), `onHudDraw`, and `onFrameBegin` (`QueueHud.cpp:752, 1604, 1791`): dropped because all three callbacks execute on the single main game thread during the HUD update/draw pass, and `g_dispatching` (`QueueHud.cpp:77, 904, 1814, 1901`) guards synchronous re-entry into H9/H10 during `dispatchAction`.
- Companion Cancel hit-test (`onFrameBegin` at `QueueHud.cpp:1636`) vs draw rectangle (`cwApply` at `QueueHud.cpp:441`): dropped because `cwApply` updates the label's engine extent via `guikit::setRect` and visibility via `cwShow` at the end of every `onHudDraw`, and `guikit::hit` reads the same control's extent and visibility flag.
- `handoffTick` `midFlight` check (`QueueHud.cpp:1104-1107`) ignoring `s.rn` `subType` 9/10: dropped because `handoffTick` intentionally queues up to `kMaxRound` (4) round nodes one per frame (guarded by `if (s.nrn + 1 > qh::kMaxRound) continue;` at line 1091).

## Not assessed
- `patch_manager_mods/queue_hud_src/override/lbl_miqueue_teal.tga` and `patch_manager_mods/_shared_src/HudKitTextures.inc`: skipped per scope instructions.
- Whether `CSWCCreature::PerformUseItem` (`0x00775600`) branches on `pos == nullptr` vs a pointer to `{0.0f, 0.0f, 0.0f}`: needs reverse engineering of `0x00775600`.
- Whether the engine allocator can reuse a freed `CSWCCreature*` address for a newly spawned creature within the 20 s `DR_IDENTITY_LOST` window across area transitions or save loads: needs reverse engineering of the engine's object heap.
