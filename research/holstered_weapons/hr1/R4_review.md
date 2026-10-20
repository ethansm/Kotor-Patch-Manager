# HR1 R4 review: Fsm::onSwap and the Fsm changes the slide needs (read-only; scratch only)

Scratch (outside holster_src, nothing in the repo touched): `.claude/waves/hr1/R4_scratch/`
- `repro.cpp` includes the REAL `HolsterLogic.h` (run: `g++ -std=c++17 -o repro repro.cpp && ./repro`).
- `inc/HolsterLogic.h` is my proposed Fsm; `fixed_test.cpp` is 2188 checks, 0 fails; `mut.sh` runs 16 mutations; `t4demo.cpp` runs T4's proposed fix.

## 1. Verified / Corrected claims

Verified by running the real header (bases 0 and 0xFFFFFC00, so tick wrap is covered):
- V1 CONFIRMED (T4 Q1). Drawn, `onCombat(0,+2000)`, `onSwap(+2500)`: phase Drawing, `stowPending=0`, `lastCombat=0`. At +20000 it is Drawn with nothing pending, so the weapon stays in the hand forever. Root cause: `onSwap` (HolsterLogic.h:204) clears `stowPending` without checking `lastCombat`.
- V2 CONFIRMED. Holstered plus `onSwap` with no combat ends Drawn at +30000 (T2c probe E).
- V4 CONFIRMED (T2c A/F). `onCombat(1,1000)` then `handFraction(999)` returns 1.0. The unsigned `now - phaseStart` wraps to about 4e9. `tick(999)` also jumps to Drawn (HolsterLogic.h:185-193, 207).
- V5 CONFIRMED (T2a F2 / T2c B). A stow at fraction 0.5 plus combat re-entry gives 0.5 -> 0.0. The weapon jumps to the hip, then slides back out. Pinned at holster_logic_test.cpp:157.
- T2a F1 CONFIRMED by code reading, not run. `runDelay` (HolsterDelay.h, combat_exit_ms, default 1500) already shortens the server countdown. `onCombat` adds `kStowDelayMs` 1500 on top (HolsterLogic.h:199). The shipped gate writes the hip pose instantly (HolsterDrawGate.h:42-45). Total stack: about 3 s, plus the 350 ms slide.
- Latent only. `rg Fsm|onSwap|handFraction Holster.cpp` finds no production use (the only `onSwap` hits are the probe flag `g_onSwap` and `HolsterSwap`). The existing swap test only covers in-combat swaps (holster_logic_test.cpp:162-170).
- T4's 22 wrap vectors are plausible. I re-derived V3 (swap 2500 -> stowAt 5000) and V8 (5500 -> 8000) with T4's patch applied in scratch: both match.

Corrected or disputed:
- C1. T4's proposed fix is wrong for the player (BLOCKING). `stowPending=(lastCombat==0); stowAt=now+kSwapMs+kStowDelayMs` makes `onSwap` draw a weapon when nothing is hostile. Demonstrated in `t4demo`: Holstered plus out-of-combat swap gives phase=Drawing, in hand, with a 2850 ms visible draw and stow. In the countdown case the already-drawn weapon snaps to the mount (0.00) and then slides out again, and stowAt moves from 6500 to 8000.
- C2. T2c finding 5 and probe D ("mid-draw swap snaps 0.5 -> 0") is NOT a bug. A swap makes a new weapon object, and the new object starts at the mount. The "no snap" rule applies only to the SAME object: combat re-entry mid-stow and the stow edge mid-draw.
- C3. "Key the Fsm per owner+hand" is not needed. The drawgate cache is already one `Ent` per weapon Gob (Holster.cpp:1092, with `ent->left`). The Fsm goes inside `Ent`, per Gob. A swap = the old Gob disappears and a new `Ent` starts Holstered, then gets fed combat=1 and slides in over 350 ms. `Fsm::onSwap` and `kSwapMs` therefore stay unused in production. Keep them fixed and tested, but do not wire them.
- C4. T2a W2 has an unreadable-owner hole (BLOCKING). It feeds `hip?0:1`, and the gate returns "drawn" for an unreadable owner. A one-frame `rdU32` blip then starts a Drawing slide from the mount, which writes a hip-ish pose. That breaks the gate's fail-safe rule "unreadable = hand, never the hip". Separately, `if (ent->gate.owner != owner) fsm = {}` fires on 0 -> owner after a blip, because `gateUpdate` stores owner 0. That resets the Fsm and replays the slide. Fix in section 3.
- C5. T4's "UNVERIFIED: does the engine fire 0x0E out of combat" is still open. My fix does not depend on it, because an out-of-combat swap is harmless.
- Also found: `tick` starts Stowing at `phaseStart=now`, not at `stowAt`. With a coarse tick cadence the stow starts late (the slide is cosmetic). With `stowDelay=0` this no longer occurs.

