# Verified findings (Haiku verifier, then Sonnet adjudicator)

Find-only. Tiers list real findings; decoys and controls are reported under Calibration only.

## Summary

| Tier | Count (representatives) | Count incl. folded duplicates |
|---|---|---|
| CONFIRMED | 19 | 23 |
| CONFIRMED-BY-SONNET-ONLY | 5 | 6 |
| CONTESTED-NEEDS-HUMAN | 4 | 5 |
| REFUTED | 13 | 14 |

## Calibration

- Decoys: 4/4 caught (final refuted or needs-human) of 8 queued; catch rate 1.0; Haiku reproduced 0.
- Haiku vs Sonnet agreement: all 0.8444 over 45 items; real 0.8293; Sonnet self-reported 0.8444; Haiku reproduced but Sonnet refuted 0; Haiku refuted but Sonnet confirmed 5.
- Fabricated quotes: Haiku 0 (trivial 0), Sonnet 0. Haiku quality by batch: {'ok': 5, 'weak': 2}.

Controls

| id | label | expected (Haiku / final) | Haiku | final | ok |
|---|---|---|---|---|---|
| 7e6e690122c8 | rejected | refuted/intended-behaviour ; refuted | None | None | None/None |
| aae3fb4ec1a2 | rejected | refuted/intended-behaviour ; refuted | None | None | None/None |
| e23534b73f29 | rejected | refuted/intended-behaviour ; refuted | None | None | None/None |

Per-specialty Haiku/Sonnet agreement

| specialty | n | agree | rate |
|---|---|---|---|
| build-config | 3 | 1 | 0.3333 |
| python-tooling | 8 | 7 | 0.875 |
| source-logic | 23 | 20 | 0.8696 |
| test-weakening | 11 | 10 | 0.9091 |

## Cost

- Haiku verifier: 45 call(s), CLI-reported $0.801955
- Sonnet adjudicator: 7 session(s), CLI-reported $0.285437
- Total CLI-reported $1.087392 (basis: cli_reported_total_cost_usd (the only figure used for budgets and decisions))
- ESTIMATE ONLY: illustrative list rates, priced per request, never used for a decision: $1.651592 (Haiku $0.225594, Sonnet $1.425998); Sonnet cache-write rate assumed 1.25x input.

## Coverage

- Complete: False (71 queue items, decoys and controls included)
- Missing verifier result (re-queue: `run_verify.sh --only 5d721af23495,1406bc629c1d,f88d7b0ead99,eace9e256d97,536923d9352a,a498027bbd1b,9337bf21a6e8,c740372f2a81,797aad6a288e,cf5b237d5853,da9267c332a9,811a5531ca00,7ac1208a057d,111569cb3a6d,8545c7465dad,6001871962be,155e569b4449,d9187acd0c6d,572a431bf016,f05db6672105,1f04e6823fc5,b1b738da5be9,b2668ae7e31c,e23534b73f29,aae3fb4ec1a2,7e6e690122c8`)
- Missing adjudication: 5d721af23495, 1406bc629c1d, f88d7b0ead99, eace9e256d97, 536923d9352a, a498027bbd1b, 9337bf21a6e8, c740372f2a81, 797aad6a288e, cf5b237d5853, da9267c332a9, 811a5531ca00, 7ac1208a057d, 111569cb3a6d, 8545c7465dad, 6001871962be, 155e569b4449, d9187acd0c6d, 572a431bf016, f05db6672105, 1f04e6823fc5, b1b738da5be9, b2668ae7e31c, e23534b73f29, aae3fb4ec1a2, 7e6e690122c8

## CONFIRMED (19)

### e606f41d177d  scripts/02_merge_2da_and_tlk.py:234  [high] logic (python-tooling)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: confirmed/high; Sonnet quotes ok 7/7
- Reasoning: Read 02 lines 1-18, 100-135, 180-200, 219-248 and 06 lines 155-170, 283-320; COMPATCH plan line 56. Line 116 excludes planetary.2da from live reads, line 188 builds echoes_plan only from staging, and line 234 overwrites the live table. Step 11 is titled 'Merging' yet no live rows are merged. The plan says overwrites that drop rows from 50+ mods must be avoided, and planetary is listed. 06 protects the build's planetary.2da. Only genericdoors/soundset carry a 'copied whole' comment. Magnitude needs a live-install diff.
  - check: On a real install, diff the rows of the live override planetary.2da against the staging Override planetary.2da and list rows from other mods that the staging copy lacks.
  - check: Confirm nothing in main() verifies the written tables, despite the 'VERIFIED' print at scripts/02_merge_2da_and_tlk.py:244.
  - check: Look in pykotor_patches/README.md patch #5 for why line 116 excludes planetary.2da from live reads.
  - quote scripts/02_merge_2da_and_tlk.py:116: `live = {n: read_2da(LIVE_OVERRIDE / n) for n in table_names if n != "planetary.2da"}`
  - quote scripts/02_merge_2da_and_tlk.py:188: `echoes_plan = echoes["planetary.2da"]`
  - quote scripts/02_merge_2da_and_tlk.py:234: `write_2da(echoes_plan, LIVE_OVERRIDE / "planetary.2da")`

### 1ce64f3b2759  scripts/agent_diff_check.py:106-117  [high] gate-integrity (python-tooling)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: confirmed/high; Sonnet quotes ok 7/7
- Reasoning: Read run_tests.py 855-940 (the return 0 at 930 sits inside 'if a.list:' at 885), and 1004; agent_diff_check.py 174-209. Line 862 applies -k to items only. The 'no test item matched' check at 932-940 covers ids and positional filters, not -k. A -k that matches nothing therefore reaches the summary with '(0 items)' and returncode 0, so test_ok is True and the verdict can be PASS (line 186). Haiku's all-SKIP sub-claim is not verified: execute() was not read.
  - check: Run `python3 scripts/run_tests.py -k zz_no_such_item; echo $?` from the repo root and confirm exit 0 with a '(0 items)' summary line.
  - check: Read execute() in scripts/run_tests.py to check whether a selection of only SKIP items also returns 0 (the all-SKIP sub-claim).
  - check: Check whether any orchestrator reads test_ok or the PASS line without requiring a non-zero test count.
  - quote scripts/run_tests.py:862: `items = [i for i in items if a.k in i.id]`
  - quote scripts/run_tests.py:885: `if a.list:`
  - quote scripts/run_tests.py:932: `# Every requested id / filter must match something; --json output is not written on this exit-2 path.`

### 119182a14fc2  scripts/plan_manager.py:349-356  [high] logic (python-tooling)
- Also covers duplicate finding(s): a5dcf2744b60 (scripts/plan_manager.py:348-350)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: confirmed/high; Sonnet quotes ok 5/5
- Reasoning: Read plan_manager.py 68-141, 749-752, 786, 530-579. mark-done passes args.phase unchanged to cmd_mark_done; resolve_plan_shorthand's detail is discarded. The table match at 533 compares bare cell text, so '24:T4' misses; the live table uses bare ids ('\| T4 \|' at 24_master_plan_status.md:34), and no '24:T4' string exists in the plan. The fallback at 579 appends a bogus checkbox line. main then prints 'Completed' while the real phase stays open. The printed next-step command is the normal path, so high.
  - check: In a disposable copy of the repo, run `python3 scripts/plan_manager.py mark-done "24:T4"` and diff patch_manager_mods/24_master_plan_status.md. Expect the T4 row unchanged and a new '- [x] 24:T4 (completed)' line.
  - check: Check git log -p for 24_master_plan_status.md around lines 153-164 to see whether the existing '- [x] M1 (completed)' lines came from this fallback.
  - check: Check AGENTS.md / CLAUDE.md to see whether the NN:Tn form is documented as valid mark-done input.
  - quote scripts/plan_manager.py:750: `resolved_p, _ = resolve_plan_shorthand(args.phase)`
  - quote scripts/plan_manager.py:786: `cmd_mark_done(plan, args.phase, commit_sha=args.commit, notes=args.notes, state=args.state)`
  - quote scripts/plan_manager.py:533: `if clean_ident.lower() == clean_target.lower() or clean_target.lower() in clean_ident.lower():`

