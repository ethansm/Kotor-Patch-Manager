# RV4 spec: selfheal vs queued Force powers (RQ13) + wiped-pending end
Mod(s) + version bump: companion-selfheal 0.9.3-dev -> 0.9.4-dev (patch_manager_mods/companion_selfheal_src/manifest.toml `version =`; CompanionSelfHeal.cpp:1564 KP_LOG(chMain, "0.9.3-dev ui=...") string; no ini header carries a version (checked: rg "0\.9\.3" hits only those two); stale "0.7.0-dev" at CompanionSelfHeal.cpp:1 and README "0.6.0-dev" stay).
Shared file touched: patch_manager_mods/_shared_src/ActQueue.h (add one pure helper; no layout/offset change) + _shared_src/SelfHealUi.h (pure helpers). Dependents of ActQueue.h: queue-hud, companion-ai, companion-selfheal, cmdq_probe (compat_check below).
Deps: RV1 (queue-hud) only for ordering; RV1 may also edit ActQueue.h comments, so re-read the anchors with `rg -n "useQueued|starved" patch_manager_mods/_shared_src/ActQueue.h` before pasting.

## Changes

### RQ13a (V2-4): `actq::castQueued` + selfheal dedupe refuses queued Force powers
- Files/anchors (re-anchor by text; line numbers from the current tree):
  1. `_shared_src/ActQueue.h` after `useQueued` (`inline bool useQueued` at line 54, ends line 57 `return s.haveCur && s.cur.itemId != 0;\n}`; `firstIndex` is defined above it).
  2. `_shared_src/SelfHealUi.h:455-458` (`enum class UseRefusal`, `useRefusalText`, `useAllowed`).
  3. `companion_selfheal_src/CompanionSelfHeal.cpp:402-407` (useNow, CS12 dedupe block).
- Before (ActQueue.h:54-57):
```
inline bool useQueued(const Snapshot& s) {
    for (int i = 0; i < s.nrn && i < kMaxRound; ++i) if (s.rn[i].itemId != 0) return true;
    return s.haveCur && s.cur.itemId != 0;
}
```
- After (insert directly below `useQueued`; `useQueued` and `starved` unchanged; no RnSub constants):
```
// RQ13: a cast/use is stalled behind FollowLeader: first 0x3F at index 1..3 with only 0x3D before it (same shape as starved()) AND a pending
// ROUND-LIST node (rn[], never cur: cur goes stale, see QueueHudLogic.h curReal/kStaleCurMs) has sub-type 9 (cast) or 10 (item). Covers queue-hud's
// server-route Force power (0x3F + node 9, itemId 0). An executing cast (0x3F at the head) and a bare leftover 0x3F do NOT count.
inline bool castQueued(const Snapshot& s) {
    const int k = firstIndex(s, T_USE_ITEM);
    if (k < 1 || k > 3) return false;
    for (int i = 0; i < k; ++i) if (s.e[i].type != T_FOLLOW_LEADER) return false;
    for (int i = 0; i < s.nrn && i < kMaxRound; ++i) if (s.rn[i].subType == 9 || s.rn[i].subType == 10) return true;
    return false;
}
```
- Before (SelfHealUi.h:455-458):
```
enum class UseRefusal { None, AlreadyQueued };
inline const char* useRefusalText(UseRefusal r) { return r == UseRefusal::AlreadyQueued ? "already queued" : ""; }
// A second heal use is refused while one is still queued (it would stack behind the stalled first).
inline UseRefusal useAllowed(bool queued) { return queued ? UseRefusal::AlreadyQueued : UseRefusal::None; }
```
- After:
```
enum class UseRefusal { None, AlreadyQueued, CastQueued };
inline const char* useRefusalText(UseRefusal r) { return r == UseRefusal::AlreadyQueued ? "already queued" : r == UseRefusal::CastQueued ? "cast queued" : ""; }
// A second heal use is refused while one is still queued (it would stack behind the stalled first). castPending = another cast/use
// (queue-hud's Force power, a hotbar item) is stalled behind FollowLeader on that creature (RQ13, actq::castQueued): refused too. The item case reports first.
inline UseRefusal useAllowed(bool queued, bool castPending = false) { return queued ? UseRefusal::AlreadyQueued : castPending ? UseRefusal::CastQueued : UseRefusal::None; }
```
- Before (CompanionSelfHeal.cpp:405, inside `{ // CS12 dedupe` block of useNow): `        shui::UseRefusal ur = shui::useAllowed(actq::useQueued(qs));`
- After: `        shui::UseRefusal ur = shui::useAllowed(actq::useQueued(qs), actq::castQueued(qs));`
  (line 406 unchanged: it already prints `REFUSE %s member=%d (%s)` via useRefusalText, so the new line is `REFUSE cast queued member=1 (auto)`.)
