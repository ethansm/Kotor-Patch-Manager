# Level-up override Phase 6b: R&D while waiting to play + forced-state / mock testing toolkit + doc-20 level-up items

## Context
- The user can't play for a while. This plan covers everything that can be done without the game, and makes the next play sessions short and
  decisive: forged saves put the game straight into each edge state, and automatic log checks mark each test item pass/fail.
- State at plan time (2026-10-03 ~02:00). Read doc 08 "Session notes - Phase 6" -> **RESUME HERE** block (commit 9904cfa) first.
  - LIVE: 0.3.1 "Pause at level N", dll aaedb93e (hand swap; hooks = the 0.3.0 install, 26 ours, set 31 ids/198 hooks).
    Backup `backups/pre_levelup_031_pause_20261003_015201/`.
  - Live `levelup_paths.ini` is still the pre-B1 table. Log is 472 lines; read from 473.
  - User is about to do run 5: Atton -> Bank -> "Pause at level 3" -> Apply -> hold checks.
- Done in Phase 6 so far: Step 0, B1 (warnings 199 -> 22), B2 (early-conversion re-base proven), C1 (R10 static map), 0.3.1 built/installed,
  V item 6 (restart -> load GUID) verified.
- User decisions (2026-10-03, this planning round):
  - (1) Forged test saves go into NEW removable save slots (consent per write, cleanup command).
  - (2) Doc-20 scope = level-up items + the shared infra they seed (TestHarness.h, pre_install_gate.sh). Other mods' phases stay with their
    sessions.
  - (3) 1.0.0 default with no ini = `mode=rewrite`.
  - (4) B1 spare-point choice (Kreia Stealth, Bao-Dur Persuade) is decided in the B3 review.
- Earlier decisions still in force: PHASE6_PLAN.md (A 1.0.0 hardening, B preset quality, C R10 research track). This plan supersedes its
  ordering for the "while waiting" period.

