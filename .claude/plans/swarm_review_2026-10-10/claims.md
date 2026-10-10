# Claim ledger (IDs are stable; verifiers cite them)
Repo root: /home/ethansm/orca/workspaces/local-setup/KOTOR-II   (ALWAYS use absolute paths)
Global bundle: /home/ethansm/.claude/skills/deep-haiku-review/bundle
Verdict rule: CONFIRMED = on-disk evidence matches the claim exactly (numbers within 5%).
PARTIAL = true with a material correction. REFUTED = contradicted on disk. UNSURE = cannot be checked read-only.

## Group A: structure and forks
C01  deep-review fix stages (fix-intake, fix-*, S9-S14) are NOT built; `dr.py fix-*` prints "not built" and exits 2.
     Check: rg -n "not built|fix-" <bundle>/dr.py ; sed -n of SKILL.md stage table.
C02  The deep-review engine (bundle *.py and *.sh, excluding fixture/, stub/, __pycache__) totals about 1,850 lines.
     Check: find <bundle> -name '*.py' -o -name '*.sh' (exclude fixture stub __pycache__) | xargs wc -l | tail -1
C03  Forked copy: .claude/review/verify/vcommon.py is 273 lines vs 554 in the bundle; .claude/review/tools/canary_test.sh is 177 vs 238.
     Check: wc -l on both pairs.
C04  About 20 files differ or exist on one side only between .claude/review/{verify,tools} and bundle/engine/{verify,tools}.
     Check: diff -rq on both pairs, excluding __pycache__ and data dirs (raw adjudications results findings).
C05  About two dozen scripts or shell runners across the bundle and .claude/review implement their own `claude -p` call (contain --output-format).
     Check: rg -l -- '--output-format' over bundle and .claude/review, excluding raw/ results/ findings/ adjudications/ __pycache__ and *.json data.
C06  The sha256 file helper is defined in three places under scripts/ and .claude/review: snapshot_live.py (file_sha256), _static_common.py (sha256_file), fix/fcommon.py (file_sha).
     Check: rg -n 'def .*sha' over scripts and .claude/review (exclude raw/ data).

## Group B: numbers and outcomes
C07  Zero-byte kotor2_ghidra_knowledge.db copies exist (repo root and scripts/); the real DB is patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db (about 9.8 MB); query_ghidra.py's default --db points at the real one.
     Check: find repo -name 'kotor2_ghidra_knowledge.db' -printf '%s %p\n' ; rg -n 'default|ghidra_knowledge' scripts/query_ghidra.py | head
C08  ~/.claude/settings.json sets modelSettings effortLevel "xhigh" for claude-haiku-5-5 and "high" for claude-sonnet-5-5.
     Check: grep -n -A2 'claude-haiku-5-5\|claude-sonnet-5-5' ~/.claude/settings.json (do NOT print any other keys).
C09  Fix-pipeline Phase C result: ladder and Sonnet-first each accepted 8/12 needs-design parents; ladder $0.028/parent vs Sonnet $0.098; rounds 2 and 3 added nothing; needs-wider-scope routing 0/2.
     Check: .claude/reviews/ladder_experiment.md (rg for "8/12", "rounds", "0/2").
C10  Haiku decoy pilot (RE / address-DB): a single Haiku pass accepted 13 of 40 decoys (33%) and raised 82 false flags.
     Check: find/rg for haiku_eval_plan.md anywhere under the repo or worktrees (rg -l 'haiku_eval_plan' .claude patch_manager_mods; locate with find). UNSURE if not found.
C11  Deep audit cost about $9.2 CLI-reported; SKILL.md says roughly $9 and 2.5 h per ~700 coverage units.
     Check: .claude/handoffs/deep_haiku_review.handoff.md and SKILL.md frontmatter (rg -n '\$9|2.5 h|700').
C12  Fix pilot cost about $0.59 for 2 patches.
     Check: rg -n '0\.59|pilot' .claude/handoffs/fix_pipeline_plan.handoff.md ; ls .claude/fixes .claude/review/fix/patches | head
