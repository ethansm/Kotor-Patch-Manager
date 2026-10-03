Title: `ExpandedKeyboardControl: Support the Steam Windows build`
Branch: `ethansm:port/k2-expandedkeyboard-steam` (c90fd4f). Open only after the user's in-game test on Steam.

### Problem
Expanded Keyboard Control supports K2 GOG only. Besides its hooks file, the C++ hard-codes K2 GOG addresses (GUI panel
vtables, the keyboard device index and eleven engine functions), so a Steam hooks file on its own would not work.

### Approach
- I found all six K2 detour sites in the Steam exe (`6A522E71...BBFFEF`):
  - Their `original_bytes` match exactly and are unique with context.
  - They read the same registers and frame slots as on GOG.
- The panel vtables were identified from RTTI, the functions by their content. Struct offsets and GUI event ids are
  the same on both builds.
- The K2 addresses move into two `GameConfig` tables, GOG and Steam. The inventory and journal vtables, previously
  standalone constants, join the struct.
- The module picks the table from `KOTOR_VERSION_SHA`, which KotorPatcher sets before loading patches
  (`src/KotorPatcher/src/core/patcher.cpp`). It does not open `addresses.db`. Any other build keeps the GOG table, so
  GOG and K1 behaviour is unchanged.

Files:
- `Patches/ExpandedKeyboardControl/kotor2-steam-aspyr.hooks.toml` (new): the six detours at 0x72CB63, 0x40C4A6,
  0x7AC470, 0x74B3F0, 0x74A750 and 0x72E0BC
- `Patches/ExpandedKeyboardControl/ExpandedKeyboardControl.cpp`: Steam `GameConfig`, `K2Config()` selector
- `Patches/ExpandedKeyboardControl/manifest.toml`: `kotor2_steam_aspyr` target, 1.19.0

### Testing
- [x] `python3 tools/validate-patches.py` (0 errors)
- [x] `create-patch.py` builds; no new `-Wall -Wextra` warnings
- [x] GOG/K1 config values identical to before (scripted comparison)
- [ ] Tested on Steam under Proton: TBD

🤖 Generated with [Claude Code](https://claude.com/claude-code)
