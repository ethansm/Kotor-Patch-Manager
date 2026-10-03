# B9 summary: Server core and world lifecycle (2026-10-03)

Scope: 270 GOG rows. By class: `CServerExoApp` 118, `CServerExoAppInternal` 98 and `CSWSModule` 54. (The README's
45/87/138 split is wrong.)

Inputs:
- `evidence.tsv`. Gates: AUTO 102, REVIEW 166, NOCAND 1, REJECT 1.
- `final_rows.tsv`, pre-Opus finalize. This is the git version at 284967e: the working file was overwritten by the
  92569e5 finalize.
- `review/sonnet_c1..c4.tsv` and `cards/key.tsv` (40 decoys).
- `opus_gate.md/json`: ESCALATE, because decoy B9-3-29 was accepted.
- The Opus parts, from `opus/parts.json`, which has 3 disjoint parts and 174 rows:
  - `opus/overrides_a.tsv/.md`: 60 rows (verified 57, likely 2, label_suspect 1);
  - `opus/overrides_b.tsv/.md`: 60 rows (verified 58, likely 2);
  - `opus/overrides_c.tsv/.md`: 54 rows (verified 54, including the 8 AUTO meaning checks, all of which pass).
  - sample_errors is 0 in all three parts.
  - The README says a = 59/1/0 and c = 52/1/1. That is wrong. The union totals are unchanged.

This consolidation writes four files:
- `overrides.tsv`: 176 rows;
- `../GOG_LABEL_SUSPECTS.md`: a new B9 subsection and a param-size note;
- `gate_report.md`: 8 entries under `## Lessons`;
- this file.

Nothing was run. **B9 was already finalized, persisted and imported in 92569e5 (branch 4b81a2b) from the 174-row
merge.** The two rows added here need a re-run. See the open items.

## Pre-Opus finalize (for reference)

| verdict | rows |
|---|---|
| verified | 182 (AUTO 102 + REVIEW 80) |
| likely | 76 |
| unresolved | 10 (REVIEW 9 reviewer corrections + NOCAND 1) |
| label_suspect | 1 (Uninitialize) |
| rejected | 1 (REJECT SetMoveToWorldTime) |
| **total** | **270** |

## How the overrides were merged

- The three parts are disjoint: 174 GOG addresses with no overlap and no conflict. Each row is copied with a `[a]`,
  `[b]` or `[c]` tag. All notes are 200 characters or fewer, with no tabs or double quotes.
- The union covers:
  - all 166 REVIEW rows (23 rows on the 100% list and 143 sample rows; ESCALATE means 100% of the reviewer matches
    get an Opus read);
  - the 8 AUTO meaning checks (5 sig, 3 c2).
- Part b's findings on part a's rows all agree with part a. I checked each one on disk (fp.tsv on both sides):

| row | part a final | check |
|---|---|---|
| ExoApp RemoveFromExclusionList 0x64C1E0 | 0x51C7C0 | 33 B RET 8. Callee 0x5394C0, not 0x5393B0 (Add) |
| Internal RemoveFromExclusionList 0x7BC710 | 0x5394C0 | 220 B / 68 insn = GOG. Add is 260 B / 76 insn (0x5393B0) |
| Internal GetPauseState 0x7BBF20 | 0x538CD0 | 36 B / 14 insn, no callees = GOG. 0x538C20 is TogglePauseState (47 B, calls Get + Set) |
| Internal GetMoveToWorldTime 0x7BFA90 | 0x53CCF0 | 117 B / 35 insn = GOG. `0053ccfd MOV EDX,[ECX+0x1009c]` reads |

- **Two new rows from this gate** (`[gate]` tag):
  - SetMoveToWorldTime (REJECT row);
  - DebugValidatePath (NOCAND row).
  - Both rulings are below.

### SetMoveToWorldTime ruling (GOG 0x7BFA20, gate REJECT 0x53C5C0): corrected to 0x53CC80, `verified`

Part b found it. I checked it on disk:
- The GOG fp `0x7bfa20 103 28 28 ... imm 0x1c` equals the Steam fp of 0x53CC80 (103 B, RET 0x1C, 28 insn, imm 0x1C).
  The fp drops displacements above 0x10000.
- Steam asm:
  - `0053cc8d MOV [EAX+0x1009c],ECX`, `0053cc99 MOV [EDX+0x100a0],AL`, ... `0053ccda MOV [EAX+0x100a8],CX`: it writes
    7 fields from 7 arguments;
  - `0053cce4 RET 0x1c`.
