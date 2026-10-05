# Action Hotbar polish review (2026-10-04)

Source for phases HP1-HP7 (doc 24). Three review passes over code, tests and docs at b6aad91.
HP2-HP7 work from this list; no new review is needed. Items tagged `(HP1) DONE` were fixed in session HP1.

Format: ID | where | sev | effort | issue -> fix. Line numbers are as of b6aad91 + Task-2 commit 706a6d3.

### Code (A)
A1 | HotbarLogic.h:363 fmtPersaveSource | bug | S | %.170s clips status after long path -> status before path (HP1) DONE
A2 | ActionHotbar.cpp:1626 | bug | S | snapshot exists but Load fails = wiped live bar -> keep live + WARN (HP1) DONE
A3 | ActionHotbar.cpp:1612 | bug | S | replaceLiveProfiles skips refreshEffective -> stale g_eff/g_slotBound after load (HP1) DONE
A4 | ActionHotbar.cpp:1620 | bug | S | prints engine folder %.120s after 1-byte probe -> bounded copy (HP1) DONE
A5 | ActionHotbar.cpp:98,503 | bug | M | only override/spells.2da; absent = target modes silently dead -> resource fallback or WARN (HP7)
A6 | :1605 vs :1091 | med | S | every load overwrites the first-edit .bak -> .load.bak + BakPath const (HP1) DONE
A7 | :413,594,1209 | med | M | resolve() re-reads 6 groups x10 slots per refresh; `g<6` literal x3 -> per-tick snapshot + InvokableGroups (HP4)
A8 | :514,553,889 | med | S | CanUseItem cast x3, Vitals->Candidate loop x2 -> canUseItem/buildCandidates (HP4)
A9 | :683/790/957 etc | med | M | 3 visibility + 2 fill + 2 alpha writers -> one Widget API (HP4)
A10 | :1140-1303 | med | M | updateEditor 165 lines -> split visibility/hover/picker/arrows/click (HP4)
A11 | :761-781,1208-1219,1235,1131,341-363 | med | M | pure tooltip/picker/modeBtn/autofill logic in DLL -> HotbarLogic.h + tests (HP5)
A12 | :1156,906,912,355,727,846,864 | med | S | function statics not reset on HUD rebuild -> move into Bar/Editor (HP4)
A13 | :1613,1708-1713 | med | S | load/new-game hooks touch g_bar widgets outside tick -> pending flag (HP1) DONE
A14 | :148-151 | low | S | seen[] sized to S_WrProjRestore, misses S_Mode -> S_Count (HP6)
A15 | :8, hooks.toml:36, :1740 | low | S | stale 75/57/base+56 counts -> ControlCount (HP1) DONE
A16 | :5-11 | low | S | header lists 3 of 8 hooks (HP1) DONE
A17 | :125 | low | S | "same file as 1.0.0" wrong (HP1) DONE
A18 | :1008 | low | S | "never restored" wrong, DllMain restores (HP1) DONE
A19 | :1148 | low | S | "667 already WARNs" stale line ref -> updateButtons (HP1) DONE
A20 | :88,91,1312,1320,1635,1727, HotbarConfig.h:16 | low | S | build/version history in comments -> drop (HP1) DONE
A21 | HotbarConfig.h:3,395; HotbarLogic.h:1; Tlk.h:1 | low | S | stale headers (future editor, FPH, twoda_test) (HP1) DONE
A22 | HotbarConfig.h:365-368 | low | S | INI header: @lowhp items only; add SyncActionBar/TargetProbe (HP1) DONE
A23 | manifest.toml:6,12; PULL_REQUEST.md | low | S | description incomplete; PR text 30-slot v1.0.0 (HP1 manifest, HP2 PR) — HP1 part DONE
A24 | :243,585,871,1685,1690,1784,1840 | low | S | ring 16 literal x7, push dup -> RingSize + helpers (HP6)
A25 | :765-769; HotbarLogic.h:51-71; HotbarConfig.h:86 | low | S | key flag literals -> KeyItemFlag/KeySpellFlag/KeyRowMask; buildTipText use tabOfKey (HP6)
A26 | :779,1248,689,794,962; HotbarLogic.h:160,168,328 | low | S | flag bits 1/2/0x20 -> named (HP6)
A27 | :428,489-492,215,819,1749,1701,1543,1554,1796,301,1819 | low | S | unnamed engine offsets; oid read S_Misc vs S_Hud -> constants, one site (HP6)
A28 | :1682 vs 1011,... | low | S | HUD-fresh 250 vs 300 ms; cadences; pt.y>1100; {3840,1600} fallback; heal cap 20 x3 -> constants, GuiRefW/H (HP6)
A29 | :377-383,399,599-602 | low | S | targetProbeMode rescans extra; TM_Self vs LowHp probe rule differ (HP1 parse; rule check) — HP1 part DONE
A30 | :533,571,842,238,44; HotbarConfig.h:42; HotbarLogic.h:383,400 | low | S | dead params/globals, RowDef::modifier, stanceKey/rectsOverlap test-only (HP6)
A31 | :841,903-904 | low | S | forward decls from definition order (HP4)
A32 | :254,227,1574,1062 vs 1403 | low | S | naming: gateWhy, IniPath mutable, Power/Powers tab names, hb:: case mix (HP6)
A33 | :126-127,1642-1643,1831,372 | low | S | literals instead of LogPath/IniName/HudSyncStance/slotDigit (HP1) DONE
A34 | :594,685,692,164-170 | low | S | log tag style (`avail` no colon, btn[] vs edit:, KP_WARN vs KP_LOGV) (HP6)
A35 | :1347-1365,1556,1560 | low | S | ignored results in createLabel/healTextures -> count g_widgetFaults (HP6)
A36 | :719-725,1314 | low | S | aspectFix own EnumWindows; enumWnd dup of guikit::gameWindow (HP4)
A37 | :919,923,927 | low | S | hide-slot sequence x3 -> hideSlot (HP4)
A38 | :474-481,520-579 | low | S | "using it normally"+dumpRing x8 -> targetFail (HP4)
A39 | HotbarConfig.h:330,334; :352,191 | low | S | raw strtol for HealThreshold/SyncActionBar; keys[256]; g%7 wrap (HP6)
A40 | :1580; TwoDA.h:100 | low | S | saveDirFor per-byte probeRead; 64 KiB stack buffer (HP6)

