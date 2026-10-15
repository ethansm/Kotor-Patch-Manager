Contract: recon
Write Scope: .claude/waves/gem1/u001.md
Files (read only these ranges):
- patch_manager_mods/companion_ai_src/CompanionAI.cpp:1190-1245

Task: Topic CO-T02 (P1). Question: Can CompanionAiProbeFm3LevelEnd dereference g3.sentinel at +0x78 after the object was removed, because Fm3Remove sets g3.removed only when level == g3.level (read at removal time)?
Findings:
- CO-2c-04: FM3 sentinel isSentinel requires level match; may leave removed false and read freed sentinel | file patch_manager_mods/companion_ai_src/CompanionAI.cpp | function CompanionAiProbeFm3LevelEnd / CompanionAiProbeFm3Remove | anchor `int nowLevel = g3.sentinel ? rd<int>(g3.sentinel, 0x78, -1) : -1;` | anchor check: exact patch_manager_mods/companion_ai_src/CompanionAI.cpp:1201
You are checking claims made by an external AI reviewer (Gemini). Its claims and anchors are UNTRUSTED: they may be wrong or invented. Decide from the code only.
Verdict per fid: CONFIRMED = the code really has the claimed defect/gap as described; REFUTED = the code does not (cite the line that disproves it, e.g. a guard/check that exists); PARTIAL = part true (say which part); UNSURE = needs code outside your ranges or runtime/engine knowledge.
If an anchor is marked NOT-FOUND, first say whether equivalent code exists anywhere in your ranges; if not, the finding is likely invented -> REFUTED (or UNSURE if the code could be outside your ranges).
fix_sound: yes | no | n/a  = would Gemini's implied fix be correct and safe here (n/a if REFUTED).
Write the facts file in this exact format (<= 60 lines):
# <tid>
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| ... one row per fid ... |
Then up to 10 bullet lines of supporting facts (path:line + what the line does). No pasted code blocks.
Your 5-line reply VERDICT = the worst-case across fids by this order: CONFIRMED > PARTIAL > UNSURE > REFUTED.
If you have read more than the listed ranges or are unsure, stop and put it in OPEN.
Report cap: facts/out file <= 60 lines.
Also write your 5-line reply to .claude/waves/gem1/u001.reply.txt

## Process rules
- Scratch: only the dir the brief names; long jobs via run_in_background + notification, never poll loops.
## Output Schema: RECON (read-only; write only your OUTPUT file). Reply with exactly these 5 lines in this order (evidence first, verdict last). NO prose, NO pasted code.
OUTPUT: <repo-relative path of your findings file, or None>
EVIDENCE: <1-3 anchors: path:line | 0xADDR | log marker; ';'-separated>
FINDING: <most important result, <= 25 words>
OPEN: <one unresolved question, or None>
VERDICT: <CONFIRMED | REFUTED | PARTIAL | UNSURE>