## 2. Decisions

D1. OPEN answer: on an out-of-combat swap inside the countdown, KEEP the earlier stowAt. Do not restart it.
- What the player sees: combat ended and the weapon is on its way back to the hip. A gear change should not make the weapon stay out 2.5 s longer. Restarting adds delay on top of combat_exit_ms, and every inventory click would postpone the holster.
- Out-of-combat swap while Holstered stays Holstered: nothing is drawn for no reason.
- Rule: `onSwap` is a no-op unless `lastCombat != 0`.
- The new weapon follows the existing timeline: Drawn on the old schedule, or already Holstered.
D2. In-combat swap: restart Drawing from 0 over `kSwapMs` (new object, see C2). Pin this with a test so nobody "fixes" it later.
D3. Continuity of the same object, in `onCombat` and `tick` only:
- Re-entry from Stowing: rebase `phaseStart` so the fraction is unchanged and only the remaining part slides.
- Re-entry from Drawn or Drawing: keep the phase. This changes the old assert at holster_logic_test.cpp:157 from Drawing to Drawn.
- Stow edge while Drawing: Stowing starts at the current fraction.
D4. Stow delay becomes a parameter, `stowDelay` (default `kStowDelayMs`, so old callers are unchanged). The drawgate passes 0: the predicate flip is already the end of the real exit countdown, and F8 hold expiry is also an instant stow.
D5. Feed predicate = `drawn = !shouldWriteHip(true, readable, combat, flourish || holdActive(...))`. Only for READABLE owners. When the owner is unreadable, call `forceDrawn()` and do not feed the predicate.
D6. Signed, wrap-safe `elapsed()` (`now < phaseStart` counts as 0). The cost is about 4 lines. In production a hoisted per-frame `now` makes this unreachable, so it is hardening. Keep it, because it is cheap and mutation-testable.
D7. `onSwap` and `forceDrawn` live in `HolsterLogic.h`. No new file.
D8. A new `Ent` in combat (load, cache eviction, new weapon) slides in from the mount over 350 ms. Accepted as cosmetic. If a seed is wanted later, `forceDrawn()` is already that primitive.

## 3. Implementation steps (ordered)

Size = lines changed. All edits are in `patch_manager_mods/holster_src/` and are host-test only.

1. `HolsterLogic.h` `Fsm::elapsed(now)`, new, 2 lines. Signed, clamps to 0.
2. `HolsterLogic.h` `Fsm::handFraction` and `tick`: replace `now - phaseStart` with `elapsed(now)`. About 4 lines.
3. `HolsterLogic.h` new `startDraw(now)` and `startStow(now)`, 2 lines each, with the rebase math.
4. `HolsterLogic.h` `onCombat(int, uint32_t, uint32_t stowDelay = kStowDelayMs)`, rewrite, about 10 lines.
5. `HolsterLogic.h` `onSwap`: the `lastCombat==0` guard, 1 line.
6. `HolsterLogic.h` new `forceDrawn()`, 1 line.
7. `HolsterLogic.h` `tick`: stow via `startStow(now)` (rebased, so a late tick no longer snaps).
8. `holster_logic_test.cpp`: change line 157 to `phase == PhDrawn`, then add the tests of section 4. Run `python3 scripts/run_tests.py holster_logic` and `holster_slide`. The slide tests must still pass: they only use fresh Fsms.
9. Topic 2 wiring contract, in Holster.cpp, NOT part of this change: add `hs::Fsm fsm; uintptr_t fsmOwner;` to `Ent` (Holster.cpp:1092).
   - Once per frame: `now = GetTickCount()`, hoisted.
   - If `!readable`: `ent->fsm.forceDrawn(); return;` (the hand pose stays).
   - Else: if `ent->fsmOwner && ent->fsmOwner != owner` then `ent->fsm = Fsm{}`. Then `ent->fsmOwner = owner`.
   - Then: `fsm.onCombat(!hip, now, 0); fsm.tick(now); f = fsm.handFraction(now);`.
   - If `f >= 1` return; else blend `applyPlacement(base)` with `in` at `easeSmooth(f)`.
   - Use `fsmOwner`, not `gate.owner`: `gateUpdate` overwrites the owner with 0 on a blip.

