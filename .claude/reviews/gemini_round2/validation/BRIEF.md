# Gemini round-2 finding validation (Haiku unit brief)

You validate findings from an external AI code review (Gemini). Past rounds: most "high" claims were inflated, many
memory-safety claims were true-in-code but unreachable (latent). Be a skeptic: confirm only what the code shows.

For EACH finding in your slice:
1. Read the finding text: `sed -n '<range>p' <review file>` (your slice only).
2. Check the Anchor exists: `rg -n -F '<anchor text>' <path>`. Note if the line moved or the text is gone (already fixed?).
3. Read the code yourself with targeted reads (`rg -n`, `sed -n 'X,Yp'`, max ~80 lines per read). Follow callers/callees
   the claim depends on. Check whether a guard elsewhere (saferead, clamps, early returns) already blocks it.
4. Check the proposed Fix is correct and minimal (would it compile, does it break other callers).
Never edit source files. Never read *credential*/*secret* files. Do not run the game or install anything.

Write ONE TSV file (header + one row per finding) at the OUTPUT path in your prompt. Columns, tab-separated:
fid	verdict	real	severity	anchor_ok	evidence	why	fix_ok	fix_note
- verdict: CONFIRMED | REFUTED | PARTIAL | UNSURE
- real: reachable (happens in normal play) | latent (true in code, not reachable) | doc (doc/test/comment only) | refuted
- severity: your own call: high | medium | low | none
- anchor_ok: yes | moved | missing
- evidence: 1-3 path:line anchors, ';'-separated
- why: <= 30 words, the deciding fact
- fix_ok: yes | no | partial | n/a ; fix_note <= 20 words
No tabs or newlines inside cells.

Then reply with exactly the 5-line RECON schema:
OUTPUT: <your tsv path>
EVIDENCE: <1-3 anchors>
FINDING: <<=25 words, e.g. "2 CONFIRMED reachable, 1 REFUTED">
OPEN: <one question or None>
VERDICT: <CONFIRMED | REFUTED | PARTIAL | UNSURE>  (overall for your slice)
