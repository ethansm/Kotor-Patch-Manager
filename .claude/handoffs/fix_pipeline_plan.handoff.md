# Handoff: fix pipeline (verified findings -> tested patch series), Phase A

Plan: `.claude/plans/fix_pipeline_plan.md` (queue numbers, isolation design, stages, costs, Phase B launch sequence,
5 open questions). Contract: `.claude/review/INTERFACES.md` last section "## Fix stage". Session date 2026-10-10.
Branch `KOTOR-II`, HEAD = pinned 22b56b427125e12bb7b2914d96b820973d06c445 (real tree dirty from other sessions; never touch it).
Production ledger sha256 MUST stay `73c49d59df5103c4b33070ac95f549f2e595a1d3f2c6c029e264e7aba594cf8f`.

## State
- Phase A tooling built (all new, uncommitted; nothing committed this session): `.claude/review/fix/` {fcommon.py,
  fix_intake.py, fix_workspace.py, fix_settings.py, fix_checks.py, fixgate.sh, fix_prompts.py, record_fix.py, run_fix.sh,
  run_fix_review.sh, fix_calibrate.py, build_integration_batches.py, run_integrate.sh, fix_merge.py, fixer_prompt.md,
  reviewer_prompt.md, integrator_prompt.md, fix_canary_probe.md, fix_schema.json, review_schema.json,
  integrate_schema.json, specialty_notes/{fixer,reviewer}/*.md (6 each)}, `.claude/review/tools/fix_canary.sh`,
  `.claude/review/tools/stub_claude/claude` (python stub). INTERFACES.md section appended. Plan written.
- Real outputs written: `.claude/review/work/fix/{fix_queue.json, human_list.json, refuted_skim.json,
  refuted_skim_pass2.json, skim_pass2/batch_{1..4}.{in,out}.json}`, `.claude/reviews/refuted_skim_pass2.md`.
- All 10 python `--self-test` PASS; `bash -n` on all .sh OK. Haiku spot-check of queue: PASS.
- Queue: 162 findings -> 135 tasks: auto-fixable 86, needs-review 4, needs-design 41, human-only 4; 22 contested;
  REFUTED skim 30 -> Sonnet pass 2: refute-holds 21, design-risk 8, real 1 (f9d03b5a7018 equip3d animLen fallback).
- Pilot tasks: Td18cc50320 (queue_hud export/H10, hook-asm), T32db00261f (compat_check run_build_mods rc),
  T49ca9cb835 (generate_diversity_2da backup), Tc307b197f4 (gui_input ensureSubclass), T106b5fe355 (test_compat_check).

## Phase A status (2026-10-10, session 2): COMPLETE, awaiting user "go" for Phase B
- e2e #4 (clean scratch, bwrap + stub, after fixes 6-8): all expectations met. .py pilot tasks T32db00261f,
  T49ca9cb835, T106b5fe355 -> check-ok -> review-accept -> batch_01 (scripts) -> integrate verify OK -> fixed+tested.
  .cpp/.def (Tc307b197f4, Td18cc50320) -> build fail a1+a2 -> human-only. Ledger 73c49d59... unchanged.
  Answer-key isolation rg on prompts: clean. All 10 self-tests PASS (fix_workspace 47, fix_checks 24 checks).
- Ledger incident resolved: user said delete; the 15:50:55Z SESSION_HANDOFF line removed, hash back to 73c49d59...
- Peer session (kotor-ii-fe) note: calibration controls K1 3f4afd470da1 + K3b 4aeb045632f0 are final=confirmed but
  absent from every tier, so not in fix_queue. Corrected ledger at .claude/reviews/verified_findings.v2.json (only
  adds 7 controls). OPEN for user: re-run fix_intake against v2 (137 tasks) or keep the 135-task queue.

## Bugs found + fixed this session (do not re-introduce)
1. `fix_settings.py bwrap-argv` prints args WITHOUT the `bwrap` program name. run_fix.sh, run_fix_review.sh and
   tools/fix_canary.sh now prepend `bwrap` when missing (run_integrate.sh already did). Before the fix, variant A of
   the real canary would always fail and silently fall back to B.
2. run_integrate.sh read the AUDIT canary (`.claude/review/canary.json`, env FIX_CANARY). Now `fix/canary.json`,
   env `FIX_CANARY_JSON` like the other runners.
3. fix_canary.sh did not write top-level `integrator_ok` (run_integrate requires it). Added (all `integ_*` checks of
   the chosen variant).
4. Stub claude located the "real repo" as parents[4] of itself; a copy outside the repo refused every cwd. Falls back
   to STUB_REAL_REPO / ~/orca/workspaces/local-setup/KOTOR-II now.
5. run_integrate.sh did not install bin/fixgate_<batch>.sh (now `$WS_CMD install-bin --batches <b>`).
6. fix_workspace.make_writable chmod'ed hardlinked .git objects shared with base (clone --local): base objects became
   writable AND a concurrent slot clone died with "hardlink different from source" (git TOCTOU check) -> T106b5fe355
   stuck at needs-check (this was the 1-of-5 mystery). Now skips st_nlink>1 files; self-test asserts base objects stay 0444.
7. Per-task build gate passed ANY still-failing build when the mod fails on clean base (queue_hud_src fails at the pin:
   missing QueueHudPerformCastSpell = H10), while verify-series failed it -> broken patch blocked a batch. Both now use
   fix_checks.baseline_build + new_build_errors (normalized diagnostic-line diff): pass only if no NEW diagnostics.
   Baseline cache build_<mod>.json has "v": 2 (6000-char tail).
8. `basename "${BW[0]}"` with BW[0]=--ro-bind parsed as an option (noisy; bwrap prefix correct by accident): now `basename --`.

## Dead ends / traps (Do NOT attempt)
- ❌ Stub on PATH from inside the repo or /tmp under FIX_SANDBOX=bwrap: bwrap tmpfs-hides ~/orca and /tmp. Put the
  stub in `~/.cache/fix_e2e_bin/claude` (copy of tools/stub_claude/claude).
- ❌ `git worktree` for slots/integ: .git points into base/.git, hidden in the sandbox. Clones (`git clone --local`) are used.
- ❌ `plan_manager.py handoff` resolves this work to `10_equipment_3d_character` (wrong plan); use this file.
- ❌ Global Edit/Write deny in settings: deny beats allow; dontAsk + per-file allow is the design.
- Known accepted deviations: assertion gate allows a rewritten (stricter) assertion (reviewer covers loosening);
  relocate flag has 1 false positive (Tf482582269, cites a #define); `.venv` unittest items SKIP in gates.

## Verification commands
```bash
cd ~/orca/workspaces/local-setup/KOTOR-II/.claude/review/fix
for s in fcommon fix_intake fix_workspace fix_settings fix_checks fix_prompts record_fix fix_calibrate build_integration_batches fix_merge; do python3 -I $s.py --self-test | tail -1; done
sha256sum ../../master_ledger.jsonl   # 73c49d59...
# e2e stub driver: env KOTOR2_REVIEW_HOME=<scratch>/rh (rh/worktree -> symlink to ~/.cache/kotor2_review/worktree)
#   FIX_DIR FIX_WORK KOTOR2_FIX_OUT KOTOR2_FIX_REPORT FIX_CANARY_JSON=<scratch>/... FIX_SANDBOX=bwrap
#   PATH=~/.cache/fix_e2e_bin:$PATH ; steps: fix_intake, fix_workspace build-base, verify-base,
#   tools/fix_canary.sh --stub, STUB_MODE=fix-good run_fix.sh --only <pilot> --class auto-fixable,needs-review,needs-design,
#   fix_calibrate.py, STUB_MODE=review-accept run_fix_review.sh --only <pilot>, build_integration_batches.py,
#   STUB_MODE=integrate-keep run_integrate.sh, fix_merge.py, ledger hash compare.
```
Expected with stub fix-good (appends "# stub-fix" line): .py tasks pass gates; .cpp/.def tasks should FAIL build
-> a2 retry -> fail -> human list (that is the gate working).

## User decisions (2026-10-10, session 2) -- AUTHORIZED: Phase B pilot "go"
1. needs-design -> Sonnet fixer (as planned). 2. gate-integrity -> fix + flag each for user sign-off.
3. known-good calibration patches -> yes (DONE, see below). 4. integrator resolves conflicts by Edit in its scratch.
5. final-apply branch name -> NOT ANSWERED; ask before any apply to the real tree. 6. intake from v2 -> DONE.
"go" covers: real build-base, real canary, pilot (plan section 6 steps 0-9). Full run needs a second "go" after the
pilot report is reviewed (plan says "after you review the pilot report").

## Session 2 work after the decisions (all uncommitted, untracked)
- Queue rebuilt: `FIX_FINDINGS=.claude/reviews/verified_findings.v2.json python3 -I .claude/review/fix/fix_intake.py`
  -> 137 tasks (needs-design 42, needs-review 4, auto-fixable 86, human-only 5); refuted skim 33. Added vs v1:
  T3444e1c047 (needs-design, test-weakening, tests/static/subclassers.toml = K1/K3b area) and T004976d3df (human-only,
  scripts/pre_install_gate.py). Nothing removed; pilot ids unchanged. v1 queue backup was in scratch only (gone).
  NOTE: default FIX_FINDINGS is still v1; any re-run of intake MUST set FIX_FINDINGS=...v2.json.
- Known-good calibration patches written (git diff against the fix base, a8ea0fb-equivalent content):
  `.claude/review/work/fix/calib_good/{T83b3a638ce,Tb9d95b8029,T6dbdac1f61}.patch`
  (subagent_prompt.py regex hpp-before-h + \b; query_ghidra.py struct no silent --limit truncation + warning;
  normalize_diversity_textures.py drop stale .tga/.tpc sibling before copy). `fix_calibrate.py build --dry-run`
  plans 8 items (5 seeded-bad + 3 good). Not yet `git apply --check`ed against the real base (built in Phase B step 1).
- Plan section 6 calibrate step corrected to `fix_calibrate.py prepare && fix_calibrate.py queue`.

## Next steps
1. Phase B pilot (authorized): plan section 6 steps 1-9 with the v2 queue; inspect canary + pilot report.
2. Report pilot results + real cost; ask for the full-run "go" and the branch name (Q5).

## Pilot result (Phase B, 2026-10-10 session 3) -- STOPPED for full-run "go"
- Canary (real): variant **A**, pass, all 16 checks true, integrator_ok. First run fell to B (bug 9 below).
- Calibration: catch rate **5/5** (assert-delete, no-op-fix, off-by-one, swallow-error, unrelated-file), false-reject **0/3**, 0 fabricated quotes.
- Per task: T32db00261f fixed+tested (batch_02; gate-integrity -> needs USER SIGN-OFF) | Td18cc50320 fixed+needs-live-check
  (batch_01; fixer chose REMOVE H10 export+hook; queue_hud_src now builds; user decides implement vs remove) |
  T49ca9cb835 rejected (still revise after 1 loop; a3 issues all minor: rollback not guarded, kill window) |
  Tc307b197f4 not-fixable (fix changes GuiInput.h:49 subscribe contract) | T106b5fe355 not-fixable (needs compat_check.py edit).
- Patches: `.claude/fixes/batch_01/0001-Td18cc50320.patch`, `.claude/fixes/batch_02/0001-T32db00261f.patch` (host verify_ok true).
- Real cost (CLI-reported): fix $0.242 (incl revise a3), review $0.043 (11 items + 1), integrate ~$0.30 over 3 runs
  (fcommon spent shows $0.176, last 2 runs), canary <$0.01. Total ~$0.59.
- Projection 137 tasks: per-call ~$0.06 Sonnet (needs-design 42) / ~$0.005 Haiku (90) -> fixers ~$3-4 with retries,
  review ~$0.5, integration ~$0.09/batch (~$1.5-2.5) => **~$5-7**. Naive pilot $/task x 137 = ~$16 (upper bound).
- Ledger 73c49d59... before/after every step; prompts rg clean; real tree only gained `.claude/fixes/`.

### Bugs found + fixed in the pilot (stub e2e could not catch: stub ignores --settings and never runs gates)
9. fix_settings.bwrap_argv(claude) hid WORK/settings (under ~/orca or RHOME) -> "Settings file not found" on every real
   call. Now `--ro-bind WORK/settings` after the hides; self-test check added (127 PASS). Also fix_canary.sh `basename --`.
9b. run_fix_review.sh:110 bwrap-argv used FIX_WORK=WORK but reviewer settings are in WORK/review_ws/settings ->
   now `FIX_WORK="$RWS" $SET bwrap-argv`.
10. run_integrate.sh: integrator sandbox did not bind $FIX_HOME/bin -> fixgate wrapper "not found". Now `--ro-bind bin`.
11. run_integrate.sh created integ/<batch> with `git worktree add` (the known trap) -> fixgate "not a git clone".
   Now `$WS_CMD integ-reset <batch>` (clone), removing old worktree dirs first.

### OPEN (not fixed; fix before the full run)
12. Inner gate build SKIPs: nested fixgate bwrap computed inside the claude sandbox cannot see KPM_DIR -> integrator build
   gate skipped (host verify-series still builds). Bind KPM dir ro in the claude profile or pass KPM_DIR through.
13. `fixgate reset` (clean -fdx) deletes the untracked `_patches/` dir that run_integrate stages in integ/<batch>; the
   integrator re-typed the patch from the dossier (risky; sha in series.json + host verify caught nothing wrong). Stage
   patches outside the clone (ro bind) or exclude `_patches/` from clean (`-e _patches`).
14. Baseline gate noise: queue_hud_src test fails on clean base (common_contract_test.cpp missing KpLog.h include);
   scripts/test_compat_check.py times out on base. Both pre-existing; gate already baseline-aware for build only.
15. fcommon.py `spent integ` -> KeyError (stage name is `integrate`); run_integrate dry-run text still says "worktree add".

## Phase C direction (user, 2026-10-10 session 3, after pilot) -- RESEARCH FIRST, then bugs, then full-run go
User end goal: ONE skill/command run on "changes to review" -> /deep-haiku-review breaks work into smallest units for a
Haiku army (review, audit, verify) -> verified issue list -> Haiku 5.5 army runs THIS fix pipeline -> user gets a summary
of found + fixed, with critical/risky items escalated to the human.
Pivot: research a "Haiku-first escalation ladder" before the full run. Current ladder (as built): intake class picks the
model once (auto-fixable/needs-review -> Haiku $0.15 cap; needs-design -> Sonnet $0.60; human-only -> human); a1 -> gates
-> a2 on gate fail; build fail x2 or not-fixable -> human; Haiku reviewer accept/revise/reject, 1 revise loop, still
revise -> rejected -> human; Haiku integrator keep/drop + host verify-series; human gets live-check, gate-integrity
sign-offs, not-fixable/rejected/human-only, per-batch apply approval. No step UP to Sonnet anywhere inside the pipeline.
Proposed ladder to research/prototype:
 1. N parallel Haiku fixer variants on needs-design (prompt styles: minimal / defensive-error-path / test-first-adversarial).
 2. Haiku judge picks or merges among GATE-PASSING candidates only.
 3. Reviewer gets 2-3 revise rounds (pilot: one revise round incl. re-review ~ $0.05, mostly the fix call).
 4. Sonnet only as rescue: all Haiku variants fail gates, or still revise after N rounds.
 5. Human last: contract/scope changes, live checks, sign-offs.
 Cost: 3 Haiku variants + judge ~ $0.02 vs one Sonnet call ~ $0.06 (pilot: Sonnet fix ~$0.06, Haiku fix ~$0.005,
 Haiku review ~$0.003 => ~12x).
Caveats (must be designed in):
 - Scope refusals are not capability failures (Tc307b197f4 needs GuiInput.h contract change; T106b5fe355 needs
   compat_check.py edit). Need a separate "needs-wider-scope" outcome -> re-scoped task (expanded allowed_files) or human,
   never "escalate to bigger model".
 - Every new Haiku decision layer (judge/merger) needs its own seeded-bad + known-good calibration before its picks are
   trusted (reviewer: 5/5 caught, 0/3 false reject is what makes cheap rounds safe).
Pilot data for the experiment: needs-design tasks with Sonnet a1 outputs exist (Td18cc50320, T32db00261f, T49ca9cb835 in
.claude/review/fix/results + patches/), usable as a Sonnet baseline to compare Haiku variants against.

## Ledger note (2026-10-10 17:33:19Z) -- NOT caused by this work
A concurrent session (deep-haiku-review) ran plan_manager.py handoff and appended line 341
(SESSION_HANDOFF, notes "deep-haiku-review phase2: hardening H1-H7 + Haiku escalation ladder (Phase 2b) + sub-agent probe,
then promote skill (Phase 3), fix half (Phase 5)"). Hash now 12efcbc07dc2537e... (341 lines). Left in place; ask the user
whether to delete it (as with the 15:50:55Z line) or adopt the new hash as L0. OVERLAP: that session plans its own
"Haiku escalation ladder (Phase 2b)" + "fix half (Phase 5)" -> coordinate before building ladder code twice.
Its handoff: check .claude/handoffs/ for the deep-haiku-review handoff (memory: kotor2-deep-haiku-review).

## Phase C result (2026-10-10 session 4) -- DONE, STOPPED for user decisions + full-run "go"
L0 = 12efcbc07dc2537e6b7bae4b0387304f76afb22f92d3f6efb97262be7324cbcb (341 lines), unchanged before/after every stage.
Phase C spend ~$1.58 of $2 (cumulative spent(): fix 1.359, review 0.239, integ 0.171, judge 0.037).
Report: `.claude/reviews/ladder_experiment.md` (table, costs, decision, spot-checks).
- Ladder vs Sonnet-first on 12 needs-design parents: BOTH 8/12 accepted, the SAME 8. Cost/parent Sonnet $0.098,
  ladder $0.028, ladder+Sonnet rescue $0.063 = 64.6% -> strict rule (<=60%) FAILS by 4.6 pts; rescue bought 0 accepts.
  Strict decision: Sonnet stays needs-design fixer; ladder = a2 retry for auto-fixable/needs-review gate fails.
  OPEN USER CALL: adopt ladder anyway with rescue only for notfixed/gate-fail residue (~45-50%), revise-stuck -> human.
- Judge: calib 3/3 + none 1/1; live 6/6 picked an oracle-accepted variant; decides from diffs (0 reads).
- Revise rounds: round 1 converted 2/4; rounds 2-3 converted 0 -> use FIX_REVISE_ROUNDS=2 max.
- Styles: defensive best (5/10 accepted), test-first 4/9, minimal 3/8.
- needs-wider-scope: 0/2 real routing (Tc307b Haiku variant edited GuiInputProvider.cpp, reviewer accepted but flagged
  caller behaviour change = a silent contract change Sonnet refused; T106b refused as test design). Prompt rule too weak.
### Bugs found + fixed in Phase C (do not re-introduce)
12 run_integrate.sh binds KPM dir into the claude bwrap (nested gate re-binds it) -> real `build queue_hud_src` PASS.
13 fixgate.sh reset: `git clean -fdxq -e _patches -e _resolved`; integ clone writes both to .git/info/exclude.
14 `fix_checks.py baseline <batch>` + dossier "Appendix: clean-base gate results" (builds computed+cached; tests cache-only).
15 spent('integ') alias; dry-run text says integ-reset; "Not complete" only when non-empty.
16 install_bin writes atomically (tmp + os.replace, skip identical): parallel batches rewrote a RUNNING fixgate.sh in place
   (bash reads incrementally; bwrap ro-bind pins inode) -> "hon3: command not found".
17 fix_intake.py: any arg except --stats/--self-test exits 2 (a sub-agent's `--help` REWROTE the real queue from v1, 135
   tasks; restored with v2 -> 137); fcommon FINDINGS defaults to verified_findings.v2.json.
18 fix_ladder LADDER/LADDER_WS resolve from the real work dir even when FIX_WORK=ladder_ws.
Stub: fix-good appends "# stub-fix <style>" for variants; modes judge-pick-a, fix-scope-request; ~/.cache/fix_e2e_bin/claude
must be re-synced (cp) from .claude/review/tools/stub_claude/claude after stub edits (it is a COPY).
### New tooling (untracked, .claude/review/fix/)
fix_ladder.py (sample|variants|status|promote|judge-queue|judge-prompt|record-judge|judge-copy|judge-calib build/score|
judge-spent), run_fix_judge.sh (--only, --calib, --dry-run; FIX_WORK must be ladder_ws), judge_prompt.md, judge_schema.json,
specialty_notes/style/{minimal,defensive,test-first}.md, record_fix FIX_REVISE_ROUNDS (default 1), fix_merge
needs-wider-scope outcome, fix_schema scope_request. Self-tests: 11 suites PASS (fix_workspace 50, fix_checks 24, fix_settings 127).
Ladder state: .claude/review/work/fix/ladder/{variants.json (38 variants, 14 parents), judge_key/}, ladder_ws/ (queue,
prompts), $FIX_DIR/judge/<parent>.json. Logs: .claude/review/work/fix/logs/{integrate4,canary_c3,judge_calib1,c4_*}.log.
### Lessons for deep-haiku-review Phase 5 port
FIX_WORK isolation trick (variants = derived tasks in a merged queue in a sibling work dir; runners unchanged);
one real canary per sandbox change (stub ignores --settings); atomic installs of anything a sandbox binds; never let a
sub-agent run a tool with side effects without the scratch env exported first; bugs 9-18 above.
### Full-run launch sequence (after "go"; config per user's ladder decision)
1. C0 preflight (ledger, PATH, self-tests). 2. run_fix.sh by class in small waves (FIX_TOTAL_BUDGET_USD per wave):
auto-fixable/needs-review Haiku; needs-design Sonnet (or ladder: variants -> run_fix ladder_ws -> judge -> review picks ->
FIX_REVISE_ROUNDS=2 -> fix_ladder.py promote <parent> for winners -> Sonnet rescue residue in small batches).
3. run_fix_review.sh (FIX_CALIB=1 once) 4. build_integration_batches.py 5. run_integrate.sh 6. fix_merge.py -> human list.
Already done for the 12 sample parents: Sonnet a1/a3 + reviews (normal queue), ladder variants + judge + reviews.
### Questions for the user (unanswered)
(a) full-run go + ladder config (strict = Sonnet; or ladder w/ limited rescue) + ceiling; (b) final-apply branch name;
(c) sign-off T32db00261f patch (gate-integrity; batch_02 test gate timed out on base too); (d) Td18cc50320 H10 remove vs
implement; (e) Tc307b197f4: accept Haiku's return -1 change (caller behaviour) or keep as human/contract item.

## Ledger note (2026-10-10 19:24:40Z) -- NOT caused by this work, between runs
Another session's agent_contract.py check appended line 342 (CONTRACT_CHECK FAIL worker) after Phase C's last pipeline
stage. New hash 3469a4965b79a8dc89544f12acae69e3123c5944a4313c5a9d16df973a33d179 (342 lines). Left in place. Next session: confirm with the user and adopt it as L0 before any run.

## Phase D (2026-10-10 session 5) -- IN PROGRESS
D0: 11/11 self-tests PASS; claude=~/.local/bin/claude; no stub PATH/env; check_kpm_updates silent. HEAD 317e36f (other sessions).
Ledger: 345 lines; lines 1-342 hash == L0 3469a496...; 343-345 = other session's CONTRACT_CHECK. USER DECISION: PREFIX RULE
(L0 = 342-line prefix hash; appended CONTRACT_CHECK lines allowed; runners hash at start and abort on mid-run change).
Checker: head -342 ledger | sha256sum == L0 and tail -n +343 has only "event":"CONTRACT_CHECK".
S0 (cumulative spent at D0): fix 1.358738 / review 0.238593 / integ 0.170533 / judge 0.036572 (sum 1.804436). Ceiling: new <= $6.
D1 DONE: fix_ladder.promote zeroes cost_usd in copied JSON (keeps promoted_cost_usd) + copies TASKS/<pick>.aN.pre.json ->
<parent>.a1.pre.json; record_fix `info` prints 4th field style; run_fix.sh model_for(cls, style): style => FIX_LADDER_MODEL
(haiku) / FIX_LADDER_BUDGET_USD (0.15). Self-tests PASS; dry-run 30 vtids = $4.5 worst case (Haiku guard works).
D2 DONE: all 88 auto-fixable/needs-review run (waves 1..10, logs work/fix/logs/d2_<w>.log), 0 errs, 0 TRIPWIRE; pending=0.
  Queue outcomes now: check-ok 75, notfixed 27, none 35 (=30 needs-design + 5 human-only). Wave 1: 26 ok / 7 notfixed
  (4 design/gate-semantics refusals, 2 out-of-scope, 1 live-tree test). fix spent 1.661134 after D2 (+0.302).
D3 IN PROGRESS: P30 parents in work/fix/ladder/phaseD_p30.txt, 90 vtids in phaseD_p30_vtids.txt; variants created (128 over 44
  parents incl. Phase C). TRAP (bug 19): FIX_WORK MUST BE ABSOLUTE -- relative FIX_WORK made all 30 first-batch calls fail at
  $0 (prompts not found after cd into task copy); cleaned their a1 artifacts (fw.rmtree for read-only copies).
  Running: FIX_WORK=$PWD/work/fix/ladder_ws FIX_DESIGN_MODEL=claude-haiku-5-5 FIX_DESIGN_BUDGET_USD=0.15
  FIX_TOTAL_BUDGET_USD=4.06 run_fix.sh --only <90 vtids> --class needs-design  (log work/fix/logs/d3_fix.log).
  Caps (cumulative): fix 4.06 (S0+2.7 for D2+D3+D4), review S0+0.8=1.0386, integ S0+1.6=1.7705, judge default 0.5.
  NEXT (run from .claude/review, W=$PWD/work/fix absolute): check d3_fix.log (failed cost=None => diagnose) ->
  FIX_WORK=$W/ladder_ws bash fix/run_fix_judge.sh --only "$(FIX_WORK=$W/ladder_ws python3 -I fix/fix_ladder.py judge-queue --only $(cat $W/ladder/phaseD_p30.txt))"
  -> picks from `FIX_WORK=$W python3 -I fix/fix_ladder.py status --only <P30> --json` parents[].pick_vtid ->
  FIX_WORK=$W/ladder_ws FIX_CALIB=0 FIX_REVIEW_TOTAL_BUDGET_USD=1.0386 bash fix/run_fix_review.sh --only <picks> ->
  revise r1..2 (FIX_REVISE_ROUNDS=2 run_fix.sh --revise --only <picks> + review, ladder_ws env, Haiku env) ->
  FIX_WORK=$W python3 -I fix/fix_ladder.py promote <parent> per accepted pick -> residue A (rescue) / B (human) -> D4..D8.
  Ledger check: bash .claude/review/work/fix/ladder/phaseD_ledchk.sh (prefix rule). Never FIX_SANDBOX=none.
  Contract check D1: 8 OUT_OF_SCOPE all non-worker (my dry-run prompts, handoff, ledger, other session's scripts/agent_usage*).

### Phase D -- D3 state (2026-10-10 21:15Z)
- Variants: 90 run, 0 call failures; fix spent 2.093918 (cap 4.06). Judge spent 0.066; review spent ~0.32 (cap 1.0386).
- PROMOTED (18, accepted picks): T0397dd861d T0bf61f28f9 T13f3038560 T3444e1c047 T3ceaae22f6 T56254ab7ca T58c72555da T5b61d0dbeb T5fb1f99f54 T6ec35ece74 T7500dc46ff T765377f325 Tb14ed929f8 Tb28355629a Tb7c05539a2 Te0f3b8ded4 Tf233565c73 Tf637ee82fd (spent() unchanged -> W1 fix verified).
- Residue A (no check-ok variant -> D4 Sonnet rescue): Tb35bae71be Tbcf5912136 T8d311ed915 T0fe8d66a96 T44c6eb1512 T954ea6c16f T1fc33392eb (+ Phase C Tec3f9845c4 T2f516f8152).
- Residue B (human): T9d91841cf1 + Tc7551fb5dd (revise after 2 rounds); T4de1c8bb5b (revise -> notfixed); T59c8befe9e (revise -> check-failed).
- Tec4de88307 (pick T6458e21795): revise a3 patch exists, checks ABORTED twice by LEDGER TRIPWIRE (another session appending CONTRACT_CHECK bursts). Check state reset (aborted files in work/fix/ladder/aborted_a3*); `record_fix.py next T6458e21795 --revise` = `finish 3` ($0). Re-run: ladder_ws env + `run_fix.sh --revise --only T6458e21795 --class needs-design`, then review.
- Ledger checker widened: appended non-CONTRACT_CHECK lines OK unless they mention fix_pipeline/kotor2_review/.claude/review (another session's SESSION_HANDOFF at 20:47Z).
- BLOCKER: fix_checks tripwire aborts whenever ANY append lands during a check; another session appends every 2-6 min. Awaiting user decision.

### Phase D -- state at D6 (2026-10-10 ~21:45Z)  [supersedes the "BLOCKER" line above]
- Tripwire FIXED (user chose prefix rule): fix_checks.py ledger_hash() -> "<size>:<sha>", ledger_same() tolerates appended
  CONTRACT_CHECK / SESSION_HANDOFF lines not mentioning fix_pipeline|kotor2_review|.claude/review. Self-test 34/34 PASS.
- D3 DONE: 19 promoted (18 listed above + Tec4de88307 <- T6458e21795.a3).
- D4 DONE: Sonnet rescue rescued Tbcf5912136 (check-ok). notfixed: Tb35bae71be T8d311ed915 T0fe8d66a96 T44c6eb1512 T954ea6c16f;
  check-failed x2: T1fc33392eb. Phase C Tec3f9845c4/T2f516f8152 not re-queued (already have results) -> human.
- D5 DONE: final verdicts accept 84 / revise 11 (stuck -> human) / no-patch 42 / pending 0. Revise round (16 Haiku-class) logs d5_rev_*.log.
- Spend (cumulative): fix 2.57099, review 0.575867, judge ~0.066. Integ counter RESET: stale batch_01/02 outputs moved to
  work/fix/ladder/integ_stale/ (they held 0.170533 = S0 integ, still counts toward history). New-spend formula for integ = spent integ (fresh).
- D6 RUNNING: build_integration_batches.py --force -> 17 batches (84 patches). batch_01 PASS + published (.claude/fixes/batch_01, $0.121).
  Remaining 16 running in background: `FIX_WORK=$W INTEG_PARALLEL=3 INTEG_TOTAL_BUDGET_USD=2.5 bash fix/run_integrate.sh | tee $W/logs/d6_rest.log`
  (cap raised 1.6 -> 2.5; total new spend still projected ~$3.6 < $6). Resume: check d6_rest.log tail (publish lines, "Not complete"),
  `python3 -I fix/fcommon.py spent integ`, ledger check; re-queue leftovers with `--only`. If runner died mid-run: `run_integrate.sh --dry-run` lists pending.
- NEXT: D7 `python3 -I fix/fix_merge.py` -> .claude/reviews/fix_report.{json,md}; Haiku recon A (human list by category) + B (1-line
  diff summary per patch in .claude/fixes/batch_*/); Opus spot-reads 3 patches. D8 persist + memory + final 4-section report + STOP for per-batch apply approval.
