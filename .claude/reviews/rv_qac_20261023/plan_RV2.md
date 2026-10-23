# RV2 spec: queue-hud verdict fixes (RQ4 RQ6 RQ7 RL1 RL2 RL3 RL5 RL8)
Mod: queue-hud 0.4.6-dev (RV1) -> 0.4.7-dev. Bump in 3 places (`rg -n "0\.4\.6-dev" patch_manager_mods/queue_hud_src`): `manifest.toml:4`, `queue_hud.ini:1` header, `QueueHud.cpp` startup `KP_LOG(chMain, "0.4.6-dev: kit=...` (~2225). No new ini keys (constants only).
Apply AFTER RV1. Anchor every edit by the quoted text, not by line number (RV1 shifts lines). Before-snippets that touch RV1 spots are quoted as they read AFTER RV1's After-snippets. Do not touch RV1's `reInjectWhy` gates, `dispatchAction`'s `SendResult`, `DR_REFUSED`/`DR_EXECUTED`. Also edited: `scripts/queue_hud_log_check.py` + `scripts/test_queue_hud_log_check.py` (flag C, R8). Files: `QueueHudLogic.h` (QL), `QueueHud.cpp` (QH), `queue_hud_logic_test.cpp` (QT), all in `patch_manager_mods/queue_hud_src/`. Build uses `-Werror`: the tick `switch (av)` MUST get the new case.

