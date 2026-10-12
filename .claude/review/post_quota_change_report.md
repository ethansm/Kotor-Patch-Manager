# Post-quota change report (input for the Opus 5.5 review-plan session)

Generated 2026-10-10 by Sonnet 5.5. Read-only: no builds, no tests, no `scripts/` gate runs. Sources: `git`, `.claude/master_ledger.jsonl`, `~/.claude/projects/.../*.jsonl` transcript timestamps, `.opencode/plans/pending-agent-review.md` (the "manifest"), `.claude/review/preflight_report.md` (the "preflight"), `.claude/review/scope.json`.
Excluded by name, content not read or quoted: `.env`, `.pem`, `.credentials.json`, `sunshine/credentials/*`, `endpoint.env`, `*credential*`, `*secret*`. `tests/static/raw_addresses.toml` is listed by name and line count only (see R1).
Companion data file: `.claude/review/post_quota_scope.json` (commit list, dirty list, gate coverage).

## 0. What the planner needs first

1. **Boundary:** the last Claude Code assistant turn is **2026-10-07T01:05:42Z (2026-10-06 21:05:42 EDT)**. Everything after it is non-Claude work. HEAD is `22b56b4`.
2. **Size of the review:** 61 commits (`11e1284..HEAD`, 266 files, +11,823 / -723) plus **84 post-boundary dirty files** (41 modified, 43 new, excluding this task's own two output files; code + tests + docs). 77 other dirty files predate the boundary and are not Gemini's work (§3.2).
3. **The manifest's central alarm is a false positive.** 36 of the 43 BUILD_GATE FAIL rows (`py:x 0.1s`, including all 12 of `antigravity:3e24a0d8`) are a hard-coded fake-runner string in `scripts/test_pre_install_gate.py:148`, written into the production ledger by unit tests (§5). There was no "persistent build failure at `b5b5aba`".
4. **The real gate record is thin.** 7 genuine FAILs and 4 PASSes in the whole period. Eleven of the 18 mods changed after the boundary have no recorded gate PASS (§5.3). The newest gate FAIL (`check-shared-headers` at HEAD, 14:09:31Z 10-09) has no recorded fix, only a later equip3d-only PASS.
5. **Unreviewed, uncommitted behaviour changes sit in the working tree** (new `PerformCastSpell` detour, leader heal button, AH3 sentinel range, LevelUpOverride KpLog port). These are the highest-value review targets (§6).
6. **Gate infrastructure was written or edited by the same agents it gates.** `master_ledger.py`, `agent_diff_check.py`, `clef_matcher.py` are untracked; `run_tests.py` gained +234 lines committed and +12 dirty. Review the gate first, then trust its output (R1).

## 1. Boundary and method

| Item | Value | Evidence |
|---|---|---|
| Last Claude assistant turn | 2026-10-07T01:05:42Z | transcript `7ee5b038-...jsonl`, final message "Step 6 of 6 done... `6a9010d`, `11e1284`" |
| Manifest baseline session | `.claude/sessions/2663725.json`, `updatedAt 1791335142410` | `date -u -d @1791335142` = 2026-10-07 01:05:42 UTC. Exact match with the row above. The manifest mislabels it "Oct 6 approx 14:25 UTC". |
| Last Claude-trailer commit | `11e1284` 2026-10-06 21:05 EDT | `git log --grep Co-Authored-By`: 4 commits, all 10-06 (`cfd03fa`, `2cfa805`, `6a9010d`, `11e1284`). None after. |
| Later Claude activity | slash commands only | `/usage-credits` login 10-08T03:49Z, `/web-setup` 10-10T02:09Z, `/doctor`, `/clear`, this session. No assistant work turns. |
| Explicit "limit reached" text | **not found after 2026-10-04T05:03Z** | transcripts grep. The 10-06 session ended with a normal summary, not a limit message. |

**Assumption to confirm:** "quota reached" is taken to mean the end of the last Claude work session (above). If the user meant an earlier limit message (latest: 2026-10-04T05:03Z, resets 1:20am), the span widens to include `15a79d29..a9c27c83` Claude work, which is not Gemini's.

**Attribution limits:** every commit has author and committer `ethansm`. None carries a model trailer after the boundary. The ledger records `antigravity:<id>` / `opencode:<role>` / `ethansm` per event but **never the model name**. "Gemini 3.8 Flash" cannot be confirmed from the repo; it is an operator statement. 192 of 340 ledger rows are `ethansm` and cannot be assigned to a human or an agent. Treat time-window overlap as the only link between a commit and an agent.

## 2. Corrections to the manifest and preflight (use these, not theirs)

| Claim | Correct |
|---|---|
| Persistent BUILD_GATE FAIL at `b5b5aba` proves a broken pipeline (manifest §11.1) | Test-fixture leakage (§5.1). The agent's own run produced no real failure rows. |
| Range "since 2026-10-06": 67 commits (preflight) / 10 listed (manifest) | Exact range `11e1284..HEAD`: **61 commits**. |
| 258 ledger lines; `changed_count` 191-212 | 340 lines. Latest `SUBAGENT_DIFF_CHECK` for `latest.patch`: 21 files, verdict FAIL, agent `8e6ee335` (ledger line 330 per preflight). |
| "190 / 188 dirty files" | 247 by `status -uall` (preflight). Of these, 84 postdate the boundary (this report), 77 predate it, and 88 are cache or dump excerpts (42 `tests/.cache`, 46 `dump_excerpts`). |
| `47022c21` "passed its gates" | 0 BUILD_GATE rows for it. "Build gate passed" is free text in a PHASE_DONE note. |
| Manifest §2 commit ranges | Mislabels C3/C4, lists pre-boundary commits (10-04/05), omits most in-range work. Ignore §2 and §4 of the manifest for scope. |
| `83cb016` in range | Orphan, not an ancestor of HEAD. Out of scope. |
| `latest.patch` as the scope source | Stale: its `EquipCharPreview.cpp` hunk is committed in `22b56b4`. Use the dirty set. |

## 3. Inventory

### 3.1 Committed: 61 commits (`11e1284..HEAD`), by theme

All times EDT. "Gate" = a recorded `BUILD_GATE` PASS covering that mod at or after the commit; blank means none recorded.

| # | Theme | Commits | Size | Review notes |
|---|---|---|---|---|
| A | Mouse probe 0.4.0 + hotbar/queue visuals (10-06 21:13-21:36) | `48c9298` `b33db1b` `20eafed` `95945d4` | 7+4+1+1 files, +299 | First commits after the boundary. `mouse_probe_src` camera delta sanitizing + double-click synthesis; dynamic user32 resolve for hot-swap. Hotbar/queue pill colors, 58px icons. |
| B | Doc 22 compat tooling U1-U12 (10-07 13:18-10-08 00:02) | `0b91725` `a91872e` `40b47d2` `f633a1f` `8d17773` `3de985d` `b756482` `ea85883` `f1d0a03` `e5d32e1` `ecd0e33` `a734895` + 8 plan-status docs commits | ~5,900 script lines | New `compat_check.py` (1,187), `compat_graph.py` (882), `abi_check.py` (401), tests; `run_tests.py` +234/-29; `K2Addr.h` GOG-only scoping (`ecd0e33`, 370 lines). **Gate-critical.** |
| C | Plan tooling | `8395275` `392f13a` `4369884` | 3 commits, +400 | `plan_manager.py` +293/-27 (handoff isolation, auto-generated handoffs, plan stems). |
| D | Level-up Phase 7 | `db49b1d` `d8438c6` `b37fcb9` `39144b7` `abf390a` `0fd1151` `2667e71` `c70fc1c` | `d8438c6` = 39 files | `d8438c6` is a mass include-style change, 25 files in `research/.../parity/stub/_shared` + 10 parity tools. `39144b7` Party.h, `0fd1151` FileWatch.h (7 files, -43). Gate: `level_up_override_src` PASS only at `0a5efd9`, before `39144b7`/`0fd1151`. |
| E | Agent tooling and docs | `f036a63` `3d1c2b9` `13c6ca9` `b5b5aba` `8d6b663` `1757da0` `902b770` | docs-heavy; `scaffold_mod.py` +270, `subagent_prompt.py` +234 | Doc 36 (+402). Low code risk; `subagent_prompt.py` also has +264/-55 uncommitted on top. |
| F | **NPC diversity dispatcher** | `57837e0` | **121 files**, +2,173 | 75 `.tpc`, 14 `.mdx`, 13 `.mdl`, 2 `.2da`, 1 `.tga` assets. Code: `NpcDiversityDispatcher.cpp` (467), `DiversityLogic.h`, `DiversityRows.h`, `diversity_logic_test.cpp`, `common_contract_test.cpp`; Steam + GOG hooks; `generate_diversity_2da.py` (692), `normalize_diversity_textures.py` (217). **No gate row ever names `npc_diversity_src`.** |
| G | Common/provider batch (10-08 18:15-18:20, 8 commits in 5 min) | `c5c67f8` `15c9bfd` `7daaec6` `f19df9a` `ec95c98` `776de4c` `7aeb296` `83354c0` | ~2,100 lines | `GuiInput.h` + `gui_input_src` provider (675, new hooks); `hud_host` k2.hud@1 (GOG hooks); `save_events` 5 detours; `action_hotbar` migration (-212, test file -31 lines); doc 33 polish across queue/combat/selfheal (+403); perf-lab 0.6.0; `83354c0` "Fix gate failures" (§6 R1). Batch-committed, so per-commit attribution is weak. |
| H | Adoption/fix batch (10-08 20:20-20:26, 8 commits in 6 min) | `39ee196` `acf927a` `7159269` `b37dc39` `a6e7e08` `f32c8dc` `0a5efd9` `7e72e02` | ~370 lines | KpLog/GameAddr/guiinput adoption; `ranged_ai` k2.rangedai@1 (+286, 5 files); shield-bar one-shot warning; perf-lab smoke path. Gate PASS at `0a5efd9` for 6 mods. |
| I | Equip3D fix | `22b56b4` (10-09 10:07) | 2 files, +54/-4 | `EquipCharPreview.cpp`: destroy creature on character change, anim fallback, syncScreen. Gate PASS twice (14:07, 14:11Z). |

### 3.2 Uncommitted: 247 dirty files, split by mtime against the boundary

- **84 after the boundary** (Gemini-era; the review set). Listed below.
- **77 at or before** 2026-10-07T01:05:42Z (not in scope): old untracked scripts `01_*.py`..`verify_all_compatch.py` (09-24/25), feedback/issue-draft docs, `steam_db_pr3/work/*.csv`, `swkotor2.ini`, `neocities_mods*.json`, `crash_triage.py` (10-05), etc.
- 88 more are `tests/.cache` build copies (42) and Ghidra dump excerpts under `mods678_research/dump_excerpts` (46): skip.

**Post-boundary dirty code and tooling (modified files):**

| File | +/- | Mtime | Change (verified from diff) |
|---|---|---|---|
| `companion_selfheal_src/CompanionSelfHeal.cpp` | +142/-29 | 10-08 23:44 | Member 0 (leader) health button, `useNow()` `mi==0`, leader widgets |
| `level_up_override_src/LevelUpOverride.cpp` | +89/-76 | 10-09 10:21 | KpLog port (`KPatchInit`, 8 `KP_CHANNEL`s, fallback file), version 1.5.0-dev -> **1.6.0-dev**, local log helpers removed |
| `level_up_override_src/manifest.toml` | +1/-1 | 10-09 10:15 | version bump **and `cxxflags` now contains `-I$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common`** (machine-specific path, build portability) |
| `action_hotbar_src/ActionHotbar.cpp` | +58/-10 | 10-08 23:15 | AH3 fast-travel profile fix |
| `action_hotbar_src/HotbarLogic.h` | +3/-2 | 10-08 23:09 | `isExile` sentinel range -> `0xFFFFFFF0..0xFFFFFFFE` |
| `action_hotbar_src/hotbar_logic_test.cpp` | +7/-0 | 10-08 23:08 | AH3 tests; one expectation `characterKeyFor(0xFFFFFFFE,1)=="objfffffffe"` removed (verify replaced) |
| `queue_hud_src/kotor2-steam-aspyr.hooks.toml` | +19/-0 | 10-09 10:17 | **New detour `QueueHudPerformCastSpell` @ `0x00775703`**, `original_bytes [83 EC 0C 89 4D F4]`, `consumed_exit_address 0x0077574C`, `exclude_from_restore=["eax"]`, params ecx, ebp |
| `queue_hud_src/QueueHud.cpp` | +6/-6 | 10-09 10:18 | Cancel button color/alpha (QH5). Detour handler `QueueHudPerformCastSpell` is declared in `exports.def` (+1); confirm a definition exists. |
| `queue_hud_src/exports.def`, `level_up_override_src/exports.def` | +1 each | 10-09 | `QueueHudPerformCastSpell`, `KPatchInit` |
| `level_up_override_src/test.toml` | +1/-1 | 10-09 10:15 | not inspected line by line |
| `_shared_src/GameAddr.h` | +1 | 10-08 23:45 | `PerformCastSpell = 0x00775700` |
| `_shared_src/SelfHealUi.h` | +5 | 10-09 10:06 | leader button support |
| `inventory_3d_viewport_src/Inventory3DViewport.cpp` | +3/-1 | 10-08 23:16 | `syncScreen()` |
| `tests/static/subclassers.toml` | +13/-4 | 10-08 01:30 | `install_hint`/`restore_hint` line numbers bumped for 2 entries, new `gui_input_src/GuiInputProvider.cpp` WindowHook entry. Allowlist edit (R1). |
| `scripts/subagent_prompt.py` | +264/-55 | 10-08 17:01 | prompt builder rewrite |
| `scripts/plan_manager.py`, `test_plan_manager.py` | +19/-7, +18/-3 | 10-08 | removes 3 `assertIn` lines, replaces with new ones (verify equivalent) |
| `scripts/pre_install_gate.py` | +19/-0 | 10-07 14:28 | **adds `master_ledger` logging of BUILD_GATE PASS/FAIL (uncommitted)** |
| `scripts/run_tests.py`, `test_run_tests_select.py` | +12/-1, +14 | 10-08 | selection tweaks |
| `scripts/query_ghidra.py` | +27 | 10-07 20:49 | |
| `scripts/kotor2_logs.py` (+test), `selfheal_log_check.py` (+test), `kpm_hotswap.py`, `agent_contract.py` | small | 10-07/08 | |

**Post-boundary dirty, untracked (new) code/tooling:** `scripts/master_ledger.py` (19 KB), `agent_diff_check.py`, `clef_matcher.py` (17.7 KB), `clef_dataset.py`, `shadow_replay.py`, `gpu_mode.py`, `test_master_ledger.py`, `test_clef_matcher.py`, `test_subagent_tools.py`, `patch_manager_mods/gui_input_src/kotor2-gog-aspyr.hooks.toml` (192 B), `tests/benchmarks/transcript_prompt_eval.json`, `.claude/clef_priors.json`, `.claude/kev_4b_benchmark.json`, `.claude/contract_baseline.tsv`, `.claude/master_ledger*.jsonl`.
**Docs/instructions (context only):** `AGENTS.md` (+15/-4), `CLAUDE.md` (+5/-3), doc 33 (+71/-6), docs 10/13/18/19b/27/30, handoffs (`PHASE7_COMMON_MASTER`, `31_combat_action_hotbar`, `session_handoff`).
**Test pollution to delete, not review:** 11 files: 10 `.claude/handoffs/tmp*_plan.handoff.md` plus `99_temp_autoresolve_test_plan.handoff.md` (created by `test_plan_manager` runs), two 0-byte `kotor2_ghidra_knowledge.db` files (repo root and `scripts/`, untracked).

## 4. Agent windows (ledger, UTC)

The model is not recorded. Windows only.

| Agent | Window | Recorded work |
|---|---|---|
| `ethansm` | 10-07 18:30 - 10-08 19:34 | 192 rows: 88 CONTRACT_CHECK, 25 BUILD_GATE, 21 PHASE_DONE (doc 22 U5-U12, Phase 7 C1/C3, tooling) |
| `opencode:*`, `opencode`, `heavy-coder` | 10-08 05:04 - 10-09 03:14 | 19 OPENCODE_RUN rows (review x5, code x4, gate, test, scout, verify) + 4 BUILD_GATE (real FAILs at `7aeb296`, `83354c0`) |
| `3e24a0d8` | 10-08 19:41 - 21:13 | 12 BUILD_GATE (all `py:x`), 32 CONTRACT_CHECK, 7 CLEF_ROUTE, 1 SUBAGENT_DIFF_CHECK (`npc_diversity_src`, 19 files, PASS). Never wrote an edit event. |
| `47022c21` | 10-08 19:54 - 21:15 | PHASE_DONE `34:Phase 5` x2 (empty commit), MOD_INSTALL md5 `a6cd5c64` |
| `7b3e34d8`, `f13fd429`, `98d6ddca` | 10-08 21:23 - 10-09 00:26 | PHASE_DONE CS3/CS8 (`174984a`), CS4/CS5 (`174984a`), CS9 (`f645863`); `98d6ddca` BUILD_GATE FAIL then PASS (portrait-slots fix `0a5efd9`) |
| `4f650173`, `53e48070` | 10-09 02:13 / 03:40 | PHASE7 C5 (`39144b7`), C6 (`0fd1151`) |
| `2ed719b0` | 10-09 03:03 - 03:16 | PHASE_DONE `AH3` (empty commit) |
| `8e6ee335` | 10-09 03:45 - 14:15 | 4 BUILD_GATE (3 PASS, 1 FAIL at HEAD), HOTSWAP_APPLY, SUBAGENT_DIFF_CHECK FAIL on `latest.patch` |
| `2e8493b4` | 10-09 02:47 - 14:13 | PHASE_DONE `SH1` `SH2` `SH3` `QH5` (all empty commit: **marked done, still uncommitted**) |
| `4313c8dd`, `585eed46`, `8ebd8c6c`, `61079956`, `eeea4b78`, `4e61e9df` | 10-08 20:02 - 10-09 03:17 | CLEF_ROUTE / SESSION_HANDOFF / SUBAGENT_DIFF_CHECK only |

Phase-done records with an empty commit field and still-dirty code: `AH3`, `SH1`, `SH2`, `SH3`, `QH5`. Treat those as "claimed complete, not committed".

## 5. Gate and test evidence

### 5.1 The `py:x` rows are fixture leakage (verified)
- `scripts/test_pre_install_gate.py:148` — fake runner returns `(1, "FAIL   py:x  0.1s\n")` for the `Ordering` tests, which call `g.gate([...], runner=runner)`.
- `scripts/pre_install_gate.py` (dirty +19, mtime 10-07 14:28) now calls `ml.log_event("BUILD_GATE", ...)` in `gate()` on both paths. The unit tests therefore wrote FAILs to the production ledger.
- `scripts/master_ledger.py:180` has a "Test Isolation Guard" (untracked file, added later) and `.claude/master_ledger.test_archive.jsonl` holds 438 archived rows incl. 12 `BUILD_GATE FAIL` rows. Cleanup was started but the live ledger still holds 36 `py:x` rows.
- Result: of 47 BUILD_GATE rows, 36 are fixture, 7 are real FAIL, 4 are PASS.

### 5.2 Real gate results (all 11 non-fixture rows)

| UTC | Agent | Head | Result |
|---|---|---|---|
| 10-07 22:23 | ethansm | `e188b49` | FAIL default: `check-exports`, `check-raw-addresses` |
| 10-08 22:18 | opencode | `7aeb296` | FAIL default: `kpm-validate-patches`, `check-exports`, ... |
| 10-08 22:21 | opencode | `83354c0` | FAIL default: `check-exports`, `kpm-validate-patches` |
| 10-08 22:51 | opencode | `83354c0` | FAIL default: `kpm-validate-patches` |
| 10-08 22:54 | opencode | `83354c0` | FAIL wine: `perflab2[run]` exit 1 |
| 10-09 00:23 | 98d6ddca | `f32c8dc` | FAIL wine: `portraitslots` build failed |
| 10-09 00:25 | 98d6ddca | `0a5efd9` | **PASS** buff_duration_hud, inventory_3d_viewport, level_up_override, portrait_slots, ranged_ai, shield_bar |
| 10-09 03:45 | 8e6ee335 | `2667e71` | **PASS** equipment_3d_character |
| 10-09 14:07 | 8e6ee335 | `22b56b4` | **PASS** equipment_3d_character |
| 10-09 14:09 | 8e6ee335 | `22b56b4` | FAIL default: `check-shared-headers` (no mods) |
| 10-09 14:11 | 8e6ee335 | `22b56b4` | **PASS** equipment_3d_character |

Not recorded: whether `check-exports`, `check-raw-addresses`, `kpm-validate-patches` and `perflab2` were re-run green after `83354c0`. The 14:09 `check-shared-headers` FAIL at HEAD is unresolved in the record.

### 5.3 Gate coverage of mods changed after the boundary

Covered by a PASS (but at an older head than later edits where noted): `buff_duration_hud`, `inventory_3d_viewport`, `portrait_slots`, `ranged_ai`, `shield_bar`, `level_up_override` (PASS predates `39144b7` and `0fd1151`), `equipment_3d_character`.
**No recorded PASS:** `npc_diversity`, `action_hotbar`, `gui_input`, `hud_host`, `save_events`, `perf_lab`, `queue_hud`, `combat_hotbar`, `mouse_probe`, `active_effects_panel`, `companion_selfheal`.
Caveat: ledger BUILD_GATE logging only exists from the uncommitted 10-07 14:28 edit, and plain `run_tests.py` runs are not logged. Absence of a row is not proof of no test, only of no recorded gate.

## 6. Risk hot spots, ranked

**R1 — Gate and allowlist integrity (review first).** Same agents edited the checks and the code they check.
- `scripts/run_tests.py` (+234 committed, +12 dirty), `pre_install_gate.py` (+19 dirty), `plan_manager.py` (+293), `compat_check.py` (new), `master_ledger.py`, `agent_diff_check.py` (both untracked, not under version control).
- `83354c0` "Fix gate failures": `npc_diversity_src/manifest.toml` `kind = "mod"` -> `"patch"`, and a 1-line `tests/static/raw_addresses.toml` change. Confirm it is a legitimate fix and not an allowlist edit to silence `check-raw-addresses`. `AGENTS.md` rule: no raw addresses without a prior `query_ghidra.py address <hex>` lookup. Review the raw-addresses entry under that rule; this report did not read it.
- `tests/static/subclassers.toml` dirty edit (hint line numbers + new entry): allowlist change, verify against source.
- `AGENTS.md`/`CLAUDE.md` edits that rewrite the gate rules (Gate Invariant, scoped tests).

**R2 — New and changed hooks (byte-level correctness).** Verify with `scripts/check_hook_bytes.py`, `check_hook_spans.py`, `query_ghidra.py address <hex>`, Steam vs GOG parity.
- Uncommitted: `QueueHudPerformCastSpell` @ `0x00775703` (6 stolen bytes, `consumed_exit_address 0x0077574C`, `exclude_from_restore eax`). Needs relative-branch check in stolen window, stack-frame assumption (`EBP` as frame, 7 stack args / `RET 0x1C`), and a handler definition in `QueueHud.cpp`.
- Committed: `npc_diversity` (Steam + GOG), `gui_input` (Steam committed; **GOG file untracked, 192 B**), `hud_host` GOG, `action_hotbar`, `perf_lab` hooks + `code_patches.toml`, `queue_hud`, `save_events` (5 detours). `skip_original_bytes = false` lines added/moved in 5 hooks files.

**R3 — Behaviour changes with weakened or replaced tests.**
- `hotbar_logic_test.cpp` committed -31/+10: `normalizeSaveFolder` assertions removed; `se_registry_test.cpp` has 22 `normalizeSaveFolder` references, so migration is plausible; confirm each case (SAVES: prefix, case, `/` and `\\` separators, empty, too-short, no-name, non-digit) survived.
- `queue_hud_logic_test.cpp` +265/-20: three `KT_CHECK` lines removed (`shouldReInject` timings 1500/4500, `stripCancelRect` 3840 case); verify replaced, not dropped.
- Parity tools in `research/.../parity/*_test.cpp` lost assertion macros in the `d8438c6` include refactor (`CHECK` macro redefinitions, `apply_test`, `ui_test`); confirm counts still match pre-refactor.
- Dirty AH3: `isExile` now true for the whole `0xFFFFFFF0..0xFFFFFFFE` range and for `partyCount==1 || isMember0 || partySlot==0`. Wide predicate, can mis-key a normal party. Needs boundary tests.

**R4 — Uncommitted feature work** claimed done: `CompanionSelfHeal.cpp` leader button (+142), `LevelUpOverride.cpp` 1.6.0-dev KpLog port, QH5 colors. Includes `manifest.toml` with a `$HOME` machine path in `cxxflags`.

**R5 — NPC diversity dispatcher** (`57837e0` + `83354c0`): 121 files, never gate-recorded. `NpcDiversity.kpatch` live copy md5 `c227395c` differs from the md5 `a6cd5c64` in MOD_INSTALL; file rewritten 10-08 18:23 EDT after the install record at 17:15 EDT (preflight U1). `Phase 5` DONE logged twice with no commit. Binary assets (tpc/mdl/mdx) need a manifest/size/provenance check, not a line review.

**R6 — Mass refactors:** `d8438c6` (39 files), `ec95c98` hotbar migration (-212), `15c9bfd`/`c5c67f8` GuiInput. Check that behaviour-preserving claims hold; `action_hotbar` has no recorded gate.

**R7 — Process hygiene (low code risk):** 11 temp handoffs, 0-byte `.db` files, `.claude/master_ledger*` untracked, mixed doc/code commits, batch commits (8 in 5 min, 8 in 6 min).

## 7. Not done / unverified

- No builds, tests, or gate scripts run. Every "passes" above is a ledger fact, not a result from this session.
- Model identity of any agent: unknown (§1).
- Whether the `check-exports`, `kpm-validate-patches`, `perflab2` failures were later fixed: not in the ledger.
- Cause of `latest.patch` FAIL verdict (ledger row has no reason field).
- Whether `QueueHudPerformCastSpell` has a body in `QueueHud.cpp` (only the export and hook entry were seen in the diff).
- `NpcDiversity.kpatch` provenance (preflight U1) and `9d3fba7` (KPM repo hash) remain open.

## 8. Suggested work packages (planner decides)

| WP | Scope | First commands |
|---|---|---|
| 1 Gate integrity | `scripts/{run_tests,pre_install_gate,plan_manager,compat_check,compat_graph,abi_check,master_ledger,agent_diff_check}.py`, `tests/static/{raw_addresses,subclassers}.toml`, `83354c0` | `git diff 11e1284 HEAD -- scripts/run_tests.py`, `python3 scripts/run_tests.py -k pre_install_gate` |
| 2 Uncommitted hooks and shared headers | `queue_hud_src/`, `_shared_src/{GameAddr,SelfHealUi}.h`, `gui_input_src/*.hooks.toml` | `python3 scripts/check_hook_bytes.py`, `check_hook_spans.py`, `query_ghidra.py address 0x00775703` |
| 3 Uncommitted mod behaviour | `action_hotbar_src`, `companion_selfheal_src`, `level_up_override_src`, `inventory_3d_viewport_src` | `run_tests.py -k hotbar`, `-k selfheal`, `-k levelup` |
| 4 Provider batch | `gui_input_src`, `hud_host_src`, `save_events_src`, `perf_lab_src`, `action_hotbar_src` migration (`c5c67f8..7aeb296`) | `build_check.py --mod <each>`, `run_tests.py -k saveevents`, `-k guiinput` |
| 5 NPC diversity | `npc_diversity_src`, `generate_diversity_2da.py`, `normalize_diversity_textures.py` | `build_check.py --mod npc_diversity_src`, `run_tests.py -k diversity`, `md5sum` of live vs repo kpatch |
| 6 Test-weakening audit | all `*_test.cpp` and `test_*.py` in range | `git diff 11e1284 HEAD -U0 -- '*test*' \| grep '^-'` |
| 7 Small adoption batch + mouse probe | themes A, H, I | `run_tests.py -k mouse`, `-k ranged`, `-k shield` |

Run Packages 1-2 before trusting any gate output from Packages 3-7. Use `agent_contract.py snapshot` before and `check --scope <globs>` after each reviewer launch, per `CLAUDE.md`.
