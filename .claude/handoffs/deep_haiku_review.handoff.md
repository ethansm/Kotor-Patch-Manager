# Handoff: deep-haiku-review pipeline (audit DONE; fix stage in another session; skill design next)

Written 2026-10-10. Branch KOTOR-II, pinned audit HEAD `22b56b427125e12bb7b2914d96b820973d06c445`.
All of this is untracked under `.claude/{review,reviews,plans,handoffs}`. No commits were made, nothing in source edited.

## 1. What was done (find-only audit of the post-Claude-quota autonomous-agent work)
Scope: `11e1284..HEAD` (61 commits) + dirty tree, boundary `2026-10-07T01:05:42Z`, 7 work packages
(WP1 gate integrity, WP2 uncommitted hooks/shared headers, WP3 uncommitted mod behaviour, WP4 provider batch
`c5c67f8..7aeb296`, WP5 NPC diversity, WP6 test weakening, WP7 small adoption/mouse probe). Docs excluded.

Pipeline (all CLI `claude -p`, OAuth so $ are API-equivalent):
| Stage | Model | Script | Output | Result | CLI-reported cost |
|---|---|---|---|---|---|
| Phase 0 prep (no LLM) | - | `.claude/review/prep.py build` | worker tree `~/.cache/kotor2_review/worktree`, `packets/` (80), `coverage_plan.json` (735 units), `snapshot.json` | deterministic | 0 |
| Canary | Haiku | `tools/canary_test.sh` | `canary.json` pass, variant A (`--restricted --safe-mode`) | deny rules proven by a deny-probe call | ~$0.001 |
| Fact runner (no LLM) | - | `tools/fact_runner.sh` | `facts/*.json` | queue_hud link fail, perf_lab harness include fail | 0 |
| Map | Haiku 5.5 x80 | `run_map.sh` (`--first-only`, `prep.py resize`, then full) | `findings/`, `runs/` | 80/80 ok, 422 raw findings | $0.61 |
| Reduce | Sonnet 5.5 x2 | `tools/run_reduce.sh gate_tests wp1,wp2,wp6` / `behaviour wp3,wp4,wp5,wp7` | `.claude/reviews/domain_*.json` | 427 findings, recall 4/5 | $1.01 |
| Merge (no LLM) | - | `merge.py` | `.claude/reviews/master_findings.{json,md}`, `coverage_report.*` | coverage PASS 735 units, 0 gaps | 0 |
| Verify | Haiku 5.5 x275 | `verify/select.py` -> `verify/run_verify.sh` | `verify/results/*.json` + `.check.json` | 168 reproduced / 60 refuted / 31 intended / 16 inconclusive; 3 fabricated quotes | $1.49 |
| Adjudicate | Sonnet 5.5 x29 batches | `verify/build_dossiers.py` -> `verify/run_adjudicate.sh` (`ADJUDICATE_TOTAL_BUDGET_USD=25`, `ADJUDICATE_PARALLEL`) | `verify/adjudications/` | 29/29 ok | $6.06 |
| Verify merge (no LLM) | - | `verify/verify_merge.py` | `.claude/reviews/verified_findings.{json,md}` | CONFIRMED 152, CONFIRMED-BY-SONNET-ONLY 10, CONTESTED 22, REFUTED 76 | 0 |
Total CLI-reported about $9.2. Wall clock about 2.5 h including waits. The merge scripts' own list-rate estimate ($3.01 for verify) is
about 2.5x low for Sonnet: budget with CLI-reported numbers.

Plan doc: `.claude/plans/master_review_audit_plan.md` (sections 1-12). Format contract: `.claude/review/INTERFACES.md`.
Fix-session prompt: `.claude/plans/fix_stage_session_prompt.md` (already sent to another session along with a course-correction blurb).
That session is building tooling in `.claude/review/fix/` (Phase A only; no model calls until the user says go).

## 2. Key results worth remembering
- Confirmed real examples: `queue_hud_src` exports/hooks `QueueHudPerformCastSpell` with no definition (link fails);
  `scripts/compat_check.py run_build_mods()` ignores build return code; `generate_diversity_2da.py` unconditional premerge backup;
  `AGENTS.md:76` orchestrator-autonomy wording has no carve-out for `kpm_hotswap.py --apply`/GitHub posting (Haiku map finding).
- Recall vs the known-issue key (lives only in `merge.py::_known_issues()`): Map found K1, K2, K3b, K4; missed K3a (`npc_diversity_src/manifest.toml:7`
  kind mod->patch in 83354c0). After Verify: K2, K4, K3a were REFUTED by both stages = **verifiers over-refute subtle design-level risks**
  (agreement: cpp-logic 86%, build-config 73%, python-tooling 94%). Do not treat the REFUTED tier as safe to discard.
- Decoy design flaw: 3 of 8 decoys came back "confirmed" because the decoy claim text named the real file/line; Haiku refuted at the cited
  location, Sonnet said "cited lines wrong" then confirmed at the true location. Fix for next time: strip file/line names from decoy claims or
  make decoys wrong in content, not citation.
