# Engine Fixes L1 + L4: dialogue letterbox and wrap-measure (doc 15 Phase 2)

**INSTALLED 2026-10-03 22:42 (user: "install"):** 1.0.2 DLL `7b3936bc` hot-swapped into `patches/k2-dialogue-letterbox-fix.dll` (hooks unchanged; the core runs `KPatchInit` by export at module load). `kplog.ini` += `[k2-dialogue-letterbox-fix] enabled=1 level=info channels=main,set`; `dialogue_letterbox_fix.ini` unchanged (`enabled=1 mode=h6 log=1`). Core `36f10b7f` re-checked; exe md5 `62411a3a` unchanged. Backup `~/Games/KOTOR2_Mod_Workspace/backups/pre_lb102_1003_2242/`. Source fork `b4e1db6`. Expect `k2-dialogue-letterbox-fix_log.txt`; the old `dialogue_letterbox_fix_log.txt` (10524 B, 01:30) must not grow. Rollback: `cp $B/k2-dialogue-letterbox-fix.dll "$G/patches/"; cp $B/kplog.ini "$G/"`. **NOT VALIDATED yet: see "Manual validation checklist (1.0.2)" (17 manual checks).**

**1.0.2 + 1.0.2-probe BUILT + SMOKED, NOT installed (2026-10-03, shared-logging session, 19h):** logging moved to KpLog.h with no behaviour change. One source; the probe variant is `dialogue_letterbox_fix_src/probe/` + `build_probe.sh`. Local-setup `2823ff6`/`aea74cb`, fork `b4e1db6`. Kpatches: shipping `~/Games/KOTOR2_Mod_Workspace/stage_lb_102/K2DialogueLetterboxFix.kpatch` (DLL `7b3936bc`), probe `KPM/letterbox_probe_stage/1.0.2-probe/K2DialogueLetterboxFixProbe.kpatch` (DLL `969fe458`). Wine smoke: shipping 11 modes, probe 14 modes. Read the log with `analyze_engfix_logs.py`: the new core writes `k2-dialogue-letterbox-fix_log.txt` (needs `kplog.ini` `[k2-dialogue-letterbox-fix] enabled=1 level=info channels=main,set[,probe,probe_w4]`); an old core keeps writing `dialogue_letterbox_fix_log.txt`. Install checklist: 19e "Phase 3 (letterbox)" Task 6. For the T1 "no bands" path, use 1.0.2-probe; stage B stays as the fallback.

**INSTALLED 2026-10-03 16:44 (user: "install everything that's ready for testing, ensure proper logging"):** L1 **1.0.1** DLL `cf45a03a` hot-swapped into `patches/k2-dialogue-letterbox-fix.dll` (hooks identical to 1.0.0, so no config change); **L4 dropped** (its runtime `simple` block removed from `patch_config.toml`, now 32 patches / 197 hooks, md5 `590360b5`; the exe bytes at 0x47B232 were already vanilla); `dialogue_letterbox_fix.ini` = `enabled=1 mode=h6 log=1` (full logging for the T1 session). Also restored the **registry core** `KotorPatcher.dll` `36f10b7f` (the live core had been reverted to the old `2a706ac9`, which leaves portrait-slots / buff 2.0.0 / shield-bar inert and has no kpatch.log); `bin/Debug/KotorPatcher.dll` updated to `36f10b7f` too (it was the source of the revert). Static check: every config DLL exists, every hook function is exported, no duplicate address, portrait-slots before its consumers. `kpm_install_state.json` NOT edited (still lists L4; the split ids were never in it). Backup `~/Games/KOTOR2_Mod_Workspace/backups/pre_phase4_install_1003_1644/` (state json, config, core, kplog.ini, patches/, old bin/Debug core). Stage A/B dirs are now superseded for L1 (stage B still valid input for a probe install if no bands).

