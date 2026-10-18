# Implementation Plan: HW-UX3 fix round (slider drag follows cursor + Reset Scope back) then install for play test

## 0. Preserved Context & Anti-Patterns (Post-Clear Memory)
- **Next-Phase Direction**: "HW-UX3 fix round: (1) slider thumb drag follows cursor (Scrub scale 2W/trackW, hints, tests), (2) restore Reset Scope button (end of pageLayoutV3 order, two-click confirm, 70 boxes + 6 wedge labels <= 80). Then Wave D install for user play test." User said "go Wave D" earlier and "Install when complete for me to play test": the install at the end of this plan is pre-authorised, but the game must be closed (check `pgrep -af swkotor2` first; if running, stop and ask).
- **Active Branch & Worktree**: `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`, branch `KOTOR-II`; other sessions share it (~173 foreign dirty files): commit ONLY explicit paths (`git commit -- <paths>`); never `git add -A`, never bare `git stash`.
- **Plan (repo copy, commit it first)**: copy this plan to `.claude/plans/holster_hw_ux3_fixes.md`; parent plan `.claude/plans/holster_hw_ux3_build.md` (its §6 Status lines are the tracker; `plan_manager.py` does not parse these headings).
- **Resume**: `python3 scripts/plan_manager.py resume .claude/plans/holster_hw_ux3_build.md`
- **Handoff**: `.claude/handoffs/holster_hw_ux3_build.handoff.md` (committed a2cfadcf; §5 holds the traps below).
- **Current State (verified)**: waves A, B, C1 built, committed, Opus-reviewed, host-tested, NOT installed. HEAD a2cfadcf. Live versions: holster 0.1.5-dev, holster-editor 0.3.2-dev, mod-config 0.1.2-dev. Repo: holster 0.2.0-dev, holster-editor 0.4.0-dev, mod-config 0.1.3-dev. `check_shared_headers.py --sync` already DONE (52 headers, 0 drifted). `kpm_status.py`: MIRROR for holster, holster-editor, mod-config (+enemy-effects-strip, not ours). `kpm_hotswap.py --prepare` NOT run yet (it builds from repo sources, so it waits for this fix round).
- **Dirty files that are mine**: none uncommitted. Foreign dirty files include `patch_manager_mods/PLAY_TEST_QUEUE.md` (a 2-line foreign edit): add rows via the index-blob trick (`git show HEAD:<f> > tmp; append; git hash-object -w tmp; git update-index --cacheinfo 100644,<sha>,<f>`, then `git add` my other paths and `git commit` WITHOUT pathspec).
- **Anti-Patterns & Traps**:
  - `agent_contract.py check <reply.txt> --scope <globs...>`: the reply file comes FIRST (`--scope ... -` from CLAUDE.md fails). Include recon output dirs in `--scope` or they show as UNCLAIMED warnings.
  - Run `agent_contract.py snapshot` ONCE per launch batch, before launching; re-snapshotting mid-run baselines another worker's in-progress edits.
  - Two Sonnets must never edit the same file concurrently (Edit rewrites whole files). Write Scopes below are disjoint; HolsterEditor.cpp has ONE owner per wave.
  - Brief prose "stay under 80k" does not hold; cap by tool calls (~15), line ranges <=120, one question per agent, exact anchors. Measured: Haiku recon 76-98k, Sonnet workers 80-366k.
  - New raw engine addresses in a .cpp fail `check_raw_addresses.py` until listed in `tests/static/raw_addresses.toml` (alphabetical by path, `0x00XXXXXX`); this round adds none.
  - `build_check.py --mod holster_editor_src` needs the KPM Common copies (now synced); if it fails on a missing header, re-run `check_shared_headers.py --sync` (never write into KPM Common by hand).
  - No `oc-*` backends. Haiku swarm only (`model: haiku`); Sonnet (`model: sonnet`) for code; Opus (`model: opus`) only for the independent review. Workers never run `--sync/--prepare/gate/install`.