- The Get twin 0x53CCF0 (117 B / 35 insn = GOG 0x7BFA90) reads the same 7 fields into its out pointers.
- Twin and caller consistency:
  - the Set/Get pair has the same run delta as GOG (GOG = Steam + 0x282DA0, the same for both);
  - the ExoApp forwarders call them: Steam Set 0x51CFF0 -> 0x53CC80 and Get 0x51D030 -> 0x53CCF0, the same as GOG
    0x64CA10 -> 0x7BFA20 and 0x64CA50 -> 0x7BFA90;
  - Mac has Internal Set 77 B and Get 91 B (Set is the smaller one).
- The gate pick 0x53C5C0 (191 B fastcall, RET 0, 2 callees) is unrelated.
- 0x53CC80 has no holder in the branch DB and no KDB name_map row. Param size: Steam RET 28; the GOG DB value is 26,
  which rounds up to 28.

### NOCAND row DebugValidatePath (GOG 0x662B80): `label_suspect` at 0x6E6880

- The GOG address is not in the CServerExoAppInternal block; it sits in creature code. A whole-binary fp search finds
  one Steam body that is identical to GOG: 0x6E6880 (446 B, RET 0, 127 insn, 7 callees, same disp/imm). Its neighbour
  run matches too (GOG 0x662A30 324 B RET 8 = Steam 0x6E6730).
- The body is `CSWSCreature::GetFirstAttacker`, the Mac symbol at 0x378234: the same disp 0x194/0x198/0x548/0x11A4/0x11A8,
  imm 0x7F000000, and callee GetCreatureByGameObjectID.
- The Steam body's only caller is `ExecuteCommandGetAttacker` 0x696BF0 (B15, high). Mac GetAttacker calls
  GetFirstAttacker in the same way.
- The counterpart is right, but the GOG name contradicts the body. Nothing is shipped.

## The 10 address corrections

9 were proposed by the reviewers (`corrected_addr`) and confirmed by part a. The SetMoveToWorldTime correction came
from part b and this gate. Every wrong pick is a same-shape or adjacent neighbour.

| GOG | name | gate pick (really) | final | anchor |
|---|---|---|---|---|
| 0x64B640 | CServerExoApp::Constructor | 0x51BCC0 (GetFPS 17 B) | 0x51BC20 | `0051bc5a PUSH 0x1f350` (= GOG imm), `0051bc7a CALL 0x52ebe0` Internal ctor |
| 0x64C140 | CServerExoApp::GetPauseState | 0x51C6F0 (IsInjured) | 0x51C720 | `0051c732 CALL 0x538cd0` Internal::GetPauseState |
| 0x64C1E0 | CServerExoApp::RemoveFromExclusionList | 0x51C790 (Add) | 0x51C7C0 | `0051c7d6 CALL 0x5394c0` (220 B Remove) |
| 0x64C500 | CServerExoApp::UpdateSubareasOnMoveTo | 0x51CA40 (ComputeHeight) | 0x51CAE0 | `0051caf9 CALL 0x539e30` (245 B = GOG 0x7BD080) |
| 0x64C940 | CServerExoApp::ShrinkLineByCreatures | 0x51CC00 (GetRoomIndex) | 0x51CF20 | `0051cf39 CALL 0x53aea0` (47 B = GOG 0x7BE0F0) |
| 0x733EF0 | CSWSModule::non_virtual_thunk_to_dtor | 0x5615C0 (64 B helper) | 0x561C80 | `00561c80 SUB ECX,0x1c` / `JMP 0x556cf0` |
| 0x7BBF20 | Internal::GetPauseState | 0x538C20 (TogglePauseState) | 0x538CD0 | `00538cda MOVZX ECX,[EAX+0x10078]`, 36 B, no callees |
| 0x7BC710 | Internal::RemoveFromExclusionList | 0x5393B0 (Add) | 0x5394C0 | 220 B = GOG; `0053951a CALL 0x64dde0` list remove |
| 0x7BFA20 | Internal::SetMoveToWorldTime | 0x53C5C0 (REJECT, unrelated) | 0x53CC80 | `0053cc8d MOV [EAX+0x1009c],ECX` writes; RET 0x1C |
| 0x7BFA90 | Internal::GetMoveToWorldTime | 0x53CC80 (the Set twin) | 0x53CCF0 | `0053ccfd MOV EDX,[ECX+0x1009c]` reads; 117 B |

