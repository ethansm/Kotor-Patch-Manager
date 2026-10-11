# Master findings ledger

Coverage: **PASS**; findings in ledger: 17; reduce files applied: 1; duplicate groups: 8 (1 duplicates folded into a representative).

## Top risks

| # | id | sev | verification | conf | cluster | file:lines | category | evidence |
|---|---|---|---|---|---|---|---|---|
| 1 | ba03658d665e | medium | downgraded | medium | C-b2a4f1e2 | patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:592-601 | logic | Rows 3, 538 and 539 return *n = 0 and nullptr on line 592, meaning no animations. The old animLen (old lines 594-596) returned false when the name was not found |
| 2 | 8cdf5396acc9 | medium | unverified | high | C-d0a55a6d | patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:709-713 | logic | Line 709 says: "a failed rebuild below leaves the old head: the tick toggle block re-attaches to it". But the new lines 711-713 call destroyCreature(s, "charact |
| 3 | 8af893ae7a19 | medium | unverified | high | C-febe1390 | patch_manager_mods/level_up_override_src/common_contract_test.cpp:196-199 | test | `LCHECK("filewatch::changed throttled within 1000ms", !filewatch::changed(rp, tmpPath, 1500));` / `LCHECK("filewatch::changed quiet after throttle when unchange |
| 4 | 7cf902d24841 | medium | unverified | medium | C-c264d85d | patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:2043-2048 | logic | Snapshot leaderCreature(): `const uintptr_t tbl = party::table(); const uintptr_t m = tbl ? party::member(tbl, 0) : 0; return m ? party::serverCreature(m) : 0;` |
| 5 | 0c57ad0882af | low | unverified | high | C-54203116 | patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1412-1414 | logic | reloadPlay() non-force path: `if (!filewatch::changed(g_ptPoll, PtName, GetTickCount(), false, &st)) return;` then `const bool changed = !g_ptPoll.primed \|\| (st |
| 6 | 3fb47e9bdf68 | low | unverified | high | C-bb2f641e | patch_manager_mods/level_up_override_src/common_contract_test.cpp:163-165 | test | `LCHECK("party::table function exists", static_cast<uintptr_t(*)(party::Fail*)>(&party::table) != nullptr);` (same form on lines 164-165). A function's address  |
| 7 | e1ee6713f3aa | low | unverified | high | C-b99f4488 | patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1434-1435 | style-risk | `// Note: retains MOVEFILE_WRITE_THROUGH (flush to disk before rename returns) per LevelUpOverride save safety.` / `// Propose additive writeThrough parameter t |
| 8 | 62da1e460980 | low | unverified | low | C-49caf94d | research/companion_levelup/tools/parity/unit_gap_test.cpp:130 | test | snprintf(g_cfg.table, sizeof g_cfg.table, "t.ini"); g_tablePoll = {}; g_tableStamp = {}; g_tableOk = false; g_cfg.nTags = 0; ... reloadTable(true);  (previously |
| 9 | 129fec265f26 | low | unverified-scanner | low | C-059ce8a9 | research/companion_levelup/tools/parity/gate_test.cpp:146 | other | [scanner pointer-math] reinterpret_cast \| return (member == reinterpret_cast<void*>(0x1000F100)) ? reinterpret_cast<uintptr_t>(g_creatureRet) : 0u; |
| 10 | 18bad252eee4 | low | unverified-scanner | low | C-b3abda6a | research/companion_levelup/tools/parity/gate_test.cpp:133 | other | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<uint32_t*>(client + 4) = (uint32_t)ci; |
| 11 | 36762ad12a41 | low | unverified-scanner | low | C-7121ff95 | research/companion_levelup/tools/parity/gate_test.cpp:135 | other | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<int*>(partyTable) = 3; |
| 12 | 7c24e67883b3 | low | unverified-scanner | low | C-1ebe1143 | research/companion_levelup/tools/parity/scenario_test.cpp:288 | other | [scanner pointer-math] reinterpret_cast \| return (member == reinterpret_cast<void*>(0x1000F100) && g_leader) ? reinterpret_cast<uintptr_t>(g_leader->creature) : |
| 13 | a9aaccbed7c4 | low | unverified-scanner | low | C-46b027cb | patch_manager_mods/level_up_override_src/common_contract_test.cpp:165 | other | [scanner pointer-math] uintptr_t-cast \| LCHECK("party::serverCreature function exists", static_cast<uintptr_t(*)(uintptr_t)>(&party::serverCreature) != nullptr) |
| 14 | c95f980c3454 | low | unverified-scanner | low | C-5fc23adb | research/companion_levelup/tools/parity/gate_test.cpp:132 | other | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<uint32_t*>(app + 4) = (uint32_t)client; |
| 15 | cea821a432d2 | low | unverified-scanner | low | C-bc7199c7 | research/companion_levelup/tools/parity/gate_test.cpp:134 | other | [scanner pointer-math] reinterpret_cast, hex-offset \| *reinterpret_cast<uint32_t*>(ci + 0x270) = (uint32_t)partyTable; |

## Coverage summary

- units: 43; reviewed 36, mechanical 2, skipped 5, excluded 0, gap 0
- packets to re-queue: 0
- errors: 0; details in coverage_report.md

## Human-only (excluded, not read)

(none)

## Cost

Basis: cli_reported_total_cost_usd (the only figure used for budgets and decisions).

- Worker calls: 4 (attempts total: 0); CLI-reported $0.0332 over 4 calls
- Reduce sessions: 1; CLI-reported $0.0039
- Tokens (as reported): input 22, cache creation 88,965, cache read 276,422, output 88
- ESTIMATE ONLY: illustrative list rates, never used for a budget or a decision: $0.0139 at haiku-5.5 list rates (0 requests over 100K tokens)

## All findings by cluster

### C-b2a4f1e2 (1 finding, patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ba03658d665e | medium | downgraded | medium | 592-601 | logic | worker |  | code-001 | 22b56b4 | Rows 3, 538 and 539 return *n = 0 and nullptr on line 592, meaning no animations. The old animLen (old lines 594-596) returned false when the name was not found. The new code (lines 596-601) returns true and sets *len = 2.0f in every case, including when t is null or n is 0: "*len = 2.0f; // Safe 2. | Check the flourish scheduler around line 762 and the rows that map to animTable returning nullptr or n=0. Confirm whether any flourish is now scheduled on rows 3, 538 or 539, or on unmapped supermodels, that previously was skipped. Then decide whether the 2.0 s default should apply only when the row |

Root cause (Reduce): animLen() was changed to return true with a 2.0s default for every name, which removes the caller's 'not in table' rejection path for rows with no static animation list.

### C-c264d85d (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7cf902d24841 | medium | unverified | medium | 2043-2048 | logic | worker |  | code-002 | 0fd1151, 39144b7 | Snapshot leaderCreature(): `const uintptr_t tbl = party::table(); const uintptr_t m = tbl ? party::member(tbl, 0) : 0; return m ? party::serverCreature(m) : 0;`. The removed code (packet o2040-o2042) read app, then `client = *(app+4)`, and obtained the party table by calling the engine ClientPartyTa | Disassemble the engine ClientPartyTable at 0x73FB90 (Steam and GOG builds) and confirm it returns *(*(arg+4)+0x270) with arg = the client value from [[AppGlobal]+4]. Check that the K2Addr table values used by party::table (AppGlobal) match the old g_eng.appGlobal for each build. In a host test, comp |

Root cause (Reduce): leaderCreature() now derives the party table from the party helper walk instead of the engine ClientPartyTable call, so its result may differ from the removed code path.

### C-d0a55a6d (2 findings, patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8cdf5396acc9 | medium | unverified | high | 709-713 | logic | worker |  | code-001 | 22b56b4 | Line 709 says: "a failed rebuild below leaves the old head: the tick toggle block re-attaches to it". But the new lines 711-713 call destroyCreature(s, "character change") before allocCreature/readApp/gate checks. destroyCreature (lines 524-534) frees the creature and sets s->creature = nullptr, s-> | Read the tick toggle block that uses helmApplied (around lines 680-705 and the tick caller of buildPreview). Confirm what it does when s->creature is null after a failed buildPreview. Then either update the comment or move destroyCreature after the readApp/gate checks so a failure keeps the old crea |
| 8f21ec108c65 | medium | unverified | medium | 711-713 | logic | worker | 8cdf5396acc9 | code-001 | 22b56b4 | if (s->creature && s->real != real) { destroyCreature(s, "character change"); } is placed before the gate check "if (!gate) { logf("build: gate 009f42b4 is zero, skipping"); return false; }" at line 724. When the gate is zero, the old preview is torn down and nothing is rebuilt, so the panel goes bl | Check how the caller of buildPreview handles a false return and whether a later tick retries the build. Compare the on-screen result when sceneview::gateOpen returns zero before and after this change. Consider moving the destroy after the gate and readApp checks. |

Root cause (Reduce): The character-change branch in the preview build path calls destroyCreature() before the allocation, readApp and gate checks, so any later failure or zero gate leaves the panel with no creature and no rebuild.

### C-febe1390 (1 finding, patch_manager_mods/level_up_override_src/common_contract_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8af893ae7a19 | medium | unverified | high | 196-199 | test | worker |  | code-002 | 0fd1151, 39144b7 | `LCHECK("filewatch::changed throttled within 1000ms", !filewatch::changed(rp, tmpPath, 1500));` / `LCHECK("filewatch::changed quiet after throttle when unchanged", !filewatch::changed(rp, tmpPath, 2500));` / `LCHECK("filewatch::changed force skips throttle if changed", !filewatch::changed(rp, tmpPat | Rewrite the sequence so the file is modified between polls (for example, write a longer body after the 1000 ms window and check that changed() returns true, and that a poll inside the window returns false even after the modification). Verify by temporarily making changed() ignore throttleMs or force |

Root cause (Reduce): Test assertions are either always true (taking the address of a function and comparing with nullptr) or depend on file-stamp timing that the test does not control, so they cannot fail as written.

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

### C-49caf94d (1 finding, research/companion_levelup/tools/parity/unit_gap_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 62da1e460980 | low | unverified | low | 130 | test | worker |  | other-002 | 0fd1151 | snprintf(g_cfg.table, sizeof g_cfg.table, "t.ini"); g_tablePoll = {}; g_tableStamp = {}; g_tableOk = false; g_cfg.nTags = 0; ... reloadTable(true);  (previously g_tableStamp = ~0ULL, a value no real file stamp can equal) | Read reloadTable() and the stamp comparison it uses (its force parameter and the operator== of the stamp type that g_tableStamp/g_tablePoll use). Confirm that force=true reloads even when the stamp equals a default-constructed value. If it does not, restore a distinct non-matching sentinel for g_tab |

Root cause (Reduce): Test assertions are either always true (taking the address of a function and comparing with nullptr) or depend on file-stamp timing that the test does not control, so they cannot fail as written.

### C-54203116 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0c57ad0882af | low | unverified | high | 1412-1414 | logic | worker |  | code-002 | 0fd1151, 39144b7 | reloadPlay() non-force path: `if (!filewatch::changed(g_ptPoll, PtName, GetTickCount(), false, &st)) return;` then `const bool changed = !g_ptPoll.primed \|\| (st != g_ptPoll.last);`. FileWatch.h:82-83 sets p.last = s and p.primed = true inside changed() before it returns true, so the local `changed`  | Compute `changed` from the result of the changed() call (true on this path) or drop the second compare. Check the log in a host run: start with a playthrough file present, and confirm a PLAYTHROUGH line appears on first load and after an edit. |

Root cause (Reduce): changed() sets the primed and last state before returning true, so the caller's recomputed 'changed' flag is always false when it reaches the log gate.

### C-5fc23adb (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| c95f980c3454 | low | unverified-scanner | low | 132 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<uint32_t*>(app + 4) = (uint32_t)client; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-6130750b (1 finding, patch_manager_mods/level_up_override_src/common_contract_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ffc4a3c91672 | low | unverified-scanner | low | 179 | other | scanner |  |  | 0fd1151, 39144b7 | [scanner pointer-math] c-pointer-cast \| party::g_serverCreature = [](void* m) -> uintptr_t { return (m == (void*)0x2000u) ? 0x3000u : 0u; }; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-7121ff95 (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 36762ad12a41 | low | unverified-scanner | low | 135 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<int*>(partyTable) = 3; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-b3abda6a (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 18bad252eee4 | low | unverified-scanner | low | 133 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast \| *reinterpret_cast<uint32_t*>(client + 4) = (uint32_t)ci; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-b99f4488 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e1ee6713f3aa | low | unverified | high | 1434-1435 | style-risk | worker |  | code-002 | 0fd1151, 39144b7 | `// Note: retains MOVEFILE_WRITE_THROUGH (flush to disk before rename returns) per LevelUpOverride save safety.` / `// Propose additive writeThrough parameter to filewatch::atomicWrite for Common owner adoption in Chunk C9.` ptWriteText keeps the hand-rolled MoveFileExA with MOVEFILE_WRITE_THROUGH o | Confirm nothing calls filewatch::atomicWrite for levelup_playthroughs.ini. Move the planning note to the design doc, or keep a comment next to the call that says the write-through flag is required. |

Root cause (Reduce): ptWriteText keeps a hand-rolled MoveFileExA with MOVEFILE_WRITE_THROUGH because the shared atomicWrite has no write-through option, so the save path does not use the shared helper.

### C-bb2f641e (1 finding, patch_manager_mods/level_up_override_src/common_contract_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3fb47e9bdf68 | low | unverified | high | 163-165 | test | worker |  | code-002 | 0fd1151, 39144b7 | `LCHECK("party::table function exists", static_cast<uintptr_t(*)(party::Fail*)>(&party::table) != nullptr);` (same form on lines 164-165). A function's address is never null, so the runtime half of each check is always true. | Keep the casts as signature checks and drop the always-true comparison, or rename the messages to say they check the signature. No runtime behaviour is at stake. |

Root cause (Reduce): Test assertions are either always true (taking the address of a function and comparing with nullptr) or depend on file-stamp timing that the test does not control, so they cannot fail as written.

### C-bc7199c7 (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cea821a432d2 | low | unverified-scanner | low | 134 | other | scanner |  |  | 39144b7 | [scanner pointer-math] reinterpret_cast, hex-offset \| *reinterpret_cast<uint32_t*>(ci + 0x270) = (uint32_t)partyTable; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |
