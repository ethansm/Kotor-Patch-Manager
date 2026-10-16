# Session Handoff & Plan State: .claude/plans/fix_pipeline_plan.md

## 0. Next-Phase Direction
Migrate to kplog.h:  load_hang_probe_src (0.1.4): 5 raw fopen sites in LoadHangProbe.cpp.

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/fix_pipeline_plan.md`
- **Branch:** `KOTOR-II` (abe262d PLAY_TEST_QUEUE: companion-ai KpLog follow-up rows CAI-PT1..3; handoff with open review findings 3-6)
- **Generated:** 2026-10-16 12:30:43 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
abe262d PLAY_TEST_QUEUE: companion-ai KpLog follow-up rows CAI-PT1..3; handoff with open review findings 3-6
 .claude/handoffs/fix_pipeline_plan.handoff.md | 5 +++++
 patch_manager_mods/PLAY_TEST_QUEUE.md         | 3 +++
 2 files changed, 8 insertions(+)
```
- **Dirty Files (166):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ... +156 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/fix_pipeline_plan.md`.

## 5. Dead Ends & Verification (manual)

### Lessons for the next KpLog migration (load_hang_probe_src), 2026-10-16
- Older history of this file (Phases A-H, log hygiene, ...) was dropped by `plan_manager.py handoff` regeneration; recover with `git show abe262d:.claude/handoffs/fix_pipeline_plan.handoff.md`.
- Pattern: guide `patch_manager_mods/19f_kplog_migration_guide.md`; references `ranged_ai_src/RangedAI.cpp` (:78 fallback, :403 KPatchInit) and `companion_ai_src/CompanionAI.cpp` (startSession / KPatchInit / writeLine). `KP_LOG_PATCH("<manifest id>")` at global scope, `KP_CHANNEL`, `KpLog_SetFallbackFile("<today's file name>")` + `KpLog_SetFallbackLevel(KPLOG_INFO)` in a file-scope initialiser so an old core keeps today's file.
- The core cuts a message at 232 bytes ("[...]"): keep each line under it, or split (companion-ai splits with `t=<ms> +` pieces and the parser re-joins). Check long lines BEFORE migrating.
- Core rotates at 8 MB with one `.1` generation; do not rely on a header written once for a long session.
- DllMain runs before KpLog_Init: no logging there on a new core (lines land in the fallback). Do file I/O (ini read) from the first hook, not under the loader lock in KPatchInit (Opus review finding 3).
- Adding `KPatchInit` changes exports: `kpm_hotswap --apply` refuses; use the full `python3 scripts/kpm_reinstall.py --mod <id>` (game closed, user GO). `kpm_status.py` now reports REINSTALL for an export/import change (9fd2cd4). Unit tests that pin `exports ==` count need +1.
- After install: copy `kplog.ini` to `kplog.ini.bak_<tag>_<date>`, append `[<id>]` `enabled=1` `level=info` `channels=*`; check with `python3 scripts/kotor2_logs.py check <id>_log.txt --new`; add the log name to `scripts/kotor2_logs.py` ROUTES if a dedicated parser reads it (`scripts/test_kotor2_logs.py` class list too).
- `git add -N .` marks every untracked file intent-to-add (1741 here); never run it. Commit explicit paths only.

## KpLog migration: buff-duration-hud 0.2.3 + shield-bar 0.1.3 (2026-10-15)
- Sources `e3b6d97`; KPM `9a1d612`, `b0b9025`. GATE PASS (smokes=portraitslots). Hot-swapped 00:30, both LIVE (85e4334b, 4a33a6b1). Rollback: `python3 scripts/kpm_hotswap.py --rollback batch_swap_20261015_003043`.
- Logs now `buff-duration-hud_log.txt` / `shield-bar_log.txt` (core format). `kplog.ini` got `[buff-duration-hud]` and `[shield-bar]` sections (level=info); backup `kplog.ini.bak_buffshield_20261015`. Probe chatter is DEBUG on channel `probe`; in-mod line caps removed.
- Play check: `python3 scripts/kotor2_logs.py check buff-duration-hud_log.txt --new`; expect `buffhud loaded` and VIS lines, 0 WARN. Remaining non-KpLog mods: companion-ai, inventory-sort-filter (leftover truncate), dialogue-letterbox-fix-probe, probes (area-hook, load-hang, voice).

### Live log check 2026-10-15 23:11 session (buff-hud 0.2.3 / shield-bar 0.1.3)
- `buff-duration-hud_log.txt` 327 L CLEAN, 0 WARN, `buffhud loaded` present, 20 `VIS slot* why=drawn`, 19 `DEBUFF` column lines (ip_slow x13, ip_stun), no untracked-debuff dump, no VIS cap. `shield-bar_log.txt` 19 L CLEAN (VIS bar/mbar/overlay). Old `buffhud_log.txt` is stale (pre-migration format).
- Not seen yet: probe-channel DEBUG lines (kplog.ini level=info). Hotbar 1-0 key press check after the EKC hook move is still the user's.
- Memory updated: `kotor2-fix-pipeline.md`, `kotor2-kplog-adoption.md`. NEXT: companion-ai KpLog migration (plan mode; plan file in `.claude/plans/`). Remaining raw-fopen mods: inventory-sort-filter (truncate), dialogue-letterbox-fix-probe, area-hook/load-hang/voice probes.

## KpLog migration: companion-ai 0.2.1-probe (2026-10-15)
- Code `c0e6d11` (repo) + KPM `2a8a877`; `kpm_status` fix `9fd2cd4` (REINSTALL when exports/imports differ; before it a new export showed SWAP and kpm_reinstall refused it). Gate PASS (307 tests), `--prepare` done, `kpm_reinstall.py --mod companion-ai --dry-run` OK (peer DLLs to restore: inventory-sort-filter, holster). NOT INSTALLED: needs user GO (game closed).
- Design: LogGate/helpers unchanged, sink = KpLog (eline -> main, tline -> trace, pline/pev -> probe, A4 NULL -> WARN). DllMain only builds paths; `startSession()` (header + loadIni) runs from `KPatchInit` or, on an old core, from the first `tick()`. Fallback file `ai_trace.txt` at INFO (appends).
- Core cuts a message at 232 bytes: `writeLine` splits lines > 224 bytes at spaces into pieces (`t=<ms> + <rest>`, `++` = mid-word); `analyze_play_logs.read_trace_lines` re-joins them. Raw greps see pieces.
- `flush_ms` ini key unused (KpLog flushes, 500 ms default; crash tail up to 0.5 s; kplog.ini `[global] flush_ms=0` for crash work).
- Install steps: `python3 scripts/kpm_status.py` (no peer installing), `python3 scripts/kpm_reinstall.py --mod companion-ai`, then `cp kplog.ini kplog.ini.bak_companionai_<date>` and append `[companion-ai]` `enabled=1` `level=info` `channels=*`. Then after play: `python3 scripts/kotor2_logs.py check companion-ai_log.txt --new` (expect CLEAN, `# companion-ai 0.2.1-probe session`, config lines, `S lines=`, no DROPPED).

