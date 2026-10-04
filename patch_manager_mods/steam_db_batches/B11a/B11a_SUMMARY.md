# B11a summary: CClientExoApp + CClientExoAppInternal (2026-10-03)

Scope: 429 GOG rows. CClientExoApp has 225 (GOG 0x40C3D0-0x40E850, mostly `MOV ECX,[this+4]; CALL Internal::X` forwarders and
field getters). CClientExoAppInternal has 204 (GOG 0x42DB10-0x460D20).

Inputs:
- `evidence.tsv`. Gates: AUTO 167 (route c2 66, sig 101), REVIEW 245, REJECT 14, NOCAND 3.
- `final_rows.tsv`: the pre-Opus finalize at 8a4feb6 (counts below). The working file is the 32781c8 finalize with parts a-e+r.
- `review/sonnet_c1..c5.tsv`: 5 Sonnet chunks with 298 cards (248 real, 50 decoys: 26 prev / 24 next).
- `opus_gate.md/json`: **ESCALATE**. A decoy was accepted (B11a-2-11) and one key error was found (B11a-3-52), so 100% of the
  reviewer matches got an Opus read.
- The Opus parts are disjoint, cover 273 GOG addresses and have no conflict:
  - `opus/overrides_a.tsv/.md`: 60 rows (verified 51, likely 3, label_suspect 6), 23 corrections.
  - `opus/overrides_b.tsv/.md`: 60 rows (verified 56, likely 1, label_suspect 3), sample_errors 0.
  - `opus/overrides_c.tsv/.md`: 60 rows (verified 60), sample_errors 0.
  - `opus/overrides_d.tsv/.md`: 60 rows (verified 58, likely 1, label_suspect 1). sample_errors 1: StopMovie, a label error.
  - `opus/overrides_e.tsv/.md`: the 8 AUTO meaning checks (8 verified, all pass).
  - `opus/overrides_r.tsv/.md`: 17 REJECT/NOCAND rows (17 verified, 16 corrections).
  - `opus/overrides_z.tsv/.md`: 8 rows (verified 2, label_suspect 6, 7 corrections) plus the AUTO rotation audit (45 checked, 0 flagged).
- `overrides.tsv` holds a-e+r (265 rows). **Part z is not merged yet** (open item 1).

This consolidation wrote these files:
- `opus/overrides_z.tsv/.md`;
- this file;
- a `## Lessons` section in `gate_report.md`;
- `gog_ledger_section.md`, the `## B11a` block for `../GOG_LABEL_SUSPECTS.md`. It is not pasted into that file yet, because another agent is editing it.

Nothing was run and no DB was written.

## Pre-Opus finalize (8a4feb6, for reference)

| verdict | rows |
|---|---|
| verified | 261 (AUTO 167 + REVIEW 94) |
| likely | 88 (REVIEW) |
| label_suspect | 9 (reviewer label_suspect) |
| unresolved | 57 (REVIEW 54 + NOCAND 3) |
| rejected | 14 (REJECT) |
| **total** | **429** |

## Decoys and sample errors

- **Both "accepted" decoys were true counterparts.** In both cases the decoy generator built a `decoy_prev` from a
  pipeline pick that was itself one slot off:
  - B11a-2-11: 0x4381A0 -> 0x78BBC0 is the real scalar deleting dtor. The pipeline's 0x78BCC0 is GetPlayerByPlayerID.
  - B11a-3-52: 0x40D250 -> 0x73FD70 is the real StopMenuMusic forwarder. The pipeline's 0x73FD90 is StopLoadMusic.
  - The reviewer was right both times, so the ESCALATE was not a reviewer fault. The 100% Opus read was still useful.
- `sample_errors: 1` (part d): 0x4452F0 `StopMovie`. The reviewer said `match`; the counterpart is right, but the body is
  ShutDownMoviePlayer, a label error. No sampled reviewer `match` had a wrong address. Parts a, b, c, e, r and z found 0.

## The 48 address corrections (final Steam address != gate pick)

