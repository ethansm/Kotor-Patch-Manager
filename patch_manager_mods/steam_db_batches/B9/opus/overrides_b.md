# B9 Opus gate, part b (rows 61-120, all `sample`, 100% read after the decoy escalation)

Verdicts: verified 58, likely 2 (GetFPS 0x64b6e0, GetConnectionLib 0x64bfc0), label_suspect 0, corrected 0, rejected 0, unresolved 0.

## Method
- Bodies: Steam asm/decomp (`steam_6a522e71/decomp`), GOG + Steam server fp.tsv (size / n_insn / retn / imm / disp / callee count / str count).
  All 60 GOG/Steam fp tuples are identical except Internal::MainLoop (str_refs 5 vs 4).
- Callees and callers: Corpus(exclude_batch=B9) trusted `pairs`. For callers with no trusted row, fp-signature identity
  (size/n_insn/retn) of the Steam caller against the GOG caller. Twin check: the caller/callee signature overlap of the
  candidate vs Steam neighbours +-3 (`disc`). The candidate scored best or tied-best in every row, and every tie was settled by content (below).
- Delta cross-check (never used as the anchor): CServerExoApp = GOG - 0x12fa20 for all 39 rows. Internal = 0x283250 / 0x283340 / 0x283380 / 0x282da0
  (piecewise). CSWSModule = 0x1d2690 / 0x1d27b0.
- +-4 label sweep against the Mac x86 real symbols:
  - CServerExoApp: GOG order = Mac source order everywhere except two moved slots.
    - GetFPS: Mac puts it last.
    - ClientToServerObjectId: Mac places it between IsInjured and GetPauseState; GOG and Steam place it after GetGlobalVariableTable.
    - Both bodies fit their names: GetFPS = Mac 11B no-member constant; ClientToServerObjectId calls Internal::ClientToServerObjectId.
  - Callee-name check, GOG callee names vs the Mac callees of the same symbol: all forwarders agree. Internal/CSWSModule rows share the
    key callees (journal msgs, ResMan, GFF). **No label rotation in this part.**

## Twin / adjacent-field resolutions (the content that decided ties)
| pair | decided by |
|---|---|
| ClearLastModuleFound 0x52f570 vs Internal ClearAreaCache 0x537c90 (both 37B, imm 0x7f000000) | +0x1f324/+0x1f328 read by GetModule 0x52f4c0 (0052f4cf); +0x1f32c/+0x1f330 read by GetAreaByGameObjectID 0x537be0 (00537bec) |
| Get/SetLoadingModule (+0x100b4) vs Get/SetAutoSavePending (+0x100b8) | Internal LoadModule sets +0x100b4=1 (00531d35); trusted LoadPCAutoSave calls 0x51bf20 only; trusted client MainLoop calls 0x51bf80 |
| World/Timestop/Pause timers 0x51c370/0x51c390/0x51c3b0 (+0x10048/4c/50) | GetActiveTimer 0x538e00 returns +0x1004c when GetActivePauseState()==1; OnApplyTimestop 0x5a4d70 toggles pause type 1 -> +0x1004c = Timestop |
| SetMoveToPausedCalendarDay 0x51cf50 (+0x100b0) vs TimeOfDay 0x51cfa0 (+0x100ac) | LoadModuleStart 0x5582f0 fills the same local from GFF 'Mod_PauseDay' (0x994c10) or from Get 0x51cf80 (+0x100b0) |
| SetActionConversation 0x51cd10 (+0x1f31c) vs SetIgnoreConversationStartRange 0x51cd60 (+0x1f320) | 3 shared callers; trusted ExecuteCommandBeginConversation calls 0x51cd10 only (0066e0b4); SetGlobalDialogState (215B) calls 0x51cd60 only |
| DeleteWorldJournalEntry 0x55f1d0 vs ...Strref 0x55f2e0 (identical 263B fp) | msg callee 0x64bb30 vs 0x64bba0 (= GOG 0x7e2250 / 0x7e22c0, both +0x70); trusted cmd-table callers 0x670580 / 0x670670 |
| Internal Add 0x5393b0 vs Remove 0x5394c0 ExclusionList | Add 260B (= GOG) calls add-tail 0x54aa30; Remove 220B calls 0x64dde0; wrappers 0x51c790 / 0x51c7c0 |
| Internal Set/GetMoveToWorldTime 0x53cc80 / 0x53ccf0 | 0x53cc80 (103B) writes +0x1009c..+0x100a8, 0x53ccf0 (117B) reads them; ExoApp Set 0x51cff0 calls 0x53cc80 |

## Per-row evidence (short)
Rows 61-120 in part order. The anchor for each row is in its OUT note. Highlights:
- 61 RunModule: callee 0x5361d0 (464B = GOG Internal::RunModule); callers sig-match.
- 64 SetMoveToWorldTime: callee 0x53cc80 = Internal Set (writes); trusted LoadPCAutoSave 0085322e.
- 70 GetDebugMode: +0x1006c; callers sig-match ReportOverflow (615B) / CheatMessage (13697B).
- 74 GetModuleLanguage: Windows forwards to return_zero (0x4558a0, 13B = GOG Global::return_zero). Mac calls the Internal function. Both sides agree.
- 87 GetFPS: likely (no member access, constant 100.0f, 0 callers on both sides; positional + fp only).
- 90 GetConnectionLib: likely (+0x10058, 0 callers on both sides, no other reader found; unique slot between 0x51c560 and 0x51c5c0).
- 96 ClientToServerObjectId: 84 Steam / 85 GOG callers; trusted client MainLoop, GetServerArea, ClearAllActions call it.
- 102 ValidatePlayerLogin: sets [player+0x24]=1; callees match GOG Login_Confirm / Module_Info by fp.
- 115 GetSystemTimeAsDWORD64: GetLocalTime + SystemTimeToFileTime imports + __allshl (the Mac callees).

## Findings for other parts (not my rows; lead to re-gate)
- **CServerExoAppInternal::GetMoveToWorldTime gog 0x7bfa90** (part a, reviewer corrected to 0x53cc80): **0x53cc80 is the
  SetMoveToWorldTime body.** It is 103B / 28 insn = GOG 0x7bfa20 and writes +0x1009c..+0x100a8 (0053cc8d MOV [EAX+0x1009c],ECX). GetMoveToWorldTime
  is **0x53ccf0** (117B / 35 insn = GOG 0x7bfa90; 0053ccfd MOV EDX,[ECX+0x1009c] reads). Internal::SetMoveToWorldTime 0x7bfa20
  (pipeline rejected 0x53c5c0) should be **0x53cc80**.
- RemoveFromExclusionList: ExoApp 0x64c1e0 -> 0x51c7c0, Internal 0x7bc710 -> 0x5394c0 (pipeline had the Add addresses).
- Internal::GetPauseState 0x7bbf20 -> 0x538cd0 (36B, tests [+0x10078]&mask). The pipeline's 0x538c20 is TogglePauseState (my row 89).

## sample_errors: 0
All 60 reviewer `match` rows in this part are correct. In rows 86, 95 and 96 the reviewers correctly rejected the decoy cards.

AUTO meaning checks: none in this part (part b has only `sample` rows).
