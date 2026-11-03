# Pull Request: Quick Loot (`quick-loot`)

### PR Title
`feat(hud): quick loot container peek, hardware bezels, and item take (#104)`

### Commit Message
```text
feat(hud): quick loot container peek, hardware bezels, and item take

Replaces intrusive full-screen container menus with a fast, in-world
overlay card supporting proximity peek, cursor hover peek, single-click
take-all, and per-item looting.

- Patches/QuickLoot/manifest.toml: patch manifest (v0.8.0-dev)
- Patches/QuickLoot/kotor2-steam-aspyr.hooks.toml: hook definitions
- Patches/QuickLoot/QuickLoot.cpp: engine detours, lifecycle callbacks, HUD rendering
- Patches/QuickLoot/QuickLootLogic.h: layout geometry, list formatting, dynamic action button
- Patches/QuickLoot/QuickLootMachine.h: state machine for select, hover, proximity, and take
- Patches/QuickLoot/QuickLootBezel.h: 7 container categories and 6 synthesized hardware bezels
- Patches/QuickLoot/QuickLootBezelTex.inc: embedded BGRA 32-bit bezel textures
- Patches/QuickLoot/QuickLootPage.h: Mod Configuration menu page and preview modal
- Patches/QuickLoot/README.md: usage, controls, and configuration documentation
```

### PR Description Body
```markdown
### Summary of Changes
Implements a non-intrusive container peek and looting system (Quick Loot) for *Star Wars: Knights of the Old Republic II*. Containers can be peeked via cursor hover, world proximity, or targeting select. Players can take all items instantly via hotkey (`F`) or action button, or take individual items by clicking list rows.

### Motivation & Background
Looting in vanilla KotOR II requires opening a modal container interface for every corpse, footlocker, and cylinder, halting movement and obscuring gameplay. Quick Loot provides modern RPG looting ergonomics while retaining full compatibility with vanilla quest scripts, inventory events, and container traps.

### Technical Implementation
- **Lifecycle & Hook Points (`kotor2-steam-aspyr.hooks.toml`):**
  - Intercepts container GUI display calls, consuming full-screen modal requests while executing appropriate inventory synchronization via `CServerExoApp::ClientToServerObjectId` and `CSWSPlaceable::CloseInventory`.
  - Detours `K2HUD_TARGET_MENU_POST` (`0x00743847`) to project the Quick Loot card lockstepped with in-game container geometry and target plates.
  - Spliced with `CClientExoAppInternal::SetMouseCursor` (`0x007B3870`) for empty-container cursor states.
- **Embedded Visual Assets (`QuickLootBezelTex.inc`):**
  - Compiles uncompressed 32-bit BGRA bezel textures directly into the patch DLL via `embtex::Tex`, enabling single-DLL hot-swap distribution without requiring loose texture file overrides in `Override/`.
  - Supports 6 distinct hardware bezels (Computer Console, Datapad, Workbench, Upgrade Station, Store Terminal, Pause Gold) matched to container categories.
- **Mod Configuration Integration:**
  - Exposes interactive settings and real-time category skin previews via `mod-config`.

### Testing & Verification
- [x] Comprehensive automated unit test suite (`quick_loot_logic_test`, `quick_loot_sim_test`, `quick_loot_page_test`, `quick_loot_rules_test`).
- [x] Pre-install gate verified (113/113 test suites passing).
- [x] Live in-game hot-swap verified across Telos containers, salvage packs, and corpses.
- [x] Verified quest containers and Give-mode screens safely fall back to the vanilla menu.
- [x] Shift+Click manual bypass verified.

Relates to #104
```
