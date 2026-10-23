# Spec review brief (Sonnet skeptic)

You review executor specs `plan_RV<n>.md` (in this dir) written by other agents from `patch_manager_mods/47_queue_hotbar_review.md`.
The executor will apply them after a context clear, in order RV1 -> RV2 -> RV3 -> RV6 (queue-hud), RV4 (selfheal + _shared_src/ActQueue.h),
RV5 (action-hotbar). Your job: find what will break or mislead the executor BEFORE code is written.

Check, against the real code (targeted `sed -n`/`rg -n`):
1. Before-snippets match the current code exactly (or, for later phases, the code after the earlier phase's After-snippets).
2. After-code compiles in principle (types, names, signatures, includes, every caller of a changed signature updated, enum switch
   coverage incl. whyName/name tables, test call sites updated).
3. Behavioural correctness: does the fix close the defect without a new double-use, lost command, stuck queue, crash risk
   (null deref, engine call on a stale pointer), or log-checker break (`scripts/queue_hud_log_check.py`, `scripts/selfheal_log_check.py`
   regexes and their tests)?
4. Cross-phase collisions: the same function edited by two phases with incompatible assumptions.
5. Test vectors: expected values actually follow from the After-code.
6. Engine safety: any new engine call or memory surgery must use only addresses already in `_shared_src/GameAddr.h` and a pattern
   already proven live in the repo. Flag anything else as BLOCKING with a safer alternative (e.g. log/measure first, PLAY row).

Write `review_<scope>.md`: a header, then numbered issues `## R<n> [BLOCKING|MINOR] plan_RVx §<item>` with Problem / Evidence
(path:line) / Required correction (exact replacement text or instruction the executor can apply). End with a one-line verdict.
Keep it under ~200 lines. Read-only except your review file; no builds, no git.
Reply with exactly the 5-line REVIEW block: OUTPUT, EVIDENCE, BLOCKING, FINDING, DECISION.
