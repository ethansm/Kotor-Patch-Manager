# Level-up override - Phase 5: UX + save binding -> one combined build (0.3.0) -> ONE combined live test

## Context
- `level-up-override` 0.3.0-a (hold gates x13, instant gates x4, GUI guards, Auto OK backstop) is INSTALLED (dll 9e8cbbff, 28 ids, 2026-10-02 10:32) but
  the user has NOT tested it. User decision 2026-10-02: build everything first, then test everything together in one session.
- Still missing for the player-facing mod (doc 08 Status R1-R11, "User decisions" a-g):
  - M3 save binding: config per playthrough, keyed by a GUID stored in DLL-added save globals.
  - M4 character-screen Configure button (R6), configure nudge (R9), question wizard over the presets (R8).
  - R11 (new 2026-10-02): visible "held" cue. **User choice: a character-screen status line + a one-time message when a level is banked.** No portrait
    badge, and no tinted vanilla icon (that would suppress Buff HUD's icons).
- Outcome: one 0.3.0 build. The player configures a companion in-game, the choice follows the save, and held companions are explained on screen. Then one
  user-consented install and one combined live checklist (M2 + M3 + M4 + R11 items).

## Read first after a context clear
- `research/companion_levelup/MASTER_PLAN.md` status line.
- `patch_manager_mods/08_level_up_override.md`:
  - Status (R1-R11).
  - "Phase 3 research" Items 2, 5, 7.
  - "Phase 4 M0 facts", "Phase 4 M1 results", "Phase 4 M2 build", "M2 install".
  - "Session notes - Phase 4" (live state + gotchas).
- `research/companion_levelup/PHASE4_PLAN.md` (data model, hook pattern, rules).
- Source: `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp` (~1210 lines).
  - Gates at ~1014-1205.
  - `tagConfig()` carries the M3 TODO at ~255.
  - `levelNow` at ~502.
- Tests: `research/companion_levelup/tools/parity/` (`parity.py`, `gate_test.cpp`, `apply_test.cpp`, `parity_main.cpp`, `stub/`); `simulate_rewrite.py`.
- UI references (UX research 2026-10-02, two Explore reports summarised under "UX facts" below):
  - `patch_manager_mods/force_power_hotbar_src/ForcePowerHotbar.cpp` (Mod 3): `createLabel` 982-1043, `cursorInGame` 476, `wHit` 729, click poll
    829-832, `playHoverSound` 608, `setFill`/`setAlpha` ~446-465, tooltip ~560-605.
  - `patch_manager_mods/active_effects_panel_src/CharEffectsPanel.cpp` (Mod 14): `bindLabel` 273, `ux`/`uy` placement 368-375, `setCtlRect`/`setText`/
    `setTextColor`, readback gates 1140-1150, `leaderClient()` 416.

## UX facts (from the 2026-10-02 research; persist into doc 08 in Step 0)
- **Runtime controls need the layout open.**
  - InitControl 0x40F620 from a template tag needs the panel layout GFF (panel+0x30). That GFF is only open inside the panel ctor, before
    StopLoadFromLayout 0x40F5A0 (DB lessons 67/113/283).
  - So the per-tick SetStats hook CANNOT create template labels.
  - Mod 14 owns the ctor bind hook 0x84D871 and ships a custom `character_p.gui` with `LBL_EFX_*` ids 69..150 (generator `scripts/mirror_character_page.py`).
  - **Need a ctor-time hook site of our own (U1).**
- **Label recipe** (Mod 3 `createLabel`):
  - new(0x148) + ctor 0x419740 (vtable 0x9878BC);
  - InitControl(panel, ctl, templateTag, addToList=0), then id = control count;
  - rect via vtable slot 1, text 0x416E30 (ECX = label+0xF0), font 0x416DE0, align 0x416FA0, fill border(+0x60)+0x74, tint border+0x28, alpha
    border+0x24, text colour 0x417140;
  - append via 0x83EA60 on panel+0x24.
  - Flags +0x48: bit1 = visible; bit5 0x20 = click-through. Keep 0x20 set while hidden, clear it while shown so clicks don't reach the world (lesson 192).
  - Draw order = list index (append = on top). Hit-test walks from the highest index.
  - DLL-set text draws left-aligned (lesson 265). Runtime extents are engine pixels.
- **Templates in live `character_p.gui`:** `LBL_EXPERIENCE_STAT` (id 25, plain, green text) and `LBL_XP_BACK` (id 3, framed).
- **Free area** under BTN_LEVELUP is about x 362..456, y 370..430 (800x600 units). Mod 14's panel occupies x 536-752.
- **Placement:** use Mod 14's method = reference label pixel rect + (unit - refX) * scale. This handles aspect and ultrawide with no projection fix.
- **Clicks:** `GetAsyncKeyState(VK_LBUTTON)` edge detection, gated on a foreground-PID check, hit-tested against the control extent (+4/+8/+0xC/+0x10).
  Esc closes the whole character screen (vanilla). The wizard resets on panel close, so no Esc-swallow hook is needed.
- **Wizard data** (`runtime/levelup_paths.ini` `[presets]`):
  - 5-8 presets per companion;
  - at most 5 answers on any page (fits 6 cells);
  - auto-skip pages with one answer, and jump to the result as soon as one preset remains.
- **Build:** `create-patch.py` compiles every `*.cpp` in the patch dir + its subdirs. Shared headers go to `patch_manager_mods/_shared_src/` and are
  copied to `$T/Patches/_shared/` (included as `../_shared/X.h`).

## Rules (every step; same as Phase 4)
- **Git:** never push or amend. Use `git add -- <new files>` then `git commit -- <explicit paths>` with trailer
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
  - Ignore `logs/debug_pykotor.log*`, `__pycache__`, and the foreign `level_up_override_src/PULL_REQUEST.md`.
- **Phase sequencing:** never overlap a background job with a coding step (verify, commit, launch). Non-Ghidra agents may run in parallel on disjoint files.
- **Ghidra (one headless run at a time across ALL sessions):**
  - Use `research/companion_levelup/tools/ghidra_run.sh`.
  - Check `pgrep -fa '^java.*analyzeHeadless'` first. If BUSY, wait 60 s, max 10 tries.
  - Query the knowledge DB + `phase3_evidence/` + `phase4_evidence/` first; never re-decompile.
  - Ghidra agents write only to `research/companion_levelup/phase5_evidence/`.
- **Engine safety:** never veto inside CanLevelUp/ValidateLevelUp/LevelUp. The LevelUp hook never vetoes. Never inject a class through the pending struct.
- **Hooks:**
  - No address already used by another patch (Mod 14 owns 0x84D871, 0x84FBB0, 0x84DEF0).
  - +-16-byte overlap scan vs all `kotor2-steam-aspyr*.hooks.toml` (repo + `$T/Patches/*`) + the live `patch_config.toml`.
  - Live bytes == vanilla (`tools/pebytes.py`).
- **Install:**
  - Only with the user's consent. Back up `$G/{kpm_install_state.json,patch_config.toml,patches/}` plus `levelup_*.ini` to
    `~/Games/KOTOR2_Mod_Workspace/backups/<name>_<ts>/`.
  - Re-read the live ids and re-md5 every time.
  - Stage `$T/levelup_stage5/` via `patch_manager_mods/mods678_research/stage_and_verify_install.py stage`, then `KPATCH_NO_LAUNCH=1 KPatchLauncher
    ... --patches`, then `verify`, expecting 0 mismatches.
  - Never launch the game.
  - Build: copy the src files to `$T/Patches/LevelUpOverride/` then `python3 ../create-patch.py`.
  - mingw check: `i686-w64-mingw32-g++-posix -std=c++17 -Wall -Wextra -fsyntax-only -I $T/Patches/LevelUpOverride <cpp>`.
- **Persist after every milestone:** doc 08 section, DB rows (python sqlite3, real date), MASTER_PLAN status line, memory `kotor2-levelup-override`, commit.

## Delegation (token conservation)
- **Sonnet 5.5 agents** (`general-purpose`, `model: sonnet`) do Ghidra batches, Python twins/tests, data fixes, first-draft C++ for spec'd functions,
  and independent reviews.
  - Each prompt names the exact files, functions, acceptance commands, and asks for a <= 400-word report with `file:line`.
  - Agents never install, launch, commit, or edit doc 08.
- **Opus** writes specs and prompts, spot-checks evidence, reviews every diff, runs gates, asks before installing, installs, persists, commits.
- At most ONE Ghidra agent at a time and ONE C++ coding agent at a time.

## Steps
### Step 0 - preflight (Opus)
1. Copy this plan to `research/companion_levelup/PHASE5_PLAN.md` (the ~/.claude slug is reused per phase).
2. Create `research/companion_levelup/phase5_evidence/README.md`.
3. Write "UX facts" + "User decision R11 (status line + banked message)" into doc 08 (new section "Phase 5 UX research").
4. Update the MASTER_PLAN status, doc 08 "Session notes - Phase 5", and memory. Commit.

### U - static research (one Sonnet Ghidra agent, sequential runs; then Opus spot-check + persist + commit)
- **U1 ctor-time creation site** for CSWGuiInGameCharacter ctor 0x84C3A0:
  - disasm the tail from the last InitControl to the StopLoadFromLayout 0x40F5A0 call;
  - Mod 14's 0x84D871 exact instruction and what it captures;
  - list `E8`/plain instruction sites with the layout still open, >= 16 bytes from 0x84D871 and from any other hook;
  - for each: register/frame holding the panel `this`, bytes live == vanilla.
  - Fallbacks:
    - (a) StartLoadFromLayout 0x40EE40 re-open feasibility from SetStats (args, what it reads, cost);
    - (b) what InitControl sets on a label so a template-less creation (ctor + manual font/text/border/rect) is possible.
  - Recommend one approach.
- **U2 banked message (R11):**
  - the FeedbackMessage path used by SetExperience (the `FeedbackMessage(0xb)` "level up available" call), signature, server vs client side;
  - a function that shows a CUSTOM string to the player: feedback/combat-log entry, `SendFeedbackString`-like, client message, or floating text over the
    creature;
  - must be callable from the hold gate context (server-side CanLevelUp callers 0x5FD7CE/0x5FE2A3/0x66C95C or the portrait tick 0x74F3C7);
  - give VA, calling convention, string type (CExoString builder 0x733570?), any rate limits.
  - Recommend one API + call context.
- **U3 save-binding leftovers** (static parts of M3):
  - at 0x656640 entry, which reg holds the table (ECX), and the CExoString construction helper for names;
  - at 0x656758 confirm `[ebp-0x1C]` = table and the hook-param capture (`[ebp-0x1C]` as a param source);
  - 0x401C52 call site (call 0x51C450 in 0x401BC0): bytes, regs, `exclude_from_restore` needs;
  - is GLOBALVARS loaded on every load path incl. autosave restore 0x8529E0 (yes per Item 7 - confirm the post-load hook covers it).
- **U4 class names + XP:**
  - where to get a class display name (classes.2da name strref -> TLK; or hard-code 0-9: Soldier, Scout, Scoundrel, Jedi Guardian, Jedi Consular, Jedi
    Sentinel, Combat Droid, Expert Droid, Minion, Tech Specialist?; verify ids with pykotor from the live classes.2da);
  - XP table read path rules(0xA1B4D0)+0x38+4*i (already known) to compute "levels banked" = count of thresholds <= XP beyond the current level.
- **Persist:** doc 08 "Phase 5 U facts", DB rows, commit.

### P - data fix (Sonnet Python agent, parallel with U: disjoint files)
- Re-plan the 5 presets that now leave a point unspent per level after the Class Skill feat change:
  - `g0t0__ranged_pistols_none_na_stealth`, `g0t0__support_pistols_none_na`, `t3m4__support_single_none_na`;
  - the 2 Kreia presets with Stealth (find via `check_paths.py` warnings).
- Spend the freed point; keep wizard answers and `short`.
- Gate: `check_paths.py` 0 errors and fewer warnings, `gen_runtime_table.py` regenerated, `simulate_rewrite.py` + `parity.py` green. Commit.

### M3 - save binding (C++ draft by Sonnet, Opus review)
- **GUID** = 32 Boolean globals `LUO_G00..LUO_G31` (bit i). 0 = none.
- **Engine API** (doc 08 M0 facts): Find 0x6545B0 -> AddBoolean 0x654430 when missing -> SetBoolean 0x654BB0 / GetBoolean 0x654740. Table =
  `*(*(*(0xA1B4A4)+8)+4) + 0x100FC`. CExoString names built per U3.
- **Hooks:**
  - pre-save 0x656640 entry (add + set from the cached GUID when nonzero);
  - post-load 0x656758 (read into the cache, log `GUID load g<hex>`);
  - 0x401C52 (server recreated -> cache = 0, log).
  - All log-rich (`GUID` lines) per the "log thoroughly early" rule.
- **GUID creation:** generated on the first wizard Apply when the cache is 0 (`QueryPerformanceCounter ^ GetTickCount ^ rand`, nonzero), and written to the
  live table immediately (so the next save carries it).
- **Config `levelup_playthroughs.ini`** (game dir):
  - section `[g<8 hex>]`, line `<tag>=<preset id>|instant=0|1|hold=0|1`;
  - read with mtime reload;
  - write = full rewrite to `.tmp` + `MoveFileExA(REPLACE_EXISTING)`.
- **`tagConfig()` precedence:** GUID section exists -> use ONLY it (missing tag = vanilla); else the dev keys in `levelup_override.ini`.
- **Tests (host):**
  - GUID bit pack/unpack;
  - INI parse/write round trip (incl. unknown lines kept, multiple GUIDs, case-insensitive tags);
  - precedence;
  - fake globals table for the hooks (gate_test style, `stub/`).

### M4 - character-screen UI + R11 (C++ draft by Sonnet in parts, Opus review)
- **`patch_manager_mods/_shared_src/GuiKit.h`** (header-only, `namespace guikit`; copied from Mod 3/Mod 14, which stay unchanged):
  - `createLabelFromTemplate`, `setRect`/`setVisible`/`setClickable`/`setText`/`setTextColor`/`setFill`/`setAlpha`;
  - `Scale` from a reference label;
  - `cursorEngine`/`hit`/`clickEdge`/`playHoverSound`.
  - Every engine fn address goes in a struct (host fakes); readback gates before colour writes (Mod 14 1140-1150).
- **`level_up_override_src/WizardLogic.h`** (pure, no engine):
  - presets for a tag -> page sequence role -> style -> alignment -> convert -> skills;
  - answers offered = distinct values among the still-matching presets;
  - skip single-answer pages;
  - result when 1 preset remains; Back pops one answer.
  - Python twin `research/companion_levelup/wizard_twin.py` + parity over the real `[presets]` index (all tags, all answer paths).
- **`level_up_override_src/LevelUpUi.h`** (included by LevelUpOverride.cpp so the harness sees it):
  - **Creation:** at the U1 ctor-time site, create our labels once per panel (Configure button, status line, wizard bg/title/6 cells/Back/Cancel; the
    result page reuses cells for summary, two toggles, Apply, "Use vanilla").
    - Templates `LBL_EXPERIENCE_STAT` / `LBL_XP_BACK`.
    - Store the control pointers keyed by the panel ptr + a magic check.
  - **Per tick** from the 0x84F221 hold gate (SetStats; panel = `[ebp-0x288]`: add `ebp` to that hook's params):
    - leader = SetStats' stats (ECX);
    - PC/unconfigurable -> hide all;
    - Configure label under BTN_LEVELUP (~[362,370,94,32]): text "Configure" / "Configured: <short>";
    - **R9 nudge:** highlight colour + "Plan level-ups" when real CanLevelUp && unconfigured;
    - **R11 status line** (~[300,405,236,20], left of Mod 14 x>=536):
      - held: "Holding N level(s) for <Jedi class> - <short>";
      - configured, not held: "Auto path: <short>" (+ "instant" when on);
      - unconfigured: hidden.
    - **Clicks:** clickEdge + hit on our visible labels only, while the character screen is the active panel; hover sound.
  - **Wizard flow:** Configure -> pages (WizardLogic) -> result page (preset name + summary, toggles "Instant level-ups" default off, "Hold until Jedi"
    shown only when hold_class != 0, default on) -> Apply writes `levelup_playthroughs.ini` under the GUID (creating it) and forces a config reload;
    "Use vanilla" removes the tag line. Cancel/close resets.
  - **Mod 14 coexistence:**
    - our ids/controls after Mod 14's;
    - the wizard bg must not overlap Mod 14's panel (x 536-752) - keep the wizard in x ~150..530;
    - never touch Mod 14 controls.
- **R11 banked message:** in the hold gate, when a configured tag first becomes HOLD at key K (or K rises), show the U2 message once per (GUID, tag, K) per
  session: "<Name>: level banked - waiting for <Jedi class> (level-up path)". Rate-limit to one per 5 s; never from inside AutoLevelup (the gates are
  callers, OK).
- **Tests:** WizardLogic parity (C++ vs `wizard_twin.py`); status-line text builder unit cases (held N=1/3, configured, instant, unconfigured, PC); UI
  handler fakes in `gate_test.cpp` (create once per panel, hidden for PC, click on Configure opens page 1, Apply writes the INI and reload switches
  `tagConfig`). Mutation spot-check of WizardLogic + status builder.

### C - combined gates, review, install (Opus)
- **Offline:**
  - mingw `-Wall -Wextra` clean;
  - `parity.py` (DRY, levelNow, wizard, apply, gate) 0 mismatches;
  - `simulate_rewrite.py`, `check_paths.py` 0 errors;
  - mutation spot-check;
  - independent Sonnet review (hang hazard, stack/EAX for every new hook, ctor-time creation safety, double-create on panel re-create, click-through leaks,
    INI write atomicity, GUID hooks on save/load/new-game paths, Mod 14 overlap).
- **Install** (ask the user):
  - stage `$T/levelup_stage5`; the game-dir `levelup_override.ini` reset to `mode=rewrite apply=1 verbose=1` (no dev tag keys);
  - copy the table; `verify` 0 mismatches; `manifest` 0.3.0.
- **Combined live checklist** (user plays; Opus reads `levelup_override_log.txt` from the recorded line, `levelup_playthroughs.ini`, and the save's
  GLOBALVARS via pykotor read-only):
  1. Character screen with Atton leading: Configure button visible; nudge when he can level and is unconfigured; layout OK on ultrawide; Mod 14 panel intact.
  2. Wizard for Atton (convertible), Kreia or Visas, and T3-M4 (droid): pages skip correctly, Back/Cancel work, Apply writes the INI.
  3. Atton configured with hold on, Scoundrel 7 with XP: no portrait indicator, buff icons visible, no Level Up/Auto buttons, the hotkey does nothing,
     status "Holding N level(s) for Jedi Sentinel", banked message shown once.
  4. Instant on for a non-held companion: it levels by itself on the XP award; log `INSTANT` + `APPLIED`.
  5. Manual level-up of a configured non-held companion: picks kept (R1).
  6. Unconfigured companions + PC: vanilla.
  7. Save -> quit -> load: config still applies (`GUID load` line). Load an older save of the same playthrough: same GUID. New game: unconfigured.
  8. No hang, no crash, no input leak to the world.
- Rollback: `mode=off` (hot), or reinstall `backups/pre_levelup_030a_20261002_103155/` (0.3.0-a) or the newer backup.

### W - wrap-up
- Doc 08 "Phase 5 results" + user guide (configure, presets, the leader-only character screen, the PC is unaffected), MASTER_PLAN, memory, DB lessons, commit.
- Open later:
  - vanilla-GUI plan-mode wizard;
  - Auto confirm-box hint;
  - dynamic per-answer values;
  - R10 PC filter mod;
  - mod #142 alignment source;
  - portrait badge (declined for now).

## Verification (end to end)
- **Offline:** all gates in step C green; the Sonnet review has no blocker.
- **Install:** `verify` 0 mismatches; live ids == stage; backup path recorded in doc 08.
- **Live:** the combined checklist 1-8 with log evidence (`HOLD`/`UNHOLD`, `INSTANT`, `APPLIED`, `GUARD`, `GUID load/save/new`, `UI` lines, banked message)
  and a `LUO_G*` read-back from the save.
- **Exit criterion:**
  - a player configures a companion in-game, and the choice persists with the save;
  - configured companions level along the preset (instant or banked);
  - held companions are clearly explained;
  - unconfigured companions + the PC are vanilla.
