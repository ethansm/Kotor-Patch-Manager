# Steam DB batches: session context 2026-10-03 (b) - READ THIS FIRST when resuming
Supersedes `SESSION_CONTEXT_2026-10-03.md` (kept for B0 history). Status per batch: `README.md`. Contract: `DESIGN.md`.

## Done this session
- **B0 SHIPPED.**
  - Branch `addressdb/k2-steam-acc` 4324098 (1,704 -> 1,683 rows, validate passed). Backup before the import:
    `backups/steam_db/kotor2_steam_aspyr.db.c9a58bd_20261003`; after: `.4324098_20261003`.
  - KDB persist B0: 3,267 rows; a re-run changes 0. KDB backup `backups/kdb/kotor2_ghidra_knowledge.db.pre_B0_persist_20261003`.
  - Commit 4d2602f.
- **Validated ledger (user request: "mark every validated row so final validation passes skip it").** DESIGN 4.7.
  - `B<n>/validated.tsv` is written by finalize; for B0, by `tools/branch_apply.py`.
  - `persist` copies it into the KDB table `steam_validated` (Steam fp hash per row).
  - `batch.py check-validated [--out]` labels branch rows validated / stale / unvalidated. B0: 1683/1683 validated.
  - Feedback memory `feedback-mark-validated`.
- **B15 (ExecuteCommand*) gated, not yet shipped.** Commits 18f0214 + this session's last commit.
  - `server/ghidra_scripts/ExportCommandStores.java` is a read-only server export, archived in `server/cmdstores/`.
  - `tools/cmdtable.py` decodes the NWScript command tables:
    - GOG 0x77B530 + 0x7F9140; Steam 0x665F50 + 0x6F5B80; Mac x86 0x324A7C + 0x3579D4 (8-byte slots).
    - 877 slots and 655 handlers on all three binaries. 655/655 GOG slot sets have one identical Steam set.
    - 653/655 GOG names equal the Mac real symbol of the same slot set.
    - Outputs: `B15/cmdtable*.tsv`, `cmdtable_report.md`, `extra_candidates.tsv`, `card_facts.tsv`.
  - `candidates B15 --classes Global,CSWVirtualMachineCommands --name-regex '^ExecuteCommand' --extra-candidates
    B15/extra_candidates.tsv` -> 656 rows. gates: AUTO 474 (all agree with the table), REVIEW 182.
  - Cards: 4 chunks, 40 decoys. Candidate sources are now withheld on every card (they used to read `card` on decoys
    only, a leak in B0 too).
  - Sonnet reviews `B15/review/sonnet_c1-4.tsv`. `finalize B15` (pre-Opus): verified 571 / likely 78 / unresolved 5 /
    label_suspect 2; 8 rechecks.
  - Decoys: 1 "accepted" (B15-4-38 0x784680 -> 0x66F0A0). It is a KEY ERROR: the decoy is the true table counterpart,
    because decoys are taken as neighbours of the gate's (wrong) pick. ESCALATE was carried out as 100% Opus.
  - **Opus gate done:** `B15/opus/overrides_a.tsv/.md` (103 rows: 102 verified + 1 label_suspect; 7 Opus corrections =
    table counterparts: ChangeFaction 0x66F0A0, GetCreatureTalent 0x67AE40, GetEmotions 0x67C430, GetMatchedStringsNum
    0x683740, GetNearestObject 0x683D40, IsAIState 0x68B4B0, MoveToPoint 0x68CEE0) and `overrides_b.tsv/.md` (100 rows:
    98 verified + 2 label_suspect; 10-row AUTO method audit pass; 8 AUTO meaning checks pass). sample_errors 0,
    table decode errors 0 (both parts re-derived all 877 slots independently).

