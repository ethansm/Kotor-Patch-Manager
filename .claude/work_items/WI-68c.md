# WI-68c Docs search cache refresh
area: search-docs
size: S
ready: ready
items: C412
files: .doc_search_cache.db
out-of-repo: none
blockers: Concrete search query: unknown (needed for Verify 3). STATE values unknown. Split from WI-68.
docs: .claude/plans/swarm_library_architecture.md:C412 (R16, line 217; re-index gate in plan 8.4 item 2)

## Goal
Refresh the docs search index (R16). `.doc_search_cache.db` is dated 2026-10-03, before the Oct 9 cutoff.

## Steps
1. Verify staleness: `stat -c '%y' .doc_search_cache.db` (verified: 2026-10-03 20:33:48). Stale against the Oct 9 cutoff.
2. Refresh: `python3 scripts/search_docs.py index`. Default dirs are `patch_manager_mods` and `.` (scripts/search_docs.py:8 and :222-223). This writes `.doc_search_cache.db`.
3. Verify with one concrete query. The query text is unknown, so use any term from the mod-ideate docs and expect hits.

## Verify
1. `stat -c '%y' .doc_search_cache.db` shows a date after 2026-10-09.
2. `python3 scripts/search_docs.py search "<query: unknown>" --limit 3` returns hits.
3. `python3 scripts/build_check.py`

## Docs
- After the row passes: `python3 scripts/plan_manager.py mark-done R16 --state <STATE> --notes "<evidence>"`. STATE values unknown. Never sed the plan doc.
- CHANGELOG: none named in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C412 .claude/plans/swarm_library_architecture.md:217 id=R16 open