In 7 of the 10 rows the right address was already a candidate, but the gate ranked a `near` pick above it. In the other
3 rows the right address was not a candidate at all:
- GetMoveToWorldTime: the mapper `drift+0x440` landed on the Set twin;
- the dtor thunk: `drift-0x2a0` missed it;
- SetMoveToWorldTime: its near window missed 0x53CC80.

## Decoy B9-3-29 (accepted): genuine reviewer error, not a key error

- The card was GOG 0x64C780 `SetIgnoreConversationStartRange` -> 0x51CD10 (`decoy_prev`). Sonnet c3 row 30 accepted
  it.
- 0x51CD10 is `SetActionConversation`: `0051cd28 MOV [EDX+0x1f31c]`, the field that GetActionConversation 0x51CD40
  reads. Trusted `ExecuteCommandBeginConversation` calls it at `0066e0b4`.
- The gate pick 0x51CD60 (`0051cd78 MOV [EDX+0x1f320]`) is right. Its only distinguishing caller is Steam 0x7CEB80. That
  caller's fp equals GOG `CGuiInGame::SetGlobalDialogState` 0x4DF280 (215 B, same disp/imm), and GOG
  SetGlobalDialogState calls 0x64C780. Mac SetGlobalDialogState calls `SetIgnoreConversationStartRange` 0x324738.
- The reviewer's evidence, the Door/Placeable EventHandler callers, is shared by both setters, so it does not decide.
- The decoy pair is not shipped by any override: 0x64C780 ships at 0x51CD60, and 0x51CD10 ships under
  SetActionConversation. So finalize keeps `decoys_accepted 1` and `key_errors 0`. That is correct, and the ESCALATE
  (a 100% Opus read) is satisfied.

## Label suspects (both `label_suspect`, not shipped)

- **0x7BDA10 `CServerExoAppInternal::Uninitialize` -> Steam 0x53A7C0.** The body is a TempDeathUpdate helper:
  `[[0x00a1b4a4]+4]` is the client app, then `0053a7d7 CALL 0x7403a0` calls CClientExoApp::RunDeathSequence (= GOG
  0x40D860). Its only caller is TempDeathUpdate 0x53A0C0.
  - Mac Internal::Uninitialize 0x399CF4 is an empty 5 B stub.
  - Mac TempDeathUpdate calls RunDeathSequence directly (the helper is inlined on Mac).
  - Where the real Uninitialize lives on Windows is still unknown.
- **0x662B80 `CServerExoAppInternal::DebugValidatePath` -> Steam 0x6E6880** = `CSWSCreature::GetFirstAttacker` (see
  above). Mac has no DebugValidatePath symbol. Linux has `CServerExoAppInternal__DebugValidatePath` 0x085CFC34 (633 B).
  GetNextAttacker is probably GOG 0x662D40 (FUN_) -> Steam 0x6E6A40 (123 B, fp equal; shape only).
- No rotation: the AUTO neighbours of 0x7BDA10 have Mac-consistent shapes (CreatureAcquireItem, TestSafeLocation,
  ComputeSafeLocation, UpdateMapData, ForceRunAreaOnExitScripts).
- Not a B9 row, but corroborated here: GOG 0x64C2F0 `CTwoDimArrays::GetSwoopUpgradeRowProvider` sits in the
  CServerExoApp block and is `CServerExoApp::GetPartyTable` (Mac 0x324362; Steam 0x51C8D0 `ADD EAX,0x1f0b4`). GOG and
  Steam TempDeathUpdate call it where Mac TempDeathUpdate calls GetPartyTable. It was already Confirmed in the ledger
  (B0).
- GOG DB param sizes: 15 rows disagree with the RET n of the GOG function itself, and the Steam RET n is the same as
  GOG's. Part a/c found 5 of them: PlayerListChange 8->12, SaveModuleStart 0->8, CopyModuleToCurrentGame 8->12,
  ExoApp GetRoomIndex 0->12, Internal SaveGame 12->16. The full list is in GOG_LABEL_SUSPECTS.md. This does not affect
  Steam, because functions.csv uses the Steam RET n and notes the GOG value.

## Likely rows (4): counterpart right, only positional or thin anchors

