# Quick Loot (`quick-loot`)

> **STATUS** Track: SHIPPABLE. Version 0.8.0-dev (matches manifest.toml).
> **What it does / explores:** FO4-style container peek and take: proximity and cursor-hover peek, take-all key, per-item take, composed hardware bezels, Mod Configuration page.
> **Requires:** Steam/Aspyr KOTOR II (target hash in manifest.toml), KPM with the `hud-host` and `gui-input` providers. `mod-config` is a soft dependency (the page is skipped when absent).
> **Known limits:** QL5 silent Close on an empty container click; the literal `{08}` name was fixed in 0.7.0 but is not yet play-confirmed. **Play-test status:** pending rows QL0-P1, QL2p-P1, QL061-P1, QL07-P1, QL071-P1, QL072-P1, QL03-P1, QL-OH1 in `PLAY_TEST_QUEUE.md`. QL2V-P1 and QL06-P1 were FAIL/PARTIAL with later fixes not yet confirmed.
> **Install:** normal play via KPM install. Set `Verbose=1` only when collecting QL0 probe logs.

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
  - Hardware bezels composed at first use from the game's own panel textures (no art ships):
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
  - Interactive preview canvas displaying bezel frames and container styles in real-time.

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
TakeKey=F             ; Key to trigger take-all action (shipped ini stores VK code 70)
ScanMs=250            ; Proximity scan interval in milliseconds
ListRows=4            ; Number of visible rows before scrolling (1..12, default 4)
Anchor=0              ; 0 = under name tag, 1 = below container, 2 = beside name tag
Verbose=0             ; 1 = also log QL0 probe lines (ITEM / EVTRAW / EVT / SCAN)
GiveTags=             ; Comma/semicolon-separated placeable tags that always open the vanilla menu
HardwareBezels=1      ; Enable the composed hardware bezels (code default 1; not in the shipped ini)
HideTargetArrow=1     ; Hide the target arrow while Quick Loot is active (code default 1; not in the shipped ini)
```

`CloseM` defaults to `OpenM + 2` when the key is absent from the ini.

---

## Dependencies

- `hud-host` (`k2.hud@1`): Lifecycle frame hooks and GUI projection.
- `gui-input` (`k2.guiinput@1`): Mouse wheel input capture and routing.
