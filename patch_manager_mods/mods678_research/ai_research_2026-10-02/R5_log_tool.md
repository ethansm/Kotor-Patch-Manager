# R5: play-log analysis tool

Tool: `patch_manager_mods/mods678_research/analyze_play_logs.py` (Python 3.12, stdlib only).
Tests: `patch_manager_mods/mods678_research/test_analyze_play_logs.py`. Fixtures: `patch_manager_mods/mods678_research/testdata/playlogs/<case>/`.

## Usage

```
cd patch_manager_mods/mods678_research
python3 analyze_play_logs.py ~/Games/KOTOR2_Mod_Workspace/playlogs_cai_<MMDD_HHMM>/
python3 analyze_play_logs.py DIR --window 14:05-14:12          # E3: restrict ai_trace / ranged_ai_log events to the noted clock time
python3 analyze_play_logs.py DIR --hooks-toml companion-ai=PATH --hooks-toml PATH2   # replaces the three default tomls (repeatable)
python3 analyze_play_logs.py DIR --anchor "2026-10-02 14:00:00=1234567"            # tick -> clock, only needed without an ai_trace header
python3 analyze_play_logs.py DIR --strict      # exit 2 when any FAIL anomaly exists
```

- It reads, if present, `ai_trace.txt`, `ranged_ai_log.txt`, `inventory_sort_filter_log.txt` and `steam-208580.log` (or `steam-*.log`) from DIR. It writes `DIR/REPORT.md` (`--out` overrides).
- It only opens files inside DIR. Copy the logs out first, as the checklist says. It never touches the live game folder.
- Other options: `--e2-window-ms` (default 3500), `--a1-strict` (require an E2 after every A1 line, not only `end`), `--attack-regex`, `--no-write`.
- Hook addresses come from the kpatch tomls (`companion_ai_src`, `ranged_ai_src`, `inventory_sort_filter_src`; the patch id comes from the sibling `manifest.toml`). Nothing is hard-coded. The tool warns if the default tomls stop having 18 (companion-ai) or 7 (ranged-ai) hooks.
- Clock: the steam log timestamps are seconds of the same `GetTickCount` clock as the `t=` field of the other logs. Wall-clock times come from the ai_trace session header (`# companion-ai ... session <date time> t=<tick>`). Without one the tool falls back to the steam log's mtime (marked approximate).

## Where the formats come from

| log | source | what is parsed |
|---|---|---|
| ai_trace.txt | `CompanionAI.cpp` `vemit`/`tline`/`eline` format strings, `design_ai_trace.md` section 6 | the P/A script lines (`:474`, `:490`), `A1` (`:617`), `E2` (`:544`), `A1 summary` (`:375`), `E1 SPIN/END` (`:420`/`:430`), `FM1` (`:695`), `FM4` (`:732`), `T1` (`:457`), `E3 P1` (`:352`), `S` (`:365`), `A4`/`A3`, config lines (`:173`/`:179`/`:232`), cap/drop/repeat markers (`:113`-`:120`), session header (`:821`) |
| script markers | `party_ai_fixes.nss`, `kite_routine.nss`, `party_ai_patches.py` | the `MARKERS` table at the top of the tool |
| ranged_ai_log.txt | `RangedAI.cpp` `logf` | `H1` (`:206`, with `los=` and `lastBlocked=`), `H2` (`:229`), `H3` (`:319`), `S1`-`S4`, `config:` |
| inventory_sort_filter_log.txt | `InventorySortFilter.cpp` `logf` | `config:`, `populate:`, `tab click:`, `flush:` (with `usable=`, `tint-candidates=`, `dropped=`), `bad frame`, `counter` |
| steam log | the saved `steam-208580.log` | `[KotorPatcher] Applied <TYPE> hook at 0x...`, `[Config] Loaded <TYPE> hook: <id> -> fn @ 0x...`, `[Config] Parsed consumed_exit_address = ... for hook at ...`, `err:` lines, `dispatch_exception code=` lines |

