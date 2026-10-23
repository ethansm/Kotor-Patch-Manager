# RV3 spec: queue-hud hand-off and Cancel fixes (RQ8-RQ12 measure-first, RL6, RL11, RL17-RL19)
Mod(s) + version bump: queue-hud 0.4.7-dev -> 0.4.8-dev (patch_manager_mods/queue_hud_src/manifest.toml:4 `version =`; queue_hud.ini:1 header; QueueHud.cpp KPatchInit `KP_LOG(chMain, "0.4.7-dev: kit=...` string, currently "0.4.5-dev" at ~:2225 on an unmodified tree: find with `rg -n '0\.4\.[57]-dev' patch_manager_mods/queue_hud_src`). Stale "0.4.x" in the QueueHud.cpp:1 comment stays (RV6 docs).
Files: QueueHud.cpp, QueueHudLogic.h, queue_hud_logic_test.cpp, queue_hud.ini, `scripts/queue_hud_log_check.py` (+ `scripts/test_queue_hud_log_check.py`), PLAY_TEST_QUEUE.md. No `_shared_src` edit, no new engine address (compat_check not needed). RQ12 is MEASURE-FIRST (Opus decision after review R1): `ServerClearAllActions` stays in `doCancel`, only logging is added.
Line numbers below are from the UNMODIFIED tree; RV1/RV2 shift them. Anchor by function name / `rg`. Engine facts used: GameAddr.h:92-93 `ActListRemoveHead 0x7F2A90` (ECX = list, returns data ptr, frees the link node), `ActListAddTail 0x54AA30`; `ServerClearAllActions 0x51CB10`. NO per-node remove exists in the repo (`rg ClearAction GameAddr.h` empty; 0x56E620 appears only in R7 dumps, ABI unverified), so RQ12 does no list surgery.

## Changes

### RL11: handoff_clear_first no longer gates RoundOrder arming (do first, RQ6-style small edit; RL6 builds on it)
- Files/anchors: QueueHudLogic.h:148 `keepNewAtHandoff`; QueueHud.cpp:1415 (handoffTick keep block); queue_hud.ini:56-57 comment; queue_hud_logic_test.cpp:808-811.
- Before: `inline bool keepNewAtHandoff(bool cfgOn, bool keepNew, int sentSoFar) { return cfgOn && keepNew && sentSoFar == 0; }` and `if (qh::keepNewAtHandoff(g_cfg.handoffClearFirst, keepNow, h.sent)) {`
- After: `inline bool keepNewAtHandoff(bool keepNow, int sentSoFar) { return keepNow && sentSoFar == 0; }` and `if (qh::keepNewAtHandoff(keepNow, h.sent)) {` (block body is rewritten under RL6). `clearBeforeHandoff` unchanged (already `cfgOn && !keepNew && sent==0`).
- ini: `; Clear the engine queue for the hand-off member before its first re-sent command (1 = on). Only matters when handoff_keep_new=0 or an unpaused combat start with handoff_keep_unpaused=0; otherwise the queue is kept.`
- Edge cases: defaults (1,1,1) behave exactly as today; clear_first=0 now arms RoundOrder + logs HANDOFF_KEEP. Logging: none new.

### RQ8: restore an unsent hand-off head at combat end
- Files/anchors: QueueHudLogic.h after `struct ActionBuffer` (~:760, below `clear()`); QueueHud.cpp tickItemQueues non-combat path, the loop commented "Combat ended before a hand-off drained" (~:1740).
- Before:
```
    // Combat ended before a hand-off drained: the held head is dropped; the still-buffered commands run through the sequential dispatch below.
    for (int k = 0; k < 3; ++k)
        if (g_handoff[k].pending) endHandoff(k, qh::DR_COMBAT_CLEAR);
```
- After (Logic.h, new pure helper; bounded buffer, see Deviation):
```
// RQ8: combat ended while a hand-off still owned an unsent head (decided "re-send" but the owner was not in slot k / midFlight stalled / list full).
// Put it back at the FRONT of the buffer so the sequential dispatch sends it first. headHeld = the engine still owns it: not restored. A full buffer
// (kMaxActionQueue) loses its newest command (*dropped, caller logs DR_BUFFER_FULL) instead of growing the array.
enum RestoreHead : uint8_t { RH_NONE, RH_FRONT, RH_FRONT_DROPPED };
inline RestoreHead restoreHead(bool headValid, bool headHeld, const QueuedAction& head, ActionBuffer& buf, QueuedAction* dropped) {
    if (!headValid || headHeld) return RH_NONE;
    RestoreHead r = RH_FRONT;
    int n = buf.count;
    if (n >= kMaxActionQueue) { if (dropped) *dropped = buf.queue[kMaxActionQueue - 1]; n = kMaxActionQueue - 1; r = RH_FRONT_DROPPED; }
    for (int i = n; i > 0; --i) buf.queue[i] = buf.queue[i - 1];
    buf.queue[0] = head;
    buf.count = n + 1;
    buf.userCancelled = false;
    return r;
}
```
  QueueHud.cpp:
