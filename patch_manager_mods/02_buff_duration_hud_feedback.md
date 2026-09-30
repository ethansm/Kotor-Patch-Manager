# Code Review & Remediation Feedback: Mod 2 (Buff Duration HUD)

> **Target**: [`patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp)  
> **Documentation**: [`patch_manager_mods/02_buff_duration_hud.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/02_buff_duration_hud.md)  
> **Review Date**: September 30, 2026  
> **Status**: Comprehensive Post-Update Audit

---

## 1. Initial Feedback Reconciliation

| Initial Finding (2026-09-29) | Implementation Status | Notes / Verdict |
| :--- | :--- | :--- |
| **Shared-Widget Dimension Mutation Bug** | 🟡 **PARTIALLY FIXED** | Added `g_saved` to save original dimensions and `RestoreBuffIcon()` to restore them. However, a major edge case remains in the debuff row (see Bug 1 below). |
| **WorldTimer Singleton CTD Hazard** | ✅ **RESOLVED** | Implemented multi-stage null checks via `readAt` across `WorldTimerSingletonGlobal -> +8 -> +4 -> +0x10048`. Safe against load-screen CTDs. |
| **Day Wraparound Arithmetic** | ✅ **RESOLVED** | Fixed duration delta math using 64-bit absolute milliseconds: `(eDay - day) * MsPerDay + (eMs - ms)`. |
| **Item 3 Gate (`spells.2da` ID check)** | ✅ **RESOLVED** | Empirically verified via live diagnostic logging in `shieldprobe_log.txt`. |

---

## 2. Critical Bugs & Latent Vulnerabilities in Updated Code

### 🚨 Bug 1: Untracked Debuff Arrows Hidden Permanently (Missing Restore)
* **Location**: [`BuffDurationHUD.cpp#L537-L560`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp#L537-L560)
* **The Problem**:
  In `RetargetBuffIcon`, if an arrow is NOT suppressed (e.g. `index >= n`, meaning an engine-native debuff or combat status that we do not track), line 550 returns immediately:
  ```cpp
  if (index >= n) return; // untracked engine buff: leave its arrow
  ```
  However, in the **debuff row**, there is no post-draw restore hook (Hook 3 only runs on the buff row).
  If Arrow 0 was suppressed (width/height set to 0), the shared `PortraitSlot_BuffIconWidget` still has width/height equal to 0 when Arrow 1 begins drawing.
  When Arrow 1 enters `RetargetBuffIcon` with `index >= n`, line 550 returns without touching width/height.
  **Result**: Arrow 1 draws with width=0 and height=0, making native untracked debuff arrows completely invisible!
* **Remediation**:
  Ensure dimensions are restored whenever returning early or keeping an arrow:
  ```cpp
  if (index >= n) {
      if (g_saved.active && g_saved.border == border) {
          RestoreBuffIcon();
      }
      return;
  }
  ```

---

### 🚨 Bug 2: Missing `CapGuard` in `DrawBuffColumn` (OpenGL State Leak)
* **Location**: [`BuffDurationHUD.cpp#L570-L667`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp#L570-L667)
* **The Problem**:
  `CapGuard` was authored specifically because GUI batch flushes and color-fill clipping toggle OpenGL state (e.g., leaving `GL_LIGHTING` enabled).
  `CapGuard` was placed in `DrawShieldBar` at line 720:
  ```cpp
  void __cdecl DrawShieldBar(...) {
      ...
      CapGuard capGuard;
      ...
  }
  ```
  However, `DrawBuffColumn` (Hook 5), which calls `flush()`, `pushClip()`, and multiple quad draws every single frame, **does not instantiate `CapGuard`**!
  This causes leaked OpenGL states from the buff column to corrupt subsequent HUD rendering, which was the exact root cause documented in probe log 02:34 where small cards failed to render their shield bars.
* **Remediation**:
  Instantiate `CapGuard capGuard;` at the beginning of `DrawBuffColumn`.

---

### ⚠️ Issue 3: Portrait Tooltip String Accumulation
* **Location**: [`BuffDurationHUD.cpp#L750-L760`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp#L750-L760)
* **The Problem**:
  `AppendShieldToTooltip` appends shield information to `charButton + 0x1d4`:
  ```cpp
  reinterpret_cast<CExoSprintf>(CExoStringSprintf)(
      reinterpret_cast<void*>(charButton + 0x1d4), 
      "%s\n%s", 
      reinterpret_cast<const char*>(oldText), 
      line
  );
  ```
  If the engine does not recreate the underlying text buffer between consecutive hover frames (or when hovering across multiple party cards), passing `oldText` repeatedly appends `\nShield: ...\nShield: ...`.
* **Remediation**:
  Guard against re-appending if `oldText` already contains `"Shield:"`:
  ```cpp
  if (strstr(reinterpret_cast<const char*>(oldText), "Shield:") != nullptr) return;
  ```

---

## Status of the Agy items (2026-09-30, Claude review + fixes; uncommitted, compile-clean)
* **Bug 1 (debuff arrow left at 0x0) - CONFIRMED, fixed differently.** Agy's snippet was ineffective: `g_saved.active=false` on entry made its restore condition dead. Fixed by calling `RestoreBuffIcon()` at the top of `RetargetBuffIcon` (the shared widget is put back before each arrow) and at the top of `DrawBuffColumn`. This is also what makes the counter bump (doc 02) safe.
* **Bug 2 (`CapGuard` missing in `DrawBuffColumn`) - CONFIRMED, fixed** (`CapGuard capGuard;` after the `drawN <= 0` early return).
* **Issue 3 (tooltip re-append) - UNPROVEN, kept as a defensive guard** (`strstr(oldText, "Shield:")` returns early); the logs show the builder re-formats the text before our RET path, so the accumulation was never observed.
* **Reviewed in full 2026-09-30 (second pass):** nothing beyond the three items. Section-1 reconciliation rows (WorldTimer null checks, day-wraparound math) verified in source. Extra defect in Agy's Bug-1 snippet: it uses `border` before it is declared (HEAD line 552), so it would not have compiled, on top of the dead `g_saved.active` condition (HEAD line 538 sets it false on entry).
