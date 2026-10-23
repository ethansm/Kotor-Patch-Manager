# rv-qac validation pass (Sonnet): shared brief

You validate and aggregate Haiku review findings for two KOTOR II mods (queue-hud, action-hotbar).
Read `.claude/waves/rv-qac/BRIEF_COMMON.md` first (engine facts, trust them). Then read your assigned Haiku files.

For EVERY Haiku finding in your files:
1. Open the cited lines (use `sed -n 'X,Yp'` or `rg -n`; never cat whole large files) and check the claim against the actual code.
   Follow the call chain as far as needed (callers, config defaults in queue_hud.ini / HotbarConfig.h, tests in *_test.cpp).
2. Verdict: CONFIRMED (code provably does this and the scenario is reachable), PLAUSIBLE (code does it but reachability
   depends on an unverified engine behaviour; name it), REJECTED (claim wrong, unreachable, already handled, or style nit; give the line that refutes it).
3. Re-grade severity: high = wrong game action (double use, lost/wrong cast, stuck queue needing Cancel, crash risk);
   med = visible misbehaviour with a workaround or rare path; low = logging/docs/cosmetic.
4. Merge duplicates across your files (and note IDs of likely duplicates in other areas if you see them).

Haiku is often wrong: about 1/3 of claims may be refuted. Be skeptical, but do not reject a real defect for vagueness; restate it precisely.

Output file (write ONLY it), markdown:
```
# <V-id> validated findings (sources: hNN, hNN, hNN)
## <V-id>-<k> [high|med|low] <CONFIRMED|PLAUSIBLE> <category>
- Sources: h02-F1, h12-F1
- Where: path:line (anchors ';'-separated)
- Defect: <precise 1-3 sentences, correct field names>
- Scenario: <concrete steps -> wrong result>
- Fix: <specific change: function, condition, new config/flag; 1-4 sentences>
- Test: <unit test vector in the named *_test.cpp (pure logic) and/or PLAY row: steps + log pass signal>
- Engine question: <unverified assumption, or None>
...
## Rejected
| Haiku ID | reason (1 line, with refuting path:line) |
```
Order confirmed findings most severe first. Keep the file under ~400 lines.
Then reply with exactly the 5-line RECON block (OUTPUT, EVIDENCE, FINDING, OPEN, VERDICT); no prose.
Read-only for everything except your output file. Do not build, do not run git.
