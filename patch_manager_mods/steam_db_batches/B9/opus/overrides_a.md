# B9 Opus gate, part a (60 rows: 23 at 100%, 37 sample)

OUT = `B9/opus/overrides_a.tsv` (60 rows): 57 verified, 2 likely, 1 label_suspect, 0 rejected, 0 unresolved.
Of these, 9 are Opus corrections to the pipeline Steam address. Each one equals the reviewer `corrected_addr`.

## Method

- **Bodies and callers.** For every row I compared the GOG fp row with the Steam fp row of the final address. All 60
  pairs are equal on `size retn n_insn disp imm`. I also compared the multisets of callee sizes and caller sizes.
  - Callee sizes: equal except an alloc helper that is 101 B on GOG and 100 B on Steam (0x921347 vs 0x919723).
  - Caller sizes: equal except one caller each for GetObjectArray (515/516), GetSWSMessage (309/312) and MainLoop
    (3581/3611). These are small Steam-side edits; the caller counts are equal (261/261, 118/118, 2/2).
- **Asm.** I read or grepped the Steam asm for every final address and quote one line in each note. I spot-checked
  every reviewer-quoted line I reused; all of them exist and mean what the reviewer said.
- **CServerExoApp layout.** GOG 0x64b640..0x64cb00 maps to Steam at exactly -0x12fa20 for all 120 functions, with equal
  sizes and RETs. I used this only as a cross-check. Each forwarder is anchored by its Internal target, and each target
  is itself fp-equal to the GOG Internal function with the same label.
  (Range 0x64b640..0x64cb00: all 120 GOG functions have a Steam function at -0x12fa20 with the same size and RET.)
- **Field offsets.** GOG fp drops large displacements: for example the 0x10010 in GetSWSMessage is not listed in GOG
  `disp`, and the Steam fp drops it the same way. So for pure getters and setters the field offset is quoted on the
  Steam side only. GOG shape equality and caller identity carry the rest.
- **Grading.** The two getters/stubs with 0 callers and no callee are graded `likely`: GetNetLayer and
  non_virtual_thunk_to_AsSWSModule.

## Escalated decoy B9-3-29 (SetIgnoreConversationStartRange): settled, 0x0051cd60, decoy key is correct

- **GOG 0x64c780 has 4 callers:** 0x4df280 `CGuiInGame::SetGlobalDialogState`, plus 0x687b00, 0x762ff0 and 0x76d3b0.
  The last three are the large door/placeable/area event handlers.
  - Those three handlers also call SetActionConversation 0x64c730, so they cannot tell the two setters apart.
  - That is why c2 and c3 disagreed: c3 relied on the Door/Placeable EventHandler callers, which call both setters.
- **The discriminating callers:**
  - GOG 0x4df280 (215 B) calls only SetIgnoreConversationStartRange. Its Steam twin 0x7ceb80 (215 B, identical disp
    list 0x4,0x4,0x4,0x8,0xcc,0xd8,0xf78,0xf78) calls **0x51cd60** and not 0x51cd10.
  - GOG 0x783550 (2159 B) calls SetActionConversation 0x64c730 and GetActionConversation 0x64c760. Its Steam twin
    0x66df70 (2159 B) calls **0x51cd10** and 0x51cd40.
  - Getter 0x51cd40 reads `[ECX+0x1f31c]` (0051cd4f), the same field 0x51cd10 writes (0051cd28). So 0x51cd10 =
    SetActionConversation and 0x51cd60 (`0051cd78 MOV [EDX+0x1f320],EAX`) = SetIgnoreConversationStartRange.
- **Order check.** The delta and Mac order agree (Mac: SetActionConversation, GetActionConversation,
  SetIgnoreConversationStartRange).
- **Decision.** The gate pick 0x0051cd60 is verified. The decoy 0x0051cd10 is not the counterpart, so this is a real
  reviewer decoy accept (c3), **not** a key error.

## Corrections (pipeline address -> final)

