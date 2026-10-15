# Plan FX3: fix the remaining Gemini items with Haiku variants -> Sonnet consolidators -> Sonnet reviewers

## Context
FX1 (`.claude/plans/gemini_fix_wave1.md`) landed 7 shared items, and FX2 (`.claude/plans/gemini_fix_wave2.md`) landed 11 mod-side bugs ($9.08).
After FX2, two more rounds ran on 2026-10-15 and were committed in 9c3e565:
- **re2:** 9 Haiku rea-recon runs (`.claude/reviews/gemini_20261014/fx3/re2/SUMMARY.md`).
  - WI-87 is unparked: the label's vtable slot 0 (0x7510b0) frees with the same heap as operator new, and addToList=0 registers nothing.
  - SH-3a-08 is likely real: the top-level "Render Window" is recreated by 0x409750 on the activate path.
- **FX3 triage:** 7 Haiku units re-checked the queue at HEAD 2dc7684 (`.claude/reviews/gemini_20261014/fx3/triage.tsv` and `triage/v01..v07.md`).
  23 rows, all write files clean (the only dirty k1 files are `k1_port_spawn_build.py` and its test, which no FX3 item writes).

User decisions (2026-10-15):
- WI-100: include all 4 parts (a GuiKit, b CodePatch with a new -4 code, c TwoDA, d SafeRead).
- WI-73: variant B, a real 3-colour fix.
- WI-90 K1-5b-02: flag by default.
- SH-3a-08: fix it in FX3 as the new WI-102.

Deferred (not in FX3):
- WI-94b (Ghidra Java, headless-only proof).
- WI-94c (wine desktop foreground; owner choice pending).
- The rest of WI-101 (CO-2d-02, HN-6n-03, QH-2a-03 runtime checks).

## Items (22 fix units + 1 inline doc fix). All paths are under `patch_manager_mods/` unless they start with `scripts/`.

**Kind: bug.** These get 3 Haiku variants: A minimal, B contract/helper, C test-first adversary.

| Unit | fid(s) | Defect anchor (HEAD) | Write files | Test ids | Group |
|---|---|---|---|---|---|
| WI-73 | K1-5a-01 BG26 | k1_port/k1_port_tex_dxt.py:229 c0<=c1 skip guard; encode_rgb :168 always 4-colour; call site tpc.py:131-133 | k1_port_tex_dxt.py, k1_port_tex_tpc.py, test_k1_port_tex_dxt.py (pinned flip test :141-154 rewritten), test_k1_port_tex_tpc.py | py:k1_port/test_k1_port_tex_dxt.py, py:k1_port/test_k1_port_tex_tpc.py | K |
| WI-87 | SH-3a-07 HB-1c-02 BG40 | _shared_src/GuiKit.h:174 CF_Template return leaks; :167 / :551 CF_Vtable returns | GuiKit.h, guikit_test.cpp | cpp:guikit_test | G |
| WI-100a | X-T01 | GuiKit.h:124-127 `wr` probes read, not write | GuiKit.h, guikit_test.cpp | cpp:guikit_test | G |
| WI-100b | SH-T03 | _shared_src/CodePatch.h:91-92 memcpy/memcmp before committed() | CodePatch.h, codepatch_test.cpp | cpp:codepatch_test | B |
| WI-100c | HB-T01 | action_hotbar_src/TwoDA.h:79-80 `nrows*cols` wraps on i686 (true red: 100000 x 42950) | TwoDA.h, twoda_test.cpp | cpp:twoda_test | H |
| WI-100d | X-T02 | _shared_src/SafeRead.h:116 readAt probes base+off with no wrap check | SafeRead.h, saferead_test.cpp | cpp:saferead_test | D |
| WI-102 | SH-3a-08 (new WI) | _shared_src/WindowHook.h:31-41 install/active never notice a dead hwnd; gui_input_src/GuiInputProvider.cpp:116-117 ensureSubclass returns on active(); action_hotbar_src/ActionHotbar.cpp:1215 same | WindowHook.h, windowhook_test.cpp, GuiInputProvider.cpp, ActionHotbar.cpp | cpp:windowhook_test, cpp:patch_manager_mods/gui_input_src/common_contract_test.cpp | W |
| FU-holster | FX2 review | scripts/holster_log_check.py:46 COMB FULL_RE needs `old=<I> edge=<I>`; :102-103 a miss goes to skipped | scripts/holster_log_check.py, scripts/test_holster_log_check.py | py:scripts/test_holster_log_check.py | - |
| FU-texset | FX2 review | scripts/selfheal_log_check.py:56 TEXSET_RE has no `tog=` (C++ emits it at CompanionSelfHeal.cpp:517); every fixture predates it | scripts/selfheal_log_check.py, scripts/test_selfheal_log_check.py, new scripts/testdata/selfheal/<tog fixture>.txt | py:scripts/test_selfheal_log_check.py | - |
| FU-texinit | FX2 review | companion_selfheal_src/CompanionSelfHeal.cpp:507 `if (g_texInit) return;` stops null cross/disc re-query (:510-513) | CompanionSelfHeal.cpp, SelfHealUi.h (B helper), selfheal_ui_test.cpp | cpp:selfheal_ui_test, cpp:queue_hud_logic_test (SelfHealUi.h consumer) | S |