A unit test renders the real printf format strings found in the three C++ files and checks that the tool's regexes parse the output (`FormatsMatchSource`). If a format string in the source changes, that test breaks.

## What it checks (all implemented)

1. **Steam log.**
   - A DETOUR table with one row per toml hook: `OK`, `LOADED-NOT-APPLIED`, `MISSING` or a type mismatch.
   - The `consumed_exit_address` parse check against the tomls, with the value compared.
   - Whether `patches/<id>.dll` was loaded.
   - KotorPatcher/Config error lines (error, fail, cannot, and similar).
   - `err:` lines.
   - Exceptions by code. The benign debug-string, thread-rename and RPC codes are excluded.
   - Access violations, split into those followed by an `IsBad*Ptr` probe (handled) and those that are not.
   - "Unhandled exception" lines, and a clean-shutdown marker.
   - The wine twin lines (debugstr plus the dispatch_exception echo) are counted once.
2. **ai_trace.txt.**
   - Config lines. `# config: unparseable value` is a FAIL.
   - The T1 tid against every FM4 `tid=` (FAIL on a mismatch).
   - FM4 snapshot/restore pairing from the last cumulative `S` line (FAIL when fm4=1 and snap != restore), plus "no FollowInfo after save" and not-restored outcomes.
   - Per-creature P/A counts by script and `ev`.
   - A1 effectiveness: an `A1 ... -> end` line needs an `E2 FALLBACK` for the same creature within 3.5 s. A missing one is a FAIL.
   - `A1 summary would-spin` totals.
   - `E1 SPIN` lines while the config in force says `a1=1` for a party stance in `a1_stances`. This is a FAIL. The config is tracked line by line, so reloads are honoured.
   - FM1 keep/clear, A4, A3, the P1 calls/s table (mean, median, min and max per `p1`/`interval` state), cap/drop/repeat markers.
   - Per-creature script-fix marker table.
   - Recast anomaly: the same `Clear 2300`/`2400` signature (code, `Spells.2DA ID`, poison flag) for more than 3 consecutive rounds of one creature (`Shout Clear` lines are skipped).
   - `Party E1: skipped` for more than 30 s after the creature's last attack line.
3. **ranged_ai_log.txt.**
   - S1-S4 counts and examples.
   - H1 per stance, with `los=0`, `hold=1` and the combination.
   - The E3 answer: the stance-11 `los=0` H1 lines, restricted by `--window`.
   - H2 retargets, with `lastBlocked == me` counts.
   - H3 decisions by (ranged, inRange, kite_approach, result).
4. **inventory_sort_filter_log.txt.**
   - The config line.
   - Per cycle state (derived from the preceding `populate:` mode/extra): the flush count, creatures, rows, usable, tint candidates and dropped.
   - The cycle transitions.
   - `bad frame`, `counter`, `caption`, `overflow` and similar errors.
5. **REPORT.md.**
   - A summary (files found/missing, time span, clock anchor).
   - The pass/fail/needs-observation table for M6-1..4, M8-1..5, CAI-1..8 and E3, with `file:line` evidence.
   - The anomalies list.
   - The steam, ai_trace, ranged and inventory sections.
   - A missing file gives "not present" in its section.

Checklist statuses: **PASS** means the log shows the fix ran and nothing contradicts it. **FAIL** means the log contradicts it. **OBSERVE** means the logs cannot decide (visual) or the case did not occur. CAI-4, M6-1 and the visual halves of most items are always at best OBSERVE or "PASS (log) plus visual check".

## Test results

`python3 test_analyze_play_logs.py`: 35 tests, OK (0.4 s).

- Fixtures: `healthy`, `a1_no_e2`, `e1_spin_a1`, `fm4_tid_mismatch`, `fm4_unpaired`, `missing_detour`, `recast`, `e1_skipped_long`, `missing_files`, `unparseable_config`, `file_cap`, `inventory_errors`, plus the shared mini tomls in `_hooks/`.
- Real tomls: the 18/7/4 hook counts, a steam log built from the tomls (complete, one hook missing, `consumed_exit_address` not parsed).
- Window test: two stance-11 `los=0` H1 lines at 01:00:20 and 01:20:00; different `--window` values select each.
- Source-format tests (`FormatsMatchSource`).
- A temp COPY of `~/Games/KOTOR2_Mod_Workspace/playlogs_mods68_1001_2335/` (no `ai_trace.txt`) runs without a crash.