**DECISION (2026-10-03, user): DROP L4 (`k2-wrap-measure-fix`) at the next install** (no-op at fs 1.0/2.0/3.0, V8; still valid for fs 1.5). Source mirror `wrap_measure_fix_src/` kept. Phase 4 (`engine_bug_research/PHASE4_PLAN.md`): L1 **1.0.1 BUILT** (log=0 default; kpatch `f1e59f7b...`, NOT installed) and **stage A (1.0.1) / stage B (probe) ready** without L4, see "Install (Phase 4 ...)"; V11 (10-03): L1 moves/clips nothing else (NO BUG). Live set re-read 10-03 11:4x: 31 ids, `kpm_install_state.json` UpdatedAt 2026-10-03 11:29 (sibling hotbar reinstall), `swkotor2.exe` md5 `62411a3a3282ecf92bdf54aeb1bac671`; `stage_lbprobe_1003_0200` is therefore STALE.

**STATUS (2026-10-03 01:15): T1 computation PASS, visual UNKNOWN; T2 not run.** The 10-03 01:12 session log (60 lines, fixture `engine_bug_research/fixtures/letterbox_log_1003_0112.txt`) has 4 launches, each `APPLY ... newBar=267 ... verify OK`, and **12 SETTOP/SETBOTTOM pairs** (dialogue starts), all target `(0,0,3840,267)` / `(0,1333,3840,267)`, start h 0 / start y 1600, **0 error lines**. The user's visual answer: "didn't notice / no bars"; no screenshots were taken, so whether the bars are drawn is unknown (the draw path after SetExtent, V1 B5, was never read). Phase 3 (`engine_bug_research/PHASE3_PLAN.md`) A2 = F12 screenshots + an `enabled=0` A/B run. **V8b (10-03):** draw path explained (bars = 50 % black quads via fill 0x479460, drawn for every ConversationType except 1 = computer); probe windows W1-W5 ready if bars are absent; **1.0.1-probe BUILT + STAGED, NOT installed** (mirror `dialogue_letterbox_fix_probe_src/`, kpatch now in KPM `letterbox_probe_stage/` (moved out of `Patches/` 10-03, doc 20d F1: duplicate id fails KPM validate; rebuild steps in the probe README), kpatch md5 `4d680986...`, DLL `e46cfcc8...`, INI `probe=1` default 0; Wine smoke 12 modes pass; checker ALL OK + PROXIMITY OK, both exes MATCH; stage `~/Games/KOTOR2_Mod_Workspace/stage_lbprobe_1003_0200`, PROBLEMS 1 = known 4gb-patch hooks=DIFF artifact). Install only if screenshots show no bands and the user says go. The 10-03 01:30-02:00 session had 0 dialogues and no screenshots (copy `~/Games/KOTOR2_Mod_Workspace/playlogs_engfix_1003_0200/`), so T1 visual is still open. **V8 (10-03): L4 is a no-op at fs 1.0/2.0** (stock TXI glyph advances are integer-pixel; +0.25 and +0.5 agree on all 95 ASCII glyphs), so T2 cannot show a difference at 3840x1600. Live set re-read 10-03: 31 ids / 198 hooks (`kpm_install_state.json` UpdatedAt 2026-10-03 00:24, sibling reinstall), both fixes still installed.

Previous status: **STATUS (2026-10-02 01:33): INSTALLED LIVE (user: "install both"), awaiting test cards T1/T2.** Live = 28 ids / 153 hooks (26 previous + `k2-wrap-measure-fix` + `k2-dialogue-letterbox-fix`); `stage_and_verify_install.py verify`: mismatches 0; `swkotor2.exe` md5 unchanged; no INI in the game dir (defaults enabled=1 mode=h6 log=1). Pre-install backup: `~/Games/KOTOR2_Mod_Workspace/backups/pre_engfix_1002_0133/` (kpm_install_state.json, patch_config.toml, patches/). Run A (without) is no longer available by reinstall-free means except `enabled=0` in the INI for L1; L4 has no switch (rollback = reinstall the 26 ids from the stage dir or restore the backup).

