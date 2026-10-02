# Level-up override - Phase 4: build 0.3.0 (hold mode, opt-in instant levelling, save-bound config, character-screen Configure + wizard)

## Context
`level-up-override` 0.2.1 is LIVE: it rewrites the pending level-up only on AUTO level-ups (AutoLevelup 0x6BEDF0 on the call chain), for tags in the
game-dir ini (`mode=rewrite tags=atton`). Phase 3 (static research, doc 08 "Phase 3 research" Items 1-7 + synthesis) and the preset work (80 presets
for 12 companions in `research/companion_levelup/paths/presets/`) are done.

User requirements: R1 never touch manual picks; R2 per-playthrough builds; R4 instant levelling; R5 hold + hide the indicator; R6 Configure button,
unconfigured = vanilla; R7 instant is opt-in per character; R8 presets + wizard; R9 configure nudge.

User decisions 2026-10-02 (doc 08 "User decisions" a-g):
- presets AND wizard; for now the wizard is question-based (dynamic per-answer values come later);
- Configure button on the character screen; unconfigured characters stay vanilla;
- the PC is out of scope (R10, a future separate PC level-up filter mod);
- config is tied to the save file;
- droids and non-Jedi companions are in scope.

Goal of 0.3.0: a player-facing mod:
1. the player opens the character screen and presses Configure;
2. a short wizard picks a preset for that companion and sets instant on/off;
3. the choice is stored per playthrough (bound to the save);
4. the companion then levels along the preset, either instantly or by banking levels until its Jedi class appears, with the indicator hidden while held.

## Read first after a context clear
- `research/companion_levelup/MASTER_PLAN.md` status line.
- `patch_manager_mods/08_level_up_override.md`:
  - Status (R1-R10).
  - "Phase 3 research" Items 1-7: hook tables with VAs and bytes, the hold rule, banking, save binding.
  - "Phase 3 synthesis" and "User decisions".
  - "Session notes - Phase 3 research + presets" and "Session notes - Phase 2 Step 4/5": live state, install steps, gotchas.
- `research/companion_levelup/paths/presets/WIZARD_SCHEMA.md`.
- Mod source: `patch_manager_mods/level_up_override_src/`:
  - `LevelUpOverride.cpp` (785 lines): config reload, `parseTable`/`lookupRecord`, `computeRewrite`, `applyRewrite`, the `LevelUpOverrideEntry` detour.
  - `kotor2-steam-aspyr.hooks.toml`, `manifest.toml` (version 0.2.1).
- Tools in `research/companion_levelup/`:
  - `gen_runtime_table.py`: reads ONLY `paths/*.toml` today.
  - `check_paths.py`, `levelup_rules.py`.
  - `simulate_rewrite.py`: the Python twin of `computeRewrite`.
  - `tools/parity/`: `parity.py`, `parity_main.cpp`, `apply_test.cpp`, `stub/`.