## 1. Context (why)
After the 10-18 build round (waves A/B/C1) the independent Opus review approved install but left two owner decisions, which the user now wants fixed before play testing:
1. **Slider thumb lags the cursor (~0.4x).** `hse::Scrub` (`HseView.h:923`) steps 1 px = `kScrubOffPerPx` 0.001 m (0.5 deg for rotation) while the slider TRACK spans the whole +-W window (`inspWindow(row)`: +-0.30 m / +-180 deg) over ~250 px, i.e. ~2.4 mm per track px. Jump-to-click (`inspTrackValue`, `HseView.h:1111`) is already track-scaled, so click lands under the cursor but dragging then falls behind. Also the hints still say "1 px = 1 mm" (`inspHint`, `HseView.h:1121`).
2. **"Reset Scope" button was dropped** by the page rebuild. Old behaviour (`git show 041aab12:patch_manager_mods/holster_editor_src/HolsterEditor.cpp`, ~lines 481 and 2159): button "Reset Scope" with description "Delete custom overrides for this scope.", click -> `guardTransition(e, ACT_RESET_SCOPE, "reset_scope")` then `g_hol.h->ResetScope()` (`_shared_src/HolsterApi.h:64`: deletes this scope's keys, then saves), `captureOpen(e,"reset_scope")`, `refreshEdit`, `setBoneText`->now `refreshSelectors(e,true)`, `updateStatusText`. Without it the UI cannot delete saved overrides.

## 2. Design decisions (made; do not re-litigate)
- **Scrub scale**: `Scrub::begin(int r, int x, float openValue, int trackW = 0)`. With `trackW > 0`: `perPx = 2 * inspWindow(r) / trackW` (value units per px) replaces `kScrubOffPerPx/kScrubRotPerPx` in `step()`; Shift still x`kFineScale`, Ctrl still snaps (`kScrubSnapOff/Rot`, carried in `acc`). With `trackW <= 0` the old 1 px = 1 mm behaviour is kept byte-for-byte (old tests stay valid). In track mode the clamp window is the track range, not "open +-0.30": `lo = min(-W, open)`, `hi = max(W, open)` then cut to the hard limits (`kScrubLimitOff/Rot`), so a drag from the far end can come back across the track and an off-track value does not pin. Shift/Ctrl intentionally detach the thumb from the cursor (documented in the hint).
- **Hints**: replace the "1 px = 1 mm" tails of `inspHint` with "Drag the marker (Shift fine, Ctrl snap)." style text, still ASCII and < 64 chars.
- **Reset Scope placement**: a third button on the Save/Revert row: `Save | Revert | Reset Scope`, each `c3` wide (same width variable the scope row uses, `HseLayoutV3.h:~122`), so column height and the tiny-pane tests do not change. Appended LAST in `pageBoxesV3` order so no existing index shifts: `PI_RESET_SCOPE = 69`, `kPageItemsV3 = 70`, wedge labels follow (`kPageItemsV3Total = 76 <= kPageItemCap 80`).
- **Safety**: Reset Scope deletes SAVED keys, unlike Revert. It needs the existing two-click guard even when the edit is clean: extend `GuardAction` (`HolsterEditor.cpp:120`, now `ACT_NONE, ACT_HAND_R, ACT_HAND_L, ACT_PREV, ACT_NEXT, ACT_SCOPE0 = 10`) with `ACT_RESET_SCOPE = 20` and make the first click arm ("Click again to delete this scope's saved overrides") and the second (within `kDiscardWindowMs`) execute. Do not reuse `guardTransition` blindly: it only arms when dirty. Add a small pure helper next to `hse::guardProceed` (`HseView.h`) e.g. `confirmProceed(armedSame, armedAt, now, windowMs)` with host tests, and call it from a new `guardConfirm(e, ACT_RESET_SCOPE, "reset_scope")`.
- **Version**: holster-editor `0.4.1-dev` (manifest.toml, banner, status text, ini header). holster and mod-config unchanged (0.2.0-dev / 0.1.3-dev).
- Not in scope: C2 flourish hooks, Wave E U4 markers, any holster_src change.

## 3. Inlined snippets (zero-read for the workers)
```cpp
// HseLayoutV3.h:120-129 (save row; c2 = half width, c3 = third width, r = row height, kGap = 2 px)
L.save = mk(x0, y, c2, r);
L.revert = mk(x0 + c2 + kGap, y, c2, r);
y += r + kGap;
L.status = mk(x0, y, W, r);
// HseLayoutV3.h:280-298
enum PageIdxV3 { PI_MEMBER_PREV = 0, ..., PI_SAVE = 16, PI_REVERT = 17, PI_STATUS = 18, ..., PI_NOTE = 66, PI_HINT = 67 /* + hint (2) */ };
static_assert(PI_HINT + kV3Hints == kPageItemsV3, "...");   // 67 + 2 == 69
constexpr int kPageItemsV3 = 69;                              // HseLayoutV3.h:53
constexpr int kPageItemsV3Total = kPageItemsV3 + kV3Rows;     // HseLayoutV3.h:302 (75 -> 76)
// pageBoxesV3 (HseLayoutV3.h:251): out[n++] = L.save; out[n++] = L.revert; out[n++] = L.status; ... hints last
// HolsterEditor.cpp:2223-2237 click handling (inside `if (e.mode == MODE_WEAPON && g_hol.ok())`)
if (clk[hse::PI_SAVE]) {...} else if (clk[hse::PI_REVERT]) { g_hol.h->Revert(); captureOpen(e,"revert"); refreshEdit(e); refreshSelectors(e,true); updateStatusText(e); }
// HolsterEditor.cpp:1890 guardTransition: dirty = editDirty(); if (hse::guardProceed(dirty, armedSame, e.discardArmedAt, now, kDiscardWindowMs)) {...proceed} else { arm; updateStatusText(e); return false; }
// HolsterEditor.cpp:1655/1666 scrubBegin: `if (!e.scrub.begin(row, x, v)) {...}`  -> pass `e.lay.rows[row].track.w` (editkit::Box track, from hse::PageLayoutV3 rows[row].track)
// HolsterEditor.cpp:1705-1716 jump-to-click uses `const editkit::Box t = e.lay.rows[row].track; hse::inspTrackValue(row, x, t.x, t.w)`
// HseView.h: begin(r,x,openValue) computes W = rot ? kScrubWindowRot : kScrubWindowOff, l=open-W,h=open+W; step(): d = dx * (rot ? kScrubRotPerPx : kScrubOffPerPx) * (shift ? kFineScale : 1)
```
Item text/description tables to extend for the new button: `HolsterEditor.cpp:422-423` (`if (i == PI_REVERT) return "Revert";`), `:457-458` (descriptions), `:384` (button vs label classification: add `PI_RESET_SCOPE`). Tests that pin 69/75 (edit these): `hse_layout_v3_test.cpp:115-117` (`kPageItemsV3 <= 70`, `== 19 + 13 + 37`), `:246` (`for i = 32; i < kPageItemsV3` inspector-overlap loop: cap it at the inspector range, the new button is in the left column), `:300`, `:325` (`kPageItemsV3Total == 75`), `common_contract_test.cpp:231` (comment/label "69 boxes + 6 wedges"), `hse_view_test.cpp:983-984` (cap 80, unchanged).

## 4. Wave schedule (execute in order; each wave's agents launch in ONE message)
Estimated total: ~55 min wall, ~1.3M tokens (Haiku ~0.45M, Sonnet ~0.55M, Opus ~0.3M).

### Wave 0 (inline, 3 min)
1. `git status --short | rg "holster|PLAY_TEST"` (confirm clean of my paths), copy this plan into `.claude/plans/holster_hw_ux3_fixes.md`, commit it by path.
2. `python3 scripts/agent_contract.py snapshot` once.

### Wave 1: Haiku recon (4 agents parallel, read-only, <=15 tool calls each, 80k target; outputs `research/holstered_weapons/ux_v3/F1_*.txt`, contract `recon`)
- **R1 (Reset Scope provider)**: confirm `ResetScope` is implemented in `patch_manager_mods/holster_src/Holster.cpp` (rg `ResetScope`), exactly what keys it deletes and that it saves, what `GetEdit`/open edit state is afterwards, and that `HolsterApi.h:110` validity requires it. Question: any reason the editor must `EndEdit`/re-`BeginEdit` after it? (old code did not).
- **R2 (label fit)**: compute the `c3` button width at 1280x720, 1920x1080, 3440x1440 from `HseLayoutV3.h` (`columnWidth`/`c3`; the layout tests list the sizes) and check via `_shared_src/TextFit.h` / `editkit` text metrics whether "Reset Scope" fits a third-width button; propose a fallback label ("Reset" with the long text in the description) if not.
- **R3 (pinned counts)**: `rg -n "69|75|kPageItemsV3|PI_HINT|PI_REVERT|19 \+ 13|kPageItemsV3Total" patch_manager_mods/holster_editor_src patch_manager_mods/mod_config_src -g "*test*" -g "*.h" -g "*.cpp"` and list every site that must change for 70/76 (file:line).
- **R4 (scrub tests)**: list every test in `hse_view_test.cpp`/`view_drag_test.cpp` touching `Scrub`, `kScrubOffPerPx`, `kScrubRotPerPx`, `inspHint`, "1 px = 1 mm", and the track width produced by the layout at the 3 sizes (so perPx = 2W/trackW can be asserted: e.g. W=0.30, trackW=250 -> 0.0024 m/px).
- Escalation: any `PARTIAL`/`REFUTED` or two Haikus disagreeing -> ONE Sonnet reads both notes + anchors and settles it (inline, before Wave 2).

### Wave 2: Sonnet workers (2 parallel; disjoint scopes; contract `worker`; briefs are the §2/§3 text plus the Haiku notes)
- **S1 scrub scale** — Write Scope: `patch_manager_mods/holster_editor_src/HseView.h`, `hse_view_test.cpp`, `view_drag_test.cpp`. Objective: `Scrub::begin(..., int trackW = 0)` per §2 (track-scaled step, track-range clamp, old behaviour when `trackW<=0`), new `confirmProceed` helper (§2), new `inspHint` texts, tests: track-scaled step (px -> value), Shift x0.2, Ctrl snap, drag back across the track, off-track open value, `trackW<=0` equals old numbers, hint length <64 and ASCII, `confirmProceed` truth table (armed/unarmed x inside/outside window). Verify: `python3 scripts/run_tests.py hse_view view_drag`.
- **S2 Reset Scope** — Write Scope: `HseLayoutV3.h`, `hse_layout_v3_test.cpp`, `common_contract_test.cpp`, `HolsterEditor.cpp`, `manifest.toml`, `holster_editor.ini` (all under `patch_manager_mods/holster_editor_src/`). Objective: layout change + `PI_RESET_SCOPE = 69`, `kPageItemsV3 = 70`, item creation/text/description/placement/button classification, `ACT_RESET_SCOPE` + `guardConfirm` using S1's `confirmProceed` (if S1 has not landed, S2 declares it locally behind the same name in a pure header stub NOT in HseView.h and the orchestrator merges in Wave 3: simpler rule: S2 waits for S1's helper by calling it from `HolsterEditor.cpp` only after S1 finishes; so launch S2 first for layout/tests and let it add the guard wiring in a second pass via `SendMessage` once S1 reports), click handler (§1), status/banner text, version 0.4.1-dev, ini header, tests per §3. Keep `refresh stale caches` behaviour (viewRect/strip/statusRect/wheel rect refresh) untouched. Verify: `python3 scripts/run_tests.py hse_layout_v3 common_contract hse_view` and `python3 scripts/build_check.py --mod holster_editor_src`.
- After both: `agent_contract.py check <reply.txt> --scope <their globs> research/holstered_weapons/ux_v3/*` per worker.

