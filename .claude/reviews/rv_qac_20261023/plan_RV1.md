# RV1 spec: queue-hud high fixes (RQ1 spell replay args, RQ5+RL12 dispatch gates, RQ2 edge head, RQ3 detach mask)
Mod: queue-hud 0.4.5-dev -> 0.4.6-dev. Edit the version in: `queue_hud_src/manifest.toml:4`, `queue_hud.ini:1`, `QueueHud.cpp:2225` (`KP_LOG(chMain, "0.4.5-dev: ...` -> `0.4.6-dev:`).
Files: `patch_manager_mods/queue_hud_src/{QueueHudLogic.h,QueueHud.cpp,queue_hud_logic_test.cpp,queue_hud.ini,manifest.toml}`, `patch_manager_mods/_shared_src/GameAddr.h:71` (comment only), `scripts/queue_hud_log_check.py` + `scripts/test_queue_hud_log_check.py` (SPELL_ARGS format), `patch_manager_mods/30_queue_hud.md:78` (wording).
Engine fact verified: `decomp_7791a0.txt:77` calls `FUN_00775700(spellId, targetClientId, &zeroFloat3, 0,0,0,0)`. Hook stack (ebp+): 8 spellId, 0xC targetClientId, 0x10 `float*` pos, 0x14 multi, 0x18 meta, 0x1C domain, 0x20 int flag.
Apply order: A) QueueHudLogic.h, B) update tests until `run_tests.py queue_hud_logic_test` passes, C) QueueHud.cpp (RQ1, RQ5, RQ2, RQ3), D) ini/manifest/docs/checker.

## Changes

### RQ1 (+V3-3): spell replay passes captured pos pointer + int flag
- Files/anchors: `QueueHudLogic.h:587-652` (QueuedAction, makeSpellAction, replayPos), `:218` + `:555` (Config.spellReplayPos + parse), `:1273-1290` (fmtSpellArgs); `QueueHud.cpp:82` (typedef), `:1111-1138` (dispatchAction spell branch), `:2277-2290`/`:2364-2445` (hooks), `:2226`; `queue_hud.ini:66-67`.
- QueuedAction: delete `bool hasPos`; add `int flag = 0;   // Spell: 7th stack arg of PerformCastSpell (int, vanilla always 0)` directly above `cmdId` (aggregate inits in tests stay valid; cmdId stays last).
- makeSpellAction Before (literal, QueueHudLogic.h:~627): `inline QueuedAction makeSpellAction(uintptr_t creature, uint32_t spellId, int arg2, uint32_t targetId,\n int multispell, int metamagic, int domain, const float* pos, uint32_t queueTime,\n uint32_t cmdId = 0) {`; its body has `a.targetId = targetId;` and `a.hasPos = (pos != nullptr);` (both go). After (4th param removed, `flag` after `pos`):
```cpp
inline QueuedAction makeSpellAction(uintptr_t creature, uint32_t spellId, int arg2, int multispell, int metamagic, int domain,
                                    const float* pos, int flag, uint32_t queueTime, uint32_t cmdId = 0) {
    QueuedAction a;
    a.creature = creature; a.clientItem = spellId; a.packedByte = arg2;
    a.targetId = static_cast<uint32_t>(arg2);   // spells: the target client id (kept so the generic id/target logs stay meaningful)
    if (pos) { a.pos[0] = pos[0]; a.pos[1] = pos[1]; a.pos[2] = pos[2]; }
    a.queueTime = queueTime; a.kind = ActionKind::Spell;
    a.spellId = spellId; a.arg2 = arg2; a.multispell = multispell; a.metamagic = metamagic; a.domain = domain;
    a.flag = flag; a.reInjectCount = 0; a.cmdId = cmdId;
    return a;
}
```
- Delete `replayPos` (`inline const float* replayPos(const QueuedAction& a, bool useOrigPos) {...}` plus the 2-line `// Position pointer for the replayed call` comment above it). Add in its place (pure, tested):
```cpp
// The two trailing args of the replayed PerformCastSpell: ALWAYS a valid pointer to our own copy of the caller's float[3] (the builder 0x87A5C0
// dereferences it unconditionally) and the caller's int flag (vanilla 0; a nonzero flag sets bit 4 = server skips the cast, QL9).
struct ReplayArgs { const float* pos; int flag; };
inline ReplayArgs buildReplayArgs(const QueuedAction& a) { return ReplayArgs{ a.pos, a.flag }; }
```
- Config: delete `bool spellReplayPos`, add `bool spellReplayPosSeen = false;   // legacy key spell_replay_pos was in the ini (accepted, ignored)`; parse line becomes `if (detail::keyEq(kb, ke, "spell_replay_pos")) { c.spellReplayPosSeen = true; return true; }`.
- fmtSpellArgs Before (literal): the 3 comment lines starting `// SPELL_ARGS where=<hook|replay> cmd=<n> m=<k> spell=<hex> arg2=<hex> tgt=<hex> ...`, then `inline int fmtSpellArgs(char* out, int cap, bool replay, uint32_t cmdId, int m, uint32_t spellId, int arg2, uint32_t targetId,\n int multispell, int metamagic, int domain, uint32_t posPtrRaw, bool hasPos) {` and its whole body (ends `w.s(" pos="); w.s(posPtrRaw == 0 ? "null" : hasPos ? "orig" : "zero");\n return w.done();\n}`). Replace all of it with:
```cpp
// SPELL_ARGS where=<hook|replay> cmd=<n> m=<k> spell=<hex> tgt=<hex> multi=<n> meta=<n> dom=<n> pos=<hex,hex,hex> flag=<n> posaddr=<hex>
// tgt = the 2nd stack arg (target client id); pos = the 3 float bit patterns behind the 3rd arg (hook) / our copy (replay); flag = 7th arg;
// posaddr = the pointer value passed (hook: the caller's dead-after-return stack address, replay: &a.pos).
inline int fmtSpellArgs(char* out, int cap, bool replay, uint32_t cmdId, int m, uint32_t spellId, int arg2,
                        int multispell, int metamagic, int domain, const float* pos3, int flag, uint32_t posAddr) {
    detail::LogWr w(out, cap);
    w.s("SPELL_ARGS where="); w.s(replay ? "replay" : "hook");
    w.s(" cmd="); w.u(cmdId); w.s(" m="); w.mm(m); w.s(" spell="); w.x(spellId);
    w.s(" tgt="); w.x(static_cast<uint32_t>(arg2));
    w.s(" multi="); w.i(multispell); w.s(" meta="); w.i(metamagic); w.s(" dom="); w.i(domain);
    w.s(" pos=");
    for (int i = 0; i < 3; ++i) { uint32_t b = 0; if (pos3) memcpy(&b, &pos3[i], 4); if (i) w.c(','); w.x(b); }
    w.s(" flag="); w.i(flag); w.s(" posaddr="); w.x(posAddr);
    return w.done();
}
```
  (`<string.h>` is already included at `QueueHudLogic.h:19`.)
