# bug-investigate M4 session review (2026-10-13, 8 Haiku recon agents, $0.62 est.)

Scope: review of the M4 measurement session (`.claude/reviews/bug_investigate_m4_measurement.md`) by 8 parallel Haiku 5.5 recon agents, one lens each. Raw findings: `.claude/reviews/bug_investigate_m4_review/u1.md` to `u8.md` (U1 gate compliance, U2 worker briefs, U3 errors, U4 run-record data, U5 recipe edge cases, U6 quote gate, U7 scorer validity, U8 runner cap). Verdicts: U5 and U6 CONFIRMED (U6 with synthetic probes), the other six PARTIAL. Nothing in this note was re-run by the lead except where marked.

## Verdict
Mechanics worked: 68 of 68 units completed on attempt 1, 0 timeouts, 0 schema failures, all costs known, $0.704 spent of $1.90 ceilings. Validity is weaker than the M4 note says, the verify layer was mostly unmeasured, and the recipe has silent-state edge cases.

## Findings (ranked, fix id in brackets; ids are tracked in the master plan section 18)
1. **Verify layer mostly unmeasured, silently** [R1]. Runner admits a call only while settled spend <= `cap_total - cap_call` (`swarm_run.py:532-537`, `budget.py:132-142`), so `$0.10/$0.05` admits 2 to 3 calls (U8, arithmetic model; `--parallel` does not change the count). 51 of 79 units never ran, all 25 skeptic units among them. `report` still says `ranked` and exits 0 (`bug_investigate.py:514-515`, `:725`); the reason is "no record", not "budget deferred" (U4, U5). Safe rule: `cap_total >= N_pending x cap_call`; the real ceiling `REAL_MAX_TOTAL_USD=1.00` (`swarm_run.py:47`) bounds N at 20 for `cap_call 0.05`. Resuming 51 pending units at about $0.022 each is about $1.1, so it needs two stages or a ceiling decision.
2. **Rubric labels are in-sample** [R4, R5]. Both rubrics were edited after seeing the hand samples; the "fresh" 8/8 sample was the tuned one. Held-out agreement was 7/8 on both cases (U1, U3, U7). Matching defects: no negation window ("is NOT an invalid id" matches), `companion` and `follower` also match the symptom text the units were shown, a hotbar HIT credits the mechanism without the multi-member trigger (the log has one party member), and one keyword makes a PARTIAL.
3. **M4 note overreaches** [R5]. "2 of 79 removed" should read 2 of 28 verified; "correct rejections" and "null abstains correctly" rest on rubric labels and on a null unit that said `insufficient` 10 of 10 with no not-a-bug control; "HIT at rank 1 every time" is a mechanism-only label on one bundle. Pooled Wilson intervals assume independence across hypotheses that share a bundle (U7).
4. **Quote gate proves presence, not relevance** [R3] (U6, probed). `rank_key` sorts on `len(valid_evidence_ids)` (`bug_investigate.py:478`): padding with irrelevant verbatim quotes, or citing two identical-text excerpts, raises rank. `L<n>|` markers are stripped, never validated (`evidence_quote_check.py:57`). `MIN_CHARS` counts punctuation (`"------"`, `"11:05:33"` pass, `:140`). A lone surrogate in an evidence id crashes the CLI print (`:246`, still a reject).
5. **Recipe state edge cases** [R2, R3] (U5, read from code, not run). Stale `out/` is trusted when `blind` fails its probe after `write_units` (`:626-627`, `:331`); `bundle` re-run after stages overwrites it (`:611`); a verify verdict binds on the generic hypothesis id only (`:458`, `:467-471`); null validity uses `all([])` so zero citations count as valid (`:495`, `:503`); the forbidden-term check is substring only (NBSP, zero-width, line breaks, `_`/`-` bypass it) and `verify` never runs it; no per-run lock or run-level spend ledger; a malformed complete unit is silent; no resume of failed units; `bundle` accepts a missing launch with zero excerpts.

## Common patterns
- Incomplete by default: stub gaps (`error_no_script`), verify cap, bundle truncation all ended in a run that looked finished.
- Tuning on the test sample, then quoting the tuned agreement (twice).
- Brittle ids and paths: short `ev:<sha8>` ids, a hand-typed run id missing `2026`, `fix_checks` hashing a live ledger another session writes (race cause inferred, one green re-run does not prove it).
- Six python-heredoc file edits against CLAUDE.md; the first note draft contained a hand-typed "120/128" that the aggregate JSON did not support (caught before commit).
- Worker briefs (U2): A1 clean; A2 had an ImportError as its "red", wrote to `/tmp` before `TMPDIR` was set, made 12 edits after green (about 40% of its calls) and read whole files (about 130k tokens vs 100k target).

## Do-not-trust list (inferred, not verified)
- U4 "69 to 92% idle gap before verify": probably the human wait for the G3 yes plus scoring and hand checks, not a runner defect.
- U3 "Monitor loop `pgrep -f` self-match never exits": inferred, no expiry event in the digest.
- U1 "G2 and G3 asked together before BG09 data": a plan-order deviation if true (plan said G3 after B1/B2); it was still an explicit yes with a named ceiling.
- `agent_contract.py check` printed FAIL for this wave only because the scope glob was wrong; the agents wrote to the scratchpad only.

## Fix packages (full plan: see the plan file named in the master plan section 18)
R1 verify budget sizing and visibility; R2 stage integrity; R3 validation gates (null, forbidden terms, quote rank); R4 scorer validity; R5 measurement protocol and docs; R6 tooling and worker-brief templates. Paid work: none planned in R1 to R5 except one gated verify resume (needs an explicit yes with ceiling).

## Cost of this review
8 Haiku agents, about $0.62 (estimate, `agent_usage.py`), all peaked 107k to 131k context, above the 100k guideline. Lenses were too wide; split by file next time.
