# Pull Request: Character Active Effects Panel

### PR Title
`feat(character): scrollable active effects breakdown panel (#104)`

### Commit Message
```text
feat(character): add scrollable active effects panel

Displays all active beneficial Force powers, stims, and harmful debuffs
with remaining duration in a dedicated panel on the character screen.

- Patches/CharEffectsPanel/manifest.toml: patch manifest (v1.0.0)
- Patches/CharEffectsPanel/kotor2-steam-aspyr.hooks.toml: hook definitions
- Patches/CharEffectsPanel/CharEffectsPanel.cpp: effect list enumeration,
  scrollable listbox, harmful/beneficial color coding
- Patches/CharEffectsPanel/README.md: panel usage and configuration
```

### PR Description Body
```markdown
### Summary of Changes
Embeds a scrollable active effects panel on the character sheet, itemizing every buff, debuff, stim, and environmental effect currently affecting the viewed companion.

### Motivation & Background
Vanilla KOTOR II displays small status icons beneath the portrait, but provides no centralized screen where players can inspect active spell names, source items, and exact durations.

### Technical Implementation
- **Lifecycle Hooks:**
  - `0x0084D871` (`BindCharEffects`): Detours `CSWGuiInGameCharacter` constructor before `StopLoadFromLayout` to create the child effects list box.
  - `0x0084FBB0` (`CharEffectsTick`): Per-frame update calculating remaining effect durations.
  - `0x0084DEF0` (`CharEffectsDestroy`): Clean destruction on panel close.
- **Effect Traversal:** Iterates the creature object's `CGameEffect` linked list, categorizing effects into Harmful (red) and Beneficial (blue/green) with formatted time strings (`MM:SS`).

### Testing & Verification
- [x] Verified effect updates when switching viewed companions (`ClientParty::GetCurrentCreature`).
- [x] Tested listbox scrolling when active effects exceed panel view height.
- [x] Built and verified under MinGW and MSVC.
- [x] Passes `python3 tools/validate-patches.py`.

Relates to #104
```
