# companion-ai 0.2.0-probe: build note (2026-10-02)

Static build only: nothing installed, the game never launched, nothing written in the game folder, DB not written, nothing committed.
Design followed: `PROBE_DESIGN.md` sections 1-8 (13 plain-detour windows + the 2 handler-only probes PARTY and A1X; no consumed exit added).

## Artifacts
| item | value |
|---|---|
| version | `0.2.0-probe` (CompanionAI.cpp `Version`, manifest.toml, ini header, toml comment) |
| build dir | `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/CompanionAI/` |
| new kpatch | `CompanionAI.kpatch`, md5 `4b2758c1c9781f8d80fa24ae72572ddf` (113385 B; the zip md5 changes with every rebuild). **Rebuilt 2026-10-02 after the Opus review fixes (below); the first build was `a631f2d2` / DLL `aef7c42e`, superseded.** |
| new DLL (`binaries/windows_x86.dll`) | md5 `f791599162dac1398eab4712b9d25179`, 29 exports |
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

## Review fixes applied (REVIEW_probe_build.md; toml / hook windows / exports untouched, the toml and manifest are byte-identical to HEAD)
- **SF1** (CompanionAI.cpp `CompanionAiProbeShout`): speaker type read first (`stype`), `party` read only for creatures; SHX line has ` type=%d`; ABORT lines are capped (first 100, then 1/s per kind via `LineThrottle`) instead of unbounded `eline`; `abortStart` split into `abortStartCr` / `abortStartOther` in the PRS SHX line.
- **SF2** (A1 gate + `summary`): STRICT is counted per creature (`strictN`/`strictRej`, reset per A1 summary) and its line is throttled (first 10, then on an ok/would-reject change or every 5 s); the `A1 summary` line ends ` strict=N/rejected`; FOLLOWUP/ORPHAN use their own `ProbeCap` `PiA1xF` (name `A1XF`, `PiCount` 14).
- **SF3** (RS handler, `probeSummary`, analyzer PR-E9): E9 deliveries (pat 15, non-party creature speaker, party listener) are counted in `g_e9` before any line budget and printed as `PRS E9 deliveries=%u` (first summary even at 0, then on change); S7 lines carry ` spkTag=%s`; S7 PR lines = first 10 per creature (50/session) plus every E9-case line (300/session, own counter). Analyzer: PASS = PRS E9 > 0 (tags listed), FAIL = SHX non-party r=35 creature speakers (`type=5`) and `PRS E9 deliveries=0`, else OBSERVE.
- **SF4** (analyzer): `budget_ln`/`budget_t` per probe and the per-second-cap drop markers are parsed; PR-FM2 (FM2C, FM2F), PR-SS (RS), PR-A1X (A1XF) judge an absence only while that budget held for the whole window and no drop marker overlaps it, else the event is not judged (OBSERVE / "N not judged"). PR-SS judges from the budget-free `PRS RS` s6c counters when the log has any (a rise within 60 s + one summary period). PR-E9 uses the budget-free PRS E9 count, so it needs no marker rule.
- **SF5** (analyzer PR-SHOUT): FAIL on `abortStartCr + abortMid > 0` or ABORT-START with `type=5` / any ABORT-MID; `abortStartOther` is INFO.
- **N5** partyEmit returns when `[server+4]` is 0. **N7** `rarFormat` always appends the type-0 bucket when non-zero. **N3** `a1xFollowup` for the slot-6 sites now runs before the `objPasses` filter. **N6 (comment only)** the wrong "why=pf lines are mid-save" comment in the analyzer. **N2** design text fixed (pairBump gap window). **N1** documented in PROBE_DESIGN (the throttle signature is n). **N9** PROBE_DESIGN section 9: perf-lab A/B step 0, party-selection swap 2b, explicit area change 3b, companion_ai.ini in the copy-out. Not applied: N4, N6 (judge `load` after 2 s), N8 (vemit flush change, not trivial).

## Gate results (after the review fixes)
0. Rebuild gates: host test **2436 checks, 0 failed / ALL PASS** (no new pure helper was added: the fixes are handler logic and analyzer rules); `python3 -m unittest test_analyze_play_logs` **53 tests OK** (51 + 2 new, plus rewritten E9 / SHOUT cases and the SF4 budget / drop-marker cases); `check_windows.py` ALL OK, 0 `WARN`, TOML cross-check passes; mingw `-fsyntax-only -Wall -Wextra` clean; exports == toml functions (31 hooks / 29 names / 29 DLL exports); kpatch toml + manifest `cmp` identical to the source. The numbers below are the first build's, kept for the record.

