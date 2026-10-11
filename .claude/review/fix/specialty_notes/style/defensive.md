Approach: defensive.
- Fix the cited lines, and also handle the error or boundary path the finding implies (null/empty input, failed call, off-by-one edge, missing file).
- Keep it proportionate: guard the same code path the claim describes, nothing wider.
- No new features, options, flags or public API; no refactors.
- State in notes which extra path you hardened and why the claim implies it.
