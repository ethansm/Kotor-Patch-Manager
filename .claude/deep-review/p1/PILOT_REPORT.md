# Pilot p1 report (2026-10-10)

Run: kotor2-pilot / p1, base 902b770..22b56b4, 43 units, 4 packets, 2 domains. 13/13 stages done. Human gate `findings-skim` PENDING (user).
Total CLI-reported spend **$0.1977** of the $1 ceiling (estimate ranges were $0.6-0.9 with Sonnet reduce/adjudicate).

## Cost per stage vs `dr.py estimate`
| stage | actual | estimate (pre-run) | note |
|---|---|---|---|
| canary | $0.0064 | - | variant A |
| map-first + map | $0.0036 + $0.0296 | high $0.057 (map) | 4 packets, 0 failures/re-queues |
| reduce | $0.0039 (14 s) | $0.30-1.25 | **Haiku 5.5** via new `models.reduce` (estimate still prices Sonnet) |
| verify | $0.0602 (16 units) | $0.052-0.216 | 0 fabricated quotes |
| adjudicate | $0.0258 (2 batches, **Haiku 5.5**) | $0.252-1.05 | ~16x below Sonnet estimate |
| refute-audit | $0.0682 (Sonnet, 1 item) | $0.12-0.50 | reference second opinion |

## Gates / integrity
- coverage gate PASS (43 units, 0 gaps). prep-ok PASS. first-packet approved by user. Protected ledger sha256 73c49d59... unchanged before/after.
- Secrets scan (`rg -il raw_addresses|credential|secret|.env`, tree/ excluded): hits only in deny-policy text (worker_settings.json, system prompts, canary.json, snapshot.json) and the worker's own "No secrets were touched" sentence. No file contents leaked.
- Cosmetic bug: runners print "Not complete (re-queue with --only):" with an empty list when everything is complete (verify, adjudicate, refute-audit).

## Findings quality (Haiku-adjudicated)
Tiers: CONFIRMED 6, REFUTED 2, CONTESTED 0, SONNET-ONLY 0. Decoys 8/8 caught (Haiku verifier not fooled once, adjudicator confirmed 0). Controls: none (pilot has no answer key), so recall is NOT measured by controls.
Compare with old Sonnet-adjudicated `verified_findings.v2.json` (same files, overlap +-3 lines, scratchpad cmp.py):
- Agree: LevelUpOverride 1434-35 (CONFIRMED), 1412-14 (CONFIRMED), common_contract_test 163-165 and 196-199 (CONFIRMED), LevelUpOverride 2043-48 (REFUTED). 5/5 where old had the same item.
- Not in old: EquipCharPreview 709-713 (new code from commit 22b56b4), common/unit_gap_test.cpp:130 (REFUTED). EquipCharPreview 592-601 CONFIRMED (low) vs old REFUTED 595-602: not a conflict, the old item described the pre-22b56b4 `animLen`; the new finding is about the new 2.0 s fallback (quotes verified).
- Old items not rediscovered (7) lie outside the 5-commit changed range the pilot reviewed (lines 1, 169-182, 1570, 2250-78 ...), expected.
- Sonnet refute-audit on the one REFUTED hard-specialty item: refute-holds (agrees with Haiku).
Caveat: tiny sample (8 real findings). Needs the larger A/B (see below) before Haiku-only adjudication becomes the default.

## Follow-ups
1. H6: `estimate` still assumes Sonnet for reduce/adjudicate/refute; use measured Haiku costs ($0.0039 reduce, ~$0.013/adjudicate batch, $0.0038/verify unit).
2. H8 DONE: `models.{reduce,adjudicate,refute,fix_design,integrate}` per-stage overrides (empty = `sonnet`); selftests still 80/80 mock, 72/72 real quick.
3. H9 (new, user idea): multiple Haiku "lens" auditors with different specializations over the same packet; findings corroborated by >=2 lenses get a boost.
4. H10 (new, user idea): adversarial build-off. 2-3 Haikus solve the same task with differently styled instructions (kept near/below the 100K-token Haiku tier), extra Haikus compare and merge the best pieces. Applies to adjudication (k independent adjudicators, vote) and to the fix half (competing patches, comparator picks pieces).
5. Larger A/B before default flip: re-adjudicate 3 original-audit batches with Haiku, compare to stored Sonnet verdicts (est. < $0.30, needs user go).

## A/B: Haiku 5.5 vs stored Sonnet 5.5 adjudication (original audit, 8 of 29 batches, 87 items; user go $0.30, spent $0.2113)
Replay: same dossiers, prompt, schema, lockdown flags, sanitized tree; outputs only in the scratchpad (`ab/`, `ab2/`), `.claude/review/` untouched.
- Cost: Haiku $0.178 for 8 batches ($0.022/batch) vs Sonnet $1.91 for the same 8 ($0.239/batch): ~10.7x cheaper (second warm-cache run: $0.011/batch).
- Verdict agreement Sonnet vs Haiku: 65/87 = 75% exact. Haiku more positive (Sonnet refuted -> Haiku confirmed) on 8, more negative on 3 confirmed->refuted; needs-human moves 4+2+4+1.
- Ground truth available for 4 items: Haiku correct 3/4 (decoy e0a8aa228b90 refuted; control K2 confirmed), Sonnet 1/4 (it confirmed that decoy and refuted K2). Both missed control K3a.
- Noise floor (3 highest-disagreement batches, 33 items, Haiku run twice): Haiku vs itself 24/33 = 73%; Sonnet vs Haiku run1 19/33, vs run2 26/33. A single adjudication pass of EITHER model flips ~25% of borderline items run to run.
- Conclusion: Haiku is not measurably worse than Sonnet as adjudicator here, and the single-pass noise argues for k=3 Haiku votes (different instruction styles, H10) at ~$0.03-0.06 per batch instead of one Sonnet pass at $0.239. Route disagreement between judges to CONTESTED/needs-human. Keep Sonnet default only for refute-audit and fix_design/integrate until their own tests. Caveat: 4 ground-truth items; a k=3 vote test with decoys/controls in every batch is the next measurement.
