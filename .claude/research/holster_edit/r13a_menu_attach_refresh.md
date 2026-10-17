# R13a: Menu Attach Refresh Investigation (vtbl+0x08 / 0x504EF5 in Menu Scene)

**Contract:** recon  
**Scope:** Swkotor2 Steam 6a522e71 / GOG 7751f7d8; `patch_manager_mods/holster_src/Holster.cpp`, `patch_manager_mods/preview_3d_src/Preview3dProvider.cpp`, `patch_manager_mods/holster_editor_src/HolsterEditor.cpp`.  
**Ledger / Evidence Sources:** Steam decomp `00504d40.asm`, `004e1d40.asm`, `0046d230.c`, `00465bb0.asm`, `0040ec90.c`, `0084de40.c`, `004184f0.c`, `00461aa0.c`.

---

## 1. Executive Verdict: Does the Engine Call vtbl+0x08 Every Frame in a Menu Scene?

**YES.** The engine DOES call `CAurBehaviorAttach::Control` (vtable slot `+0x08`, address `0x00504D40` in Steam, `0x008C3B00` in GOG, hook site `0x00504EF5` / H7 `HolsterPlace`) every single frame for weapons attached by `preview-3d` at `rhand` / `lhand` in a menu 3D scene view (`CSWGui3DSceneView`).

The call chain was fully traced from the top-level GUI panel draw down to the hook point.

---

## 2. Complete Call Chain: Who Calls vtbl+0x08 Per Frame?

```
CSWGuiPanel::Draw (0x0040EC90)
  |-- for each control in panel->m_controls (panel + 0x24):
  |   `-- control->vtbl+0x38(dt) [0x0040EC90 line 36]
        |
        `-- (When control is CSWGui3DSceneView, vtable 0x009A3D34):
            CSWGui3DSceneView::vtbl+0x38 = 0x0084DE40
              |-- calls *(view + 0x60)->vtbl+0x0C(dt) [0x0084DE40 line 6]
              |   (where view + 0x60 is CSWGuiScene, vtable 0x0098771C)
              |
              `-- CSWGuiScene::vtbl+0x0C = 0x004184F0
                    |-- calls scene->vtbl+0x1C(dt) [0x004184F0 line 8]
                    |   (where scene is CAurScene* at view + 0x74, vtable 0x0098BB44)
                    |
                    `-- CAurScene::vtbl+0x1C (slot 0x1C at 0x0098BB60) = 0x0046D230 [CAurScene::Animate]
                          |-- loops over all models in scene->m_models (scene + 0x54) [0x0046D230 lines 11-17, 27-34]
                          |   `-- model->vtbl+0x14(dt) [Gob::Update / Gob vtable 0x0098B5CC slot +0x14]
                                |
                                `-- Gob::Update (0x004E1D40)
                                      |-- checks if gob->m_pBehavior != 0 (gob + 0x1B4) [0x004E2065]
                                      `-- calls behavior->vtbl+0x08(dt) [0x004E208F: CALL [EDX + 0x08]]
                                            |
                                            `-- CAurBehaviorAttach::Control (0x00504D40)
                                                  |-- copies node N world pose into child Gob (gob + 0xA4, gob + 0xB0)
                                                  `-- executes 0x00504EF5 (MOV [EBP-0x3C], 1)
                                                        --> HOOK SITE H7: HolsterPlace(gob, beh)
                                                              `-- previewPlace(gob, beh)
```

---

## 3. Scene Model Membership: Is the Weapon Gob in `scene->m_models`?

**YES.**
1. In `patch_manager_mods/preview_3d_src/Preview3dProvider.cpp:290`:
   ```cpp
   reinterpret_cast<AttachFn>(vt[GobAttachSlot / 4])(g, body, hook, 0);
   ```
   `g` is the weapon Gob created via `FnCreateModel`, `body` is the creature body Gob in the preview scene, and `GobAttachSlot` is `0x50` (`Gob::AttachToPart`, `0x00461670`).
2. Inside `Gob::AttachToPart` (`0x00461670`):
   - Type `0` constructs `CAurBehaviorAttach` (`0x00504810`).
   - `weapon_gob + 0x1B4` is set to the new behavior pointer (`0x00461600`).
   - It invokes `0x00461AA0(weapon_gob, body->m_pScene)` (`body + 0x94`).
3. Inside `0x00461AA0`:
   - `weapon_gob->m_pScene` (`gob + 0x94`) is set to `body->m_pScene`.
   - It calls `scene->vtbl+0xB0(weapon_gob)` (`0x00461AA0` line 12).
