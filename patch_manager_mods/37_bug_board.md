# 37 — Bug board: verified defects from the p2c deep-haiku-review audit

Source: deep-haiku-review run `p2c` (13 stages, $3.40, pinned tree 8ba3029), tiers CONFIRMED / CONFIRMED-BY-SONNET-ONLY / CONTESTED-NEEDS-HUMAN, triaged for staleness on 2026-10-11 against HEAD efc5335 + working tree
(`.claude/deep-review/p2c/triage/triage.md`, `triage.json`; full audit data `.claude/deep-review/p2c/verified_findings.json`).
This is the standing home for **verified, not-yet-fixed defects** found by audits and reviews. Add new findings as `BGnn` rows (append, never renumber); close a row with `plan_manager.py mark-done BGnn --commit <sha>`.
Doc 15 stays the register for *engine* bug candidates; this board is for defects in *our* code, tests and tooling.

## Batch protocol
1. One BG row = one small fix batch (colocated test or check that fails before, passes after). Fix `scripts/` rows in the repo; mod rows follow the owner's doc.
2. Owner gate `-` = free to take. Anything else: `git status --short <paths>` first, SendMessage the owner session, stop if it is mid-edit.
3. CONTESTED rows (owner gate `user judgment`) need the user's call on whether it is a defect at all before any code change.
4. Before fixing, re-read the cited `file:line`: other sessions edit these files constantly (re-verify with the row's Check step).
5. Never take the audit gate for the user: `findings-skim`, `fix-go`, `fix-pilot` and `fix-apply` are the user's (deep-haiku-review).
6. Fixes touching mod hooks/live behaviour: installs only with user GO (doc 24 rule); commit explicit paths only.
7. Fix-half input: `.claude/deep-review/p2c/fix/verified_findings.live.json` (stale dropped); readiness in `.claude/deep-review/p2c/fix/READY.md`.

## Batch table

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| BG01 | **high** `e606f41d177d` scripts/02_merge_2da_and_tlk.py:234 (logic): 02_merge_2da_and_tlk never reads live planetary.2da; overwrites it and copies genericdoors/soundset over live (other mods' rows lost, run reports success) | - | - | FIXED 4e7a038 (Opus direct, guard + --replace-tables; not run against live) | | On a real install, diff the rows of the live override planetary.2da against the staging Override planetary.2da and list rows from other mods that the staging copy lacks. |
| BG02 | **high** `1ce64f3b2759` scripts/agent_diff_check.py:106-117 (gate-integrity): agent_diff_check passes when `run_tests -k` matches 0 items (rc 0), so a typo'd test filter gives PASS | - | - | FIXED 01ebc67 (tests_ran_ok requires >=1 passed in the run_tests summary; not installed) | | Run `python3 scripts/run_tests.py -k zz_no_such_item; echo $?` from the repo root and confirm exit 0 with a '(0 items)' summary line. |
| BG03 | **high** `119182a14fc2` scripts/plan_manager.py:349-356 (logic): plan_manager mark-done: printed `NN:Tn` ident never matches table row/checkbox; fallback appends a bogus line and reports success | - | - | FIXED f1afc12 (mark-done half was already fixed by 355612d; delegate now strips NN: scope; not installed) | | In a disposable copy of the repo, run `python3 scripts/plan_manager.py mark-done "24:T4"` and diff patch_manager_mods/24_master_plan_status.md. Expect the T4 row unchange... |
| BG04 | **medium** `bc58247fd299` patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:358-364 (logic): ActionHotbar partySlot match by OID alone: sentinel 0xFFFFFFFF id gives a non-Exile the Exile bindings | - | hotbar (doc 07/22) | FIXED e98ecaa (pickPartySlot: pointer first, OID only for valid ids, 0xFFFFFFFF only when one member holds it; ambiguous sentinel -> "" refusal; pinned in hotbar_logic_test, bite-checked in scratch, call site unit-untestable; defensive fix, no live evidence of the shared sentinel; NOT installed) | | Check a session log for 'has invalid object id' (ActionHotbar.cpp:378) with slot=0 while the controlled creature pointer differs from party member 0's. |
| BG05 | **medium** `9e76e8f4642a` patch_manager_mods/gui_input_src/gi_registry_test.cpp:137-144 (test): gi_registry_test no-window wheel_subscribe case only checks a -1 the fake returns itself; reverting the GuiInputProvider fix stays green | - | - | FIXED d3a4109 (gid::subscribeGated extracted so the no-window branch is host-tested in gi_dispatch_test; bite-checked in scratch (8 checks fail with the gate line deleted); gi_registry_test step 7 relabelled as client forwarding, exact -1; apiWheelSubscribe glue unit-untestable; NOT installed) | | Confirm no host test target compiles or links GuiInputProvider.cpp (check the gui_input_src build configuration). |
| BG06 | **medium** `81dabb3f4f5e` patch_manager_mods/mouse_probe_src/MouseProbe.cpp:1-3 (logic): MouseProbe is no longer read-only by default (sanitize_norm=1 writes engine memory) but its header/0.1.0-parity claims say otherwise | - | - | FIXED 29c0610 (header was stale: 0.1.0 read-only, 0.4.0 added sanitizeNorm + SYNTHDBL writes on purpose; header now lists both and the enabled=0 parity condition; default-on kept; comment-only, NOT installed) | | Confirm from version history outside this tree whether 0.1.0 wrote engine memory and what its removed header said. |
| BG07 | **medium** `eee579216787` patch_manager_mods/mouse_probe_src/MouseProbe.cpp:189-193 (bug): MouseProbe discards GetCursorPos/flag-read results, so a failed call uses cursor (0,0): saturated delta / edge warp | - | - | FIXED a4b50d2 (mousefix::sanitizeFrame: failed/unresolved GetCursorPos skips the frame, no write, state untouched; host-tested + bite-checked in scratch; DragFlagAddr read failure only logs; NOT installed) | | Under Wine or Windows, force dyn_GetCursorPos to fail during an RMB camera drag and check the NORM SAT lines and the recentre after MouseProbe.cpp:193-221. |
| BG08 | **medium** `d682bb833922` patch_manager_mods/queue_hud_src/QueueHud.cpp:973-1036 (hook): QueueHud PerformCastSpell hook maps non-party casters to member 0 and buffers/dispatches them | - | queue-hud (doc 30) | FIXED b64a59c (non-party/unreadable-table casters pass through both hooks as dec=pass_nonparty; isPartyCaster pinned, hook edit itself unit-untestable; design call made by the plan, not the author; NOT installed, live check pending) | | Xref or disassemble callers of CSWCCreature::PerformCastSpell (0x00775700, hook at 0x00775703) and look for non-party or AI-driven call sites. |
| BG09 | **medium** `f77ae8184a09` patch_manager_mods/queue_hud_src/QueueHudLogic.h:575-580 (logic): QueueHudLogic shouldReInject returns hasFollowLeader with no completion signal (seenInQueue set but never read) | - | queue-hud (doc 30) | FIXED da75b91 (logic gate on sawExecuting; live check of completed-spell + 0x3D pending, user play test; not installed). Limitation: a wipe AFTER the engine showed the action executing is now treated as completed (AV_EXECUTED), not re-cast. | | Inspect gatherFor (QueueHud.cpp:216-261) to confirm whether a completed spell can still appear in s.e, s.cur or s.rn on the tick it leaves the queue. |
| BG10 | **medium** `035d6f241dd2` research/companion_levelup/tools/parity/gate_test.cpp:301-310 (test): gate_test F4: four CHECKs call LuoLeaderInstant with an unreadable ecx, so the ecx guard answers first and they pass whatever the rules do | - | level-up (doc 25) | FIXED 63425aa (test-only: 4 CHECKs now readable ecx, +1 unreadable-ecx CHECK, mutants GT10-12 kill; PC rule not mutated: no clean one-line mutant) | | In a scratch copy, temporarily drop the mode check at LevelUpOverride.cpp:2024 and run gate_test; the CHECKs at gate_test.cpp:301-310 should still pass, which confirms th... |
| BG11 | **medium** `d2c66b77d42f` research/companion_levelup/tools/parity/scenario_test.cpp:408-409 (gate-integrity): scenario_test X13 `logcap` branch is an empty comment: the scenario never forces the cap | - | level-up (doc 25) | FIXED ad01b78 (test-only: logcap now fills the real KpLog sink to its cap, X13 asserts log.capped=1; LG02/LG03 retired (code gone), LG01 retired (SURVIVED, equivalent: logOpen() constant true); stale AN03/CO16/CO26/RD01 untouched) | | Run mutate.py against this tree (or --only LG02) and confirm it exits with 'STALE mutation LG02' before any mutant runs. |
| BG12 | **medium** `d121f4a74209` research/companion_levelup/tools/parity/unit_gap_test.cpp:48 (gate-integrity): unit_gap_test lost six log-cap CHECKs and nothing replaces them (cap behaviour unpinned) | - | level-up (doc 25) | FIXED 7ccb0c9 (test-only + tooling: unit_gap_test drives the real KpLog cap (1 marker, MAX lines, last, silent after); KpLog cap also pinned in action_hotbar common_contract_test:740 but not in the level-up one; luo_log_check LOG_CAP regex was stale (old "== log line cap reached ==") and is fixed) | | Locate KpLog.h and its test file in the Common tree; confirm they assert exactly one cap marker, the cut-off line, and that writes past the cap emit nothing. |
| BG13 | **medium** `2bb645e71f18` scripts/abi_check.py:362-367 (gate-integrity): abi_check --update can rewrite golden X from another interface's dumper (first abi_dump_*.cpp), destroying the baseline | - | - | DONE | pending | WI-48 verified |
| BG14 | **medium** `6ccefb375bd5` scripts/generate_ultrawide_gui_overrides.py:1680-1684 (gate-integrity): ultrawide registration_check can never report MISMATCH (always-true expression) and its result is discarded | - | ultrawide | FIXED 0e6c4c8 (Haiku worker, Opus-reviewed; not installed) | | Call registration_check(k) for k = 0.7407 and 0.5, and again with the portrait anchor moved to 750, to confirm ok only flips when a REGISTRATION constant is edited. |
| BG15 | **medium** `822afff6842c` scripts/generate_ultrawide_gui_overrides.py:694-696 (gate-integrity): ultrawide generator is_game_running() swallows probe errors and returns False, so deploy/rollback can run while the game is up | - | ultrawide | FIXED d0757ab (probe failure now returns True with a stderr warning; covers generate_ultrawide_textures via shared import; snapshot_live.py has the same fail-open pattern, not changed) | | On the deploy host, run the generator with ps removed from PATH and confirm --deploy proceeds while swkotor2.exe is running. |
| BG16 | **medium** `66c1305db6f7` scripts/subagent_prompt.py:192-218 (logic): subagent_prompt.py mod-source detection picks the first *_src token / 'companion' substring (can route to the wrong source dir) | - | user judgment | FIXED 4030997 (plan-path owner now beats *_src/keyword heuristics, restoring pre-054ee19 ordering; --mod in verif cmd still wins; chosen without author input) | | Run extract_phase_info on a real level-up phase (research/companion_levelup MASTER or PHASE plans) and print mod_scope; check whether 'companion' appears with no *_src to... |
| BG17 | **low** `6a2684bd832c` neocities_mods_detailed.json:10 (other): neocities_mods_detailed.json entry 1 tells readers to override AV for an unsigned exe, no hash or source link | - | - | WONTFIX (upstream guide text in a scraped snapshot, not published by us; only consumer is research/companion_levelup README; hash/HERE link unverified offline; no change) | | Open the live guide entry for 3C-FD and check whether 'HERE' links anywhere; compare with the export that produced neocities_mods_detailed.json. |
| BG18 | **low** `d74365c9d82d` patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:696-697 (logic): CompanionSelfHeal leader `have` is set every refresh and only pk/apk are cleared when false (stale leader HP feeds the button) | - | selfheal (doc 27) | FIXED (leader have now follows refreshCache; slot loop no longer forces it true; stale m/cfg cleared; LEADER gone/back log line; not installed, play check pending) | | Trace a frame where party::members returns 0 while the k==0 HUD slot is still matched. Record g_card0.st and the 'CLICK member=0 noitem' log line on a leader click. |
| BG19 | **low** `6adfec63f6fb` patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:70-73 (logic): Inventory3DViewport shared 4000-line warning cap: per-ShowItem spam can silently swallow later bind-time H1 failures | - | - | DONE | pending | WI-48 verified |
| BG20 | **low** `ce3f285cbcb0` patch_manager_mods/level_up_override_src/test.toml:4 (build): level_up_override test.toml commits a machine-specific -I$HOME/Games/... include path | - | level-up (doc 25) | FIXED (dropped unneeded Patches/Common -I from level_up_override test.toml; builds with -Werror without the external tree; other 23 test.toml files keep the literal, follow-up) | | On a clean checkout without the Patches/Common directory, build common_contract_test with the test.toml flags and -Werror to confirm it compiles. |
| BG21 | **low** `ed6f9f335c3f` patch_manager_mods/mouse_probe_src/MouseProbe.cpp:305-306 (memory-safety): MouseProbe synth_dblclk stores a message id through msgp (line 306) after read-only checks; second undocumented engine write | - | - | FIXED 682320e (probeWrite guard before the store; skip + WARN log when not writable; RE: msg is a WndProc 0x40A750 stack local, msg+4 read once at 0x72E042 then jump-table dispatch so the 0x201->0x203 rewrite reaches axis -1; NOT installed) | | Disassemble WndProc 0x40A750 and OnMouseMessage 0x72E030 to confirm the message struct at [ebp+8] is a per-call writable local. |
| BG22 | **low** `ff23d43b210c` patch_manager_mods/mouse_probe_src/MouseProbe.cpp:220-221 (memory-safety): MouseProbe stores floats through `self` (+0x408/+0x40C) after only a null check, on by default | - | - | FIXED 682320e (probeWrite guard on self+0x408..0x40F before the stores; skip + NORMWRITE log when not writable; NOT installed) | | Disassemble PollMouseDelta (around 0x0072F110) and confirm this+0x408/+0x40C is stored on every path before the hook at 0x0072F2EC, so self is always engine-live there. |
| BG23 | **low** `9059eb31f144` patch_manager_mods/queue_hud_src/QueueHud.cpp:734-739 (logic): QueueHud actionInQueue counts T_CAST_SPELL / subType 9, so a manual spell suppresses re-inject (may be intended per doc 30) | - | queue-hud (doc 30) | FIXED 209de3a (intended per doc 30:80-82; comment + pin test; foreign-cast misattribution documented as known limit; NOT installed) | | Inspect actq::read to confirm whether the same creature's engine queue holds player-issued casts as T_CAST_SPELL entries. |
| BG24 | **low** `b5a7154f89eb` research/companion_levelup/tools/parity/parity.py:33 (build): parity.py hardcodes a ~/Games/... Common include path in every g++ build | - | level-up (doc 25) | FIXED (parity.py/scenarios.py now take Patches/Common from $KPM_DIR, same default; clear error if absent. The include is required: KpLog.h/kpatch_api.h are not in-repo) | | Inspect the workspace Patches/Common directory: does it hold TestHarness.h at its top level? tests/include/TestHarness.h says it is synced from Patches/Common/test/, whic... |
| BG25 | **low** `e3ae385c8f94` research/companion_levelup/tools/parity/stub/LuoLogHost.h:9 (logic): LuoLogHost luoHostLogReset still remove()s the fallback file despite its own comment warning against it | - | user judgment | FIXED (comment-only: SetFallbackFile drops the cached FILE* and reopens lazily, remove() is correct and required; no code change, no KpLog vendoring) | | Check out feat/kplog-header (KpLog.h, about 462 lines, per 19e_phase2_plan.md:282) and read KpLog_SetFallbackFile. See whether it closes the previous FILE* and resets laz... |
| BG26 | **medium** `gem-K1-5a-01` patch_manager_mods/k1_port/k1_port_tex_dxt.py:229 (wrong-behaviour): DXT1 splice skips RGB edits on parents with c0<=c1 since encode_rgb is always 4-colour; Guard is deliberate alpha protection but silently leaves edited blocks unchange... | - | k1 (doc 38) | OPEN | | Write the failing test: test_k1_port_tex_tpc.py: DXT1 parent with c0<=c1 blocks, masked red edit; assert every masked pixel red-shifted. Fails today. |
| BG27 | **medium** `gem-QH-2a-02` patch_manager_mods/queue_hud_src/QueueHud.cpp:782 (wrong-behaviour): engineNe counts resting 0x3D/bare 0x3F, forcing buffering and permanent engine_busy; engineNe counts every entry; ActQueue.h documents 0x3D as a follower's permanent r... | - | queue-hud (doc 30) | OPEN | | Write the failing test: queue_hud_logic_test.cpp: companion queue [0x3D] only must give DECIDE_DISPATCH_NOW / DS_OK; today BUFFER / DS_ENGINE_BUSY. |
| BG28 | **medium** `gem-CO-2d-03` patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707 (wrong-behaviour): keyFor(0) returns slot0 ignoring tag, so settings follow party position not creature after leader swap; keyFor(0) returns slot0 even though c0.tag is read at 886; afte... | - | selfheal (doc 27) | OPEN | | Fix: Key leader by c0.tag (non-empty, incl. 'player') like companions; keep slot0 only as the fallback. |
| BG29 | **medium** `gem-SH-3a-05` patch_manager_mods/_shared_src/GuiKit.h:299 (wrong-behaviour): imageFor pools failed (null) lookups so hudkit::image retry never reloads; imageFor pools null (line 299) so lazy retries in hudkit::image, resolveArrows and ensureTex... | - | - | OPEN | | Write the failing test: hudkit_test.cpp: loader returns null first then a handle; second image(id) call should call loadImage again and return the handle. (compat_check.py required) |
| BG30 | **low** `gem-CO-2c-03` patch_manager_mods/companion_ai_src/CompanionAI.cpp:707 (log-only): CrTrack caches raw creature pointer; stale spinning state and dangling cr after death or area change; e1Evaluate only runs from AiTraceAttackEntry, so spinning stays t... | - | companion-ai (owner session) | OPEN | | Fix: Expire tracks unseen for N seconds in a periodic sweep: clear spinning and emit E1 END. |
| BG31 | **low** `gem-CO-2c-05` patch_manager_mods/companion_ai_src/CompanionAI.cpp:589 (log-only): g_lastCtrlArea overwritten with 0 during transition so PARTY why=area misses; Line 589 overwrites the baseline with 0 on a transient FnGetArea 0, so the next area chan... | - | companion-ai (owner session) | OPEN | | Fix: Store g_lastCtrlArea only when area is non-zero. |
| BG32 | **low** `gem-HN-6h-04` patch_manager_mods/holster_src/Holster.cpp:128 (log-only): Unreadable creature logs false old=0 edge=1 combat transition; rdU32 returns 0 on saferead failure, so an unreadable creature logs old=0 and a bogus edge=1 when new!=0... | - | holster (doc 42) | OPEN | | Fix: Make a read-fail return a sentinel (0xFFFFFFFF) and print old=? edge=? when the read failed. |
| BG33 | **low** `gem-HB-1c-03` patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708 (wrong-behaviour): Stale tooltip kept when new slot name read is empty; tipKey and tipSet update even when setNative is skipped for an empty name, so the prior entry's tooltip persists; ... | - | hotbar (doc 07/31) | OPEN | | Write the failing test: No host seam for updateButtons; extract the tooltip decision into CombatLogic.h and test empty-name clearing. |
| BG34 | **low** `gem-HB-1c-06` patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650 (log-only): Vanilla tint read immediately overwritten; failure branches are dead code; tOk and aOk are forced true right after the reads, so the failure log branch and the 0.85f f... | - | hotbar (doc 07/31) | OPEN | | Fix: Drop the dead tOk/aOk checks and the 0.85f alpha fallback; keep the forced red tint per the :652 comment. |
| BG35 | **low** `gem-QH-2a-07` patch_manager_mods/queue_hud_src/QueueHud.cpp:1253 (wrong-behaviour): Transient g_combatUnreadable overwrites g_prevCombat, firing false combat rising edge; Unreadable frame stores prev=false so next readable combat frame re-fires handof... | - | queue-hud (doc 30) | OPEN | | Write the failing test: Host test needs an extracted edge helper: sequence combat,unreadable,combat must yield one edge; today two. |
| BG36 | **low** `gem-CO-2d-04` patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394 (wrong-behaviour): Pending, auto and cooldown state indexed by party slot not creature; leader swap misattributes; Pending/auto/cooldown state is slot-indexed with no creature check; a s... | - | selfheal (doc 27) | OPEN | | Fix: Store the server creature in Pending and cancel when v[i].server differs. |
| BG37 | **low** `gem-CO-2d-06` patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1176 (log-only): LMB on companion button at FullHp/Pending skips popover close and noneed log; Companion LMB at FullHp/Pending falls through silently and leaves the popover open (it st... | - | selfheal (doc 27) | OPEN | | Fix: Log noneed/pending for companions like the leader branch; closing the popover on a button click is optional UX. |
| BG38 | **low** `gem-CO-2d-09` patch_manager_mods/_shared_src/EmbedTex.h:149 (log-only): heal() logs giving up on the 20th refill even if that fill succeeded; heals counts attempts and ignores fill() result; line 155 fires at heals==20 even when that fill ... | - | - | OPEN | | Write the failing test: embtex_test.cpp: 20th refill succeeds then uploads; assert no giving-up line. Today :93 pins exactly one line. (compat_check.py required) |
| BG39 | **low** `gem-SH-3a-01` patch_manager_mods/_shared_src/GuiKit.h:583 (wrong-behaviour): Wheel callback passes engine screen size as client rect size to clientToEngineF; cw/ch are the provider's engine size (client rect only as fallback), so the INI path m... | - | - | OPEN | | Write the failing test: guikit_test.cpp: wheel event with client 1000x500, screen_w/h 1920x1080, iniW/iniH set; expected engine_x must scale from 1000, not 1920. (compat_check.py required) |
| BG40 | **low** `gem-SH-3a-07` patch_manager_mods/_shared_src/GuiKit.h:174 (wrong-behaviour): Allocated label leaked on createLabelFromTemplate and bindByTag failure paths; Leak real in createLabelFromTemplate and bindByTag:551; bindByTag template/flags failure... | - | - | OPEN | | Write the failing test: guikit_test.cpp: bindByTag with wrong vtable fake; assert alloc released; today leaked. (compat_check.py required) |
| BG41 | **low** `gem-SH-3b-05` patch_manager_mods/_shared_src/SelfHealUi.h:382 (wrong-behaviour): pendingDone2 now==0 sentinel branch untested; Untested sentinel branch is also buggy: storing 1 for now==0 then computing 0-1 wraps and times out at once; one tick per... | - | - | OPEN | | Write the failing test: selfheal_ui_test.cpp: pendingDone2(p, 0, 10, 3, false, &u=0) must be false; today now-1 wraps to 0xFFFFFFFF and returns true. (compat_check.py required) |

## Fix-half intake class (fix_intake.py --stats, 2026-10-11)
- auto-fixable (swarm may take): BG13, BG14, BG19.
- human-only (swarm will not touch; fix by hand): BG01, BG02, BG03, BG09, BG21, BG22, BG23.
- needs-design (user decides the approach first): BG04, BG05, BG06, BG07, BG08, BG10, BG11, BG12, BG15, BG17, BG18, BG24.
- contested (human list): BG16, BG25.

## Triaged out (not on the board)
- `8ea1731f8816` QueueHud stale buffer creature: STALE-FIXED by `df26bd3` (QL1 identity guard). Residual: address reuse (ABA) after free is unverified; reopen as a BG row if a probe shows it.
- `843bd1103025` LevelUpOverride log cap/flush and `5a447a280525` LuoLogHost stale fd: UNCLEAR, both depend on the Common `KpLog.h` core (not in this worktree). Reopen once someone reads KpLog.h.
- 13 REFUTED findings (not triaged, already refuted by Sonnet).
- Audit gaps that still apply: refute-audit `batch_02` never ran; 26 queued verify units never verified (`allow_unverified`); medium-and-below findings beyond the 60 queued were not verified; map packets `other-024` and `other-035` waived. More defects may exist than this board lists.

## Finding details (self-contained; no need to open the audit files)

### BG01 — e606f41d177d (high, CONFIRMED, logic/python-tooling)
- **Where**: `scripts/02_merge_2da_and_tlk.py:234`
- **Claim**: Line 116 reads every live table except planetary.2da (`if n != "planetary.2da"`), so the live file is never opened. Line 234 then does `write_2da(echoes_plan, LIVE_OVERRIDE / "planetary.2da")`, replacing the live table with the staging copy. Lines 235-236 likewise `shutil.copy2` genericdoors.2da and soundset.2da over the live versions. The module docstring (line 4) promises the run 'prevents any table wiping or truncation' and 'Preserves all 154 existing mods'. Why it matters: Any rows that other installed mods added to the live planetary.2da (or changed in genericdoors/soundset) are dropped silently. The run then reports success, which contradicts the stated no-wipe contract.
- **Still real (2026-10-11 triage)**: scripts/02_merge_2da_and_tlk.py:116,234-236 (untracked file) [haiku+orchestrator]
- **Audit evidence**:
  - `scripts/02_merge_2da_and_tlk.py:116` `live = {n: read_2da(LIVE_OVERRIDE / n) for n in table_names if n != "planetary.2da"}`
  - `scripts/02_merge_2da_and_tlk.py:188` `echoes_plan = echoes["planetary.2da"]`
  - `scripts/02_merge_2da_and_tlk.py:234` `write_2da(echoes_plan, LIVE_OVERRIDE / "planetary.2da")`
  - `scripts/02_merge_2da_and_tlk.py:4` `Preserves all 154 existing mods, prevents any table wiping or truncation,`
- **Check steps**: (1) On a real install, diff the rows of the live override planetary.2da against the staging Override planetary.2da and list rows from other mods that the staging copy lacks. (2) Confirm nothing in main() verifies the written tables, despite the 'VERIFIED' print at scripts/02_merge_2da_and_tlk.py:244. (3) Look in pykotor_patches/README.md patch #5 for why line 116 excludes planetary.2da from live reads.
- **Investigate**: Before running, diff the live override planetary.2da, genericdoors.2da and soundset.2da against the staging copies (row count, labels, values). Check whether any installed mod adds planet rows or edits these tables. Confirm the post-run file matches the staging copy and that the pre-run live rows are gone.

### BG02 — 1ce64f3b2759 (high, CONFIRMED, gate-integrity/python-tooling)
- **Where**: `scripts/agent_diff_check.py:106-117`
- **Claim**: run_tests() sets all_ok=False only when `res.returncode != 0` (lines 114-115) and otherwise returns True. It passes the filter as `-k tf` (line 107). In run_tests.py, main() applies -k as a bare substring filter (run_tests.py:861-862). Its 'no test item matched' check covers only --id and positional filters (run_tests.py:932-940), not -k. With zero matching items it prints '0 passed, 0 failed ... (0 items)' and returns 0 (run_tests.py:992, 1004). A mistyped --test value therefore yields test_ok=True and a PASS verdict. A selection that is all SKIP also returns 0. Why it matters: The test gate can be satisfied without running any test. A typo in the filter an orchestrator passes produces a green PASS.
- **Still real (2026-10-11 triage)**: scripts/run_tests.py:862 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `scripts/run_tests.py:862` `items = [i for i in items if a.k in i.id]`
  - `scripts/run_tests.py:885` `if a.list:`
  - `scripts/run_tests.py:932` `# Every requested id / filter must match something; --json output is not written on this exit-2 path.`
  - `scripts/run_tests.py:937` `missing += sorted(s for s in pos_subs if not any(s in m for m in matched_ids)) if a.filters else []`
- **Check steps**: (1) Run `python3 scripts/run_tests.py -k zz_no_such_item; echo $?` from the repo root and confirm exit 0 with a '(0 items)' summary line. (2) Read execute() in scripts/run_tests.py to check whether a selection of only SKIP items also returns 0 (the all-SKIP sub-claim). (3) Check whether any orchestrator reads test_ok or the PASS line without requiring a non-zero test count.
- **Investigate**: From the repo root run `python3 scripts/run_tests.py -k no_such_item_zz; echo rc=$?` and confirm rc=0 with '0 passed'. Then check whether any caller can pass a -k value that is misspelled or empty, and whether agent_diff_check's own verdict relies on that exit code alone.

### BG03 — 119182a14fc2 (high, CONFIRMED, logic/python-tooling)
- **Where**: `scripts/plan_manager.py:349-356`
- **Claim**: Line 350 builds scoped_ident = f"{doc_prefix}:{clean_ident}" (e.g. '25:T4'), and lines 353-356 print it as the phase argument for delegate/resume/handoff/mark-done. cmd_mark_done matches table rows against the bare cell from parse_plan (line 206: ident = cells[id_col_idx], e.g. 'T4') using clean_ident == clean_target or clean_target in clean_ident (lines 531-533). '25:T4' matches neither; the checkbox regex (line 561) misses too, so control falls to lines 570-581, which append '- 25:T4 (done): verified' or '- [x] 25:T4 (completed)' and return. main (lines 785-809) then prints 'Completed: 25:T4' while the real phase row stays open, so the next-phase block still names T4. Why it matters: The printed next-step commands are the normal way to advance a plan. For numbered plans, mark-done reports success but leaves the actual phase row unchanged and appends a bogus line, so the orchestrator keeps re-running the same phase and the plan state silently diverges. cmd_delegate's PASS path (line 488) passes the same string into cmd_mark_done.
- **Still real (2026-10-11 triage)**: scripts/plan_manager.py:531 bare-cell match, scoped 'NN:Tn' ident (:348) never matches; fallback append :578-580 [haiku+orchestrator]
- **Audit evidence**:
  - `scripts/plan_manager.py:750` `resolved_p, _ = resolve_plan_shorthand(args.phase)`
  - `scripts/plan_manager.py:786` `cmd_mark_done(plan, args.phase, commit_sha=args.commit, notes=args.notes, state=args.state)`
  - `scripts/plan_manager.py:533` `if clean_ident.lower() == clean_target.lower() or clean_target.lower() in clean_ident.lower():`
  - `scripts/plan_manager.py:579` `- [x] {target_str} (completed)`
- **Check steps**: (1) In a disposable copy of the repo, run `python3 scripts/plan_manager.py mark-done "24:T4"` and diff patch_manager_mods/24_master_plan_status.md. Expect the T4 row unchanged and a new '- [x] 24:T4 (completed)' line. (2) Check git log -p for 24_master_plan_status.md around lines 153-164 to see whether the existing '- [x] M1 (completed)' lines came from this fallback. (3) Check AGENTS.md / CLAUDE.md to see whether the NN:Tn form is documented as valid mark-done input.
- **Investigate**: In a scratch copy of the repo (not the real plan or .claude dir), create a plan with a table row 'T4' (status PENDING) and a '## Execution log' section. Run `plan_manager.py mark-done T4` and `plan_manager.py mark-done '25:T4'` (with --no-handoff) and diff the status cell and appended lines. Also check phase matching for 'NN:Tn' keys in subagent_prompt.extract_phase_info (scripts/subagent_prompt.py:109) on the delegate path.

### BG04 — bc58247fd299 (medium, CONFIRMED, logic/source-logic)
- **Where**: `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:358-364`
- **Claim**: Loop reads: `uintptr_t mem = party::member(t, i); if (mem == c || (mem != 0 && creatureOid(mem) == oid)) { partySlot = i; break; }`. The OID clause has no guard for the invalid/sentinel ids 0 and 0xFFFFFFFF, and it breaks at the first OID-only match before later slots are checked by pointer. The comment at 381-382 says the Exile's id can read 0xFFFFFFFF for a whole session. The pre-change code (old lines 353-358) matched only by pointer. If the controlled creature is not the Exile but its id also reads 0xFFFFFFFF, slot 0 matches first, so isMember0 is true and HotbarLogic.h:90-93 returns "objfffffffd" (the Exile's bindings) where the old path refused with "" (HotbarLogic.h:92). Why it matters: A controlled character that is not the Exile can be keyed to the Exile's hotbar, so one character's bindings are read or written under another's key. The failure only appears when the sentinel id is shared, which the comment says happens for a whole session.
- **Still real (2026-10-11 triage)**: ActionHotbar.cpp:385 OID-only match; needs live evidence a non-Exile shares sentinel 0xFFFFFFFF [haiku]
- **Audit evidence**:
  - `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:360` `if (mem == c // (mem != 0 && creatureOid(mem) == oid)) {`
  - `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:381` `the id can read 0xFFFFFFFF for a whole session`
  - `patch_manager_mods/action_hotbar_src/HotbarLogic.h:92` `if (oid == 0xFFFFFFFFu && !(partyCount == 1 // isMember0 // partySlot == 0))`
  - `patch_manager_mods/action_hotbar_src/HotbarLogic.h:93` `return "objfffffffd";`
- **Check steps**: (1) Check a session log for 'has invalid object id' (ActionHotbar.cpp:378) with slot=0 while the controlled creature pointer differs from party member 0's. (2) Check whether GetControlledCreature can return a non-Exile (Party.h:51-61) during control switches or cutscenes. (3) Confirm readCreatureTag fails for that creature in that state, since a readable tag bypasses the id logic.
- **Investigate**: In computeCharacterKey, log partySlot together with the match source (pointer or OID) whenever oid is 0xFFFFFFFF. Check whether a non-Exile controlled creature can read 0xFFFFFFFF in the same session as the Exile (Ebon Hawk case). Check whether the OID fallback should be skipped for oid 0 and 0xFFFFFFFF, or whether all pointer matches should be tried before any OID match. computeCharacterKey has no unit test (hotbar_logic_test.cpp covers only characterKeyFor), so a harness or in-game log check is needed.

### BG05 — 9e76e8f4642a (medium, CONFIRMED, test/test-weakening)
- **Where**: `patch_manager_mods/gui_input_src/gi_registry_test.cpp:137-144`
- **Claim**: The new case routes wheel_subscribe to `fakeWheelSubNoWindow` (line 47: `{ ++g_subNoWinCalls; return -1; }`) through `g_fakeTableNoWindow` (line 139). The provider's changed branch (GuiInputProvider.cpp:158-161) never runs. The assertions at lines 142-144 (`noWinTok <= 0`, `g_subNoWinCalls == 1`) only show that the client forwards a -1 that the fake itself returns. Reverting the provider fix would leave this test green. The `<= 0` check also accepts 0. The fake's game_hwnd (fakeGameHwnd, line 39) still returns 0x12345678, so the comment 'game window not yet available' at line 137 describes a state the fake does not model. Why it matters: The bug fix in GuiInputProvider.cpp has no test that can fail if it regresses. The test name ('wheel_subscribe without game window returns token <= 0') claims coverage the test does not provide. The `<= 0` check would also pass on the inert-provider 0 path, so even the client-side assertion is looser than its name.
- **Still real (2026-10-11 triage)**: patch_manager_mods/gui_input_src/gi_registry_test.cpp:137 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `patch_manager_mods/gui_input_src/gi_registry_test.cpp:137` `// 7. Provider present but game window not yet available: wheel_subscribe fails`
  - `patch_manager_mods/gui_input_src/gi_registry_test.cpp:143` `GCHECK("wheel_subscribe without game window returns token <= 0", noWinTok <= 0);`
  - `patch_manager_mods/gui_input_src/gi_registry_test.cpp:144` `GEQ("no-window subCalls count", g_subNoWinCalls, 1);`
  - `patch_manager_mods/gui_input_src/GuiInputProvider.cpp:158` `if (!ensureSubclass()) {`
- **Check steps**: (1) Confirm no host test target compiles or links GuiInputProvider.cpp (check the gui_input_src build configuration). (2) In a scratch copy, force ensureSubclass() to fail and run gi_registry_test; it should still pass, which confirms the gap.
- **Investigate**: Read the test to confirm: step 7 only exercises the client wrapper (GuiInput.h, outside the packet) against a fake. For real coverage, drive apiWheelSubscribe itself in a host build with window discovery and the Hook install seam faked so that ensureSubclass returns false. Then assert exactly -1 and that g_disp.subscribe was not reached. As a cheap check, revert lines 158-161 in a scratch copy and run gi_registry_test. If it still passes, the gap is confirmed.

### BG06 — 81dabb3f4f5e (medium, CONFIRMED, logic/source-logic)
- **Where**: `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:1-3`
- **Claim**: Header line 3 now reads: "The only engine write is the sanitizeNorm path in ProbeMouseNorm (this+0x408/+0x40C), on by default (sanitize_norm=1 in [mouse_fix]...)". The removed text (old lines 1-2) said "STRICTLY READ-ONLY ... it never writes engine memory". Lines 8-10 still promise behaviour that "matches 0.1.0". The default is confirmed in code: MouseProbe.cpp:139 `GetPrivateProfileIntA("mouse_fix", "sanitize_norm", 1, ini)` and MouseFixLogic.h:12 `bool sanitizeNorm = true;`. Why it matters: A diagnostics probe used to measure the camera/mouse bug now mutates the engine's normalised mouse input on every ProbeMouseNorm call by default. The read-only guarantee the investigation depends on is gone, the header and the 0.1.0-parity claim are now false, and nothing in the packet gates the write behind an explicit opt-in.
- **Still real (2026-10-11 triage)**: patch_manager_mods/mouse_probe_src/MouseProbe.cpp:3 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:3` `The only engine write is the sanitizeNorm path in ProbeMouseNorm (this+0x408/+0x40C), on by default`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:306` `*reinterpret_cast<int*>(msgp + 4) = clicklogic::MsgLDbl;`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:140` `g_fixCfg.synthDblClk = GetPrivateProfileIntA(`
  - `patch_manager_mods/mouse_probe_src/MouseFixLogic.h:69` `// Decision helper for double-click synthesis when Wine drops WM_LBUTTONDBLCLK under XWayland.`
- **Check steps**: (1) Confirm from version history outside this tree whether 0.1.0 wrote engine memory and what its removed header said. (2) Decide whether the header should list the SYNTHDBL write at MouseProbe.cpp:306 and whether default-on sanitize_norm is acceptable for a probe that measures the bug. (3) Check the deployed mouse_probe.ini (not in the tree) for sanitize_norm and synth_dblclk values.
- **Investigate**: Decide whether the sanitizer belongs in the diagnostics build at all. Compare doc 18 (patch_manager_mods/18_mouse_camera_investigation.md) and the shipped mouse_probe.ini / manifest.toml / test.toml in the packet directory for the intended default of sanitize_norm. Run the same scenario with sanitize_norm=0 and with the default, and diff the NORM/CAM log lines to see how much the probe changes the input it is measuring.

### BG07 — eee579216787 (medium, CONFIRMED, bug/source-logic)
- **Where**: `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:189-193`
- **Claim**: Line 193: `POINT cur = {0, 0}; if (dyn_GetCursorPos) dyn_GetCursorPos(&cur);`. The return value is discarded, so when the function is unresolved or returns FALSE, the origin (0,0) is used as the real cursor. It feeds computeSanitizedDelta (lines 196-199), checkEdgeWarp (line 209), and g_lastDragPos (line 202). Line 189 `rd(DragFlagAddr, 0, &drag);` also discards its result, so an unreadable flag silently becomes 0. Why it matters: A failed GetCursorPos produces a one-frame saturated delta toward the origin (MouseFixLogic.h:38-39 clamp to +-1), which shows up as a camera spin. While dragging with the window at the screen origin, the (0,0) point matches the edge band (MouseFixLogic.h:59-60), so the sanitizer warps the cursor on every frame.
- **Still real (2026-10-11 triage)**: patch_manager_mods/mouse_probe_src/MouseProbe.cpp:192 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:192` `POINT cur = {0, 0};`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:193` `if (dyn_GetCursorPos) dyn_GetCursorPos(&cur);`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:189` `rd(DragFlagAddr, 0, &drag);`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:209` `if (mousefix::checkEdgeWarp(cur.x, cur.y, rc.left, rc.top, rc.right, rc.bottom, g_fixCfg.edgeMargin, wx, wy)) {`
- **Check steps**: (1) Under Wine or Windows, force dyn_GetCursorPos to fail during an RMB camera drag and check the NORM SAT lines and the recentre after MouseProbe.cpp:193-221. (2) Count real GetCursorPos failures in a session to decide whether the spin is one frame or persistent.
- **Investigate**: Find the conditions where GetCursorPos fails on the target (locked desktop, no input desktop, Wine under XWayland). Stub it to return FALSE while dragging and trace one frame of NORM and CAM output. Check whether the sanitizer should skip the frame and keep the last delta when the read fails, rather than use (0,0). Check the same for a failed DragFlagAddr read.

### BG08 — d682bb833922 (medium, CONFIRMED, hook/source-logic)
- **Where**: `patch_manager_mods/queue_hud_src/QueueHud.cpp:973-1036`
- **Claim**: QueueHudPerformCastSpell returns early only on g_dispatching / !g_cfg.enabled / g_combat != 0 (line 974) and never checks party membership. Line 1013 `if (memberK < 0) memberK = 0;` assigns any non-party caster to member 0. Line 1016 decides on g_itemQueue[memberK]. DISPATCH_NOW stores makeSpellAction(clientCreature, ...) as member 0's activeAction and sets active (1018-1025). BUFFER pushes it and returns 1, so the original PerformCastSpell is skipped (1028-1036). Why it matters: Out-of-combat casts by creatures outside the party (scripted or cutscene casts) can be swallowed or replayed later on that creature, and they block member 0's queue. The item hook has the same fallback (line 939), but the spell hook now applies it to a second action class, which widens its reach.
- **Still real (2026-10-11 triage)**: QueueHud.cpp:1634-1635 `if (fb) memberK = 0;` non-party caster still mapped to member 0 [haiku+orchestrator]
- **Audit evidence**:
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1006` `if (m && (m == static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature)) //`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1013` `if (memberK < 0) memberK = 0;`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:939` `if (memberK < 0) memberK = 0;`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1032` `g_itemQueue[memberK].push(a);`
- **Check steps**: (1) Xref or disassemble callers of CSWCCreature::PerformCastSpell (0x00775700, hook at 0x00775703) and look for non-party or AI-driven call sites. (2) Run with QLOG on and check whether 'buffered spell use for member %d' (QueueHud.cpp:1033) fires for a creature absent from the party table. (3) Check when party::table() returns 0. If only that failure path reaches the line 1013 fallback, the exposure is narrower than 'any non-party caster'.
- **Investigate**: Log clientCreature against the party table inside QueueHudPerformCastSpell during a cutscene or scripted NPC cast out of combat. Check whether the match loop at 1000-1012 ever finds a member for those calls, and whether the item fallback at line 939 has ever fired in a real session.

### BG09 — f77ae8184a09 (medium, CONFIRMED, logic/source-logic)
- **Where**: `patch_manager_mods/queue_hud_src/QueueHudLogic.h:575-580`
- **Claim**: shouldReInject returns hasFollowLeader whenever the action is absent from the engine queue, the buffer is active, and it is not pending, cancelled or over the cap (lines 578-580). The caller takes that branch before the else-if that clears active after 500 ms (QueueHud.cpp:744 and 778). A spell that finished normally while a follow-leader entry (0x3D) is queued therefore looks the same as an eaten one. queue_hud_logic_test.cpp:885-888 labels this exact input state 'wipe' and has no completion signal. Why it matters: Buffered spells are now recast up to kMaxReInjectCount (2) extra times when this misfire occurs. The cap limits the damage, but a completed spell can still be cast a second time.
- **Still real (2026-10-11 triage)**: QueueHudLogic.h:674-679 shouldReInject returns hasFollowLeader, no completion signal [haiku]
- **Audit evidence**:
  - `patch_manager_mods/queue_hud_src/QueueHudLogic.h:580` `return hasFollowLeader;`
  - `patch_manager_mods/queue_hud_src/QueueHudLogic.h:516` `bool reInjectPending = false;`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:735` `if (s.e[i].type == 0x3D) hasFollowLeader = true; // AIActionFollowLeader`
  - `patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:762` `// Follower AI 0x3D wiped queue -> re-inject!`
- **Check steps**: (1) Inspect gatherFor (QueueHud.cpp:216-261) to confirm whether a completed spell can still appear in s.e, s.cur or s.rn on the tick it leaves the queue. (2) Live run with a follower present: cast a buffered spell that completes normally and check the log for 're-injecting eaten spell' (QueueHud.cpp:751) while a 0x3D entry is queued. (3) Consider a test for completion with hasFollowLeader=true, which none of the current tests cover.
- **Investigate**: Trace a follower's engine queue across the tick a buffered spell leaves the queue. Check whether 0x3D is present on that tick after a normal finish, not only during a real wipe. Check whether the engine exposes a completion signal (cur/rn state, or a spell-slot or cost change) that could distinguish a completed action from an eaten one.

### BG10 — 035d6f241dd2 (medium, CONFIRMED, test/test-weakening)
- **Where**: `research/companion_levelup/tools/parity/gate_test.cpp:301-310`
- **Claim**: Line 289: `// F4: the 0x740FC0 guarded read needs a readable ecx`. Line 299: `held -> real object (readable ecx: only the HOLD rule may refuse)`. Yet lines 301 (PC -> real object), 303 (unreadable creature -> real object), 307 (instant=0 -> real object) and 310 (log mode -> real object) all call LuoLeaderInstant_74F423(P(1), P(ebp)), i.e. with an unreadable ecx. The four assertions expect the real object, and the comment on 299 says that with an unreadable ecx some other rule, not the HOLD rule, makes the call return the real object. Why it matters: If the guard on ecx returns the real object before the PC, unreadable-creature, instant=0 and log-mode decisions run, those four CHECKs pass whatever those rules do. They cannot fail, so the leader-instant gate does not test the rules their names describe. This is preexisting in the context lines, not introduced by the packet, but it weakens the gate.
- **Still real (2026-10-11 triage)**: research/companion_levelup/tools/parity/gate_test.cpp:301 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `research/companion_levelup/tools/parity/gate_test.cpp:301` `CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: PC -> real object");`
  - `research/companion_levelup/tools/parity/gate_test.cpp:310` `CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: log mode -> real object");`
  - `research/companion_levelup/tools/parity/gate_test.cpp:97` `static void* P(uintptr_t a) { return reinterpret_cast<void*>(a); }`
  - `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:2037` `if (!probeRead(S_Chain, ecx, 8) // reinterpret_cast<CharChange_t>(g_eng.charChange)(ecx)) return p;`
- **Check steps**: (1) In a scratch copy, temporarily drop the mode check at LevelUpOverride.cpp:2024 and run gate_test; the CHECKs at gate_test.cpp:301-310 should still pass, which confirms they are vacuous. (2) Optionally check git history for gate_test.cpp:301-310 to see whether the P(1) ecx predates this packet (the tree has no .git).
- **Investigate**: In LevelUpOverride.cpp, read LuoLeaderInstant_74F423 and the 0x740FC0 readability guard. Check whether an unreadable ecx returns the real object before the creature, PC, instant and mode checks. If it does, rerun lines 301, 303, 307 and 310 with the readable ctlEcxMem. Then confirm the test fails when the PC rule or the instant=0 rule is mutated in the handler.

### BG11 — d2c66b77d42f (medium, CONFIRMED-BY-SONNET-ONLY, gate-integrity/test-weakening)
- **Where**: `research/companion_levelup/tools/parity/scenario_test.cpp:408-409`
- **Claim**: scenario_test.cpp:408-409 is `} else if (a == "logcap") {` whose only body line is a comment: 'nothing to force here, the scenario still checks the gate is unaffected'. The base line (o408) was `g_logLines = MaxLogLines;`, so the 20000-line cap is no longer forced anywhere in this path. The branch is empty, so the unknown-action guard never fires and the action silently succeeds. scenarios.py:172-173 (X13_log_capped) still runs ['load', 'logcap', 'tick atton', 'auto atton'] and expects only atton.canLevelUp=PASS and atton.pending=REWRITTEN. Both expectations are log-independent: the 'auto' branch comments its pending check as 'log-independent'. mutate.py:63-65 (LG01 'F1 regression: capped log disables the rewrite', LG02 'log cap never reached', LG03 'logOpen ignores the cap') can only be detected when the cap is actually reached. Why it matters: X13 exists to catch the F1 log-cap regression. After this edit it passes whether or not the cap is enforced, so LG01-LG03 are likely to SURVIVE through the scenario gate. The cap check moved to KpLog.h, whose tests are outside this packet, so nothing visible here proves LevelUpOverride still behaves correctly when the log is capped.
- **Still real (2026-10-11 triage)**: research/companion_levelup/tools/parity/scenario_test.cpp:418-419 logcap branch still empty (moved from 408-409) [haiku]
- **Audit evidence**:
  - `research/companion_levelup/tools/parity/scenario_test.cpp:408` `} else if (a == "logcap") { // the 20000-line log cap reached (F1)`
  - `research/companion_levelup/tools/parity/scenario_test.cpp:409` `// the 20000-line cap moved to KpLog.h (Common) and is pinned by its own tests; nothing to force here, the scenario still checks the gate is...`
  - `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:164` `inline bool logOpen() { return true; }`
  - `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:1576` `logOpen(); // F1: the log is diagnostics only - logf never gates the decision and rewrite below (logOpen is a no-op here)`
- **Check steps**: (1) Run mutate.py against this tree (or --only LG02) and confirm it exits with 'STALE mutation LG02' before any mutant runs. (2) Read KpLog.h and its tests in the Common tree to see whether the 20000-line cap is pinned there. (3) Decide whether X13_log_capped should be renamed or dropped, and whether LG01-LG03 should be removed.
- **Investigate**: Run `python3 tools/parity/mutate.py --only LG01` (then LG02, LG03) and check whether each reports SURVIVED or STALE. Grep the parity tests for any other driver of the cap (unit_gap_test.cpp:48 and scenario_test.cpp:409 both say it is 'pinned by its own tests'; check that claim against KpLog's test files). Confirm whether LevelUpOverride.cpp still contains g_logLines / MaxLogLines, since mutate.py:64-65 patterns depend on it. If nothing else covers the cap, the logcap action should drive the cap through the KpLog API instead of being an empty branch.

### BG12 — d121f4a74209 (medium, CONFIRMED, gate-integrity/test-weakening)
- **Where**: `research/companion_levelup/tools/parity/unit_gap_test.cpp:48`
- **Claim**: Known facts record six CHECKs removed (old lines 48-57): logOpen below cap, logf stops at cap, one cap marker, counter == MaxLogLines, logOpen refuses at cap, logLine writes nothing at cap. Post-image line 48 now holds only the comment '// F1 log cap moved to KpLog.h (Common); pinned by KpLog's own tests.' The code those checks exercised no longer lives in LevelUpOverride.cpp: line 164 is 'inline bool logOpen() { return true; }', and logf/logLine (173-190) only call KP_LOG with no cap check. A grep of level_up_override_src finds no g_logLines or MaxLogLines. The only other cap coverage in the parity directory is scenario_test.cpp:408 ('logcap' step), and scenarios.py:173 lists decision expectations only. I did not read the scenario_test.cpp logcap body, so I cannot rule out a marker or line-count check there. Why it matters: No test in this packet now fails if the cap stops at the wrong line, emits zero or two markers, or lets logLine write past the limit. If KpLog's own tests do not pin those exact properties, an F1 regression passes the gate unnoticed.
- **Still real (2026-10-11 triage)**: research/companion_levelup/tools/parity/unit_gap_test.cpp:48 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `research/companion_levelup/tools/parity/unit_gap_test.cpp:48` `// F1 log cap moved to KpLog.h (Common); pinned by KpLog's own tests.`
  - `scripts/luo_log_check.py:155` `("LOG_CAP", r"== log line cap reached ==", "cpp:135"),`
  - `scripts/testdata/kplog/fallback_log.txt:4` `--- log cap reached ---`
  - `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:164` `inline bool logOpen() { return true; }`
- **Check steps**: (1) Locate KpLog.h and its test file in the Common tree; confirm they assert exactly one cap marker, the cut-off line, and that writes past the cap emit nothing. (2) Check VCS history for the pre-change unit_gap_test.cpp lines 48-57 to confirm the six removed CHECKs. (3) Compare the LOG_CAP pattern in scripts/luo_log_check.py:155 with the text KpLog actually writes at the cap.
- **Investigate**: Locate KpLog.h (included at LevelUpOverride.cpp:79) and its test file in the Common tree, which were outside this packet's allowed reads. Check that some test asserts: exactly MaxLogLines lines written, exactly one cap marker, and refusal of further writes. Also read scenario_test.cpp around line 408 for a marker or line-count assertion. Verification: temporarily change the cap constant in KpLog and see whether any test in the suite fails.

### BG13 — 2bb645e71f18 (medium, CONFIRMED, gate-integrity/python-tooling)
- **Where**: `scripts/abi_check.py:362-367`
- **Claim**: When the per-interface dumper is missing, `candidates = sorted((repo_root / "tests/abi").glob("abi_dump_*.cpp"))` and `dumper_src = candidates[0]` take whichever dumper sorts first; the only new effect is a stderr line (367). The update branch (384-388) then does `toml_content = dump_to_toml_str(dumped_data)` and `g_path.write_text(...)` with no comparison of the dumped interface name to the golden. The interface-name check at diff_abi_golden line 157 only runs in non-update mode, so `--update` can overwrite golden X with the name and layout of another interface. Why it matters: Rebaselining a golden from a different interface's dumper silently destroys the baseline for X, and later check runs then compare against the wrong contract. The only signal is a warning printed before 'Updated golden', and the `--header` guard added at 325-327 does not cover this path.
- **Still real (2026-10-11 triage)**: scripts/abi_check.py:366 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `scripts/abi_check.py:366` `dumper_src = candidates[0]`
  - `scripts/abi_check.py:367` `sys.stderr.write(f"No dumper for {iface_tag}; falling back to {dumper_src.name}`
  - `scripts/abi_check.py:384` `if args.update:`
  - `scripts/abi_check.py:386` `g_path.write_text(toml_content, encoding="utf-8")`
- **Check steps**: (1) In a scratch copy, rename a golden's interface to a name with no matching abi_dump file, run abi_check.py --update, and see whether the golden is rewritten with another interface's name. (2) Grep CI or hook configuration for abi_check.py --update invocations to judge whether the update path can run unattended.
- **Investigate**: In a scratch copy of tests/abi, keep a golden whose interface has no abi_dump_<stem>.cpp and run `python3 scripts/abi_check.py --update`; diff the golden against its previous content and confirm the [interface] name came from the other dumper. Then check whether update mode should refuse when the dumped name/major differs from the golden, reusing the comparison at diff_abi_golden line 157, or whether the fallback should be removed.

### BG14 — 6ccefb375bd5 (medium, CONFIRMED, gate-integrity/python-tooling)
- **Where**: `scripts/generate_ultrawide_gui_overrides.py:1680-1684`
- **Claim**: registration_check's `ok` is an always-true expression for any content_k in (0, 1]: `abs(fx(729) - fx(723)) <= 6*k` is exactly 6k, and the two portrait checks (`409 < 583 < 716`, `530 < 583 < 636`) are strict orderings that fx(x,k) = 400 + (x-400)*k preserves for every k > 0. Separately, main discards the boolean at 1835 (`reg, _ = registration_check(content_k)`), so even a real MISMATCH would not affect the deploy. Why it matters: The report header prints 'registration: OK' as if overlay registration had been verified, but the check cannot return MISMATCH for any content aspect. Its docstring says top_p 'must land' on the equip_p/inventory_p geometry, so the stated safety check is a no-op.
- **Still real (2026-10-11 triage)**: generate_ultrawide_gui_overrides.py:1681-1683 check cannot MISMATCH; result discarded :1835 [haiku]
- **Audit evidence**:
  - `scripts/generate_ultrawide_gui_overrides.py:1681` `abs(f["top_p tabs anchor"] - f["equip_p LB_DESC right edge"]) <= 6 * k + 1e-9`
  - `scripts/generate_ultrawide_gui_overrides.py:607` `return 400.0 + (x - 400.0) * k`
  - `scripts/generate_ultrawide_gui_overrides.py:1835` `reg, _ = registration_check(content_k)`
- **Check steps**: (1) Call registration_check(k) for k = 0.7407 and 0.5, and again with the portrait anchor moved to 750, to confirm ok only flips when a REGISTRATION constant is edited. (2) Confirm the printed header line reads 'registration: OK' for any valid --width/--height.
- **Investigate**: Call registration_check with several values of k in (0, 1] (for example 0.1, 0.5, 0.74, 1.0) and confirm ok is always True. Then compare with what the check should measure, such as the engine's rendered positions, to decide whether it needs a different test or should be removed.

### BG15 — 822afff6842c (medium, CONFIRMED, gate-integrity/python-tooling)
- **Where**: `scripts/generate_ultrawide_gui_overrides.py:694-696`
- **Claim**: is_game_running() wraps the ps probe in `except Exception: pass` and then `return False` (694-696). The only caller is the abort gate `if (args.deploy or args.rollback) and is_game_running():` at 1743-1745, so any failure of the probe (ps missing, non-zero exit from check_output, odd output) is read as 'game not running' and deploy/rollback proceed. Why it matters: The module docstring promises 'Aborts deploy if swkotor2.exe is running'. A failing probe silently disables that protection, so --deploy can overwrite override/*.gui while the game holds them open or has them loaded.
- **Still real (2026-10-11 triage)**: generate_ultrawide_gui_overrides.py:694-696 probe swallows exception -> False (untracked) [haiku]
- **Audit evidence**:
  - `scripts/generate_ultrawide_gui_overrides.py:694` `except Exception:`
  - `scripts/generate_ultrawide_gui_overrides.py:1743` `if (args.deploy or args.rollback) and is_game_running():`
  - `scripts/generate_ultrawide_textures.py:2577` `or getattr(args, "world_rollback", False)) and is_game_running():`
- **Check steps**: (1) On the deploy host, run the generator with ps removed from PATH and confirm --deploy proceeds while swkotor2.exe is running. (2) Confirm the same fail-open in generate_ultrawide_textures.py's deploy/rollback gate near line 2577. (3) Run `ps -eo pid,args` under wine and confirm the swkotor2.exe line matches the regex `[s]wkotor2\.exe`.
- **Investigate**: Run the script with --deploy and a PATH where ps is absent (or a shim that exits 1) and confirm it proceeds past line 1743. Check whether the caller can distinguish 'probe failed' from 'not running'; from the code it cannot.

### BG16 — 66c1305db6f7 (medium, CONTESTED-NEEDS-HUMAN, logic/python-tooling)
- **Where**: `scripts/subagent_prompt.py:192-218`
- **Claim**: Line 192 takes the first `[a-zA-Z0-9_]+_src` token in all_context; lines 211-212 `elif "companion" in ctx_lower:` set `companion_ai_src`; the path test `if "companion_levelup" in plan_str or "level_up" in plan_str:` (217-218) now runs last, only when mod_scope is still empty. The pre-change code (o131-o133) tested the plan path first and returned `level_up_override_src/` directly. Why it matters: A level-up plan whose text mentions 'companion' but names no *_src token gets the wrong STRICT boundary (companion_ai_src) and is told NEVER to edit other mods, which excludes the mod the phase is meant to change. A plan that names another mod first in its text also gets that mod. The scope string is injected into the subagent prompt as the edit boundary.
- **Still real (2026-10-11 triage)**: scripts/subagent_prompt.py:211 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `scripts/subagent_prompt.py:211` `elif "companion" in ctx_lower:`
  - `scripts/subagent_prompt.py:214` `# 5d. Plan-level fallback`
  - `scripts/subagent_prompt.py:217` `if "companion_levelup" in plan_str or "level_up" in plan_str:`
  - `research/companion_levelup/MASTER_PLAN.md:11` `User converts every convertible KOTOR II companion to Jedi with a fixed feat`
- **Check steps**: (1) Run extract_phase_info on a real level-up phase (research/companion_levelup MASTER or PHASE plans) and print mod_scope; check whether 'companion' appears with no *_src token. (2) Ask the author whether keyword precedence over the plan-path fallback is intended. (3) Check git history outside this tree for the o131-o133 ordering the claim cites.
- **Investigate**: Build a fixture plan under a path containing companion_levelup whose body mentions 'companion' and no *_src token; call extract_phase_info and inspect the fifth return value. Check whether the precedence change in 31aa40c/054ee19 was intended (commit messages are not in this snapshot).

### BG17 — 6a2684bd832c (low, CONFIRMED-BY-SONNET-ONLY, other/build-config)
- **Where**: `neocities_mods_detailed.json:10`
- **Claim**: Entry 1 notes (line 10): "When attempting to run this executable, Windows/your antivirus might attempt to block it, because it's an unsigned executable ... please feel assured that this program is safe, and you can override Windows/your AV and run it." The same entry tells users to run an executable that rewrites their game binary ("select your game's executable and choose to apply all patches"), gives no hash or signature to check, and the source link is the bare word "HERE" with no URL. Why it matters: Published install guidance that tells readers to bypass an antivirus warning on an unsigned executable that modifies the game binary weakens a user-side safety check, and gives readers no way to verify the download.
- **Still real (2026-10-11 triage)**: neocities_mods_detailed.json:10 still advises AV override for unsigned exe ('HERE' link not re-checked) [haiku]
- **Audit evidence**:
  - `neocities_mods_detailed.json:10` `When attempting to run this executable, Windows/your antivirus might attempt to block it, because it's an unsigned executable.`
  - `neocities_mods_detailed.json:10` `the source code is publicly available HERE --please feel assured that this program is safe`
  - `neocities_mods_detailed.json:10` `you can override Windows/your AV and run it.`
  - `neocities_mods_detailed.json:10` `select your game's executable and choose to apply all patches except the borderless windowed patch`
- **Check steps**: (1) Open the live guide entry for 3C-FD and check whether 'HERE' links anywhere; compare with the export that produced neocities_mods_detailed.json. (2) Check the deadlystream file 2734 page for a published SHA-256 or signature and for a linked source repository. (3) Decide editorially whether telling readers to override an AV block on an unsigned binary should stay.
- **Investigate**: Compare this notes paragraph with the source guide it was copied from to confirm the AV-override sentence is intended as published text. Check whether the 3C-FD source page publishes a hash or signature that the guide could cite. This is a content review; no code path is involved.
- **Resolution**: closed as not-ours; if we ever republish 3C-FD install guidance, cite the deadlystream file 2734 hash instead of AV-override wording.

### BG18 — d74365c9d82d (low, CONFIRMED, logic/source-logic)
- **Where**: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:696-697`
- **Claim**: refreshCache sets `c0.have = 0 < cnt && v[0].ok;` (696) and clears only pk/apk when false (697). uiFrame then re-asserts `g_card0.have = true;` on every frame the slot exists (777). Nothing in the packet sets have back to false: hideAll (554-561) resets only seen and st. c0.m is never cleared, and c0.m.cur and c0.m.max feed resolveState at 829. Why it matters: After the leader's party entry disappears (cnt==0 or v[0].ok false, e.g. during teardown or load), the leader button is still drawn with HP from the last good refresh. Clicks reach useNow, which refuses with 'member not in party' (307), so the button looks live but does nothing. The companion path (674-675) does honour its have gate, so the two paths diverge. The comment 'always have if slot exists' suggests the bypass is deliberate, but the stale data is not bounded.
- **Still real (2026-10-11 triage)**: CompanionSelfHeal.cpp:770,852 leader `have` never cleared (moved from 696/777) [haiku]
- **Audit evidence**:
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:696` `c0.have = 0 < cnt && v[0].ok;`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:697` `if (!c0.have) { c0.pk = Pick(); c0.apk = Pick(); } else {`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:777` `g_card0.have = true;`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:891` `if (shui::clickAllowed(c.st)) {`
- **Check steps**: (1) Trace a frame where party::members returns 0 while the k==0 HUD slot is still matched. Record g_card0.st and the 'CLICK member=0 noitem' log line on a leader click. (2) Confirm no 'REFUSE member 0 not in party' line appears for the leader in that state, since the stale state resolves to NoItem.
- **Investigate**: Check shui::resolveState with have=true, pk.pick<0 and cur==max (and with stale cur/max) to see which look is shown when the leader is gone. Check whether v[0].ok can be false while the HUD slot k==0 still exists. Compare with the companion have semantics at 674-675, and consider whether c0.m should be cleared when the refresh finds no leader.
- **Resolution**: root cause was CompanionSelfHeal.cpp slot loop (`g_card0.have = true`) overwriting the refreshCache result every frame, so a lost leader kept a live-looking inert button (useNow refuses `member 0 not in party`). Now `have` follows refreshCache (`0 < cnt && v[0].ok`), the slot loop only maps/`seen`s the leader while `have`, stale `m`/`cfg` are cleared, and an edge-triggered `LEADER gone/back` log line is emitted. Companion path already honored `have`. Built + scoped selfheal suites PASS; not installed, play check pending.

### BG19 — 6adfec63f6fb (low, CONFIRMED, logic/source-logic)
- **Where**: `patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:70-73`
- **Claim**: Line 70: `constexpr int MaxWarnLines = 4000;` and lines 71-73: `void logw(...) { if (g_warnLines >= MaxWarnLines) return; ++g_warnLines;`. Once the counter reaches 4000 every later warning is dropped with no 'suppressed' marker. logw is shared by one-off H1 bind-time failures (lines 400, 447-453, 463, 465) and by per-call H2 warnings in ShowItem (lines 479, 485, 488, 503, which fire on every ShowItem while a slot is unusable, as the comment on line 70 says). Compare loge() (lines 77-80), which has no cap. Why it matters: The H2 per-ShowItem spam can spend the shared warning budget. After that, a later bind-time H1 failure (the most diagnostic warning) is silently discarded, and the log gives no sign that anything was lost. The cap trades log size for silent loss of the most useful lines.
- **Still real (2026-10-11 triage)**: patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:70 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:70` `constexpr int MaxWarnLines = 4000; // warnings can fire per ShowItem call while a slot stays unusable`
  - `patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:72` `if (g_warnLines >= MaxWarnLines) return;`
  - `patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:447` `if (id != ExpectedControlId) { logw("H1: GUI control missing (id=%d), slot unusable", id); return; }`
  - `patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:479` `if (!s->usable) { logw("H2: slot unusable (panel=%08x)", panel); return; }`
- **Check steps**: (1) In a long session log (inventory-3d-viewport_log.txt, not in the tree), count 'H2: slot unusable' lines against the 4000 cap and check whether any later 'H1' line is missing. (2) Check how often the item-selection hook (0x008A8172) fires on an unusable slot in a real session.
- **Investigate**: Check how many logw() calls one unusable slot produces per ShowItem (H2 path, lines 479-503) and whether a long session can reach 4000 before an H1 failure (lines 400-465) occurs. Check whether the cap path ever writes a marker line. Compare with the logf() cap at lines 60-68, which is also silent, to see whether this follows an existing pattern or adds a new failure mode. Verify in the log file that the last warning before silence is an H2 line.

### BG20 — ce3f285cbcb0 (low, CONFIRMED-BY-SONNET-ONLY, build/build-config)
- **Where**: `patch_manager_mods/level_up_override_src/test.toml:4`
- **Claim**: test.toml:4 adds "-I$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common" to the committed [common_contract_test] cxxflags. This is a per-user absolute location outside this repo; the in-repo shared headers are the first entry ("-I../_shared_src"). The comment at test.toml:1 says the test needs 'the shared headers and the host Win32 stub' and does not mention a Patches/Common include dir. TOML strings are literal, so the file itself does not expand $HOME. The identical literal appears at patch_manager_mods/shield_bar_src/test.toml:6, whose comment says the Common dir supplies PortraitSlots.h -> kpatch_api.h. A Glob for **/kpatch_api.h under patch_manager_mods/_shared_src returned no files. Why it matters: On any other checkout or CI machine the -I either resolves to nothing (gcc/clang silently ignore a missing -I dir) or to a literal directory named '$HOME' if the runner does not shell-expand. If kpatch_api.h is needed and not found, -Werror builds fail. On the author's machine the contract test can compile against an external copy of the shared headers outside this repo, so it may pass while testing code that differs from what is committed here, which defeats the 'any edit to the shared headers re-runs it' purpose stated in common_contract_test.cpp:5.
- **Still real (2026-10-11 triage)**: patch_manager_mods/level_up_override_src/test.toml:4 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `patch_manager_mods/level_up_override_src/test.toml:4` `cxxflags = ["-I../_shared_src", "-I$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common", "-I../../research/companion_l...`
  - `patch_manager_mods/level_up_override_src/test.toml:1` `# Per-test table: the Common contract test needs the shared headers and the host Win32 stub (mirrors shield_bar_src/test.toml).`
  - `patch_manager_mods/level_up_override_src/common_contract_test.cpp:11` `#include "GuiTip.h"`
  - `patch_manager_mods/_shared_src/GuiKit.h:25` `#include "WindowHook.h"`
- **Check steps**: (1) On a clean checkout without the Patches/Common directory, build common_contract_test with the test.toml flags and -Werror to confirm it compiles. (2) Ask the author whether the Patches/Common entry was copied from shield_bar_src/test.toml on purpose, since this test has no PortraitSlots.h dependency.
- **Investigate**: Find the test runner's handling of cxxflags: does it run through a shell or expand environment variables, or does it pass argv directly? Do a clean checkout without the external KOTOR2_Mod_Workspace tree and build common_contract_test with -Werror; see whether kpatch_api.h or any other header that only Patches/Common provides is missing. Check whether kpatch_api.h exists anywhere under patch_manager_mods/ (the glob of _shared_src found nothing). Diff the external Patches/Common headers against the in-repo _shared_src headers that common_contract_test.cpp includes (GuiTip.h, GameAddr.h, Party.h, FileWatch.h). Check whether the comment at test.toml:1 should be updated to name the Common dir.
- **Resolution**: removed the copy-pasted external include; common_contract_test compiles against in-repo _shared_src only. Same literal remains in 23 other test.toml (they may need kpatch_api.h); candidate follow-up: switch them to ${KPM_DIR}/Patches/Common.

### BG21 — ed6f9f335c3f (low, CONFIRMED-BY-SONNET-ONLY, memory-safety/source-logic)
- **Where**: `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:305-306`
- **Claim**: Lines 288-290 validate msgp only through reads: `rd(msgp, 4, &id)` and `rd(msgp, 0xC, &lp)`. Line 306 then stores through the same pointer: `*reinterpret_cast<int*>(msgp + 4) = clicklogic::MsgLDbl;`. The gate at line 305 is enabled by default (line 140 `synth_dblclk` defaults to 1). Header line 3 says the sanitizeNorm path is "the only engine write", which this contradicts. The file-wide grep found no other stores to engine memory besides lines 220, 221 and 306. Why it matters: This is a second, undocumented engine write into a game-owned message struct. A read-only mapping, a struct the game reuses after dispatch, or a pointer that changes between the read and the store would fault or push a forged message id into the game.
- **Still real (2026-10-11 triage)**: patch_manager_mods/mouse_probe_src/MouseProbe.cpp:286 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:286` `if (!rd(ebp, 8, &msgp)) return;`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:288` `if (!rd(msgp, 4, &id) // !clicklogic::isMouseButtonMsg(id)) return;`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:306` `*reinterpret_cast<int*>(msgp + 4) = clicklogic::MsgLDbl;`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:17` `live in ClickLogic.h (pure, tested).`
- **RE result (2026-10-12, rea-recon)**: msg struct is a per-call stack local of WndProc FUN_0040a750 (0x72FBD0 only forwards the pointer, `RET 4`, no reads after the call). OnMouseMessage reads msg+4 once (0x72E042) then dispatches via jump table 0x72E700 (0x201->0x72E0C6 sets axis +1, 0x203->0x72E172 sets -1 at 0x72E200), so the SYNTHDBL rewrite reaches the dispatch. Open: only 1 of 10 WndProc call sites decoded. Fixed in 682320e (probeWrite guard).
- **Check steps**: (1) Disassemble WndProc 0x40A750 and OnMouseMessage 0x72E030 to confirm the message struct at [ebp+8] is a per-call writable local. (2) Check whether the engine re-reads msg+4 after the 0x72E039 hook, which decides whether the rewrite reaches dispatch. (3) Run with synth_dblclk=1 on a Wine session that drops WM_LBUTTONDBLCLK and check the SYNTHDBL log lines and any access violation at the hook.
- **Investigate**: At 0x72E039 (OnMouseMessage), check that ebp+8 points to writable memory owned by the dispatcher for the duration of the call, and whether msg+4 is read again after the probe returns. Confirm clicklogic::MsgLDbl equals 0x203 in ClickLogic.h. Confirm the gate at line 305 (shouldSynthesizeDblClick in MouseFixLogic.h:70-72) can only fire for id 0x201. Correct the header claim in line 3.

### BG22 — ff23d43b210c (low, CONFIRMED, memory-safety/source-logic)
- **Where**: `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:220-221`
- **Claim**: Line 184 only rejects a null pointer: `if (!rd(ebp, -0x28, &self) || self == 0) return;`. Lines 220-221 then store through that pointer with no readability or writability check: `*reinterpret_cast<float*>(self + 0x408) = fixedNx;` / `... self + 0x40C = fixedNy;`. This runs by default (line 139). The only SafeRead-checked access to the same addresses is the read-back at line 226, after the store. Header line 2 says every hook access goes through SafeRead. Why it matters: If [ebp-0x28] holds a stale, freed or garbage pointer on any call of this hook, a per-frame store lands in arbitrary memory and can crash the game or corrupt the engine's input object. Before this change, every access in the probe was guarded.
- **Still real (2026-10-11 triage)**: patch_manager_mods/mouse_probe_src/MouseProbe.cpp:184 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:184` `if (!rd(ebp, -0x28, &self) // self == 0) return;`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:220` `*reinterpret_cast<float*>(self + 0x408) = fixedNx;`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:226` `if (!rd(self, 0x408, &nx) // !rd(self, 0x40C, &ny)) return;`
  - `patch_manager_mods/mouse_probe_src/MouseProbe.cpp:2` `Hooks read engine memory through SafeRead and`
- **Check steps**: (1) Disassemble PollMouseDelta (around 0x0072F110) and confirm this+0x408/+0x40C is stored on every path before the hook at 0x0072F2EC, so self is always engine-live there. (2) Confirm no path reaches the 0x0072F2EC detour without the engine having dereferenced [ebp-0x28] first.
- **Investigate**: At the hook site 0x72F2EC (doc 18), confirm that [ebp-0x28] always holds a live engine object on every call, including load, menu and world transitions where the object may be recreated. Check that the object is at least 0x410 bytes. Check saferead.h for a write-checked helper and whether the read at line 226 could be moved before the store. Run with sanitize_norm=0 on the same build and compare crash or hang behaviour.

### BG23 — 9059eb31f144 (low, CONFIRMED-BY-SONNET-ONLY, logic/source-logic)
- **Where**: `patch_manager_mods/queue_hud_src/QueueHud.cpp:734-739`
- **Claim**: actionInQueue is set for any T_CAST_SPELL engine entry (line 734), for cur.subType 9 (line 737) and for rn subType 9 (line 739). It feeds shouldReInject (QueueHudLogic.h:579 `if (actionInQueue) return false;`) and the else-if clear (line 778). The item-only predicate became spell-inclusive, and the subType 9 = cast-spell mapping appears only in the comment on line 737. Why it matters: Any spell the same creature casts itself (a manual cast by the player) keeps buf.active set and suppresses recovery of an eaten buffered action. That recovery loss happens in normal use.
- **Still real (2026-10-11 triage)**: QueueHud.cpp:1162-1167 actionInQueue includes T_CAST_SPELL (spec in 30_queue_hud.md may intend it) [haiku]
- **Audit evidence**:
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:734` `if (s.e[i].type == qh::T_USE_ITEM // s.e[i].type == qh::T_CAST_SPELL) actionInQueue = true;`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:737` `if (s.haveCur && (s.cur.subType == 9 // s.cur.subType == 10)) actionInQueue = true;`
  - `patch_manager_mods/queue_hud_src/QueueHudLogic.h:579` `if (actionInQueue) return false;`
  - `patch_manager_mods/queue_hud_src/QueueHudLogic.h:74` `if (n.subType == 9) id = n.spellId;`
- **Check steps**: (1) Inspect actq::read to confirm whether the same creature's engine queue holds player-issued casts as T_CAST_SPELL entries. (2) During a manual cast with buf.active set, log actionInQueue (QueueHud.cpp:734-740) and the shouldReInject result (QueueHud.cpp:744), and check whether re-inject stays blocked after the manual cast ends. (3) Check version history outside this tree for when line 734 widened from item-only, to see whether own-cast suppression was considered.
- **Investigate**: Confirm the subType enum (9 = cast spell, 10 = use item) in the engine or actq header. Then reproduce: buffer a spell, cast a different spell manually while the buffered one is eaten, and check whether re-inject is skipped and buf.active stays true.

### BG24 — b5a7154f89eb (low, CONFIRMED, build/python-tooling)
- **Where**: `research/companion_levelup/tools/parity/parity.py:33`
- **Claim**: parity.py:33 adds `"-I", os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common")` to the g++ argv of every build() call (gate, apply, ui, playthrough, m32). The base line had no such -I. gate_test.cpp:21 and apply_test.cpp:86 `#include "TestHarness.h"`, but Glob `**/TestHarness.h` under the parity tree returns nothing, and literal probes of stub/_shared/TestHarness.h and stub/inc/TestHarness.h also return nothing. build() at parity.py:35-36 calls sys.exit(stderr) on a compile failure. Why it matters: A machine-specific absolute home-directory path is now on the build line of the gate. If that path is the only thing that supplies TestHarness.h, parity.py cannot compile gate_test.cpp on any other checkout or CI runner, and the gate depends on an unversioned file outside the repository. The R10 builds at parity.py:524 and 533 use a repo-relative `tests/include` instead, which shows the in-repo convention.
- **Still real (2026-10-11 triage)**: research/companion_levelup/tools/parity/parity.py:33 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `research/companion_levelup/tools/parity/parity.py:33` `os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common")`
  - `research/companion_levelup/tools/parity/parity.py:36` `sys.exit(r.stderr)`
  - `research/companion_levelup/tools/parity/gate_test.cpp:22` `#include "TestHarness.h"`
  - `research/companion_levelup/tools/parity/parity.py:430` `build(GATE_EXE, "gate_test.cpp")`
- **Check steps**: (1) Inspect the workspace Patches/Common directory: does it hold TestHarness.h at its top level? tests/include/TestHarness.h says it is synced from Patches/Common/test/, which is a different path from the -I target. (2) Run the gate's build line with the home path absent (for example `g++ -std=c++17 -fsyntax-only` with the stub dirs) to confirm the compile failure on gate_test.cpp.
- **Investigate**: On a checkout without the ~/Games tree, rebuild gate_test.cpp with the exact argv that build() produces (drop only the home -I) and confirm the first error is 'TestHarness.h: No such file'. Then locate where TestHarness.h is actually versioned and point the -I at a repo-relative directory. Caveat: my probes of the repo-root tests/include and patch_manager_mods locations used '..' glob patterns, so their negative results may be unreliable; the stub/ probes and the **/ search are reliable.
- **Resolution**: the -I is NOT removable: LevelUpOverride.cpp includes KpLog.h -> kpatch_api.h, present only in external Patches/Common. TestHarness.h concern was moot (stub/inc/TestHarness.h is in-tree). Hardcoded ~ path replaced by $KPM_DIR (default unchanged) in parity.py and scenarios.py with a missing-dir guard.

### BG25 — e3ae385c8f94 (low, CONTESTED-NEEDS-HUMAN, logic/source-logic)
- **Where**: `research/companion_levelup/tools/parity/stub/LuoLogHost.h:9`
- **Claim**: Lines 3-4 say: "KpLog also keeps the fallback file open, so remove()ing it leaves later writes on an unlinked inode: reset through the public API instead." Line 9 still does `static inline void luoHostLogReset(const char* path) { std::remove(path); KpLog_SetFallbackFile(path); }`, so it calls the remove() that the comment warns against, before the public call. Why it matters: If KpLog_SetFallbackFile() returns early or reuses the existing FILE* when the path is unchanged, the unlink at line 9 detaches the file the logger still writes to. Later INFO lines then go to an unlinked inode, the file at `path` stays empty or missing, and tests that grep it either fail spuriously or, if they assert absence of a line, pass without proving anything. Either the comment or the code is wrong, and the test-host reset is the only thing that makes these log-grep tests deterministic.
- **Still real (2026-10-11 triage)**: research/companion_levelup/tools/parity/stub/LuoLogHost.h:3 quote verbatim present; no commits or edits since 7020d54 [prefilter]
- **Audit evidence**:
  - `research/companion_levelup/tools/parity/stub/LuoLogHost.h:3` `KpLog also keeps the fallback file open, so remove()ing it leaves later writes on an`
  - `research/companion_levelup/tools/parity/stub/LuoLogHost.h:9` `static inline void luoHostLogReset(const char* path) { std::remove(path); KpLog_SetFallbackFile(path); }`
  - `patch_manager_mods/19e_phase2_plan.md:106` `Uses the fallback file if one was set, opened lazily.`
  - `patch_manager_mods/robustness_fixes_src/common_contract_test.cpp:124` `KpLog_SetFallbackFile only stores the path, no I/O (safe from a static initialiser)`
- **Check steps**: (1) Check out feat/kplog-header (KpLog.h, about 462 lines, per 19e_phase2_plan.md:282) and read KpLog_SetFallbackFile. See whether it closes the previous FILE* and resets lazy-open state when the path is unchanged, or returns early. (2) Run research/companion_levelup/tools/parity gate_test or scenario_test. Compare 'ls -li' on levelup_override_log.txt before and after clearLog() plus one INFO line, to see whether later writes land on the unlinked inode.
- **Investigate**: Locate KpLog.h (it is not in this packet directory; find it through the include in LevelUpOverride.cpp) and read KpLog_SetFallbackFile(). Check whether it fcloses the previous handle and fopens `path` fresh on every call, including when `path` equals the currently configured path. If it does not reopen in that case, the remove() at line 9 orphans the log and the reset is broken. If it does reopen unconditionally, the remove() only clears old content and the comment at lines 3-4 is misleading; then fix the comment or drop the redundant order dependency. To check the impact, find the tests that call luoHostLogReset and see whether any assert on the absence of a log line or on an empty file, and run one such test with a log path that already exists from a previous run.
- **Resolution** (2026-10-12): remove() is correct and required, the comment was wrong. `KpLog.h:423` `KpLog_SetFallbackFile` stores the path, nulls `gFile` (old FILE* leaked on purpose) and clears `gOpenFailed`; `KpLog.h:197` `OpenFallbackFile` reopens with `"a"` on the next write, so the file is not orphaned, and append mode keeps old lines without the remove(). The old comment claimed the opposite. Fixed the comment in `LuoLogHost.h` only; `KpLog.h` lives at `$KPM_DIR/Patches/Common/KpLog.h`, not vendored.

<!-- BG26-BG41: Gemini 2026-10-14 review, G2-validated (.claude/reviews/gemini_20261014/validated.tsv, gate g2/gate.md) -->

### BG26 — gem-K1-5a-01 (medium, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/k1_port/k1_port_tex_dxt.py:229`
- **Claim** (Gemini 2026-10-14, untrusted until checked): DXT1 splice skips RGB edits on parents with c0<=c1 since encode_rgb is always 4-colour
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, medium. Guard is deliberate alpha protection but silently leaves edited blocks unchanged whenever parent is 3-colour (c0<=c1, incl. flat blocks); encoder is always 4-colour. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/k1_port/k1_port_tex_dxt.py:229` `ch = ch & ((p0 <= p1) == (n0 <= n1))`
  - `patch_manager_mods/k1_port/k1_port_tex_dxt.py:168` `"""(h,w,3) uint8 -> (by,bx,8) uint8 DXT1 colour halves, always 4-colour mode (c0 > c1).`
- **Check steps**: test_k1_port_tex_tpc.py: DXT1 parent with c0<=c1 blocks, masked red edit; assert every masked pixel red-shifted. Fails today.
- **Investigate**: fix sketch: Encode changed blocks of c0<=c1 parents in 3-colour mode (keep index 3 pixels) instead of skipping them, or count and report skipped blocks. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (K1-5a-01).
- **Resolution**:

### BG27 — gem-QH-2a-02 (medium, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/queue_hud_src/QueueHud.cpp:782`
- **Claim** (Gemini 2026-10-14, untrusted until checked): engineNe counts resting 0x3D/bare 0x3F, forcing buffering and permanent engine_busy
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 PARTIAL -> skeptic real=yes, wrong-behaviour, medium. engineNe counts every entry; ActQueue.h documents 0x3D as a follower's permanent resting action so companions always read engq>=1. Logs only cover m=0. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:782` `return ok ? s.engineNe : -1;`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1475` `qh::DispatchWhy dsWhy = qh::dispatchWhy(buf, s.engineNe, creal, s.nrn);`
  - `patch_manager_mods/_shared_src/ActQueue.h:27` `constexpr int T_FOLLOW_LEADER = 0x3D;  // FollowLeader: a follower's permanent resting action; its handler 0x5BF7F0 re-adds itself at the head (doc 29 D5)`
- **Check steps**: queue_hud_logic_test.cpp: companion queue [0x3D] only must give DECIDE_DISPATCH_NOW / DS_OK; today BUFFER / DS_ENGINE_BUSY.
- **Investigate**: fix sketch: Count only non-0x3D entries (and bare 0x3F with no round nodes) in engineNe for decideAction/dispatchWhy, like makeEngineView ignores them. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (QH-2a-02).
- **Resolution**:

### BG28 — gem-CO-2d-03 (medium, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707`
- **Claim** (Gemini 2026-10-14, untrusted until checked): keyFor(0) returns slot0 ignoring tag, so settings follow party position not creature after leader swap
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, medium. keyFor(0) returns slot0 even though c0.tag is read at 886; after a leader swap the new leader inherits the previous leader's settings. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707` `if (mi == 0) return "slot0";`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:887` `c0.cfg = shui::lookup(g_cfg, keyFor(0));`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Key leader by c0.tag (non-empty, incl. 'player') like companions; keep slot0 only as the fallback. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2d-03).
- **Resolution**:

### BG29 — gem-SH-3a-05 (medium, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/_shared_src/GuiKit.h:299`
- **Claim** (Gemini 2026-10-14, untrusted until checked): imageFor pools failed (null) lookups so hudkit::image retry never reloads
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, medium. imageFor pools null (line 299) so lazy retries in hudkit::image, resolveArrows and ensureTextures hit the pooled null and never reload. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/GuiKit.h:299` `memset(&pool[n], 0, sizeof pool[n]); strncpy(pool[n].name, buf, 16); pool[n].im = im; ++n;`
  - `patch_manager_mods/_shared_src/HudKit.h:45` `if (!cache[id]) cache[id] = guikit::imageFor(detail::kHudKitTextures[id].resref);`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:504` `if (!g_arrowNeg) g_arrowNeg = guikit::imageFor("uibit_arrowneg_p");`
- **Check steps**: hudkit_test.cpp: loader returns null first then a handle; second image(id) call should call loadImage again and return the handle. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: Do not pool null loadImage results (store only non-null), or return a retry flag so lazy callers re-query once the texture exists. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (SH-3a-05).
- **Resolution**:

### BG30 — gem-CO-2c-03 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/companion_ai_src/CompanionAI.cpp:707`
- **Claim** (Gemini 2026-10-14, untrusted until checked): CrTrack caches raw creature pointer; stale spinning state and dangling cr after death or area change
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 PARTIAL -> skeptic real=yes, log-only, low. e1Evaluate only runs from AiTraceAttackEntry, so spinning stays true and END is never logged after death or area change; t.cr is never dereferenced stale.
- **Audit evidence**:
  - `patch_manager_mods/companion_ai_src/CompanionAI.cpp:707` `} else if (t.spinning) {`
  - `patch_manager_mods/companion_ai_src/CompanionAI.cpp:790` `if (now - t->secStart >= 1000) e1Evaluate(*t, now);`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Expire tracks unseen for N seconds in a periodic sweep: clear spinning and emit E1 END. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2c-03).
- **Resolution**:

### BG31 — gem-CO-2c-05 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/companion_ai_src/CompanionAI.cpp:589`
- **Claim** (Gemini 2026-10-14, untrusted until checked): g_lastCtrlArea overwritten with 0 during transition so PARTY why=area misses
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, log-only, low. Line 589 overwrites the baseline with 0 on a transient FnGetArea 0, so the next area change logs no why=area; probe log only.
- **Audit evidence**:
  - `patch_manager_mods/companion_ai_src/CompanionAI.cpp:589` `if (have) { g_partyLoaded = true; g_lastCtrl = ctrl; g_lastCtrlArea = area; }`
  - `patch_manager_mods/companion_ai_src/CompanionAI.cpp:587` `else if (have && area && g_lastCtrlArea && area != g_lastCtrlArea) why = "area";`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Store g_lastCtrlArea only when area is non-zero. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2c-05).
- **Resolution**:

### BG32 — gem-HN-6h-04 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/holster_src/Holster.cpp:128`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Unreadable creature logs false old=0 edge=1 combat transition
- **Still real (2026-10-14 G2)**: G1 UNSURE -> v2 CONFIRMED -> skeptic real=yes, log-only, low. rdU32 returns 0 on saferead failure, so an unreadable creature logs old=0 and a bogus edge=1 when new!=0; probe log only.
- **Audit evidence**:
  - `patch_manager_mods/holster_src/Holster.cpp:128` `uint32_t v = 0;`
  - `patch_manager_mods/holster_src/Holster.cpp:161` `const uint32_t oldState = off ? rdU32(kSiteComb, c, off) : 0xFFFFFFFFu;`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Make a read-fail return a sentinel (0xFFFFFFFF) and print old=? edge=? when the read failed. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (HN-6h-04).
- **Resolution**:

### BG33 — gem-HB-1c-03 (low, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Stale tooltip kept when new slot name read is empty
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, low. tipKey and tipSet update even when setNative is skipped for an empty name, so the prior entry's tooltip persists; needs an unreadable name.
- **Audit evidence**:
  - `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708` `if (!b.tipSet || b.tipKey != e.key) {`
  - `patch_manager_mods/_shared_src/GuiTip.h:53` `inline bool clearNative(uintptr_t ctl) { return setNative(ctl, ""); }`
- **Check steps**: No host seam for updateButtons; extract the tooltip decision into CombatLogic.h and test empty-name clearing.
- **Investigate**: fix sketch: Call guitip::clearNative(b.bg.c) when name[0]==0 instead of skipping, so tipKey and the shown tooltip agree. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (HB-1c-03).
- **Resolution**:

### BG34 — gem-HB-1c-06 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Vanilla tint read immediately overwritten; failure branches are dead code
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, log-only, low. tOk and aOk are forced true right after the reads, so the failure log branch and the 0.85f fallback are unreachable.
- **Audit evidence**:
  - `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650` `tOk[g] = guikit::getBorderTint(btnCtl, tint[g]) && (tint[g][0] > 0.01f || tint[g][1] > 0.01f || tint[g][2] > 0.01f);`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Drop the dead tOk/aOk checks and the 0.85f alpha fallback; keep the forced red tint per the :652 comment. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (HB-1c-06).
- **Resolution**:

### BG35 — gem-QH-2a-07 (low, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/queue_hud_src/QueueHud.cpp:1253`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Transient g_combatUnreadable overwrites g_prevCombat, firing false combat rising edge
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, low. Unreadable frame stores prev=false so next readable combat frame re-fires handoffTick with a fresh first-send ServerClearAllActions; rare, needs HUD flag read failing mid-combat.
- **Audit evidence**:
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1253` `const bool combatNow = g_combat != 0 && !g_combatUnreadable;`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1255` `for (int k = 0; k < 3; ++k) { edgeFlags[k] = combatNow && !g_prevCombat[k]; g_prevCombat[k] = combatNow; }`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1810` `g_capTick = GetTickCount();`
- **Check steps**: Host test needs an extracted edge helper: sequence combat,unreadable,combat must yield one edge; today two.
- **Investigate**: fix sketch: Do not write g_prevCombat when g_combatUnreadable; keep last readable state so no false rising edge. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (QH-2a-07).
- **Resolution**:

### BG36 — gem-CO-2d-04 (low, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Pending, auto and cooldown state indexed by party slot not creature; leader swap misattributes
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 PARTIAL -> skeptic real=yes, wrong-behaviour, low. Pending/auto/cooldown state is slot-indexed with no creature check; a swap inside the 6 s window misattributes DONE/timeout and can allow an early second use.
- **Audit evidence**:
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394` `else if (mi == 0) { g_pend0 = shui::Pending(); g_pend0.on = true; g_pend0.since = now; g_pend0.preHp = m.cur; g_pend0.preStack = pk.count; g_unqueuedSince0 = 0;`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:757` `int hp0 = 0 < cnt ? v[0].cur : g_pend0.preHp;`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:790` `for (int i = 1; i <= 2 && i < cnt; ++i) {   // never member 0 (the controlled leader)`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Store the server creature in Pending and cancel when v[i].server differs. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2d-04).
- **Resolution**:

### BG37 — gem-CO-2d-06 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1176`
- **Claim** (Gemini 2026-10-14, untrusted until checked): LMB on companion button at FullHp/Pending skips popover close and noneed log
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, log-only, low. Companion LMB at FullHp/Pending falls through silently and leaves the popover open (it stays open on valid clicks too); leader branch logs noneed.
- **Audit evidence**:
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1176` `} else if (Card& c = g_card[onBtn]; shui::clickAllowed(c.st)) {`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1182` `if (onBtn >= 0) g_popMi = (g_popMi == onBtn) ? -1 : onBtn;   // 0 = leader, 1..2 = companions`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Log noneed/pending for companions like the leader branch; closing the popover on a button click is optional UX. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2d-06).
- **Resolution**:

### BG38 — gem-CO-2d-09 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/_shared_src/EmbedTex.h:149` (same root cause: CO-2d-09, QH-2a-11, SH-3a-06)
- **Claim** (Gemini 2026-10-14, untrusted until checked): heal() logs giving up on the 20th refill even if that fill succeeded
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, log-only, low. heals counts attempts and ignores fill() result; line 155 fires at heals==20 even when that fill succeeded. Counter re-arms on upload, so harmless. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/EmbedTex.h:149` `if (!built && !px) { ++s.heals; detail::fill(s, "refilled after a texture reset"); }`
  - `patch_manager_mods/_shared_src/EmbedTex.h:155` `if (s.heals == MaxHeals) EMBTEX_LOG("tex %s: giving up after %d failed refills (built=%d missing=%d pixels=%p); art missing, labels still work",`
- **Check steps**: embtex_test.cpp: 20th refill succeeds then uploads; assert no giving-up line. Today :93 pins exactly one line. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: Log give-up only when the final fill failed (capture the detail::fill result) or reword the message to say refill attempts. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2d-09, QH-2a-11, SH-3a-06).
- **Resolution**:

### BG39 — gem-SH-3a-01 (low, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/_shared_src/GuiKit.h:583`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Wheel callback passes engine screen size as client rect size to clientToEngineF
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 PARTIAL -> skeptic real=yes, wrong-behaviour, low. cw/ch are the provider's engine size (client rect only as fallback), so the INI path maps 1:1 instead of client to INI size; never zero, only differs when client != engine size.
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/GuiKit.h:583` `int sw = ev->screen_w, sh = ev->screen_h;`
  - `patch_manager_mods/gui_input_src/GuiInputProvider.cpp:92` `ex = static_cast<float>(pt.x) * static_cast<float>(sw) / static_cast<float>(r.right);`
- **Check steps**: guikit_test.cpp: wheel event with client 1000x500, screen_w/h 1920x1080, iniW/iniH set; expected engine_x must scale from 1000, not 1920. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: With INI override, scale from client coords using a real client size (add client_w/h to K2WheelEvent) instead of ev->screen_w/h. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (SH-3a-01).
- **Resolution**:

### BG40 — gem-SH-3a-07 (low, G2-validated, memory-safety/wrong-behaviour)
- **Where**: `patch_manager_mods/_shared_src/GuiKit.h:174` (same root cause: SH-3a-07, HB-1c-02)
- **Claim** (Gemini 2026-10-14, untrusted until checked): Allocated label leaked on createLabelFromTemplate and bindByTag failure paths
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, low. Leak real in createLabelFromTemplate and bindByTag:551; bindByTag template/flags failures keep the control in the panel list (addToList=1), not leaked.
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/GuiKit.h:174` `if (!rdInt(c, oId, &id) || id == IdSentinel) { if (why) *why = CF_Template; return 0; }   // template not in the layout`
  - `patch_manager_mods/_shared_src/GuiKit.h:551` `if (!rd32(c, 0, &vt) || vt != labelVt) { r.why = CF_Vtable; r.badVtable = vt; return 0; }`
  - `patch_manager_mods/_shared_src/GuiKit.h:167` `if (!rd32(c, 0, &vt) || vt != labelVt) { if (why) *why = CF_Vtable; if (badVtable) *badVtable = vt; return 0; }`
- **Check steps**: guikit_test.cpp: bindByTag with wrong vtable fake; assert alloc released; today leaked. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: Same as HB-1c-02; bindByTag only leaks on the CF_Vtable return at :551. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (SH-3a-07, HB-1c-02).
- **Resolution**:

### BG41 — gem-SH-3b-05 (low, G2-validated, test-gap/wrong-behaviour)
- **Where**: `patch_manager_mods/_shared_src/SelfHealUi.h:382`
- **Claim** (Gemini 2026-10-14, untrusted until checked): pendingDone2 now==0 sentinel branch untested
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, low. Untested sentinel branch is also buggy: storing 1 for now==0 then computing 0-1 wraps and times out at once; one tick per 49.7 day GetTickCount wrap, benign early clear.
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/SelfHealUi.h:382` `if (*unqueuedSince == 0) *unqueuedSince = now ? now : 1u;   // 0 is the "not running" sentinel`
  - `patch_manager_mods/_shared_src/selfheal_ui_test.cpp:335` `KT_CHECK(pendingDone2(p, 1100, 11, 3, true, &u));          // hp up wins even while queued`
- **Check steps**: selfheal_ui_test.cpp: pendingDone2(p, 0, 10, 3, false, &u=0) must be false; today now-1 wraps to 0xFFFFFFFF and returns true. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: Compute elapsed against a local start (now ? now : 1 stored, but compare using now - start with start==now on first arm), or return false on arming call. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (SH-3b-05).
- **Resolution**:
