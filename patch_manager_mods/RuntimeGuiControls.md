# Runtime GUI Controls and Textures from a Patch DLL

This guide shows how a patch can add GUI elements to an existing engine panel from its own DLL: labels with text, images and
hover tooltips, plus custom art. It needs no `.gui` edit and no files in `override/`. The technique was developed for the
Force Power Hotbar (57 controls and 4 textures on the in-game HUD, live since 2026-10-02). It improves on the older
`.gui`-plus-bind approach that CharEffectsPanel, Inventory3DViewport and EquipCharPreview still use.

All addresses below are for **KOTOR II, Steam Aspyr** (`swkotor2.exe`, the two hashes in the hotbar's `hooks.toml`). Other
builds need the same functions looked up in their address database.

## Why Not Edit the .gui?

The usual way to add a control is to add it to the panel's `.gui` file (a GFF) in `override/`. This has four problems:

- **Unbound controls do nothing.** A panel only creates the controls its constructor binds by tag (`InitControl`). Extra controls
  in the `.gui` are inert (DB lesson 113), so a DLL is needed anyway.
- **One file wins.** Only one `override/<panel>.gui` can be live. Every other mod that edits the same panel (UI mods,
  widescreen packs, another patch) conflicts with you, and uninstalling has to restore the right version.
- **Resolution variants.** Some panels have per-resolution layouts, and each one needs the same edit.
- **Loose art goes stale.** Art shipped as loose `override/*.tga` files can be shadowed by, or go stale against, the DLL.

The runtime technique builds controls **by copying a native control that already exists in the layout** (a template). It
appends them to the panel's control list and serves the art from DLL memory. The patch is then a single `.kpatch`.
Uninstalling it removes everything, and a replaced `.gui` keeps working as long as it still contains the template tags.

## Requirements

A detour **inside the panel constructor's layout window**: after the last native `InitControl` and before
`StopLoadFromLayout` (`0x0040F5A0`). The layout GFF (panel `+0x30`) is only open in this window, and `InitControl` reads
the template from it.

| Panel | Hook address | Original bytes | Panel pointer |
|---|---|---|---|
| `CSWGuiMainInterface` (HUD), ctor `0x00747210` | `0x00748502` | `8B 8D 0C FA FF FF` (`mov ecx,[ebp-0x5F4]`) | `[ebp-0x5F4]` |
| Inventory, ctor `0x008A6170` | `0x008A69AD` | `8B 8D 08 FD FF FF` (`mov ecx,[ebp-0x2F8]`) | `[ebp-0x2F8]` |

You also need a per-frame hook for the panel to drive state, hover and clicks. The HUD uses `CSWGuiMainInterface::Draw`
`0x0074B3F0` (`ECX` = HUD).

## How It Works

### 1. Create a label from a template

```cpp
unsigned char* ctl = (unsigned char*)FnNew(0x148);            // engine operator new 0x00919723 (cdecl)
memset(ctl, 0, 0x148);
LabelCtor(ctl);                                               // CSWGuiLabel ctor 0x00419740, ECX = this, no stack args
*(int*)(ctl + 0x54) = -31337;                                 // id sentinel
CExoString tag("LBL_QUEUE1");                                 // 0x00733570 / dtor 0x00733780
InitControl(panel, ctl, &tag, /*addToList*/0, /*scale*/1);    // 0x0040F620 thiscall, RET 16
if (*(int*)(ctl + 0x54) == -31337) fail();                    // tag not in this layout: Load never ran
int index = *(int*)(panel + 0x28);                            // current control count
*(int*)(ctl + 0x54) = index;                                  // id = list index
vt(ctl)[1](ctl, rect);                                        // SetRect(int[4] x,y,w,h) in screen pixels
*(int*)(ctl + 0x48) = (*(int*)(ctl + 0x48) & ~0x2) | 0x20;    // hidden + click-through until the tick decides
PtrListAdd(panel + 0x24, ctl);                                // CExoArrayList<void*>::Add 0x0083EA60 thiscall, RET 4
// verify: count == index + 1 and array[index] == ctl, else do not use the control
```

- **`addToList=0` is essential.** With 1, `InitControl` writes the control into the list at index = its GFF `ID`, which belongs
  to the native template. With 0 it only loads the template's struct (border, text, font, colours) and sets the parent
  (`ctl+0x38`). The list is left alone (DB lesson 283).
- **Templates must exist in every variant of the layout.** The hotbar uses `LBL_QUEUE1` (a blank label: no fill, text `''`,
  `dialogfont16x16`) and `LBL_MOULDING3` (the native framed panel, used for the picker background and the tooltip frame).
- **Always append, never write by index.** Read the count just before each append. Then two patches that both append to
  the same panel never collide.

### 2. Position and scale

`InitControl` converts layout units to pixels. **While the global `0x00A1B9F0` is non-zero** (the whole HUD constructor runs
with it set), it applies a non-linear aspect projection, which made some hotbar slots 964 px wide (lesson 111). To avoid it:

- Clear `0x00A1B9F0` around your binds and restore it afterwards. The constructor clears it right after the hook anyway
  (`0x0074850D`).
- Compute rects yourself. The GUI reference space is `*(int*)0x009F4390` x `*(int*)0x009F4394` (800 x 600). Take the game
  window's client size from `GetClientRect`, **not** from `FUN_0073FEA0`: that is a thiscall that crashed when called without
  `this`.
- The HUD draws at a uniform `H/600` px per unit, so scale y by `H/600`. Scale x by `W/800` and then squeeze it by
  `c = (H/600)/(W/800)`. At 3840x1600 the hotbar's wrench (layout `4,478,20,35`) lands at `[11,1274,53,93]`.
- Control extents at `ctl+4/+8/+0xC/+0x10` are **screen pixels**. Hit-testing compares them with
  `GetCursorPos` + `ScreenToClient`.

### 3. Text, font, alignment, colour

The text sub-object is at `ctl + 0xD8 + 0x18` (= `ctl+0xF0`). All of these are thiscall on that pointer:

| Function | Address | Argument |
|---|---|---|
| `CSWGuiText::Set` | `0x00416E30` | `CExoString*` (re-lays out the text) |
| `SetFont` | `0x00416DE0` | `char resref[16]`, e.g. `dialogfont10x10` |
| `SetAlignment` | `0x00416FA0` | `unsigned`: 9 = top-left, 17 = top-centre, 18 = centre |
| `SetColor` | `0x00417140` | `float rgb[3]` |

Without an explicit `SetAlignment`, DLL-set text draws left-aligned whatever the template says (lesson 265).

### 4. Visibility, hit-testing, draw order

`ctl+0x48` holds the flags:

- **bit 1 (`0x2`), visible.** The panel's `Draw` skips the control when it is clear.
- **bit 5 (`0x20`), click-through.** The label hit-test `0x00418D20` rejects the mouse when it is set.
  - Keep it set on hidden and decorative controls, so they never swallow a click meant for the world.
  - Clear it on controls that must absorb clicks. For example, a visible hotbar slot, so clicking it does not also move
    the character.

The panel draws its list in ascending order, so **list index = z-order**. Appended controls draw above all native ones. Put
whatever must be on top (tooltips) **last** in your creation order.

### 5. Images on a label

The label's border is at `ctl+0x60`; its fill image pointer is at `border+0x74`, its alpha at `border+0x24` and its tint
(rgb) at `border+0x28`.

- **Getting an image.** `FnLoadImage` `0x0047EB60` (cdecl, `char resref[17]`) returns an image wrapper, and **allocates a new
  one on every call**. Pool the results by resref and never release them. Swapping a fill is then a pointer write.

### 6. Textures from DLL memory (no `override/` files)

The art is embedded in the DLL as uncompressed 32-bit TGA. The hotbar's `scripts/gen_hotbar_textures.py` writes it to
`HotbarTextures.inc`. It is registered as a real engine texture under a unique name (lessons 283, 290):

1. **Allocate and construct.** Allocate `0xF4` bytes with the engine `new` (`0x00919723`) and zero them. Call the
   `CAurTextureBasic` ctor `0x00423000` (thiscall `(this, name, name)`, RET 8). This **registers the name globally**
   (case-insensitive), so use a unique prefix (`fph_`).
2. **Fill the pixels.** Allocate the RGBA buffer with the engine allocator `0x0091D288`, which the engine later frees with
   `delete[]`. Swap BGRA to RGBA and keep the rows as stored. Then set these fields:

   | Offset | Value | Meaning |
   |---|---|---|
   | `+0x44` | buffer | pixels |
   | `+0x68` | w | width |
   | `+0x6C` | h | height |
   | `+0x74` | 4 | bytes per pixel |
   | `+0x78` | 0 | format |
   | `+0x5C` | 0 | mip bytes |
   | `+0xDF` | 0 | not a cube map |
   | `+0xE7` | 0 | not from a resource |
   | `+0xE9` | 0 | not a TPC |
   | `+0xE5` | 0 | not missing |
   | `+0xE4` | 1 | "constructed": the engine never asks the resource manager for it |

   Never set `+0xE6` (NormCubeMap only).
3. **Queue the upload.** Push the texture on the texture work list `0x00A1BC2C` with `0x004491C0`. The texture tick uploads
   it to GL and frees the buffer.
4. **Reference it from a label.** Call `FnLoadImage(name)` once and keep the handle for the life of the process; that keeps
   the texture alive.
5. **Self-heal after a GL reset.** A video-mode change or texture-quality change runs ResetAll `0x00427D90` / SetMem
   `0x00427DE0`. This Resets every registered texture (`+0xE4 = 0`, pixels and GL names freed) and re-queues them all. Check
   once per frame:
   - **Not built and no pixels:** refill.
   - **Built but `+0xE5` missing, or the wrong width:** the engine rebuilt first and its resource lookup failed. Reset
     (`0x00423990`, fastcall) and refill.

   Duplicate work-list entries are harmless. Cap the retries (the hotbar uses 20).

### 7. Input without engine handlers

Runtime labels have no event handlers. The hotbar polls in its per-frame hook:

- **Mouse.** `GetAsyncKeyState(VK_LBUTTON)` edges, with the cursor from `GetCursorPos`/`ScreenToClient` tested against the
  control's pixel extent. Only poll when the game window is in the foreground and your gate is open, so menus and dialogue
  keep their clicks.
- **Hover sound.** The native mouse-enter sound is `PlayGuiSound` `0x004122A0`, thiscall on `*(void**)0x00A1B49C`, with
  `char idx = 1`.
- **Mouse wheel.** Subclass the game window (`SetWindowLongPtrA(GWLP_WNDPROC)`) and consume `WM_MOUSEWHEEL` only inside your
  area. On unload, restore the old proc **only if yours is still on top**, because other patches subclass the same window.
- **Keyboard.** A separate DirectInput hook; see the hotbar's `CaptureHotbarKey` (`0x0072CB63`).

### 8. Lifetime and ownership

- **The engine never frees runtime controls.** The panel destructor (`0x0040E860` base) does not walk the `+0x24` list, so
  appended controls leak with their panel (lesson 303). The upside is that there is no double free. Never free them yourself
  either: the list still points at them until the panel is gone.
- **Keep per-panel state in one struct reset at construction.** The hotbar keeps everything in `Bar` and runs
  `g_bar = Bar()` in the constructor hook, keyed on the panel pointer. A new panel never sees stale widgets, timers or hover
  state.
- **Textures and pooled images are per process.** They outlive every panel.

## Side Effects (audit, 2026-10-02)

**Verified, by static analysis and/or live testing:**

| Area | Result |
|---|---|
| Files | None in `override/`; the `.gui` is untouched. Uninstalling the patch leaves only its INI and log. |
| Native controls | Unaffected. `addToList=0` never touches the list. Native controls keep indices `0..N-1`, and ours are `N..N+56`. |
| Other `.gui` mods | Compatible if the template tags exist. If they don't, the control is not created, the log says so, and the dependent feature (slot, editor or tooltip) disables itself. No crash. |
| Memory | 57 x `0x148` B (about 19 KB) per HUD construction, plus small text buffers, never freed. Live: 55 HUD builds in 50 game sessions (about once per launch), so negligible. |
| Textures | 4 small RGBA textures, engine-allocated, so the engine's own free and reset paths match. The GL-reset ordering was analysed statically (lesson 290). |
| Projection flag | Cleared and restored inside the hook. The constructor clears it right after anyway. |
| Clicks | Hidden controls carry `0x20`, so the world and native controls get every click. Only visible hotbar slots and open editor controls absorb clicks, by design. |
| Saves | Nothing is written to save games. |
| Per-frame cost | State is cached; engine memory is written only when a value changes. No per-frame allocations (images are pooled). |

**Bounded, with rules for contributors:**

- **Several patches on one panel.** Each must append with a fresh count read. A late `InitControl(..., addToList=1)` of a
  `.gui` control whose GFF `ID` is at or above the native count would overwrite a runtime slot. All native binds happen
  before the hook, and no other installed patch touches the HUD list today (checked against every patch source in this repo).
- **Visibility cache.** The hotbar skips a flag write when its cached state already matches. If engine code ever flipped
  bit 1 on a runtime control directly, the hotbar would only re-assert it on its next state change. This has not been seen
  in 50+ live sessions. The hotbar 1.0.0 review compares against the live flag.
- **Old controls.** After a HUD rebuild, the old HUD's runtime controls leak with it and still point at pooled images and
  textures. That is safe because those are never freed.

**Not yet tested:**

- **Resolution change** in the options menu while a save is loaded. Rects are computed once, at construction. If the engine
  rescales existing controls instead of rebuilding the HUD, they could be misplaced.
- **Gamepad UI navigation** on the HUD. Labels are not selectable, so they are expected to be inert.
- **The GL-reset self-heal live** (texture-quality change).
- **Other game builds** (GOG, K1). The addresses differ.

## Reference Implementations

- **Force Power Hotbar.** `Patches/ForcePowerHotbar/ForcePowerHotbar.cpp`:
  - `createLabel` and the `Layout[]` table (57 controls, layout units);
  - `ensureTextures` / `fillTexture` / `healTextures`;
  - `HotbarBuildButtons` (constructor hook) and `HotbarHudTick` (per-frame).
- **Character Effects Panel, Inventory 3D Viewport, Equipment Character Preview.** These use the older variant. They add
  controls to the panel's `.gui` override and bind them by their own tags with `addToList=1`, so each control lands at its
  GFF `ID`. It works, but it carries the `.gui` problems listed above. They could move to template copy + append.
- **`GuiKit.h`.** A header-only helper (`patch_manager_mods/_shared_src/` in the authors' workspace) that wraps the label recipe,
  scaling and polling from the hotbar and the effects panel behind testable function pointers.

Research trail and every live log quoted here: `patch_manager_mods/07_force_power_hotbar.md`, plus lessons 67, 104, 111, 113,
265, 283, 290 and 303 in `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db`.