- Edge cases: failed `actq::read` -> empty snapshot -> both false -> allowed (same as today, QDIAG line logs the read failure under trace). Pause/unpause: pure read, no state. Leader (mi==0): its server queue holds no 0x3D, so `castQueued` needs k>=1 and is always false there (an in-flight leader cast sits at the head; not a stacking risk; unchanged behaviour). Followers only refuse in the STALLED shape that `starved()` detects. NOT refused (by design, R1): an executing cast (0x3F at the head), a bare leftover 0x3F behind 0x3D with an empty round list, a stale flagless `cur` node (rn[] only), a combat-AI cast chain (executing at the head) so combat heals are not starved by 6 s back-offs. Auto path: a refused `useNow` backs off one 6 s cooldown (`g_auto[i].lastUseMs = now`, CompanionSelfHeal.cpp:914/944). Selfheal's own queued item is a subset of castQueued but useAllowed reports AlreadyQueued first (log text unchanged for the old case). Tick wrap: no ticks involved.
- Logging: INFO (REFUSEI) `REFUSE cast queued member=<mi> (<src>)`; with `[queue] trace=1` the existing `QDIAG dedupe ...` line precedes it. scripts/selfheal_log_check.py REFUSE_RE (`^REFUSE\s*(.*?)\s*$`) already matches; its refuse_queued counter (selfheal_log_check.py:201 `why.startswith("already queued")`) must also count the new text: change to `why.startswith(("already queued", "cast queued"))` and do NOT touch the V06 lines/asserts (test_selfheal_log_check.py:172 `refuse_queued == 1` and :175 `REFUSE-queued=1` stay); cover the new text in its own new test (see Tooling/Tests below).

