# 19h. Hand-off: fold the dialogue-letterbox probe into the shared logging/probe library (2026-10-03)

**Status: DONE 2026-10-03 (built + smoked, not installed).** Commits: local-setup `2823ff6` (source + analyzer), `e77f557` (allowlist), `aea74cb` (smoke + fixtures); fork `3a86d89` (1.0.1 baseline), `b4e1db6` (1.0.2). Install checklist: 19e "Phase 3 (letterbox)" Task 6.

**For:** the shared-logging session (owner of docs 19/19e/19f, `KpLog.h`, mouse-probe 0.2.0).
**From:** the vanilla-exe/launcher session, at the user's request ("generate a detailed handoff document so the probe session can integrate this probe as well").
**Subject:** `k2-dialogue-letterbox-fix` and its log-only probe variant `1.0.1-probe` (doc 15 finding L1, build doc 17).
**Coordinate with:** the engine-bug session (docs 15d/15e/16/17, `engine_bug_research/`). It owns the fix, the probe and the pending T1 decision. 19e's adoption table already lists `k2-dialogue-letterbox-fix` under "engine research / engine-bug: pending".

Everything below was checked on disk at ~22:30 on 10-03 unless marked otherwise. Re-check the "Live state" block before acting, because sibling sessions reinstall often.

---

## 1. Why this needs doing

The probe is the clearest case of the problem doc 19 set out to solve:

1. **It is a separate build with the same patch id.** `1.0.1-probe` reuses `k2-dialogue-letterbox-fix` on purpose, so installing it replaces the fix. The same id in two `Patches/` dirs makes KPM `tools/validate-patches.py` FAIL (doc 20d F1).
   - Its catalog dir `Patches/K2DialogueLetterboxFixProbe/` has been removed and recreated several times today. One session's rebuild recipe creates it; doc 20d's cleanup deletes it. The 15:29 recreation was the engine-bug session building stage B (15e: "probe dir vanished; restored from the mirror").
   - The fix-phase1 session removed it again and asked the other sessions to stop recreating it.
2. **Its version is stale and collides.** The probe is `1.0.1-probe`, but it was built on **1.0.0** (10-03 01:58). The fix itself has since moved to **1.0.1**, which makes `log=0` the default and keeps the load line, the first APPLY pair, the first SETTOP/SETBOTTOM and problem lines (KPM `2efb746`).
   - So "1.0.1-probe" is not 1.0.1 plus the probe. Installing stage B today would also roll the fix back to 1.0.0 behaviour.
3. **It has its own logger.** The logger is `fopen`/`fprintf`/`fclose` per line to `dialogue_letterbox_fix_log.txt`, with `[GetTickCount]` prefixes. It has its own caps (5000 total, 400 SETTOP/SETBOTTOM, 3000 PROBE) and its own rate limiter (first 120 per window, then 1/s, reset at SETTOP). On/off is controlled by INI keys `log=` and `probe=`. That's exactly the per-mod pattern KpLog.h replaces.

## 2. Live state (verify again before acting)

| item | value |
|---|---|
| live exe | vanilla `6A522E71` + `4gb-patch` = `7751F7D8` |
| live core `KotorPatcher.dll` | `36f10b7f` = registry core with `kpatch.log` (restored 16:44 by the engine-bug install). `bin/NoLaunch/` and `bin/Debug/` both ship `36f10b7f` now. `bin/NoLaunch_old_0928` ships `2a706ac9` and would revert it. |
| live `k2-dialogue-letterbox-fix.dll` | md5 `cf45a03a…` = **1.0.1** (`KPM/Patches/K2DialogueLetterboxFix/K2DialogueLetterboxFix.kpatch`, built 15:24; installed at the 16:44 install, per 15e line 23) |
| live INI | `dialogue_letterbox_fix.ini` with `log=1` (per 15e; re-read it) |
| live `kplog.ini` | `[global] enabled=1 sink=perpatch` + `[portrait-slots]` (19e chunk A) |
| fix source of record | `patch_manager_mods/dialogue_letterbox_fix_src/` = `KPM/Patches/K2DialogueLetterboxFix/` (byte-identical, `cmp` OK) |
| probe source of record | `patch_manager_mods/dialogue_letterbox_fix_probe_src/` (only copy of its smoke harness + README; built on 1.0.0) |
| probe kpatch | `KPM/letterbox_probe_stage/K2DialogueLetterboxFixProbe.kpatch` (10-03 01:58); also in stage B `~/Games/KOTOR2_Mod_Workspace/stage_lbprobe_1003_1529/` |
| stage A (1.0.1) | `~/Games/KOTOR2_Mod_Workspace/stage_l1_101_1003_1529/` (now superseded: 1.0.1 is live) |
| pending decision | **T1 screenshot (user):** is the top letterbox band visible at 3840x1600? Yes: L1 accepted, no probe needed. No: install the probe, set `probe=1`, play one conversation, get the analyzer's DIAGNOSIS, then Opus designs the fix (15e A3, register 16 T1 card). |

