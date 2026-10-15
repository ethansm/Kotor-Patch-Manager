# Steam address-DB master plan (persisted; read first, update at the end of every phase)

Rule (user, 2026-10-03): work in many small plan-mode phases. Each plan covers ONE phase from the table below. At
the end of the phase, update its row (status, commits, counts) and the "Next phase" line, then commit this file.
Use at most 1-3 parallel Opus agents per phase. Wait for running subagents before a handoff.

Contract: `DESIGN.md`. Per-batch records: `README.md` sections. Briefs: `REVIEWER_BRIEF.md`, `OPUS_GATE_BRIEF.md`.
Branch: `~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db`, `addressdb/k2-steam-acc` (not pushed; no squash until
the user directs PR 5). User decisions: mod-demand order, then Phase E and the PR; B10b, B13b, B14, Phase D deferred.

## Phases

| # | Phase | Status | Record |
|---|---|---|---|
| P1 | Task 0 tooling + Task 1 mod_audit | DONE (086491a, b341618) | README Roadmap |
| P2 | B12b CSWSCreature upper half | DONE: SHIPPED (branch a72c344, +178) | README B12b |
| P3 | B16 mod GUI/engine classes | DONE: SHIPPED (branch caf65ec, +146) | README B16 |
| P4 | B11a CClientExoApp(+Internal) | DONE: SHIPPED (branch bcb4acb, +356) | README B11a |
| P5 | B12a CSWSCreature lower half | DONE: SHIPPED (branch db4f5cf, +171; 3,420 rows) | README B12a |
| R1 | REA mapping plan: design how REA MCP (rea-recon Haiku, findings ledger `ghidra_knowledge/rea_findings/`, cc6a424) maps the Steam exe into the DB: where REA replaces/augments candidates, reviewer cards, Opus gates, rotation audits; pilot on one batch; cost/accuracy gate vs the Sonnet-card baseline | TODO (plan-mode next) | handoff MASTER_PLAN |
| P6 | B13a CSWCCreature: ship (merge a/r/z -> finalize --opus-sample-errors 1 -> expect verified 177, label_suspect 19, unresolved 3; kdb_overrides.tsv 28 rows) | TODO (gate work finished; ship only) | B13a/ |
| P7 | B10a CSWSObject+CSWCObject: ship (merge a/r/z -> finalize --opus-sample-errors 0 -> expect verified 210, likely 29, label_suspect 5, rejected 1; KDB has kdb_overrides.tsv 6 rows) | TODO (gate work finished, 4ed13ec; ship only) | B10a/ |
| P8 | B11b CGuiInGame+CSWGuiMainInterface: consolidation (summary, lessons, ledger side file, AUTO rotation audit; GOG labels heavily rotated), then ship | TODO (Opus a/b/c/r finished; sample_errors a 2, b 0, c 0 -> finalize --opus-sample-errors 2) | B11b/ |
| P9 | Ledger + audit catch-up: append B13a/B10a/B11b `gog_ledger_section.md` to GOG_LABEL_SUSPECTS.md; `mod_audit.py` re-run + diff; report label findings to the user | TODO | mod_audit/ |
| P10 | Phase E-1: scripted same-name Mac-score sweep over all AUTO rows of B12b/B16/B11a/B13a/B10a/B11b; Opus reads low scorers; fix via overrides / KDB renames (B12a lesson) | TODO | - |
| P11 | Phase E-2: `check-validated --out`; Opus final review of branch commits since c9a58bd (evidence pointer per row, validate, no dup, ICF folds documented) | TODO | - |
| P12 | Phase E-3 [USER]: live check (back up game addresses.db, copy branch DB, ACC commands walkmeshrender/addfeat/freecam/runscript/teleport; user plays, lead reads logs) | TODO, user-gated | - |
| P13 | Phase E-4: PR text draft from `upstream_prs/drafts/pr_db_note.md` (counts per batch, method, deferred list, GOG label-suspect summary) | TODO | - |
| P14 | Phase E-5 [USER]: user directs PR 5 (squash + push); ACC PR follows. Separately propose GOG_LABEL_SUSPECTS + GOG param-size errors | TODO, user-gated | - |
| D | Deferred follow-up PR: B10b (needs TBL family + ranking calibration), B13b, B14, Phase D | not started | README Roadmap |

## Ship recipe per batch (lead, inline; ~12 tool calls)
1. `git log --oneline -3 -- B<n>` (parallel-session check).
2. `python3 tools/merge_overrides.py B<n>` (no CONFLICT lines), `python3 tools/batch.py finalize B<n> --opus-sample-errors N`.
3. Undecided Opus rows (rows in opus_gate.json full+sample not in overrides.tsv) -> part z (Opus) or lead fp check.
4. `python3 tools/precheck_import.py B<n>` (0 problems).
5. 3 lead spot-checks: Steam fp = GOG fp + one quoted asm line each.
6. KDB backup -> `persist B<n>` dry -> `--apply` -> `--apply` (second = 0, integrity ok). CONFLICT skips must be equivalent names.
7. Branch backup -> `SqliteTools import-ghidra --csv B<n>/functions.csv --database DB --mode append` -> `validate --database DB` -> dup check.
8. `check-validated --out B<n>/check_validated.tsv` (all validated).
9. Branch commit (DB path only); KOTOR-II `git commit -- B<n> README.md ...` (never the KDB binary, never `-a`).
10. README section; update this file.

## Traps (do not repeat)
- `opus-parts` deletes part files it did not write: write part_r/part_z after it (`tools/prep_opus.sh` does).
- GOG labels are often rotated while bodies are right: run the AUTO rotation audit in every consolidation.
- Accepted decoys can be key errors (decoy_prev from a mis-slotted pick): check before blaming the reviewer.
- Symmetric pairs need a quoted distinguishing anchor for both sides, else `unresolved`.
- Never edit `*_src`, the GOG DB, the game folder; only the lead writes DBs.

## Next phase
R1: plan how to use REA MCP tooling (rea-recon) for optimal Steam DB mapping (user, 2026-10-15). Then P6-P8 ship
(gate work done) and P9. User-approved roadmap add (10-15): B17 'mod GUI round 2' (CSWGuiInGameEquip 31, CSWGuiTargetActionMenu 15,
CSWGuiManager 48 left, CSWGuiPanel 14 left, CExoInputInternal 43) after P8; re-run mod_audit first (37 mods now vs 22).