4. `CAurScene::vtbl+0xB0` is `0x00465BB0`:
   - In `00465bb0.asm` lines 20-24:
     ```asm
     00465be8  PUSH EDX       ; weapon_gob
     00465be9  MOV ECX, [EBP - 4] ; scene
     00465bec  ADD ECX, 0x54  ; scene + 0x54 = scene->m_models (CExoArrayList<Gob*>)
     00465bef  CALL 0x00473F40 ; ArrayList::Add(weapon_gob)
     ```
   - Therefore, `weapon_gob` is explicitly added to `scene->m_models` of the non-world preview scene.
5. In `0x0046D230` (`CAurScene::Animate`), every model in `scene->m_models` is iterated and updated via `vtbl+0x14`.

---

## 4. Why Did Earlier Research Log "OmenRef01 Only"?

In `holster_c4_weapon_attach.md`:
- Hook H7 was originally placed at `0x005047F4` inside `0x005047B0` (`CAurBehaviorAttach` vtable slot `+0x20`).
- `0x005047B0` is an **event listener** triggered by `UpdateDependentPosition` via `0x005045E0`.
- Only objects whose parent was itself a child being transformed sent that specific event (e.g. `OmenRef01`), while direct attachments on animated character nodes rely on the **per-frame composer `vtbl+0x08`** (`0x00504D40`).
- Hooking `0x00504EF5` inside `0x00504D40` intercepts the true per-frame composer.

---

## 5. Potential Gaps & Smallest Fallback Design

While the engine does call `0x00504EF5` every frame when `CSWGui3DSceneView` is drawn:
1. **The 1-Frame Registration Latency**:
   - `RegisterPreview(gob, beh, left)` accepts `beh = NULL`.
   - If `beh` is null at register time, `e.parent` and `e.node` remain `nullptr` until the engine executes `0x00504EF5`.
   - If the caller invokes `BeginEdit(gob, scope)` in the very same frame before `0x00504EF5` runs, `e.parent` is still null and the body model name cannot be resolved.
2. **Inactive / Hidden Panel Frames**:
   - If `CSWGui3DSceneView` is hidden or during menu transitions, `CSWGuiPanel::Draw` skips calling `control->vtbl+0x38`, pausing the preview scene animation.

### Recommended Smallest Fallback:

#### Fallback 1: Resolve `beh`, `parent`, and `node` at `RegisterPreview` Time (Zero Latency)
- **Target File:** `patch_manager_mods/holster_src/Holster.cpp`
- **Function:** `apiRegisterPreview(void* gob, void* beh, int32_t left)` (lines 837-856)
- **Change:** If `beh == nullptr`, read `beh` directly from `gob + 0x1B4`:
  ```cpp
  if (!beh) {
      saferead::beginScope();
      beh = reinterpret_cast<void*>(static_cast<uintptr_t>(rdU32(kSitePlace, reinterpret_cast<uintptr_t>(gob), 0x1B4)));
  }
  ```
- **Rationale:** Because `Gob::AttachToPart` already wrote `*(gob + 0x1B4) = beh` when preview-3d attached the weapon, `gob + 0x1B4` is populated immediately upon attach. Reading it in `apiRegisterPreview` allows `e.parent` (`rdU32(beh, 0x14)`) and `e.node` (`rdU32(beh, 0x18)`) to be filled at register time, before waiting for the first engine refresh frame.

#### Fallback 2: Direct Behavior Refresh from Editor Tick
- **Target File:** `patch_manager_mods/holster_editor_src/HolsterEditor.cpp`
- **Function:** `cbTick(uint32_t page, void*)` (lines 198-201)
- **Change:** For each registered weapon Gob, if `previewPlace` has not yet run or if the panel is not drawing, invoke `beh->vtbl+0x08(0.0f)`:
  ```cpp
  // Call CAurBehaviorAttach::Control directly to force placement even if GUI Draw was skipped:
  void* beh = reinterpret_cast<void*>(rdU32(0xE70100u, reinterpret_cast<uintptr_t>(wpnGob), 0x1B4));
  if (beh) {
      uintptr_t vt = rdU32(0xE70100u, reinterpret_cast<uintptr_t>(beh), 0);
      uintptr_t fn = vt ? rdU32(0xE70100u, vt, 0x08) : 0;
      if (fn) reinterpret_cast<void(__thiscall*)(void*, float)>(fn)(beh, 0.0f);
  }
  ```
  This immediately triggers hook site `0x00504EF5` -> `HolsterPlace` -> `previewPlace`.