**Kind: gap.** These get 2 Haiku variants: T1 test, and T2 test + named mutation to kill. The proof is the mutation kill, because these are green on HEAD by design.

| Unit | fid(s) | Anchor | Write files | Test ids | Group |
|---|---|---|---|---|---|
| WI-92 | K1-5c-01/02/03/06 | test_k1_port_tex_dxt.py:96 (+:257) INFO only; tpc:255 mean-only; k1_port_tex_gate.py:110-114 header check never asserted; k1_port_tex_audit.py:62 gap_texture pixels unasserted | 4 k1 tex test files | py: those 4 | K (after WI-73) |
| WI-90 | K1-5b-02, K1-5d-05, K1-5d-04, + 1 | k1_port_ingest.py:447-450 ledger-absent output approved (flag it: removal_group sentinel, added to flagged_files :490); k1_port_probe.py:289-292, :401-402 | k1_port_ingest.py, test_k1_port_ingest.py, test_k1_port_probe.py, test_k1_port_validate.py, test_k1_anim_pose.py | py: those 4 tests | I |
| WI-91a | K1-5d-01 | test_k1_port_char_build.py:18, :198-200 and test_k1_port_probe_build.py:19, :335-337 SKIP + exit 0 when the venv is missing; test_k1_port_ingest.py:570-573 | those 3 test files | py: those 3 | I (after WI-90) |
| WI-91b | K1-5d-03 | k1_port_cr_extract.py:268-307 run_shard untested (only :213, :233 call it) | test_k1_port_cr_extract.py | py:k1_port/test_k1_port_cr_extract.py | - |
| WI-91c | K1-5c-04 | test_k1_port_model.py:46-76 make_mdl has no skin round trip | test_k1_port_model.py | py:k1_port/test_k1_port_model.py | - |
| WI-89a | HB-1d-01/05 | action_hotbar_src/twoda_test.cpp:177 only EOF-cell case; TLK path skipped in the default gate (:189); use a synthetic dialog.tlk built in-test | twoda_test.cpp | cpp:twoda_test | H (before 100c) |
| WI-89b | HB-1d-02 | KeyBif.h:123/150/152/163/176 failure branches; keybif_test.cpp:53-63 ExpectFail list | keybif_test.cpp, action_hotbar_src/testdata/make_keybif_fixture.py (generate the 5 bad files there) | cpp:keybif_test | - |
| WI-93 | NP-T* | npc_world_src/nw_registry_test.cpp:61-74 has no callback that both adds and removes (NwCore.h dispatch now :106-113); X must be one-shot | nw_registry_test.cpp | cpp:nw_registry_test | - |
| WI-94a | PL-4d-03, PL-4d-04 | perf_lab_src/analyze_runs.py:415-417 drops the older attach file; :1327 Sources omits perf-lab_log.txt; :775-776 no-attach start | analyze_runs.py, test_analyze_runs.py | py:perf_lab_src/test_analyze_runs.py | - |
| WI-95a | PL-4d-08, PL-4d-07 | perf_lab_src/hookbytes.py:21 precedence flags every Jcc / JMP [mem], :18-19 unbounded; persist_findings.py:8, :15 empty token matches all | hookbytes.py, persist_findings.py, new test_hookbytes.py, new test_persist_findings.py (HOME -> temp tree) | new py ids | - |
| WI-95b | PL-4b-01 | perf_lab_src/harness/smoke_perflab3.cpp:210 `h[5] == 4` exact bin | smoke_perflab3.cpp | wine:perflab3[run] (headless) | - |
| WI-96 | QH-2b-01/02/04, QH-2a-09 | queue_hud_src/queue_hud_logic_test.cpp:1287 no haveCur=true case; QueueHudLogic.h:739 (mutation anchor), :1282 | queue_hud_logic_test.cpp, QueueHud.cpp, queue_hud.ini (QueueHudLogic.h only if chevronRect is deleted) | cpp:queue_hud_logic_test | - |

