# GOG-only KPM patches: Steam (sha 6A522E71...) port triage (task R3, read-only)

Date 2026-10-03. Upstream source: clean export of `upstream/master` (8b33840). Steam exe: vanilla `swkotor2_steam_vanilla.exe`
(image base 0x400000, `.text` 0x401000-0x985648). Branch Steam DB: `addressdb/k2-steam-acc` @ 4324098 (1,683 functions).
No port work was done; nothing outside this directory was written.

## Result in one table

| Patch | Hooks needing Steam VAs | Verbatim bytes found, function-relative | Needs DB at runtime? | Effort | Order |
| --- | --- | --- | --- | --- | --- |
| K2ClassMenuClothes | 4 (all detours) | 4/4 | No (7 hard-coded fn addrs) | S | 1 |
| Movie Patch (K2 part only) | 2 (detours) | 2/2 (one with the IAT operand changed) | No (2 IAT slots, 2 globals) | S (code), M (live test) | 2 |
| PartyRosterLimit32K2 | 211 (168 simple, 43 detours) | 211/211 | No (43 fn addrs, 3 data addrs) | L | 3 |

Every hook of all three patches resolves on Steam. None of them is at the same VA as on GOG (0 of 217), so every address
has to be re-derived; the bytes themselves are mostly unchanged. The Steam exe is a different build layout (for example
GUI classes moved from 0x58xxxx to 0x89xxxx-0x8fxxxx, `CSWPartyTable` from 0x6fxxxx-0x70xxxx to 0x5f7xxx-0x5ffxxx), so
there is no global delta, only per-class deltas.

## Method (scripts and raw data in this directory)

- `byte_check.py` -> `byte_check.tsv`: for each hook, exact occurrence count of `original_bytes` in Steam `.text`, whether
  it matches at the identical VA (never), and a masked count (rel32/abs32 operands wildcarded using `objdump` decoding).
  Whole-exe exact search alone is not decisive: 185 of 211 PartyRoster hooks are <8 bytes (168 are 1 or 4 byte immediates),
  so 177 are "multiple".
- `fn_delta_check.py` -> `fn_delta_check.tsv`: GOG hook address -> containing GOG function (`inputs/gog_functions.tsv`) ->
  Steam function -> expected Steam VA (same offset in function) -> compare `original_bytes` there. Function mapping =
  `gog_cluster_delta_map.py` votes (`gog_to_steam_fn_map.tsv`, 2,035 rows), overridden by `inputs/name_map.tsv` rows with
  confidence high/medium matched on `Class::Function`. Two hooks were resolved by hand (see notes). Arbiter is the byte match.
- `addr_map.py` -> `addr_map.tsv`: every hard-coded address in the C++ -> GOG name -> Steam address -> presence in the GOG
  DB and branch Steam DB, with a sanity column (16-byte alignment and `ret N` vs the GOG parameter size).

Caveat on evidence strength: 41 of the 168 PartyRoster simple hooks and 3 detours have only 1 fixed byte (for example a
`cmp ..., 0Ch` immediate or `E8 xx xx xx xx`). For those, "match" means the right byte sits at the right function-relative
offset; they need a one-time read of the surrounding instructions (the decomp/asm dump has them) before shipping.

## 1. K2ClassMenuClothes

How it targets GOG: `hooks.toml` with `target_versions = [777BEE23...]`, manifest `supported_versions.kotor2_gog_aspyr`.
C++ (`ClassMenuModels.cpp`, 190 lines, no GameAPI) hard-codes 7 engine function addresses as `constexpr` and calls them
through function pointers; no DB lookup anywhere. Ships `additional/classmenu.2da`.
Branch `upstream/patch_add/k2classmenuclothes`: a single commit 3a5de1e (J, 2026-09-27) whose 5 files are byte-identical to
master's `Patches/K2ClassMenuClothes` (checked with diff/cmp). It does not change targeting; it is still GOG-only.

| # | GOG VA | Hook | len | Steam VA (fn + off) | Exact hits in .text | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 0x5DE18A | BeginClassMenuAppearance | 10 | 0x8F9DCA (`CSWGuiClassSelection::Constructor` 0x8F92B0 + 0xB1A) | 1 (same place) | verbatim-unique |
| 1 | 0x48E574 | OverrideClassMenuModel | 7 | 0x85E4E4 (`CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` 0x85E320 + 0x1C4) | 3 | multiple; matches at the function-relative VA |
| 2 | 0x48E691 | OverrideClassMenuTexture | 7 | 0x85E601 (same fn + 0x2E1) | 32 | multiple; matches at the function-relative VA |
| 3 | 0x5DE199 | EndClassMenuAppearance | 10 | 0x8F9DD9 (ctor + 0xB29) | 1 (same place) | verbatim-unique |

