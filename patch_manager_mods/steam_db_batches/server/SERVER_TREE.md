# OpenKotOR Ghidra server inventory (read-only)

Server `ghidra.openkotor.com:13100`, repository `Odyssey`, listed 2026-10-03 with
`python3 scripts/ghidra_server_api.py list-tree <folder>` (user Ethansm, vault `authenticated_ready: true`).
Nothing is ever written back. Exports land in `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/<program>/`
(not in the repo).

## Tree (program, server version)
| folder | programs |
|---|---|
| `/TSL` | `k2_win_gog_aspyr_swkotor2.exe` v18, `k2_win_steam_aspyr_swkotor2.exe` v5, `k2_linux_swkotor2.elf` v14, `k2_mac_KOTOR2_x86` v4, `k2_mac_KOTOR2_x64` v3, `k2_win_CD_1.0_swkotor2.exe` v2, `k2_win_CD_1.0b_swkotor2.exe` v3, `k2_xbox_default.xbe` v2, `k2_ios_KOTOR_II.ipa` v2, `k2_android_libkotor2.so_{arm64-v8a,armeabi-v7a}android` v2, `k2_android_libkotor2.so_{x86_64,x86}android` v3 |
| `/K1` | `k1.0.0/1.0.1/1.0.2_win_CD_swkotor.exe` v1, `k1_win_gog_swkotor.exe` v144, `k1_win_amazongames_swkotor.exe` v2, `k1_win_collection_swkotor.exe` v1, `k1_mac_aspyr_swkotor.app_x64` v6, `k1_mac_swkotor.app` v7, `k1_iOS_KOTOR.ipa` v2, `k1_android_ARM64` v1, `k1_android_ARMEABI` v3, `k1_xbox_default.xbe` v1 |
| `/Libraries` | `Mss32.dll` v12, `Patchw32.dll` v2, `binkw32.dll` v10; subfolders `Miles61a/win`, `Miles65h/win`, `Miles93u/{linux,win}` |
| `/Miscellaneous Programs` | `mss32midi.dll`, `swconfig.exe`, `swstub.exe`, `swupdate.exe` (v1) |
| `/Other BioWare Engines` | `Aurora/nwmain.exe` v1, `Eclipse/DragonAge2.exe` v1, `Eclipse/daorigins.exe` v1 |
| `/JE` | `JadeEmpire.exe` v1 |

## Programs exported for the Steam address DB
| program | why | exported |
|---|---|---|
| `k2_win_gog_aspyr_swkotor2.exe` | the GOG names (source of KPM's GOG DB) + GOG bodies for fingerprints | symbols, fp (26,975 fns, 6,396 strings), types (479 types / 4,476 rows) |
| `k2_win_steam_aspyr_swkotor2.exe` | our target; few user names, but the same analysis base for fingerprints | symbols, fp (26,965 fns, 16,996 strings), types (308 / 886) |
| `k2_linux_swkotor2.elf` | independent names (`Class__Method`, 4,952 USER_DEFINED + 406 IMPORTED) | symbols, types (1,052 rows) |
| `k2_mac_KOTOR2_x86` | independent names from the Mach-O symbol table (~13k demangled, tagged ANALYSIS, e.g. `CSWGuiStore::ShowBuyGUI`) | symbols, fp (15,639 fns, 72,455 strings) |
| `k2_mac_KOTOR2_x64` | same names as x86 (redundant) | symbols |

How: `python3 scripts/ghidra_server_api.py export <program> --folder /TSL --out <dir>` (symbols),
`export-script <program> ExportFingerprints.java --out <dir>`, `export-types <program> --out <dir>`; all `-readOnly
-noanalysis` (the account has no write permission anyway: headless logs "commit not allowed"). Script copies:
`server/ghidra_scripts/` (live copies in the KPM clone `tools/ghidra_scripts/`). Fingerprint check vs our local dump:
callee sets equal for 24,328 / 24,839 functions; string refs equal or superset for 1,314 / 1,315.

## Knowledge-DB ingest (2026-10-03, `tools/ingest_server.py --functions trusted --apply`)
Tables `server_functions` (trusted names only: GOG 10,423, Steam 25, Linux 5,358), `server_symbols` (labels: GOG 3,542,
Steam 3,227, Linux 179), `server_types` (GOG 4,476, Steam 886, Linux 1,052). Row counts = TSV counts, second apply = 0
changes, integrity ok. KDB 2.0 -> 8.3 MB (mostly prototypes). Backup:
`~/Games/KOTOR2_Mod_Workspace/backups/kdb/kotor2_ghidra_knowledge.db.pre_server_ingest_20261003`. Mac programs are not
ingested (their real names are tagged ANALYSIS; a strings/callee bridge must come first). Full rows stay in the TSVs;
the pipeline reads those directly. Report: `INGEST_REPORT.md`.
- Steam: the 25 trusted server names give 0 agreements and 7 conflicts with `name_map`. All 7 are naming style
  (`lpStartAddress_*`, `Class__Func`, `WinMain`) except `0x0070caa0` (server
  `CExoStreamingSoundSourceInternal::InitializeSource` vs `name_map` `StreamingSource::OpenStream`, medium). No branch-DB
  overlap.
- GOG: 8,385 agree / 1,940 disagree with the KPM GOG DB after normalisation; most disagreements are naming conventions
  (overload suffixes, template namespaces). Not mined yet for real label suspects.
- GOG types include `/Vtables` (111) and game structs (`Camera` 616 B, `CResGFF`, `CSWArea`, ...): a GOG-side vtable
  source for a non-circular A-VT and for Phase D offsets.

## Findings
- GOG program: 26,975 functions; 10,422 USER_DEFINED, 15,172 ANALYSIS, 1,380 DEFAULT, 1 IMPORTED. All 10,425
  KPM GOG DB rows are function starts in it. 8,874 names agree exactly. The 1,551 others are mostly naming
  conventions (`CExoString::DefaultConstructor` in the DB vs `CExoString::CExoString` on the server, `Destructor`
  vs `~X`, template namespaces `2012_` vs `2012>`). So the KPM GOG DB is a derivative of this program, not an
  independent name source; its value for us is the GOG function bodies (fingerprints: callees, strings, operand
  offsets, immediates).
