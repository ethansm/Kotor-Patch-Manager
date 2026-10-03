# Opus gate B16 part a: evidence

Method: GOG/Steam/Mac fp.tsv rows (size, retn, n_insn, disp, imm, callees, str_refs), whole-binary exact-fp search on
Steam, callee shape mapping (each GOG callee matched to a Steam callee of equal size/retn/n_insn), Steam asm quoted from
`steam_6a522e71/decomp`, Steam vftables read from `swkotor2_vanilla_6a522e71.exe` (.rdata). "fp 1:1 unique" = size,
retn, n_insn, disp and imm identical to GOG, and the only such function in the Steam binary.

## 100% rows (22)

1. **0x595eb0 Character ctor -> 0x84c3a0 verified (correction).** Pick 0x73b4d0 is a 149b list accessor. 0x84c3a0 fp 1:1
   unique (6331/1436 insn), 17/18 callees mapped; `0084c3ea MOV [ECX],0x9a3e7c` (Steam symbol: CSWGuiInGameCharacter
   vftable), `0084da65 MOV [EDX+0x4b6c],EAX` (GOG disp 0x4b6c), `0084dc58 RET 4`.
2. **0x597770 Character dtor -> 0x84def0 verified (correction).** Pick 0x84de70 stores CSWGui3DSceneView vftable. 0x84def0
   is slot 0 of the Character vftable 0x9a3e7c; `0084defa CALL 0x84df70` (~CSWGuiInGameCharacter), `0084df0b CALL 0x9196fd`.
