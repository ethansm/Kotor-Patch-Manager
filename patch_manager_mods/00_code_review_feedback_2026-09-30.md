# Master Code Review & Architectural Audit: Custom Engine Mods (2026-09-30)

> **Session Context**: Complete technical code review and audit covering all in-development KotOR II engine mods (Mods 1, 2, 7/3, 10, 11, 13, and related diagnostic/research tooling) evaluated against engine contracts, decompiled assembly, and live Steam Aspyr binaries.  
> **Target Audience**: Owning agent sessions (Claude Code / subagents) dispatched to implement fixes.

---

## 1. Executive Status & Audit Matrix

```
┌───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                           COMPREHENSIVE MOD AUDIT MATRIX                                              │
├──────────────────────────┬──────────────────────────┬─────────────────────────────┬───────────────────────────────────┤
│ Mod                      │ Implementation Status    │ Critical Bugs / Hazards     │ Readiness Verdict                 │
├──────────────────────────┼──────────────────────────┼─────────────────────────────┼───────────────────────────────────┤
│ Mod 1: Inv 3D Viewport   │ C++ v2 complete          │ 2 Critical (Live mutation,  │ Fix & harden before packaging     │
│                          │                          │             detach unhook)  │                                   │
│ Mod 2: Buff Duration HUD │ C++ v2 + Shields added   │ 2 High (Debuff icon hide,   │ Fix & harden before packaging     │
│                          │                          │         GL Cap leak)        │                                   │
│ Mod 3: Force Hotbar      │ C++ v2 + In-game editor  │ 1 High (Transient obj ID,   │ Fix & harden before packaging     │
│                          │                          │         stuck modifier)     │                                   │
│ Mod 10: Equip 3D Preview │ C++ complete (920 lines) │ 2 Critical (Scene leak,     │ Fix & harden before packaging     │
│                          │                          │             weapon Gob)     │                                   │
│ Mod 11: Inv Sort/Filter  │ Stage 0 Installed        │ 1 High (Stale upgrade flag  │ Stage 0 verified; apply 2 fixes   │
│                          │                          │         on panel reopen)    │ for Stage 1-4 rollout             │
│ Mod 13: Ranged AI        │ Stage 0 Built            │ 1 Med (Null area pointer in │ Stage 0 ready for telemetry run;  │
│                          │                          │        hold condition)      │ Stage 3 UI hooks roadmap defined  │
└──────────────────────────┴──────────────────────────┴─────────────────────────────┴───────────────────────────────────┘
```

---

## 2. Review Summaries & Dedicated Feedback Documents

### A. Mod 1: [Inventory 3D Viewport](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/01_inventory_3d_viewport.md)
* **Feedback Document**: [`patch_manager_mods/01_inventory_3d_viewport_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/01_inventory_3d_viewport_feedback.md)
* **Core Findings**:
  1. *Live CItem Mutation*: `subst()` overwrites variation bytes in live game objects without an RAII guard; an unhandled exception or abrupt click permanently alters inventory item data into save files.
  2. *Missing Unsubclassing*: `g_oldProc` is never restored in `DLL_PROCESS_DETACH`, causing immediate CTDs on dynamic reload.
  3. *Hardcoded Default Resolution*: Defaults to 3840x1600 instead of querying globals `0x009F42A4`/`0x009F42A8`.

### B. Mod 2: [Visual Buff Duration HUD](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/02_buff_duration_hud.md)
* **Feedback Document**: [`patch_manager_mods/02_buff_duration_hud_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/02_buff_duration_hud_feedback.md)
* **Core Findings**:
  1. *Debuff Row Untracked Arrow Suppression*: `RetargetBuffIcon` hides native down-arrows when active debuffs exceed tracked duration icons instead of calling `RestoreBuffIcon()`.
  2. *Missing CapGuard in DrawBuffColumn*: Leaves OpenGL states (`GL_BLEND`, `GL_TEXTURE_2D`) unmanaged when rendering multi-column buff grids.
  3. *Shield Tooltip Accumulation*: `AppendShieldToTooltip` can repeatedly append shield absorption text if multiple hover events fire in the same frame.

### C. Mod 3 / Mod 7: [Force Power Hotbar](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/07_force_power_hotbar.md)
* **Feedback Document**: [`patch_manager_mods/07_force_power_hotbar_evidence/FEEDBACK_2026-09-30.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/07_force_power_hotbar_evidence/FEEDBACK_2026-09-30.md)
* **Core Findings**:
  1. *Transient Object ID Profiles*: Uses transient heap pointers/object IDs for companion config sections (`[Profile.obj%08x]`) rather than stable roster indices (`CSWPartyTable`), losing custom hotbar mappings on module transitions.
  2. *Stuck Modifier Keys*: DirectInput hook can leave Ctrl/Alt stuck if focus changes while held.
  3. *Party Switch on Ring Full*: Fails to swallow digit keypresses when the internal input ring is saturated.

### D. Mod 10: [Equipment 3D Character Preview](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/10_equipment_3d_character.md)
* **Feedback Document**: [`patch_manager_mods/10_equipment_3d_character_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/10_equipment_3d_character_feedback.md)
* **Core Findings**:
  1. *Scene Model Accumulation*: Switching companions reuses the preview creature holder and calls `FnSetupScene` without first calling `CSWGuiScene::ClearModels` or `destroyCreature(s)`, accumulating body/head Gobs in the OpenGL scene view and creating ghosting artifacts and memory leaks.
  2. *Unparented Weapon Gob Leaks*: `detachWeapons` detaches weapon Gobs prior to `FnBuildApp`; if appearance construction fails or node indices mutate, orphaned Gobs leak from the node hierarchy.
  3. *Supermodel Table Gaps*: Hardcoded supermodel lists cause unrecognized droid/alien companion models to return `false` in `animLen()`, stalling idle/flourish rotation at 0 FPS.
  4. *Subclass Chain Break*: `DLL_PROCESS_DETACH` unconditionally restores `g_oldProc` without checking if another mod hooked the window afterwards.

