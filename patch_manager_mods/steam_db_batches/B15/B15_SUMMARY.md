# B15 summary: NWScript `ExecuteCommand*` command handlers (2026-10-03)

Scope: 656 GOG rows (classes `Global` 638 and `CSWVirtualMachineCommands` 18, name `^ExecuteCommand`): the 655
`ExecuteCommand*` handler rows plus the VM dispatcher `CSWVirtualMachineCommands::ExecuteCommand`.

Inputs:
- `evidence.tsv` (gates: AUTO 474, REVIEW 182) and `final_rows.tsv` (pre-Opus finalize);
- `review/sonnet_c1..c4.tsv`, `cards/key.tsv` (40 decoys);
- `cmdtable.tsv`, `cmdtable_rows.tsv`, `cmdtable_report.md` (`tools/cmdtable.py`, DESIGN 4.8);
- `opus_gate.md/json` (ESCALATE: 1 decoy accepted);
- Opus parts `opus/overrides_a.tsv/.md` (103 rows) and `opus/overrides_b.tsv/.md` (100 rows).

Output of this consolidation: `overrides.tsv` (200 rows), `../GOG_LABEL_SUSPECTS.md` (new B15 subsection), lessons in
`gate_report.md ## Lessons`. Nothing was run: `finalize B15 --opus-sample-errors 0` is the lead's next step.

## Pre-Opus finalize (for reference)

| verdict | rows |
|---|---|
| verified | 571 (AUTO 474 + REVIEW 97) |
| likely | 78 |
| unresolved | 5 |
| label_suspect | 2 |
| **total** | **656** |

## How the overrides were merged

- Part a: the 26 rows of the 100% list plus 77 sample rows (102 verified, 1 label_suspect). Part b: 79 sample rows,
  the 8 AUTO meaning checks, a 10-row AUTO method audit (one row overlaps the meaning checks), the slot-156 pair and the
  dispatcher (98 verified, 2 label_suspect).
- Union = 200 GOG addresses: all 182 REVIEW rows (so the ESCALATE rule, 100% Opus read of reviewer rows, is met) plus
  18 AUTO rows (8 meaning checks + 9 more audit rows + the dispatcher 0x77E5B0).
- Both parts re-decoded all 877 slots on GOG, Steam and Mac by themselves: **0 table decode errors**. sample_errors 0 in
  both parts.
- Overlap (3 GOG addresses):
  - 0x793C50 GetHasSpellEffect: both verified at 0x67E670; notes merged (`[a+b]`).
  - 0x78C440 EffectTemporaryForcePoints: same Steam address 0x676E60; a = verified, b = label_suspect. **Ruling:
    verified (GOG parity)**, see below.
  - 0x78D6B0 EffectTemporaryFP: both label_suspect at 0x6780D0. **Ruling: verified (GOG parity)**.
- Reviewer label_suspect on GetEmotions (0x791A10) and IsAIState (0x7A0B00) is **overruled**: these are source names
  that differ from the nwscript names, and both are the Mac x86 real symbols of their slot sets (Mac 0x3400AE slots
  235-237, Mac 0x3498F4 slot 140). Part a already ruled `verified`; the notes now say so.
- Every other row is taken from its part with a `[a]` / `[b]` tag. All notes are <= 200 chars.
- Part b side note corrected: it says the reviewer correction for GetCreatureTalent (0x790420) was 0x67AC20. That is
  wrong. 0x67AC20 was the gate pick, and the reviewer (sonnet_c2) corrected it to 0x67AE40, which is the table
  counterpart and part a's final. The ship result does not change.

## Slot-156 ruling (user decision 2026-10-03: GOG parity)

All three binaries store slot 156 (nwscript `EffectTemporaryForcePoints`) twice. The later store is the runtime
handler:
- Steam: `006667bd MOV [EDX+0x270],0x6780d0`, overwritten by `00666bed MOV [ECX+0x270],0x676e60`.
- GOG: `0077bd9d` -> 0x78D6B0, overwritten by `0077c1cd` -> 0x78C440.
- Mac: `0032566d` -> 0x333C92, overwritten by `00325c72` -> 0x337B80.

The Mac x86 names are swapped against the GOG names. Mac live 0x337B80 is `ExecuteCommandEffectTemporaryFP` (295 B,
10 callees, disp 0x8 x5,0xA,0xA,0x34). Mac dead 0x333C92 is `ExecuteCommandEffectTemporaryForcePoints` (165 B, 7
callees, disp 0x8).