- `QueueHud.cpp:82` Before (literal): `using PerformCastSpellFn = void(__thiscall*)(void* creature, uint32_t spellId, int arg2, uint32_t targetId, int multispell, int metamagic, int domain, const float* pos);` After: `using PerformCastSpellFn = void(__thiscall*)(void* creature, uint32_t spellId, uint32_t targetClientId, const float* pos, int multispell, int metamagic, int domain, int flag);`
- dispatchAction spell branch Before (literal, `QueueHud.cpp:~1118-1140`):
```cpp
        const float* pp = qh::replayPos(a, g_cfg.spellReplayPos);
        if (g_logOn && g_cfg.traceQueue) {
            char line[200];
            qh::fmtSpellArgs(line, sizeof line, true, a.cmdId, k, a.spellId, a.arg2, a.targetId, a.multispell, a.metamagic, a.domain,
                             static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pp)), a.hasPos);
            QTRACE("%s", line);
        }
        if (k >= 0) spellWatchArm(k, a.cmdId, GetTickCount());
        reinterpret_cast<PerformCastSpellFn>(AddrPerformCastSpell)(
            reinterpret_cast<void*>(a.creature),
            a.spellId,
            a.arg2,
            a.targetId,      // <- lands in the pos slot (the bug)
            a.multispell,
            a.metamagic,
            a.domain,
            pp);             // <- lands in the int-flag slot (the bug)
```
  After:
