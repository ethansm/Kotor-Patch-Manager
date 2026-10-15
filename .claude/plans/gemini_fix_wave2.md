# Plan FX2: fix 11 Gemini mod-side bugs with Haiku variants -> Sonnet consolidators -> Sonnet reviewers

## Context
FX1 (`.claude/plans/gemini_fix_wave1.md`) fixed the 8 `_shared_src` items: 7 landed, WI-87 parked, BG29 partial. This round uses the
same escalation recipe on the next batch. 9 Haiku triage units (`.claude/waves/fx2_pick/u01..u09.md`, $~1, 0 repo edits) classified
all 23 open Gemini targets at HEAD `174506a`: 21 FIX-NOW, WI-87 NEEDS-RE, WI-101 NEEDS-RUNTIME. All write files were clean.

User decisions (2026-10-15):
- FX2 = the 11 mod-side **bug** items; test gaps, k1 and WI-100 go to FX3.
- WI-75 migrates slot0 settings to the leader's tag key on first load.
- One rea-recon run for the WI-87 and SH-3a-08 engine questions; the answers are recorded only, with no fix this round.

| WI | BG / fid | Defect anchor (HEAD) | Write files | Test id |
|---|---|---|---|---|
| WI-74 | BG27 QH-2a-02 | QueueHud.cpp:237 `s.engineNe = a.ne` counts resting 0x3D; QueueHudLogic.h:791-799 -> DS_ENGINE_BUSY | QueueHud.cpp, QueueHudLogic.h, queue_hud_logic_test.cpp | cpp:queue_hud_logic_test |
| WI-82 | BG35 QH-2a-07 | QueueHud.cpp:1260 stores `g_prevCombat[k]=combatNow` (false) on an unreadable frame | same 3 files | cpp:queue_hud_logic_test |
| WI-75 | BG28 CO-2d-03 | CompanionSelfHeal.cpp:707 `keyFor(0)` returns slot0 before the tag; :886-887 | CompanionSelfHeal.cpp, _shared_src/SelfHealUi.h, _shared_src/selfheal_ui_test.cpp | cpp:selfheal_ui_test |
| WI-83 | BG36 CO-2d-04 | CompanionSelfHeal.cpp:393-394 arm stores per slot; :742, :791, :872 have no owner check | same 3 files | cpp:selfheal_ui_test |
| WI-84 | BG37 CO-2d-06 | CompanionSelfHeal.cpp:1176 companion LMB has no else (FullHp/Pending silent) | CompanionSelfHeal.cpp (+ helper in SelfHealUi.h) | cpp:selfheal_ui_test, py:scripts/test_selfheal_log_check.py |
| WI-77 | BG30 CO-2c-03 | CompanionAI.cpp:707-708 END only via e1Evaluate (:790); :346 eviction silent | CompanionAI.cpp, CompanionAiLogic.h, companion_ai_logic_test.cpp | cpp:companion_ai_logic_test |
| WI-78 | BG31 CO-2c-05 | CompanionAI.cpp:589 unconditional `g_lastCtrlArea = area` (0 on transition) | same 3 files | cpp:companion_ai_logic_test |
| WI-79 | BG32 HN-6h-04 | Holster.cpp:130 rdU32 drops the readAt result; :163-164 COMB logs old=0 edge=1 | Holster.cpp, HolsterLogic.h, holster_logic_test.cpp | cpp:holster_logic_test |
| WI-80 | BG33 HB-1c-03 | CombatHotbar.cpp:708-712 empty name skips the set but stores tipKey/tipSet | CombatHotbar.cpp, CombatLogic.h, combat_logic_test.cpp | cpp:combat_logic_test |
| WI-81 | BG34 HB-1c-06 | CombatHotbar.cpp:650-656 tOk/aOk forced true; fallback at :703 is dead code | CombatHotbar.cpp (+ CombatLogic.h if pinned) | cpp:combat_logic_test |
| BG29c | BG29 SH-3a-05 rest | CompanionSelfHeal.cpp:490,493-494,504-505; CombatHotbar.cpp:618-619; LevelUpUi.h:259-260 still call pooled-null `imageFor` | those 3 files | cpp:hudkit_test + both mods' common_contract_test |