**Hard constraint:** do not break the T1 "no bands" path. Until T1 is answered, a working probe build must stay installable. Either keep stage B as is, or replace it with the KpLog-based probe described below once that passes its smoke test and analyzer tests.

## 3. What the probe measures (summary; full spec in `dialogue_letterbox_fix_probe_src/README.md`)

The fix has 3 detours:
- `dlbOnScreenSize` @0x411AC1 writes the constant `c` at 0x9A7D30.
- `dlbLogSetTop` @0x8BAD7C and `dlbLogSetBottom` @0x8BAF01 are log-only (plus a one-shot LATE fallback).

The probe adds 5 log-only detours (design: `engine_research_findings/V8b.md` (c)):

| window | address | handler | where | heat |
|---|---|---|---|---|
| W1 | 0x8BB461 | `dlbProbeW1` | letterbox Draw, top bar after SetExtent | per frame during slide |
| W2 | 0x8BB52B | `dlbProbeW2` | same, bottom bar | per frame during slide |
| W3 | 0x8BB538 | `dlbProbeW3` | before base panel Draw (x87 FLD/FSTP stolen) | per frame |
| W4 | 0x40ED9F | `dlbProbeW4` | base `CSWGuiPanel::Draw` after fill | **HOT: every panel, every frame**; filters on vtable `0x009A7CBC` |
| W5 | 0x7CEB95 | `dlbProbeW5` | `CGuiInGame::SetGlobalDialogState` | per conversation start/end |

All reads go through `IsBadReadPtr`-guarded copies. Floats are logged as raw bits; the handlers do no float work.

**Expected signals** at 3840x1600 in h6 mode:
- W1: 34 records, h 8..267.
- W3: `WOULD-DRAW`, alpha `0x3f000000`.
- W4: `eax=1`.
- W5: arg 1 at the start, arg 0 at the end.

The full table is in the probe README, "Expected signals".

## 4. Recommended design

**Recommendation: one source, two build variants, all lines through KpLog.h.** Don't make the probe hooks permanent in the shipping fix.

