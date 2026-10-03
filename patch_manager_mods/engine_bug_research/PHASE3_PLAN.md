# Plan: doc-15 Phase 3 — validate + finalize L1/L4 from the user's T1/T2, and a static triage round of the unexamined KMRP topics

## Context
Phase 2 (2026-10-02) built and installed two engine fixes, which have been live since 01:33:
- `k2-dialogue-letterbox-fix` 1.0.0 (L1, KMRP K7): bar = H/6, so 267 px at 3840x1600 instead of -22.
- `k2-wrap-measure-fix` 1.0.0 (L4, KMRP K6): glyph advance rounding +0.25 -> +0.5.

The L1 log proves the computation works: in the 10-03 01:12 session, 12 dialogue starts all have bar target 267 / bottom y 1333 and there are 0 errors. The **visual result is unknown**: the user "didn't notice / no bars" and took no screenshots. T2 has not been run. L4 has no log and no switch.

**Step 0 also records this T1 finding:** add it to doc 17 STATUS and the register row L1 ("computation PASS 10-03, visual unknown"), and to `15c` section 2. Commit.

User decision (2026-10-03): the next phase =
- (A) validate and finalize L1/L4 from the user's test cards;
- (B) a **static triage round** of the KMRP K1 topics never checked against K2.

Out of scope: the upstream KPM PR, the L15 fix, and a GOG port.

Delegation: **Sonnet 5.5 subagents** do the routine work (analysis sweeps, log analysis, coding, doc drafting). Opus writes the briefs, verifies every claim against the asm / bytes, owns the gates, the live writes and the commits.

### Read first after the context clear
1. `patch_manager_mods/15c_phase2_session_context_2026-10-02.md`: owned files, live state, verified facts, gotchas.
2. `17_dialogue_letterbox_and_wrap_fixes.md`: hooks, INI, log format, test plan, rollback.
3. `16_candidate_register.md`: register, test cards T1-T6.
4. `K1_ENGINE_BUGS_IN_K2.md`: the community table this phase extends.
5. `engine_research_findings/CONTRACT.md` + `V_BRIEFS.md`: analyst rules; V7 is the newest brief.
6. `engine_bug_research/kmrp_reference_notes.md`: KMRP re-fetch commands and the list of topics not yet examined.

### Standing rules
- Never launch the game.
- Installs only with the user's explicit go. Before any install:
  - re-read `kpm_install_state.json` `UpdatedAt` and the live id list (sibling sessions reinstall at any time; live = 28 ids / 186 hooks as of 10-02 11:36);
  - re-stage with `mods678_research/stage_and_verify_install.py stage <NEW_DIR> <kpatches>`. `stage_engfix_1002_0121` is STALE: never use it.
