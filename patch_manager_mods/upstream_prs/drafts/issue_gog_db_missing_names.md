Title: `K2 address DBs: empty classes table breaks runscript; GOG DB missing names cut wrapper inits short`

(Report only. We propose no GOG DB edit unless the maintainer wants one. Lead: decide whether to split 1 into its own issue.)

### 1. `runscript` does nothing on K2 (GOG and Steam)
Since 840f515 ("Class Constructor dispatch"), `CExoString`'s allocating constructors use
`malloc(GameVersion::GetClassSize("CExoString"))`. The `classes` table has 0 rows in `kotor2_gog_aspyr.db` and
`kotor2_steam_aspyr.db` (K1 1.03 has 977, including `CExoString` = 8). With no row, `GetClassSize` throws, so
`classSize` stays -1 and `CExoString(const char*)` leaves `objectPtr` null. AdditionalConsoleCommands' `runscript`
(`ConsoleCommands.cpp`, `CExoString scriptFile(script)`) then passes a null string to `RunScript`, and nothing runs.

The real size on K2 is 8 (`char*` + length; the Steam ctor at 0x733540 writes two dwords). Fix options:
- add `classes` rows to the K2 DBs (`CExoString` 8; `ConsoleFunc` 84 like K1);
- or fall back to 8 in `CExoString.cpp` when the lookup fails.

### 2. The GOG DB lacks names that GameAPI inits look up
Each wrapper class resolves its names in one `try`, so the first missing name skips every later lookup in that class.
The affected methods null-check, so they do nothing rather than crash. Examples from a simulation of every wrapper
init against `kotor2_gog_aspyr.db` (378 missing lookups in total):

| Missing | Effect |
| --- | --- |
| `CClientExoApp::IsValidResolution` (only `CAuroraInterface::IsValidResolution` exists) | skips 33 later names, among them `SetVideoMode`, `GetInGameGui`, `ExitProgram` |
| `CClientExoApp::GetLoadingSaveGame`, `SetRunScript` | behind the miss above |
| `CSWSObject::SetTag` (only `CSWSCreatureStats::SetTag` exists) | skips `GetPortrait` ... `StopSoundPlayingInDialog` |
| `CClientExoAppInternal` offsets (11; the class has no rows) | every internal offset unavailable |
| `CSWSObject` offsets from `tag` on (20) | only `Position`/`Orientation`/`AreaId` usable |
| `CSWSCreature` offsets from `joining_xp` on (35) | only `CreatureStats`/`Inventory` usable |

In AdditionalConsoleCommands, `teleport`, `freecam`, `addfeat` and the render toggles resolve their names before these
misses, so they still work on GOG. Other patches using the classes above would silently lose functionality.

Method: a script that walks each wrapper's `InitializeFunctions`/`InitializeOffsets` lookups in source order against
the DB (read-only).

🤖 Generated with [Claude Code](https://claude.com/claude-code)
