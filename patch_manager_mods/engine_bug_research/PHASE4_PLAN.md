# Plan: doc-15 Phase 4: everything except the user's T1 screenshot (pre-built 1.0.1, L4 drop, probe-log analysis, loose static ends)

(Verbatim copy of the user-approved plan, 2026-10-03.)

## Context
Phase 3 (2026-10-03) is complete except one user action. The fresh-context entry is
`patch_manager_mods/15d_phase3_session_context_2026-10-03.md`; READ IT FIRST. It lists the plan state, live state, every verified fact, the probe build and the gotchas.

**Phase 3 results**
- T1 log side PASS: 12 dialogues, bar 267 / bottom y 1333, 0 errors.
- T1 visual still OPEN: the user needs one F12 screenshot during a conversation.
- **L4 is a proven no-op at 3840x1600.**
- Static triage of all KMRP topics: 0 user-visible bugs. One latent bug, L16 (movies).
- `k2-dialogue-letterbox-fix` **1.0.1-probe** is built and staged, NOT installed.

**User decisions (2026-10-03)**
- This phase = all remaining work except the user's testing:
  - pre-build the L1 **1.0.1** release so it is ready the moment T1 passes;
  - the "loose static ends" analysis.
- **Drop L4 (`k2-wrap-measure-fix`) at the next install.**
- Out of scope:
  - the upstream KPM PR (draft `engine_bug_research/upstream_kpm_k2_status_note.patch` stays a draft);
  - the L16 movie fix build;
  - GOG ports;
  - L15.

**Delegation.** Sonnet 5.5 subagents (`general-purpose`, `model: sonnet`) do the analysis, coding and drafting. Opus writes the briefs, verifies every claim against the asm/bytes, reviews every changed code line, owns the gates, the knowledge-DB writes and the commits. Brief every agent with absolute paths ("KPM = /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager", an absolute path), "write only your files", "no game launch, no writes under ~/.local/share/Steam, no git commit".

## Standing rules (carry over)
- Never launch the game.
- Installs only on the user's explicit go. Before an install:
  - re-read `$G/kpm_install_state.json` `UpdatedAt` + the id list (live on 10-03: 31 ids / 198 hooks, UpdatedAt 00:24);
  - re-stage into a NEW dir with `mods678_research/stage_and_verify_install.py stage <NEW_DIR> <kpatches>`;
  - never reuse an old stage dir (`stage_engfix_1002_0121` is stale; `stage_lbprobe_1003_0200` goes stale once the live set changes).
- `G="$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II"`.
- **Phase sequencing (memory `feedback-phase-sequencing`): never overlap a background agent with a coding step.** Order: verify, commit, then launch the next.
- Commit after each step, with explicit paths only. Never stage these sibling-owned paths:
  - `research/companion_levelup/logs/*`
  - `ranged_ai_src/script/kite_routine.nss`
  - `steam_db_pr3/*`
  - `force_power_hotbar_src/*`