### RQ13b (V2-8): end the pending at once when its use vanished from an emptied queue (`DONE ... reason=wiped`), no re-fire inside the cooldown
- Files/anchors: `_shared_src/SelfHealUi.h:~403-412` (`struct Pending`) and new helper after `pendingDone2` (ends ~line 443); `CompanionSelfHeal.cpp:778-791` (pendingTick follower loop `for (int i = 1; i <= 2; ++i)`). Member 0 (leader) loop after it is NOT changed (see Deviation).
- Design: a follower with a loaded area always has 0x3D at the head, so after a normal completion its queue is not empty. ServerClearAllActions (queue-hud strip Cancel, QueueHud.cpp:941-966) leaves queue AND round list empty. Signature of a wipe = the pending was seen queued at least once, now not queued, and the whole snapshot is empty (`ne==0 && nrn==0 && !haveCur`, with `round != 0` proving the reads worked) continuously for >= 400 ms. HP up / stack down (pendingDone2) is evaluated first and wins, so a heal that landed is never reported as wiped.
- Before (SelfHealUi.h Pending):
```
struct Pending {
    bool on = false;
    uint32_t since = 0;
    int preHp = 0, preStack = 0;
};
```
- After:
```
struct Pending {
    bool on = false;
    bool seenQueued = false;   // RQ13: the use was observed queued (round node present) since arming; the caller sets it, `Pending()` at arming clears it
    uint32_t since = 0;
    int preHp = 0, preStack = 0;
};
```
- New pure helper in full (SelfHealUi.h, directly after `pendingDone2`):
```
// RQ13 wiped-pending: an outside Cancel (queue-hud strip) cleared the creature's whole action queue while our use was pending. True when the use
// WAS seen queued, is not queued now, the whole queue is empty (queueEmpty: ne==0, no round node, no current node, round readable) and that has
// lasted kWipedGraceMs. unqueuedSince is the g_unqueuedSince stretch start that pendingDone2 arms (0 = not running). Call after pendingDone2 returned false.
constexpr uint32_t kWipedGraceMs = 400;
inline bool pendingWiped(const Pending& p, uint32_t now, bool queued, bool queueEmpty, uint32_t unqueuedSince) {
    if (!p.on || !p.seenQueued || queued || !queueEmpty || unqueuedSince == 0) return false;
    const uint32_t t = now ? now : 1u;   // same 0-sentinel rule as pendingDone2
    return (uint32_t)(t - unqueuedSince) >= kWipedGraceMs;
}
```
- Before (CompanionSelfHeal.cpp:784-790; line 782 `else stack = g_pend[i].preStack;` and 778-783 are NOT part of the replaced range):
```
        actq::Snapshot qs;
        if (i < cnt) actq::read(v[i].server, &qs, SiteQueue, SiteRound);
        bool queued = actq::useQueued(qs);
        if (!shui::pendingDone2(g_pend[i], now, hp, stack, queued, &g_unqueuedSince[i])) continue;
        bool progress = hp > g_pend[i].preHp || (stack >= 0 && stack < g_pend[i].preStack) || (stack < 0 && g_pend[i].preStack > 0);
        KP_LOG(chCall, "DONE member=%d hp%+d stack%+d queued=%d", i, hp - g_pend[i].preHp, (stack < 0 ? 0 : stack) - g_pend[i].preStack, (int)queued);
        if (!progress) KP_LOGV(chCall, KPLOG_WARN, "TIMEOUT unqueued member=%d", i);
```
- After (replace from the line `actq::Snapshot qs;` through the line `if (!progress) KP_LOGV(chCall, KPLOG_WARN, "TIMEOUT unqueued member=%d", i);` (currently 784-789); do not touch 778-783 (incl. `else stack = ...`) or the reset line 790 `g_pend[i].on = false; ...`, which stays exactly as is):
```
        actq::Snapshot qs;
        const bool qok = i < cnt && actq::read(v[i].server, &qs, SiteQueue, SiteRound);
        bool queued = actq::useQueued(qs);
        if (queued) g_pend[i].seenQueued = true;
        bool done = shui::pendingDone2(g_pend[i], now, hp, stack, queued, &g_unqueuedSince[i]);
        bool wiped = false;
        if (!done && shui::pendingWiped(g_pend[i], now, queued, qok && qs.round && qs.ne == 0 && qs.nrn == 0 && !qs.haveCur, g_unqueuedSince[i])) {
            done = wiped = true;
            g_auto[i].lastUseMs = now;   // restart the auto cooldown (6 s): a Cancel is not a reason to fire the potion again at once
        }
        if (!done) continue;
        bool progress = hp > g_pend[i].preHp || (stack >= 0 && stack < g_pend[i].preStack) || (stack < 0 && g_pend[i].preStack > 0);
        KP_LOG(chCall, "DONE member=%d hp%+d stack%+d queued=%d%s", i, hp - g_pend[i].preHp, (stack < 0 ? 0 : stack) - g_pend[i].preStack, (int)queued, wiped ? " reason=wiped" : "");
        if (!progress && !wiped) KP_LOGV(chCall, KPLOG_WARN, "TIMEOUT unqueued member=%d", i);
```
  `g_auto[i].pending = false` is already in the unchanged reset line 790 (`g_pend[i].on = false; g_auto[i].pending = false; g_unqueuedSince[i] = 0; ...`), which also clears kick state (kick stops: no pending).
- Edge cases: Cancel then the engine re-adds 0x3D (open question, V2-8): queue not empty -> `wiped` never fires; the old 6 s not-queued path (pendingDone2 timeout) still ends it, unchanged behaviour (safe fallback). Leader switch / party reorder: `swapOwner(i, v[i].server)` runs first (line ~777) and resets Pending (seenQueued cleared) for a newcomer. Unreadable queue (`qok` false or round==0): never wiped. Pause: nothing pops while paused, queue stays queued -> seenQueued stays, no wipe; a Cancel while paused wipes -> fires after 400 ms of (unpaused or paused) time because the stretch uses GetTickCount `now`; fine (the queue is genuinely empty). Heal landed in the same frame as the wipe: pendingDone2 returns true first, plain DONE, no `reason=wiped`. Tick wrap: unsigned subtraction + 0 sentinel as pendingDone2. Use consumed normally on a follower: 0x3D remains -> queue not empty -> no false wipe.
- Logging: default level (KP_LOG): `DONE member=2 hp+0 stack+0 queued=0 reason=wiped`; no `TIMEOUT unqueued` follows. Update scripts/selfheal_log_check.py:67 `DONE2_RE` to `r"^DONE member=(\S+) hp(\S+) stack(\S+) queued=(\S+)(?: reason=(\S+))?\s*$"` (existing groups g[0..3] unchanged; g[4] = reason or None; store `"reason": g[4]` in the v["done"] dict at line ~160) and add a case to scripts/test_selfheal_log_check.py (below the V06 tests, ~line 190): lines `USE member=2 item=0c000100 "Medical Supplies" heal=27 hp=12/40 src=click`, `DONE member=2 hp+0 stack+0 queued=0 reason=wiped` -> `r["v"]["done"][0]["reason"] == "wiped"`, `timeout_unq == 0`, verdict PASS. Mention `DONE ... reason=wiped` in README "Logs" tags list (README.md:70) and one README sentence that `REFUSE cast queued` means a cast/use is queued behind follow (not "any cast").