Hard-coded function addresses (all code, no vtables or data globals):

| GOG | Name | Steam | Steam DB (branch) |
| --- | --- | --- | --- |
| 0x4EAED0 | CExoString default ctor | 0x733540 | yes (`DefaultConstructor`) |
| 0x605680 | CExoString(char*) ctor | 0x733570 | yes (`CStrConstructor`) |
| 0x605890 | CExoString dtor | 0x733780 | yes |
| 0x6207E0 | C2DA ctor (ResRef by value, int) | 0x71AD40 (`ret 20`, calls 0x733540; mapper missed it, found by neighbour delta of 0x71AE80) | no |
| 0x620920 | C2DA dtor | 0x71AE80 | no |
| 0x621E70 | C2DA::Load2DArray | 0x71C3D0 | no |
| 0x620C70 | C2DA::GetCExoStringEntry (ExoString col) | 0x71B1D0 (`ret 12`) | no |

All Steam candidates are 16-byte aligned with `ret N` equal to the GOG parameter bytes. Steam DB rows missing: 4 C2DA rows
(only needed if the port goes through DB lookups; the current code does not).
Effort **S**: 4 hooks, 7 addresses, no struct-layout dependence beyond the frame offsets that the verbatim bytes already
pin (`[ebp-0x34]` slot, resolver args +0x0C/+0x10). Work items: a Steam address table (exports with a Steam suffix, or
runtime selection), `kotor2-steam-aspyr.hooks.toml`, manifest `kotor2_steam_aspyr`, in-game class-menu check.
Gate: the 7 C2DA/CExoString addresses need the usual model verification before any DB row is added.

## 2. Movie Patch (K2 half)

How it targets GOG: manifest lists K1 GOG, K1 CD-crack and `kotor2_gog_aspyr`. `hooks.toml` has `target_versions` = the two K1
shas only (9 hooks, K1; not portable). K2 uses `kotor2_hooks.toml` (`target_versions` = the K2 GOG sha): 2 detours, no simple
hooks. So K2 gets only the Mitchell-Netravali bicubic blit, not the in-window or aspect hooks. C++ hard-codes, for K2, a
`MovieFilterAddresses` struct and `Kotor2MoviePointerFrameOffset = 0xE0`; no DB.

| # | GOG VA | Hook | len | Steam VA | Exact hits | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 0x920AF1 | blitMovieMitchellNetravaliKotor2 | 30 | 0x40D901 (`Movie::PlayFrames` 0x40D6F0 + 0x211) | 0 verbatim; 1 masked | same code; only the trailing `FF 15 <IAT>` operand differs: Steam `FF 15 A0 64 98 00` (BinkBufferBlit) |
| 1 | 0x91FD10 | releaseMovieFilterKotor2 | 9 | 0x40CB20 (`CExoMoviePlayerInternal::ShutDown`) | 13 | multiple; verbatim at the function-relative VA (prologue `55 8B EC 81 EC 84 00 00 00`) |

Hard-coded data (Steam equivalents located; none are in the Steam DB, which has no IAT or video globals):

| GOG | Meaning | Steam | Evidence |
| --- | --- | --- | --- |
| 0x9854BC | IAT slot `_BinkBufferBlit@12` | 0x9864A0 | imports.tsv `CALL [0x009864A0]` at 0x40D919 |
| 0x9854C8 | IAT slot `_BinkCopyToBuffer@28` | 0x986494 | imports.tsv `CALL [0x00986494]` at 0x40D8CE |
| 0x9F22A4 / 0x9F22A8 | screen width / height | 0x9F42A4 / 0x9F42A8 | repo `db_functions.tsv` (PushClipViewport notes) |
| frame `[ebp-0xE0]`, movie +0x48 (Bink), +0x4C (buffer) | struct offsets | unchanged | Steam asm `8B 85 20 FF FF FF` and decomp `param_1+0x48/+0x4C` |

Effort **S** for code (two exports plus a Steam `MovieFilterAddresses`, one hooks file), but the real cost is the live test:
it is an OpenGL filter running on the movie thread. Interactions to check: the live `k2-activation-fix` hooks the movie
thread's `PostMessageA(WM_ACTIVATEAPP)` at 0x40D69E (not the same bytes, but same area), and the Steam movie loop must
tolerate `prepareMovieContext`/`releaseMovieContext` as GOG does. The K1-only parts (`postMovieCleanup` ActivateRenderWindow
0x401E00, aspect scale, 640x480 override) do not apply and should stay out of the port.