### E. Mod 11: [Inventory Sort & Sub-Filters](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/11_inventory_sort_filter.md)
* **Feedback Document**: [`patch_manager_mods/11_inventory_sort_filter_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/11_inventory_sort_filter_feedback.md)
* **Core Findings**:
  1. *Stale Upgrade State on Panel Reopen*: `g_upgrade` persists if the panel reopens at the same address because the engine reinitializes mode to `0` (and `normaliseState` only clears `g_upgrade` when `mode != 0`).
  2. *Dropped Selection ID*: When an active item is dropped by the upgrade filter, `panel + PanelSelectedIdOff` is not reset to `NoSelection`, leaving a temporary mismatch with fallback row 0.
  3. *Empty Slot Sentinel*: Default fallback in `rd<int>` for upgrade slots should be `-1` (uninstalled) rather than `0`.

### F. Mod 13: [Ranged AI Spacing & Kiting](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/13_ranged_ai_spacing.md)
* **Feedback Document**: [`patch_manager_mods/13_ranged_ai_spacing_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/13_ranged_ai_spacing_feedback.md)
* **Core Findings**:
  1. *Null Area Pointer Guard*: `myArea == tgtArea` can evaluate to true during area transitions when both are `0`, triggering an unintended hold condition because the engine sets `los = 0` when area is null.
  2. *Minimum Weapon Range Check*: Add `maxR > 1.0f` to guard against uninitialized weapon ranges.
  3. *Stage 3 Roadmap Defined*: Detailed specifications provided for S-H1 (clamp bypass), S-H2 (row generator), S-H3 (click handler), and S-H4 (HUD indicator).

---

## 3. Status on Related Research & Tooling

| Project / Tool | Relevant Files | Current Status & Notes |
| :--- | :--- | :--- |
| **Shield Bar (Mod 8)** | [`08_shield_bar.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/08_shield_bar.md), `shield_bar_src/` | Pure research logs and VFX texture dumps. Shield absorption bar functionality was integrated directly into Mod 2 (`BuffDurationHUD.cpp`) rather than shipping as a separate binary. |
| **Load Hang Probe (Mod 9)** | [`09_load_hang_investigation.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/09_load_hang_investigation.md), `load_hang_probe_src/LoadHangProbe.cpp` | Diagnostic v1.3.0 actively installed. Features watchdog thread, stack trace sampling, IAT hooks for file I/O, and optional window activation recovery (`fix=auto`). |
| **Active Effects Panel (Mod 14)** | [`14_character_active_effects_panel.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/14_character_active_effects_panel.md), `active_effects_panel_src/research/` | Active disassembly research of `0x007FDFA0` and message effects populator (`0x00803CD0`). Implementation planned following Mod 6/8 rollouts. |

---

## 4. Master Action Plan for Owning Agent Sessions

All owning agent sessions should follow this priority execution order:

1. **Session 1 (Mod 1 & Mod 10 - 3D Rendering & Viewports)**:
   - Read [`01_inventory_3d_viewport_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/01_inventory_3d_viewport_feedback.md) and [`10_equipment_3d_character_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/10_equipment_3d_character_feedback.md).
   - Implement RAII guard for `CItem` variation mutations in Mod 1.
   - Fix preview creature destruction and scene model clearing in Mod 10 `buildPreview`.
   - Add safe subclass chain verification on `DLL_PROCESS_DETACH` to both mods.

2. **Session 2 (Mod 2 & Mod 3 - HUD & Input Mechanics)**:
   - Read [`02_buff_duration_hud_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/02_buff_duration_hud_feedback.md) and [`07_force_power_hotbar_evidence/FEEDBACK_2026-09-30.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/07_force_power_hotbar_evidence/FEEDBACK_2026-09-30.md).
   - Fix debuff arrow restoration in Mod 2 `RetargetBuffIcon` and wrap `DrawBuffColumn` with `CapGuard`.
   - Migrate Mod 3 companion hotbar profiles from transient object IDs to roster indices.

3. **Session 3 (Mod 11 & Mod 13 - Inventory & Combat AI)**:
   - Read [`11_inventory_sort_filter_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/11_inventory_sort_filter_feedback.md) and [`13_ranged_ai_spacing_feedback.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/13_ranged_ai_spacing_feedback.md).
   - Hook `OnPanelRemoved` (`0x008A7520`) in Mod 11 to reset `g_upgrade` on inventory closure.
   - Add non-null area guards and weapon range thresholds in Mod 13 `RangedAiHold`.
   - Prepare Mod 13 Stage 3 hooks (S-H1 .. S-H4) once Stage 0-2 combat telemetry completes.
