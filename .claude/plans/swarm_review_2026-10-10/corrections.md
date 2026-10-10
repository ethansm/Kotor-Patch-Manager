# Corrections to the earlier investigation report (verified by swarm, 2026-10-10)

Use these numbers, not the earlier ones.

| ID  | Earlier statement | Verified value | Source |
|-----|-------------------|----------------|--------|
| C02 | Deep-review engine is about 1,850 lines | 12,826 lines in 33 .py and .sh files (excluding fixture, stub, __pycache__). The 1,850 figure was a wc total that included .md files. | V1 (wc on bundle) |
| C04 | About 20 files differ between .claude/review and the bundle | 33 entries differ or exist on one side only: 22 in verify/, 11 in tools/ | V1 (diff -rq) |
| C05 | About two dozen scripts implement claude -p | About 17 real implementers; 23 raw grep hits, the rest are stubs, docs or record-keeping | V1 (rg, classified by filename) |
| C06 | sha256 helper defined in three places | At least 10 path-taking hash helpers (e.g. scripts/shift_hud_leader_buttons.py:36, scripts/mirror_character_page.py:76, .claude/review/merge.py:153, .claude/review/fix/fix_workspace.py:163) | V1 (signatures only; bodies not all read) |
| C12 | Fix pilot $0.59 for 2 patches | $0.59 is the total for a 5-task Phase B pilot that produced 2 patches (3 rejected or not fixable) | V2 (fix_pipeline_plan.handoff.md:110, :102) |
| C16 | Cost comes from total_cost_usd in the result | Stream-json lines do carry total_cost_usd. dr.py sum_cost does NOT read it first: it reads cost_usd_total, then attempts[].cost_usd, then cost_usd; total_cost_usd is only a 4th fallback (dr.py:440-455) | V3 (body read) |
| C18 | Dry-run plans 6 files | 6 files plus the plan doc = 7 entries. Doc 37 is free. Auto-pick is max+1 with no collision check; an explicit doc_id is not checked. Writes skip existing targets with [EXISTS]. | V3 (scaffold_mod.py:45-51, :233, :244) |
| C23 | K2, K3a, K4 were "design-level risks" refuted by Haiku | K4 and K3a: refuted by Haiku AND by the Sonnet adjudicator. K2: Haiku said inconclusive, Sonnet refuted. The phrase "design-level" is not on disk. They are recall keys in verified_findings.v2.md:20-31 | V2 |
| C10 | Haiku decoy pilot file location unknown | patch_manager_mods/steam_db_pr3/haiku_eval_plan.md:94-101 (13/40 decoys accepted, 82 real rows flagged, 0 DB errors) | V2 |

## Confirmed as stated
C01 (fix-* stages refuse with exit 2), C03 (vcommon 273 vs 554; canary_test 177 vs 238), C07 (zero-byte DB copies; query_ghidra default is the 9.8 MB file), C08 (haiku xhigh, sonnet high), C09 (ladder 8/12 vs Sonnet 8/12; $0.028 vs $0.098; 0/2 routed), C11 ($9.2 and 2.5 h), C13 (no temperature or seed flag), C14 (--bare skips CLAUDE.md auto-discovery, per help text), C15 (CLAUDE_CODE_EFFORT_LEVEL overrides --effort), C17 (21 live logs; perflab_metrics.txt is 45 MB).

## V4 policy group (verified)
| ID  | Finding |
|-----|---------|
| C19 | PARTIAL. The NEVER cat/tail/grep rule exists (CLAUDE.md:7). It does NOT say kotor2_logs.py is the only permitted route; rg and sed are not addressed. |
| C20 | PARTIAL. The credential rule is in the PROJECT CLAUDE.md (:18-20), not the global one. |
| C21 | PARTIAL. run_fix.sh:91-94 allows FIX_SANDBOX=none for the stub or canary VARIANT B. ALLOW_LAYER1_ONLY=1 only matters on the layer1_only branch (:83). |
| C22 | PARTIAL, important. Git and install guards are real code (dr.py:682-684, 740-742, 1085-1087) but ONLY when a profile lists them; the default forbidden_commands list is EMPTY (engine/dr_profile.py:37). git add and git commit are NOT guarded in the default config. engine/prep.py:1228 and :1237 run git unguarded. |
| C24 | PARTIAL. Code comments record bugs 12, 13, 16, 17 as fixed. No record for bugs 14, 15, 18. deep_haiku_review_phase3b_impl.md:25 still lists 12 and 13 as open. |
| C25 | CONFIRMED. Doc 36 is the highest; 37 is free. |

## Design consequence from C22
The "never git add or commit, never install" invariants must be built into the library's own deny list, not supplied by each profile. An empty default is unsafe.

## Still pending
K1-K6 critic findings (see out/K*.md).
