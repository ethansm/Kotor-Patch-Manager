# Plan: KPM upstream series, next phases (research, automated testing and development; no manual testing)

## 0. Preserved context (post-clear memory)
- **Next-phase direction (user):** first "- The Expanded Keyboard Control test on Steam; tell me when you're not playing
  and I'll install it, after backing up patch_config." It was then REPLACED by: "I do not currently have time to manually
  test this right now, we should focus on additional research, investigation, testing and development for next phases".
  The EKC in-game test is deferred. Do not install anything into the game in this plan.
- **Backlog request (user, 10-04):** "future mod/feature to add to backlog, Expanded Keyboard Control support for our
  Action Hotbar (New name for Force Power Hotbar, rename in progress in another session)".
- **Repo:** KOTOR-II (`/home/ethansm/orca/workspaces/local-setup/KOTOR-II`). Outputs go in
  `patch_manager_mods/upstream_prs/<topic>/`. Resume file: `patch_manager_mods/steam_db_pr3/SESSION_CONTEXT_2026-10-03.md`.
  Handoff: `.claude/session_handoff.md` (EKC section at the bottom).
- **KPM state:**
  - #292-#294 MERGED.
  - #291 open: follow-up 31dd299 pushed, reply posted.
- **Branches committed locally, not pushed:**
  - `gameapi/match-engine-signatures` (worktree `tools/kpm-pr-gameapi`)
  - `port/k2-expandedkeyboard-steam` (`tools/kpm-pr-ekc`, c90fd4f)
  - `port/k2-acc-steam` (`tools/kpm-pr-acc`)
  - The tools dir is `~/Games/KOTOR2_Mod_Workspace/tools/`. upstream/master is 8b33840.