## Tests
- Unit, `_shared_src/actqueue_test.cpp` (suite `actqueue_test`), insert after line 59 (`{ Snapshot s; s.cur.itemId = 0; s.haveCur = true; KT_CHECK(!useQueued(s)); }`, last useQueued check) and before `// ---- read() on a fake creature` (line 61), inside main(); `snapOf` (line 17) and `KT_CHECK` exist; `snapOf` leaves nrn=0 so nodes are set by hand:
```
    // castQueued (RQ13): stalled behind FollowLeader (first 0x3F at index 1..3, only 0x3D before it) + a pending ROUND node of sub-type 9/10
    { Snapshot s = snapOf({0x3D, 0x3F}); s.rn[0].subType = 9; s.nrn = 1; KT_CHECK(castQueued(s)); KT_CHECK(!useQueued(s)); }      // Force power: node 9, itemId 0
    { Snapshot s = snapOf({0x3D, 0x3F}); s.rn[0].subType = 10; s.nrn = 1; KT_CHECK(castQueued(s)); }                              // item node
    { Snapshot s = snapOf({0x3D, 0x3F}); s.rn[0].subType = 3; s.nrn = 1; KT_CHECK(!castQueued(s)); }                              // other sub-type
    { Snapshot s = snapOf({0x3D, 0x3F}); s.nrn = 0; KT_CHECK(!castQueued(s)); }                                                   // bare leftover 0x3F, empty round list
    { Snapshot s = snapOf({0x3D}); s.rn[0].subType = 9; s.nrn = 1; KT_CHECK(!castQueued(s)); }                                   // node but no 0x3F
    { Snapshot s = snapOf({0x3F}); s.rn[0].subType = 9; s.nrn = 1; KT_CHECK(!castQueued(s)); }                                   // 0x3F at the head = executing
    { Snapshot s = snapOf({0x3D, 0x40, 0x3F}); s.rn[0].subType = 9; s.nrn = 1; KT_CHECK(!castQueued(s)); }                       // non-0x3D ahead of the 0x3F
    { Snapshot s = snapOf({0x3D, 0x3F}); s.cur.subType = 9; s.haveCur = true; KT_CHECK(!castQueued(s)); }                        // stale cur node never counts
    KT_CHECK(!castQueued(Snapshot()));
```
- Unit, `_shared_src/selfheal_ui_test.cpp` (suite `selfheal_ui_test`): the `useAllowed` asserts are lines 368-371 (last one: `KT_CHECK(std::string(useRefusalText(UseRefusal::None)).empty());` at 371) inside the block that already declares `p` and `u`, closed by `}` at 372. Insert the RQ13 block after line 371 as its own braces (not at ~332-356, which is above the useAllowed lines); name the inner Pending `pw` (no shadowing of the outer `p`):
```
        // RQ13: useAllowed(castPending) + pendingWiped
        KT_CHECK(useAllowed(false, true) == UseRefusal::CastQueued && std::string(useRefusalText(UseRefusal::CastQueued)) == "cast queued");
        KT_CHECK(useAllowed(true, true) == UseRefusal::AlreadyQueued && useAllowed(true) == UseRefusal::AlreadyQueued && useAllowed(false) == UseRefusal::None);
        {
            Pending pw; pw.on = true; pw.since = 1000; pw.seenQueued = true;
            KT_CHECK(pendingWiped(pw, 10400, false, true, 10000));          // 400 ms of empty queue
            KT_CHECK(!pendingWiped(pw, 10399, false, true, 10000));         // 399 ms
            KT_CHECK(!pendingWiped(pw, 20000, true, true, 10000));          // queued again
            KT_CHECK(!pendingWiped(pw, 20000, false, false, 10000));        // queue not empty (0x3D left: normal completion)
            KT_CHECK(!pendingWiped(pw, 20000, false, true, 0));             // stretch not armed
            KT_CHECK(pendingWiped(pw, 0x00000100u, false, true, 0xFFFFFF00u));   // tick wrap: 0x200 ms
            Pending q = pw; q.seenQueued = false; KT_CHECK(!pendingWiped(q, 20000, false, true, 10000));   // never seen queued
            Pending off = pw; off.on = false; KT_CHECK(!pendingWiped(off, 20000, false, true, 10000));
        }
```
  (`std::string` is already used in that block; add `#include <string>` only if the compile complains.)