The GOG name of the live handler is the nwscript name of its slot (`name_slot = yes`). The branch DB already ships
`CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints` at 0x676E60 (B0, validated). So both rows ship
at their GOG body counterparts:

| GOG | name | Steam | verdict | anchors (checked on disk) |
|---|---|---|---|---|
| 0x78C440 | `CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints` | 0x676E60 | verified | `00666bed` last store; `00676f8f CALL 0x005e5ea0` CGameEffect::SetCreator; fp = GOG (418 B, 127 insn, RET 8) |
| 0x78D6B0 | `Global::ExecuteCommandEffectTemporaryFP` | 0x6780D0 | verified | `006667bd` store overwritten at `00666bed`; 0 callers; fp = GOG (282 B, 84 insn, 7 callees, no SetCreator) |

GOG_LABEL_SUSPECTS.md records the pair as "Mac names differ; GOG label follows nwscript; shipped at GOG parity". It is
not listed as a confirmed GOG label error.

The Mac-semantic alternative (each name at its Mac body) was written first and then withdrawn on the user's decision.
It would have moved the B0 row to the dead body.

## The 7 Opus corrections (table counterparts the gate missed)

In every case, the gate ranked a `near` candidate above the `cmdtable` candidate. 6 of the 7 picks were already held by
another name (G6). Each wrong pick is another slot's handler. All 7 finals equal the table counterpart, have an
identical fp to GOG, and match the reviewer `corrected_addr`.

| GOG | name | gate pick (really) | final | slots | anchor |
|---|---|---|---|---|---|
| 0x784680 | ChangeFaction | 0x66F330 (ClearAllActions, slot 9) | 0x66F0A0 | 173, 412 | `0066f0cd CMP [EBP+8],0xad`, `0066f0f5 CMP 0x19c` |
| 0x790420 | GetCreatureTalent | 0x67AC20 (GetCommandable, 163) | 0x67AE40 | 307, 308 | `0067aea8 CMP [EBP+8],0x134`; CALL GetCreatureTalentRandom |
| 0x791A10 | GetEmotions | 0x67C250 (GetEffectSpellId, 305) | 0x67C430 | 235-237 | stores `00667498/006674a8/00667518`; `0067c545 [ECX+0x398]` |
| 0x798D10 | GetMatchedStringsNum | 0x6837C0 (GetMetaMagicFeat, 105) | 0x683740 | 195 | `006679b2` store; `00683790 [ECX+0x1bc]` |
| 0x799310 | GetNearestObject | 0x6839A0 (GetModuleFileName/Name, 210/561) | 0x683D40 | 38, 226-229 | `00683de6 CMP [EBP+8],0x26`, `00683dec CMP 0xe2` (4828 B) |
| 0x7A0B00 | IsAIState | 0x68B3C0 (IntToString, 92) | 0x68B4B0 | 140 | `0068b55f CMP [EAX+0x11ac],0` |
| 0x7A2530 | MoveToPoint | 0x68CB50 (ActionMoveToObject, 22/383) | 0x68CEE0 | 21, 382 | `0068cf3b CMP [EBP+8],0x17e`, `0068cfef CMP 0x15` |

## Decoy key error B15-4-38

- The card was GOG 0x784680 (ChangeFaction) -> 0x66F0A0, kind `decoy_prev`, and the reviewer accepted it.
- 0x66F0A0 is the true counterpart: the table handler of slots 173/412, and the Opus final.
- Decoys are taken as same-retn neighbours of the gate's pick. Here the pick (0x66F330) was wrong, so its neighbour was
  the right function.
- This is a key error, not a reviewer failure. It is not a sample error.
- Fix F1 (DESIGN 4.6):
  - `cards` skips proof-source (cmdtable) candidates when it picks decoys;
  - finalize lists an accepted decoy that an Opus override ships verified/likely under `## Decoy key errors` (json
    `key_errors`) and leaves it out of `decoys_accepted`.
- After the re-finalize, expect `decoys_accepted 0`, `key_errors 1`, sample_errors 0. Not run here: confirm on the
  re-run.

## Label suspects

- **No confirmed GOG label error in B15.** 653 of the 655 handler rows carry the Mac symbol of their slot set. The 2
  exceptions are the slot-156 pair (Mac names swapped; GOG follows nwscript; GOG parity).
- About 152 rows have source names that are not nwscript names (`name_slot n/a`), for example GetEmotions, IsAIState
  and GetPartyLeader (slots 238/845, which Mac also calls GetPCSpeaker). These are `verified`, not label_suspect.
