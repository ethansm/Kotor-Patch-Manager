REVIEW companion-ai | files read: 14 | findings: 5

### R2-CA-01 - spinEnd leaves stale BurstTrack counters on CrTrack, causing a false E1 SPIN/END cycle on the next attack
- Class: reachable
- Severity: low
- Confidence: 98
- Location: patch_manager_mods/companion_ai_src/CompanionAI.cpp:339-344 :: spinEnd
- Anchor: `void spinEnd(CrTrack& t, DWORD now) {`
- Trigger: `AIActionPhysicalAttacks` entry hook `0x006D740F` -> `AiTraceAttackEntry` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:798-808) calls `e1Evaluate(*t, now)` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:707-736) which sets `t.spinning = true` (line 715) and resets `t.burst.resetSecond(); t.exits = 0; t.secStart = now;` (line 735) -> within the next `< 1000 ms` (`now - t->secStart < 1000`), the creature spins on at least one more frame (`t->burst.onEntry(nowUs())` at line 807 sets `t.burst.maxBurst >= a1BurstThreshold`) and then stops attacking (target dies, combat ends, or area unloads) before `now - t->secStart >= 1000` -> 3000 ms later, any hook entry calls `tick(now)` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:689) -> `spinSweep(now)` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:346-350) -> `spinEnd(t, now)` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:339-344), which clears `t.spinning = false` without resetting `t.burst`, `t.exits`, or `t.secStart` -> on the creature's next `AiTraceAttackEntry` call (even a single normal attack minutes later), `now - t->secStart >= 1000` is immediately true and `e1Evaluate` sees the leftover `t.burst.maxBurst` with `!t.spinning`, logging a false `E1 SPIN` (`++g_st.e1spins`) followed later by a second `E1 END`.
- Impact: Whenever a spin ends between 1 Hz `e1Evaluate` boundaries and expires via `spinSweep` (or eviction in `track`), the leftover burst counters stay armed in `t.burst` and trigger a phantom `E1 SPIN` + `E1 END` pair and an extra `e1spins` count on the creature's next attack (the `SPIN/END/SPIN after >3.5s gaps` symptom noted in commit `b4c80a5`).
- Fix: Reset `t.burst.resetSecond()`, `t.exits = 0`, and `t.secStart = now` inside `spinEnd(CrTrack& t, DWORD now)`.
- Diff:
```diff
--- a/patch_manager_mods/companion_ai_src/CompanionAI.cpp
+++ b/patch_manager_mods/companion_ai_src/CompanionAI.cpp
@@ -341,3 +341,4 @@ void spinEnd(CrTrack& t, DWORD now) {
     if (t.spinShow)
         eline(now, "t=%lu E1 END cr=%08x %s spin lasted %.1f s", now, t.id, t.spinTag, (t.seen - t.spinStart) / 1000.0);
     t.spinShow = false;
+    t.burst.resetSecond(); t.exits = 0; t.secStart = now;
 }
```

