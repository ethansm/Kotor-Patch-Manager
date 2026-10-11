## Specialty lens: test-weakening (reviewer)
- Count assertions before (_pre/) and after. Any removed, loosened (== to >=, exact to approximate), skipped, or commented-out check is a reject.
- Does the strengthened test now actually fail on the defect it targets? A tautology (assert x == x) fixes nothing.
- Message-buffer changes: make sure the full message still reaches the failure report.
