# GUI Input (`gui-input`)

> **STATUS** Track: SHIPPABLE. Version 0.1.0-dev (matches manifest.toml).
> **What it does / explores:** Infrastructure provider (`k2.guiinput@1`) that owns the single subclass of the game window and exposes the game HWND, the cursor in engine coordinates, foreground state, and mouse-wheel subscriptions to other patches.
> **Requires:** A KotorPatcher build with the patch registry (`KPatchInit`, see manifest.toml description). Steam/Aspyr and GOG/Aspyr targets are both listed in manifest.toml `[patch.supported_versions]`. No other patch is required.
> **Known limits:** The Win32 layer (`GuiInputProvider.cpp`: game-window discovery, wheel hook, cursor-to-engine mapping, screen-size reads) has no host test. The host tests cover only `GiDispatch.h` and the registry/common contract. Consumers still keep some legacy per-mod window subclassers (tracked in the author's development workspace). **Testing:** live-tested on Steam; the mod-config wheel through gui-input still needs an in-game check.
> **Install:** Install gui-input before every consumer (see Install order). It has no engine hooks (`hooks = []` in both `kotor2-*.hooks.toml`), so it is DLL-only.

## Interface: `k2.guiinput@1`

Declared in Patches/Common/GuiInput.h` (`K2GUIINPUT_IFACE "k2.guiinput"`, `K2GUIINPUT_VERSION 1`). The C ABI is `K2GuiInputApi`; every field is a function pointer and `struct_size` gates newer fields.

| Member | Purpose |
| --- | --- |
| `GameHwnd()` | HWND of the game window, or 0 if not found yet. |
| `CursorEngine(x, y)` | Cursor in engine virtual pixels (top-left origin). Returns 1 if valid, 0 if unavailable. |
| `WheelSubscribe(cb, user)` | Register a wheel callback. Returns a token > 0, or <= 0 on failure (null cb, table full, game window not yet available). |
| `WheelUnsubscribe(cb)` | Remove all registrations matching `cb`. |
| `Foreground()` | 1 if the game window is the foreground window. |

A wheel callback returns 1 to consume the event or 0 to pass it on. The event (`K2WheelEvent`) carries `engine_x/engine_y` and `client_x/client_y`.

C++ consumers use the helper in `GuiInput.h` (`k2input::Client`): call `init(api, "my-patch-id")` and `connect()` in `KPatchInit`, then `poll()` per frame. Without a provider the helper's calls are safe no-ops or fallbacks.

## Install order

1. gui-input (this patch).
2. Consumers that list `k2.guiinput@1` or `requires = ["gui-input"]`: action-hotbar, inventory-3d-viewport, equipment-3d-character, active-effects-panel, quick-loot.

A consumer installed without gui-input falls back to its own behaviour (no shared cursor or wheel).

## Features

- One owner for the game-window subclass (consolidates the stacked subclasses into one provider).
- Cursor in engine coordinates, so consumers do not each redo the client-to-engine mapping.
- Foreground tracking.
- Mouse-wheel subscription dispatch with consume/pass-through.
- Logging through the shared `KpLog.h` logger (`kplog.ini`).

## Configuration

None. The provider has no ini keys or Mod Configuration page.

## Build

From this folder run `python3 ../create-patch.py`. Host unit tests (dispatch, registry, common contract) live in the author's development workspace and are not shipped here.

## Known issues

- `GuiInputProvider.cpp` is not compiled by any host test target (see Known limits).
- Consumers' wheel handling is mixed. `18_mouse_camera_investigation.md` says the wheel is unified; `23_menu_3d_shared_migration.md` says consumers keep their own wheel proc. Treat the provider API as the intended path, not as a guarantee every consumer uses it.
- Version keeps the `-dev` suffix until the upstream PR decision (project versioning rule: 0.x.y).

## Changelog pointer

Versions and history: `manifest.toml` `[patch] version` and the repo commit log for the project workspace.