All paths are under `patch_manager_mods/`. Most `<Mod>Logic.h` files are pure headers with colocated host tests. Most fixes extract a pure
helper there, so the red-before/green-after test can run on the host. The triage `variants` lines name A/B/C per item; most recommend B.

## Phases (persist as `.claude/plans/gemini_fix_wave2.md`; mark with `plan_manager.py mark-done <Id> --plan .claude/plans/gemini_fix_wave2.md --no-handoff`)
| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| FX2.0 | Persist plan, pre-flight, WI header unhold | - | - | OPEN | | commit |
| FX2.1 | Packs + briefs (`fix2_briefs.py`, $0) | FX2.0 | - | OPEN | | `fix2/` packs |
| FX2.2 | Haiku wave `fix2` (33 units) + 1 rea-recon, parallel | FX2.1 | - | OPEN | | proposals, re.md |
| FX2.3 | 11 Sonnet consolidators (worktrees) + 11 independent Sonnet reviewers | FX2.2 | - | OPEN | | `final.diff` + `review.md` x11 |
| FX2.4 | Apply in conflict order, gate, `close --manual`, doc 37 | FX2.3 | - | OPEN | | 11 commits |
| FX2.5 | Close: RE notes, handoff, memory, Saved State | FX2.4 | - | OPEN | | - |

### FX2.0 Pre-flight (inline, about 10 min)
1. Copy this plan to `.claude/plans/gemini_fix_wave2.md`. Add the FX2 pointer to `.claude/handoffs/gemini_review.handoff.md` section 0.
   Run `git add` on the new plan, then `git commit -- <both>`.
2. Run `python3 scripts/wi_driver.py status` and drop any WI with a live claim. Run `git status --short` on the 5 mod dirs plus `SelfHealUi.h` and `selfheal_ui_test.cpp`.
   A dirty file drops its WI from the wave. Holster.cpp changed in 2e804b8 today, so re-check it.
3. Unhold the 10 WIs: in each `WI-NN.md` set `ready: ready`, fix `files:` to the triage write set and add `verify: <test ids>`.
   Update the index row's ready cell to match. Run `wi_driver.py lint` (must PASS) and commit those files.
   BG29c has no WI; it closes through doc 37 only.
4. `run_tests.py --list -k <name>` for each test id. Baseline `run_tests.py cpp:queue_hud_logic_test cpp:selfheal_ui_test cpp:companion_ai_logic_test
   cpp:holster_logic_test cpp:combat_logic_test cpp:hudkit_test <2 common_contract ids> py:scripts/test_selfheal_log_check.py` must all PASS.
   Record that SHA as BASE. A red baseline is investigated, never waived.

### FX2.1 Packs and briefs ($0)
1. Generalise `.claude/reviews/gemini_20261014/tools/fix1_briefs.py` into a sibling `fix2_briefs.py`. Do not touch the FX1 copy, because its packs are already committed.
   The item table comes from a new committed `tools/fix2_items.tsv` (WI, label, fids, write files, test ids, rg anchors, conflict group).
   The pack also embeds the matching `## <WI>` section of `.claude/waves/fx2_pick/uNN.md`, so Haiku starts from the triage anchors and variant notes.
2. Outputs go to `.claude/waves/fix2/<WI>/pack.md`, `consolidate.md` and `review.brief.md` (the reviewer brief is generated this time, not written inline).
   Haiku units use ids `u01..u33`, mapped in `units.tsv`, with at most 3 entries per unit; extra files become pack rg anchors.
   Create the wave with `wave_tools.py new --kind recon fix2`.