## companion-ai KpLog follow-up (2026-10-16)
- Opus review (`.claude/reviews/kplog_migrations_review.md`): fixes 1+2 done and LIVE (repo 24c5415, KPM 7b6fcbd, hot-swap backup `companion-ai_swap_20261016_000159`): A4 crash line also goes to `companion-ai_crash.txt` + Sleep(100); session header + `# config (rotate)` re-emitted every ~5 MB; `archive_play_logs` takes `*log*.txt.1`.
- Live log check 00:24: CLEAN (0 FAIL/0 WARN, 3668 lines, 0 truncated, no DROPPED, 0.2.1-probe header + config). 26 checklist ids still OBSERVE, queued in `patch_manager_mods/PLAY_TEST_QUEUE.md` as CAI-PT1 (CAI-1..6, CAI-8, E3), CAI-PT2 (PR-* probes; PR-P2 gap window question) and CAI-PT3 (M6/M8, other mods). `kotor2_logs check` printed "22" because it counts differently; the analyzer lists 26.
- OPEN follow-ups from the review, not done: (3) KPatchInit does file I/O under the loader lock (move header formatting to DllMain, `loadIni` to the first tick; also probe handlers read `g_cfg` before tick on the old-core path); (4) shield-bar probe lines exceed 232 B (GL :365, APPLY :137, CHANGE :144) and lost the frame stamp; (5) buff-hud `logUntrackedDebuff` walks the effect list per frame when INFO is on (gate :411), `visLine` burns its 64-line cap while logging is off; (6) old-core fallback appends with a 60000-line cap and sessions merge silently. Also: inventory-sort-filter got its pending kpatch (442498b5) during the 10-15 full reinstall, not just the peer restore.

