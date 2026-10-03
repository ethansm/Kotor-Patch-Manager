# Engine Fixes L1 + L4: dialogue letterbox and wrap-measure (doc 15 Phase 2)

**STATUS (2026-10-03 01:15): T1 computation PASS, visual UNKNOWN; T2 not run.** The 10-03 01:12 session log (60 lines, fixture `engine_bug_research/fixtures/letterbox_log_1003_0112.txt`) has 4 launches, each `APPLY ... newBar=267 ... verify OK`, and **12 SETTOP/SETBOTTOM pairs** (dialogue starts), all target `(0,0,3840,267)` / `(0,1333,3840,267)`, start h 0 / start y 1600, **0 error lines**. The user's visual answer: "didn't notice / no bars"; no screenshots were taken, so whether the bars are drawn is unknown (the draw path after SetExtent, V1 B5, was never read). Phase 3 (`engine_bug_research/PHASE3_PLAN.md`) A2 = F12 screenshots + an `enabled=0` A/B run. Live set re-read 10-03: 31 ids / 198 hooks (`kpm_install_state.json` UpdatedAt 2026-10-03 00:24, sibling reinstall), both fixes still installed.

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
`G=~/.local/share/Steam/steamapps/common/Knights\ of\ the\ Old\ Republic\ II`, `STAGE=/home/ethansm/Games/KOTOR2_Mod_Workspace/stage_engfix_1002_0121`. Procedure = `mods678_research/stage_and_verify_install.py` header + doc 10 "Standing rules".
1. Game CLOSED: `pgrep -f '^S:.*swkotor2\.exe'` prints nothing.
2. Re-read `$G/kpm_install_state.json` (`UpdatedAt`) right before; if it changed since the stage (or any `$G/patches/*.dll` is newer than the stage), re-stage first.
3. Back up `kpm_install_state.json`, `patch_config.toml` and `patches/` into a dated dir, e.g. `~/Games/KOTOR2_Mod_Workspace/backups/pre_engfix_1002/`.
4. Install with ALL ids (the 26 live + 2 new):
```
KPATCH_NO_LAUNCH=1 ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches "$STAGE" \
 additional-console-commands buff-duration-hud equipment-3d-character force-power-hotbar good-alignment-heads inventory-3d-viewport \
 loading_screen_line_fix_k2 k2-aspyr-map-aspect-fix k2-aspyr-normal-map-fix k2-aspyr-saber-hilt-texture-fix k2-visible-console \
 k2-finesse-melee-fix k2_grass_fog_fix k2-mp3-playback-fix load-hang-probe max-lights perf-lab post-combat-movement-fix-v1 \
 semi-transparent-letterbox inventory-sort-filter character-effects-panel k2-activation-fix k2-equip-icon-fix companion-ai ranged-ai \
 level-up-override k2-wrap-measure-fix k2-dialogue-letterbox-fix
```
5. Verify: `python3 /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research/stage_and_verify_install.py verify "$STAGE"` (live DLL md5s vs the staged kpatches, hook counts). Afterwards diff the `[Config] Loaded DETOUR` lines against the previous Proton log.

## Test plan (doc 16 cards T1 / T2, run once WITHOUT and once WITH the staged patches)
Run A = current live (no new patches; do this BEFORE installing), run B = after the install. Resolution 3840x1600 for both.
- **T1 (L1).** Start a voiced conversation (first Atton/Kreia talk, or any on Peragus). Screenshot during an NPC line and while the reply list is shown, with `semi-transparent-letterbox` as installed in both runs. Expected: A = no bars (-22 px), B = bars about 267 px at top and bottom, reply list inside the bottom bar. Note NPC text position and reply list position/scrollbar.
  `dialogue_letterbox_fix_log.txt` after run B: a `loaded: enabled=1 mode=h6 ...` line; an `APPLY src=SetScreenSize W=3840 H=1600 ... newBar=267 ... verify OK` line at startup (and again after each options Apply or credits); on the first dialogue `SETTOP ... target h 267` and `SETBOTTOM ... target y 1333`. No `FOREIGN`, `INVALID`, `protect`, or `verify` failure lines.
- **T2 (L4).** Character sheet / item description / Journal entry with a long description at 3840x1600; screenshot a wrapped paragraph next to a scrollbar. Look for glyphs touching or overlapping the scrollbar track on the right of wrapped lines: A may touch, B should have about 0.25 px/char more room. If A shows nothing either, L4 stays cosmetic.
- **A/B without reinstalling (L1):** edit `dialogue_letterbox_fix.ini` in the game dir (game closed), restart: `mode=clamp` (bar 0 instead of -22), `mode=vanilla` (keeps vanilla bytes), `enabled=0`. Read the load/APPLY lines for the mode actually used.

## Rollback
- L1 only: `enabled=0` in `dialogue_letterbox_fix.ini`; the `.rdata` constant is vanilla again after the next start (it is never written while disabled).
- Either patch: reinstall the 26 live ids only (same command without `k2-wrap-measure-fix k2-dialogue-letterbox-fix`) from the same stage dir, or restore the dated backup (`kpm_install_state.json`, `patch_config.toml`, `patches/`).

## Residuals
- 5120x1440: the 300 px reply panel (fs 3.0) sticks out 60 px above the 240 px bar (also 640x480: 100 vs 80). V7 Q5 UNCLEAR (visual).
- L15 latent last-line drop in Draw (only >= 5120 wide), not addressed.
- KMRP K1-style reply-panel stretch not done (out of scope).
- Note on numbers: the plan's "3840x1600 -> 266" was an arithmetic slip; the engine formula gives 267.

## Files
- `17_dialogue_letterbox_and_wrap_fixes.md` (this), `16_candidate_register.md` (rows L1/L4, test cards, Phase 2 section).
- `dialogue_letterbox_fix_src/` (`K2DialogueLetterboxFix.cpp`, `kotor2-steam-aspyr.hooks.toml`, `manifest.toml`, `exports.def`, `smoke_dialogueletterbox.cpp`, `README.md`), `wrap_measure_fix_src/` (`kotor2-steam-aspyr.hooks.toml`, `manifest.toml`, `README.md`).
- Live source of both: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/{K2DialogueLetterboxFix,K2WrapMeasureFix}`.
- `engine_research_findings/V1.md`, `V3.md`, `V7.md`; `engine_bug_research/tools/check_engfix_windows.py` + `check_engfix_windows_output_2026-10-02.txt`, `engine_bug_research/tools/persist_phase2_db.py` (knowledge DB rows).