## Rules (every step)
- Never push or amend. Commit with `git commit -- <explicit paths>`; sibling sessions commit on the same branch, so never `git add -A`.
  - Ignore `research/companion_levelup/logs/debug_pykotor.log*` and `__pycache__`.
  - Trailer: `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Phase sequencing: never overlap a background job with a coding step. Verify, commit, then launch.
- **One Ghidra headless run at a time across ALL sessions.**
  - Run via `research/companion_levelup/tools/ghidra_run.sh`.
  - Check `pgrep -fa '^java.*analyzeHeadless'` first. BUSY: wait 60 s and retry, at most 10 times. A BUSY with no java process is a false positive.
  - Query the knowledge DB `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db` (python sqlite3) and `phase1_evidence/` + `phase3_evidence/` first.
  - Never re-decompile what is already there.
- **Engine safety rules (doc 08 Item 4):**
  - NEVER veto inside CanLevelUp 0x6B9790, ValidateLevelUp 0x6B9640 or LevelUp 0x6B9870. The AutoLevelup loop would spin forever.
  - Veto only at CALLER sites. Never inject a class through the pending struct.
- **Hooks:**
  - A hook address must not be used by any other installed patch: the patch manager hard-fails, and there is no chaining. Mod 14 owns 0x84D871, 0x84FBB0 and 0x84DEF0.
  - Run a +-16-byte overlap scan against all `kotor2-steam-aspyr*.hooks.toml` (repo + `$T/Patches/*`) for every new hook.
  - Live bytes == vanilla bytes (`tools/pebytes.py`).
- **Install:**
  - Only with the user's consent per install. Back up first: `$G/{kpm_install_state.json,patch_config.toml,patches/}` -> `~/Games/KOTOR2_Mod_Workspace/backups/<name>_<ts>/`.
  - Re-read the live id set and re-md5 every time; sibling sessions change it.
  - Use stage `$T/levelup_stage4/`, `stage_and_verify_install.py`, `KPatchLauncher ... --patches`, then `verify`, expecting 0 mismatches.
  - Rollback: reinstall the backup set, or `mode=off` in `levelup_override.ini`.
  - Never launch the game: the user plays and reports.
- Persist after every milestone: doc 08 section, DB rows (python sqlite3 with the real date, not `kdb_add.add_lesson`), MASTER_PLAN status line, memory `kotor2-levelup-override`, commit.

## Delegation (token conservation)
- **Sonnet 5.5 subagents** (`subagent_type: general-purpose`, `model: sonnet`) do:
  - every Ghidra batch;
  - Python tooling;
  - first-draft C++ for well-specified functions;
  - test-harness extensions;
  - independent code reviews.
- Each agent gets: exact files and functions to touch, the acceptance commands, and a <= 400-word report with `file:line` refs.
  - Agents never install, launch, commit or edit doc 08.
  - Ghidra agents write only to `research/companion_levelup/phase4_evidence/`.
- **Opus**:
  - writes the specs and agent prompts;
  - spot-checks every cited evidence line and reviews every diff;
  - runs the gates;
  - asks the user before installs;
  - performs installs, writes docs, DB rows and memory, and commits.
- At most one Ghidra agent at a time. Non-Ghidra agents may run in parallel when they touch disjoint files.

## Design (decided)
### Data model
- **Runtime table** `levelup_paths.ini`, emitted by `gen_runtime_table.py`:
  - one section per preset `[<tag>__<variant>]` with the existing `L<n>=` record format;
  - plus `[presets]` rows `<tag>__<variant>=<role>|<style>|<alignment>|<convert>|<skills>|<convert_at>|<hold_until_class>|<summary (<=60 chars)>`.
  - The live `paths/atton.toml` and `paths/kreia.toml` stay as the dev/default presets `atton__live` and `kreia__live`.
  - Tags must equal the live creature Tag (case-insensitive). Verify `VisasMarr`, `hk47`, `t3m4`, `g0t0`, `mand`, `hanharr` against the live save GFFs; fix the preset meta if needed.
- **Per-playthrough config** `levelup_playthroughs.ini` (game dir), section `[g<GUID hex>]`, one line per tag: `<tag>=<preset>|instant=0/1|hold=0/1`.
  - A missing tag = unconfigured = vanilla.
  - `levelup_override.ini` keeps the global `mode`/`apply`/`verbose` switches, plus a dev fallback `tags=`/`preset.<tag>=` used only when no playthrough config exists.
- **Hold/level rule** (pure function, C++ plus a Python twin):
  - `level_now(creature classes, preset, keyLevel)`. Primary key = char level + 1.
  - If the record's class == the newest class -> level with that record.
  - **Re-base:** else, if the preset has records for the newest class, use the record matching (newest class, its class level + 1). This covers a story conversion EARLIER than `convert_at`.
  - Else, if the newest class is the base class and the record class is the preset's Jedi class (conversion later than planned) -> HOLD when `hold=1`, or level with the base-class record matching (base class, class level + 1) when one exists.
  - Else vanilla (log `NOREC`).

### Hooks (all 5-byte `E8 rel32` call sites)
**Pattern:**
- `skip_original_bytes = true` and `exclude_from_restore = ["eax"]`.
- The handler calls the real function itself through a typed thiscall/fastcall pointer and returns the possibly-substituted EAX.
- No stack arguments at these sites, so the wrapper needs no stack fix. Each site is verified in Step S1.

**Hold (R5)** - handler `LuoCanLevelUpGate(stats)`. It calls the real CanLevelUp, then returns 0 when the creature's tag is configured and `level_now` = HOLD. Sites:
- **Confirmed:** 0x74F3C7 (portraits), 0x84F221 (character-screen buttons; also the UI tick), 0x5FD7CE (XP tail), 0x5FE2A3 (DistributeExperience), 0x66C95C (AddMultiClass auto branch), 0x5FEE32 (AutoLevelUpNPCs).
- **Collect in S1:** the 4 sites in 0x756B70, plus 0x752050, 0x7C9DF0 and 0x75FC30. Decide each one: veto or leave.
- **Guard the paths that skip CanLevelUp:** ShowLevelUpGUI 0x84FCD0 and the DoAutoLevelup OK path 0x850081, via their call sites or the HandleInputEvent 0x84F900 dispatch. S1 picks the site.

**Instant (R4/R7)** - handler `LuoOptionsGate(ecx, ebp, site)`. It calls the real accessor 0x72FB00 (fastcall ECX). For a configured `instant=1` tag that is not held, it returns a pointer to a DLL-static 16-byte copy whose +8 has bit 0 set; otherwise it returns the real pointer.
- Sites and creature locations: 0x74F3E9 `[ebp-0x38]`, 0x5FD7A0 `[ebp-0x14]`, 0x5FE281 `[ebp-0x18]`, 0x66C93A `[ebp-8]`.
- S1 confirms that only `+8` of the returned object is read before it dies at each site.

**Save binding:**
- Pre-save hook at the globals-saver entry 0x656640 (`thiscall(table, CExoString*)`, prologue `55 8B EC 6A FF`): add or set 32 Boolean globals `LUO_G00..LUO_G31` (GUID bits) through AddBoolean 0x654430 plus the boolean setter (find it in S1).
- Post-load hook 0x656758 (`89 45 F0 C7 45 FC FF FF FF FF`): read them back.
- GUID = 0 -> generate on the first Configure save (new game or a pre-mod save).
- Cache the GUID in DLL state. Clear it when the server object is recreated: the new-game/load path through 0x401BC0, so hook a call site there or detect a table-pointer change.

**UI (R6/R8/R9):** no ctor/tick/dtor hooks (Mod 14 owns them). Everything is driven from the 0x84F221 detour (runs every tick inside SetStats):
- recover the panel `this` from SetStats' frame (S1);
- create controls lazily with Mod 3's `createLabel` recipe (`patch_manager_mods/force_power_hotbar_src/ForcePowerHotbar.cpp:980-1030`) using a native `character_p.gui` label tag as the template (S1);
- detect a new panel by pointer + a magic check;
- poll clicks with `GetAsyncKeyState` + `cursorInGame` + `wHit` (ForcePowerHotbar.cpp:476/729/832);
- `playHoverSound` (~608).

Copy those helpers into `patch_manager_mods/_shared_src/GuiKit.h` (new, header-only). The level-up DLL is the first user; Mod 3 and Mod 14 stay unchanged.

**UI behaviour:**
- **Configure button:** placed under BTN_LEVELUP (character_p ~[362,370,94,32] in 800x600 units, aspect-fixed). Always visible on the character screen for a companion (never the PC).
  - Highlighted label + tooltip "Plan this character's level-ups" when the creature can level up and is unconfigured (R9 nudge).
  - Shows "Configured: <preset>" when configured.
- **Wizard panel:** background + title + up to 6 answer cells + Back/Cancel. Pages: role -> style -> alignment -> convert -> skills, each offering only the answers present among that companion's presets (skip a page with one answer).
  - Result page: matched preset name + summary, an "Instant level-ups" toggle and a "Hold until Jedi" toggle (when the preset converts), plus Apply / "Use vanilla (unconfigure)".
  - Apply writes `levelup_playthroughs.ini` under the current GUID and hot-reloads the config.
- Leader-only limitation: the character screen shows party slot 0 (`leaderClient()` in `patch_manager_mods/active_effects_panel_src/CharEffectsPanel.cpp:416`). Document it; switching the leader switches the configured creature.
- Deferred to Phase 5: a hint line in the Auto confirm box (needs the 0x84FE20 string trace); the vanilla-GUI plan-mode wizard (doc 08 Item 6).

## Milestones (each one: build -> offline gates -> user-consented install -> user live test -> persist)
### Step 0 - preflight (Opus)
- `git log --oneline -3` (expect 7c032ed or later) and `git status --short`.
- Write this plan's path (`~/.claude/plans/dazzling-brewing-quasar.md`, Phase 4) into the MASTER_PLAN status line and doc 08 "Session notes - Phase 3 research + presets". Both currently say "plan file named elsewhere".
- Update memory `kotor2-levelup-override` (Phase 4 in progress). Create `research/companion_levelup/phase4_evidence/README.md`. Commit.

### M0 - static facts for 0.3.0 (Sonnet Ghidra agent S1, then Opus review). No build.
1. **Each hook site above** (incl. the 0x756B70 x4 / 0x752050 / 0x7C9DF0 / 0x75FC30 CanLevelUp sites): disasm +-0x30 around the site.
   - Confirm: no stack args, and the callee's ret N; ECX/EAX liveness after the call; at the 0x72FB00 sites, which offsets of the result are read.
   - Bytes live == vanilla; overlap scan.
   - Evidence: `phase4_evidence/S1_sites.txt`.
2. **ShowLevelUpGUI and DoAutoLevelup guards:** call-site VAs + bytes (the HandleInputEvent 0x84F900 cases 0x27/0x29, and the AddEvent thunks).
3. **SetStats 0x84E6F0:** the frame slot holding `this` at 0x84F221, and the ECX source (stats?).
4. **Globals:** the Boolean setter/getter VAs + signatures next to AddBoolean 0x654430; the hook-framework param capture at 0x656758 (which register holds the table).
5. **Template tag:** a native label tag in the live `character_p.gui` usable as an InitControl template (read the GUI with pykotor; no Ghidra).
6. **Runtime-added controls:** whether the panel dtor frees controls appended to `panel+0x24` (double-free risk). Read Mod 3 doc 07 lessons and the dtor 0x84DEF0 decompile if needed.
- Persist: doc 08 "Phase 4 M0 facts", DB rows, commit.

### M1 - data + tooling (Sonnet Python agents, parallel on disjoint files)
- **`gen_runtime_table.py`:** emit all presets + the `[presets]` index; keep `paths/*.toml` as `<tag>__live`. Refuse presets with check errors.
  - Size check: count the records; raise the DLL parse caps in M2 accordingly.
- **`check_paths.py`:** model "Class Skill: X" feats (class-skill cost 1 after the feat). Re-run all 80 presets; record the warning delta.
- **Tag verification:** read creature Tags from the latest live save (`.../cloudsaves/62703561/`, availnpc*/PARTYTABLE via pykotor, read-only) and the utc templates; fix preset `meta.tag` mismatches.
- **`simulate_rewrite.py`:** add `level_now` + re-base + hold (Python twin) with unit cases:
  - early conversion;
  - late conversion with hold on and off;
  - droid single class;
  - unconfigured;
  - Atton held 8 -> 15 then Sentinel added (expected keys 9..15, all class 5, in one AutoLevelup; doc 08 Item 4).