### bc58247fd299  patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:358-364  [medium] logic (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read ActionHotbar.cpp 357-362 and 379-383, and HotbarLogic.h 76-93. The OID clause at 360 has no sentinel guard and breaks at the first match. If slot 0 also reads 0xFFFFFFFF, isMember0 becomes true and characterKeyFor returns 'objfffffffd' (HotbarLogic.h:92-93) for a non-Exile controlled creature, sharing the Exile's bindings. A readable tag wins first (83-85), which limits reach. It needs a non-Exile to be controlled while both ids read the sentinel. The pointer-only 'old' loop cannot be checked (no git).
  - check: Check a session log for 'has invalid object id' (ActionHotbar.cpp:378) with slot=0 while the controlled creature pointer differs from party member 0's.
  - check: Check whether GetControlledCreature can return a non-Exile (Party.h:51-61) during control switches or cutscenes.
  - check: Confirm readCreatureTag fails for that creature in that state, since a readable tag bypasses the id logic.
  - quote patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:360: `if (mem == c \|\| (mem != 0 && creatureOid(mem) == oid)) {`
  - quote patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:381: `the id can read 0xFFFFFFFF for a whole session`
  - quote patch_manager_mods/action_hotbar_src/HotbarLogic.h:92: `if (oid == 0xFFFFFFFFu && !(partyCount == 1 \|\| isMember0 \|\| partySlot == 0))`

### 9e76e8f4642a  patch_manager_mods/gui_input_src/gi_registry_test.cpp:137-144  [medium] test (test-weakening)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 5/5
- Reasoning: Read gi_registry_test.cpp:137-144 and :47, GuiInputProvider.cpp:156-160, GuiInput.h:49. The no-window case drives only a fake table. apiWheelSubscribe appears only in GuiInputProvider.cpp (Grep), and gui_input_src/common_contract_test.cpp:3 names the provider only in a comment, so the provider's no-window branch has no test that can fail. Sub-points rejected: the <= 0 check matches the documented failure range at GuiInput.h:49. Line 144 is GEQ (>= 1), not == 1 as the claim states.
  - check: Confirm no host test target compiles or links GuiInputProvider.cpp (check the gui_input_src build configuration).
  - check: In a scratch copy, force ensureSubclass() to fail and run gi_registry_test; it should still pass, which confirms the gap.
  - quote patch_manager_mods/gui_input_src/gi_registry_test.cpp:137: `// 7. Provider present but game window not yet available: wheel_subscribe fails`
  - quote patch_manager_mods/gui_input_src/gi_registry_test.cpp:143: `GCHECK("wheel_subscribe without game window returns token <= 0", noWinTok <= 0);`
  - quote patch_manager_mods/gui_input_src/gi_registry_test.cpp:144: `GEQ("no-window subCalls count", g_subNoWinCalls, 1);`

### 81dabb3f4f5e  patch_manager_mods/mouse_probe_src/MouseProbe.cpp:1-3  [medium] logic (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: downgraded/medium; Sonnet quotes ok 5/5
- Reasoning: Re-read MouseProbe.cpp 1-30, 56-146, 262-313 and MouseFixLogic.h. Header line 3 names sanitizeNorm as the only engine write, but line 306 stores clicklogic::MsgLDbl into msgp+4 whenever the synth_dblclk gate (default 1, line 140) passes, and the header's click paragraph (lines 12-17) never mentions it. Sanitize's default-on state is disclosed on line 3, so the defect is the false write-scope statement in a measurement probe. The removed 0.1.0 text and the parity sentence at line 10 (scoped to the log sink) cannot be checked from this tree.
  - check: Confirm from version history outside this tree whether 0.1.0 wrote engine memory and what its removed header said.
  - check: Decide whether the header should list the SYNTHDBL write at MouseProbe.cpp:306 and whether default-on sanitize_norm is acceptable for a probe that measures the bug.
  - check: Check the deployed mouse_probe.ini (not in the tree) for sanitize_norm and synth_dblclk values.
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:3: `The only engine write is the sanitizeNorm path in ProbeMouseNorm (this+0x408/+0x40C), on by default`
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:306: `*reinterpret_cast<int*>(msgp + 4) = clicklogic::MsgLDbl;`
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:140: `g_fixCfg.synthDblClk = GetPrivateProfileIntA(`

### eee579216787  patch_manager_mods/mouse_probe_src/MouseProbe.cpp:189-193  [medium] bug (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 6/6
- Reasoning: Re-read MouseProbe.cpp 124, 180-221 and MouseFixLogic.h 38-72. POINT is pre-zeroed at line 192, so when GetCursorPos is skipped or returns FALSE the origin is used as the real cursor. That origin feeds the drag delta (saturating to plus or minus 1), g_lastDragPos, and checkEdgeWarp, which fires a SetCursorPos recentre when the window origin is non-negative. The null-pointer branch is practically unreachable because user32 is always loaded, so the live path is the FALSE return. The discarded rd() at line 189 is benign. Real but rare-trigger edge case.
  - check: Under Wine or Windows, force dyn_GetCursorPos to fail during an RMB camera drag and check the NORM SAT lines and the recentre after MouseProbe.cpp:193-221.
  - check: Count real GetCursorPos failures in a session to decide whether the spin is one frame or persistent.
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:192: `POINT cur = {0, 0};`
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:193: `if (dyn_GetCursorPos) dyn_GetCursorPos(&cur);`
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:189: `rd(DragFlagAddr, 0, &drag);`

### 8ea1731f8816  patch_manager_mods/queue_hud_src/QueueHud.cpp:720-799  [medium] memory-safety (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: downgraded/medium; Sonnet quotes ok 5/5
- Reasoning: Read QueueHud.cpp 706-745 and 795-809. Each tick re-reads slot k's creature (720-721) and gates the queue on that current creature via gatherFor(srv) (728). Buffer clear happens only on a null srv (723). Dispatch and re-inject pass the stored creature (758, 800) with no identity check against the slot. If the slot's pointer changes to a new non-null object without an intervening null read, a buffered action runs on the old creature while gating on the new one's queue. Use-after-free is not provable from the tree. Medium.
  - check: At runtime, log nextA.creature (QueueHud.cpp:787-800) against party::member(tbl, k) (QueueHud.cpp:720) through a companion swap and an area load while buf.count > 0. Check whether slot k ever changes to a new non-null pointer without a null read.
  - check: Confirm which pointer the engine passes as `this` to PerformCastSpell and PerformUseItem, and whether those entry points reject a stale or freed object (PQ1.md:39 notes).
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:721: `uintptr_t srv = cli ? party::serverCreature(cli) : 0;`
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:723: `if (buf.count > 0 \|\| buf.active) buf.clear();`
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:728: `if (!gatherFor(srv, s)) continue;`

