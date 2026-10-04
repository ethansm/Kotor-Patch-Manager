# Steam address-DB batches (KOTOR II Steam Aspyr, exe 6a522e71)

Master plan (read first): `MASTER_PLAN.md`. Contract: `DESIGN.md`. Briefs: `REVIEWER_BRIEF.md` (Sonnet), `OPUS_GATE_BRIEF.md` (Opus). Resume notes (latest):
`SESSION_CONTEXT_2026-10-03b.md`, then the newest batch section below. GOG DB label errors (read-only, for the user): `GOG_LABEL_SUSPECTS.md`.

## Roadmap (user decision 2026-10-03: mod demand first, then Phase E and PR 5)

Order: B12a/B12b `CSWSCreature` (split at GOG 0x696F00) -> B11a `CClientExoApp`+`Internal` -> B16 -> B13a
`CSWCCreature` -> B10a `CSWSObject`+`CSWCObject` -> B11b `CGuiInGame`+`CSWGuiMainInterface` -> Phase E.
- **B16 (new): mod GUI and engine classes** (149 GOG rows): CSWGuiInGameCharacter, CSWGuiTextParams,
  CSWGuiInGameGameplay, CSWGuiDialogLetterbox, CAurTextureBasic, CSWGlobalVariableTable, CServerAIMaster,
  CExoMoviePlayerInternal. In no other batch, but used by level_up_override, Mod 14, the letterbox fix, perf_lab,
  texture_memory_release and companion_ai.
- Mod address audit: `tools/mod_audit.py` -> `mod_audit/REPORT.md`; re-run after every batch.
- **Deferred to a follow-up PR (do not start):** B10b `CSWSMessage`/`CSWCMessage` (needs the TBL anchor family and
  the unheld-candidate ranking change, Opus calibration first); B13b `CSWCAnimBase*` (permuted-label risk, low mod
  demand); B14 `CSWSDialog`, `CSWGuiManager`, `CExoSoundInternal`, `CExoInput*`; Phase D offsets and globals.

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

## B15: NWScript command handlers, `ExecuteCommand*` (2026-10-03)

- Scope: 656 GOG rows, `Global` 638 and `CSWVirtualMachineCommands` 18. That is the 655 handlers plus the VM
  dispatcher.
- Proof: the NWScript command table (`tools/cmdtable.py`, DESIGN 4.8). Each GOG handler's slot set has exactly one
  identical Steam slot set (655/655), and 653/655 GOG names equal the Mac x86 symbols.
- Gates: AUTO 474, REVIEW 182. 4 Sonnet card chunks, 40 decoys.
- Opus: parts a/b covered 200 rows (every REVIEW row), then a consolidation pass. Details in `B15/B15_SUMMARY.md`.
- Final verdicts: verified 656 (0 likely / label_suspect / rejected / unresolved).
- Opus gate: 0 sample errors, 0 decoys accepted, 1 decoy key error (B15-4-38, see the tool fixes below). There were
  7 Opus corrections: the gate ranked `near` over the table counterpart in rows held by wave2 names.
- Slot 156 (stored twice) ships at GOG parity, by user decision:
  - ExecuteCommandEffectTemporaryForcePoints at the live handler 0x676E60 (B0 row unchanged);
  - ExecuteCommandEffectTemporaryFP at the dead first store 0x6780D0.
  - The Mac symbols are swapped, but GOG follows nwscript.
- Branch DB (`addressdb/k2-steam-acc` 7b357da, `import-ghidra --mode append`): 1,683 -> 2,321 rows. 638 rows are
  new, and 18 B0 rows were confirmed at the same address (notes refreshed). `validate` passed, and no address or
  name appears twice. Backup: `backups/steam_db/kotor2_steam_aspyr.db.4324098_preB15_20261003`.
