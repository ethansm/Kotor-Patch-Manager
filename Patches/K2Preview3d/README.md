# Preview 3D

> **STATUS** Track: SHIPPABLE. Version 0.2.0-dev (matches manifest.toml).
> **What it does / explores:** A shared provider of per-token 3D character viewers for GUI panels: bind a creature, rebuild its model, set weapons, drive the camera and play or pause an animation. Exposes `k2.preview3d@1`.
> **Requires:** Kotor Patch Manager with the patch registry (`KPatchInit`); KOTOR II Steam (Aspyr) LAA build, sha256 `6A522E71...` (manifest.toml also lists the GOG Aspyr build, `777BEE23...`); the `Patches/Common` headers listed under Dependencies.
> **Known limits:** No player-facing UI of its own, so it is only useful with a client mod (the Holstered Weapons settings page in K2Holster). No engine hooks. `Preview3dProvider.cpp` has no host test; only `Preview3dLogic.h` and the ABI are covered. Helmet visor animation is not frozen by `SetAnimPaused`. **Testing:** live-tested on Steam; the Holstered Weapons page 3D view still needs an in-game check.
> **Install:** install through Kotor Patch Manager, before the client mod (holster lists `preview-3d` in its `requires` for its settings page). Install with holster; alone it does nothing visible.

## What the provider is

`preview-3d` is a DLL-only provider (`kind = "provider"`, `hooks = []`). It owns the view block, the creature Gob, the weapon and helmet Gobs and the camera maths for each panel token. A consumer never builds a creature itself; it asks the provider through `k2.preview3d@1`.

Source files:

- `Preview3dProvider.cpp`: the provider. `KPatchInit` (line 762) registers the table.
- `Preview3dLogic.h`: pure logic for the viewer table, the clock and the drag state (host-tested by `preview3d_logic_test.cpp`).
- `common_contract_test.cpp`: pins the Common APIs this mod depends on.
- `manifest.toml`, `test.toml`, `exports.def` (exports `KPatchInit` only), `kotor2-steam-aspyr.hooks.toml`.

## API summary: k2.preview3d@1

Defined in `Patches/Common/Preview3dApi.h`: the interface name and version are at lines 46-47, the struct at lines 68-105. Slots 1-11 are the original v1 table; slot 12 was appended in provider 0.2.0. The number is the pointer index after `struct_size`.

| Slot | Call | Purpose |
|---|---|---|
| 1 | `CreateView(panel, tag)` | Add a 3D view on layout control `tag`. Valid only inside the panel constructor. Returns a token, 0 on failure. |
| 2 | `Bind(token, creature)` | Set the real creature to show. Builds nothing. |
| 3 | `Rebuild(token)` | Build the preview from the bound creature (appearance, scene, idle, view, weapons, helmet). |
| 4 | `SetWeapons(token, on)` | Weapons shown or hidden. View state is untouched. |
| 5 | `WeaponGob(token, hand)` | Right or left weapon Gob, or 0. |
| 6 | `SetView(token, yaw, zoom, panZ)` | Camera and facing. |
| 7 | `PlayAnim(token, anim)` | Play a clip on the body and head. |
| 8 | `RigGob(token, part)` | Body or head rig Gob, or 0. |
| 9 | `Destroy(token)` | Free the creature and its Gobs. Keeps yaw, zoom, panZ and Bind; the token stays valid. |
| 10 | `SetBeforeFree(cb, user)` | Register a callback that runs before any weapon Gob is freed (up to 4). |
| 11 | `ViewControl(token)` | The CSWGui3DSceneView control, for listing and resizing. |
| 12 | `SetAnimPaused(token, paused)` | Optional (appended in 0.2.0). Freezes the body and head at the current pose. Check `Client::canPause()` before calling. |

Consumers connect through the `Client` helper at `Preview3dApi.h:133-175` (`api->require(K2PREVIEW3D_IFACE, K2PREVIEW3D_VERSION)`, with the struct-size and null checks). Tokens are `uint32_t`; an unknown or stale token makes a call a no-op and never crashes.

## Consumers

- **holster** (`Patches/K2Holster`): the 3D view on its Holstered Weapons settings page (merged from holster-editor in 0.22.0). Its manifest lists `requires = ["preview-3d", "mod-config"]`.
- **equipment-3d-character**: may follow. Not a dependency today.

## Dependencies on Patches/Common and KPM Common

The provider includes these headers from Patches/Common/`:

- `Preview3dApi.h` (the interface), `GameAddr.h` (engine addresses), `SceneView.h` (which pulls in `SafeRead.h` and `K2Addr.h`), `ViewDrag.h` (camera maths, via `Preview3dLogic.h`).

It also includes `kpatch_api.h` and `KpLog.h`, which are not in `Patches/Common`. They come from the KPM Common directory: `$KPM_COMMON_DIR`, else `<KPM_DIR>/Patches/Common`. `GuiKit.h` is not used by this mod.

A fork must carry all of `Patches/Common` that the list above names. The fork also needs a KPM Common checkout for `kpatch_api.h` and `KpLog.h`.

## Build

From this folder run `python3 ../create-patch.py`. Host unit tests for `Preview3dLogic.h` live in the author's development workspace and are not shipped here.

## Changelog pointer

- No CHANGELOG.md.