Head of the REPORT produced on that copy:

```
# Play-log report
| file | status | size |
| ai_trace.txt | **missing** | - |
| ranged_ai_log.txt | found | 505 B |
| inventory_sort_filter_log.txt | found | 250 B |
| steam-208580.log | found | 11168591 B |
- Session time span: steam log: tick 2781765.5..2786712.9 s (82.5 min), 22:13:01..23:35:28; ranged_ai_log: t=2781793766..2781793861
- Clock anchor: steam log mtime (approximate) = 2026-10-01 23:35:28 at tick 2786712933
- Anomalies: 1 FAIL, 0 WARN, 2 INFO
- Checklist: 0 PASS, 0 FAIL, 18 needs observation
FAIL [steam-208580.log] companion-ai: 18/18 hooks have no 'Applied' line (companion-ai.dll NOT loaded): 0x00692C50, 0x0068F301, ...
```

That saved session predates the companion-ai install, so the 18 missing companion-ai DETOURs are correct. ranged-ai 7/7 and inventory-sort-filter 4/4 show `OK`, and all 9 toml `consumed_exit_address` hooks are accounted for (6 parsed, the 3 companion-ai ones absent). The 12 access violations in that log are all followed by an `IsBadReadPtr` probe (handled).

## Known gaps

- **No real `ai_trace.txt` was available.** The ai_trace parsing is verified against the C++ format strings and synthetic fixtures only. The first real trace may show format drift in sections the unit tests do not cover.
- **"Attack line" is a heuristic.** The vanilla attack/cast print texts are not in the repo. `DEFAULT_ATTACK_RE` is `attack`, `Spells.2DA ID`, `Clear 1450`, `Clear 1[01]00`. Correct it with `--attack-regex` once a real trace shows the actual texts. When no attack line was seen, the 30 s are measured from the first `skipped` line of the run.
- **Recast round detection** uses `Clear NNNN` lines (spell id from the following `Spells.2DA ID`, poison flag from the preceding poison-check lines). The `R previous line repeated` markers carry no creature id, so repeated lines hidden by the 1 s de-dupe cannot be attributed.
- **A1 effectiveness:**
  - E2 lines are throttled to the first 20 per creature, then one per 10 s, so an A1 without E2 for a creature with more than 20 E2 lines is downgraded to WARN (inconclusive).
  - A1 lines themselves are rate-limited (10 per creature, then 1 per 2 s).
  - `retarget` and not-applied `spin` outcomes are not required to have an E2 (`--a1-strict` requires it for all).
- **Cap and drop markers:** the per-second cap drops P/A lines, so counts are lower bounds. The report says so when a drop marker exists.
- **Time handling:** window matching uses the tick-to-wall-clock anchor. No ai_trace header, no `--anchor` and no steam log means the window is ignored (the report says so). `GetTickCount` wrap (49.7 days) is ignored. The ranged log is opened in append mode and may hold several sessions. The tool warns but does not split them.
- **Checklist items whose evidence is visual** (M6-1, CAI-4, the visual halves of the others) cannot reach PASS from logs alone.
- **Steam-log error detection** is generic. The KotorPatcher source is not in the repo, so a patcher-specific failure line text is only caught by the error-word regex (`error`, `fail`, `cannot`, ...). `[Tag] ERROR/WARNING` strings from other mods are counted but not treated as failures.
- **Inventory:** H6d ("no wrongly red rows") has no direct log line. M6-3 uses "an equipment-list populate (`flush: no backpack rows`) happened with no counter/bad-frame errors" as the proxy. The panel with `log_items=0` does not log per-row tint decisions.