- **Why not one always-on DLL?** W4 is a detour on `CSWGuiPanel::Draw` for every panel, every frame. Even an early return pays the KPM wrapper (register save + FXSAVE/FXRSTOR) on each call. Doc 10 measured about 6100-6200 cycles per call averaged over perf-lab's hooks, so W4 alone could cost real frame time on GUI-heavy screens. Logging channels can switch output off but can't remove a hook; only `patch_config.toml` decides what is hooked.
- **Variant layout:**
  - `Patches/K2DialogueLetterboxFix/K2DialogueLetterboxFix.cpp` becomes the single source. Probe handlers sit behind `#ifdef DLB_PROBE` (or are always compiled but exported only in the probe variant; pick one, and keep `create-patch`'s export scan in mind, 19f point 2).
  - The shipping variant keeps the 3-hook `kotor2-steam-aspyr.hooks.toml` and its `exports.def`.
  - The probe variant needs its own hooks toml (3 + 5 hooks) and exports.def. Build it from a **temporary dir outside the catalog**, or from a dir under `Patches/` that is created and deleted in the same command, the way the probe README does it today. Its output goes only to `KPM/letterbox_probe_stage/`. Never leave a second `Patches/` dir with this id.
  - Look first for an existing variant mechanism in `create-patch.py` (e.g. a selectable hooks file or `-D` flags). If none exists, a small wrapper script in `patch_manager_mods/dialogue_letterbox_fix_src/` that assembles the probe build dir is enough. Don't edit `create-patch.py` for this without the user.
- **Versions:** the shipping fix with KpLog becomes **1.0.2**. The probe variant becomes **1.0.2-probe**, built from the same commit. That kills the "1.0.1-probe is really 1.0.0" trap.
- **Alternative** (if you find the wrapper cost is negligible, measured with perf-lab, not assumed): put W1/W2/W3/W5 into the shipping fix behind a channel, and keep only W4 in the probe variant. Record the measurement in 19e if you go this way.

## 5. KpLog mapping (follow 19f's 8 points; mouse-probe 0.2.0 is the reference)

`KP_LOG_PATCH("k2-dialogue-letterbox-fix");` (must equal the manifest id). Add `KPatchInit` literally in the `.cpp` and in the committed `exports.def`; the fix has none today.

**Proposed channels** (≤12 chars, lowercase, stable because users type them in `kplog.ini`):

| channel | lines (tag text unchanged) | old gate |
|---|---|---|
| `main` | `loaded:` line, `APPLY src=…` header + result (`wrote …; verify OK`, `unchanged`, `DISABLED, no write`) | `log=1` (1.0.1: first APPLY pair always) |
| `set` | `SETTOP` / `SETBOTTOM` field lines, `LATE` fallback notice | `log=1` (+ first pair always), own cap 400 |
| `probe` | `PROBE W1/W2/W3/W5` | `probe=1` AND `log=1` |
| `probe_w4` | `PROBE W4` (separate so it can be silenced on its own; still filtered to the letterbox vtable) | same |

**These go to `KP_WARN` / `KP_ERR`:** `INVALID size`, `FOREIGN value … another mod owns`, `VirtualProtect(...) FAILED`, `verify MISMATCH`, `unknown mode`, null manager. 1.0.1 already always logs these as "problem lines". That keeps them visible with no `kplog.ini`, matching today's `log=0` default.

**What happens to the INI keys** (19f points 4 and 5):
- `enabled=` and `mode=` are **behavioural kill switches. Keep them unchanged** in `dialogue_letterbox_fix.ini`.
- `log=` and `probe=` are logging switches. On a new core, `kplog.ini` `[k2-dialogue-letterbox-fix] enabled=1 channels=…` replaces them. On an old core (`!KpLog_HasCore()`) they drive `KpLog_SetFallbackLevel` and the probe-channel gate. Keep `KpLog_SetFallbackFile("dialogue_letterbox_fix_log.txt")` so the old core writes today's file name.
- `probe=0` must still make every probe handler return on its first instruction (cheap check, no I/O, no `KP_ENABLED` call needed if a plain bool is cheaper). In the probe variant, also gate on `KP_ENABLED(chProbe, KPLOG_INFO)` before formatting.

**Rate limits and caps:**
- The core drops above 2000 INFO lines/s per patch. The probe's own limiter (first 120 records per window then 1/s, reset at SETTOP, global 3000 cap) is *content* logic the analyzer relies on (`n=` and `skip=` fields), so keep it.
- Drop the 5000-line total cap: the core rotates at `max_bytes`, and the fallback sink caps at 60000.
- Keep or drop the 400-line SET cap, and say which in the doc.
- Use `KP_LOG_EVERY` only where it doesn't change the `n=`/`skip=` semantics.

**232-byte line cut (19f pitfall).**
- Today's probe formats into `char line[700]`. `PROBE W3` carries vtbl, flags, bit2, bit6, rect, alpha, color, mgr, W, H, predicate, dt and three DAT values, so it is very likely over 232 bytes. W1/W2 may be too.
- 1.0.1's longest smoke line is 183 bytes (`fixtures/letterbox_log_101_smoke_default.txt`).
- **Measure each probe line's length** in the smoke output. For any line over ~220 bytes, either split it (e.g. `PROBE W3 n= … predicate=…` + `PROBE W3b n= … color=… DAT_…`), keeping `n=` in both so the analyzer can join them, or move the fields the analyzer needs to the front. Update `W3_RE` in the same change.

**Frame tick:** do **not** call `KP_FRAME_TICK()`. W4 looks frame-global but is per-panel, and no letterbox hook is frame-global (19f point 7).

**Prefix:** on a new core the core prefix `T=<ms> f=<frame> k2-dialogue-letterbox-fix/<ch> ` replaces `[GetTickCount]`. Drop the own clock unless the analyzer needs it.

## 6. Parser / analyzer impact (must change in the same commit)

`engine_bug_research/tools/analyze_engfix_logs.py`:
- Every regex is `^`-anchored. `LINE_RE = ^\[(\d+)\]\s?(.*)$` peels the `[ms]` prefix, then `LOAD_RE`, `APPLY_*`, `SET_RE`, `PROBE_RE`, `W12_RE`, `W3_RE` and the rest match the bare message.
- Change `LINE_RE` handling to accept three shapes: legacy `[ms] text`; new-core `T=<ms> f=<n> k2-dialogue-letterbox-fix/<ch> text`; old-core fallback `<ms> k2-dialogue-letterbox-fix/<ch> text`.
- Easiest is `strip_prefix` from `scripts/kplog_parse.py` (exists) plus the legacy branch.
- Keep the old shape working: the fixtures `letterbox_log_1002_0238.txt`, `letterbox_log_1003_0112.txt` and `letterbox_log_101_smoke_{default,log0}.txt` are legacy and must still parse.
- The file name changes on a new core: `k2-dialogue-letterbox-fix_log.txt` (perpatch) instead of `dialogue_letterbox_fix_log.txt`. The analyzer's default input path and the doc 17 / 15e "read the log" instructions need both names.
- Add new fixtures (new-core and fallback shapes, including a split W3 if you split it) and run `engine_bug_research/tools/test_analyze_engfix_logs.py`.
- `persist_phase2_db.py` and `persist_phase4_db.py` also grep these tags. Check that they still find them.

## 7. Tests and gates

1. **Smoke harness.** Port `dialogue_letterbox_fix_probe_src/smoke_dialogueletterbox.cpp` (12 modes: h6 clamp vanilla disabled foreign unchanged late ini log0 probe probe0 probelog0) onto the merged source. The fix dir has its own 1.0.1 harness (`dialogue_letterbox_fix_src/smoke_dialogueletterbox.cpp`). Merge the two into one harness that runs against both variants.
   - The harness has no core, so it exercises the **fallback** path (`KPatchInit` never called). Add one mode that calls `KPatchInit` with a stub `KPatchApi` if mouse-probe's tests have one (check `KPM/tests/` and doc 20's `kplog-check`).
   - Recipe: probe README "Smoke test" (Wine from Proton Experimental, `WINEPREFIX` in `/tmp/claude-1000/…`).