- KDB (`persist B15 --apply`): insert 47, update 577, 8 lessons, 656 `steam_validated` marks; skip 32.
  - The skips are 25 rows already trusted under the same name and 7 CONFLICT rows. The CONFLICT rows keep their
    engine-ai-atlas wave2 descriptive names, which match the table's slot sets.
  - 0x66C7A0 changes from LevelUpHenchman to ExecuteCommandAddMultiClass (slot 389; doc 08 residual resolved).
  - A second apply changed 0 rows, and `integrity_check` is ok.
  - Backup: `backups/kdb/kotor2_ghidra_knowledge.db.pre_B15_persist_20261003`.
- `check-validated`: 2,321 of 2,321 branch rows are `validated` (`B15/check_validated.tsv`).
- Lead spot-checks on disk (Steam asm, fingerprints equal to GOG):
  - ChangeFaction 0x66F0A0 (650 B, RET 8): store `00666496 [EDX+0x2b4]` = slot 173, plus `0066f0cd CMP [EBP+8],0xad`.
  - GetNearestObject 0x683D40 (4828 B): `00683e50 CMP [EBP+0x8],0xe3` (slot 227), plus 0xe4 and 0xe5.
  - Slot-156 live 0x676E60 (418 B): `00676f05 MOV EAX,[0x00996084]`, `00676f8f CALL 0x005e5ea0` (SetCreator).
- GOG label ledger:
  - no confirmed B15 GOG label error;
  - GOG 0x44F600 `CSWArea::GetIsIDInExcludeList` is a Windows ICF fold of 3 command stubs (Mac has 4 separate
    bodies). See `GOG_LABEL_SUSPECTS.md`.
- Tool fixes this round (DESIGN 4.3/4.4/4.6):
  - F1: decoys skip proof-source candidates, and finalize lists decoy key errors.
  - F2: `opus-parts`.
  - F4: decoy-card answers no longer drive verdicts, rechecks or the Opus 100% list. In B15 that list shrinks from
    26 rows to 8.
  - `functions.csv` notes cite the final note (Opus / reviewer anchor) for reviewed rows. Before, corrected rows
    carried the gate evidence of the replaced pick.
  - `branch_apply.py` honours BD_ROOT and BD_BRANCHDB.
- Tests: 257 (`tools/tests/COVERAGE.md`: 89% -> 92%; cmdtable 35% -> 98%, branch_apply 0% -> 99%).
- Open items:
  - A TBL anchor family needs an Opus calibration review before B10b.
  - The gate prefers a held pick over an unheld proof candidate (6 of the 7 misses). That is a ranking change, so it
    also needs the calibration review.
  - An empty `fp_sha` mark never goes stale.
  - A key-error decoy still counts toward the chunk minimum.

## B9: Server Core & World Lifecycle (2026-10-03)

- Scope: 270 GOG rows: `CServerExoApp` 118, `CServerExoAppInternal` 98, `CSWSModule` 54.
- Gates: AUTO 102, REVIEW 166, NOCAND 1, REJECT 1. 4 Sonnet card chunks, 40 decoys.
- Reviewers: 1 decoy card accepted (B9-3-29 SetIgnoreConversationStartRange: the decoy 0x51CD10 is
  SetActionConversation, so this was a genuine reviewer error, not a key error). That triggered ESCALATE: Opus read
  100% of the reviewer matches.
- Opus: parts a/b/c covered 174 rows (all 166 REVIEW rows plus 8 AUTO meaning checks), 0 sample errors.
  - a: 57 verified, 2 likely, 1 label_suspect;
  - b: 58 verified, 2 likely;
  - c: 54 verified.
  - The 10 address corrections were all the same-shape neighbour (Get/Set twin or adjacent field).
- Two passes:
  - **First pass** (another session, KOTOR-II 92569e5, branch 4b81a2b): merged the three parts as they stood and
    shipped 263 verified rows. The branch went from 2,321 to 2,568 rows.
  - **Opus consolidation** (`B9/B9_SUMMARY.md`): `overrides.tsv` 176 rows. It added:
    - CServerExoAppInternal::SetMoveToWorldTime: GOG 0x7BFA20 is Steam 0x53CC80. The gate had rejected a wrong pick.
      The fingerprint is equal (103 B, RET 0x1C), and `0053cc8d MOV [EAX+0x1009c],ECX` writes the fields that its
      twin 0x53CCF0 reads.
    - DebugValidatePath, GOG 0x662B80 (the NOCAND row): `label_suspect`. Its body is CSWSCreature::GetFirstAttacker,
      Steam 0x6E6880.