- **GOG 0x44F600 `CSWArea::GetIsIDInExcludeList` is an ICF fold, confirmed with Mac.** It is also the handler of
  command slots 321/327/403 (GetLastAssociateCommand, SetAssociateListenPatterns, ExploreAreaForPlayer; Steam 0x681880,
  15 B `XOR EAX,EAX; RET 8`).
  - Mac x86 has four separate 7 B `return 0` stubs:
    - `ExecuteCommandGetLastAssociateCommand` 0x341974;
    - `ExecuteCommandSetAssociateListenPatterns` 0x34CDD6;
    - `ExecuteCommandExploreAreaForPlayer` 0x338B7C;
    - `CSWArea::GetIsIDInExcludeList` 0x270766.
  - MSVC folded them into one body.
  - The GOG label is correct, but it is not the only name of the body. It is recorded under Confirmed in
    GOG_LABEL_SUSPECTS.md (ICF fold row).
  - The three command names have no GOG DB row, so they are not shipped.
  - Mac has 658 handlers and Windows 655: these three stubs are the difference.

## Expected final verdicts after `finalize B15 --opus-sample-errors 0`

Computed by applying `overrides.tsv` (the override's verdict and steam_addr) to `final_rows.tsv`:

| verdict | rows |
|---|---|
| verified | **656** |
| likely | 0 |
| label_suspect | 0 |
| rejected | 0 |
| unresolved | 0 |

- Moves: likely -> verified 78, unresolved -> verified 5, label_suspect -> verified 2. Steam address changed in 7 rows
  (the 7 corrections).
- Tiers: opus 200, AUTO 456.
- Checks:
  - no Steam address is shipped under two names, and no name appears twice;
  - every shipped address equals the `cmdtable` counterpart, except the dispatcher (not a table handler) and the dead
    slot-156 store (`overwritten_slot_156` counterpart, 0x6780D0).
- `functions.csv` should have 656 rows.
- **Coverage check:** 0 rows are non-AUTO or non-`verified` before Opus and have no override row. All 182 REVIEW rows
  and all 85 non-verified rows are covered.
- Branch DB (read-only check): the 18 existing `CSWVirtualMachineCommands::ExecuteCommand*` rows (from B0) all equal
  the B15 finals (same class, name and address).
  - Expected import result: 18 updated in place and 638 new `Global` rows, so 1683 + 638 = **2321** rows.
  - Confirm that `import-ghidra --mode append` matches on class + name.

## Open items

1. Run `finalize B15 --opus-sample-errors 0` and check the counts above. Also check that `recheck.tsv` (8 entries, 5 GOG
   addresses) and `gog_label_suspects.tsv` are all superseded by the overrides (the 7 corrections).
2. `opus_gate` 100% list noise: 18 of the 25 rows listed as `reviewer corrected_addr` came from reviewers who rightly
   rejected a decoy card and named the real address. Only 7 were real corrections. Fix: build the list without decoy
   cards (`cards/key.tsv` kind != real). This did not matter here, because B15 escalated to 100%.
3. KDB at persist time:
   - 571 rotated `gog_cluster_delta_map.py`/medium ExecuteCommand rows. For example, 0x67F9B0 is named
     GetIsDoorActionPossible but is GetIsDay, slot 405. Persist replaces them.
   - 9 `engine-ai-atlas wave2`/high descriptive rows (e.g. 0x67C430 "GetIsEnemy/Friend/Neutral (235-237)") stay as
     trusted holders. Decide on kdb_overrides renames to the GOG/Mac names.
   - 0x66C7A0 "LevelUpHenchman" (`level-up-P1`/medium) -> `ExecuteCommandAddMultiClass` (slot 389, store `006662e9`).
4. `LevelUpOverride.cpp:103` labels 0x66C7A0 "LevelUpHenchman". The address is right and the label is wrong (it is
   ExecuteCommandAddMultiClass). Doc 08 residual line 413 is resolved: the routine id is 389.
5. 0x44F600 ICF stub: the three command names (GetLastAssociateCommand / SetAssociateListenPatterns /
   ExploreAreaForPlayer -> Steam 0x681880) have no GOG DB row. Decide whether to add them as extras (one address under
   three names breaks the R4 uniqueness rule, so at most one could ship).
6. TBL anchor family (an exact unique slot set counts toward C2): deferred to an Opus calibration review before B10b
   (DESIGN 4.8).
7. Gate rule follow-up: in 6 of the 7 misses the gate kept a G6-held `near` pick while an unheld `cmdtable` candidate
   existed (5 of those 6 also had `mapper:vote`). Ranking unheld candidates first would have avoided those 6, even
   without TBL. GetMatchedStringsNum was a `near` pick that was not held.
