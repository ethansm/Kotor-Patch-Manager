# p2c fix-half readiness (find-only, $0)
No fix gate was passed (`findings-skim`, `fix-go`, `fix-pilot`, `fix-apply` all still pending). Nothing was spent. `profile_p2c.toml` and `verified_findings.json` are untouched.

## Files (this dir)
- `make_live_findings.py` -> `verified_findings.live.json`: original minus STALE-FIXED `8ea1731f8816`. CONFIRMED 18, SONNET-ONLY 5, CONTESTED 4, REFUTED 13.
- `profile_p2c_fix.toml`: copy of the p2c profile (`repo` -> `../../../..`, notes -> `../../pilot/notes`) + `phase3f/fix_section_kotor2.toml`. Same run dir and spend as p2c (13 of 19 stages: the 6 extra are the fix half). `fix_total` is NOT set here; the ceiling is yours.
- `intake_stats.txt` (free `fix_intake.py --stats`), `intake_by_id.txt` (per-finding class).

## Intake: 27 findings = 23 fixable tasks + 4 contested
tasks by class: auto-fixable 3, needs-design 13, human-only 7. Flags: gate_semantics 1, sandbox_tests_only 2, relocate 2, needs_live_check 0. 2 waves.
- **auto-fixable (swarm can take)**: BG13 abi_check.py, BG14 ultrawide registration_check, BG19 Inventory3DViewport warning cap. = BG13, BG14, BG19.
- **needs-design (your call before a fixer)**: BG04, BG05, BG06, BG07, BG08, BG10, BG11, BG12, BG15, BG17, BG18, BG20, BG24.
- **human-only (swarm will not touch)**: BG01, BG02, BG03 (all three high; BG02 is `agent_diff_check.py`, never_edit + gate_semantics), BG09, BG21, BG22, BG23 (evidence cites a never-edit/excluded path).
- **contested -> human list**: BG16, BG25, plus 2 UNCLEAR not on the board (`843bd1103025`, `5a447a280525`).
- Correction to the triage note: `hook-asm`/`cpp-logic`/`gate-integrity` in `specialty_names` is NOT needed; these findings use builtin specialties only.

## Still yours
1. `python3 $B/dr.py approve findings-skim --note "<what you checked>" --run p2c --profile .claude/deep-review/p2c/fix/profile_p2c_fix.toml`
2. Set a fix ceiling (`fix_total` in `[budgets]` of `profile_p2c_fix.toml`; sample value 25 is in the phase3f profile, far above need for 3 auto-fixable tasks).
3. Then `approve fix-go` with the same `--run/--profile` flags. Only after reading this file.
4. Decide if BG01-BG03 are fixed by hand (recommended: all human-only anyway).
