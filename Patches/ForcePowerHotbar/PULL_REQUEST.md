# Pull Request: Force Power Hotbar

### PR Title
`feat(hud): 30-slot keybound Force Power and ability hotbar (#104, #72, #180)`

### Commit Message
```text
feat(hud): add 30-slot keybound Force Power hotbar

Implements an on-screen HUD hotbar (1-0, Ctrl+1-0, Alt+1-0) for rapid
power invocation with hover tooltips and in-game configuration.

- Patches/ForcePowerHotbar/manifest.toml: patch manifest (v1.0.0)
- Patches/ForcePowerHotbar/kotor2-steam-aspyr.hooks.toml: HUD hooks
- Patches/ForcePowerHotbar/ForcePowerHotbar.cpp: hotbar buttons, input
  interception, spell slot queueing, INI persistence
- Patches/ForcePowerHotbar/HotbarConfig.h: hotbar slot mapping structures
- Patches/ForcePowerHotbar/README.md: hotbar keybindings and INI options
```

### PR Description Body
```markdown
### Summary of Changes
Implements a 30-slot customizable HUD hotbar mapped to `1-0`, `Ctrl+1-0`, and `Alt+1-0` above the action bar.

### Motivation & Background
Navigating KOTOR II's multi-tier radial menus to cast Force powers or use stims interrupts combat pacing. This patch brings modern hotbar functionality directly to the main gameplay HUD, queueing actions into the engine's combat pipeline without breaking turn-based combat rules.

### Technical Implementation
- **HUD Injection:** Hooks `CSWGuiMainInterface` constructor to dynamically instantiate 10 on-screen button controls (`LBL_HOTBAR*`) positioned above the action bar.
- **Action Dispatch:** Routes keypresses and clicks through `CServerExoApp::UseForcePower` / `ExecuteActionQueue`, respecting Force point costs, alignment restrictions, and combat cooldowns.
- **In-Game Picker:** Middle-click or quick-toggle allows assigning known Force powers, feats, or combat items to any slot, persisting to `force_power_hotbar.ini`.

### Testing & Verification
- [x] Verified combat casting, pause-and-queue, and out-of-combat buffing.
- [x] Tooltip hover shows power name, Force cost, and range.
- [x] Verified compatibility with ultrawide and 16:9 HUD layouts.
- [x] Passes `python3 tools/validate-patches.py`.

Relates to #104, #72, #180
```
