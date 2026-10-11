# Master findings ledger

Coverage: **PASS**; findings in ledger: 18; reduce files applied: 1; duplicate groups: 10 (0 duplicates folded into a representative).

## Top risks

| # | id | sev | verification | conf | cluster | file:lines | category | evidence |
|---|---|---|---|---|---|---|---|---|
| 1 | 3c9acfeb0b55 | medium | unverified | high | C-0c9ddbf4 | patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:596-601 | api-break | animLen now ends with `*len = 2.0f; // Safe 2.0s fallback ...` and `return true;` unconditionally (601). The old body returned false when the name was not in th |
| 2 | a81b5d039ec1 | medium | unverified | high | C-4916bced | patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:595-601 | logic | animLen now ends with `*len = 2.0f; // Safe 2.0s fallback ...` and `return true;` (596-601). The old code returned false when the name was not in the table (old |
| 3 | 6d18e325826a | medium | unverified | medium | C-463fefc4 | patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:709-714 | logic | Line 709 comment: "a failed rebuild below leaves the old head: the tick toggle block re-attaches to it". But the new block at 711-713 runs `destroyCreature(s, " |
| 4 | d9d1f418193b | medium | unverified | medium | C-e9215ec0 | research/companion_levelup/tools/parity/gate_test.cpp:320-322 | test | Base assertion removed at old line 306: CHECK(g_lastTableEcx == P(0x10001234), "table-get ECX must be [[appGlobal]+4]..."). Its nearest replacement at line 322  |
| 5 | 9dd171cb3452 | medium | unverified | medium | C-091b4fbd | patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:144-165 | bug | Line 164: `g_cfg = c; ++g_cfgGen; syncWheelCfg();` copies g_cfg.screenW/screenH into guikit::g_wheel.iniW/iniH. Line 165 (new): `syncScreen();` runs afterwards, |
| 6 | e3a9cfa43da6 | medium | unverified | medium | C-6abb165a | research/companion_levelup/tools/parity/unit_gap_test.cpp:127-136 | test | Line 130 resets only g_tablePoll, g_tableStamp, g_tableOk and g_cfg.nTags before reloadTable(true). Line 136 then asserts !g_rules && !g_fr[3].present && !g_pr[ |
| 7 | 380b0153b1a8 | low | unverified | high | C-6d44a05f | patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1391-1402 | logic | Non-forced path at 1392-1393: `if (!filewatch::changed(g_ptPoll, PtName, GetTickCount(), false, &st)) return;` then 1399: `const bool changed = !g_ptPoll.primed |
| 8 | 18f79695128b | low | unverified | high | C-ec29b3f5 | patch_manager_mods/level_up_override_src/common_contract_test.cpp:195-199 | test | LCHECK("filewatch::changed force skips throttle if changed", !filewatch::changed(rp, tmpPath, 2600, true));  The file is not modified between the 2500 check at  |
| 9 | 74362fd5891c | low | unverified | high | C-ce300838 | research/companion_levelup/tools/parity/unit_gap_test.cpp:111-115 | test | auto put = [](const char* name, const char* rules) { FILE* f = fopen(name, "wb"); fprintf(f, "[classskills]\ncs2=11111111\n...", rules); fclose(f); };  The fope |
| 10 | a9cf75e9ba64 | low | unverified | high | C-5558f557 | patch_manager_mods/level_up_override_src/common_contract_test.cpp:163-165 | logic | `LCHECK("party::table function exists", static_cast<uintptr_t(*)(party::Fail*)>(&party::table) != nullptr);` (same pattern at 164 and 165). The address of a nam |
| 11 | 129fec265f26 | low | unverified-scanner | low | C-059ce8a9 | research/companion_levelup/tools/parity/gate_test.cpp:146 | other | [scanner pointer-math] reinterpret_cast \| return (member == reinterpret_cast<void*>(0x1000F100)) ? reinterpret_cast<uintptr_t>(g_creatureRet) : 0u; |
| 12 | 18bad252eee4 | low | unverified-scanner | low | C-b3abda6a | research/companion_levelup/tools/parity/gate_test.cpp:133 | other | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<uint32_t*>(client + 4) = (uint32_t)ci; |
| 13 | 36762ad12a41 | low | unverified-scanner | low | C-7121ff95 | research/companion_levelup/tools/parity/gate_test.cpp:135 | other | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<int*>(partyTable) = 3; |
| 14 | 7c24e67883b3 | low | unverified-scanner | low | C-1ebe1143 | research/companion_levelup/tools/parity/scenario_test.cpp:288 | other | [scanner pointer-math] reinterpret_cast \| return (member == reinterpret_cast<void*>(0x1000F100) && g_leader) ? reinterpret_cast<uintptr_t>(g_leader->creature) : |
| 15 | a9aaccbed7c4 | low | unverified-scanner | low | C-46b027cb | patch_manager_mods/level_up_override_src/common_contract_test.cpp:165 | other | [scanner pointer-math] uintptr_t-cast \| LCHECK("party::serverCreature function exists", static_cast<uintptr_t(*)(uintptr_t)>(&party::serverCreature) != nullptr) |

## Coverage summary

- units: 43; reviewed 36, mechanical 2, skipped 5, excluded 0, gap 0
- packets to re-queue: 0
- errors: 0; details in coverage_report.md

## Human-only (excluded, not read)

(none)

## Cost

Basis: cli_reported_total_cost_usd (the only figure used for budgets and decisions).

- Worker calls: 16 (attempts total: 0); CLI-reported $0.0934 over 16 calls
- Reduce sessions: 1; CLI-reported $0.0028
- Tokens (as reported): input 76, cache creation 264,316, cache read 632,728, output 240
- ESTIMATE ONLY: illustrative list rates, never used for a budget or a decision: $0.0395 at haiku-5.5 list rates (0 requests over 100K tokens)

## All findings by cluster

### C-091b4fbd (1 finding, patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9dd171cb3452 | medium | unverified | medium | 144-165 | bug | worker |  | code-001 | 22b56b4 | Line 164: `g_cfg = c; ++g_cfgGen; syncWheelCfg();` copies g_cfg.screenW/screenH into guikit::g_wheel.iniW/iniH. Line 165 (new): `syncScreen();` runs afterwards, and syncScreen (lines 245-247) rewrites `g_cfg.screenW = (float)w; g_cfg.screenH = (float)h;` when guikit::syncScreen succeeds. The wheel's | Check whether the per-tick path calls syncWheelCfg() before any wheel use. If it does not, move the syncScreen() call before syncWheelCfg() in both loadIni branches, or call syncWheelCfg() again after it. Verify by logging guikit::g_wheel.iniW/iniH and g_cfg.screenW right after loadIni on a non-defa |

### C-0c9ddbf4 (1 finding, patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3c9acfeb0b55 | medium | unverified | high | 596-601 | api-break | worker |  | code-001 | 22b56b4 | animLen now ends with `*len = 2.0f; // Safe 2.0s fallback ...` and `return true;` unconditionally (601). The old body returned false when the name was not in the table. The only caller in the packet, playFlourish at line 762, still uses the old convention: `else if (!animLen(s, name, &len)) { dlogf( | Check whether any other caller of animLen exists outside this file. Then decide whether the fallback should live in playFlourish instead, keeping animLen's false-on-unknown contract. Confirm by tracing playFlourish for row 3/538/539 and for an unmapped name: the old code logged 'not in table' and re |

Root cause (Reduce): animLen was changed to return true with a 2.0s fallback for names missing from the table, but its only caller in playFlourish still treats false as the not-in-table case, so that branch is now dead and the table-miss behaviour changed silently.

### C-463fefc4 (1 finding, patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6d18e325826a | medium | unverified | medium | 709-714 | logic | worker |  | code-001 | 22b56b4 | Line 709 comment: "a failed rebuild below leaves the old head: the tick toggle block re-attaches to it". But the new block at 711-713 runs `destroyCreature(s, "character change")` before the build. destroyCreature (524-535) sets s->creature = nullptr, s->built = false, s->real = 0. A later failure a | Trace the tick and toggle code that reads s->creature, s->built and s->helmApplied after buildPreview returns false, and check whether it assumes the old head still exists. Also check whether the early returns at 716, 724 and 727 should leave s->creature null, or whether the destroy at 711-713 shoul |

### C-4916bced (1 finding, patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a81b5d039ec1 | medium | unverified | high | 595-601 | logic | worker |  | code-001 | 22b56b4 | animLen now ends with `*len = 2.0f; // Safe 2.0s fallback ...` and `return true;` (596-601). The old code returned false when the name was not in the table (old 595-596). The only caller, playFlourish at line 762, has `else if (!animLen(s, name, &len)) { ... return false; }`, so that skip branch can | Check whether playFlourish or any other caller relied on a false return for unknown names. Confirm which rows reach the n==0 path via animTable (rows 3, 538, 539) and whether a 2.0 s playback there is intended. If the fallback is wanted, decide whether it belongs in playFlourish rather than in the l |

Root cause (Reduce): animLen was changed to return true with a 2.0s fallback for names missing from the table, but its only caller in playFlourish still treats false as the not-in-table case, so that branch is now dead and the table-miss behaviour changed silently.

### C-6abb165a (1 finding, research/companion_levelup/tools/parity/unit_gap_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e3a9cfa43da6 | medium | unverified | medium | 127-136 | test | worker |  | other-002 | 0fd1151 | Line 130 resets only g_tablePoll, g_tableStamp, g_tableOk and g_cfg.nTags before reloadTable(true). Line 136 then asserts !g_rules && !g_fr[3].present && !g_pr[10].present && !g_pr[11].present. No good table with [featrules]/[powerrules] is loaded before the loop, and the rule globals are not reset  | In reloadTable, check whether a rule-parse failure clears g_rules, g_fr[] and g_pr[] or only sets g_ruleBadLine. Then make the test falsifiable: load a good table with rules first (as the control at lines 139-142 does), assert the rules are present, then run the bad table and assert they are cleared |

### C-e9215ec0 (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d9d1f418193b | medium | unverified | medium | 320-322 | test | worker |  | other-001 | 39144b7 | Base assertion removed at old line 306: CHECK(g_lastTableEcx == P(0x10001234), "table-get ECX must be [[appGlobal]+4]..."). Its nearest replacement at line 322 is CHECK(g_lastPartyTable == P(partyTable) && g_lastPartyIdx == 0, "party::member table must be partyTable..."), which checks a different ca | In gate_test.cpp, grep the parity directory for g_lastTableEcx and tableGet to confirm no CHECK reads g_lastTableEcx. Then confirm what the real table-get caller passes as ECX (the value that should equal the client pointer read via [[appGlobal]+4], i.e. the client object at app+4 in the new chain)  |

### C-059ce8a9 (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 129fec265f26 | low | unverified-scanner | low | 146 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast \| return (member == reinterpret_cast<void*>(0x1000F100)) ? reinterpret_cast<uintptr_t>(g_creatureRet) : 0u; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-1ebe1143 (1 finding, research/companion_levelup/tools/parity/scenario_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7c24e67883b3 | low | unverified-scanner | low | 288 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast \| return (member == reinterpret_cast<void*>(0x1000F100) && g_leader) ? reinterpret_cast<uintptr_t>(g_leader->creature) : 0u; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-46b027cb (1 finding, patch_manager_mods/level_up_override_src/common_contract_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a9aaccbed7c4 | low | unverified-scanner | low | 165 | other | scanner |  |  | 0fd1151, 39144b7 | [scanner pointer-math] uintptr_t-cast \| LCHECK("party::serverCreature function exists", static_cast<uintptr_t(*)(uintptr_t)>(&party::serverCreature) != nullptr); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-5558f557 (1 finding, patch_manager_mods/level_up_override_src/common_contract_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a9cf75e9ba64 | low | unverified | high | 163-165 | logic | worker |  | code-002 | 39144b7 | `LCHECK("party::table function exists", static_cast<uintptr_t(*)(party::Fail*)>(&party::table) != nullptr);` (same pattern at 164 and 165). The address of a named non-weak function is never null, so each condition is always true and these three checks cannot fail. Party.h:33, 80 and 89 define the fu | Confirm that the test passes with these lines removed and that the build fails when a Party.h function is renamed. If a real signature check is wanted, assert on behaviour (e.g. party::member(0, 0) == 0 and party::table with a null app global returns 0 with F_App), not on the function's address. |

### C-5fc23adb (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| c95f980c3454 | low | unverified-scanner | low | 132 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<uint32_t*>(app + 4) = (uint32_t)client; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-6130750b (1 finding, patch_manager_mods/level_up_override_src/common_contract_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ffc4a3c91672 | low | unverified-scanner | low | 179 | other | scanner |  |  | 0fd1151, 39144b7 | [scanner pointer-math] c-pointer-cast \| party::g_serverCreature = [](void* m) -> uintptr_t { return (m == (void*)0x2000u) ? 0x3000u : 0u; }; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-6d44a05f (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 380b0153b1a8 | low | unverified | high | 1391-1402 | logic | worker |  | code-002 | 0fd1151, 39144b7 | Non-forced path at 1392-1393: `if (!filewatch::changed(g_ptPoll, PtName, GetTickCount(), false, &st)) return;` then 1399: `const bool changed = !g_ptPoll.primed \|\| (st != g_ptPoll.last);`. In FileWatch.h:82-83 `changed()` already sets `p.last = s; p.primed = true;` before returning true, so at line  | Compare the logged output of a session that starts with levelup_playthroughs.ini present and then edits it. The expected PLAYTHROUGH line is absent. Fix idea to verify, not apply: compute `changed` from the poll state before calling filewatch::changed(), or use the return value of changed() directly |

### C-7121ff95 (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 36762ad12a41 | low | unverified-scanner | low | 135 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<int*>(partyTable) = 3; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-b3abda6a (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 18bad252eee4 | low | unverified-scanner | low | 133 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<uint32_t*>(client + 4) = (uint32_t)ci; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-bc7199c7 (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cea821a432d2 | low | unverified-scanner | low | 134 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast, hex-offset \| *reinterpret_cast<uint32_t*>(ci + 0x270) = (uint32_t)partyTable; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-ce300838 (1 finding, research/companion_levelup/tools/parity/unit_gap_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 74362fd5891c | low | unverified | high | 111-115 | test | worker |  | other-002 | 0fd1151 | auto put = [](const char* name, const char* rules) { FILE* f = fopen(name, "wb"); fprintf(f, "[classskills]\ncs2=11111111\n...", rules); fclose(f); };  The fopen result is used with no null check before fprintf and fclose. | Confirm that fopen("t.ini", "wb") can fail in the test's working directory (the mkdtemp dir from line 29-30). Add a null check that returns a nonzero exit or a CHECK failure before fprintf, then rerun unit_gap_test to see the failure reported cleanly. |

### C-ec29b3f5 (1 finding, patch_manager_mods/level_up_override_src/common_contract_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 18f79695128b | low | unverified | high | 195-199 | test | worker |  | code-002 | 0fd1151, 39144b7 | LCHECK("filewatch::changed force skips throttle if changed", !filewatch::changed(rp, tmpPath, 2600, true));  The file is not modified between the 2500 check at line 198 and this call, so the assertion expects false for an unchanged file. | Modify tmpPath (for example write more bytes) between the 2500 check and a forced call at a time inside the throttle window (e.g. 2600), then assert true. Confirm with FileWatch.h changed() lines 78 and 82 that force skips the throttle branch and that the size change alters the stamp. |
