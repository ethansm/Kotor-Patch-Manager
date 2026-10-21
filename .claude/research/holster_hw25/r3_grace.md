# HW25 item 3: hard-reset grace (recon r3, read-only)

## 1. first vs !readable and what each resets
- readable = owner!=0 && combatOff!=0 && rdU32Ok(owner+combat) (Holster.cpp:1765). Owner nulled or combat unreadable => false.
- hip = shouldWriteHip(true, readable, ...) (Holster.cpp:1766). Unreadable forces hip=0 (HolsterDrawGate.h:47).
- first = gate.hip<0 || gate.owner!=owner (Holster.cpp:1773), read BEFORE gateUpdate (L1774). True on first eval or owner change.
- Branch Holster.cpp:1781 `if (first || !readable)`: reset(!hip, now) (L1786), slFrames=0 (L1787).
  - first && readable: silent snap to hip (hip=1) or hand (drawn). Log reason=first (L1784).
  - !readable: hip=0, so snap to HAND (engine pose left in hand). Log reason=unreadable.
- Else (L1788-1815): Slide::target(!hip) (L1791) retargets from frac(now). No snap.
- Recovery after an unreadable frame:
  - Combat-only unreadable (owner kept): first=false, recovery = full stow from hand.
  - Owner went 0 then back: gateUpdate stored owner=0 (HolsterDrawGate.h:88), so recovery has first=true and hip-snaps. A grace on !readable alone does not stop this.

## 2. Per-entry field
- struct Ent, Holster.cpp:1662: slFrames/slMaxDt/slPrev (uint32), gate, slide.
- Add `uint8_t graceN = 0;` to Ent after slPrev.
- Clear: rebuild `*ent = Ent{}` (L1713) zeroes it. The helper clears it on readable, inactive, or expiry, so the reset at L1786 needs no extra clear.
- Not in Slide (HolsterSlide.h:40): keeps HolsterDrawGate.h free of the Slide type.
- Slide::reset sets len=0, so active() is false after a hard reset (HolsterSlide.h:47-53, 91-95).

## 3. Proposed helper (HolsterDrawGate.h, after gateUpdate at L91)
- Signature: `inline bool slideGraceHold(bool readable, bool slideActive, uint8_t& held, uint8_t maxFrames = 3)`
- Semantics:
  - readable: held=0, return false.
  - !slideActive: held=0, return false (at rest, no grace; existing behaviour).
  - held >= maxFrames: held=0, return false (expired; caller resets as before).
  - otherwise: ++held, return true (hold this frame).
- Call-site edits in Holster.cpp:
  1. Hoist `const uint32_t now = slideNowMs();` above L1773; delete the copy at L1780.
  2. Before L1774: `const bool grace = hs::slideGraceHold(readable, ent->slide.active(now), ent->graceN);`. Guard gateUpdate with `!grace &&`, so owner stays X and recovery sees first=false.
  3. At L1781: first branch `if (grace) ent->slide.touch(now);`, then `else if (first || !readable)` (existing reset), then the existing else.
- Held frames render slideF = frac(now) (L1816) on the running timeline, blended as today (L1848). No target(), no slFrames counting.
- Do not freeze at the last pose: a freeze stalls about 50 ms at 60 fps, then resumes (a hitch). Direction stays the last readable intent.

## 4. Unit tests (holster_drawgate_test.cpp)
- Add `static void testSlideGrace()` before main (L215) and call it from main. Style: KT_CHECK(expr) (tests/include/TestHarness.h:140), `using namespace hs;` (test L7), `return kt::Report("holster_drawgate")` (test L227; TestHarness.h:91).
- Inputs (readable, slideActive, held, maxFrames) -> (return, held after):
  - (T,T,2,3) -> false,0
  - (T,F,0,3) -> false,0; (F,F,0,3) -> false,0
  - (F,T,0,3) -> true,1; (F,T,1,3) -> true,2; (F,T,2,3) -> true,3; (F,T,3,3) -> false,0
  - (F,F,2,3) -> false,0 (slide ended mid-grace clears the count)
  - (F,T,0,0) -> false,0; (F,T,0,1) -> true,1; (F,T,1,1) -> false,0
  - Episode restart: (F,T,0) -> true,1; (T,T,1) -> false,0; (F,T,0) -> true,1
- The owner-0 gate skip has no host test (call-site only). Do not change gateUpdate: test L94 pins owner 0 as a change.

## Notes
- At rest, unreadable still snaps hip to hand (L1786). The spec says active-only, so this is kept.
- Reach path (g_prevSlide L1573-1582, ent->slide L1854) reads the same slide. Not traced in depth.
