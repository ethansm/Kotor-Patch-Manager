# Preflight report: `.opencode/plans/pending-agent-review.md`

Read-only. Nothing under `scripts/` was run. No builds or tests. Excluded files were not opened.
HEAD: `22b56b4` on branch `KOTOR-II`. Outputs: this file and `.claude/review/scope.json`.

## Verified (command that proved it)

- **V1** HEAD is `22b56b4`. Ledger head `b5b5aba39c2e` and `0fd1151` are ancestors of HEAD. `0fd1151` is 2 commits behind. (`git rev-parse HEAD`, `git merge-base --is-ancestor`)
- **V2** Every commit hash in manifest §2 and §4 exists in the repo except `9d3fba7`, which the manifest calls a KPM hash. (`git cat-file -e`)
- **V3** The ledger parses as JSON on every line (`jq -c .` exit 0). All 340 rows carry an `agent` key, and every event type has agent values. (`jq`)
- **V4** `3e24a0d8` owns 12 BUILD_GATE rows, all FAIL, all `head: b5b5aba39c2e`. Lines 200-202, 211-213, 223-225, 236-239. (`jq`, `sed -n`)
- **V5** Line 203-204: worker CONTRACT_CHECK FAIL with fails 2 and 6, matching the manifest.
- **V6** Line 250: `OPENCODE_RUN` `opencode:gate`, PASS, parent `47022c21`, 382691 ms. Line 251: `4313c8dd` CLEF review, confidence 0.49. (`sed -n`)
- **V7** `47022c21` has 0 BUILD_GATE rows and 0 CONTRACT_CHECK rows. (`jq`)
- **V8** `latest.patch`: 1044 lines, 14 `diff --git` entries. (`wc -l`, `grep -c`)
- **V9** Scripts named in the manifest exist: `build_check.py`, `run_tests.py`, `agent_diff_check.py`, `check_kpm_updates.py`, `check_hook_spans.py`, `check_hook_bytes.py`, `pre_install_gate.sh`, `query_ghidra.py`, `agent_contract.py`. (`ls`)
- **V10** Every full path in manifest §5, §6 and §11.4 exists in the tree. The §3 directories exist. Bare basenames in §3 and §11.4 were matched by name only, which is a weak check: `manifest.toml` matches 34 tracked files and `test.toml` matches 29. `hooks.toml` matches none (see C9). (`git ls-files` + existence)
- **V11** `AGENTS.md` and `CLAUDE.md` are dirty (`M`). (`git status`)
- **V12** `EquipCharPreview.cpp` is clean. Its change is committed in `22b56b4`, and HEAD contains the `character change` string. (`git show HEAD:... | grep -c`)
- **V13** After both CLEF prompts ("Fix pointer math…", "Fix memory leak…"), no ledger event is an edit, diff or write. Only the existing event types follow. (`jq`, `sed -n` on lines 197-296)

## Contradicted (repo or ledger disagrees)

