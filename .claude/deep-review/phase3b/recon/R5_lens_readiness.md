# R5 lens-mode readiness for run p1-lens (read-only recon)

Bundle B = ~/.claude/skills/deep-haiku-review/bundle. All citations verified on disk.

## 1. reduce vs lens findings
- Reduce DOES read lens findings. run_reduce.sh:88-89 runs `merge.py --emit-bundle` for the group; emit_bundle (merge.py:1032-1046) calls load_worker_findings(..., lens_names(P)), which reads RUN/lens/<lens>/findings (merge.py:357-366).
- Order is fixed: reduce before merge (dr.py:55-62 STEPS). Merge's coverage gate needs each reduce group's domain_<g>.json (merge.py:856-860 warning; dr.py:77 "coverage" gate text).
- So: map (all lenses) -> reduce -> merge. Reduce cannot be skipped.

## 2. profile [run] head as SHA
- YES for resolution: prep.py:454-455 `rev-parse --verify <ref>^{commit}` accepts 22b56b4.
- BUT prep.py:462-464 scope() dies if `git rev-parse HEAD` != pinned head ("HEAD moved"). Worktree HEAD is 0d67415, so 22b56b4 must be checked out (detached) before prep, or the run fails at prep.
- Rebuild on the same head: prep.py:450-456 resolve_head; resize keeps snapshot head.

## 3. lens mode on p1
- NO. dr.py:868 init on an existing run id only resumes ("nothing changed"); p1 state.json has all stages done (no lens stages).
- Use a new run id: `--run p1-lens` with a new profile copy adding `[lenses] names = ["logic","state","contract","security"]`. All four templates exist (templates/lens_*.md, incl. lens_contract.md).
- dr_profile.py:124-134 validates lens names and template presence.

## 4. Ordered command list (prefix: `dr.py --profile <lens-profile.toml> --run p1-lens`)
1. `git checkout --detach 22b56b4` (prep head pin, see 2)
2. `init <lens-profile.toml>` (free)
3. `run prep` (free) -> auto gate prep-ok
4. `approve spend-canary` then `run canary` (PAID)
5. `run facts` (free)
6. `approve spend-map-first` then `run map-first` (PAID, first packet only)
7. HUMAN: read first packet under RUN/lens/<lens>/runs and findings, then `approve first-packet --note "..."` (dr.py:75-77)
8. `run resize` (free; dr.py:379-381 sizes from first lens run)
9. `approve spend-map` then `run map` (PAID, all packet:lens units)
10. `approve spend-reduce` then `run reduce` (PAID, Sonnet per group)
11. `run merge` (free; auto coverage gate)
12. `run verify-select` (free), `approve spend-verify` + `run verify` (PAID), `approve spend-adjudicate` + `run adjudicate` (PAID), `run verify-merge` (free), `approve spend-refute-audit` + `run refute-audit` (PAID), HUMAN `approve findings-skim`
- Free stages: prep, facts, resize, merge, verify-select, verify-merge. Ladder stages are N/A unless [ladder].adjudicate_styles is set.

## 5. corroborated output
- OUT = repo/.claude/deep-review/p1-lens/ (dr_profile.py:222 out_dir_abs + RUN_ID).
- `master_findings.json` -> findings[].lenses and findings[].corroborated (merge.py:831-832, written at merge.py:1014).
- `master_findings.md` does not render corroborated (no match in merge.py render path); ledger tie-break only (merge.py:840).

## Open
- Whether `dr.py run <stage>` needs `--yes-spend` instead of approve spend-<stage> for paid stages (both documented, dr.py:19, 1015).