- **Gate:** `check_paths.py` 0 errors on all presets; `simulate_rewrite.py` self-test pass; commit.

### M2 - 0.3.0-a: hold mode + indicator fix + instant (no UI; config from ini). One coding agent at a time.
- **C++** (Sonnet draft, Opus review) in `LevelUpOverride.cpp`:
  - table v2 parser (per-preset sections + index);
  - the playthrough config reader with the dev fallback in `levelup_override.ini` (`preset.<tag>=`, `instant.<tag>=`, `hold.<tag>=`);
  - `levelNow()` twin;
  - handlers `LuoCanLevelUpGate`, `LuoOptionsGate`, the GUI guards;
  - logging per decision: `HOLD`, `INSTANT`, `REBASE`, `VANILLA`, with tag, key and site.
  - Keep the 0.2.1 LevelUp rewrite and change it to use the configured preset.
- **toml:** the new hooks per M0, all `type="detour"`, `skip_original_bytes=true`, `exclude_from_restore=["eax"]`. `manifest.toml` -> 0.3.0.
- **Gates:**
  - mingw `-Wall -Wextra -fsyntax-only` clean;
  - `python3 tools/parity/parity.py` pass;
  - parity extended with `levelNow` cases (C++ vs the Python twin);
  - `apply_test` pass;
  - a gate-handler unit test with fake engine calls (MAP_32BIT arena; new stubs in `stub/`);
  - mutation spot-check of `levelNow` (copies under `tools/parity/_mut/`);
  - an independent Sonnet code review (hang hazard, stack balance, EAX/ECX, null creature, PC/non-companion passthrough).