3. **0x5994b0 Draw -> 0x84fbb0 verified.** fp 1:1 unique; `0084fbc4 MOV ECX,[EAX+0x3ad8]`, `0084fbd4 ADD ECX,0x3a60`;
   slot 13 of 0x9a3e7c (the Draw slot: CSWGuiPanel::Draw 0x40ec90 sits there in Gameplay's vftable). Mac contra is only
   Mac offsets (Mac 0x3a24); shape (panel Draw + SetCurValue) agrees.
4. **0x5e43c0 Gameplay ctor -> 0x8df930 verified.** fp 1:1 unique, disp set identical (no layout shift); 20/20 callees;
   `008dfebf PUSH 0x9a1ecc` (LBL_MOUSESEN), `008df97a MOV [ECX],0x9a8ee4` (CSWGuiInGameGameplay vftable).
5. **0x5e5a70 OnKeyMap -> 0x8e0fc0 verified.** fp 1:1 unique; `008e0ff7 PUSH 0x1370`, callee 0x8fff20 = KeyMapping ctor,
   `008e105c CALL 0x410530` (AddPanel). Sibling OnMouse differs only in 0x147c, so the immediate settles it.
6. **0x5e5b30 OnMouse -> 0x8e1080 verified.** `008e10b7 PUSH 0x147c`, callee 0x7dc410 = CSWGuiOptionsMouse ctor.
7. **0x5e6020 OnEnterDiff -> 0x8e1570 verified.** fp 1:1 unique; `008e15b6 CALL 0x8e1280` (SetDifficultyText),
   `008e15bb PUSH 0xa519`, SetDescription 0x8e1410. Mac OnEnterDiff also has 3 callees; contra = Mac offsets.
8. **0x5e6080 OnExitDiff -> 0x8e15d0 verified.** fp 1:1 unique (70b); `008e15e1 ADD ECX,0xea0`, `008e1605 AND EAX,0xffffffbf`,
   sole callee SetColor 0x417010 (Mac OnExitDiff: 1 callee).
9. **0x64dc40 AddEventAbsoluteTime -> 0x51e220 verified.** fp 1:1 unique, disp identical (the "layout" flag is spurious);
   `0051e24a PUSH 0x18`, `0051e2eb CALL 0x51afe0` (CompareWorldTimes).
10. **0x651470 FindIdentifierIndex -> 0x6545b0 verified (correction).** Pick 0x6544f0 = AddIdentifierLocation (row 39).
    `006545f6 MOV ECX,0x93d`, `00654626 CALL 0x734f60` (CompareNoCase), Hash 0x654210. Mac same shape (0x93d, 0x18).
11. **0x651600 GetValueBoolean -> 0x654740 verified (correction).** Pick 0x654670 is GetBit (78b/ret8 = GOG GetBit
    0x651530). `0065479e PUSH 0x9984f8` "Script var BOOLEAN '%s' not in catalogue!", `0065483d CALL 0x654670` (GetBit).
12. **0x651720 GetValueNumber -> 0x654860 verified (correction).** `00654951 MOV DL,[ECX+EDX+0xde29]`, RET 8, 5/5 callees.
13. **0x651a70 SetValueBoolean -> 0x654bb0 verified (correction).** Pick 0x654aa0 is the STRING getter (+0xeb74).
    `00654c9f ADD EAX,0xddb8`, `00654ca8 CALL 0x6546c0` (SetBit).
14. **0x652550 ReadTableWithoutCatalogue -> 0x655690 verified.** size/retn/n_insn/disp/imm identical; GOG has one extra
    str ref 0x9858f5 (empty string) that Steam lacks, so not "exact". `00655736 PUSH 0x998654` "ValBoolean", no Cat*
    strings (0x655b00 has them); ReadTable `006565e8 CALL 0x655690`.
15. **0x653500 Save -> 0x656640 verified (correction).** Pick 0x6564c0 is ReadTable (row 40). `00656668 PUSH 0x99874c`
    "GLOBALVARS", `006566e7 CALL 0x655010` (WriteTable).
16. **0x6535d0 Load -> 0x656710 verified (correction).** Pick 0x656780 is 31b. `00656738 PUSH 0x99874c` "GLOBALVARS",
    `00656753 CALL 0x6564c0` (ReadTable).
17. **0x77e650 AppendToScriptsRun -> 0x669070 verified.** fp 1:1 unique; `006690b7 ADD ECX,0x60`; CALL 0x733df0
    (CExoString operator+) x2; Mac shape agrees (imm 0x2c/0x60, CExoString ops).
18. **0x815aa0 GetController -> 0x4233a0 likely.** 17b getter, fp 1:1 unique; `004233aa MOV EAX,[EAX+0x34]`. Getter, so
    likely by rule.
19. **0x815b80 CAurTextureBasic dtor -> 0x423480 verified (correction).** Pick 0x4233a0 = GetController (row 18).
    `00423645 CALL 0x425160` (FreeImage), `0042365c CALL 0x423930` (RemoveFromOrderedLists); 9/9 callees.
20. **0x816030 RemoveFromOrderedLists -> 0x423930 verified (correction).** `00423940 CALL 0x476620`,
    `0042395a MOVZX EAX,byte [EDX+0xd7]`.
21. **0x920c10 StopMovie -> 0x40da20 verified (correction).** `0040da2a CALL 0x40cb20` (ShutDown), `0040da32 MOV [EAX+0x30],0`.
    Pick 0x40dac0 (390b) is Internal::ShowFadePanel: Steam wrapper 0x93b5c0 calls it; it matches GOG 0x921210
    CExoMoviePlayer::ShowFadePanel -> 0x920cb0. **GOG label error:** 0x920cb0 is labelled
    `CSWGuiSaveLoad::HandleCloudButton2` but is CExoMoviePlayerInternal::ShowFadePanel.
22. **0x921000 PauseMovie -> 0x40de10 verified (correction).** GOG fps of Pause/Resume are identical (imm 0/1 not in fp), so
    caller topology decides. `0040de2c MOV [EDX+0x64],1`, `0040de41 CALL [0x9864cc]` (BinkPause@8, arg +0x64=1). Its
    caller is 0x40c4a0 (693b = GOG ExoMoviePlayerWindowProc 0x91f690, which calls 0x921000). Resume is called by
    RunMovieFrameLoop on both sides. Address order agrees.

## Sample rows (12): reviewer match rows re-derived

| # | row | Steam | result | anchor |
|---|---|---|---|---|
| 23 | SetBaseFont | 0x416de0 | correct | `00416dea ADD EAX,0x3c`; `00416df9 CALL 0x416d60` |
| 24 | ResumeMovie | 0x40de50 | correct | `0040de6c MOV [EDX+0x64],0`; caller `0040d74a` in 0x40d6f0 = RunMovieFrameLoop |
| 25 | ReSetFont | 0x416e10 | correct | `00416e1a CALL 0x416d60`; SetFontRename callers 36/19/198b 1:1 both sides |
| 26 | SetDefaultColor_2 | 0x417180 | correct | `0041718a ADD EAX,0x2c`; `00417191 CALL 0x417010` |
| 27 | SetLabelBonusPenaltyColour | 0x850090 | correct | `008500a5 ADD ECX,0xf0` x3; RET 8 |
| 28 | Letterbox scalar dtor | 0x8baaa0 | correct | `008baae5 CALL 0x40e860` (~CSWGuiPanel) |
| 29 | Gameplay dtor | 0x8e0780 | correct | slot 0 of 0x9a8ee4; `008e078c CALL 0x8e07b0` |
| 30 | OnEnterSlot | 0x8e0f40 | correct | `008e0f4c MOV ECX,[EAX+0x5c]`; `008e0f59 CALL 0x8e1410` |
| 31 | SaveEventQueue | 0x51ee20 | correct | `0051ee43 CALL 0x719740`; `0051ee7d PUSH 0xabcd` |
| 32 | SetModText | 0x84f770 | correct | `0084f856 CALL 0x416e30` (SetText) |
| 33 | AdjustTargetAndWitnessReputations | 0x51f0a0 | correct | `0051f0b1 SUB ESP,0x118`; `0051f907 RET 0xc` |
| 34 | ClearValues | 0x653dd0 | correct | `00653dde ADD EAX,0xddb8`; `00653e25 ADD EAX,0xeb74` |

## AUTO meaning checks

| # | route | row | Steam | pass/fail | anchor |
|---|---|---|---|---|---|
| 35 | sig | OnEffectRemoved | 0x51f000 | pass | `0051f012 MOV EAX,[EDX+0x58]`; `0051f025 RET 8` |
| 36 | sig | SetTextObject | 0x417070 | pass | `004170ce CALL 0x47ec30`; `00417109 CALL 0x416fa0` |
| 37 | sig | OnEffectApplied | 0x51efd0 | pass | `0051efe6 MOV ECX,[EAX+0x58]`; `0051effb RET 0xc` |
| 38 | sig | glImage | 0x4265e0 | pass | `00426771 CALL 0x432fa0`; `004267d6 CALL 0x433cd0` |
| 39 | sig | AddIdentifierLocation | 0x6544f0 | pass | `00654503 CMP [EAX+0xeba4],0x64`; `0065451f CALL 0x6542d0` |
| 40 | c2 | ReadTable | 0x6564c0 | pass | `0065655e PUSH 0xbe76`; `006565d6 CALL 0x655b00`; `006565e8 CALL 0x655690` |
| 41 | c2 | AddObject | 0x51e040 | pass | `0051e075 CALL 0x51e0c0`; `0051e094 CALL 0x51d150` |
| 42 | c2 | RemoveObject | 0x51e0c0 | pass | `0051e0fc CALL 0x51d170`; `0051e10d MOV [ECX+0x78],-1` |

## Lead notes checked (outside this part)

- CSWGlobalVariableTable: GetBit 0x654670, GetValueBoolean 0x654740, GetValueNumber 0x654860, SetValueBoolean 0x654bb0,
  0x654aa0 = string getter, ReadTableWithoutCatalogue 0x655690: **all confirmed** (rows 11-14 above).
- Pause/Resume: confirmed, Pause 0x40de10, Resume 0x40de50 (rows 22/24).
- RotateCharacterLeft/Right (not in this part): Steam 0x850330 `00850355 FLD [0x98bd88]` = -10.0f; 0x850550
  `00850575 FLD [0x987704]` = +10.0f. Mac Left imm 0xc1200000 (-10.0), Right 0x41200000 (+10.0). I could not read the
  GOG constant: there is no GOG binary or asm on disk, and GOG fp has no float values. So by the lead's rule
  **unresolved for both**. The current evidence rows are also wrong on their face: Left -> 0x8500f0 is RotateCharacter
  (566b, = GOG 0x5999f0), and Right -> 0x850330 is the -10.0 body. To re-gate: GOG float at the FLD in 0x599c30 / 0x599e50.
- GUI layout: no shifted offset in any row here. Every "layout/offset difference" row has an identical disp set.

sample_errors: 0
