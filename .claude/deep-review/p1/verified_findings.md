# Verified findings (Haiku verifier, then Sonnet adjudicator)

Find-only. Tiers list real findings; decoys and controls are reported under Calibration only.

## Summary

| Tier | Count (representatives) | Count incl. folded duplicates |
|---|---|---|
| CONFIRMED | 6 | 7 |
| CONFIRMED-BY-SONNET-ONLY | 0 | 0 |
| CONTESTED-NEEDS-HUMAN | 0 | 0 |
| REFUTED | 2 | 2 |

## Calibration

- Decoys: 8/8 caught (final refuted or needs-human) of 8 queued; catch rate 1.0; Haiku reproduced 0.
- Haiku vs Sonnet agreement: all 1.0 over 16 items; real 1.0; Sonnet self-reported 1.0; Haiku reproduced but Sonnet refuted 0; Haiku refuted but Sonnet confirmed 0.
- Fabricated quotes: Haiku 0 (trivial 0), Sonnet 0. Haiku quality by batch: {'ok': 2}.

Controls

| id | label | expected (Haiku / final) | Haiku | final | ok |
|---|---|---|---|---|---|

Per-specialty Haiku/Sonnet agreement

| specialty | n | agree | rate |
|---|---|---|---|
| source-logic | 8 | 8 | 1.0 |
| test-weakening | 8 | 8 | 1.0 |

## Cost

- Haiku verifier: 16 call(s), CLI-reported $0.060196
- Sonnet adjudicator: 2 session(s), CLI-reported $0.025812
- Total CLI-reported $0.086008 (basis: cli_reported_total_cost_usd (the only figure used for budgets and decisions))
- ESTIMATE ONLY: illustrative list rates, priced per request, never used for a decision: $0.205524 (Haiku $0.024972, Sonnet $0.180552); Sonnet cache-write rate assumed 1.25x input.

## Coverage

- Complete: True (16 queue items, decoys and controls included)

## CONFIRMED (6)

### ba03658d665e  patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:592-601  [low] logic (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: downgraded/medium; Sonnet quotes ok 3/3
- Reasoning: Read EquipCharPreview.cpp:589-601, 757-763. animTable returns null/0 for rows 3/538/539 and animLen then unconditionally returns true with *len=2.0f, so the 'not in table' branch in playFlourish (line 762) is unreachable. Scheduled flourishes from config names therefore play a 2.0 s guess on those rows instead of being skipped. The fallback is documented as deliberate in the comment at 600, so this is a behaviour risk rather than an accident. The old-code regression itself cannot be checked here.
  - check: Check the prior version of animLen (git history or backup) to confirm it returned false for unknown names, which is the regression claimed.
  - check: Run a row 3/538/539 supermodel with a flourish list configured and confirm whether a 2.0 s clip with a missing animation is played or is harmless.
  - quote patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:592: `if (row == 3 \|\| row == 538 \|\| row == 539) { *n = 0; return nullptr; }`
  - quote patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:600: `*len = 2.0f; // Safe 2.0s fallback for unmapped supermodels/animations`
  - quote patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:762: `else if (!animLen(s, name, &len)) { dlogf("flourish: '%s' not in table for row %d", name, appRow(s)); return false; }`

### 8cdf5396acc9  patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:709-713  [low] logic (source-logic)
- Also covers duplicate finding(s): 8f21ec108c65 (patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:711-713)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 3/3
- Reasoning: Read EquipCharPreview.cpp:706-728 and 520-535 and tick block 967-970. destroyCreature runs when the character changes and sets s->creature to null, s->built to false and s->real to 0, so a later failure at allocCreature, readApp, the gate or the vtable check leaves no old head. The comment at 709 describes a recovery path that exists only on the same-character path, where the creature is kept. Effect is a misleading comment and a tick re-attach on a null creature, not a crash.
  - check: Trigger a character change whose readApp or gate fails and check the log for 'no head part' from attachHelmet, to confirm the tick path skips cleanly.
  - check: Check partGob returns null when s->creature is null.
  - quote patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:709: `// a failed rebuild below leaves the old head: the tick toggle block re-attaches to it`
  - quote patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:534: `s->creature = nullptr; s->built = false; s->real = 0; s->flourishActive = false; s->baseModels = -1;`
  - quote patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:712: `destroyCreature(s, "character change");`

### 0c57ad0882af  patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1412-1414  [low] logic (source-logic)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: unverified/low; Sonnet quotes ok 3/3
- Reasoning: Read FileWatch.h:77-85 and LevelUpOverride.cpp:1403-1414. changed() stores p.last and sets p.primed before returning true. reloadPlay's non-force path then computes changed = !primed \|\| st != last, and both terms are false, so the local is always false. The 'PLAYTHROUGH loaded/missing' log at 1429 never fires on that path, including the first load. Diagnostics only; no data impact.
  - check: Run a polled reload and a first load through the non-force caller (near LevelUpOverride.cpp:1763 or 2256) and confirm the PLAYTHROUGH log line is absent.
  - quote patch_manager_mods/_shared_src/FileWatch.h:83: `p.last = s; p.primed = true;`
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1412: `const bool changed = !g_ptPoll.primed \|\| (st != g_ptPoll.last);`
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1429: `if (changed) KP_LOG(chPlay, "PLAYTHROUGH %s: %d section(s)%s"`

