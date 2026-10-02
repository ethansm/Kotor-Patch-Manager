# companion-ai 0.2.0-probe: build note (2026-10-02)

Static build only: nothing installed, the game never launched, nothing written in the game folder, DB not written, nothing committed.
Design followed: `PROBE_DESIGN.md` sections 1-8 (13 plain-detour windows + the 2 handler-only probes PARTY and A1X; no consumed exit added).

## Artifacts
| item | value |
|---|---|
| version | `0.2.0-probe` (CompanionAI.cpp `Version`, manifest.toml, ini header, toml comment) |
| build dir | `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/CompanionAI/` |
| new kpatch | `CompanionAI.kpatch`, md5 `a631f2d20c812f8e79a854854b8e9f4e` (112849 B; the zip md5 changes with every rebuild) |
| new DLL (`binaries/windows_x86.dll`) | md5 `aef7c42e357605793fb1db7fb8c8f037`, 29 exports |
| live kpatch backup | `Patches/CompanionAI/CompanionAI-0.1.0-live.kpatch`, md5 `db371b9f86531c3ab23d923578b843a8` (its DLL md5 `1961ded0379a8e0f7f978c7ddc6ddb7f` = the live `patches/companion-ai.dll`, checked read-only) |
| install stager | `stage_and_verify_install.kpatch_index()` finds the live DLL md5 in `CompanionAI-0.1.0-live.kpatch` (plus the older stage copies), and the new build under `CompanionAI.kpatch` |

**Rollback** = re-stage `CompanionAI-0.1.0-live.kpatch` (the stager picks it by the live DLL md5) and install it as the companion-ai id, the same way as any
install (game closed, back up `kpm_install_state.json` + `patch_config.toml` + `patches/` first). The INI is backwards compatible: 0.1.0 ignores the `probe_*` keys.

## Files changed (source = `patch_manager_mods/`, copied to the build dir: CompanionAI.cpp, CompanionAiLogic.h, exports.def, hooks.toml, manifest.toml)
- `companion_ai_src/CompanionAI.cpp`: probe state + `pline/pev` budget layer, `probeSummary` (PRS), `partyTick`, `orphanTick`, 13 handlers; A1 gate (strict test, `pendingA1`, line suffix), E2 (`a1xFollowup`), `logConfig` (`# config probes ...` line), `ObjInfo.ispc`.
- `companion_ai_src/CompanionAiLogic.h`: `Config` probe keys, `rsSite/rsName`, `pfSite/pfName/pfTransient`, `p2Runs`, `shoutExit/shoutName`, `fm2cReason`, `fm3Lost`, `a1Strict`, `a1OrphanDue`, `PairCount/pairBump`, `rarIndex`, `ProbeCap`.
- `companion_ai_src/companion_ai_logic_test.cpp` (+`testProbeHelpers`, probe INI/known-key/exports checks), `companion_ai.ini`, `exports.def` (+13), `kotor2-steam-aspyr.hooks.toml` (+13 hooks, FM3 comment reworded), `manifest.toml`.
- `mods678_research/check_windows.py` (+13 `CA P ...` rows), `analyze_play_logs.py` (`analyze_probes`, 15 `PR-*` checklist rows, section 8, `EXPECTED_HOOK_COUNTS` 31), `test_analyze_play_logs.py` (+16 tests), fixtures `testdata/playlogs/probe_pass`, `probe_fail`.
- `mods678_research/check_windows_output_2026-10-02a.txt` (gate output).