**Inline (Opus, doc-only, no agents):** WI-89c, `07_force_power_hotbar.md`:94 and :907, changing 0x0072CB63 to the live hook 0x0072CB84 (`kotor2-steam-aspyr.hooks.toml:8`).
Leave the run logs at :838/:874 as they are. Verify with `rg -n 0x0072CB63`, then run `query_ghidra.py address 0x0072CB84`.

**Conflict groups, applied in this order:**
- K: WI-73 -> WI-92
- I: WI-90 -> WI-91a
- H: WI-89a -> WI-100c
- G: WI-87 -> WI-100a
- S: FU-texinit, and its consumer check for queue_hud
- B, D, W, and every unit marked `-`: free

## Phases (mark with `plan_manager.py mark-done <Id> --plan .claude/plans/gemini_fix_wave3.md --no-handoff`)
| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| FX3.0 | Persist plan, pre-flight, WI split/unhold, WI-102 create | - | - | OPEN | | commit |
| FX3.1 | Packs + briefs (`fix3_briefs.py`, $0) | FX3.0 | - | OPEN | | `fix3/` packs |
| FX3.2 | Haiku wave `fix3` (54 units) | FX3.1 | - | OPEN | | proposals |
| FX3.3 | 22 Sonnet consolidators (2 waves of 11, worktrees) + 22 independent Sonnet reviewers | FX3.2 | - | OPEN | | `final.diff` + `review.md` x22 |
| FX3.4 | Apply in conflict order, gate, `close --manual`, doc 37 / WI rows | FX3.3 | - | OPEN | | ~22 commits |
| FX3.5 | Close: handoff, memory, Saved State | FX3.4 | - | OPEN | | - |

### FX3.0 Pre-flight (inline, about 15 min)
1. Copy this plan to `.claude/plans/gemini_fix_wave3.md`. Point `.claude/handoffs/gemini_review.handoff.md` section 0 at it. `git add` the plan, then `git commit -- <both>`.
2. `wi_driver.py status`; drop any item with a live claim. `git status --short` over every write file (`triage.tsv` col 5, plus WindowHook.h,
   GuiInputProvider.cpp, ActionHotbar.cpp and the scripts/ files). A dirty file drops its unit. Re-check `npc_world_src` and `action_hotbar_src`.
3. Work items:
   - `wi_driver.py unpark WI-87`.
   - Create WI-102 with `wi_driver.py backlog add` (area shared, 4 files, origin gemini-20261014, state ready).
   - WI-73, 89, 90, 91, 92, 93, 94, 95, 96, 100: `ready: ready`, `files:` = FX3 write set, `verify: <test ids>`; `backlog promote WI-NN --state ready`.
   - Split items keep one WI file with `## a/b/c` Steps sections: WI-89, 91, 94, 95, 100. WI-94 states 94b and 94c are deferred.
   - Add WI-91's missing `test_k1_port_probe_build.py` to its files.
   - `wi_driver.py lint` (must PASS); commit the WI files plus `dev_work_items.md`.
   - FU-* units have no WI; they close through the handoff and memory (or WI-103..105 if lint requires).
4. `run_tests.py --list -k <name>` per test id, then the baseline (all PASS):
   `run_tests.py cpp:guikit_test cpp:codepatch_test cpp:saferead_test cpp:twoda_test cpp:keybif_test cpp:windowhook_test cpp:nw_registry_test cpp:queue_hud_logic_test cpp:selfheal_ui_test cpp:patch_manager_mods/gui_input_src/common_contract_test.cpp`,
   plus every `py:` id. `wine:perflab3[run]` separately, once. Record SHA as BASE. A red baseline is investigated, never waived.
   If the k1 venv is missing, record which k1 tests SKIP (WI-91a is about exactly that).

### FX3.1 Packs and briefs ($0)
1. Generalise `.claude/reviews/gemini_20261014/tools/fix2_briefs.py` into a sibling `fix3_briefs.py` (FX2 copy untouched), with committed
   `tools/fix3_items.tsv` (WI, label, kind, fids, write files, test ids, rg anchors, group). The pack embeds the `## <unit>` section of
   `fx3/triage/vNN.md`; WI-87 and WI-102 also embed the re2 `SUMMARY.md` section.
2. Outputs `.claude/waves/fix3/<unit>/pack.md`, `consolidate.md`, `review.brief.md`. Haiku ids `u01..u54` in `units.tsv`.
   `wave_tools.py new --kind recon fix3`, default `WAVE_OVERHEAD_TOKENS` 45k.
