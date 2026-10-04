# Session context: doc-15 Phase 4 (everything except the T1 screenshot) (2026-10-03)

Companion to `15d_phase3_session_context_2026-10-03.md` (Phase 3: verified facts, probe build, gotchas; still valid), plan
`engine_bug_research/PHASE4_PLAN.md` (verbatim user-approved plan), `17_dialogue_letterbox_and_wrap_fixes.md` (build doc; DECISION + STATUS at top,
"Install (Phase 4 ...)" section), `16_candidate_register.md` (Phase 4 paragraph, T1 decision card). Branch `KOTOR-II`.
Commits this session: `c7a6303` Step 0, `226c437` Step 1 (L4 drop), `16f52b7` + `d93fa6c` Step 2 (V11 + Opus gate), `2efb746` Step 3 (1.0.1),
`d9dc152` Step 4 (analyzer), `c8e52f8` Step 5 (stages + doc 17), `d3441a8` Step 6 DB, then the Step 6 docs commit.

## 1. Where the plan stands
| Step | State |
|---|---|
| 0 plan copy | DONE |
| 1 L4 drop recorded (register, doc 17, K1 doc, 15d) | DONE |
| 2 V11 (letterbox globals, first-draw white, Bink offset) + Opus gate | DONE: NO BUG x3; one UNCLEAR low-risk lead (SetReply) -> T1 add-on |
| 3 L1 1.0.1 build (log=0 default) | DONE, built + staged, **NOT installed** |
| 4 analyzer: PROBE W1-W5 diagnosis + 1.0.1 log=0 format | DONE (46 tests) |
| 5 stage A (1.0.1) / stage B (probe), install commands without L4 in doc 17 | DONE |
| 6 persist (register, DB, K1 doc, 15e, memory) | DONE |
| Install (user go 16:44) | DONE: 1.0.1 live, L4 dropped, core restored |
| T1 screenshot (user) | **OPEN**: the only remaining input |

## 2. Live state
- **16:44 INSTALL (user go):** L1 1.0.1 live (log=1 INI), L4 dropped from `patch_config.toml`, registry core `36f10b7f` restored (had been reverted to `2a706ac9`), `bin/Debug` core updated. Details + backup: doc 17 top.

Re-read 2026-10-03 15:29 (before the install):
- 31 ids, `kpm_install_state.json` UpdatedAt **2026-10-03 12:00:17** (sibling reinstalls at 11:29 and 12:00 during this session), `swkotor2.exe` md5
  `62411a3a3282ecf92bdf54aeb1bac671`. This session wrote nothing to the game dir. L1 1.0.0 and L4 still installed; no `dialogue_letterbox_fix.ini`.
