# CMC Steam port (I-1) build summary
- Worktree: ~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-cmc, branch port/k2-classmenuclothes-steam (uncommitted)
- Files: kotor2-steam-aspyr.hooks.toml (new, = draft), ClassMenuModels.cpp (NativeAddresses GOG/Steam tables, KOTOR_VERSION_SHA select), manifest.toml (1.0.0 -> 1.1.0, + kotor2_steam_aspyr sha); GOG hooks.toml byte-identical
- validate-patches: 64 checked, 0 errors, 4 pre-existing version-format warnings (none for this patch) -> validate.log
- create-patch.py: OK -> create_patch.log; -Wall -Wextra: 0 warnings old, 0 new -> warnings_*.log; junk removed -> removed_junk.log
- verify_steam_hooks: 17 rows, PASS=17 FAIL=0 -> verify.tsv
- kpatch sha256: 123d3c3fe0a115eebcee236e86c45e4f9e594855d7e12e6c1eed0df8571346fd  K2ClassMenuClothes.kpatch
- Not run in game/Wine.
