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

### State after Phases 2-3 (10-03)
- Phase 2 DONE, lead-reviewed, committed (nothing pushed):
  - `gameapi/match-engine-signatures` (worktree tools/kpm-pr-gameapi): c9d1aba, ec8b268, b3c6cca, f19550c (7 rows,
    R1 Commit 1). validate 0 errors; publish-patches windows_x86 25/25. Lead re-checked Gob::EnableRenderBumpedOut
    (Steam `fld [ebp+0xc]` 0x45C45C, `ret 8` 0x45C48C); no callers outside GameAPI.
  - `port/k2-expandedkeyboard-steam` (tools/kpm-pr-ekc): c90fd4f, 1.19.0. Lead checked GOG/Steam config field order,
    constant deltas (+0x22E880 action-bar fns, -0x4FE480 GUI manager fns), prologues. `.kpatch` for the user's test:
    `upstream_prs/ekc_steam/build/ExpandedKeyboardControl.kpatch` (sha 7197ec83...).
    It conflicts with ForcePowerHotbar (both detour 0x72CB63/0x74B3F0).
  - `port/k2-acc-steam` (tools/kpm-pr-acc): acdfeeb, 1.5.0. Hook bytes re-read by lead. `.kpatch` in acc_parity/build/.
- Phase 3 DONE: drafts in `upstream_prs/drafts/` (pr_gameapi, pr_ekc, pr_acc, issue_gog_db_missing_names, pr_db_note,
  followups_291_294). Nothing posted.
- 10-03 evening: #292/#293/#294 MERGED (16:14Z). #291 has 3 Lane review comments; license fix applied by user
  (4e17f12); local commit 31dd299 (external patches + hooks link) on docs/contributing-guide, NOT pushed;
  reply draft in drafts/followups_291_294.md.
- Phase 4 (10-03): user approved ONLY the #291 follow-up -> pushed 31dd299, reply posted (issuecomment-5973037915).
  runscript decision: issue only, let Lane choose; fallback documented in upstream_prs/runscript_classes/FALLBACK.md.
  NOT approved yet: GameAPI PR, pushing EKC/ACC, filing the issue.
- NEXT (10-04 plan-handoff): EKC Steam in-game test. EKC collides with live force-power-hotbar at 0x72CB63/0x74B3F0;
  install by scripted patch_config edit (no full KPatchLauncher apply). See .claude/handoffs/24_master_plan_status.handoff.md.
- Remaining user gates: push the 3 branches; open GameAPI PR; EKC PR after the in-game test; ACC PR after the DB
  PR; file the issue; runscript/classes decision.

## Next phase, 2026-10-04 (plan `upstream_prs/NEXT_PHASE_PLAN_2026-10-04.md`; no in-game testing, user deferred the EKC test)
Upstream master = 6374f73: #291 (contributing guide) MERGED, plus #296, #297, #289, #306.

Branches (all committed locally, NOT pushed; worktrees under `~/Games/KOTOR2_Mod_Workspace/tools/`):
| branch | worktree | head | base | kpatch (untested) | PR draft |
|---|---|---|---|---|---|
| port/k2-classmenuclothes-steam | kpm-pr-cmc | 746265f | 6374f73 | upstream_prs/cmc_steam/build/K2ClassMenuClothes.kpatch | drafts/pr_cmc.md |
| port/movie-patch-k2-steam | kpm-pr-movie | 8cc2647 | 6374f73 | upstream_prs/movie_steam/build/Movie Patch.kpatch | drafts/pr_movie.md |
| gameapi/k2-title-gated-signatures | kpm-pr-gameapi2 | 5a7821f | 6374f73 | (library; publish 25/25 OK) | drafts/pr_gameapi_k2.md |
| port/k2-expandedkeyboard-steam | kpm-pr-ekc | c90fd4f | 8b33840 (merges clean into 6374f73) | upstream_prs/ekc_steam/build/ | drafts/pr_ekc.md |
| port/k2-acc-steam | kpm-pr-acc | acdfeeb | 8b33840 (merges clean) | upstream_prs/acc_parity/build/ | drafts/pr_acc.md |
| gameapi/match-engine-signatures | kpm-pr-gameapi | f19550c | 8b33840 (merges clean) | - | drafts/pr_gameapi.md |
Rebase the 8b33840-based branches onto upstream/master before opening their PRs.

Research done (commit d841fae): R-A EKC x Action Hotbar = option (c), move hotbar hooks to 0x72CB84 / 0x74B3F5 (backlog U4, doc 07);
R-B EKC GOG semantics 6/6 PASS; R-C PartyRosterLimit32K2 conditional GO (211/211 hooks same; its partyselect_p.gui would
overwrite our ultrawide override, needs a 32-slot ultrawide regen); R-D GameAPI follow-up (K2 ShutDownGlobal DB rows wrong:
real GOG 0x4D8230 / Steam 0x7C7B40, a DB fix for upstream + the DB session).
Tools: `scripts/verify_steam_hooks.py` (all worktrees 0 FAIL; allowlist in upstream_prs/verify_steam_hooks/), `scripts/disasm_va.py`
(offline GOG disassembly from `ghidra_dumps/upstream_server/k2_win_gog_aspyr_swkotor2.exe/memblocks/`).
Pending user approvals: push which branches; open which PRs (GameAPI K2 needs no in-game test); file the runscript issue.
Untested in game: EKC, ACC, CMC, Movie kpatches.