2. **Window check:** `python3 engine_bug_research/tools/check_engfix_windows.py` (9 windows ALL OK, both exes ALL MATCH). Addresses don't change, but re-run after rebuilding.
3. **Doc 20 gates:** `kpm-validate-patches` must stay green (no duplicate id dir). Also run `check_hook_bytes`, `build_check.py` and the 19e `kplog-check`.
4. **DLL check:**
   - `objdump -p` lists `KPatchInit` and the handlers: 3 in shipping, 8 in probe.
   - Imports: KERNEL32 + msvcrt only. Watch for the std::chrono `SetSystemTime` import trap that mouse-probe hit; it's fixed in KpLog.h 22add9a, so use the merged header.
   - Note the DLL sizes.
5. **Install** (only with the user's go; game closed). Follow CLAUDE.md "KPM Install Rule" + doc 10 "Standing rules":
   1. Commit the source on `ethansm/local-mods` first.
   2. Run `mods678_research/stage_and_verify_install.py stage <dir> <new kpatch>`. It exits 2 on dirty sources. A `dll=NO-MATCH` for another id means you pass that id's live kpatch as an EXTRA.
   3. Install with `KPATCH_NO_LAUNCH=1 bin/NoLaunch/KPatchLauncher …`.
   4. Run `verify`, re-md5 `KotorPatcher.dll` (must stay `36f10b7f`), and note the `STAGE_INFO.txt` head in doc 17.
6. **Live A/B** (user plays): one conversation with no `kplog.ini` section (only WARN/ERR lines may appear), then one with `[k2-dialogue-letterbox-fix] enabled=1 channels=main,set` (+ `probe,probe_w4` for the probe variant). Run the analyzer on both. Check for `DROPPED` lines.

## 8. Ownership and order (proposed; confirm with the user and the engine-bug session)

1. **Engine-bug session:** finish T1 first, or agree that the KpLog probe replaces stage B for the "no bands" path. It owns doc 17 and the fix's behaviour (`mode`, constant write, FOREIGN guard). **Do not change behaviour in this migration.**
2. **Shared-logging session (you):** the logging migration (sections 4-6), smoke merge, analyzer change, fixtures, 19e adoption-table row, doc 17 "Planned shared state and shared code" note. Commit per 19f; the fix dir is mirrored in `patch_manager_mods/dialogue_letterbox_fix_src/`, so keep both copies byte-identical.
3. **Upstream:** the logger's upstream PR is triggered by "whichever mod ships first with the logger" (19e/19g). This fix is also an upstream candidate (`engine_bug_research/upstream_kpm_k2_status_note.patch`). The user said **no upstream PRs** as of 10-03 (this session's plan). Tell the user if this fix becomes the first shipped KpLog mod; post nothing.

