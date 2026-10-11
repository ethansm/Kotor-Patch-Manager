OUTPUT: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/deep-review/phase3b/recon/R8_bug12_verify.md
EVIDENCE: .claude/review/fix/fix_settings.py:109-111 hides ~/Games via `--tmpfs`; fix_settings.py:115-117 gate profile `--ro-bind kpm kpm` runs after the hides so the later mount wins; .claude/review/fix/fixgate.sh:95 `gitc clean -fdxq -e _patches -e _resolved`
FINDING: Gate profile re-binds KPM_DIR read-only after the ~/Games tmpfs hide, so it is visible; self-test (lines 181-212) has no KPM check.
OPEN: Is the KPM_DIR env override under another hidden tree intended, and should the self-test assert KPM visibility?
VERDICT: CONFIRMED
