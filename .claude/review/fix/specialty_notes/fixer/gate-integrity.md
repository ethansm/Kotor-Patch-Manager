## Specialty: gate-integrity (fixer)
- Gates are scripts whose exit code or PASS/FAIL line decides something (build_check, run_tests, compat_check, agent_diff_check).
- Typical fixes: check a subprocess returncode, fail when a result file is empty or a filter selected zero items, stop treating a missing status as PASS, stop swallowing a parse error.
- Fail closed only in the exact case the finding describes. Do NOT change what a gate accepts in any other case; if the right behaviour is a policy choice, report not-fixable.
- Keep messages and exit codes consistent with the rest of the script (Reporter FAIL/WARN conventions).
