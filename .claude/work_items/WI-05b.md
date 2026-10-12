# WI-05b Shared providers: GuiInput, GuiKit, IniKit (A1, B2, B3)
area: shared-src
size: S
ready: hold
items: C023,C025,C026,C027
files: patch_manager_mods/_shared_src/GuiInput.h; patch_manager_mods/_shared_src/GuiKit.h; patch_manager_mods/_shared_src/IniKit.h
out-of-repo: none
blockers: all rows gated by owner GO (doc 01:640 rollout). Provider state (checked 10-12): GuiInput.h has `game_hwnd` (:119), but `cursor_engine` and `wheel_subscribe` appear only in a usage comment (:66-67). GuiKit.h defines no `bindCover`, `setCtlVisible` or `setLabelText` (only a comment at :527). IniKit.h exists, but `maybeReloadIni` is not in `_shared_src`. C027 is consumed by WI-04 C129.
docs: patch_manager_mods/01_inventory_3d_viewport.md:638 (C023), :642-644 (C025-C027; line numbers are from 10-02); patch_manager_mods/19b_cross_patch_sharing_survey.md (A1, B2, B3)
deferred: C023,C025,C026,C027

## Goal
Hold. The shared providers for the registry (C023), GuiInput (A1), GuiKit (B2) and IniKit (B3) wait on the owner's GO. Nothing here is edited in this WI.

## Steps
1. None runnable now (all rows deferred). Gate notes for when the GO lands:
   - C023: registry interfaces (A-ids) use the RFC #180 registry from `19_shared_logging_design.md`. Keep the local fallback when the provider is absent.
   - C025: A1 consumer. Replace the window subclass, `gameWindow()` and `cursorEngine`/`syncScreen` with `game_hwnd`, `cursor_engine`, `wheel_subscribe`.
   - C026: B2. Move `bindCover`, `cursorEngine` and `syncScreen` into GuiKit.
   - C027: B3. Move `trim`, `parseFloats` and `maybeReloadIni` into IniKit (`trim` and `parseFloats` already exist there).

## Verify
1. When unlocked: `python3 scripts/compat_check.py <changed headers>` and `python3 scripts/check_shared_headers.py` (both exist).

## Docs
1. Mark-done phase ids are unknown. Use `python3 scripts/plan_manager.py mark-done <phase> --state <STATE> --commit <sha>` only after the GO.
2. Never sed the master plan. CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C023 patch_manager_mods/01_inventory_3d_viewport.md:638 id=- parked
- C025 patch_manager_mods/01_inventory_3d_viewport.md:642 id=A1 parked
- C026 patch_manager_mods/01_inventory_3d_viewport.md:643 id=B2 parked
- C027 patch_manager_mods/01_inventory_3d_viewport.md:644 id=B3 parked