## 3. PartyRosterLimit32K2 (Vriff, version 0.0.1b)

How it targets GOG: `kotor2-gog-aspyr.hooks.toml` (1,525 lines, `target_versions` = K2 GOG sha), manifest
`supported_versions.kotor2_gog_aspyr`, `conflicts = ["full-party-select"]`, ships `Additional/partyselect_p.gui`. C++
(`PartyRosterLimit32.cpp`, 1,326 lines) includes `Common.h` and `GameAPI/CResRef.h` only (CResRef is a local 16-byte buffer
wrapper with no DB-resolved functions). All engine calls are `GameFunction<...>(ADDRESS_x)` with 43 `constexpr` GOG
addresses; no runtime DB lookup. Hard-coded struct layouts: `CSWPartyTable` (+0x1C ids, +0x4C avail, +0x7C selectable,
+0x114/+0x118/+0x254), party-select panel (size 0x50B4, stride 0x478, tail offsets), creature 0x1220, CResGFF 0xA0,
CGameEffect 0x8C.

Hooks: 211 = 168 `simple` (127 four-byte immediates, 41 one-byte, 1 five-byte call NOP) + 43 `detour` (3 with fewer than
2 fixed bytes). Exact search: 28 verbatim-unique, 177 multiple, 6 absent (detours whose bytes contain a GOG absolute
operand). Function-relative check: **211/211 match**. Containing GOG functions (all mapped to Steam):

| GOG class | Hooks (simple + detour) | Steam VAs | Notes |
| --- | --- | --- | --- |
| `CSWGuiPartySelection` (GOG 0x58AA40+) | 140 + 6 | 0x89CF9D-0x8A0947 | slot/stride/panel-size immediates all at the expected offsets, including the `0x50B4` panel size |
| `CSWPartyTable` (GOG 0x6FD210+) | 13 + 34 | 0x5F76A0-0x5FF3FC | the 13-23 byte verbatim detour patterns live here |
| `CSWGuiInGameEquip` | 5 + 2 | 0x8AAF18-0x8AB2EB | next/prev NPC clicked |
| `CGuiInGame` | 3 + 0 | 0x7BF814-0x7D096C | allocation sizes |
| `CSWSCreatureStats` (GetAttackModifierVersus, ModifyAlignment, AutoLevelup) | 4 + 0 | 0x6AB180-0x6C015C | one-byte slot-count immediates |
| `CSWGuiUpgradeSelection` / `CSWGuiUpgradeItemSelect` | 2 + 0 | 0x8C76C8, 0x8C9173 | |
| `CSWSMessage::HandlePlayerToServerLevelUpMessage` (Steam 0x661C30) | 1 + 0 | 0x661FE2 (`cmp [ebp-0x34],0Ch` at 0x661FDF) | resolved by hand; mapper does not cover it |
| `ExecuteCommandRebuildPartyTable` (Steam 0x69C460) | 0 + 1 | 0x69C97F | resolved by hand |

Detours whose whole pattern is verbatim-unique in `.text` and sit exactly at the function-relative VA (strongest evidence,
17 hooks): 0x5F92D2, 0x5F9958, 0x5F99C2, 0x5FAA97, 0x5FB187, 0x8AB161, 0x5FE13C, 0x5FE239, 0x5FEFB9, 0x5FEE00, 0x5F8E18,
0x5F8E3F, 0x5FA93B, 0x5FD9D3, 0x5FD9FB, 0x5FF3FC, 0x5FE5C0.

Mapper pitfall found: the vote for `CSWPartyTable::UnstealthParty` was 0x5FED60
(actually `GetPlanetAvailable`); the verified Steam row is 0x5FEF20 and that is where the bytes match (offset 0x99 as in
GOG). Treat any mapper-only function row as a hypothesis until bytes match.

Hard-coded function addresses (43): 24 are already in the branch Steam DB; 16 are mapped but have no DB row; 3 are unresolved.