- Unit, `companion_selfheal_src/common_contract_test.cpp` (CS12 block, after line 300, before its closing `}`): use a NEW local `actq::Snapshot qc; qc.e[0].type = 0x3D; qc.e[1].type = 0x3F; qc.ne = 2;` (do not reuse `qa`, whose e[0] is mutated at line 289). Checks:
  `HCHECK("actq::castQueued: 0x3D,0x3F + round node sub-type 9 true, bare leftover 0x3F false, 0x3F at head false; shui::useAllowed(false,true) == CastQueued with text \"cast queued\"", !actq::castQueued(qc) && (qc.rn[0].subType = 9, qc.nrn = 1, actq::castQueued(qc)) && (qc.e[0].type = 0x3F, qc.e[1].type = 0x3D, !actq::castQueued(qc)) && useAllowed(false, true) == UseRefusal::CastQueued && strcmp(useRefusalText(UseRefusal::CastQueued), "cast queued") == 0)`;
  `HCHECK("shui::pendingWiped: 400 ms of empty queue after a seen-queued use ends it, 399 ms does not", (Pending pw; pw.on = true; pw.seenQueued = true; pendingWiped(pw, 10400, false, true, 10000) && !pendingWiped(pw, 10399, false, true, 10000)))` (if a declaration inside the HCHECK expression does not compile, hoist `Pending pw` above it); `HEQ("kWipedGraceMs is 400", (long long)kWipedGraceMs, 400LL)`.
- Python, `scripts/test_selfheal_log_check.py`: leave `test_v060_kick_pass` (V06 list, asserts at :172 and :175) UNCHANGED. Add two new tests after `test_v060_kick_pass` (before `test_v060_starved_and_timeout_are_warn_not_fail`, line 179), using the `v30(...)` helper as the V06 test does:
  1. `test_v094_refuse_cast_queued`: `r = S.check(v30(['REFUSE cast queued member=1 (auto)']))` -> verdict PASS, `r["v"]["refuse_queued"] == 1`, `r["v"]["refuse"]["cast queued member=1 (auto)"] == 1`, `"REFUSE-queued=1" in S.fmt(r)` (same shape as `test_v060_kick_that_did_not_reach_head_warns`).
  2. `test_v094_done_reason_wiped`: `r = S.check(v30(['USE member=2 item=0c000100 "Medical Supplies" heal=27 hp=12/40 src=click', "DONE member=2 hp+0 stack+0 queued=0 reason=wiped"]))` -> `r["v"]["done"][0]["reason"] == "wiped"`, `r["v"]["timeout_unq"] == 0`, `r["verdict"] == "PASS"`, and `not r["v"]["done"][0]["timeout"]` (a wiped DONE is a non-timeout in `fmt`). A second DONE line `DONE member=1 hp+27 stack-1 queued=0` yields `reason is None`.
- PLAY rows for patch_manager_mods/PLAY_TEST_QUEUE.md:
  - SH-RQ13a: companion 1 at ~40% HP with Auto on (use below 50%), queue-hud on, paused: queue a Force power (Force Heal) on that companion's strip, unpause. Pass: selfheal log `REFUSE cast queued member=1 (auto)` (no `USE member=1` while the spell node is pending) and one potion total after the spell resolves.
  - SH-RQ13b: queue a selfheal heal on a starved follower (item still queued: `QDIAG`/`REFUSE already queued` on a second click), click that strip's Cancel in queue-hud. Pass: one line `DONE member=1 hp+0 stack+0 queued=0 reason=wiped` within ~1 s of the click, NO `TIMEOUT unqueued member=1` 6 s later, no `AUTO member=1` for >= 6 s after the DONE. If the engine re-adds 0x3D after the clear the line will not appear and the old `TIMEOUT unqueued` does: record that in doc 47 open question 2.

