# Paragraph for the Steam address-DB PR description (hand to the steam_db_batches session / user; do not open the PR from here)

**Notes for review**
- All rows were verified against the Steam code (content anchors: strings, confirmed callees, field offsets) and
  checked against decoys; the import CSV is `<path in branch>` (fill in).
- `CSWSCreatureStats::AddFeat`: the previous 48-row DB pointed at `0x006F5B40`, a level-up helper. This DB uses
  `0x006B8280`. That fixes AdditionalConsoleCommands' `addfeat` on Steam.
- While checking signatures we found seven GameAPI wrappers whose typedefs disagree with the engine on every build.
  They are fixed separately in #<GameAPI PR>, and the K2-only mismatches are listed there. DB `param_size` errata
  found on the way (GOG/K1, not changed here): GOG `ShowUpgradeScreen` 20 -> 24, `ActionMenuSecurity` GOG 0 / K1 4 -> 8,
  K1 `DisplayMainMenu` 0 -> 4.
- Open decision: add `classes` rows (`CExoString` 8, `ConsoleFunc` 84) so `runscript` works on Steam (see issue
  #<issue>). The `classes` table is empty in both K2 DBs.
