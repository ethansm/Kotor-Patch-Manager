# Upstream PR series: session context 2026-10-03 (READ FIRST when resuming PR-series work)

The Steam address-DB work moved to `patch_manager_mods/steam_db_batches/`. For anything DB-related,
read `steam_db_batches/SESSION_CONTEXT_2026-10-03b.md` (newest) and `steam_db_batches/README.md`.
This file covers the PR series as a whole and this session's decisions that live nowhere else.

## PR state (checked 10-03 11:40 against the fork and upstream)

| Upstream PR | Branch (fork ethansm/Kotor-Patch-Manager) | Head | Notes |
| --- | --- | --- | --- |
| #291 docs: add contributing guide (#56) | `docs/contributing-guide` | 8dfd625 | Edited and re-committed after this session's 8f5ffe0. Worktree `tools/kpm-pr-contributing`. |
| #292 K2AspyrLoadingScreenLineFix: Support the Steam Windows build | `port/k2-loadscreen-linefix-steam` | 0bdffed | One PR per patch (the combined `patches/k2-steam-ports` was split). |
| #293 Post-Combat Movement Fix: Support the Steam Windows build | `port/k2-postcombat-steam` | 4dcf49b | |
| #294 K2AspyrMapAspectFix: Support the Steam Windows build | `port/k2-mapaspect-steam` | 746471c | Worktree `tools/kpm-pr-steam-ports` is on this branch. |
| (not opened) Steam address DB | `addressdb/k2-steam-acc` | 4324098 | 1,683 functions after the B0 re-audit. Worktree `tools/kpm-pr-steam-db`. Owned by steam_db_batches. |
| (planned) AdditionalConsoleCommands GOG/Steam parity | - | - | Own PR (user decision 10-02). Needs the DB rows; its Steam hooks TOML exists on `ethansm/local-mods`. |
| (planned) GameAPI wrapper signature fixes | - | - | See "Decisions" below. |

Clones: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager` (origin = fork, upstream = LaneDibello with push
disabled; work branch `ethansm/local-mods` has uncommitted LevelUpOverride/RobustnessFixes edits, so don't touch it).
PR worktrees are siblings under `tools/`.

## Live game state

- Game folder `addresses.db` sha 364d29c4 = the original 48-function upstream DB. The PR3 swap from 10-02
  (sha 1117ecb5) is no longer live. Backup `addresses.db.pre_pr3_20261002` still exists. A KPM re-apply also
  re-stages the 48-function DB.
- The in-game console checklist (steam_db_pr3/README.md "Live test") was never reported as run. If it's re-run,
  build the live DB from the current branch head, not 4bb2736.
- Only live patch that reads the DB: AdditionalConsoleCommands.

## Decisions this session (user)

1. CONTRIBUTING.md: concise and generic, no per-author audit; links to the README for field reference.
2. AdditionalConsoleCommands is not part of the Steam-ports PR; it gets its own GOG/Steam parity PR.
3. DB scope: demand-driven, "fill in as we go", not full GOG parity (later sessions widened this via batches).
4. No GOG exe on this machine. Since then the `openkotor-ghidra-server` skill exists (shared Ghidra server with
   GOG/Steam/Mac binaries), and steam_db_batches already uses it.
5. Every shipped DB row needs at least a Haiku-level model verification (memory `feedback-model-verify-db`).
6. Ship verified + likely rows to the live test and PR3 (434 rows at the time).
7. GameAPI wrapper bugs stay OUT of PR3. Plan: separate PR `GameAPI: Match wrapper signatures to the engine`
   after a research pass:
   - `CGuiInGame::ShowUpgradeScreen`: the typedef pushes 4 bytes, the function pops 24 (GOG 20). Find the 5
     missing args.
   - `CSWCPlaceable::ActionMenuSecurity`: the typedef pushes 4, the function pops 8 (GOG says 0).
   - Plus the ~21 `wrapper_issue` rows in `verified.tsv` (most are harmless width differences).
   - Check K1 and the GOG build too (shared code).
   - Nothing calls the two broken wrappers today. Note them in the PR3 description and open an issue.

## Findings worth keeping

- Upstream commit style is `Area: Do the thing`, not Conventional Commits (PR bodies may carry `feat(...)`). Adding
  a Steam target bumps the minor version (#257).
- The Antigravity PR drafts had wrong claims: exports.def "required" (create-patch.py generates it), "679 tested",
  "tested on Windows", "addresses #4" (#4 = flesh out the Steam address library). Verify every drafted claim on disk.
- GameAPI init: each wrapper class resolves its addresses in one `try`. The first missing name aborts the rest of
  that class; methods null-check, so they fail silently rather than crash.
- Upstream bug candidate: the GOG DB lacks names GameAPI inits need (CClientExoApp::IsValidResolution,
  GetLoadingSaveGame, SetRunScript, CSWSObject::SetTag, CClientExoAppInternal/CSWSCreature script offsets), so
  AdditionalConsoleCommands is partly degraded on GOG too. Issue not yet filed.
- Haiku pilot (`haiku_eval_plan.md` bottom, `work/haiku/`): single-pass Haiku accepted 13 of 40 decoys and raised 82
  false flags (4-byte slot widening, thunks Ghidra prints without `__thiscall`, library-signature labels, decimal
  offsets read as hex). Use Haiku as a flagger feeding Sonnet, not as a confirmer. steam_db_batches later reached
  0 of 140 decoys accepted with Sonnet cards + Opus gate.
- PR2 evidence: all 7 hook original_bytes match vanilla 6A522E71. Post-Combat `[ebp-0x48]`/`[ebp-0x50]` = input
  getter 0x72F9D0 axes 7/8 stored at 0x7AE871/0x7AE883. MapAspectFix frame offsets are in `hooks_ported`.

## Open items (PR series)

- Watch #291-#294 for review comments.
- Open the DB PR when steam_db_batches says it's ready. The description should note the wrapper bugs (decision 7).
- AdditionalConsoleCommands parity PR after the DB PR.
- File the GOG-DB missing-names issue.
- Research + PR for the wrapper signatures (Sonnet subagent, static only).
- Remaining GOG-only patches for Steam ports: ExpandedKeyboardControl (6 hooks, no DB lookups, best next
  candidate), PartyRosterLimit32K2 (211 hooks), K2ClassMenuClothes and Movie Patch (check how they target GOG).
  Skip ShaderDebug.

## Next phase (plan "next phase of the upstream KPM PR series", started 10-03 afternoon)

Outputs live in `patch_manager_mods/upstream_prs/` (gameapi_signatures, ekc_steam, gog_only_triage, acc_parity, drafts).

Phase 0 (refresh):
- #291-#294: still OPEN, 0 comments, 0 reviews.
- upstream/master moved 55725f7 -> 8b33840 (#288, #304: Referenced Model Stealth Fix only). No overlap with our branches.
- DB PR not ready: steam_db_batches is consolidating B15 (branch head still 4324098, 1,683 functions).
- Phase 1 research inputs snapshotted to the session scratchpad: upstream/master export, Steam vanilla exe (6a522e71),
  branch DB copy at 4324098.