### R2-CA-02 - CompanionAiA1Gate overwrites newId with Invalid on a1_retarget_strict rejection before probe_a1x STRICT logging runs
- Class: reachable
- Severity: low
- Confidence: 100
- Location: patch_manager_mods/companion_ai_src/CompanionAI.cpp:905-938 :: CompanionAiA1Gate
- Anchor: `newId = cai::a1GateTarget(g_cfg, newId, known && cai::a1Strict(ax, ay, tx, ty, mx));`
- Trigger: With `a1_retarget_strict=1` and `probe_a1x=1` in `companion_ai.ini`, `AIActionPhysicalAttacks` hook `0x006D853E` -> `CompanionAiA1Gate` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:881) -> `FnNearestEnemy` returns a candidate `newId != Invalid` (line 904) that lies in the 3D-minus-radii band outside the strict 2D range `(mx + 0.1f)^2` -> at lines 905-913, `cai::a1Strict(ax, ay, tx, ty, mx)` returns `false` and `cai::a1GateTarget` overwrites `newId` with `Invalid` (line 912) -> at line 924, `if (newId != Invalid && crPasses(attacker, party))` evaluates to `false` because `newId` was already set to `Invalid`.
- Impact: When `a1_retarget_strict=1` is enabled, every candidate rejected by the strict range gate is skipped by the `probe_a1x` `STRICT` block: `t->strictN` is not incremented for rejected candidates, `t->strictRej` remains permanently `0` in `A1 summary` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:402-403), `PR A1X STRICT ... -> would-reject` is never emitted, and accepted candidates duplicate the `gameObject(newId)` lookup and four `readAt<float>` reads at lines 907-911 and 926-930.
- Fix: Preserve the raw `FnNearestEnemy` candidate ID (or compute the strict check once before `a1GateTarget`) so the `probe_a1x` block at lines 924-938 inspects the candidate before `newId` is gated to `Invalid`.
- Diff:
```diff
--- a/patch_manager_mods/companion_ai_src/CompanionAI.cpp
+++ b/patch_manager_mods/companion_ai_src/CompanionAI.cpp
@@ -903,11 +903,13 @@ extern "C" int __cdecl CompanionAiA1Gate(int ebp) {
         if (cai::a1TryRetarget(g_cfg, raw, mx))
             newId = cai::a1NewTarget(reinterpret_cast<RetargetFn>(FnNearestEnemy)(attacker, mx, tgt, Invalid));
+        int candId = newId;
+        float ax = 0, ay = 0, tx = 0, ty = 0;
+        int cand = (candId != Invalid && (g_cfg.a1RetargetStrict || g_cfg.probeA1x)) ? gameObject(candId) : 0;
+        bool known = cand && saferead::readAt<float>(9, static_cast<uintptr_t>(attacker), CrPos, &ax) &&
+                     saferead::readAt<float>(9, static_cast<uintptr_t>(attacker), CrPos + 4, &ay) &&
+                     saferead::readAt<float>(9, static_cast<uintptr_t>(cand), CrPos, &tx) &&
+                     saferead::readAt<float>(9, static_cast<uintptr_t>(cand), CrPos + 4, &ty);
+        bool strictOk = known && cai::a1Strict(ax, ay, tx, ty, mx);
-        if (g_cfg.a1RetargetStrict && newId != Invalid) {   // C145 gate (a1_retarget_strict): unreadable positions = out of range
-            float ax = 0, ay = 0, tx = 0, ty = 0;
-            int cand = gameObject(newId);
-            bool known = cand && saferead::readAt<float>(9, static_cast<uintptr_t>(attacker), CrPos, &ax) &&
-                         saferead::readAt<float>(9, static_cast<uintptr_t>(attacker), CrPos + 4, &ay) &&
-                         saferead::readAt<float>(9, static_cast<uintptr_t>(cand), CrPos, &tx) &&
-                         saferead::readAt<float>(9, static_cast<uintptr_t>(cand), CrPos + 4, &ty);
-            newId = cai::a1GateTarget(g_cfg, newId, known && cai::a1Strict(ax, ay, tx, ty, mx));
-        }
+        newId = cai::a1GateTarget(g_cfg, newId, strictOk);
         frw<int>(ebp, F_NewTarget, newId);
@@ -922,19 +924,11 @@ extern "C" int __cdecl CompanionAiA1Gate(int ebp) {
         if (g_cfg.probeA1x) {   // log only: strict 2D test of the retarget candidate, and the follow-up watch (RS S6*/E2 clear it)
             t->pendingA1 = now ? now : 1;
-            if (newId != Invalid && crPasses(attacker, party)) {
-                float ax = 0, ay = 0, tx = 0, ty = 0;
-                int cand = gameObject(newId);
-                if (cand && saferead::readAt<float>(9, static_cast<uintptr_t>(attacker), CrPos, &ax) &&
-                    saferead::readAt<float>(9, static_cast<uintptr_t>(attacker), CrPos + 4, &ay) &&
-                    saferead::readAt<float>(9, static_cast<uintptr_t>(cand), CrPos, &tx) &&
-                    saferead::readAt<float>(9, static_cast<uintptr_t>(cand), CrPos + 4, &ty)) {
-                    float lim = mx + 0.1f;
-                    bool ok = cai::a1Strict(ax, ay, tx, ty, mx);
-                    ++t->strictN; if (!ok) ++t->strictRej;   // always counted (A1 summary strict=N/rejected); the line is throttled
-                    if (t->strictLog.allow(now, ok ? 1u : 0u, 10, 5000))   // first 10, then on an ok/would-reject change or every 5 s
-                        pev(PiA1x, now, "t=%lu PR A1X STRICT cr=%08x new=%08x d2=%.2f lim2=%.2f -> %s", now, id, newId,
-                            (ax - tx) * (ax - tx) + (ay - ty) * (ay - ty), lim * lim, ok ? "ok" : "would-reject");
-                }
+            if (candId != Invalid && known && crPasses(attacker, party)) {
+                float lim = mx + 0.1f;
+                ++t->strictN; if (!strictOk) ++t->strictRej;   // always counted (A1 summary strict=N/rejected); the line is throttled
+                if (t->strictLog.allow(now, strictOk ? 1u : 0u, 10, 5000))   // first 10, then on an ok/would-reject change or every 5 s
+                    pev(PiA1x, now, "t=%lu PR A1X STRICT cr=%08x new=%08x d2=%.2f lim2=%.2f -> %s", now, id, candId,
+                        (ax - tx) * (ax - tx) + (ay - ty) * (ay - ty), lim * lim, strictOk ? "ok" : "would-reject");
             }
         }
```