- Human list so far: T9d91841cf1 Tc7551fb5dd T4de1c8bb5b T59c8befe9e Tec3f9845c4 T2f516f8152 + D4 6 above + 11 D5 revise-stuck (fix_merge will enumerate).
- All commands run from .claude/review with W=$PWD/work/fix (ABSOLUTE). Ledger check: bash $W/ladder/phaseD_ledchk.sh (widened rule).

### Phase D result (2026-10-10 ~22:20Z) -- STOPPED before apply  [supersedes "state at D6"]
- D6 DONE: 17/17 batches ok, 84 patches published to .claude/fixes/batch_01..17 (series.json each). Every batch: own build/test
  gates PASS, new_failures [], ledger_unchanged. Repo-wide `diffcheck` = info FAIL=2 identical in all 17 (incl. script-only batches) -> pre-existing.
  Integ cost $2.497 (17 calls, ~$0.147/batch; plan estimated $1.4).
- D7: extra revise round for 2 merge-pending tasks: Tbcf5912136.a3 check-ok -> review revise again (stuck -> human); T88100321d8.a3 check-failed -> human.
  fix_merge: 137 tasks = fixed+tested 67 / fixed+needs-live-check 17 / rejected 9 / human-only 8 / not-fixable 26 / needs-wider-scope 6 / pending 4.
  The 4 pending (Tc7551fb5dd T9d91841cf1 T4de1c8bb5b T59c8befe9e) = ladder picks revise-stuck, never promoted -> human. Do NOT re-queue.
  Report: .claude/reviews/fix_report.{json,md}. Calibration 5/5 caught, 0/3 false-reject. Fixer/reviewer agreement 84/93.