- KPM rewrites `swkotor2.exe` on every install, so its md5 is only a "nothing changed since X" check, not a constant (Phase 2's `c9a1e3c5...` is history).

## 3. Verified facts (new this phase; Opus-checked)
- **V11 Q1:** `0xA309B4/B8/BC` writers = SetTop 0x8BACA0, SetBottom 0x8BADA0, Draw 0x8BB260, ResizeToText 0x8BAB20 (BC: 0x7CEB80 only); readers = 5 copies of the
  video-effect composite quad (0x4426F0, 0x443190, 0x445310 live; 0x4421F0, 0x442C90 dead behind 0x484910 which always returns 0). Band y-up [b4-25, b8+25]
  (25.0 @0x989ED0). Vanilla b4 = -22 (unclamped), band [-47,1425]; L1 band [242,1358]. Fires only for clairvoyance/Fury videoeffects rows via CamVidEffect
  (42 DLGs / 308 nodes, incl. override kreia.dlg) with Frame Buffer=1. **L1 moves or clips nothing else.**
- **SetReply 0x8BB8D0** returns early while the top bar's done flag `[+0x8C]` is 0 (UNCLEAR, low risk; 16:9 vanilla has the same slide). T1 add-on.
- **V11 Q2:** 0x47D2F0 drains pending texture builds (`0x47D3F3 CALL 0x427F30`) before binding (`0x47D401 CALL 0x4DAE60`): no first-draw white.
- **V11 Q3:** Bink offset (0,-3) is signed (CDQ/SUB/SAR) at 0x40D392 and 0x40DD3D; popup fully covered; `0x5D000000` bit names unknown.

## 4. Builds and stages (none installed)
- **1.0.1** (`dialogue_letterbox_fix_src/` = `KPM/Patches/K2DialogueLetterboxFix/`, byte-identical): kpatch `f1e59f7b12b339f2bb879cfb480a9b0c`, DLL
  `cf45a03af96881362e586c8baf0654b9` (KERNEL32 + msvcrt). Hooks TOML identical to 1.0.0. log=0 default writes load line, first non-problem APPLY pair, first
  SETTOP + first SETBOTTOM, all problem lines (LATE notice now forced too). Wine smoke 10 modes pass. Checker output `check_engfix_windows_output_2026-10-03b.txt`
  (ALL OK, PROXIMITY OK, subset OK; both exes ALL MATCH). 1.0.0 kpatch backup `~/Games/KOTOR2_Mod_Workspace/backups/k2_dialogue_letterbox_fix_1.0.0/` (md5 `6033b700...`).
- **1.0.1-probe** unchanged (kpatch `4d680986...`, DLL `e46cfcc8...`). It lives ONLY in `KPM/letterbox_probe_stage/` (doc 20d F1: a `Patches/K2DialogueLetterboxFixProbe/` dir duplicates the id and turns `validate-patches` red). This session wrongly recreated that dir at 15:29; the testing session removed it again (byte-identical, nothing lost). Never recreate it; build per `dialogue_letterbox_fix_probe_src/README.md` "Build" (temp Patches dir).
- **Stage A** `~/Games/KOTOR2_Mod_Workspace/stage_l1_101_1003_1529`, **stage B** `~/Games/KOTOR2_Mod_Workspace/stage_lbprobe_1003_1529`: 32 kpatches each,
  PROBLEMS 1 = known 4gb-patch hooks=DIFF artifact; STAGE_INFO head `28ae185ef061`. Install commands (30 ids, no L4) in doc 17. Stale on any sibling reinstall.
- Analyzer fixtures: `engine_bug_research/fixtures/letterbox_log_101_smoke_{default,log0}.txt` (raw smoke logs: FAIL on the deliberate INVALID line only, WARN on c).

## 5. Next actions
**RESUME HERE (10-04):** the user finished a game session (logs ~1.5 h old; 18 live logs incl. `k2-dialogue-letterbox-fix_log.txt` 32 lines). Next = analyze ALL logs with the `kotor2-log-analysis` skill (`scripts/kotor2_logs.py status|check`), archive first; handoff `.claude/handoffs/PHASE4_PLAN.handoff.md`.
1. User: one F12 screenshot during a conversation (bright area); also check that the first NPC line shows its text (V11 add-on).
2. Copy log + screenshots to `~/Games/KOTOR2_Mod_Workspace/playlogs_engfix_<MMDD_HHMM>/`, run `analyze_engfix_logs.py` on the copy, Read the images.
3. A3 decision (register T1 card): top band visible -> install stage A (re-stage first if UpdatedAt / ids changed), verify, mark L1 accepted; no bands ->
   install stage B, `probe=1`, one conversation, analyzer DIAGNOSIS, Opus designs the fix.
- Out of scope (unchanged): upstream KPM PR (draft `engine_bug_research/upstream_kpm_k2_status_note.patch`), L16 build, GOG ports, L15.

## 6. Gotchas
- Sibling sessions reinstall often (twice during this session): always re-read UpdatedAt + `InstalledPatches` right before an install and re-stage into a NEW dir.
- `stage_and_verify_install.py stage` needs `--allow-dirty` for this mod (KPM clone stays uncommitted; repo mirror is the committed copy). DIRTY warnings for
  `character-effects-panel` / `force-power-hotbar` are sibling-owned.
- Do NOT recreate `KPM/Patches/K2DialogueLetterboxFixProbe/` (duplicate id; doc 20d F1). Probe kpatches live in `KPM/letterbox_probe_stage/`.
- **Superseded later on 10-03:** the shared-logging session moved L1 to KpLog.h as **1.0.2** (installed live 22:42, DLL `7b3936bc`; log `k2-dialogue-letterbox-fix_log.txt` via `kplog.ini`). See doc 17 top + 19e. 1.0.1 and stage A are history; 1.0.2-probe is the no-bands path.
- `analyze_engfix_logs.py` takes `<logfile> [--json]` (no `--help`). Smoke harness logs are synthetic (fake structs); judge the analyzer on real game logs.
- Earlier gotchas in 15d section 6 still apply (pgrep self-match, sibling-owned paths, DB idempotency).

- Note (log analysis 2026-10-04): T1 log PASS (12 dialogues, 0 errs); visual check OPEN (no screenshots), so L1 is not accepted yet and the register T1 card waits on F12 + first-NPC-line text. Report `play_session_reports/2026-10-03_evening.md`.