### d682bb833922  patch_manager_mods/queue_hud_src/QueueHud.cpp:973-1036  [medium] hook (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: confirmed/high; Sonnet quotes ok 4/4
- Reasoning: Read QueueHud.cpp 925-954 and 1000-1036. The claim's 'never checks party membership' is wrong: lines 1002-1012 match the caster against the party table. The real defect is the fallback at line 1013, which sends any unmatched caster to member 0, including every caster when party::table() fails. BUFFER then consumes the engine cast (1028-1035). The same fallback is at line 939. Whether non-party casts reach the client wrapper is not in the tree; PQ1 documents only UI callers. Medium, not high.
  - check: Xref or disassemble callers of CSWCCreature::PerformCastSpell (0x00775700, hook at 0x00775703) and look for non-party or AI-driven call sites.
  - check: Run with QLOG on and check whether 'buffered spell use for member %d' (QueueHud.cpp:1033) fires for a creature absent from the party table.
  - check: Check when party::table() returns 0. If only that failure path reaches the line 1013 fallback, the exposure is narrower than 'any non-party caster'.
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:1006: `if (m && (m == static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature)) \|\|`
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:1013: `if (memberK < 0) memberK = 0;`
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:939: `if (memberK < 0) memberK = 0;`

### f77ae8184a09  patch_manager_mods/queue_hud_src/QueueHudLogic.h:575-580  [medium] logic (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: downgraded/medium; Sonnet quotes ok 5/5
- Reasoning: Read QueueHudLogic.h 512-517 and 575-581, QueueHud.cpp 733-744, and tests 760-768 and 1186. ActionBuffer has no completion or 'seen in engine queue' field. So a buffered action that leaves the queue (finished, failed or eaten) while a 0x3D entry is present is indistinguishable and gets re-injected (QueueHudLogic.h:580), up to two times (578). The design (30_queue_hud.md:88) treats 0x3D as the wipe signal, and no test covers completion with 0x3D present. Whether that coexistence happens in play is runtime. Medium.
  - check: Inspect gatherFor (QueueHud.cpp:216-261) to confirm whether a completed spell can still appear in s.e, s.cur or s.rn on the tick it leaves the queue.
  - check: Live run with a follower present: cast a buffered spell that completes normally and check the log for 're-injecting eaten spell' (QueueHud.cpp:751) while a 0x3D entry is queued.
  - check: Consider a test for completion with hasFollowLeader=true, which none of the current tests cover.
  - quote patch_manager_mods/queue_hud_src/QueueHudLogic.h:580: `return hasFollowLeader;`
  - quote patch_manager_mods/queue_hud_src/QueueHudLogic.h:516: `bool reInjectPending = false;`
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:735: `if (s.e[i].type == 0x3D) hasFollowLeader = true; // AIActionFollowLeader`

### 035d6f241dd2  research/companion_levelup/tools/parity/gate_test.cpp:301-310  [medium] test (test-weakening)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 6/6
- Reasoning: Read gate_test.cpp:270-310, P() at :97, LevelUpOverride.cpp:2021-2046 and SafeRead.h:97-101. For ecx=1, probeRead fails the a < 0x10000 test, so the guard at 2037 returns the real object before the dummy return at 2046. Every ecx=1 path returns real, so the PC, unreadable-creature, instant=0 and log-mode CHECKs (301-310) cannot fail. No other test covers log mode or PC with a readable ecx. The claim misreads the line-299 comment, which concerns a readable ecx.
  - check: In a scratch copy, temporarily drop the mode check at LevelUpOverride.cpp:2024 and run gate_test; the CHECKs at gate_test.cpp:301-310 should still pass, which confirms they are vacuous.
  - check: Optionally check git history for gate_test.cpp:301-310 to see whether the P(1) ecx predates this packet (the tree has no .git).
  - quote research/companion_levelup/tools/parity/gate_test.cpp:301: `CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: PC -> real object");`
  - quote research/companion_levelup/tools/parity/gate_test.cpp:310: `CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: log mode -> real object");`
  - quote research/companion_levelup/tools/parity/gate_test.cpp:97: `static void* P(uintptr_t a) { return reinterpret_cast<void*>(a); }`

### d121f4a74209  research/companion_levelup/tools/parity/unit_gap_test.cpp:48  [medium] gate-integrity (test-weakening)
- Also covers duplicate finding(s): 32982de3d2be (research/companion_levelup/tools/parity/unit_gap_test.cpp:48)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: confirmed/high; Sonnet quotes ok 4/4
- Reasoning: Read unit_gap_test.cpp:33-63: line 48 holds only the comment, and no F1 cap CHECK remains in the packet. LevelUpOverride.cpp:164 shows the cap logic has left this tree, and KpLog.h and its tests are absent, so in-tree F1 cap coverage is zero. Corroborating: the in-tree LOG_CAP checker (luo_log_check.py:155) looks for '== log line cap reached ==', which no tree emitter writes; the fallback writes '--- log cap reached ---' (fallback_log.txt:4), so the only in-tree cap-marker check cannot fire. The six removed CHECKs cannot be compared without git.
  - check: Locate KpLog.h and its test file in the Common tree; confirm they assert exactly one cap marker, the cut-off line, and that writes past the cap emit nothing.
  - check: Check VCS history for the pre-change unit_gap_test.cpp lines 48-57 to confirm the six removed CHECKs.
  - check: Compare the LOG_CAP pattern in scripts/luo_log_check.py:155 with the text KpLog actually writes at the cap.
  - quote research/companion_levelup/tools/parity/unit_gap_test.cpp:48: `// F1 log cap moved to KpLog.h (Common); pinned by KpLog's own tests.`
  - quote scripts/luo_log_check.py:155: `("LOG_CAP", r"== log line cap reached ==", "cpp:135"),`
  - quote scripts/testdata/kplog/fallback_log.txt:4: `--- log cap reached ---`

### 2bb645e71f18  scripts/abi_check.py:362-367  [medium] gate-integrity (python-tooling)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: confirmed/high; Sonnet quotes ok 7/7
- Reasoning: Read abi_check.py 150-161, 296-330, 359-400. Lines 362-367 fall back to candidates[0] with only a stderr note. Lines 384-386 write dumped_data to golden X with no name check. The name check at 157 is in diff_abi_golden, reached only at 394, after the continue at 388, so update mode skips it. The --header guard at 325 does not cover this path. The defect is real but latent: the tree has one golden and one matching dumper, so the trigger needs a new golden without a dumper.
  - check: In a scratch copy, rename a golden's interface to a name with no matching abi_dump file, run abi_check.py --update, and see whether the golden is rewritten with another interface's name.
  - check: Grep CI or hook configuration for abi_check.py --update invocations to judge whether the update path can run unattended.
  - quote scripts/abi_check.py:366: `dumper_src = candidates[0]`
  - quote scripts/abi_check.py:367: `sys.stderr.write(f"No dumper for {iface_tag}; falling back to {dumper_src.name}`
  - quote scripts/abi_check.py:384: `if args.update:`

### 822afff6842c  scripts/generate_ultrawide_gui_overrides.py:694-696  [medium] gate-integrity (python-tooling)
- Also covers duplicate finding(s): 601ca469508b (scripts/generate_ultrawide_gui_overrides.py:683-696)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: confirmed/high; Sonnet quotes ok 3/3
- Reasoning: Read 683-696 and the caller at 1743-1745. Any exception from the ps probe (missing binary or non-zero exit) is swallowed by 'except Exception' and returns False, so the deploy/rollback abort fails open as claimed. The 'only caller' detail is wrong: generate_ultrawide_textures.py:2577 uses the same probe as its own deploy/rollback gate, which widens the impact. No comment marks the fail-open as deliberate. Medium because it needs ps to fail.
  - check: On the deploy host, run the generator with ps removed from PATH and confirm --deploy proceeds while swkotor2.exe is running.
  - check: Confirm the same fail-open in generate_ultrawide_textures.py's deploy/rollback gate near line 2577.
  - check: Run `ps -eo pid,args` under wine and confirm the swkotor2.exe line matches the regex `[s]wkotor2\.exe`.
  - quote scripts/generate_ultrawide_gui_overrides.py:694: `except Exception:`
  - quote scripts/generate_ultrawide_gui_overrides.py:1743: `if (args.deploy or args.rollback) and is_game_running():`
  - quote scripts/generate_ultrawide_textures.py:2577: `or getattr(args, "world_rollback", False)) and is_game_running():`

