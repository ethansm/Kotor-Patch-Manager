# Phase 1 inventory: KPM repo uncommitted state

Run: 2026-10-03, started 11:50:45 EDT (last checks 11:51:53). READ-ONLY; nothing in the KPM repo or the game dir was changed.

- K = `/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`, branch `ethansm/local-mods`, HEAD `fa653d0`
- G = `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II`
- 35 paths in `git status --porcelain -uall` (34 at 11:50:45; `src/KPatchLauncher/Program.cs` appeared at 11:50:52 while this inventory ran, see flags).
- Live DLL install: the whole `$G/patches/*.dll` set was written in one batch at 2026-10-03 11:29:44 (build 28 install). "DLL newer" therefore means source mtime < 11:29:44; "source newer" means the live DLL cannot contain that edit.

## `.git/info/exclude` (verbatim, current contents)

```
# git ls-files --others --exclude-from=.git/info/exclude
# Lines that start with '#' are comments.
# For a project mostly in C, the following would be a good set of
# exclude patterns (uncomment them if you want to use them):
# *.[oa]
# *~
# local install staging / evidence (not for upstream)
*_live_patches/
probe13_stage/
phase0_evidence/
AddressDatabases/*.empty_moved_*
AddressDatabases/kotor2_steam_aspyr_laa.db
*.kpatch.old
```

Repo `.gitignore` already ignores `*.kpatch`, `*.dll`, `[Bb]in/`, `[Oo]bj/`, `bin/Debug/KPatchLauncher`. So `*.kpatch` and `registry_core_stage/KotorPatcher.dll` do not appear in status (confirmed with `git check-ignore -v`).

## Active sessions (mid-edit evidence)

Files modified within 30 min of 11:50 (see table): `CharEffectsPanel.cpp` 11:46:46, `ForcePowerHotbar.cpp` / `HotbarConfig.h` / `HotbarLogic.h` 11:49:43, `Program.cs` 11:50:52, `registry_core_stage/*` 11:44:39. `pgrep -af claude` (truncated at 200 chars) at 11:50:55 showed other sessions active:

```
18163   claude --dangerously-skip-permissions --resume b717d74e-2ae1-4b06-ad32-6a330f05b655
103019  claude --dangerously-skip-permissions
327813  claude --resume e2f5b59d-1cf1-4f85-a04f-54df84e672a6 --dangerously-skip-permissions
1377453 claude --dangerously-skip-permissions
1903422 claude --dangerously-skip-permissions
1989161 claude --resume b68a7716-4d12-4842-a152-13240bc0c2ab --dangerously-skip-permissions
1991569 claude --dangerously-skip-permissions
2079253 claude --dangerously-skip-permissions
2505689 claude --dangerously-skip-permissions
3104925 claude --dangerously-skip-permissions
3505331 claude --resume 763b8197-92bd-48c4-85ea-4388c13d86da --dangerously-skip-permissions
3549110 claude --resume 22e14246-9dc5-4e85-914b-ec72897ff11c --dangerously-skip-permissions
3552659 claude --resume 6c959218-6f21-4071-91f6-770c86150673 --dangerously-skip-permissions
3560272 claude --dangerously-skip-permissions
3564703 claude --dangerously-skip-permissions
3626316 claude --dangerously-skip-permissions
4001125 claude --dangerously-skip-permissions
(+ 3 bash -c shell-snapshot wrappers: 3209382, 3209387, 3210512)
```

About 20 claude processes; at least one is editing the KPM tree right now (Hotbar, CharEffectsPanel, Program.cs).

## Patch id map and duplicate-id check

All `Patches/*/manifest.toml` `[patch] id` values scanned (77 patches). Exactly one duplicate:

- `k2-dialogue-letterbox-fix` is declared by BOTH `Patches/K2DialogueLetterboxFix/` (tracked, 1.0.0, live) and `Patches/K2DialogueLetterboxFixProbe/` (untracked, version `1.0.1-probe`). Committing the probe dir as-is would put two patches with one id in the tree (this is the "kpm-validate-patches red on probe dup id" noted in memory).
- `mouse-probe` is unique.

## Table

Owning dir -> id; Live = id present in `$G/patch_config.toml`.