- Commit after each step, with explicit paths only. Never stage `research/companion_levelup/logs/*` or `ranged_ai_src/script/kite_routine.nss`.
- The KPM clone stays uncommitted (the repo `*_src/` mirrors are the committed copies). This includes the local KOTOR II status notes in `Patches/K1StrayBugFixes/README.md` and `K1WidescreenPatch/KMRP-ENGINE-FIXES.md`.
- Ghidra: use the dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`. If a headless run is unavoidable: `pgrep` first, `-readOnly`, and query `ghidra_knowledge/` first.

## Step 0 (first action after approval)
Copy this plan verbatim to `patch_manager_mods/engine_bug_research/PHASE3_PLAN.md`; `15c` and memory already point there. Commit it.

## Track A — L1/L4 validation and finalization

### A1. Offline log analyzer (Sonnet coder; Opus review). Do this BEFORE the user plays.
New file `engine_bug_research/tools/analyze_engfix_logs.py`, read-only. It is pointed at a copy of the logs, never at the live files.

It parses `dialogue_letterbox_fix_log.txt` (formats in doc 17, "Config and log") and reports per launch (split on `loaded:` lines):
- **Load line:** mode, enabled, start bytes. FAIL if the start bytes are not vanilla `00 00 00 80 aa aa 02 40`.
- **APPLY lines:** W, H, newBar, `verify OK`.
- **Error lines:** any `FOREIGN`, `INVALID`, `protect`, `MISMATCH` or `null` line is a FAIL.
- **SETTOP:** each line's target h must equal the newBar of the last APPLY (267 at 3840x1600) and its start h must be 0.
- **SETBOTTOM:** target y must be H - max(bar, 100*fs), i.e. 1333 at 3840x1600. Use the same fs formula as the DLL: `1 + 0.5*floor(max(W-1022,0)/1024)`.
- **LATE fallback:** report its use.
- **Counts:** dialogues = number of SETTOP lines; plus a cap-reached flag.

Output: a PASS/FAIL table plus a one-line verdict.

Fixture test: `engine_bug_research/tools/test_analyze_engfix_logs.py`, using
- the real 12-line log from 10-02 02:38, copied into `engine_bug_research/fixtures/letterbox_log_1002_0238.txt`, which must give "no dialogue yet";
- synthetic logs built from the h6 smoke output lines (the format is in `dialogue_letterbox_fix_src/README.md`): a pass case, a FOREIGN case and a wrong-bar case.

### State at plan time (2026-10-03 ~01:15, verified read-only)
The user played on 10-03 (log mtime 01:12). The game log now has 60 lines; save a copy of it in A1 as fixture `letterbox_log_1003_0112.txt`. It shows:
- 4 launches, each with `APPLY ... newBar=267 ... verify OK`;
- **12 SETTOP/SETBOTTOM pairs** (dialogue starts), all with target `(0,0,3840,267)` / `(0,1333,3840,267)` and start h 0 / start y 1600;
- 0 error lines.

**The computation side of T1 PASSES.**

The user's visual answer was "**Didn't notice / no bars**". No screenshots were taken: the Steam screenshot dir has none for 208580. So the visual result is UNKNOWN, and "no bars drawn despite a 267 target" is possible. V1 B5 marked the draw path SPECULATIVE: Draw 0x8BB260 slides start -> target and calls the vtable +4 SetExtent; what the control then renders, and whether the panel is shown, was never read. T2 has not been checked.

### A2. Disambiguate T1 visually (user, next launch) AND pre-build a draw-path probe offline (in parallel; do not install it unless the bars are absent)
**User checklist** (give it verbatim):
1. Start a conversation.
2. Press **F12** (Steam screenshot) during an NPC line and again while the replies are shown. Screenshots land in `~/.local/share/Steam/userdata/62703561/760/remote/208580/screenshots/`.
3. Note whether dark bands are visible at the top and bottom. With `semi-transparent-letterbox` they are 50 % black, so they are subtle on dark scenes: use a bright area, e.g. Peragus or Telos interiors.
4. A/B without reinstalling: set `enabled=0` in `$G/dialogue_letterbox_fix.ini` (game closed), start the same conversation, F12, then set `enabled=1` again. Vanilla bars are -22 at 3840x1600, so vanilla shows NO bars: a visible difference between the two runs = the fix works.
5. T2: a long Journal entry or item description next to a scrollbar. F12.
6. Optional: T3 (icon/datapad popup + quit dialog), T4 (full map, M key), T5 (Soft Shadows on/off FPS).

After the user closes the game:
- Copy `$G/dialogue_letterbox_fix_log.txt` and the screenshots to `~/Games/KOTOR2_Mod_Workspace/playlogs_engfix_<MMDD_HHMM>/`.
- Run A1 on the copy, and read the screenshots with the Read tool (images).

**Offline probe build (only staged; install only if the screenshots show no bars):**
1. **Sonnet analyst `V8b.md`** (read-only, CONTRACT rules) traces the draw path of `CSWGuiDialogLetterbox`:
   - Draw 0x8BB260: the cases for `[this+0x88]` 1/2/3, the slide step helpers 0x8BB550 / 0x8BB590, and the vtable +4 SetExtent target;
   - the render method of the control: what it fills, with which alpha `[this+0x50]` and colour;
   - how the panel is shown or hidden: manager show/hide 0x410530 flags `+0x48`, the active flag `DAT_00a309bc` set by 0x7CEB80, and the caller 0x7BFB10 gate `DAT_00a7fd30`;
   - whether a negative or clamped rect, or the semi-transparent-letterbox alpha writes (0x8BAA5E / 0x8BB043 / 0x8BB108 / 0x8BB3C3), could make the bars invisible.

   It proposes log-only windows:
   - after the Draw SetExtent call: log the rect actually set, alpha, `[this+0x88]` and the frame count;
   - in the control render: was a fill issued, with which rect;
   - the 0x7CEB80 argument.

   Each window needs `original_bytes` and the parameter source.
2. **Opus gate** on V8b. Then a **Sonnet coder** builds `k2-dialogue-letterbox-fix` **1.0.1-probe**: the same DLL plus these log hooks, rate-limited to the first 120 frames per dialogue and then 1 line/s, all gated by a new INI key `probe=1`.
   - Smoke harness: add a probe mode.
   - Checker: add the windows to `engine_bug_research/tools/check_engfix_windows.py`.
   - Both exes: `verify_windows_both_exes.py`.
   - Stage against the CURRENT live set. Do NOT install unless the A2 result is "no bars" and the user says go.

### A3. Decision gate on the results (Opus; ask the user with AskUserQuestion when a branch is a design choice)

| Result | Action |
|---|---|
| **T1 pass** (bars visible ~267 px; the enabled=0 vs 1 screenshots differ; replies inside the bottom bar; NPC text inside the top bar; no error lines) | Go to A4 (1.0.1 = trim). Archive the unused 1.0.1-probe build. |
| **No bars in either run** (screenshots identical) | Ask the user for the install go, then install 1.0.1-probe (re-stage first). One conversation. Analyse the probe log with V8b's expectations. Then a fix design: Opus designs, asks the user, then a Sonnet coder builds it. |
| **T1 issue: reply panel / NPC label overlap or misplacement** | Design a fix with the K1-style reply-panel handling as the starting point (KMRP K7: panel immediates -> `bar - 36`, min 100; `LB_REPLIES` stretched to the panel in `CSWGuiDialogCinematic::SetExtent` 0x8BC050). Sites: the Cinematic ctor 0x8BBA80 and Reset 0x8BC3E0 compute `y = min((X+H)/2, H - 100*fs)` and `h = trunc(100*fs)` (V1 B3). A Sonnet analyst writes `V8b.md` with the windows. Then Opus designs and asks the user, and only then builds (same S3 pattern: DLL + smoke + checker). |
| **Bars look wrong** (e.g. too tall at 267 / H/6) | Offer `mode=clamp` (bar 0) or a configurable bar fraction (new INI key `bar_frac=6`, default 6). Ask the user. |
| **T2 shows no visible difference** | Keep L4 (harmless, matches the community fix) and record "cosmetic, not visible at 3840x1600". If the user prefers fewer patches, remove it from the live set at the next install. |
| **T2 regression** (text now wraps too early or clips) | Remove L4 at the next install (ask the user). Rollback = reinstall without `k2-wrap-measure-fix`. |

### A4. Finalize L1 1.0.1 (Sonnet coder; Opus reviews every line). Only after T1 passes.
Follows `feedback-log-thoroughly`: exhaustive logging first, trim once accepted.
1. Default `log=0`: only the load line and the problem lines are written (that behaviour already exists).
2. The SetTop/SetBottom log hooks: **keep them** (they also carry the LATE fallback), but log only the first dialogue per launch when `log=0`.
3. Version `1.0.1` in `manifest.toml`, `kVersion` and the README.
4. Mirror the changes to `dialogue_letterbox_fix_src/`, rebuild with `create-patch.py`, re-run all 9 smoke modes (README command block) and extend the `log0` mode for the new default.
5. Run `engine_bug_research/tools/check_engfix_windows.py` (TOML cross-check) and `mods678_research/verify_windows_both_exes.py engine_bug_research/tools/check_engfix_windows_output_<date>.txt`.
6. Re-stage against the CURRENT live set, then ask the user for the install go. If yes: run the install procedure (doc 17 "Install", with a fresh dated backup), then `stage_and_verify_install.py verify`.
7. Update doc 17 STATUS, the register rows L1/L4 ("accepted <date>"), `K1_ENGINE_BUGS_IN_K2.md` caveat line 2 ("visible effect confirmed in-game"), and memory.

## Track B — static triage of unexamined KMRP topics. Runs while waiting for the user's play session.
Never overlap a Track-B agent with a Track-A coding step.

### B0. Fetch the KMRP sources (Opus, read-only network)
Commands are in `kmrp_reference_notes.md`. Fetch:
- `reverse-engineering/movies.md`, `text-padding.md`, `map-markers.md`, `area-map-surface.md`, `font-atlases.md`, `button-focus-badge-geometry.md`, `texture-residency.md`;
- the K3 section of KPM PR #286 `K1StrayBugFixes/README.md`. This one is local already in the clone.

Save them to `~/Games/KOTOR2_Mod_Workspace/kmrp_docs/` (outside the repo: third-party text, not committed). Record the fetch date and commit sha in `kmrp_reference_notes.md`, and commit that note.

### B1. Briefs V8-V10 (Opus; append to `engine_research_findings/V_BRIEFS.md`, same rules as V1-V7)
Each brief = the K1 mechanism summary (from B0) + the K2 starting points + the questions + an evaluation at 3840x1600 (the user's resolution).

**V8 — text and fonts**
- K3 leading newline: K2 `CSWGuiText::SetText` 0x416E30. Does K2's description builder prefix `\n`, and does SetText trim it?
- `text-padding.md`: K1 listbox client rect `width -= scrollbarWidth`; the K1 site is around 0x415E0D. Find the K2 equivalent through the bridge `engine_bug_research/k1_k2_bridge.tsv`.
- `font-atlases.md`: font texture/TXI handling, and whether K2 reads `spacingR` in Draw and in the breaker (V3 noted an inconsistency).

**V9 — movies and maps**
- `movies.md`: Bink fullscreen size / aspect fit; the K1 hook is at 0x4057AC. Is a 3440x1440 rescaled `.bik` (K2RS, see `LEGACY_MOVIES_UPSCALING_PIPELINE.md`) letterboxed or pillarboxed or stretched at 3840x1600? Find the K2 movie playback code through `imports.tsv` (Bink imports) and the R7 movie notes.
- `map-markers.md` + `area-map-surface.md`: marker placement / hit testing / surface size on the full map, including the T4 question: `CSWGuiInGameMap::Draw` 0x894CD0 subtracts 640x480 under `[this+0x48]&1`.
- Interaction with the live `k2-aspyr-map-aspect-fix` hooks 0x894435 / 0x895E6D / 0x7508AA: report overlap, do not resolve it.

**V10 — GUI misc and residency**
- `button-focus-badge-geometry.md`: button focus highlight and badge rects at large `fs`.
- `texture-residency.md`: K1 texture residency / eviction. Cross-check with doc 10 / R3 (194.8 MB TPC resident, CRes refcount facts in CONTRACT "Verified facts") and only report what is NEW versus R3.

### B2. Run the analysts (3 Sonnet `general-purpose` agents, in parallel; read-only; each writes ONE file `engine_research_findings/V8.md` / `V9.md` / `V10.md`)
Brief them like V7 (prompt template = the V7 launch: CONTRACT + the V_BRIEFS header + their section). Include:
- absolute paths;
- "write only your file";
- "no Ghidra, no launch, no game-dir writes";
- `.rdata` constants are read from `~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe`;
- the KMRP docs from B0 as LEAD material only;
- per-question verdict BUG / NO BUG / UNCLEAR (section (e)) and unnamed functions (section (f));
- for any BUG: a hook window with `original_bytes` and a one-line fix sketch. No code.

### B3. Opus gate (per V file)
- Re-check every BUG / UNCLEAR claim and 10 random quoted asm lines against `decomp/*.asm`.
- Read constants from the vanilla exe with `mods678_research/read_exe_consts.py` (it takes VAs; works on any exe path variable).
- Recompute the arithmetic.
- For BUG windows: add them to `check_engfix_windows.py` and run it (live overlap +-8 bytes, both exes).
- Append "Opus verification (<date>)" to each V file.

### B4. Persist (Sonnet drafts; Opus edits and commits)
1. Register rows **L16+** in `16_candidate_register.md`, one per topic, with status, evidence, impact at 3840x1600, hooks needed and risk. Add a "Phase 3 static triage" paragraph to the hand-off summary and update the ranking table.
2. `K1_ENGINE_BUGS_IN_K2.md`:
   - move K3 and each examined topic out of "Not yet looked at" into the tables, with verdict + deciding K2 address;
   - keep the caveats.
3. Knowledge DB: a new idempotent `engine_bug_research/tools/persist_phase3_db.py` (pattern: `persist_phase2_db.py`, upsert by topic; names only where the address has no name yet). Run it twice to prove idempotency.
4. Memory: update the `kotor2-patch-manager.md` line and the MEMORY.md "engine-bug candidates" line.
5. Any confirmed, user-visible bug becomes a **build sketch only**: hook, size, risk, and a test card added to the register. Then STOP and ask the user whether to build it in the next phase.

## Delegation map

| Step | Who | Opus checks before accepting |
|---|---|---|
| A1 analyzer + tests | Sonnet coder | parser covers every doc-17 log line format; fixtures pass (incl. the 10-03 60-line log = PASS with 12 dialogues); read-only (no game-dir paths written) |
| A2 V8b draw-path analysis | Sonnet analyst | windows / bytes in the asm; overlap with semi-transparent-letterbox and with every live hook (+-8 bytes); that Draw/render claims quote the asm |
| A2 1.0.1-probe build | Sonnet coder | every new cpp line; rate limit; `probe=0` = silent; smoke; checker; staged only |
| A3 V8b (only if T1 shows a panel issue) | Sonnet analyst | windows / bytes in the asm, overlap check |
| A4 1.0.1 | Sonnet coder | every changed cpp line; smoke 9 modes; checker; kpatch hooks identical to 1.0.0 |
| B0 fetch, B1 briefs | Opus | — |
| B2 V8/V9/V10 | 3 Sonnet analysts in parallel | B3 checks |
| B4 drafts | Sonnet | facts match the verified V files |

## Verification (end of phase)
1. `analyze_engfix_logs.py` passes its fixtures and gives a verdict on the user's T1 log.
2. L1/L4 state recorded in doc 17 + the register (accepted / changed / removed) with screenshot evidence paths.
3. If 1.0.1 was built:
   - 9 smoke modes pass;
   - the checker reports ALL OK + PROXIMITY OK and both exes MATCH;
   - after the user-approved install: `stage_and_verify_install.py verify` gives mismatches 0, and `swkotor2.exe` md5 is still `c9a1e3c57fd5491b4256dac1c0ab97cb`.
4. V8/V9/V10 each have an Opus verification note, and every BUG has a validated window.
5. Register L16+ rows, the updated community doc, DB rows (second run adds 0) and memory are all committed. `git status` shows only intended paths.
