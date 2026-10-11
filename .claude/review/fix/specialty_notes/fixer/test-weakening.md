## Specialty: test-weakening (fixer)
- The defect is usually a test that passes when it should fail: missing assertion, buffer too small for the message, macro that hides the failing value, early return, wrong expected value.
- Restore strength: add or tighten an assertion, never remove one. The file's assertion count must not go down.
- Keep the test framework already used (KT_CHECK / CHECK macros in tests/include, unittest in Python). Do not add new dependencies.
- If the expected value itself is in doubt, report not-fixable with the reason.
