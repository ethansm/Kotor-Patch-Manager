Domain checks for claims that a test or its configuration was weakened. Apply the ones the claim touches.
- Removed or changed assertions: for each one the claim names, search the same file and its siblings for a replacement assertion on the same behaviour before accepting that coverage was lost. Quote the replacement if you find it.
- Tolerances and expected values: was a tolerance widened, an expected value changed to match new output, or an `EXPECT`/`CHECK` turned into something that cannot fail (always-true condition, result never compared)?
- Skips and gating: is a test disabled, wrapped in `#if 0`, marked skip/xfail, filtered out by a selection list, or made conditional on an environment variable?
- Name vs content: does the test still exercise what its name says, and does the cited line assert on the value the claim says it ignores?
- Variable flow: for claims about a flag or out-parameter, read the lines between the call and the assertion to see whether the variable is actually set by that call.
- Contract tests: does the mod's `common_contract_test.cpp` (or equivalent) still pin the APIs the claim says are unpinned?
- Fixture and config files: check whether a changed entry is justified by a nearby comment or by the code it covers.