- **Install:**
  - Ask the user. Stage `$T/levelup_stage4`.
  - Dev config: `preset.atton=atton__live hold.atton=1 instant.atton=0`.
- **Live test checklist** (user plays):
  1. A held Atton (Scoundrel, Jedi not yet added) with enough XP: no indicator, buff icons visible, no Level Up / Auto buttons, the hotkey does not open the level-up GUI, XP keeps growing.
  2. Unconfigured companion: vanilla indicator + buttons.
  3. `instant.atton=1` with hold off: Atton levels automatically on the XP award, path picks are logged APPLIED.
  4. Manual level-up of a configured, non-held companion: picks kept (R1).
  5. No hang, no crash.
- Logs are read by Opus.

### M3 - 0.3.0-b: save binding (log-only first)
- **Step 1 (probe build):** the save/load hooks only log (table pointer, GUID globals found, names written), plus a dev ini key `guid.debug=1`.
  - The user does save -> load -> new game; Opus checks `GLOBALVARS.res` in the save (pykotor, read-only) for `LUO_G*`.
- **Step 2:** the real GUID read/write + `levelup_playthroughs.ini`. Precedence: config by GUID, else the dev fallback.
- **Gates:** as M2, plus unit tests for the GUID bit packing and INI read/write.
- **Live:**
  1. Configure (via ini edit for now) -> save -> quit -> load: config applies.
  2. Load an older save of the same playthrough: same GUID.
  3. New game: unconfigured.