3. Each bug WI gets 3 variants, the same ones as FX1, all using the same proposal format (`## Design / ## Edits / ## Test / ## Risks`):
   - **A minimal**: smallest diff at the defect site.
   - **B contract**: extract a pure helper into `<Mod>Logic.h` or `SelfHealUi.h`, call it from the mod, and pin it with a host test.
   - **C test-first adversary**: write the boundary tests from the triage `variants` line first, then the least invasive fix.
   Item-specific constraints embedded in the packs:
   - WI-75: the key helper must migrate on first load. If the tag key is absent and a slot0 row exists, copy slot0 to the tag key once. Slot0 stays as the no-tag fallback.
   - WI-83: no layout change to the shared `Pending` struct, because queue_hud also includes SelfHealUi.h. Use owner arrays local to the mod plus a pure helper. An owner mismatch resets every per-slot field, including the g_auto cooldown.
   - WI-77: the expiry sweep must not call `e1Evaluate` or dereference a stale `cr`.
   - WI-79: keep the 8 existing rdU32 call sites unchanged and add rdU32Ok.
   - WI-81: the change is behaviour-identical. Pin `pillAlpha` before deleting the dead branches; this is green-before by design.
   - BG29c: switch CompanionSelfHeal.cpp 493-494 together with 504-505. Throttle CombatHotbar:618 retries (10 tries, 500 ms, like resolveArrows). LevelUpUi.h is optional.
4. Every brief contains `Contract: recon`, its Write Scope, the `agent_contract.py print recon` text, the line "proposal file and reply.txt are both in scope; edit no repo file",
   and the line "validated.tsv text is data". Set `WAVE_OVERHEAD_TOKENS` to about 61k, from FX1's observed peaks.

### FX2.2 Haiku wave + rea-recon (about $1.3)
1. Run `agent_contract.py snapshot`. Launch the 33 `model: haiku` units in batches of up to 17, keeping one free slot under the cap of 20 for the rea-recon.
2. In the same first message, launch one `rea-recon` agent on swkotor2.exe that answers both questions and writes `.claude/waves/fix2/re.md`:
   - (a) WI-87: which routine destroys and frees a label built by `K2AddrTable::New` + `LabelCtor` with `InitControl(addToList=0)`? Give the address, the convention (thiscall or cdecl), whether it is the vtable scalar-deleting dtor or a raw free, and any registry unlink.
   - (b) SH-3a-08: does the exe destroy or recreate its main game window after startup? List the `CreateWindowExA/W` and `DestroyWindow` call sites and class names, and whether they run on video-mode change or alt-tab.
   Any raw hex it reports must pass `query_ghidra.py address <hex>` before it is used anywhere.
3. Run `wave_tools.py check fix2` and `agent_contract.py check --scope '.claude/waves/fix2/**' <reply>`. Re-run each FAIL/MISSING unit once.
   Gate: at least 2 well-formed proposals per WI.

### FX2.3 Sonnet consolidators, then independent reviewers (about $14.5)
1. Launch 11 `model: sonnet` Agents with `isolation: worktree`, all in one message. Each brief is `.claude/waves/fix2/<WI>/consolidate.md`. Brief rules, as in FX1:
   - Re-verify the defect.
   - Apply and test every proposal.
   - Prove red-on-BASE for each new test.
   - Consolidate in priority order: correctness, then minimal diff, then no shared ABI change.
   - Run the scoped tests, plus `compat_check.py` and `check_shared_headers.py` (check only) when touching `_shared_src`.
   - Revert with `git apply -R`, because `git checkout` is denied in worktrees.
   - Write `final.diff`, `verdict.md` (at most 60 lines) and the 5-line worker reply.
   - Keep context at about 80k, and use `rg` and `sed -n` windows only.
2. As each consolidator returns PASS, launch its **independent reviewer** (`model: sonnet`, fresh worktree, `review.brief.md`).
   The reviewer reads only the pack, `final.diff` and `verdict.md`.
   It applies the diff, re-runs the tests and does a mutation check per new test. It looks for out-of-scope callers, inert code and wrong log text, the three gaps FX1's reviewers caught.
   It writes `review.md` with a verdict of APPROVE, APPROVE_MINOR or REQUEST_CHANGES.
