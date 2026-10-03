Title: `AdditionalConsoleCommands: Support the Steam Windows build`
Branch: `ethansm:port/k2-acc-steam` (acdfeeb). Open only after the Steam DB PR is open (link it below).

### Problem
Additional Console Commands has no Steam Windows target. `manifest.toml` lists the GOG K2 build only, so KPM reports a Steam
install (SHA-256 `6A522E71...BBFFEF`) as unsupported, and there is no hooks file for the Steam binary.

### Approach
Both hook sites exist in the Steam binary with the same shape as the GOG ones, and `ConsoleCommands.cpp` has no version- or
build-specific logic (every game address comes from `addresses.db` through `GameVersion`/GameAPI), so the existing
`windows_x86` module serves both builds unchanged. This adds the Steam hooks and the manifest target, nothing else.

- `Patches/AdditionalConsoleCommands/kotor2-steam-aspyr.hooks.toml` (new)
  - detour `InitializeAdditionalCommands` at `0x0091D52D`: the `PUSH 0x400000` before the WinMain call in `___tmainCRTStartup`,
    after `__cinit`, mirroring GOG `0x00925133`
  - "Set Console History to save" at `0x004752D8`: `MOVZX EDX, byte [0xA30828]` replaced by `MOV EDX,1`, mirroring GOG `0x00867248`
- `Patches/AdditionalConsoleCommands/manifest.toml`: add the `kotor2_steam_aspyr` target, bump to 1.5.0

### Dependency
Depends on the Steam address-DB PR (TBD: link when opened), which replaces the 48-row `kotor2_steam_aspyr.db`. With the current DB
`teleport`, `freecam` and the render toggles already resolve, but `addfeat` would call the wrong function: the 48-row DB maps
`CSWSCreatureStats::AddFeat` to `0x006F5B40` (a level-up helper, not the creature-stats method); the DB PR corrects it to `0x006B8280`.

### Known issue, not part of this PR
`runscript` does nothing on any K2 build (GOG included): `CExoString` allocation needs a class size from the `classes` table,
which is empty in both K2 databases since the `GetClassSize` change in 840f515. Tracked separately.

### Testing
- [x] `python3 tools/validate-patches.py` (0 errors)
- [x] Hook `original_bytes` match the vanilla Steam executable (SHA-256 `6A522E71...BBFFEF`) at both addresses
- [ ] Tested on Steam under Proton: TBD
- [x] GOG and K1 hook files left untouched

🤖 Generated with [Claude Code](https://claude.com/claude-code)
