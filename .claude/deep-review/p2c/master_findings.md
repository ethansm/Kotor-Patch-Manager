# Master findings ledger

Coverage: **PASS**; findings in ledger: 352; reduce files applied: 2; duplicate groups: 284 (18 duplicates folded into a representative).

## Top risks

| # | id | sev | verification | conf | cluster | file:lines | category | evidence |
|---|---|---|---|---|---|---|---|---|
| 1 | 6ccefb375bd5 | high | confirmed | high | C-0f5a3099 | scripts/generate_ultrawide_gui_overrides.py:1680-1684 | gate-integrity | registration_check's `ok` is an always-true expression for any content_k in (0, 1]: `abs(fx(729) - fx(723)) <= 6*k` is exactly 6k, and the two portrait checks ( |
| 2 | 119182a14fc2 | high | confirmed | medium | C-572db3bf | scripts/plan_manager.py:349-356 | logic | Line 350 builds scoped_ident = f"{doc_prefix}:{clean_ident}" (e.g. '25:T4'), and lines 353-356 print it as the phase argument for delegate/resume/handoff/mark-d |
| 3 | 822afff6842c | high | confirmed | medium | C-cfa19d20 | scripts/generate_ultrawide_gui_overrides.py:694-696 | gate-integrity | is_game_running() wraps the ps probe in `except Exception: pass` and then `return False` (694-696). The only caller is the abort gate `if (args.deploy or args.r |
| 4 | d121f4a74209 | high | confirmed | medium | C-533d619b | research/companion_levelup/tools/parity/unit_gap_test.cpp:48 | gate-integrity | Known facts record six CHECKs removed (old lines 48-57): logOpen below cap, logf stops at cap, one cap marker, counter == MaxLogLines, logOpen refuses at cap, l |
| 5 | 1ce64f3b2759 | high | confirmed | medium | C-0995e300 | scripts/agent_diff_check.py:106-117 | gate-integrity | run_tests() sets all_ok=False only when `res.returncode != 0` (lines 114-115) and otherwise returns True. It passes the filter as `-k tf` (line 107). In run_tes |
| 6 | 2bb645e71f18 | high | confirmed | medium | C-f6697f9b | scripts/abi_check.py:362-367 | gate-integrity | When the per-interface dumper is missing, `candidates = sorted((repo_root / "tests/abi").glob("abi_dump_*.cpp"))` and `dumper_src = candidates[0]` take whicheve |
| 7 | d682bb833922 | high | confirmed | medium | C-ac7c9cfd | patch_manager_mods/queue_hud_src/QueueHud.cpp:973-1036 | hook | QueueHudPerformCastSpell returns early only on g_dispatching / !g_cfg.enabled / g_combat != 0 (line 974) and never checks party membership. Line 1013 `if (membe |
| 8 | e606f41d177d | high | confirmed | medium | C-ec5ec37f | scripts/02_merge_2da_and_tlk.py:234 | logic | Line 116 reads every live table except planetary.2da (`if n != "planetary.2da"`), so the live file is never opened. Line 234 then does `write_2da(echoes_plan, L |
| 9 | d2c66b77d42f | high | downgraded | medium | C-ccaf9aad | research/companion_levelup/tools/parity/scenario_test.cpp:408-409 | gate-integrity | scenario_test.cpp:408-409 is `} else if (a == "logcap") {` whose only body line is a comment: 'nothing to force here, the scenario still checks the gate is unaf |
| 10 | 66c1305db6f7 | high | unverified | medium | C-9a60e7d4 | scripts/subagent_prompt.py:192-218 | logic | Line 192 takes the first `[a-zA-Z0-9_]+_src` token in all_context; lines 211-212 `elif "companion" in ctx_lower:` set `companion_ai_src`; the path test `if "com |
| 11 | 155e569b4449 | medium | downgraded | high | C-459aba38 | scripts/agent_diff_check.py:62-71 | gate-integrity | `EXCLUDED_MODS = {"perf_lab_src"}` (line 62) and `if m and m.group(1) not in EXCLUDED_MODS:` (line 71). Changes under patch_manager_mods/perf_lab_src/ are dropp |
| 12 | 21a7b90bd94e | medium | downgraded | high | C-25de68a2 | scripts/verify_2da.py:106-109 | gate-integrity | Line 106 globs `os.path.join(args.override_dir, "*.2da")`. If that is empty, lines 107-109 print 'No .2da files found in ...' and `return 0`. A wrong KOTOR2_ROO |
| 13 | 57af67597f2b | medium | downgraded | high | C-43284b93 | scripts/snapshot_live.py:54-57 | gate-integrity | try: out = subprocess.check_output(["ps", "-eo", "pid,args"], ...) except Exception: return False. When ps fails or is missing, is_game_running() reports 'not r |
| 14 | 81dabb3f4f5e | medium | downgraded | high | C-06a692a4 | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:1-3 | logic | Header line 3 now reads: "The only engine write is the sanitizeNorm path in ProbeMouseNorm (this+0x408/+0x40C), on by default (sanitize_norm=1 in [mouse_fix]... |
| 15 | bc8be186d7ca | medium | downgraded | high | C-1873ea8f | scripts/clef_matcher.py:452-454 | test | Lines 452-454: "for p, m in test_prompts:" / "d = route_task(p, explicit_mod=m, log_telemetry=False)" / "print(f\"PASS: '{p[:40]}...' -> role=... mod=...\")". T |

## Coverage summary

- units: 446; reviewed 370, mechanical 18, skipped 58, excluded 0, gap 0
- packets to re-queue: 0
- waived (truncated accepted by profile): 2: other-024, other-035
- errors: 0; details in coverage_report.md

## Human-only (excluded, not read)

(none)

## Cost

Basis: cli_reported_total_cost_usd (the only figure used for budgets and decisions).

- Worker calls: 52 (attempts total: 0); CLI-reported $1.7291 over 52 calls
- Reduce sessions: 3; CLI-reported $0.3225
- Tokens (as reported): input 480, cache creation 2,393,021, cache read 7,488,233, output 1,347
- ESTIMATE ONLY: illustrative list rates, never used for a budget or a decision: $0.3957 at haiku-5.5 list rates (1 requests over 100K tokens)

## All findings by cluster

### C-0995e300 (1 finding, scripts/agent_diff_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1ce64f3b2759 | high | confirmed | medium | 106-117 | gate-integrity | worker |  | other-016 | 054ee19, 1fad5c9 | run_tests() sets all_ok=False only when `res.returncode != 0` (lines 114-115) and otherwise returns True. It passes the filter as `-k tf` (line 107). In run_tests.py, main() applies -k as a bare substring filter (run_tests.py:861-862). Its 'no test item matched' check covers only --id and positional | From the repo root run `python3 scripts/run_tests.py -k no_such_item_zz; echo rc=$?` and confirm rc=0 with '0 passed'. Then check whether any caller can pass a -k value that is misspelled or empty, and whether agent_diff_check's own verdict relies on that exit code alone. |

### C-0f5a3099 (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6ccefb375bd5 | high | confirmed | high | 1680-1684 | gate-integrity | worker |  | other-022 | WORKTREE | registration_check's `ok` is an always-true expression for any content_k in (0, 1]: `abs(fx(729) - fx(723)) <= 6*k` is exactly 6k, and the two portrait checks (`409 < 583 < 716`, `530 < 583 < 636`) are strict orderings that fx(x,k) = 400 + (x-400)*k preserves for every k > 0. Separately, main discar | Call registration_check with several values of k in (0, 1] (for example 0.1, 0.5, 0.74, 1.0) and confirm ok is always True. Then compare with what the check should measure, such as the engine's rendered positions, to decide whether it needs a different test or should be removed. |

### C-533d619b (2 findings, research/companion_levelup/tools/parity/unit_gap_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d121f4a74209 | high | confirmed | medium | 48 | gate-integrity | worker |  | other-011 | 7020d54 | Known facts record six CHECKs removed (old lines 48-57): logOpen below cap, logf stops at cap, one cap marker, counter == MaxLogLines, logOpen refuses at cap, logLine writes nothing at cap. Post-image line 48 now holds only the comment '// F1 log cap moved to KpLog.h (Common); pinned by KpLog's own  | Locate KpLog.h (included at LevelUpOverride.cpp:79) and its test file in the Common tree, which were outside this packet's allowed reads. Check that some test asserts: exactly MaxLogLines lines written, exactly one cap marker, and refusal of further writes. Also read scenario_test.cpp around line 40 |
| 32982de3d2be | medium | unverified | high | 48 | gate-integrity | worker | d121f4a74209 | other-011 | 7020d54 | mutate.py:65-66 target the in-file cap. LG02 regex 'if \(!g_f \\|\\| g_logLines >= MaxLogLines\) return;' and LG03 regex 'if \(g_logLines >= MaxLogLines\) return false;' match nothing now, since no g_logLines or MaxLogLines remains in level_up_override_src. mutate.py:256-258 apply() raises SystemExit( | Run mutate.py's F1 mutants (LG01-LG03) against this tree and record each status. Confirm that LG02 and LG03 raise STALE and that LG01 builds and survives. Confirm that CPP in mutate.py maps to LevelUpOverride.cpp, which I inferred and did not verify. Then check whether mutate.py has any entry aimed  |

Root cause (Reduce): The 20000-line log-cap coverage was moved to KpLog tests that are not in this snapshot, so the cap has no check in this tree.

### C-572db3bf (2 findings, scripts/plan_manager.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 119182a14fc2 | high | confirmed | medium | 349-356 | logic | worker |  | other-029 | d331219, f6231a3 | Line 350 builds scoped_ident = f"{doc_prefix}:{clean_ident}" (e.g. '25:T4'), and lines 353-356 print it as the phase argument for delegate/resume/handoff/mark-done. cmd_mark_done matches table rows against the bare cell from parse_plan (line 206: ident = cells[id_col_idx], e.g. 'T4') using clean_ide | In a scratch copy of the repo (not the real plan or .claude dir), create a plan with a table row 'T4' (status PENDING) and a '## Execution log' section. Run `plan_manager.py mark-done T4` and `plan_manager.py mark-done '25:T4'` (with --no-handoff) and diff the status cell and appended lines. Also ch |
| a5dcf2744b60 | medium | downgraded | high | 348-350 | logic | worker | 119182a14fc2 | other-029 | d331219, f6231a3 | Line 348: doc_prefix = plan_stem[:2] if (len(plan_stem) >= 2 and plan_stem[:2].isdigit()) else plan_stem. For a non-numbered plan such as PHASE7_COMMON_MASTER (the example named in the comment at line 84), line 350 yields 'PHASE7_COMMON_MASTER:T4'. resolve_plan_shorthand has no branch for that form: | In a scratch repo, create a plan named PHASE7_TEST.md with a table row T4 and run `plan_manager.py resume` (with the active plan set to it). Read the printed 'Resume:' line, run that exact command, and check the exit code (expect 2). Call resolve_plan_shorthand('PHASE7_TEST:T4') directly and expect  |

Root cause (Reduce): plan_manager prints a 'NN:ident' or 'STEM:ident' string as the next command, but mark-done does not parse it and resolve_plan_shorthand cannot resolve the non-numbered form.

### C-9a60e7d4 (2 findings, scripts/subagent_prompt.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 66c1305db6f7 | high | unverified | medium | 192-218 | logic | worker |  | other-034 | 054ee19, 31aa40c | Line 192 takes the first `[a-zA-Z0-9_]+_src` token in all_context; lines 211-212 `elif "companion" in ctx_lower:` set `companion_ai_src`; the path test `if "companion_levelup" in plan_str or "level_up" in plan_str:` (217-218) now runs last, only when mod_scope is still empty. The pre-change code (o1 | Build a fixture plan under a path containing companion_levelup whose body mentions 'companion' and no *_src token; call extract_phase_info and inspect the fifth return value. Check whether the precedence change in 31aa40c/054ee19 was intended (commit messages are not in this snapshot). |
| 9f94f6db6294 | medium | unverified | medium | 199-200 | logic | worker | 66c1305db6f7 | other-034 | 054ee19, 31aa40c | `if "queue hud" in ctx_lower or "queue_hud" in ctx_lower or "queue" in ctx_lower:` then `mod_scope = "patch_manager_mods/queue_hud_src/"`. The bare substring 'queue' also matches 'queued', 'enqueue', 'dequeue', and it is checked before the action hotbar (201) and combat hotbar (203) branches. | Call extract_phase_info on a fixture plan titled 'Action Hotbar' whose text contains 'queued' and no *_src token; expect action_hotbar_src. Compare with the current output. |

### C-ac7c9cfd (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d682bb833922 | high | confirmed | medium | 973-1036 | hook | worker |  | code-009 | 2c071d8, d4ef936 | QueueHudPerformCastSpell returns early only on g_dispatching / !g_cfg.enabled / g_combat != 0 (line 974) and never checks party membership. Line 1013 `if (memberK < 0) memberK = 0;` assigns any non-party caster to member 0. Line 1016 decides on g_itemQueue[memberK]. DISPATCH_NOW stores makeSpellActi | Log clientCreature against the party table inside QueueHudPerformCastSpell during a cutscene or scripted NPC cast out of combat. Check whether the match loop at 1000-1012 ever finds a member for those calls, and whether the item fallback at line 939 has ever fired in a real session. |

Root cause (Reduce): Buffered queue actions are keyed by party slot index but store a raw creature pointer, and neither the non-party fallback at enqueue (memberK=0) nor the dispatch path (no comparison to the current slot creature) checks that the pointer still belongs to that slot.

### C-ccaf9aad (1 finding, research/companion_levelup/tools/parity/scenario_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d2c66b77d42f | high | downgraded | medium | 408-409 | gate-integrity | worker |  | other-010 | 7020d54 | scenario_test.cpp:408-409 is `} else if (a == "logcap") {` whose only body line is a comment: 'nothing to force here, the scenario still checks the gate is unaffected'. The base line (o408) was `g_logLines = MaxLogLines;`, so the 20000-line cap is no longer forced anywhere in this path. The branch i | Run `python3 tools/parity/mutate.py --only LG01` (then LG02, LG03) and check whether each reports SURVIVED or STALE. Grep the parity tests for any other driver of the cap (unit_gap_test.cpp:48 and scenario_test.cpp:409 both say it is 'pinned by its own tests'; check that claim against KpLog's test f |

Root cause (Reduce): The 20000-line log-cap coverage was moved to KpLog tests that are not in this snapshot, so the cap has no check in this tree.

### C-cfa19d20 (2 findings, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 822afff6842c | high | confirmed | medium | 694-696 | gate-integrity | worker |  | other-022 | WORKTREE | is_game_running() wraps the ps probe in `except Exception: pass` and then `return False` (694-696). The only caller is the abort gate `if (args.deploy or args.rollback) and is_game_running():` at 1743-1745, so any failure of the probe (ps missing, non-zero exit from check_output, odd output) is read | Run the script with --deploy and a PATH where ps is absent (or a shim that exits 1) and confirm it proceeds past line 1743. Check whether the caller can distinguish 'probe failed' from 'not running'; from the code it cannot. |
| 601ca469508b | medium | unverified | high | 683-696 | gate-integrity | worker | 822afff6842c | other-023 | WORKTREE | is_game_running() wraps subprocess.check_output(["ps", "-eo", "pid,args"], ...) in `except Exception: pass` and then falls through to `return False` (lines 694-696). The only guard before deploy/rollback, main() line 1743 `if (args.deploy or args.rollback) and is_game_running():`, therefore passes s | Run the script with --deploy under a PATH that lacks ps (or stub ps to exit 1) and confirm the deploy proceeds with no warning. Then decide whether an unknown result should refuse the deploy instead of returning False. |

Root cause (Reduce): is_game_running() returns False on any ps exception, so a failed probe reads as 'game not running' and the guard fails open.

### C-ec5ec37f (1 finding, scripts/02_merge_2da_and_tlk.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e606f41d177d | high | confirmed | medium | 234 | logic | worker |  | other-013 | WORKTREE | Line 116 reads every live table except planetary.2da (`if n != "planetary.2da"`), so the live file is never opened. Line 234 then does `write_2da(echoes_plan, LIVE_OVERRIDE / "planetary.2da")`, replacing the live table with the staging copy. Lines 235-236 likewise `shutil.copy2` genericdoors.2da and | Before running, diff the live override planetary.2da, genericdoors.2da and soundset.2da against the staging copies (row count, labels, values). Check whether any installed mod adds planet rows or edits these tables. Confirm the post-run file matches the staging copy and that the pre-run live rows ar |

### C-f6697f9b (1 finding, scripts/abi_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2bb645e71f18 | high | confirmed | medium | 362-367 | gate-integrity | worker |  | other-015 | 1fad5c9 | When the per-interface dumper is missing, `candidates = sorted((repo_root / "tests/abi").glob("abi_dump_*.cpp"))` and `dumper_src = candidates[0]` take whichever dumper sorts first; the only new effect is a stderr line (367). The update branch (384-388) then does `toml_content = dump_to_toml_str(dum | In a scratch copy of tests/abi, keep a golden whose interface has no abi_dump_<stem>.cpp and run `python3 scripts/abi_check.py --update`; diff the golden against its previous content and confirm the [interface] name came from the other dumper. Then check whether update mode should refuse when the du |

### C-00f82984 (1 finding, scripts/verify_2da.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 287ab8b87b3e | medium | unverified | medium | 106 | logic | worker |  | other-040 | WORKTREE | `glob.glob(os.path.join(args.override_dir, "*.2da"))` is case-sensitive on Linux, so `APPEARANCE.2DA` or `.2Da` files are silently skipped. The sibling _common.py `find_one_ci` (lines 41-50) exists because Linux can hold case variants of one name and the game under Wine loads either. This script doe | List override/ and look for any `*.2DA` or mixed-case `.2da` names. Then compare the file count printed by this script (line 117) with a case-insensitive count of the same directory. Decide whether the match should be case-insensitive, as find_one_ci does. |

### C-014e732f (2 findings, neocities_mods_detailed.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 20f906aab1cd | medium | unverified | medium | 10 | other | worker | 6a2684bd832c | other-004 | WORKTREE | Line 10 (notes of index 1, 3C-FD Patcher): "Windows/your antivirus might attempt to block it, because it's an unsigned executable ... please feel assured that this program is safe, and you can override Windows/your AV and run it." The same line says the source code is "publicly available HERE", but  | Compare line 10 with the upstream guide text it was copied from, and check whether the stripped link was the 3C-FD source repository. Confirm the link resolves to the author's official release. Decide whether the AV-override wording should stay in the catalog without a verifiable source link. |
| 6a2684bd832c | medium | unverified | medium | 10 | other | worker |  | other-003 | WORKTREE | Entry 1 notes (line 10): "When attempting to run this executable, Windows/your antivirus might attempt to block it, because it's an unsigned executable ... please feel assured that this program is safe, and you can override Windows/your AV and run it." The same entry tells users to run an executable | Compare this notes paragraph with the source guide it was copied from to confirm the AV-override sentence is intended as published text. Check whether the 3C-FD source page publishes a hash or signature that the guide could cite. This is a content review; no code path is involved. |

### C-02ce2073 (1 finding, patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6adfec63f6fb | medium | unverified | high | 70-73 | logic | worker |  | code-005 | 2db77c4, bb080cf | Line 70: `constexpr int MaxWarnLines = 4000;` and lines 71-73: `void logw(...) { if (g_warnLines >= MaxWarnLines) return; ++g_warnLines;`. Once the counter reaches 4000 every later warning is dropped with no 'suppressed' marker. logw is shared by one-off H1 bind-time failures (lines 400, 447-453, 46 | Check how many logw() calls one unusable slot produces per ShowItem (H2 path, lines 479-503) and whether a long session can reach 4000 before an H1 failure (lines 400-465) occurs. Check whether the cap path ever writes a marker line. Compare with the logf() cap at lines 60-68, which is also silent,  |

### C-03c0b3c8 (1 finding, scripts/snapshot_live.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4e7b5eb48fe6 | medium | unverified | high | 96 | logic | worker |  | other-033 | WORKTREE | earlier = sorted(p for p in dest.iterdir() if p.is_dir() and p.name < name and (p / "override").is_dir()). Only the override directory is required. take_snapshot creates the directory (line 178) and runs rsync (line 179) before any SHA256SUMS (line 144) or snapshot.json (line 201) exists. It can exi | Make rsync fail mid-run (for example an unreadable file under override/), then run again and check the 'previous' field in the new snapshot.json and whether the failed directory has SHA256SUMS. Check whether previous_snapshot should require SHA256SUMS and a complete snapshot.json. |

### C-048e5da5 (1 finding, research/companion_levelup/tools/parity/scenario_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5a447a280525 | medium | unverified | low | 177 | test | worker |  | other-010 | 7020d54 | scenario_test.cpp:177 defines clearLog() as luoHostLogReset('levelup_override_log.txt'). stub/LuoLogHost.h:9 implements that as std::remove(path) followed by KpLog_SetFallbackFile(path). The stub comment (line 3) says KpLog keeps the fallback file open across remove(), so later writes land in the un | Read KpLog_SetFallbackFile in the Common KpLog.h (not in this snapshot) for an early return when the path is unchanged. unit_gap_test.cpp:20 says the reopen is required, which supports the concern. Add a positive control to one scenario: after clearLog(), a line the mod is known to write must appear |

### C-06a692a4 (1 finding, patch_manager_mods/mouse_probe_src/MouseProbe.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 81dabb3f4f5e | medium | downgraded | high | 1-3 | logic | worker |  | code-008 | b2c3e46 | Header line 3 now reads: "The only engine write is the sanitizeNorm path in ProbeMouseNorm (this+0x408/+0x40C), on by default (sanitize_norm=1 in [mouse_fix]...)". The removed text (old lines 1-2) said "STRICTLY READ-ONLY ... it never writes engine memory". Lines 8-10 still promise behaviour that "m | Decide whether the sanitizer belongs in the diagnostics build at all. Compare doc 18 (patch_manager_mods/18_mouse_camera_investigation.md) and the shipped mouse_probe.ini / manifest.toml / test.toml in the packet directory for the intended default of sanitize_norm. Run the same scenario with sanitiz |

Root cause (Reduce): The mouse probe is documented as read-only apart from one sanitizeNorm path, but it performs additional engine writes (the message-struct rewrite at 305-306 and the unguarded this-field stores at 220-221), and the header does not disclose the first.

### C-070134a5 (1 finding, scripts/normalize_diversity_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 987ba38fd5da | medium | unverified | low | 29 | logic | worker |  | other-029 | f6231a3 | The packet removes the first entry: o29 '-     os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/override"),'. build_catalog keeps the first match for full names (lines 117-118), so this root was the highest-priority source. Now any slot in MALE_SLOTS/FEMALE_SL | Read the f6231a3 commit message for the reason. List the Steam override folder against the MALE_SLOTS/FEMALE_SLOTS and MODEL_INGEST_MAP keys. Run normalize_diversity_textures.py --dry-run into a temp --out-dir with and without the entry (on a copy of the search roots) and compare the chosen source p |

### C-0a0ceaea (1 finding, patch_manager_mods/level_up_override_src/test.toml)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ce3f285cbcb0 | medium | downgraded | high | 4 | build | worker |  | other-007 | 97c1bd8 | test.toml:4 adds "-I$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common" to the committed [common_contract_test] cxxflags. This is a per-user absolute location outside this repo; the in-repo shared headers are the first entry ("-I../_shared_src"). The comment at test.toml:1 say | Find the test runner's handling of cxxflags: does it run through a shell or expand environment variables, or does it pass argv directly? Do a clean checkout without the external KOTOR2_Mod_Workspace tree and build common_contract_test with -Werror; see whether kpatch_api.h or any other header that o |

Root cause (Reduce): Build configs add a home-directory Patches/Common include path that a clean checkout does not have, so test builds depend on one user's workspace layout.

### C-0d496cf3 (1 finding, patch_manager_mods/mouse_probe_src/MouseProbe.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| eee579216787 | medium | unverified | medium | 189-193 | bug | worker |  | code-008 | b2c3e46 | Line 193: `POINT cur = {0, 0}; if (dyn_GetCursorPos) dyn_GetCursorPos(&cur);`. The return value is discarded, so when the function is unresolved or returns FALSE, the origin (0,0) is used as the real cursor. It feeds computeSanitizedDelta (lines 196-199), checkEdgeWarp (line 209), and g_lastDragPos  | Find the conditions where GetCursorPos fails on the target (locked desktop, no input desktop, Wine under XWayland). Stub it to return FALSE while dragging and trace one frame of NORM and CAM output. Check whether the sanitizer should skip the frame and keep the last delta when the read fails, rather |

### C-0fe06c3e (1 finding, scripts/check_ncs_arg_order.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9922de6547ae | medium | downgraded | medium | 114-123 | gate-integrity | worker |  | other-017 | WORKTREE | scripts() returns silently at 114-115 when `override_only or not (game_dir / "Modules").is_dir()`. At 120-123, `try: capsule = read_erf(p)` / `except Exception: continue` drops an unreadable .mod/.erf with no output line, unlike unreadable scripts at 147-148. The summary at 151 counts only flagged s | In a scratch copy of the game directory, truncate or corrupt one .mod in Modules/ and run the script. Confirm there is no line for it and that the exit code is 0. Then rename Modules/ and confirm exit 0 with no per-capsule output. Consider whether the skipped-capsule and missing-directory cases shou |

Root cause (Reduce): These gate scripts return exit 0 when nothing was scanned or found, or silently skip an input, so an empty run looks like a pass.

### C-110c447c (1 finding, patch_manager_mods/mouse_probe_src/MouseProbe.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ff23d43b210c | medium | downgraded | medium | 220-221 | memory-safety | worker |  | code-008 | b2c3e46 | Line 184 only rejects a null pointer: `if (!rd(ebp, -0x28, &self) \|\| self == 0) return;`. Lines 220-221 then store through that pointer with no readability or writability check: `*reinterpret_cast<float*>(self + 0x408) = fixedNx;` / `... self + 0x40C = fixedNy;`. This runs by default (line 139). The | At the hook site 0x72F2EC (doc 18), confirm that [ebp-0x28] always holds a live engine object on every call, including load, menu and world transitions where the object may be recreated. Check that the object is at least 0x410 bytes. Check saferead.h for a write-checked helper and whether the read a |

Root cause (Reduce): The mouse probe is documented as read-only apart from one sanitizeNorm path, but it performs additional engine writes (the message-struct rewrite at 305-306 and the unguarded this-field stores at 220-221), and the header does not disclose the first.

### C-11c6d34d (1 finding, patch_manager_mods/level_up_override_src/common_contract_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3a122c1e3473 | medium | unverified | medium | 15 | build | worker |  | code-007 | dc4bb61 | common_contract_test.cpp:15 is #include "check.h". No check.h exists in level_up_override_src (glob **/check.h empty), at the top level of ../_shared_src (glob found only Party, GuiTip, SafeRead, GuiKit, GameAddr, K2Addr .h), or in research/companion_levelup/tools/parity/stub (windows.h, m3_win_shim | From a clean clone (or with HOME pointing elsewhere), build common_contract_test with test.toml's [common_contract_test] cxxflags and see whether check.h resolves. Check whether Patches/Common exists anywhere in the repo tree or only in the author's workspace, and whether other test.toml tables use  |

### C-142aad27 (1 finding, patch_manager_mods/queue_hud_src/kotor2-steam-aspyr.hooks.toml)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7326b0db3b09 | medium | unverified | low | 29 | hook | worker |  | other-008 | 2c071d8 | Line 29 hardcodes `address = 0x00775703` for the new H10 detour. The same hook is described in QueueHud.cpp:972 as "gameaddr::PerformCastSpell + 3; address in hooks.toml", and the queue's own re-dispatch goes through `AddrPerformCastSpell = gameaddr::PerformCastSpell` (QueueHud.cpp:78, called at 757 | Open GameAddr.h (included at QueueHud.cpp:39, not in this packet and not read) and confirm gameaddr::PerformCastSpell == 0x00775700 (so +3 == 0x00775703) and gameaddr::PerformUseItem == 0x00775600 (H9 uses 0x00775603). Then, against the target binary, confirm bytes at 0x00775703 equal [83 EC 0C 89 4 |

### C-14567f48 (1 finding, patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f0b28bacd0ef | medium | unverified | medium | 775-780 | logic | worker |  | code-003 | 4519b96, eff4a32 | Leader branch: `if (k == 0) { g_card0.have = true; g_card0.k = k; g_card0.vit = vit; g_card0.seen = true;` has no identity check against v[0]. The companion branch (770-772) requires `cr` to equal `c.m.client` or `c.m.server` and sets `c.seen = !isLeader`. `isLeader` is computed at 766 and never con | Confirm on a live HUD, or from the MAP log at line 785, that the slot with k==0 always has cr equal to v[0].client or v[0].server and leader-width vit. Check PortraitSlots.h (outside this packet) for whether slot index 0 is fixed to the leader. If not guaranteed, compare with the companion identity  |

### C-1873ea8f (1 finding, scripts/clef_matcher.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bc8be186d7ca | medium | downgraded | high | 452-454 | test | worker |  | other-018 | bbb038f, d331219 | Lines 452-454: "for p, m in test_prompts:" / "d = route_task(p, explicit_mod=m, log_telemetry=False)" / "print(f\"PASS: '{p[:40]}...' -> role=... mod=...\")". The expected mod m is only passed in as input; no expected role or mod is ever compared, PASS is printed for every prompt, and the subcommand | Run `grep -rn "clef_matcher" scripts/` to confirm no gate calls `clef_matcher.py test`. In a scratch copy, change one expected mod or role in test_prompts and run `python3 -I scripts/clef_matcher.py test; echo $?`. Expect PASS lines and exit 0 regardless. Check whether test_clef_matcher.py already a |

### C-1a72708d (1 finding, scripts/install_echoes_compatch.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 80bbaa515de1 | medium | unverified | low | 83-89 | logic | worker |  | other-027 | WORKTREE | `blob = sum(len(v) + 1 for v in {*labels, "1.5", "2.1"})` (line 85) and `pos = len(data) - blob - 2` (line 86) locate the uint16 data-size field by arithmetic from the end of the buffer. `data[pos:pos + 2] = b"\0\0"` (line 87) then zeroes it without checking that the bytes there were the expected no | Dump bytes_2da output for the two-row test table and confirm the uint16 at len(data)-blob-2 is the size field, and that it equals blob before zeroing. Then run check_pykotor() against stock pykotor 2.3.12 and confirm it exits with the blank-read-back message instead of passing. |

### C-1b97daa5 (1 finding, scripts/dedup_tga_tpc.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| be5c7bd8b6fd | medium | unverified | high | 24-28 | bug | worker |  | other-020 | WORKTREE | KOTOR2_ROOT = os.environ.get('KOTOR2_ROOT', os.path.expanduser(...)) (lines 24-27). If the variable is set but empty, get returns '' and OVERRIDE = os.path.join(KOTOR2_ROOT, 'override') (line 28) becomes the relative path 'override'. The default override_dir at line 56 uses OVERRIDE, so a non-dry ru | In a temp directory with override/ containing a .tga and a same-basename .tpc, run the script with KOTOR2_ROOT set to an empty value and --dry-run. Confirm that the printed path is relative to the CWD. Then check how an empty or unexpanded KOTOR2_ROOT is handled by the build guide's own scripts (out |

### C-1fa1d43f (1 finding, scripts/master_ledger.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2bd4e8663c08 | medium | unverified | high | 441-469 | logic | worker |  | other-028 | 93385ec, d331219 | `events = read_events(ledger_path)` (line 442), then the ledger is rewritten from the in-memory list: `with open(tmp_path, "w") ...` (lines 466-468) and `tmp_path.replace(ledger_path)` (line 469). There is no lock, so any `log_event` append between the read and the replace is lost. read_events also  | Add a file lock shared with log_event, or write the pruned copy with an append-only archive step first. Test by appending in a loop while prune runs, then count the lines. Confirm whether malformed lines are removed. |

### C-206408f3 (1 finding, scripts/case_dedup.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b1b738da5be9 | medium | unverified | medium | 17-18 | other | worker |  | other-017 | WORKTREE | The docstring says "copy_loose.py and pk_install.py call collapse() after every install, with the files that install just wrote as the winners, so new pairs can't form." Glob found neither file under scripts/. Grep for `collapse(` under scripts/ finds only the definition at line 59 and the call insi | Search the whole repository (outside this packet, by a human or a later packet) for `collapse(` and for imports of case_dedup. If no installer calls collapse(), correct the docstring. If the installers exist elsewhere, confirm they pass the just-written spellings as winners in the same form that aud |

### C-226ace81 (1 finding, scripts/compat_graph.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4811f0e7ead8 | medium | downgraded | medium | 444-447 | gate-integrity | worker |  | other-020 | 3ec1365 | _scan_suites wraps the parse in 'try: cfg = tomllib.loads(...) except Exception: return' (lines 444-447). A syntax error in tests/suites.toml leaves cpp_to_suites empty with no message, so affected() never emits suite items (lines 668-677). The same silent return is at lines 485-487 for smokes.toml. | In a temp copy of the tree, corrupt tests/suites.toml (for example an unclosed quote), call affected() on a touched .cpp that a suite references, and confirm the result has no suite items and no error or warning. Then check the Plan consumer outside this packet to see whether an empty suite list is  |

### C-25de68a2 (1 finding, scripts/verify_2da.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 21a7b90bd94e | medium | downgraded | high | 106-109 | gate-integrity | worker |  | other-040 | WORKTREE | Line 106 globs `os.path.join(args.override_dir, "*.2da")`. If that is empty, lines 107-109 print 'No .2da files found in ...' and `return 0`. A wrong KOTOR2_ROOT (the docstring tells users to set it for GOG or manual copies) or a nonexistent override directory therefore exits 0, the same code as a c | Run `python3 scripts/verify_2da.py /nonexistent; echo $?` and confirm it prints the 'No .2da files' line and exits 0. Then check whether any wrapper or CI step that calls this script checks the exit code, and whether a zero-files result should be a non-zero exit. |

Root cause (Reduce): These gate scripts return exit 0 when nothing was scanned or found, or silently skip an input, so an empty run looks like a pass.

### C-2711f87a (1 finding, scripts/04_merge_modules.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| da9267c332a9 | medium | unverified | medium | 322-325 | bug | worker |  | other-014 | WORKTREE | `entry_list = dlg_gff.root.get_list("EntryList")` / `entry_list[14].set_string("Quest", "kor_purebld1")` / `entry_list[14].set_uint32("QuestEntry", 10)`. The index is hard-coded and the previous Quest and QuestEntry values are not read or compared. The same pattern is used for 701kor.are OnEnter at  | Read kreia_kr.dlg from the live 701KOR.mod and print EntryList[14] (Text, Quest, QuestEntry, Script) before the write. Confirm the entry text matches the kor_purebld1 quest step. Check whether the list has at least 15 entries. |

### C-276834d2 (1 finding, scripts/master_ledger.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 25d482090698 | medium | unverified | medium | 130-132 | logic | worker |  | other-028 | 93385ec, d331219 | `if "claude" in cmd.lower():` / `m = re.search(r"--resume\s+([a-f0-9-]+)", cmd)` / `return f"claude:{m.group(1)[:8]}" if m else "claude-code"`. Any Claude Code session started without --resume gets the same identity, 'claude-code'. The collision check at lines 377-378 (`if agent != my_agent:`) there | Run two Claude Code sessions without --resume, log HOTSWAP_APPLY from one, then run check-collision from the other and check whether it reports CLEAN. Consider a per-process or per-session identifier as the fallback. |

### C-279f58a8 (1 finding, scripts/compat_graph.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 99c145fd9f7b | medium | unverified | medium | 774 | logic | worker |  | other-020 | 3ec1365 | _is_classified uses path.startswith(f'patch_manager_mods/{m}') with no trailing slash (line 774). _apply_static_checkers uses f'patch_manager_mods/{m}/' with the slash (line 793). A file under a sibling directory whose name starts with a mod directory name is classified as belonging to that mod. | List mod_dirs for the repo and look for a manifest-less directory whose name starts with a mod directory name (for example patch_manager_mods/level_up_override_src against a mod named level_up_override, if that mod exists). Call _is_classified on a file in that directory and check whether it returns |

### C-27b664ff (1 finding, scripts/crash_triage.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 11d664596a19 | medium | unverified | medium | 135-138 | gate-integrity | worker |  | other-020 | WORKTREE | cmd_log returns 0 after printing 'VERDICT: FAULT(...)' or 'VERDICT: EXCEPTION' (lines 135-138). cmd_proc returns 0 for HANG (line 93). Only cmd_kpm returns non-zero for a failed verdict (ABORTED, line 165). | Run the log subcommand on a log that contains an exception signature and check the shell exit status. Grep the repo (outside this packet) for callers of crash_triage.py that test the exit status. |

### C-28cadcc0 (1 finding, scripts/check_ncs_arg_order.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4be4b4deed04 | medium | unverified | high | 146-152 | gate-integrity | worker |  | other-017 | WORKTREE | main() gets `flagged, unreadable = check(...)` at 146 and prints each unreadable label at 147-148, but the return at 152 is `return 1 if flagged else 0`, so `unreadable` never affects the exit code. A .ncs that pykotor cannot parse is never checked and still yields exit 0. | Place a deliberately malformed .ncs in a scratch override/, run the script, and confirm the '(unreadable by pykotor, not checked: ...)' line appears with exit 0. Then check how kotor2_toolset.py interprets the exit code for this step. Decide whether unreadable scripts should make the run fail, as th |

### C-297d5cb1 (1 finding, scripts/05_merge_dlgs_and_jrl.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7ac1208a057d | medium | unverified | medium | 85-101 | logic | worker |  | other-014 | WORKTREE | `for i in range(min(live_entries_count, stage_entries_count)):` / `st_links = e_stage[i].get_list("RepliesList") or []` / `lv_links = e_live[i].get_list("RepliesList")` / `if lv_links is None: continue`. Live entry i is paired with stage entry i by position only. The reply Index values are then copi | For each companion dialogue in COMPANION_DLGS, compare Text or Speaker of live EntryList[i] against stage EntryList[i] for a sample of indices, and count mismatches. Also count entries where the live RepliesList is None, since those are skipped at lines 88-89. |

### C-2a3b5c4a (1 finding, scripts/master_ledger.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cf57dbb3353d | medium | unverified | high | 172-179 | logic | worker |  | other-028 | 93385ec, d331219 | `for k, v in data.items():` / `if k == "mod" and v: ...` / `elif k == "mods" ...` / `else: record[k] = v`. Caller data is written over the record's own keys, so 'ts', 'event' and 'agent' can be replaced. The manual `log` command passes every `k=v` field straight through (lines 515-520), so `log PHAS | Run `python3 scripts/master_ledger.py --ledger <scratch> log PHASE_DONE agent=x ts=2000-01-01T00:00:00Z` and inspect the stored record. Consider reserving 'ts', 'event' and 'agent' so data cannot overwrite them. |

### C-2afcd659 (1 finding, patch_manager_mods/gui_input_src/gi_registry_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9e76e8f4642a | medium | unverified | high | 137-144 | test | worker |  | code-004 | 68a7739 | The new case routes wheel_subscribe to `fakeWheelSubNoWindow` (line 47: `{ ++g_subNoWinCalls; return -1; }`) through `g_fakeTableNoWindow` (line 139). The provider's changed branch (GuiInputProvider.cpp:158-161) never runs. The assertions at lines 142-144 (`noWinTok <= 0`, `g_subNoWinCalls == 1`) on | Read the test to confirm: step 7 only exercises the client wrapper (GuiInput.h, outside the packet) against a fake. For real coverage, drive apiWheelSubscribe itself in a host build with window discovery and the Hook install seam faked so that ensureSubclass returns false. Then assert exactly -1 and |

### C-2b18154a (1 finding, scripts/06_deploy_assets.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8545c7465dad | medium | downgraded | medium | 209-222 | gate-integrity | worker |  | other-014 | WORKTREE | `have = existing_spellings(live_mod_dir)` (209), then for each staged file not in COLLIDING_MODULES: `target = live_mod_dir / have.get(mod_file.name.lower(), mod_file.name)` / `shutil.copy2(mod_file, target)` (220-221). There is no skip branch for a name already present. The comment at line 219 says | Pick a staged module name, place a different file under the same name in a scratch copy of Modules/, run deploy_modules against that copy, and check whether the scratch file's hash changes. Also list which staged module names overlap the live Modules/ names. |

### C-2c450909 (1 finding, scripts/verify_all_compatch.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e15120e5d7fa | medium | downgraded | medium | 89-93 | gate-integrity | worker |  | other-040 | WORKTREE | Every check is a bare `assert`: line 89 `assert len(tlk) >= MIN_TLK_STRINGS`, and the same pattern at lines 92, 93, 100, 112, 115, 125-126, 134, 138, 140, 156, 161, 166, 169. Under `python -O` or PYTHONOPTIMIZE these are removed. `main()` (lines 174-189) then prints 'ALL AUDIT CHECKS PASSED: 100% HE | Corrupt an expected value in a copy of the checker (for example MIN_TLK_STRINGS set very high) and run it with and without `-O`. Confirm the non-optimized run fails and the -O run prints the PASSED banner with exit 0. Check whether any launcher, Makefile or CI step runs Python with -O or PYTHONOPTIM |

### C-3800d9ef (1 finding, tests/suites.toml)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 94d01d8b0b22 | medium | unverified | medium | 178-206 | gate-integrity | worker |  | other-041 | a59cd6f | All five swarm-fix suites require files under the repo's .claude tree: `requires = ["$REPO/.claude/review/fix/fcommon.py"]` (line 178), and the same pattern for fix_intake.py (185), fix_workspace.py (192), fix_settings.py (199), fix_checks.py (206). In this snapshot, Grep on `.claude/review/fix/fix_ | Run `git ls-files .claude/review/fix` and `git check-ignore -v .claude/review/fix/fcommon.py` in the repo. Then run `python3 scripts/run_tests.py --list` and check whether the swarm-fix-* items show SKIP (missing ...) in a clean clone. |

### C-38a11f57 (1 finding, scripts/subagent_prompt.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 79c78aae63f3 | medium | unverified | high | 414-418 | logic | worker |  | other-034 | 054ee19, 31aa40c | The new check `if args.plan and (not plan_path or not plan_path.exists()): ... return 2` runs only for an explicit --plan. Without --plan, extract_phase_info returns `return phase_key, "", [], [], None` (line 111) when no plan resolves, and line 264 substitutes `f"Process {phase_key} as specified in | Run `python3 -I scripts/subagent_prompt.py NOPE99 --json` in a snapshot with no matching plan and check the exit code and the prompt body. Trace resolve_phase_and_plan (lines 87-106) for the None return and check plan_manager.py:446 for the same behaviour. |

### C-3a6429af (2 findings, patch_manager_mods/_shared_src/SelfHealUi.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 97f5356f31c8 | medium | unverified | medium | 46-49 | logic | worker |  | code-001 | 4519b96 | Line 48: `return Rect{card.x, card.y, 58, 58};`. The size is a fixed 58 and the anchor is the top-left corner of the input. Other geometry helpers in this file scale with height: buttonRect (lines 25-31) uses s = clamp(round(h*0.25), 16, 24), gap = round(2*h/36), and vertical centring; ringRect (lin | Read the draw code in the consuming mod that calls leaderButtonRect and confirm whether it passes the vitality bar or the whole card. Compare the 58x58 output with a leader bar at the probe size (x 3511, y 1386, w 103, h 208, from selfheal_ui_test.cpp line 21). Confirm that 58 is a deliberate consta |
| c1437e3479e9 | medium | unverified | medium | 46-48 | logic | worker | 97f5356f31c8 | code-001 | 4519b96 | Lines 46-48: `inline Rect leaderButtonRect(Rect card) { if (card.w <= 0 \|\| card.h <= 0) return Rect{0, 0, 0, 0}; return Rect{card.x, card.y, 58, 58}; }`. The function never calls isLeaderVit or the leader predicate added at line 22. The rule at lines 19-20 and the early return in buttonRect (line 24 | Find the caller of leaderButtonRect in the consuming mod (outside this packet) and check whether it tests isLeaderVit or isLeaderVitForButton before calling. If it does not, pass a companion-sized card such as {3671, 1285, 47, 96} and check whether the returned rect overlaps that card's own vitality |

### C-3c02dde2 (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5322cea53579 | medium | unverified | medium | 981-995 | bug | worker |  | other-025 | WORKTREE | ErfIndex._load sets `self._index = {}` (982) before the loop over ERF_ORDER, and `read_erf(path)` (987) can raise mid-loop. get() and names() reload only when `self._index is None` (992-994, 997-999), so after a failed _load they return from the partial dict. Callers catch (ValueError, FileNotFoundE | Point ErfIndex at a scratch TexturePacks dir whose second ERF in ERF_ORDER is corrupt (or with read_erf patched to raise on it). Call get() for a texture that exists only in a later ERF, then call get() again and see whether the result differs. |

### C-3e4e7047 (1 finding, scripts/clef_dataset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5fbfd30be491 | medium | unverified | high | 82-92 | logic | worker |  | other-018 | bbb038f, d331219 | clef_dataset.py:82-83 keys the cache on the ledger's size and mtime_ns, and line 92 requires an exact match. clef_matcher.py:298 reads the facts at the start of every route_task, and clef_matcher.py:398 appends a CLEF_ROUTE row to the default ledger (log_telemetry defaults to True, line 293). master | Copy .claude/master_ledger.jsonl to a scratch path. Run `python3 -I scripts/clef_dataset.py summary --ledger <scratch>` twice with one append in between, and check the cache file mtime and whether the second run re-parses (instrument line 84). Compare with a `clef_matcher.py route ... --no-log` run  |

### C-409da0c4 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8ea1731f8816 | medium | downgraded | medium | 720-799 | memory-safety | worker |  | code-009 | 2c071d8, d4ef936 | Each tick re-reads srv = serverCreature(party::member(tbl,k)) (lines 720-721) and clears the buffer only when srv is 0 (line 723). Dispatch then probes and calls with the stored nextA.creature (lines 787-789 and 799-800) with no comparison to srv. Re-inject does the same with buf.activeAction.creatu | Trace party::member(tbl,k) and party::serverCreature across an area transition and a companion swap while items or spells are buffered. Check whether a different non-null creature can appear in slot k without an intervening null read. Compare nextA.creature and activeAction.creature with srv before  |

Root cause (Reduce): Buffered queue actions are keyed by party slot index but store a raw creature pointer, and neither the non-party fallback at enqueue (memberK=0) nor the dispatch path (no comparison to the current slot creature) checks that the pointer still belongs to that slot.

### C-40b7a7d0 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 843bd1103025 | medium | unverified | medium | 164-180 | logic | worker |  | code-006 | 97c1bd8, dc4bb61 | Removed code (old lines 146-165): MaxLogLines = 20000 and g_logLines; old logOpen() returned false at 'if (g_logLines >= MaxLogLines)' and opened the file with fopen per hook call; old logClose() called fclose; old logf wrote only when 'g_f && g_logLines < MaxLogLines' and emitted '== log line cap r | Read KpLog.h and its implementation (not in this packet; I could not find KpLog.h at patch_manager_mods/_shared/, _shared/, or one level around the packet directory). Check: (1) whether KP_LOG or the fallback sink enforces a line or byte cap and rotates the file; (2) whether it flushes after each li |

### C-41a483a7 (1 finding, scripts/compat_graph.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 53345fbe0793 | medium | unverified | high | 551-572 | gate-integrity | worker |  | other-020 | 3ec1365 | Per-file 'except Exception: continue' on test.toml (lines 551-552) and 'except Exception: pass' on tests/compat.toml (lines 571-572). A malformed file drops its covers rows and generator edges (covers_map, generators, inc_to_generator) with no message. | In a temp copy, add a syntax error to tests/compat.toml or to one test.toml that has covers rows. Check whether any warning is printed and whether plan.items still contains the covered test or suite for a touched path that the broken row mapped to. |

### C-4263cf90 (1 finding, patch_manager_mods/mouse_probe_src/MouseProbe.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0fc98e09742d | medium | unverified | low | 27 | build | worker |  | code-008 | b2c3e46 | Line 27: `#include "../_shared/SafeRead.h"`. A Glob for `../_shared/SafeRead*` from mouse_probe_src returned no files. The directory listing (MouseFixLogic.h, MouseProbe.cpp, ClickLogic.h, common_contract_test.cpp, exports.def, manifest.toml, test.toml, kotor2-steam-aspyr.hooks.toml) has no SafeRead | In the real repository at HEAD 7020d54, run `git ls-files \| grep -E 'SafeRead\.h\|KpLog\.h'` and check whether the build adds a shared include root. If the header is tracked elsewhere, drop this finding. |

### C-42fc810b (2 findings, scripts/upscale_legacy_movies.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 650652282b59 | medium | unverified | medium | 236-237 | bug | worker |  | other-039 | WORKTREE | Lines 99-104 resolve the on-disk live file case-insensitively into `live_target`, and line 106 uses that match for the already-upscaled check. The deploy loop ignores the match: line 236 builds `target = dest_dir / filename` from the TARGET_FILES spelling, then line 237 copies to it. TARGET_FILES mi | In LIVE_DIR, list entries matching the name case-insensitively (for example `ls \| grep -i '^live_1'`) and compare them with TARGET_FILES. After a run on a mismatched name, check whether both spellings exist and which one the game's movie loader requests. |
| ff08c7c94443 | low | unverified | high | 234-238 | bug | worker | 650652282b59 | other-039 | WORKTREE | Line 235 `if dest_dir.exists():` has no else branch. When LIVE_DIR is missing, nothing is copied and no message is printed. The function still prints 'Finished' (line 248), and main() prints 'BATCH UPSCALING RUN FINISHED.' (line 265). | Run process_movie with LIVE_DIR pointing at a nonexistent path (or temporarily renamed) and check that the output has no 'Deployed to' line and no warning, and that the batch still ends with the success banner. |

### C-43284b93 (1 finding, scripts/snapshot_live.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 57af67597f2b | medium | downgraded | high | 54-57 | gate-integrity | worker |  | other-033 | WORKTREE | try: out = subprocess.check_output(["ps", "-eo", "pid,args"], ...) except Exception: return False. When ps fails or is missing, is_game_running() reports 'not running', so the pre-copy check at line 167 passes and the post-copy check at line 185 sets complete = True. | Run the script with a PATH that has no ps (or a stub ps that exits 1) against a temp game dir and state dir. Check whether it writes a snapshot whose snapshot.json lacks 'incomplete' and whether the exit code is 0. Decide whether an unknown process state should count as running. |

Root cause (Reduce): is_game_running() returns False on any ps exception, so a failed probe reads as 'game not running' and the guard fails open.

### C-4439e45a (1 finding, scripts/compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 568f31a7d27b | medium | unverified | low | 341-355 | gate-integrity | worker |  | other-036 | 5b856ce | Narrowing keeps only kind=='build' items and cpp:k2addr_test (341-346) and drops everything else, including suite, static and wine items. The info note says 'GOG-only: Steam runtime unchanged' (359), but the edited table is kGog. test_compat_check.py:541-545 asserts that cpp:gui2d_test, cpp:buff_hud | Grep the sources of cpp:gui2d_test, suite:check-hook-bytes and the wine smokes for kGog/makeGog versus kSteam. If they reference only the Steam table, the narrowing is sound. If any reference kGog, the narrowing drops a GOG check. Check Doc 22 U11 for the stated rationale. |

### C-4461f8cf (1 finding, scripts/crash_triage.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f920843d92fb | medium | unverified | medium | 88-90 | logic | worker |  | other-020 | WORKTREE | cmd_proc takes pcpu from 'ps -o stat,pcpu,etime -p PID' (line 54) and reports HANG when '"R" in stat and pcpu >= 75.0' (line 88). ps %CPU is a lifetime average (cputime over elapsed time), so a game that ran normally for a long time and then starts spinning shows a low value. | Take ps -o stat,pcpu -p PID on a long-running game process while it spins, and compare with an instantaneous CPU reading (two /proc/PID/stat utime+stime samples about a second apart). Check whether the verdict stays RUNNING when the instantaneous value is high. |

### C-459aba38 (1 finding, scripts/agent_diff_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 155e569b4449 | medium | downgraded | high | 62-71 | gate-integrity | worker |  | other-016 | 054ee19, 1fad5c9 | `EXCLUDED_MODS = {"perf_lab_src"}` (line 62) and `if m and m.group(1) not in EXCLUDED_MODS:` (line 71). Changes under patch_manager_mods/perf_lab_src/ are dropped from target_mods, so run_build_checks() never builds them (line 168 feeds target_mods into line 171), and the verdict can still be PASS.  | Find the rationale for the exclusion in the upstream history of 1fad5c9 and 054ee19 (not present in this snapshot). Then add a change under patch_manager_mods/perf_lab_src/ and run `python3 scripts/agent_diff_check.py --json --no-patch`. Confirm checked_mods omits it and that build_check.py is never |

### C-4ae1affb (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 550daf36b945 | medium | unverified | medium | 287-292 | bug | worker |  | other-033 | 31aa40c, d331219 | Line 287: parse_answer(res_4b.get("answers", {}).get("role", {}) if res_4b else {}). If a parseable response has "answers": null, .get is called on None and the AttributeError is not caught; query_systemone (lines 66-68) only guards the request. Lines 190-192 also crash on null confidence or probabi | Stand up a stub server on port 8101 that returns {"answers": null}, then {"answers": {}}, and run with --limit 2. Check for a traceback in the first case and for an 'unknown' row counted as a disagreement in the second, and compare with the 'Cases without endpoint response' line. |

### C-4c89b781 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3c032fcc205b | medium | unverified | medium | 744 | logic | reduce |  |  | 2c071d8, 32c8125, 4add886, d4ef936 | QueueHud.cpp:744 passes s.paused into qh::shouldReInject. QueueHudLogic.h:576 discards it with (void)paused, and the re-inject return at 580 ignores pause. queue_hud_logic_test.cpp:1173 asserts shouldReInject(...paused=true) is still true, with the comment 'paused/now are ignored by the header'. The | Check git history of QueueHudLogic.h shouldReInject for a removed pause guard. Confirm whether re-inject during pause is intended. If it is not, add a test where paused=true and hasFollowLeader=true with an inactive engine queue, and check how often 0x3D entries appear while the game is paused. |

### C-4dc95f11 (1 finding, scripts/snapshot_live.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3f8dcbee3d1a | medium | unverified | high | 185 | gate-integrity | worker |  | other-033 | WORKTREE | complete = not is_game_running() samples the process state once, after the copy. The docstring (lines 17-18) promises that the snapshot is marked incomplete 'if the game started meanwhile'. A game that starts and exits during the copy is not detected. | Stub is_game_running to return True during the copy and False at line 185, then check that snapshot.json does not get 'incomplete'. Compare the intended semantics in the docstring with what the post-copy check actually samples. |

### C-50582285 (1 finding, patch_manager_mods/action_hotbar_src/ActionHotbar.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fc0796ea9fce | medium | unverified | medium | 1809-1817 | logic | worker |  | code-002 | 25d4e42, 7df1b4a | restoreFromSave adds the currentSaveFolder fallback (1811-1813), then `if (!dir \|\| !*dir) { KP_LOGV(chCfg, KPLOG_WARN, "persave: %s: save folder not recognised ('%s'), live hotbar kept", ...); return; }` with no pending state. snapshotToSave handles the same unresolved-folder condition by deferring  | Find when save-events fills ev->folder and when currentSaveFolder first returns a non-empty path for LOAD_REQUEST compared with SAVED. Add a temporary log in the !dir branch of restoreFromSave, load a save, and compare the bar shown after the load with that save's snapshot. Check whether the pending |

### C-50efa9d9 (1 finding, scripts/upscale_legacy_movies.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f66c964caf85 | medium | unverified | high | 113-117 | bug | worker |  | other-039 | WORKTREE | Line 113 `if not backup_file.exists():` then line 114 `for f in BACKUP_DIR.iterdir():` runs whenever the backup file is missing. Nothing creates BACKUP_DIR before this lookup: the `BACKUP_DIR.mkdir` is at line 132, after source selection, and main() (lines 256-257) only creates WORK_DIR. The same pa | Confirm that backups/legacy_movies_2003 is absent on the target machine, then run process_movie on 05_3a.bik and check that the traceback comes from the iterdir call at line 114. Compare with the mkdir at line 132 to decide where the directory must exist before the lookup. |

### C-5335b3fa (1 finding, scripts/compat_graph.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9843416ac6c0 | medium | unverified | medium | 815-829 | gate-integrity | worker |  | other-020 | 3ec1365 | Line 815 reads 'if has_mod_source and not has_shared:' and wraps suite:check-subclassers (lines 823-829). The shared branch (lines 799-813) adds only check-shared-headers and check-raw-addresses. A change that touches both _shared_src and mod C++ source never schedules suite:check-subclassers. | Call _apply_static_checkers with touched = ['patch_manager_mods/_shared_src/x.h', 'patch_manager_mods/<mod>/a.cpp'] where <mod> is in mod_dirs, then inspect plan.items for suite:check-subclassers. Check the intent of that check (and its entry in tests/suites.toml, outside this packet) to confirm it  |

### C-54462276 (1 finding, scripts/agent_diff_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d9187acd0c6d | medium | unverified | high | 164-165 | gate-integrity | worker |  | other-016 | 054ee19, 1fad5c9 | `if args.scope: scope_violations = check_scope(...)` (lines 164-165). With no --scope, scope_violations stays [] and the scope term in `passed` (line 186) is vacuously true. Likewise run_tests(None) returns (True, 'No specific tests requested.') (lines 102-103), so without --test the test term is al | Find every caller of agent_diff_check.py (test_subagent_tools.py:103 passes only --mod) and confirm --scope and --test are always supplied. Run it on a dirty tree with neither flag and confirm the verdict is PASS with scope_violations=[] and test_output='No specific tests requested.'. |

### C-578aea88 (1 finding, patch_manager_mods/action_hotbar_src/HotbarLogic.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 10750a5a4474 | medium | unverified | medium | 89-93 | logic | worker |  | code-002 | 25d4e42 | `bool isExile = ((oid >= 0xFFFFFFF0u && oid <= 0xFFFFFFFEu) \|\| partyCount == 1 \|\| isMember0 \|\| partySlot == 0);` The range term has no party-slot condition. For a companion (partySlot 1, partyCount 2 or more) with an id in the range, the branch at 90-93 returns "objfffffffd", the Exile's canonical k | Add a case to hotbar_logic_test.cpp with partyCount 3, partySlot 1 and oid 0xFFFFFFF9, and record what the function returns. Confirm from the research note 07_force_power_hotbar.md (outside this packet) whether any companion object id can fall in 0xFFFFFFF0..0xFFFFFFFE. If none can, restrict the ran |

### C-5790e748 (1 finding, scripts/test_master_ledger.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e97945f0fa31 | medium | unverified | medium | 113-120 | test | worker |  | other-037 | d331219 | test_test_isolation_guard calls ml.log_event("TEST_ISOLATION", {"test": True}) with no ledger path (line 115), then asserts only on ml.DEFAULT_LEDGER (lines 118-120). master_ledger.py:181 skips the write only when "MASTER_LEDGER_PATH" is not in os.environ. When that variable is set, get_ledger_path  | Run the module with MASTER_LEDGER_PATH set to a temp file and check whether a TEST_ISOLATION line is appended there and whether the test still passes. Check whether any harness, CI job or agent wrapper exports MASTER_LEDGER_PATH. In a scratch copy with the guard at master_ledger.py:181 removed, chec |

### C-58ffc978 (1 finding, patch_manager_mods/_shared_src/GameAddr.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6572c3d2f7ca | medium | unverified | medium | 69 | other | worker |  | code-001 | 2c071d8 | Line 69: `constexpr uintptr_t PerformCastSpell = 0x00775700;`. Lines 3-6 say: 'K2Addr.h defines struct K2AddrTable (one field per name here, same spelling ...)', 'kGog (unfilled, all zero)', and 'zero field = unknown = do nothing'. K2Addr.h lines 185-187 list the neighbouring hotbar fields (HudSyncS | Grep the consuming mods (outside this packet) for PerformCastSpell and gameaddr::PerformCastSpell. If anything calls the constant directly, check whether it is guarded by the version SHA. Check whether K2Addr.h should get a matching field with a zero GOG value, and whether checkSteamFields in k2addr |

### C-5c0cbc25 (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b79d18747713 | medium | unverified | high | 315-317 | logic | worker |  | other-033 | 31aa40c, d331219 | def agreement(...): pairs = [...]; return sum(...) / len(pairs) * 100.0 if pairs else 0.0. With no endpoint answers pairs is empty, so 0.0 is printed as Top-1 agreement (lines 333-335). Separately, avg_08b_lat defaults to 0.0 (line 329) and is printed as '0.0ms' (line 337) instead of n/a like the Ke | Set KEV_4B_URL and KEV_08B_URL to a closed local port, point the cache file at an empty object or remove it, run with --limit 3 and --source transcripts, and check that the summary shows 0.0% and 0.0ms. Then check whether the summary should print n/a when a pair list is empty. |

### C-5c687bd5 (1 finding, scripts/kpm_hotswap.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| afc6e9a0d28e | medium | unverified | high | 326-344 | logic | worker |  | other-028 | d331219 | Line 329: `print(f"  {pid}: {dst.name} -> {got[:8]}{'' if got == old else '  (MD5 MISMATCH vs snapshot)'}")`. Line 338: `WARNING: ... config NOT restored, only the DLLs were.` Lines 339-343 then call `ml.log_event("HOTSWAP_ROLLBACK", {...})` unconditionally, and line 344 is `return 0`. A rollback wh | Track whether any DLL mismatched or the config was skipped, and return non-zero and/or log a distinct outcome (for example a failed or partial verdict) in that case. Check whether any caller depends on rollback returning 0. |

### C-5c7c0bdc (1 finding, scripts/compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 20118d0537f2 | medium | unverified | high | 1021-1024 | bug | worker |  | other-036 | b348ca3 | The PASS regex (line 1021) has an optional label group `(?:\s+\([^)]+\))?`, but the FAIL regex (line 1024) `FAIL\s+build:\s+([^\s:]+):\s*(.*)$` has none. A line such as 'FAIL build: shield_bar_src (ShieldBar): compile/link failed' does not match m_fail. build_check.py:40 says 'Exit 1 on any FAIL', s | Call run_build_mods with canned output containing one plain FAIL line and one labelled FAIL line, rc=1, and compare the result ids and counts['FAIL']. Check how build_check.py formats its FAIL line (a literal 'FAIL build' grep found nothing, so the format is built elsewhere) to confirm labels really |

### C-5c88cd7a (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9059eb31f144 | medium | unverified | medium | 734-739 | logic | worker |  | code-009 | 2c071d8, d4ef936 | actionInQueue is set for any T_CAST_SPELL engine entry (line 734), for cur.subType 9 (line 737) and for rn subType 9 (line 739). It feeds shouldReInject (QueueHudLogic.h:579 `if (actionInQueue) return false;`) and the else-if clear (line 778). The item-only predicate became spell-inclusive, and the  | Confirm the subType enum (9 = cast spell, 10 = use item) in the engine or actq header. Then reproduce: buffer a spell, cast a different spell manually while the buffered one is eaten, and check whether re-inject is skipped and buf.active stays true. |

Root cause (Reduce): Re-inject and actionInQueue decisions infer an 'eaten' action from absence in the engine queue plus a follow-leader entry, with no completion signal, and the spell-inclusive predicate (734-739) widened the input without a completion check.

### C-5dc687e5 (1 finding, scripts/run_tests.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0d9f46c44e2a | medium | unverified | high | 863-871 | logic | worker |  | other-032 | 3ec1365, d331219, fadc3c9 | Line 867 adds the exact positional id to target_ids, and line 869 narrows items to ids containing the substring pos_subs. Lines 870-871 then run `if target_ids: items = [i for i in items if i.id in target_ids]`, which intersects the two filter kinds instead of taking their union. For `run_tests.py s | Trace main() with argv `<exact-id> <substring>` where the exact id does not contain the substring. Print items after line 869 and after line 871, then confirm whether union or intersection was intended. Check the positional help text and any docs that describe the syntax. |

### C-5dd90b24 (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| edb391b6492b | medium | unverified | high | 1509-1513 | bug | worker |  | other-022 | WORKTREE | discover_root_fills does `try: gff = read_gff(data); root_ext = _extent(gff.root)` then `except Exception: continue` (1509-1513). Any parse failure silently removes that GUI from the fills map. | Place a truncated or corrupt .gui named like a pnl_pause_pc menu in override/, run a dry run with --frame-edges, and check report.txt and stdout for any mention of that name. The expected result is silence. |

### C-60a1c669 (2 findings, scripts/agent_diff_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 572a431bf016 | medium | unverified | high | 181-183 | logic | worker |  | other-016 | 054ee19, 1fad5c9 | The audit patch is built with `git diff -- <changed_paths>` (line 181). `git diff` ignores untracked files, but changed_paths includes untracked entries. get_git_status() uses `git status --porcelain -uall` (line 44), and the baseline path uses changed_since(), which also runs `-uall` (agent_contrac | In a scratch clone, have a change create only a new file under a mod directory, run agent_diff_check.py without --no-patch, and check whether .claude/subagent_runs/latest.patch exists. Then check whether `git apply -R` on it removes the new file. |
| 8cace570ccc2 | medium | unverified | high | 176-183 | logic | worker | 572a431bf016 | other-039 | 054ee19, 1fad5c9 | Line 44 `["git", "status", "--porcelain", "-uall"]` puts untracked files into changed_paths (default path when no baseline file exists, lines 144-160). Line 181 runs `git diff -- <changed_paths>`, which compares only tracked files against the index, so untracked new files and staged changes produce  | In a scratch worktree, make a change that only adds a new file, run agent_diff_check.py with --json and --mod for a quick build (or --no-patch off), and check that changed_files lists the new file while patch_file is null. Repeat with a tracked edit that is staged with git add and check whether it a |

### C-615d62f8 (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 98200490980a | medium | unverified | high | 1509-1513 | gate-integrity | worker |  | other-023 | WORKTREE | discover_root_fills does `try: gff = read_gff(data); root_ext = _extent(gff.root) except Exception: continue` (lines 1510-1513). data comes from peek_template (1508), which returns None when no copy exists (line 1492 `return chitin_gui_bytes(...)`), so read_gff(None) would raise and be swallowed. | In a scratch workspace, remove one pause-menu GUI from originals/ and make its chitin lookup miss, then run with --frame-edges and --content-aspect. Check whether that GUI is absent from the report with no warning. Compare the count of discovered fills with the count of readable GUIs. |

### C-62f670c8 (1 finding, tests/benchmarks/transcript_prompt_eval.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9b365ee9aedf | medium | unverified | medium | 5-10 | test | worker |  | other-042 | WORKTREE | Every one of the 30 entries carries the same baseline values, e.g. lines 5-10: "baseline_engine": "transcript (real)", "baseline_backend": "cloud", "baseline_model": "openrouter/poolside/laguna-s-2.1:free", "baseline_latency_ms": 160.0, "baseline_confidence": 0.8. The same literal values repeat at l | Find the script or agent that writes this file (the directory has no generator or runner; the Glob and Grep inside tests/benchmarks found only this JSON, so search the scripts/ or tools/ tree for the string "transcript (real)" or for the key baseline_latency_ms). Check whether 160.0 and 0.8 are hard |

### C-6616378b (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 035d6f241dd2 | medium | unverified | medium | 301-310 | test | worker |  | other-009 | 7020d54, 8e68204 | Line 289: `// F4: the 0x740FC0 guarded read needs a readable ecx`. Line 299: `held -> real object (readable ecx: only the HOLD rule may refuse)`. Yet lines 301 (PC -> real object), 303 (unreadable creature -> real object), 307 (instant=0 -> real object) and 310 (log mode -> real object) all call Luo | In LevelUpOverride.cpp, read LuoLeaderInstant_74F423 and the 0x740FC0 readability guard. Check whether an unreadable ecx returns the real object before the creature, PC, instant and mode checks. If it does, rerun lines 301, 303, 307 and 310 with the readable ctlEcxMem. Then confirm the test fails wh |

### C-66eb87e0 (2 findings, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1b8b761af733 | medium | unverified | high | 1782-1800 | bug | worker |  | other-025 | WORKTREE | deploy_one: `if rm is not None: rm.unlink()` (1782-1783) runs before `target.write_bytes(plan["data"])` (1784) and the TXI write (1785-1786). The manifest is saved only at `state.save_manifest(manifest)` (1800). Nothing is wrapped in try. An OSError at 1784 or 1786 leaves the original .tpc deleted,  | In a scratch game dir, deploy an override_tpc source through cmd_loadscreens with Path.write_bytes patched to raise OSError for the target. Then check whether the original .tpc is gone and whether manifest_tex.json has a new entry or the old one. |
| 90d13288aa18 | medium | unverified | medium | 1782-1800 | bug | worker | 1b8b761af733 | other-024 | WORKTREE | deploy_one removes the old override file first (`if rm is not None:` / `rm.unlink()`, lines 1782-1783), then writes the new file (`target.write_bytes(plan["data"])`, line 1784), and only at the end records the manifest entry and saves it (lines 1787-1800). cmd_world catches only RuntimeError around  | Check where the removed override's original is archived (the sources and originals_tex step, outside this excerpt). Then simulate a failing write_bytes for the new target after rm.unlink() on a temp copy and see whether rollback restores the removed file. Consider whether the manifest should be writ |

### C-67ba59a9 (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 366483889826 | medium | unverified | medium | 2210-2212 | gate-integrity | worker |  | other-026 | WORKTREE | `if src is None:` then `log.append(f"  SKIP {name}: no longer in override or TexturePacks")` and `continue` (exceptions at 2207-2209 follow the same SKIP path). A skipped member is never added to plans and never counted in `refused`. The deploy gate at 2237 is `if args.deploy and not refused:` and t | Remove one member's source from TexturePacks and override (or make a recorded ERF source vanish, which resolve_texture raises as FileNotFoundError at line 1623), then run `set ui --upscaled <model> --deploy`. Check that the exit status is 0, the other members are deployed, and the SKIP line is the o |

### C-69fa212a (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1dfcb8bb9191 | medium | unverified | high | 2459-2471 | logic | worker |  | other-025 | WORKTREE | cmd_frame discards the TXI at `img, _ = decode(src)` (2459) and then calls `plan = plan_output(name, src, out_img)` (2471) with no txi argument. plan_output defaults to `txi: str = ""` (line 1700), so image_to_tpc(img, "") writes no TXI (line 907) and no .txi sidecar is planned. By contrast cmd_load | Use tpc_raw_txi on the pnl_pause_pc and kotor2logo sources (override or ERF) and compare with the TXI in the planned output of `frame` and `unsqueeze` (out_tex/). Check whether any keyword present in the source is missing from the output. |

### C-6efeb33b (1 finding, scripts/symlink_pass.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1d360b58038c | medium | unverified | high | 39 | bug | worker |  | other-034 | WORKTREE | `for root, dirs, _files in os.walk(root_dir, followlinks=False):` passes no onerror, so a missing or mistyped root produces no entries. main() (lines 57-58) then prints `Created 0 new case-insensitive symlinks` and returns normally, which exits 0. | Run `KOTOR2_ROOT=/nonexistent python3 -I scripts/symlink_pass.py; echo $?` and expect 'Created 0' with exit 0. Check how the callers at kotor2_toolset.py:48 and _common.py:46 react to a missing alias. |

### C-795f2149 (1 finding, patch_manager_mods/gui_input_src/GuiInputProvider.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d56064e2a823 | medium | unverified | medium | 158-162 | logic | worker |  | code-004 | 68a7739 | Pre-image line o158 was a bare `ensureSubclass();` whose bool result was ignored, so g_disp.subscribe ran regardless. Now lines 158-161 `if (!ensureSubclass()) { ...; return -1; }` return before `int32_t tok = g_disp.subscribe(cb, user);` at line 162. ensureSubclass() (lines 116-127) is the only cod | Check k2input::Client::wheel_subscribe in GuiInput.h (outside this packet, not read) and the mod callers for retry on <= 0. Then reproduce with two subscribers: subscribe A while findGameWindow returns null, then subscribe B after a window exists. Compare whether A's callback fires on the diff base  |

### C-7b007e5e (1 finding, scripts/install_echoes_compatch.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e73d1cf697c4 | medium | unverified | medium | 157-176 | gate-integrity | worker |  | other-027 | WORKTREE | phase_env starts from `**os.environ` (line 158). run_phase_script runs `subprocess.run([sys.executable, str(script_path)], env=env)` (line 176) with no -O flag. The phase-7 gate scripts/verify_all_compatch.py is made of bare `assert` statements (e.g. its lines 89, 112, 125, 161) and ends with `print | With PYTHONOPTIMIZE=1 exported, run verify_all_compatch.py against a deliberately altered copy of the game (for example a module whose list counts are wrong) and check the exit code. Also confirm whether phase_env passes the variable through unchanged. Decide whether phase 7 should refuse to run und |

### C-7d0bdb05 (1 finding, scripts/router_eval.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a208ec98894c | medium | unverified | high | 955-967 | gate-integrity | worker |  | other-031 | WORKTREE | cmd_run gets its cases from _load_cases_or_exit (907-919), which only runs load_cases_raw and validate_case_lines. The distribution checks (total 40, per-stage counts, at least 12 hard, at least 3 hard per stage, rules under-route at least 50%) live in validate_cases (196-222), which is called only  | In a scratch copy that includes .claude/router_eval, truncate cases.jsonl to 30 lines or drop hard cases, then run `python3 -I scripts/router_eval.py run --engines rules --passes 1 --run-id gate-probe` and `score --run gate-probe` (rules needs no API). Confirm both run without any validate error. Ch |

### C-7dd30fea (1 finding, scripts/06_deploy_assets.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 111569cb3a6d | medium | unverified | high | 195-201 | logic | worker |  | other-014 | WORKTREE | `return {p.name.lower(): p.name for p in d.iterdir() if p.is_file() and not p.is_symlink()}`. The docstring says two real spellings such as 'Foo.mod' and 'foo.mod' can coexist and that a deploy must reuse the existing spelling or skip. The dict comprehension keeps the last spelling it sees and never | In a scratch directory, create 'Foo.mod' and 'foo.mod' as two real files, call existing_spellings on it, and check which spelling survives. Then run deploy_modules against that scratch Modules/ dir and check whether the other spelling is left unchanged. |

### C-81869ff6 (1 finding, scripts/02_merge_2da_and_tlk.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5d721af23495 | medium | unverified | medium | 116 | logic | worker |  | other-013 | WORKTREE | Line 116 opens `read_2da(LIVE_OVERRIDE / n)` with the exact lowercase name, and line 233 writes back to `LIVE_OVERRIDE / name` the same way. Line 30 imports only GAME_DIR and STAGING_DIR from _common. _common.py lines 44-47 say: 'Linux can hold "701kor.mod" and "701KOR.mod" as two files, and Wine th | List the live override for case-insensitive duplicates of each table name in the script (appearance, placeables, heads, spells, effecticon, upgrade, ambientmusic, modulesave, globalcat, dialog.tlk). Also check whether the script's table reads ever resolve to a different file than find_one_ci would. |

### C-8de81efc (1 finding, scripts/crash_triage.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| dd1f356620c9 | medium | unverified | medium | 110-119 | logic | worker |  | other-020 | WORKTREE | The loop reads the file from the first line and breaks at 'if len(matches) >= args.limit: break' (lines 118-119). With the default limit of 40 it keeps the first 40 signature lines. offsets[0] (lines 133-135) comes from the earliest match. SIGNAL_REGEX (line 29) includes err:module and err:seh, whic | Count SIGNAL_REGEX matches in a real Proton log and check whether the last signature falls after the 40th match. Run log with a much larger --limit on the same file and compare the VERDICT offset with the default run. |

### C-909df8a6 (1 finding, scripts/swarm_argv_probe.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5bf8a785b9b1 | medium | unverified | high | 9-10 | test | worker |  | other-034 | fdee555 | Docstring line 9-10: 'Compares the argv each engine passes to `claude -p`, plus the content of the three file-valued inputs (settings, schema, system prompt).' Lines 124-125 and 136-137 say the settings and system-prompt values 'are placeholders: their content is NOT compared'; line 189 lists them u | Compare the docstring with normalise() (lines 123-146) and the summary keys at line 189. Decide whether the docstring or the code is intended, and whether the settings and system-prompt files copied at lines 93-94 should be hashed. |

### C-9506dc85 (1 finding, patch_manager_mods/mouse_probe_src/MouseProbe.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ed6f9f335c3f | medium | unverified | medium | 305-306 | memory-safety | worker |  | code-008 | b2c3e46 | Lines 288-290 validate msgp only through reads: `rd(msgp, 4, &id)` and `rd(msgp, 0xC, &lp)`. Line 306 then stores through the same pointer: `*reinterpret_cast<int*>(msgp + 4) = clicklogic::MsgLDbl;`. The gate at line 305 is enabled by default (line 140 `synth_dblclk` defaults to 1). Header line 3 sa | At 0x72E039 (OnMouseMessage), check that ebp+8 points to writable memory owned by the dispatcher for the duration of the call, and whether msg+4 is read again after the probe returns. Confirm clicklogic::MsgLDbl equals 0x203 in ClickLogic.h. Confirm the gate at line 305 (shouldSynthesizeDblClick in  |

Root cause (Reduce): The mouse probe is documented as read-only apart from one sanitizeNorm path, but it performs additional engine writes (the message-struct rewrite at 305-306 and the unguarded this-field stores at 220-221), and the header does not disclose the first.

### C-95ddb575 (1 finding, scripts/check_txi_cycles.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 96d2527bd58e | medium | downgraded | medium | 63-64 | gate-integrity | worker |  | other-017 | WORKTREE | erf_textures() opens with `if not names or texpacks is None or not texpacks.is_dir(): return found` (63-64), which returns an empty dict with no message. main() (107) derives the default as `args.override.parent / "TexturePacks"`, which for a relative override argument resolves against the cwd. The  | Build a scratch override/ with a cycle .txi whose stem has no .tpc or .tga there, and a TexturePacks ERF holding a non-flipbook texture of that name. Run the script once with --texturepacks pointing at a nonexistent directory and once with the correct one. The first run is expected to exit 0 silentl |

Root cause (Reduce): These gate scripts return exit 0 when nothing was scanned or found, or silently skip an input, so an empty run looks like a pass.

### C-96401923 (1 finding, scripts/case_dedup.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b2668ae7e31c | medium | unverified | medium | 66-74 | bug | worker |  | other-017 | WORKTREE | `others` excludes symlinks (66-67: `and not os.path.islink(...)`), so an existing lower-case symlink is never removed. Then 73-74 run `os.rename(os.path.join(d, w), os.path.join(d, low))`. On POSIX, rename(2) silently replaces an existing directory entry, so the alias the docstring says is kept on p | In a scratch directory, create Tel_SFtr.tpc with content A, TEL_SFtr.tpc with content B, and a symlink tel_sftr.tpc pointing at Tel_SFtr.tpc. Run case_dedup.py on that directory with --keep Tel_SFtr.tpc and then ls -l. Check whether the symlink survived. Also read symlink_pass.py (not in packet) to  |

### C-9713a577 (1 finding, scripts/04_merge_modules.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a498027bbd1b | medium | unverified | medium | 110-114 | bug | worker |  | other-014 | WORKTREE | `reply_list = dlg_gff.root.get_list("ReplyList")` / `reply_list[40].set_resref("Script", ResRef("spwnteltark1"))` / `print(f"  Hooked 'spwnteltark1' script into grenn.dlg ReplyList[40]")`. The node is chosen by a hard-coded index, with no check of its length, its Text, or whether its existing Script | In a copy of the live 221TEL.MOD, read grenn.dlg and print len(ReplyList) and the Script (and Text) of ReplyList[40] before the write. Check whether the Script is already non-empty. Compare against the staged grenn.dlg reply that has the spwnteltark1 hook. |

### C-9857b2a5 (1 finding, scripts/06_deploy_assets.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6001871962be | medium | unverified | high | 233-238 | gate-integrity | worker |  | other-014 | WORKTREE | `target = live_dir / f.name` / `shutil.copy2(f, target)` / `copied += 1` in deploy_stream_music (235-238). deploy_movies (250-253) does the same. Neither checks whether the target exists. deploy_stream_voice (272-276) skips existing files with `if not dst_file.exists()`. | Count the staged StreamMusic and Movies names that already exist in the live StreamMusic/ and Movies/ dirs. For any that overlap, compare hashes before and after deploy_stream_music or deploy_movies runs against a scratch copy. |

### C-9992e0a1 (1 finding, scripts/test_plan_manager.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7a9c9046118e | medium | unverified | high | 256-267 | test | worker |  | other-038 | 5b856ce, d331219 | test_delegate_dry_run runs self.run_cli("--plan", str(test_plan), "delegate", "DG1", "--dry-run") with no save/restore of .claude/active_plan.txt. plan_manager.py:771-774 does `if explicit_plan and not getattr(args, "handoff_dir", None): ACTIVE_PLAN_FILE.write_text(str(plan_path))`, and plan_manager | Put a known value in .claude/active_plan.txt, run `python3 scripts/test_plan_manager.py TestPlanManager.test_delegate_dry_run`, and diff the file. Check what discover_active_plan (plan_manager.py:143) returns when the pointed-to file is gone. Check whether the delegate subparser accepts --handoff-di |

### C-9a91f972 (1 finding, scripts/plan_manager.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fe12f10d0659 | medium | unverified | high | 498-500 | logic | worker |  | other-029 | d331219, f6231a3 | Lines 498-500: if not target_str.strip("*_ "): print(f"Error: phase argument {target_str!r} has no usable identifier; plan not modified."); return. cmd_mark_done returns None on every path, and main calls it without checking (line 786), then unconditionally rewrites ACTIVE_PLAN_FILE, regenerates the | In a scratch repo run `python3 scripts/plan_manager.py mark-done '**'` with an active plan set; check the exit status and whether 'SAVED STATE' prints with an empty name. Confirm that cmd_mark_done has no non-None return path that main inspects. |

### C-9b795ffb (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3f686ced9eb9 | medium | unverified | medium | 2376-2385 | gate-integrity | worker |  | other-026 | WORKTREE | `if args.deploy and not refused:` then `for name, src, plan in plans:` wraps `deploy_one(ws, ...)` in `try ... except RuntimeError as exc: failed += 1; log.append(REFUSED)` and continues with the next texture. deploy_one raises per texture (e.g. 'Refusing to overwrite foreign', line 1758) and its ch | Pick two eligible textures for `world <prefix> --upscaled <model> --deploy`. After the upscale, edit the second texture's override file so deploy_one raises 'Refusing to overwrite foreign'. Run the command, then inspect override/ and manifest_world.json: the first texture is expected to be present i |

### C-9c3a6557 (1 finding, patch_manager_mods/queue_hud_src/QueueHudLogic.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b5a62bbbd646 | medium | unverified | low | 583-584 | logic | worker |  | code-009 | 32c8125, d4ef936 | shouldDispatchNext now also requires !haveCur && nrn == 0 (line 584). The caller passes s.haveCur and s.nrn from the engine snapshot (QueueHud.cpp:784). The previous gate was engineQueueLen == 0 alone; the comment at QueueHud.cpp:783 documents the change. queue_hud_logic_test.cpp:1199-1212 checks ea | Log haveCur and nrn for each party member while items are buffered and the member is idle out of combat. Check whether a follow-leader or other persistent current action keeps haveCur true, which would starve the buffer. |

### C-9e1359ea (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cad51433cd4e | medium | unverified | high | 1618-1622 | bug | worker |  | other-022 | WORKTREE | `if only is None:` then `for stale in out_dir.glob("*.gui"): if stale.stem not in generated: stale.unlink()`. Every .gui in out_dir that this run did not produce is deleted. The code does not check that out_dir differs from override_dir (game_dir / "override", line 1736) or from any other shared fol | Put an unrelated x.gui in a directory, run with --out-dir set to that directory and without --only, and confirm the file is unlinked. Then check whether the script refuses an out_dir equal to override_dir or whether it only removes files that appear in the manifest. |

### C-a0beb271 (1 finding, tests/suites.toml)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2d70e3779175 | medium | unverified | low | 176-204 | bug | worker |  | other-041 | a59cd6f | The fix suites run `python3 -I fcommon.py --self-test` (line 176) and the same form for the other four (183, 190, 197, 204) with `cwd` set to the fix directory. `-I` implies `-P`/`-s`, and under isolated mode the script's own directory is not placed on sys.path. Sibling-module imports such as `impor | Where the fix files exist, run `python3 -I fix_intake.py --self-test` from .claude/review/fix and look for ModuleNotFoundError. Grep those scripts for `sys.path` and their `import` lines to see whether they add their own directory. |

### C-a24fcb2c (1 finding, scripts/normalize_diversity_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 035056f3b915 | medium | unverified | high | 113-123 | logic | worker |  | other-029 | f6231a3 | Lines 117-118 keep the first file for each full name (catalog[lkey] set only when absent), but lines 122-123 'if base not in catalog or ext == ".tpc": catalog[base] = ...' overwrite the base key on every .tpc. So for a texture base such as n_commm01 present as .tpc in two roots, the later root (or l | Put identical basenames (e.g. n_commm01.tpc) in two search roots of a scratch fixture, run normalize_assets into a temp out_dir, and compare the copied N_CommM01.tpc bytes with each source. Also print catalog['n_commm01'] and catalog['n_commm01.tpc'] to show the two keys disagree. |

### C-a5531832 (1 finding, scripts/verify_2da.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 54f70ce8059b | medium | unverified | high | 83-99 | logic | worker |  | other-040 | WORKTREE | `--min-blank-prefix` is a bare `type=int` with no lower bound (lines 98-99). The test at line 83 is `if prefix >= min_blank_prefix and prefix < total_rows:`. With N=0, any table whose first column is filled gives prefix=0, which passes both comparisons, so the file is reported. The message then read | Run `python3 scripts/verify_2da.py <dir with healthy 2DAs> --min-blank-prefix 0` and count the flagged files. Check that the reason string for prefix 0 is malformed. Check whether argparse should reject values below 1. |

### C-a777a637 (1 finding, scripts/test_clef_matcher.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1bbb5aac0b54 | medium | unverified | medium | 140-157 | test | worker |  | other-035 | 5b856ce, d331219 | test_ambiguous_tie_fallback patches only query_clef_engine (line 140) and asserts self.assertIn('inkling', res['model']) (line 157). route_task rewrites the model when it is in the cooldown set: clef_matcher.py:369-380 builds model_str, then 'if model_str in cooled:' swaps in the first non-cooled ch | Read get_cooled_down_models (clef_matcher.py:206-223) to see what state it reads. Add @patch('clef_matcher.get_cooled_down_models', return_value=set()) to test_ambiguous_tie_fallback and rerun. If the test passes only with the patch, the dependency is confirmed. This finding rests on a read made aft |

### C-a7f89285 (1 finding, scripts/run_tests.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9cc6aff95748 | medium | unverified | low | 644-649 | logic | worker |  | other-032 | 3ec1365, d331219, fadc3c9 | Line 644 adds KPM_DIR and build_check.py to src_dirs. Lines 646-649 now walk each directory with rglob('*') and compare the mtime of every regular file against the DLL mtime. The old code (o646-o649) only looked at .cpp/.h/.hpp/.inc/.def. KPM_DIR defaults to a home-directory path (line 69). | Run _ensure_built_dll twice for one mod with no edits. If build_check.py runs the second time, list files under KPM_DIR and mod_dir newer than the newest build/*.dll to find the writer. Also check whether build_check.py writes into either tree. |

### C-a9526289 (1 finding, scripts/compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d7b084182617 | medium | unverified | low | 1053-1056 | gate-integrity | worker |  | other-019 | 3ec1365 | The new synthesized ERROR at 1053-1056 fires only when `b_proc.returncode != 0`. If build_check exits 0 and no line matches the PASS or FAIL regexes at 1020-1024, no result and no count is recorded for the requested build mods. build_check.py:493 is `return rep.finish("build_check")`, and the litera | Run scripts/build_check.py with one --mod and capture stdout. Confirm that the PASS line matches the regex at compat_check.py:1021. Then compare the count of parsed build results against len(build_mods) and check that a shortfall is recorded as ERROR. |

### C-af3e05db (1 finding, scripts/02_merge_2da_and_tlk.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f88d7b0ead99 | medium | unverified | high | 228-229 | bug | worker |  | other-013 | WORKTREE | The for/else at lines 221-229 prints `Warning: Could not find CONSTI 114 in ekun_holo.ncs` when no match is found, then execution continues. Step 13 still writes the merged 2DAs and TLK. Line 244 always prints `Phase 2 (2DA, TLK, GFF re-indexing) 100% COMPLETE and VERIFIED!`. | Run the phase against a staging ekun_holo.ncs with no CONSTI 114 and confirm the warning is followed by a success banner. Separately, check whether the compiled ekun_holo.ncs contains CONSTI 114 more than once, since the loop breaks after the first match (line 227). |

### C-c10845fc (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6e6c819015a0 | medium | unverified | medium | 2576-2579 | gate-integrity | worker |  | other-025 | WORKTREE | The game-running gate: `if (args.rollback or getattr(args, "deploy", False) or getattr(args, "world_rollback", False)) and is_game_running():` guards every deploy and rollback. In scripts/generate_ultrawide_gui_overrides.py:693-695, is_game_running() wraps the `ps -eo pid,args` call in `try: ... exc | In a shell whose PATH has no `ps` (or with ps replaced by a failing stub), call is_game_running() and confirm it returns False without any message. Then check whether `main()` at line 2576 has any other signal (e.g. a lock file or a process list read another way) that would still refuse the deploy. |

### C-c11a55bf (1 finding, patch_manager_mods/action_hotbar_src/ActionHotbar.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bc58247fd299 | medium | unverified | medium | 358-364 | logic | worker |  | code-002 | 25d4e42, 7df1b4a | Loop reads: `uintptr_t mem = party::member(t, i); if (mem == c \|\| (mem != 0 && creatureOid(mem) == oid)) { partySlot = i; break; }`. The OID clause has no guard for the invalid/sentinel ids 0 and 0xFFFFFFFF, and it breaks at the first OID-only match before later slots are checked by pointer. The com | In computeCharacterKey, log partySlot together with the match source (pointer or OID) whenever oid is 0xFFFFFFFF. Check whether a non-Exile controlled creature can read 0xFFFFFFFF in the same session as the Exile (Ebon Hawk case). Check whether the OID fallback should be skipped for oid 0 and 0xFFFF |

### C-c388d284 (1 finding, scripts/query_ghidra.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6a2d865f4e25 | medium | unverified | high | 164-167 | logic | worker |  | other-030 | b1ccd94, f6231a3 | `raw_name = item.get("name", f"field_0x{off:x}")` (164), then `clean_name = re.sub(r"[^a-zA-Z0-9_]", "_", raw_name).strip("_")` (165) and `if not clean_name or clean_name[0].isdigit(): clean_name = f"field_{clean_name}"` (166-167). An empty or all-punctuation name yields clean_name == "", so the fal | Run `python3 -I scripts/query_ghidra.py struct <Class> --cpp` against a DB where a class has two rows with name '' (or NULL, see f2). Inspect the emitted member names for duplicates. Check whether the fallback should include the offset whenever clean_name is empty, and whether duplicate sanitized na |

### C-c3d2cabe (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f284f8dc0be3 | medium | unverified | medium | 2178-2179 | bug | worker |  | other-026 | WORKTREE | `(out / "upscaled.json").write_text(json.dumps(meta, ...), encoding="utf-8")` runs inside the loop after each texture, and again at line 2179. The write is in place (no temp file or rename), and the file holds the only record of every upscale. Readers take no lock: load_upscaled_meta (1997-1999) cal | Check whether any atomic-write pattern is used elsewhere in the file (grep for os.replace, or tempfile in a write path). To reproduce, start `upscale` on several textures, kill it during a write (or add a delay in a scratch copy), then run `upscale --skip-done` and `set` and look for JSONDecodeError |

### C-c58ccced (1 finding, scripts/plan_manager.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 695f3ed2a0f8 | medium | unverified | medium | 96-114 | logic | worker |  | other-029 | d331219, f6231a3 | Line 96 builds candidates from unsorted REPO_ROOT.glob(...) results across patch_manager_mods and top-level *NN*.md; lines 112-114 fall back to 'return candidates[0].resolve(), rest' when no candidate's words overlap the rest text. main (lines 771-774) then rewrites ACTIVE_PLAN_FILE to the resolved  | In a scratch repo create patch_manager_mods/25_a.md and 25_b.md, each with a T4 row. Call resolve_plan_shorthand('25:T4') and check which file returns, then run `plan_manager.py resume '25:T4'` and inspect .claude/active_plan.txt. Compare with a sorted() candidate list. |

### C-c5ef537d (1 finding, scripts/compat_graph.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f451390fed3d | medium | unverified | high | 546-550 | logic | worker |  | other-020 | 3ec1365 | Line 549 looks up self.mod_colocated_tests.get(self._rel(Path(root)), []). _rel (line 180) returns a repo-relative path such as 'patch_manager_mods/<mod>'. mod_colocated_tests is keyed by _find_mod_dir results (line 307 setdefault(mod_dir, ...)), and _find_mod_dir returns pm-relative paths (line 197 | Build a CompatGraph over a fixture tree with patch_manager_mods/<mod>/manifest.toml, a colocated *_test.cpp, and a test.toml with a top-level covers list. Print covers_map and the keys of mod_colocated_tests. Expect the list form to produce no covers_map rows, which shows the key mismatch. The per-t |

### C-c7e81b6e (2 findings, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a1ff4216d929 | medium | unverified | high | 2376-2385 | logic | worker |  | other-024 | WORKTREE | The cmd_world docstring (line 2276) says deploy goes into override/ "(all or nothing)". The code only enforces that for upscale refusals (line 2376 `if args.deploy and not refused:`). The deploy loop at 2377-2384 wraps each deploy_one in `try ... except RuntimeError`, counts the failure, and continu | Read cmd_world's deploy loop and deploy_one. Build a fixture with two eligible textures where the second has a foreign override file, run `world --deploy` on it, and check whether the first texture was written and recorded in manifest_world.json. Then decide whether the docstring or the loop is the  |
| 10c7a79d6f6a | low | unverified | high | 2376-2384 | logic | worker | a1ff4216d929 | other-025 | WORKTREE | cmd_world's docstring at line 2276 promises deploys are 'all or nothing'. The code gates only upscale refusals (`if args.deploy and not refused:` at 2376). Inside the loop, `deploy_one(ws, ...)` errors are caught per texture (`except RuntimeError as exc: failed += 1; log.append(...)`, 2382-2384) and | In a scratch copy of the game dir, run `world --upscaled MODEL --deploy` after creating a non-matching file at the target path of the second planned texture (after the survey, before deploy). Check override/ and manifest_world.json for the earlier members. |

### C-c92e6b20 (2 findings, scripts/compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1cbacdcd03bc | medium | downgraded | medium | 357-358 | gate-integrity | worker |  | other-036 | 5b856ce | narrow_plan_to_gog_scoping ends with `plan.items = narrowed_items` (357) and `plan.gaps.clear()` (358). Gaps are graph-wide (compat_graph.py:849-854 _detect_contract_gaps: 'includes Common headers but owns no *_contract_test.cpp'), not tied to the touched file. main() returns 3 under --strict when p | Use a tree with a known gap (a mod that includes a Common header and has no *_contract_test.cpp). Run compat_check.py K2Addr.h --list --strict with and without a scratch GOG-only edit in makeGog, and compare exit codes and GAP lines. Also confirm _detect_contract_gaps is called from graph.affected() |
| 1ea820031439 | medium | downgraded | medium | 357-359 | gate-integrity | worker | 1cbacdcd03bc | other-019 | 3ec1365 | Line 357 sets `plan.items = narrowed_items`, which keeps only kind == "build" items and the cpp:k2addr_test test (lines 341-355). Line 358 calls `plan.gaps.clear()`, and line 359 appends "GOG-only: Steam runtime unchanged". This runs from lines 922-923 when K2Addr.h is the only touched file and is_k | Check Doc 22 U11 to see whether dropping wine and suite items and clearing gaps is intended. Run `compat_check.py --list --strict` on a GOG-only K2Addr.h edit, once with and once without the narrowing branch, and compare plan.gaps and plan.items. Also check find_k2addr_gog_range and the hunk-range t |

Root cause (Reduce): narrow_plan_to_gog_scoping clears plan.gaps, which are graph-wide, so a K2Addr-only change drops gap reporting that --strict reads.

### C-cc8c3b38 (2 findings, tests/suites.toml)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3e518afd3cbb | medium | unverified | high | 158-171 | build | worker |  | other-041 | a59cd6f | The three swarm-lib suites depend on home-directory paths outside the repo: `requires = ["~/.claude/swarm/tests/test_invariants.py"]` (line 159), `~/.claude/swarm/tests/test_sandbox.py` (165), `~/.claude/swarm/tests/test_home_probe.py` (171). The header (suites.toml:13-14) says a missing requirement | On a machine without ~/.claude/swarm, run `python3 scripts/run_tests.py --list` and confirm the swarm-lib-* items show SKIP. Then run the full runner and check its exit status. Also confirm whether the swarm library is tracked anywhere in this repo. |
| 8b2962d15175 | low | unverified | high | 158-204 | build | worker | 3e518afd3cbb | other-041 | a59cd6f | The eight new swarm suites use `cmd = ["python3", "-I", ...]` (e.g. line 158, 176, 204). The runner rewrites only a literal leading "python": `if cmd[0] == "python":` (run_tests.py:382), then sets `sys.executable` (387) or the venv interpreter (384). The header documents only "python" (suites.toml:8 | Run `--list` and compare the resolved command of a swarm-* item with the runner's `sys.executable`. Check `which -a python3` in the environment where the gate runs. Changing the leading token to "python" would make the swarm suites follow the same path as the rest of the file. |

### C-ccc8c194 (2 findings, research/companion_levelup/tools/parity/parity.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b5a7154f89eb | medium | downgraded | medium | 33 | build | worker |  | other-009 | 7020d54 | parity.py:33 adds `"-I", os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common")` to the g++ argv of every build() call (gate, apply, ui, playthrough, m32). The base line had no such -I. gate_test.cpp:21 and apply_test.cpp:86 `#include "TestHarness.h"`, but Glob ` | On a checkout without the ~/Games tree, rebuild gate_test.cpp with the exact argv that build() produces (drop only the home -I) and confirm the first error is 'TestHarness.h: No such file'. Then locate where TestHarness.h is actually versioned and point the -I at a repo-relative directory. Caveat: m |
| 6fdc7c29d62d | medium | unverified | medium | 32-34 | build | reduce | b5a7154f89eb |  | 7020d54, 8e68204 | parity.py build() passes -I for stub, stub/_shared and stub/inc, plus the home-directory Patches/Common. It does not pass the repo's tests/include directory. Glob **/TestHarness.h returns only tests/include/TestHarness.h. Per the bundle, gate_test.cpp:21 includes "TestHarness.h". Why it matters: The | In a clean clone with no ~/Games workspace, run the parity build argv for gate_test.cpp and see whether TestHarness.h resolves. Then check whether stub/ or tests/include/ was meant to be on the -I path. |

Root cause (Reduce): Build configs add a home-directory Patches/Common include path that a clean checkout does not have, so test builds depend on one user's workspace layout.

### C-cdc52b06 (1 finding, scripts/agent_diff_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f05db6672105 | medium | unverified | high | 206-207 | gate-integrity | worker |  | other-016 | 054ee19, 1fad5c9 | On PASS the text line is printed unconditionally: `print(f"PASS: {len(changed_paths)} file(s) changed, scope clean, {len(target_mods)} mod(s) built clean, tests passing.")` (line 207). When --scope or --test was omitted (lines 164-165, 102-103), neither check ran, yet the line still says 'scope clea | Run the script without --scope or --test and compare the text line at 207 with the JSON fields scope_violations and test_output. |

### C-cea8580f (1 finding, patch_manager_mods/queue_hud_src/QueueHudLogic.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f77ae8184a09 | medium | downgraded | low | 575-580 | logic | worker |  | code-009 | 32c8125, d4ef936 | shouldReInject returns hasFollowLeader whenever the action is absent from the engine queue, the buffer is active, and it is not pending, cancelled or over the cap (lines 578-580). The caller takes that branch before the else-if that clears active after 500 ms (QueueHud.cpp:744 and 778). A spell that | Trace a follower's engine queue across the tick a buffered spell leaves the queue. Check whether 0x3D is present on that tick after a normal finish, not only during a real wipe. Check whether the engine exposes a completion signal (cur/rn state, or a spell-slot or cost change) that could distinguish |

Root cause (Reduce): Re-inject and actionInQueue decisions infer an 'eaten' action from absence in the engine queue plus a follow-leader entry, with no completion signal, and the spell-inclusive predicate (734-739) widened the input without a completion check.

### C-d05e4b62 (1 finding, scripts/kotor2_toolset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1f995ce511e9 | medium | unverified | high | 80-83 | gate-integrity | worker |  | other-028 | WORKTREE | `if verify_script.exists(): checks.append(("Echoes compatch", [verify_script]))` / `else: print("[!] verify_all_compatch.py not found in script directory.")`. The else branch only prints; `failed` is not touched, so the summary at line 97 (`'FAILED: ' + ... if failed else 'all checks passed'`) repor | In a scratch copy of the tree, remove scripts/verify_all_compatch.py and run `python3 scripts/kotor2_toolset.py verify --game-dir <dir>`. Check whether the exit code is 0 and whether the last line says 'all checks passed'. Compare with the other checks, where a missing script exits 2 and is counted  |

### C-d0dede54 (1 finding, scripts/generate_diversity_2da.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d00ed5f13e80 | medium | unverified | medium | 378-398 | logic | worker |  | other-021 | 3ec1365 | Line 378 `"normalhead": "198",`, line 388 `"normalhead": "200",`, line 398 `"normalhead": "204",`. In the same packet the Twilek_Commoner entries were switched to names (lines 343/351/359/367, e.g. `"normalhead": "llk_Ahrnell"`) and are resolved via `head_mapping` at lines 534-536. The three dancer/ | Read the base heads.2da that main() loads from the live override (not in this snapshot) and check the labels/head names at rows 198, 200 and 204. If they are intended named heads, add them to NEW_HEADS or otherwise resolve them through head_mapping so all entries use one convention. Also grep the C+ |

### C-d2a13545 (1 finding, scripts/swarm_argv_probe.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| acf48427b321 | medium | unverified | low | 75-82 | other | worker |  | other-034 | fdee555 | The bundle harness sets `CLAUDE_BIN="{root}/bin/claude"` (line 77), but the fork harness (81-82) sets no CLAUDE_BIN and relies on run_map.sh to call `claude` through PATH. The subprocess env at 107-109 keeps `HOME` (line 107), so a real claude binary resolved by the fork engine would see the user's  | Once .claude/review/run_map.sh is available, grep it for `claude` and for any CLAUDE_BIN default or absolute path. Run the probe with a PATH that contains no real claude and check that the shim log has one entry per case. Check whether passing HOME at line 107 exposes login state to a real binary. |

### C-d2af9f5b (1 finding, scripts/02_merge_2da_and_tlk.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1406bc629c1d | medium | unverified | medium | 154 | bug | worker |  | other-013 | WORKTREE | Step 5 calls `copy_2da_row(echoes["spells.2da"], ECHOES_SPELL_ROW, live["spells.2da"])` with no remap dict. Step 6 for effecticon passes `{"namestrref": tlk_map}` (line 159), and step 9 passes `{"areaname": tlk_map}`, so the script knows TLK string refs must be re-indexed. Spell row 282 is copied ra | Print the header of the staging spells.2da and look for strref-type columns. Check the value in row 282 and compare it with ECHOES_TLK_START and len(live dialog.tlk). If live TLK length differs from 136375, tlk_map is not the identity and the spell string ref is wrong. Repeat the header check for pl |

### C-d2fd6745 (1 finding, scripts/run_tests.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f6408200506e | medium | unverified | medium | 624-626 | logic | worker |  | other-031 | WORKTREE | The directory fallback in resolve_mod_for_smoke changed from `if clean and (clean in stem or stem in clean):` to `if clean and clean == stem:` (625). A kpatch stem that carries a suffix relative to the directory, such as 'foo_patch' for dir 'foo_src', no longer matches and the function returns None  | List smokes in patch_manager_mods/perf_lab_src/harness/smokes.toml with dll_from = "build" and their kpatch stems. Call resolve_mod_for_smoke on each under the base-commit and worktree versions of the function. Check whether the _static_common lookup already resolves them, which would make this fall |

### C-d663687b (1 finding, scripts/05_merge_dlgs_and_jrl.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 811a5531ca00 | medium | unverified | medium | 77-81 | logic | worker |  | other-014 | WORKTREE | `for idx in HANDMAIDEN_REPLACED_ENTRIES:` / `if idx < len(e_live) and idx < len(e_stage):` / `e_live[idx] = copy.deepcopy(e_stage[idx])`. The whole live entry is replaced, so any live RepliesList links or Script on entries 418-420 are discarded. Out-of-range indices are skipped silently, yet line 81 | Compare live and stage handmaiden.dlg EntryList[418:421] (RepliesList, Script, Text). Look for links or fields present in live but missing from stage. Then check len(EntryList) in both files to see whether the skip branch can run. |

### C-dd552930 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 89fc8d95002c | medium | unverified | low | 1539-1545 | logic | worker |  | code-006 | 97c1bd8, dc4bb61 | void guidLoad(void* table, int result) { ... if (table && guidEngineOk()) readGuid(table, &g, &found); g_guid = found == 32 ? g : 0; ... KP_LOG(chPlay, 'GUID load g%08x found=%d/32 result=%d table=%08x', ...). The 'result' parameter is used only in that log line; no branch checks it. The hook note a | Check the success value of the LoadFromSave return (0x656710) and whether a failed load can still reach the post-load hook at 0x65675B. Check whether the globals table is cleared before the save's globals are read, so a failed or partial load cannot leave the previous GUID's 32 bits in place. If the |

### C-dfea305d (1 finding, scripts/04_merge_modules.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| c740372f2a81 | medium | unverified | medium | 155-162 | logic | worker |  | other-014 | WORKTREE | `for d in doors:` / `if x == 19.2 and y == 170.2:` / `d.set_resref("TemplateResRef", ResRef("door_corig12"))` / `d.set_string("Tag", "door_corig12")` / `break`. There is no else branch. If no door matches, nothing is changed and no message is printed, yet the function still reports 'Saved merged ... | Dump the Door List X/Y values from the live 302nar.git and confirm whether an entry at (19.2, 170.2) exists and what its Tag and TemplateResRef were before the edit. Then check whether the function can reach line 189 with no door changed. |

### C-e58d5740 (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| dca7ccbc8421 | medium | unverified | high | 1618-1622 | logic | worker |  | other-023 | WORKTREE | `if only is None: for stale in out_dir.glob("*.gui"): if stale.stem not in generated: stale.unlink()`. out_dir is taken unchecked from --out-dir (main line 1734, `Path(args.out_dir).resolve()`). deploy_file (1403-1408) refuses to overwrite foreign files, but this sweep deletes any *.gui in the direc | Create a scratch directory with an unrelated foo.gui, run the script with --out-dir pointing there, and check whether foo.gui is removed. Consider limiting the sweep to files recorded in the manifest or to the default out/ directory. |

### C-e5d38863 (1 finding, patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d74365c9d82d | medium | unverified | medium | 696-697 | logic | worker |  | code-003 | 4519b96, eff4a32 | refreshCache sets `c0.have = 0 < cnt && v[0].ok;` (696) and clears only pk/apk when false (697). uiFrame then re-asserts `g_card0.have = true;` on every frame the slot exists (777). Nothing in the packet sets have back to false: hideAll (554-561) resets only seen and st. c0.m is never cleared, and c | Check shui::resolveState with have=true, pk.pick<0 and cur==max (and with stale cur/max) to see which look is shown when the leader is gone. Check whether v[0].ok can be false while the HUD slot k==0 still exists. Compare with the companion have semantics at 674-675, and consider whether c0.m should |

### C-e5ff6972 (1 finding, scripts/query_ghidra.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| df0d6ee734db | medium | unverified | medium | 163 | logic | worker |  | other-030 | b1ccd94, f6231a3 | `c_type = type_map.get(t_raw, "uint32_t")` (163). Any DB type outside the 8 keys in type_map (lines 148-157) silently becomes a 4-byte uint32_t. The `size` column is selected by query_struct (line 133) but format_cpp_struct never reads it (lines 158-168), and offsets appear only as comments with no  | Run `SELECT DISTINCT type FROM struct_fields` on the bundled DB and compare the result with the type_map keys. For each unmapped value, check its real size against the `size` column. Then compare `sizeof` of a generated struct with the largest offset plus size for that class. |

### C-e7523283 (1 finding, scripts/gpu_mode.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2f3acf2bdd25 | medium | unverified | high | 167-174 | gate-integrity | worker |  | other-026 | 93385ec, d331219 | Module docstring line 7: '3. game: AI models unloaded (>10.2 GB free VRAM for KOTOR II / games)'. In cmd_game, free_mb is computed at line 169 and only printed at line 173. The only failure check is line 170: `if not unload_ok or total_mb == 0:`. So 'OK: VRAM ready' is printed for any amount of free | Mock nvidia-smi (or run on a host) so that free VRAM is well below 10.2 GB while a model or kev is resident, and run `gpu-mode game`. Expect exit 0 and 'VRAM ready' under the current code. Also run it with the lms binary path missing and confirm no unload message is printed and the verdict is still  |

### C-e75dc95d (1 finding, research/companion_levelup/tools/parity/stub/LuoLogHost.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e3ae385c8f94 | medium | unverified | medium | 9 | logic | worker |  | other-012 | 7020d54 | Lines 3-4 say: "KpLog also keeps the fallback file open, so remove()ing it leaves later writes on an unlinked inode: reset through the public API instead." Line 9 still does `static inline void luoHostLogReset(const char* path) { std::remove(path); KpLog_SetFallbackFile(path); }`, so it calls the re | Locate KpLog.h (it is not in this packet directory; find it through the include in LevelUpOverride.cpp) and read KpLog_SetFallbackFile(). Check whether it fcloses the previous handle and fopens `path` fresh on every call, including when `path` equals the currently configured path. If it does not reo |

### C-e89f63b4 (1 finding, scripts/04_merge_modules.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cf5b237d5853 | medium | unverified | medium | 218-221 | logic | worker |  | other-014 | WORKTREE | `stage_enter = stage_erf.get("k_303nar_enter", ResourceType.NCS)` / `live_erf.set_data("k_303nar_enter", ResourceType.NCS, stage_enter)` replaces the live script with no check of its existing bytes or of whether it is a community fix. Line 230 then reports: `print(f"  Saved merged {live_path.name} s | Compare the live k_303nar_enter.ncs bytes with the bytes of the Korriban/Community Build fix for this script (if one exists in the build). Confirm whether the live copy differs from the stage copy before line 220 runs. Check what stage_erf.get returns when the resource is absent. |

### C-e8ff6321 (1 finding, scripts/router_eval.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8854f830489f | medium | unverified | high | 510-512 | logic | worker |  | other-031 | WORKTREE | RowWriter.write opens each (engine, pass) file in append mode: `self.handles[key] = open(self.run_dir / f"{key[0]}_pass{key[1]}.jsonl", "a", ...)`. cmd_run takes the run id straight from --run-id (977) with no check for an existing run directory. summary.json is rewritten with "w" (624). | In a scratch copy, run `run --engines rules --run-id dup` twice, then `wc -l` on runs/dup/rules_pass1.jsonl (expect 2x the case count) and `score --run dup`. Check whether any caller reuses run ids. |

### C-ecf656ff (1 finding, scripts/query_ghidra.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 82021aebeae4 | medium | unverified | medium | 164-165 | bug | worker |  | other-030 | b1ccd94, f6231a3 | `raw_name = item.get("name", f"field_0x{off:x}")` (164) returns None when the struct_fields row has a NULL name, because the key exists. Line 165 then calls `re.sub(r"[^a-zA-Z0-9_]", "_", raw_name)`, which raises TypeError on None. The default on line 164 only covers a missing key. | Check the struct_fields CREATE TABLE for NOT NULL on name, and run `SELECT COUNT(*) FROM struct_fields WHERE name IS NULL` on the bundled DB. If any rows match, run `struct <Class> --cpp` on one of them. Consider coercing None to '' before sanitizing, as f1 requires. |

### C-edde6419 (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a43f0ce9739b | medium | unverified | high | 160-162 | other | worker |  | other-033 | 31aa40c, d331219 | Comment says '# Strict PII redaction', but the code applies only two regexes: an email pattern (line 161) and r"password[:\s=]+[^\s]+" (line 162). Nothing else is removed before the prompt is sent to the endpoints (lines 255, 268) or written by --export (lines 216-221). | Build a transcript record whose USER_REQUEST contains 'OPENROUTER_API_KEY=sk-test-123' and a bearer-style token, run extract_transcript_cases, then run --export to a temp path and grep the output. Decide whether the comment should be corrected or the redaction widened, and whether the export path sh |

### C-f0d3a8b1 (1 finding, scripts/upscale_legacy_movies.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 13ec39e1442c | medium | unverified | low | 185 | logic | worker |  | other-039 | WORKTREE | Line 185 hardcodes `"-framerate", "29.97"` for the upscaled PNG sequence, and line 215 hardcodes `"/f29.97"` for binkc. The source frame rate is never probed; lines 152-166 only extract frames and count them. | Use ffprobe on each original .bik to read its frame rate and duration. Compare them with 29.97 and with the audio.wav duration produced at line 159. Check the duration of out_bik against the source as well. |

### C-f6e2d6f4 (2 findings, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| abc55e10f4d1 | medium | unverified | medium | 786-798 | logic | worker |  | other-022 | WORKTREE | In resolve_template, the 'refuse to reuse our own deploy' guard depends on `deployed = (manifest or {}).get(gui_name, {}).get("deployed_sha256")` and runs only `if deployed and sha256(data) == deployed` (789-794). When the manifest file or its entry is missing, deployed is None, the guard is skipped | Create a deploy, then delete originals/ and manifest.json, and run a dry run followed by --deploy. Compare the GFF extents in the new output against the chitin template. Check whether any other record (sources.json, manifest_tex.json) holds the deployed sha that the guard could use. |
| 5297c3e594eb | low | unverified | medium | 786-798 | logic | worker | abc55e10f4d1 | other-023 | WORKTREE | resolve_template uses `deployed = (manifest or {}).get(gui_name, {}).get("deployed_sha256")` and refuses only `if deployed and sha256(data) == deployed` (789-794). load_manifest returns {} when manifest.json is absent (1368-1372), so the guard is skipped. The live file, which may be this tool's own  | Deploy one GUI, then delete manifest.json and originals/<gui>.gui, and run again. Check whether originals/ now holds the deployed bytes and whether --rollback restores them. |

### C-f7b44b78 (1 finding, scripts/verify_2da.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2da6a61b901b | medium | unverified | low | 59 | logic | worker |  | other-040 | WORKTREE | Line 59 `if twoda.get_row(i).get_string(first_col) != "":` counts a row as blank only when the value is exactly the empty string. The docstring (lines 7-8) says pykotor returns "" for every cell of a mis-parsed table. If read_2da returns None (or raises) for empty cells instead, prefix is 0 for ever | Load a 2DA with an empty first-column cell via read_2da and print repr(get_row(i).get_string(first_col)). Confirm it is '' and not None. Then run the scanner on a known-blanked table, such as the corrupted appearance.2da mentioned in the docstring, if one is available, and check that it is flagged. |

### C-f9925429 (1 finding, scripts/test_plan_manager.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 67d4b6474922 | medium | unverified | medium | 135-145 | test | worker |  | other-038 | 5b856ce, d331219 | test_mark_done_custom_state calls mark-done twice (lines 135 and 140) with only --plan. It passes neither --handoff-dir nor --no-handoff. plan_manager.py:792 runs the handoff step unless no_handoff is set. plan_manager.py:802 falls back to HANDOFFS_DIR when handoff_dir is empty. plan_manager.py:788- | Snapshot .claude/handoffs and .claude/session_handoff*.md, run this test, and diff them. Read the handoff_dir-None branch of cmd_handoff to confirm where it writes. Confirm that HANDOFFS_DIR (plan_manager.py:802) lies under REPO_ROOT. Compare with test_handoff_rotation, which passes --handoff-dir to |

### C-fc4d08ce (1 finding, patch_manager_mods/level_up_override_src/test.toml)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6ef957c705fd | medium | unverified | low | 3-7 | test | worker |  | other-007 | 97c1bd8 | test.toml registers only [common_contract_test] (line 3) and [r10_probe_test] (line 7). The packet directory also contains r10_filter_test.cpp and r10_page_test.cpp (Glob of level_up_override_src), and neither has a table here. The [common_contract_test] table (lines 3-4) also has no covers key, whi | Find how the test runner discovers tests: by glob over *_test.cpp, or only by test.toml tables. Run it on this directory and check whether r10_filter_test and r10_page_test appear in the executed list. Check whether the runner uses the covers key for contract accounting and whether a contract test w |

### C-fda90e3d (1 finding, scripts/02_merge_2da_and_tlk.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 536923d9352a | medium | unverified | medium | 232-233 | gate-integrity | worker |  | other-013 | WORKTREE | Lines 232-233 `for name, table in live.items(): write_2da(table, LIVE_OVERRIDE / name)` write every live table with no check on its height or contents. The docstring (lines 16-17) says: 'Stock 2.3.12 reads some shipped 2DAs as blank tables, and this script would write them back.' Nothing in main() c | Before writing, read each live table's height in a scratch run and confirm it matches the known pre-merge height (for example appearance.2da at 678 rows). Check that a blank read aborts the run. Compare the live height before and after a run against the expected append count. |

### C-012f30d9 (1 finding, scripts/test_subagent_tools.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| baa86b511c8b | low | unverified | low | 37-49 | test | worker |  | other-039 | 054ee19 | Line 38 `plan_path = sap.resolve_phase_and_plan("QH3")` and lines 45 and 47 assert on exact planning-document text: `self.assertIn("Queue HUD", title)` and `any("build_check.py" in c for c in cmds)`. | Read resolve_phase_and_plan and extract_phase_info in scripts/subagent_prompt.py to see which file they read, then confirm that file is tracked. Consider a small fixture plan file under the test's control in place of the live QH3 entry. |

### C-01744816 (1 finding, scripts/compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7e6e690122c8 | low | rejected | medium | 1092-1112 | gate-integrity | worker |  | other-019 | 3ec1365 | Line 1092 passes test_suite_ids via --ids-file, and lines 1109-1112 count only the results the runner emitted (`for r in runner_json.get("results", []): all_results.append(r)`). Nothing checks that every requested id produced a result. In scripts/run_tests.py:870-871, `if target_ids: items = [i for  | After a run, diff the ids in test_suite_ids against the ids in runner_json['results']. Then pass a made-up id through --ids-file to run_tests.py and confirm it exits 0 with no result for that id. Consider counting any requested id with no result as ERROR. |

### C-04178891 (2 findings, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3fd49ba324a4 | low | unverified | high | 145-149 | other | worker |  | other-025 | WORKTREE | The comment at line 145 reads `store_p: bars LBL_BAR1/2/3 at L61 R739, TOP 104 -> row (104 + 7) * 1024 / 600 = 191.` The arithmetic gives 111 * 1024 / 600 = 189.4, not 191 (191 is 112 * 1024 / 600). The code uses edge_row = 191 for pnl_store_pc (line 149), and that row feeds the E_left measurement b | Compute 111*1024/600 and 112*1024/600, then run inner_edge on pnl_store_pc at both rows (the `frame --detect` path) to see whether E_left changes. Correct either the comment or the row, whichever the measurement supports. |
| d80ae35012ff | low | unverified | high | 146-147 | other | worker | 3fd49ba324a4 | other-024 | WORKTREE | Comment: "store_p: ... TOP 104 -> row (104 + 7) * 1024 / 600 = 191." The arithmetic gives 111 * 1024 / 600 = 189.44, not 191. The value 191 matches virtual row 112, which is the row the comment at line 135 names ("virtual y=112"). The formula in the comment does not produce the number it claims. | Confirm which virtual row is intended (112 is consistent with line 135) and correct the offset in the comment so that the formula evaluates to the stored edge_row for pnl_store_pc (line 150). |

### C-044e6d45 (1 finding, neocities_mods.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bd7c4a96396c | low | unverified | low | 1419 | logic | worker |  | other-002 | WORKTREE | Line 1419 is `"method": "TSLPatcher"` and line 1469 is `"method": "TSLPatcher"`. Lines 1379, 1399 and 1449 use `"TSLPatcher Mod"`, and line 1409 uses `"HoloPatcher Mod"`. The same split appears earlier: `TSLPatcher` at 1089 and 1199 versus `TSLPatcher Mod` at 1209, and `HoloPatcher` at 1099 versus ` | Find the generator or renderer that reads the `method` key. That code is outside this packet, so it needs a separate read. Check whether it matches exact strings or by prefix. If exact, count the distinct method values across the file to see how many rows the bare and `... Mod` forms split. |

### C-0690457c (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 430549c50713 | low | unverified-scanner | low | 989 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| !rd(SiteQueue + 0x33, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x14, &multispell) \|\| | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-08980a12 (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| caa47d7cf51b | low | unverified | high | 24-26 | style-risk | worker |  | other-009 | 7020d54, 8e68204 | The CHECK macro now expands __VA_ARGS__ twice: `int _n = std::snprintf(nullptr, 0, __VA_ARGS__);` (line 24) and `std::snprintf(_detail.data(), _detail.size(), __VA_ARGS__);` (line 26). Format arguments with side effects therefore run twice. Line 460 passes `fakeGuid(P(liveTbl), &found)` as a format  | Grep every CHECK in gate_test.cpp and the other host tests for format arguments with side effects (function calls that mutate globals or output parameters). Confirm each is safe to evaluate twice, and decide whether the macro should evaluate the arguments once. |

### C-0d40c497 (1 finding, scripts/01_sanitize_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d656b2a7d534 | low | unverified | low | 54-58 | logic | worker |  | other-013 | WORKTREE | The Step 1 comment (line 6) says 'delete duplicate .tpc so matching .tga/lightmap stays'. Step 2 Case 1 then computes `c1 = live_tpc & staging_tga` and removes `staging_lower_map[base + ".tga"]` (lines 56-58). A base that Step 1 kept as its .tga, and that live holds as .tpc, is removed from staging  | On a copy, compute the intersection of the internal-duplicate set from Step 1 with c1 in Step 2. If it is non-empty, confirm with the author whether those bases should be dropped from staging, and update the comment or the logic to match. |

### C-0ef7f6a2 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bce719aa62a8 | low | unverified-scanner | low | 990 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| !rd(SiteQueue + 0x34, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x18, &metamagic) \|\| | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-1045f760 (1 finding, neocities_mods_detailed.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f438f87b07cf | low | unverified | high | 1308 | logic | worker |  | other-003 | WORKTREE | Line 1308: "category": "Graphical Improvement / 2 - Recommended", while the other graphics entries use "Graphics Improvement" (e.g. line 148 "Graphics Improvement / 2 - Recommended"). | If a consumer filters on the category string, confirm whether entry 131 disappears from the Graphics group. Correcting the string is a one-word edit if the consumer is exact-match. |

### C-112ff92a (1 finding, neocities_mods_detailed.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 16d1a1cab2a3 | low | unverified | medium | 1146 | other | worker |  | other-004 | WORKTREE | Line 1146: "url": "http://deadlystream.com/files/file/431-nihilusvisas-scene-vo-tweak/". A grep for http:// finds 34 url values in the file, for example line 56 and lines 1156, 1206, 1226, 1286, 1326, 1336, 1346 and 1446. Most other url values use https://. | Request each http:// URL and check whether the host redirects to or serves the same path over https. List the hosts that do not support https. Decide whether the catalog should use https links only. |

### C-14366c9b (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cf3c83ec322d | low | unverified | high | 1836-1851 | gate-integrity | worker |  | other-024 | WORKTREE | deploy_one records `"original_sha256": orig_sha` (line 1792), and rollback_txi checks the sidecar against its recorded sha (line 1810). But in rollback's override branch, the restore reads `orig = state.originals / entry["original_file"]` (1837) and writes `orig.read_bytes()` (1847 and 1850) with no | Compare `sha256(orig.read_bytes())` with entry["original_sha256"] before each restore in rollback(), then check whether any test or fixture covers a corrupted originals_tex file. Also confirm that the originals are written once and never rewritten, so the recorded hash stays valid. |

### C-14813b65 (1 finding, scripts/verify_all_compatch.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 30c7a05a7b5a | low | unverified | low | 48-49 | gate-integrity | worker |  | other-040 | WORKTREE | The exact-equality expectation for '303nar.mod' WaypointList is `100`, with the comment '100 since the 2026-09-24 capsule rebuild restored a mod waypoint edit (was 99)'. The gate now encodes the count that the rebuild produced. Nothing in the packet shows that 100 is the intended result and not a ne | Open 303nar.mod's GIT WaypointList in the live Modules directory and compare it with the mod source or the rebuild log for 2026-09-24. Confirm the 100th waypoint is the restored edit. |

### C-19507248 (1 finding, scripts/kotor2_toolset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| db26620a0470 | low | unverified | high | 26-28 | build | worker |  | other-028 | WORKTREE | `DEFAULT_GAME_DIR = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"` and `DEFAULT_WORKSPACE = Path.home() / "Games/KOTOR2_Mod_Workspace"`. Both defaults are hard-coded to one user's Steam layout and home layout. | Check the defaults against the documented install layout. Confirm that verify, dedup-textures and backup fail with a clear message when the default game dir is missing, and whether a --game-dir override is documented. |

### C-1a7a1284 (1 finding, scripts/master_ledger.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cf44e64cb01e | low | unverified | medium | 56-59 | logic | worker |  | other-028 | 93385ec, d331219 | `for alias, canonical in KNOWN_MOD_ALIASES.items(): if alias in low and not low.endswith("_src"): return canonical`. This is a substring test, so any name without the _src suffix that contains 'queue' maps to queue_hud_src (for example 'dialogue_queue'). 'companion' and 'diversity' behave the same w | Test canonical_mod_name('dialogue_queue') and canonical_mod_name('npc_companion_dialog') in a scratch run. Consider exact-match aliases or a word-boundary match. |

### C-1c2dde03 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2f6a91a7b652 | low | unverified-scanner | low | 15 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_7C9F3B has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-228f30d6 (1 finding, patch_manager_mods/_shared_src/SelfHealUi.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2a0581dfca11 | low | unverified | high | 21-22 | other | worker |  | code-001 | 4519b96 | Line 21: `inline bool isLeaderVit(Rect vit) { return vit.w >= 70; }`. Line 22: `inline bool isLeaderVitForButton(Rect vit) { return vit.w >= 70; }`. The bodies and the threshold are identical. A grep of _shared_src finds no caller of isLeaderVitForButton, only its definition. | Search the consuming mod directories (outside this packet) for isLeaderVitForButton. If there are no callers, the function is dead code. If there are callers, check whether they meant isLeaderVit and whether both names should share one definition. Check that the 70 px threshold still matches the 384 |

### C-22e3bd28 (1 finding, scripts/gpu_mode.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| dc2508b8dcbc | low | unverified | medium | 45-54 | bug | worker |  | other-026 | 93385ec, d331219 | `out.split(",")` runs over the whole nvidia-smi stdout, then `int(parts[1])` is applied. With one line per GPU, parts[1] is like '16384\n512' and int() raises ValueError. The `except ValueError: pass` at lines 52-53 falls through to `return 0, 0` at line 54. | Feed get_gpu_memory_mb a two-line nvidia-smi output such as '1000, 16384' followed by '200, 8192' (mock run_cmd) and confirm it returns (0, 0). Then check whether the same host runs gpu-mode status. |

### C-24b9a167 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 47eb7c479f8b | low | unverified-scanner | low | 16 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_75FCB8 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-26762fd0 (1 finding, scripts/01_sanitize_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 54d5cdade6ba | low | unverified | medium | 86-88 | gate-integrity | worker |  | other-013 | WORKTREE | The only verification is `assert len(remaining_internal) == 0, ...` and `assert len(remaining_cross) == 0, ...` (lines 86-87), followed by an unconditional success print (line 88). The docstring (lines 5-7) gives expected counts of 59 internal and 35 cross collisions, but no code checks the number o | Run the script with python -O on a copy and confirm the final line still prints. On a copy of the live tree, compare the printed removal counts with 59 and 35 and check whether any unexpected base name is removed. |

### C-26abffba (1 finding, scripts/compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| c682bc88ec89 | low | unverified | medium | 1053-1056 | logic | worker |  | other-036 | b348ca3 | `if b_proc.returncode != 0 and not any(r["kind"] == "build" and r["status"] == "FAIL" and r["id"].startswith("build_check:") for r in all_results):` The generic ERROR is added only when no per-mod FAIL exists. A nonzero exit after one FAIL line (crash, kill, timeout mid-run) records nothing about mo | Feed run_build_mods canned output with one FAIL line, rc=1, and fewer result lines than the number of build_mods. Check whether the unreported mod appears anywhere in results or counts. |

### C-26e120d6 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 15ffaec0a61a | low | unverified-scanner | low | 809 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] reinterpret_cast \| reinterpret_cast<PerformUseItemFn>(AddrPerformUseItem)( | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-2797b391 (1 finding, scripts/crash_triage.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7e35742b6c0d | low | unverified | high | 68-69 | logic | worker |  | other-020 | WORKTREE | cmd_proc never reads args.game_dir. The option is declared at line 217 with default DEFAULT_GAME_DIR (lines 24-26), and the function uses only get_game_pids() and get_proc_stats(). | Grep crash_triage.py for game_dir and confirm the only reference is the argparse declaration at line 217. If the option stays, check whether the help text or docstring should say it is unused. |

### C-281c4e26 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| abc1aa0719c8 | low | unverified-scanner | low | 12 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_757808 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-288e3178 (1 finding, scripts/clef_matcher.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ace85fb62b76 | low | unverified | medium | 82-84 | logic | worker |  | other-018 | bbb038f, d331219 | clef_matcher.py:82-84 derives base by stripping _src and underscores, then tests base.lower() in prompt_lower as a bare substring. A short base such as 'ui' matches 'build', 'guide' or 'suite', and the first sorted match wins (line 85). The self-test prompt 'Run build_check.py on all mods' (line 448 | List patch_manager_mods/*_src in the snapshot and find any base name (after removing _src and _) shorter than about five characters. For each, run `python3 -I scripts/clef_matcher.py route 'Run build_check.py on all mods' --no-log --json` and check whether active_mod is set. |

### C-28c8097f (1 finding, scripts/snapshot_live.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| c41f978ac61a | low | unverified | high | 87 | bug | worker |  | other-033 | WORKTREE | digest, rel = line.split("  ", 1). A line without the double-space separator raises ValueError. verify_snapshot calls read_sums (line 209) on the --verify path (lines 230-231), which has no try, so the result is a traceback instead of a FAILED line. | Add a blank line or a single-space line to a snapshot's SHA256SUMS and run --verify on that snapshot. Check the exit path and output. |

### C-299fa1d3 (1 finding, scripts/abi_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d639f13c7327 | low | unverified | medium | 96-101 | logic | worker |  | other-015 | 1fad5c9 | `if override_header:` / `target_name = header_name or "PortraitSlots.h"` / `shutil.copy2(override_header, hdr_dir / target_name)`, while main() passes each golden's own header name (line 373-374). With no --golden, one `--header` file is copied in under every golden's header name. | Create two goldens whose `header` keys differ, run `python3 scripts/abi_check.py --header <file>` without --golden, and inspect the inc_override directory and the compile errors. Decide whether --header is meant to be single-interface, in which case main() should reject it when more than one golden  |

### C-2a0d1dc6 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 342c4a0fc3e3 | low | unverified-scanner | low | 768 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] reinterpret_cast \| reinterpret_cast<void*>(buf.activeAction.creature), | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-2ac1b5d6 (1 finding, scripts/06_deploy_assets.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1f5155fe02a0 | low | unverified | high | 363-368 | logic | worker |  | other-014 | WORKTREE | main() calls `deploy_modules()`, `deploy_stream_music()`, `deploy_movies()`, `deploy_stream_voice()`, `deploy_override()` and then `deploy_galaxy_map()`. The precondition check at lines 324-325, `sys.exit(f"Missing {name} in {STAGING_DIR / 'Override'}: run phase 3 first.")`, and the copy-of guard at | Run the deploy against a scratch game dir with a staging Override/ lacking a_galaxymap.ncs. Check whether the earlier deploy steps have already written files before the sys.exit at line 325 fires. |

### C-2b0db333 (1 finding, patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7dc2b81c673e | low | unverified-scanner | low | 229 | other | scanner |  |  | 25d4e42, 7df1b4a | [scanner sentinel-range] 0xFFFFFFF0u \| KT_CHECK_EQ(characterKeyFor(0xFFFFFFF0u, 2, &bad, true).c_str(), "objfffffffd"); KT_CHECK(!bad); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-2be13dc0 (1 finding, scripts/clef_matcher.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| c7b083d12712 | low | unverified | high | 279 | logic | worker |  | other-018 | bbb038f, d331219 | clef_matcher.py:279 falls back to prior.get("avg_duration_ms", 3200) while clef_matcher.py:367 uses 3500. clef_dataset.py:67 sets the missing-ledger global prior to 3200, and clef_dataset.py:222 sets the populated-ledger default to 3500.0. | Call route_task with query_clef_engine patched to return None, and again with a valid answer, using a ledger that has no entry for the active mod. Compare predicted_duration_ms. Confirm which default is intended and whether the two constants are meant to differ. |

### C-2df74a45 (1 finding, neocities_mods.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7d2ae7cdb9bb | low | unverified | medium | 1509 | style-risk | worker |  | other-001 | WORKTREE | The method field is free text with many variants: "Loose-File" (line 69), "Loose-File (with optional automation)" (line 1479), "Loose-File Components, possible .bat patcher" (line 1509), ".bat Patcher / .sh script" (line 1459), "Multi-Run TSLPatcher" (line 59), "HoloPatcher" vs "HoloPatcher Mod", an | Check how the renderer or any consumer uses the method string (exact match, switch statement, or a lookup table). List the distinct method values in this file and see which ones would fall through. Confirm with the mod's page whether the 'possible .bat patcher' entry is correct. |

### C-2faec89f (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 677d4b0bcf1b | low | unverified-scanner | low | 758 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] reinterpret_cast \| reinterpret_cast<void*>(buf.activeAction.creature), | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-314252ab (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ac0101fca213 | low | unverified-scanner | low | 987 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| !rd(SiteQueue + 0x31, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0xC, &arg2) \|\| | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-31befe58 (1 finding, patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3d181c82a6e4 | low | unverified | high | 1022 | logic | worker |  | code-003 | 4519b96, eff4a32 | New line 1022: `KP_LOGV(chHud, n == NWidgets ? KPLOG_INFO : KPLOG_WARN, "BUILD n=%d/%d why=%s", n, (int)NWidgets, whys.c_str());`. The removed line (old 910) passed `whys.empty() ? "ok" : whys.c_str()`. | Grep the log consumers and docs for 'BUILD n=' and 'why=ok'. Restore the empty-string fallback that the old call had, or confirm the change is intended. |

### C-33949e49 (1 finding, patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a81d7421c8b1 | low | unverified-scanner | low | 227 | other | scanner |  |  | 25d4e42, 7df1b4a | [scanner sentinel-range] 0xFFFFFFF0 \| // AH3: full Odyssey negative player sentinel range 0xFFFFFFF0..0xFFFFFFFE maps to canonical Exile key | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-353054ab (1 finding, scripts/query_ghidra.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8197dd1ab9a4 | low | unverified | high | 240-243 | logic | worker |  | other-030 | b1ccd94, f6231a3 | `if len(results) >= args.limit: sys.stderr.write("Warning: struct output may be truncated ...")` (241-242) is followed by `print(format_cpp_struct(results, args.class_name))` (243) and then `sys.exit(0)` at line 260. The closing brace is emitted for a possibly partial field list. The only truncation | Run `struct <BigClass> --cpp` with the default limit and check whether the output is cut short. Check whether the struct_fields row count for common classes exceeds 25. Consider whether a truncated --cpp declaration should be refused or marked in stdout. |

### C-3531ac08 (1 finding, patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| faf0a30b3961 | low | unverified-scanner | low | 231 | other | scanner |  |  | 25d4e42, 7df1b4a | [scanner sentinel-range] 0xFFFFFFFAu \| KT_CHECK_EQ(characterKeyFor(0xFFFFFFFAu, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(!bad); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-397c4975 (1 finding, patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d7d61950be4f | low | unverified-scanner | low | 783 | other | scanner |  |  | 4519b96, eff4a32 | [scanner pointer-math] uintptr_t-cast \| for (int j = 0; j < 4; ++j) okCd = rd<int>((uintptr_t)slot, Slot_CardRect + 4 * j, &cd[j]) && okCd; | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-3ad8ee7f (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| faaa8b044425 | low | unverified | low | 162 | logic | worker |  | code-006 | 97c1bd8, dc4bb61 | Line 162: 'const bool g_fallbackSet = (KpLog_SetFallbackFile(LogName), true);' is a namespace-scope dynamic initializer, so it runs at DLL load, before KPatchInit (lines 97-98) and before ensureHeader() calls rotateIfLarge() (lines 1217-1219). rotateIfLarge() (lines 1162-1167) calls MoveFileA(LogNam | Read KpLog_SetFallbackFile and check (a) whether it opens the file or only stores the name, and (b) whether KpLog's registry or channel state is a namespace-scope object in another .cpp. If the setter opens the file eagerly, check whether rotation still succeeds after line 1219 (it does not if the h |

### C-3c717095 (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d3e07ebfe326 | low | unverified | medium | 966-971 | bug | worker |  | other-024 | WORKTREE | tpc_roundtrip_ok wraps tpc_to_image in `except Exception:` and returns False (lines 968-970). Per the module docstring (lines 54-56), a False result makes the tool write a .tga instead of a .tpc and move the .tpc to originals_tex. Any exception while decoding the freshly built TPC is therefore turne | Check the caller of tpc_roundtrip_ok (outside the excerpt I read) to see whether the fallback is logged or counted anywhere. Then narrow the except to the expected decode errors (ValueError, struct.error, RuntimeError), as the other call sites do (e.g. lines 1120, 1335). |

### C-3d9b7b21 (1 finding, neocities_mods.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b36bff049eac | low | unverified | high | 1308 | style-risk | worker |  | other-001 | WORKTREE | Line 1308 reads "category": "Graphical Improvement / 2 - Recommended", while every other entry (e.g. line 118, 148, 1478) uses "Graphics Improvement". Line 478 has the same kind of drift: "Appearance Change / 4 - Option" where the other rank-4 entries (e.g. line 288, 608) say "4 - Optional". | Find the code that consumes category values (filter list, sort or group-by) and check whether it matches on exact strings. Compare the set of distinct category and rank labels in this file against that list. Also confirm whether these two entries are the only off-spec labels. |

### C-3e2fc8de (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5858beec824c | low | unverified-scanner | low | 7 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_5FE2A3 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-404a3390 (1 finding, scripts/master_ledger.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 92dbfd1dd047 | low | unverified | high | 412-417 | logic | worker |  | other-028 | 93385ec, d331219 | Line 397 prints `--- Recent Mod Actions ---`, but line 412 is `for mod, e in sorted(mod_latest.items())[:10]:`. The sort is by mod name, so the 10 shown are the alphabetically first mods, not the most recent. Line 417 has the same pattern for plan milestones with [:5]. | Sort by the latest event timestamp (e.get('ts')) before slicing, or drop the 'Recent' label. Check with a ledger that has more than 10 distinct mods. |

### C-426dfdeb (1 finding, scripts/agent_usage.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 35d0b086887e | low | unverified | high | 80-82 | bug | worker |  | other-016 | 0d67415, b334383 | `rates = m[tier]` (line 80), then `if set(rates) != set(RATE_KEYS) or not all(valid_rate(v) for v in rates.values()):` (line 81). If a tier value is a JSON array whose five elements are exactly the key names, the set comparison passes and `.values()` raises AttributeError. The except tuple at line 8 | In a copy of the tree, replace one tier in agent_usage_rates.json with the list of the five key names and call load_rates() on that copy. Confirm the AttributeError is not caught by the except at line 88. Run Python on the copy with -I. |

### C-4452a8da (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4f5945b793d3 | low | unverified | high | 181-182 | bug | worker |  | other-033 | 31aa40c, d331219 | The per-file try (line 149) wraps the whole line loop. json.loads(line) on line 152 and rec.get on line 154 have no per-line guard. The bare 'except Exception: pass' at lines 181-182 then abandons the remainder of that transcript file without any message. | Create a transcript.jsonl with a blank line between two valid USER_INPUT records and call extract_transcript_cases. Check whether the second record is returned and whether any warning is printed. |

### C-4471bda8 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| eb32c29ac0fb | low | unverified-scanner | low | 14 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_75218B has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-44840e4e (1 finding, scripts/install_echoes_compatch.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4b76e57c68ee | low | unverified | low | 24-25 | logic | worker |  | other-027 | WORKTREE | The docstring says a second full run 'stops there with the game unchanged by it'. But main() runs 01_sanitize_textures.py before 02_merge_2da_and_tlk.py (lines 283-286), so the claim holds only if phase 1 writes nothing on a second run. The phase-2 refusal itself does exist (scripts/02_merge_2da_and | Copy a fully installed game to a scratch directory, run phase 1 twice, and diff override/ and Modules/ between the two runs. Or read 01_sanitize_textures.py for unguarded writes that an already-applied check would normally cover. |

### C-4552e7b9 (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| eb6e723b3ab3 | low | unverified | high | 138 | build | worker |  | other-033 | 31aa40c, d331219 | glob.glob(os.path.expanduser("~/.gemini/antigravity-cli/brain/*/.system_generated/logs/transcript.jsonl")). The default --source is 'transcripts' (line 200), so a run with no arguments reads personal session transcripts from a hard-coded home path. | Confirm that a run with no arguments reads the transcript directory. Consider an environment override like the KEV_* variables, and check that the README or script usage mentions the source. |

### C-484b3d2a (1 finding, scripts/02_merge_2da_and_tlk.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 13bc62d71b4d | low | unverified | low | 194 | logic | worker |  | other-013 | WORKTREE | Line 60 hard-codes `PLANETARY_M478_ROW = 4`. Line 194 runs `echoes_plan.set_cell(PLANETARY_M478_ROW, "label", "M4_78_Disabled")`, then lines 195-197 blank the model, icon and guitag of that row, with no check that row 4 is M4-78. Line 50 likewise assumes ECHOES_SPELL_ROW = 282 is ekunholo with no la | Print the label of row 4 in the staging planetary.2da and the label of row 282 in the staging spells.2da. Confirm they are M4-78 and ekunholo. |

### C-4915220a (1 finding, scripts/03_compile_galaxy_map.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5253e4623f4f | low | unverified | high | 147 | gate-integrity | worker |  | other-013 | WORKTREE | Line 147 prints `Phase 3 (Core Scripting & PartySwap Preservation) 100% COMPLETE and VERIFIED!`. main() (lines 109-147) contains no read-back, byte check, or call to check_ncs_arg_order.py. The output .ncs files are written at lines 122 and 133 and the byte counts are only printed. | Read main() and confirm there is no check after the two write_bytes calls. On the builtin path (KOTOR2_ALLOW_BUILTIN_NSS=1), run check_ncs_arg_order.py on the output and see whether it is ever invoked by the script. |

### C-4aadaa0b (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 705c58709245 | low | unverified-scanner | low | 986 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| if (!rd(SiteQueue + 0x30, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 8, &spellId) \|\| | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-4c4d809f (1 finding, scripts/test_agent_usage.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 01fac65d1b46 | low | unverified | high | 491-492 | test | worker |  | other-035 | 0d67415, b334383 | Lines 491-492: @unittest.skipIf(hasattr(os, 'geteuid') and os.geteuid() == 0, 'root reads mode-0 files') on test_unreadable_transcript_skipped. Under root, the only test of the unreadable-transcript branch in agent_usage.py:365-372 is reported as skipped, not run. | Run the suite as root and check the summary for the skip. To keep the branch covered under root, simulate the failure in a subtest by patching parse_transcript (or Path.is_file) to raise PermissionError, then drop the root skip. |

### C-4d4026dd (1 finding, scripts/router_eval.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9e9c0abd7619 | low | unverified | medium | 894-897 | gate-integrity | worker |  | other-031 | WORKTREE | The label gate uses only the rubric basis: `a, n = stats["rubric"]["_all"]`, `passed = n > 0 and _ge(a / n, thr)`, and the verdict line is labelled 'label_agreement (rubric basis)' (894-897). Outcome-basis cases are tallied (871-889) and printed, but they never enter pass/fail. | Read the label_agreement_min definition in .claude/router_eval/gates.json (absent from this snapshot). Count outcome-basis cases in cases.jsonl. Run compare-labels with a label file that disagrees only on an outcome-basis case and check the exit code. |

### C-4d85fd76 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 14ada5e3037c | low | unverified-scanner | low | 1545 | other | scanner |  |  | 97c1bd8, dc4bb61 | [scanner pointer-math] reinterpret_cast \| KP_LOG(chPlay, "GUID load g%08x found=%d/32 result=%d table=%08x", (unsigned)g_guid, found, result, (unsigned)reinterpret_cast<uintptr_t>(table)); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-4e6692ef (1 finding, patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ecd0f3671877 | low | unverified-scanner | low | 230 | other | scanner |  |  | 25d4e42, 7df1b4a | [scanner sentinel-range] 0xFFFFFFF9u \| KT_CHECK_EQ(characterKeyFor(0xFFFFFFF9u, 2, &bad, false).c_str(), "objfffffffd"); KT_CHECK(!bad);   // fast-travel sentinel | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-4eae7b27 (1 finding, scripts/router_eval.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d0e6c9899fc4 | low | unverified | medium | 486-487 | logic | worker |  | other-031 | WORKTREE | plan_run estimates Haiku spend as `est = 0.001 + 0.0003 * (h - 1) if h > 0 else 0.0` with no derivation in the file. The pre-run refusal (971-973) compares only this number with --budget-usd. The per-call cap passed to the CLI is 0.01 (365). | Compare the estimate printed by `run --dry-run` with haiku_spent_usd from a completed run's summary.json. Look in the design notes for where 0.0003 came from, and check whether this refusal has ever fired. |

### C-4f6256b8 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8d99303b0ad6 | low | unverified-scanner | low | 800 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] reinterpret_cast \| reinterpret_cast<void*>(nextA.creature), | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-4fa6941e (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ba6f5ccc5931 | low | unverified | high | 2308-2314 | logic | worker |  | other-026 | WORKTREE | `if not model:` branch prints the survey and returns `0` without reading args.deploy or args.wave. The deploy branches elsewhere report an outcome, for example `log.append("nothing deployed: fix the refused upscales first")` at lines 2386-2387. | Run `world plc --deploy` with and without `--upscaled` against a scratch state directory. Compare the exit codes and the contents of override/ and manifest_world.json. Consider whether an error, like the --only validation at lines 2305-2306, matches the other error paths. |

### C-512854a6 (1 finding, neocities_mods.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fc08e6bf3298 | low | unverified | low | 47 | style-risk | worker |  | other-001 | WORKTREE | Display strings hold HTML entities, e.g. line 47 "author": "zbyl2, DarthStoney, Hassat Hunter &amp; VarsityPuppet", and line 1475 "name": "Character Textures &amp; Model Fixes". The same pattern appears in category values such as line 1368. | Find where the name, author and category fields are rendered. If they go through textContent or an escaping helper, the entity text will show literally. Check the rendered page for a visible '&amp;'. |

### C-51faf484 (1 finding, tests/static/subclassers.toml)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 30598fe13fb5 | low | unverified | high | 49 | other | worker |  | other-043 | 2db77c4, e947d12 | The new GuiInputProvider.cpp entry sets `restore_hint = 198`, but the restore call is at patch_manager_mods/gui_input_src/GuiInputProvider.cpp:201: `Hook::uninstall(&wheelProc);`. The install hint (line 48, `120`) matches GuiInputProvider.cpp:120 `bool ok = Hook::install(h, wheelProc);`. | Grep GuiInputProvider.cpp for `Hook::uninstall\(` to confirm line 201, then run scripts/check_subclassers.py and confirm the only effect is an INFO drift line for this entry. |

### C-535d95d5 (1 finding, scripts/04_merge_modules.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ed3cd607eebc | low | unverified | low | 168-173 | logic | worker |  | other-014 | WORKTREE | `existing_wp_tags = {w.get_string("Tag") for w in wps}` is built once (line 168). In the loop, `if tag and tag not in existing_wp_tags:` / `wps.append(copy.deepcopy(w))` (171-172) never adds the appended tag to existing_wp_tags. The sibling 701kor block (lines 298-303) has the same pattern. | Check the staged 302nar.git WaypointList for duplicate Tag values that are absent from the live list. If there are any, count how many times each appears in the output WaypointList after the merge. |

### C-54023e47 (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fd1b3313dd01 | low | unverified | medium | 96 | logic | worker |  | other-033 | 31aa40c, d331219 | "baseline_engine": rec.get("engine", "clef-flash"). A ledger record without an engine field is labelled clef-flash. Because that label does not end with '(synthetic)' or '(real)', real_baseline (line 228) becomes True, and the header (line 233) and the speedup line (line 342) present it as measured  | Count CLEF_ROUTE records in the ledger with and without an engine key. If the count without one is nonzero, check whether those records came from clef-flash. |

### C-54e86318 (2 findings, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 244d5ba5ee57 | low | unverified | high | 1636-1641 | gate-integrity | worker | a6e9c8ad5cac | other-023 | WORKTREE | `if "M" in edges and abs(float(edges["M"]) - stretch_factor) > 1e-4: raise ValueError(...)` (1637). When an entry has no "M", the resolution check is skipped entirely and the entry is accepted. | Pass a frame_edges.json whose entry has E_left but no M, with --width/--height that differ from the producer's, and confirm the run proceeds. Check whether generate_ultrawide_textures.py always writes M, which would make this latent. |
| a6e9c8ad5cac | low | unverified | high | 1637 | gate-integrity | worker |  | other-022 | WORKTREE | load_frame_edges checks the target resolution only `if "M" in edges and abs(float(edges["M"]) - stretch_factor) > 1e-4:` (1637). An entry without an M field passes the resolution check for any --width/--height. | Check the frame writer in generate_ultrawide_textures.py (not in this packet) for whether M is always emitted. If it is not, pass an M-less frame_edges.json with a different --width/--height and confirm the run accepts it. |

### C-5640c300 (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 702da963f192 | low | unverified | high | 1846-1857 | bug | worker |  | other-025 | WORKTREE | rollback: `target.unlink()` (1846) is followed by `(target.parent / entry["removed_file"]).write_bytes(orig.read_bytes())` (1847). The manifest is saved only once, after the loop (1857). If the write fails, the deployed file is gone and the original is not back. On the next run, the branch at 1827-1 | Simulate a write failure at line 1847 on a scratch manifest with an override_tpc entry (removed_file set). Run rollback twice and check whether the .tpc is restored on the second pass. |

### C-569ba6c5 (1 finding, scripts/check_ncs_arg_order.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 50b5bd2dc0b1 | low | unverified | medium | 56-69 | gate-integrity | worker |  | other-017 | WORKTREE | `TABLE_ERRORS = {"PlayPazaak", "SetAvailableNPCId"}` (56) and `if f.name in TABLE_ERRORS or not f.params: return None` (68-69) exempt every call to these two functions. The comment (52-53) only justifies the exemption as avoiding false flags, but the same code also hides a real source-order call to  | Check the pykotor definitions of PlayPazaak and SetAvailableNPCId against nwscript.nss (outside this packet). Build a fixture with source-order calls to each, with the correct arity and types, and see whether a corrected parameter list (or an explicit override of the table entry) flags it while keep |

### C-573b9b0c (1 finding, scripts/test_master_ledger.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6971460a2aeb | low | unverified | high | 53-74 | test | worker |  | other-037 | d331219 | test_metrics_and_parent_agent sets os.environ["PARENT_AGENT_ID"] = "gemini-flash" (line 54). Its finally block runs del os.environ["PARENT_AGENT_ID"] (line 73) without saving the previous value. tearDown (lines 22-27) restores only AGENT_ID. If the caller's environment already defines PARENT_AGENT_I | Export PARENT_AGENT_ID=sentinel, run the module, and print os.environ.get("PARENT_AGENT_ID") after the run to confirm whether the value is lost. Confirm that no other test in the module reads parent_agent. |

### C-578065d7 (1 finding, scripts/test_compat_check_rc.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 74d03bd8b5ae | low | unverified | high | 81-88 | test | worker |  | other-036 | b348ca3 | `@unittest.expectedFailure` wraps the whole body, including `_, data = self._run_main(1, out)` and the `data["results"]` comprehension. Any exception (missing JSON file, KeyError, a changed helper signature) also counts as the expected failure. | Run the test with -v and check the exception type. Run the body outside the decorator in a scratch copy outside the repo to confirm the failure is the AssertionError at line 88, not a harness error. |

### C-597f8756 (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fe7b54c334a0 | low | unverified | low | 1173-1181 | logic | worker |  | other-023 | WORKTREE | apply_popup iterates only `gff.root.get_list("CONTROLS")` and scales each control's extent paths (1173-1181) with no nested-CONTROLS check. apply_content rejects the same case explicitly: `if control.exists("CONTROLS") and len(control.get_list("CONTROLS")): raise ValueError(...nested CONTROLS)` (110 | Dump the control tree of datapad_p.gui (originals/ or chitin) and check for any control whose CONTROLS list is non-empty. If one exists, confirm the popup report lacks lines for it and add the same guard that apply_content uses. |

### C-5d5dd47c (1 finding, patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| aa45591786fe | low | unverified | low | 57 | other | worker |  | code-003 | 4519b96, eff4a32 | `constexpr int Slot_CardRect = 0x54C;                      // portrait card rect (screen px)` has no source reference, while the neighbouring line 58 cites `K2Addr Slot_VitRect`. The offset is read raw every frame at 783: `rd<int>((uintptr_t)slot, Slot_CardRect + 4 * j, &cd[j])`. | Confirm 0x54C against K2Addr.h or the slot struct dump, and check that the 16 bytes at 0x54C-0x55B do not overlap another known field. Add the source reference to the comment, as Slot_VitRect has. |

### C-5ee3b407 (1 finding, scripts/test_clef_matcher.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fe72957cf878 | low | unverified | high | 10 | test | worker |  | other-035 | 5b856ce, d331219 | Module docstring line 10 claims '6. Telemetry logging to master_ledger.jsonl' is verified. Every route_task call in the file passes log_telemetry=False (lines 96, 113, 135, 155), and no test patches log_event or checks for a CLEF_ROUTE record. clef_matcher.py:397 gates log_event on log_telemetry, so | Patch clef_matcher.log_event and call route_task with log_telemetry=True in one test, then assert the call includes event CLEF_ROUTE. Alternatively, remove docstring item 6 if telemetry is meant to stay untested. |

### C-5faf9948 (1 finding, scripts/test_agent_usage.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 89e9f6ea961f | low | unverified | medium | 483-484 | build | worker |  | other-035 | 0d67415, b334383 | Lines 483-484 call os.symlink with no guard. The file guards its other OS-specific cases: geteuid (line 491), mkfifo (line 500), and linux-only bytes filenames (line 531). os.symlink needs Developer Mode or SeCreateSymbolicLinkPrivilege on Windows, so this test would error there instead of skipping. | Check whether this repo's test tooling runs on Windows. If it does, wrap the symlink creation with a skip on OSError or a skipUnless on a symlink capability probe. If not, document the Linux-only requirement in the module docstring. |

### C-601f5484 (1 finding, research/companion_levelup/tools/parity/unit_gap_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8e5c36190ff1 | low | unverified | high | 1 | style-risk | worker |  | other-011 | 7020d54 | Line 1 says this file covers 'the log cap' edge cases, but line 48 says the F1 log cap moved to KpLog.h and is no longer tested here. The header and the body disagree about what this file pins. | Check whether the header comment should drop 'the log cap' once the KpLog replacement is confirmed (see f1). Verification is a read-through of the header against the remaining checks in main(). |

### C-620955a7 (1 finding, scripts/compat_graph.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 581b63bdf1a1 | low | unverified | high | 359 | logic | worker |  | other-020 | 3ec1365 | 'if any(part in source_file.parts for part in ("research", "tests"))' tests every component of the absolute path. source_file comes from dpath / fn under repo_root (lines 287 and 410), so a checkout under any directory named research or tests enables the stub search for every quoted include. Line 29 | Call _resolve_include with source_file set to <repo_root>/x/y.cpp for a quoted include that exists only under research/companion_levelup/stub. Do this once with a repo root that has a research or tests component in its absolute path and once without. Compare the results. Also compare source_file.par |

### C-6416b992 (1 finding, scripts/clef_matcher.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| aeb2dc8156d6 | low | unverified | high | 237 | logic | worker |  | other-018 | bbb038f, d331219 | clef_matcher.py:237 'any(k in p_lower for k in ["build check", "gate", "compile check"])' runs before the test and review branches. Plain substring matching makes 'gate' match 'investigate', 'navigate' and 'aggregate', and line 245 'test' matches 'latest' and 'contest'. | Call deterministic_fallback_routing('Investigate the latest crash', None, {}) directly (it reads only the cooldown file) and check that role is 'gate'. Repeat with 'Write the latest changelog' to see whether it lands in 'test' rather than 'doc'. |

### C-642930f2 (1 finding, patch_manager_mods/action_hotbar_src/ActionHotbar.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d65f505ec5a3 | low | unverified | low | 2050-2058 | logic | worker |  | code-002 | 25d4e42, 7df1b4a | `} else if (++g_pendingSaveRetries > 60) { g_pendingSaveSnapshot = false; ... KP_LOGV(chCfg, KPLOG_WARN, "persave: %s: folder '%s' not found, no snapshot", ...); }`. The counter increments once per HUD tick, and the tick returns early at line 2030 when the HUD is unreadable, so the retry window is 6 | Measure how long the save folder takes to appear after K2SE_SAVED in a real save, at the frame rates seen in play. Check the logs for the 'folder not found, no snapshot' WARN. Consider whether the retry limit should be time-based. |

### C-6443cb05 (1 finding, scripts/agent_diff_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e3ef9d297d8e | low | unverified | medium | 43-58 | logic | worker |  | other-016 | 054ee19, 1fad5c9 | get_git_status() parses `git status --porcelain -uall` without -z (line 44) and strips only spaces (line 55). Git quotes paths with unusual bytes (quotes, backslashes, control characters, and non-ASCII under the default core.quotePath). A quoted name keeps its leading quote, so the mod regex at line | In a throwaway repo, create a file whose name has a non-ASCII character or a double quote under patch_manager_mods/<x>_src/, run agent_diff_check.py without --baseline, and check checked_mods in the --json output. |

### C-65798d65 (1 finding, scripts/kotor2_toolset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 28871048b2a0 | low | unverified | high | 136-139 | logic | worker |  | other-028 | WORKTREE | Docstring line 13 says 'Create an immutable timestamped backup'. Code uses second-resolution names: `timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")` and `with tarfile.open(tar_path, "w") as tar:`. A second backup in the same second truncates the first archive without warning. | Run `backup --backup-dir <scratch>` twice within one second and count the .tar files in the directory. Consider opening with exclusive creation or adding a suffix, and correct the docstring either way. |

### C-659179e1 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 455c5011d7d0 | low | unverified-scanner | low | 6 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_5FD7CE has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-660a8e89 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4e96ecfc4cd1 | low | unverified-scanner | low | 996 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| rd(SiteQueue + 0x38, posPtr, 4, &posVal[1]); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-66d9873c (1 finding, scripts/verify_2da.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ed2a405756e2 | low | unverified | medium | 69-78 | logic | worker |  | other-040 | WORKTREE | Only `read_2da(path)` is inside the try (lines 69-72). `blank_prefix_length(twoda)` (line 78) and its `get_row`/`get_string` loop (lines 58-61) run outside it. `main` (lines 112-115) does not catch exceptions either. An exception from those calls escapes the loop, so later files are never scanned an | Give check_file a 2DA whose read succeeds but whose row or column access raises (or mock get_row to raise), with a second valid file after it in the directory. Confirm the second file is never reported. |

### C-685255a8 (1 finding, scripts/snapshot_live.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 262053b7ebbf | low | unverified | high | 76-77 | logic | worker |  | other-033 | WORKTREE | if rel in META_FILES or p.is_symlink(): continue. rsync -a copies symlinks as symlinks, and they are then left out of SHA256SUMS. The docstring (line 14) says 'sha256 of every file above'. | Put a symlink in a test override/ directory, take a snapshot, and check whether it appears in SHA256SUMS and whether --verify reports anything. |

### C-6a08072e (1 finding, scripts/run_tests.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 53f0abe2a602 | low | unverified | medium | 796-804 | logic | worker |  | other-032 | 3ec1365, d331219, fadc3c9 | Line 796 declares `--json` with nargs="?", and line 804 adds a positional `filters` with nargs="*" placed after it. Under argparse, `--json` takes the next non-option token as its value, and a single `*` positional is consumed at its first chunk, so later positionals error. | In an isolated check, call `ap.parse_args(['--json','diversity'])` and `ap.parse_args(['a','--jobs','4','b'])` on the same parser, or inspect the parse result. Decide whether --json should take an explicit `=` form, or whether filters should use nargs='+' and be placed before the options. |

### C-6a5a4d7d (1 finding, scripts/symlink_pass.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f8a137832a26 | low | unverified | low | 32-35 | logic | worker |  | other-034 | WORKTREE | The comment at lines 32-34 says cloudsaves is pruned because 'game and Steam Cloud enumerate subdirectories directly'. But lines 42-51 create a lowercase alias next to every other mixed-case directory at every depth, so the same enumeration duplicate can appear for those directories (for example 'Mo | Check whether the game or ReShade enumerates the install root or Modules/ and would see both 'Modules' and 'modules'. Compare that with the reasoning behind the cloudsaves prune. |

### C-6b1fce19 (1 finding, scripts/compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ef7ef2c66b27 | low | unverified | high | 1107-1130 | logic | worker |  | other-019 | 3ec1365 | Lines 1109-1112 append each runner result and increment `counts[st]` one at a time. If an exception is raised partway through the loop, the `except Exception:` at 1129 then runs `counts["ERROR"] = counts.get("ERROR", 0) + len(test_suite_ids)` at 1130. The results already counted stay counted, and a  | Give the runner a JSON results list that has a valid dict entry followed by a non-dict entry. Compare the printed RESULT line and the --json `counts` with the `results` array. Consider counting only the results that were not yet processed when the exception occurred. |

### C-6be7502a (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9277a463b7fc | low | unverified | medium | 1118-1126 | bug | worker |  | other-025 | WORKTREE | scan_gui_textures wraps only resolve_texture in `try` / `except (ValueError, FileNotFoundError, RuntimeError)` (1118-1122). The next statement `img, txi = decode(src)` (1126) is outside any handler. A corrupt override TPC or TGA raises struct.error or an OSError subclass (e.g. PIL's UnidentifiedImag | Place a truncated .tga (a few header bytes) in a scratch override dir that holds a GUI FILL texture, run `scan` and see whether it aborts or records a reason for that texture. |

### C-6c2ea0d0 (1 finding, scripts/04_merge_modules.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4ec0f45fd568 | low | unverified | high | 59-64 | logic | worker |  | other-014 | WORKTREE | `if corteldc and "corteldc" not in existing_creatures:` ... `else:` `print(f"  Creature 'corteldc' already present in Creature List")`. When the stage file has no corteldc, corteldc is None, so the else branch runs and reports 'already present' although nothing was found in either file. The merge th | Run merge_module_204tel() against a staging 204tel.mod whose GIT has no 'corteldc' template and confirm the printed message. Then check that the else branch cannot distinguish 'absent from stage' from 'present in live'. |

### C-6eacdb46 (1 finding, scripts/upscale_legacy_movies.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d41decb367a7 | low | unverified | high | 30-44 | build | worker |  | other-039 | WORKTREE | Every tool and data path is an absolute home-directory path, for example line 30 `FFMPEG_BIN = Path("/home/ethansm/.local/bin/ffmpeg")`, line 35 `PROTON_BIN = Path("/home/ethansm/.local/share/Steam/steamapps/common/Proton - Experimental/proton")`, and lines 41-44 SOURCE_DIR, LIVE_DIR, BACKUP_DIR and | Run `grep -n '/home/ethansm' scripts/upscale_legacy_movies.py` to list every occurrence, then check that each tool path exists on the machine that will run it. Decide whether these values should come from environment variables or a config file. |

### C-6f52ab3b (1 finding, scripts/_common.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9cc808633d13 | low | unverified | low | 62 | other | worker |  | other-015 | WORKTREE | The error text reads `Keep one spelling (see ../case_dedup.py) and re-run.` The only case_dedup.py in this tree is scripts/case_dedup.py (Glob), which sits in _common.py's own directory, so `../` from there resolves to the repo root. | Work out which directory `../case_dedup.py` is meant to be relative to (the game or staging directory named in the error, or scripts/). If it is not valid from any of them, the message should name the real location. |

### C-7053a784 (1 finding, scripts/check_txi_cycles.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8ce0dd63a669 | low | unverified | medium | 78 | logic | worker |  | other-017 | WORKTREE | `files = {p.name.lower(): p for p in override.iterdir() if p.is_file()}` merges case-variant spellings (e.g. Foo.tpc and foo.tpc) into one key, and the later one in iteration order silently wins. Lines 89-91 then read `files[f"{stem}.tpc"]`, so the TPC inspected may not be the spelling the engine lo | Create a scratch override/ with Foo.tpc (embedded cycle TXI) and foo.tpc (non-cycle) plus a foo.txi cycle TXI. Run the script a few times, and check which texture it inspects and whether that depends on directory listing order. |

### C-71e798c5 (3 findings, neocities_mods_detailed.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5339266d46c2 | low | unverified | high | 1460-1480 | other | worker |  | other-004 | WORKTREE | Line 1460: "a list of all the files you will need to manually delete can be found here ." Line 1480: "navigate to this page and click on the button", "go to here and repeat the same step", "following the instructions here ." Lines 1500 and 1510 also read "go to this video" and "this video" with no l | Find the source text for these entries and check for stripped markdown link syntax. Restore the URLs or confirm that the link text is intentionally plain. Run a search for dangling phrases such as "here", "this page", "this video" and "HERE" across the file to find any other affected entries. |
| c91ce25c5fcb | low | unverified | high | 1480 | other | worker | 5339266d46c2 | other-003 | WORKTREE | Line 1480 (entry 148 notes): "navigate to this page", "then go to here and repeat the same step", and "following the instructions here ." None of these has a target. The same pattern appears at line 10 ("publicly available HERE --"), line 1460 ("can be found here .") and line 1500 ("go to this video | Compare the notes with the source mod pages to recover the URLs. Check whether the generator that writes this JSON drops link markup. This is a data-quality check, not a code path. |
| afa93f268f11 | low | unverified | medium | 1480 | other | worker | 5339266d46c2 | other-004 | WORKTREE | Line 1480 says "Place cleaner.bat in the folder ... placing cleanlist_k2.txt in the same location. You can now run cleanlist.bat ... delete cleaner.bat and cleanlist_k1_sf.txt, and then move the remaining files". The list file is first named cleanlist_k2.txt, then cleanlist_k1_sf.txt appears only in | Check the upstream instructions for the exact script and list file names. Confirm which list file the linked download provides and whether cleanlist_k1_sf.txt is ever introduced as a file the reader places. Check that the names in this entry match the actual download. |

### C-744d68fc (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| c6a33c70ef5b | low | unverified | medium | 1436-1442 | logic | worker |  | other-023 | WORKTREE | rollback takes `target = Path(entry.get("target_path") or ...)` (1437). deploy_file stores the absolute path with `"target_path": str(target_path)` (1416). If that path no longer exists, rollback logs `is gone; dropping manifest entry` and deletes the record (1439-1442) without looking for the file  | Deploy a GUI, then change game_dir to a copy of the folder (or edit target_path in manifest.json) and run --rollback. Check whether the entry is dropped and the modified file is left in place. |

### C-75ba4320 (1 finding, scripts/agent_diff_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4f112cf937b3 | low | unverified | high | 237-238 | logic | worker |  | other-016 | 054ee19, 1fad5c9 | `except Exception as err:` followed by `print(f"  Warning: could not read ledger events: {err}")` (line 238) writes to stdout, not stderr. It runs after the JSON verdict has been printed to stdout (line 204). Under --json, if the ledger read fails and --subagent was not given, the stream gets non-JS | Make master_ledger.read_events() raise in a scratch copy, run `agent_diff_check.py --json --no-patch`, and feed stdout to a JSON parser to confirm the trailing text. |

### C-7a38faa2 (1 finding, scripts/clef_dataset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ca9f7ab1da0f | low | unverified | high | 137-142 | logic | worker |  | other-018 | bbb038f, d331219 | clef_dataset.py:137 increments total_events before the demo/test filter at lines 141-142 executes continue, so the summary line at 324 ('events processed') counts pollution that the per-mod and per-role tables exclude. | Count rows in a scratch ledger copy whose mod is demo or demo_src, or whose backup contains demo_swap_, and compare with total_events from `python3 -I scripts/clef_dataset.py json --ledger <scratch>`. |

### C-7ac854dc (1 finding, patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cc4d4e5cccf0 | low | unverified-scanner | low | 233 | other | scanner |  |  | 25d4e42, 7df1b4a | [scanner sentinel-range] 0xFFFFFFFF, 0xFFFFFFFFu \| KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 2, &bad, false).c_str(), ""); KT_CHECK(bad);   // 0xFFFFFFFF stays invalid (outside sentinel range) | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-7afb6d1f (1 finding, patch_manager_mods/action_hotbar_src/HotbarLogic.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d5faf5996a5f | low | unverified-scanner | low | 74 | other | scanner |  |  | 25d4e42 | [scanner sentinel-range] 0xFFFFFFF0 \| //    or sentinel IDs 0xFFFFFFF0..0xFFFFFFFE (the Odyssey negative player sentinel range, | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-7c6b0890 (1 finding, scripts/install_echoes_compatch.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5616d32ddea5 | low | unverified | high | 263-265 | logic | worker |  | other-027 | WORKTREE | `if args.archive and args.archive.exists():` (line 263) has no else branch. A --archive path that does not exist is silently ignored, and execution falls through to the staging check at line 267, which only fails if staging_dir is missing. | Run the installer with --archive pointing at a nonexistent file while the staging dir is populated, and check the output for any line about the archive. Compare with the run where --archive is omitted. Decide whether a given-but-missing archive should be a hard error. |

### C-7eb71801 (1 finding, scripts/check_ncs_arg_order.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cc494b519c55 | low | unverified | medium | 72-75 | logic | worker |  | other-017 | WORKTREE | Line 72 maps every vector parameter to "float" in `types`, and 75 returns `set(types[1:])` as the later-parameter set. For a function with first parameter float and a later vector, a source-order call ends with the vector's z component (a CONSTF) right before the ACTION. classify() at 89-90 then see | Find a TSL function in pykotor's TSL_FUNCTIONS (outside this packet) whose parameter list has float first and vector later. Walk classify() by hand for a source-order push sequence, or build a fixture, to confirm that it returns 'normal'. Decide whether the vector-as-float mapping should be kept onl |

### C-80cde936 (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b32f7ff94f06 | low | unverified | medium | 2323-2324 | bug | worker |  | other-026 | WORKTREE | `src = resolve_texture(name, state.override_dir, erfs, ws.originals, ws.sources, ws.manifest())` is followed directly by `ws.remember(name, src)` and `decode(src)`, with no try and no None check. The equivalent calls in upscale_batch (2108-2117) and cmd_set (2205-2212) catch (ValueError, FileNotFoun | Remove or rename one eligible texture's recorded original under originals_tex (or make a recorded ERF source vanish), then run `world <prefix> --upscaled <model>`. Expect a traceback from line 2323 rather than a REFUSED or SKIP line. Check whether world_textures (line 1319) can yield a name that res |

### C-821a32aa (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 647c25cb27c2 | low | unverified-scanner | low | 77 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] c-pointer-cast \| using PerformCastSpellFn = void(__thiscall*)(void* creature, uint32_t spellId, int arg2, uint32_t targetId, int multispell, int metamagic, int domain, const float* pos); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-826a3698 (1 finding, scripts/scaffold_mod.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8ed23f6bd5d3 | low | unverified | high | 57-61 | bug | worker |  | other-032 | fadc3c9 | Line 39-41 now validates clean_name with re.fullmatch, but line 57-58 only normalises doc_id when it is all digits. Line 60 builds `plan_filename = f"{doc_id}_{clean_name}.md"` from any other doc_id and line 61 joins it onto patch_manager_mods/. Line 240 adds plan_path to files_to_write, and line 25 | Call scaffold_mod with doc_id='../x' and dry_run=False in a throwaway copy of the repo, and check where plan_path lands and whether the ValueError from relative_to at 255 fires after the write. Consider whether doc_id should get the same fullmatch check as the mod name. |

### C-831d4e41 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0b3b372bcdc1 | low | unverified | high | 104 | other | worker |  | code-006 | 97c1bd8, dc4bb61 | Line 1 of the header comment: 'Level-Up Override 1.0.0 (Steam Aspyr build 6A522E71...)'. The history comment at lines 6-14 ends with '1.0.0 cleanup'. This diff changes the code constant to 'constexpr const char* Version = "1.6.0-dev";' (line 104; old value 1.5.0-dev at o99). No line in the history m | Grep the packet directory (README.md, PULL_REQUEST.md, the doc links) for '1.5.0', '1.6.0' and '1.0.0' to see which version is documented. Decide whether the header comment and history should be updated in the same change as the Version bump. |

### C-83abae6f (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a48f7b980a1f | low | unverified-scanner | low | 767 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] reinterpret_cast \| reinterpret_cast<PerformUseItemFn>(AddrPerformUseItem)( | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-8c8b5f49 (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 028010e9763e | low | unverified | high | 39-50 | style-risk | worker |  | other-033 | 31aa40c, d331219 | BACKEND_CRITERIA (line 39) and MODEL_CRITERIA (line 45) are defined, but a search of the file shows they are never referenced. The payload (lines 258-264) sends only ROLE_CRITERIA. margin is computed at line 193 and discarded into margin_4b and margin_08b at lines 287-288. | Grep shadow_replay.py for BACKEND_CRITERIA, MODEL_CRITERIA and margin_. If the benchmark is meant to cover backend or model, check the payload. If not, the tables and margin should be removed or marked as unused. |

### C-8de1952b (1 finding, scripts/clef_matcher.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 214bbe548b82 | low | unverified | high | 88-93 | logic | worker |  | other-018 | bbb038f, d331219 | clef_matcher.py:88-89 'if "queue" in prompt_lower: return "queue_hud_src"' (and the similar branches at 90-99) return alias names without the existing_mods check that the explicit-argument path applies at lines 71-78. | In a scratch copy without patch_manager_mods/queue_hud_src, run `python3 -I scripts/clef_matcher.py route 'queue fix' --no-log --json` and compare active_mod and write_scope with the explicit-mod path using --mod queue_hud_src. |

### C-8de7129c (1 finding, scripts/router_eval.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a8af347e949e | low | unverified | high | 328-331 | bug | worker |  | other-031 | WORKTREE | kev_call wraps the Kev invocation in `try: resp = kev_fn(...) except Exception: resp = None` (328-331). The row built afterwards (333-335) has no error field, so an ImportError from default_kev_fn (320) or a bad payload looks the same as a daemon returning None. Three failures in a row abort the eng | With the Kev daemon down, run `run --engines kev --passes 1 --limit 3` in a scratch copy and read a row in kev_pass1.jsonl. Then run `python3 -I -c` with scripts/ on the path to import clef_matcher and see whether the import itself raises. Check whether the error text could be kept in the row withou |

### C-91379b6c (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d1c5eb7431d1 | low | unverified | low | 1121-1122 | logic | worker |  | other-023 | WORKTREE | In the picture branch, `new_l, new_w = plan["EXTENT"]` (1122) applies only EXTENT. plan_content can also return "PROTOITEM.EXTENT" and "SCROLLBAR.EXTENT" for role "content" (1062-1067), and apply_partial_content (1219-1220) applies only EXTENT too. | Dump the controls named in CONTENT_PICTURE (LBL_SCREENSHOT, LBL_PCNAME, LBL_PLANETNAME) and CONTENT_EXCLUDE_PARTIAL (map_p LBL_BAR3/4/5) for PROTOITEM or SCROLLBAR children. If any exist, confirm they are unchanged in the output. |

### C-91c23627 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7a5047082974 | low | unverified-scanner | low | 10 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_756DD4 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-93e2dbda (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 753a7e1c64cb | low | unverified-scanner | low | 995 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| rd(SiteQueue + 0x37, posPtr, 0, &posVal[0]); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-93e5a8b2 (1 finding, patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 13362f21f5aa | low | unverified-scanner | low | 228 | other | scanner |  |  | 25d4e42, 7df1b4a | [scanner sentinel-range] 0xFFFFFFF0u \| KT_CHECK_EQ(characterKeyFor(0xFFFFFFF0u, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(!bad); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-9782474f (1 finding, scripts/subagent_prompt.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d98a56d40c52 | low | unverified | high | 109-111 | logic | worker |  | other-033 | 054ee19, 31aa40c | if not plan_path or not plan_path.exists(): return phase_key, "", [], [], None. main() (lines 414-418) only checks the plan when --plan is given (line 415). Without --plan, a phase that is not in any plan prints a prompt with 'Process QH3 as specified in the master plan.' and no files or verificatio | Run python3 scripts/subagent_prompt.py ZZZ9 in a checkout with no active_plan.txt and check the exit code and output. Decide whether a missing plan should be an error like the explicit --plan case. |

### C-98ce79a9 (1 finding, scripts/gpu_mode.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| aea08206532a | low | unverified | high | 13 | bug | worker |  | other-026 | 93385ec, d331219 | The usage text at line 13 is `gpu-mode lms [model_identifier]`, but the parser at line 180 defines only `--model` as an option, and `mode` is the only positional. `gpu-mode lms qwen/qwen3.5-9b` therefore has an unrecognized extra argument. | Run `python3 scripts/gpu_mode.py lms qwen/qwen3.5-9b` and confirm argparse reports unrecognized arguments. Then run it with --model and compare the usage text at line 13 with the argparse help output. |

### C-99562843 (1 finding, neocities_mods_detailed.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6fcaba9672c3 | low | unverified | high | 1480 | logic | worker |  | other-003 | WORKTREE | Line 1480: "placing cleanlist_k2.txt in the same location. You can now run cleanlist.bat" and later "delete cleaner.bat and cleanlist_k1_sf.txt". The step names cleanlist_k2.txt as the file to place, then asks for deletion of cleanlist_k1_sf.txt, a different file the reader never placed. | Check the source page for the list file name that cleanlist.bat expects and make both mentions of the file in the note agree. |

### C-9a6357be (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fd1eca41057e | low | unverified | medium | 1642-1646 | logic | worker |  | other-023 | WORKTREE | `limit = FRAME_BAR_RULES[texture][0] if texture in FRAME_BAR_RULES else 400` (1643) tests the raw JSON key, but the rule table is keyed in lowercase and the stored key is only lowercased afterwards at 1646 `out[texture.lower()] = e_left`. A key such as "PNL_PAUSE_PC" fails the membership test and ge | Write a frame_edges.json with key "PNL_PAUSE_PC" and E_left=200 and run load_frame_edges; confirm it is accepted. Then check that generate_ultrawide_textures.py always writes lowercase keys; if it does, this is latent only. |

### C-9e42f7e0 (1 finding, scripts/_common.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b11214b2bac0 | low | unverified | high | 26-27 | build | worker |  | other-015 | WORKTREE | `GAME_DIR = _env_path("KOTOR2_ROOT", "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")` and `WORKSPACE = _env_path("KOTOR2_WORKSPACE", "~/Games/KOTOR2_Mod_Workspace")`. abi_check.py:50 repeats the same personal path literal: `Path.home() / "Games/KOTOR2_Mod_Workspace/tools/koto | With the KOTOR2_* variables unset and a HOME that lacks ~/Games/KOTOR2_Mod_Workspace, run one phase script and check that the error names the missing path. Check whether abi_check.py's default should come from _common.WORKSPACE instead of repeating the literal. |

### C-9e6ee6dc (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 48c1bd511211 | low | unverified | high | 99 | style-risk | worker |  | other-024 | WORKTREE | Line 99 is `logging.disable(logging.CRITICAL)`, which runs at import time and disables every logger in the process. The same module imports pykotor (lines 102-106) and the sibling generator (lines 112-122), so their warnings are dropped as well. | Check whether any call in the rest of the file (for example around the deploy and upscale paths) depends on a logger or on a dependency's warning. If none does, the risk is limited, and a narrower scope or removing the line would keep the tool's own messages working. |

### C-9ee6165b (1 finding, research/companion_levelup/tools/parity/gate_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5b6ebba582fe | low | unverified | medium | 110 | gate-integrity | worker |  | other-009 | 7020d54, 8e68204 | gate_test.cpp:110 `static void clearLog() { luoHostLogReset("levelup_override_log.txt"); }` replaces the old remove(). LuoLogHost.h:9 implements it as `std::remove(path); KpLog_SetFallbackFile(path);`. LuoLogHost.h:3-4 says: 'KpLog also keeps the fallback file open, so remove()ing it leaves later wr | Read KpLog_SetFallbackFile and KpLog_SetFallbackLevel in KpLog.h (not found by my probes; locate it from LevelUpOverride.cpp's includes). Confirm the function closes the previous FILE* and reopens by name even for an identical path. Run gate_test once and confirm the line-182 banner check passes on  |

### C-9fd7b7d2 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1b7607a12148 | low | unverified-scanner | low | 9 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_5FEE32 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-a12db993 (1 finding, scripts/snapshot_live.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1ce27347a7b1 | low | unverified | high | 106 | bug | worker |  | other-033 | WORKTREE | subprocess.run(cmd, check=True) raises CalledProcessError on rsync failure. main() catches only RuntimeError, ValueError, FileNotFoundError and FileExistsError (line 242), so this exits with an uncaught traceback and leaves a partial snapshot directory with no 'ERROR:' line. | Make rsync exit non-zero (for example an unreadable override file) and check the output and whether the snapshot directory remains. |

### C-a1a10d28 (1 finding, patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e03f2ebb58da | low | unverified | medium | 1173 | test | worker |  | code-009 | 32c8125, 4add886 | Line 1173 asserts that shouldReInject still re-injects with paused=true: `KT_CHECK(shouldReInject(base, false, true, /*paused=*/true, 999, false)); // paused/now are ignored by the header`. The header body has `(void)paused; (void)now;` (QueueHudLogic.h:576, unchanged context). The earlier paused ch | Check the git history of QueueHudLogic.h for when the paused and now parameters were disabled. Decide whether pause should suppress re-inject. Then either restore the guard and flip line 1173, or reword the paused tests so they state what they actually check. |

### C-a5541da9 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 046ec2ad5510 | low | unverified-scanner | low | 997 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| rd(SiteQueue + 0x39, posPtr, 8, &posVal[2]); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-a5a1c80a (1 finding, scripts/clef_matcher.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 80249e3ddefc | low | unverified | medium | 104-110 | logic | worker |  | other-018 | bbb038f, d331219 | clef_matcher.py:104-110 and 127-133 call subprocess.run(["git", ...]) with no cwd=, so they inspect whatever repository the process's current directory belongs to, not REPO_ROOT (line 25). | From /tmp, run `python3 -I <repo>/scripts/clef_matcher.py route 'review this diff' --no-log --json` in a repo with uncommitted changes and check whether has_diff and diff_lines are reported as zero. |

### C-a6147877 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 296577fe3ca3 | low | unverified-scanner | low | 991 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| !rd(SiteQueue + 0x35, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x1C, &domain)) { | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-a69eedc5 (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8b61188e9eaa | low | unverified | medium | 249 | logic | worker |  | other-033 | 31aa40c, d331219 | base_role = c["baseline_role"] or "code". If the ledger record has neither decision nor clef_output.role (line 97 yields None), the baseline is silently set to 'code' and then scored against the endpoints. | Find CLEF_ROUTE records in .claude/master_ledger.jsonl that lack both decision and clef_output.role, run with --source ledger, and check whether those rows report base role 'code'. |

### C-a7bb4f06 (1 finding, scripts/router_eval.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 665c36b12255 | low | unverified | high | 164-166 | bug | worker |  | other-031 | WORKTREE | `gold = c.get("gold")` (164) followed by `if gold not in cat.by_id:` (165). If the JSON value is a list or object, the dict membership test raises TypeError because it is unhashable, instead of appending the 'gold not in catalog' error. | In a scratch copy, put one line with "gold": [] into cases.jsonl and run `validate-cases --cases <copy>`. Confirm the TypeError and whether the other per-line errors are lost. |

### C-a862a3c3 (1 finding, scripts/router_eval.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 126c258b819e | low | unverified | high | 575-588 | logic | worker |  | other-031 | WORKTREE | The budget check (578-580) runs under the lock, then the call runs with the lock released (581-582), and spend is added only after it returns (584-588). HAIKU_WORKERS is 4 (41), so up to four calls can pass the check at the same time. Each call is capped at --max-budget-usd 0.01 (365), but the in-fl | Call run_eval with a fake runner that returns total_cost_usd 0.02 per call, budget 0.05 and workers=4 (run_eval accepts runner and workers). Count how many calls start after state spent reaches the budget and compare the final haiku_spent_usd with 0.05. |

### C-a9c98e5f (1 finding, scripts/agent_usage_rates.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0a65f960da4f | low | unverified | high | 9 | build | worker |  | other-016 | 0d67415, b334383 | The haiku entry's "source" string says the input/output/cache_write_5m/cache_read values are "mirrored from ~/.claude/skills/deep-haiku-review/bundle/engine/merge.py:33-45". That is a home-directory path to a file outside the repository, so the mirrored values cannot be checked from the tree and the | Confirm whether merge.py lines 33-45 in the cited skill bundle still contain the five values listed for the haiku entry. If the bundle moved, the citation is stale. |

### C-ae694197 (1 finding, scripts/symlink_pass.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a8071e33373d | low | unverified | high | 18-21 | build | worker |  | other-034 | WORKTREE | `os.environ.get("KOTOR2_ROOT", os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"))` makes a Steam-on-Linux home path the default. The override is documented in the comment at lines 16-17. | Check the callers of create_case_symlinks and main() for any reliance on the default path, and confirm they set KOTOR2_ROOT where the install is elsewhere. |

### C-b27c60a1 (1 finding, patch_manager_mods/mouse_probe_src/MouseFixLogic.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 08d0b4bef53e | low | unverified | high | 56-59 | logic | worker |  | code-008 | b2c3e46 | Line 56: `long long maxMargin = (w < h ? w : h) / 4;` is 0 whenever the smaller side is 1-3 px, and line 58 `if (m > maxMargin) m = maxMargin;` then forces m to 0. For w == 1, the warp centre `(winLeft + winRight) / 2` (line 62) equals winLeft, so line 59 `curX <= winLeft + m` is true for the centre | Call checkEdgeWarp with curX=0, curY=50, winLeft=0, winTop=0, winRight=1, winBottom=100, margin=50 and confirm it returns true with the warp point (0,50). Decide whether such a rect can reach this call (GetWindowRect on the foreground window during minimise or transitions). The existing assertions a |

### C-b2e09eec (1 finding, patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 49f44d1e3da5 | low | unverified | medium | 784 | logic | worker |  | code-003 | 4519b96, eff4a32 | `if (okCd && cd[2] > 0 && cd[3] > 0) g_card0.card = shui::Rect{ cd[0], cd[1], cd[2], cd[3] };` has no else branch. `card` is never reset: hideAll (554-561) and the per-frame seen reset (754) do not touch it. Line 824 uses `c0.card.w > 0 ? shui::leaderButtonRect(c0.card) : ...` whenever c0.seen is tr | Check whether a failed or zero card read should clear g_card0.card, so the vit-based fallback applies. Trace what happens to the leader portrait's rect during teardown or screen-size changes. |

### C-b3347c75 (1 finding, scripts/generate_diversity_2da.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 67ff7176fc39 | low | unverified | high | 534-536 | logic | worker |  | other-021 | 3ec1365 | `normalhead_val = row_vals.get("normalhead", "")` / `if normalhead_val in head_mapping:` / `row_vals["normalhead"] = str(head_mapping[normalhead_val])`. The lookup is an exact, case-sensitive dict match, while find_head_row at line 461 compares with `.lower()`. When no key matches, the raw name fall | Confirm by reading merge_appearance_table that nothing validates the post-remap normalhead value. Check whether a value that is neither numeric nor a key in head_mapping can reach set_cell. Consider whether the lookup should match the same way as find_head_row, and whether an unmatched non-numeric v |

### C-b36f22e5 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e1ceca0dff7f | low | unverified-scanner | low | 17 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoInstant_74F3E9 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-b378d8e7 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 03a6af4022a4 | low | unverified-scanner | low | 19 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoInstant_5FE281 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-b5708eb3 (1 finding, patch_manager_mods/mouse_probe_src/MouseProbe.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b4a144dedd40 | low | unverified | high | 104 | other | worker |  | code-008 | b2c3e46 | Line 104 says `// ---- mouse fix state (0.4.0) ----`, but the header on line 1 says "Mouse Probe 0.3.0" and the START banner on line 68 is `"START mouseprobe 0.3.0"`. | Check manifest.toml and test.toml in the packet directory for the version they declare. Decide whether the mouse-fix feature should be 0.3.0 or 0.4.0, then make the banner, header and section comment agree. |

### C-b7ebed75 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 79fcd96b879a | low | unverified-scanner | low | 757 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] reinterpret_cast \| reinterpret_cast<PerformCastSpellFn>(AddrPerformCastSpell)( | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-b8fca185 (1 finding, neocities_mods_detailed.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e0a042a8cbf7 | low | unverified | medium | 69 | logic | worker |  | other-003 | WORKTREE | The method field is free text with near-duplicate values: line 69 "method": "Loose-File" vs "Loose-File Mod" elsewhere (e.g. line 89); line 1479 "Loose-File (with optional automation)"; line 1509 "Loose-File Components, possible .bat patcher"; line 79 "TSLPatcher" vs line 1049 "TSLPatcher Mod"; line | Find the code that reads the method field (the site template or any script that parses this JSON) and check whether it matches exact strings. If it does, count entries per distinct method value and compare with the intended set. The file itself has no schema to check against. |

### C-b95ae62c (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d9e2ed400aa8 | low | unverified-scanner | low | 810 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] reinterpret_cast \| reinterpret_cast<void*>(nextA.creature), | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-b9d25518 (1 finding, tests/benchmarks/transcript_prompt_eval.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 861ca18ec27d | low | unverified | high | 22-23 | other | worker |  | other-042 | WORKTREE | Line 23 embeds a raw session prompt containing an operator handle: "...I see a lot of the entries are marked as me (ethansm). Example that doesn't look clear to me: [2026-10-08 18:52:38] SUBAGENT_DIFF_CHECK (ethansm) mods=npc_diversity_src verdict=PASS". The whole file (lines 2-301) stores raw user  | Confirm whether this file is meant to be committed to the repository or kept local (check .gitignore and the repo's policy for test fixtures). If it stays in the tree, decide whether prompts should be redacted or anonymised first. Check that the file is not picked up by any shared publish or export  |

### C-b9ddae06 (1 finding, neocities_mods.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2081531fb7ff | low | unverified | high | 10 | other | worker |  | other-001 | WORKTREE | Every one of the 154 entries has "instructions": "" (for example line 10 and the last entry at lines 1538-1540). The field is never filled in, so it looks like a stubbed placeholder left by the generator. | Grep the consumer for the 'instructions' key to see whether an empty string hides or breaks a section. Decide whether the field should be populated or removed from the schema. |

### C-ba768d77 (1 finding, scripts/snapshot_live.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 81ec56f20e05 | low | unverified | high | 44-46 | build | worker |  | other-033 | WORKTREE | DEFAULT_GAME_DIR uses os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II") and DEFAULT_WORKSPACE_DIR uses "~/Games/KOTOR2_Mod_Workspace", both as hard-coded home-relative defaults. Env vars override them. | Check whether KOTOR2_ROOT and KOTOR2_WORKSPACE are set in the usage docs. Decide whether the defaults should be required arguments instead. |

### C-baefdcd6 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| c5ace8ea19db | low | unverified | medium | 1939-1965 | logic | worker |  | code-007 | 97c1bd8, dc4bb61 | logLine (LevelUpOverride.cpp:183-190) formats into a 2048-byte buffer, runs markTruncated (167-170) on overflow, and logs on chUi. The converted sites now call KP_LOG directly on other channels, for example line 1939 KP_LOG(chGate, ...) in place of logLine, and similarly at 1943-1965, 2007-2083 and  | Locate KpLog.h (the KP_LOG, KP_LOGV, KP_CHANNEL and KPLOG_WARN definitions, outside this packet). Check its buffer size, whether it marks truncation, and whether channels are filtered by default. Then grep the test and tooling for the old ui channel tag or the INSTANT apply-now text to see whether a |

### C-bb0122e6 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 075651f29d4d | low | unverified-scanner | low | 799 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] reinterpret_cast \| reinterpret_cast<PerformCastSpellFn>(AddrPerformCastSpell)( | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-bcc8b25b (1 finding, scripts/compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 37b122f68ec2 | low | unverified | high | 715 | gate-integrity | worker |  | other-018 | 3ec1365 | compat_check.py:715 'return 0, warnings' is the only return in run_lint, so the exit code is always 0 although the docstring (line 535) promises one. The sole caller at line 835 discards it ('code, warnings = run_lint(...)'), and the only failure path is 'if args.strict and warnings: return 3' (line | grep scripts/ for '--lint' and 'run_lint(' to confirm there is no gate that relies on the exit status. Check scripts/test_compat_check.py:251 to see whether it asserts on the returned code. |

### C-bcd573d5 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6add2fa181c5 | low | unverified | high | 1441-1442 | other | worker |  | code-006 | 97c1bd8, dc4bb61 | Removed (old line 1422): '// Propose additive writeThrough parameter to filewatch::atomicWrite for Common owner adoption in Chunk C9.' Post-image line 1441 keeps only '// Note: retains MOVEFILE_WRITE_THROUGH ...', and lines 1442-1450 (ptWriteText) still hand-roll fopen/fwrite/fclose/MoveFileExA inst | Check whether filewatch::atomicWrite exists in the shared FileWatch header and whether it lacks a write-through option (grep for atomicWrite in the shared header, which I could not locate). If it does, restore the follow-up note or track it elsewhere, and confirm that ptWriteText's MOVEFILE_WRITE_TH |

### C-bebd7971 (1 finding, scripts/swarm_argv_probe.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9f18d98d3255 | low | unverified | medium | 27-28 | build | worker |  | other-034 | fdee555 | `BUNDLE = Path.home() / ".claude/skills/deep-haiku-review/bundle"` (line 27) and `FORK = REPO / ".claude/review"` (line 28). In this snapshot .claude/ is absent (Glob '.claude/**' returned nothing) and .gitignore excludes only `.claude/session_handoff.prev.md`, so main() at 164-167 prints 'missing'  | Confirm whether .claude/review/run_map.sh is tracked in the real repository (the snapshot has no git history). Run `python3 scripts/swarm_argv_probe.py` on a clean clone and check the 'missing' message. |

### C-bf8000fb (1 finding, patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 819e132f53e2 | low | unverified-scanner | low | 1078 | other | scanner |  |  | 32c8125, 4add886 | [scanner pointer-math] uintptr_t-cast \| KT_CHECK_EQ(sp.creature, (uintptr_t)0x2000); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-c1aa3257 (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4ffecc8b0058 | low | unverified | medium | 1759-1761 | logic | worker |  | other-025 | WORKTREE | `rm = find_ci(ovr, plan["remove"]) if plan["remove"] else None` followed by `if rm is not None and sha256(rm.read_bytes()) != orig_sha: raise`. If the file to be removed is already gone, rm is None and the deploy proceeds with no error. The manifest still records `removed_file` (1791), so rollback ( | Delete the override .tpc of an override_tpc source by hand, run loadscreens --deploy, then rollback. Check whether the .tpc is recreated and whether any warning was printed. |

### C-c4366fbd (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ad391c5fa7ea | low | unverified | medium | 986-997 | other | worker |  | code-009 | 2c071d8, d4ef936 | Site IDs are base + offset: SiteQueue = 0x7110 and SiteRules = 0x7140 (line 56). The new spell reads use SiteQueue + 0x30..0x39 (lines 986-997), which is 0x7140..0x7149, the same values as SiteRules + 0..9. SiteRules + 0..4 are used for the rules reads (lines 149-154). The existing item reads at Sit | Read saferead::readAt and probeRead (shared saferead header, not in this packet) for any use of the site value beyond logging. If it is only logging, record it as hygiene and ignore it. |

### C-c4f4cc9d (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9e1a9ef07b30 | low | unverified-scanner | low | 988 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| !rd(SiteQueue + 0x32, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x10, &targetId) \|\| | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-c5a1e53b (1 finding, neocities_mods.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f97c72a483fb | low | unverified | high | 1446 | other | worker |  | other-002 | WORKTREE | Line 1446 reads `"url": "http://deadlystream.com/files/file/735-realistic-visual-effects/"`. The neighbouring entries use https (e.g. line 1366 `https://deadlystream.com/files/file/2752-...`). A grep for `http://` in the file returns 34 lines (56, 86, 426, 1346, 1446, ...), so the list mixes schemes | Confirm that deadlystream.com and nexusmods.com serve these paths over https and redirect http to https. If they do, normalise the 34 http:// URLs in one pass. Check whether the site renderer already upgrades links, which would make this moot. |

### C-c631836d (1 finding, scripts/snapshot_live.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 9433c10db604 | low | unverified | low | 100-106 | logic | worker |  | other-033 | WORKTREE | cmd = ["rsync", "-a"] with no --checksum, so rsync uses size and mtime to decide that a file is unchanged. write_sums (lines 139-142) then reuses the previous digest for any file that shares an inode with the previous snapshot. | Rewrite a file under a test override/ with the same size and its mtime restored (touch -r), take a snapshot, and compare the snapshot copy and its SHA256SUMS line with the live file. Check whether the ultrawide tools ever preserve mtime. |

### C-c8deeff9 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 74af9fdbc513 | low | unverified-scanner | low | 20 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoInstant_66C93A has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-ca41c0a9 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 001d7d11fd68 | low | unverified | high | 2103 | other | worker |  | code-007 | 97c1bd8, dc4bb61 | LevelUpOverride.cpp:2103 r10Emit now calls KP_LOG(chR10, "%s", line) instead of logLine. R10Probe.h:3 still says lines leave through an EmitFn callback and that the mod routes them to its logLine. | Read R10Probe.h header comment and r10_probe_test.cpp (not read here) to confirm nothing else depends on the logLine route. Then confirm the comment is the only stale reference to logLine for R10 output. |

### C-cf39dbf3 (1 finding, scripts/clef_dataset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2a2e1de745e2 | low | unverified | medium | 237 | logic | worker |  | other-018 | bbb038f, d331219 | clef_dataset.py:189 increments runs for every matched event, but lines 190-193 count only PASS/OK and FAIL/ERROR as outcomes. Line 237 divides by runs: pass_rate = round((data["passes"] + 1.9) / (data["runs"] + 2.0), 2). Any run with a missing or other status (SKIP, TIMEOUT) lowers pass_rate as if i | In a scratch copy of the ledger, tally the distinct status and verdict values for OPENCODE_RUN, SUBAGENT_DIFF_CHECK and BUILD_GATE. If any value falls outside PASS/OK/FAIL/ERROR, compare pass_rate with passes/(passes+fails) for that mod. |

### C-cf417721 (1 finding, scripts/test_subagent_tools.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fd09597808f4 | low | unverified | high | 98-110 | test | worker |  | other-039 | 054ee19 | Lines 100-101 create patch_path, lines 103-107 run agent_diff_check.py and assert only returncode == 0 and the keys 'verdict' and 'changed_files'. Line 110 unlinks patch_path in finally. The patch file is never read or checked for existence, and no --baseline is passed, so the 'baseline' in the test | Check whether patch_path exists and is non-empty after the run, and assert on it when the tree has a change. Run the test from a copy without a .git directory to see whether it fails in get_git_status. |

### C-cf4faaed (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6ab0a8b04e54 | low | unverified-scanner | low | 13 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_75798C has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-cf7c9586 (1 finding, scripts/verify_all_compatch.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 41f3b43940dc | low | unverified | medium | 27-29 | other | worker |  | other-040 | WORKTREE | Line 28 `sys.path.insert(0, str(_HERE.parent))  # dedup_tga_tpc.py in the repo layout` says dedup_tga_tpc.py is in the parent directory. Line 29 says it is only in a 'flat copy' beside the script. In this snapshot scripts/dedup_tga_tpc.py and scripts/_common.py sit in the script's own directory (`_H | Check whether the repo root has any dedup_tga_tpc.py. If it does not, the line-28 insert and its comment are stale. If the real repo layout differs from this snapshot, reconcile the comment with the real layout. |

### C-cfdb7c43 (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 36969a8ed550 | low | unverified | high | 2307-2314 | other | worker |  | other-025 | WORKTREE | cmd_world: `model = args.upscaled` then `if not model: for r in eligible: ...; print("\n".join(log)); return 0`. args.deploy (declared at 2560) and --only are never checked on this path. The survey prints and the process exits 0 with no 'nothing deployed' message. | Run `generate_ultrawide_textures.py world --category plc --deploy` on a scratch install and check the exit code and output for any message about the ignored --deploy. |

### C-d0d988e3 (1 finding, scripts/swarm_argv_probe.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7cb7652cdde4 | low | unverified | high | 212 | test | worker |  | other-034 | fdee555 | `return 0 if control_ok else 1` (line 212). base_diff is computed at line 179 and printed as 'DIFFERS' at 199-202, but it never affects the return value. | Check the current output with the fork and bundle argv differing (or inspect the printed block at 199-204) and confirm rc is 0. Decide whether a non-empty base_diff should set the exit code. Confirm that no CI or gate script invokes swarm_argv_probe.py. |

### C-d207ccb9 (1 finding, patch_manager_mods/action_hotbar_src/ActionHotbar.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6131064bad35 | low | unverified | medium | 2082 | logic | worker |  | code-002 | 25d4e42, 7df1b4a | `same = (!g_charKey.empty() && g_charKey.rfind("slot", 0) != 0 && cr == lastCreature && oid == lastOid && g_partyCount == lastParty)`, with the comment "...so fast-travel sentinels / load-in-progress re-evaluate". A fast-travel sentinel such as 0xFFFFFFF9 now resolves to "objfffffffd" (HotbarLogic.h | Count computeCharacterKey calls per second while the controlled creature has an invalid id (0xFFFFFFFF, not the Exile). Decide whether the comment or the predicate reflects the intent, and check whether a fast-travel sentinel key should be re-evaluated while the id stays the same. |

### C-d213c6d8 (1 finding, scripts/master_ledger.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bd95cfd36735 | low | unverified | medium | 86-89 | logic | worker |  | other-028 | 93385ec, d331219 | `if "unittest" in sys.modules: for arg in sys.argv: if "test" in arg.lower(): return True`. Combined with the guard at lines 181-183 (`if is_test_environment() and ledger_path is None and "MASTER_LEDGER_PATH" not in os.environ: return record`), a production run whose argv contains any 'test' substri | Check which modules import unittest in the kpm_hotswap import chain (kpm_status, _static_common). Consider replacing the argv scan with an explicit env flag, or write a stderr notice when the guard drops an event. |

### C-d2f022e4 (1 finding, scripts/shadow_replay.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 841a6271e85a | low | unverified | high | 66-68 | bug | worker |  | other-033 | 31aa40c, d331219 | except Exception as e: elapsed_ms = ...; return None, elapsed_ms. The exception is never used or logged. An HTTP 404 or 500, a bad URL, or a non-JSON body all become the same 'OFF' result. | Point KEV_08B_URL at a live server with a wrong path that returns 404, run the script, and check whether the only evidence is 'OFF' in the table. Consider whether the status code or exception text should be printed to stderr. |

### C-d899c024 (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d9cb6fa0fc97 | low | unverified-scanner | low | 11 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_7575B5 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-d8fc3a30 (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| fff9527fde44 | low | unverified | medium | 98 | style-risk | worker |  | other-025 | WORKTREE | `logging.disable(logging.CRITICAL)` at module import disables every logger in the process, including those of pykotor and Pillow and anything this script might add. | Temporarily remove the call, run `scan` and `world --category plc` on the install, and check whether any pykotor or Pillow warnings appear that the run currently hides. |

### C-d9c42c05 (1 finding, scripts/kotor2_toolset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0f817a2e8855 | low | unverified | medium | 158-160 | build | worker |  | other-028 | WORKTREE | `with tarfile.open(tar_path, "r") as tar:` / `tar.extractall(game_dir, filter="data")`. The `filter=` keyword exists only on Python 3.12+ and on backported patch releases (3.8.17+, 3.9.17+, 3.10.12+, 3.11.4+). Older interpreters raise TypeError before any file is written. | Check the interpreter version this toolset targets (the shebang is python3). Confirm that `restore` on an older patch release fails before writing anything, and decide whether to guard on hasattr(tarfile, 'data_filter') with a clear error. |

### C-db4e81ca (1 finding, scripts/clef_dataset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 96fafd9faea4 | low | unverified | medium | 42 | build | worker |  | other-018 | bbb038f, d331219 | clef_dataset.py:42 'def parse_duration_ms(val, bare_is_ms: bool = False) -> float \| None:' and line 60 'Path \| str \| None' use PEP 604 unions that are evaluated at definition time, with no 'from __future__ import annotations' (the header at lines 14-19 has none). compat_check.py:23 uses that import  | Find the interpreter version the repo targets (CI config or any version file). If 3.9 is supported, run `python3.9 -I -c "import sys; sys.path.insert(0,'scripts'); import clef_dataset"` and expect a TypeError at line 42. |

### C-dc40e2ae (1 finding, scripts/test_compat_check.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 35e099fc761f | low | unverified | low | 520-547 | test | worker |  | other-036 | 5b856ce | `orig_content = self.k2addr_path.read_text(encoding="utf-8")` (520) and the restore `self.k2addr_path.write_text(orig_content, encoding="utf-8")` (547) mutate a tracked source file in place. read_text uses universal newlines, so if K2Addr.h is stored with CRLF the restore writes LF endings. | Check the line endings of patch_manager_mods/_shared_src/K2Addr.h (e.g. with `file`). On a clean tree, run the two scratch tests, then compare `git status`/`git diff --stat` for K2Addr.h. |

### C-dc54aa8d (1 finding, scripts/test_plan_manager.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 7e3d91b4a6ef | low | unverified | medium | 56-57 | logic | worker |  | other-038 | 5b856ce, d331219 | Lines 56-57 now require the scoped text form '/plan-handoff "<stem>:M1"'. plan_manager.py:355 prints that scoped form, but the same file still emits the unscoped form at plan_manager.py:310 ("next_handoff": f"/plan-handoff {current['ident']}") and plan_manager.py:428 (f"Next Handoff: /plan-handoff { | Find which command prints plan_manager.py:428 and whether it should use the scoped identifier. Check whether the JSON next_handoff should match the text form. If scoping is intended, add a value assertion for next_handoff in test_resume_json. |

### C-dd11bf6e (1 finding, research/companion_levelup/tools/parity/scenarios.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| e23534b73f29 | low | rejected | high | 42 | build | worker |  | other-010 | WORKTREE | scenarios.py:42 adds `"-I", os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common")` to the g++ command in build(). This is a home-directory path to a workspace outside the repository. A Glob for '**/KpLog*' returned no files under tools/parity or under patch_mana | Locate KpLog.h. Check whether it is vendored in the repo or whether a repo-relative or environment-configured include directory should replace the home path. To confirm the failure mode, run scenarios.py on a clean checkout (or with HOME pointed at an empty temporary directory) and expect a COMPILE  |

### C-dd53ea84 (1 finding, patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| febeacd5069c | low | unverified-scanner | low | 232 | other | scanner |  |  | 25d4e42, 7df1b4a | [scanner sentinel-range] 0xFFFFFFFEu \| KT_CHECK_EQ(characterKeyFor(0xFFFFFFFEu, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(!bad); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-dfacc0ac (1 finding, scripts/clef_dataset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| d483dec38aca | low | unverified | medium | 26 | logic | worker |  | other-018 | bbb038f, d331219 | clef_dataset.py:26 hard-codes DEFAULT_LEDGER = REPO_ROOT / ".claude" / "master_ledger.jsonl", and clef_matcher.py:298 calls extract_ledger_facts() with no path. master_ledger.py:97 reads MASTER_LEDGER_PATH, and log_event writes to get_ledger_path(None) (master_ledger.py:185). With the variable set,  | grep scripts/test_clef_matcher.py for MASTER_LEDGER_PATH and check whether any test asserts priors that depend on the temp ledger's contents. Then compare `MASTER_LEDGER_PATH=<scratch> python3 -I scripts/clef_dataset.py summary` with `python3 -I scripts/clef_dataset.py summary --ledger <scratch>` to |

### C-dfc6e71c (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 2ba85aedca09 | low | unverified | high | 2191-2194 | logic | worker |  | code-007 | 97c1bd8, dc4bb61 | Line 2191 fread(g_r10TagsBuf, 1, sizeof g_r10TagsBuf - 1, f) requests 65535 bytes. Line 2192 sets full = feof(f) != 0. Line 2194 returns false with 'larger than 65535 bytes' when full is false. A file of exactly 65535 bytes is read completely without setting the EOF indicator, since no read past the | Host test: write a file of exactly sizeof(g_r10TagsBuf)-1 bytes beside a table and call r10TagsReady; check whether the 'larger than' warning fires. Confirm the stdio behaviour for an exact-count fread on the target C runtime, and check whether reading one more byte, or comparing against the stat si |

### C-e075fc0b (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8f25adb1e270 | low | unverified-scanner | low | 8 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_66C95C has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-e0cc51bf (1 finding, scripts/kpm_hotswap.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3655a14d11f0 | low | unverified | medium | 35-38 | logic | worker |  | other-028 | d331219 | `try:` / `import master_ledger as ml` / `except ImportError:` / `ml = None`. Every later ledger write is guarded by `if ml:` (lines 339, 485), so a missing or broken ledger module turns the audit record off with no message. | Print a one-line warning on the ImportError path, or make the import unconditional, since master_ledger.py is in the same directory and has no third-party dependencies. |

### C-e1c7d378 (1 finding, scripts/02_merge_2da_and_tlk.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| aae3fb4ec1a2 | low | rejected | medium | 208-213 | logic | worker |  | other-013 | WORKTREE | Line 209 reads `old_app = gff.root.get_uint32("Appearance")` from the staging .utp, then line 210 checks `if old_app in plc_map:` and writes the mapped value back. Lines 205, 212, 219 and 225 write these edits into STAGING_OVERRIDE in place. The only idempotency guard (lines 125-128, `already_applie | Check whether the 'pre-compatch backup' the docstring refers to (lines 13-14) includes STAGING_OVERRIDE. On a scratch copy, run the script once, record the Appearance values in the staging .utp files, run it again after restoring only the live side, and diff the values. |

### C-e295dc0e (1 finding, scripts/compat_graph.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 3ff8a6367727 | low | unverified | high | 122 | build | worker |  | other-020 | 3ec1365 | Fallback default: self.kpm_dir = (Path.home() / 'Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager').resolve() (line 122), used when neither the kpm_dir argument nor KPM_DIR is set. | On a machine or temp HOME without that path, leave KPM_DIR unset and construct CompatGraph. Check that no warning is emitted and that the kpm: resolution branch (lines 349-356) is skipped. Grep the repo for other references to the same path. |

### C-e3304d33 (1 finding, patch_manager_mods/gui_input_src/GuiInputProvider.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 413c092c1e9c | low | unverified-scanner | low | 159 | other | scanner |  |  | 68a7739 | [scanner pointer-math] c-pointer-cast \| KP_LOGV(chCore, KPLOG_WARN, "wheel subscribe cb=%p -> -1 (game window not subclassed yet)", (void*)cb); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-e389fae9 (1 finding, patch_manager_mods/action_hotbar_src/HotbarLogic.h)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 5cc3bf09d149 | low | unverified-scanner | low | 75 | other | scanner |  |  | 25d4e42 | [scanner sentinel-range] 0xFFFFFFF9 \| //    e.g. 0xFFFFFFF9 during fast-travel); all map to canonical "objfffffffd".) | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-e57185db (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 889ce2b6d80d | low | unverified-scanner | low | 4 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoHold_74F3C7 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-e74f3a28 (1 finding, scripts/generate_ultrawide_gui_overrides.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a459337eb7ec | low | unverified | high | 1532-1535 | logic | worker |  | other-023 | WORKTREE | plan_targets adds a frame-bar GUI only `for texture in frame_edges or {}: if texture in FRAME_BAR_RULES:` (1532-1533). Textures that are not in FRAME_BAR_RULES are silently ignored. main() still prints `"Frame-bar rule enabled: " + ...` for every entry of frame_edges (1780). | Run with a frame_edges.json that contains only a texture not in FRAME_BAR_RULES and check the console line, the report header (1829) and the output files. Confirm no GUI is generated for it. |

### C-e9ef5275 (1 finding, scripts/verify_all_compatch.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1ba4523222b3 | low | unverified | medium | 114-116 | test | worker |  | other-040 | WORKTREE | The docstring (line 3) says 'Module capsules (7 merged modules + 128 new standalone modules)'. The only check on the wider set is `assert len(all_mods) >= MIN_MODULES` (line 115, with MIN_MODULES = 200 on line 54). The 128 standalone modules are never checked for content or exact count, yet line 116 | Compare the 200 floor with the documented 7 + 128 and the vanilla module count, and confirm where 200 came from. Decide whether standalone modules need an exact count or a per-file check, and whether the line-116 wording should say only that the total was counted. |

### C-ea19bd5a (1 finding, patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 06dcdea97558 | low | unverified | low | 144 | logic | worker |  | code-005 | 2db77c4, bb080cf | Line 144 (`syncScreen();` in loadIni) and line 122 (same call on the no-INI path) are new. loadIni() is reached from ensureInit() (lines 404-410), which the first hook can trigger (lines 418-422, 474, 548) before any tick. Before this change, syncScreen() ran only from cursorEngine() (line 335) and  | Read guikit::syncScreen (GuiKit.h, outside this packet's directory) and confirm it only reads the engine globals and returns false for 0x0 or out-of-range values, without dereferencing any pointer that could be null at that point. Confirm which hook first calls ensureInit() (lines 418-422) and wheth |

### C-ea9a73db (1 finding, patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| abff05d4105e | low | unverified-scanner | low | 1067 | other | scanner |  |  | 32c8125, 4add886 | [scanner pointer-math] uintptr_t-cast \| KT_CHECK_EQ(it.creature, (uintptr_t)0x1000); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-eb080bf9 (1 finding, scripts/subagent_prompt.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4ed98223b686 | low | unverified | high | 352-353 | logic | worker |  | other-034 | 054ee19, 31aa40c | The else branch is commented `# Worker roles (code, gate, test, doc)`, but RECON_ROLES (line 43, outside this packet) includes 'deep', and no branch at 295-351 handles 'deep' or 'gate' (only ghidra, scout, verify, review). So role 'deep' (header 249-250 'conduct deep technical investigation') and 'g | Call build_self_contained_prompt(role='deep') with mod_scope=None and compare the body with role='scout'. Check whether 'Run verification commands' and 'Edit ONLY targeted files' appear, and whether any read-only statement appears. |

### C-eb4d5ee0 (1 finding, patch_manager_mods/level_up_override_src/LevelUpOverride.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 82c0ff3b45e5 | low | unverified-scanner | low | 1537 | other | scanner |  |  | 97c1bd8, dc4bb61 | [scanner pointer-math] reinterpret_cast \| KP_LOG(chPlay, "GUID save g%08x table=%08x ok=%d", (unsigned)g_guid, (unsigned)reinterpret_cast<uintptr_t>(table), ok); | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-ec2fb23e (1 finding, patch_manager_mods/action_hotbar_src/ActionHotbar.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 839b7ed15467 | low | unverified | low | 1763-1765 | logic | worker |  | code-002 | 25d4e42, 7df1b4a | `char cur[260] = {}; if ((!dir \|\| !*dir) && g_se.currentSaveFolder(cur, sizeof cur) > 0 && cur[0]) { dir = cur; }`. The return value is only checked for > 0. The contract fake at common_contract_test.cpp:812-820 returns the full strlen while copying at most cap-1 bytes, so a longer path is silently  | Check k2se::Client::currentSaveFolder in SaveEvents.h (not in this packet) for its return value: full length or bytes copied. If it is the full length, check whether the return value is less than sizeof cur before using the buffer. |

### C-ee7b459b (1 finding, scripts/generate_ultrawide_textures.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 271463fdc6da | low | unverified | high | 1351-1352 | other | worker |  | other-025 | WORKTREE | `if rec["reason"] is None and world_factor(rec["layout"]["width"], rec["layout"]["height"]) < 2: rec["reason"] = f"already {WORLD_MAX_SIDE} or more"`. world_factor (1313-1316) drops the factor to 1 whenever side * 2 > WORLD_MAX_SIDE, i.e. for any long side above 512. A 600 px texture is therefore re | Feed world_textures a TPC with a 600 px long side (or call world_factor(600, 600)) and compare the reason text with the actual cap rule. Reword the reason so it states the cap rule that applies. |

### C-f09735ac (1 finding, scripts/pre_install_gate.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a430978e960a | low | unverified | medium | 273-279 | logic | worker |  | other-029 | d331219 | Line 258 captures 'skipped = step_wine(...)', the DISPLAY-skipped rows, and lines 268-269 print them as 'not proven by this gate'. The new BUILD_GATE PASS record (lines 274-279) writes "verdict": "PASS" and "smokes": all_rows with no skipped list and no record of --skip-default (line 241-242), so th | Run the gate in an environment with no DISPLAY so a wine row is skipped, then inspect the appended BUILD_GATE event in master_ledger.jsonl for any field marking the skip. Check whether any reader of BUILD_GATE events in scripts/ depends on the smokes list meaning 'proven'. |

### C-f1a1517f (1 finding, scripts/test_compat_check_rc.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0322f77b9961 | low | unverified | high | 90-98 | test | worker |  | other-036 | b348ca3 | `rc, data = self._run_main(0, out)` (line 92) binds rc, but no assertion ever checks it. The assertions at lines 93-98 only look at PASS entries, ERROR entries and counts['ERROR']. | Confirm what main() returns when build rc=0 and the fake run_tests returns 0 (compat_check.py ~1190-1200). If it should be 0 on this path, the test is missing that assertion. |

### C-f62e791f (1 finding, tests/static/subclassers.toml)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0adc440c6f2a | low | unverified | high | 25-26 | other | worker |  | other-043 | 2db77c4, e947d12 | Line 26 sets `restore_hint = 2108` for ActionHotbar.cpp, but the restore call is at patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:2110: `WheelHook::uninstall(&wheelProc);`. The install hint (line 25, `1188`) does match ActionHotbar.cpp:1188. The file header (line 11) says hints are informati | Grep ActionHotbar.cpp for `WheelHook::uninstall` to confirm the line number, then check that scripts/check_subclassers.py prints the hint-drift INFO line for this entry and still exits 0. Decide whether hints should be kept exact or whether the header's 'never checked' note is the intended contract. |

### C-f6415a21 (1 finding, scripts/clef_dataset.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 8f476c5dc9e8 | low | unverified | high | 162-179 | logic | worker |  | other-018 | bbb038f, d331219 | clef_dataset.py:179 puts CLEF_ROUTE in the stat-producing event tuple, but the reader looks up keys that CLEF_ROUTE rows do not carry: rec.get("mod") (145), rec.get("role") (162), rec.get("model") (164), rec.get("duration_ms") (165). clef_matcher.py:400-412 logs 'decision', 'routed_model', 'active_m | Using a scratch ledger copy, run route_task with MASTER_LEDGER_PATH set to that copy (log_event honours it, master_ledger.py:97), then run `python3 -I scripts/clef_dataset.py json --ledger <scratch>`. Confirm `mods` and `roles` do not change while total_events increases and global_prior.best_backend |

### C-f69a97ab (1 finding, neocities_mods_detailed.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| b85d3036aaeb | low | unverified | high | 478 | logic | worker |  | other-003 | WORKTREE | Line 478: "category": "Appearance Change / 4 - Option", while every other tier-4 entry uses "4 - Optional" (e.g. line 268 "Immersion / 4 - Optional", line 1518 "Graphics Improvement / 4 - Optional"). | Find the template or script that groups by the tier label and check whether it matches the literal '4 - Optional'. Confirm where entry 48 is rendered. |

### C-f7bd1d9a (1 finding, patch_manager_mods/level_up_override_src/exports.def)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cfde70543e9e | low | unverified-scanner | low | 18 | other | scanner |  |  | 97c1bd8 | [scanner export-no-definition] symbol LuoInstant_5FD7A0 has no definition in 12 source files under patch_manager_mods/level_up_override_src | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-f91cc687 (1 finding, scripts/agent_contract.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0e8a2f5aa464 | low | unverified | medium | 29-32 | other | worker |  | other-015 | d331219 | `try:` / `import master_ledger as ml` / `except ImportError:` / `ml = None`, then at line 317 `if ml:` guards `ml.log_event("CONTRACT_CHECK", {...})`. Any ImportError disables the call with no message. | Import agent_contract from a directory other than scripts/ (for example from a test) and check whether `ml` is None and whether a CONTRACT_CHECK line is appended to .claude/master_ledger.jsonl. Decide whether an ImportError should be reported on stderr rather than ignored. |

### C-fc56edb5 (1 finding, scripts/gpu_mode.py)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 6da8d392f47c | low | unverified | high | 163 | gate-integrity | worker |  | other-026 | 93385ec, d331219 | `run_cmd(["systemctl", "--user", "stop", KEV_SERVICE])` in the `if stop_all:` branch. The return value (rc) is discarded, and the next lines (167-174) proceed to the VRAM verdict. | Make the user unit fail to stop (for example, a missing unit name in a test copy of KEV_SERVICE) and run `gpu-mode game --all`. Check whether the exit status and the printed verdict still reflect the failed stop. |

### C-fc905fc3 (1 finding, patch_manager_mods/queue_hud_src/QueueHud.cpp)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| f06710524125 | low | unverified-scanner | low | 994 | other | scanner |  |  | 2c071d8, d4ef936 | [scanner pointer-math] hex-offset \| if (rd(SiteQueue + 0x36, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x20, &posPtr) && posPtr) { | Confirm or dismiss this scanner candidate by reading the cited lines; no worker reported it. |

### C-fcbb8368 (1 finding, neocities_mods.json)

| id | sev | verification | conf | lines | category | source | dup-of | packets | commits | evidence | investigation |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 21702abb4f6d | low | unverified | low | 426 | other | worker |  | other-001 | WORKTREE | Line 426 reads "url": "http://www.nexusmods.com/kotor2/mods/9/?". The URL ends in a bare '?' with an empty query, and mod ID 9 is out of line with the 1xxx IDs used for the other kotor2 Nexus entries (lines 146, 156, 166). Line 566 also ends in an empty '#' fragment. | Open the Nexus page for kotor2 mod 9 in a browser and check that its title matches 'Darth Malak's Armor'. If it does not, the correct ID needs to come from the mod author or from the Nexus search page. |
