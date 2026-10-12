# Reduce: gate_tests

Verdicts: confirmed 3, downgraded 10
Cross-file findings: 2

## Verdicts

| finding | status | sev | conf | reason |
|---|---|---|---|---|
| 436dae85e39d | downgraded | medium | medium | AGENTS.md:14 still requires GO before --apply; line 76 only exempts the plan/review/build delegation chain from human approval pauses and does not mention --apply. It is an ambiguity that could be read as overriding the GO rule, not an explicit override. |
| c8b1757d9b9a | confirmed | high | high | Grep of queue_hud_src finds QueueHudPerformCastSpell only in exports.def:6 and kotor2-steam-aspyr.hooks.toml:31. There is no definition, so the linker fails on the export, matching the known build_check ld error. |
| 48c14e9c7358 | downgraded | medium | high | The docstring (line 11) promises a static-invariants step and main() (123-235) has none. The claim is correct, but it is a documentation/coverage mismatch, not a wrong verdict in code that exists. |
| 27458c270f8d | downgraded | medium | high | run_tests returns (True,'No specific tests requested.') for empty --test (98-99) and the PASS message at 194 says 'tests passing'. Behaviour is as described, but the flag is optional by design; the defect is the misleading message and a PASS with zero checks. |
| 4f54a05c5934 | downgraded | medium | high | Scope is checked only when --scope is given (151-152) and PASS still prints 'scope clean' at 194. The wording is misleading, but --scope is opt-in, so it is not a silent bypass of a requested check. |
| a57cf0cc6866 | downgraded | medium | medium | narrow_plan_to_gog_scoping (337-359) does drop non-build tests, clear gaps and add a note with no Steam check. This is the documented intent (docstring) and is gated by an is_k2addr_gog_only_diff caller. The risk lies in the accuracy of that predicate, which I did not read here. |
| 72bae8dcfdca | confirmed | high | high | b_proc is created at 1007-1015 and only its stdout is parsed with PASS/FAIL regexes (1017-1051). returncode is never read, so a crash or non-zero exit without a 'FAIL build:' line adds no FAIL or ERROR. |
| 20bde0dc5b32 | downgraded | high | high | NamedTemporaryFile(delete=False) at 1068 guarantees the file exists, so the is_file() check at 1089 is always true. An empty or invalid JSON makes the except at 1112 swallow the error, the else branch at 1114-1116 is dead, and proc.returncode is ignored. A crashed run_tests therefore yields no FAIL  |
| dca7571f614e | downgraded | medium | medium | Line 1094 does default a missing status to PASS. This only matters if run_tests.py emits results without a status; I found no evidence it does. It is a fail-open default, so I judged it a hardening issue. |
| 84259063375f | downgraded | medium | high | A suites.toml parse failure does 'except Exception: return' (444-447), silently dropping suite edges. This makes the plan under-select tests on a malformed file, but it takes a corrupt tracked TOML, and the failure is loud elsewhere (the suites are used by run_tests). |
| 66025cc973bd | downgraded | medium | medium | The substring logic at 509/518 does match everything when m_name is '' (name default ''), and the first dict-order hit wins. It requires a manifest with no name, and the id default is a non-empty path, so impact depends on whether such manifests exist. |
| 19f1a3e76f85 | downgraded | medium | high | include_build_tier and include_wine (856-860) are computed before positional filters are merged into target_ids (864-868), so positional 'wine:x' or 'suite:build-check' will not enable those tiers. The --id and --ids-file routes are processed earlier (830-833) and work, and compat_check uses --ids-f |
| 4c1cb3bd2726 | confirmed | high | medium | Line 871-872 filters items to target_ids with no check that anything is left. A grep for any no-tests or empty-selection guard in run_tests.py found none, so a stale or mistyped id gives '0 passed' and exit 0 unless the later code treats it differently (I did not read the return path at 982-995). |

## Cross-file

- [high/medium] scripts/compat_check.py, scripts/run_tests.py 1068-1116 (gate-integrity): compat_check passes test ids via --ids-file and ignores proc.returncode when the JSON file exists, but is empty (compat_check.py:1089-1113). run_tests.py filters items to target_ids with no empty-selection check (871-872) and exits 0 on zero tests. Both fail open: a stale or typo'd id, or a runner c
- [high/high] patch_manager_mods/queue_hud_src/exports.def, patch_manager_mods/queue_hud_src/kotor2-steam-aspyr.hooks.toml, patch_manager_mods/queue_hud_src/QueueHud.cpp 6 (hook): The export and the H10 hook binding both name QueueHudPerformCastSpell, and the symbol has no definition in queue_hud_src.

## Clusters

- fail-open-gate-runner: Test and build runners ignore subprocess return codes and empty selections, so crashes and nothing-ran cases report PASS. (8 findings)
- agent-diff-vacuous-pass: agent_diff_check reports PASS and 'scope clean/tests passing' even when no scope, test or static check was requested or run. (5 findings)
- silent-parse-fallback: TOML manifest and config parse failures are swallowed with except/continue, silently shrinking coverage. (3 findings)
- queue-hud-link: The H10 hook and export reference QueueHudPerformCastSpell, which has no definition. (2 findings)
- vacuous-contract-tests: Contract tests compare locally defined constants or addresses against literals, so they pin nothing in the real code. (4 findings)
- testharness-include-missing: Parity tests include TestHarness.h from stub/_shared, which is not present in the snapshot. (3 findings)

## Notes

I verified 13 high/critical findings and did not read the symbol-level callers of compat_check's exit path (1176) or is_k2addr_gog_only_diff. The py:x ledger rows are fixture leakage, so no finding was classed as a build failure on that basis. The AGENTS.md:14 evidence was truncated. The human items (kpatch md5 mismatch, raw_addresses.toml, shared-headers FAIL) are not re-verified.
