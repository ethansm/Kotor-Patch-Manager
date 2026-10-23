# Review RV4 (selfheal + ActQueue.h/SelfHealUi.h) and RV5 (action-hotbar)
Reviewer: Sonnet skeptic. Read-only except this file. Code checked at the current tree (HEAD 76653c96 + dirty worktree).

## Checked and OK (no action)
- Includers of the shared headers (rg -l): ActQueue.h <- queue_hud (QueueHud.cpp, common_contract_test), companion_selfheal (cpp + common_contract_test), companion_ai (CompanionAI.cpp, playerPaused only), cmdq_probe, actqueue_test. SelfHealUi.h <- queue_hud (QueueHud.cpp, QueueHudLogic.h, tests), SelfHealPage.h, companion_selfheal, selfheal_ui_test. `castQueued`, `RnSubCastSpell`, `RnSubUseItem` are not defined anywhere else; `useAllowed(bool, bool = false)` keeps every existing caller (only CompanionSelfHeal.cpp:405 calls it outside tests). No other mod changes behaviour; queue_hud/companion_ai/cmdq_probe only recompile. `Pending` is never aggregate-initialised (CompanionSelfHeal.cpp:338/339/421/422 use `Pending()` then field sets), so the new member is safe.
- RQ13b: `seenQueued` is cleared by every arming (`g_pend[mi] = shui::Pending()`, :421) and by swapOwner (:339); pendingWiped vectors (10400/10399, wrap 0x100 vs 0xFFFFFF00 = 512 ms) follow from the code. Leader exclusion is correct (leader queue is empty after any completed use).
- RH2: `makeScale` has exactly one production caller (ActionHotbar.cpp:1551 -> g_scale -> unitRect in createLabel only). Hover/tooltip/picker maths use getExtent pixel rects (:1032, :1172), not layout units. Existing ultrawide rows are untouched: for every c != 1.0 the new body is the same formula (hotbar_logic_test:399-415 and 423 unchanged). Only exact-4:3 non-800x600 changes (:419, already rewritten by the plan; vectors 1.28 -> {5,611,25,44} and 2.0 -> {8,956,40,70} follow from `(int)(x*sx)`). Cosmetic: constants in px (hb::TipPad) do not scale at 4:3; not a blocker.
- RH1 contract check: commit ec95c98e (saveevents migration) changed the old `replaceLiveProfiles(nullptr, ...)` on SS_Absent into "keep live"; README:86 and the comment at ActionHotbar.cpp:1801-1802 still document "empty bar". RH1 restores the documented behaviour, not a new one.

## R1 [BLOCKING] plan_RV4 §RQ13a  castQueued can refuse selfheal forever (stale cur node + permanent 0x3F leftover) and starves combat heals
Problem: `castQueued` = "any 0x3F in the queue AND a pending/cur node with sub-type 9/10". Two facts in the repo make that unsafe:
(a) A bare 0x3F stays behind 0x3D forever on followers (ActQueue.h:50-51 comment, R8 Q3). A flagless sub-type 9/10 `cur` node goes stale and is NOT a real action (QueueHudLogic.h:1490-1499 `curReal`, kStaleCurMs). Leftover 0x3F + stale cur => castQueued true indefinitely => every manual click and every auto use prints `REFUSE cast queued` until the round resets. The plan's own test vector (`s.cur.subType = 9; haveCur = true` => true) bakes this in. `useQueued` has the same stale-cur exposure for itemId, but it is item-only and was live-proven; the cast variant is new and unproven.
(b) The plan admits it also refuses while the companion's own combat AI has a spell node pending. In combat a Force-user companion chains casts; each refusal on the auto path sets `g_auto[i].lastUseMs = now` (CompanionSelfHeal.cpp:915) = a 6 s back-off, so an auto heal at <50% HP can be delayed by several 6 s steps. "Short refuse" is not true for chained casts.
What the defect (RQ13) actually needs: refuse when the cast is STALLED behind FollowLeader (the shape `starved()` already detects), i.e. the stacking case. An executing cast sits at the queue head and is not a stacking risk.
Required correction (replace the whole After block for ActQueue.h; drop the RnSub constants; keep the name):
```
// RQ13: a cast/use is stalled behind FollowLeader: first 0x3F at index 1..3 with only 0x3D before it (same shape as starved()) AND a pending
// ROUND-LIST node (rn[], never cur: cur goes stale, curReal) has sub-type 9 (cast) or 10 (item). Covers queue-hud's server-route Force power
// (0x3F + node 9, itemId 0). An executing cast (0x3F at the head) and a bare leftover 0x3F do NOT count.
inline bool castQueued(const Snapshot& s) {
    const int k = firstIndex(s, T_USE_ITEM);
    if (k < 1 || k > 3) return false;
    for (int i = 0; i < k; ++i) if (s.e[i].type != T_FOLLOW_LEADER) return false;
    for (int i = 0; i < s.nrn && i < kMaxRound; ++i) if (s.rn[i].subType == 9 || s.rn[i].subType == 10) return true;
    return false;
}
```
Tests (actqueue_test) replace the plan's list with: `{0x3D,0x3F}` + rn[0]=9,nrn=1 => true (and `!useQueued`); same with 10 => true; sub-type 3 => false; `{0x3D,0x3F}` nrn=0 => false; `{0x3D}` + node => false; `{0x3F}` + node 9 (head, executing) => false; `{0x3D,0x40,0x3F}` + node 9 => false; `{0x3D,0x3F}` with only `cur.subType=9, haveCur` => false; `Snapshot()` => false. In companion_selfheal common_contract_test use `qc.e[0].type = 0x3D; qc.e[1].type = 0x3F; qc.ne = 2;` before the rn[0].subType=9 step. PLAY row SH-RQ13a is unchanged (paused leader/follower strip queue, follower => 0x3D,0x3F). Note `README`/doc 47 should say "queued behind follow", not "any cast".

