# Level-up override - Phase 6: live verification -> 1.0.0 release hardening + preset quality + R10 PC filter research

(Repo copy of the approved plan, 2026-10-03. Authoritative for Phase 6. Progress: doc 08 "Session notes - Phase 6".)

## Context
- Phase 5 (2026-10-02/03) built and installed 0.3.0 plus follow-ups.
  - LIVE: dll e64cb387, 26 hooks; set 31 ids / 198 hooks; stage `$T/levelup_stage7`; backup `backups/pre_levelup_030_page_20261003_002447/`.
  - Features: save binding (GUID in 32 `LUO_G*` globals + `levelup_playthroughs.ini`); a single options page on the character screen (replaced the
    wizard); a `[recommended]` canon-style preset per companion; R9 nudge; R11 status line + banked message; leader instant (hook 0x74F423).
- Verified live: UI creation and clicks; Apply -> GUID -> ini; non-leader instant 3 -> 7 along the preset; the GUID round-trips through the save
  file (QUICKSAVE `g11b3b802`).
- **Not yet verified live:** options page look; leader instant; HOLD + status line + banked message; GUI guards on a held leader; R1 manual picks
  kept; restart -> load -> GUID; new game -> unconfigured; no input leak.
- User decision 2026-10-03, next-phase scope: (A) **Release hardening to 1.0.0**; (B) **Preset quality**; (C) **R10 PC level-up filter: research
  track** (a separate mod, user decision (d)). Dynamic per-answer builds are deferred.
- Related plans by other sessions (stay aligned): `patch_manager_mods/COMMON_EXTRACTION_PLAN.md` rows 4/7/9 (IniFile, GuiKit, SaveBinding incl. our
  GUID globals); `patch_manager_mods/20_TESTING_PLAN.md` (our parity suite is the model for `Common/TestHarness.h`; `*_bin` belongs in `$TMPDIR`);
  doc 07 shared save-binding plan.

## Read first after a context clear
1. `research/companion_levelup/MASTER_PLAN.md` status line.
2. `patch_manager_mods/08_level_up_override.md`: Status R1-R11; "Phase 5 UX research", "Phase 5 U facts", "U5", "Phase 5 C"; "Session notes - Phase 5"
   end block; **"Session notes - Phase 6"**.
3. Sources in `patch_manager_mods/level_up_override_src/`: `LevelUpOverride.cpp` (gates, M3 block, R11 helpers, leader instant, exports),
   `LevelUpUi.h` (options page), `OptionsLogic.h`, `WizardLogic.h`, `Playthrough.h`, `../_shared_src/GuiKit.h`.
4. Tests in `research/companion_levelup/tools/parity/`: `parity.py` (DRY, levelNow, apply, gate_test, playthrough_test, ui_test), `options_parity.py`,
   `wizard_parity.py`, `guikit_test.py`. Also `simulate_rewrite.py`, `check_paths.py` (`.venv/bin/python3`), `gen_runtime_table.py` (`.venv/bin/python3`).