Original build status (2026-10-02, built + staged): two engine fixes from the confirmed leads of doc 16.
Nothing was launched, nothing was written to the game dir (`swkotor2.exe` md5 `c9a1e3c57fd5491b4256dac1c0ab97cb`; `patch_config.toml`, `kpm_install_state.json`, `patches/` mtimes unchanged).
- Stage dir: `/home/ethansm/Games/KOTOR2_Mod_Workspace/stage_engfix_1002_0121` (28 kpatches = the 26 live ids, each DLL md5 + hook set identical to live, plus the 2 new ones; PROBLEMS 0).
- `k2-wrap-measure-fix` 1.0.0 (L4): kpatch md5 `11bfc231e0b10dbfa1706a475453fcda`, no DLL. Source mirror `wrap_measure_fix_src/`.
- `k2-dialogue-letterbox-fix` 1.0.0 (L1): kpatch md5 `6033b7003c32caddce5933a3daa8139f`, DLL (`binaries/windows_x86.dll`) md5 `9bbbfcddd08ca5de1ceabc651878ce54` (imports KERNEL32 + msvcrt only). Source mirror `dialogue_letterbox_fix_src/` (its README has the full detail).
- Commits: `7d7e310` S1 (V7 hook point + window checker), `0fa8046` S2 (L4 build), `40e2161` S3 (L1 build + Wine smoke harness).
- Sibling sessions reinstall at any time. If the live set changes before the user installs, re-run the stage command (`mods678_research/stage_and_verify_install.py stage <NEW_STAGE> <the 2 new kpatches>`) instead of using this stage dir.

Evidence: `engine_research_findings/V1.md` (formula), `V3.md` (WrapStrings), `V7.md` (hook point; read its summary and "Opus verification"), `16_candidate_register.md` (rows L1/L4, test cards).

## Mechanism

### L1 dialogue letterbox
Bar height = `trunc((H - trunc(W / c)) / 2)`, `c` = the double at `.rdata` `0x009A7D30` (vanilla 2.3333330154418945, bytes `00 00 00 80 AA AA 02 40`). It is read by exactly 7 `FDIV double ptr [0x9A7D30]` (`DC 35 30 7D 9A 00`) at `0x8BACE9` SetTop, `0x8BADE9` SetBottom, `0x8BAF6E` (dead), `0x8BB6A9` DialogTop ctor, `0x8BB9A6` SetReply, `0x8BBC25` Cinematic ctor, `0x8BC467` Reset, and nothing else (whole-exe byte scan of both exes: exactly 7 hits).
Past 7:3 the bar goes negative: 3840x1600 -> -22, 3440x1440 -> -17, 5120x1440 -> -377 (bars vanish).

Fix: whenever the GUI screen size changes, write `c = 1.5 * W / H * (1 - 1e-9)` over the double. Then `W/c = 2H/3` and bar = `H/6` at every resolution. The `(1 - 1e-9)` nudge matters: with the naive double, `trunc(W/c)` lands one below `2H/3` for H divisible by 3 (388,370 of 27 M (W,H) pairs; 0 after the nudge; V7 Q5).
- W/H source: shorts at GUI manager `+0x6C` / `+0x6E`. Only writer: `CSWGuiManager::SetScreenSize` `0x411A80` (plus the ctor `0x4100C0` zeroing them). 4 callers: startup `0x78A20F`, options Apply `0x78DDA7`, credits 640x480 force `0x89222B` and restore `0x892756`.
- Hook `0x411AC1` (7 B) sits after both shorts are stored (`0x411AAA` / `0x411AB5`) and after the rescale `CALL 0x413DD0`; the manager is `[ebp-0xC]` (ECX saved at `0x411A86`). It is reached only when W or H changed (early-out at `0x411AA1`), which is exactly when `c` must change.
- Write is via `VirtualProtect` (RW, write 8 bytes, restore the old protection). Guard: it writes only if the current 8 bytes are the vanilla bytes or its own last value; otherwise it logs `FOREIGN` and leaves them (another mod owns the constant).
- Bars with h6: 3840x1600 267 (vanilla -22), 1920x1080 180 (129), 3440x1440 240 (-17), 5120x1440 240 (-377), 800x600 100 (129), 640x480 80. `clamp` mode: 3840x1600 -> 0 (vanilla bar only where it would be negative).