```
    // Combat ended before a hand-off drained. RQ8: an unsent re-send head goes back to the front of its buffer (the sequential dispatch below sends it);
    // an engine-held head stays the engine's and is dropped as before. The still-buffered commands run through the sequential dispatch below.
    for (int k = 0; k < 3; ++k) {
        Handoff& h = g_handoff[k];
        if (!h.pending) continue;
        // R2: the vanilla Cancel Combat button (BTN_CLEARALL) ends combat in the same press; a head the player just cancelled must not come back.
        // g_lastClickTick is only precise after RQ11 (apply RQ11 first; see Risks / ordering).
        if (qh::cancelClickRecent(g_lastClickTick, h.edgeAt, now)) {
            endHandoff(k, qh::DR_CANCEL); traceDropAll(k, qh::DR_CANCEL); g_itemQueue[k].clear();
            continue;
        }
        qh::QueuedAction lost;
        const uint32_t headCmd = h.head.cmdId;
        // RV1 EDGE_WAIT sets headHeld=true together with h.waiting ("not in the engine yet"): that head is still ours, so it is restorable. Without RV1 pass h.headHeld as is.
        const qh::RestoreHead rh = qh::restoreHead(h.headValid, h.headHeld && !h.waiting, h.head, g_itemQueue[k], &lost);
        if (rh != qh::RH_NONE) {
            h.headValid = false;   // restored: endHandoff must not log it as a DROP
            QLOG(chMain, "HANDOFF_RESTORE m=%d cmd=%u pos=front dropped=%u", k, (unsigned)headCmd, rh == qh::RH_FRONT_DROPPED ? (unsigned)lost.cmdId : 0u);
            if (rh == qh::RH_FRONT_DROPPED && g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, lost.cmdId, k, qh::DR_BUFFER_FULL, 1); QTRACE("%s", line); }
        }
        endHandoff(k, qh::DR_COMBAT_CLEAR);
    }
```
- Edge cases: a cancel click since the edge (vanilla Cancel Combat, or our Cancel) -> `DROP ... why=cancel`, buffer cleared, NO restore (R2; double-use of a cancelled consumable); leader switch (owner stays in `head.creature`; B1 `ownerCtrl` holds the buffer until the owner leads, then dispatches, no loss); head already sent (`headValid` false) -> RH_NONE; engine-held head (`headHeld && !waiting`) -> RH_NONE (never double-use); RV1 `waiting` head (not in the engine yet) -> restored; full buffer -> newest dropped + trace DROP; unreadable creature is re-checked by the normal dispatch `probeRead`. After RV1: RV1 adds `Handoff.waiting/waitUntil` (plan_RV1 RQ3); `headHeld && waiting` means "not in the engine yet" and is restored via the `!h.waiting` argument above (drop `&& !h.waiting` only if RV1 is not applied).
- Logging: default level `HANDOFF_RESTORE m=<k> cmd=<n> pos=front dropped=<cmd|0>` (unknown keyword: `queue_hud_log_check.py` ignores it, header line 15). Trace `DROP ... why=buffer_full` only when a command was displaced.

### RQ10: Detached stores its owner; detachTick resolves the owner's slot (and RQ9 below shares this edit)
- Files/anchors: QueueHud.cpp:628 `struct Detached`; `detachRound` (~:1525) one line; `detachTick` (~:1547); QueueHudLogic.h after `reattachReady` (~:1232).
- Before: `struct Detached { uint32_t ptr[qh::kMaxRound] = {}; qh::RNode rn[qh::kMaxRound]; int n = 0; DWORD at = 0; };` and in detachRound `if (ok) { for (...) {...} d.n = s.nrn; d.at = now; }`
- After: `struct Detached { uint32_t ptr[qh::kMaxRound] = {}; qh::RNode rn[qh::kMaxRound]; int n = 0; DWORD at = 0; uintptr_t owner = 0; DWORD endAt = 0; };` (endAt: RQ9) and `d.n = s.nrn; d.at = now; d.owner = srv;` (srv is detachRound's server-creature parameter).
- Pure (Logic.h):
```
// RQ10: the slot a detached batch must be processed for: its owner's CURRENT slot (leader switches reorder the party), -1 = owner gone.
inline int detachTarget(int ownerSlot, int k) { (void)k; return (ownerSlot >= 0 && ownerSlot < 3) ? ownerSlot : -1; }
```
- detachTick resolution (replaces `const uintptr_t cli = party::member(tbl, k); const uintptr_t srv = ...; Snapshot s; if (!srv || !gatherFor(srv, s)) continue;`):
```
        const int j = qh::detachTarget(slotOfCreature(tbl, d.owner), k);   // RQ10: slotOfCreature matches client or server pointers
        if (j < 0) { discardDetached(k, "identity_lost", afterCombat); continue; }
        const uintptr_t srv = d.owner;
        Snapshot s;
        if (!gatherFor(srv, s)) continue;
```
  Every later use of `k` for gating/buffers inside the loop body becomes `j`: `busy = bufBusy(g_itemQueue[j]) || g_handoff[k].pending` (R5: only `g_itemQueue` and `g_lastOurs` follow the owner in the identity pre-pass swap, QueueHud.cpp:1710-1713; `g_handoff`, `g_roundOrder`, `g_detached` stay indexed by the slot the batch was detached under, so the hand-off flag uses `k`), `g_lastOurs[j]`, and in the cancel-click branch `endHandoff(k, ...); traceDropAll(j, ...); g_itemQueue[j].clear();`. `d` stays `g_detached[k]`. The reattach log `why` becomes `j == k ? "done" : "moved"`.
- Edge cases: stale owner pointer (creature freed/party change) -> slotOfCreature -1 -> discard `identity_lost`; owner moved slot -> nodes go back into the OWNER's round list (never another creature's); `drawDetachedSlots` still reads `g_detached[0]` (RL22, not here). Open question 3 (leader switch mid-combat) is only hit on the PLAY row; after a leader switch `busy` mixes the owner's buffer (slot j) with the hand-off flag of the detach slot (k), which only causes extra waiting or an early reattach, bounded by the 5 s / RQ9 20 s timers (R5).

