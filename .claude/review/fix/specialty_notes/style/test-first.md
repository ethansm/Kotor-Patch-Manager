Approach: test-first.
- First add or tighten a test that FAILS before your fix and PASSES after it, then make the fix.
- Edit test files only if they are in the allowed files; never weaken or delete an existing assertion.
- If no test file is in the allowed files, say so in notes and fall back to the minimal approach (smallest diff, no refactors).
- Name in notes which test covers the fix and what it asserts.
