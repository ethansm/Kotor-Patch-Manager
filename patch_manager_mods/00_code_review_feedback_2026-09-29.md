# Code Review Feedback & Technical Audit (Session: 2026-09-29)

> **Recovered Session ID**: `81e4d069-0b70-4393-9ddd-98bd49bc9068`  
> **Original Timestamp**: September 29, 2026 | 15:16:06 – 15:35:26 EDT  
> **Source Log**: [`transcript_full.jsonl`](file:///home/ethansm/.gemini/antigravity-cli/brain/81e4d069-0b70-4393-9ddd-98bd49bc9068/.system_generated/logs/transcript_full.jsonl)  
> **Scope**: Code reviews and architecture audits of Claude's custom mods (Mods 1, 2, and 3), GOG→Steam patch conversions, and Ghidra reverse-engineering tooling.

---

## 1. Architectural Matrix of Custom Mods

```
┌───────────────────────────────────────────────────────────────────────────────┐
│                           CUSTOM MOD ARCHITECTURE                             │
├────────────────────────┬─────────────────────────────┬────────────────────────┤
│ Mod 1: Viewport 3D     │ Mod 2: Buff Duration HUD    │ Mod 3: Force Hotbar    │
│ [UI Layout + Scene/Gob]│ [Per-Frame HUD Detour + Time]│ [Input + Spell Action] │
├────────────────────────┼─────────────────────────────┼────────────────────────┤
│ • GFF Struct Injection │ • Native Widget Repurposing │ • Keyboard Intercept   │
│ • CSWGuiScene::AddModel│ • Dimension-Shrink Wipe     │ • Action Queue Dispatch│
│ • baseitems/appearance │ • WorldTimer Singleton      │ • CreatureStats Spells │
│ • Dual-Branch Render   │ • Side-Table Pointer Key    │ • External Persistence │
└────────────────────────┴─────────────────────────────┴────────────────────────┘
```

| Mod | Primary Domain | Engine Footprint | Status at Review | Risk Profile |
| :--- | :--- | :--- | :--- | :--- |
| **Mod 1: [Inventory 3D Viewport](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/01_inventory_3d_viewport.md)** | UI / 3D Scene Rendering | `CSWGuiInGameInventory`, `CSWGuiScene`, `CONTROL_3DVIEW` | **PAUSED** (Render pipeline resolved) | Moderate (Layout bounds, missing model crash) |
| **Mod 2: [Visual Buff Duration HUD](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/02_buff_duration_hud.md)** | HUD Repurposing / Cooldowns | `CSWGuiMainInterface`, `CSWGuiLabel`, `CGameEffect` | **RESEARCH COMPLETE** (Pre-dev gate) | Low–Moderate (Numeric ID space check) |
| **Mod 3: [Force Power Hotbar](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/07_force_power_hotbar.md)** | Input Dispatch / Action Invocation | Keyboard handler, `CSWGuiMainInterfaceAction`, `CSWSCreatureStats` | **CONCEPTUAL** (Pre-Ghidra) | High (Input conflicts, dialogue clash) |

---

## 2. Mod 1: Inventory 3D Viewport

### Core Findings & Strengths
* **The Shared Render Call Breakthrough**: Both standalone items (Branch A: weapons/shields via direct model resref) and wearable armor (Branch B: via `GetArmorModelAndTextureResRef` at `0x008cd060` indexing `appearance.2da` columns `modelA..N`/`texA..N`) converge onto a single shared call to `CSWGuiScene::AddModel` (`0x00418360`) inside `FUN_008cd2c0`. This avoids instantiating dummy creature objects or touching the equipment screen.
* **GFF Re-use from `upgrade_p.gui`**: Extracting `CONTROL_3DVIEW` from the workbench screen rather than the character screen gives an item-centered camera and rotation setup out of the box, already proven to work with KotOR II's standalone item assets.

### Critical Feedback & Actionable Fixes
1. **Party Member Context Switch (Branch B)**:
   * `GetArmorModelAndTextureResRef` (`0x008cd060`) takes:
     ```cpp
     __cdecl(context, bodyvar, raceVariantIndex, outModelResRef, outTexResRef)
     ```
   * *The Pitfall*: If `raceVariantIndex` is hardcoded to the player character (PC), inspecting armor while a companion (Atton, Kreia) or droid (HK-47, T3-M4) is selected will render the wrong head/body or crash on invalid droid appearance lookups.
   * *The Fix*: Sample the **currently active inventory creature** (`CSWGuiInGameInventory::m_pCurrentCreature`), extracting that specific creature's appearance row. If the active creature is a droid, suppress Branch B armor rendering entirely.
2. **Missing 3D Model Crash Hazard (Consumables & Spikes)**:
   * Medpacs, computer spikes, repair kits, and datapads have no 3D model resref in `baseitems.2da` and `bodyvar == 0`.
   * *The Fix*: Add an explicit defensive guard before calling `AddModel`:
     ```cpp
     if (modelResRef[0] == '\0' || strcmp(modelResRef, "NULL") == 0) {
         // Clear or hide the 3D scene Gob instead of calling AddModel
         return;
     }
     ```
3. **Injected Control Discovery**:
   * Do not hardcode a heap pointer. In the inventory panel initialization hook, locate the injected control by its Control ID via `panel->GetControl(INVENTORY_3D_VIEW_ID)` and cache the underlying `CSWGuiScene*` (`this + offsetScene`).

---

## 3. Mod 2: Visual Buff Duration HUD

### Core Findings & Strengths
* **Native Widget Repurposing**: Reusing native `LBL_CMBTEFCTINC%d`/`RED%d` widgets eliminates the need for custom OpenGL/D3D quad rendering or shader passes.
* **Zero-Extra-Draw Wipe**: Repurposing `CSWGuiBorder::Draw` (`0x00415300`) which re-reads dimensions (`self+0xc`/`self+0x10`) live each frame. Shrinking the inner fill border (`self+0xd8`, `"TEXT"`) proportional to duration while leaving the outer border (`self+0x60`, `"BORDER"`) intact creates a clean wipe with zero additional draw calls.
* **Pointer Address Keying**: Side-stepping slot remapping by keying side tables directly by raw `slotPtr` from `UpdatePortraits` (`0x0074ee90`) to `PortraitIconRowDraw` (`0x007457e0`).

### Critical Bugs & Edge Cases

#### 🚨 Critical Bug: The Shared-Widget Dimension Mutation Bug
* **The Flaw**: `PortraitSlot_BuffIconWidget` (`slotPtr + 0x968`) is a **SINGLE SHARED WIDGET INSTANCE** repositioned and drawn $N$ times per frame!
  * If Icon 0 has 40% duration remaining, shrinking `self+0xd8+0x10` from `32` to `12` leaves the widget height at `12` for Icon 1, Icon 2, and all subsequent frames even after buffs expire!
* **The Fix**:
  1. Save original dimensions (`origW`, `origH`) before modifying.
  2. Set `skip_original_bytes = true`.
  3. Overwrite `self+0xd8+0xc`/`0x10` with shrunken dimensions.
  4. Manually invoke original instructions (`CALL vtbl[14]` `Draw(dt)`).
  5. **Immediately restore** `self+0xd8+0xc = origW` and `self+0xd8+0x10 = origH` right after the call returns.

#### Issue 2: Null-Pointer Defensive Guarding on WorldTimer Singleton
* Tracing `worldTimerObj` via `*( *(int*)0x00a1b4a4 + 0x8 ) + 0x4` can encounter null pointers during module loads, fast travel, or cutscenes, causing a Crash to Desktop (CTD).
* *The Fix*: Add null checks at every indirection level before dereferencing.

#### Issue 3: Day-Boundary Wraparound Countdown Arithmetic
* Time is split into integer `Day` and integer millisecond `Time` ($0 \le t < 86,400,000$). A buff spanning midnight has `ExpiryTime < CurrentTime` but `ExpiryDay > CurrentDay`.
* *The Fix*: Compute 64-bit absolute milliseconds:
  ```cpp
  int64_t currentTotalMs = (int64_t)curDay * 86400000LL + curTimeMs;
  int64_t expiryTotalMs  = (int64_t)expDay * 86400000LL + expTimeMs;
  int64_t remainingMs    = expiryTotalMs - currentTotalMs;
  ```

#### Issue 4: Closing the Item 3 Gate Empirically
* Instead of decompiling complex NWScript virtual machine layers, close Item 3 (verifying whether `CGameEffect+0x1c` maps to `spells.2da` rows) with a 5-minute diagnostic hook:
  ```cpp
  printf("[BuffHUD] ApplyEffect: effectPtr=%p, +0x1c=%d\n", effect, *(int*)((char*)effect + 0x1c));
  ```
  Cast *Burst of Speed* (Row 8). If `+0x1c == 8`, the gate is closed.

---

## 4. Mod 3: Force Power Hotbar (Lessons Applied)

| Lesson from Mods 1 & 2 | Application to Mod 3 (Hotbar) |
| :--- | :--- |
| **A. Native Widget Reuse** | Do not inject raw D3D quads. Reuse/clone `CSWGuiMainInterfaceAction` (7 native quickbar slots with icon, hover, click, disabled states) or `CSWGuiButton`. |
| **B. Exploit Pre-Existing Symbols** | `kotor2_steam_aspyr.db` contains exact addresses: `CSWSCreatureStats::HasSpell` (`0x006BD9C0`), `AddKnownSpell` (`0x006B86A0`), and `Global::CSWGuiInterfaceAction_UseForcePower` (GOG `0x004254A0`). |
| **C. Decode Live 2DAs with `pykotor`** | Read `override/spells.2da` (binary V2.b) via `pykotor` to associate slot IDs with localized names, icon resrefs, and Force costs. |
| **D. Detour Splice Timing & Key Swallowing** | Use `skip_original_bytes = true` on keypress to consume keys 1–0 when casting. Bypass hook when in dialogue or console typing. |
| **E. External Persistence** | Store hotbar bindings in external `kotor_hotbar.json` keyed by character tag/name. Never inject into `.sav` files. |

### Critical Warnings for Mod 3
* **Keyboard Conflict Hazard**: Keys 1, 2, and 3 are bound to party selection and dialogue responses. Keyboard interception must be strictly context-gated: active *only* when the main HUD is visible, conversation is inactive, and no modal menus have focus.
* **Castability State Machine**: Distinguish between unassigned, known/ready, known/disabled (armor restriction or low Force points), and active/cooling down.

---

## 5. Ghidra Tooling Audit (LaneDibello/Kotor-Patch-Manager PR Readiness)

### Key Metrics
* Upstream baseline: 2 files. Current worktree: **71 Java scripts**.
* **Problem**: 71 out of 71 scripts hardcode memory addresses and take 0 arguments. Submitting all 71 would be rejected as scratchpad clutter.
* **Consolidation**: Consolidate into **6 canonical tools**: `ExportFunctionsCsv`, `DisassembleFunction`, `DecompileTargets`, `SearchBytePattern`, `DumpVtable`, `FindXrefsAndDecompile`.

### Critical Tooling Bugs Identified

1. **Stack Size Calculation Flaw in [`ExportFunctionsCsv.java`](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts/ExportFunctionsCsv.java#L59-L72)**:
   * Assumed i386 System V ABI instead of MSVC conventions (`__thiscall`, `__fastcall`).
   * Counted `ECX` (`this`) and `EDX` register parameters as 4-byte stack variables, inflating stack deltas (e.g. 12 bytes instead of 8), causing stack pointer misalignment and corrupted `RET` trampolines.
   * *Fix*: Guard with `p.isStackVariable()`.
2. **Headless Mode Crash in [`ExportFunctionsCsv.java`](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts/ExportFunctionsCsv.java#L26)**:
   * Called `askFile()` unconditionally, crashing in headless mode.
   * *Fix*: Check `getScriptArgs()` first.
3. **Address Space Boundary Overflow in `find()` Loops**:
   * Calling `addr = hit.add(1)` throws `AddressOutOfBoundsException` near memory boundaries.
   * *Fix*: Wrap in `try/catch (AddressOutOfBoundsException)`.
4. **Hardcoded Memory Block Boundaries**:
   * Hardcoded `0x00401000` and `0x009f4000` break across different executable variants (GOG, Steam, Linux).
   * *Fix*: Query `currentProgram.getMemory().getBlock(".text")` and `.getBlock(".data")` dynamically.
5. **Symbol Table Optimization**:
   * Replaced expensive $O(N)$ full symbol table iteration (`getAllSymbols(true)`) with $O(1)$ indexed lookup: `st.getSymbols("vftable")`.