## 9. Gotchas collected today

- **Don't leave `Patches/K2DialogueLetterboxFixProbe/` behind.** The fix-phase1 session deletes it on sight (20d F1), and `.git/info/exclude` no longer hides it, so it shows in `git status`.
- **Sibling cleanups delete untracked KPM build dirs.** Kpatches also live in stage dirs and `KPM/letterbox_probe_stage/`, and `patch_manager_mods/*_src/` is the source of record.
- **The live core has been reverted to `2a706ac9` more than once** (installs via an old bin, or `bin/Debug` before it was updated). Re-md5 before reading any test result. `bin/NoLaunch_old_0928` is the remaining old-core bin.
- **`stage_and_verify_install.py` picks the newest kpatch per id when no DLL md5 matches.** If the probe kpatch is ever back in the tree, a stage may pick it for `k2-dialogue-letterbox-fix`. Check the `<-` path in the stage output.
- **`log=0` is the 1.0.1 default.** A 1.0.1 log with only a handful of lines is normal, not a bug.
- **The SetReply lead** (0x8BB8D0 returns early while the top bar's `[+0x8C]` done flag is 0; 15e, UNCLEAR, low risk) is a T1 add-on. If you add a probe window for it, put it in the probe variant, give it its own tag, and coordinate with the engine-bug session.

## 10. Files to read, in order

1. `patch_manager_mods/19f_kplog_migration_guide.md` (8-point checklist) and `mouse_probe_src/MouseProbe.cpp` (reference).
2. `patch_manager_mods/dialogue_letterbox_fix_probe_src/README.md` (probe spec, smoke, expected signals).
3. `patch_manager_mods/dialogue_letterbox_fix_src/README.md` + `K2DialogueLetterboxFix.cpp` (1.0.1 source of record).
4. `patch_manager_mods/15e_phase4_session_context_2026-10-03.md` (current state, stage A/B, T1) and `17_dialogue_letterbox_and_wrap_fixes.md` (DECISION + STATUS at top).
5. `engine_research_findings/V8b.md` (probe design) and `engine_bug_research/tools/analyze_engfix_logs.py` (+ its test).
6. `patch_manager_mods/19e_phase2_plan.md` (adoption table, chunk M lessons) and KPM `docs/PatchRegistry.md` "Logging with KpLog.h".
