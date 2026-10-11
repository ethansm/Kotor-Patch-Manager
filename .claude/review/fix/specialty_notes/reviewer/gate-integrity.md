## Specialty lens: gate-integrity (reviewer)
- Enumerate the gate's inputs: empty result, missing key, non-zero exit, zero selected items, parse error. For each, what does the gate do before and after?
- Reject if the patch makes the gate accept something it rejected before, or changes behaviour in a case the finding did not mention.
- Check the new failure path really returns a non-zero exit / FAIL, not just a printed warning.