| # | GOG | Name | pipeline | final | what the pipeline address really is |
|---|---|---|---|---|---|
| 1 | 0x64b640 | CServerExoApp::Constructor | 0x51bcc0 | 0x51bc20 | GetFPS (17 B) |
| 8 | 0x64c140 | CServerExoApp::GetPauseState | 0x51c6f0 | 0x51c720 | CServerExoApp::IsInjured |
| 9 | 0x64c1e0 | CServerExoApp::RemoveFromExclusionList | 0x51c790 | 0x51c7c0 | AddToExclusionList |
| 10 | 0x64c500 | CServerExoApp::UpdateSubareasOnMoveTo | 0x51ca40 | 0x51cae0 | ComputeHeight |
| 12 | 0x64c940 | CServerExoApp::ShrinkLineByCreatures | 0x51cc00 | 0x51cf20 | GetRoomIndex |
| 16 | 0x733ef0 | CSWSModule::non_virtual_thunk_to_dtor_CSWSModule | 0x5615c0 | 0x561c80 | 64 B helper (5 callers) |
| 20 | 0x7bbf20 | CServerExoAppInternal::GetPauseState | 0x538c20 | 0x538cd0 | Internal::TogglePauseState |
| 21 | 0x7bc710 | CServerExoAppInternal::RemoveFromExclusionList | 0x5393b0 | 0x5394c0 | Internal::AddToExclusionList |
| 23 | 0x7bfa90 | CServerExoAppInternal::GetMoveToWorldTime | 0x53cc80 | 0x53ccf0 | Internal::SetMoveToWorldTime |

Every wrong pipeline pick was a near neighbour in a run of same-shape forwarders. Each correction is anchored by its
forward target, which is fp-equal to the GOG callee of the same GOG forwarder.

## Per-row evidence (short)

100% rows:
1. Constructor -> 0x51bc20: `0051bc5a PUSH 0x1f350` (= GOG imm), `0051bc7a CALL 0x52ebe0` (Internal ctor), 147 B = GOG.
2. GetObjectArray -> 0x51c080: `0051c08f MOV EDX,[ECX+0x1005c]`; 261 = 261 callers.
3. GetNetLayer -> 0x51c2c0 (likely): `0051c2cf [ECX+0x10040]`; 0 callers both sides; position confirmed by Mac order.
4. GetSWSMessage -> 0x51c2e0: `0051c2ef [ECX+0x10010]`; called from Internal::InitiateModuleForPlayer on both sides.
5. Initialize -> 0x51c450: `0051c45d CALL 0x5315a0` (49 B = GOG 0x7b4920; `PUSH 0x993f00`, then CALL 0x710840).
6. ContinueMessageProcessing -> 0x51c4c0: `0051c4cd CALL 0x537830` (47 B, disp 0x14 = GOG 0x7baa80).
7. InitiateModuleForPlayer -> 0x51c650: CALL 0x5356d0 (819 B, disp list identical to GOG 0x7b8a10).
8. GetPauseState -> 0x51c720 (corr): `0051c732 CALL 0x538cd0`.
9. RemoveFromExclusionList -> 0x51c7c0 (corr): `0051c7d6 CALL 0x5394c0` (220 B).
10. UpdateSubareasOnMoveTo -> 0x51cae0 (corr): `0051caf9 CALL 0x539e30` (245 B; `00539f00 CALL 0x5c25e0`).
11. CreatureAcquireItem -> 0x51cc60: CALL 0x539f80 (306 B, `00539fff PUSH 0x30c`).
12. ShrinkLineByCreatures -> 0x51cf20 (corr): `0051cf39 CALL 0x53aea0` (47 B = GOG 0x7be0f0).
13. GetMoveToPausedCalendarDay -> 0x51cf80: `0051cf8f [ECX+0x100b0]`.
    - In Internal::MainLoop, `00533b11/00533b15` pass locals -0x28 and -0x34 to GetWorldTime. Its 1st out (-0x34,
      calendar day) goes to setter 0x51cf50 at 00533bfa; its 2nd out (-0x28, time of day) goes to 0x51cfa0 at 00533bdd.
    - So the CalendarDay / TimeOfDay names are not swapped.
14. GetMoveToPausedTimeOfDay -> 0x51cfd0: `0051cfdf [ECX+0x100ac]`, same chain as row 13.
15. GetModulePreviouslyVisited -> 0x51d070: CALL 0x53cf10 (1367 B = GOG 0x7bfcb0).
16. non_virtual_thunk_to_dtor_CSWSModule -> 0x561c80 (corr): `00561c80 SUB ECX,0x1c` / `00561c83 JMP 0x556cf0`, 8 B.
17. Internal::InitiateModuleForPlayer -> 0x5356d0: `00535704 PUSH 0x1220`, `0053572b CALL 0x561f30`.
18. Internal::PlayerListChange -> 0x535c30: `00536031 [EDX+0x100f8]`, `0053614b ADD ECX,0x1001c`.
    - GOG DB param 8 is wrong: RET 0xc on both, so the right value is 12 (`gog_db_param_bad`).