## B15 open decisions for consolidation (next step)
1. Slot 156 pair (both exes store slot 156 twice). GOG names are swapped vs the Mac symbols:
   - live GOG 0x78C440 "EffectTemporaryForcePoints" -> Steam 0x676E60 (SetCreator call; Mac live = EffectTemporaryFP);
   - dead GOG 0x78D6B0 "EffectTemporaryFP" -> Steam 0x6780D0 (overwritten store, no callers).
   Parts a and b disagree on 0x78C440 (verified vs label_suspect). Lead proposal = B0 semantic-shipping rule: ship each
   name at its proven body (EffectTemporaryFP at 0x676E60, EffectTemporaryForcePoints at 0x6780D0), and record the swap
   in GOG_LABEL_SUSPECTS.md.
2. Source names that differ from nwscript names (GetEmotions, IsAIState, ChangeFaction, ...) are Mac real symbols:
   `verified`, not label_suspect (part a ruling).
3. Opus consolidation -> `B15/overrides.tsv` (gog_addr steam_addr final_verdict note) from overrides_a + overrides_b.
   Then `finalize B15 --opus-sample-errors 0`, B15_SUMMARY.md and lessons in `B15/gate_report.md ## Lessons`:
   - 0x66C7A0 = ExecuteCommandAddMultiClass (slot 389); this resolves doc 08 residual line 413. LevelUpOverride.cpp:103
     labels it "LevelUpHenchman" (address right, label wrong).
   - the decoy key error;
   - the Mac 8-byte stride.
   Then the lead's ship steps (DESIGN / README): spot-check 3, KDB backup, `persist B15` dry/apply/apply (second = 0),
   branch backup, `SqliteTools import-ghidra --csv B15/functions.csv --mode append`, validate, row count 1683 + N,
   check-validated, commits, README B15, HANDOFF, memory.

## KDB facts seen for B15 persist
- 571 `gog_cluster_delta_map.py`/medium ExecuteCommand rows. Several are rotated (e.g. 0x67F9B0 named
  GetIsDoorActionPossible = really GetIsDay, slot 405). They are untrusted, so persist replaces them.
- 9 `engine-ai-atlas wave2`/high rows use descriptive names with slot numbers at the RIGHT addresses (e.g.
  "ExecuteCommandGetIsEnemy/Friend/Neutral (235-237)" at 0x67C430). Persist keeps them as CONFLICT/trusted holder
  (equivalent names, like CExoString in B0). Decide whether to add kdb_overrides renames to the GOG/Mac names.
- 0x66C7A0 `level-up-P1-decompile-confirmed`/medium "LevelUpHenchman" -> to be replaced by AddMultiClass.
- GOG 0x44F600 `CSWArea::GetIsIDInExcludeList` is the GOG handler for command slots 321/327/403 (Steam 0x681880).
  It is a GOG label-ledger candidate (check the Mac symbol).
- The branch DB already has 18 ExecuteCommand rows (from B0). B15 functions.csv must not duplicate names: check
  before import (append mode updates by name).

## Issues found (for the fix plan)
- Decoy picking can select the true function when the gate's pick is wrong (B15-4-38). score_review / finalize should
  flag "decoy accepted == Opus final address" as a key error, and cards should never use a candidate with an
  engine-data source (cmdtable) as a decoy.
- The gate ranked `near` over `cmdtable` in 7 rows (G6 holders from rotated mapper rows). A TBL anchor family
  (exact unique slot-set identity) needs an Opus calibration review before it may count toward C2 (DESIGN 4.2 rule).
- B0 decoy cards leaked "Candidate sources: card" (fixed in 18f0214 for future batches).
- `opus_gate.md` part line ranges are hand-split; a helper to emit part files would avoid off-by-one splits.
- Test gaps: branch_apply.py has no tests; cmdtable.py main() end-to-end; check-validated with --branch-db and missing
  table; persist without validated.tsv; csv_safe_note; score_review.py.

## Next after B15
B9 (CServerExoApp, CServerExoAppInternal, CSWSModule), then the DESIGN order. Phase D/E as in the plan
(transcript 0f4d4efb). Agents: Sonnet for reviewers and routine work, Opus for gates. The lead alone writes DBs and commits.