## Deviations from doc 47 (read first)
- **RQ6 hook half is dead code.** Both hooks return at the top when `g_combat != 0` (`QueueHudPerformUseItem` / `QueueHudPerformCastSpell`: `traceCmd(... "pass_combat" ...); return 0;`), so `DECIDE_DISPATCH_NOW` with `inCombat` never runs and in-combat clicks are never tracked (V1-2's premise is wrong, and no overwrite can happen: DISPATCH_NOW needs `!buf.active`). Do NOT edit the hooks. The real reach is a buffer left `active` across a combat: the B1 hold in `handoffTick` (`continue` after `handoffHoldTrace`) keeps `buf.active`; at combat end it is judged with unknown history. Implement only the combat_end resolve (below).
- **RQ4 "eaten -> re-inject" needs `buf.eaten = true`.** After the orphan list is removed an unpaused absent action hits verdict rule 3 (`!eaten && !paused` -> AV_UNSEEN = silently lost). ORPHAN therefore latches `buf.eaten = true` so rules 4-8 can re-send it (only when `pause_reinject=1`; with 0 the existing mapping gives UNSEEN, as for any eaten verdict).
- **RQ7 interpretation of "expanded but unfired".** A matching 0x0F/0x2E node is NOT `executing` any more (it is the very thing that made a wipe read as EXECUTED); it sets new `ev.expanded` -> `buf.sawExpanded`, and rule 3 skips UNSEEN when `sawExpanded` (so a wipe after expansion is eaten-eligible). `executing` = matching cur node or a flag that ROSE after dispatch. Item ids: the item-id space of 0x2E/0x0F `param0` vs `clientItem` is unproven (no existing compare in code), so items match leniently (any 0x0F/0x2E, any 9/10 cur); only spells are matched by id. A trace line `EXEC_ID` logs the first item mismatch per cmd so it can be tightened later.
- **RL8 give-up also removes the starved round node** (same helper as ORPHAN, R3: keep-mask, only the active action's own node). Otherwise the abandoned 0x3F + node still fires when the member next leads (the 53 s late cast in the 10-23 log) and the DROP would be a lie. If removal fails the DROP still happens. The give-up never runs for a deferred (never sent) action or when no round node is the active action's own (a starved 0x3F of another mod, e.g. selfheal, is left alone).
- **RL8 cap**: `castStarvedAt` accepts the first 0x3F at index 1..`kMaxEntries-1` (was 1..3). Existing test `q5` flips 0 -> 4 (update it). Foreign-before-0x3F still returns 0.
- RL3 is FlagGate only; the 150/500 ms settle timers (`kVanishDebounceMs`, `kMinActiveMs`) stay wall-clock. H2-F10 (verdict skipped when `s.round == 0`) is left as is (OPEN).

## Changes
### QL constants (after `kReInjectResetMs`)
```cpp
constexpr int kMaxTotalReInjects = 12;          // RL5: verdict re-injects per action over its whole life (a present frame never resets it)
constexpr uint32_t kPresentCeilingMs = 30000;   // RQ4: unpaused ms an action may stay PRESENT before tracking is abandoned
constexpr uint32_t kOrphanMs = 1500;            // RQ4: unpaused ms a 9/10 round node may sit with no consumer entry and no real cur (= kStaleCurMs)
constexpr uint32_t kKickGiveUpMs = 5000;        // RL8: unpaused ms a server-route cast may stay starved (selfheal gives up at 5 s too)
constexpr uint32_t kClockGapMs = 500;           // UnpausedClock: a longer gap between two ticks (combat, menu, hitch) adds nothing
constexpr int kKickMaxAt = kMaxEntries - 1;     // RL8: deepest index of the 0x3F a kick may rotate to the front
```
### QL ActionBuffer (RL1, RL5, RQ4, RQ7)
- Append after `uint32_t lastVerdictTick = 0;`:
```cpp
    // RQ4/RQ7/RL5 (appended; cancel() and clear() reset every one of them)
    bool sawExpanded = false;        // a 0x0F/0x2E node of ours was seen: a wipe after it is eaten-eligible, not UNSEEN
    int flagBase = -1;               // flagBits() that were set at dispatch and have not read clear since; -1 = unknown
    uint32_t activeUnpausedMs = 0;   // unpaused ms since (re)dispatch: present ceiling
    int totalReInjects = 0;          // verdict re-injects of this action; never reset by a present frame
```
- In BOTH `cancel()` and `clear()` append: `sawExpanded = false; flagBase = -1; activeUnpausedMs = 0; totalReInjects = 0;`
- Before: `void markDispatched(uint32_t now) { activeStartTick = now; sawExecuting = false; eaten = false; pausedEaten = false; goneSinceTick = 0; }`
- After:
```cpp
    // A NEW action was sent: stamp the start, clear every per-action verdict counter (RL1: hand-off / overwrite exits left them spent) and record the flag baseline (-1 = unknown, the hooks).
    void markDispatched(uint32_t now, int flags = -1) {
        activeStartTick = now; sawExecuting = false; eaten = false; pausedEaten = false; goneSinceTick = 0;
        eatenReInjects = 0; eatenUnpausedMs = 0; lastReInjectTick = 0;
        sawExpanded = false; flagBase = flags; activeUnpausedMs = 0; totalReInjects = 0;
    }
    // The verdict re-sent the SAME action: restart the settle window only; the re-inject counters keep counting.
    void markReInjected(uint32_t now, int flags = -1) {
        activeStartTick = now; sawExecuting = false; eaten = false; pausedEaten = false; goneSinceTick = 0;
        sawExpanded = false; flagBase = flags; activeUnpausedMs = 0;
    }
    // Baseline shrinks to the bits still set: only a bit that read clear and came back (or never was set) counts as "rose" (RQ7).
    void noteFlags(int bits) { flagBase = flagBase < 0 ? bits : (flagBase & bits); }
```
- `accrueEaten` (in `QueueHudLogic.h`): replace exactly this block (literal; the two `pausedEaten` lines above it stay):
```cpp
        if (eaten && !paused) {
            const uint32_t d = now - lastVerdictTick;
            eatenUnpausedMs += d < 1000u ? d : 1000u;
        }
        lastVerdictTick = now;
```
  with:
```cpp
        const uint32_t d = now - lastVerdictTick, step = d < 1000u ? d : 1000u;
        if (eaten && !paused) eatenUnpausedMs += step;
        if (!paused) activeUnpausedMs += step;   // RQ4 present ceiling
        lastVerdictTick = now;
```
- `notePresent`: Before (literal) `void notePresent(bool executing, bool actionPresent, uint32_t now) {\n        if (executing) sawExecuting = true;\n        if (actionPresent && now - lastReInjectTick >= kReInjectResetMs) { eatenReInjects = 0; eatenUnpausedMs = 0; }\n    }`. After
```cpp
    void notePresent(bool executing, bool actionPresent, uint32_t now, bool expanded = false) {
        if (executing) sawExecuting = true;
        if (expanded) sawExpanded = true;
        if (actionPresent && now - lastReInjectTick >= kReInjectResetMs) { eatenReInjects = 0; eatenUnpausedMs = 0; }
    }
```
### QL FlagGate (RL3). Replace the whole `struct FlagGate {...};` (Before: members `holdSince, clearSince`, `reset()`, 3-arg `step`; keep the comment block above it and add "paused time inside a hold never counts toward the cap"); `step(busy, now, capped)` callers/tests keep working (new args default).
```cpp
struct FlagGate {
    uint32_t holdSince = 0, clearSince = 0, lastTick = 0, pausedMs = 0;   // pausedMs: paused time inside this hold (never counts toward the cap)
    bool holding = false, clearing = false, sawBusy = false;
    void reset() { holdSince = 0; clearSince = 0; lastTick = 0; pausedMs = 0; holding = false; clearing = false; sawBusy = false; }
    bool step(bool busy, uint32_t now, bool* capped = nullptr, bool paused = false) {
        if (capped) *capped = false;
        if (!holding) { holding = true; holdSince = now; lastTick = now; pausedMs = 0; }
        const uint32_t d = now - lastTick; lastTick = now;
        if (paused) {              // aa8/ab8 are sticky in a frozen round: once busy was seen keep holding, restart the clear run on unpause
            pausedMs += d; clearing = false;
            return !sawBusy;
        }
        if (busy) {
            sawBusy = true;
            clearing = false;
            if (now - holdSince - pausedMs >= kFlagBusyCapMs) { if (capped) *capped = true; return true; }
            return false;
        }
        if (!sawBusy) return true;
        if (!clearing) { clearing = true; clearSince = now; return false; }
        return now - clearSince >= kVanishDebounceMs;
    }
};
```
### QL new pure helpers (insert before `struct EngineView`)
```cpp
// Unpaused-time clock for "this condition has held for N ms" (RQ4 orphan, RL8 kick). Paused = frozen; a gap > kClockGapMs between ticks adds nothing.
struct UnpausedClock {
    uint32_t ms = 0, last = 0; bool run = false;
    void reset() { ms = 0; last = 0; run = false; }
    uint32_t tick(bool paused, uint32_t now) {
        if (paused) { run = false; return ms; }
        if (run) { const uint32_t d = now - last; if (d <= kClockGapMs) ms += d; }
        run = true; last = now;
        return ms;
    }
};
// An engine queue entry that can still consume a pending round node: the 0x3F placeholder or its expansion (0x0F cast / 0x2E item).
inline bool entryConsumesRound(int type) { return type == 0x3F || type == T_CAST_SPELL || type == T_USE_ITEM; }
// RQ4: age (unpaused ms) of an ORPHAN round node: a 9/10 node pending, no consumer entry, no real cur. Any other state resets the clock.
inline uint32_t orphanRoundAge(bool hasConsumer, bool creal, bool nrn910, bool paused, UnpausedClock& c, uint32_t now) {
    if (hasConsumer || creal || !nrn910) { c.reset(); return 0; }
    return c.tick(paused, now);
}
// RQ6: is an action started at activeStartTick newer than the last combat tick? (false = history unknown, stop tracking). 0 = no combat seen; wrap-safe; equal counts as after.
inline bool shouldTrackAfterCombat(uint32_t activeStartTick, uint32_t combatEndTick) {
    return combatEndTick == 0 || static_cast<int32_t>(activeStartTick - combatEndTick) >= 0;
}
// RQ7: aa8/ab8 as bits; a flag "rose" when a bit is set now that was clear at dispatch (or read clear since).
inline int flagBits(int aa8, int ab8) { return (aa8 != 0 ? 1 : 0) | (ab8 != 0 ? 2 : 0); }
inline bool flagsRose(int flagBase, int aa8, int ab8) { return (flagBits(aa8, ab8) & ~(flagBase < 0 ? 0 : flagBase)) != 0; }
// RL8/RQ4: round-node masks for removeRoundNodes. headNodeMask = bit of the FIRST node that is the active action's own (0 = none); itemSpellMask = bits of the 9/10 nodes.
inline unsigned headNodeMask(const RNode* rn, int n, const QueuedAction& head) {   // nodeIsHead is RV1's
    if (!rn) return 0;
    for (int i = 0; i < n && i < kMaxRound; ++i) if (nodeIsHead(rn[i], head)) return 1u << i;
    return 0;
}
inline unsigned itemSpellMask(const RNode* rn, int n) {
    unsigned m = 0;
    for (int i = 0; rn && i < n && i < kMaxRound; ++i) if (rn[i].subType == 9 || rn[i].subType == 10) m |= 1u << i;
    return m;
}
// RQ7: what makeEngineView ties the executing evidence to. spell: strict by id; item: lenient (see Deviations).
struct ExecTie { bool spell = false; uint32_t id = 0; uint32_t curSpell = 0; int flagBase = -1; };
```
### QL verdict + view (RQ4, RQ7, RL5)
- `struct EngineView { bool paused = false, actionPresent = false, executing = false, foreign = false, idle = false; };` -> add `, expanded = false`.
- `enum ActiveVerdict`: append `AV_PRESENT_TIMEOUT` after `AV_GIVE_UP`; `verdictName`: `case AV_PRESENT_TIMEOUT: return "present_timeout";`.
- `activeVerdict`: Before first line (literal) `if (ev.actionPresent) return AV_PRESENT;                                              // 1`. After, two lines:
```cpp
    if (ev.actionPresent && !ev.paused && buf.activeUnpausedMs >= kPresentCeilingMs) return AV_PRESENT_TIMEOUT;   // 0: RQ4 ceiling
    if (ev.actionPresent) return AV_PRESENT;                                              // 1
```
  Rule 3 Before (literal) `if (!buf.eaten && !ev.paused) return AV_UNSEEN;                                       // 3` -> After `if (!buf.eaten && !ev.paused && !buf.sawExpanded) return AV_UNSEEN;                   // 3`. Rule 6 Before (literal) `if (buf.eatenReInjects >= kMaxEatenReInject) return AV_GIVE_UP;                       // 6` -> After (R6: after an expansion a missed execution would double the cast or medpac, so allow ONE re-send; RL5 total cap too):
```cpp
    if (buf.eatenReInjects >= (buf.sawExpanded ? 1 : kMaxEatenReInject) || buf.totalReInjects >= kMaxTotalReInjects) return AV_GIVE_UP;   // 6 (the DROP why stays reinject_exhausted)
```
- `makeEngineView`: Before (literal) is the whole function (`inline EngineView makeEngineView(const int* entryTypes, int ne, bool actionPresent, bool haveCur, int curSub, int nrn,\n int aa8, int ab8, bool paused, uint32_t staleSince, uint32_t now) {` ... `return ev;`). After (full):
```cpp
inline EngineView makeEngineView(const int* entryTypes, int ne, bool actionPresent, bool haveCur, int curSub, int nrn,
                                 int aa8, int ab8, bool paused, uint32_t staleSince, uint32_t now,
                                 const uint32_t* entryParam0 = nullptr, const ExecTie* tie = nullptr) {
    bool execNode = false, ours = false, foreign = false;
    for (int i = 0; entryTypes != nullptr && i < ne; ++i) {
        const int t = entryTypes[i];
        if (t == 0x2E || t == 0x0F) {
            execNode = true;
            if (!tie || !tie->spell || (t == 0x0F && entryParam0 && entryParam0[i] == tie->id)) ours = true;   // RQ7
        }
        if (t != 0x3F && t != 0x3D && t != 0x2E && t != 0x0F) foreign = true;
    }
    const bool creal = curReal(haveCur, curSub, aa8, ab8, paused, staleSince, now);
    const bool curNode = creal && (curSub == 9 || curSub == 10);
    EngineView ev;
    ev.paused = paused;
    ev.actionPresent = actionPresent;
    if (!tie) ev.executing = execNode || (!paused && (curNode || aa8 != 0 || ab8 != 0));   // legacy creature-wide reading (tests, other callers)
    else {   // RQ7: only evidence of OUR action: a matching cur node, or a flag that rose after dispatch; an expanded node is ev.expanded, not executing
        const bool curOurs = curNode && (!tie->spell || (curSub == 9 && (tie->curSpell == 0 || tie->curSpell == tie->id)));
        ev.executing = !paused && (curOurs || flagsRose(tie->flagBase, aa8, ab8));
    }
    ev.expanded = ours;
    ev.foreign = foreign;
    ev.idle = !foreign && !execNode && nrn == 0 && !creal;
    return ev;
}
```
### QL DropWhy / formats / castStarvedAt
- `DropWhy`: append `, DR_PRESENT_TIMEOUT` after `DR_EXECUTED` (RV1 appended `DR_REFUSED, DR_EXECUTED`); `whyName(DropWhy)`: `case DR_PRESENT_TIMEOUT: return "present_timeout";`; add it to the `fmtDrop` grammar comment.
- `castStarvedAt`: Before (literal) `if (k < 1 || k > 3) return 0;` -> After `if (k < 1 || k > kKickMaxAt) return 0;`.
- `fmtResolve(char* out, int cap, uint32_t cmdId, int m, bool seen, int reinj, uint32_t ageMs)` -> add last param `const char* why = nullptr`; before `return w.done();` add `if (why) { w.s(" why="); w.s(why); }`.
- New, after `fmtDrop`:
```cpp
// ORPHAN m=<k> cmd=<n> n=<removed|0> age=<ms> act=removed|failed   (removed < 0 = guard/mismatch, list untouched)
inline int fmtOrphan(char* out, int cap, int m, uint32_t cmdId, int removed, uint32_t ageMs) {
    detail::LogWr w(out, cap);
    w.s("ORPHAN m="); w.mm(m); w.s(" cmd="); w.u(cmdId); w.s(" n="); w.i(removed < 0 ? 0 : removed);
    w.s(" age="); w.u(ageMs); w.s(removed < 0 ? " act=failed" : " act=removed");
    return w.done();
}
// SRV_KICK_GIVEUP m=<k> cmd=<n> ms=<starved ms> n=<removed|0> act=removed|failed
inline int fmtKickGiveup(char* out, int cap, int m, uint32_t cmdId, uint32_t ms, int removed) {
    detail::LogWr w(out, cap);
    w.s("SRV_KICK_GIVEUP m="); w.mm(m); w.s(" cmd="); w.u(cmdId); w.s(" ms="); w.u(ms); w.s(" n="); w.i(removed < 0 ? 0 : removed);
    w.s(removed < 0 ? " act=failed" : " act=removed");
    return w.done();
}
```
### QH globals (next to `g_kickLast` / `g_kickSkipWhy`, before `kickSkip`)
```cpp
qh::UnpausedClock g_orphanClk[3];   // RQ4: orphan round node clock per member
qh::UnpausedClock g_kickClk[3];     // RL8: starved server-route cast clock per member
uint32_t g_execIdCmd[3] = {0, 0, 0};   // RQ7: cmd whose EXEC_ID line was already traced
DWORD g_lastCombatTick = 0;            // RQ6: last tick of a readable combat frame
int removeRoundNodes(uintptr_t srv, const Snapshot& s, unsigned removeMask);   // defined after roundListHandle
```
### QH removeRoundNodes (RQ4, RL8). Define right after `roundListHandle` (~1513); same namespace
```cpp
// Take the round nodes selected by removeMask (bit i = s.rn[i]) out of the list WITHOUT ServerClearAllActions (that also empties cr+0x100: a companion would lose its 0x3D, RQ12).
// Same guards, ActListRemoveHead pop and identity check as detachRound; every node whose bit is clear is pushed straight back (list is empty after the pop, so order is kept); removed nodes
// are leaked on purpose (free path unverified). Returns the count removed; 0 = empty mask (list untouched); -1 = guard/mismatch (all popped nodes restored).
int removeRoundNodes(uintptr_t srv, const Snapshot& s, unsigned removeMask) {
    uint32_t w = 0;
    if (s.nrn <= 0 || s.nrn > qh::kMaxRound || (removeMask & ((1u << s.nrn) - 1u)) == 0) return 0;
    if (!roundListHandle(srv, s.round, &w)) return -1;
    auto pop = reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::ActListRemoveHead);
    auto push = reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail);
    uint32_t got[qh::kMaxRound];
    int popped = 0;
    for (; popped < s.nrn; ++popped) { got[popped] = pop(w); if (got[popped] == 0) break; }
    bool ok = popped == s.nrn;
    for (int i = 0; i < s.nrn && ok; ++i) ok = got[i] == s.rn[i].ptr;
    if (!ok) { for (int i = 0; i < popped; ++i) if (got[i]) push(w, got[i]); return -1; }
    int removed = 0;
    for (int i = 0; i < s.nrn; ++i) { if (removeMask & (1u << i)) ++removed; else push(w, got[i]); }
    return removed;
}
```
(Addresses already in `_shared_src/GameAddr.h`: `ActListRemoveHead 0x007F2A90`, `ActListAddTail`; no new addresses.)
### QH resolveActive (RQ6)
- Before (literal) `static void resolveActive(int k, qh::ActionBuffer& buf, DWORD now) {` ... `qh::fmtResolve(line, sizeof line, buf.activeAction.cmdId, k, buf.seenInQueue, buf.activeAction.reInjectCount,\n                       now - buf.activeAction.queueTime);`
- After: signature `..., DWORD now, const char* why = nullptr)` and the call gets a trailing `, why)`.
### QH kickStarved (RL8). Signature + give-up
- Before (literal): `static void kickStarved(int k, uintptr_t cr, const Snapshot& s, DWORD now) {` ... `if (at == 0) { g_kickSkipWhy[k] = 0; return; }\n    if (s.paused) { kickSkip(k, 1, "paused"); return; }`
- After (the gate/rotate body below stays unchanged except `return;` -> `return false;` and a final `return false;`):
```cpp
static bool kickStarved(int k, uintptr_t cr, qh::ActionBuffer& buf, const Snapshot& s, DWORD now) {   // true = gave up (buf.active cleared, caller skips the rest of this member's tick)
    if (buf.dispatchDeferred || !buf.active) { g_kickClk[k].reset(); return false; }   // R3: a deferred action was never sent; any starved 0x3F is another mod's (selfheal), not ours
    // ... types[] / ne / at unchanged
    if (at == 0) { g_kickSkipWhy[k] = 0; g_kickClk[k].reset(); return false; }
    const uint32_t starvedMs = g_kickClk[k].tick(s.paused, now);   // RL8: unpaused time this cast has been starved, gates included
    if (starvedMs >= qh::kKickGiveUpMs) {
        const unsigned rmask = qh::headNodeMask(s.rn, s.nrn, buf.activeAction);   // R3: only the active action's own node, never another subtype's
        if (rmask == 0) { g_kickClk[k].reset(); return false; }   // not our cast starving (e.g. selfheal's 0x3F, which gives up on its own 5 s clock): do not drop, do not remove
        const int removed = removeRoundNodes(cr, s, rmask);   // else the starved 0x3F fires late, when the member next leads
        char line[200];
        qh::fmtKickGiveup(line, sizeof line, k, buf.activeAction.cmdId, starvedMs, removed);
        QLOG(chMain, "%s", line);
        if (g_logOn && g_cfg.traceQueue) { qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_EATEN_TIMEOUT, 1); QTRACE("%s", line); }
        endAction(buf);
        g_kickClk[k].reset(); g_kickSkipWhy[k] = 0;
        g_lastOurs[k].tag = "kick"; g_lastOurs[k].tick = now;
        return true;
    }
    if (s.paused) { kickSkip(k, 1, "paused"); return false; }
```
  (`endAction` is defined above `kickStarved`; `removeRoundNodes` is forward-declared.) Call site: Before (literal) `if (k > 0 && buf.active && g_cfg.companionServerCast) kickStarved(k, srv, s, now);` -> After (R5: a clock left at e.g. 3000 ms by an action that ended must not shorten the next give-up):
```cpp
        if (k > 0 && buf.active && g_cfg.companionServerCast) { if (kickStarved(k, srv, buf, s, now)) continue; }
        else g_kickClk[k].reset();
```
### QH tickItemQueues
1. **RQ6 combat tick.** Before (literal) `if (combatNow) { for (int k = 0; k < 3; ++k) if (edgeFlags[k]) { g_roundOrder[k] = RoundOrder(); ...` -> insert `g_lastCombatTick = now;` right after `if (combatNow) { ` (rest unchanged). After the loop (literal) `for (int k = 0; k < 3; ++k)\n        if (g_handoff[k].pending) endHandoff(k, qh::DR_COMBAT_CLEAR);` add:
```cpp
    // RQ6: an action still tracked from before the last combat (B1 hold keeps buf.active through a fight) has unknown history; stop tracking it, never re-inject it.
    for (int k = 0; k < 3; ++k) {
        qh::ActionBuffer& b = g_itemQueue[k];
        if (b.active && !qh::shouldTrackAfterCombat(b.activeStartTick, g_lastCombatTick)) resolveActive(k, b, now, "combat_end");
    }
```
2. **RL2 identity swap + skip[].** Before `bool skip[3] = {false, false, false};` add `bool swapped[3] = {false, false, false};`. In the swap block after `std::swap(g_lastOurs[k], g_lastOurs[slot]);` add:
```cpp
            std::swap(g_flagGate[k], g_flagGate[slot]);   // RL2: per-buffer hold / throttle state travels with the buffer
            std::swap(g_kickLast[k], g_kickLast[slot]); std::swap(g_kickSkipWhy[k], g_kickSkipWhy[slot]);
            std::swap(g_lastWaitTick[k], g_lastWaitTick[slot]);
            std::swap(g_orphanClk[k], g_orphanClk[slot]); std::swap(g_kickClk[k], g_kickClk[slot]);
```
   Replace these two literal lines (`// One swap per k per tick. If the buffer now at k still belongs to another slot, it is fixed next tick and must not run against member k's snapshot now.` and `skip[k] = bufBusy(g_itemQueue[k]) && slotOfCreature(tbl, bufOwner(g_itemQueue[k])) != k;`) by `swapped[k] = swapped[slot] = true;   // judged after the loop` (keep the `continue;` after them). After the closing `}` of the pre-pass `for`, before `// Combat ended before a hand-off drained`:
```cpp
    // RL2: judge swapped buffers after ALL swaps (a 3-way rotation left skip[0] set for a buffer that ended in place). Still in the wrong slot = wait one tick.
    for (int k = 0; k < 3; ++k)
        if (swapped[k]) skip[k] = bufBusy(g_itemQueue[k]) && slotOfCreature(tbl, bufOwner(g_itemQueue[k])) != k;
```
3. **RQ7 inputs.** Before `const qh::EngineView ev = qh::makeEngineView(types, s.engineNe < qh::kMaxEntries ? s.engineNe : qh::kMaxEntries, actionInQueue,` insert, and extend the call with `, p0, tp` after `buf.staleCurSince, now`:
```cpp
        uint32_t p0[qh::kMaxEntries] = {};
        for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) p0[i] = s.e[i].param0;
        qh::ExecTie tie;
        const qh::ExecTie* tp = nullptr;
        if (buf.active) {   // RQ7: tie the executing evidence to the active action
            buf.noteFlags(qh::flagBits(s.aa8, s.ab8));
            tie.spell = buf.activeAction.kind == qh::ActionKind::Spell;
            tie.id = buf.activeAction.spellId;
            tie.curSpell = s.haveCur ? s.cur.spellId : 0;
            tie.flagBase = buf.flagBase;
            tp = &tie;
        }
```
   After the `ev` line add the trace (item id space is unproven):
```cpp
        if (tp && !tie.spell && g_execIdCmd[k] != buf.activeAction.cmdId && g_logOn && g_cfg.traceQueue)
            for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i)
                if (types[i] == 0x2E || types[i] == 0x0F) {
                    g_execIdCmd[k] = buf.activeAction.cmdId;
                    QTRACE("EXEC_ID cmd=%u m=%d type=%02x want=%08x got=%08x", (unsigned)buf.activeAction.cmdId, k, types[i], (unsigned)buf.activeAction.clientItem, (unsigned)p0[i]);
                    break;
                }
```
4. **RQ4 orphan block.** Insert right after `if (buf.active && actionInQueue) buf.seenInQueue = true;   // bookkeeping only (RESOLVE seen=)` (before the `kickStarved` comment/call):
```cpp
        // RQ4: orphan round node. ClearAllActions never empties the round list, so after a wipe a 9/10 node can sit with no 0x3F/0x0F/0x2E entry to consume it:
        // it keeps the action PRESENT and blocks dispatch (DS_NRN_BUSY). kOrphanMs unpaused with no real cur: remove the list (no ServerClearAllActions), let the verdict re-send.
        {
            bool consumer = false;
            for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) consumer = consumer || qh::entryConsumesRound(s.e[i].type);
            const unsigned omask = qh::itemSpellMask(s.rn, s.nrn);   // R3: remove only the 9/10 orphans, never another subtype's node
            uint32_t age = 0;
            if (bufBusy(buf)) age = qh::orphanRoundAge(consumer, creal, omask != 0, s.paused, g_orphanClk[k], now); else g_orphanClk[k].reset();
            if (age >= qh::kOrphanMs) {
                const int removed = removeRoundNodes(srv, s, omask);
                char line[200];
                const uint32_t cmd = buf.active ? buf.activeAction.cmdId : buf.queue[0].cmdId;
                qh::fmtOrphan(line, sizeof line, k, cmd, removed, age);
                QLOG(chMain, "%s", line);
                if (removed >= 0 && buf.active) buf.eaten = true;   // else rule 3 would resolve it UNSEEN (lost)
                g_orphanClk[k].reset();                             // a failed removal retries after another kOrphanMs, never every frame
                g_lastOurs[k].tag = "orphan"; g_lastOurs[k].tick = now;
                continue;                                           // s is stale: the next frame re-reads the queue
            }
        }
```
5. **AV switch.** `case qh::AV_PRESENT:` Before (literal) `buf.notePresent(ev.executing, actionInQueue, now);` -> `buf.notePresent(ev.executing, actionInQueue, now, ev.expanded);` and add a case:
```cpp
                    case qh::AV_PRESENT_TIMEOUT:   // RQ4 ceiling: PRESENT for kPresentCeilingMs unpaused; abandon tracking (an orphan node, if any, goes through ORPHAN next)
                        if (g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_PRESENT_TIMEOUT, 1); QTRACE("%s", line); }
                        endAction(buf);
                        break;
```
   `case qh::AV_EATEN_REINJECT:` Before (literal) `buf.eatenReInjects++;\n                        buf.lastReInjectTick = now;` -> insert `buf.totalReInjects++;` after the first line. Later in the same case, RV1's After text reads `const SendResult r = dispatchAction(buf.activeAction, k);\n                        buf.markDispatched(now);\n                        afterSend(k, buf, r, now);`: replace only the middle line (anchor on `buf.markDispatched(now);` inside this `case`) by `buf.markReInjected(now, qh::flagBits(s.aa8, s.ab8));`.
   RV1's deferred-retry branch (`if (buf.dispatchDeferred) {` ... `if (r == SendResult::Sent) buf.markDispatched(now);`) gets `buf.markReInjected(now, qh::flagBits(s.aa8, s.ab8));` instead (R4: `markDispatched` now zeros `eatenReInjects`/`totalReInjects`/`lastReInjectTick`, so a refused-then-sent re-inject would wipe the RL5 cap and the spacing gate; do not pass flags=-1 there).
6. **RL1/RQ7 new dispatch.** Before (literal) `buf.activeAction = nextA;\n                    buf.markDispatched(now);` (RV1 leaves these two lines; its `const SendResult r = dispatchAction(nextA, k);` follows) -> `buf.markDispatched(now, qh::flagBits(s.aa8, s.ab8));`. Hooks' `markDispatched(GetTickCount())` stay (flags unknown = -1; `noteFlags` sets the baseline on the first tick).
7. **RL3 call.** Before (literal) `if (!g_flagGate[k].step(flagsBusy, now, &flagCapped)) dsWhy = qh::DS_FLAG_BUSY;` -> `step(flagsBusy, now, &flagCapped, s.paused)`.
- Edge cases: pause (orphan/kick clocks frozen; flag hold ignores paused time; ceiling counts unpaused only); leader switch (clocks + FlagGate swap with the buffer; an unreadable owner keeps `skip`); combat (tick skipped; `UnpausedClock` ignores the >500 ms gap; `g_lastCombatTick` only on readable combat frames); save/load (round pointer 0 -> verdict block skipped, orphan block still reads `s.rn` = empty -> clock resets); tick wrap (`shouldTrackAfterCombat` signed diff; clocks use deltas); unreadable list (`removeRoundNodes` -1, logged `act=failed`).
- Logging: default level (`QLOG chMain`): `ORPHAN m= cmd= n= age= act=`, `SRV_KICK_GIVEUP m= cmd= ms= n= act=`. Trace (`trace_queue`): `DROP ... why=present_timeout`, `DROP ... why=eaten_timeout` after a kick give-up, `RESOLVE ... why=combat_end`, `EXEC_ID cmd= m= type= want= got=`.

## Tests (QT; register after `testFlagGate();` in main; all pure)
- Update: `verdictFieldsClear` add `&& !b.sawExpanded && b.flagBase == -1 && b.activeUnpausedMs == 0 && b.totalReInjects == 0`; `dirtyVerdictFields` set `sawExpanded=true; flagBase=3; activeUnpausedMs=9; totalReInjects=9`. `testServerRouteQL20`: `castStarvedAt(q5, 5, true)` expectation 0 -> 4 (comment "4th leading 0x3D now kicked"). `testWhyNames`: `whyName(DR_PRESENT_TIMEOUT)=="present_timeout"`. `testActiveVerdictRules`: `verdictName(AV_PRESENT_TIMEOUT)=="present_timeout"`.
- `testOrphanRoundAge` (RQ4): `UnpausedClock c;` ticks every 100 ms from 1000 with (hasConsumer=false, creal=false, nrn910=true, paused=false): t=1000 -> 0; t=2400 -> 1400 (< kOrphanMs); t=2499 (d=99) -> 1499; t=2500 -> 1500 (>=). Consumer present at 2600 -> 0 and `c.ms==0`; creal=true -> 0; nrn910=false -> 0. Pause: ticks 1000..1500 -> 500; paused tick 1600 and 5000 -> 500 (frozen); unpaused 5100 -> 500 (first tick after pause adds nothing); 5200 -> 600. Gap: tick 1000, 1100 (100), 3000 (gap 1900) -> still 100. `entryConsumesRound`: 0x3F/0x0F/0x2E true, 0x3D/0x01/-1 false.
- `testPresentCeiling` (RQ4): `ActionBuffer b; Ev(true)`: `activeUnpausedMs=29999` -> AV_PRESENT; `=30000` -> AV_PRESENT_TIMEOUT; same with `Ev(true, /*paused*/true)` -> AV_PRESENT; `Ev(false)` with 30000 and goneSinceTick=1 -> old rule (AV_UNSEEN). `accrueEaten`: unpaused 3 steps (now 1000, 1500, 9000) -> `activeUnpausedMs` 1000+500+1000 (first call d capped), paused step adds 0. `fmtDrop(.., 14, 0, DR_PRESENT_TIMEOUT, 1)` == `"DROP cmd=14 m=0 why=present_timeout n=1"`.
- `testShouldTrackAfterCombat` (RQ6): (5000,5000) true; (4999,5000) false; (5001,5000) true; (123,0) true; wrap (0x10, 0xFFFFFFF0) true; (0xFFFFFFF0, 0x10) false. `fmtResolve(out, sizeof out, 5, 0, true, 1, 3400, "combat_end")` == `"RESOLVE cmd=5 m=0 seen=1 reinj=1 age=3400 why=combat_end"`; existing no-why goldens unchanged.
- `testExecTie` (RQ7): spell tie `{spell=true, id=0x28, curSpell=0, flagBase=2}`; entries `{0x0F}`, p0 `{0x28}`: `expanded`, `!executing`. p0 `{0x99}`: `!expanded`. entries `{0x2E}`: `!expanded` (spell). ab8=1, flagBase=2 (set at dispatch): `!executing`; flagBase=0: executing; flagBase=1 with ab8=1: executing; flagBase=-1: executing; paused with ab8=1: `!executing`. cur sub 9, curSpell 0x28, creal (aa8=1): executing; curSpell 0x99 with aa8=0/ab8=0 and flagBase=0: `!executing`; curSpell 0: executing. Item tie `{spell=false}`: entries `{0x2E}` -> expanded; cur sub 10 -> executing. `tie==nullptr`: existing `e2e/e0f` assertions unchanged. `noteFlags`: from `flagBase=-1`: `noteFlags(3)` -> 3; `noteFlags(1)` -> 1; `noteFlags(3)` -> 1 (the baseline only shrinks); `noteFlags(0)` -> 0, then `flagsRose(0, 0, 1)` true. Verdict: `b.goneSinceTick=1; b.sawExpanded=true;` unpaused idle `Ev(false)` at 10000 -> AV_EATEN_REINJECT; without `sawExpanded` -> AV_UNSEEN. `notePresent(false, true, 1000, true)` sets `sawExpanded`.
- `testMarkDispatchedResets` (RL1): `eatenReInjects=5; eatenUnpausedMs=15000; lastReInjectTick=900; sawExpanded=true; totalReInjects=7; activeUnpausedMs=9; active=false; markDispatched(2000, 3)` -> all 0/false, `flagBase==3`; first eaten episode then gets the full `kMaxEatenReInject` (6 REINJECT verdicts before GIVE_UP). `markReInjected(3000, 1)`: counters keep (eatenReInjects 5 stays), `sawExecuting/eaten/goneSinceTick/activeUnpausedMs` cleared, `flagBase==1`.
- `testTotalReInjectCap` (RL5): loop 13 cycles: `b.eaten=true; b.goneSinceTick=1; now=10000+3000*i`; verdict AV_EATEN_REINJECT for i=0..11 (after each: `eatenReInjects++; totalReInjects++; lastReInjectTick=now; markReInjected(now); notePresent(false,true,now+2500)` which resets `eatenReInjects`/`eatenUnpausedMs` but not `totalReInjects`); cycle i=12 -> AV_GIVE_UP.
- `testFlagGatePaused` (RL3): `FlagGate g; bool cap;` `step(true,1000)` false; `step(true,2500)` false (1.5 s busy); `step(false,2600,&cap,true)` false; `step(false,5600,&cap,true)` false (3 s paused); `step(true,5700,&cap)` false; `step(true,6099,&cap)` false (eff 1999); `step(true,6100,&cap)` true and `cap`. Paused first tick with `sawBusy=false` returns true. After a pause mid clear-run (`step(false)` clearing, paused tick, `step(false)`) the gate needs a fresh 150 ms clear run (`clearing` restarted). Existing `testFlagGate` vectors unchanged.
- `testKickGiveUp` (RL8): `castStarvedAt({3D,3D,3D,3D,3F},5,true)==4`; `{3D x15, 3F}` (ne=16) == 15; `{3D,01,3F}` == 0; `{3D,3F,3F}` == 1. `UnpausedClock k;` ticks every 100 ms: `>= kKickGiveUpMs` first true at the 51st tick (5000 ms); a paused stretch adds nothing. `fmtOrphan(out, sizeof out, 0, 7, 1, 1500)` == `"ORPHAN m=0 cmd=7 n=1 age=1500 act=removed"`; `(1, 0, -1, 1500)` == `"ORPHAN m=1 cmd=0 n=0 age=1500 act=failed"`; `fmtKickGiveup(out, sizeof out, 1, 17, 5000, 1)` == `"SRV_KICK_GIVEUP m=1 cmd=17 ms=5000 n=1 act=removed"`. `-Werror` bounds: add both to `testLogFormatBounds` style tiny-buffer check (null/0-cap safe like the others). R3 masks (RV1's `N(sub,ptr,spell,item)` helper; move it to file scope if it is local): head Spell 0x55, `headNodeMask({N(9,1,0x66),N(9,2,0x55)},2,head)==0b10`, `{N(9,1,0x55),N(9,2,0x55)}`==0b01 (first only), `{N(9,1,0x66)}`==0, `nullptr`==0; `itemSpellMask({N(1,1),N(9,2),N(10,3)},3)==0b110`, `{N(1,1)}`==0, `nullptr`==0.
- `testSawExpandedCap` (R6, in `testActiveVerdictRules`): base `b.eaten=true; b.goneSinceTick=1; b.activeStartTick=0;` now=10000, idle unpaused `Ev(false)`: `sawExpanded=true, eatenReInjects=1` -> AV_GIVE_UP; `sawExpanded=true, eatenReInjects=0` -> AV_EATEN_REINJECT; `sawExpanded=false, eatenReInjects=1` -> AV_EATEN_REINJECT (cap stays `kMaxEatenReInject`); `sawExpanded=false, eatenReInjects=6` -> AV_GIVE_UP.
- PLAY rows for `PLAY_TEST_QUEUE.md` (all `pending`):
  - **QV-OR1** (RQ4): pause, queue a medpac on the leader (buffered then dispatched), quicksave (F5) before unpausing (or any script clear), unpause. Pass: `ORPHAN m=0 ... act=removed` about 1.5 s after unpause, then `REINJECT`/`RESOLVE` or `DROP`; the next buffered item dispatches without Cancel. If no ORPHAN ever appears and the item just runs, record that (answers open Q1).
  - **QV-CE1** (RQ6): queue a medpac on the leader while paused, switch leader to a companion, start combat paused (B1 hold), end combat, unpause. Pass: `RESOLVE ... why=combat_end` on the first non-combat tick, no `re-injecting eaten`, no second use.
  - **QV-EX1** (RQ7): two Force powers back to back with `dispatch_wait_flags=1` and a medpac right after a cast. Pass: each cmd ends with `VANISH kind=executed`/`RESOLVE seen=1`, no duplicate cast; any `EXEC_ID` line shows item `want`/`got` (if they differ, tighten the item match in a follow-up). R6: also record the medpac stack count before and after each run; a decrement of 2 for one cmd means `sawExpanded` re-sent a finished action (then drop the `sawExpanded` skip of rule 3).
  - **QV-KG1** (RL8): no forced repro; watch every play log for `SRV_KICK_GIVEUP` (companion Force power starved behind 0x3D); pass = none, or one followed by no late cast when the member next leads.

## Tooling
- `python3 scripts/run_tests.py queue_hud_logic_test` (fix any `-Werror` fallout first: unused `swapped`, non-exhaustive `switch`), no `_shared_src` change, so no `compat_check.py`.
- `python3 scripts/test_queue_hud_log_check.py`. The checker ignores unknown keywords (`ORPHAN`, `SRV_KICK_GIVEUP`, `EXEC_ID`) and accepts the extra `why=combat_end` on RESOLVE (unlisted keys are only validated through `FIELD_RE`, `why` is `\w+`); document the new lines and `RESOLVE ... [why=combat_end]` in its docstring (line 18).
- Checker flag C (R8): a held action resolved at `combat_end` can have `seen=0 reinj>0` legitimately. In `qj_flags` (`queue_hud_log_check.py:~311`) Before (literal) `if _num(f["seen"]) == 0 and (_num(f["reinj"]) or 0) > 0:` -> After `if _num(f["seen"]) == 0 and (_num(f["reinj"]) or 0) > 0 and f.get("why") != "combat_end":`; docstring line 58: append `(not why=combat_end)`. Tests next to `test_C_quiet` (`:~357`, helper `resolve` has no `why`, append it to the string): `test_C_quiet_when_combat_end` (`resolve(5, seen=0, reinj=1) + " why=combat_end"` -> `flags_of` == `[]`) and a parse guard (`parse([that line])` -> one record, `f["why"] == "combat_end"`, nothing skipped), so the quiet result is not just a skipped line.
- `scripts/pre_install_gate.sh --mod queue_hud_src` must print `GATE PASS` (no install in RV2; install is RV7).
- Commit with explicit paths: `git commit -- patch_manager_mods/queue_hud_src/QueueHud.cpp QueueHudLogic.h queue_hud_logic_test.cpp manifest.toml queue_hud.ini scripts/queue_hud_log_check.py scripts/test_queue_hud_log_check.py` (+ `PLAY_TEST_QUEUE.md`).

## Risks / ordering
- Order inside RV2: (1) QL constants + ActionBuffer + FlagGate + helpers + view/verdict + formats, (2) QT updates and new tests, run `queue_hud_logic_test` green, (3) QH globals, `removeRoundNodes`, `resolveActive`, `kickStarved`, tick edits, (4) version bump, gate.
- RV1 overlap: same functions (`tickItemQueues` re-inject block, `AV_EATEN_REINJECT` case, `dispatchAction`, `reInjectWhy`). Edits here swap the `markDispatched` call inside `case AV_EATEN_REINJECT` AND the one in RV1's deferred-retry branch to `markReInjected(now, flags)` (R4; never the flags=-1 default there), add `totalReInjects++`, and add the orphan/tie blocks before RV1's `reInjectWhy` call. `DropWhy` and `whyName` both gain an enumerator: append after RV1's `DR_EXECUTED`.
- RV3 (next) edits `handoffTick`/`doCancel`/`detach*`: it should reuse `removeRoundNodes` for RQ12 (Cancel on a companion without ServerClearAllActions) instead of writing a second pop loop.
- Behavioural risks: ORPHAN removes list nodes the engine never frees (leak ~0x88 B each, same as `discardDetached`); a false ORPHAN needs 1.5 s unpaused with no consumer entry, no real cur and a pending 9/10 node, which no normal cast has (0x3F -> 0x0F/0x2E -> cur). `sawExpanded` makes a post-expansion wipe re-send (possible double cast only if the cast fired without ever raising a flag or a matching cur for its whole ~3 s window, which no log shows).

## OPEN
- Engine Q1 (doc 47): do orphan 9/10 nodes ever vanish without a 0x3F (wedge vs long stall)? QV-OR1 answers it.
- Item id space of 0x2E `param0` vs `clientItem` (EXEC_ID data), then tighten the lenient item match.
- H2-F10: `s.round == 0` skips the verdict block, so an active action with an unreadable round pointer is never resolved or ceilinged (unverified reach).

## Review corrections applied
Source: `review_RV12.md` (folded in place; the executor applies this spec top-to-bottom, no separate review file).
- R1 [BLOCKING]: RV1 only (follower-wipe line keeps its `cmd=` tail); nothing to do here.
- R2 [BLOCKING]: RV1 only (checker flag I for `DROP why=executed|refused`).
- R3 [BLOCKING]: `removeRoundNodes(srv, s, removeMask)` keeps unmasked nodes; ORPHAN mask = 9/10 nodes (`itemSpellMask`); kick give-up mask = the active action's own node (`headNodeMask`, via RV1 `nodeIsHead`), skipped when `dispatchDeferred` or mask 0; `kickStarved` first line resets the clock for deferred/inactive; masks tested.
- R4 [MINOR]: RV1's deferred-retry and `AV_EATEN_REINJECT` sends use `markReInjected(now, flags)`, so the per-action budget and spacing gate survive a refused-then-sent re-inject.
- R5 [MINOR]: call site `else g_kickClk[k].reset();` so a stale clock never shortens the next give-up.
- R6 [MINOR]: rule 6 caps re-injects at 1 when `sawExpanded`; `testSawExpandedCap` added; QV-EX1 records stack counts before and after.
- R7 [MINOR]: RV1 only (QH24 criteria, OPEN item).
- R8 [MINOR]: checker flag C skips `why=combat_end`; tests plus parse guard added.
- R9 [MINOR]: `accrueEaten` literal block, `AV_EATEN_REINJECT` text as it reads after RV1, and the other Before-snippets (activeVerdict rules with their `// n` comments, resolveActive, swap lines, combat tick, notePresent, FlagGate) quoted literally.