- Final verdicts: verified 264, likely 4, label_suspect 2, rejected 0, unresolved 0.
  - The likely rows (not shipped) are GetFPS, GetNetLayer, GetConnectionLib and the AsSWSModule thunk.
- KDB (`persist B9 --apply` after the consolidation): insert 1, evidence refresh 157, 8 lessons, 1 mark. A second
  apply changed 0 rows. Backup: `backups/kdb/kotor2_ghidra_knowledge.db.pre_B9b_persist_20261003`.
- Branch DB (commit 76cedbe): 2,568 -> 2,569 rows (+SetMoveToWorldTime). The notes on the other 263 rows were
  refreshed. `validate` passed, and no address or name appears twice. Backup:
  `backups/steam_db/kotor2_steam_aspyr.db.4b81a2b_preB9b_20261003`.
- `check-validated`: 2,569 of 2,569 rows are `validated`.
- Lead spot-checks on disk (Steam fingerprint equals GOG for each):
  - SetMoveToWorldTime 0x53CC80, quoted above.
  - SetIgnoreConversationStartRange 0x51CD60 (36 B, RET 4): `0051cd78 MOV [EDX+0x1f320],EAX`. The decoy 0x51CD10
    writes +0x1f31c instead.
  - Constructor 0x51BC20 (147 B): `0051bc5f CALL 0x00919723` (operator new) and `0051bc7a CALL 0x0052ebe0`
    (CServerExoAppInternal ctor). These are the GOG callees 0x921347 and 0x7b1f60.
- GOG label ledger: Internal::Uninitialize and DebugValidatePath are confirmed GOG label errors; 15 GOG DB param-size
  errors are recorded (`GOG_LABEL_SUSPECTS.md`, B9 subsection).
- Open: the KDB names 0x6E6880 "ApplyOnHitInstantDeath" (low confidence), but it is GetFirstAttacker. The rename
  needs a kdb_overrides row.

## B12b: `CSWSCreature` upper half, GOG >= 0x696F00 (2026-10-03)

- Scope: 201 GOG rows. Gates: AUTO 148, REVIEW 33, REJECT 17, NOCAND 3. 1 Sonnet chunk, 0/10 decoys accepted.
- Opus: part a (27 rows) found 1 sample error (a rotated GOG label) -> ESCALATE. Part r (lead addition: the 20
  REJECT/NOCAND rows) recovered all 20 by whole-binary fp scans (local delta -0x10ADC0, not the run delta -0x118B30).
  Part z (consolidation): the 14 escalated rows + an AUTO rotation audit (14 checked, 2 flagged). Sample errors 3, all
  name errors (AddChemical/AddComponent swapped, 0x6C66E0 rotated). Details: `B12b/B12b_SUMMARY.md`.