| GOG | Name | Steam | Status |
| --- | --- | --- | --- |
| 0x64C2F0 | party-table accessor (GOG DB label `CTwoDimArrays::GetSwoopUpgradeRowProvider` is wrong) | 0x51C8D0 (body `this[+4]+0x1F0B4`) | mapped by hand; name_map marks the GOG label "wrong" |
| 0x40D3A0, 0x4CE630, 0x64BCA0, 0x64DB40, 0x67AB60, 0x683A70, 0x68B5A0, 0x68D030, 0x6A34D0, 0x6AAEF0, 0x6DB8D0, 0x7E4850, 0x916990, 0x9184B0 | CClientExoApp::ServerToClientObjectId, CSWGuiButton dtor, CServerExoApp::GetServerAIMaster, CServerAIMaster::SetAILevel, CSWSCreature ctor/HandleAlignmentInfluence/LoadFromTemplate/SaveCreature/ReceiveExperience/GetFaction, CSWSObject::ApplyEffect, CSWSFaction::AddMember, CSWGuiNavigable::SetMoveToControl, CSWGuiButton::SetEnabled | 0x73FEC0, 0x41A3D0, 0x51C280, 0x51E120, 0x561F30, 0x56AE40, 0x572A70, 0x574500, 0x58A9F0, 0x544210, 0x5D0CB0, 0x6D3330, 0x4186F0, 0x41A210 | mapped, no branch-DB row |
| 0x921347 | operator new | 0x919723 | CONTRACT.md / alloc_sites (not 16-aligned, as in GOG) |
| 0x512A40 | CSWGuiButton ctor | hypothesis 0x41A510 (`ret 0`, called right after `new(0x1D0)` at 0x8BFE7A, 0x7D8945, ...) | needs RE confirmation |
| 0x58A930 | CSWGuiPartySelectionButton::SetSelected | unresolved | needs RE |
| 0x903DF0 | CExoArrayList<ptr>::DelIndex (template instance) | unresolved | needs RE |

Data addresses: `APP_MANAGER_PTR` 0xA11C04 -> 0xA1B4A4 (in branch DB `global_pointers`); effect type/subtype words
0x99C95C and 0x99C8B8 (read as `WORD`, GOG `.rdata`) have no Steam equivalent found yet. Vtable usage is by index
(button SetActive slot 0x40, object-status slot), which needs a separate check on Steam.

Effort **L**: 211 edits plus 43 detours, 3 functions and 2 data words still to locate, ~19 DB rows missing if the port goes
through the DB, struct layouts only spot-verified (panel alloc `0x50B4`, creature `0x1220` and `[ecx+0x7C]` agree on Steam),
the 44 one-byte hooks need context reads, `Additional/partyselect_p.gui` must be compared with the Steam copy, and the
C++ needs address indirection across ~43 call sites plus 43 exported detours (a per-version table, since both builds
would share one DLL). The patch is upstream beta, rewrites save data ("PRL4" extension), and conflicts with full-party-select,
so it needs the heaviest in-game test (recruit 13+, save/load, level-up, equipment next/prev, kill/unstealth paths).

## Cross-cutting design point (affects all three)

None of the three uses DB lookups, so the existing Steam address DB is not a blocker; the blocker is that each DLL has
hard-coded GOG constants. Movie Patch already shows the pattern (a per-game address struct plus separately named exports,
e.g. `blitMovieMitchellNetravaliKotor2`); K2 Steam hooks files can point at Steam-suffixed exports that call the shared body
with a Steam table. For PartyRoster, a runtime table (selected once, for example by checking a byte at a known Steam
address) is simpler than 43 duplicated exports. Naming: new file `kotor2-steam-aspyr.hooks.toml` (any `*hooks.toml` is
picked by `target_versions`), manifest key `kotor2_steam_aspyr = "6A522E71..."`, as in PRs #292-294.

## Recommendation (port order)

1. **K2ClassMenuClothes** (S): smallest, self-contained, 4 hooks with verified bytes, no layout risk, easy visual test
   (new-game class menu). Good second Steam-port PR after #292-294; the branch `patch_add/k2classmenuclothes` needs no
   rebase since it matches master.
2. **Movie Patch, K2 filter only** (S code, M test): 2 hooks and 4 constants, all located. Keep the K1 parts untouched; do
   a short movie-playback test (intro logos, a cutscene, skip, alt-tab) alongside `k2-activation-fix`.
3. **PartyRosterLimit32K2** (L): do last, as a multi-step effort: resolve the 3 functions and 2 data words with model
   verification, add the missing DB rows through the usual batch gate, review the 44 one-byte hooks in context, then port.

Raw outputs: `byte_check.tsv` (217 hooks), `fn_delta_check.tsv` (hook -> Steam VA, verdict, informative bytes),
`addr_map.tsv` (64 hard-coded addresses), `gog_to_steam_fn_map.tsv` (mapper rows used).
