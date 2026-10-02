# Pull Request: Activation Fix

### PR Title
`fix(engine): drop synthetic window deactivation after intro movies (#91, #49)`

### Commit Message
```text
fix(engine): prevent post-movie load-screen hang

Drops spurious WM_ACTIVATEAPP(0,0) messages posted at movie completion
while the game window remains in the foreground, eliminating load hangs.

- Patches/ActivationFix/manifest.toml: patch manifest (v1.0.0)
- Patches/ActivationFix/kotor2-steam-aspyr.hooks.toml: hook definition
- Patches/ActivationFix/ActivationFix.cpp: post-message hook and foreground
  window check
- Patches/ActivationFix/README.md: technical explanation of movie-thread hang
```

### PR Description Body
```markdown
### Summary of Changes
Fixes the intermittent hang where KOTOR II gets permanently stuck on a black screen or load screen following video playback.

### Motivation & Background
When a full-screen movie finishes, the engine's playback thread issues a synthetic `WM_ACTIVATEAPP(FALSE, 0)` message to the main game window (`0x0040D69E`). Under Proton/Wine and modern Windows desktop window managers, this synthetic deactivation tricks the engine into believing it was minimized. The main thread pauses rendering and stops advancing scene loading, causing the load screen to hang indefinitely. This resolves **#91 (K2 Steam Won't Launch)**.

### Technical Implementation
- **Hook Site (`0x0040D69E`):** Intercepts the `CALL [PostMessageA]` in the movie completion handler.
- **Foreground Gate:** Checks `GetForegroundWindow()`. If the active foreground window belongs to the game process, the synthetic deactivation message is dropped. Genuine user Alt-Tabs, Win-key presses, and Alt+Enter toggles continue to function normally.

### Testing & Verification
- [x] Tested across 50 consecutive game loads, module transitions, and intro movie playbacks on Proton/Linux.
- [x] Zero load-screen hangs observed.
- [x] Genuine Alt-Tab minimize/restore behavior verified intact.
- [x] Passes `python3 tools/validate-patches.py`.

Closes #91
Relates to #49
```