### L4 glyph advance
`CAurGUIStringInternal::WrapStrings` `0x47AF30` measures each glyph as `ftol(x + 0.25)` (`FADD double [0x98C1D8]` at `0x47B232`, truncated), while Draw uses exact float widths: wrapped lines are about 0.25 px/char wider than measured (text can touch the scrollbar). The patch re-points that one operand to the existing 0.5 double at `0x987F08`, so the truncation becomes round-to-nearest. The 0.25 constant is not changed (48 other references). KMRP "K6" equivalent.

## Hooks

| Patch | Address | Window | Type | Function | Param |
|---|---|---|---|---|---|
| `k2-wrap-measure-fix` | `0x0047B232` | 6 B `DC 05 D8 C1 98 00` -> `DC 05 08 7F 98 00` | `simple` (byte write after VerifyBytes) | in `CAurGUIStringInternal::WrapStrings` `0x47AF30` | none |
| `k2-dialogue-letterbox-fix` | `0x00411AC1` | 7 B `8B 55 F4 83 7A 7C 00` | `detour` (cdecl) | `dlbOnScreenSize`, in `CSWGuiManager::SetScreenSize` `0x411A80` | `ebp` (pointer); manager = `[ebp-0xC]` |
| `k2-dialogue-letterbox-fix` | `0x008BAD7C` | 10 B `C7 81 88 00 00 00 01 00 00 00` | `detour` | `dlbLogSetTop`, last store of `CSWGuiDialogLetterbox::SetTop` `0x8BACA0` | `ecx` (pointer) = letterbox this |
| `k2-dialogue-letterbox-fix` | `0x008BAF01` | 10 B `C7 81 88 00 00 00 02 00 00 00` | `detour` | `dlbLogSetBottom`, last store of `SetBottom` `0x8BADA0` | `ecx` (pointer) |

The two log hooks only log (plus a one-shot LATE fallback if the SetScreenSize hook was never seen). The KPM detour handler runs BEFORE the stolen bytes, so `+0x88` in the log shows the old mode. Identical bytes in the LAA exe and the base exe (`swkotor2.exe.unpatched_vanilla`).

## Config and log (L1)
`dialogue_letterbox_fix.ini` in the game dir, read once at load; a missing file = defaults:
```
enabled=1     ; 0 = never write (log "DISABLED, no write")
mode=h6       ; h6 (default): c = 1.5*W/H*(1-1e-9); clamp: only when the vanilla bar would be negative (c = W/H*(1-1e-9), bar 0); vanilla: keep/restore vanilla c
log=1         ; 0 = only the load line and problem lines (INVALID/FOREIGN/protect/verify)
```
Log `dialogue_letterbox_fix_log.txt` (game dir, appended, `[GetTickCount] text`, max 5000 lines per process):
- `loaded: enabled=1 mode=h6 log=1 const@0x009a7d30=<8 bytes> (<double>) pid=N`
- `APPLY src=<SetScreenSize|LATE(SetTop)|LATE(SetBottom)> W= H= mode= old= new= vanillaBar= newBar= X= fs= panelH= replyY=` then `wrote (protect RW ok ...; protect restore ok -> ...); verify OK, now <bytes> = <double>` (or `unchanged`, `DISABLED, no write`, `INVALID size, no write`, `FOREIGN value ...`).
- `SETTOP` / `SETBOTTOM lb= mode(+0x88)= start(x,y,w,h)= target(x,y,w,h)= mgr= W= H= c=` (max 400 lines per session).