- **Evidence inputs:**
  - Steam vanilla exe: `"$G/swkotor2.exe.unpatched_vanilla"` (sha 6a522e71). Copy it to a scratch dir first; never write the game dir.
  - Steam dump: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`.
  - GOG and K1 binaries: `openkotor-ghidra-server` skill / `scripts/ghidra_server_api.py`, read-only.
  - Branch Steam DB (read-only): `tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db`.
  - Earlier research: `upstream_prs/{gameapi_signatures,ekc_steam,gog_only_triage,acc_parity}/`.
- **Do NOT:**
  - ❌ Install into or launch the game, edit `"$G/patch_config.toml"`, or touch `patches/`. The hotbar session is live there: `action-hotbar`, backup `backups/inst20_1004_1224`.
  - ❌ Touch `steam_db_batches/`, the KDB, the `kpm-pr-steam-db` worktree or any `AddressDatabases/*.db` (owned by the DB session).
  - ❌ Touch the `kotor-patch-manager` clone's working tree (uncommitted local-mods edits). Read branches with `git show`.
  - ❌ Push, open PRs or issues, or comment without explicit approval in that turn.
  - ❌ Use a mapper-only GOG→Steam function vote as truth. R3 found 2 wrong votes; the byte or content anchor decides.
  - ❌ Use `findBytes` for uniqueness. Use exact search over `.text` with context bytes.
  - ❌ Use agents for commits. The lead commits; agents never commit, push or write a DB.

## 1. Context
The upstream Steam-port series is moving: Lane merged three ports in one day. The user can't test in game right now, so
this phase pre-builds the next work that needs no human: the next two Steam ports, harder research for the large port
and the GameAPI follow-up, test tooling that catches hook-byte regressions, and the design research for the new backlog
feature (EKC and Action Hotbar coexisting). Every output is a reviewed commit or doc, ready for a later test-and-approve
pass.

## 2. Phase A (lead, inline, about 10 min): record and prepare
1. Backlog entry. Append to the hotbar doc's user backlog in `patch_manager_mods/07_force_power_hotbar.md` (append only;
   the other session is renaming the doc) and add one line to memory `kotor2-force-power-hotbar.md`:
   - EKC support for the Action Hotbar.
   - Both detour 0x72CB63 (action-bar key capture) and 0x74B3F0 (action-bar update), so today they are mutually exclusive.
   - Design research is task R-A below. Not started.
2. Run `git fetch upstream`. If upstream/master moved, note it.
3. Create the worktrees from upstream/master, then run `git branch --unset-upstream` in each:
   - `port/k2-classmenuclothes-steam` → `tools/kpm-pr-cmc`
   - `port/movie-patch-k2-steam` → `tools/kpm-pr-movie`
   - `gameapi/k2-title-gated-signatures` → `tools/kpm-pr-gameapi2`
4. Copy the Steam vanilla exe to the session scratchpad and make it read-only (agents use this copy).

## 3. Phase B: research (5 Sonnet agents in parallel, read-only, about 20-30 min)
Every brief:
- points to this plan, `ghidra_knowledge/RE_AGENT_GUIDE.md` §1-3 and the exact input files;
- requires a cited anchor for every match (string, confirmed callee, offset access, RTTI);
- writes only its own `upstream_prs/<topic>/` dir;
- ends with the 5-line subagent output contract from CLAUDE.md.

**R-A. EKC × Action Hotbar coexistence design** (`upstream_prs/ekc_hotbar_coexist/DESIGN.md`)
- Read the KotorPatcher hook installer (`src/KotorPatcher/src/` in `tools/kpm-pr-ekc`):
  - What happens when two patches detour the same address? Check KPatchCore validation/conflict detection too.
  - Does a detour's trampoline re-run the stolen bytes, so that two detours at adjacent sites would work?
- Read the Action Hotbar sources (`patch_manager_mods/force_power_hotbar_src/`, read-only) and EKC's two hooks:
  - which registers and frame slots each uses at 0x72CB63 and 0x74B3F0;
  - what each does with the input event (consume vs pass through).
- Propose 2-3 options with pros and cons:
  - (a) the hotbar calls EKC's logic as a library;
  - (b) EKC exports an optional handler the hotbar calls;
  - (c) move one patch's hook to a different, equivalent site (find candidate Steam addresses with bytes and context).
  - (d) a KPM feature: chained detours.
- Recommend one. No code.

**R-B. EKC GOG-side semantic check**: append to `upstream_prs/ekc_steam/gog_check.md`.
- Port spec risk 3: the GOG semantics came from fingerprints, not a GOG disassembly.
- From the server's GOG disassembly, confirm for all 6 hooks that the registers and frame slots the C++ reads mean the
  same on GOG as on Steam. In particular, hook 6's EDX is `this` with x/y stored at +0x3E4/+0x3E8, and hook 2's
  `[ebp+0xC]` is uMsg.
- Report PASS or FAIL per hook.

**R-C. PartyRosterLimit32K2 open items** (`upstream_prs/gog_only_triage/partyroster_resolve.md` + tsv)
- Resolve with anchors:
  - the `CSWGuiButton` constructor (hypothesis 0x41A510);
  - `CSWGuiPartySelectionButton::SetSelected`;
  - `CExoArrayList::DelIndex`;
  - the 2 effect-type data words (GOG 0x99C95C, 0x99C8B8).
- For the 41 single-byte simple hooks and the 3 single-byte detours: read the surrounding instructions in the Steam dump
  and the GOG disassembly, then give a verdict per hook (same instruction, same operand role).
- Compare `partyselect_p.gui` (patch `Additional/`) with the Steam game's copy, extracted read-only from the game's
  archives into scratch with existing tooling.
- Output: a go/no-go for porting, plus the full GOG→Steam address table.

**R-D. GameAPI K2 follow-up research** (`upstream_prs/gameapi_signatures/followup.md` + tsv)
- On Steam (dump/exe) and GOG (server): verify the K2 deleting-destructor pattern for the `Destructor` names that
  `CSWGuiImage`, `CSWGuiListBox`, `CSWGuiScrollBar`, `CSWGuiSlider` (`Destructor_2`) and `CSWGuiText` use in their wrappers.
- Locate the real K2 `CGuiInGame::ShutDownGlobal`, using the K1 body (frees panels at this+0x98..0xA4, zeroes +0x38) as
  the template.
- Find who owns `UpdateMessageGUI`'s pointer argument on K2.
- Exact typedefs for the title-gated Commit 2 (base: R1 README "Commit 2").

**R-E. K2ClassMenuClothes and Movie Patch port specs** (`upstream_prs/cmc_steam/port_spec.md`,
`upstream_prs/movie_steam/port_spec.md`)
- Start from R3 `TRIAGE.md` §1-2 and `addr_map.tsv`.
- For each hook: Steam address, exact `original_bytes`, uniqueness with context, and the frame and registers the C++ uses.
- For each hard-coded constant: its Steam value with an anchor.
  - CMC: 7 functions including C2DA ctor 0x71AD40.
  - Movie: IAT slots 0x9864A0 and 0x986494, globals 0x9F42A4 and 0x9F42A8, hook 0x40D901.
- A C++ switch design that reuses the EKC pattern (`KOTOR_VERSION_SHA` selector, unknown sha = GOG). For Movie Patch,
  check its existing per-game address struct first.
- Write both draft `kotor2-steam-aspyr.hooks.toml` files.

**Lead gate B:**
- Re-verify on disk every hook's bytes from R-E and 3 R-C single-byte verdicts (objdump on the scratch exe).
- Check RTTI or the anchors for any new vtable or constant.
- Read R-A and pick the coexistence option for the backlog.
- Commit `upstream_prs/` and update the session-context file.

## 4. Phase C: development (4 Sonnet agents, one worktree or dir each, in parallel, about 30 min)

**I-1. K2ClassMenuClothes Steam port** (`tools/kpm-pr-cmc`, only `Patches/K2ClassMenuClothes/`)
- New `kotor2-steam-aspyr.hooks.toml`, the C++ selector per R-E, and a manifest Steam target with a minor bump.
- GOG values byte-identical.
- Verify:
  - `python3 tools/validate-patches.py` (0 errors)
  - `cd Patches/K2ClassMenuClothes && python3 ../create-patch.py`
  - `-Wall -Wextra` gives no new warnings
  - the shared verifier from I-4 passes

**I-2. Movie Patch K2 Steam port** (`tools/kpm-pr-movie`, only `Patches/Movie Patch/`)
- Same shape and verification as I-1. The K1 hooks and K1 code paths stay untouched.

**I-3. GameAPI title-gated signatures** (`tools/kpm-pr-gameapi2`)
- Apply R1 Commit 2 plus the R-D findings:
  - per-title typedefs branching on `GameVersion::GetTitle()`;
  - K1 paths byte-identical.
- Verify:
  - validate-patches, 0 errors;
  - `python3 publish-patches.py --target windows_x86` (all OK);
  - a K1 patch that uses these wrappers (ModOptions, Snake) still builds;
  - `git diff upstream/master --stat` shows only `Patches/Common/GameAPI/` files.

**I-4. Steam hook verifier tool + tests** (KOTOR-II repo: `scripts/verify_steam_hooks.py` +
`scripts/test_verify_steam_hooks.py`)
- Behaviour: for a KPM worktree, read every `Patches/*/kotor2-steam-aspyr.hooks.toml` and check each hook:
  - `original_bytes` equal the vanilla exe at the VA (PE section mapping);
  - the address sits on an instruction boundary (objdump);
  - the byte pattern plus 8 context bytes is unique in `.text`;
  - the fields match the GOG file apart from address, bytes and sha.
- Output: a TSV, and a non-zero exit on any FAIL.
- Tests: fixture mini-PE plus fixture TOMLs (`tmp_path`), covering:
  - a matching hook;
  - a byte mismatch;
  - a non-unique pattern;
  - a GOG/Steam field drift.
- Verify:
  - `python3 -m pytest scripts/test_verify_steam_hooks.py -q`;
  - running the tool on `tools/kpm-pr-ekc`, `tools/kpm-pr-acc` and upstream master gives all PASS for EKC, ACC, the three merged ports and any other upstream Steam files.

**Lead gate C:**
- Review each diff with `code-review` at medium.
- Re-run validate, create-patch and publish myself, one build at a time per worktree.
- Run I-4 against the CMC and Movie worktrees.
- Commit each branch in upstream style (`K2ClassMenuClothes: Support the Steam Windows build`, `Movie Patch: Support the
  Steam Windows build`, `GameAPI: Match K2-only wrapper signatures`) with the attribution line.
- Commit I-4 and the build logs (KOTOR-II).

## 5. Phase D (lead, about 15 min): drafts and records
- PR drafts in `upstream_prs/drafts/`: `pr_cmc.md`, `pr_movie.md`, `pr_gameapi_k2.md`. Each body: problem, approach,
  files, testing. Leave "Tested on Steam under Proton: TBD".
- Update `SESSION_CONTEXT_2026-10-03.md` "Next phase" with the branches, heads and kpatch paths.
- Queue for the user: all untested `.kpatch` builds (EKC, ACC, CMC, Movie) and the approvals still pending.
- Memory: update `kpm-upstream-prs`, plus the backlog line in the hotbar memory.
- Final summary to the user, with one approval question set: what to push, which PRs to open (GameAPI is the only one
  that needs no in-game test), and whether to file the runscript issue.

## 6. Verification
- [ ] Each Phase B deliverable exists, and lead gate B's re-checks pass (bytes equal the vanilla exe at every new Steam hook).
- [ ] `pytest scripts/test_verify_steam_hooks.py` passes, and the tool reports all PASS on the EKC, ACC, CMC and Movie worktrees and upstream master.
- [ ] Per branch: validate-patches 0 errors, the `.kpatch` builds, `git diff upstream/master --stat` shows only the intended files, and the GOG/K1 hook files are unchanged.
- [ ] I-3: publish-patches windows_x86 succeeds, and the K1 users of the changed wrappers build.
- [ ] Nothing in `"$G"` changed: compare the mtime of `patch_config.toml` and the md5s of `patches/` with Phase A.
- [ ] No push, PR, issue or comment without approval.

## Phase A results (lead, 2026-10-04)
- upstream/master moved to 6374f73: #291 (contributing guide) MERGED; also #296, #297, #289, #306. New worktrees cut from 6374f73.
- GOG disassembly is now available offline: `scripts/ghidra_scripts/ExportMemoryBlocks.java` exported the GOG exe's raw blocks to
  `ghidra_dumps/upstream_server/k2_win_gog_aspyr_swkotor2.exe/memblocks/`. Use `python3 scripts/disasm_va.py gog|steam <VA> [--before N] [--len N] [--bytes N]`.
  Checked: GOG 0x613863 bytes = upstream EKC toml original_bytes.
- Steam scratch exe (read-only, sha256 6a522e71): session scratchpad `swkotor2_steam_vanilla.exe`; set `K2_STEAM_EXE` to it.
