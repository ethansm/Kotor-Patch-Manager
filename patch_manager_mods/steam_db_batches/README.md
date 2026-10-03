# Steam address-DB batches (KOTOR II Steam Aspyr, exe 6a522e71)

Contract: `DESIGN.md`. Briefs: `REVIEWER_BRIEF.md` (Sonnet), `OPUS_GATE_BRIEF.md` (Opus). Resume notes:
`SESSION_CONTEXT_2026-10-03.md`. GOG DB label errors (read-only, for the user): `GOG_LABEL_SUSPECTS.md`.

Every shipped row is marked in the KDB table `steam_validated` (DESIGN 4.7). A final validation pass runs
`python3 tools/batch.py check-validated --out <file>` and reads only the rows that are not `validated`.

## B0: re-audit of the 1,704 branch rows (2026-10-03)

- Final verdicts: verified 1579, likely 98, label_suspect 15, rejected 4, unresolved 8. 0 of 140 decoy cards were
  accepted. The Opus gate escalated to 100% of the reviewer matches (1 sample error in 102); details in `B0/B0_SUMMARY.md`
  and `B0/opus/`.
- Branch DB (`addressdb/k2-steam-acc` commit 4324098, `import-ghidra --mode replace`): 1,704 -> 1,683 rows.
  - 1,644 rows keep their address.
  - 39 rows moved (list in `B0/branch_apply_report.md`), e.g. OnLoadGamePicked 0x8827A0 -> 0x8824F0 and
    GetClassByPosition 0x844E40 -> 0x89AEF0.
  - 6 GOG-parity rows (`B0/parity.tsv`): GameAPI names whose GOG label is wrong ship at the unique
    fingerprint-identical Steam counterpart of GOG's address.
  - 21 rows were removed (label_suspect, rejected or unresolved).
  - `SqliteTools validate` passed. The DB equals `B0/branch_functions.csv` row for row, notes included. No address
    is held by two names.
- Knowledge DB (`persist B0 --apply`): name_map insert 24, update 1,535, demote 17, rename 3 (`B0/kdb_overrides.tsv`:
  4 Opus-approved draft rows plus the 0x746FC0 demote), 5 lessons, 1,683 `steam_validated` marks. A second apply changed
  0 rows, and `integrity_check` is ok. Backup: `~/Games/KOTOR2_Mod_Workspace/backups/kdb/kotor2_ghidra_knowledge.db.pre_B0_persist_20261003`.
- Validated ledger: `B0/validated.tsv` (verified 1579, likely 98, parity 6; tiers opus 775, auto:sig 554,
  auto:c2 354). `B0/check_validated.tsv`: 1,683 of 1,683 branch rows are `validated`.
- Not shipped:
  - `B0/semantic_extra.tsv` (16): the GOG DB lacks these names, which is out of scope.
  - `Gob::SetProcessFlag`: two identical Steam twins, so not proven.
- GOG label ledger: about 62 GOG DB label errors (rotations); not proposed to the user yet (Phase E).
- Lead spot-checks on disk:
  - GetClassByPosition 0x89AEF0: `0089af00 MOVZX EDX,[ECX+0x8d]`, `0089af13 IMUL EAX,EAX,0x28`,
    `0089af19 MOV DL,[ECX+EAX+0xab]`; same fingerprint as GOG 0x588990.
  - OnLoadGamePicked 0x8824F0: same fingerprint as GOG 0x53CF90 (679 B, ret 0, the same disp/imm set).
  - AnimationPause 0x7EF350: `007ef3a7 MOV EAX,[0x00a0fe2c]` ('Pause' column).
  - GetMiniMapVisible 0x7D05A0: `007d05b7 CALL 0x0072fb00` (GetClientOptions), `007d0602 MOV EAX,[EDX+0x59b4]`.
- KDB lesson 358 was wrong (OnLoadGamePicked = 0x8827A0). A correction lesson was added: 0x8827A0 is OnLoadSaveGame
  (Mac symbols).
- PR-text items only (upstream GameAPI typedef bugs): ShowUpgradeScreen pops 24; ActionMenuSecurity pops 8.
- Tool changes:
  - `branch_apply.py` and `finalize` write `validated.tsv`.
  - The CSV writers turn `"` into `'` in notes, because SqliteTools' line-based CSV reader drops quotes.