## Rules (every step)
- **Git:** never push or amend. `git add -- <new>`, then `git commit -- <explicit paths>` with trailer `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
  Ignore `logs/debug_pykotor.log*`, `__pycache__`, and the foreign `level_up_override_src/PULL_REQUEST.md`.
- **Phase sequencing:** verify -> commit -> launch. At most ONE C++ coding agent and ONE Ghidra agent at a time. Python/data agents may run in
  parallel on disjoint files.
- **Ghidra:** `research/companion_levelup/tools/ghidra_run.sh`. First `pgrep -fa '^java.*analyzeHeadless'`; if busy wait 60 s, max 10 tries. Query the
  knowledge DB and `phase3/4/5_evidence` first. Evidence -> `research/companion_levelup/phase6_evidence/` (prefixes `R10_*`, `V_*`).
- **Engine safety:** never veto inside CanLevelUp/ValidateLevelUp/LevelUp; never inject a class through the pending struct.
- **Hooks:** no address used by another patch; +-16 overlap checked with `research/companion_levelup/phase5_evidence/U_site_check.py`; live == vanilla
  (`tools/pebytes.py`).
- **Install (user consent every time):** game closed (`pgrep -fa '^S:.*swkotor2\.exe'` empty). Back up `$G/{kpm_install_state.json,patch_config.toml,patches/}`
  + `levelup_*.ini` to `~/Games/KOTOR2_Mod_Workspace/backups/<name>_<ts>/`. Hooks/exports changed: stage FROM LIVE with
  `patch_manager_mods/mods678_research/stage_and_verify_install.py stage $T/levelup_stageN <our kpatch>` (`4gb-patch hooks=DIFF` is an artifact), `verify`
  the stage (only our dll may differ) -> `KPATCH_NO_LAUNCH=1 $T/bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches <stage> <live ids in patch_config
  order>` -> `verify` 0 mismatches. Dll-only change: hand swap of `$G/patches/level-up-override.dll` (backup first; re-md5 live first, other sessions
  swap too). Build: copy `*.cpp *.h *.toml exports.def` to `$T/Patches/LevelUpOverride/`, run `python3 ../create-patch.py`. `$T/Patches/_shared/*.h`
  forwards to `Patches/Common/`. Never launch the game.
- **Never `pkill -f`** a pattern contained in your own command.
- **Persist after every milestone:** doc 08 section + "Session notes - Phase 6"; DB rows (`python3` sqlite3, real date); MASTER_PLAN status line; memory
  `kotor2-levelup-override` + MEMORY.md line; commit.

## Delegation
- **Sonnet 5.5** (`general-purpose`, `model: sonnet`): Ghidra batches, Python/data work, first-draft C++ to a precise spec, tests, mutation runs,
  independent reviews, user-guide drafts. Each prompt names the files allowed to edit, the acceptance commands, and asks for a <= 400-word report with
  `file:line`. Agents never install, launch, commit, or edit doc 08.
- **Opus:** specs, live-log analysis, diff review, gates, user questions, installs, persistence, commits.

## Steps
### Step 0 - preflight (Opus)
Copy this plan; create `phase6_evidence/README.md`; add "Session notes - Phase 6" to doc 08; MASTER_PLAN status, memory, commit.

### V - live run 4 (user plays; Opus reads)
- Before: record log line count (386), re-md5 live dll (e64cb387), confirm `levelup_playthroughs.ini` has `[g11b3b802]`.
- Checklist (load the 16:04 QUICKSAVE first):
  1. Options page: opaque look, ultrawide layout, Mod 14 panel intact; Kreia has no Jedi row; g0t0 has a Variant row; Use recommended, toggles, Apply,
     Vanilla, Cancel.
  2. Leader instant: Atton leading, configured instant, levels by himself: `INSTANT-LEADER` + `APPLIED`.
  3. Hold: Atton at Scoundrel 7 with XP >= 28000, bank on: no portrait icon, buff icons visible; Level Up/Auto/hotkey refused (`GUARD`); status "Holding
     N levels for Jedi Sentinel"; `BANKED` once (and again when N rises).
  4. R1: manual level-up of a configured, non-held companion keeps the player's picks.
  5. Unconfigured companions + PC: vanilla.
  6. Quit -> restart -> load QUICKSAVE: `GUID load g11b3b802 found=32/32`. New game -> unconfigured.
  7. No hang, crash, or input leak (incl. Auto confirm box or a dialog over the character screen).
- Opus analysis: log from the recorded line; `levelup_playthroughs.ini`; save GLOBALVARS via pykotor (ValBoolean msb-first, bit = catalogue position,
  `LUO_Gii` = GUID bit ii; snippet in doc 08 run-3 notes). On a crash: `kotor2-crash-triage` skill + `CRASH_INVESTIGATION.md`; check other mods' frames.
- Fix loop: finding -> test first -> fix -> gates -> install (consent) -> re-run affected items.

### A - release hardening -> 1.0.0 (after V is green)
- A1 dev-probe cleanup (Sonnet draft, Opus review): remove/trim `GUARD-PROBE` and `UI colour readback` spam; keep one-line
  `HOLD/UNHOLD/INSTANT/INSTANT-LEADER/APPLIED/GUARD/GUID/UI apply/BANKED`; `verbose=0` default; default `mode=rewrite` when the ini is missing? **ask the
  user** (today `log`); drop the legacy `tags=` / `<tag>__live` dev path or keep it behind a dev flag; remove dead wizard-page code (WizardLogic.h stays as
  the Item provider); update header comments.
- A2 robustness review (Sonnet, read-only): UI tick re-entrancy during load screens; panel re-create at the same address; playthrough cap (64); table
  reload while the page is open; leader instant vs 0x740FC0 (character change in progress) - decide on run-4 logs; thread assumptions; static memory
  (~1.3 MB Doc/text) - shrink `pt::Doc` caps if cheap.
- A3 tests to the shared standard: `*_bin` -> `$TMPDIR/kotor-tests/` + `.gitignore`; `run_all.sh` (or `parity.py --all`); `tools/parity/mutate.py`
  (>= 20 mutations over levelNow, gates, OptionsLogic, Playthrough, status builder, banked; 100 % killed); the onLevelUp T2 test (20_TESTING_PLAN ~175).
- A4 Common alignment: read COMMON_EXTRACTION_PLAN rows 4/7/9 + doc 07 save binding; write "level_up adoption notes" in doc 08 (SaveBinding: playthroughId,
  hooks 0x656640 / 0x65675B / 0x401C52, LUO_G globals; IniFile: Playthrough.h parse/serialise, mtime reload); offer to the Common owner; migrate only when
  Common lands. No unilateral moves of shared headers.
- A5 player docs: `level_up_override_src/README.md` user guide (install, Configure button, options page, recommended builds, instant/bank, per-save, PC
  unaffected, uninstall leaves harmless globals, troubleshooting, rollback); doc 08 "1.0.0 release" section.
- A6 1.0.0 build: manifest 1.0.0, all gates, independent Sonnet review, install (consent), smoke checklist (subset of V), commit message "1.0.0".

### B - preset quality (parallel with A1-A3; disjoint files)
- B1 warning triage: classify the 199 `check_paths.py` warnings; fix real ones; accepted ones -> `paths/presets/WARNINGS.md`. Gate: 0 errors, fewer
  warnings, gen_runtime_table + simulate_rewrite + parity green.
- B2 early-conversion re-base: prove every Jedi preset REBASEs (no HOLD-forever) when the story converts before `convert_at`; add cases per preset with
  `hold_class != 0`; report presets that would skip feats/powers.
- B3 build review vs research (read-only first): dark powers in light presets, missing key feats, stat mismatches; Opus decides with the user.
- B4 recommended list: confirm/adjust the 12 `recommended = true` picks with the user; regenerate the table.

### C - R10 PC level-up filter: research track (static only; future mod `pc-levelup-filter`)
- C1 static map (one Sonnet Ghidra agent, `phase6_evidence/R10_*`), from DB rows CSWGuiLevelUpCharGen 0x8F8070, CSWGuiFeatsCharGen::OnAcceptButton
  0x90B430, CSWGuiPowersLevelUp::OnAcceptButton 0x908B50, CSWCLevelUpStats::CanChooseFeat 0x8480B0, CalcNumberFeats 0x847E50,
  ComputeNumberKnownSpellsLeft 0x847F90. Find (a) list build sites; (b) per-entry availability test; (c) greyed vs hidden and index safety; (d) PC
  identification on the client; (e) candidate hook sites (live == vanilla, +-16 clean vs 198 live hooks).
- C2 design note (doc 08 or new doc 21): filter source, hide/dim/sort, configuration UX, interaction with level-up-override, effort/risk.
- C3 user decision gate: build or park; if build -> PHASE7_PLAN with a log-only probe first.

### W - wrap-up
Doc 08 "Phase 6 results"; MASTER_PLAN, memory, DB lessons, commit. Open later: dynamic per-answer builds; vanilla-GUI plan-mode wizard (Item 6);
portrait badge; Auto confirm-box hint; mod #142 alignment source; Common/SaveBinding migration.

## Verification (end to end)
- Offline (every build): `parity.py` (DRY + levelNow 0 mismatches; apply 0; gate/playthrough/ui 0 failures); `options_parity.py`, `wizard_parity.py`,
  `guikit_test.py`; `simulate_rewrite.py` 0 failures; `check_paths.py` 0 errors; mingw `i686-w64-mingw32-g++-posix -std=c++17 -Wall -Wextra
  -fsyntax-only` on a scratch `Patches/` tree; `mutate.py` 100 % killed (from A3).
- Install: `stage_and_verify_install.py verify` 0 mismatches; live dll md5 == build; our hook count as expected (26 today).
- Live: V checklist 1-7 with log evidence plus a pykotor GLOBALVARS read-back.
- Exit: 1.0.0 installed with a clean smoke run and a user guide; presets triaged + re-base proven; R10 research + design note + recorded build/park decision.