### Wave 3: Haiku (3 parallel)
- **H1 call-site**: in `HolsterEditor.cpp` pass the track width: `e.scrub.begin(row, x, v, e.lay.rows[row].track.w)` (scrubBegin ~1666) and refresh the comment above `scrubTick` (~1683-1689, "the track spans..."); scope: that file only; verify `build_check.py --mod holster_editor_src`. (Runs only after S2 is finished: same file.)
- **H2 static checker** (read-only): every `Add*` has an `Editor` field and every field is placed in `pageLayoutV3`; count == `kPageItemsV3` (70) + 6 wedges <= 80; the new item is created before the wedges and refused cleanly on `AddX == -1`.
- **H3 test runner** (read-only): `python3 scripts/run_tests.py hse_view hse_layout_v3 reset_icon slider_art guikit common_contract menu_ui_logic view_drag` and `-k holster_`, `check_raw_addresses.py` (FAIL=0); reply counts only. Any failure -> one Sonnet fixer (fix, re-run that suite), never "pre-existing" without investigation (Verify-After-Fix Invariant).

### Wave 4: independent Opus review + fixes (1 agent, fresh context, model `opus`)
Given: `git diff a2cfadcf..HEAD`-equivalent of the uncommitted delta (commit Waves 2-3 by explicit paths first so the diff is stable), the §1/§2 claims (not conclusions), prior reviews `OPUS_review_round.txt`. Check: track-scaled scrub correctness (units, clamp, hard limits, NaN/zero trackW, tiny track < 4 px), thumb-under-cursor across all 6 rows incl. rotation rows, ResetScope two-click confirm (window expiry, arming by another action, body-scope, not-open edit, provider missing), layout of the 3-wide button at 1280x720/1920x1080/3440x1440, index/static_assert consistency, item cap 76 <= 80, version strings, no regression in bone/hand/scope guards, logging (`log_verbose=0` quiet; one `BTN ResetScope` line). Fix scope: `holster_editor_src/**` only. Output `research/holstered_weapons/ux_v3/OPUS_review_fix_round.txt` with BLOCKING/NON-BLOCKING and `DECISION: APPROVE-FOR-INSTALL | NOT-READY`. Re-run Wave 3 H3 after its fixes.