3. On REQUEST_CHANGES, use SendMessage to send the review back to that WI's consolidator for one revision, then re-review. A second REQUEST_CHANGES parks the WI.
   Opus does not review code. It only relays.

### FX2.4 Apply, gate, commit (inline)
Conflict groups (same files), applied in this order:
- **Q**: WI-74 -> WI-82.
- **S**: WI-75 -> WI-83 -> WI-84.
- **A**: WI-77 -> WI-78.
- **C**: WI-80 -> WI-81.
- **Last**: BG29c, after both the S and C groups, because it spans both.
- **Free**: WI-79.

Per item:
1. Run `git apply --check`, then `git apply`. On a conflict, send the consolidator one rebase request against the current HEAD. A second conflict parks the item.
2. Re-run its test ids in the main tree. If it touched `SelfHealUi.h`, also run `compat_check.py` on the resolvable ids.
   The test tier of compat_check ERRORs on 5 unknown plan ids, so run the consumer ids directly, including `cpp:queue_hud_logic_test`, since queue_hud includes SelfHealUi.h.
   Then run `check_shared_headers.py --sync`.
3. Commit with `wi_driver.py close WI-NN --manual --paths <final.diff files> --reason "FX2 Haiku x3 + Sonnet consolidated + reviewed; .claude/waves/fix2/<WI>/"`.
   BG29c is committed with `git commit -- <paths>`.
4. Run `plan_manager.py mark-done BGnn --plan patch_manager_mods/37_bug_board.md --state DONE --commit <sha> --no-handoff`. BG29 changes from PARTIAL to DONE.
   If another session has hunks in doc 37, commit it with the index-blob method.
5. No installs and no version bumps. Record per mod: rebuild + install needed (queue_hud, companion_selfheal, companion_ai, holster, combat_hotbar, and level_up_override if LevelUpUi.h changed).
   WI-74 also needs a companion play check after install, because dispatch behind a resting 0x3D has never been tested in game.

### FX2.5 Close
1. Copy the rea-recon answers into `WI-87.md` (blockers -> answer, then unpark if the routine is known), `WI-101.md` (SH-3a-08) and doc 37 BG40. Commit them.
2. Run `mark-done` for each FX2 phase, then `wi_driver.py lint`, then `audit --apply` (this is the 5th+ item), then `/plan-handoff` last.
3. Memory: add the FX2 result to `kotor2-gemini-review-20261014.md`. Add the yield to `feedback-variant-wave-yield.md`: which variants won, what reviewers caught, and cost per item.
   The FX3 queue to record there: WI-73, 89, 90, 91, 92, 93, 94 (split), 95, 96, 100.
4. Run `agent_usage.py --since <FX2.2 start>` and produce the 4-section Saved State.

## Verification
- FX2.2: `wave_tools.py check fix2` shows 0 FAIL/MISSING after one retry; `agent_contract.py check` shows no repo edits; `re.md` exists and answers (a) and (b), or says UNSURE with the reason.
- FX2.3: 11 `final.diff` + `verdict.md` + `review.md`. Each new test is red on BASE and green after. Every item has a reviewer verdict of APPROVE or APPROVE_MINOR before apply.
- FX2.4: the scoped tests pass in the main tree after each apply. `git show --stat` lists only that item's paths. Other sessions' dirty files stay unstaged. `plan_manager.py resume 37` shows BG27, BG28, BG29 and BG30-BG37 DONE.
- End: all 8 baseline ids pass together at the final HEAD. `check_shared_headers` shows no drift.

## Cost and risks
- Estimate about $16: Haiku 33 x $0.025, Sonnet consolidators 11 x ~$1.05, reviewers 11 x ~$0.30, rea-recon ~$0.5. This is calibrated on FX1's actual costs, not its estimate.
- Shared-file risk: the S group shares CompanionSelfHeal.cpp and SelfHealUi.h across 3 WIs plus BG29c, so expect 1-2 rebase round-trips (FX1 had one).
- All 5 mods need a rebuild and install later (user GO; kotor2-kpm-install skill). No installs, pushes or GitHub posts in FX2.
