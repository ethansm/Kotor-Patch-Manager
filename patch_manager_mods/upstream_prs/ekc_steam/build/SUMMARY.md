# EKC Steam port (I2) build summary
- Worktree: ~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-ekc, branch port/k2-expandedkeyboard-steam (uncommitted)
- Files: kotor2-steam-aspyr.hooks.toml (new), ExpandedKeyboardControl.cpp, manifest.toml (1.18.0 -> 1.19.0, + kotor2_steam_aspyr sha)
- validate-patches: 64 checked, 0 errors, 3 pre-existing warnings (K2AntiSkipPatch 0.1, PartyRosterLimit32K1/K2 0.0.1b version format) -> validate.log
- create-patch.py: OK -> create_patch.log; -Wall -Wextra: 0 warnings old, 0 new -> warnings_*.log
- script_check.log: GOG/K1 values identical, 6/6 Steam original_bytes match vanilla exe, fields equal GOG toml
- kpatch sha256: 7197ec83c6d59da203624e70ca39bce96ef1574767aba5ea6e9a3e512fd3d9be  ExpandedKeyboardControl.kpatch
- Not run in game/Wine.