By part: a 23, r 16, z 7, b 2. By class: App 21, Internal 27. Causes:
1. **App forwarder slot shift** (part a; part z for EndLoadBar).
   - GOG App::EndLoadBar 0x40C510 has no slot in the Steam App block. Steam relinked it to 0x7400D0, between GetMiniGame
     0x7400A0 and PlayLoadModuleMovies 0x7400F0.
   - GetClientOptions sits at Steam 0x72FB00, inside the CExoInput forwarder block.
   - So the positional pick for GOG 0x40C510-0x40D580 was one slot off. This is the G6 `held_by` chain in
     `gate_report.md`: GetFPS took GetConnectionLib, BeginIntro took GetModuleCamera, and so on.
   - Every App forwarder was re-anchored by its forwarded Internal callee.
2. **Internal one-slot shift in the REJECT rows** (part r). DisplayMainMenu, HandleMessage, ShowObject, HideLoadScreen,
   SetMouseMode, StartMenuMusic and AquireLoadMusic each took the neighbour's body. The music cluster GOG 0x43A5E0-0x43B150
   is a strict monotone bijection onto Steam 0x78E010-0x78EBA0.
3. **Template-instance cluster GOG 0x44DE70-0x44E0D0** (parts b, z). Compiler-generated `CExoArrayList<T>` instances
   carry Internal names. Steam scatters these instances, so the drift picks hit neighbouring instances. Six of the
   counterparts were found only by a whole-binary scan of own fp + callee fp, plus caller sets: 0x6D2CB0, 0x838F70,
   0x7A18C0, 0x7A1900, 0x8A6110 and 0x7A19A0.
4. NOCAND fills (part r): StartServices 0x788D90, GetPlayerByGameObjectID 0x78BD20, SelectNearestObject 0x79B700.

## Label-rotation findings (GOG DB names; every Steam counterpart is fp 1:1 with the GOG body)

All 16 label_suspect rows are recorded in `gog_ledger_section.md` (`## B11a`).

| GOG | GOG label | body is | Steam (body) | name really at (GOG -> Steam) |
|---|---|---|---|---|
| 0x430300 | StallServerCode | CExoString getter on this+0x10 | 0x783D80 | to locate (Mac 97 B, +0x264) |
| 0x431750 | DestroyQuestJournal_2 | member-array dtor (+0x44) | 0x785130 | DestroyQuestJournal = 0x4316E0 -> 0x7850C0 |
| 0x4317F0 | AddToPauseExclusionList_2 | dtor of 2 arrays | 0x7851D0 | 0x430CA0 -> 0x784680 |
| 0x4381A0 | ShutDownMoviePlayer | scalar deleting dtor | 0x78BBC0 | 0x4452F0 -> 0x798CD0 |
| 0x43A510 | EndLoadBar | Internal::GetNearestVideoMode | 0x78DF40 | EndLoadBar is ICF-folded into return_void 0x6133B0 -> 0x623320 |
| 0x43A5E0 | AddCreatureModelDirectlyToIgnoreList | GetExactVideoMode wrapper | 0x78E010 | 0x43A920 (`_2`) -> 0x78E350 |
| 0x444B60 | IndicateObject | Internal::GetLoadScreenVisible | 0x798540 | 0x443170 (label CSWCCreature::SwitchMode) -> 0x796B50 |
| 0x4452F0 | StopMovie | Internal::ShutDownMoviePlayer | 0x798CD0 | Mac StopMovie is a 5 B stub; to locate |
| 0x44DE70 | IsMoviePlaying(_2) | CExoArrayList<SJournalEntry> dtor | 0x6D2CB0 | 0x445220 -> 0x798C00 |
| 0x44DEB0 | GetPausedByCombat | CExoLinkedList RemoveTail wrapper | 0x5936D0 | 0x448500 (FUN_) -> 0x79BF00 |
| 0x44DED0 | InitializeMoviePlayer(_2) | find-with-callback (`CALL [EBP+0xc]` loop) | 0x7A17A0 | 0x445250 -> 0x798C30 |
| 0x44DF50 | GetNearestVideoMode | CExoArrayList<8 B>::SetSize | 0x838F70 | 0x43A510 -> 0x78DF40 |
| 0x44DFB0 | HandleExitGameButton | CExoArrayList<CSWRumblePattern> ctor | 0x7A18C0 | 0x4433C0 (label BuildSortedPowerList) -> 0x796DA0 (shape) |
| 0x44DFF0 | GetLoadScreenVisible | CExoArrayList<CSWRumblePattern> dtor | 0x7A1900 | 0x444B60 -> 0x798540 |
| 0x44E070 | AddMoveToModuleMovie(_2) | CExoArrayList<12 B>::SetSize | 0x8A6110 | 0x445190 -> 0x798B70 |
| 0x44E0D0 | SetGameOver | CExoArrayList<CGuiStringStruct> ctor | 0x7A19A0 | 0x448830 (FUN_) -> 0x79C230 |

