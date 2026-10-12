# WI-68b Mod scaffold kind selector
area: mod-ideate
size: S
ready: ready
items: C409
files: scripts/scaffold_mod.py
out-of-repo: none
blockers: Files each kind generates (hook, data, ui): unknown beyond plan 8.4 item 6. If 8.4 does not define them, stop and write it in NOTES. Mark-done row id: R13. STATE values unknown. Split from WI-68.
docs: .claude/plans/swarm_library_architecture.md:C409

## Goal
Add a `kind` selector (hook, data, ui) to the mod scaffold (R13). Keep the existing collision check. Plan 8.4 item 6 says the scaffold has no kind parameter and always emits a DLL detour scaffold.

## Steps
1. In `scripts/scaffold_mod.py`, `scaffold_mod(mod_name, doc_id, title, dry_run)` (line 37) has no kind parameter. The manifest hard-codes `kind = "mod"` (line 129). Add `--kind {hook,data,ui}` to `main()` (argparse at lines 270-275) and pass it into `scaffold_mod`. Default: hook, which keeps the current output.
2. Keep the existing collision check (`if not target.exists()`, line 252).
3. Generate each kind to a dry run and check the file list.

## Verify
1. `python3 scripts/scaffold_mod.py probe_kind_hook --kind hook --dry-run`, then the same with `data` and `ui`. Dry runs create nothing.
2. `python3 scripts/build_check.py`
3. `python3 scripts/run_tests.py <filter>`: suite name unknown.

## Docs
- After the row passes: `python3 scripts/plan_manager.py mark-done R13 --state <STATE> --notes "<evidence>"`. STATE values unknown. Never sed the plan doc.
- CHANGELOG: none named in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C409 .claude/plans/swarm_library_architecture.md:214 id=R13 open
