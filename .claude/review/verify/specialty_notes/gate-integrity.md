Domain checks for gate, check and policy claims. Apply the ones the claim touches.
- Fails closed: when the check cannot run, cannot read its input, or hits an exception, does it return a failing status, or does it silently pass? Look at `except` clauses, default return values and early `return 0`.
- Return codes: are subprocess return codes and exit statuses read and acted on? Is `check=False`, a discarded result, or `|| true` hiding a failure?
- Selection logic: can a filter, a skip list or a "changed files only" rule drop the files the gate exists to check?
- Can the gate still fail: is a comparison always true, a threshold lowered, a required list emptied, or a failure demoted to a warning?
- State written to production paths: does the gate or tooling write, copy, install or hot-swap anything outside a scratch location, or reach the live game directory? Quote the path construction.
- Policy text (instruction documents): compare the cited sentence with the rules stated elsewhere in the same file; a rule is only loosened if the later text removes or overrides an earlier requirement, not if it merely adds scope.
- Waivers: is a failure dismissed as pre-existing or expected without a written, dated waiver?
