# 42 manual holster hotkey vs Fsm (recon, read-only; PROPOSAL parts marked)

## 1. Fsm today (patch_manager_mods/holster_src/HolsterLogic.h)
- Phases :168 Holstered, Drawing, Drawn, Stowing. Consts :170-172: kDrawMs 350, kSwapMs 1000, kStowDelayMs 1500.
- inHand() :182 true in Drawing, Drawn, Stowing. handFraction :184-192 (0 at mount, 1 in hand).
- Transitions:
  - Holstered -> Drawing: onCombat(1) edge :195-197 (also clears stowPending :196).
  - Drawing -> Drawn: tick :207 once elapsed >= kDrawMs.
  - Drawn or Drawing -> Stowing: combat-end edge :198-199 sets stowAt = now + 1500; tick :208-210 fires it.
  - Stowing -> Holstered: tick :212 once elapsed >= kDrawMs.
  - Any -> Drawing: onSwap :204 (kSwapMs, clears stowPending). No combat check.
- Wiring: Holster.cpp:13 says the state machine is "not used by the probe yet". Only holster_logic_test.cpp:127-158 and holster_slide_test.cpp:92-140 drive it. No test calls onSwap.

Latent issues (from reading, not reproduced):
- F1: onSwap :204 out of combat leaves phase Drawn with stowPending cleared, and no combat edge ever stows it. The play checklist (42_holstered_weapons.md:75) asks for swaps out of combat.
- F2: onCombat :195-197 mid-stow restarts Drawing at handFraction 0 (:188), so the hand snaps from the current fraction to 0. Fix: read f = handFraction(now) first, then phaseStart = now - f * phaseLen.

## 2. PROPOSAL: manual toggle
- Input: one method, `bool onManualToggle(uint32_t now)`. Returns false and changes nothing when lastCombat != 0 (key ignored in combat).
- Out of combat it flips the visible state now with the kDrawMs slide, and skips kStowDelayMs:
  - If inHand(): stowPending=false; phase=PhStowing; phaseStart = now - (1-f)*kDrawMs, where f = handFraction(now).
  - Else: stowPending=false; phase=PhDrawing; phaseStart = now - f*kDrawMs.
- In combat: ignored. Combat already holds the weapon in hand (:195-197). A key stow mid-fight breaks "never delays an attack" (HolsterLogic.h:166-167).
- Combat end, Option A (recommended): no new flag. A manual draw is ordinary state, and combat end still auto-stows after kStowDelayMs.
  Option B: add `bool holdDrawn`, set by a toggle-draw, cleared by the combat edge; tick skips the auto stow while it is set.
- Per character: the key drives only the PC's Fsm (P1 = PC then party, master plan :8 and :29). Party stays auto. Fsm has no identity field, so the glue picks the creature. Holster.cpp has no PC or leader identification (G3 open).
- Input: poll a VK from holster.ini with previous-state edge detect, so a held key does not flip every frame. Precedent: CombatHotbar.cpp:238 and ActionHotbar.cpp:1974 (GetAsyncKeyState).

## 3. Host-test cases (holster_logic_test.cpp, KT_CHECK style)
1. Holstered, out of combat: toggle gives inHand and phase PhDrawing; handFraction 0 at t0, 1 at t0+kDrawMs.
2. Drawn, out of combat: toggle gives PhStowing at once (no kStowDelayMs wait); Holstered at +kDrawMs.
3. lastCombat=1: toggle returns false; phase, stowPending and stowAt are unchanged.
4. Combat-end countdown pending: toggle clears stowPending; no stow fires at stowAt.
5. Manual draw out of combat, then combat enter and exit: stow fires at exit + kStowDelayMs (Option A).
6. Toggle mid-stow at fraction 0.5: draw starts at 0.5, no snap to 0 (F2).
7. Toggle near the uint32 wrap (e.g. now=0xFFFFFF00), following the wrap pattern at holster_slide_test.cpp:124-140.
8. F1 pin: onSwap at lastCombat=0 must not leave Drawn with no pending stow. Expected to FAIL today.

## 4. Phase and dependencies
- Plan slot: not in HW7's listed scope (master plan :29, :48-49). Put it as a late HW7 sub-item, after the PC hip<->hand move passes play. Not HW8 or HW9.
- Why: a toggle without the move only changes the slide pose (HolsterSlide.h:38). The weapon stays where the engine put it.
- Dependencies:
  - H-ATTACH wrap of 0x7637E0 plus the 0x5047B0 detour (master plan :49). Blocked on G1 (world vs local node space) and G6 (bone lookup) from HW5/HW6.
  - PC identity (G3; no PC field in Holster.cpp).
  - F1 and F2 fixed first, since the toggle uses the same slide paths.
  - Live install needs user GO (master plan :14).
