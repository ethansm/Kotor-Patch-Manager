# Code Review & Remediation Feedback: Mod 13 (Ranged AI Spacing & Kiting)

> **Target**: [`patch_manager_mods/ranged_ai_src/RangedAI.cpp`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/RangedAI.cpp)  
> **Hook Configuration**: [`patch_manager_mods/ranged_ai_src/kotor2-steam-aspyr.hooks.toml`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/kotor2-steam-aspyr.hooks.toml)  
> **Documentation**: [`patch_manager_mods/13_ranged_ai_spacing.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/13_ranged_ai_spacing.md)  
> **Community Research**: [`patch_manager_mods/13a_ai_community_research.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/13a_ai_community_research.md)  
> **Review Date**: September 30, 2026  
> **Status**: Comprehensive Engineering Audit & Stage 0-3 Implementation Roadmap

---

## 1. Architectural Highlights & Strengths

This mod resolves one of KotOR II's most frustrating combat behaviors (ranged companions charging into melee when line of sight is obstructed):
- **Root-Cause Accuracy**: Identifies the exact engine condition in `CSWSCreature::AIActionPhysicalAttacks` (`0x006D7390`) where blocked line of fire (`los == 0`) queues an approach move toward the target rather than holding or repositioning.
- **Surgical Detour Hooks**:
  - **E-H1 Hold (`0x006D83EA`)**: Employs `consumed_exit_address = 0x006D83FF` to end the attack action (`EndAttackAction(1)`) as complete without movement, whenever a ranged companion has line-of-fire blocked but is within maximum weapon range.
  - **E-H2 Retarget Gate (`0x006D7C73`)**: Employs `consumed_exit_address = 0x006D7C8C` to extend the engine's built-in player stuck retarget mechanism to companions in stance 16/11. When blocked at the same location across consecutive rounds (`pos == lastBlockedPos`), the companion searches for another visible target (`0x0057C990`) instead of idling indefinitely.
- **Observability & Throttling**: Implements per-creature logging (`Track g_tr[8]`) with 2-second rate-limiting, allowing telemetry collection without impacting combat framerates.

---

## 2. Technical Findings & Latent Issues

### ⚠️ Issue 1: Unchecked Zero Area Pointer in Hold Predicate
* **Location**: [`RangedAI.cpp#L139-L147`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/RangedAI.cpp#L139-L147)
* **The Problem**:
  In `RangedAiHold`:
  ```cpp
  int myArea = reinterpret_cast<AreaFn>(FnGetArea)(attacker), tgtArea = rd<int>(ebp, F_TargetArea);
  ...
  bool hold = ... && myArea == tgtArea ...;
  ```
  If an attacker is in the middle of an area transition or during module loading where `GetArea` returns `0`, and the target's area local is also `0`, `myArea == tgtArea` evaluates to `true`.
  When area is `0`, the engine skips line-of-sight checks and leaves `los = 0`.
* **Remediation**:
  Explicitly require non-null area pointers:
  ```cpp
  bool hold = ... && myArea != 0 && tgtArea != 0 && myArea == tgtArea ...;
  ```

---

### ⚠️ Issue 2: Guarding Minimum Range Threshold
* **Location**: [`RangedAI.cpp#L144-L145`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/RangedAI.cpp#L144-L145)
* **The Problem**:
  `lim` is calculated as `(maxR + 0.1f)^2`. If `maxR` is `0.0f` (uninitialized weapon range or edge-case weapon configuration), `lim = 0.01f`. If the distance is nearly 0, `inRange` can evaluate to true.
* **Remediation**:
  Ensure `maxR > 1.0f` as part of the `inRange` qualification:
  ```cpp
  bool inRange = maxR > 1.0f && dist2 <= lim;
  ```

---

## 3. Stage 3 Roadmap: Stance 16 & UI Hooks (S-H1 .. S-H4)

When advancing from Stage 0-2 (stance 11 testing) to Stage 3 (dedicated stance 16 "Ranged (Kiting)"), implement the four UI/stance hooks in `RangedAI.cpp` and `kotor2-steam-aspyr.hooks.toml`:

### A. S-H1 Stance Clamp Bypass (`0x0077AA5E`)
* Prevents the engine from resetting stance 16 (`0x10`) to 9 (Aggressive) in `AddStanceActions` (`0x0077A9E0`).
* **Hook**: Detour at `0x0077AA5E`, `consumed_exit_address = 0x0077AAB4`, `exclude_from_restore = ["eax"]`.
* **Handler**: If `state == 0x10`, return 1 (consumed, skips clamp); else return 0.

### B. S-H2 Stance Row Generator (`0x0077ACBC`)
* Adds the 5th stance row to the stance dropdown menu.
* **Hook**: Detour at `0x0077ACBC`, `consumed_exit_address = 0x0077AE77`, `exclude_from_restore = ["eax"]`.
* **Handler**: If `state == 0x10`: set `row + 8 = 0x415` (action ID), set `row + 0x20` to CResRef `"ib_ranged"`, return 1.

### C. S-H3 Stance Click Handler (`0x0077D8D4`)
* Intercepts clicks on the new stance row (`0x415`) in `StanceActionClicked` (`0x0077D890`).
* **Hook**: Detour at `0x0077D8D4`, `consumed_exit_address = 0x0077D9CA`, `exclude_from_restore = ["eax"]`.
* **Handler**: If `actionId == 0x415`: write `0x10` to `stats + 0x146`, set floaty feedback strref, return 1.

### D. S-H4 HUD Stance Indicator (`0x0074DBDD`)
* Maps stance 16 to HUD icon in `0x0074DA40`.
* **Hook**: Detour at `0x0074DBDD`, `skip_original_bytes = true`.
* **Handler**: `*(int*)(ebp - 0x1C) = (*(int*)(ebp - 0x28) == 0x10) ? 0x415 : -1`.

---

## 4. Verification & Testing Matrix

| Scenario | Target Verification | Expected Result |
| :--- | :--- | :--- |
| **Line-of-Sight Obstruction** | Place an ally or pillar directly between a ranged companion and their target. | Companion holds position; does not charge forward into melee. |
| **Second Round Retarget** | Keep target obstructed for 2+ combat rounds while companion holds. | Companion retargets nearest visible unblocked enemy or aborts attack cleanly. |
| **Melee Weapon Equipping** | Equip companion with melee weapon while stance 11/16 is selected. | Hold logic is automatically bypassed (`haveRanged == false`); companion approaches normally. |
| **Player Character Control** | Control the main character or companion directly. | Hold logic is bypassed (`isPC == true`); player retains full direct movement control. |