## R2 [MINOR] plan_RV4 §RQ13b  wrong line anchors in the pendingTick replacement
Problem: the plan says "replace lines 782-789". Actual: 782 is `else stack = g_pend[i].preStack;`; `actq::Snapshot qs;` is at CompanionSelfHeal.cpp:784, `if (!shui::pendingDone2` at 786, `if (!progress) KP_LOGV(... TIMEOUT unqueued member=%d", i)` at 789, reset line at 790. An executor following the numbers deletes the `else stack = ...` line.
Required correction: say "replace from the line `actq::Snapshot qs;` through the line `if (!progress) KP_LOGV(chCall, KPLOG_WARN, "TIMEOUT unqueued member=%d", i);` (currently 784-789, inside `for (int i = 1; i <= 2; ++i)`); do not touch 778-783 or the reset line 790". Re-anchor by text, as RV5 does.

## R3 [MINOR] plan_RV4 §Tests  existing assertions and insertion point
(a) scripts/test_selfheal_log_check.py:172 asserts `v["refuse_queued"] == 1` and :175 asserts the string `REFUSE-queued=1`. Adding `REFUSE cast queued member=1 (auto)` to V06 makes both 2; the plan only mentions "== 2". Required: change both literals to 2 (or put the new REFUSE line in its own new test and leave V06 alone, simpler). Also add the `reason=wiped` test below the V06 tests as planned; `DONE2_RE` change keeps g[0..3], and `v["done"]` entries need `"reason": g[4]` (:160) - the fmt at :616 counts `timeout`, so a wiped DONE counts as non-timeout (correct).
(b) selfheal_ui_test.cpp: the `useAllowed` asserts are at :368-371, inside the same braces as the pendingDone2 block that already declares `p` and `u`. Insert the RQ13 block after :371 as its own `{ ... }` (the plan's "~332-356" is above the useAllowed lines); inner `Pending p` shadows the outer `p` (legal, but name it `pw` to avoid -Wshadow noise).

## R4 [MINOR] plan_RV5 §RH1  a slow or lost snapshot can make a real save look absent
Problem: deferral path ActionHotbar.cpp:2108-2123: when `ev->folder` is empty or not yet a directory, one global pending slot is armed (`g_pendingSaveSnapshot`) and consumed by the next HUD ticks; it gives up after `++g_pendingSaveRetries > 60` HUD TICKS (about 1 s at 60 fps, 0.2 s at 300 fps), and a second save/autosave event before the first resolved overwrites `g_pendingSaveDir/Why` (the first save never gets a copy). Under RH1 any such save loads later as an empty bar and mergeSnapshot drops ALL profiles (not just the current character's). The immediate path (folder already exists, normal case) is fine. Recovery exists only via `.load.bak` (RL26 helps).
Required correction (small, same files):
1. Make the give-up time-based: add `DWORD g_pendingSaveSince = 0;` set at both `g_pendingSaveRetries = 0` sites in snapshotToSave (:1816, :1824) and replace the three `++g_pendingSaveRetries > 60` tests with `GetTickCount() - g_pendingSaveSince > 10000`.
2. In snapshotToSave, before arming, if `g_pendingSaveSnapshot` is already true log WARN `persave: <why>: previous pending snapshot (<dir>) replaced` (keeps the loss visible).
3. In restoreFromSave, first line after the folder check: if `g_pendingSaveSnapshot` is set and its dir (or `dir`) is an existing directory, call `snapshotToSave(...)` once to flush it before `pickSnapshot`.
Add to PLAY row AHR1: "save then load the same slot within 1 s": expect the bar kept.

## R5 [MINOR] plan_RV5 §RH4  the 40% heal guess is speculative; use "skip members with a live pending heal"
Problem: PendingHealPct=40 is unrelated to the item (medpac 27 HP vs advanced pack, other max HP) and gives inconsistent results: a 20% member counts as 60% and is skipped, a 5% member counts as 45% and is healed again (double heal on the lowest member, the exact complaint). It also adds a new chooser (`chooseTargetPending`, copy into `Candidate t[8]`) and a `n > 8` guard for no gain.
Simpler fix (no engine read, no magic number): a member with a live pending heal is ineligible for the next press.
```
// HotbarLogic.h (replace the plan's constants + chooseTargetPending; keep pendingHealLive and PendingHealMs)
constexpr uint32_t PendingHealMs = 5000;
inline bool pendingHealLive(uint32_t now, uint32_t since, int hpNow, int hpAtPress) { return now - since < PendingHealMs && hpNow <= hpAtPress; }
```
```
// ActionHotbar.cpp targetedUse, replacing the plan's `pend[]`/chooseTargetPending block (keep mids[], g_pendHeal, notePendingHeal):
    DWORD nowT = GetTickCount();
    for (int i = 0; i < n; ++i)
        for (PendHeal& p : g_pendHeal) if (p.id && p.id == mids[i]) {
            if (hb::pendingHealLive(nowT, p.since, v[i].cur, p.hp)) { c[i].eligible = false; KP_LOG(chTarget, "target: member %d id %08x has a pending heal (age %lu ms), skipped", i, (unsigned)p.id, (unsigned long)(nowT - p.since)); }
            else { KP_LOGV(chTarget, KPLOG_DEBUG, "target: pending heal cleared member id %08x", (unsigned)p.id); p.id = 0; }
        }
    int idx = hb::chooseTarget(c, n, 0, thr, fbSelf);
```
(`c[i].eligible` is already honoured by chooseTarget and slotEnabled; fallbackSelf behaviour is the same as in the plan.) Unit test: Candidates {20/100,30/100,80/100}, thr 50: eligible all => 0; c[0].eligible=false => 1; c[0],c[1] false => -1 (fallback off) / 0 only if self eligible (fallback on); pendingHealLive vectors as in the plan.
Also fix a bug in the plan's `notePendingHeal`: the loop breaks on the FIRST empty slot even when a later slot already holds `id`, creating a duplicate. Do two passes: first look for `id == id`, then for an empty slot, then the oldest.

## R6 [MINOR] plan_RV5 §RL24  EntryUsable gate on the @lowhp route may block a working feature
Problem: `directRouteAllowed` forces the vanilla invoke when HUD bit0 is clear for UR_LowHpDirect too. The bit is the HUD entry's usable flag for the CONTROLLED character; nothing in the repo shows it is independent of the leader's own state (e.g. an item that is "unusable" for a full-HP leader would stop healing a wounded companion). useOnMember already asks the engine (`canUseItem` on the TARGET member) which is the correct check for @lowhp. Required: gate only `UR_SelfDirect` (`directRouteAllowed(r, flags) = r != UR_SelfDirect || (flags & EntryUsable)`), leave @lowhp to canUseItem; update the RL24 unit vectors (`(UR_LowHpDirect,0) == true`). Keep the new `showRefusal` for the existing `CanUseItem refused` branch. Optional: log `usable=` first (already in the `slot ...` resolve line) and decide from AHR6.

## R7 [MINOR] plan_RV5 §RL25  a deleted INI must not block editing
Problem: `g_cfgStale` is set on any `c.Load` failure, including a missing file. Before, a deleted INI was harmless: the in-memory g_cfg stays and `assignSlot`'s Save recreates the file. With the plan the player cannot bind anything after deleting the file until a game load rewrites it (the plan accepts this and calls it a feature).
Required correction: set `g_cfgStale = true` only when `st.exists` (file present but unreadable/locked); for a missing file keep the old WARN ("keeping previous") and leave `g_cfgStale = false`. The stamp restore (`g_cfgPoll.last/primed`) is still right for the exists case. AHR6 wording: "lock/unreadable INI" instead of "delete the INI".

## Notes for the executor (no change needed)
- RH5, RL23, RL26, RL27, RL28, RL29, RL30, RL31 snippets match the current code shape (KP_LOG_EVERY exists; `g_frame` is `long` at :277; FileWatch.h needs no edit: `ReloadPoll.last/primed` are public).
- Order inside RV5 as written is right (RH4 before RL24 on useOnMember). If R5/R6 are applied, drop `chooseTargetPending` and the PendingHealPct constant from the plan and from the RH4 unit-test list.

VERDICT: RV4 not safe to apply until R1 (narrow castQueued to the stalled-behind-0x3D shape using rn[] only) is applied; RV5 is safe with R4-R7 folded in (all MINOR).