## Risk screen (2026-10-02)
- `engine_bug_research/tools/check_engfix_windows.py` (wraps `mods678_research/check_windows.py`; output `engine_bug_research/tools/check_engfix_windows_output_2026-10-02.txt`): all 4 windows OK, no overlap and nothing within +-8 bytes among 269 live hooks (09-30g snapshot 120 + live `patch_config.toml` 26 ids / 149 hooks). `mods678_research/verify_windows_both_exes.py`-style check: all 4 windows identical in the LAA and the base exe.
- L1 vs `semi-transparent-letterbox`: orthogonal (alpha-only hooks at `0x8BAA5E`, `0x8BB043`, `0x8BB108`, `0x8BB3C3`; L1 changes only the bar height).
- No GL state touched by either fix. L1 imports KERNEL32 + msvcrt only.
- Wine smoke test (`dialogue_letterbox_fix_src/smoke_dialogueletterbox.cpp`, no game): 9 modes pass (h6 / clamp / vanilla / disabled / foreign / unchanged / late / ini / log0): bars as listed above, vanilla/disabled keep the bytes, foreign refuses, protection restored to `PAGE_READONLY`.

## Install (user-gated; the session never installs)
`G=~/.local/share/Steam/steamapps/common/Knights\ of\ the\ Old\ Republic\ II`. Procedure = `mods678_research/stage_and_verify_install.py` header + doc 10 "Standing rules". Phase 4 stage dirs and the exact commands: see "Install (Phase 4, L1 1.0.1 / probe, without L4)" below.
1. Game CLOSED: `pgrep -x swkotor2.exe` (or `pgrep -f '^S:.*swkotor2\.exe'`) prints nothing.
2. Re-read `$G/kpm_install_state.json` (`UpdatedAt` + `InstalledPatches`) right before; if it changed since the stage (or any `$G/patches/*.dll` is newer than the stage), re-stage into a NEW dir first.
3. Back up `kpm_install_state.json`, `patch_config.toml` and `patches/` into a dated dir.
4. Install with the id list = **the live ids MINUS `k2-wrap-measure-fix`** (decision 10-03). Live set 10-03 11:29 minus L4 = 30 ids:
```
additional-console-commands buff-duration-hud equipment-3d-character force-power-hotbar good-alignment-heads inventory-3d-viewport \
 loading_screen_line_fix_k2 k2-aspyr-map-aspect-fix k2-visible-console k2-finesse-melee-fix k2_grass_fog_fix k2-mp3-playback-fix \
 max-lights perf-lab post-combat-movement-fix-v1 semi-transparent-letterbox inventory-sort-filter character-effects-panel \
 k2-activation-fix k2-equip-icon-fix ranged-ai k2-dialogue-letterbox-fix level-up-override companion-ai 4gb-patch \
 k2_aspyr_shader_fixes mouse-probe K2_music_fix k2-robustness-fixes k2-texture-memory-release
```
5. Verify: `python3 /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research/stage_and_verify_install.py verify "$STAGE"` (live DLL md5s vs the staged kpatches, hook counts). Afterwards diff the `[Config] Loaded DETOUR` lines against the previous Proton log.

Original 10-02 install (historical, 28 ids from `stage_engfix_1002_0121`, now stale): the 26 then-live ids + `k2-wrap-measure-fix k2-dialogue-letterbox-fix`.

## Install (Phase 4, L1 1.0.1 / probe, without L4)
Staged 2026-10-03 15:29 against the live set read at that time: 31 ids, `kpm_install_state.json` UpdatedAt **2026-10-03 12:00:17**, `swkotor2.exe` md5 `62411a3a3282ecf92bdf54aeb1bac671`. Both stage dirs hold 32 kpatches (the 31 live builds incl. `K2WrapMeasureFix.kpatch`, which the id list below simply leaves out, + the new letterbox build); PROBLEMS 1 = the known `4gb-patch` hooks=DIFF artifact. STAGE_INFO head `28ae185ef061`.
**Both stage dirs go STALE on any sibling reinstall: at install time re-read UpdatedAt + `InstalledPatches`; if either changed, re-stage into a NEW dir** (`mods678_research/stage_and_verify_install.py stage <NEW_DIR> <kpatch> --allow-dirty`; `--allow-dirty` because the KPM clone stays uncommitted, the repo `*_src/` mirrors are the committed copies).
- **Stage A (T1 passes: top band visible) = L1 1.0.1:** `STAGE=/home/ethansm/Games/KOTOR2_Mod_Workspace/stage_l1_101_1003_1529` (letterbox kpatch md5 `f1e59f7b12b339f2bb879cfb480a9b0c`, DLL `cf45a03af96881362e586c8baf0654b9`, log=0 default).
- **Stage B (no bands) = 1.0.1-probe:** `STAGE=/home/ethansm/Games/KOTOR2_Mod_Workspace/stage_lbprobe_1003_1529` (kpatch md5 `4d68098658809e25307117a9bb33f460`, DLL `e46cfcc8d5684de395d2c472f764deb0`); after the install the user sets `probe=1` in `$G/dialogue_letterbox_fix.ini` (game closed), has ONE conversation, then copy the log to `~/Games/KOTOR2_Mod_Workspace/playlogs_engfix_<MMDD_HHMM>/` and run `engine_bug_research/tools/analyze_engfix_logs.py` (per-dialogue DIAGNOSIS line).