### Tests (HB) — coverage: HotbarLogic 97.7, HotbarConfig 99.2, TwoDA 100, Tlk 97.8; gaps = logic still in the DLL
HB01 | :761 buildTipText | high | S | "Medpac (Type)" -> "Quantity: Type" -> tipTitleQty (HP1) DONE
HB02 | :1601 replaceLiveProfiles | high | M | merge rule untested -> mergeSnapshot (HP1) DONE
HB03 | :1622 restoreFromSave | med | S | source pick untested -> pickSnapshot (HP1) DONE
HB04 | :1072 assignSlot freeze | med | S | -> freezeEffective + tests (HP5)
HB05 | :583 drainRing route | med | S | -> useRoute decision table (HP5)
HB06 | :377 targetProbeMode atoi | med | S | -> hb::targetProbe parseInt32 (HP1) DONE
HB07 | :337 refreshEffective autos | med | M | -> collectAutos (HP5)
HB08 | :498 classifyItem | med | S | -> classifyRow vs fixture (HP5)
HB09 | make_spells_fixture.py:19 | med | S | fixture lacks usertype/hostilesetting + healing row (HP5)
HB10 | HotbarConfig.h:320 | med | S | SchemaVersion "validated on Save" is false (HP1 comment; HP5 test) — HP1 part DONE
HB11 | HotbarConfig.h:342 | med | S | [General] notes dropped; "Foo=a;b" -> "a" -> pin in test (HP5)
HB12 | HotbarConfig.h:111 | med | S | names ending @self/@lowhp or with ';'/" (" don't round-trip -> property test + refuse (HP5)
HB13 | config_roundtrip_test.cpp:365 | med | S | Save failure path untested (HP5)
HB14 | HotbarConfig.h:248 | low | S | non-ASCII names/paths (HP5)
HB15 | HotbarConfig.h:311 | low | S | profile name not trimmed; junk after ] accepted (HP5)
HB16 | HotbarLogic.h:423 parseHexOrInt | low | S | octal "010", 32-bit overflow (HP5)
HB17 | HotbarLogic.h:428 | low | S | range "t" vs "T" mismatch (HP5)
HB18 | HotbarLogic.h:156; HotbarConfig.h:220 | low | S | slotView wantEmpty&&have; moveDuplicates differing suffix (HP5)
HB19 | HotbarConfig.h:75 | low | S | normalizeItemName direct tests (HP5)
HB20 | HotbarLogic.h:383 | low | S | stanceKey unused; rectsOverlap test-only (HP5/HP6)
HB21 | hotbar_logic_test.cpp:239 | low | S | effSignature test copies algorithm -> golden constants (HP5)
HB22 | hotbar_logic_test.cpp:341 etc | low | S | constant-restating checks -> drop (HP5)
HB23 | hotbar_bugs_test.cpp:7 | low | S | duplicates -> fold into hotbar_logic_test (HP5)
HB24 | config_roundtrip_test.cpp:23 etc | low | S | temp-path snippet x5 -> kt::TempDir (HP5)
HB25 | textures_test.cpp:8 | low | S | expected table dup with py test -> generate kExpected (HP5)
HB26 | make_tlk_fixture.py:1 | low | M | no fixture freshness check; real-data mode never run (HP5)
HB27 | hotbar_log_test.cpp:148; HotbarConfig.h:395; common_contract_test.cpp:35 | low | S | FPH naming in comments (HP1) DONE
HB28 | Tlk.h:1; test.toml:4 | low | S | stale comments; -I../_shared_src into default cxxflags (HP1 comments) — HP1 part DONE
HB29 | tests/static/gui_ids.toml:113 | low | S | stale source line numbers (HP1) DONE
HB30 | config_roundtrip_test.cpp:381 | low | S | prints whole INI on pass -> only on failure (HP5)