Signatures (final):
```cpp
uint32_t Fsm::elapsed(uint32_t now) const;
float    Fsm::handFraction(uint32_t now) const;
void     Fsm::onCombat(int combat, uint32_t now, uint32_t stowDelay = kStowDelayMs);
void     Fsm::onSwap(uint32_t now);      // no-op when lastCombat == 0
void     Fsm::forceDrawn();              // Drawn, lastCombat=1, no pending stow
void     Fsm::tick(uint32_t now);
void     Fsm::startDraw(uint32_t now);   // keeps fraction
void     Fsm::startStow(uint32_t now);   // keeps fraction
```
Core change (verified in scratch):
```cpp
uint32_t elapsed(uint32_t now) const { int32_t d = (int32_t)(now - phaseStart); return d < 0 ? 0u : (uint32_t)d; }
void startDraw(uint32_t now) { float f = handFraction(now); phase = PhDrawing; phaseLen = kDrawMs;
                               phaseStart = now - (uint32_t)(f * kDrawMs + 0.5f); }
void startStow(uint32_t now) { float f = handFraction(now); phase = PhStowing; phaseLen = kDrawMs;
                               phaseStart = now - (uint32_t)((1.f - f) * kDrawMs + 0.5f); }
void onCombat(int combat, uint32_t now, uint32_t stowDelay = kStowDelayMs) {
    if (combat != 0 && lastCombat == 0) {
        stowPending = false;
        if (phase == PhHolstered || phase == PhStowing) startDraw(now);   // Drawn/Drawing stay
    } else if (combat == 0 && lastCombat != 0) {
        if (stowDelay == 0) { stowPending = false; if (phase != PhHolstered) startStow(now); }
        else { stowPending = true; stowAt = now + stowDelay; }
    }
    lastCombat = combat;
}
void onSwap(uint32_t now) { if (lastCombat == 0) return; stowPending = false; phase = PhDrawing; phaseStart = now; phaseLen = kSwapMs; }
void forceDrawn() { phase = PhDrawn; stowPending = false; lastCombat = 1; }
```

## 4. Host-test additions (colocated in holster_logic_test.cpp; each loops over bases {0, 5000, 0xFFFFFC00, 0xFFFFFFF0})

Existing tests that must still hold: DrawIsInstant, StowDelayAndCancel (except line 157), SwapAndRollover. Names and vectors below are all implemented in `fixed_test.cpp`.