- **C1 Ledger size.** 340 lines, not 258 (`wc -l`). Event types also include `HOTSWAP_APPLY` (3) and `MOD_INSTALL` (1), which the manifest omits.
- **C2 `py:x` attribution.** 36 of 43 FAIL rows carry `py:x`. 24 are from `ethansm` (lines 12-185, 2026-10-07T22:22Z to 2026-10-08T17:41Z, heads `e188b4951f34`, `d0d6c7b1efea`, `db49b1dc306e`, `13c6ca969efc`, `b5b5aba39c2e`). 12 are from `3e24a0d8` (lines 200-239). It is not specific to `3e24a0d8`.
- **C3 "47022c21 passed its gates."** 0 BUILD_GATE rows for `47022c21`. 0 BUILD_GATE rows of any agent mention `npc_diversity_src`. The only gate evidence is line 250, an `OPENCODE_RUN` PASS with `parent_agent` `47022c21`. The "Build gate passed" text is in a PHASE_DONE `notes` field, not a gate record.
- **C4 "CLEF gate route passed" (§11.2).** The CLEF_ROUTE row at 19:54:12Z (`47022c21`) records a routing decision with `confidence` 0.98. It has no verdict and no completion field.
- **C5 `latest.patch` scope check.** The manifest cites ledger line 222 for `scope_violations=0`. Line 222 is `3e24a0d8`'s `npc_diversity_src` check (19 files, `patch_file` null). The `latest.patch` check is line 330: 2026-10-09T14:06:04Z, `antigravity:8e6ee335`, subagent `opencode:code`, `changed_count` 21, `scope_violations` 0, **verdict FAIL**.
- **C6 Counts.** `changed_count` 191-212 is wrong. The ledger's latest value is 21, and the patch has 14 entries. "190 / 188" dirty files is wrong. The actual count is 247 files (`git status --porcelain -uall`: 42 M, 205 ??), or 197 entries in default porcelain.
- **C7 Patch composition (§11.4).** The patch holds 10 code files (6 `.cpp`, 3 `.h`, 1 `.toml`) and 4 `.md` files (`AGENTS.md`, `PHASE7_COMMON_MASTER.handoff.md`, `session_handoff.md`, doc 33). The manifest's "14 source + 2 handoff + 2 docs" is wrong. Nine of the 13 `.cpp` files it lists are not in the patch: `InventorySortFilter`, `BuffDurationHUD`, `LoadHangProbe`, `PerfLab`, `EquipIconFix`, `MouseProbe`, `RangedAI`, `CompanionAI`, `LevelUpOverride`. `CLAUDE.md` is also absent from the patch.
- **C8 Patch author.** §11.4 says the patch was made by `opencode:test` or `opencode:code`. The ledger row (C5) names `antigravity:8e6ee335`.
- **C9 `hooks.toml`.** The manifest names `hooks.toml`. No file of that name exists. The real files are `kotor2-steam-aspyr.hooks.toml` and `kotor2-gog-aspyr.hooks.toml`.
- **C10 Section 2 scope.** 40 of the 50 unique hashes are outside "since 2026-10-06": 39 are dated 2026-10-04 or 10-05, and `83cb016` is an orphan (see C12). The manifest covers only 10 of the 67 in-range commits. 57 in-range commits are not listed.
- **C11 Label collisions (§2, §4).** `ba8e7d4` is one commit, "C6+CS7 T2". `f6cc2bd` is one commit, "CS11/CS12". Both are single commits, not two each. The manifest's "C3 = 83cb016, C4 = d8438c6" is wrong: `d8438c6` is "Phase 7 Chunk C3 include style", and `83cb016` is "Chunk C1 integration audit baseline". The two different "C4" labels are real: `fde5680`/`c9b54d2`/`8e23979`/`d581600` are CharEffectsPanel C4a-c, and the PHASE7 C4 evidence commit is `c70fc1c`, which the manifest omits.
- **C12 Orphan commit.** `83cb016` (dated 2026-10-08) is not an ancestor of HEAD and no ref contains it. The manifest presents it as in-range work.

## Unverifiable (needs a human or a later pass)

- **U1 `NpcDiversity.kpatch` (corrected).** It exists at `/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/NpcDiversity/NpcDiversity.kpatch`, 24451 bytes, mtime 2026-10-08 18:23 -0400. It is not in the repo tree, so my earlier search under `/home/ethansm/orca` was the wrong place. The folder also holds `NpcDiversity.kpatch.20261008_182234_pre.bak` (16:01). The file's md5 is `c227395c`. `MOD_INSTALL` (2026-10-08T21:15:28Z, `47022c21`) records md5 `a6cd5c64`. These differ, and the kpatch was rewritten at 18:23 local (22:23Z), after the install record was written at 17:15 local (21:15Z). The hash the ledger recorded is for an unknown artifact. Human check: `md5sum` of the live installed copy and of this file.
- **U2 Cause of `py:x`.** The ledger does not store the command. `scripts/run_tests.py:317` builds ids as `py:<rel path>`, so `py:x` is a test item whose path is `x`. No tracked file named `x` exists. Duration 0.1s fits an immediate failure, but no cause is established. Human check, in a scratch worktree: `git worktree add ../scratch-b5 b5b5aba39c2e && cd ../scratch-b5 && python3 scripts/run_tests.py --help`, then rerun the one gate command you use.
- **U3 `deep` route completion.** The `npc_diversity_src` CLEF route (19:47:39Z, predicted 141713 ms) has no completion event. Unverified.
- **U4 `0fd1151` as "handoff branch".** The manifest's label is not a branch name. Human check: `grep -l 0fd1151 .claude/handoffs/*.md`.
- **U5 `9d3fba7`.** A KPM-repo hash. Human check: `git -C <Kotor-Patch-Manager path> cat-file -e 9d3fba7`.
- **U6 `83cb016` origin.** Human check in a scratch clone: `git fsck --lost-found`, or `git reflog --all | grep 83cb016`.
- **U7 `latest.patch` FAIL reason.** The ledger row has no reason field, so the cause of the FAIL verdict is unknown.