### First-build gate results
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
The `# config probes ...` line in `companion-ai_log.txt` (`ai_trace.txt` on an old core) records them. Shipped INI keys: 50 (`known[]` == file, defaults == file).

## Notes / small decisions inside the design (nothing left unimplemented)
- Hot handlers: P2 = flag, `cr` null, `+0x11AC` test, `+0x10E8`, `p2Runs` (all direct loads, no tick) then the work; RAR = flag, `obj+8 == 5`, `+0x11AC`, then `GetTickCount/tick`; RS = flag + `rsSite` switch (<= 7 compares).
- A1X: `pendingA1` is set on end and on retarget as designed. A successful retarget needs no slot-6 run, so it can ORPHAN legitimately: `PR-A1X` fails only an ORPHAN (or a missing FOLLOWUP) after an `end`; ORPHANs after a retarget are counted in the summary text.
- `PR-FM2`: the design text overlaps (PASS "FM2C or FM2F", FAIL "no FM2C"); implemented as PASS = FM2C or FM2F for that creature within 30 s, FAIL = neither.
- `PR-RAH2`: OBSERVE when there is no H2 `retarget=1` line, or no H1 line carries `end52c` (older ranged-ai log); PASS/FAIL otherwise.
- `PR-SS`: inconclusive (not judged) when the creature already has 20 S6C lines (RS line budget) or the log ends inside the 60 s window.
- `PR-P2`: only creatures with >= 3 runs are judged; gaps > 30 s (area change) are not cadence samples; avg between 3500..3899 or > 4500 = OBSERVE.
- PRS RAR histogram: `type:r1/r2/r3/r4` (hex type) plus `+oN` only when results outside 1..4 occurred; `PR-RAR` FAIL = the type-0 bucket (type 0 or > 0x47) has any result other than 3.
- SHX ABORT lines are emitted with `eline` outside the per-probe budget but capped (first 100, then 1/s per kind; SF1); other SHX lines: first 30. FM3L: only logged when `fm3Lost` (<= 200); FM3X first 50 then 1/s.
- `fm2cReason` is a host-tested helper only (the FM2C line format has no reason field).
- A1 line: the format literal now ends `(wouldSpin=%u)%s` (suffix ` c524=%d f570=%d combat=%d` only with `probe_a1x`); the two old analyzer format-sync tests were re-anchored to the new literal, `A1_RE` is unchanged.
- Old analyzer tests changed only where the facts changed: companion-ai hook count 18 -> 31, healthy CLI "Checklist: 16 PASS" -> 17 (PR-RAH2 legitimately passes on the healthy fixture's ranged log).
- ObjInfo gained an `ispc` field (read with the party flag, once a second per object) so RS can track the `pc=` id.
- Not exercised at run time (static only): the perf-lab check of the P2/RAR cost is still required before the play session.

## Logging (0.2.1-probe, KpLog)
- Files: core `companion-ai_log.txt` in the game dir. On an old core (no `KPatchInit`) the fallback is `ai_trace.txt` at INFO, appended per session (split on the `# companion-ai <ver> session` line).
- Lines: `T=<s.ms> f=- companion-ai/<main|trace|probe> <old text>`; text and field layout unchanged. Channels: `main` = fix events, summaries, config, gate markers; `trace` = deduped trace; `probe` = PR/PRS lines. The A4 NULL spell-row line is WARN (visible without kplog.ini).
- Enable: `[companion-ai]` section in `kplog.ini` with `enabled=1`, `level=info`, `channels=*`.
- Flush: KpLog flushes every 500 ms by default, so a crash can lose up to 0.5 s. `flush_ms` in companion_ai.ini is parsed but unused; for crash work set `flush_ms=0` in `kplog.ini` `[global]`.
- Reader: `analyze_play_logs` reads the core file, the fallback and the legacy raw file. The core cuts a message at 232 bytes, so lines longer than ~224 bytes (S, PRS, `# config`) are written as pieces: the first as-is, the rest `t=<ms> + <rest>` (`++` = cut mid-word); `read_trace_lines` re-joins them. A tool that greps the raw file sees the pieces.
- Install: new export `KPatchInit` (29 -> 30), so use the full `python3 scripts/kpm_reinstall.py --mod companion-ai` (hot-swap `--apply` refuses export changes). Built and unit-tested 2026-10-15, NOT installed.