### 6ccefb375bd5  scripts/generate_ultrawide_gui_overrides.py:1680-1684  [medium] gate-integrity (python-tooling)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: confirmed/high; Sonnet quotes ok 3/3
- Reasoning: Read 1675-1687, fx at 605-607, content_factor at 600-602, and the call at 1835. For k>0 the tabs term is exactly 6k, and fx is strictly increasing, so both portrait orderings hold. ok is therefore always True for valid inputs (k cannot reach 0 because aspect >= 4:3). main discards the boolean, so the 'registration: OK' header line is a check that cannot fail. It gates nothing, so medium.
  - check: Call registration_check(k) for k = 0.7407 and 0.5, and again with the portrait anchor moved to 750, to confirm ok only flips when a REGISTRATION constant is edited.
  - check: Confirm the printed header line reads 'registration: OK' for any valid --width/--height.
  - quote scripts/generate_ultrawide_gui_overrides.py:1681: `abs(f["top_p tabs anchor"] - f["equip_p LB_DESC right edge"]) <= 6 * k + 1e-9`
  - quote scripts/generate_ultrawide_gui_overrides.py:607: `return 400.0 + (x - 400.0) * k`
  - quote scripts/generate_ultrawide_gui_overrides.py:1835: `reg, _ = registration_check(content_k)`

### d74365c9d82d  patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:696-697  [low] logic (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 6/6
- Reasoning: Read CompanionSelfHeal.cpp 296-340, 696-698, 750, 770-780, 829, 889-894; SelfHealUi.h:53-62; Pick at :132-133. Stale have and c0.m are real: refreshCache sets have false but keeps c0.m, and line 777 re-asserts have. Haiku's mechanism is wrong. refreshCache clears c0.pk to Pick() (pick -1), so resolveState returns NoItem (or FullHp). clickAllowed(NoItem) routes to the log-only branch at 892, so useNow and the 307 refusal are not reached. The result is a misleading no-item leader button with dead clicks, transient while the party read fails with the HUD open.
  - check: Trace a frame where party::members returns 0 while the k==0 HUD slot is still matched. Record g_card0.st and the 'CLICK member=0 noitem' log line on a leader click.
  - check: Confirm no 'REFUSE member 0 not in party' line appears for the leader in that state, since the stale state resolves to NoItem.
  - quote patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:696: `c0.have = 0 < cnt && v[0].ok;`
  - quote patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:697: `if (!c0.have) { c0.pk = Pick(); c0.apk = Pick(); } else {`
  - quote patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:777: `g_card0.have = true;`