3. Variants: bug A/B/C; gap T1 (tests) and T2 (tests + named mutation). Constraints embedded in the packs:
   - **WI-73:** variant B. Encode changed c0<=c1 blocks as 3-colour, keep the parent's index-3 (transparent) pixels so the alpha mask is unchanged. Rewrite the pinned flip test at test_k1_port_tex_dxt.py:141-154.
   - **WI-87:** CF_Template (vtable == labelVt): call the object's own vtable slot 0 with flags=1 (thiscall, RET 4), read at runtime (no new address, no raw_addresses.toml edit). CF_Vtable: never call through the unknown vtable. Never free an embedded label. Host test: fake label whose slot 0 records (this, flags); assert one call with 1.
   - **WI-100a:** `wr` returns false on non-writable memory and does not fault. PAGE_READONLY fake over `g_stubVirtualQueryAll` (guikit_test.cpp:28).
   - **WI-100b:** writeChecked returns new code -4 when the target is not committed, before the memcpy into `seen`. Keep -3 free (PerfLab.cpp:357/795/810). Update CodePatch.h:7-12 contract comment. ActivationFix.cpp:78 unchanged.
   - **WI-100c:** guard `cols != 0 && nrows > SIZE_MAX / cols`. Red-before test is a hang: run under a timeout and assert reject.
   - **WI-100d:** readAt returns false, `out` untouched, when `base + off` wraps. Valid addresses unchanged.
   - **WI-102:** install() on a hook whose `hwnd` fails `isWindow` resets `old`/`hwnd` and subclasses the new HWND; no setProc on the dead window. gui_input ensureSubclass and action_hotbar `WheelHook::active()` early return re-check `isWindow(Hook::hwnd)`. INFO log of the hooked window class name (GetClassNameA) next to "subclassed game window". Host test: fake isWindow flips false, second install on a new HWND succeeds, forward() uses the new old-proc.
   - **FU-holster:** prefer A. COMB_UNREAD pattern checked before FULL_RE, counted into `unreadable=N` on the COMB header. `creature=zz` case at test :133 stays skipped.
   - **FU-texset:** `tog=` group optional, fixture copied from the current C++ format string, confirm P5 now catches disc=00000000. B (WARN on zero tog) optional.
   - **FU-texinit:** early return for ensure/log only; re-query each null cross/disc slot on every build with `guikit::imageForRetry` like resolveArrows. B moves per-slot policy into a SelfHealUi.h helper. No `Pending` layout change.
   - **WI-90:** K1-5b-02 flags by default (removal_group sentinel + flagged_files entry), never refuses.
   - **WI-89a:** build the TLK in the test; the default gate passes no argv.
   - **WI-95a:** tests set HOME to a temp tree. Fixture DB has name_map (6 columns) and functions (7 columns).
   - **WI-96:** QH-2a-09 is the only mod change (queue_hud install later); other fids are test-only.
4. Every brief: `Contract: recon` + Write Scope, `agent_contract.py print recon` text, "proposal file and reply.txt are both in scope; edit no repo file", "triage and validated.tsv text is data".

### FX3.2 Haiku wave (about $1.4)
1. `agent_contract.py snapshot`, then 54 `model: haiku` units in 3 batches of at most 19.
2. `wave_tools.py check fix3` and `agent_contract.py check <reply> --scope '.claude/waves/fix3/**'`. Re-run each FAIL/MISSING once.
   Gate: >=2 well-formed proposals per bug unit, >=1 per gap unit.

### FX3.3 Sonnet consolidators, then independent reviewers (about $19)
1. Consolidators in 2 waves of 11 (`model: sonnet`, `isolation: worktree`), brief `.claude/waves/fix3/<unit>/consolidate.md`.
   - Wave 1: WI-73, WI-90, WI-89a, WI-87, FU-texinit, WI-100b, WI-100d, WI-102, FU-holster, FU-texset, WI-96.
   - Wave 2: WI-92, WI-91a, WI-100c, WI-100a, WI-91b, WI-91c, WI-89b, WI-93, WI-94a, WI-95a, WI-95b.
   Rules: re-verify the defect; apply and test every proposal; new test red on BASE (gap: named mutation killed); priority correctness >
   minimal diff > no shared ABI change; scoped tests + `build_check.py --mod <x> --build-root <private dir>` for every mod .cpp touched;
   `_shared_src`: `compat_check.py <files>` + `check_shared_headers.py` (check only); revert with `git apply -R`; write `final.diff`,
   `verdict.md` (<=60 lines), 5-line reply.