19. Internal::IsInjured -> 0x538ca0: `00538cb0 CALL 0x537b70`, `00538cc1 CALL 0x58bc50`.
20. Internal::GetPauseState -> 0x538cd0 (corr): `00538cda MOVZX ECX,[EAX+0x10078]`; 4 = 4 callers.
21. Internal::RemoveFromExclusionList -> 0x5394c0 (corr): 220 B, `0053951a CALL 0x64dde0`.
22. Internal::Uninitialize -> 0x53a7c0: **label_suspect** (see below).
23. Internal::GetMoveToWorldTime -> 0x53ccf0 (corr): 117 B, `0053ccfd [ECX+0x1009c]`, `0053cd62 RET 0x1c`.

Sample rows (24-60): all reviewer `match` picks equal my finals, with fp equality and the anchors in the TSV notes.
- Row 59 (non_virtual_thunk_to_AsSWSModule, 17 B stub, `00556cda SUB EAX,0x1c`, 0 callers) is graded likely.
- Row 53 is the escalated decoy row (above).

## Label sweep (+-4, GOG name vs Mac x86 real symbols)

- **CServerExoApp.** I aligned the GOG CServerExoApp sequence 0x64b640..0x64cae0 with the Mac sequence (difflib).
  - Every row in this part sits in an `equal` block: no label rotations.
  - Every GOG forwarder calls the CServerExoAppInternal function with the same label, as on Mac.
  - The only differences are outside this part:
    - GetFPS is placed differently (first on GOG, last on Mac).
    - Mac-only Uninitialize, GetApplicationId, GetSystemTimeAsDWORD64, ShutDownToMainMenu and SetApplicationIdsMatch
      have no GOG body in that range.
    - ClientToServerObjectId sits at a different position.
    - **GOG 0x64c2f0 `CTwoDimArrays::GetSwoopUpgradeRowProvider`** (22 B, imm 0x1f0b4, 175 callers) sits in the slot of
      Mac `CServerExoApp::GetPartyTable` (16 B, next to GetGlobalVariableTable 0x100fc). It is a probable GOG label
      suspect, but not in this part.
- **CServerExoAppInternal.** GOG order differs slightly from Mac (compiler), but every row's body fits its Mac
  namesake. Examples: IsInjured = GetCreature + CSWSCreature::IsInjured; UpDatePlayerPosition = GetCreature +
  SetPosition + SetOrientation; AddCharListRequest = AddUnique; GetPauseState is leaf; GetMoveToWorldTime is leaf.
- **Row 22 `CServerExoAppInternal::Uninitialize` (GOG 0x7bda10): label_suspect.** The pair is right: 32 B = 32 B, and
  the sole callee is CClientExoApp::RunDeathSequence (GOG 0x40d860 22 B / Steam 0x7403a0 22 B). The sole caller is
  TempDeathUpdate (GOG 0x7bd310 / Steam 0x53a0c0, 1773 B each).
  - Mac `CServerExoAppInternal::Uninitialize` (0x399cf4) is an empty 5 B stub.
  - Mac TempDeathUpdate calls RunDeathSequence directly, so this helper is inlined on Mac and has no Mac symbol.
  - The GOG name does not fit the body. It is an unnamed TempDeathUpdate -> RunDeathSequence helper. The real GOG
    Uninitialize is most likely an empty body folded away (ICF).
  - This is not a rotation: the GOG name does not belong at another GOG address that I could find.
- **CSWSModule.** Rows 16, 24, 34, 43 and 59 fit their bodies and the Mac shapes:
  - AddObjectToLimbo -> Add; RemoveObjectFromLimbo -> DelIndex. Both use the 0x1fc/0x200 limbo array, and
    IsObjectInLimbo follows.
  - The dtor thunk is this-0x1c -> deleting dtor; the AsSWSModule thunk is this-0x1c, Mac imm 0xffffffe4.

## AUTO meaning checks

None in this part (part a holds only 100% and sample rows).

sample_errors: 0