### Wave 5: commit, queue, install (orchestrator, Opus; needs game CLOSED)
1. Commit own paths (explicit); update `PLAY_TEST_QUEUE.md` via the index-blob trick: edit UX3's steps (drag follows cursor) and add **UX3c** (Reset Scope: first click arms with the status warning, second within the window deletes the scope's saved overrides and the weapon returns to defaults/inherited values; Esc/other click disarms) and **UX3d** (drag a slider marker: the marker stays under the cursor along the whole track for Slide/Depth/Height/Tilt/Roll/Turn; Shift detaches for fine control; Ctrl snaps). Update parent plan §6 Status, handoff, memory `kotor2-holstered-weapons.md` + MEMORY.md line, parent plan copy.
2. `pgrep -af swkotor2` (must print nothing besides the shell); then `python3 scripts/check_shared_headers.py --sync` (idempotent, expect 0 drifted) -> `python3 scripts/kpm_status.py`.
3. `python3 scripts/kpm_hotswap.py --mod mod-config holster holster-editor --prepare` (expect SWAP x3; REINSTALL = STOP and report).
4. `scripts/pre_install_gate.sh --mod mod_config_src --mod holster_src --mod holster_editor_src` (fresh; `GATE PASS` required; no `--skip-default`; a first failure: investigate and fix, second failure: stop and name the doubtful assumption).
5. `python3 scripts/kpm_hotswap.py --mod mod-config holster holster-editor --apply` (one apply, all three; mod-config first is handled by the tool), then `python3 scripts/kpm_status.py` shows LIVE x3. Rollback if anything misbehaves: `python3 scripts/kpm_hotswap.py --rollback latest`.
6. Give the 4-section end-of-step output and tell the user the play rows to run (UX3, UX3b, UX3c, UX3d, ICON1, HW7a, HW7c, LOG1, CAM2).