Procedure (same for A and B; only `STAGE` differs; user-gated, game CLOSED: `pgrep -x swkotor2.exe` empty):
```
G="$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
BK=~/Games/KOTOR2_Mod_Workspace/backups/pre_l1_101_$(date +%m%d_%H%M); mkdir -p "$BK"
cp -p "$G/kpm_install_state.json" "$G/patch_config.toml" "$BK/" && cp -rp "$G/patches" "$BK/"
KPATCH_NO_LAUNCH=1 ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches "$STAGE" \
 additional-console-commands buff-duration-hud equipment-3d-character good-alignment-heads inventory-3d-viewport \
 loading_screen_line_fix_k2 k2-aspyr-map-aspect-fix k2-visible-console k2-finesse-melee-fix k2_grass_fog_fix k2-mp3-playback-fix \
 max-lights perf-lab post-combat-movement-fix-v1 semi-transparent-letterbox inventory-sort-filter character-effects-panel \
 k2-activation-fix k2-equip-icon-fix ranged-ai level-up-override companion-ai 4gb-patch k2_aspyr_shader_fixes mouse-probe \
 K2_music_fix k2-robustness-fixes k2-texture-memory-release force-power-hotbar k2-dialogue-letterbox-fix
python3 /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research/stage_and_verify_install.py verify "$STAGE"
md5sum "$G/swkotor2.exe"   # compare with the pre-install md5 (KPM rewrites the exe per install; record the new value)
```
30 ids = the live 31 minus `k2-wrap-measure-fix` (decision 10-03); `k2-dialogue-letterbox-fix` comes from the stage. Rollback: restore `$BK` (state json, config, patches/) or reinstall from the stage with the 1.0.0 kpatch (`~/Games/KOTOR2_Mod_Workspace/backups/k2_dialogue_letterbox_fix_1.0.0/`, md5 `6033b700...`).

## Manual validation checklist (1.0.2, live since 10-03 22:42): ALL must pass before "validated"
Status: **NOT VALIDATED, 0 / 17 done.** Setup: 3840x1600, live core `36f10b7f`, letterbox DLL `7b3936bc` (re-md5 both before each session), `kplog.ini` section `channels=main,set`, `dialogue_letterbox_fix.ini` `enabled=1 mode=h6 log=1`. Edit INI files only with the game closed, except `kplog.ini`, which the core re-reads about once a second. Log = `$G/k2-dialogue-letterbox-fix_log.txt`; analyzer = `python3 patch_manager_mods/engine_bug_research/tools/analyze_engfix_logs.py <log>`. Mark each row `[x] date + evidence` (screenshot path, log line, analyzer verdict).

