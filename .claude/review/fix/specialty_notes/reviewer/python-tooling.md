## Specialty lens: python-tooling (reviewer)
- New exceptions: are they caught where the old code relied on them not happening? Does an error now abort a loop that used to continue (and should)?
- Paths: no new absolute home paths, no writes to the live game dir, ledger or install paths.
- returncode checks: is the check on the right process object, and does failure propagate to the exit status?
