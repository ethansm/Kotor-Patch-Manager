# Pull Request: Companion AI Suite

### PR Title
`feat(ai): companion combat fixes, attack requeue, and perception throttling (#55, #72)`

### Commit Message
```text
feat(ai): companion combat AI engine fixes and perception throttle

Resolves companion combat freezing, follow-queue stalls, stationary
attack requeueing, and throttles perception checks for smoother combat.

- Patches/CompanionAI/manifest.toml: patch manifest (v1.0.0)
- Patches/CompanionAI/kotor2-steam-aspyr.hooks.toml: hook definitions
- Patches/CompanionAI/CompanionAI.cpp: combat queue hooks and decision tree
- Patches/CompanionAI/CompanionAiLogic.h: isolated logic decision headers
- Patches/CompanionAI/companion_ai.ini: configurable AI behavior flags
- Patches/CompanionAI/README.md: full issue breakdown and mechanics
```

### PR Description Body
```markdown
### Summary of Changes
Comprehensive companion AI overhaul fixing combat stalls, follow-queue pathing bumps, attack requeue loops, and perception update lag.

### Motivation & Background
Vanilla KOTOR II companion AI frequently bugs out in combat: party members get stuck in attack loops, ignore enemy target switches, fail to receive bonus attacks during Flurry (**#55**), or stutter when following the player through doorway transitions.

### Technical Implementation
- **Attack Action Self-Requeue (`0x006D740F`):** Clears stagnant fallback combat queue entries to ensure companions execute queued abilities rather than idling.
- **Follow-Queue Bump (`0x00586EB3`):** Resets stalled pathing waypoints when party members get stuck against doorways or geometry.
- **Perception Throttle (`0x0056BD50`):** Replaces per-frame line-of-sight polling with an INI-configurable interval (default 100 ms), drastically improving CPU pacing in areas with many combatants.
- **Null Spell Row Guard:** Prevents game crashes when companions query uninitialized spell table rows.

### Testing & Verification
- [x] In-game combat testing across Peragus, Telos surface, and Dxun jungle battles.
- [x] Verified companions switch targets cleanly when current target dies.
- [x] 286 automated unit checks in `CompanionAiLogic_test.cpp`.
- [x] Passes `python3 tools/validate-patches.py`.

Relates to #55, #72
```
