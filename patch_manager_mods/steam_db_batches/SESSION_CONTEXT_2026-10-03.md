# Steam DB batches: session context (2026-10-03) - READ THIS FIRST when resuming

Plan of record: "map the Steam Aspyr exe into our DBs, in verified batches" (Phases A-E). Contract: `DESIGN.md`.
Briefs: `REVIEWER_BRIEF.md` (Sonnet card reviewers), `OPUS_GATE_BRIEF.md` (Opus per-batch gate).

## User decisions (2026-10-03)
1. Write to both DBs, staged (knowledge DB `name_map` first, then branch `addressdb/k2-steam-acc`).
2. Batch-1 rigour; re-audit Batches 2-8 (= B0). Read-only server ingest.
3. **The PR changes only the Steam DB.** `kotor2_gog_aspyr.db` is read-only; GOG label bugs go to
   `GOG_LABEL_SUSPECTS.md` and are proposed to the user only as confirmed bugs (not proposed yet).
4. **GOG parity for GameAPI-required names whose correct Steam function is not found:** ship the name at the proven
   (unique fingerprint-identical) Steam counterpart of GOG's address, note "GOG parity". Unproven -> omit.
   Applied in `B0/parity.tsv` (6 rows). `Gob::SetProcessFlag` omitted (GOG body = a destructor called only from unwind
   funclets; two identical Steam twins 0x459560/0x464bd0 -> not proven).
5. Earlier (PR3, 10-02): names the GOG DB lacks are out of scope -> `B0/semantic_extra.tsv` (16) not shipped.

## Done (commits on KOTOR-II, latest first)
- (uncommitted at write time, see "State") B0 consolidation, g-part, parity, branch_apply.py.
- cf67ddf B0 Opus gate a/b/c + 100% escalation e1-e4, G8, re-gate. 793d580 all 14 review chunks.
- 861232e/110b8ad/911d30d/13df697/4bbf6dd Phase A (server ingest, pipeline, calibration, Opus sign-off R1-R7).
- Branch DB backup before any write: `~/Games/KOTOR2_Mod_Workspace/backups/steam_db/kotor2_steam_aspyr.db.c9a58bd_20261003`.
  KDB backup: `~/Games/KOTOR2_Mod_Workspace/backups/kdb/kotor2_ghidra_knowledge.db.pre_server_ingest_20261003`.

## Key facts learned
- Server: `python3 scripts/ghidra_server_api.py {status,list-tree,export,export-script,export-types}`; exports in
  `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/<program>/` (fp.tsv = per-function fingerprints:
  callees, str_refs, disp, imm; list cells unpadded hex). Mac x86 build has real symbol names (tagged ANALYSIS).
- KPM GOG DB = derived from the server GOG program; ~62 GOG label errors found (rotations in CGuiInGame,
  CSWCObject animation block, CSWSArea path/DFS/Smooth*, CSWSCreatureStats weapon/talent, CSWSItem property block,
  CSWCArea map pins) + 3 GOG pseudo-functions (0x720DC0/0x720F20 = pieces of ClearLineOfSight, 0x4DB100).
  KPM GOG `GetMiniMapVisible` calls ShowDialogMessage. Our own B1 row OnLoadGamePicked 0x8827A0 was wrong
  (= OnLoadSaveGame); correct 0x8824F0.
- Gemini "Option A" rewrote 13 `steam-db-pr3-phase2` name_map rows -> that source is trusted only via
  `gold/b1_verified_head.tsv`. Batches 2-8 sources unaudited until B0 persists.
- Every pipeline Steam address in B0 was the right BODY; the errors are GOG names. AUTO ships GOG names, so
  G7 (Mac contradiction), G8 (Mac rotation) and a "+-4 of a confirmed label error -> Opus" sweep guard it; G7+G8 catch
  15/37 known label errors (Mac lacks symbols for the rest) -> keep the +-4 sweep in every batch.
- Reviewer tier (Sonnet) is a filter, not a gate: B0 sample found 1 label error in 102 -> escalation to 100% Opus.
  Expect the same for future batches; budget Opus for all reviewer matches.
- Another session writes the KDB concurrently (lessons/name_map, e.g. commit fbcfe26): re-read before persist.

## State at session close
- B0 finalize: verified 1579 / likely 98 / label_suspect 15 / rejected 4 / unresolved 8 (all 1,704 rows decided;
  0 of 140 decoys accepted). Inputs: `B0/overrides.tsv` (787 rows, consolidated Opus), `B0/opus/*.md` evidence.
- `tools/branch_apply.py B0` -> `B0/branch_functions.csv` (FULL table for SqliteTools `import-ghidra --mode replace`)
  + `B0/branch_apply_report.md`: 1704 branch rows -> 1683 shipped (1644 same address, 39 moved, 6 parity), 21 removed,
  0 duplicates. Keeps branch (KPM API) names. Ships `likely` rows (lead call: same body as GOG's address).
  CHECK before import: GetClass (parity 0x844E40) vs GetClassByPosition (same address in the branch) - confirm
  GetClassByPosition's final address and whether an address shared by two names is acceptable (GOG DB has both names
  at 0x48F9B0 too).
- `B0/kdb_overrides_draft.tsv` (Sonnet draft, 4 rows: rename 0x8827A0 -> OnLoadSaveGame, demote 0x54D3D0,
  rename 0x6076D0, 0x740FC0) needs Opus/lead approval -> copy to `B0/kdb_overrides.tsv`.
- Not yet done: KDB persist (`batch.py persist B0` dry run, then `--apply`, run twice -> 0), SqliteTools import +
  `validate`, branch commit, lead spot-check of 3 corrected rows on disk, B0 README section, HANDOFF line.

## Next phases (from the plan)
- Finish B0 (above), then Phase C batches B9 (CServerExoApp/Internal, CSWSModule), B10a/b, B11a/b, B12, B13a/b, B14,
  B15 (Global ExecuteCommand* via command table 0x665F50); Phase D offsets/globals; Phase E live check + PR (user-gated).
- Per batch: candidates -> gates -> cards (>=10 decoys/chunk) -> Sonnet reviewers (4 parallel, 1 chunk each, ~250k
  tokens/chunk) -> score_review -> Opus gate (100% classes + sample; escalate on any error) + +-4 label sweep ->
  Opus consolidation -> finalize -> lead spot-check 3 -> persist --apply -> branch import (append mode for new rows)
  -> validate -> commits -> README/HANDOFF/memory.
- Open: propose the GOG DB fixes to the user (GOG_LABEL_SUSPECTS.md); CRes/CExoString naming equivalents in KDB;
  semantic_extra names (out of scope unless the user changes the PR3 rule).