### RQ9: reattach-or-discard detached nodes at combat end
- Files/anchors: `traceDetach`, `discardDetached` (+ its forward declaration ~:1296 `void discardDetached(int k, const char* why);`), `detachTick`; tickItemQueues non-combat path (right after the RQ8 loop).
- After (signatures; default arg ONLY on the forward declaration, the definitions repeat the parameter without it):
```
constexpr DWORD kDetachEndGraceMs = 20000;   // after combat: how long a detached batch may wait for the buffer to drain before it is discarded
void traceDetach(int k, int n, const char* act, bool ok, const char* why, bool always = false) {
    if (!g_logOn || (!always && !g_cfg.traceQueue)) return;
    char line[160];
    qh::fmtRoundDetach(line, sizeof line, k, n, act, ok, why);
    if (always) QLOG(chMain, "%s", line); else QTRACE("%s", line);
}
void discardDetached(int k, const char* why, bool always = false) {   // always = default-level line (combat-end paths)
    Detached& d = g_detached[k];
    if (d.n == 0) return;
    traceDetach(k, d.n, "discard", true, why, always);
    d = Detached();
}
void detachTick(DWORD now, bool afterCombat = false);   // forward declaration; afterCombat: called from the non-combat path
```
  detachTick body changes (everything not listed stays): after `if (d.n == 0) continue;` add `if (afterCombat && d.endAt == 0) d.endAt = now;`; pass `afterCombat` to the `timeout` discard; keep the `quiet && cancelClickRecent(g_lastClickTick, d.at, now)` block ACTIVE in the afterCombat call too, with `discardDetached(k, "cancel_click", afterCombat)` (R6: the vanilla Cancel Combat press ends combat and clears the queue in one click, so after combat the click IS the Cancel; without the guard the cancelled queue's detached nodes would be reattached and run once the buffer drains); the reattach logging `traceDetach(j_or_k..., "reattach", ok, ..., afterCombat)` with `why = afterCombat ? "combat_end" : (j == k ? "done" : "moved")` (on `!ok`: "guard"); and at the end of the loop body, in place of the old `else if (now - d.at > 5000) discardDetached(k, "reattach_guard")`:
```
        if (ok) d = Detached();
        else if (static_cast<int>(now - d.at) > 5000) discardDetached(k, "reattach_guard", afterCombat);   // never wedge on an unwritable list
```
  and just before `if (!qh::reattachReady(...)) continue;` turn the `continue` into:
```
        if (!qh::reattachReady(busy, midFlight, oursNodes, sinceSend, s.nrn, d.n)) {
            if (afterCombat && now - d.endAt > kDetachEndGraceMs) discardDetached(k, "combat_end_timeout", true);
            continue;
        }
```
  Call site (tickItemQueues non-combat path, after the RQ8 loop, before the per-member dispatch loop): `detachTick(now, true);` (no-op unless a `g_detached[k].n > 0`). The combat call `detachTick(now)` in the `inMenu || g_combat != 0` branch is unchanged.
- Edge cases: Cancel Combat click (R6) -> `discard ... why=cancel_click`, never reattach; reattach waits for the restored head/buffered commands to drain (`busy` = j's buffer), so the player's click runs AFTER the hand-off commands (same order rule as in combat); list full -> `reattachReady` false -> wait/timeout; new combat before reattach -> existing `discardDetached(k,"new_combat")` at the edge; menu open -> tickItemQueues returns early, grace timer keeps running on real time; save/load: pointers die with `owner` check (slotOfCreature fails -> identity_lost).
- Logging (deviation from doc 47 text `act=combat_end_*`): the log checker regex allows only `act=detach|reattach|discard` (queue_hud_log_check.py:125), so lines are `ROUND_DETACH m=<k> n=<n> act=reattach ok=1 why=combat_end` and `act=discard ok=1 why=combat_end_timeout|identity_lost|reattach_guard`, default level (INFO).

### RQ11: Cancel click recorded only when it hits a Cancel control
- Files/anchors: QueueHud.cpp:67 (constants; add `#include <initializer_list>` to the includes, needed for the range-for over a braced list in `cancelClickAt`, R4), :1237 `g_lastClickTick` decl, onFrameBegin (~:2053) `if (ledge) g_lastClickTick = GetTickCount();`; QueueHudLogic.h after `stripCancelRect` (~:395).
- Pure (Logic.h):
```
// RQ11: is (x,y) inside any of the n Cancel rects (queue-hud's own, vanilla BTN_CLEARALL / BTN_CLEARONE / BTN_CLEARONE2)? Half-open, empty rects never match.
inline bool clickIsCancel(const Rect* rects, int n, int x, int y) {
    for (int i = 0; rects && i < n; ++i)
        if (rects[i].w > 0 && rects[i].h > 0 && x >= rects[i].x && x < rects[i].x + rects[i].w && y >= rects[i].y && y < rects[i].y + rects[i].h) return true;
    return false;
}
```
- QueueHud.cpp, after `constexpr int HudSlotBase...` (:67): `constexpr int HudClearAll = 0x6C3C, HudClearOne = 0x689C, HudClearOne2 = 0x6A6C;   // vanilla BTN_CLEARALL / CLEARONE / CLEARONE2 control bases (R7 FINDINGS sec 1; = the kBannerCtl flag offsets 0x6C84 / 0x68E4 / 0x6AB4 minus 0x48)`. Helper placed before onFrameBegin:
```
// RQ11: does a left press at engine px (x,y) land on a Cancel control (ours, or the vanilla combat Cancel / clear-one boxes)? Only these count as the player's cancel.
static bool cancelClickAt(int x, int y) {
    qh::Rect r[6]; int n = 0;
    for (const CW& w : g_cw) if (w.c && w.vis == 1 && n < 6) r[n++] = w.r;
    if (g_hud) for (int off : { HudClearAll, HudClearOne, HudClearOne2 }) {
        qh::Rect q{}; const uintptr_t c = g_hud + off;
        if (n < 6 && guikit::isVisible(c) && guikit::getExtent(c, &q.x, &q.y, &q.w, &q.h)) r[n++] = q;
    }
    return qh::clickIsCancel(r, n, x, y);
}
```
- Before: `if (ledge) g_lastClickTick = GetTickCount();`  After:
```
    if (ledge) {
        int cx = 0, cy = 0;
        if (!guikit::cursorEngine(&cx, &cy) || cancelClickAt(cx, cy)) g_lastClickTick = GetTickCount();   // RQ11: unknown cursor keeps the old behaviour; other clicks never read as Cancel
    }
```
- Evidence (R4): control bases are static-RE values (R7 FINDINGS.md:10-12, doc 30_queue_hud.md:180-182) and equal the `kBannerCtl` flag offsets 0x68E4/0x6AB4/0x6C84 minus 0x48 (QueueHud.cpp:2108-2110), which are already written live; `isVisible`/`getExtent`/`cursorEngine` exist (GuiKit.h:227, 267, 484); `CW` has `c/vis/r`. Known non-regression: Esc/pad cancel (0x7469F0 case 0x28) is not a mouse press and was never recorded.
- Edge cases: queue-hud's Cancel is hidden in hard combat, so the combat-time cancel is the vanilla control; vanilla controls are only visible in combat (R7 sec 1), so out of combat only our CW rects match; click on enemy/world -> not recorded -> a vanished head without exec evidence is RESEND (`heldHeadAction` unchanged). `doCancel` itself does not depend on `g_lastClickTick`. `cancelClickRecent` unchanged (still `> edgeAt`, 600 ms).
- Logging: none (CLICK trace line already prints x/y, `trace_clicks`).

### RQ12 (MEASURE-FIRST): does a companion Cancel lose its FollowLeader 0x3D? Log it, change nothing
- Decision (Opus, after review R1): the earlier "pop the whole queue, re-add only the 0x3D, drop round nodes" surgery is NOT built. It leaks live cr+0x100 entries and round nodes, skips `CServerExoApp::ClearAllActions` 0x51CB10 (which also runs `0x541080`/`CSWSCreature::ClearAction` 0x56E620 per node and `RemoveAllActions` 0x590430, and resets `[round+0xB20]`, `[cr+0x548]`, `[cr+0x568]`, `[cr+0x56c]`), and its premise (the follower AI does not re-add 0x3D) is unmeasured. No `cancelRoute`, `cancelKeepFollow`, `cancel_keep_follow`, `T_FOLLOW` Logic.h constant, pop/re-add code or `roundListHandle` forward declaration exists in this spec. `ServerClearAllActions` stays in `doCancel` for every k.
- Files/anchors: QueueHud.cpp `queueCountOf` (:915), `doCancel` (:941-967), `struct CPending` (:397), onFrameBegin `CANCEL ... act=done` (:2063-2064); `scripts/queue_hud_log_check.py` (+ `scripts/test_queue_hud_log_check.py`). No Logic.h, ini or Config change.
- queueCountOf: add 4th optional out param `int* n3D = nullptr`: `if (n3D) { int c = 0; if (ok) for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) if (s.e[i].type == 0x3D) ++c; *n3D = ok ? c : -1; }` (0x3D = AIActionFollowLeader, as in tickItemQueues' `hasFollowLeader`; -1 = unreadable). Before: `int queueCountOf(int k, bool* haveCur = nullptr, int* gateNe = nullptr) {`  After: `int queueCountOf(int k, bool* haveCur = nullptr, int* gateNe = nullptr, int* n3D = nullptr) {`.
- `struct CPending { bool on = false; int n = 0; int had3D = -1; DWORD at = 0; bool late = false; };` (Before: `struct CPending { bool on = false; int n = 0; };`).
- doCancel tail. Before (after the `bad` check):
```
    const int n = queueCountOf(k);
    QLOG(chCancel, "CANCEL m=%d act=click n=%d route=ServerClearAll", k, n);
    ...trace DROP loop...
    g_lastOurs[k].tag = "cancel"; g_lastOurs[k].tick = GetTickCount();
    g_itemQueue[k].cancel();
    reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ServerClearAllActions)(server, id);
    g_cpend[k].on = true; g_cpend[k].n = n;
```
  After:
```
    int had3D = -1;
    const int n = queueCountOf(k, nullptr, nullptr, &had3D);
    const int drawn = k == 0 ? g_heldLeader.nd : g_heldStrip[k].d.n;   // read BEFORE the RL17 reset below
    QLOG(chCancel, "CANCEL m=%d act=click n=%d drawn=%d had3D=%d route=ServerClearAll why=measure", k, n, drawn, had3D);
    ...trace DROP loop unchanged...
    g_lastOurs[k].tag = "cancel"; g_lastOurs[k].tick = GetTickCount();
    g_itemQueue[k].cancel();
    reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ServerClearAllActions)(server, id);   // unchanged: vanilla cleanup
    if (k == 0) g_heldLeader.nd = 0; else g_heldStrip[k] = HeldStrip();   // RL17: the hold repaint must not keep cancelled icons (and the Cancel control) for 400 ms
    g_cpend[k].on = true; g_cpend[k].n = n; g_cpend[k].had3D = had3D; g_cpend[k].at = GetTickCount(); g_cpend[k].late = false;
```
  `why=` must stay LAST on the CANCEL click line (the checker takes free text to the end of line: `_split_free_why`; `drawn=`/`had3D=`/`route=` are plain key=value tokens, no spaces in values, and unknown keys are not field-checked). Both `g_heldLeader` and `g_heldStrip` are declared before `doCancel` (:592, :747).
- onFrameBegin, Before: `if (g_cpend[k].on) { g_cpend[k].on = false; QLOG(chCancel, "CANCEL m=%d act=done n=%d", k, queueCountOf(k)); }`  After (two samples per Cancel: the next frame, and ~1500 ms later because the follower AI may re-add 0x3D a tick or seconds later):
```
        CPending& cp = g_cpend[k];
        if (cp.on) {
            const DWORD t = GetTickCount();
            if (!cp.late || t - cp.at >= 1500u) {
                int after3D = -1;
                const int nn = queueCountOf(k, nullptr, nullptr, &after3D);
                QLOG(chCancel, "CANCEL_AFTER m=%d n=%d had3D=%d after3D=%d late=%d", k, nn, cp.had3D, after3D, cp.late ? 1 : 0);
                if (cp.late) {
                    if (k >= 1 && cp.had3D > 0 && after3D == 0) QLOGV(chCancel, KPLOG_WARN, "CANCEL_LOST3D m=%d had3D=%d after3D=%d", k, cp.had3D, after3D);
                    cp = CPending();   // done
                } else cp.late = true;   // keep the entry for the ~1500 ms sample
            }
        }
```
  (loop body is the existing `for (int k = 0; k < 3; ++k)` over `g_cpend`; the `CANCEL ... act=done` line no longer exists.) The WARN uses the unknown keyword `CANCEL_LOST3D` (R3): the checker ignores unknown first words, whereas a `CANCEL`-prefixed warn without `act=` and with bare tokens would raise `Bad("bare token")` and count as SKIPPED. The line is emitted only on the late sample.
- Checker support (needed: `CANCEL_AFTER` must parse, not be ignored). `scripts/queue_hud_log_check.py`: REQUIRED add `"CANCEL_AFTER": ("m", "n", "had3D", "after3D", "late"),` (after the `"CANCEL"` row); FIELD_RE add `"had3D": r"-?\d+", "after3D": r"-?\d+", "late": r"[01]", "drawn": r"-?\d+",` (n and m already exist; `-1` = unreadable); docstring grammar block (after the `CANCEL m=<k> act=...` entry, line ~25) add: `  CANCEL_AFTER m=<k> n=<n> had3D=<n> after3D=<n> late=<0|1>   (RQ12 measurement: 0x3D entries before the Cancel click and on the next frame (late=0) / ~1500 ms later (late=1); -1 = unreadable; no lifecycle)` and extend the click entry with `[drawn=<n>] [had3D=<n>]`. No view/flag change (a CANCEL_AFTER line opens no lifecycle: `kw not in CMD_KWS`). `scripts/test_queue_hud_log_check.py`: add a helper `def cancel_after(m, n, had, after, late): return "CANCEL_AFTER m=%d n=%d had3D=%d after3D=%d late=%d" % (m, n, had, after, late)` next to `cancel_new` and in `CancelParseTests`: `test_cancel_click_measure_fields_and_cancel_after_parse`: parse `[cancel_new(1, "click", 2, " drawn=2 had3D=1 route=ServerClearAll why=measure"), cancel_after(1, 0, 1, 1, 0), cancel_after(1, 0, 1, 0, 1)]` -> `skipped == []`, kws `["CANCEL", "CANCEL_AFTER", "CANCEL_AFTER"]`, `recs[0]["f"]["had3D"] == "1"`, `recs[0]["f"]["why"] == "measure"`, `recs[2]["f"]["after3D"] == "0"`, `recs[2]["f"]["late"] == "1"`; bad forms `["CANCEL_AFTER m=1 n=0 had3D=1 after3D=x late=0", "CANCEL_AFTER m=1 n=0 had3D=1 after3D=1", "CANCEL_AFTER m=1 n=0 had3D=1 after3D=1 late=2"]` -> `recs == []`, reasons `["malformed after3D=x", "missing late", "malformed late=2"]`; and a `run_log([cancel_after(1, 0, 1, 0, 1)])` case asserting `out[0]` starts with `QUEUE-HUD lines=1 parsed=1 skipped=0 cmds=0` (no lifecycle).
- Edge cases: pause/unpause (reads only; the Cancel call is unchanged); leader (k=0): `had3D` is logged but the WARN is companion-only (the leader has no follower 0x3D); unreadable creature -> `had3D=-1`/`after3D=-1`, no WARN; two Cancels inside 1500 ms on the same slot restart the sample window (the entry is overwritten); a menu or combat frame delaying onFrameBegin only delays the sample (the late test is `>=`, wrap-safe unsigned subtraction).
- Logging: default level `CANCEL ... act=click ... drawn= had3D=`, `CANCEL_AFTER ... late=0|1`, WARN `CANCEL_LOST3D`. What the PLAY row decides: `late=1 after3D=0` and the companion stands still -> the follower AI does NOT re-add 0x3D after a full clear; then open a WI for a proper fix (needs a Ghidra/REA pass on `CSWSCreature::ClearAction` 0x56E620 and the follow re-add, plus a verified per-node remove address) rather than ActListRemoveHead surgery.
- Exception (only if the owner insists on shipping a real RQ12 before the measurement): acceptable ONLY behind a `cancel_keep_follow=0` default-OFF ini key plus these measurement lines; never default-on.

### RL6: grow the RoundOrder foreign set with in-combat clicks during a hand-off
- Files/anchors: QueueHud.cpp `struct RoundOrder` (:1224), handoffTick keep block (:1415-1427), `roundOrderTick` (:1455), hooks `QueueHudPerformUseItem` / `QueueHudPerformCastSpell` `if (g_combat != 0) {` blocks (:2269, :2356), `g_lastClickTick` decl (:1237); QueueHudLogic.h after `roundPartition`.
- Pure (Logic.h): 
```
// RL6: of nNew round nodes first seen this frame, how many are the player's in-combat clicks (the last ones): needs a click since the hand-off began,
// minus the nodes our own send of the previous frame created.
inline int lateForeignCount(int nNew, bool clickPending, int ownSends) {
    if (!clickPending || nNew <= 0) return 0;
    const int n = nNew - ownSends;
    return n > 0 ? n : 0;
}
```
- Hooks: both `if (g_combat != 0) {` blocks add first line `g_combatClickTick = GetTickCount();` (the hooks already `return 0` for `g_dispatching`, so only real player clicks get here); decl next to `g_lastClickTick`: `DWORD g_combatClickTick = 0;   // RL6: last player Force power / item click made in combat (tick)`.
- `struct RoundOrder { uint32_t foreign[qh::kMaxRound] = {}; int n = 0; DWORD armedAt = 0; bool watch = false; uint32_t seen[qh::kMaxRound] = {}; int nSeen = 0; DWORD lastScan = 0, clickConsumed = 0; };`
- handoffTick keep block, Before (body of `if (qh::keepNewAtHandoff(...)) {`): `if (s.nrn > 0) { RoundOrder& ro = g_roundOrder[k]; ro = RoundOrder(); for (...) ro.foreign[ro.n++] = s.rn[i].ptr; ro.armedAt = now; }` then the HANDOFF_KEEP trace. After:
```
                RoundOrder& ro = g_roundOrder[k];
                ro = RoundOrder();
                for (int i = 0; i < s.nrn && i < qh::kMaxRound; ++i) { ro.foreign[ro.n++] = s.rn[i].ptr; ro.seen[ro.nSeen++] = s.rn[i].ptr; }
                ro.watch = true; ro.armedAt = now; ro.lastScan = now;   // RL6: watch for later clicks even when nothing was pending at the first send
```
  (HANDOFF_KEEP trace unchanged.)
- roundOrderTick: `if (ro.n == 0) continue;` -> `if (ro.n == 0 && !ro.watch) continue;`; after `if (!srv || !gatherFor(srv, s)) continue;` insert:
```
        if (ro.watch) {   // RL6: a node that appears while the hand-off runs, after a combat click, is the player's: order it last too
            int newIdx[qh::kMaxRound], nNew = 0;
            for (int i = 0; i < s.nrn; ++i) {
                bool seen = false;
                for (int j = 0; j < ro.nSeen; ++j) seen = seen || ro.seen[j] == s.rn[i].ptr;
                if (!seen) newIdx[nNew++] = i;
            }
            const bool clickPending = g_combatClickTick != 0 && g_combatClickTick != ro.clickConsumed && static_cast<int>(g_combatClickTick - ro.armedAt) >= 0 && now - g_combatClickTick < 1500u;
            const int ownSends = (g_lastOurs[k].tag && !strcmp(g_lastOurs[k].tag, "handoff") && static_cast<int>(g_lastOurs[k].tick - ro.lastScan) >= 0) ? 1 : 0;
            const int add = qh::lateForeignCount(nNew, clickPending, ownSends);
            for (int a = nNew - add; a < nNew && ro.n < qh::kMaxRound; ++a) ro.foreign[ro.n++] = s.rn[newIdx[a]].ptr;
            if (add > 0) ro.clickConsumed = g_combatClickTick;
            ro.nSeen = 0;
            for (int i = 0; i < s.nrn; ++i) ro.seen[ro.nSeen++] = s.rn[i].ptr;
            ro.lastScan = now;
        }
```
  and `if (nForeignPresent == 0) { ro = RoundOrder(); continue; }` -> `if (nForeignPresent == 0) { if (!(ro.watch && g_handoff[k].pending)) ro = RoundOrder(); continue; }`.
- Edge cases: attack clicks (no Perform* hook) are only covered if already pending at the first send (limit, document in the ini comment of handoff_keep_new is NOT needed); both a send and a click in one window -> the LAST new node is taken as the click (list append order), worst case a mis-ordered pair, never a lost node; cap kMaxRound (4) = engine cap; `watch` ends with the hand-off (`pending` false) once no foreign node remains. Logging: existing `ROUND_ROT` line covers the reorder.

### RL17 / RL18 / RL19: Cancel visuals
- RL17: done in the RQ12 `doCancel` tail (`g_heldLeader.nd = 0` / `g_heldStrip[k] = HeldStrip()`); both are declared before `doCancel`.
- RL18: Logic.h: 
```
// RL18: why a Cancel control is hidden this frame (nullptr = shown): off by config, no queue (nothing wanted), or the strip is shown but has no room for the square.
inline const char* cancelHideWhy(bool cancelOn, bool show, int rectW) {
    if (!cancelOn) return "cancel-off";
    if (!show) return "no-queue";
    return rectW <= 0 ? "no-room" : nullptr;
}
```
  drawStrips (:836) Before: `g_want[k].show = g_want[k].r.w > 0;` After: `g_want[k].show = true;` (leader path :602 unchanged). cwApply (:485) Before: `if (!g_cfg.cancel || !d.show || d.r.w <= 0) { cwShow(w, false, !g_cfg.cancel ? "cancel-off" : "no-queue"); continue; }` After: `if (const char* hid = qh::cancelHideWhy(g_cfg.cancel, d.show, d.r.w)) { cwShow(w, false, hid); continue; }`. modeEmit (:880) add before the final `else`: `else if (!strcmp(g_cwWhy[k], "no-room")) n.cancel = "hidden:no-room";` and `else if (!strcmp(g_cwWhy[k], "stale capture")) n.cancel = "hidden:stale";`. `scripts/queue_hud_log_check.py:103`: `MODE_STATE = r"shown|hidden:(?:combat|menu|empty|stale|off|no-room)|\?"` and the docstring list at :27; add a MODE line with `cancel=hidden:no-room` to `scripts/test_queue_hud_log_check.py` next to the existing MODE parse test.
- RL19: onFrameBegin, Before: `if (!g_cfg.cancel || inMenu || hardCombat) { cwHideAll(...); } else {` After:
```
    if (!g_cfg.cancel || inMenu || hardCombat) {
        cwHideAll(inMenu ? "hud-inactive" : hardCombat ? "combat" : "cancel-off");
    } else if (GetTickCount() - g_capTick >= 250u) {
        cwHideAll("stale capture");   // RL19: HUD_DRAW stopped (dialogue / cutscene / load): a leftover Cancel must not swallow clicks
    } else {
```
  (the `else` body is unchanged: tooltips, hover, `doCancel`.) Logging: `VIS cancelN shown=0 why=stale capture` (existing visLog, change-gated).

## Tests
- Unit, all in queue_hud_logic_test.cpp, new `void testRV3()` called from `main()` after `testServerRouteQL20();`, plus edits to existing tests:
  - Edit `testHandoffClearFirstIni` (:808-811) to the new 2-arg signature: `KT_CHECK(keepNewAtHandoff(true, 0)); KT_CHECK(!keepNewAtHandoff(true, 1)); KT_CHECK(!keepNewAtHandoff(false, 0)); KT_CHECK(clearBeforeHandoff(true, 0, false) && !clearBeforeHandoff(true, 0, true));` (clear_first=0 + keepNow=true is now `keepNewAtHandoff(true,0)`).
  - restoreHead (`makeItemAction(0x1000, id, 0, 1, nullptr, 1, cmd)`): empty buf + head cmd 7 -> RH_FRONT, count 1, queue[0].cmdId 7; buf [11,12] + head 7 -> RH_FRONT, order 7,11,12; full buf [11,12,13] + head 7 -> RH_FRONT_DROPPED, *dropped.cmdId 13, order 7,11,12, count 3; headValid=false -> RH_NONE and buf unchanged; headHeld=true -> RH_NONE.
  - detachTarget: (1,1)=1, (2,1)=2, (0,2)=0, (-1,1)=-1, (3,0)=-1.
  - clickIsCancel (rects {100,100,20,20}, {0,0,0,0}, {300,50,10,10}): (100,100) T, (119,119) T, (120,119) F, (99,100) F, (0,0) F (empty rect), (305,55) T, n=0 F, nullptr F.
  - lateForeignCount: (2,true,1)=1, (1,true,1)=0, (3,true,0)=3, (1,false,0)=0, (0,true,0)=0; roundPartition with the grown set: ptrs {10,20,30}, foreign {20} -> order {0,2,1} moved 2; foreign {20,10} -> order {2,0,1} moved 3.
  - cancelHideWhy: (false,true,10) "cancel-off"; (true,false,0) "no-queue"; (true,true,0) "no-room"; (true,true,24) nullptr (compare with `== nullptr`).
  - Python (RQ12 measure-first): `python3 scripts/run_tests.py scripts/test_queue_hud_log_check.py` covers the new `CANCEL_AFTER` / click-measure parse tests and the `hidden:no-room` MODE golden (RL18). No Config, `configEqual` or `CFG_FLIP` change in this spec.
  - stripCancelRect itself is unchanged (existing vectors at queue_hud_logic_test.cpp:~1289-1349 cover the empty-rect return); RL18 only changes the label.
- PLAY rows to append to patch_manager_mods/PLAY_TEST_QUEUE.md (`pending`, after RV install; trace_queue=1 is the shipped default):
  - RV3-1 (RQ8): combat start with a buffered grenade/medpac on the leader, switch leader in combat (Tab) before the head releases, kill the enemies. Pass: `HANDOFF_RESTORE m=0 cmd=<n> pos=front dropped=0`, then `dispatching buffered ... cmd=<n>`, no `DROP ... why=combat_clear` for that cmd, one item used.
  - RV3-1b (RQ8, R2): unpaused combat start with a buffered grenade on the leader; click the vanilla Cancel Combat button before the head releases. Pass: `DROP ... why=cancel`, no `HANDOFF_RESTORE`, the grenade is not used after combat (needs RQ11 so `g_lastClickTick` is precise).
  - RV3-2 (RQ9/RQ10): grenade as combat trigger, click a Force power right after, end combat at once. Pass: `ROUND_DETACH ... act=reattach ok=1 why=combat_end` (or `act=discard ... why=combat_end_timeout|identity_lost`) within 20 s of combat end, never silence; the power fires after the buffered commands. Bonus: Tab switch in combat -> `reattach ... why=moved` or `discard why=identity_lost`, never in the wrong creature.
  - RV3-3 (RQ11): same combat trigger, click an enemy ~200 ms after the edge. Pass: `HANDOFF_HELD ... act=resend`, no `why=cancel_click`, buffered commands survive; clicking the vanilla Cancel Combat button still logs `why=cancel_click`. Also (R4) enable `trace_clicks=1` and record one `CLICK` trace line (x/y) taken ON the vanilla Cancel Combat button; compare with the extent the control reports (`guikit::getExtent` of hud+0x6C3C): a mismatch means the static-RE offset is wrong (the old QL10 "Cancel needs two clicks" symptom returns).
  - RV3-4 (RQ12, measure-first): Atton (not leader), pause, queue 2 items/powers on him, click his strip X, unpause, walk 10 s. Pass = information only: record `CANCEL m=1 act=click ... had3D=<n>`, `CANCEL_AFTER m=1 ... after3D=<n> late=0` and `CANCEL_AFTER ... late=1`. If `late=1 after3D=0` AND Atton stands still, open a WI for a proper RQ12 (Ghidra/REA pass on `CSWSCreature::ClearAction` 0x56E620 + the follow re-add + a verified per-node remove address); if `after3D>=1` or he keeps following, close the question (ServerClearAll is fine). `CANCEL_LOST3D` WARN appears only on the late sample.
  - RV3-5 (RL6): hand-off with 2 buffered powers, click a third power between the two sends. Pass: it executes last; `ROUND_ROT` lines show `moved>0`.
  - RV3-6 (RL17-19): X on a 2-item queue: strip gone within a frame (`MODE ... cancel=hidden:empty`); open a dialogue with a queue: `VIS cancel0 shown=0 why=stale capture`; strip_side=left near the screen edge: `cancel=hidden:no-room` (trace_mode).

## Tooling
- `python3 scripts/run_tests.py queue_hud` (runs queue_hud_logic_test + scripts/test_queue_hud_log_check.py); then individually `python3 scripts/run_tests.py queue_hud_logic_test` after each edit group.
- Build/gate: `scripts/pre_install_gate.sh --mod queue_hud_src` (GATE PASS required; a failure is investigated, never called pre-existing). No `_shared_src` change, so no `compat_check.py`.
- Log-check smoke after a play session: `python3 scripts/kotor2_logs.py check queue-hud --new`.
- Commit explicit paths only: `git commit -- patch_manager_mods/queue_hud_src/{QueueHud.cpp,QueueHudLogic.h,queue_hud_logic_test.cpp,queue_hud.ini,manifest.toml} scripts/queue_hud_log_check.py scripts/test_queue_hud_log_check.py patch_manager_mods/PLAY_TEST_QUEUE.md`.

## Risks / ordering
- Apply order: RL11 -> RQ11 (so `g_lastClickTick` is precise before the RQ8 / RQ9 cancel guards) -> RQ8 -> RQ10+RQ9 (one edit of Detached/detachTick) -> RQ12 (measure-first, + RL17) -> RL6 -> RL18 -> RL19 -> version bump -> tests. RL6 edits the same keep block as RL11; RQ8/RQ9 edit adjacent loops in the non-combat path (RQ8 first, `detachTick(now, true)` after it).
- RV1 conflicts: RV1 rewrites the handoffTick edge block (`h.headValid/headHeld` derivation) and adds `Handoff` fields; RQ8 reads `headValid && !(headHeld && !waiting)` at combat end (R2): re-check that meaning after RV1, `waiting` is plan_RV1's field. RV1 `detachableMask` edits `detachRound` (the `d.owner = srv` line goes in its success branch). RV2 may add a round-list pop helper and edit `tickItemQueues` near the same non-combat loop; 
- RQ12 risk: none for the engine (reads and log lines only; `ServerClearAllActions` unchanged). If the measurement shows the companion loses 0x3D and stands still, the fix is a separate WI with RE (no ActListRemoveHead surgery on unverified node types).
- RQ11 risk: the 3 vanilla control offsets are static-RE values (R7); if `isVisible` reads wrong the click is simply not recorded and a vanilla Cancel Combat click falls back to the 600 ms window never matching: the cost is the old QL10 "Cancel needs two clicks" symptom. Mitigate by checking the `CLICK` trace x/y against the vanilla button in the RV3-3 row.
- Open engine questions (do not block): (2) does the follower AI re-add 0x3D after a full clear (decides whether a real RQ12 is ever needed; `after3D` at `late=0` and `late=1` measures it); (3) can the leader switch mid-combat (RQ10 reach; after a switch RQ10's `busy` mixes slot j's buffer with slot k's hand-off flag, R5: only extra waiting or an early reattach, bounded by the 5 s / 20 s timers); (5) `ClearAction` side effects: moot now, no skipped call.

## Review corrections applied
- R1 (BLOCKING): RQ12 replaced by measure-first: ServerClearAllActions kept; cancelRoute/CancelRoute/T_FOLLOW/cancelKeepFollow/cancel_keep_follow/pop-re-add code removed; `queueCountOf` n3D out param; click line gains `drawn= had3D=`; `CANCEL_AFTER` at next frame and ~1500 ms (`late=0|1`); checker REQUIRED/FIELD_RE + tests added; PLAY RV3-4 reworded to a measurement.
- R2 (BLOCKING): RQ8 loop gets the `cancelClickRecent` guard (DR_CANCEL, buffer cleared, no restore) and passes `h.headHeld && !h.waiting`; PLAY row RV3-1b; RQ11 now applies before RQ8.
- R3 (MINOR): WARN renamed `CANCEL_LOST3D` (unknown keyword for the checker, no bare tokens), emitted on the late sample only.
- R4 (MINOR): RQ11 adds `#include <initializer_list>`, control-offset evidence note, RV3-3 PLAY row requires a CLICK trace on the vanilla Cancel Combat button.
- R5 (MINOR): RQ10 detachTick `busy = bufBusy(g_itemQueue[j]) || g_handoff[k].pending`; `endHandoff(k)` / `traceDropAll(j)` split; sentence added to open question 3.
- R6 (MINOR): RQ9 keeps the `quiet && cancelClickRecent` discard active in the afterCombat call (`cancel_click`, default-level line).
- R7, R8 (RV6 items): applied in plan_RV6.md. R9: verified OK, no change.
