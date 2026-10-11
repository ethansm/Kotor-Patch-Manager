# p2c findings triage (read-only; $0 dr.py spend)
Tree: HEAD efc5335 + working tree, vs audit pin 8ba3029 (earliest pin 7020d54). Verdicts: prefilter (quote check + git log), Haiku recon x4, Opus spot-checks.

## 1. Summary
28 representatives = **25 STILL-REAL + 1 STALE + 2 UNCLEAR**. The 13 REFUTED findings were not triaged. Of the 25 real: 14 by deterministic prefilter (file untouched since the pin, quotes verbatim), 11 by Haiku recon.

## 2. STILL-REAL (by severity)
| id | severity | tier | fl | claim2 | evidence |
|---|---|---|---|---|---|
| e606f41d177d | high | CONFIRMED | scripts/02_merge_2da_and_tlk.py:234 | Line 116 reads every live table except planetary.2da (`if n != "planetary.2da"`), so the live file is never opened. | scripts/02_merge_2da_and_tlk.py:116,234-236 (untracked file) |
| 1ce64f3b2759 | high | CONFIRMED | scripts/agent_diff_check.py:106-117 | run_tests() sets all_ok=False only when `res.returncode != 0` (lines 114-115) and otherwise returns True. | scripts/run_tests.py:862 quote verbatim present; no commits or edits since 7020d54 |
| 119182a14fc2 | high | CONFIRMED | scripts/plan_manager.py:349-356 | Line 350 builds scoped_ident = f"{doc_prefix}:{clean_ident}" (e.g. | scripts/plan_manager.py:531 bare-cell match, scoped 'NN:Tn' ident (:348) never matches; fallback append :578-580 |
| bc58247fd299 | medium | CONFIRMED | patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:358-364 | Loop reads: `uintptr_t mem = party::member(t, i); if (mem == c // (mem != 0 && creatureOid(mem) == oid)) { partySlot = i; break; }`. | ActionHotbar.cpp:385 OID-only match; needs live evidence a non-Exile shares sentinel 0xFFFFFFFF |
| 9e76e8f4642a | medium | CONFIRMED | patch_manager_mods/gui_input_src/gi_registry_test.cpp:137-144 | The new case routes wheel_subscribe to `fakeWheelSubNoWindow` (line 47: `{ ++g_subNoWinCalls; return -1; }`) through `g_fakeTableNoWindow` (line 139). | patch_manager_mods/gui_input_src/gi_registry_test.cpp:137 quote verbatim present; no commits or edits since 7020d54 |
| 81dabb3f4f5e | medium | CONFIRMED | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:1-3 | Header line 3 now reads: "The only engine write is the sanitizeNorm path in ProbeMouseNorm (this+0x408/+0x40C), on by default (sanitize_norm=1 in [mouse_fix]...)". | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:3 quote verbatim present; no commits or edits since 7020d54 |
| eee579216787 | medium | CONFIRMED | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:189-193 | Line 193: `POINT cur = {0, 0}; if (dyn_GetCursorPos) dyn_GetCursorPos(&cur);`. | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:192 quote verbatim present; no commits or edits since 7020d54 |
| d682bb833922 | medium | CONFIRMED | patch_manager_mods/queue_hud_src/QueueHud.cpp:973-1036 | QueueHudPerformCastSpell returns early only on g_dispatching / !g_cfg.enabled / g_combat != 0 (line 974) and never checks party membership. | QueueHud.cpp:1634-1635 `if (fb) memberK = 0;` non-party caster still mapped to member 0 |
| f77ae8184a09 | medium | CONFIRMED | patch_manager_mods/queue_hud_src/QueueHudLogic.h:575-580 | shouldReInject returns hasFollowLeader whenever the action is absent from the engine queue, the buffer is active, and it is not pending, cancelled or over the cap (lines 578-580). | QueueHudLogic.h:674-679 shouldReInject returns hasFollowLeader, no completion signal |
| 035d6f241dd2 | medium | CONFIRMED | research/companion_levelup/tools/parity/gate_test.cpp:301-310 | Line 289: `// F4: the 0x740FC0 guarded read needs a readable ecx`. | research/companion_levelup/tools/parity/gate_test.cpp:301 quote verbatim present; no commits or edits since 7020d54 |
| d121f4a74209 | medium | CONFIRMED | research/companion_levelup/tools/parity/unit_gap_test.cpp:48 | Known facts record six CHECKs removed (old lines 48-57): logOpen below cap, logf stops at cap, one cap marker, counter == MaxLogLines, logOpen refuses at cap, logLine writes nothing at cap. | research/companion_levelup/tools/parity/unit_gap_test.cpp:48 quote verbatim present; no commits or edits since 7020d54 |
| 2bb645e71f18 | medium | CONFIRMED | scripts/abi_check.py:362-367 | When the per-interface dumper is missing, `candidates = sorted((repo_root / "tests/abi").glob("abi_dump_*.cpp"))` and `dumper_src = candidates[0]` take whichever dumper sorts first; the only new effect is a stderr line ( | scripts/abi_check.py:366 quote verbatim present; no commits or edits since 7020d54 |
| 822afff6842c | medium | CONFIRMED | scripts/generate_ultrawide_gui_overrides.py:694-696 | is_game_running() wraps the ps probe in `except Exception: pass` and then `return False` (694-696). | generate_ultrawide_gui_overrides.py:694-696 probe swallows exception -> False (untracked) |
| 6ccefb375bd5 | medium | CONFIRMED | scripts/generate_ultrawide_gui_overrides.py:1680-1684 | registration_check's `ok` is an always-true expression for any content_k in (0, 1]: `abs(fx(729) - fx(723)) <= 6*k` is exactly 6k, and the two portrait checks (`409 < 583 < 716`, `530 < 583 < 636`) are strict orderings t | generate_ultrawide_gui_overrides.py:1681-1683 check cannot MISMATCH; result discarded :1835 |
| d2c66b77d42f | medium | CONFIRMED-BY-SONNET-ONLY | research/companion_levelup/tools/parity/scenario_test.cpp:408-409 | scenario_test.cpp:408-409 is `} else if (a == "logcap") {` whose only body line is a comment: 'nothing to force here, the scenario still checks the gate is unaffected'. | research/companion_levelup/tools/parity/scenario_test.cpp:418-419 logcap branch still empty (moved from 408-409) |
| 66c1305db6f7 | medium | CONTESTED-NEEDS-HUMAN | scripts/subagent_prompt.py:192-218 | Line 192 takes the first `[a-zA-Z0-9_]+_src` token in all_context; lines 211-212 `elif "companion" in ctx_lower:` set `companion_ai_src`; the path test `if "companion_levelup" in plan_str or "level_up" in plan_str:` (217 **needs human judgment before any fix** | scripts/subagent_prompt.py:211 quote verbatim present; no commits or edits since 7020d54 |
| d74365c9d82d | low | CONFIRMED | patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:696-697 | refreshCache sets `c0.have = 0 < cnt && v[0].ok;` (696) and clears only pk/apk when false (697). | CompanionSelfHeal.cpp:770,852 leader `have` never cleared (moved from 696/777) |
| 6adfec63f6fb | low | CONFIRMED | patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:70-73 | Line 70: `constexpr int MaxWarnLines = 4000;` and lines 71-73: `void logw(...) { if (g_warnLines >= MaxWarnLines) return; ++g_warnLines;`. | patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:70 quote verbatim present; no commits or edits since 7020d54 |
| ff23d43b210c | low | CONFIRMED | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:220-221 | Line 184 only rejects a null pointer: `if (!rd(ebp, -0x28, &self) // self == 0) return;`. | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:184 quote verbatim present; no commits or edits since 7020d54 |
| b5a7154f89eb | low | CONFIRMED | research/companion_levelup/tools/parity/parity.py:33 | parity.py:33 adds `"-I", os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common")` to the g++ argv of every build() call (gate, apply, ui, playthrough, m32). | research/companion_levelup/tools/parity/parity.py:33 quote verbatim present; no commits or edits since 7020d54 |
| 6a2684bd832c | low | CONFIRMED-BY-SONNET-ONLY | neocities_mods_detailed.json:10 | Entry 1 notes (line 10): "When attempting to run this executable, Windows/your antivirus might attempt to block it, because it's an unsigned executable ... | neocities_mods_detailed.json:10 still advises AV override for unsigned exe ('HERE' link not re-checked) |
| ce3f285cbcb0 | low | CONFIRMED-BY-SONNET-ONLY | patch_manager_mods/level_up_override_src/test.toml:4 | test.toml:4 adds "-I$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common" to the committed [common_contract_test] cxxflags. | patch_manager_mods/level_up_override_src/test.toml:4 quote verbatim present; no commits or edits since 7020d54 |
| ed6f9f335c3f | low | CONFIRMED-BY-SONNET-ONLY | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:305-306 | Lines 288-290 validate msgp only through reads: `rd(msgp, 4, &id)` and `rd(msgp, 0xC, &lp)`. | patch_manager_mods/mouse_probe_src/MouseProbe.cpp:286 quote verbatim present; no commits or edits since 7020d54 |
| 9059eb31f144 | low | CONFIRMED-BY-SONNET-ONLY | patch_manager_mods/queue_hud_src/QueueHud.cpp:734-739 | actionInQueue is set for any T_CAST_SPELL engine entry (line 734), for cur.subType 9 (line 737) and for rn subType 9 (line 739). | QueueHud.cpp:1162-1167 actionInQueue includes T_CAST_SPELL (spec in 30_queue_hud.md may intend it) |
| e3ae385c8f94 | low | CONTESTED-NEEDS-HUMAN | research/companion_levelup/tools/parity/stub/LuoLogHost.h:9 | Lines 3-4 say: "KpLog also keeps the fallback file open, so remove()ing it leaves later writes on an unlinked inode: reset through the public API instead." Line 9 still does `static inline void luoHostLogReset(const char **needs human judgment before any fix** | research/companion_levelup/tools/parity/stub/LuoLogHost.h:3 quote verbatim present; no commits or edits since 7020d54 |

## 3. STALE (fixed by other sessions)
| id | severity | fl | evidence |
|---|---|---|---|
| 8ea1731f8816 | medium | patch_manager_mods/queue_hud_src/QueueHud.cpp:720-799 | df26bd3 (QL1 identity guard) QueueHud.cpp:1095-1144; skip[k] blocks dispatch when buffer owner is in no slot; residual: address reuse (ABA) unverified |
Who fixed: df26bd3 "feat(queue-hud 0.5.0-dev): pause restore, ghost cur, identity guard, combat hand-off (QL1)" (2026-10-11).

## 4. UNCLEAR
| id | severity | tier | fl | evidence |
|---|---|---|---|---|
| 843bd1103025 | low | CONTESTED-NEEDS-HUMAN | patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:164-180 | LevelUpOverride.cpp:164-180 logging moved to KpLog core; the Common KpLog.h source is not in this repo, so cap/flush behaviour cannot be checked |
| 5a447a280525 | low | CONTESTED-NEEDS-HUMAN | research/companion_levelup/tools/parity/scenario_test.cpp:177 | stub/LuoLogHost.h:9 remove+KpLog_SetFallbackFile; stale-fd behaviour depends on KpLog.h, absent from repo |
Both depend on the Common `KpLog.h` core, which is not in this worktree; resolve by reading it where it lives, or ask the human.

## 5. Audit gaps that still apply
- refute-audit `batch_02` never ran.
- 26 queued verify units never verified (waived via `[coverage] allow_unverified`).
- Medium-and-below findings beyond the 60 queued were not verified.
- 2 map packets waived (`other-024`, `other-035`).

## 6. Before the fix half
- Add a `[fix]` section to the p2c profile (model: `.claude/deep-review/pilot/profile.toml`).
- CORRECTION (2026-10-11): adding `hook-asm`/`cpp-logic`/`gate-integrity` to `specialty_names` is NOT needed; these 28 findings use builtin specialties only. A `[fix]` profile copy and stale-free input are built: see `.claude/deep-review/p2c/fix/READY.md`.
- `python3 $DR fix_intake.py --stats` is the free find-only check; `fix-go` and `findings-skim` are the user's gates.