## Rules (unchanged from PHASE6_PLAN.md; short form)
- **Git:** never push/amend. `git add -- <new>`, `git commit -- <explicit paths>`, trailer `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
  - Never commit `logs/debug_pykotor.log*`, `__pycache__`, `level_up_override_src/PULL_REQUEST.md`.
  - Never commit other sessions' uncommitted files: `_shared_src/{GuiKit.h,SafeRead.h}`, doc 07, `steam_db_pr3/*`.
- **Sequencing:** verify -> commit -> launch. At most ONE C++ coding agent and ONE Ghidra agent at a time. Python/data agents may run in
  parallel on disjoint files. Agents get: allowed files, acceptance commands, a <= 400-word report with file:line. Agents never install, launch,
  commit, or edit doc 08.
- **Never launch the game.** Live game files are touched only with consent, game closed (`pgrep -fa '^S:.*swkotor2\.exe'` empty), after a backup.
  Always re-md5 the live dll first (other sessions swap DLLs).
- **Doc-20 coordination:** another session owns doc 20 phases. Before touching `scripts/run_tests.py`, `tests/`, or creating
  `Patches/Common/TestHarness.h`, read the latest `patch_manager_mods/20*_session_*.md` + `git log -5 -- scripts/run_tests.py tests/` and
  coordinate via doc 20 notes. Never fork it.
- **Persist after every milestone:**
  - doc 08 "Session notes - Phase 6" (keep the RESUME HERE block current);
  - MASTER_PLAN status line;
  - memory `kotor2-levelup-override` + MEMORY.md line;
  - commit.
- Paths:
  - `$G` = `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II`; saves are in `$G/cloudsaves/62703561/<NNNNNN - Name>/`.
  - `$T` = `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`.

## Step 0 - persist this plan (Opus)
- Copy this plan to `research/companion_levelup/PHASE6B_PLAN.md`.
- Doc 08 RESUME HERE: add a pointer to it and record decisions (1)-(4).
- MASTER_PLAN status, memory, commit.

## Track M - forced-state + mock testing toolkit (the user's main ask)
Goal: every V-checklist item and every edge case can be (a) proven offline against the real C++ with fake engine memory, and (b) reproduced in
game in about a minute from a forged save, with the log auto-checked.

### M0 - research: where the engine reads party creatures from a save (Sonnet, read-only, Python)
- Today, Atton's `AVAILNPC0.utc` in SAVEGAME.sav says XP 30014, Scoundrel 3. Determine which copy the engine loads for an ACTIVE party member:
  - `AVAILNPC<n>.utc` in SAVEGAME.sav;
  - the creature list in the nested module `.sav` (`<module>.git` creature entries);
  - or both.
- Method:
  - Diff the copies in the existing QUICKSAVE/Game2 with `pykotor.extract.savedata` (`SaveFolderEntry`, `SaveNestedCapsule.get_character`,
    PARTYTABLE `PartyMemberEntry`).
  - Plus the knowledge DB / doc 08 notes on load (hook 0x65675B area).
- Also check: the leader/party-member list in PARTYTABLE; the PC creature location; savenfo fields shown in the load menu (name, area, time).
- **pykotor round-trip safety:** the venv is pykotor 2.3.12, and `replay/README.md` documents a bug that silently reverted nested capsule
  edits (patch #6 under `hardware-tools-docs/.../pykotor_patches/`).
  - Check whether this venv has the patch.
  - Prove read -> write -> read byte stability on a COPY in `$TMPDIR`.
  - If unpatched, the forge uses its own minimal ERF writer or applies the patch to the venv (ask the user).
- Output: `research/companion_levelup/phase6_evidence/M0_save_layout.md` (which resource holds class list / XP / feats / powers for active vs
  inactive members; GFF field names; the safe-write verdict).

### M1 - save forge: `scripts/kotor_save_forge.py` (Sonnet draft, Opus review)
- Built on `pykotor.extract.savedata`. Also `pykotor` read/write erf/gff.
- **Commands:**
  - `list` — slots, savenfo name, GUID decoded from LUO_G*, party with class/level/XP.
  - `inspect <slot>` — full dump for one slot.
  - `forge <source-slot> <scenario> [--name]` — writes a NEW slot.
  - `cleanup` — removes every slot whose savenfo name starts `LUO TEST`, after listing them and asking for confirmation.
- **Safety:**
  - Never writes an existing slot. A new slot number is max+1.
  - savenfo name `LUO TEST <scenario>`.
  - `--dry-run` default; `--write` needs an explicit flag.
  - Refuses while the game runs.
  - Writes `forge_manifest.json` into the slot: source slot, scenario, edits, and the md5 of each changed resource.
  - Stdout says "Steam Cloud may sync this slot until `cleanup`".
- **Scenario library:** `tests/luo_scenarios/saves.toml`, data not code. Each scenario = edits on named creatures + globals + expected V items.
  Initial set:
  - S1 `atton_hold_default`: Scoundrel 7, XP 28000 (default pause -> HOLD 1).
  - S2 `atton_pause3_many`: Scoundrel 3, XP 105000 (pause 3 -> HOLD with N=11; BANKED text).
  - S3 `atton_converted_early`: Scoundrel 3 + Jedi Sentinel 1, XP for 8. Expect REBASE onto the Jedi records; no HOLD.
    - Adding a class by save edit: feats/powers/class entry per M0. Flag as "engine-validity unproven" until loaded once.
  - S4 `atton_cap`: level 30 / XP max (no level-up; no status crash).
  - S5 `guid_partial`: 16 of 32 LUO_G set (load -> treated as none / logged).
  - S6 `guid_orphan`: valid GUID with no ini section (-> unconfigured).
  - S7 `guid_cleared`: LUO_G removed entirely (mod-removal safety: loads cleanly).
  - S8 `kreia_levelable` / S9 `g0t0_levelable`: options page rows (no Jedi row / Variant row).
  - S10 `pc_led_levelable`: the PC leads and can level (PC untouched).
  - S11 `multi_levelable`: 3 companions with banked XP at once (status/banked text per companion; banked message gap).
- **Tests:** `tests/luo_scenarios/forge_test.py`. Forge each scenario into `$TMPDIR` from a fixture copy, re-read it, assert the edits plus an
  unchanged byte-hash for every untouched resource.
  - Fixture: a minimal checked-in save copy is too big/personal, so tests run against a `$TMPDIR` copy of the user's Game2 when present and
    skip otherwise (documented).

### M2 - ini scenario switcher: `scripts/luo_scenario.py` (Sonnet, Python)
- Applies a named config scenario to the game-dir `levelup_override.ini` / `levelup_playthroughs.ini`:
  - dev keys `preset./hold./instant./pause.<tag>=`;
  - `mode=log` dry-run;
  - `verbose=1`;
  - a hand-written GUID section matching a forged save's GUID.
- Backup -> write -> print diff. `restore` puts the backup back. Game must be closed; consent per apply (the tool prints the plan, Opus asks).
- Scenarios live next to the save scenarios (same toml, `[config]` table) so one id = save + config + expectations.

### M3 - offline scenario runner (fake engine; the T2 harness)
- New `research/companion_levelup/tools/parity/scenario_test.cpp` + `scenarios.py`. Table-driven from the SAME `tests/luo_scenarios/*.toml`.
  Each scenario becomes:
  - fake creature(s) — reuse `makeCreature` (`gate_test.cpp:68-83`, XP variant `ui_test.cpp:129`);
  - fake party/leader — `fakeControlled`/`fakePartyGet` (`gate_test.cpp:31-39`);
  - fake globals table (`gate_test.cpp:41-66`) preloaded with the scenario's LUO_G bits;
  - rules XP table (`ui_test.cpp:232-237`, swap in the real exp table values from `exptable.2da` via a fixture);
  - an ini via `setIni` (`gate_test.cpp:88-93`).
- Then an action script runs through the REAL `LevelUpOverride.cpp`: `load`, `tick`, `canLevelUp@site`, `autoLevel`, `levelUp`, `save`,
  `newServer`, `clickPage <ctl>`.
- Asserts expected decisions plus expected log lines (`logHas`). The same scenario ids are used in game (M1) and offline, so a live failure maps
  1:1 to an offline repro.
- **onLevelUp T2 test** (doc 20 line 177): seam `decide(...)` out of `onLevelUp` (`LevelUpOverride.cpp` ~1250-1293; audit 20a §3.11). Drive
  `onLevelUp`/`LevelUpOverrideEntry` with a fake caller chain (AutoLevelup on/off the chain) -> DECIDE/DRY/SKIP/APPLIED.
- **LUO_G removal safety** (doc 20 line 178): a load with no LUO_G globals -> unconfigured, no write until Apply; save -> globals added; a load
  of a partial set -> none.
- **F10 fix** (20d): the stub `VirtualQuery` returns `RegionSize = 1<<40`, so `-m32` loops forever. Return the arena bounds / a 32-bit-safe
  size. Add a `-m32` build of gate/ui/scenario tests (T3) if the multilib toolchain is present.

### M4 - live log checker: `scripts/luo_log_check.py` (Sonnet, stdlib)
- `luo_log_check.py --from <line> [--scenario S2]`.
- Parses `levelup_override_log.txt` into events (GUID new/load/save/new-server, CONFIG, UI click/apply/page, HOLD/UNHOLD, GUARD, INSTANT,
  INSTANT-LEADER, APPLIED, BANKED, ENGINE mismatch, APPLY ABORTED).
- Evaluates the V checklist (items 1-7) and the scenario's expectations: pass / fail / not-exercised, with line numbers.
- Also replays the observed (classes, key, cfg) from DECIDE lines through `simulate_rewrite.level_now` to flag live/offline divergence.
- Also reports the first-launch banner version and dll md5 vs expected.
- Pattern from `patch_manager_mods/mods678_research/analyze_play_logs.py`.
- Tests: fixtures cut from the committed run-3/4 logs (`phase6_evidence/V_run4*.txt`, doc 08 run-3 excerpts).

### M5 - persist the strategy: doc + skill
- Doc `patch_manager_mods/26_FORCED_STATE_TESTING.md` covers:
  - the 3 layers (offline fake-engine scenarios / ini overrides / forged saves);
  - the safety rules (new slots only, consent, cleanup, cloud-sync note, game closed, never launch);
  - the scenario-file format;
  - how to add a scenario for a new mod (the hotbar/BuffHUD can reuse the forge for party/XP/global states);
  - the T-tier mapping (M3 = T2/T3, M1+M4 = T5 accelerators);
  - M0 findings.
- Skill `.claude/skills/kotor2-forced-state-testing/SKILL.md`. Same format as `kotor2-crash-triage`: frontmatter `name`/`description`, then
  `## Step` sections.
  - Triggers: "test edge case", "force a state", "make a test save", "check the play log".
  - Steps: pick/define scenario -> run offline scenario -> forge save (consent) -> config switch (consent) -> user plays -> `luo_log_check.py`
    -> cleanup.
  - Points at doc 21 for details.
- Add a doc-20 cross-reference (one line in doc 20's level-up section) via the doc-20 coordination rule.

## Track A - 1.0.0 hardening (no game needed; install waits for consent)
- **A1 cleanup** (Sonnet C++ draft, Opus review):
  - Remove GUARD-PROBE / `UI colour readback`. Keep one-line event logs.
  - `verbose=0` default.
  - **Default `mode=rewrite` when the ini is missing** (decision 3); update the header comment + manifest text + gate_test expectations.
  - Legacy `tags=` / `<tag>__live` behind `dev=1`.
  - Remove dead wizard-page code (WizardLogic.h stays as the Item provider).
  - Keep log tag text stable (M4 + parsers depend on it; update M4 fixtures if a line changes).
- **A2 robustness review** (Sonnet read-only):
  - UI tick during load screens; panel re-create at the same address; playthrough cap 64; table reload with the page open.
  - Leader instant vs 0x740FC0 (character change in progress); single-thread assumption; static memory (~1.3 MB) and `pt::Doc` caps.
  - Pause edge cases: pause value from a dev key > convertAt; preset changed in the table under a stored pause; a companion who joins above
    pause.
  - Findings become M3 scenarios first, then fixes.
- **A3 tests to the shared standard:**
  - `*_bin` -> `$TMPDIR/kotor-tests/` (update `.gitignore`).
  - Confirm `tests/suites.toml` runs all level-up suites (4 entries today; add scenario_test, forge_test, luo_log_check tests).
  - `tools/parity/mutate.py`: >= 25 (file, regex, replacement) mutations over levelNow (incl. pause), gates, OptionsLogic, Playthrough (pause
    parse/serialise), the bank cycler, the status builder, banked; 100 % killed; scratch copies, never in place.
- **A4 Common adoption notes** in doc 08: SaveBinding (playthroughId, hooks 0x656640/0x65675B/0x401C52, LUO_G globals, removal safety from M3)
  and IniFile (Playthrough.h incl. `|pause=`). Offer to the Common owner; no moves.
- **A5 README** `level_up_override_src/README.md` (Sonnet draft, Opus edit):
  - install, Configure, options page, recommended builds, **Pause at level N** (incl. the "bank everything until Jedi" recipe and the clamp on
    preset change);
  - instant/bank, per-save, the PC is unaffected, uninstall leaves harmless globals (proven by M3/S7), troubleshooting via `luo_log_check.py`,
    rollback.
- **A6 1.0.0 build:**
  - manifest 1.0.0; all gates + mutate + scenario runner;
  - independent Sonnet review;
  - install with consent: stage from live if hooks change, else a dll swap; plus copy the B1 table;
  - smoke = forged scenarios S1/S2/S7 + log check.

## Track B - preset quality
- **B3 build review** (Sonnet read-only first): per companion vs research (`research/companion_levelup/raw`, `text/`,
  `installed_mods_progression_relevant.json`, mod #142 alignment), utc start.
  - Flags: dark powers in light presets, missing key feats, stat mismatches.
  - Includes the B1 spare-point choice (Kreia Stealth L10-30, Bao-Dur Persuade): `AskUserQuestion` per contentious item.
- **B4 recommended list:** confirm/adjust the 12 `recommended = true` picks with the user; regenerate the table; gates.
- **B5 pause-aware presets** (new): for every Jedi preset, report what "Pause at minPause" loses (base-class feats skipped).
  - Reuse `phase6_evidence/B2_rebase_check.py` with a pause dimension; a table for the README ("pausing early trades X for Y").

## Track C - R10 PC level-up filter (research only)
- **C1b DB rows:** verify the R10 identities from `phase6_evidence/R10_findings.md` per `ghidra_knowledge/RE_AGENT_GUIDE.md` (Haiku + decoys),
  then insert.
- **C2 design note:** `patch_manager_mods/25_pc_levelup_filter.md`. Covers:
  - filter source (preset-style PC paths vs a theme filter);
  - hide (F1+P1 with F2+P2) vs dim (F2/P2 only);
  - the accept-trap (hidden-but-counted soft-lock) and the Recommended-button bypass;
  - PC test via the server creature's empty tag (`0x77D800`);
  - config UX (options page when the PC leads); skills (S1 0x90EDE0 veto vs grey 0x90F610);
  - the log-only probe spec (what C1 listed); effort/risk.
- **C3:** `AskUserQuestion` build or park; if build -> PHASE7_PLAN with the probe first.

## Order of work (while waiting)
1. Step 0.
2. In parallel (disjoint files):
   - M0 (Python, read-only);
   - C1b (Haiku/Sonnet DB);
   - B3 (read-only).
3. M1 forge + M4 log checker (Python, parallel) -> Opus review -> commit.
4. M3 scenario runner + onLevelUp seam + F10 (the one C++ agent) -> commit.
5. A1 (C++ agent, after M3 commit) -> A2 review -> fixes as scenarios -> A3 mutate -> commit.
6. M2 config switcher, M5 doc 21 + skill, A4 notes, A5 README, C2 note -> commit.
7. B4/B5 with the user; A6 build when the user is ready to play (install consent).
- When the user returns to play:
  - the run-5 card (Pause at level 3) as already given;
  - offer forged saves S1/S2/S3/S7/S10 (consent);
  - check with `luo_log_check.py --from 473`.

## Verification
- **Offline gates (every change):**
  - `cd research/companion_levelup && python3 tools/parity/parity.py` (all suites 0 mismatches/failures, incl. pause + early-conversion);
  - `options_parity.py`, `wizard_parity.py`, `guikit_test.py`; `simulate_rewrite.py` 0 failures;
  - `../../.venv/bin/python3 check_paths.py paths/*.toml paths/presets/*.toml` 0 errors;
  - the mingw `-fsyntax-only` check on a `$TMPDIR` Patches tree;
  - new: `scenario_test`, `forge_test.py`, `luo_log_check.py` self-tests, `mutate.py` 100 % killed;
  - `python3 scripts/run_tests.py` green for the level-up items.
- **Forge:** `forge_test.py` round-trip on a `$TMPDIR` copy (edits present, untouched resources byte-identical); `list` decodes the GUID of
  the user's current save as gfb7bdbf7.
- **Log checker:** reproduces the run-4/4b verdicts (item 6 pass; hold not exercised) from the committed excerpts.
- **Live** (user, later): forged scenario -> expected log events confirmed by `luo_log_check.py`; then `cleanup` removes all LUO TEST slots.