### Docs (D) — doc 07 = 844 lines (status 1-191, old spec 193-523, dev log 524-844); no README, no CHANGELOG
D01 | no README anywhere | high | M | player README in Patches/ActionHotbar (HP2) DONE ad68a19 (KPM e16a378)
D02 | HotbarConfig.h:231-239 | high | S | INI key reference table (HP2) DONE ad68a19 (KPM e16a378)
D03 | HotbarConfig.h:362-368 | med | S | INI header missing keys (HP1) DONE
D04 | action_hotbar_src/PULL_REQUEST.md | high | M | 30 slots/Ctrl-Alt/v1.0.0 -> rewrite 1.2.x (HP2) DONE ad68a19 (KPM e16a378)
D05 | clone PULL_REQUEST.md | high | S | drifted copy -> one canonical (HP2) DONE ad68a19 (KPM e16a378)
D06 | PR readiness | high | S | screenshots, CHANGELOG, test evidence list (HP2) DONE ad68a19 (KPM e16a378); screenshots still a live TODO
D07 | ActionHotbar.cpp:5 | med | S | 75 -> 86 controls (HP1) DONE
D08 | ActionHotbar.cpp:5-8 | med | S | header 3 of 8 hooks (HP1) DONE
D09 | HotbarConfig.h:2-3 | low | S | "future editor" (HP1) DONE
D10 | Tlk.h:1 | low | S | tlk_test.cpp (HP1) DONE
D11 | manifest.toml:6 | med | S | description incomplete, log names (HP1) DONE
D12 | RuntimeGuiControls.md:5,186,188,216 | med | S | old name, 57/75 counts, old path (HP2) DONE ad68a19 (KPM e16a378)
D13 | clone docs/RuntimeGuiControls.md | med | S | sync copies (HP2) DONE ad68a19 (KPM e16a378)
D14 | doc07:3-191 | high | M | 6 RESUME blocks -> one status block + History (HP3)
D15 | doc07:11 vs 18 | med | S | mode cycle contradiction (HP3)
D16 | doc07:12 vs 4 | low | S | log name in checklist (HP3)
D17 | doc07:74,78,182,526,650,741 | med | S | force_power_hotbar_src paths (HP3)
D18 | doc07:104,160,166,183,189,526 | med | S | Patches/ForcePowerHotbar paths (HP3)
D19 | doc07:182-183 | med | S | Sources bullet stale (1200 lines, 2 tests) (HP3)
D20 | doc07:184 | med | S | Known gaps re-triage (HP3)
D21 | doc07:50,73,78,142,170 | low | S | 75/168 control counts (HP3)
D22 | doc07:23-26 | med | S | install recipe id force-power-hotbar (HP3)
D23 | doc07:187-191 | med | S | second old install workflow (HP3)
D24 | doc07:21,30,33 | low | S | stale build-30/probe lines (HP3)
D25 | doc07:1,186,193-523 | med | M | title, index, old spec -> appendix (HP3)
D26 | doc07:408-523 | low | M | 30-slot design/open questions -> Deferred ideas (HP3)
D27 | doc07:174,138,162 | med | S | persistence rule not written down (HP2 README + HP3 section) DONE ad68a19 (KPM e16a378)
D28 | 07_force_power_hotbar_feedback.md | med | S | all resolved -> resolution column, archive in evidence (HP3)
D29 | evidence dir | low | S | README index; rename decision (HP3)
D30 | doc07:4,12 | med | S | troubleshooting for players (HP2) DONE ad68a19 (KPM e16a378)

### HP1 result (2026-10-04)
- Commits KOTOR-II 6fcb8ec (helpers + bug fixes, red first: hotbar_logic_test, hotbar_log_test failed to build), 054e0e9 (comment sweep), eb86aef (1.2.1); local-mods 8557111. DLL md5 df48fc9e.
- A29 rule check: `@self` stays live only at TargetProbe=2 (no target decision to log at 1); comment at the drainRing call site. Not in the doc 07 decision table: add it in HP3.
- fmtPersaveSource format is `persave: <why>: <status>: source=<path %.150s>`.
- Still open from HP1 scope: the parser accepts `@lowhp`/`@self` on spell:/feat: values (header now says "Item value suffix"); HB12/HP5 should refuse or pin it.