| Test | Vector |
|---|---|
| testSwapOutOfCombatKeepsStow | draw, off@+2000, swap@+2500: `stowPending`, stowAt unchanged, Drawn; at +3499 Drawn; at +3500 Stowing, fraction 1; +350 Holstered |
| testSwapWhileHolsteredNoDraw | Holstered, swap, tick +30000: Holstered, `!inHand`, fraction 0, `!stowPending` |
| testSwapInCombatRestartsFromMount | Drawn, swap: Drawing, len 1000, fraction 0, `!stowPending`; at +999 Drawing, at +1000 Drawn. Mid-draw swap at fraction 0.5 gives 0 (pins C2) |
| testReentryFromDrawnStaysDrawn | off, then on during the countdown: Drawn, fraction 1, `!stowPending`; stays Drawn at +60000 |
| testReentryMidStowContinuous | stow at 0.5 for delay 1500 and delay 0, then re-entry: fraction unchanged within 0.012, monotone up, Drawn after the remaining 175 ms |
| testReentryAtFractionsSweep | k=0..350 step 25 into a delay-0 stow, then re-entry: fraction before equals after |
| testStowDelayZero | off with delay 0 from Drawn: immediate Stowing, 1.0 / 0.5 / Holstered at +350. Never drawn: stays Holstered. Off while Drawing at 0.5: continues down, 0.25 at +87. Default delay stays pending 1500 |
| testStowAtWrapNotEarly | off@0xFFFFFC00, tick@0xFFFFFFF0 must stay Drawn; +0x1DB Drawn; +0x1DC Stowing |
| testStaleNow | `onCombat(1,b+1000)`, `handFraction(b+999)==0`, `tick(b+999)` stays Drawing; Stowing with `now < phaseStart` gives fraction 1 |
| testForceDrawn | fraction 1; later `onCombat(1)` does not re-draw; `onCombat(0,t,0)` stows from 1.0; clears a pending stow |
| testIndependence | two Fsms, only A is fed: B stays 0 |
| testGateFeed | real `shouldWriteHip` + `holdActive` as predicate: combat on/off, F8 hold keeps Drawn when combat clears, hold expiry stows at once (no 1500 stack), flourish draws, unreadable stays Drawn |
| testPropertyNoJump | 20 ms steps, predicate flips every 7 steps, delay 0: `abs(dFrac) <= 20/350 + 0.02`, always in [0,1] |

Mutation targets (run via `mut.sh`, killed unless noted):
- Swap guard removed (24 failures).
- `elapsed` unsigned (16).
- `startDraw` without rebase (72).
- `startStow` without rebase (8).
- delay-0 branch off (72).
- `forceDrawn` sets `lastCombat=0` (4).
- Drawn re-entry restarts the draw (4).
- `stowAt` compared unsigned (3, killed only after adding `testStowAtWrapNotEarly`).
- Rounding term removed (97).
- `forceDrawn` keeps the pending stow (8).
- Swap length wrong (12).
- `>` instead of `>=` at both slide ends (12 and 89).

Equivalent mutants (not killable, harmless):
- Swap no longer clears `stowPending`: always false in combat.
- Dropped `phase != PhHolstered` in the tick stow: the same tick finishes Stowing -> Holstered.
- Dropped the Holstered check in the delay-0 branch: unreachable, because `lastCombat != 0` with Holstered cannot occur.

## 5. Install type
Host-test only until wired. No DLL change, no hot-swap, no PLAY_TEST_QUEUE row for steps 1-8 (`Fsm` has no production caller). Step 9 (topic 2) is the first live behavior: a play row for draw, stow, and F8 hold mid-slide. It stays a queue item, never blocking.

## 6. Risks
- R1. `GetTickCount` resolution (about 15.6 ms on native Windows; unknown on Proton, UNVERIFIED) quantizes the 350 ms slide into 22 steps at worst. Consider `timeGetTime` or QPC at wiring time.
- R2. Gob address reuse: a freed Gob and a new Gob at the same address with the same owner inherit the old `Ent` Fsm. The effect is a new weapon that appears Drawn with no slide. It is benign, not a snap.
- R3. H7 may not run every frame, or Translate/Rotate may overwrite after H7 (T2a U1, U3). If so the slide stutters. Unverified, play row.
- R4. Pause: `GetTickCount` keeps running while the game is paused. A slide that straddles a pause completes instantly on resume. That is cosmetic. Whether H7 runs during pause is UNVERIFIED.
- R5. Changing line 157 of the existing test is a deliberate contract change (re-entry from Drawn no longer replays the draw). Record it in the commit message.
- R6. The `kSwapMs`=1000 and `kStowDelayMs` constants stay while production never calls them with default delay. Dead-ish but tested; do not delete in this change.