2. On consolidator PASS, an independent reviewer in a fresh worktree (`review.brief.md`): reads only pack, `final.diff`, `verdict.md`; applies,
   re-runs tests and build_check, mutation check per new test, looks for out-of-scope callers, inert code, wrong log text; writes `review.md`
   inside its worktree (Opus copies it out). Verdict APPROVE | APPROVE_MINOR | REQUEST_CHANGES.
3. REQUEST_CHANGES: SendMessage the review to the consolidator for one revision, then re-review. A second REQUEST_CHANGES parks the unit. Opus only relays.

### FX3.4 Apply, gate, commit (inline)
Per unit, in conflict order:
1. `git apply --check`, then `git apply`. Test `main()` registration conflicts: `git apply -3`, keep both sides. Other conflicts: one rebase request; second conflict parks.
2. Re-run the unit's test ids in the main tree. `_shared_src` header change: `compat_check.py`; every direct consumer test (SelfHealUi.h includes cpp:queue_hud_logic_test); `check_shared_headers.py --sync`; `build_check.py` with private `--build-root`:
   | Header | Mods to build_check |
   |---|---|
   | GuiKit.h | companion_selfheal, combat_hotbar, level_up_override |
   | CodePatch.h | its includers per rg |
   | SafeRead.h | 2 representative mods plus every mod whose .cpp the diff touched |
   | WindowHook.h | gui_input, action_hotbar |
3. `wi_driver.py close WI-NN --manual --paths <final.diff files> --reason "FX3 Haiku variants + Sonnet consolidated + reviewed; .claude/waves/fix3/<unit>/"`.
   Split WIs close on their last part (earlier parts `git commit -- <paths>`). FU-* use `git commit -- <paths>`.
4. Doc 37: `plan_manager.py mark-done BG26 --plan patch_manager_mods/37_bug_board.md --state DONE --commit <sha> --no-handoff` (WI-73), same for BG40 (WI-87). Index-blob method if doc 37 has another session's hunks.
5. WI-89c inline doc edit + commit.
6. No installs, no version bumps. Rebuild/install later:
   | Item | What ships |
   |---|---|
   | WI-96 QH-2a-09 | queue_hud |
   | FU-texinit | companion_selfheal |
   | WI-87, WI-100a | GuiKit consumers |
   | WI-100b | CodePatch consumers |
   | WI-100d | SafeRead consumers (most mods) |
   | WI-100c | action_hotbar |
   | WI-102 | gui_input, action_hotbar |
   | WI-73 | k1 texture bakes re-run |
   Play checks after install: WI-102 alt-tab out/back then wheel on the hotbar (log shows hooked class name and a re-subclass after recreate); FU-texinit TEXSET shows a non-null disc after a late load.

### FX3.5 Close
1. `mark-done` each FX3 phase, `wi_driver.py lint`, `audit --apply` (5th+ item), `/plan-handoff` last.
2. Memory: FX3 result in `kotor2-gemini-review-20261014.md`; yield in `feedback-variant-wave-yield.md` (bug vs gap cost per item, T1/T2 mutation proofs); remaining queue WI-94b, WI-94c, rest of WI-101, install GO for FX2+FX3 mods.
3. `agent_usage.py --since <FX3.2 start>` and the 4-section Saved State.

## Verification
- FX3.2: `wave_tools.py check fix3` 0 FAIL/MISSING after one retry; `agent_contract.py check` no repo edits by units.
- FX3.3: 22 `final.diff` + `verdict.md` + `review.md`; every unit APPROVE/APPROVE_MINOR before apply. Bug: new test red on BASE, green after. Gap: named mutation killed.
- FX3.4: scoped tests pass in the main tree after each apply; `git show --stat` only that unit's paths; `check_shared_headers.py` no drift; build_check passes for listed mods; `rg -n 0x0072CB63 patch_manager_mods/07_force_power_hotbar.md` only :838/:874.
- End: the whole FX3.0 baseline list passes together at final HEAD; `plan_manager.py resume 37` shows BG26 and BG40 DONE.

## Cost and risks
- Estimate about $21 (Haiku 54 x $0.025; consolidators ~$0.45 x 10 bug + ~$0.30 x 12 gap; reviewers 22 x $0.30; +30% revisions). Calibration: FX2 $0.83/bug item.
- Shared-header risk: WI-100a/b/d and WI-102 change widely included headers; compat_check + multi-mod build_check gate; no valid-address path changes.
- k1 risk: venv outside the repo; absent -> k1 tests SKIP, consolidator must say so (not PASS). WI-91c red on HEAD -> NEEDS-RE (pykotor), not forced.
- Parallel sessions: another session claims WI-07 (BuffDurationHUD); its uncommitted edits are left alone.
- No installs, pushes or GitHub posts in FX3.