### R2-CA-03 - A1X follow-up hooks run tick(now) -> orphanTick(now) before a1xFollowup and gate S6A/S6C follow-ups on probe_rs
- Class: reachable
- Severity: low
- Confidence: 96
- Location: patch_manager_mods/companion_ai_src/CompanionAI.cpp:830-831 :: AiTraceFallbackFires
- Anchor: `DWORD now = GetTickCount(); tick(now);`
- Trigger: `CompanionAiA1Gate` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:923) sets `t->pendingA1 = now ? now : 1` -> the player pauses combat (or `a1_rethink_ms < 2000` is used on a retarget where `a1RethinkWanted` at CompanionAiLogic.h:509 keeps `+0x570` at 3000 ms) so wall-clock `GetTickCount()` advances past `a1RethinkMs + 1000` (`a1OrphanDue` at patch_manager_mods/companion_ai_src/CompanionAiLogic.h:659-661) before the engine's `+0x570` timer fires -> when the slot-6 follow-up hook finally runs (`AiTraceFallbackFires` at patch_manager_mods/companion_ai_src/CompanionAI.cpp:828-831 or `CompanionAiProbeRunScript` at patch_manager_mods/companion_ai_src/CompanionAI.cpp:1266-1272), it calls `tick(now)` (line 830 / 1271) -> `orphanTick(now)` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:688, 622-630) BEFORE calling `a1xFollowup` on line 831 / 1272; `orphanTick` emits `PR A1X ORPHAN` and zeroes `t.pendingA1 = 0`, so `a1xFollowup` on the very next line sees `t.pendingA1 == 0` and drops `PR A1X FOLLOWUP`. In addition, if `probe_rs=0` and `probe_a1x=1`, `CompanionAiProbeRunScript` returns at line 1267 (`if (!g_cfg.probeRs) return;`) before reaching `a1xFollowup` at line 1272.
- Impact: Any combat pause longer than 1 s (or any retarget with `a1_rethink_ms < 2000`, or `probe_rs=0` with `probe_a1x=1`) causes `orphanTick` to preempt `a1xFollowup`, logging a false `PR A1X ORPHAN` and dropping the `PR A1X FOLLOWUP` event.
- Fix: Run `a1xFollowup` before `tick(now)` in `AiTraceFallbackFires` and `CompanionAiProbeRunScript`, and allow `CompanionAiProbeRunScript` to record `a1xFollowup` when `g_cfg.probeA1x` is on even if `g_cfg.probeRs` is off.
- Diff:
```diff
--- a/patch_manager_mods/companion_ai_src/CompanionAI.cpp
+++ b/patch_manager_mods/companion_ai_src/CompanionAI.cpp
@@ -830,2 +830,3 @@ extern "C" void __cdecl AiTraceFallbackFires(int ebp) {
-    DWORD now = GetTickCount(); tick(now);
+    DWORD now = GetTickCount();
     if (g_cfg.probeA1x) a1xFollowup(rd<int>(fr<int>(ebp, -0x194), ObId), "E2", now);   // A1X: slot-6 run seen (RS S6B follows)
+    tick(now);
@@ -1267,6 +1268,7 @@ extern "C" void __cdecl CompanionAiProbeRunScript(int ret, int script, int self)
-    if (!g_cfg.probeRs) return;
+    if (!g_cfg.probeRs && !g_cfg.probeA1x) return;
     int site = cai::rsSite(static_cast<unsigned>(ret));
     if (!site) return;
     saferead::beginScope();
-    DWORD now = GetTickCount(); tick(now);
+    DWORD now = GetTickCount();
     if (site <= 3) a1xFollowup(self, cai::rsName(site), now);   // slot-6 sites: the A1 end was followed up (before the filter: review N3)
+    tick(now);
+    if (!g_cfg.probeRs) return;
```

