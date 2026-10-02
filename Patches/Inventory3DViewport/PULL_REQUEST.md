# Pull Request: Inventory 3D Viewport

### PR Title
`feat(inventory): live 3D item viewport in ultrawide margin (#104)`

### Commit Message
```text
feat(inventory): add live 3D item model viewport

Renders an interactive 3D model of the selected inventory item in the
unused ultrawide screen margin of the inventory screen.

- Patches/Inventory3DViewport/manifest.toml: patch manifest (v1.0.0)
- Patches/Inventory3DViewport/kotor2-steam-aspyr.hooks.toml: detour hooks
  for inventory ctor, item selection event, and per-frame tick
- Patches/Inventory3DViewport/Inventory3DViewport.cpp: 3D scene view
  lifecycle, quaternion orbit camera, custom lighting rig
- Patches/Inventory3DViewport/README.md: setup, controls, model fit table
```

### PR Description Body
```markdown
### Summary of Changes
Adds a real-time, interactive 3D model preview of the selected inventory item in the unused margin of the inventory panel on 16:9, 21:9, and 32:9 displays.

### Motivation & Background
In vanilla KOTOR II, item inspection is limited to static 2D icons and text descriptions. Ultrawide displays leave significant unused screen space to the right of the inventory list. This patch proves the integration of engine 3D scene views into standard 2D GUI panels, directly fulfilling goals outlined in **#104 (GUI Modding Framework)**.

### Technical Implementation
The mod injects a live `CONTROL_3DVIEW` into `CSWGuiInGameInventory` and binds custom camera and lighting rigs:
- **Hook 1 (`0x008A69AD` - `BindInventoryView`):** Detours `CSWGuiInGameInventory` constructor immediately before `StopLoadFromLayout`. Slices panel pointer from `[ebp-0x2F8]` and initializes the 3D scene view child control.
- **Hook 2 (`0x008A8172` - `ShowItem`):** Detours the inventory item click/selection event handler right after `GetItemByObjectId`. Passes item pointer (`EAX`) and panel (`[ebp-0x68]`) to load and fit the item model based on base-item category.
- **Hook 3 (`0x008A7350` - `InventoryTick`):** Detours `CSWGuiInGameInventory::Update` to handle mouse drag rotation, yaw/pitch inertia, and zoom wheel interpolation.

### Controls & Configuration
- **Left Mouse Click + Drag:** Free orbit rotation.
- **Right Mouse Click + Drag:** Roll / tilt orientation.
- **Mouse Wheel:** Smooth zoom in / out.
- **Auto-Fit:** Per-category bounding boxes ensure lightsabers, blasters, and armor models are framed correctly.

### Testing & Verification
- [x] Verified on Steam Aspyr KOTOR II (Windows & Proton/Linux).
- [x] Tested across 16:9 (1920x1080), 21:9 (3440x1440), and 24:10 (3840x1600).
- [x] Verified zero memory leaks on repeated panel open/close cycles.
- [x] Passes `python3 tools/validate-patches.py`.

Relates to #104
```
