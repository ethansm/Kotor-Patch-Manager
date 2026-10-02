# Pull Request: Buff Duration HUD

### PR Title
`feat(hud): party buff/debuff timers and portrait shield absorb bars (#104)`

### Commit Message
```text
feat(hud): add buff duration countdowns and portrait shield bars

Adds visible countdown/drain overlays to party portrait status icons
and renders an energy shield absorption bar beside vitality bars.

- Patches/BuffDurationHUD/manifest.toml: patch manifest (v1.1.0)
- Patches/BuffDurationHUD/kotor2-steam-aspyr.hooks.toml: hook definitions
- Patches/BuffDurationHUD/BuffDurationHUD.cpp: icon column rendering,
  duration calculations, shield bar geometry and tooltips
- Patches/BuffDurationHUD/README.md: full documentation and INI options
```

### PR Description Body
```markdown
### Summary of Changes
Provides visible countdown timers and circular radial wipes on active party buff/debuff icons, plus a dedicated energy shield absorption bar beside party portraits.

### Motivation & Background
Players cannot easily tell when Force buffs (Speed, Valor, Aura) or energy shields are expiring in vanilla KOTOR II without pausing and opening the character sheet. This patch exposes real-time duration and shield integrity directly on the HUD.

### Technical Implementation
- **Status Icon Overlays:** Hooks portrait status bar rendering to read `CGameEffect` remaining durations, drawing a translucent radial wipe and second countdown over active icons.
- **Shield Absorption Bar:** Hooks `CSWGuiMainInterface::UpdatePortraits` to calculate cumulative active shield HP from creature effect lists and draws a proportional shield bar stacked beside health.
- **Tooltip Integration:** Updates portrait tooltip readouts to include current shield absorption points remaining.

### Testing & Verification
- [x] Verified against single-target and party-wide buffs (Force Speed, Force Armor, Battle Meditation).
- [x] Verified debuffs (Poison, Stun, Horror) render in distinct red/orange indicator columns.
- [x] Shield bar updates accurately as damage is absorbed and drops cleanly when depleted.
- [x] Passes `python3 tools/validate-patches.py`.

Relates to #104
```
