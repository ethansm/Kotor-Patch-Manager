# WI-20c RB keep-nodes patch at 0x4957AA (target unknown, C112)
area: engine-optimization-research
size: S
ready: hold
items: C112
files: none (target file not named)
out-of-repo: none
blockers: Target file unknown. 0x4957AA is not in any repo .cpp or .h (rg empty; hits only in research dumps). Doc 10 RB table row is at line 387 ("per-frame bucket map rebuild"); the checker's line 207 is an unrelated U4 entry. The 30-byte keep-nodes wording needs owner confirmation. M5 data reference (doc line 159): unknown, not checked. Build gate after the target is named: unknown.
docs: patch_manager_mods/10_engine_optimization_research.md:C112@L387
deferred: none

## Goal
Hold C112 until a target file is named. Do not create a file for it.

## Steps
1. (C112) Report BLOCKED: no target file is named for the RB patch at 0x4957AA.

## Verify
None until the target file is named.

## Docs
- Keep C112 open.
- CHANGELOG bump: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C112 patch_manager_mods/10_engine_optimization_research.md:387 id=RB open
