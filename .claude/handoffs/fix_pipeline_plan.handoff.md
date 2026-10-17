# Session Handoff & Plan State: .claude/plans/fix_pipeline_plan.md

## 0. Next-Phase Direction
migrate k1_spawn_probe_src to KpLog.h (3 raw fopen sites, LIVE 0.3.2-probe) and the inventory-sort-filter leftover truncate (InventorySortFilter.cpp:436) in parallel

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/fix_pipeline_plan.md`
- **Branch:** `KOTOR-II` (229ed69 PLAY_TEST_QUEUE: LHP-PT1 load-hang-probe 0.1.5 live log check)
- **Generated:** 2026-10-16 14:10:39 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
229ed69 PLAY_TEST_QUEUE: LHP-PT1 load-hang-probe 0.1.5 live log check
 patch_manager_mods/PLAY_TEST_QUEUE.md | 1 +
 1 file changed, 1 insertion(+)
```
- **Dirty Files (167):**
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
  - ... +157 more (run `git status --short`)

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
- **load-hang-probe 0.1.5 INSTALLED 2026-10-16 13:28** (user GO "install"): hot-swap/ship refuse NOT-LIVE ids, so the KPM mirror + `create-patch.py` ran via `kpm_hotswap.mirror/run_create_patch` (KPM stale kpatch 1.4.1 from 10-01 kept as `.lhp015_pre.bak`; KPM commit e54ad8b), then `kpm_reinstall.py --mod load-hang-probe` (46 patches applied, holster peer DLL 302eaba4 restored). `kpm_status`: LIVE fadc1aa8/fadc1aa8. Backups: `backups/load-hang-probe_reinstall_20261016_132844` (rollback: `kpm_reinstall.py --rollback <dir>`), `kplog.ini.bak_loadhang_20261016`; `[load-hang-probe] enabled=1 level=info channels=*` appended to kplog.ini. `hangprobe_debug.txt` (empty, from 10-01) already existed: probe is ACTIVE (log only, fix off, file-I/O capture on). Holster now reports LIVE-OTHER (was SWAP) with the same md5 pair: its DLL was restored, not reverted.
- Pending: live log check after the next play session (`kotor2_logs.py check load-hang-probe_log.txt --new`; expect `T=.. f=- load-hang-probe/main f.. load-hang-probe 0.1.5 loaded` first, then IAT hooks line, no `[...]` cuts). Caveat: the Steam exe's old `hangprobe_log.txt` (2.8 MB, 10-02) stays as is.

## Next batch: k1_spawn_probe_src KpLog + inventory-sort-filter truncate (2026-10-16, planned)
- User decisions: migrate k1-spawn-probe (LIVE 0.3.2-probe, 3 raw fopen: `writeLine` :58, `DllEnv::openLog` :102, `dumpBlock` :120) and, in parallel, **delete only** the `fopen("inventory_sort_filter_log.txt","w")` truncate at `InventorySortFilter.cpp:436` (sessions then split on the `inventory-sort-filter 0.2.0 loaded` line; NO KPatchInit/export change, stays hot-swappable; mod stays fallback-only because it never calls KpLog_Init).
- k1-spawn-probe facts: `KPatchInit` is already exported (no export change => hot-swappable); log name `k1-spawn-probe_log.txt` equals the core file name (same file either way); `ClientCore.h` `LogBudget` (3000/2500 lines per process) + `Env` (`line/openLog/closeLog/flush`) stay; dump rows are 16 dwords (~170 B < 232). `logfKind` buf is 1024 B so ADDTOAREA lines can exceed the 232-byte core cut: split like load-hang-probe `emit()` (220 B pieces, `+ ` continuation). Logging before `KpLog_Init` (KPatchInit's first `logf`) must come after `KpLog_Init(api)`; DllMain only calls DisableThreadLibraryCalls. Doc 38 line 237 gives the play check (`204tel` save, mouse droid + "K1 Probe Citizen").
- Gate lessons from load-hang-probe: bumping a version string breaks the Wine smoke that greps it (grep `smoke_*.cpp`/`smokes.toml` for the old version before the gate); `test_override_watch.py` race fixed (4b05287). `kpm_hotswap`/`kpm_ship` refuse NOT-LIVE ids; LIVE mods use `kpm_hotswap --prepare` then `--apply` (needs GO, game closed).

## k1-spawn-probe + inventory-sort-filter (2026-10-16, DONE, not installed)
- k1-spawn-probe 0.3.3-probe: KpLog.h (channel main, fallback+core file `k1-spawn-probe_log.txt`), `emit()` splits lines >220 B into `+ ` pieces, dumpBlock builds rows in char[200], openLog/closeLog/flush are no-ops (crash durability = KpLog flush thread, ~500 ms loss accepted). Commit 5cdb29b. LIVE is still 0.3.2.
- inventory-sort-filter 0.2.1: attach truncate deleted (+ ini comment). Commit 7788c58. LIVE is still 0.2.0.
- Gate PASS both ids; Opus review APPROVE (0 blocking); comment fixes applied. Open (not done): analyze_play_logs `analyze_inv` sums all sessions in the file without an INFO note (ranged does at :1081); stale "0.2.0 live" comments in check_atlas_windows.py:58, mods678_research/check_windows.py:53.
- Install (needs user GO, game closed): `python3 scripts/kpm_hotswap.py --mod k1-spawn-probe inventory-sort-filter --prepare` then `--apply`, plus kplog.ini section `[k1-spawn-probe] enabled=1 level=info channels=*` (back up first). Play rows KSP-PT1, ISF-PT1.

## area-hook-probe (2026-10-17, DONE, not installed)
- 0.2.0 -> 0.2.1: raw fopen log (2 sites) replaced by KpLog.h (channel main, file `area_hook_probe_log.txt`, old core fallback same name at INFO); emit() splits lines >220 B; lines start `t<ticks> `; attached line moved from DllMain to the new `KPatchInit` export (so install = full `kpm_reinstall.py --mod area-hook-probe`, not hot-swap; NOT-LIVE probe, hotswap/ship refuse it, see load-hang-probe 10-16 13:28 for the by-hand mirror route). kplog.ini needs `[area-hook-probe]`. Gate PASS. No Opus review (mechanical copy of load-hang-probe emit). Next raw-log mods: voice_probe_src (peer-owned, 1 site); dialogue_letterbox_fix_probe_src is superseded by 1.0.2-probe.