## 5. Edge cases and tests
- Scrub: `trackW` 0/negative/tiny (<4 px) falls back or clamps without div-by-zero or NaN; open value outside +-W; very fast mouse (dx > trackW); Shift/Ctrl toggled mid-drag never jumps (per-tick delta kept); hard limits 3 m / 720 deg; body scope disables rotation rows (unchanged).
- Reset Scope: two-click window (`kDiscardWindowMs`), arm by another action then click => re-arms (not executes), no open edit => button inert, provider missing (`g_hol.ok()` false) => no call, scope switch while armed disarms, log lines capped.
- Layout: three-wide Save row at 1280x720, 1920x1080, 3440x1440, tiny panes, preview overlapping the view; boxes inside the column, none overlapping the view; item count 70 boxes + 6 wedges = 76 <= 80; refusal path with a 40-cap provider unchanged.

## 6. Verification (final integration)
- [ ] `python3 scripts/run_tests.py hse_view view_drag hse_layout_v3 reset_icon slider_art guikit common_contract menu_ui_logic` all pass
- [ ] `python3 scripts/run_tests.py -k holster_` 14/14 and `python3 scripts/check_raw_addresses.py` FAIL=0
- [ ] `python3 scripts/build_check.py --mod holster_src --mod holster_editor_src --mod mod_config_src` PASS
- [ ] `python3 scripts/agent_diff_check.py --scope "patch_manager_mods/holster_editor_src/*"` clean
- [ ] Opus review DECISION APPROVE-FOR-INSTALL, then gate `GATE PASS`, then `kpm_status.py` LIVE x3
- [ ] Logs after play only via `python3 scripts/kotor2_logs.py check holster-editor_log.txt --new` and `holster_log.txt --new` (never cat raw logs)

## 7. Risks
| Risk | Mitigation |
|---|---|
| Track-scaled drag detaches on Shift/Ctrl | By design; hint text says so; UX3d row |
| Clamp change lets a value leave the +-0.30 m comfort window | Hard limits (3 m / 720 deg) still hold; Revert/row reset exist |
| "Reset Scope" label too wide at 1/3 column | R2 recon; fallback label "Reset" + description |
| Two Sonnets conflict in HolsterEditor.cpp | S1 never touches it; H1 runs after S2 |
| Install while game running | `pgrep` check; stop and ask |
| Gate fails twice | stop, name the doubtful assumption, report |