- Final: verified 179, likely 2, label_suspect 20 (GOG label rotations, recorded in `GOG_LABEL_SUSPECTS.md` ## B12b).
- Tool fix: a reviewer `match` on a `mac_rotation` row now goes to the Opus 100% list (b7114f4).
- KDB (`persist B12b --apply`): insert 20, update 154, demote 14 (rotated names), 6 lessons, 179 marks; skip 5 (3
  CONFLICT rows keep trusted descriptive names of the same bodies: SetPartyMemberFlag 0x58A330, FollowState5_Hold
  0x5CA6A0, UpdatePerception 0x5D0F30 = UpdateVisibleList, which calls DoPerceptionUpdateOnCreature). Second apply 0,
  integrity ok. Backup `backups/kdb/kotor2_ghidra_knowledge.db.pre_B12b_persist_20261003`.
- Branch (a72c344, append, `tools/precheck_import.py` 0 problems): 2,569 -> 2,747 (+178, 1 confirm). `validate`
  passed, 0 duplicate addresses/names. Backup `backups/steam_db/kotor2_steam_aspyr.db.76cedbe_preB12b_20261003`.
- `check-validated`: 2,747 / 2,747 validated.
- Lead spot-checks (Steam fp = GOG fp: size, RET, n_insn, disp, imm):
  - SetInParty 0x58A330 (318 B, RET 8): `0058a3ef CALL 0x0051c280` GetServerAIMaster, `0058a3f6 CALL 0x0051e120`.
  - QuickWalk OffsetWaitDest 0x5CACD0 (286 B, RET 0x10): `005cada2 MOV dword ptr [ECX + 0x4],0x1`.
  - SetDetectMode 0x5B7A60 (51 B, RET 4): `005b7a7f MOV byte ptr [EAX + 0x510],CL`.

## B16: mod GUI and engine classes (2026-10-03)

- Scope: 149 GOG rows over 8 classes (see Roadmap). Gates: AUTO 65, REVIEW 81, REJECT 2, NOCAND 1. 2 Sonnet
  chunks, 0/20 decoys accepted.
- Opus a (42 rows) + r (3 rows; `part_r.md` was deleted by `opus-parts`, so Opus rebuilt it from evidence.tsv):
  44 verified, 1 likely (GetController 0x4233A0 getter), sample_errors 0, 8/8 AUTO checks pass. 14 corrections; the
  CSWGlobalVariableTable cluster 0x6544xx-0x6567xx was off by one function. Lead: 4 re-drawn sample rows fp-identical.
- RotateCharacterLeft/Right: `unresolved` (lead override). The GOG disp/imm sets are identical; Steam 0x850330 loads
  -10.0 and 0x850550 +10.0 (Mac signs say Left/Right), but no GOG float is quoted. The gate had Left on 0x8500F0
  (= RotateCharacter) and Right on 0x850330. Follow-up: quote the GOG constants.
- Notable: Gameplay OnPanelAdded 0x7DCEA0 is shared with CSWGuiOptionsMouse (a hook fires for both). GOG label error:
  0x920CB0 `CSWGuiSaveLoad::HandleCloudButton2` is CExoMoviePlayerInternal::ShowFadePanel.
- Final: verified 146, likely 1, unresolved 2.
- KDB: insert 84, update 35, 146 marks, skip 27 (trusted equivalents from the level-up, letterbox, texture and AI
  research, e.g. "GlobalVars GetBoolean" = GetValueBoolean 0x654740). Second apply 0, integrity ok. Backup
  `backups/kdb/kotor2_ghidra_knowledge.db.pre_B16_persist_20261003`.
- Branch (append, precheck 0 problems): 2,747 -> 2,893 (+146). `validate` passed, 0 duplicate addresses/names.
  Backup `backups/steam_db/kotor2_steam_aspyr.db.a72c344_preB16_20261003`. `check-validated`: 2,893 / 2,893.
- Lead spot-checks: Pause 0x40DE10 `0040de2c MOV [EDX+0x64],1` then pushes +0x64 to BinkPause `0040de41 CALL
  [0x009864cc]`; SetValueBoolean 0x654BB0 `00654bdf CALL 0x006545b0` (FindIdentifier); StartMovie 0x40CC70 fp =
  GOG (1975 B, RET 4, 468 insn, 12 callees).

## B11a: `CClientExoApp`, `CClientExoAppInternal` (2026-10-03)

- Scope: 429 GOG rows (52 already in the branch). Gates: AUTO 164, REVIEW 248, REJECT 14, NOCAND 3. 5 Sonnet chunks.
- Decoys: 2/50 accepted -> ESCALATE (100% Opus). Both were true counterparts: `decoy_prev` cards were built one slot
  off from mis-slotted pipeline picks (lesson). 
- Opus a-e (248 rows) + r (17 REJECT/NOCAND) + z (8 unresolved + AUTO rotation audit 45/45 clean). Sample errors 1
  (a name: GOG 0x4452F0 "StopMovie" is ShutDownMoviePlayer). Details: `B11a/B11a_SUMMARY.md`.
  - The CClientExoApp forwarders were one 0x20 slot off for most REVIEW rows; the Internal music cluster maps 1:1 in
    order (GOG 0x43A5E0-0x43B150 -> Steam 0x78E010-0x78EBA0).
  - GOG 0x44DE70-0x44E0D0 are CExoArrayList<T> template instances under Internal method names (label_suspect).
  - EndLoadBar 0x7400D0 forwards to an empty ICF body (a hook there patches nothing).
- Final: verified 408, likely 5, label_suspect 16, rejected 0, unresolved 0.
- KDB: insert 56, update 295, demote 7, 6 lessons, 408 marks; 3 CONFLICT skips keep equivalent trusted names
  (GetSWParty 0x73FB90, IsMoviePlaying 0x798C00, ActionIconToMouseCursor 0x7B3EE0). Second apply 0, integrity ok.
  Backup `backups/kdb/kotor2_ghidra_knowledge.db.pre_B11a_persist_20261003`.
- Branch (bcb4acb, append, precheck 0 problems): 2,893 -> 3,249 (+356 new, 52 confirmed at the same address). `validate`
  passed, 0 duplicate addresses/names. Backup `backups/steam_db/kotor2_steam_aspyr.db.caf65ec_preB11a_20261003`.
  `check-validated`: 3,249 / 3,249.
- Lead spot-checks (fp = GOG): GetModuleCamera 0x781810 (46 B) `0078182d MOV ECX,[EAX+0x40]`; EndLoadBar 0x7400D0
  (22 B) `007400dd CALL 0x00623320`; GetPlayerByGameObjectID 0x78BD20 (93 B, RET 4) `0078bd4e MOV ECX,[EAX+0x28]`.

## B12a: `CSWSCreature` lower half, GOG < 0x696F00 (2026-10-03)

- Scope: 201 GOG rows. Gates: AUTO 147, REVIEW 39, REJECT 13, NOCAND 2. 1 Sonnet chunk, 0/10 decoys accepted.
- Opus a (31) + r (16 REJECT/NOCAND, all recovered) + z (3 rows + AUTO audit). Sample errors 0. Details:
  `B12a/B12a_SUMMARY.md`.
  - The combat block GOG 0x6571B0-0x663170 sits at Steam = GOG + 0x83D00 (not the class run delta). Its GOG labels
    (ranged combat, ApplyOnHit*) are rotated; ResolveOnHitEffect 0x6E2FA0's dispatch order names the handlers.
  - AUTO audit: 37 rows read in the rotation windows plus a scripted same-name Mac score over all 150 AUTO rows;
    7 AUTO rows had wrong GOG names (now label_suspect). Lesson: rotation windows are not enough.
- Final: verified 171, label_suspect 29, rejected 1 (GOG 0x692F53, a 3-byte fragment).
- KDB: insert 23, update 139, demote 5, rename 10 (`B12a/kdb_overrides.tsv`: e.g. 0x6E51D0 CreateLinkedEffect_2,
  0x57C990 GetNearestEnemy, 0x57A960 ResolveInitiative), 6 lessons, 171 marks. Second apply 0, integrity ok. Backup
  `backups/kdb/kotor2_ghidra_knowledge.db.pre_B12a_persist_20261003`.
- Branch (db4f5cf, append, precheck 0 problems): 3,249 -> 3,420 (+171). `validate` passed, 0 duplicates.
  `check-validated`: 3,420 / 3,420.
- Lead spot-checks (fp = GOG): ResolveMeleeSpecialAttack 0x6DF2A0 (4075 B, RET 0x14, 973 insn) `006df337 CALL
  0x0051c280`; DoCombatStep 0x6E3A40 (3044 B, RET 0xC, 786 insn) `006e3aa2 CALL 0x0051c0a0`; GetPortrait 0x563390
  (100 B, RET 4, disp 0x1198) `005633e6 CALL 0x00710840`.
- Open: the real `Rest` is unlocated (both GOG `Rest` labels are wrong).