## Tooling
- `python3 scripts/run_tests.py actqueue_test selfheal_ui_test` (host suites; both scoped), then `python3 scripts/run_tests.py patch_manager_mods/companion_selfheal_src/common_contract_test.cpp` (id from `run_tests.py --list | rg companion_selfheal`), then `python3 -m pytest scripts/test_selfheal_log_check.py -q` (or `python3 scripts/run_tests.py test_selfheal_log_check`).
- Dependents of the shared header: `python3 scripts/compat_check.py patch_manager_mods/_shared_src/ActQueue.h patch_manager_mods/_shared_src/SelfHealUi.h` (expect queue_hud, companion_ai, companion_selfheal, cmdq_probe, SelfHealPage.h users); rebuild/run `python3 scripts/run_tests.py queue_hud companion_ai` (their common_contract_test pins ActQueue.h offsets; the added constexprs must not collide: `rg -n "castQueued" patch_manager_mods` must show only ActQueue.h, SelfHealUi.h (param `castPending`), CompanionSelfHeal.cpp and the three test files).
- `scripts/pre_install_gate.sh --mod companion_selfheal_src` -> GATE PASS (install itself is RV7, needs user GO).
- Commit explicit paths: ActQueue.h, SelfHealUi.h, actqueue_test.cpp, selfheal_ui_test.cpp, CompanionSelfHeal.cpp, companion_selfheal_src/{manifest.toml,common_contract_test.cpp,README.md}, scripts/selfheal_log_check.py, scripts/test_selfheal_log_check.py, PLAY_TEST_QUEUE.md rows.

## Risks / ordering
- Order: (1) ActQueue.h + actqueue_test, (2) SelfHealUi.h + selfheal_ui_test, (3) CompanionSelfHeal.cpp (dedupe line, pendingTick, version string) + manifest + common_contract_test, (4) checker regex + test + README, (5) run all suites, compat_check, gate.
- Deviation 1 (scope): the wiped end is applied to followers (i=1..2) only. The leader's server queue is normally empty after a completed use (no 0x3D), so "queue empty" cannot tell a Cancel from a completion there, and queue-hud's Cancel on the leader strip clears its own buffer, not the server queue the leader's selfheal use sits in. Opus text says "when its use vanished from an emptied queue", which this implements for the only mod/path where it can happen.
- Deviation 2 (grace): not "at once" but 400 ms (`kWipedGraceMs`) of continuous empty queue, so a transient unreadable/mid-update snapshot or the one-frame gap between a node pop and the HP/stack update cannot yield a false wipe; still ~15x faster than the 6 s timeout.
- Behaviour change to accept: castQueued refuses selfheal only for a cast/use STALLED behind FollowLeader (first 0x3F at index 1..3 behind only 0x3D, pending rn[] node 9/10), the shape that would stack a second use. It never refuses for an executing cast, a bare leftover 0x3F, a stale `cur` node, or the leader. Not a regression for item-only users.
- Same-function conflicts: RV1/RV2 edit queue-hud only; RV4 edits CompanionSelfHeal.cpp/SelfHealUi.h which no other RV phase touches (RV6 docs batch touches only comments/docs). ActQueue.h: if RV1 added helpers, paste `castQueued` after `useQueued` anyway; no ordering dependency beyond the clean merge.

## Review corrections applied
- R1: castQueued narrowed to the stalled shape (first 0x3F at index 1..3 behind only 0x3D + rn[] node 9/10, never cur); RnSub constants dropped; actqueue_test/common_contract_test vectors replaced; risk text and edge cases rewritten; README/doc 47 wording is "queued behind follow".
- R2: RQ13b replacement re-anchored by text (`actq::Snapshot qs;` through the `TIMEOUT unqueued` line, 784-789); `else stack = ...` and reset line 790 untouched; the Before block no longer contains the reset line.
- R3a: V06 test and its asserts (:172 `refuse_queued == 1`, :175 `REFUSE-queued=1`) left alone; new `REFUSE cast queued` and `reason=wiped` cases live in two new tests; `reason` stored as g[4].
- R3b: selfheal_ui_test RQ13 block inserted after line 371 (the useAllowed asserts at 368-371) as its own braces, inner Pending named `pw`.
