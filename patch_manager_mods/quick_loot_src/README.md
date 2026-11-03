# Quick Loot (`quick-loot`)

A non-intrusive container peek and instant looting system for *Star Wars: Knights of the Old Republic II: The Sith Lords*.

Inspired by modern RPG container mechanics (Fallout 4 / Starfield), Quick Loot replaces the intrusive full-screen container menu with a compact in-world overlay card positioned directly beneath target containers or under the cursor.

---

## Features

- **Proximity & Hover Peek:**
  - **Proximity:** Approaching an unlocked container within range (`OpenM = 3.0m`) reveals its contents. Leaving (`CloseM = 5.0m`) cleanly closes it.
  - **Hover:** Hovering over a container in the 3D world with the mouse cursor reveals a floating card with container title and contents.
  - **Select:** Clicking a container targets it and opens the card in place.
- **Single-Click & Per-Item Take:**
  - Press the take key (`TakeKey = F`) or click the `[F] GET ITEMS` action button to take everything in one smooth action.
  - Click any individual item row to walk up and take just that item without closing the overlay.
- **Hardware Bezel Overhaul (QL14):**
  - High-resolution synthesized hardware bezels embedded directly into the DLL:
    - **Computer Console:** 4-sided steel bezel with electric blue ribbed vents and cyan inner rim.
    - **Datapad:** Rugged handheld tactical casing with corner bumpers and scanline CRT backing.
    - **Workbench:** Bio-emerald circuit matrix designed for lab stations and crafting benches.
    - **Upgrade Station:** Industrial metallic housing with circuit micro-traces.
    - **Store Terminal:** Art-Deco bronze and gold framing.
    - **Pause Gold:** Classic KotOR gold ornamentation with corner boss studs.
    - **Plain Wireframe:** Clean, lightweight teal hudkit frame.
- **Intelligent Category Classifier:**
  - Containers are automatically classified into 7 loot categories based on appearance and tags (`CatDroidTech`, `CatWeaponArmory`, `CatMedicalLab`, `CatStrongboxCache`, `CatRemainsCorpse`, `CatFootlockerGen`, `CatDefaultFallback`), skinning the frame to match the placeable type.
- **Rich Hover Tooltips:**
  - Hovering over an item row displays a full vanilla-styled inspection card showing item icon, base damage/defense properties, requirements, equipped comparison, and description.
- **In-Game Mod Configuration & Live Preview:**
  - Fully integrated with `mod-config` (`Options -> Mod Configuration -> Quick Loot`).
  - Interactive preview canvas displaying bezel frames, color palettes, and container styles in real-time.

---

## Controls

| Action | Control | Notes |
|---|---|---|
| **Take All** | `F` or Click `[F] GET ITEMS` | Takes all items; flashes acquired summary |
| **Take One Item** | Left Click on item row | Moves character to container and acquires selected item |
| **Scroll Item List** | Mouse Wheel over card | Visible when container has more than `ListRows` items |
| **Vanilla Menu Fallback** | Hold `Shift` while clicking container | Bypasses Quick Loot and opens the classic container screen |

---

## Configuration (`quick_loot.ini`)

```ini
[QuickLoot]
Enabled=1             ; Master enable toggle (1 = on, 0 = off)
ProximityEnabled=1    ; Auto-peek containers within proximity distance
HoverPeek=1           ; Auto-peek containers when cursor hovers over them
EmptyCursor=1         ; Display empty cursor pointer on looted containers
HoverDwellMs=150      ; Cursor hover debounce time in milliseconds
OpenM=3.0             ; Distance in meters to trigger proximity peek
CloseM=5.0            ; Distance in meters to close proximity peek
TakeKey=F             ; Key to trigger take-all action
ListRows=8            ; Number of visible rows before scrolling (1..12)
Anchor=0              ; 0 = under name tag, 1 = projected world coordinate, 2 = beside name tag
HardwareBezels=1      ; Enable high-res embedded hardware bezels
DefaultBezel=2        ; Default fallback bezel (2 = Computer Console)
DefaultPalette=0      ; Default palette tint
```

---

## Dependencies

- `hud-host` (`k2.hud@1`): Lifecycle frame hooks and GUI projection.
- `gui-input` (`k2.guiinput@1`): Mouse wheel input capture and routing.
