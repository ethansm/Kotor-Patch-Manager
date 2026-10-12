# Code Review & Remediation Feedback: Mod 10 (Equipment 3D Character Preview)

> **Target**: [`patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp)  
> **Documentation**: [`patch_manager_mods/10_equipment_3d_character.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/10_equipment_3d_character.md)  
> **Review Date**: September 30, 2026  
> **Status**: Comprehensive Engineering Audit & Actionable Remediation Plan

---

## 1. Architectural Highlights & Strengths

The implementation is a sophisticated, high-impact engine modification that adds a real-time, rotatable 3D character preview to the Equipment panel:
- **Creature Lifecycle**: Successfully mirrors the engine's `SetStats` (`0x0084F2C6..0x0084F482`) creature synthesis path to create an isolated client-side preview creature.
- **Dynamic Resolution Sync**: Accurately tracks viewport and aspect ratio changes using global resolution pointers (`0x009F42A4`, `0x009F42A8`), recalculating projection and field-of-view matrices on the fly.
- **Clean Destruction Hook**: Properly intercepts `CSWGuiInGameEquipment::Destroy` (`0x008AA6F0`) to release all created 3D resources, viewports, and creature structs when the panel is closed.

---

## 2. Critical Bugs & Latent Vulnerabilities

### 🚨 Bug 1: Scene Model Accumulation / Gob Leak on Character Switching
* **Location**: [`EquipCharPreview.cpp#L504-L536`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp#L504-L536)
* **The Problem**:
  In `buildPreview(Slot* s, int charIdx, bool forceRebuild)`:
  ```cpp
  if (!s->creature) {
      allocCreature(s);
  }
  ```
  When the player toggles between party members (e.g., clicking the companion left/right arrows), `s->creature` already exists from the previous party member. The function proceeds to update creature appearance and calls `FnSetupScene` (`0x0077EC20`).
  `FnSetupScene` internally calls `CSWGuiScene::AddModel` (`0x00418360`) to attach body and head Gobs to the scene view. Because `CSWGuiScene::ClearModels` or `destroyCreature(s)` is not called before switching character models, models accumulate in the scene.
  **Impact**:
  - Visual ghosting (detached heads or armor meshes floating in the preview).
  - Unbounded Gob accumulation in the OpenGL scene view (`view + 0x60`), leading to memory leaks and eventual crashes after browsing party equipment.
* **Remediation**:
  Whenever `charIdx` changes, completely destroy the existing preview creature before allocating a new one:
  ```cpp
  if (s->creature && (s->charIdx != charIdx || forceRebuild)) {
      destroyCreature(s);
  }
  if (!s->creature) {
      if (!allocCreature(s)) return;
  }
  s->charIdx = charIdx;
  ```

---

### 🚨 Bug 2: Dangling Node & Unparented Gob Leak on Weapon Appearance Rebuild
* **Location**: [`EquipCharPreview.cpp#L545-L572`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp#L545-L572)
* **The Problem**:
  `buildPreview` calls `detachWeapons(s, "before rebuild")` to detach weapon models from creature hand nodes before invoking `FnBuildApp` (`0x0076A9A0`).
  If `FnBuildApp` encounters an invalid appearance row, custom modded model, or fails memory allocation, the detached weapon Gobs remain unattached to any scene node. Subsequent calls or teardown paths lose track of these pointers, leaking the weapon Gobs.
  Furthermore, `FnBuildApp` can reallocate the body model's node tree, rendering cached node indices in `s->weaponNodes` stale.
* **Remediation**:
  Wrap weapon detachment and reattachment with an explicit validation check. If `FnBuildApp` fails, immediately invoke `destroyWeapons(s)` to clean up unparented models.

---

### ⚠️ Bug 3: Hardcoded Supermodel Table Stalls Animations for Custom / Droid Appearances
* **Location**: [`EquipCharPreview.cpp#L285-L330`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp#L285-L330)
* **The Problem**:
  `animLen()` calculates flourish and idle animation lengths by matching against hardcoded tables: `kHumanoid`, `kDroidT3`, and `kDroidG0`.
  If a party member or custom companion uses an unmapped supermodel (e.g., custom droid chassis, HK-47 variants, Bao-Dur remote, or alien disguise appearances), `animLen()` returns `false`.
  When `animLen()` fails:
  - Animation tick code does not advance time.
  - The character freezes in a static pose or T-pose in the preview viewport.
* **Remediation**:
  Add a fallback duration (e.g., `2.0f` seconds) when an unrecognized supermodel is encountered, ensuring smooth animation cycling regardless of character model:
  ```cpp
  if (!found) {
      *outSec = 2.0f; // Default 2-second loop fallback
      return true;
  }
  ```

---

### ⚠️ Bug 4: Subclass Unhook Chain Safety on `DLL_PROCESS_DETACH`
* **Location**: [`EquipCharPreview.cpp#L880-L905`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp#L880-L905)
* **The Problem**:
  `EquipCharPreview` installs a window subclass via `SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, wheelProc)` to intercept mouse dragging for 3D character rotation.
  In `DllMain`, on `DLL_PROCESS_DETACH`, it restores `g_oldProc`:
  ```cpp
  if (reason == DLL_PROCESS_DETACH) {
      if (g_oldProc && g_hwnd) {
          SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_oldProc));
      }
  }
  ```
  If another DLL (or an overlay / Wine / Mod 1 `Inventory3DViewport`) subclassed the window after `EquipCharPreview`, restoring `g_oldProc` directly overwrites the head of the chain, silently unhooking subsequent hooks.
* **Remediation**:
  Only restore `g_oldProc` if the current `GWLP_WNDPROC` is still `wheelProc`:
  ```cpp
  if (reason == DLL_PROCESS_DETACH) {
      if (g_oldProc && g_hwnd && IsWindow(g_hwnd)) {
          LONG_PTR curProc = GetWindowLongPtrA(g_hwnd, GWLP_WNDPROC);
          if (curProc == reinterpret_cast<LONG_PTR>(wheelProc)) {
              SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_oldProc));
          }
      }
  }
  ```

---

## 3. Recommended Remediation Code Changes

### Patch A: Fix Character Switch Scene Leak (`EquipCharPreview.cpp:L504`)
```diff
--- a/equipment_3d_character_src/EquipCharPreview.cpp
+++ b/equipment_3d_character_src/EquipCharPreview.cpp
@@ -504,11 +504,14 @@ void buildPreview(Slot* s, int charIdx, bool forceRebuild) {
     if (!s) return;
+    if (s->creature && (s->charIdx != charIdx || forceRebuild)) {
+        destroyCreature(s);
+    }
     if (!s->creature) {
         if (!allocCreature(s)) {
             logf("buildPreview: allocCreature failed for char %d", charIdx);
             return;
         }
     }
+    s->charIdx = charIdx;
```

### Patch B: Add Animation Fallback Duration (`EquipCharPreview.cpp:L325`)
```diff
--- a/equipment_3d_character_src/EquipCharPreview.cpp
+++ b/equipment_3d_character_src/EquipCharPreview.cpp
@@ -325,4 +325,6 @@ bool animLen(int supermodel, int animId, float* outSec) {
         }
     }
-    return false;
+    *outSec = 2.0f; // Safe fallback for unmapped supermodels
+    return true;
 }
```

---

## 4. Verification & Testing Matrix

| Scenario | Target Verification | Expected Result |
| :--- | :--- | :--- |
| **Rapid Companion Arrow Cycling** | Cycle through all 10 companions rapidly in Equipment UI. | Scene models reset cleanly; no ghost heads or dangling armor. |
| **Droid / Alien Companion Preview** | Inspect T3-M4, HK-47, and G0-T0 in Equipment screen. | Model renders correctly without crashing; animation loops at fallback duration. |
| **Weapon Swapping & Unequipping** | Equip and unequip single-saber, dual pistols, and heavy rifles. | Weapon Gobs attach to correct hand nodes; no memory leak in Gob pool. |
| **Resolution / Aspect Ratio Change** | Change game resolution while Equipment panel is open. | Viewport recalculates projection correctly without black screen. |
