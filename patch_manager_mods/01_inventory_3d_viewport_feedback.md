# Code Review & Remediation Feedback: Mod 1 (Inventory 3D Viewport)

> **Target**: [`patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp)  
> **Documentation**: [`patch_manager_mods/01_inventory_3d_viewport.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/01_inventory_3d_viewport.md)  
> **Review Date**: September 30, 2026  
> **Status**: Comprehensive Post-Update Audit

---

## 1. Initial Feedback Reconciliation

| Initial Finding (2026-09-29) | Implementation Status in v2 | Notes / Verdict |
| :--- | :--- | :--- |
| **Missing Model Crash Hazard** (Consumables, crystals, spikes) | ✅ **RESOLVED** | Implemented [`ItemModelTable.inc`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_3d_viewport_src/ItemModelTable.inc) and category filtering. Category 0 items cleanly bypass `AddModel` and call `RemoveModel`, preventing invalid Gob asserts. |
| **Dynamic Control Discovery** | ✅ **RESOLVED** | Bound via `FnInitControl` (`0x0040f620`) and matched against `ExpectedControlId` (21). |
| **Active Companion Appearance** | ⚠️ **POSTPONED TO V2** | Reuses Workbench row 137 (`male-medium` body) as an interim baseline. Stable against droid crashes, but armor inspect does not reflect companion appearance. |

---

## 2. Critical Bugs & Latent Vulnerabilities in Updated Code

### 🚨 Bug 1: Live In-Place Mutation of Real `CItem` Memory
* **Location**: [`Inventory3DViewport.cpp#L680-L706`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp#L680-L706)
* **The Problem**:
  To render items with missing variations, `subst()` overwrites the variation byte directly in the live game object's memory:
  ```cpp
  *reinterpret_cast<unsigned char*>(item + ItemVarOff) = 1;
  ...
  doRender();
  ...
  *reinterpret_cast<int*>(item + ItemVarOff) = savedVarWord;
  ```
  If `doRender()` triggers an unhandled engine assert, a DirectX surface loss, or if the user clicks another inventory slot while rendering, the restore step is skipped. The item permanently mutates to variation 1, which will be saved into the player's `.sav` file.
* **Remediation**:
  Wrap the call with structured exception handling / RAII guard to guarantee restoration even on abort, or pass the variation override through an isolated render parameter rather than mutating the live `CItem` struct.

---

### 🚨 Bug 2: Missing Unsubclassing on `DLL_PROCESS_DETACH` (CTD Hazard)
* **Location**: [`Inventory3DViewport.cpp#L481-L496`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp#L481-L496) & [`#L754-L757`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp#L754-L757)
* **The Problem**:
  `gameWindow()` subclasses the window:
  ```cpp
  g_oldProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wheelProc)));
  ```
  However, in `DllMain`, there is zero teardown on `DLL_PROCESS_DETACH`:
  ```cpp
  BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
      if (reason == DLL_PROCESS_ATTACH) { logf("inv3d v2 loaded"); loadIni(); }
      return TRUE;
  }
  ```
  If the DLL is unloaded or reloaded dynamically by KPatchLauncher, the window procedure still points to `wheelProc` in unmapped memory. The next window message results in an immediate Crash to Desktop.
* **Remediation**:
  Restore `g_oldProc` on detach:
  ```cpp
  if (reason == DLL_PROCESS_DETACH) {
      if (g_hwnd && g_oldProc && IsWindow(g_hwnd)) {
          SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_oldProc));
      }
  }
  ```

---

### ⚠️ Issue 3: Hardcoded 3840x1600 Default Breaks Hit-Testing on Other Resolutions
* **Location**: [`Inventory3DViewport.cpp#L133`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp#L133) & [`#L497-L503`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp#L497-L503)
* **The Problem**:
  Default configuration hardcodes `screenW = 3840, screenH = 1600`. In `cursorEngine`:
  ```cpp
  *x = pt.x * g_cfg.screenW / r.right;
  *y = pt.y * g_cfg.screenH / r.bottom;
  ```
  If a player runs at 1920x1080 or 2560x1440 and does not provide an `inv3d.ini`, `r.right` is 1920, causing `pt.x` to be scaled by `3840 / 1920 = 2.0x`. All mouse drag and wheel hit-test checks (`inside`) will be miscalculated and fail.
* **Remediation**:
  Query the actual engine framebuffer resolution dynamically from Aspyr globals `0x009F42A4` (width) and `0x009F42A8` (height), or default `screenW`/`screenH` to `r.right`/`r.bottom` when unconfigured.

---

### ⚠️ Issue 4: Panel Block Heap Leak on Rebind
* **Location**: [`Inventory3DViewport.cpp#L591-L615`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp#L591-L615)
* **The Problem**:
  Every time `BindInventoryView` runs on panel initialization, `operator new(0x400)` allocates a new block. When `claimSlot(panel)` reuses an existing slot entry for that panel, the previous `s->block` pointer is overwritten without being freed.
* **Remediation**:
  Free the previous block before claiming, or maintain a persistent static buffer per panel instance.
