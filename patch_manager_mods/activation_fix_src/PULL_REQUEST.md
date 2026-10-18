# Pull Request: K2ActivationFix (upstream port)

Upstream branch: `fix/k2-activation-fix-steam` (worktree `~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-activation`,
commit 67b17c3 on `origin/master` 55725f7). NOT pushed. The upstream patch is a fresh, self-contained port
(`Patches/K2ActivationFix`: one detour, no local `_shared` headers, no config file, no log). The local
`activation_fix_src` (id `k2-activation-fix` 0.1.x) keeps its DllMain self-patch, KpLog and ini switch.

### PR Title
`K2ActivationFix: Fix load-screen hang after the intro movie`

### PR Description Body
```markdown
### Summary
Adds `K2ActivationFix` for KOTOR II Steam (Aspyr). Fixes the hang on the first load after the intro movie,
where the load screen stays up forever.

### Cause
At the end of the intro movie the movie thread posts a synthetic `WM_ACTIVATEAPP(0, 0)` to the main window
(`0x0040D69E`, `CALL [PostMessageA]`, args pushed at `0x0040D692..0x0040D69D`). Under Wine the movie window only
saw activation move to the game's own main window, so this "the game was deactivated" is false. The window
procedure then marks the engine inactive and minimizes it. When the window stays in front, no
`WM_ACTIVATEAPP(TRUE)` ever arrives, `MainLoop` never hides the load screen, and the game is stuck.

### Fix
One detour at `0x0040D69E`, run just before that CALL. It gets a pointer to the four pushed arguments. If the
message is `WM_ACTIVATEAPP(0, 0)` and the foreground window belongs to this process, it rewrites the call to
`PostMessageA(NULL, WM_NULL, 0, 0)`, which does nothing. Otherwise (the user really alt-tabbed during the movie) the
call is untouched. The window procedure and Alt+Enter handling are not changed.

### Testing
- [ ] PENDING: this exact build on Proton (see below). Do not tick until run.
- `python3 tools/validate-patches.py`: 65 patches checked, 0 errors (3 warnings, all in other patches).
- `python3 ../create-patch.py` builds `K2ActivationFix.kpatch` with MinGW; the DLL exports `dropSyntheticDeactivate`.
- Site bytes `FF 15 6C 64 98 00` checked against the Steam exe `6A522E71...`; KPM verifies them at install.
- The same mechanism, as a different implementation that dropped the post instead of neutralising it, was soaked
  locally on Proton: 9 launches, 0 hangs, 7 synthetic posts ignored, 8 of 8 first loads fine. Before the fix the
  hang happened about 1 launch in 8.

Not tested: GOG, Windows native, Linux native (hooks file is Steam Aspyr only).

Relates to: (issue numbers to be confirmed before posting)
```
