# Work-item sizing rubric (S / M / L / XL)

Size = effort for an agent to finish the item, not lines of code. A 5-line fix behind a Ghidra hunt is bigger than
a 150-line mechanical edit. Haiku scorers give the five axes; the size comes from the rule below, never from the
scorer's gut. Scores live in `.claude/work_items/sizing.tsv`; only the size letter goes in the WI header and index.

## Axes (score each from the WI file plus a quick look at the named code)
- **R research** (weighted x2)
  - 0: the WI names the fix site (file:line or function) and the cause.
  - 1: the cause is known, but the site must be found (rg, reading 2-5 files, one log or dump).
  - 2: the root cause is unknown, or it needs Ghidra/REA/static dumps, engine offsets, log forensics, or a design comparison.
- **C change**
  - 0: <=40 changed lines in 1-2 files.
  - 1: 41-150 lines, or 3 files.
  - 2: >150 lines, >=4 files, or a new module.
- **V verify**
  - 0: an existing named suite already covers it (rerun only).
  - 1: one new or extended unit test.
  - 2: a new fixture, harness, fake-engine scenario or mutation run, or a mod DLL build plus host tests.
  - 3: only a live game run or play test can confirm it.
- **K risk**
  - 0: local to one mod or script.
  - 1: shared header (`_shared_src`, Common, KpLog), a library several mods use, a hook or patch address, or the WI/claim tooling itself.
- **U owner**
  - 0: no open decision.
  - 1: the WI needs an owner decision or design choice before work can start.

## Size rule
points = 2R + C + V + K (V counted as 0-2)
- S: 0-1 · M: 2-3 · L: 4-5 · XL: >=6
- XL also when V = 3 (only a live game or play test can confirm it: not agent-finishable in one pass).
- U = 1 is a flag, not a size: the item waits for an owner answer. (10-15 run: scorers set U = 1 on 55 of 80 open
  items, so "U -> XL" made 60 of 80 XL and was dropped.)

Calibration (10-15, 10 done items scored blind against runs.jsonl effort): 3/10 exact tier, 5/10 within one tier.
Weak: done items were scored on post-fix code. Next calibration should score the pre-fix tree.

Examples: named site, 20 lines, existing suite -> 0 = S. Unknown cause needing a dump, 5-line fix, new test
-> 2*2+0+1 = 5 = L. 200-line refactor of a shared header with a new fixture -> 2+2+1 = 5 = L.

## Fix-wave use
Planned (not built yet): `fix_wave.py select` skips XL, takes at most 2 L per batch; R = 2 items get all 3 Haiku
variants.

## Scorer reply (one line, TAB-separated, nothing else)
`WI-NN	R	C	V	K	U	conf(H|M|L)	why (<=25 words, name the fix site or what is unknown)`