| path | mod (Patches/Dir) | id | live | mtime vs DLL | summary | flags |
|---|---|---|---|---|---|---|
| `Patches/BuffDurationHUD/BuffDurationHUD.cpp` | BuffDurationHUD | buff-duration-hud | yes | src 10-03 01:25:46, DLL 11:29:44: DLL newer | 53 lines. Fixes crash (10-02): vtable slots 38/39 HP calls take ONE stack arg, were called with two; empty teal track for droids (mx<1); menu companion bars moved via engine SetRect (vtable slot 1) instead of raw int writes (MENUMOVE log); bar geometry flush to portrait, force bar 20% wider; leader FORCEMOVE gap fix | Clean. Identical to `buff_duration_hud_src/BuffDurationHUD.cpp`. Deployed (memory: live dll includes it) |
| `Patches/CharEffectsPanel/CharEffectsPanel.cpp` | CharEffectsPanel | character-effects-panel | yes | src 11:46:46, DLL 11:29:44: **source newer** | 81 lines. Build 4b/4c: content-sized columns (time/stat/name, `stat_min_frac`, `text_k10` 1.03 INI keys), stat_max_w default 0 (uncapped), damage-type text up to 3 names, movement-speed decode fix (0x1c/0x1d, +100 bias, engine cap +50% note) from Mod 14 R3c | **MID-EDIT** (modified 4 min before run). `-fsyntax-only` with mingw: no errors. Live DLL may lack the 11:46 edits. Identical to `active_effects_panel_src/CharEffectsPanel.cpp` (ws copy 11:48:20, newer by mtime only) |
| `Patches/ForcePowerHotbar/ForcePowerHotbar.cpp` | ForcePowerHotbar | force-power-hotbar | yes | src 11:49:43, DLL 11:29:44: **source newer** | 527 lines changed. @lowhp target-item probe (CanUseItem 0x5D2F80 + PerformUseItem 0x775600, lowest-HP member), per-save hotbar snapshot/restore (HotbarOnSaved/Autosaved/LoadRequest/AutosaveSlotLoad/NewGame), qty "0" gray, item stack counts, tooltip controls (75 controls) | **MID-EDIT** (11:49:43, ~1 min before run). `-fsyntax-only`: no errors at snapshot time. Live DLL = build 28; probe build 29p (afe1553c) is NOT installed (memory), so source contains more than the live DLL. Identical to `force_power_hotbar_src/ForcePowerHotbar.cpp` |
| `Patches/ForcePowerHotbar/HotbarConfig.h` | ForcePowerHotbar | force-power-hotbar | yes | src 11:49:43: **source newer** | 122 lines. TargetSpec (`@lowhp[:pct[:self\|none]]` grammar, round-trip), HealThreshold/HealFallback, item stack-count parse, "move on duplicate pick" | **MID-EDIT** (11:49:43). Identical to ws `force_power_hotbar_src/HotbarConfig.h` (ws 11:43:20) |
| `Patches/ForcePowerHotbar/exports.def` | ForcePowerHotbar | force-power-hotbar | yes | src 10-03 01:56:48, DLL 11:29:44: DLL newer | +5 exports (HotbarOnSaved, HotbarOnAutosaved, HotbarOnLoadRequest, HotbarOnAutosaveSlotLoad, HotbarOnNewGame) | Clean. Identical to ws `force_power_hotbar_src/exports.def` |
| `Patches/ForcePowerHotbar/kotor2-steam-aspyr.hooks.toml` | ForcePowerHotbar | force-power-hotbar | yes | src 01:56:48, DLL newer | +57 lines: per-save hooks S1 0x00534527, S2 0x0053C5AA, L1.. (original_bytes recorded) | Clean. Identical to ws copy |
| `Patches/ForcePowerHotbar/HotbarLogic.h` (untracked) | ForcePowerHotbar | force-power-hotbar | yes | src 11:49:43: **source newer** | New pure C++17 header: item target selection `chooseTarget`/`slotEnabled`/`isTargetableItemType` (tested by hotbar_logic_test, 85 checks) | **MID-EDIT** (11:49:43). Required by ForcePowerHotbar.cpp, must be committed with it. Identical to ws copy (11:46:36) |
| `Patches/LevelUpOverride/LevelUpOverride.cpp` | LevelUpOverride | level-up-override | yes | src 01:51:18, DLL 11:29:44: DLL newer | 477 lines. 0.3.1: M3 save binding via playthrough GUID (LUO_G00..G31 globals), M4 character-screen UI (Configure button, options page), instant leader level-up, pause-at-level | Clean (no recent mtime). Identical to `level_up_override_src/LevelUpOverride.cpp` (ws 01:46:57). Memory: live dll aaedb93e 01:52 |
| `Patches/LevelUpOverride/exports.def` | LevelUpOverride | level-up-override | yes | 01:51:18: DLL newer | +5 exports (LuoGuidSave/Load/NewServer, LuoUiPanelCtor, LuoLeaderInstant_74F423) | Clean. Identical to ws |
| `Patches/LevelUpOverride/kotor2-steam-aspyr.hooks.toml` | LevelUpOverride | level-up-override | yes | 01:51:18: DLL newer | +88 lines: SetStats hold gate/UI tick, globals saver 0x656640, LoadFromSave 0x65675B hooks | Clean. Identical to ws |
| `Patches/LevelUpOverride/manifest.toml` | LevelUpOverride | level-up-override | yes | 01:51:18: DLL newer | version 0.3.0 -> 0.3.1, description updated | Clean. Identical to ws |
| `Patches/LevelUpOverride/LevelUpUi.h` (untracked) | LevelUpOverride | level-up-override | yes | 01:51:18: DLL newer | 463 lines, M4 UI (not standalone: included inside the .cpp's anonymous namespace) | Needed by .cpp. Identical to ws |
| `Patches/LevelUpOverride/OptionsLogic.h` (untracked) | LevelUpOverride | level-up-override | yes | 01:51:18: DLL newer | 124 lines, pure model of the single options page | Needed. Identical to ws |
| `Patches/LevelUpOverride/Playthrough.h` (untracked) | LevelUpOverride | level-up-override | yes | 01:51:18: DLL newer | 248 lines, per-playthrough config + GUID bit packing | Needed. Identical to ws |
| `Patches/LevelUpOverride/WizardLogic.h` (untracked) | LevelUpOverride | level-up-override | yes | 01:51:18: DLL newer | 160 lines, wizard state machine | Needed. Identical to ws |
| `Patches/RobustnessFixes/RobustnessFixes.cpp` | RobustnessFixes | k2-robustness-fixes | yes | src 10-02 12:49:04, DLL 10-03 11:29:44: DLL newer | 125 lines. 1.1.0: counter caves (vmid, sndri, netovf, guiid), rel8 fixup checks, RobFixTestCounters export, 5 s counters/FIRED log lines | Clean. Identical to `robustness_fixes_src/RobustnessFixes.cpp`. Live DLL strings show `robustness-fixes 1.1.0` |
| `Patches/RobustnessFixes/kotor2-steam-aspyr.hooks.toml` | RobustnessFixes | k2-robustness-fixes | yes | 10-02 12:49: DLL newer | Comment-only: documents new cave byte sequences (vmid, sndri, netovf, guiid) | Clean. Identical to ws |
| `Patches/RobustnessFixes/manifest.toml` | RobustnessFixes | k2-robustness-fixes | yes | 10-02 12:49: DLL newer | version 1.0.0 -> 1.1.0 | Clean. Identical to ws |
| `docs/RuntimeGuiControls.md` | n/a (docs, covers hotbar/GUI controls) | n/a | n/a | 10-03 00:03:27 | 10 lines: memory figure (75 labels ~25 KB, HUD rebuilt on every load/area transition), resolution-change note | Clean. Doc only; belongs with ForcePowerHotbar |
| `src/KPatchLauncher/Program.cs` | n/a (launcher, core) | n/a | n/a | 11:50:52 | +6 lines: `KPATCH_NO_LAUNCH=1` env var applies patches then returns without launching the game | **MID-EDIT** (appeared mid-run, 11:50:52; not in the first status snapshot). Likely a session's safe "install without launching" helper. Not mod code; hold until the owning session finishes |
| `Patches/K2DialogueLetterboxFixProbe/K2DialogueLetterboxFix.cpp` (untracked) | K2DialogueLetterboxFixProbe | k2-dialogue-letterbox-fix (**DUPLICATE** of Patches/K2DialogueLetterboxFix) | yes (id live, but the live DLL is 1.0.0: no PROBE strings, so the probe build is not installed) | 01:58:10; not in live DLL | 456 lines: 1.0.0 + log-only probe windows W1-W5 (`probe=1` INI, default off; diff vs 1.0.0 is +~118 lines of probe code) | PROBE-ONLY. Duplicate id. Identical to ws `dialogue_letterbox_fix_probe_src/` |
| `Patches/K2DialogueLetterboxFixProbe/exports.def` (untracked) | same | same | same | 01:57:34 | exports | PROBE, duplicate id |
| `Patches/K2DialogueLetterboxFixProbe/kotor2-steam-aspyr.hooks.toml` (untracked) | same | same | same | 01:57:34 | 119 lines, hooks incl. SetTop/SetBottom log hooks | PROBE, duplicate id |
| `Patches/K2DialogueLetterboxFixProbe/manifest.toml` (untracked) | same | same | same | 01:57:34 | version `1.0.1-probe`, same id as the shipped fix | PROBE, duplicate id (would conflict) |
| `Patches/MouseProbe/MouseProbe.cpp` (untracked) | MouseProbe | mouse-probe | yes | 10-02 12:01:55, DLL 10-03 11:29:44: DLL newer | 138 lines. Strictly read-only DirectInput/camera probe, logs to mouseprobe_log.txt, gated by `mouseprobe.txt` flag file | PROBE-ONLY, live. Identical to `mouse_probe_src/MouseProbe.cpp` |
| `Patches/MouseProbe/exports.def` (untracked) | MouseProbe | mouse-probe | yes | 10-02 12:01:55: DLL newer | exports | PROBE |
| `Patches/MouseProbe/kotor2-steam-aspyr.hooks.toml` (untracked) | MouseProbe | mouse-probe | yes | 10-02 12:01:55: DLL newer | 40 lines hooks | PROBE |
| `Patches/MouseProbe/manifest.toml` (untracked) | MouseProbe | mouse-probe | yes | 10-02 12:15:47: DLL newer | version 0.1.0, author "local" | PROBE (author "local" is not a contributor identity) |
| `launch_kpm_ui.sh` (untracked) | n/a | n/a | n/a | 10-02 22:32:48 | 4-line wrapper: `cd "$SCRIPT_DIR/bin/Debug"; exec ./KPatchLauncher "$@"` (runs the built launcher) | SCRATCH: local convenience launcher into the build output dir |
| `registry_core_stage/MD5` (untracked) | n/a (build stage dir) | n/a | n/a | 11:44:39 | md5 `36f10b7f...` of the staged `KotorPatcher.dll` | SCRATCH: build/stage output. Dir also holds a 1.4 MB `KotorPatcher.dll` (gitignored by `*.dll`). Staged DLL md5 `36f10b7f` differs from the live `$G/KotorPatcher.dll` (md5 `2a706ac9`, dated 09-28 15:43), so the stage is a not-yet-installed core build |
| `registry_core_stage/SOURCE_COMMIT` (untracked) | n/a | n/a | n/a | 11:44:39 | `fa67a3b882c6...` = the commit the stage was built from (a commit on `ethansm/local-mods`; kpm-registry worktree is a separate branch) | SCRATCH (stage metadata); **MID-EDIT** (11:44, 6 min old, a build session is producing it) |
| `tools/ghidra_scripts/ExportCommandStores.java` (untracked) | n/a (tools) | n/a | n/a | 10:57:52 | Read-only Ghidra headless export of function-pointer table stores (NWScript command tables), B15 Steam address-DB pipeline | Source, keep. Not recent |
| `tools/ghidra_scripts/ExportDataTypes.java` (untracked) | n/a (tools) | n/a | n/a | 01:02:08 | Read-only struct/union/enum/typedef export (types.tsv), steam_db_batches DESIGN.md section 2 | Source, keep |
| `tools/ghidra_scripts/ExportFingerprints.java` (untracked) | n/a (tools) | n/a | n/a | 01:02:08 | Read-only per-function fingerprint + string export (fp.tsv, strings.tsv) | Source, keep |
| `tools/ghidra_scripts/ExportProgramSymbols.java` (untracked) | n/a (tools) | n/a | n/a | 10-02 23:59:40 | Exports functions and symbols to TSV for comparative auditing | Source, keep |

## Cross-check against KOTOR-II workspace `patch_manager_mods/*_src/` copies

`cmp` of every modified/untracked file under `Patches/` against the same-named file in its mod's `*_src` dir: all 27 are BYTE-IDENTICAL.

| KPM file | workspace copy | result | newer by mtime |
|---|---|---|---|
| BuffDurationHUD.cpp | buff_duration_hud_src | identical | same (01:25:46) |
| CharEffectsPanel.cpp | active_effects_panel_src | identical | ws 11:48:20 vs kpm 11:46:46 |
| ForcePowerHotbar.cpp, HotbarConfig.h, HotbarLogic.h | force_power_hotbar_src | identical | kpm 11:49:43 vs ws 11:49:09 / 11:43:20 / 11:46:36 |
| LevelUpOverride.cpp + 4 headers + exports/hooks/manifest | level_up_override_src | identical | kpm 01:51:18 vs ws 00:10-01:47 |
| RobustnessFixes.cpp + hooks + manifest | robustness_fixes_src | identical | kpm 10-02 12:49 vs ws 12:47 |
| K2DialogueLetterboxFixProbe/* | dialogue_letterbox_fix_probe_src | identical | ws 02:00:10 vs kpm 01:57-01:58 |
| MouseProbe/* | mouse_probe_src | identical | same / 1 s apart |

Source of truth: the two trees are in sync right now (no divergence), with the KPM tree the build tree and `*_src` the workspace-repo mirror. For the actively edited mods (Hotbar, CharEffectsPanel) either side may get the next edit first, so re-`cmp` right before committing. Note: `*_src` dirs also hold files with no KPM counterpart (tests such as `hotbar_logic_test.cpp`, `config_roundtrip_test.cpp`, `HotbarTextures.inc`, `Tlk.h`, `TwoDA.h`, `PULL_REQUEST.md`); those were not part of this status.

## Flags summary

MID-EDIT (do not commit now): CharEffectsPanel.cpp (11:46), all three ForcePowerHotbar files (11:49: ForcePowerHotbar.cpp, HotbarConfig.h, HotbarLogic.h), `src/KPatchLauncher/Program.cs` (11:50:52), `registry_core_stage/MD5` + `SOURCE_COMMIT` (11:44). No `TODO WIP` / `XXX` / `FIXME` / `WIP` markers in any added diff line. No syntax errors from `i686-w64-mingw32-g++ -fsyntax-only` on ForcePowerHotbar.cpp and CharEffectsPanel.cpp.

Source newer than live DLL (live DLL lacks these edits): CharEffectsPanel.cpp, ForcePowerHotbar.cpp, HotbarConfig.h, HotbarLogic.h. Every other modified file is older than the 11:29:44 install.

SCRATCH: `launch_kpm_ui.sh`, `registry_core_stage/` (MD5, SOURCE_COMMIT, plus the gitignored 1.4 MB KotorPatcher.dll). Probe-only: `K2DialogueLetterboxFixProbe/` (duplicate id with a shipped patch, not installed) and `MouseProbe/` (live, read-only diagnostics, author "local").

## Proposed commit groups

All messages end with `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`. Run from K. One commit per mod. Nothing here has been staged or committed.

1. WIP BuffDurationHUD (ready now)
   `git add Patches/BuffDurationHUD/BuffDurationHUD.cpp`
   `WIP BuffDurationHUD: droid empty track, menu bars via engine SetRect, HP vtable 1-arg crash fix, bar geometry`

2. WIP LevelUpOverride (ready now; all files older than the 11:29 install)
   `git add Patches/LevelUpOverride/LevelUpOverride.cpp Patches/LevelUpOverride/exports.def Patches/LevelUpOverride/kotor2-steam-aspyr.hooks.toml Patches/LevelUpOverride/manifest.toml Patches/LevelUpOverride/LevelUpUi.h Patches/LevelUpOverride/OptionsLogic.h Patches/LevelUpOverride/Playthrough.h Patches/LevelUpOverride/WizardLogic.h`
   `WIP LevelUpOverride: 0.3.1 playthrough-GUID save binding, character-screen options UI, instant leader level-up`

3. WIP RobustnessFixes (ready now)
   `git add Patches/RobustnessFixes/RobustnessFixes.cpp Patches/RobustnessFixes/kotor2-steam-aspyr.hooks.toml Patches/RobustnessFixes/manifest.toml`
   `WIP RobustnessFixes: 1.1.0 counter caves for vmid/sndri/netovf/guiid with periodic COUNTERS log`

4. WIP ForcePowerHotbar (HOLD: files modified 11:49, include after the owning session settles; then re-`cmp` against force_power_hotbar_src)
   `git add Patches/ForcePowerHotbar/ForcePowerHotbar.cpp Patches/ForcePowerHotbar/HotbarConfig.h Patches/ForcePowerHotbar/HotbarLogic.h Patches/ForcePowerHotbar/exports.def Patches/ForcePowerHotbar/kotor2-steam-aspyr.hooks.toml docs/RuntimeGuiControls.md`
   `WIP ForcePowerHotbar: @lowhp item targeting, per-save hotbars, stack counts, HotbarLogic.h`
   (HotbarLogic.h must go with the .cpp or the build breaks. docs/RuntimeGuiControls.md is hotbar-related and could be a separate "WIP docs" commit if preferred.)

5. WIP CharEffectsPanel (HOLD: modified 11:46; source newer than live DLL)
   `git add Patches/CharEffectsPanel/CharEffectsPanel.cpp`
   `WIP CharEffectsPanel: content-sized columns, text_k10/stat_min_frac INI keys, speed decode fix`

6. WIP tools/ghidra_scripts (ready now; none recent)
   `git add tools/ghidra_scripts/ExportCommandStores.java tools/ghidra_scripts/ExportDataTypes.java tools/ghidra_scripts/ExportFingerprints.java tools/ghidra_scripts/ExportProgramSymbols.java`
   `WIP tools: Ghidra headless exports for the Steam address-DB pipeline (command stores, data types, fingerprints, program symbols)`

7. WIP MouseProbe (optional; ready if the user wants probes tracked locally on `ethansm/local-mods`; unique id, live)
   `git add Patches/MouseProbe/MouseProbe.cpp Patches/MouseProbe/exports.def Patches/MouseProbe/kotor2-steam-aspyr.hooks.toml Patches/MouseProbe/manifest.toml`
   `WIP MouseProbe: read-only DirectInput/camera probe 0.1.0`
   (Probe-only; consider keeping it local/excluded instead. Its `author = "local"` and it must never go into an upstream PR.)

Docs: no separate docs group needed except the optional split of `docs/RuntimeGuiControls.md` out of commit 4.

## SKIP list (mid-edit or owned by another live session; do not stage now)

- `src/KPatchLauncher/Program.cs` (11:50:52, appeared during this run; `KPATCH_NO_LAUNCH` helper). Own commit later: `WIP KPatchLauncher: KPATCH_NO_LAUNCH=1 applies patches without launching`.
- `Patches/CharEffectsPanel/CharEffectsPanel.cpp` (11:46) until commit 5 is released.
- `Patches/ForcePowerHotbar/*` and `docs/RuntimeGuiControls.md` (11:49) until commit 4 is released.
- `registry_core_stage/MD5`, `registry_core_stage/SOURCE_COMMIT` (11:44; also in EXCLUDE).
- `Patches/K2DialogueLetterboxFixProbe/*` (4 files): duplicate id `k2-dialogue-letterbox-fix` with the shipped fix. Do not commit as-is (breaks `kpm-validate-patches`). Either exclude, or change the probe manifest id (for example `k2-dialogue-letterbox-fix-probe`) in a deliberate step. Not installed live (live DLL is 1.0.0).

## EXCLUDE list (scratch; recommend adding to `.git/info/exclude`)

Append these lines to `$K/.git/info/exclude` (not done, read-only task):

```
launch_kpm_ui.sh
registry_core_stage/
Patches/K2DialogueLetterboxFixProbe/
```

- `launch_kpm_ui.sh`: local wrapper that `exec`s `bin/Debug/KPatchLauncher`.
- `registry_core_stage/`: stage output of a `KotorPatcher.dll` build (binary + MD5 + SOURCE_COMMIT); the DLL is already ignored by `*.dll`, the two text files are not.
- `Patches/K2DialogueLetterboxFixProbe/`: probe variant, duplicate id. (`MouseProbe/` is the user's call: either commit 7 or add `Patches/MouseProbe/` to exclude; it is live and its id is unique.)

Already handled by `.gitignore` and not in status: `Patches/*/*.kpatch` (including `MouseProbe.kpatch`, `K2DialogueLetterboxFixProbe.kpatch`), `registry_core_stage/KotorPatcher.dll`, `bin/`, `obj/`.