## load-hang-probe KpLog migration (2026-10-16)
- load-hang-probe 0.1.5 migrated to KpLog.h (repo cb471ea): channel `main`, core file `load-hang-probe_log.txt`, old-core fallback `hangprobe_log.txt` at INFO; `KPatchInit` export added (15 exports); DllMain's 3 pre-init lines queued (`g_early[8]`) and flushed by `KPatchInit` (only if `KpLog_Init` succeeds) or by the watchdog after <= 1.5 s; lines > 220 B split at spaces (`+ ` continuation). Report file, `hangprobe_debug.txt` kill switches and `g_on` gate unchanged. 19e adoption table updated.
- Verified: `build_check --mod load_hang_probe_src` PASS (15 exports), `run_tests.py load_hang_probe` PASS, host check of the splitter (100..1000 B, round-trips, pieces <= 220 B).
- `pre_install_gate.sh --mod load_hang_probe_src`: **GATE FAIL 1**, 312/313 passed. Only failure: `ghidra_knowledge/tools/tests/test_rea_ledger.py`, caused by the peer's untracked `rea_findings/20261016_holster-c5-weapon-identity.tsv` (date column `20261016`, needs `2026-10-16`). Not touched by this commit; re-run the gate after the holster session fixes it, before any install.
- Opus review (APPROVE, no BLOCKING): fixed note 2 (no fallback file I/O in KPatchInit if `KpLog_Init` fails) and note 3 (flush loser waits for `g_logReady`). Open, not done: `log_triage` does not detect banners in the old-core fallback shape `<ms> id/chan ...` (generic gap, affects every fallback file; new-core files are fine via `# kplog session`); report still tails stale other-mod names (`force_power_hotbar_log.txt`, `buffhud_log.txt`, `inv3d_log.txt`); on a new core the lines only appear once `[load-hang-probe] enabled=1 level=info` is in kplog.ini.
- NOT installed (NOT-LIVE probe). Install needs: user GO, game closed, `hangprobe_debug.txt`, `kpm_hotswap --prepare` then `kpm_reinstall.py` (new export), kplog.ini backup + section.
- Still raw fopen: k1-spawn-probe, area-hook-probe, voice-probe (peer), dialogue-letterbox-fix-probe, inventory-sort-filter truncate (`InventorySortFilter.cpp:436`).
- Gate update 10-16: after d03e67c (holster TSV dates) the first re-run failed on `wine:probe14` (smoke checked the `0.1.4 loaded` line; bumped to 0.1.5 in `smoke_probe14.cpp`) and then on `test_override_watch.py` (inotify merges identical unread events, so two back-to-back reads raced the watcher; 0.1s gap added). **`pre_install_gate.sh --mod load_hang_probe_src` now GATE PASS** (313 default tests, 39 wine, probe11/12/14). The probe14 pass also exercised the old-core fallback path: `hangprobe_log.txt` got the early `loaded` line.