### M4 - 0.3.0-c: Configure button + R9 nudge + wizard panel
- Create `patch_manager_mods/_shared_src/GuiKit.h` (copied helpers from ForcePowerHotbar.cpp; Sonnet extracts, Opus reviews), and the UI module `LevelUpUi.cpp` in `level_up_override_src`.
- Wizard data comes from the `[presets]` index; filter by tag; derive the page answers.
- **Gates:** build clean; a pure-function unit test for the wizard page/answer derivation and preset matching (Python twin over the real presets index); code review (lifetime, double-free, input focus, aspect).
- **Live:**
  1. Button placement at the user's resolution (ultrawide).
  2. The nudge appears only for unconfigured + can-level.
  3. A wizard run for 3 companions (a Jedi convertible, Kreia/Visas, a droid).
  4. Apply -> ini written under the GUID -> behaviour switches without a restart.
  5. Unconfigure -> vanilla.
  6. No input leak to the world; Esc closes.

### M5 - wrap-up
- Doc 08 "Phase 4 results", user guide section (how to configure, presets list, limitations: leader-only character screen, PC not affected), MASTER_PLAN status, memory, DB lessons.
- Open Phase 5 items:
  - vanilla-GUI plan-mode wizard;
  - Auto confirm-box hint;
  - dynamic per-answer values;
  - R10 PC filter mod;
  - the mod #142 alignment-source check.

## Verification (end to end)
- **Offline:**
  - `check_paths.py` 0 errors (80+2 presets);
  - `simulate_rewrite.py` and the new `level_now` cases pass;
  - `tools/parity/parity.py` C++ == Python on all cases;
  - `apply_test` and the gate-handler tests pass;
  - mingw warnings clean;
  - Sonnet review findings resolved.
- **Install:** `verify` 0 mismatches; live ids match the stage; backups recorded in doc 08.
- **Live:** the M2-M4 checklists, with the user reporting and Opus reading `levelup_override_log.txt` (`HOLD` / `INSTANT` / `REBASE` / `APPLIED` lines with keys) and the save's `GLOBALVARS.res`.
- **Exit criterion:**
  - a configured companion levels along its preset with no interaction (instant) or banks levels until its Jedi class appears (hold), with the indicator hidden and the buff icons visible;
  - unconfigured companions and the PC behave exactly as vanilla;
  - the config follows the save.