C23  Haiku over-refuted real findings: K2, K4 and K3a were REFUTED by verifiers but are design-level risks.
     Check: rg -n 'K2|K4|K3a' .claude/reviews/verified_findings.v2.md (look at the REFUTED tier).

## Group C: CLI facts and tooling behaviour
C13  The claude CLI has no temperature or seed flag.
     Check: claude --help 2>&1 | grep -i -E 'temperature|seed|top.p'  (report exact match or none).
C14  `claude --bare` is documented as skipping hooks and some config; determine whether it is documented to skip CLAUDE.md auto-discovery.
     Check: claude --help 2>&1 | grep -n -A6 -- '--bare'. Verdict UNSURE unless the text states CLAUDE.md behaviour.
C15  CLAUDE_CODE_EFFORT_LEVEL in the environment overrides --effort (the map runner warns about this).
     Check: sed -n 160,166p bundle/engine/run_map.sh ; then grep the memory note at /home/ethansm/.claude/projects/-home-ethansm-orca-projects-local-setup/memory/kotor2-deep-haiku-review.md (memory files are on disk). Verdict CONFIRMED only if the runner's warning code exists AND a doc or code states the override; a memory note alone is PARTIAL.
C16  Stream-json result lines carry total_cost_usd, and dr.py's sum_cost reads it.
     Check: rg -n 'def sum_cost' -A8 bundle/dr.py ; rg -m1 -o 'total_cost_usd[^,}]*' .claude/review/raw | head -2
C17  `python3 scripts/kotor2_logs.py status --since-hours 72` lists about 21 live logs; perflab_metrics.txt is about 44 MB (STAT ONLY, never read it).
     Check: run the status command (read-only dispatcher) and stat -c %s on the file.
C18  `scaffold_mod.py --dry-run <name>` plans the next free doc number and 6 files; doc 37 is the next free number.
     Check: python3 scripts/scaffold_mod.py --dry-run zz_probe_demo (dry-run ONLY). ls patch_manager_mods/3[6-9]_* . Also read how scaffold_mod.py picks the doc id (rg -n 'doc_id|glob' scripts/scaffold_mod.py) and state whether it checks for collisions.

## Group D: policy and invariants
C19  Project CLAUDE.md forbids cat, tail, grep on raw mod .txt logs; the only permitted route is `python3 scripts/kotor2_logs.py`.
     Check: rg -n 'NEVER .cat' CLAUDE.md
C20  Global CLAUDE.md forbids viewing or grepping ~/.config/ghidra/.server_credentials and any *credential* or *secret* file; Ghidra access goes through scripts/ghidra_server_api.py.
     Check: rg -n 'credential' ~/.claude/CLAUDE.md CLAUDE.md (do NOT open the credential file itself).
C21  run_fix.sh allows FIX_SANDBOX=none only with the stub claude, or canary variant B plus ALLOW_LAYER1_ONLY=1.
     Check: sed -n 1,30p .claude/review/fix/run_fix.sh | rg -n 'SANDBOX'; then rg -n 'FIX_SANDBOX' .claude/review/fix/run_fix.sh | head.
C22  deep-review invariant "never run git add or git commit" is ENFORCED in code, not only written in prose.
     Check: rg -n 'git add|git commit|forbidden' bundle/dr.py bundle/engine | head -20. CONFIRMED only if a code path refuses or aborts; PARTIAL if only profile-level text.
C24  Fix-pipeline bugs 12 to 18 are recorded as fixed.
     Check: rg -n -i 'bug 1[2-8]|bugs 12' .claude/handoffs .claude/review/fix .claude/plans | head. UNSURE if not recorded on disk.
C25  Doc numbering: patch_manager_mods has docs up to 36 with 37 free.
     Check: ls patch_manager_mods | rg '^3[6-9]_|^4[0-9]_' ; expect no 37_ file.
