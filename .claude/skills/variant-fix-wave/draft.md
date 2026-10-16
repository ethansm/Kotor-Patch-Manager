Recon draft for {WI}. Turn one candidate work item into one fix-wave row, or mark it UNFIT.
Read-only except {ROW}. Run nothing except `python3 scripts/run_tests.py --list`.

Read: .claude/work_items/{WI}.md (sections Goal, Steps, Verify). Then open only the line windows
its files name, and use rg for anchors. Never cat a whole source file.

Pre-filled from select (write= is the WI's declared files; tests= are short names; correct a cell only if it is wrong):
{PREFILL}

Write {ROW} as exactly one of A or B.

A. Row: a header line, then one row. Columns are TAB-separated, not spaces:
id	kind	entries	anchors	tests	constraint	playtest
{WI}	bug|gap	<entries>	<anchors>	<tests>	<constraint>	<playtest>
- kind: bug = behaviour is wrong today; gap = behaviour is right but untested.
- entries: at most 5 `path` or `path:lo-hi` (ints, lo<=hi), `;`-separated, fix site first.
- anchors: `;`-separated rg patterns that hit today in the entry files. Run each rg before you write it.
- tests: keep the pre-filled names unless one is wrong. A new name must be an id printed by run_tests.py --list.
- constraint: at most 6 sentences: the exact behaviour required; the test that is red before the fix; what must not change; what is deferred. No tabs or newlines inside it.
- playtest: `-` when host tests prove the fix. When only the live game can confirm it, do NOT mark UNFIT: write
  `steps: <what the player does> | signal: <log line or kotor2_logs.py check result that proves it>` (at most 3
  sentences, no tabs). It goes to patch_manager_mods/PLAY_TEST_QUEUE.md for the next batched play session.

B. UNFIT: one line, `UNFIT: <reason>`, and nothing else in {ROW}. Use it when any of these hold:
- the fix needs an owner decision or a design choice;
- the fix needs edits to more than 4 files;
- the fix site cannot be found in the repo;
- the WI is docs-only.

Caps: at most 5 entries and at most 6 sentences in constraint.

Reply: exactly these 5 lines, in this order, nothing else. Write them to the reply file the brief names.
OUTPUT: {ROW}
EVIDENCE: <1-3 anchors path:line, ;-separated>
FINDING: <the decision in 25 words or fewer>
OPEN: <one open question, or None>
VERDICT: CONFIRMED if A (row); REFUTED if B (UNFIT)