## Gate results
1. `check_windows.py`: **ALL OK**, 0 `WARN`, 46 `[OK]` rows, "Pairwise overlap among 46 windows: none", "TOML cross-check ... all hooks match".
2. `verify_windows_both_exes.py check_windows_output_2026-10-02a.txt`: **ALL MATCH**.
3. Host test: `2436 checks, 0 failed / ALL PASS`. Analyzer: `python3 -m unittest test_analyze_play_logs` -> 51 tests OK (35 old + 16 new).
4. `i686-w64-mingw32-g++-posix -std=c++17 -O2 -DWIN32 -DNDEBUG -D_WINDOWS -D_USRDLL -fsyntax-only -Wall -Wextra CompanionAI.cpp` in the build dir: clean (rc 0, no output).
5. exports.def == hooks.toml functions: 31 hooks / 29 distinct names / 29 exports (and the built DLL exports 29).
6. kpatch hooks toml and manifest `cmp` byte-identical to the source files; `tomllib` loads it (31 hooks).
7. Sibling self-patch sites (`0x40D69E 0x8ABA8C 0x8ADA08 0x8ADA81 0x8ADFA2 0x8AE02D 0x8AE19A 0x8AE1CD 0x8AE793`): minimum distance 1115459 B (> 12 required).
8. Consumed exits: none added (the companion-ai toml still has its 3, T2 / A1 / A4, unchanged).

## INI keys (all `=1` in the shipped probe INI; a flag 0 stops the logging, the hook stays installed)
`probe_fm2c probe_fm2f probe_fm2r probe_fm3 probe_rs probe_p2 probe_pf probe_rar probe_shx probe_ss1 probe_gca probe_party probe_a1x`, `probe_lines=400` (0..100000, per-probe PR budget; PRS summaries are exempt).
The `# config probes ...` line in ai_trace.txt records them. Shipped INI keys: 50 (`known[]` == file, defaults == file).

## Notes / small decisions inside the design (nothing left unimplemented)
- Hot handlers: P2 = flag, `cr` null, `+0x11AC` test, `+0x10E8`, `p2Runs` (all direct loads, no tick) then the work; RAR = flag, `obj+8 == 5`, `+0x11AC`, then `GetTickCount/tick`; RS = flag + `rsSite` switch (<= 7 compares).
- A1X: `pendingA1` is set on end and on retarget as designed. A successful retarget needs no slot-6 run, so it can ORPHAN legitimately: `PR-A1X` fails only an ORPHAN (or a missing FOLLOWUP) after an `end`; ORPHANs after a retarget are counted in the summary text.
- `PR-FM2`: the design text overlaps (PASS "FM2C or FM2F", FAIL "no FM2C"); implemented as PASS = FM2C or FM2F for that creature within 30 s, FAIL = neither.
- `PR-RAH2`: OBSERVE when there is no H2 `retarget=1` line, or no H1 line carries `end52c` (older ranged-ai log); PASS/FAIL otherwise.
- `PR-SS`: inconclusive (not judged) when the creature already has 20 S6C lines (RS line budget) or the log ends inside the 60 s window.
- `PR-P2`: only creatures with >= 3 runs are judged; gaps > 30 s (area change) are not cadence samples; avg between 3500..3899 or > 4500 = OBSERVE.
- PRS RAR histogram: `type:r1/r2/r3/r4` (hex type) plus `+oN` only when results outside 1..4 occurred; `PR-RAR` FAIL = the type-0 bucket (type 0 or > 0x47) has any result other than 3.
- SHX ABORT lines are emitted with `eline` outside the per-probe budget ("every ABORT"); other SHX lines: first 30. FM3L: only logged when `fm3Lost` (<= 200); FM3X first 50 then 1/s.
- `fm2cReason` is a host-tested helper only (the FM2C line format has no reason field).
- A1 line: the format literal now ends `(wouldSpin=%u)%s` (suffix ` c524=%d f570=%d combat=%d` only with `probe_a1x`); the two old analyzer format-sync tests were re-anchored to the new literal, `A1_RE` is unchanged.
- Old analyzer tests changed only where the facts changed: companion-ai hook count 18 -> 31, healthy CLI "Checklist: 16 PASS" -> 17 (PR-RAH2 legitimately passes on the healthy fixture's ranged log).
- ObjInfo gained an `ispc` field (read with the party flag, once a second per object) so RS can track the `pc=` id.
- Not exercised at run time (static only): the perf-lab check of the P2/RAR cost is still required before the play session.
