# Pull Request: Ranged AI

### PR Title
`feat(ai): party stance 16 Ranged (Kiting) and line-of-sight hold (#72, #55)`

### Commit Message
```text
feat(ai): add Ranged (Kiting) stance and line-of-sight hold

Prevents ranged companions from charging into melee when line-of-sight
is blocked and adds Stance 16 (Ranged Kiting) to the stance menu.

- Patches/RangedAI/manifest.toml: patch manifest (v1.0.0)
- Patches/RangedAI/kotor2-steam-aspyr.hooks.toml: stance & attack hooks
- Patches/RangedAI/RangedAI.cpp: kiting positioning, stance clamp expansion
- Patches/RangedAI/ranged_ai.ini: kiting distance thresholds
- Patches/RangedAI/README.md: combat stance mechanics and 2DA setup
```

### PR Description Body
```markdown
### Summary of Changes
Adds tactical kiting behavior for ranged characters and introduces party Stance 16 (`Ranged (Kiting)`) to the combat stance menu.

### Motivation & Background
In vanilla KOTOR II, companions equipped with blasters immediately break formation and run point-blank into melee combat if an obstacle or allied character momentarily blocks their line of sight. Furthermore, stance selection is artificially clamped to vanilla stances `{9, 11, 12, 13}`, preventing modders from adding new combat styles (**#72**).

### Technical Implementation
- **Line-of-Sight Hold (`0x006D83EA` - `RangedAiHold`):** When line of fire is obstructed, ranged companions hold position and acquire alternate targets instead of walking into melee range.
- **Dynamic Retargeting (`0x006D7C79` - `RangedAiRetarget`):** Emulates player retargeting behavior for ranged party members.
- **Stance Clamp Expansion (`0x0077AA5E` - `RangedAiStanceClamp`):** Detours `AddStanceActions` to lift the hardcoded clamp, allowing Stance 16 to be populated in the UI and selected from the action menu.

### Testing & Verification
- [x] Verified Mira, HK-47, and blaster-wielding companions kite aggressive melee enemies (Tusken Raiders, Kath Hounds).
- [x] Stance icon and name render correctly in HUD stance action bar.
- [x] Passes `python3 tools/validate-patches.py`.

Closes #72
Relates to #55
```