| GOG | name | Steam | why not verified |
|---|---|---|---|
| 0x64B6E0 | CServerExoApp::GetFPS | 0x51BCC0 | 17 B `FLD [0x98c270]` (100.0f constant), no member access, 0 callers on both sides; Mac GetFPS is 11 B with no member access |
| 0x64BCE0 | CServerExoApp::GetNetLayer | 0x51C2C0 | `0051c2cf MOV EDX,[ECX+0x10040]` getter with 0 callers on both sides; Mac order only |
| 0x64BFC0 | CServerExoApp::GetConnectionLib | 0x51C5A0 | `[ECX+0x10058]` getter, 0 callers, no other reader; the only 31 B slot between GetPlayerListString and StartServices |
| 0x729480 | CSWSModule::non_virtual_thunk_to_AsSWSModule | 0x556CD0 | 17 B `SUB EAX,0x1c` stub (GOG imm 0x1C), 0 callers; it sits before the dtor, as in GOG |

They are not exported unless `--with-likely` is used.

## Expected final verdicts after `finalize B9 --opus-sample-errors 0`

I computed these by applying `overrides.tsv` (the verdict and steam_addr of each override) to the pre-Opus
`final_rows.tsv`:

| verdict | rows |
|---|---|
| verified | **264** |
| likely | 4 |
| label_suspect | 2 |
| rejected | 0 |
| unresolved | 0 |

- Moves:
  - likely -> verified: 72;
  - unresolved -> verified: 9;
  - rejected -> verified: 1;
  - unresolved -> label_suspect: 1.
- The Steam address changes in 10 rows (the corrections above).
- Tiers of shipped verified rows: opus 170, AUTO 94. `functions.csv` should have 264 rows. Compared with the shipped
  92569e5 state, it adds one row: `CServerExoAppInternal::SetMoveToWorldTime` 0x53CC80, RET 28.
- **Duplicate check:** no Steam address is held by two names among the 270 verified, likely or label_suspect rows, and
  no name sits at two addresses. No ICF fold is involved.
- **Coverage check:** before this gate, 168 rows were non-AUTO or non-`verified`. Two of them had no part override:
  - NOCAND DebugValidatePath -> now `label_suspect`;
  - REJECT SetMoveToWorldTime -> now `verified` 0x53CC80.
  - After this gate, 0 rows are uncovered.
- Every shipped and label_suspect row has a GOG fp size and RET equal to the Steam fp. All 118 CServerExoApp rows have
  fully identical fingerprints at Steam = GOG - 0x12FA20 (a cross-check only).

## Open items

1. Re-run `finalize B9 --opus-sample-errors 0` and check the counts above. Expect `decoys_accepted 1`, `key_errors 0`,
   sample_errors 0, and a `recheck.tsv` with only its header, as in the 92569e5 run (all 9 reviewer corrections ship
   through overrides).
2. B9 is already live in the branch DB (4b81a2b, 2,568 rows) and the KDB:
   - Re-run `persist B9 --apply`, which adds 1 mark and 8 lessons.
   - Re-import with `import-ghidra --mode append`, which should add 1 row (2,568 -> 2,569). Then run `SqliteTools
     validate` and `check-validated`.
3. README B9 section: fix the class counts (118/98/54) and the per-part counts (a = 57/2/1, c = 54/0/0). Add the 2 new
   rows and the final counts 264/4/2/0/0.
4. KDB: 0x6E6880 is named `CSWSCreature::ApplyOnHitInstantDeath` (gog_cluster_delta_map, low). That is wrong: the
   body is GetFirstAttacker. Decide whether to add a `kdb_overrides.tsv` rename. Check what persist does with the
   label_suspect row at that address (dry-run first).
5. Extras with no GOG DB row: `CSWSCreature::GetFirstAttacker` 0x6E6880, `GetNextAttacker` 0x6E6A40 (unverified) and
   `CServerExoApp::GetPartyTable` 0x51C8D0. Not shipped.
6. To locate: the real `CServerExoAppInternal::Uninitialize` and `DebugValidatePath` on Windows. Either may be
   compiled out or ICF-folded.
7. Tool follow-ups (gate_report Lessons):
   - add an fp-exact candidate source inside the class run window, because 3 of the 10 misses had no right
     candidate;
   - rank by content over `near`, because in 7 of the 10 the right candidate was present.
8. Upstream: the 15 GOG DB param-size errors are for PR text only. The GOG DB stays read-only.