**A. Logging (KpLog.h migration; the shared-logging session checks these)**
- [ ] A1 Log file: after launch + load, `k2-dialogue-letterbox-fix_log.txt` exists and starts with a `# kplog session` header, followed by `.../main k2-dialogue-letterbox-fix 1.0.2 loaded: enabled=1 mode=h6 log=1 const@...` exactly once per launch.
- [ ] A2 APPLY: a `main` line `APPLY src=SetScreenSize W=3840 H=1600 ... newBar=267 ...` followed by `wrote ...; verify OK`.
- [ ] A3 SET lines: the first conversation gives `.../set SETTOP ... target h 267` and `.../set SETBOTTOM ... target y 1333`, one pair per conversation.
- [ ] A4 Old file silent: `dialogue_letterbox_fix_log.txt` stays at 10524 B (01:30). It must not grow.
- [ ] A5 Clean: no `WARN`, `ERR`, `DROPPED` or `[...]` (line-cut) text in the log.
- [ ] A6 Analyzer: PASS on the new log, 0 unparsed lines.
- [ ] A7 Live channel switch: during play, set `channels=main` in `kplog.ini`, wait 2 s, start another conversation. No new SETTOP lines. Put `main,set` back.
- [ ] A8 No-ini run: remove the `[k2-dialogue-letterbox-fix]` section and play one conversation. Only WARN/ERR lines may appear (expect none, so no file or a header-only file). Restore the section.
- [ ] A9 `log=0` run (`dialogue_letterbox_fix.ini`): only the load line, the first APPLY pair and the first SETTOP/SETBOTTOM pair, then nothing more across 2+ conversations.

**B. Fix behaviour (engine-bug session; doc 16 card T1)**
- [ ] B1 T1 top band: F12 screenshot during an NPC line in a voiced conversation (Peragus or the first Kreia/Atton talk). A top band about 267 px tall is visible. **If it is absent, go to section C.**
- [ ] B2 Bottom band + reply list: screenshot with the reply list shown. Bottom band about 267 px from y 1333; reply list inside it, not clipped, scrollbar usable.
- [ ] B3 A/B: `enabled=0`, restart, same conversation. Expect no top band and a 200 px bottom band (vanilla). This shows the fix is what changes the picture.
- [ ] B4 Computer terminal conversation (ConversationType 1): no bars, as in vanilla. No crash.
- [ ] B5 Cinematic/cutscene conversation (a Bink movie or camera-animated scene): bars slide in and out normally, no stuck bar after the conversation ends.
- [ ] B6 Options > Graphics Apply (or a resolution change and back): a new APPLY line (`unchanged` or `wrote ... verify OK`), bars still right in the next conversation.
- [ ] B7 Module transition + save/load: 3+ conversations across 2 modules and one save load. No crash or hang; SETTOP/SETBOTTOM per conversation.
- [ ] B8 Clairvoyance/Fury video effect, if reachable (V11: effect band [242,1358]): nothing looks clipped wrongly.

**C. Only if B1 fails (no bands): probe path**
- [ ] C1 Install 1.0.2-probe (`KPM/letterbox_probe_stage/1.0.2-probe/`, hooks change, so use `stage_and_verify_install.py` + KPatchLauncher). Set `probe=1` and `channels=main,set,probe,probe_w4`. Play one conversation, run the analyzer, and give its DIAGNOSIS to the engine-bug session (15e A3). Stage B (1.0.1-probe) was the fallback; its source dir `dialogue_letterbox_fix_probe_src` was retired 2026-10-18 (recover with `git show 35a72bb^:patch_manager_mods/dialogue_letterbox_fix_probe_src/<file>`; the staged kpatch outside the repo is untouched).

**Validated** = every A and B row checked (C is not needed when B1 passes). Then update the doc 17 top line, 19e adoption row ("validated"), register 16 T1 card, and memory.

## Test plan (doc 16 cards T1 / T2, run once WITHOUT and once WITH the staged patches)
Run A = current live (no new patches; do this BEFORE installing), run B = after the install. Resolution 3840x1600 for both.
- **T1 (L1).** Start a voiced conversation (first Atton/Kreia talk, or any on Peragus). Screenshot during an NPC line and while the reply list is shown, with `semi-transparent-letterbox` as installed in both runs. Expected (corrected by V8b 10-03): A = NO top bar but a **200 px bottom band** (vanilla bottom y = min(H+22, H-100fs) = 1400, SetExtent mode 2 sets h = H-y), B = bars 267 px at top and bottom (50 % black with semi-transparent-letterbox); the A/B tell is the top band, reply list inside the bottom bar. Note NPC text position and reply list position/scrollbar.
  `dialogue_letterbox_fix_log.txt` after run B: a `loaded: enabled=1 mode=h6 ...` line; an `APPLY src=SetScreenSize W=3840 H=1600 ... newBar=267 ... verify OK` line at startup (and again after each options Apply or credits); on the first dialogue `SETTOP ... target h 267` and `SETBOTTOM ... target y 1333`. No `FOREIGN`, `INVALID`, `protect`, or `verify` failure lines.