- Duplicates exist in the ledger (same root cause under different ids, e.g. 5cacb52504ba / c8b1757d9b9a): any fix intake must cluster first.
- Human-only leftovers (in `withheld_leads.md`): NpcDiversity.kpatch md5 mismatch `c227395c` vs `a6cd5c64`; `raw_addresses.toml` line touched in `83354c0`;
  unresolved `check-shared-headers` FAIL; perf_lab harness include errors (probably a scratch-copy artifact).
- Pending user decision: propose deleting 11 temp handoffs (`.claude/handoffs/tmp*_plan.handoff.md` x10, `99_temp_autoresolve_test_plan.handoff.md`)
  and 2 zero-byte `kotor2_ghidra_knowledge.db` (repo root, `scripts/`). User said "leave the file(s)" for now. NOT done.

## 3. Lessons / gotchas (CLI 2.1.296)
- `--json-schema` rejects a `$schema` key; CLI adds a `StructuredOutput` tool; `--max-budget-usd` is enforced under OAuth; no `--max-turns`; `--bare` breaks OAuth.
- Lockdown flags that work: `--restricted --safe-mode --settings <deny file> --tools "Read,Grep,Glob" --permission-mode dontAsk --max-budget-usd N --json-schema ... --strict-mcp-config --mcp-config '{"mcpServers":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <file>`, cwd = sanitized tree outside the repo.
- Canary must include a **deny probe** with a plain sandbox prompt; otherwise the worker prompt makes Haiku refuse by itself and the deny rules are never tested.
- Haiku sometimes fails with `error_max_structured_output_retries` (one re-queue fixes); occasionally fabricates quotes (always run the deterministic quote check).
- Haiku 5.5 packets: first packet is a sizing probe (`run_map.sh --first-only`, then `prep.py resize`). Calls cost ~$0.006-0.03 each; 8 parallel hit no rate limits; 5 parallel Sonnet was fine.
- Haiku/Sonnet pricing used: Haiku <=100K $0.10 in / $0.125 cache-write / $0.01 cache-read / $0.50 out per MTok; Sonnet $2 / $10 / $0.10 hit.
- A second runner on a disjoint `--only` list is safe alongside a serial run (each unit checks for a finished result first).
- `prep.py is_excluded` returns False for a root-level `.env` (`lstrip("./")` -> `env`): unfixed; verify scripts guard. The sanitized tree had no secret-pattern files (asserted with `find`).
- `verify/select.py` name clashes with stdlib `select`: scripts append (not prepend) their dir to sys.path.
- Blind-recall discipline mattered: K-issue hints stripped from worker prompts/domain notes; answer key only in `merge.py` / `work/verify_key.json`; explanations only in `withheld_leads.md` for Reduce.
- Test pollution landmine: unscoped tests write the production ledger (K1). `clef_dataset.py` / `shadow_replay.py` ignore `MASTER_LEDGER_PATH`. Ledger hash before/after fact runner: `73c49d59df5103c4b33070ac95f549f2e595a1d3f2c6c029e264e7aba594cf8f`.

## 4. Next: design a `deep-haiku-review` skill (investigate after compaction)
Goal: one skill encapsulating the whole flow, parameterised by repo/scope:
1. Opus/main: scope + split into work packages, sanitized snapshot, answer-key setup, domain notes (Phase 0, deterministic `prep.py`-style).
2. Canary (with deny probe) -> first-packet probe -> resize.
3. Army of Haiku Map -> Sonnet Reduce (per domain) -> deterministic merge with coverage gate.
4. Army of Haiku Verify (specialty-routed, blind, quote-checked) -> Sonnet adjudication batches (devil's advocate on refutations) -> verify merge with calibration (controls + decoys).
5. (Other session's pieces) Haiku fixers in isolated per-task copies -> deterministic checks -> blind Haiku reviewers -> Sonnet integrators run scoped build/tests -> fix merge/report.
Design questions to settle: which parts are skill instructions vs. a shipped script bundle (skill dir with `scripts/`, templates for prompts/schemas/settings);
repo-agnostic parameters (boundary/base/pinned HEAD, WP definitions, exclusion list, specialties, DOMAINS notes, answer key optional); where run artifacts live;
approval gates (canary pass, first-packet, budgets, go before fix stage, never commit); stage budgets from observed costs; fix known gaps (decoy design,
over-refutation mitigation e.g. a Sonnet "refutation audit" of cpp-logic, `.env` guard, adj-pending/log hygiene, CLI-reported vs estimated cost).
Existing skills to model on: see `.claude/skills/` and `scripts/check_skills.py` (project skill linter gates skills). Check `kotor2-*` skills for conventions first.

## 5. Resume
- Re-read this file, then `less .claude/reviews/verified_findings.md`.
- Check the other session's progress: `ls .claude/review/fix .claude/plans/fix_pipeline_plan.md`.
- Resume command: `python3 -I .claude/review/verify/verify_merge.py` (deterministic, safe, reruns byte-identical) or start the skill investigation.

## 6. Update 2026-10-10 (skill build)
The "Next: design a skill" section above is now superseded: see `.claude/handoffs/deep_haiku_review_skill.handoff.md` (global skill built under `~/.claude/skills/deep-haiku-review/`, pilot p1 in flight, v2 ledger `.claude/reviews/verified_findings.v2.json` adds confirmed K1 + K3b that the original tiers missed).
