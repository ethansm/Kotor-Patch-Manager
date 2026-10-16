# Gemini round-2: Sonnet verify + consolidate (one review file per agent)

Inputs: the review file `responses/<name>.md` and the Haiku unit TSVs `validation/<name>_u*.tsv`
(columns: fid verdict real severity anchor_ok evidence why fix_ok fix_note). Haiku verdicts are a first sieve, not truth.

For EACH finding fid:
1. Read the Gemini finding and the Haiku row(s).
2. Independently re-check the deciding fact in the code (rg -n, sed -n 'X,Yp'; follow callers/guards). Prioritise:
   (a) any CONFIRMED/reachable claim (most expensive if wrong), (b) PARTIAL/UNSURE rows and Haiku OPEN questions,
   (c) REFUTED rows (spot-check that the refutation's evidence really says that).
3. Settle: verdict (CONFIRMED|REFUTED|PARTIAL), real (reachable|latent|doc|refuted), severity (high|medium|low|none),
   impact (crash|wrong-behaviour|visual|log-only|test-gap|doc), and whether Gemini's fix is right; if not, give a
   corrected fix sketch. Note when you overturn Haiku and why.
Repo policy facts: mod manifest versions stay 0.X.Y until the public PR (a fix that bumps to 1.x/2.x is wrong);
runtime addresses go through k2addr/raw_addresses.toml; memory reads use saferead. Live-only checks are not blockers;
mark them `play-test` in the gate column.
Never edit source, tests or docs. Write only your OUTPUT file. Never read *credential*/*secret* files.

OUTPUT: `validation/<name>.consolidated.tsv`, tab-separated, header then one row per fid:
fid	verdict	real	severity	impact	haiku_agree	evidence	why	fix	gate	title
- haiku_agree: yes|no ; evidence: 1-3 path:line ';'-separated ; why <= 35 words ; fix <= 35 words (corrected sketch or "Gemini fix OK")
- gate: failing test to add, or play-test, or - ; title: Gemini's title, shortened. No tabs/newlines in cells.

Reply with exactly the 5-line RECON schema:
OUTPUT: <path>
EVIDENCE: <1-3 anchors>
FINDING: <<=25 words: counts by real class, how many Haiku verdicts overturned>
OPEN: <one question or None>
VERDICT: <CONFIRMED | REFUTED | PARTIAL | UNSURE>