```cpp
        const qh::ReplayArgs ra = qh::buildReplayArgs(a);
        if (g_logOn && g_cfg.traceQueue) {
            char line[200];
            qh::fmtSpellArgs(line, sizeof line, true, a.cmdId, k, a.spellId, a.arg2, a.multispell, a.metamagic, a.domain,
                             ra.pos, ra.flag, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ra.pos)));
            QTRACE("%s", line);
        }
        if (k >= 0) spellWatchArm(k, a.cmdId, GetTickCount());
        reinterpret_cast<PerformCastSpellFn>(AddrPerformCastSpell)(reinterpret_cast<void*>(a.creature), a.spellId,
            static_cast<uint32_t>(a.arg2), ra.pos, a.multispell, a.metamagic, a.domain, ra.flag);
```
- Spell hook (`QueueHudPerformCastSpell`, `QueueHud.cpp:~2364-2398`) Before (literal): declarations `uint32_t targetId = 0;` ... `uintptr_t posPtr = 0;\n    float posVal[3] = {};`; in the required `if (!rd(...) || ...)` group the line `!rd(SiteQueue + 0x32, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x10, &targetId) ||`; and the block `if (rd(SiteQueue + 0x36, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x20, &posPtr) && posPtr) { rd(SiteQueue + 0x37, posPtr, 0, &posVal[0]); ... 0x39 ... }`. After: delete `uint32_t targetId = 0;` and the `0x10` line from the required group; replace `uintptr_t posPtr = 0;` by `uintptr_t posAddr = 0; int flag = 0;` (keep `float posVal[3] = {};`) and replace the whole `if (rd(SiteQueue + 0x36 ...` block by
```cpp
    rd(SiteQueue + 0x32, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x10, &posAddr);   // 3rd stack arg = caller's float[3] (stack address, dead after return)
    rd(SiteQueue + 0x36, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x20, &flag);      // 7th stack arg = int flag, NOT a pointer
    if (posAddr) {
        rd(SiteQueue + 0x37, posAddr, 0, &posVal[0]);
        rd(SiteQueue + 0x38, posAddr, 4, &posVal[1]);
        rd(SiteQueue + 0x39, posAddr, 8, &posVal[2]);
    }
```
  Then `const uint32_t tgt = static_cast<uint32_t>(arg2);` and replace every `targetId` in that function (traceCmd x6 args, QLOG "target=%08x") with `tgt`; `fmtSpellArgs(line, sizeof line, false, cmd, memberK, spellId, arg2, multispell, metamagic, domain, posVal, flag, posAddr)` (replaces the call ending `static_cast<uint32_t>(posPtr), posPtr != 0);`); both `makeSpellAction(crPtr, spellId, arg2, targetId, multispell, metamagic, domain, posPtr ? posVal : nullptr, GetTickCount(), cmd)` calls become `makeSpellAction(crPtr, spellId, arg2, multispell, metamagic, domain, posVal, flag, GetTickCount(), cmd)` (always `posVal`, zeros when unreadable = vanilla's `&zero`; never nullptr). The Item hook is unchanged (items pass `a.pos`, their ebp+0x10 really is targetId).
- Edge cases: unreadable pos pointer -> zeros (vanilla-equivalent); nonzero flag preserved (previously impossible); NULL pos at capture -> zeros, replay still passes `&a.pos` (never NULL). Server route (serverCast) ignores pos/flag. Buffer copy/move keeps `pos` inline, so `&a.pos` is valid for the duration of the call only (dispatchAction receives a reference to the stored/local action, fine).
- Ini: replace `queue_hud.ini:66-67` by `; spell_replay_pos was removed in 0.4.6-dev (the replay now passes the captured position and flag); the key is accepted and ignored.`
- Logging: startup `QueueHud.cpp:2226` Before `"pause_eaten_wait=%d dispatch_wait_flags=%d spell_replay_pos=%d"` -> After drop the last field/arg, and add after it: `if (g_cfg.spellReplayPosSeen) QLOG(chMain, "ini: spell_replay_pos is obsolete and ignored (0.4.6-dev)");` (one INFO line). Trace: SPELL_ARGS new format above (trace_queue only).
- Docs: `GameAddr.h:71` comment After: `thiscall(controlled client creature; spellId, targetClientId, float* pos, multispell, metamagic, domain, int flag) RET 0x1C, void`. `30_queue_hud.md:78` stack args list -> `spellId, targetClientId, pos*, multispell, metamagic, domain, flag`.

### RQ5 (+RL12, V1-3/V1-10/V2-3): gates on RI_OK, dispatch result, deferred retry
- Files/anchors: `QueueHudLogic.h:677-700` (ActionBuffer fields + cancel()/clear()/markDispatched), `:828-834` (shouldReInject), `:870-883` (enums, reInjectWhy), `:895-935` (whyName); `QueueHud.cpp:1033` (endAction), `:1063-1138` (serverCast, dispatchAction), `:1441` (hand-off send), `:1822-1846` (RI_OK), `:1881-1897` (AV_EATEN_REINJECT), `:1955-1957` (next dispatch); `queue_hud.ini:47-51`.
- ActionBuffer: add (after `lastVerdictTick`): `bool dispatchDeferred = false;  // RQ5: a server-route send was refused; retry every kDeferRetryMs up to kDeferMaxMs unpaused` `uint32_t deferTick = 0, deferLastTry = 0, deferUnpausedMs = 0;`. Reset all four in `cancel()` and `clear()` (same lines as `lastVerdictTick = 0;`), and in the cpp `endAction()` (`b.dispatchDeferred = false; b.deferUnpausedMs = 0;`).
- Enums (literal Before `enum ReInjectWhy : uint8_t { RI_OK, ..., RI_EXECUTED, RI_NOT_CTRL };`): `RI_NOT_CTRL` is last; append `, RI_DEFERRED, RI_OFF, RI_PAUSED, RI_SETTLE, RI_SPACING` (names in `whyName(ReInjectWhy)`: "deferred","off","paused","settle","spacing"). `DropWhy` (Before ends `..., DR_EATEN_TIMEOUT, DR_IDENTITY_LOST };`): append `, DR_REFUSED, DR_EXECUTED` (names "refused","executed" in `whyName(DropWhy)`).
- reInjectWhy / shouldReInject. Before (literal): `inline ReInjectWhy reInjectWhy(const ActionBuffer& buf, bool actionInQueue, bool hasFollowLeader, bool paused = false, uint32_t now = 0, bool userCancelled = false) {\n    (void)paused; (void)now;\n    if (!buf.active) return RI_INACTIVE;` ... ending `return hasFollowLeader ? RI_OK : RI_NO_LEADER;`. After (replace the whole function; the `(void)` line goes because paused/now are now used; legacy callers keep the default empty gate, so the ~25 existing test calls are unchanged):
```cpp
struct RiGate { bool on = false; bool pauseEatenWait = false; bool pauseReinject = true; };   // production passes on=true
inline ReInjectWhy reInjectWhy(const ActionBuffer& buf, bool actionInQueue, bool hasFollowLeader, bool paused = false, uint32_t now = 0,
                               bool userCancelled = false, const RiGate& gate = RiGate{}) {
    if (!buf.active) return RI_INACTIVE;
    if (buf.reInjectPending) return RI_PENDING;
    if (buf.userCancelled || userCancelled) return RI_CANCEL;
    if (buf.dispatchDeferred) return RI_DEFERRED;          // the deferred retry owns the command; nothing is in the engine on purpose
    if (buf.activeAction.reInjectCount >= kMaxReInjectCount) return RI_MAX;
    if (buf.sawExecuting) return RI_EXECUTED;
    if (actionInQueue) return RI_IN_QUEUE;
    if (!hasFollowLeader) return RI_NO_LEADER;
    if (gate.on) {                                          // RQ5: the same gates the verdict path has
        if (!gate.pauseReinject) return RI_OFF;
        if (paused && gate.pauseEatenWait) return RI_PAUSED;
        if (now - buf.activeStartTick <= kMinActiveMs) return RI_SETTLE;
        if (buf.lastReInjectTick != 0 && now - buf.lastReInjectTick < kReInjectSpacingMs) return RI_SPACING;
    }
    return RI_OK;
}
```
  In `shouldReInject` (keep its `(void)paused; (void)now;`) add `if (buf.dispatchDeferred) return false;` directly after the line `if (!buf.active || buf.reInjectPending || buf.userCancelled || userCancelled) return false;` (keeps `reInjectWhy(...)==RI_OK <=> shouldReInject(...)` for the default gate).
- deferStep (pure, mutates the accrual fields; place after reInjectWhy):
```cpp
constexpr uint32_t kDeferRetryMs = 250, kDeferMaxMs = 5000;
enum DeferStep : uint8_t { DEFER_WAIT, DEFER_RETRY, DEFER_GIVE_UP };
inline DeferStep deferStep(ActionBuffer& b, bool paused, uint32_t now) {
    const uint32_t dt = now - b.deferTick; b.deferTick = now;       // wrap-safe
    if (!paused) b.deferUnpausedMs += dt < 1000u ? dt : 1000u;      // paused time is free (nothing pops while paused)
    if (b.deferUnpausedMs >= kDeferMaxMs) return DEFER_GIVE_UP;
    if (paused) return DEFER_WAIT;
    return now - b.deferLastTry >= kDeferRetryMs ? DEFER_RETRY : DEFER_WAIT;
}
```
- QueueHud.cpp `serverCast` Before (literal): `static bool serverCast(const qh::QueuedAction& a, int slot, uintptr_t tbl) {`, the chain line `else if (!rd(SiteSrvCast + 1, gameaddr::ServerIdMask, 0, &mask)) bad = "id_mask";`, and the tail `if (bad) QLOGV(chMain, KPLOG_WARN, "server-route cast refused for member %d cmd=%u why=%s", slot, (unsigned)a.cmdId, bad);\n    return bad == nullptr;`. After: `enum class SendResult : uint8_t { Sent, Retry, Refused };` (Refused = permanent) declared above serverCast; `static SendResult serverCast(const qh::QueuedAction& a, int slot, uintptr_t tbl, bool retry)`; add `bool perm = false;` next to `const char* bad = nullptr;` and make the chain line `else if (!rd(SiteSrvCast + 1, gameaddr::ServerIdMask, 0, &mask)) { bad = "id_mask"; perm = true; }`; the WARN line becomes `if (bad && !retry) QLOGV(...)` (retries stay quiet, the SRV_CAST trace line still prints per attempt); final `if (!bad) return SendResult::Sent; return perm ? SendResult::Refused : SendResult::Retry;` (`not_commandable`, `no_server_creature`, engine 0 = transient).
- dispatchAction. Before (literal): `void dispatchAction(const qh::QueuedAction& a, int k) {` and in the `slot != 0` block `if (slot > 0 && g_cfg.companionServerCast && qh::serverRouteEligible(a)) {\n                serverCast(a, slot, tbl);\n            } else if (g_logOn && g_cfg.traceQueue) {\n ...not_ctrl trace... }\n            return;`. After: `SendResult dispatchAction(const qh::QueuedAction& a, int k, bool retry = false) {`; `if (slot > 0 && g_cfg.companionServerCast && qh::serverRouteEligible(a)) return serverCast(a, slot, tbl, retry);\n            if (g_logOn && g_cfg.traceQueue) { ...the same not_ctrl trace... }\n            return SendResult::Sent;` (unchanged behaviour). Client route: after the final `g_dispatching = false;` add `return SendResult::Sent;` (client wrappers return void; see Deviation). `dispatchAction` has no forward declaration; declare `SendResult` above `serverCast`.
- New helper right after dispatchAction:
```cpp
// RQ5: settle the active action after a send. Retry keeps it active and defers the next send; Refused (permanent) drops it with a DROP line.
static void afterSend(int k, qh::ActionBuffer& buf, SendResult r, DWORD now) {
    if (r == SendResult::Sent) { buf.dispatchDeferred = false; return; }
    if (r == SendResult::Refused) {
        if (g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_REFUSED, 1); QTRACE("%s", line); }
        endAction(buf);
        return;
    }
    if (!buf.dispatchDeferred) { buf.dispatchDeferred = true; buf.deferTick = now; buf.deferUnpausedMs = 0; }
    buf.deferLastTry = now;
}
```
- Tick wiring (`QueueHud.cpp:~1821`, inside `if (buf.active) {`). Before: `if (riWhy == qh::RI_OK) {`. After, in order:
  1. riWhy computation (`:1799`) Before (literal) `qh::ReInjectWhy riWhy = qh::reInjectWhy(buf, actionInQueue, hasFollowLeader, s.paused, now);` -> After the call is `qh::reInjectWhy(buf, actionInQueue, hasFollowLeader, s.paused, now, false, qh::RiGate{ true, g_cfg.pauseEatenWait, g_cfg.pauseReinject })`.
  2. New first branch (Before, literal, 12 spaces indent: `if (riWhy == qh::RI_OK) {` directly under `if (buf.active) {`):
```cpp
            if (buf.dispatchDeferred) {   // RQ5: a refused server cast waits and retries; no verdict, no re-inject meanwhile
                const qh::DeferStep ds = qh::deferStep(buf, s.paused, now);
                if (ds == qh::DEFER_GIVE_UP) {
                    if (g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_REFUSED, 1); QTRACE("%s", line); }
                    endAction(buf);
                } else if (ds == qh::DEFER_RETRY) {
                    const SendResult r = dispatchAction(buf.activeAction, k, true);
                    if (r == SendResult::Sent) buf.markDispatched(now);
                    afterSend(k, buf, r, now);
                }
            } else if (riWhy == qh::RI_OK) {
```
  3. RI_OK body, Before (literal, inside the `else {` after the unreadable-creature check):
```cpp
                    buf.activeAction.reInjectCount++;
                    g_lastOurs[k].tag = "reinject"; g_lastOurs[k].tick = GetTickCount();
                    QLOG(chMain, "re-injecting eaten %s action for member %d (id=%08x, target=%08x, attempt=%d) cmd=%u",
                         buf.activeAction.kind == qh::ActionKind::Spell ? "spell" : "item",
                         k, buf.activeAction.clientItem, buf.activeAction.targetId, buf.activeAction.reInjectCount,
                         (unsigned)buf.activeAction.cmdId);
                    buf.reInjectPending = true;
                    dispatchAction(buf.activeAction, k);
                    buf.activeStartTick = now;
                    buf.reInjectPending = false;
```
     After (R1: `paused=%d` goes BEFORE `cmd=`, the tail stays `cmd=%u` with no `why=`, so `_REINJECT_OLD` in `scripts/queue_hud_log_check.py:191` still matches and `normalize_old` still yields `attempt=`; do NOT add `why=ri_ok`):
```cpp
                    buf.activeAction.reInjectCount++;
                    buf.lastReInjectTick = now;
                    g_lastOurs[k].tag = "reinject"; g_lastOurs[k].tick = GetTickCount();
                    QLOG(chMain, "re-injecting eaten %s action for member %d (id=%08x, target=%08x, attempt=%d, paused=%d) cmd=%u",
                         buf.activeAction.kind == qh::ActionKind::Spell ? "spell" : "item",
                         k, buf.activeAction.clientItem, buf.activeAction.targetId, buf.activeAction.reInjectCount,
                         s.paused ? 1 : 0, (unsigned)buf.activeAction.cmdId);
                    buf.reInjectPending = true;
                    const SendResult r = dispatchAction(buf.activeAction, k);
                    buf.activeStartTick = now;
                    buf.reInjectPending = false;
                    afterSend(k, buf, r, now);
```
  4. `case qh::AV_EATEN_REINJECT:` (`:1893`), Before (literal) `dispatchAction(buf.activeAction, k);\n                        buf.markDispatched(now);\n                        break;`. After (RV2 anchors on this exact text):
```cpp
                        const SendResult r = dispatchAction(buf.activeAction, k);
                        buf.markDispatched(now);
                        afterSend(k, buf, r, now);
                        break;
```
  5. Next dispatch (`:1956`), Before (literal) `dispatchAction(nextA, k);` (right after `buf.activeAction = nextA;\n buf.markDispatched(now);`, which stay). After: `const SendResult r = dispatchAction(nextA, k);\n                    afterSend(k, buf, r, now);`.
  6. Hand-off send (`:1441`), Before (literal) `dispatchAction(a, k);` (followed by `outWatchArm(k, a.cmdId, now);`). After: `const SendResult r = dispatchAction(a, k); if (r != SendResult::Sent && g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, a.cmdId, k, qh::DR_REFUSED, 1); QTRACE("%s", line); }` (combat: no deferral; a Retry is dropped).
  7. Edge reset (RQ2 block below): `buf.dispatchDeferred = false;` next to `buf.active = false; buf.seenInQueue = false; buf.eaten = false; buf.reInjectPending = false;`.
  Deferred retry (step 2) success keeps `buf.markDispatched(now)` here only because RV1 stands alone; RV2 replaces it with `buf.markReInjected(now, qh::flagBits(s.aa8, s.ab8))` (R4: markDispatched would zero the re-inject budget).
- Edge cases: paused -> deferStep accrues nothing, no retry; unpause continues the 5 s budget; Cancel/clear reset via cancel()/clear(); leader switch to this member -> dispatchAction slot==0 takes the client route on the next retry (fine); combat edge mid-deferral -> edge reset above (deferred head resent through hand-off, see edgeHeadAction `deferred`); pause_reinject=0 -> RI_OFF falls through to the verdict branch (eaten -> UNSEEN as before for the verdict path); pause_eaten_wait=1 + paused -> RI_PAUSED falls to verdict (waits). Tick wrap: all compares use unsigned subtraction.
- Logging: trace `REINJECT_NO cmd= m= why=deferred|off|paused|settle|spacing` (existing edge-triggered line, no code); `DROP cmd= m= why=refused n=1` / `why=executed`; per-attempt `SRV_CAST ... ok=0 why=<reason>` (existing); one WARN per first refusal (existing text). Checker docs `scripts/queue_hud_log_check.py:31` REINJECT_NO why list: append `deferred|off|paused|settle|spacing`. No parser change for REINJECT_NO/DROP `why` (it is `\w+`); the follower-wipe REINJECT line keeps its tail (R1), so none there either. The only checker logic change is flag I for `DROP why=executed|refused` (R2, in the checker bullet below).
- Ini: `queue_hud.ini:47-51` pause_reinject comment append `Also gates the follower-wipe re-inject.`; add `; Refused companion Force-power sends are retried every 250 ms for 5 s (unpaused), then dropped (DROP why=refused).`

### RQ2: pure edgeHeadAction at the combat edge
- Files/anchors: `QueueHudLogic.h` after `heldHeadAction` (`:162-165`); `QueueHud.cpp:1322-1338` (edge block), `:1360-1385` (held branch), `Handoff` struct `:1195`.
- Pure helper:
```cpp
// Combat-start edge: what to do with the active (non-engine-queued) head. NONE = no active head. WAIT = hold it one stage (engine entry may still appear).
enum EdgeHeadAction : uint8_t { EDGE_NONE, EDGE_RESEND, EDGE_WAIT, EDGE_DROP_EXEC };
inline EdgeHeadAction edgeHeadAction(bool active, bool inEngine, bool sawExecuting, bool sawExec, uint32_t ageMs, bool gatherOk, bool deferred = false) {
    if (!active) return EDGE_NONE;
    if (!gatherOk) return EDGE_WAIT;                     // unreadable engine state: not proof it is absent
    if (inEngine) return EDGE_WAIT;                      // held path (caller applies handoff_resend_held)
    if (deferred) return EDGE_RESEND;                    // a refused send never reached the engine
    if (sawExecuting || sawExec) return EDGE_DROP_EXEC;  // it ran and left: a re-send would use it twice
    if (ageMs <= kMinActiveMs) return EDGE_WAIT;         // same settle window as activeVerdict
    return EDGE_RESEND;
}
```
- Handoff struct (literal Before ends `... int heldUnpausedMs = 0; qh::QueuedAction head; int total = 0; int sent = 0; };`): append `bool waiting = false; DWORD waitUntil = 0;` after `int sent = 0;` (reset by `h = Handoff{}`).
- Edge block Before (literal, `:1326-1336`; three separate spots in this order):
```cpp
            h.headHeld = buf.active && inEngine && g_cfg.handoffResendHeld;   // engine-held head: wait for the combat-start wipe, then re-send
            h.edgeAt = now;
            h.lastTick = now;
            h.heldUnpausedMs = 0;
            h.sawExec = gEdgeOk && execEvidence(s);
            h.headValid = buf.active && (!inEngine || h.headHeld);
```
  After: delete the `h.headHeld = ...` line (first line above); keep `h.edgeAt/lastTick/heldUnpausedMs`; replace the last two lines (`h.sawExec = ...;` and `h.headValid = ...;`) by (`if (h.headValid) h.head = buf.activeAction;` stays right after):
```cpp
            h.sawExec = gEdgeOk && execEvidence(s);
            const qh::EdgeHeadAction eh = qh::edgeHeadAction(buf.active, inEngine, buf.sawExecuting, h.sawExec,
                                                             now - buf.activeStartTick, gEdgeOk, buf.dispatchDeferred);
            const bool waitOk = eh == qh::EDGE_WAIT && (!inEngine || g_cfg.handoffResendHeld);
            h.headHeld = waitOk;
            h.headValid = waitOk || eh == qh::EDGE_RESEND;
            h.waiting = waitOk && !inEngine;     // not in the engine yet: give a late entry until waitUntil
            h.waitUntil = h.waiting ? (gEdgeOk ? buf.activeStartTick : now) + qh::kMinActiveMs : 0;
            if (eh == qh::EDGE_DROP_EXEC && g_logOn && g_cfg.traceQueue) {
                char line[200]; qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_EXECUTED, 1); QTRACE("%s", line);
            }
```
  Then the existing reset line `buf.active = false; buf.seenInQueue = false; buf.eaten = false; buf.reInjectPending = false;` gets `buf.dispatchDeferred = false;` appended.
- Held branch Before (literal, `:1385`): `const qh::HeldHeadAction act = qh::heldHeadAction(engineHoldsAction(s), h.sawExec, s.paused, h.heldUnpausedMs, cancelClick);`. After: compute once above it
```cpp
            if (engineHoldsAction(s)) h.waiting = false;   // it showed up: from now on a vanish means a real wipe
            const bool holds = engineHoldsAction(s) || (h.waiting && static_cast<int>(h.waitUntil - now) > 0);
```
  and pass `holds` instead of `engineHoldsAction(s)`. Leave the `heldUnpausedMs` clock on the engine truth.
- Edge cases: `!gEdgeOk` (WAIT up to 500 ms, then RESEND/DROP via heldHeadAction as exec evidence accrues in the held branch, which already latches `h.sawExec`); `handoff_resend_held=0` with inEngine keeps old behaviour (headValid=false); head already executed -> DROP why=executed, buffered commands still go (`h.total` counts buf.count only); combat edge while paused: age huge, not inEngine -> RESEND as before.
- Logging: `DROP cmd=<n> m=<k> why=executed n=1`; existing `HANDOFF_HELD ... act=wait|resend|drop` covers the wait outcome.

### RQ3: pure detachableMask excluding the head's own round node
- Files/anchors: `QueueHudLogic.h` after `keepPathAllowed` (`:1214`); `QueueHud.cpp:1294` (fwd decl), `:1373-1376` (call), `:1521-1536` (detachRound).
- Pure helpers:
```cpp
// Is round node n the held head's own node? Spell: sub 9 + spell id. Item: sub 10 + item id compared on the low 24 bits (client vs server object id).
inline bool nodeIsHead(const RNode& n, const QueuedAction& head) {
    if (head.kind == ActionKind::Spell) return n.subType == 9 && n.spellId == head.spellId;
    return n.subType == 10 && (n.itemId & 0xFFFFFFu) != 0 && (n.itemId & 0xFFFFFFu) == (head.clientItem & 0xFFFFFFu);
}
// Bit i set = rn[i] may be detached. Only the FIRST node matching the head is kept (a second identical node is the player's new click).
inline unsigned detachableMask(const RNode* rn, int n, const QueuedAction& head) {
    unsigned m = 0; bool headSeen = false;
    if (!rn) return 0;
    for (int i = 0; i < n && i < kMaxRound; ++i) {
        if (!headSeen && nodeIsHead(rn[i], head)) { headSeen = true; continue; }
        m |= 1u << i;
    }
    return m;
}
```
- Call site Before (literal, `:1373-1376`): `if (g_cfg.handoffDetachHeld && g_cfg.handoffKeepNew && h.headValid && h.headHeld && !h.detachTried && qh::keepPathAllowed(true, s.paused, g_cfg.handoffKeepUnpaused) && s.nrn > 0) {\n            h.detachTried = true;   // once per hand-off: ...\n            detachRound(k, srv, s, now, "held_head");\n        }`. After: compute `const unsigned dmask = qh::detachableMask(s.rn, s.nrn, h.head);` before the `if`, replace `s.nrn > 0` by `dmask != 0`, call `detachRound(k, srv, s, now, "held_head", dmask)`; update the fwd decl `void detachRound(int k, uintptr_t srv, const Snapshot& s, DWORD now, const char* why, unsigned mask);`.
- detachRound Before (literal): guard line `if (d.n != 0 || s.nrn <= 0 || !roundListHandle(srv, s.round, &w)) { traceDetach(k, s.nrn, "detach", false, "guard"); return; }` and the tail `if (ok) { for (int i = 0; i < s.nrn; ++i) { d.ptr[i] = got[i]; d.rn[i] = s.rn[i]; } d.n = s.nrn; d.at = now; }\n    else for (int i = 0; i < popped; ++i) if (got[i]) push(w, got[i]);   // put it back exactly as it was\n    traceDetach(k, s.nrn, "detach", ok, why);`. After (pop all `s.nrn` as before, because ActListRemoveHead can only take from the front; push the kept head nodes straight back, preserving relative order since the list is empty after popping). Replace the guard line with the first line below and the tail with the rest; `popped`/`got`/`ok` and the pop loop are unchanged:
```cpp
    if (d.n != 0 || s.nrn <= 0 || mask == 0 || !roundListHandle(srv, s.round, &w)) { traceDetach(k, s.nrn, "detach", false, "guard"); return; }
    // ... pop loop and the `ok` identity check stay as they are ...
    int nDet = 0;
    if (ok) {
        for (int i = 0; i < s.nrn; ++i) {
            if (mask & (1u << i)) { d.ptr[nDet] = got[i]; d.rn[nDet] = s.rn[i]; ++nDet; }
            else push(w, got[i]);                       // the head's own node stays in the list
        }
        d.n = nDet; d.at = now;
    } else for (int i = 0; i < popped; ++i) if (got[i]) push(w, got[i]);
    traceDetach(k, ok ? nDet : s.nrn, "detach", ok, why);
```
  `d.ptr`/`d.rn` capacity is kMaxRound (unchanged).
- Edge cases: only the head node in the list -> mask 0 -> no detach, `detachTried` stays false so a later foreign click still detaches; kept head node keeps `engineHoldsAction` true (held, no RESEND) and `detachTick` waits (`oursNodes` counts it) until it runs, then reattaches the rest; combat-start wipe that removed the node -> nothing to exclude, all detachable (old behaviour); R7: if the wipe removes the head's queue entry but NOT its round node (doc 47: ClearAllActions never empties the round list), the kept node keeps `engineHoldsAction` true, so the hand-off waits `kHeldStuckMs` (3 s) then `HANDOFF_HELD act=drop` (head lost, never doubled) and an orphan 9/10 node may remain; list mismatch -> everything pushed back exactly as before; item-id low-24 match is an assumption (see OPEN).
- Logging: `ROUND_DETACH m=<k> n=<nDet> act=detach ok=1 why=held_head` (n now counts only detached non-head nodes).

## Tests
Edit `queue_hud_logic_test.cpp` (all new tests are plain functions in the anon namespace; add one call each in `main` near `testUnifiedActionMake();` at `:2742` / `testHeldHeadAction();` `:2770`).
- Mechanical: `perl -pi -e 's/makeSpellAction\(([^,()]+), ([^,()]+), ([^,()]+), [^,()]+, ([^,()]+), ([^,()]+), ([^,()]+), ([^,()]+), /makeSpellAction($1, $2, $3, $4, $5, $6, $7, 0, /g' queue_hud_logic_test.cpp` (drops the old 4th arg, adds flag 0 after pos). Fix by hand (parenthesised arg 3): `:2674` -> `makeSpellAction(0x1000, 0x28, (int)0x8000086Bu, 0, 0, 0, none, 0, 0, 21)`, `:2676` -> `(int)kInvalidObjectId, 0, 0, 0, none, 0, 0, 22)`. `:2678` is matched by the regex.
- Remove `spellReplayPos` from the Config equality (`:398`) and `CFG_FLIP(spellReplayPos);` (`:408`).
- `testUnifiedActionMake` (`:1412-1438`): `sp = makeSpellAction(0x2000, 0x55, 3, 2, 4, 5, pos, 7, 77)`; `KT_CHECK_EQ(sp.targetId, 3u)` (mirror of arg2, was 9u); `KT_CHECK_EQ(sp.flag, 7)`; delete the hasPos/replayPos checks and add `testReplayArgs`: for `a = makeSpellAction(0x2000,0x55,3,0,0,0,pos{1,2,3},0,1)` -> `buildReplayArgs(a).pos == a.pos`, `!= nullptr`, `[0..2] == 1,2,3`, flag 0; for `makeSpellAction(...,nullptr,5,1)` -> pos non-null and all 0, flag 5; `sizeof` unchanged check not needed.
- fmtSpellArgs goldens (`:2450-2456`) replaced by: `{1,2,3}` hook -> `"SPELL_ARGS where=hook cmd=9 m=0 spell=55 tgt=3 multi=0 meta=2 dom=0 pos=3f800000,40000000,40400000 flag=0 posaddr=12ab30"`; zero pos replay flag 5 m=-1 arg2=-1 -> `"SPELL_ARGS where=replay cmd=4 m=? spell=1a3 tgt=ffffffff multi=0 meta=0 dom=0 pos=0,0,0 flag=5 posaddr=19f000"`; `pos3 == nullptr` prints `pos=0,0,0`.
- Config test (`:2548-2552`): default `!c.spellReplayPosSeen`; `parseIniLine(c, "spell_replay_pos=0") && c.spellReplayPosSeen && c.handoffResendHeld` (no other field changes).
- `whyName` test (`:1668-1675`): add `DR_REFUSED -> "refused"`, `DR_EXECUTED -> "executed"`, and `RI_DEFERRED/OFF/PAUSED/SETTLE/SPACING -> "deferred"/"off"/"paused"/"settle"/"spacing"` next to `:1652-1660`.
- `testReInjectEachCondition` (`:1518`): keep the legacy check, reword comment to "(no gate) paused/now are ignored", then add `testReInjectGate` (base: active, `activeStartTick=1000`, `activeAction.reInjectCount=0`, hasFollowLeader=true, `RiGate g{true,true,true}`): paused now=2000 -> RI_PAUSED; same with `g.pauseEatenWait=false` -> RI_OK; now=1500 -> RI_SETTLE, now=1501 -> RI_OK; `lastReInjectTick=1800`: now=2049 -> RI_SPACING, now=2050 -> RI_OK; `g.pauseReinject=false` -> RI_OFF; `dispatchDeferred=true` -> RI_DEFERRED and legacy `shouldReInject` false; `hasFollowLeader=false` -> RI_NO_LEADER; `actionInQueue=true` -> RI_IN_QUEUE (gate never masks it).
- `testDeferStep` (fresh `ActionBuffer b; b.dispatchDeferred=true; b.deferTick=1000; b.deferLastTry=1000;`): now=1100 unpaused -> WAIT (`deferUnpausedMs==100`); now=1250 -> RETRY; paused now=1300 -> WAIT and `deferUnpausedMs` unchanged; `deferUnpausedMs=4900, deferTick=1000`, now=1100 -> GIVE_UP; `4899` -> not GIVE_UP; `deferTick=1000`, now=9000 unpaused -> `deferUnpausedMs==1000` (dt cap); tick wrap: `deferTick=0xFFFFFFF0u`, now=0x10 -> dt 32.
- `testEdgeHeadAction` (args `active,inEngine,sawExecuting,sawExec,ageMs,gatherOk,deferred`): `(false,..)` -> NONE; `(t,f,f,f,900,t)` -> RESEND; `(t,f,f,f,80,t)` -> WAIT; ageMs 500 -> WAIT, 501 -> RESEND; `(t,f,t,f,80,t)` and `(t,f,f,t,900,t)` -> DROP_EXEC; `(t,t,t,t,900,t)` -> WAIT (held path decides); `(t,f,t,t,900,f)` -> WAIT (gather failed); `(t,f,f,f,80,t,deferred=t)` -> RESEND.
- `testDetachableMask` (`N(sub,ptr,spell,item)` helper exists): head Spell 0x55: `{N(9,1,0x55)}` -> 0; `{N(9,1,0x55),N(9,2,0x66)}` -> 0b10; `{N(9,1,0x66),N(9,2,0x55)}` -> 0b01; `{N(9,1,0x55),N(9,2,0x55)}` -> 0b10 (only first kept); head Item clientItem 0x80000860: `{N(10,1,0,0x00000860)}` -> 0; `{N(10,1,0,0x860),N(1,2)}` -> 0b10; `{N(1,1),N(10,2,0,0x860)}` -> 0b01; unrelated item 0x861 -> 0b1; n=0 / rn=nullptr -> 0; n=4 all foreign -> 0b1111.
- Log checker (`scripts/queue_hud_log_check.py`): `:46` doc line, `:130` FIELD_RE -> `"pos": r"[0-9a-fA-F]+,[0-9a-fA-F]+,[0-9a-fA-F]+", "flag": r"-?\d+", "posaddr": HEXV` (drop `posptr`, drop `arg2` use), `:167` REQUIRED `"SPELL_ARGS": ("where","cmd","m","spell","tgt","multi","meta","dom","pos","flag","posaddr")`. `scripts/test_queue_hud_log_check.py`: `spell_args()` helper (`:957`) and `SPELL_GOLDEN` (`:968-970`) -> new format (e.g. `... spell=2c tgt=0 multi=1 meta=0 dom=0 pos=0,0,0 flag=0 posaddr=12ab30`), update asserts at `:985-986` (`recs[1]["f"]["pos"] == "0,0,0"`, `["flag"] == "0"`), and the `bad` list at `:996-1001` (old `posptr=`/`pos=orig` forms are now malformed; add `pos=1,2` and `flag=x` as bad forms; the `where=cast` / `spell=zz` / `tgt=-` cases stay bad). K flag is unaffected (uses only `where`).
- Checker test for R1 (next to `:574`): `Q.parse_lines([(1, "re-injecting eaten item action for member 1 (id=00001234, target=00000000, attempt=2, paused=0) cmd=7")])` -> one record `REINJECT` with `cmd == 7`, `m == 1`, `f["attempt"] == "2"`, no `why` key, no skipped (guards the line order against future format changes).
- Checker flag I (R2): QH23's pass case emits `HANDOFF active=<cmd>` then only `DROP cmd=<cmd> why=executed` (no `HANDOFF_ONE`, no `HANDOFF_HELD act=drop`), so flag I would fire "trigger lost". In `handoff_trigger_flags` (`queue_hud_log_check.py:~463`) Before (literal) `for h, follow in handoff_windows(recs, ("HANDOFF_ONE", "HANDOFF_HELD")):` and
  `settled = any(r["cmd"] == cmd and (r["kw"] == "HANDOFF_ONE" or (r["kw"] == "HANDOFF_HELD" and r["f"]["act"] == "drop"))\n                      for r in follow)`. After:
```python
    for h, follow in handoff_windows(recs, ("HANDOFF_ONE", "HANDOFF_HELD", "DROP")):
        ...
        settled = any(r["cmd"] == cmd and (r["kw"] == "HANDOFF_ONE"
                                           or (r["kw"] == "HANDOFF_HELD" and r["f"]["act"] == "drop")
                                           or (r["kw"] == "DROP" and r["f"].get("why") in ("executed", "refused")))
                      for r in follow)
```
  Docstring line 65 (flag I): append `or DROP cmd=<n> why=executed|refused`; the `handoff_trigger_flags` docstring likewise. `scripts/test_queue_hud_log_check.py`: next to `test_I_quiet_when_held_drop_settles_active_without_handoff_one` (`:~889`; helpers `handoff`, `drop` exist) add `test_I_quiet_when_active_dropped_executed` (`[handoff(0, "8", 0, "-"), drop(8, 0, "executed")]` -> `[]`), `test_I_quiet_when_active_dropped_refused` (same with `"refused"`), `test_I_fires_when_active_dropped_for_other_reason` (`drop(8, 0, "cancel")` -> the usual flag-I line at L1).
- PLAY rows for `patch_manager_mods/PLAY_TEST_QUEUE.md` (all after installing 0.4.6-dev; check with `python3 scripts/kotor2_logs.py check queue-hud --new`):
  - QH21 (RQ1): pause, queue Force Heal (self) + Force Push (enemy), unpause. Pass: `SPELL_ARGS where=hook` and `where=replay` show the same `tgt=`, `pos=0,0,0 flag=0`, `posaddr` differs (hook stack addr vs replay &a.pos); both `SPELL_OUT exec=1`.
  - QH22 (RQ5): companion (not leader) buffered Force power while it is non-commandable (stun/cutscene) or 0 FP. Pass: one `SRV_CAST ... ok=0 why=...`, `REINJECT_NO why=deferred`, then `SRV_CAST ok=1` after recovery or exactly one `DROP ... why=refused` at ~5 s; no `re-injecting eaten` lines for that cmd.
  - QH23 (RQ2): hotbar medpac, provoke combat within 0.3 s. Pass: exactly one stack decrement; either `DROP why=executed` or a single `HANDOFF_ONE` for that cmd (the checker must print no flag I for the executed case, R2).
  - QH24 (RQ3): auto-pause on combat start ON, buffered medpac, combat begins while paused. Pass: `ROUND_DETACH act=detach n=` counts only non-head nodes (or no detach line), one medpac used. R7: if `HANDOFF_HELD act=drop` follows after about 3 s with no `HANDOFF_ONE`, record it (OPEN: the wipe leaves the head's round node).

## Tooling
- `python3 scripts/run_tests.py queue_hud` (runs `queue_hud_logic_test`, `common_contract_test` and `test_queue_hud_log_check.py`); fast loop: `python3 scripts/run_tests.py queue_hud_logic_test`.
- `python3 scripts/compat_check.py patch_manager_mods/_shared_src/GameAddr.h` (comment-only edit, still run it).
- `scripts/pre_install_gate.sh --mod queue_hud_src` must print `GATE PASS` before any `kpm_hotswap.py --apply` (install needs the user's GO; never skip the fresh gate after `--prepare`).
- After build, `rg -n "replayPos|hasPos|spellReplayPos|posPtrRaw" patch_manager_mods/queue_hud_src` must return only the `spellReplayPosSeen` lines.

## Deviations from doc 47
- RQ5 "dispatchAction returns bool": it returns `SendResult {Sent, Retry, Refused}` (still bool-equivalent: Sent = true) because `id_mask` must drop at once while the other refusals defer.
- RL12 client-route refusal logging is NOT done: `PerformUseItem/PerformCastSpell` return void and no cheap stack/FP before-after probe exists, so the client route reports `Sent`; only the server route (SRV_CAST ok=0 / DROP refused) becomes visible.
- RQ2 adds a 4th result `EDGE_NONE` and a 7th arg `deferred` (a head whose send was refused never reached the engine, so it resends at once instead of waiting).
- RQ3 pops all `s.nrn` nodes and pushes the head's node back (ActListRemoveHead can only take from the front) instead of removing only the foreign nodes.

## Risks / ordering
- Same functions are edited by RV3 (RQ8 restore unsent head, RQ9 detach at combat end, RQ10 detached owner, RQ11 click, RQ12): `handoffTick` edge block, `detachRound`/`detachTick`, `Handoff` struct, `ActionBuffer::cancel/clear`. Land RV1 first; RV3 must rebase onto the new `detachRound(..., mask)` signature and the `Handoff.waiting/waitUntil` fields. RV4 changes `ActQueue.h`/selfheal, not these functions, but its queue-hud half calls `dispatchAction` (new return type and default `retry` param).
- `makeSpellAction` and `fmtSpellArgs` signatures change: every caller in `QueueHud.cpp` (hook x2, replay) and the test file must compile in the same commit; no other mod calls them.
- Existing in-game logs from 0.4.5-dev no longer parse as SPELL_ARGS in the checker (strict new REQUIRED); logs keep 2 sessions only, accepted.

## OPEN
- Orphan head node after a combat-start wipe (RQ3 + R7): the kept node leaves `engineHoldsAction` true; HELD_DROP after 3 s loses the head and a 9/10 orphan may stay. RV2 ORPHAN only runs while `bufBusy`; consider running the orphan clock when `h.pending` too.
- Item-id low-24-bit match in `nodeIsHead` (client vs server object id) is unproven; no match = the old detach-all behaviour (fails safe).

## Review corrections applied
Source: `review_RV12.md` (folded in place; the executor applies this spec top-to-bottom, no separate review file).
- R1 [BLOCKING]: RQ5 step 3 follower-wipe line is `... attempt=%d, paused=%d) cmd=%u` (field before `cmd=`, no `why=`); full Before/After written; checker regex unchanged; parse test added.
- R2 [BLOCKING]: checker flag I treats `DROP why=executed|refused` as settled; code, docstring and three tests written (QH23 pass case).
- R3 [BLOCKING]: lands in RV2 (keep-mask `removeRoundNodes`, kick give-up gates); RV1 only exports `nodeIsHead` which RV2 reuses.
- R4 [MINOR]: deferred-retry success keeps `markDispatched` here; RV2 swaps it to `markReInjected` (note in RQ5 step 7).
- R7 [MINOR]: QH24 pass criteria now record `HANDOFF_HELD act=drop` after about 3 s; RQ3 edge cases describe the kept-node branch; OPEN section added.
- R9 [MINOR]: Before-snippets are literal copies (`qh::QueuedAction` in `serverCast`, dispatch spell branch, hook fragments, makeSpellAction/fmtSpellArgs/replayPos, edge block, held branch, detach call site and tail, RI_OK body, AV_EATEN_REINJECT text that RV2 anchors on).
- R5, R6, R8: RV2 only.