### 6adfec63f6fb  patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:70-73  [low] logic (source-logic)
- Haiku: reproduced (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 5/5
- Reasoning: Read Inventory3DViewport.cpp 55-88 and 395-480. Confirmed: logw has one shared 4000-line cap (70-72) with no suppressed marker. H1 bind failures (447-465) and per-call H2 warnings (479-503) both use it, while loge (77-80) is uncapped. The author's dlogf comment (83) shows bind lines were meant to be protected from starvation, which logw does not do. The trigger needs thousands of ShowItem calls on an unusable slot. ShowItem is an item-selection hook, so the rate is human-driven and the cap is rarely reached. Silent loss is real but edge-case.
  - check: In a long session log (inventory-3d-viewport_log.txt, not in the tree), count 'H2: slot unusable' lines against the 4000 cap and check whether any later 'H1' line is missing.
  - check: Check how often the item-selection hook (0x008A8172) fires on an unusable slot in a real session.
  - quote patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:70: `constexpr int MaxWarnLines = 4000; // warnings can fire per ShowItem call while a slot stays unusable`
  - quote patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:72: `if (g_warnLines >= MaxWarnLines) return;`
  - quote patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:447: `if (id != ExpectedControlId) { logw("H1: GUI control missing (id=%d), slot unusable", id); return; }`

### ff23d43b210c  patch_manager_mods/mouse_probe_src/MouseProbe.cpp:220-221  [low] memory-safety (source-logic)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: downgraded/medium; Sonnet quotes ok 7/7
- Reasoning: Re-read MouseProbe.cpp 2, 180-230, SafeRead.h 107, and doc 18 lines 184 and 212. The stores at 220-221 are unchecked, and the header's SafeRead statement covers reads only, so the write gap is real. The hook sits at the PollMouseDelta merge point, where the engine has just written this+0x408/+0x40C on both paths (doc 18 says both are 0 on failure), so self is engine-live and already dereferenced in the same function. The stale-pointer scenario is therefore unlikely. probeWrite exists but is unused by this probe.
  - check: Disassemble PollMouseDelta (around 0x0072F110) and confirm this+0x408/+0x40C is stored on every path before the hook at 0x0072F2EC, so self is always engine-live there.
  - check: Confirm no path reaches the 0x0072F2EC detour without the engine having dereferenced [ebp-0x28] first.
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:184: `if (!rd(ebp, -0x28, &self) \|\| self == 0) return;`
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:220: `*reinterpret_cast<float*>(self + 0x408) = fixedNx;`
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:226: `if (!rd(self, 0x408, &nx) \|\| !rd(self, 0x40C, &ny)) return;`

### b5a7154f89eb  research/companion_levelup/tools/parity/parity.py:33  [low] build (python-tooling)
- Also covers duplicate finding(s): 6fdc7c29d62d (research/companion_levelup/tools/parity/parity.py:32-34)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: downgraded/medium; Sonnet quotes ok 5/5
- Reasoning: Read parity.py 32-36, 424-436 and 518-536; gate_test.cpp 17-25; a tree-wide Glob for TestHarness.h. build() adds the expanduser Patches/Common -I to every compile (line 33) and exits on failure (36). gate_test.cpp:22 includes TestHarness.h. The only TestHarness.h in the tree is tests/include/TestHarness.h, which the gate's -I list omits, while the R10 builds (524, 533) add it. So the gate cannot compile gate_test.cpp on a clean checkout. The failure is loud and the workspace-bound layout looks deliberate, so low.
  - check: Inspect the workspace Patches/Common directory: does it hold TestHarness.h at its top level? tests/include/TestHarness.h says it is synced from Patches/Common/test/, which is a different path from the -I target.
  - check: Run the gate's build line with the home path absent (for example `g++ -std=c++17 -fsyntax-only` with the stub dirs) to confirm the compile failure on gate_test.cpp.
  - quote research/companion_levelup/tools/parity/parity.py:33: `os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common")`
  - quote research/companion_levelup/tools/parity/parity.py:36: `sys.exit(r.stderr)`
  - quote research/companion_levelup/tools/parity/gate_test.cpp:22: `#include "TestHarness.h"`

## CONFIRMED-BY-SONNET-ONLY (5)

### d2c66b77d42f  research/companion_levelup/tools/parity/scenario_test.cpp:408-409  [medium] gate-integrity (test-weakening)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: downgraded/high; Sonnet quotes ok 6/6
- Reasoning: Read scenario_test.cpp:406-411, LevelUpOverride.cpp:164,1576, and mutate.py:23-28,64-66,254-262,326-329. The logcap branch is empty as claimed, but the cap is no longer in this tree: logOpen() is constant true and no g_logLines/MaxLogLines remain, so X13 asserts nothing about the cap. The survival wording is wrong: LG02/LG03 match zero times, and apply() raises SystemExit on any pattern not matching exactly once, so the staleness pass aborts the whole mutation run. LG01 matches line 1576 but is equivalent.
  - check: Run mutate.py against this tree (or --only LG02) and confirm it exits with 'STALE mutation LG02' before any mutant runs.
  - check: Read KpLog.h and its tests in the Common tree to see whether the 20000-line cap is pinned there.
  - check: Decide whether X13_log_capped should be renamed or dropped, and whether LG01-LG03 should be removed.
  - quote research/companion_levelup/tools/parity/scenario_test.cpp:408: `} else if (a == "logcap") { // the 20000-line log cap reached (F1)`
  - quote research/companion_levelup/tools/parity/scenario_test.cpp:409: `// the 20000-line cap moved to KpLog.h (Common) and is pinned by its own tests; nothing to force here, the scenario still checks the gate is unaffected`
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:164: `inline bool logOpen() { return true; }`

### 6a2684bd832c  neocities_mods_detailed.json:10  [low] other (build-config)
- Also covers duplicate finding(s): 20f906aab1cd (neocities_mods_detailed.json:10)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read neocities_mods_detailed.json:10 (the whole entry note). The tree has the text verbatim: an unsigned executable that rewrites the game binary, 'you can override Windows/your AV and run it', no hash or signature, and the source only as 'HERE' with no URL in the file. Haiku is right that the wording is deliberate, but deliberate editorial guidance still weakens a user-side check. Low severity: it is advice, and the missing link may be an export artifact.
  - check: Open the live guide entry for 3C-FD and check whether 'HERE' links anywhere; compare with the export that produced neocities_mods_detailed.json.
  - check: Check the deadlystream file 2734 page for a published SHA-256 or signature and for a linked source repository.
  - check: Decide editorially whether telling readers to override an AV block on an unsigned binary should stay.
  - quote neocities_mods_detailed.json:10: `When attempting to run this executable, Windows/your antivirus might attempt to block it, because it's an unsigned executable.`
  - quote neocities_mods_detailed.json:10: `the source code is publicly available HERE --please feel assured that this program is safe`
  - quote neocities_mods_detailed.json:10: `you can override Windows/your AV and run it.`

### ce3f285cbcb0  patch_manager_mods/level_up_override_src/test.toml:4  [low] build (build-config)
- Haiku: refuted (medium), quote ok ratio 1.0; Reduce: downgraded/medium; Sonnet quotes ok 6/6
- Reasoning: Read test.toml 1-4, common_contract_test.cpp 1-20, and the #include lines of GuiTip, GuiKit, GameAddr, K2Addr, Party, SafeRead, FileWatch and WindowHook. Line 4 commits a per-user $HOME Patches/Common -I, copied from shield_bar_src. No header in this test's direct include closure is in Patches/Common, and no kpatch_api.h is reached, so the test does not need the path. Haiku's $HOME expansion point is right (run_tests.py expand), but the committed per-user path is still a hygiene defect. Low.
  - check: On a clean checkout without the Patches/Common directory, build common_contract_test with the test.toml flags and -Werror to confirm it compiles.
  - check: Ask the author whether the Patches/Common entry was copied from shield_bar_src/test.toml on purpose, since this test has no PortraitSlots.h dependency.
  - quote patch_manager_mods/level_up_override_src/test.toml:4: `cxxflags = ["-I../_shared_src", "-I$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common", "-I../../research/companion_levelup/tools/parity/stub", "-DGUIKIT_HOST", "-Werror"]`
  - quote patch_manager_mods/level_up_override_src/test.toml:1: `# Per-test table: the Common contract test needs the shared headers and the host Win32 stub (mirrors shield_bar_src/test.toml).`
  - quote patch_manager_mods/level_up_override_src/common_contract_test.cpp:11: `#include "GuiTip.h"`

### ed6f9f335c3f  patch_manager_mods/mouse_probe_src/MouseProbe.cpp:305-306  [low] memory-safety (source-logic)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 6/6
- Reasoning: Re-read MouseProbe.cpp 281-313, MouseFixLogic.h 69-72 and doc 18 line 178. The synthesis is deliberate (MouseFixLogic.h:69, default-on, logged as SYNTHDBL), so the rewrite itself is intended. Two parts of the claim hold: the file header's click paragraph (lines 12-17) does not mention the write, and the store at 306 has no writability check. The message struct is built per call by the engine's WndProc, so it is likely a writable local and the practical risk is low. Haiku's intended-behaviour verdict misses the undocumented and unchecked points.
  - check: Disassemble WndProc 0x40A750 and OnMouseMessage 0x72E030 to confirm the message struct at [ebp+8] is a per-call writable local.
  - check: Check whether the engine re-reads msg+4 after the 0x72E039 hook, which decides whether the rewrite reaches dispatch.
  - check: Run with synth_dblclk=1 on a Wine session that drops WM_LBUTTONDBLCLK and check the SYNTHDBL log lines and any access violation at the hook.
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:286: `if (!rd(ebp, 8, &msgp)) return;`
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:288: `if (!rd(msgp, 4, &id) \|\| !clicklogic::isMouseButtonMsg(id)) return;`
  - quote patch_manager_mods/mouse_probe_src/MouseProbe.cpp:306: `*reinterpret_cast<int*>(msgp + 4) = clicklogic::MsgLDbl;`

### 9059eb31f144  patch_manager_mods/queue_hud_src/QueueHud.cpp:734-739  [low] logic (source-logic)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 5/5
- Reasoning: Read QueueHud.cpp 706-745, QueueHudLogic.h 572-586 and 60-84, and 30_queue_hud.md 76-90. The predicate is identity-blind: any T_CAST_SPELL or subType 9/10 entry in the creature's queue blocks re-inject (QueueHudLogic.h:579) and the 500 ms clear. The design doc (line 81) asks for type-level detection, so the mechanism is intended, but its effect on the player's own manual casts is not discussed. Permanent loss needs the follow entry to clear during the overlap, so low. The claim's 'mapping only in a comment' part is wrong: QueueHudLogic.h:74 maps subType 9 to spellId.
  - check: Inspect actq::read to confirm whether the same creature's engine queue holds player-issued casts as T_CAST_SPELL entries.
  - check: During a manual cast with buf.active set, log actionInQueue (QueueHud.cpp:734-740) and the shouldReInject result (QueueHud.cpp:744), and check whether re-inject stays blocked after the manual cast ends.
  - check: Check version history outside this tree for when line 734 widened from item-only, to see whether own-cast suppression was considered.
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:734: `if (s.e[i].type == qh::T_USE_ITEM \|\| s.e[i].type == qh::T_CAST_SPELL) actionInQueue = true;`
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:737: `if (s.haveCur && (s.cur.subType == 9 \|\| s.cur.subType == 10)) actionInQueue = true;`
  - quote patch_manager_mods/queue_hud_src/QueueHudLogic.h:579: `if (actionInQueue) return false;`

## CONTESTED-NEEDS-HUMAN (4)

### 66c1305db6f7  scripts/subagent_prompt.py:192-218  [medium] logic (python-tooling)
- Also covers duplicate finding(s): 9f94f6db6294 (scripts/subagent_prompt.py:199-200)
- Haiku: reproduced (medium), quote ok ratio 1.0; Reduce: unverified/high; Sonnet quotes ok 4/4
- Reasoning: Read subagent_prompt.py 140-218. The keyword tests at 209-212 ('companion') run before the plan-path test at 217-218, which is labelled '# 5d. Plan-level fallback', so precedence is deliberate-looking. A level-up plan that mentions companion with no *_src token would get companion_ai_src; the level-up MASTER_PLAN is companion-heavy, so the trigger is plausible. The old path-first ordering cannot be checked (no git). Whether real phase sections hit this needs extract_phase_info output.
  - check: Run extract_phase_info on a real level-up phase (research/companion_levelup MASTER or PHASE plans) and print mod_scope; check whether 'companion' appears with no *_src token.
  - check: Ask the author whether keyword precedence over the plan-path fallback is intended.
  - check: Check git history outside this tree for the o131-o133 ordering the claim cites.
  - quote scripts/subagent_prompt.py:211: `elif "companion" in ctx_lower:`
  - quote scripts/subagent_prompt.py:214: `# 5d. Plan-level fallback`
  - quote scripts/subagent_prompt.py:217: `if "companion_levelup" in plan_str or "level_up" in plan_str:`

### 843bd1103025  patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:164-180  [low] logic (source-logic)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read LevelUpOverride.cpp 149-195 and 1205-1225. The in-file cap is gone and logf calls KP_LOG directly. Growth looks bounded by design in the shared library: the 07b doc says the fallback sink has a 60000-line default and the core rotates by max_bytes, and LevelUpOverride calls KpLog_SetFallbackFile (162). KpLog.h is not in the tree, so the core's rotation and whether each KP_LOG line reaches disk before a crash cannot be read here. Unbounded growth is mitigated by design; the flush-on-crash question needs the library source or a live run.
  - check: Read the KpLog.h fallback line-count path and the core max_bytes rotation, and confirm whether each KP_LOG write is flushed before return.
  - check: Kill a game session just after a level-up dump and check whether the last dump lines survive in levelup_override_log.txt or the core log.
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:162: `const bool g_fallbackSet = (KpLog_SetFallbackFile(LogName), true);`
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:179: `KP_LOG(chLevelUp, "%s", buf);`
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1217: `if (!KpLog_HasCore()) {`

### 5a447a280525  research/companion_levelup/tools/parity/scenario_test.cpp:177  [low] test (test-weakening)
- Haiku: inconclusive (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 5/5
- Reasoning: Read stub/LuoLogHost.h:1-10, scenario_test.cpp:175-177 and 470-477, scenarios.py:162-167. The reset depends on KpLog_SetFallbackFile, which is absent from the tree, so whether it reopens after remove() cannot be settled here. The cited example is wrong: X11_no_ini has an EVENT check (scenarios.py:166), and scenario_test.cpp:476 runs it with CHECK(logRe(e)), so an empty log turns the run red, not silently green. A silent-pass risk remains only for negative-only checks, which the tree does not show.
  - check: Read KpLog_SetFallbackFile in KpLog.h (outside this tree) and check whether it reopens the file when the path is unchanged after remove().
  - check: Run scenario_test on X11_no_ini and inspect levelup_override_log.txt after clearLog(); confirm the '^CONFIG missing' line is present.
  - check: Find any scenario with only ABSENT or NONE expectations and no EVENT check, since only those could pass on an empty log.
  - quote research/companion_levelup/tools/parity/stub/LuoLogHost.h:9: `static inline void luoHostLogReset(const char* path) { std::remove(path); KpLog_SetFallbackFile(path); }`
  - quote research/companion_levelup/tools/parity/stub/LuoLogHost.h:3: `KpLog also keeps the fallback file open, so remove()ing it leaves later writes on an`
  - quote research/companion_levelup/tools/parity/scenario_test.cpp:177: `static void clearLog() { luoHostLogReset("levelup_override_log.txt"); }`

### e3ae385c8f94  research/companion_levelup/tools/parity/stub/LuoLogHost.h:9  [low] logic (source-logic)
- Haiku: inconclusive (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read LuoLogHost.h (9 lines), 19e_phase2_plan.md:106 and robustness common_contract_test.cpp:124. Line 9 calls std::remove() before KpLog_SetFallbackFile(), which is exactly what the comment at lines 3-4 warns against, so comment and code contradict each other. Whether this leaves writes on an unlinked inode depends on KpLog.h, which is not in the snapshot (no KpLog file under research/companion_levelup). The impact is limited to host parity tests; shipped code is not touched.
  - check: Check out feat/kplog-header (KpLog.h, about 462 lines, per 19e_phase2_plan.md:282) and read KpLog_SetFallbackFile. See whether it closes the previous FILE* and resets lazy-open state when the path is unchanged, or returns early.
  - check: Run research/companion_levelup/tools/parity gate_test or scenario_test. Compare 'ls -li' on levelup_override_log.txt before and after clearLog() plus one INFO line, to see whether later writes land on the unlinked inode.
  - quote research/companion_levelup/tools/parity/stub/LuoLogHost.h:3: `KpLog also keeps the fallback file open, so remove()ing it leaves later writes on an`
  - quote research/companion_levelup/tools/parity/stub/LuoLogHost.h:9: `static inline void luoHostLogReset(const char* path) { std::remove(path); KpLog_SetFallbackFile(path); }`
  - quote patch_manager_mods/19e_phase2_plan.md:106: `Uses the fallback file if one was set, opened lazily.`

## REFUTED (13)

### 6572c3d2f7ca  patch_manager_mods/_shared_src/GameAddr.h:69  [none] other (source-logic)
- Haiku: refuted (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 3/3
- Reasoning: Read GameAddr.h 1-10 and 66-69, QueueHud.cpp 56-60, and the queue_hud_src hooks glob (only kotor2-steam-aspyr.hooks.toml). The header limits the K2AddrTable requirement to code that must also run on GOG. QueueHud is documented Steam-only, so no GOG-running caller uses PerformCastSpell today. The name appears only at GameAddr.h:69. The gap is latent and would matter only if a GOG queue_hud build were added.
  - check: Confirm no build target or manifest compiles queue_hud_src against the GOG Aspyr exe (the manifest cited at QueueHud.cpp:58 is not in the tree).
  - check: If a GOG port is planned, check the PerformCastSpell row in patch_manager_mods/ghidra_knowledge/gog_map_mods678.tsv and add a table field first.
  - quote patch_manager_mods/_shared_src/GameAddr.h:3: `code that must also run on GOG Aspyr reads the same`
  - quote patch_manager_mods/_shared_src/GameAddr.h:69: `constexpr uintptr_t PerformCastSpell = 0x00775700;`
  - quote patch_manager_mods/queue_hud_src/QueueHud.cpp:58: `// Steam Aspyr addresses / offsets (RE.md; the manifest supports the Steam exe only).`

### 97f5356f31c8  patch_manager_mods/_shared_src/SelfHealUi.h:46-49  [none] logic (source-logic)
- Also covers duplicate finding(s): c1437e3479e9 (patch_manager_mods/_shared_src/SelfHealUi.h:46-48)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read SelfHealUi.h 15-32 and 44-49. The 'left of the vitality bar, vertically centred' comment at 19-20 is explicitly scoped to small companion cards, i.e. buttonRect, not leaderButtonRect. Docs 27:278 and 33:240 (SH2, DONE) specify a 58x58 button at the leader portrait's top-left, so the fixed size and anchor are deliberate and documented. The overlap risk with the leader vit bar is a visual question the docs do not address, not a source defect, so the placement claim is refuted.
  - check: At 3840x1600 with live Slot_CardRect (0x54C) and Slot_VitRect (0x694) values, render the leader card and check whether the 58x58 button covers part of the leader vitality bar.
  - check: Decide whether any overlap is acceptable, and confirm SH2 in doc 33 is the current design rather than doc 27:76 ('Leader card gets no button').
  - quote patch_manager_mods/_shared_src/SelfHealUi.h:19: `// Button sits left of the vitality bar, vertically centred.`
  - quote patch_manager_mods/_shared_src/SelfHealUi.h:48: `return Rect{card.x, card.y, 58, 58};`
  - quote patch_manager_mods/27_companion_selfheal.md:278: `Anchor a 58x58 px health button at top-left of player portrait`

### fc0796ea9fce  patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1809-1817  [none] logic (source-logic)
- Haiku: refuted (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read SeDispatch.h 129-145, SaveEventsProvider.cpp 104-116, and ActionHotbar.cpp 1809-1817. Unresolved folders are dropped before dispatch: emitFolder returns at 110-113, and Hub::emit rejects empty folders for non-NEW_GAME kinds (SeDispatch.h:139). So restoreFromSave's empty-folder branch cannot be reached from LOAD_REQUEST, and the claimed 'previous bar stays live with no retry' path does not occur. The save-side deferral covers a missing directory, a different condition the claim does not name.
  - check: In a live log, search for LOAD_REQUEST lines followed by 'save folder not recognised ... not dispatched' (SaveEventsProvider.cpp:111).
  - check: Confirm every 'persave: save loading' line carries a non-empty folder.
  - quote patch_manager_mods/save_events_src/SeDispatch.h:139: `if (!folder \|\| !*folder) return -1;`
  - quote patch_manager_mods/save_events_src/SaveEventsProvider.cpp:110: `if (abs.empty()) {`
  - quote patch_manager_mods/save_events_src/SaveEventsProvider.cpp:111: `save folder not recognised ('%.80s'), not dispatched`

### 10750a5a4474  patch_manager_mods/action_hotbar_src/HotbarLogic.h:89-93  [none] logic (source-logic)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 3/3
- Reasoning: Read HotbarLogic.h:60-110 and hotbar_logic_test.cpp:215-233. The sentinel range is a deliberate, documented Exile criterion: the header lists sentinel ids as a standalone rule (73-75) and test AH3 (227) pins it. The companion-with-sentinel combination is untested (rows use partySlot -1 or 0), a real coverage gap. The tree gives no evidence that a companion ever holds an id in 0xFFFFFFF0..0xFFFFFFFE, so this is an untested assumption, not a demonstrated defect.
  - check: In a live session, grep the companion log lines and the 'character: creature ... invalid object id' KP_LOG lines and confirm companion creature ids never read 0xFFFFFFF0..0xFFFFFFFE.
  - check: Confirm from engine object-id allocation that companion creatures never take ids in that range. If they can, the Exile criterion at HotbarLogic.h:89 is the place to examine.
  - quote patch_manager_mods/action_hotbar_src/HotbarLogic.h:73: `// 2. Player character (Exile): recognized by party slot 0 / isMember0, partyCount == 1, tag "player"/"exile",`
  - quote patch_manager_mods/action_hotbar_src/HotbarLogic.h:89: `bool isExile = ((oid >= 0xFFFFFFF0u && oid <= 0xFFFFFFFEu) \|\| partyCount == 1 \|\| isMember0 \|\| partySlot == 0);`
  - quote patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp:227: `// AH3: full Odyssey negative player sentinel range 0xFFFFFFF0..0xFFFFFFFE maps to canonical Exile key`

### f0b28bacd0ef  patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:775-780  [none] logic (source-logic)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 5/5
- Reasoning: Read CompanionSelfHeal.cpp 740-790 and SelfHealUi.h. The leader card is chosen by fixed HUD index: k is derived from the slot address minus g_hud (759), and the k==0 branch sets g_card0 without an identity or isLeader check (776-780). The companion branch does use isLeader (772), an asymmetry but not a shown defect. Tree docs record that the slot->creature mapping was log-checked for every leader choice (08_shield_bar.md:413) and that the P3 log confirms the slot-to-HUD formula (27:21). Nothing in the tree shows a mismatch.
  - check: In a live session with a leader swap, compare the 'P3 slot[i] ... k=' line for the k=0 slot with the creature that useNow reports as member 0 ('USE member=0' line).
  - check: The 'MAP member=0' log (CompanionSelfHeal.cpp:785) does not log cr, so use the P3 line for this check.
  - quote patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:759: `for (int kk = 0; kk < 3; ++kk) if ((uintptr_t)slot - SlotFromHud - (uintptr_t)kk * SlotStride == g_hud) { k = kk; break; }`
  - quote patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:772: `c.k = k; c.vit = vit; c.seen = !isLeader;`
  - quote patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:776: `if (k == 0) {`

### d56064e2a823  patch_manager_mods/gui_input_src/GuiInputProvider.cpp:158-162  [none] logic (source-logic)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 6/6
- Reasoning: Read GuiInputProvider.cpp 76-127 and 156-165, GuiInput.h:49, ActionHotbar.cpp 1168-1179, GuiKit.h 669-680. The shared contract (GuiInput.h:49) lists 'game window not yet available' as a failure return, so the new early return matches the documented contract. The old code returned a token before any hook existed, which the contract did not promise. In-tree consumers retry: ActionHotbar gates on !subscribed and runs per pass, and GuiKit wheelInstall retries while subOk is false. Loss applies only to an out-of-tree caller that ignores the documented failure.
  - check: In a live session, confirm ActionHotbar's ensureWheelHook (ActionHotbar.cpp:1321) and GuiKit's wheelInstall run every frame, so the retry actually happens.
  - check: Search outside this tree for other k2.guiinput wheel_subscribe callers that subscribe once and never retry on a failure token.
  - quote patch_manager_mods/_shared_src/GuiInput.h:49: `/* Subscribe to mouse wheel events. Returns token > 0 on success, <= 0 on failure (null cb, table full, or game window not yet available). */`
  - quote patch_manager_mods/gui_input_src/GuiInputProvider.cpp:158: `if (!ensureSubclass()) {`
  - quote patch_manager_mods/gui_input_src/GuiInputProvider.cpp:162: `int32_t tok = g_disp.subscribe(cb, user);`

### 89fc8d95002c  patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1539-1545  [none] logic (source-logic)
- Haiku: refuted (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 3/3
- Reasoning: Read LevelUpOverride.cpp 1305-1360 and 1539-1556, plus the hook notes at hooks.toml 321-345. A GUID is adopted only when readGuid finds all 32 names (1543-1544); partial sets are rejected. The new-server hook note states that every new game or load passes through a fresh CServerExoApp, so the globals table starts empty. A 32/32 read can then only come from the load being reported, so the adopted GUID is that save's own identity, not another playthrough's. The result parameter is logged only, which is harmless here.
  - check: Confirm the autosave-restore path (0x8529E0) also reaches 0x401BC0 and so starts with a zeroed globals table.
  - check: Confirm the CServerExoApp constructor zeroes the globals table at +0x100FC before LoadFromSave runs, using the engine or a live log.
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1544: `g_guid = found == 32 ? g : 0; // a partial set (1..31 names) is not a GUID we wrote: treat as none (logged below)`
  - quote patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1543: `if (table && guidEngineOk()) readGuid(table, &g, &found);`
  - quote patch_manager_mods/level_up_override_src/kotor2-steam-aspyr.hooks.toml:340: `# 0x51C450 itself and returns its EAX (nothing after the site reads EAX; ecx is reloaded from [ebp-0xC]). Every new game / load passes here.`

### 3a122c1e3473  patch_manager_mods/level_up_override_src/common_contract_test.cpp:15  [none] build (test-weakening)
- Haiku: refuted (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read common_contract_test.cpp:15, tests/include/check.h:1-3 and scripts/run_tests.py:278-287. check.h exists in tests/include as a forwarder to TestHarness.h, and run_tests.py:278 adds -I tests/include to every host C++ test before the test's own cxxflags. The claim's premise that check.h is missing and kt::Check is reachable only via $HOME is false. Not checked: the windows.h include in FileWatch.h and whether the stub holds windows.h. A clean-checkout build would settle those.
  - check: Run scripts/run_tests.py for common_contract_test in a copy where $HOME/Games/KOTOR2_Mod_Workspace is absent, and confirm it compiles and passes.
  - check: Confirm the windows.h include at FileWatch.h:13-16 sits behind a _WIN32 guard or is supplied by the stub directory.
  - quote patch_manager_mods/level_up_override_src/common_contract_test.cpp:15: `#include "check.h"`
  - quote tests/include/check.h:1: `// check.h - forwarder; canonical header is TestHarness.h (doc 24 T9b).`
  - quote tests/include/check.h:3: `#include "TestHarness.h"`

### 6ef957c705fd  patch_manager_mods/level_up_override_src/test.toml:3-7  [none] test (test-weakening)
- Haiku: refuted (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 3/3
- Reasoning: Read level_up_override_src/test.toml in full (18 lines). Tables [r10_page_test] (line 11) and [r10_filter_test] (line 15) exist, alongside [r10_probe_test] (line 7). Glob confirms r10_page_test.cpp and r10_filter_test.cpp are in the packet. The covers point is weak: run_tests.py:287 defaults covers to [], and the packet requires no k2.portraitslots interface, so a missing covers key has no consumer here. Not a defect.
  - check: Run the runner's verbose or listing mode and confirm cpp:r10_page_test and cpp:r10_filter_test appear as items, not SKIP or excluded.
  - quote patch_manager_mods/level_up_override_src/test.toml:11: `[r10_page_test]`
  - quote patch_manager_mods/level_up_override_src/test.toml:15: `[r10_filter_test]`
  - quote scripts/run_tests.py:287: `cov = cfg.get("covers", [])`

### 0fc98e09742d  patch_manager_mods/mouse_probe_src/MouseProbe.cpp:27  [none] build (source-logic)
- Haiku: refuted (high), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 5/5
- Reasoning: Re-read scripts/build_check.py:10 and stage_mod (209-232), SafeRead.h 37 and 115, and mouse_probe_src/test.toml:4. The build copies patch_manager_mods/_shared_src into a sibling _shared/ directory so that ../_shared/SafeRead.h resolves, and SafeRead.h defines both beginScope and readAt, so the claimed undefined symbols are not the case. The absent committed _shared/ directory is by design. KpLog.h is external per test.toml:4 and is outside this claim. A real compile still needs a build with the KPM checkout.
  - check: Run scripts/build_check.py for mouse_probe_src with KPM_DIR set and confirm the staged _shared/SafeRead.h is present and the compile succeeds.
  - check: Confirm the KPM checkout's Patches/Common contains KpLog.h at the path given in mouse_probe_src/test.toml.
  - quote scripts/build_check.py:10: `(+ a sibling _shared/ copied from patch_manager_mods/_shared_src,`
  - quote scripts/build_check.py:229: `shared = key_dir / "_shared"`
  - quote patch_manager_mods/_shared_src/SafeRead.h:37: `inline void beginScope() {`

### 3c032fcc205b  patch_manager_mods/queue_hud_src/QueueHud.cpp:744  [none] logic (source-logic)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read QueueHud.cpp 744, QueueHudLogic.h 575-584, and the logic tests at 760-802 and 1172-1174. The caller does pass s.paused and the header drops it with (void)paused, but this is deliberate. Test line 1173 says 'paused/now are ignored by the header', and test line 782 records 'no infinite pause re-inject loop'. shouldDispatchNext (QueueHudLogic.h:584) also ignores pause, so buffered dispatch is not pause-gated either. Line 1173 directly pins the paused=true case, so the claim's coverage complaint is wrong.
  - check: Optional: build and run queue_hud_logic_test to confirm the line 1173 assertion passes.
  - check: Optional: check in the engine whether AddrPerformCastSpell holds a re-injected action until unpause. This only matters if the deliberate pause-ignoring is later reconsidered.
  - quote patch_manager_mods/queue_hud_src/QueueHudLogic.h:576: `(void)paused; (void)now;`
  - quote patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1173: `// paused/now are ignored by the header`
  - quote patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:782: `no infinite pause re-inject loop`

### b5a62bbbd646  patch_manager_mods/queue_hud_src/QueueHudLogic.h:583-584  [none] logic (source-logic)
- Haiku: intended-behaviour (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 4/4
- Reasoning: Read QueueHudLogic.h 583-585 and the tests at 1199-1212, plus 30_queue_hud.md 79-80. The haveCur and nrn terms are documented (QH4a): dispatching while haveCur is true makes the server's ClearAllActions wipe the in-flight action. The wait is therefore the intended trade-off, bounded by the 3000 ms round node the doc describes. The claim's 'never dispatch' is not supported by the tree; a permanent stall would need haveCur or nrn to stick, which is runtime. Tests 1210 and 1212 cover both terms in isolation.
  - check: Log s.haveCur and s.nrn in tickItemQueues (QueueHud.cpp:720-784) over a buffered action after a cast completes, to confirm both clear within about 3000 ms when the creature is idle.
  - check: Check whether any state leaves haveCur or nrn set indefinitely while the engine queue is empty.
  - quote patch_manager_mods/queue_hud_src/QueueHudLogic.h:584: `return !buf.active && !buf.empty() && engineQueueLen == 0 && !haveCur && nrn == 0;`
  - quote patch_manager_mods/30_queue_hud.md:79: `Dispatching while `haveCur` is true causes server`
  - quote patch_manager_mods/30_queue_hud.md:80: `must strictly require `engineQueueLen == 0 && !haveCur && nrn == 0``

### 7326b0db3b09  patch_manager_mods/queue_hud_src/kotor2-steam-aspyr.hooks.toml:29  [none] hook (build-config)
- Haiku: refuted (medium), quote ok ratio 1.0; Reduce: unverified/medium; Sonnet quotes ok 6/6
- Reasoning: Read hooks.toml 25-33, GameAddr.h 62-72, QueueHud.cpp 968-976, disasm 33-39, verify_steam_hooks.py 1-30. The values agree: GameAddr PerformCastSpell 0x00775700 + 3 = 0x00775703, and the comment at QueueHud.cpp:972 says so. The disasm shows SUB ESP,0xc at 0x775703, matching original_bytes. The claim's 'no visible check' is wrong: verify_steam_hooks' BYTES check ties address and original_bytes to the exe, though it is manual and not in CI. What remains is hand-synced constants, a latent drift risk, not a present defect.
  - check: Run scripts/verify_steam_hooks.py against the vanilla Steam swkotor2.exe with the Steam hooks.toml and confirm the BYTES and BOUNDARY checks pass at 0x00775703.
  - check: Search for any local release or build script that invokes verify_steam_hooks.py or check_hook_bytes.py, since no CI workflow exists in the tree.
  - quote patch_manager_mods/_shared_src/GameAddr.h:69: `constexpr uintptr_t PerformCastSpell = 0x00775700;`
  - quote patch_manager_mods/queue_hud_src/kotor2-steam-aspyr.hooks.toml:29: `address = 0x00775703`
  - quote patch_manager_mods/queue_hud_src/kotor2-steam-aspyr.hooks.toml:32: `original_bytes = [0x83, 0xEC, 0x0C, 0x89, 0x4D, 0xF4]`