- **T2 (L4).** Character sheet / item description / Journal entry with a long description at 3840x1600; screenshot a wrapped paragraph next to a scrollbar. Look for glyphs touching or overlapping the scrollbar track on the right of wrapped lines: A may touch, B should have about 0.25 px/char more room. If A shows nothing either, L4 stays cosmetic.
- **A/B without reinstalling (L1):** edit `dialogue_letterbox_fix.ini` in the game dir (game closed), restart: `mode=clamp` (bar 0 instead of -22), `mode=vanilla` (keeps vanilla bytes), `enabled=0`. Read the load/APPLY lines for the mode actually used.

## Rollback
- L1 only: `enabled=0` in `dialogue_letterbox_fix.ini`; the `.rdata` constant is vanilla again after the next start (it is never written while disabled).
- Either patch: reinstall the 26 live ids only (same command without `k2-wrap-measure-fix k2-dialogue-letterbox-fix`) from the same stage dir, or restore the dated backup (`kpm_install_state.json`, `patch_config.toml`, `patches/`).
- L4 after the drop: re-add it by re-including the id `k2-wrap-measure-fix` in the install id list, staged from `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/K2WrapMeasureFix/K2WrapMeasureFix.kpatch` (kpatch md5 `11bfc231e0b10dbfa1706a475453fcda`).

## Residuals
- 5120x1440: the 300 px reply panel (fs 3.0) sticks out 60 px above the 240 px bar (also 640x480: 100 vs 80). V7 Q5 UNCLEAR (visual).
- L15 latent last-line drop in Draw (only >= 5120 wide), not addressed.
- KMRP K1-style reply-panel stretch not done (out of scope).
- Note on numbers: the plan's "3840x1600 -> 266" was an arithmetic slip; the engine formula gives 267.

## Planned shared state and shared code
- Logging: KpLog.h since 1.0.2 (19h, adoption table in 19e). Channels: `main` (load, APPLY, LATE), `set` (SETTOP/SETBOTTOM), `probe` (W1/W2/W3/W3b/W5), `probe_w4` (W4). Problem lines are WARN/ERR and need no ini. `log=`/`probe=` keep their 1.0.1 content meaning; `enabled=`/`mode=` are behaviour switches. PROBE W3 is split into W3 + W3b with the same `n=`; the analyzer joins them.
- No shared engine state. The probe addresses are allowlisted in `tests/static/raw_addresses.toml` and are GameAddr.h candidates.

## Files
- `17_dialogue_letterbox_and_wrap_fixes.md` (this), `16_candidate_register.md` (rows L1/L4, test cards, Phase 2 section).
- `dialogue_letterbox_fix_src/` (`K2DialogueLetterboxFix.cpp`, `kotor2-steam-aspyr.hooks.toml`, `manifest.toml`, `exports.def`, `smoke_dialogueletterbox.cpp`, `README.md`), `wrap_measure_fix_src/` (`kotor2-steam-aspyr.hooks.toml`, `manifest.toml`, `README.md`).
- Live source of both: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/{K2DialogueLetterboxFix,K2WrapMeasureFix}`.
- `engine_research_findings/V1.md`, `V3.md`, `V7.md`; `engine_bug_research/tools/check_engfix_windows.py` + `check_engfix_windows_output_2026-10-02.txt`, `engine_bug_research/tools/persist_phase2_db.py` (knowledge DB rows).

- Note (log analysis 2026-10-04): session 10-03 evening, letterbox 1.0.2 log PASS (h6 3840x1600, bar 267, 12 dialogues, verifyOK, 0 errs). No F12 screenshots, so the T1 visual check stays OPEN; L1 not yet accepted. Report `play_session_reports/2026-10-03_evening.md`.