### e1ee6713f3aa  patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1434-1435  [low] style-risk (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: unverified/low; Sonnet quotes ok 2/2
- Reasoning: Read LevelUpOverride.cpp:1433-1444 and FileWatch.h:113-135. ptWriteText uses MOVEFILE_WRITE_THROUGH on its rename. atomicWrite has no write-through option, and it calls _commit or fsync before a plain REPLACE_EXISTING rename. The planning note is accurate. The real loss on conversion would be rename durability across a crash, not the file-data flush, so the claim's wording is slightly overstated. This is a latent risk.
  - check: If the conversion is planned, test on Windows whether a rename without MOVEFILE_WRITE_THROUGH survives a forced power-off right after a level-up save.
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1441: `ok = ok && MoveFileExA(PtTmp, PtName, MOVEFILE_REPLACE_EXISTING \| MOVEFILE_WRITE_THROUGH);`
  - quote patch_manager_mods/_shared_src/FileWatch.h:128: `ok = MoveFileExA(tmp.c_str(), path, MOVEFILE_REPLACE_EXISTING) != 0;`

### 3fb47e9bdf68  patch_manager_mods/level_up_override_src/common_contract_test.cpp:163-165  [low] test (test-weakening)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: unverified/low; Sonnet quotes ok 2/2
- Reasoning: Read common_contract_test.cpp lines 163-165. Each LCHECK compares a function address cast to a function-pointer type against nullptr. A non-weak function's address is never null, so the runtime half is always true. The cast does give a compile-time signature check, and the behavioural checks at lines 175-180 do real work, so the impact is misleading naming rather than lost coverage.
  - check: Compile with -Wall -Wextra and look for -Waddress warnings on lines 163-165.
  - check: Confirm party::table, member and serverCreature are not declared weak in the party header, which is not in this tree.
  - quote patch_manager_mods/level_up_override_src/common_contract_test.cpp:163: `LCHECK("party::table function exists", static_cast<uintptr_t(*)(party::Fail*)>(&party::table) != nullptr);`
  - quote patch_manager_mods/level_up_override_src/common_contract_test.cpp:175: `LEQ("party::table walks app -> client -> ci -> partyTable", party::table(&why), partyTable);`

### 8af893ae7a19  patch_manager_mods/level_up_override_src/common_contract_test.cpp:196-199  [low] test (test-weakening)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 2/2
- Reasoning: Read common_contract_test.cpp lines 196-200. Lines 197-199 expect `!changed(...)` on an unchanged file after the initial load. Per FileWatch.h, changed() returns false whenever primed and the stamp matches, so these would also pass with the throttle or force removed. The sibling filewatch_test.cpp covers throttle, force and post-load change detection, which lowers practical risk. The contract gate itself is weak.
  - check: Mutate FileWatch.h to drop the throttle check (line 78) or the force bypass, then run common_contract_test; confirm it still passes.
  - check: Check whether this contract test is the only FileWatch gate for level_up_override.
  - quote patch_manager_mods/level_up_override_src/common_contract_test.cpp:197: `LCHECK("filewatch::changed throttled within 1000ms", !filewatch::changed(rp, tmpPath, 1500));`
  - quote patch_manager_mods/_shared_src/FileWatch.h:82: `if (p.primed && s == p.last) return false;`

## CONFIRMED-BY-SONNET-ONLY (0)

## CONTESTED-NEEDS-HUMAN (0)

## REFUTED (2)

### 7cf902d24841  patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:2043-2048  [none] logic (source-logic)
- Haiku: refuted (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 3/3
- Reasoning: Read Party.h:33-42 and GameAddr.h:36. party::table() walks app=*(appGlobal), a=*(app+4), b=*(a+4), t=*(b+0x270). The engine getter documented as FUN_0073fb90(A) = *(*(A+4)+0x270) applied to client=*(app+4) gives *(*(client+4)+0x270), which is the same chain. The raw walk therefore matches the engine getter's documented body. Residual risk is only that the GOG build's getter address differs, which the tree does not show to differ in body.
  - check: In a debugger on the Steam and GOG exes, compare party::table() with the engine getter's return for the controlled player.
  - check: Confirm the GOG ClientPartyTable function body matches the Steam dump's *(*(A+4)+0x270).
  - quote patch_manager_mods/_shared_src/GameAddr.h:36: `constexpr uintptr_t ClientPartyTable = 0x0073FB90; // FUN_0073fb90(A) = *(*(A+4)+0x270), the party table getter`
  - quote patch_manager_mods/_shared_src/Party.h:38: `if (!saferead::readAt<uint32_t>(SiteBase, a, 4, &b) \|\| !b) return fail(F_A4);`
  - quote patch_manager_mods/_shared_src/Party.h:39: `if (!saferead::readAt<uint32_t>(SiteBase, b, 0x270, &t) \|\| !t) return fail(F_Table);`

### 62da1e460980  research/companion_levelup/tools/parity/unit_gap_test.cpp:130  [none] test (test-weakening)
- Haiku: refuted (high), quote ok ratio 1.0; Reduce: unverified/low; Sonnet quotes ok 3/3
- Reasoning: FileWatch.h changed() (lines 77-85) skips the throttle only when p.checked is set, and skips the stamp comparison only when p.primed is set. Resetting g_tablePoll = {} clears both, so reloadTable(true) always reaches the reload path. The stamp g_tableStamp is only written, never compared. The stale-stamp concern does not apply. The line citation is slightly off: the CARRY checks are at 136-137.
  - check: Optionally check git history of unit_gap_test.cpp to confirm the old ~0ULL sentinel; this does not change the verdict.
  - quote patch_manager_mods/_shared_src/FileWatch.h:82: `if (p.primed && s == p.last) return false;`
  - quote patch_manager_mods/_shared_src/FileWatch.h:65: `bool primed = false; // a stamp has been seen / accepted`
  - quote research/companion_levelup/tools/parity/unit_gap_test.cpp:130: `snprintf(g_cfg.table, sizeof g_cfg.table, "t.ini"); g_tablePoll = {}; g_tableStamp = {}; g_tableOk = false; g_cfg.nTags = 0;`