### R2-CA-04 - CompanionAiA1Gate never sets CrTrack::party, leaving party tracks evictable when E1/E1b trace points are off
- Class: latent
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/companion_ai_src/CompanionAI.cpp:898-899 :: CompanionAiA1Gate
- Anchor: `CrTrack* t = track(id, now);`
- Trigger: With `trace=0` or `engine_points=0` in `companion_ai.ini` (so `AiTraceAttackEntry` at patch_manager_mods/companion_ai_src/CompanionAI.cpp:800 and `AiTraceAttackApproachExit` at patch_manager_mods/companion_ai_src/CompanionAI.cpp:814 return early without setting `t->party = party`), `CompanionAiA1Gate` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:895-899) reads `party = rd<int>(attacker, CrParty)` at line 895 and calls `CrTrack* t = track(id, now)` at line 898, leaving `t->party` at `0` -> when more than 32 creatures pass `CompanionAiA1Gate` (e.g. with `a1_party_only=0`), `track()` (patch_manager_mods/companion_ai_src/CompanionAI.cpp:359) treats the party member's `CrTrack` as non-party (`victim->party == 0`) and evicts it.
- Impact: Party members' `CrTrack` entries (holding `a1Log`, `strictLog`, `pendingA1`, and `wouldSpin`/`ends`/`retargets` counters) lose their party-eviction protection in `track()` when `AiTraceAttackEntry` is disabled.
- Fix: Assign `t->party = party;` in `CompanionAiA1Gate` after `CrTrack* t = track(id, now);`.
- Diff:
```diff
--- a/patch_manager_mods/companion_ai_src/CompanionAI.cpp
+++ b/patch_manager_mods/companion_ai_src/CompanionAI.cpp
@@ -898,2 +898,3 @@ extern "C" int __cdecl CompanionAiA1Gate(int ebp) {
     CrTrack* t = track(id, now);
+    t->party = party;
     ++t->wouldSpin; ++g_st.a1Would;
```

