# Movie Patch Steam port (I2) build summary
- Worktree: ~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-movie, branch port/movie-patch-k2-steam (uncommitted)
- Files: kotor2-steam-aspyr.hooks.toml (new), better_movie_playback.h, export.cpp (K2 export only, sha selector), manifest.toml (1.5.3 -> 1.6.0, + kotor2_steam_aspyr sha)
- validate-patches: 64 checked, 0 errors, 4 pre-existing version-format warnings -> validate.log
- create-patch.py: OK -> create_patch.log; -Wall -Wextra: 33 warnings old, 33 new, sets identical (pre-existing) -> warnings_*.log
- verify_steam_hooks: PASS=8 FAIL=0 WARN=1 (hook 1 UNIQUE, allowlisted) -> verify.tsv; K1 hooks.toml and kotor2_hooks.toml byte-identical
- kpatch sha256: 0fc1f4fae56107600621ba98868acc3b334fc31210101136e1f19b4ad324e651  Movie Patch.kpatch
- Not run in game/Wine.
