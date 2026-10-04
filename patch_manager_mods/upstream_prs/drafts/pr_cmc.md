Title: `K2ClassMenuClothes: Support the Steam Windows build`
Branch: `ethansm:port/k2-classmenuclothes-steam` (746265f, on master 6374f73). Open only after an in-game test on Steam.

### Problem
K2 Class Menu Clothes supports K2 GOG only. Besides its hooks file, the C++ hard-codes seven K2 GOG function
addresses (the C2DA constructor, destructor, Load and GetString, and three CExoString functions), so a Steam hooks
file on its own would not work.

### Approach
- I found all four detour sites in the Steam exe (`6A522E71...BBFFEF`):
  - Their `original_bytes` are the same as on GOG, and they are unique in `.text` with 8 bytes of context.
  - They read the same frame slots as on GOG (the class-slot loop counter at `[ebp-0x34]`, and the model and
    texture ResRef outputs at `[ebp+0xC]` and `[ebp+0x10]`).
- The seven functions were identified by content, not by name: the 2DA loaders that push the same 2DA name string
  (`CLASSES`, `Appearance`, ...) call the same constructor, Load and destructor sequence on both builds, and the
  CExoString functions match the existing Steam address database rows.
- The addresses move into a GOG and a Steam table. The module picks one from `KOTOR_VERSION_SHA`, which KotorPatcher
  sets before loading patches. Any other build keeps the GOG table, so GOG behaviour is unchanged.

Files:
- `Patches/K2ClassMenuClothes/kotor2-steam-aspyr.hooks.toml` (new): the four detours at 0x8F9DCA, 0x85E4E4,
  0x85E601 and 0x8F9DD9
- `Patches/K2ClassMenuClothes/ClassMenuModels.cpp`: GOG and Steam address tables and the selector
- `Patches/K2ClassMenuClothes/manifest.toml`: `kotor2_steam_aspyr` target, 1.1.0

### Testing
- [x] `python3 tools/validate-patches.py` (0 errors)
- [x] `create-patch.py` builds; no new `-Wall -Wextra` warnings
- [x] GOG `hooks.toml` and GOG addresses unchanged
- [ ] Tested on Steam under Proton: TBD

🤖 Generated with [Claude Code](https://claude.com/claude-code)