## Corrected facts (use these in the audit)

- **HEAD:** `22b56b4` (2026-10-09 10:07 -0400). Ledger head: `b5b5aba39c2e`, an ancestor.
- **Dirty set:** 247 files: 42 M, 205 ??. By top dir: `patch_manager_mods` 110, `scripts` 46, `tests` 44, `.claude` 33, root 9, `.agents` 2, `research` 2, `.opencode` 1.
- **Classes:** code 74, tests 12, docs 116, generated_binary 45, excluded 0.
- **Ledger, 340 rows:** CONTRACT_CHECK 121, SESSION_HANDOFF 53, BUILD_GATE 47 (43 FAIL, 4 PASS), CLEF_ROUTE 39, PHASE_DONE 35, SUBAGENT_DIFF_CHECK 26, OPENCODE_RUN 15, HOTSWAP_APPLY 3, MOD_INSTALL 1.
- **Duplicates:** 27 distinct lines appear more than once (`sort | uniq -d`).
- **Agent field:** present on every row, for all 9 event types. So the answer to "which event types carry an agent" is all of them.
- **`3e24a0d8`:** 12 BUILD_GATE (FAIL), 32 CONTRACT_CHECK (4 bursts of 8), 7 CLEF_ROUTE with 4 decisions (`deep`, `ghidra`, `review`, `code`).
- **`47022c21`:** 1 CLEF_ROUTE (`gate`), 2 PHASE_DONE, 3 SESSION_HANDOFF, 1 MOD_INSTALL. 0 BUILD_GATE, 0 CONTRACT_CHECK.
- **BUILD_GATE heads:** `b5b5aba39c2e` 21 rows, `d0d6c7b1efea` 6, `e188b4951f34` 4, others 1-3 each. All exist.
- **HEAD gates (`22b56b4`):** 3 rows. 14:07:28Z PASS and 14:11:56Z PASS (`equipment_3d_character_src`). 14:09:31Z FAIL with no mods.
- **PASS rows (4):** `0a5efd9` (`98d6ddca`, 00:25:48Z), `2667e71` (`8e6ee335`), and two at `22b56b4` (`8e6ee335`).
- **Committed since 2026-10-06:** 67 commits by committer date. All 67 have author and committer `ethansm`. 4 carry `Co-Authored-By: Claude` trailers. In-range, the ledger attributes `0fd1151` to `53e48070` and `39144b7` to `4f650173`.
- **Note:** `git log --since` returned the same 67 for this range. It omits `83cb016` because that commit is an orphan.

## Scope recommendation

1. **Review the uncommitted set only, against HEAD `22b56b4`.** Committed work is already in HEAD and is not pending. The manifest's "pending" framing applies to the dirty set.
2. **Include:** code 51 (74 minus the 23 `.c` Ghidra dump excerpts under `patch_manager_mods/mods678_research/dump_excerpts/`), and tests 12. Use docs (116) as context only.
3. **Skip:** generated_binary 45 (42 files under `tests/.cache`, 2 `.db`, `latest.patch`), the 23 dump excerpts, and `83cb016`.
4. **Do not use `latest.patch` as the scope source.** It contains a hunk already committed in `22b56b4`, and its last verdict is FAIL. Use the dirty set.
5. **Committed-work evidence, if you widen scope:** the 4 Claude-trailer commits are `11e1284` (Sonnet 5.5), `6a9010d` (Sonnet 5.5), `2cfa805` (Opus 5.5), and `cfd03fa` (Opus 5.5), all dated 2026-10-06. Author and committer metadata cannot separate agent work from human work, so the trailers and ledger are the only evidence. `scope.json` lists `commits_in_scope` as empty.
6. **Use the counts in `scope.json`, not the manifest's counts.**