### R2-CA-05 - a1GateTarget and a1_retarget_strict have no unit tests and a1_retarget_strict is missing from companion_ai.ini
- Class: test-gap
- Severity: low
- Confidence: 100
- Location: patch_manager_mods/companion_ai_src/CompanionAiLogic.h:507 :: a1GateTarget
- Anchor: `inline int a1GateTarget(const Config& c, int newId, bool strictOk) { return (c.a1RetargetStrict && newId != Invalid && !strictOk) ? Invalid : newId; }`
- Trigger: Commit `f948e99` added `Config::a1RetargetStrict` (patch_manager_mods/companion_ai_src/CompanionAiLogic.h:218), `parseConfig` handling for `"a1_retarget_strict"` (line 269), `sameConfig` comparison (line 307), and `a1GateTarget` (line 507), without adding `a1_retarget_strict=0` to `patch_manager_mods/companion_ai_src/companion_ai.ini` (contradicting `CompanionAiLogic.h:203`: `// Every key, its default (= the shipped value) and its meaning are in companion_ai.ini.`) and without adding any assertions for `a1GateTarget` or `a1_retarget_strict` in `patch_manager_mods/companion_ai_src/companion_ai_logic_test.cpp` (`testIni` lines 42-98, `testShippedIni` lines 121-130, `testHandlerDecisions` lines 246-266).
- Impact: Breaking `a1GateTarget`, breaking `a1_retarget_strict` INI parsing, or dropping `a1RetargetStrict` from `sameConfig` cannot fail `companion_ai_logic_test.cpp`, and `companion_ai.ini` omits the `a1_retarget_strict` key.
- Fix: Add `a1_retarget_strict=0` to `companion_ai.ini`, add `"a1_retarget_strict"` to `known[]` (`knownN == 51`) in `testShippedIni()`, and test `a1GateTarget` and `a1_retarget_strict` in `testHandlerDecisions()` and `testIni()`.

## Fix check
- b4c80a5: R2-CA-01 (`spinEnd` does not reset `t.burst`, `t.exits`, or `t.secStart`, causing a false `E1 SPIN`/`E1 END` pair on the next attack after spin expiry).
- 49a57a7 looks correct.
- f948e99: R2-CA-02 (`newId` is overwritten with `Invalid` before the `probe_a1x` `STRICT` block runs, so strict rejections are never counted in `strictN`/`strictRej` or logged) and R2-CA-05 (`a1GateTarget` and `a1_retarget_strict` lack tests and `a1_retarget_strict` is missing from `companion_ai.ini`).

## Retracted
- `AiTraceAurPostString` (`CompanionAI.cpp:778-791`) buffer bounds and reentrancy: `copyStr` (`CompanionAI.cpp:108-111`) delegates to `saferead::readExoLoose` (`SafeRead.h:151-167`), which clamps the byte count to `cap - 1` (`399` into `char text[400]`) and NUL-terminates before `cai::sanitize`, and `AiTraceAurPostString` never calls back into the NWScript VM or `ExecuteCommandAurPostString`.
- `CompanionAiProbeFm3Remove` (`CompanionAI.cpp:1241-1262`), `CompanionAiBumpClear` (`CompanionAI.cpp:991-1038`), and `CompanionAiC1` (`CompanionAI.cpp:1109-1124`) removing player- or script-issued commands: `CompanionAiProbeFm3Remove` is a read-only probe (`skip_original_bytes = false`); `CompanionAiBumpClear` only preserves follow-only queues that vanilla `BumpFriends` would clear; and `CompanionAiC1` only resets the loop index to `-1` in `ClearHostileActionsVersus` after a hostile group delete so slot `0` is checked by the engine's existing target and action-type filter.
- `CompanionAiA1Gate` (`CompanionAI.cpp:903-914`) infinite retarget flip-flop within one `RunActions` slice: when `CompanionAiA1Gate` returns `1` with `newId != Invalid`, the consumed exit `0x006D7D2F` calls `InitializeNewAttackTarget`, which sets `creature+0x52C = 1`, causing vanilla `0x006D83EA` to end the attack via `EndAttackAction(1)` if the new target also reaches the approach branch on the next pass.
- `CompanionAiFm4Snap` / `CompanionAiFm4Restore` (`CompanionAI.cpp:1045-1075`) cross-creature snapshot mismatch: `Fm4Pair::begin` and `Fm4Pair::take` (`CompanionAiLogic.h:537-539`) clear `cr = 0` on every entry and restore attempt, preventing a snapshot from being restored onto a different creature.

## Not assessed
- Live binary hook window bytes, frame offsets, and struct offsets in `swkotor2.exe`: cannot be verified from source alone without the executable binary and Ghidra database (per review rules).