- The KPM clone stays uncommitted; the repo `*_src/` mirrors are the committed copies.
- Ghidra: use the dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` only (no headless run is needed this phase).
- Run `python3 scripts/check_kpm_updates.py --quiet` (repo root) at session start; silent = nothing new.

## Step 0: persist this plan
Copy this plan verbatim to `patch_manager_mods/engine_bug_research/PHASE4_PLAN.md`. Add a pointer line to the top of 15d section 5 and to the MEMORY.md "engine-bug candidates" line. Commit.

## Step 1: record the L4 drop decision (Opus, docs only)
- **Register row L4** (`16_candidate_register.md`): "user decision 10-03: DROP at the next install (no-op at fs 1.0/2.0/3.0, V8); source mirror `wrap_measure_fix_src/` kept; still valid for fs 1.5 (community K6)".
- **Doc 17:**
  - add the decision to STATUS;
  - update the Install section: the id list = live ids MINUS `k2-wrap-measure-fix`;
  - update Rollback: L4 can be re-added by re-including the id from `Patches/K2WrapMeasureFix/K2WrapMeasureFix.kpatch`.
- **`K1_ENGINE_BUGS_IN_K2.md` K6 row:** "the fix is correct but has no effect at fs 1/2/3; this install does not carry it". Keep the caveats.
- **15d:** sections 2 and 5.
- Commit.

## Step 2: loose static ends: analyst V11 (1 Sonnet analyst, read-only; runs ALONE, before any coding step)
Append brief **V11** to `engine_research_findings/V_BRIEFS.md`. Use the same "Phase 3 briefs" header rules: CONTRACT, sections (a)-(f), KMRP docs in `~/Games/KOTOR2_Mod_Workspace/kmrp_docs/` as LEAD only, constants from `~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe`, live-hook overlap vs `mods678_research/live_hooks_2026-09-30g.tsv` + the live `$G/patch_config.toml`.

The analyst writes ONE file, `engine_research_findings/V11.md`. Prompt template = the V8b launch (read CONTRACT -> V_BRIEFS header -> Phase 3 header -> own section; read V8b/V9/V10 + their Opus notes first).

Questions:
1. **Letterbox globals.**
   - Find every reader of `0x00A309B4` (top-bar h, written at 0x8BB467), `0x00A309B8` (bottom-bar y, 0x8BB531) and `0x00A309BC` (dialog-active byte, 0x7CEB9C). Grep the asm for `0x00a309b4]` etc. and the bytes `b409a300` / `b809a300` / `bc09a300`.
   - For each reader: the function, its role (subtitle / NPC text placement, 3D viewport or scissor, post-process composite, HUD hiding?), and what changes now that the top bar is 267 instead of 0 and the bottom y is 1333 instead of 1400.
   - Verdict: does L1 move or clip anything else? (BUG / NO BUG / UNCLEAR, evaluated at 3840x1600.) This is the one question that could reveal an L1 side effect, so it gets the most care.
2. **V10 white first draw (UNCLEAR).** Trace the fill-only first draw of a GUI border/texture whose TPC has not been built yet:
   - `CSWGuiBorder::Draw` 0x415300 -> 0x47EA60 (quad-batch flush, drains pending builds via 0x427F30 / 0x427810);
   - can a quad be emitted with texture id 0 / a white placeholder before the build completes?
   - Verdict + the deciding quote.
3. **V9 open (d): Bink negative offset.**
   - At 3840x1600 with a 3440x1440 movie the y offset is (1600-1607)/2 < 0. Does K2 pass a negative offset to `BinkBufferSetOffset`, or clamp it?
   - What does the `0x5D000000` buffer-open flag mean (Bink SDK flag names if derivable from strings/imports)?
   - What shows in the unpainted window area (class brush NULL)?
   - Verdict: is anything visible today?

For any BUG: a hook window (CONTRACT rules, whole-dump branch-target grep, `original_bytes`) + a one-line fix sketch. No code.

**Opus gate (B3 pattern):**
- re-check every BUG/UNCLEAR claim + 10 random quoted asm lines against `decomp/*.asm`;
- read constants with `mods678_research/read_exe_consts.py`;
- add any BUG window to `engine_bug_research/tools/check_engfix_windows.py` and run it;
- append "Opus verification (<date>)" to V11.md.

Commit V11 + the brief.

If V11 Q1 finds a real L1 side effect (e.g. subtitles pushed off-screen or clipped by the 267 px bar), STOP and ask the user (AskUserQuestion) before Step 3. A3 needs a design choice then: bar fraction, `mode=clamp`, or a follow-up hook.

## Step 3: pre-build L1 1.0.1 (1 Sonnet coder; Opus reviews every changed line). Built + staged, NOT installed.
Plan-3 A4, done ahead of the screenshot so the install is ready when T1 passes.

**Source:** `patch_manager_mods/dialogue_letterbox_fix_src/` (`K2DialogueLetterboxFix.cpp`, `kotor2-steam-aspyr.hooks.toml`, `manifest.toml`, `exports.def`, `smoke_dialogueletterbox.cpp`, `README.md` with the build + smoke command block). Build dir `KPM/Patches/K2DialogueLetterboxFix/`.

**Before building:** copy the 1.0.0 `K2DialogueLetterboxFix.kpatch` to `~/Games/KOTOR2_Mod_Workspace/backups/k2_dialogue_letterbox_fix_1.0.0/` (rollback artifact; md5 `6033b7003c32caddce5933a3daa8139f`).

**Changes (1.0.0 hooks byte-identical; the probe hooks are NOT included):**
1. **Default `log=0`.** Currently `bool g_log = true;` at cpp line 38; the INI key at line 117 is unchanged.
2. **When `log=0`, also write:**
   - the FIRST `APPLY ...` header + result pair per process (one line pair per launch, so the analyzer can still check the bar);
   - the FIRST `SETTOP`/`SETBOTTOM` pair per process.
   Today the gate at lines 246/251 is `if (!g_log && (g_lateDone || g_hits != 0)) return;`. Replace it with a per-process "first dialogue logged" flag. The LATE fallback behaviour is unchanged. Problem lines (INVALID/FOREIGN/protect/MISMATCH/null) stay forced.
3. **Version** `1.0.1`: `kVersion` (line 27), `manifest.toml` `version`, README.
4. **Smoke harness:**
   - extend the `log0` mode (lines 89/199-206) to the new semantics: one APPLY pair, then the first SETTOP/SETBOTTOM pair logged and a second dialogue NOT logged;
   - add a `default` mode (no INI -> log=0 behaviour);
   - all modes must pass under Wine with the README command block (throwaway prefix under `/tmp/claude-1000/`).
5. **Build:**
   - `cd KPM/Patches/K2DialogueLetterboxFix && python3 ../create-patch.py`;
   - report the kpatch/DLL md5 and the imports (KERNEL32 + msvcrt only);
   - mirror byte-identical to `dialogue_letterbox_fix_src/` (`cmp`).
6. **Windows:**
   - run `engine_bug_research/tools/check_engfix_windows.py` (TOML cross-check; owner L1 = whichever mirror the probe step set; make sure the 1.0.1 mirror's 3 hooks are checked) and save the output as `check_engfix_windows_output_<date>.txt`;
   - run `python3 mods678_research/verify_windows_both_exes.py <that output>`;
   - required: ALL OK + PROXIMITY OK + ALL MATCH.
7. **Analyzer:** run `engine_bug_research/tools/analyze_engfix_logs.py` on the smoke output logs of the `default` and `log0` modes. It must PASS. If the analyzer warns or fails on the new one-dialogue-per-launch log=0 format, the Step 4 coder fixes it.

**Opus review:**
- every changed cpp line;
- the hooks TOML is identical to 1.0.0 (diff);
- the smoke output;
- the checker output.

Commit the mirror + checker output.

## Step 4: analyzer extension for the probe branch + the 1.0.1 format (1 Sonnet coder; Opus review). After Step 3 is committed.
Extend `engine_bug_research/tools/analyze_engfix_logs.py` + `test_analyze_engfix_logs.py`:
1. **Parse `PROBE W1..W5` lines.** The formats are in `dialogue_letterbox_fix_probe_src/K2DialogueLetterboxFix.cpp` lines 346, 394, 404, 427, 440, and the README. Report per dialogue (split at SETTOP):
   - W1: top-bar h progression and final h (expect a rise to 267, `done=1`);
   - W2: bottom y progression (1600 -> 1333);
   - W3: predicate tally (`WOULD-DRAW` vs each `FAIL:*` term), vtable (letterbox/OTHER), alpha bits (0x3F000000 = 0.5 with semi-transparent-letterbox, 0x3F800000 = 1.0);
   - W4: eax=1 count vs eax=0 count for the letterbox;
   - W5: arg 1 at start / 0 at end.
   Then give a one-line diagnosis that maps to the V8b expectations (V8b section (c) "What each logs"):
   - "bars drawn (fill issued)";
   - "Draw never called on the bar" (no W1);
   - "predicate fails: <term>";
   - "fill rejected (eax=0)";
   - "alpha 0".
2. **1.0.1 `log=0` format:** one APPLY pair + one dialogue per launch = PASS. The `probe=` field in the `loaded:` line is accepted.
3. **Tests:**
   - synthetic PROBE logs for each diagnosis branch (build them from the smoke `probe` mode output format in `dialogue_letterbox_fix_probe_src/smoke_dialogueletterbox.cpp`);
   - a 1.0.1 log=0 case;
   - all existing 22 tests still pass.
Commit.

## Step 5: stage both branches against the CURRENT live set, without L4 (Opus; read-only on the game dir)
1. Re-read `UpdatedAt` + the id list.
2. Stage **A** (T1 pass): `stage_and_verify_install.py stage ~/Games/KOTOR2_Mod_Workspace/stage_l1_101_<MMDD_HHMM> KPM/Patches/K2DialogueLetterboxFix/K2DialogueLetterboxFix.kpatch`.
3. Stage **B** (no bars): the same with `KPM/Patches/K2DialogueLetterboxFixProbe/K2DialogueLetterboxFixProbe.kpatch` into `stage_lbprobe_<MMDD_HHMM>`.
4. Expected PROBLEMS: only the known `4gb-patch` hooks=DIFF artifact.
5. Write both install commands into doc 17 "Install" (`KPATCH_NO_LAUNCH=1 .../KPatchLauncher "$G/swkotor2.exe" --patches "$STAGE" <ids>`):
   - id list = the live ids minus `k2-wrap-measure-fix`, with `k2-dialogue-letterbox-fix` from the stage;
   - pre-install backup dir `~/Games/KOTOR2_Mod_Workspace/backups/pre_l1_101_<date>/` (`kpm_install_state.json`, `patch_config.toml`, `patches/`);
   - post-install `stage_and_verify_install.py verify <STAGE>` + the `swkotor2.exe` md5 check.
6. Note in doc 17 that both stage dirs go stale on any sibling reinstall (re-stage at install time).
7. Commit doc 17.

## Step 6: persist (Sonnet drafts; Opus edits and commits)
1. Register:
   - V11 results as notes on L1 (Q1), L23 (Q2) and L16/L17 (Q3), or a new row if a new bug appears;
   - the hand-off summary gets a "Phase 4" paragraph;
   - the A3 decision table copied into the register's T1 card (pass -> install stage A; no bands -> install stage B, one conversation, run the analyzer's probe diagnosis).
2. Knowledge DB: new idempotent `engine_bug_research/tools/persist_phase4_db.py`.
   - Pattern = `persist_phase3_db.py`: a `--db` option, upsert by topic, names only for unnamed addresses, separate "unchanged" count.
   - Test it twice on a scratchpad copy first, then twice on the real DB (the second run adds 0).
   - Commit the DB.
3. `K1_ENGINE_BUGS_IN_K2.md`: add any V11 result that changes a row.
4. 15d -> write a new `15e_phase4_session_context_<date>.md` (same structure as 15d) and point 15d + MEMORY.md at it.
5. Memory:
   - MEMORY.md "engine-bug candidates" line;
   - `kotor2-patch-manager.md` dated paragraph.
6. Final `git status`: only intended paths.

## After the user's screenshot (not this phase's work, but the plan must leave it ready)
Decision table (A3 from Phase 3):
- **Top band visible** (≈267 px dim band; enabled=0 run, if done, shows none at the top and 200 px at the bottom):
  1. ask for the install go;
  2. re-stage A;
  3. install (L4 dropped);
  4. verify;
  5. mark L1 "accepted <date>" in doc 17 / register / `K1_ENGINE_BUGS_IN_K2.md` caveat 2.
- **No bands:**
  1. ask for the install go;
  2. re-stage B;
  3. install;
  4. the user sets `probe=1` in `$G/dialogue_letterbox_fix.ini` (game closed) and has one conversation;
  5. copy the log to `playlogs_engfix_<MMDD_HHMM>/`;
  6. run the analyzer (probe diagnosis);
  7. Opus designs the fix, asks the user, then a Sonnet coder builds it.
- **Panel overlap / bars look wrong:** as in the Phase 3 A3 table (`PHASE3_PLAN.md`).

## Critical files
- Docs:
  - `patch_manager_mods/15d_phase3_session_context_2026-10-03.md`
  - `17_dialogue_letterbox_and_wrap_fixes.md`
  - `16_candidate_register.md`
  - `K1_ENGINE_BUGS_IN_K2.md`
  - `engine_bug_research/PHASE3_PLAN.md` (A3 table)
- Code:
  - `dialogue_letterbox_fix_src/K2DialogueLetterboxFix.cpp` (+ smoke, toml, manifest, README)
  - `dialogue_letterbox_fix_probe_src/` (probe formats)
  - `engine_bug_research/tools/analyze_engfix_logs.py` + `test_analyze_engfix_logs.py`
  - `engine_bug_research/tools/check_engfix_windows.py`
  - `engine_bug_research/tools/persist_phase3_db.py` (pattern)
  - `mods678_research/stage_and_verify_install.py`, `verify_windows_both_exes.py`, `read_exe_consts.py`
- Analysis: `engine_research_findings/CONTRACT.md`, `V_BRIEFS.md`, `V8b.md`, `V9.md`, `V10.md`.

## Verification (end of phase)
1. V11.md has an Opus verification note; any BUG window passes `check_engfix_windows.py` (ALL OK + PROXIMITY OK, both exes MATCH).
2. 1.0.1:
   - all smoke modes pass (incl. the new `default` and the updated `log0`);
   - the hooks TOML is identical to 1.0.0;
   - checker ALL OK + PROXIMITY OK + ALL MATCH;
   - the mirror is byte-identical to the build dir;
   - the 1.0.0 kpatch is backed up.
3. `python3 engine_bug_research/tools/test_analyze_engfix_logs.py`: all tests pass (22 old + new probe/1.0.1 cases). The analyzer gives PASS on the 1.0.1 smoke logs and the right diagnosis on each synthetic probe log.
4. Stage A and stage B exist with PROBLEMS = only the 4gb-patch artifact. Doc 17 has both install commands without `k2-wrap-measure-fix`.
5. The game dir is untouched: `swkotor2.exe` md5 `c9a1e3c57fd5491b4256dac1c0ab97cb`; `kpm_install_state.json` UpdatedAt unchanged by this session.
6. The DB second run adds 0. 15e + MEMORY.md + register committed. `git status` shows only intended paths.