Method that placed the names: GOG App::X forwards to the real Internal::X. App::GetLoadScreenVisible -> 0x444B60,
IsMoviePlaying -> 0x445220, AddMoveToModuleMovie -> 0x445190, SetGameOver -> 0x448830, IndicateObject -> 0x443170.
The GOG DB also puts the same Internal names on unrelated bodies. The Mac x86 symbol shapes confirm each one.

## AUTO rotation audit (part z, job 2)

**45 AUTO rows were checked** against the Mac symbol of the same name: 34 in the App slot-shift block (GOG 0x40C110-0x40D9B0)
and 11 Internal rows near 0x443170, 0x444B60, 0x4452F0 and 0x44DE70. The 0x43A510-0x43A920 window has no AUTO rows.
- **0 were flagged**: all fp 1:1, with disp/callees/strings equal to Mac.
- Mac Internal offsets above +0x3C0 are Windows - 4 (uniform layout drift).
- Together with part e's 8 meaning checks (4 overlap), 49 distinct AUTO rows of 167 had an Opus meaning read, and 0 were wrong.
- The rotations hit only REVIEW rows.

## Expected final verdicts after `finalize B11a --opus-sample-errors 1`

Computed by applying overrides a-e + r + z (the verdict and steam_addr of each) to `final_rows.tsv`:

| verdict | rows |
|---|---|
| verified | **408** |
| likely | 5 (App GetFPS, GetConnectionLib, GetPlayerList, ContinueMessageProcessing, GetApplicationId) |
| label_suspect | 16 |
| rejected | 0 |
| unresolved | 0 |
| **total** | **429** |

- Moves from pre-Opus:
  - likely -> verified 87;
  - unresolved -> verified 48;
  - rejected -> verified 14;
  - unresolved -> label_suspect 6;
  - unresolved -> likely 3;
  - verified -> likely 1;
  - verified -> label_suspect 1.
- Tiers of the shipped verified rows: Opus 252, AUTO 156 (167 AUTO minus 11 with an Opus override).
- `functions.csv` (verified only) should go from 406 rows (the current a-e+r state) to **408**: EndLoadBar 0x7400D0 and MovieWindowDeactivated 0x741250 are added.
- **Duplicate check:** among the verified, likely and label_suspect rows, no Steam address is held by two names.
- **Coverage check:** all 262 non-AUTO rows (REVIEW 245, REJECT 14, NOCAND 3) have an Opus override; 0 are uncovered.
- Overrides total 273 (265 + 8).

## Open items

1. Merge part z: `python3 tools/merge_overrides.py B11a` (265 -> 273 rows, tag `[z]`; expect no CONFLICT). Then run
   `finalize B11a --opus-sample-errors 1` and check the counts above.
2. Paste `gog_ledger_section.md` into `../GOG_LABEL_SUSPECTS.md` once the other editor is done. Then:
   - `persist B11a --apply`, which adds the `## Lessons` lines;
   - `import-ghidra --mode append`;
   - `SqliteTools validate`;
   - `check-validated`.
3. Semantic extras: these are not shipped, because the GOG rows carry the wrong names. Decide whether to ship them by meaning (B0 `semantic:` precedent):
   - Internal::GetNearestVideoMode 0x78DF40, GetLoadScreenVisible 0x798540, ShutDownMoviePlayer 0x798CD0;
   - IndicateObject 0x796B50, GetPausedByCombat 0x79BF00, SetGameOver 0x79C230;
   - HandleExitGameButton 0x796DA0 (shape only).
   - These names already ship correctly on their own GOG rows: IsMoviePlaying 0x798C00, AddMoveToModuleMovie 0x798B70,
     InitializeMoviePlayer 0x798C30, AddToPauseExclusionList 0x784680, DestroyQuestJournal 0x7850C0.
4. To locate: Internal::StallServerCode (Mac 97 B, +0x264) and StopMovie (Mac 5 B stub, possibly ICF-folded).
5. KPM / mod impact:
   - `CClientExoApp::EndLoadBar` (0x7400D0) forwards to an empty body: a hook sees the call but there is nothing to patch.
   - Any GOG-side call by the rotated Internal names above runs a template helper, not the named method.