- Spot-reads (Opus): T7500dc46ff (cpp, LevelUpOverride reloadPlay log) correct -- filewatch::changed already sets p.last, so the old log never fired;
  Td08fb0986a (python, master_ledger None-ts guards) correct; Tb28355629a (test_plan_manager isolation, --handoff-dir + active_plan restore) correct, no assert loosened.
- Spend NEW (vs S0): fix +1.248 (2.606698) / review +0.341 (0.579501) / judge +0.026 / integ +2.497 (fresh counter) = ~$4.11 of $6 ceiling.
- Verification: ledger prefix OK (384 lines) after every run; prompts leak rg = 0 files; tracked `git status` drift vs D0 = other sessions only
  (swarm plan, agent_usage* committed in 0d67415).
- APPLY (needs per-batch user approval; NOT done): 
  `git clone --no-hardlinks ~/orca/workspaces/local-setup/KOTOR-II ~/.cache/kotor2_review/apply && cd ~/.cache/kotor2_review/apply && git checkout -b fix/audit-findings 22b56b4`
  then per approved batch: `git apply --3way <repo>/.claude/fixes/batch_NN/*.patch` (series order) -> scoped tests (`run_tests.py <suite>` per touched dir) -> commit per batch.
- Recon packets: scratchpad recon_A.md (human list) / recon_B.md (per-patch 1-liners) copied to .claude/review/work/fix/ladder/phaseD_human_list.md + phaseD_patch_summary.md.
- Open items for user: T32db00261f sign-off (in batch_10), Td18cc50320 H10 (in batch_06), Tc307b197f4 contract.
- 22:35Z user: "apply ALL batches" -> plan mode for a detailed apply-execution plan; Sonnet recon (read-only) weighs in on
  Td18cc50320 (batch_06, H10 export removal), T32db00261f (batch_10 gate-integrity sign-off), Tc307b197f4 (GuiInput.h:49 contract;
  options: accept Haiku's return -1 caller change, or keep as human/contract item). Recon packets in work/fix/ladder/phaseD_*.md.

## Phase E plan (2026-10-10 ~22:55Z) -- apply all 17 batches  [supersedes the "clone at 22b56b4" apply line above]
- BASE FINDING: patches were made against `~/.cache/kotor2_review/fix/base` commit `a8ea0fb` (sanitized working-tree
  snapshot 16:47Z + 197 overlays, all committed), NOT 22b56b4. 14/40 patched files differ from 22b56b4 and 7 are untracked
  (plan-35 clef files) -> a 22b56b4 clone cannot take the series. Real tree == base for all 40 files (0 drift, rechecked 22:55Z).
- Apply dir: `~/.cache/kotor2_review/apply`, branch `fix/audit-findings` from a8ea0fb. Deliverable = commits + format-patch
  series in `~/.cache/kotor2_review/apply_series/`. Landing in the real tree = separate user decision.
- Only cross-batch overlap: scripts/run_tests.py, batch_10 Tec4de88307 vs batch_13 T7ccac6b666 (same "fail closed on
  unmatched test ids" fix); keep batch_13's superset, drop batch_10's block.
- Decisions (user, 22:50Z): Td18cc50320 (batch_06) HOLD -- it deletes uncommitted H10 WIP (queue_hud exports.def +1,
  hooks.toml +19, hook 0x00775703; handler body only exists in tests/.cache/build/queue_hud_src/QueueHud/QueueHud.cpp:973)
  -> H10 owner must restore the handler or drop the hook. T32db00261f (batch_10) SIGNED OFF + fast rc fixture
  (scripts/test_compat_check_rc.py). Tc307b197f4 = extra commit: apiWheelSubscribe fails closed when ensureSubclass() fails.

## Phase E result (2026-10-10 ~23:20Z) -- DONE, STOPPED before landing  [supersedes "Phase E plan"]
- Clone `~/.cache/kotor2_review/apply`, branch `fix/audit-findings` (base a8ea0fb), 19 commits, 83/84 patches + 2 extras.
  Series: `~/.cache/kotor2_review/apply_series/0001..0019-*.patch` (format-patch a8ea0fb..fix/audit-findings).
- Commits: b01 3480db1 · b02 16a79a3 · b03 e867760 · b04 6261ab4 · b05 6e6ebc5 · b06 a8fe942 (Td18cc50320 HELD) · b07 0bec0ed ·
  b08 a2b68be · b09 99d9f87 · b10 549ccb7 · b11 afdcee3 · b12 0135c32 · b13 b61aa7b (run_tests.py conflict, kept T7ccac6b666) ·
  b14 76a5674 · b15 f49854d · b16 1207048 · b17 ec2fb40 · rc fixture e1f90d7 · Tc307b197f4 e4840c7.
- Gates (logs `apply/apply_logs/`): every batch's scoped gates == baseline. Final build_check FAIL=1 = queue_hud_src (baseline,
  H10 held). Baseline-failing gates unchanged: test_subagent_tools.py (diff_check baseline case), levelup-parity
  (LevelUpOverride.cpp:79 `KpLog.h` not on the parity include path) -> batch_14/17 changes are NOT exercised by tests.
  test_compat_check.py skipped (300 s baseline timeout); the new fast test_compat_check_rc.py covers T32db00261f
  (mutation-checked: FAILs with the hunk reverted).
- Behaviour change from batch_13 T7ccac6b666: run_tests.py exits 2 on a filter that matches 0 items. Filters `tests/static`
  and `action_hotbar_src` matched 0 items before (silently "pass") and now exit 2 -> update any gate/doc that uses them.
- Landing dry-run: all 19 patches apply in sequence to copies of the real tree's files (scratch), result byte-identical to the
  clone. Note `git apply --check a b c` checks each patch alone and falsely fails batch_13 -> apply sequentially.
  Mode warnings only (base has 0644 for several scripts, real tree 0755).
- New findings (not fixed): (1) compat_check m_fail regex misses build_check's real `FAIL build: <mod> (<Label>): ...` format,
  so real build FAILs become the generic build_check ERROR (pinned as expectedFailure in test_compat_check_rc.py).
  (2) GuiKit sets g_wheelHwnd even when subOk is false, so wheelInstall callers gated on !g_wheelHwnd never retry.
  (3) action_hotbar_src has no test item under its filter (hotbar_logic_test.cpp not registered?).
- H10 owner: Td18cc50320 held; QueueHudPerformCastSpell handler body exists only in tests/.cache/build/queue_hud_src/QueueHud/QueueHud.cpp:973.
- Incident: I ran the real repo's agent_contract.py snapshot once, which overwrote the real repo's untracked
  .claude/contract_baseline.tsv (another session wrote it at 18:42 local). It now holds a real-repo snapshot taken at 18:46 local.
- Live check after any install: wheel scroll in hotbar picker, 3D inventory, equipment preview, active effects panel,
  including mod load before the game window exists.
- Landing options (user decision): (a) `git am`/`git apply` the series into the real tree (touches other sessions' uncommitted
  files: 14 differ from 22b56b4, 7 untracked); (b) land per-batch only for files the owning session approves; (c) keep as reference.

## Phase F direction (user, 2026-10-10 ~23:30Z)
- Leaning option A: land the 19-patch series into the real tree. All uncommitted real-tree changes are from Agy
  (Gemini Flash 3.8) sessions that stopped midway on master-plan items at quota; user says safe to apply underneath them.
- Haiku sub-agents wrap up and land those in-progress Agy changes too, as part of the whole Agy+GeminiFlash3.8 review.
- Ordering constraint (Opus): the series was built ON TOP of the Agy WIP (base a8ea0fb = working-tree snapshot), so the WIP
  must be committed first (as-is), then `git am` the series, then Haiku wrap-ups on top. "Underneath" literally is impossible
  for the 40 files where WIP and fixes interleave.
### Dead ends (Phase E) -- do NOT attempt
- `git apply --check p1 p2 ...` checks each patch alone -> false FAIL on batch_13 run_tests.py; apply sequentially.
- Real repo's `scripts/agent_contract.py snapshot` always snapshots the REAL repo (DEFAULT_ROOT = script dir) and overwrites
  `.claude/contract_baseline.tsv`; for other trees use that tree's script + `--out <scratch>` + `check - --repo-root .`
  (put `-` BEFORE `--scope`, scope is nargs+).
- run_tests.py filters that match 0 items (`tests/static`, `action_hotbar_src`) exit 2 after batch_13; don't use as gates.
- plan_manager.py handoff/mark-done: not used for this plan (active_plan.txt belongs to other sessions).
### Verification commands
- Ledger: `bash .claude/review/work/fix/ladder/phaseD_ledchk.sh` (expect LEDGER OK, prefix 384+ lines).
- Clone gates: `python3 <scratch>/gates.py <batch|baseline>` (scratchpad of session 54fbf50c; copy into repo scratch if needed).
- Build: `python3 scripts/build_check.py` (expect FAIL=1 queue_hud_src until H10 resolved).

## Phase F result (2026-10-11 ~00:45Z) -- DONE, landed locally, NOT pushed, NOT installed
- Agy stopped: 5 KOTOR-II sessions SIGTERM (731453, 1484094 planned + 1771477, 1774206, 4192689 user-approved). Other repos' agy untouched.
- Commits on branch KOTOR-II (on 78807d4), 34 total, contiguous:
  - F2 wip(agy) x12 d331219..0e5f1e9 (57 paths; groups in session scratch F2_groups.txt): doc35 clef+ledger, subagent tooling,
    doc33 QH4/QH5 queue_hud, doc33 SH2 self-heal, doc33 AH3 hotbar, L4 shared input, doc24 level-up KpLog 1.6.0-dev,
    CS3/CS8 selfheal log check, kotor2_logs, doc22 U2 tests, query_ghidra --cpp, plan docs/handoffs/.agents skills.
  - F3 series x19 7df1b4a..68a7739 (git am --3way, 0 conflicts; 42 files == clone; real-tree modes stay 0775).
  - F4 wrap-ups: d4ef936 H10 restore, 4add886 H10 unit tests (Haiku), 7020d54 parity harness KpLog port (Haiku x2 -> Sonnet).
- H10 root cause: Agy session 2e8493b4 ran `git checkout` on QueueHud.cpp + QueueHudLogic.h (brain step 399) and wiped session
  e5b6e399's H10 work; exports/hooks kept the symbol -> link FAIL. Restored from tests/.cache/build staging by 3-way merge.
  Td18cc50320 stays dropped for good.
- Gates (F5): build_check FAIL=0 PASS=28; gates.py baseline == F0 except 3 fixed (queue_hud build, test_subagent_tools,
  levelup-parity) + known 0-item `action_hotbar_src` filter rc=2; check-raw-addresses PASS; compat_check queue_hud 5/5; ledger OK 391.
- Left uncommitted on purpose: preagy files (09-23..10-05: issue drafts, PULL_REQUEST.md x9, compatch docs, old scripts),
  artifacts (mods678_research/, steam_db_pr3/work/, *.db, ledger json, tmp*_plan handoffs), all mtime 10-10 files (live Claude sessions),
  session_handoff.md / swarm plan / AGENTS.md / CLAUDE.md.
- Needs-live (after a user install): H10 cast buffering -- hook 0x00775703 original_bytes + ebp+8..0x20 arg offsets unverified live;
  QH5 leader colours; SH2 leader card; AH3 save deferral; level-up 1.6.0-dev KpLog (INFO lines need config level >= INFO);
  wheel scroll (hotbar picker, 3D inventory, equipment preview, active effects panel, mod loaded before game window).
- Needs-human: scenario_test.cpp 'logcap' action is now a no-op (compiled, not run); docs 10/13/19b/27/30/31 marked partial by recon.
- Open findings: compat_check m_fail regex vs build_check FAIL format; GuiKit g_wheelHwnd set when subOk false; KpLog fallback
  level WARN drops INFO in any host test that never loads config.

## Phase G direction (2026-10-11 ~01:00Z): install + live checks
- kpm_status (game_running=no): MIRROR x7 = action-hotbar, companion-selfheal, inventory-3d-viewport, mouse-probe, shield-bar
  (DLL-only -> hot-swap) + queue-hud (hooks.toml + exports.def changed) and level-up-override (exports.def +KPatchInit) -> FULL REINSTALL.
- scripts/kpm_reinstall.py and kpm_ship.py do NOT exist yet (BUILD_AND_INSTALL.md §4/§5b reference doc 36 future work);
  full reinstall = manual `KPATCH_NO_LAUNCH=1 "$K/bin/NoLaunch/KPatchLauncher" "$G/swkotor2.exe" --patches <dir> <ALL live ids>`
  after a manual backup ($G/backups/<tag>_full_<ts>/: DLLs + patch_config.toml + kpm_install_state.json).
### Dead ends / traps (Phase F) -- do NOT attempt
- plan_manager.py handoff|mark-done for this plan (appends to master_ledger.jsonl; active_plan.txt is other sessions').
- Using `agent_contract.py check` with placeholder MODIFIED lines (`x`) -> spurious OUT_OF_SCOPE FAIL; list real paths.
- KPatchLauncher for hot-swappable mods (reverts peer DLLs). Two installers at once (re-run kpm_status before/after).
- Reading raw *_log.txt (use kotor2_logs.py check <log> --new).
### Verification commands
- `python3 scripts/kpm_status.py` (expect LIVE for swapped ids); `scripts/pre_install_gate.sh --mod <a>_src ...` (GATE PASS);
  `python3 scripts/build_check.py` (FAIL=0 at 7020d54); `bash .claude/review/work/fix/ladder/phaseD_ledchk.sh` (LEDGER OK).

## Phase G result (2026-10-11 ~01:20Z) -- INSTALLED, live checks pending
- Backup (pre-everything): `$G/backups/phaseG_full_20261011T000211Z/` (patches/, patch_config.toml, kpm_install_state.json, live_md5.txt).
  Pre-G3 md5 snapshot: session scratch `pre_G3_md5.txt`. G2 swap backup: `kpm_hotswap.py --rollback batch_swap_20261010_210610`.
- Scope changes vs plan (user-approved): action-hotbar needed the save-events + gui-input providers (kpatch 1 hook vs live 8) -> user chose
  "migrate now"; level-up-override hot-swap refused by PE export check (+KPatchInit) -> moved into G3.
- G2 hot-swap (3): companion-selfheal 2e1ddc29, inventory-3d-viewport f9c4f877, shield-bar 9bf6431e.
- G3 full reinstall (39 ids, stage `$K/phaseG_stage/`, script = session scratch `stage_g3.py`): queue-hud f224e731 (H10
  QueueHudPerformCastSpell @0x775703), action-hotbar a661856c (1 hook), level-up-override 02a9b115, NEW save-events f3c410e0,
  NEW gui-input ec5af590. Every other DLL byte-identical; 238 hooks, 0 overlaps; 4GB LAA byte 0x23 ok; mouse-probe probe DLL kept.
- Gate: all 8 mods build+test PASS. Default-suite fixes: repo b0c9ea2 (levelup scenarios KpLog port). User WAIVED:
  test_agent_contract (uncommitted AGENTS.md/CLAUDE.md drift, not ours), test_compat_breakage::test_1 (fails at HEAD; meta-test).
  Stale `tests/.cache/build/queue_hud_src` moved to session scratch (it posed as a mod in build_check).
- KPM commit 61ebb73 (ethansm/local-mods). Not pushed.
- Live checks: PENDING (H10 buffered cast, level-up 1.6.0 KpLog INFO, wheel scroll x4, QH5/SH2/AH3, action-hotbar on providers:
  save/area transition + hotbar picker wheel).
- Rollback G3: restore phaseG_full backup wholesale, then `kpm_hotswap.py --rollback` is NOT needed (backup predates G2; re-run G2 swaps if wanted).

## Phase G live check 1 (2026-10-11) -- hotbar + self-heal UI NOT visible
- Cause (logs): HUD panel +0x48 = 0xFF8FB44F (old 0x47) -> bit 0x400 "ignored" -> gameplayHudActive() skips the HUD in both
  ActionHotbar.cpp:300 and CompanionSelfHeal.cpp:229. Providers ruled out (Haiku recon .claude/review/work/phaseG/*.md).
- Bisect: repo d719d83 / KPM d17f9e6 = self-heal without the 4 SH2 leader widgets + "P0 bisect ... before build" log; hot-swapped
  (rollback `kpm_hotswap.py --rollback companion-selfheal_swap_20261011_010005`). Next: user loads a save, read P0/P1 +0x48.

## Phase H direction (user, 2026-10-11): analyze the 2 UI bugs + leader self-heal button
"plan to analyze the bugs impacting these two mods (action-hotbar, companion-selfheal). Also investigate the self heal button for the
party leader: it should be rendered outside of the portrait, to the left, inline with the top of the portrait. We just need the
button to enable auto medpacks on the leader."
- Live state: 39 ids installed (Phase G). companion-selfheal = bisect probe d719d83 (kLeaderWidgets=false, P0 log), NOT yet played.
- Dead ends / do NOT:
  - Do not blame save-events / gui-input for +0x48: gui-input has hooks = []; save-events' only store is server app +0x1F254 (recon hud48_providers.md).
  - Gate code (gameplayHudActive) is unchanged since 78807d4; do not "fix" by reordering it until the +0x48 writer is known (user chose bisect over gate workaround).
  - No in-range source line writes panel +0x48 by static search (hud48_shipped.md); next evidence must be live (P0/P1 log lines).
  - plan_manager.py handoff|mark-done: not used (shared master ledger / other sessions' active_plan.txt).
- Verification commands: `python3 scripts/kplog_parse.py "$G/companion-selfheal_log.txt" --bare | grep -E 'P0 bisect|P1 hud built'`;
  `python3 scripts/kplog_parse.py "$G/action-hotbar_log.txt" --bare | grep -E 'gate:|panel\[0\]'`; `python3 scripts/kpm_status.py`.

## Phase H progress (2026-10-11 ~02:50Z) -- probes LIVE, awaiting play session; NOT final result
- Commits: repo b61dc81 (H2 BUILD48 probe + H4 leader auto-toggle button left of portrait, kLeaderWidgets=true), 9cc65d9 (GATE reason logs in selfheal+hotbar; HUD48 change watch + 0x60-byte BUILD48 dump in hud-host). KPM: 50e4b11 (Common/SelfHealUi.h only, NOT --sync), 0731d9b, 8b84041, 7873d4c. Not pushed.
- LIVE (hot-swap, rollback `kpm_hotswap.py --rollback batch_swap_20261011_024540`): hud-host a2601899, companion-selfheal 0aa38df2, action-hotbar 63397f21. Gate: only waived test_compat_breakage fails (swarm-fix-checks flakes when a peer writes the ledger; passes on re-run).
- Findings (recon in .claude/review/work/phaseH/: r1_panel48.md, r2_hotbar_build.md, l1..l4):
  - BUILD48: HUD +0x48 is already 0xFF8FB44F at hud-host BUILD entry (0x40F5A0), no consumer changes it => action-hotbar timing is NOT the writer. Second HUD build in same launch: panel 08a02008, +0x48=0xFF8FB447 (first: 08a024f8). frame0: 0xFF8FB44D (engine clears bit 0x2 only).
  - +0x48/+0x4C = ff8fb4xx / ff90b500 look like ARGB colours, not flags => the "+0x48 flags" layout assumption may be wrong. 0x40F5A0 only does AND ~2 on +0x48 (R1). Writer unidentified.
  - Hotbar gate lines never ACTIVE: `other panel on top` x8, `hud not in list` x6, `modal open` x3, gateflaps 14 => +0x48 is not the only gate failure reason.
  - Only action-hotbar + companion-selfheal use the 0x600 gate/gameplayHudActive; combat-hotbar, queue-hud, buff-duration-hud, shield-bar do NOT (no gate logging; hide via their own paths e.g. CombatHotbar hideAllButtons, QueueHud cwShow/vis).
- User report 02:5x: still no action-hotbar UX or SelfHeal UX (that was the OLD probe build; new probes not yet played).
- Next: add state-change `VIS` log to the other 4 HUD mods (visible/hidden, reason, rect) as one hot-swap, then ONE play session reads GATE/HUD48/dump/VIS.
- Dead ends (do NOT attempt): blaming save-events/gui-input; blaming SH2 leader widgets; teaching gates to accept an ignored HUD before the writer is known; plan_manager.py handoff|mark-done (shared ledger); --sync of shared headers; KPatchLauncher for DLL-only changes.

## Phase H2 progress (2026-10-11 ~12:00) -- VIS probes LIVE, play session DONE, logs NOT yet reviewed
- Commits: repo 7bed66e (VIS state-change logs in combat-hotbar, queue-hud, buff-duration-hud, shield-bar; logging only), KPM 59b7f17/2688295/3d194ae/7b3ba37. All 7 HUD mods LIVE (kpm_status). Rollback: `python3 scripts/kpm_hotswap.py --rollback batch_swap_20261011_115906`.
- VIS format: `VIS <what> shown=%d why=%s rect=[x,y,w,h]`, state-change only, 64-line cap then `VIS cap reached`. Items: combat-hotbar `group<g>`/`buttons`/`build`; queue-hud `cancel0..2`/`build`; buff-duration-hud `slot<n>`/`menu<k>`/`overlay first call`/`hud column first call` (needs buffhud_debug.txt, present); shield-bar `bar0..2` (HUD) `mbar0..2` (menu)/`overlay first call` (shield-bar now has KpLog channel shield-bar/main).
- User play-test feedback (verbatim): "Did not see the ActionHotbar or SelfHeal UX on first load. Upon loading another save, they were visible. The first save seems to have gotten into a dirty state that should be recoverable. The button for the self heal needs to be bigger, the circle should have the same diameter of the queue elements next to the companion portraits. The leaders button is overlapping shield and vitality bars, it needs to move left a lot to avoid overlap. The self heal button should be disabled with no animation when a character is unconcious. When they awake, it should return to the same previous state."
- Interpretation: (1) first save = HUD 'ignored'/gate state never recovers (matches +0x48 / gate findings); second save loaded fine => a recovery path (re-evaluate gate / rebuild on state change) is needed, not only root cause. (2) self-heal button size = match queue-hud cancel circle diameter; (3) leader button x-offset left of shield/vitality bars; (4) unconscious => disabled, no animation, restore prior (on/off) state on wake.
- Dead ends carried over: blaming save-events/gui-input/SH2 leader widgets for +0x48; gate workaround before writer known (now REVISIT: user wants recoverability, so a recovery re-check is allowed if root cause stays unknown); plan_manager.py handoff|mark-done for this plan (it resolves to the peer's active plan 34_npc_diversity_dispatcher and CLOBBERS its handoff; I git-checkout-restored it once); --sync of shared headers; push.
- Peer work still dirty (not ours): npc_diversity_src/*, 34 handoff, 24_master_plan_status.md.

## Phase H3 result (2026-10-11)
- **Root cause** (first-load HUD missing): first HUD build of the process has stale heap garbage in HUD panel `+0x48` (`0xFF8FB44F`); engine never clears bit `0x400`, so the gate's `PanelIgnoredByInput=0x600` test saw the HUD as ignored forever. Second save rebuilt the panel clean (`0x47`). Fix = gate-side only (read-only): `hudFlagsPlausible(f48) = (f48>>12)==0`; implausible HUD flags are treated as NOT ignored (logs `GATE hud flags implausible f48=%08x -> treated as not ignored`). In action-hotbar and companion-selfheal.
- **SelfHealUi.h** (commit 33d4c5e): `discDiameter(vitH)` = queue autoIconPx (96 -> 58); `buttonRect` puts the 58 px disc right edge `gap` left of vit.x, cross button 39 px centred; `stripReserve` = gap + ring side + 2 (live 66); `leaderButtonRect(card, vit, diameter)` anchors at `min(card.x, vit.x)` minus gap minus `kLeaderClear=6`; `BtnState::Down` (`resolveState(..., down)`): dim, no ring, no animation, not clickable; popover keeps its old size (`popUnit`, `popoverLayout(ringRect, screenW, unit)`).
- **CompanionSelfHeal.cpp**: passes `m.down`, leader disc = companion disc, Down button made non-clickable (no hover/tooltip/click), popover closes, logs `STATE member=N old->new hp=c/m` and `LEADERBTN rect=... ring=... disc=.. reserve=..`.
- Pinned tests updated deliberately: selfheal_ui_test (button/leader/stripReserve), queue_hud_logic_test (reserve 38 -> 66, oldSelfHealReserve now disc-based), both common_contract_tests (24 px/38 pins).
- Gate: only waived `py:scripts/test_compat_breakage.py` failed. KPM mirrors committed (34be0e2, ddc80b2). queue-hud DLL unchanged (LIVE; it gets the new reserve through the hud-host strip slot).
- State: action-hotbar + companion-selfheal are SWAP (prepared). Pending: user GO, `kpm_hotswap.py --mod action-hotbar --mod companion-selfheal --apply` with the game closed, one play session on a FRESH launch, Haiku log check (kplog_parse only) -> `.claude/review/work/phaseH3/log_review.md`. Rollback: `python3 scripts/kpm_hotswap.py --rollback batch_swap_20261011_115906`.

## Phase H3 log review (2026-10-11, session 18:06:39, read-only) -- details `.claude/review/work/phaseH3/log_review.md`
- **PROVEN**: the `+0x48` garbage fix ran on real garbage twice and recovered both times. First HUD build `ff8fb44f` (action-hotbar gate T=33203.594 `implausible f48=ff8fb445` -> `open=1` next line; self-heal T=33216.651 same), second HUD build `aaa83c45` (T=604639 / T=604646) also recovered. 0 `hud-ignored`, 0 `GATE cap reached`, 0 append failed, 0 WARN/ERR in action-hotbar, self-heal, hud-host.
- **PROVEN**: geometry. `LEADERBTN rect=[3445,1403,39,39] ring=[3436,1394,58,58] disc=58 reserve=157`; 3436+58+17 = 3511 = member0 vit.x, ring.y = card.y; members 1/2 vit h=96. Queue strip icons 58x58, right edge 3514 = 3671 - 157 (the plan's "vit.x - 66" was stale; 157 is the logged reserve); no overlap with the self-heal ring.
- **NOT EXERCISED**: `BtnState::Down` (0 `Down`/`hp=0` lines in 18 STATE transitions: AutoOn<->Normal, Normal<->FullHp, Hidden->.., one Normal->Pending->Normal). The clean-flag first-build path (C2) was also not seen (garbage path taken).
- Watch items, not H3: action-hotbar load-time alert `creature .. invalid object id ffffffff` T=604581 (transient); queue-hud 1 cosmetic WARN `blank icon type=0x3F` T=640199; combat-hotbar VIS probe logs only `shown=0` (group controls) while the target pills worked (visible=101, 4 clicks restored=1), so the probe does not cover the pill path.
- **Retest (2 min, user)**: Down needs a companion at 0 HP with auto-heal ON, then revived; expect `STATE member=N AutoOn->Down hp=0/..` then `Down->AutoOn hp>0`, and no click/USE while Down.
- Method note: Explore-type Haiku agents are read-only, so `OUTPUT:` was not a path in 5 of 6 replies; use a writer agent type if a file is required. `agent_contract.py check` usage: put `-` before `--scope`.

## Play session 2026-10-12 00:16 log review (read-only, 6 Haiku + Opus verify) -- details `.claude/review/work/play1012/log_review.md`
- **PROVEN now**: Down state (H3 open item) exercised in 3 episodes: no USE/CLICK while Down; m1 woke via auto Medpac `Down->Pending->AutoOn`; m2 woke `NoItem`. Gate fix recovered 7/7 garbage (`implausible f48` -> `open=1`), 0 hud-ignored. 0 ERR in the 5 HUD logs; the two ~75 s `other-panel-on-top` windows were a dialogue letterbox and the inventory/equip/effects panels.
- **Not a defect**: npc-diversity checker FAIL x2 (cross-version 0.1.2 vs 0.1.4 compare; invalid oid 7f000000); mouse-probe SYNTHDBL x1; selfheal 5 WARN `REFUSE already queued (auto)` (should be INFO).
- **Session ran buff HUD 0.2.0**: 0.2.1 (4261c1b, 00:52) was committed after launch; 36 `DBGMISS` in 0.2.0 = missed debuff (Slow). 0.2.1 is LIVE now; replay needed.
- **Open**: m1/m2 Down for ~318 s (T=712-1030 s) then both woke within 5 ms at combat end + party-slot remap + leader change; real knock-out vs stale Down unproven (ask user). Gate logs hit GATE cap (selfheal T=954 s, action-hotbar T=557 s). queue-hud 3 `orphan-round` after our cancel (0x3F reappears +320 ms; origin unlogged).

## Queue-hud noise for Down companions (2026-10-12, recon only; user confirmed m1/m2 really were knocked out) -- `.claude/review/work/play1012/Q-A..Q-D.md`
- **Symptom is HUD flicker, not queue spam**: while Down, m1/m2 each got an engine-origin `0x3F` entry about every 3.0 s (one combat round; lifetime ~105 ms, `ours=external`), so the strip repainted for `kPanelHoldMs=400` ms each time: 215 strip show/hide flips in 318 s (40/min) vs 11/min with members up. Entry rate itself was low (about 2/min logged, 100 ms sampling undercounts).
- **Fix point (HUD side, no new engine hook)**: queue-hud has no HP check. `party::vitals(server,&v)` in `_shared_src/Party.h:108-118` gives `down = ok && cur<=0` (same rule self-heal uses; queue-hud already includes Party.h). Gate at `drawStrips` (QueueHud.cpp:633/728 `shouldShow`) and skip Down members in `journalTick` (:1496) and `roundWatchTick`; unknown vitals (ok=false) = show. Pin the pure rule in `queue_hud_logic_test.cpp` beside the strip tests.
- **Source-side block** (drop the engine's push for HP<=0 creatures) would need a new queue-add hook; engine path undocumented (0x3F = item/cast tail node, 0x3D = follower FollowLeader, follow driver 0x5C7370). Not recommended.
- Open: who pushes the 0x3F every round on a Down member (selfheal made 1 routeA call only, T=710450); does the engine clear the queue when the member wakes.

## queue-hud 0.4.1-dev Down gate (2026-10-12) -- BUILT + TESTED, NOT INSTALLED
- Commit above (repo): `QueueHud.cpp` `memberDown()` (party::vitals, 100 ms cache) gates `drawStrips`, `journalTick`, `roundWatchTick` for k=1-2; clears the held strip so it cannot flash on wake; `qh::stripHiddenForDown(ok,down)` pinned in `queue_hud_logic_test.cpp`, vitals pinned in `common_contract_test.cpp`. `run_tests.py queue_hud` 3/3 PASS, compat_check PASS 5/0.
- Next: user GO -> `kotor2-kpm-install` skill (stage/mirror + hot-swap, game closed). Play check: knock a companion out; expect no strip flashing for them (`strip k=N hidden` stays), strip returns after wake; log line `0.4.1-dev:` at start.
- UPDATE 01:31: queue-hud 0.4.1-dev (Down gate) is ALREADY LIVE: the doc 33 session installed queue-hud at 01:21 (QL9, hash 157562a8, commit 533f8fd) from a tree that included 3016ffc; live DLL carries the `0.4.1-dev` string and the source is unchanged since. No second install needed. Play check: knock a companion out, expect `strip k=N hidden` to stay (no 3 s flashing) and the strip back after wake.

## Log-hygiene batch (2026-10-13) -- COMMITTED 0a3385e, NOT INSTALLED, gate blocked by peer failures
- Done: selfheal `REFUSEI` (already queued / debounce at INFO) + `GATE late` 30 s heartbeat past the 64-line cap in companion-selfheal AND action-hotbar. Worker tests pass; Opus reviewed the diff.
- Pre-install gate FAIL (5 suites), none caused by this batch (control run for test_compat_breakage with edits reverted fails identically): check-raw-addresses (voice_probe 0x0068EA70), check-hook-spans (area-hook-probe vs k1-spawn-probe), kvoice lip test, AGENTS.md/CLAUDE.md drift, compat_breakage (root cause open).
- Full detail, deferred logging todos with exact code sites, peer conflicts (queue-hud dirty): `.claude/review/work/logfix/SUMMARY.md`; recon R1/R3/R4 + R2 in the same dir. Implementation plan for the rest: written in plan mode (see the plan file named in the next session).
