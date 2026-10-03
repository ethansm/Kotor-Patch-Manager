# B0 Opus gate, part b (74 rows, "Opus reads 100%")

Method: for every row I read the GOG and Steam `fp.tsv` rows, the Mac x86 row of the same symbol, all reviewer rows (real and decoy cards) and the Steam asm/decompile, and checked that every quoted asm line exists in the Steam dump (`decomp/<xx>/<addr>.asm`).

Result: the GOG and Steam fingerprints are **identical** (size, retn, n_insn, disp, imm, callee/string counts) for 73 of the 74 pipeline pairs. The one exception is 0x6fcf40, where the pipeline Steam address is wrong.

So no row in this part has a GOG/Steam layout difference. The rows tagged "match cites a layout/offset difference" are Mac-vs-Windows differences:
- CGameEffect `+0x30` reads as `+0x34` on Mac.
- Mac vtable slots are +4.
- Mac handler tables use 8-byte entries (0x1d8 -> 0x3b0, 0x118 -> 0x230, SWMG table offsets x2).

Every "reviewer corrected_addr" row in this part is a decoy card the reviewer correctly pointed back to the pipeline address. Reviewer decoy calls: all correct.

Verdicts: **verified 55, likely 4, label_suspect 15, rejected 0, unresolved 0**. One label_suspect row also corrects the Steam address: 0x6fcf40 -> 0x6c0aa0.

## Systematic findings (GOG label rotations)
1. **CSWSArea path / DFS block** (GOG 0x709770 .. 0x70ceb0). The labels are rotated relative to the Mac symbols, and the call graph confirms it:
   - GOG 0x70ccb0 is labelled `GridDFSTransTablePut`, but it is GridDFSTransTableInitialize:
     - Steam 0x552a90 has `00552aa5 PUSH 0x2000` and `00552b18 MOV [..+0x8],0x80000000`;
     - Mac Initialize has imm 0x80000000;
     - it is called by PlotGridPath.
   - GOG 0x70cd90 is labelled `PlotSoundPath`, but it is GridDFSTransTableGet (Steam 0x552b70).
   - GOG 0x70ceb0 is labelled `PathPointDFSTransTableInitialize`, but it is GridDFSTransTablePut (Steam 0x552c90).
   - Both of these are called only by GridDFSearch (Steam 0x551c30, 785 B), which matches Mac GridDFSearch -> Get/Put.
   - GOG 0x709770 is labelled `GridDFSTransTableInitialize`, but it is SmoothPointsOnPath (Steam 0x54f3d0):
     - it is called by PlotGridPath and PlotPathPointPath, as Mac SmoothPointsOnPath is;
     - it has the same disp set 0x1c, 0x8c-0x98, 0x1b0.
   - The Smooth* labels after it look shifted too. GOG 0x709980 `SmoothPointsOnPath` is likely SmoothPointsOnPathRestartable, and 0x709fd0 `SmoothSelectNodes` is likely SmoothSelection. These are outside this part and unverified.
2. **CSWSArea object iterator**: GOG 0x718400 and 0x718460 are a swapped First/Next pair. The Steam bodies show which is which:
   - 0x523250 resets `[+0x1a0]=0`, so it is First;
   - 0x5232b0 increments `[+0x1a0]`, so it is Next.
3. **CSWSArea stealth XP**:
   - GOG 0x7184d0 `SetMaxStealthXP` is IncrementPlayersInArea. Mac IncrementPlayersInArea: 45 B, disp 0x12c x3, calls IncreaseAILevelPriority.
   - The SetMaxStealthXP body is GOG 0x722ee0 (DB `GetClientArea_2`), Steam 0x52de00. Its disp 0x2e8,0x2ec,0x2ec equals Mac SetMaxStealthXP.
   - GOG 0x722f20 (FUN) fits SetCurrentStealthXP.
   - The real GetClientArea is GOG 0x722c70 (Mac disp 0x334).
4. **CSWSCreatureStats weapon block**:
   - GOG 0x6e5320 `GetWeaponFinesse` is GetUseMonkAttackTables. It matches the Mac shape exactly, and its GOG callees are named GetItemInSlot 0x67a5b0 and GetUseMonkAbilities 0x68f220.
   - GOG 0x6e5390 `GetWeaponSpecialization` is a monk-level/size tier function. It is probably what is labelled GetUseMonkAttackTables at 0x6e5460 that is the die-size sibling (NWN GetUnarmedDamageDice/Die).
   - The real GetWeaponFinesse and GetWeaponSpecialization still need to be located.
5. **CSWSCreatureStats talent/spell block**:
   - GOG 0x6fa1e0 `AddSpellLikeAbilityToList` is CheckInclusionMatch. Its callers are TalentRandomPowerFromItem and TalentRandomFeat, the same callers as Mac CheckInclusionMatch.
   - GOG 0x6f8bb0 `CheckInclusionMatch` is an effect-type -> immunity-index helper of GetEffectLinkImmunity, inlined on Mac. It has no Mac name.
   - GOG 0x6f5a80 `GetCreatureTalentRandomPower` is GetSpellsPerDayLeft. Mac GetSpellsPerDayLeft has the same shape and the same two callers.
   - The GOG ClassInfo accessor labels may be off too: GOG 0x6e0580 is labelled GetSpellGain but reads +0x19, which is Mac GetSpellsPerDayLeft's field. Not checked.
6. **SetNumActiveProperties**: the GOG label sits on 0x6fcf40.
   - That body is a CExoArrayList<ushort>::Contains in the template block (neighbours `long>::operator=`, `long>::Remove`), whose only caller is HasFeat.
   - The real SetNumActiveProperties is GOG 0x667970 (unnamed) = Steam 0x604140. The fingerprint is identical (133 B, 0x288/0x290, imm 0x1c, new/delete) and the Mac shape matches.
   - Steam anchors: `0060414f MOV [EAX+0x288],ECX` and `00604199 CALL 0x919723` (new count*0x1c).

## Proposed name overrides (proven by meaning: Mac symbol/callers + Steam anchor)
| name | Steam | GOG body | proof |
|---|---|---|---|
| CSWSItem::SetNumActiveProperties | 0x604140 | 0x667970 | Mac shape + `0060414f MOV [EAX+0x288],ECX` |
| CSWSCreatureStats::GetUseMonkAttackTables | 0x6a8ea0 | 0x6e5320 | Mac disp/imm/callees exact |
| CSWSCreatureStats::GetSpellsPerDayLeft | 0x6b9600 | 0x6f5a80 | Mac shape + same 2 callers |
| CSWSCreatureStats::CheckInclusionMatch | 0x6bdd60 | 0x6fa1e0 | Mac callers (3 talent fns) + nibble wildcard body |
| CSWSArea::SmoothPointsOnPath | 0x54f3d0 | 0x709770 | Mac disp set + callers PlotGridPath/PlotPathPointPath |
| CSWSArea::GridDFSTransTableInitialize | 0x552a90 | 0x70ccb0 | `00552aa5 PUSH 0x2000`, imm 0x80000000, caller PlotGridPath (RE guide 3.5) |
| CSWSArea::GridDFSTransTableGet | 0x552b70 | 0x70cd90 | caller GridDFSearch, lookup body |
| CSWSArea::GridDFSTransTablePut | 0x552c90 | 0x70ceb0 | caller GridDFSearch, store body |
| CSWSArea::GetFirstObjectInArea | 0x523250 | 0x718400 | `0052325c MOV [EAX+0x1a0],0` |
| CSWSArea::GetNextObjectInArea | 0x5232b0 | 0x718460 | `005232c8 MOV [EDX+0x1a0],ECX` after ADD 1 |
| CSWSArea::IncrementPlayersInArea | 0x523320 | 0x7184d0 | Mac 45 B, 0x12c x3, Increase callee |
| CSWSArea::SetMaxStealthXP | 0x52de00 | 0x722ee0 | Mac disp 0x2e8,0x2ec,0x2ec exact |
| FindGobIdx (global) | 0x458ae0 | 0x84b0b0 | Mac FindGobIdx: stricmp, +0x64, caller FindGob |

## Per-row evidence (short)
Notation: "fp=" means the GOG and Steam fp rows are identical. Asm lines are quoted from the Steam dump.

| # | GOG | Steam | name (GOG DB) | verdict | evidence |
|---|---|---|---|---|---|
| 1 | 0x679590 | 0x615d60 | CSWSItem::AddUpgrade | verified | fp=; `00615e6a CALL 0x6042a0` GetPassiveProperty (GOG callee 0x667ad0 same name), `00615e7c MOV [ECX+0x12],AX`; decoy RemoveUpgrade 349 B |
| 2 | 0x6e5320 | 0x6a8ea0 | GetWeaponFinesse | label_suspect | = GetUseMonkAttackTables (Mac 0x3fbeee disp 0x24,0x24,0x1150 imm 0x10). Body: monk abilities && no item in slot 0x10 |
| 3 | 0x6e5390 | 0x6a8f10 | GetWeaponSpecialization | label_suspect | Mac name = HasFeat via base item (disp 0xb0); body: monk-level tiers 4/8/12/16 and size `[+0x53c]<=2`, returns 1/2 |
| 4 | 0x6f4c80 | 0x6b8800 | GetNumberKnownSpells | verified | twin of GetKnownSpell resolved by callee: `006b8833 CALL 0x6a4190` (fp = GOG 0x6e0640 ClassInfo::GetNumberKnownSpells) |
| 5 | 0x6f5a80 | 0x6b9600 | GetCreatureTalentRandomPower | label_suspect | = GetSpellsPerDayLeft (ClassInfo +0x19; callers GuiKnownSpellUses/PlayerUpdate, as Mac); Mac TalentRandomPower is 161 B cdecl |
| 6 | 0x6f6e70 | 0x6ba9f0 | GetTotalSTRBonus | verified | `006baa0d PUSH 0x0` (STR) vs DEX twin PUSH 0x1; `006baa27 CALL 0x582010` |
| 7 | 0x6f88f0 | 0x6bc470 | ComputeNumberKnownSpellsLeft | label_suspect | skill -> feat 0xb7..0xbd, HasFeat; Mac version uses ClassInfo 0xab/0xac and CSWClass::GetSpellGain. Caller AutoLevelup. Name unknown |
| 8 | 0x6f8bb0 | 0x6bc730 | CheckInclusionMatch | label_suspect | jump-table effect -> immunity index (`006bc8d6 MOV [EBP-4],0x14`); caller GetEffectLinkImmunity only; Mac CheckInclusionMatch has other callers |
| 9 | 0x6fa1e0 | 0x6bdd60 | AddSpellLikeAbilityToList | label_suspect | = CheckInclusionMatch (nibble wildcard, `006bdd87 CMP [EBP-8],0xf000`); no list write; Mac AddSLAToList is 97 B, disp 0x134, caller ReadStatsFromGff |
| 10 | 0x6fcf40 | **0x6c0aa0** (pipeline 0x604140) | CSWSItem::SetNumActiveProperties | label_suspect (steam corrected) | GOG body fp = Steam 0x6c0aa0 and 0x64dc30; caller HasFeat (GOG 0x6f4870 / Steam 0x6b83f0) picks 0x6c0aa0; ushort Contains. 0x604140 is the real SetNumActiveProperties (GOG 0x667970) |
| 11 | 0x6fe490 | 0x5f8920 | CSWPartyTable::ClearInventory | verified | `005f893c CALL 0x606200` ClearItems, `005f8944 MOV [EDX+0x1a4],0` |
| 12 | 0x700490 | 0x5fa920 | RemoveNPC | verified | `005fa92e CALL 0x5fa960`, `005fa941 MOV [EDX+ECX*4+0x4c],0` |
| 13 | 0x700cb0 | 0x5fb140 | GetNPCSelectability | verified | `005fb15d MOV AL,[ECX+0x7c]` after GetIsNPCAvailable |
| 14 | 0x703650 | 0x5fdca0 | AddGameInProgress | verified | `005fdcd4 PUSH 0x993e84` "GAMEINPROGRESS:", `005fdcf2 CALL 0x711690` (Remove twin calls 0x711710) |
| 15 | 0x7048a0 | 0x5feef0 | SetSoloMode | verified | `005fef08 CALL 0x5fef20` UnstealthParty, `005fef13 MOV [EAX+0x238],ECX` |
| 16 | 0x709770 | 0x54f3d0 | GridDFSTransTableInitialize | label_suspect | = SmoothPointsOnPath (see finding 1); `0054f579 CALL 0x54ee60`, `0054f561 CALL 0x54fdb0` |
| 17 | 0x70cd90 | 0x552b70 | PlotSoundPath | label_suspect | = GridDFSTransTableGet; Mac PlotSoundPath is a 10 B stub |
| 18 | 0x70ceb0 | 0x552c90 | PathPointDFSTransTableInitialize | label_suspect | = GridDFSTransTablePut; Mac PathPoint Init uses 0x274 and imm 0x7dcccccc, which are absent here |
| 19 | 0x718400 | 0x523250 | GetNextObjectInArea | label_suspect | = GetFirstObjectInArea (swap) |
| 20 | 0x718460 | 0x5232b0 | GetFirstObjectInArea | label_suspect | = GetNextObjectInArea (swap) |
| 21 | 0x7184d0 | 0x523320 | SetMaxStealthXP | label_suspect | = IncrementPlayersInArea; Decrement twin 0x523360 calls 0x5235a0 |
| 22 | 0x722ee0 | 0x52de00 | GetClientArea_2 | label_suspect | = SetMaxStealthXP; twin 0x52de40 = SetCurrentStealthXP (Mac 28 B) |
| 23 | 0x7231f0 | 0x52e110 | SetTransitionPending | verified | `0052e11f MOV [EAX+0x2f8],ECX`, `0052e12b MOV [EDX+0x2fd],AL`, `0052e14a CALL 0x73fe40` |
| 24 | 0x734960 | 0x5e4ca0 | CGameEffect::Destructor | verified | `005e4cd2 CMP [EAX+0x28],0`, `005e4d7c CMP [ECX+0x34],0`, `005e4db8 CALL 0x919bcc` |
| 25 | 0x734a90 | 0x5e4dd0 | SetNumIntegers | verified | `005e4ddf MOV [EAX+0x30],ECX`, delete/new/memset |
| 26 | 0x734be0 | 0x5e4f20 | GetString | verified | `005e4f36 LEA EDX,[ECX+EAX*8+0x48]`, `005e4f3e CALL 0x7335f0` |
| 27 | 0x736310 | 0x593a30 | InitializeEffects | verified | `00593a39 PUSH 0x1d8`, `00593a7d CMP [EBP-4],0x76` |
| 28 | 0x737020 | 0x594740 | OnEffectApplied | verified | `00594761 CMP [EBP-4],0x76`, `0059476a MOV ECX,[EAX+0x4]` |
| 29 | 0x73c6d0 | 0x599de0 | OnApplySavingThrowIncrease | likely | asm-identical to OnApplyAttackIncrease (GOG 0x73d2d0 -> 0x59a9e0) apart from block order; positional |
| 30 | 0x73da20 | 0x59b130 | OnApplyDamageImmunityIncrease | verified | `0059b173 MOV EAX,[EDX+0xb4]`, `0059b191 CALL 0x5431f0` |
| 31 | 0x73dbd0 | 0x59b2e0 | OnApplyDamageImmunityDecrease | verified | `0059b384 PUSH 0x16`, `0059b377 MOV ECX,[EAX+0x1198]` |
| 32 | 0x73e160 | 0x59b870 | OnRemoveEntangled | verified | `0059b8a3 MOV EDX,[ECX+0x10e8]`, `0059b8b5 CALL 0x5d1510` |
| 33 | 0x73f0e0 | 0x59c7f0 | OnApplyEnemyAttackBonus | verified | `0059c858 MOV CL,[EAX+0x107]`, `0059c81d MOV EDX,[ECX+0xfc]` |
| 34 | 0x741150 | 0x59e860 | OnRemoveSetStateInternal | verified | `0059e904 CALL 0x6fd8d0`, `0059e8e1 LEA ECX,[EAX+EDX+0x270]` |
| 35 | 0x741260 | 0x59e970 | OnApplyMovementSpeedIncrease | verified | `0059e9a4 CMP EAX,0x64`, `0059e9b3 ADD EAX,0x64` |
| 36 | 0x741de0 | 0x59f4f0 | OnRemoveAreaOfEffect | verified | `0059f523 CALL 0x5e4ee0` (GetObjectID; neighbour of GetString, GOG 0x734ba0) |
| 37 | 0x7444e0 | 0x5a1c10 | OnRemoveSlowInternal | verified | `005a1c32 MOV [ECX+0xfd8],0` |
| 38 | 0x744f70 | 0x5a26a0 | OnRemoveModifyNumAttacks | verified | `005a26c3 MOV EDX,[ECX+0x10dc]`, `005a26ec SUB ESI,EAX` |
| 39 | 0x744fe0 | 0x5a2710 | OnApplyCurse | verified | `005a27ab PUSH 0x11`, `005a2887 PUSH 0x8d` |
| 40 | 0x745760 | 0x5a2e90 | OnRemoveSilence | verified | `005a2ebd MOV [ECX+0xfd0],0` |
| 41 | 0x748790 | 0x5a5ea0 | OnApplyMissChance | verified | `005a5eb1 CMP EAX,0x64` (1..100 check) |
| 42 | 0x748db0 | 0x5a64c0 | OnApplyDisappear | verified | `005a64fa CALL 0x57d790`, `005a6542 PUSH 0x7d0` |
| 43 | 0x74ade0 | 0x5a8530 | OnRemoveHorrified | likely | 36 B stub: vcall [+0x30], return 1 (`005a8551 RET 0x8`) |
| 44 | 0x74b090 | 0x5a87e0 | OnApplyBodyFuel | verified | `005a8802 MOV [ECX+0x11c4],1` |
| 45 | 0x74c480 | 0x5a9ba0 | OnApplyPureEvilPowers | verified | `005a9dd3 MOV CL,[EAX+0x122]`, `005a9d1f ADD EDX,0x32` |
| 46 | 0x761e60 | 0x622bd0 | CSWSTrigger::GetClientTrigger | verified | `00622bdc CALL 0x54a840`, `00622bf8 MOV EAX,[EDX+0x3c]` (Mac 0x40 = vtable +4) |
| 47 | 0x768410 | 0x61d000 | CSWSDoor::GetIsLinked | verified | `0061d00a MOVZX ECX,[EAX+0x3d4]`, `0061d020 CMP EAX,0x2` |
| 48 | 0x774c10 | 0x590560 | SetOffHandAttack | verified | `0059056d MOV [EAX+0xac0],ECX` |
| 49 | 0x77e710 | 0x669130 | CopyGameDefinedStructure | verified | `00669180 CALL 0x5e4920`, `006691ad CALL 0x5e4fd0` |
| 50 | 0x77ed70 | 0x669790 | CreateGameDefinedStructure | verified | `006697be PUSH 0x8c`, `00669812 PUSH 0x34` |
| 51 | 0x783260 | 0x66dc80 | ExecuteCommandCutsceneMove | verified | `0066dcbc CALL 0x6fda20`, `0066dd18 CALL 0x568480` |
| 52 | 0x78ebd0 | 0x6795f0 | ExecuteCommandFloatToInt | verified | `00679616 CALL 0x91c860` (ftol) between pop float and push int |
| 53 | 0x7c18a0 | 0x5aa490 | InitializeItemProperties | verified | `005aa499 PUSH 0x118`, `005aa4dd CMP [EBP-4],0x46` |
| 54 | 0x7c2600 | 0x5ab1f0 | RemoveEnhancementBonus | verified | `005ab21f PUSH 0x992e8c`, `005ab5c8 CALL 0x544790` |
| 55 | 0x7c3300 | 0x5abef0 | RemoveEnhancementPenalty | verified | `005abf97 MOV AX,[EDX+0x13c]`, `005ac3df RET 0x10` |
| 56 | 0x7c4dc0 | 0x5ad9b0 | ApplyLight | verified | `005ad9da PUSH 0x1d`, `005ada8e PUSH 0x1388` |
| 57 | 0x7c53c0 | 0x5adfb0 | ApplyACBonus | verified | `005ae0f3 MOV DL,[ECX+0x8b]`, `005ae135 MOV DL,[ECX+0x122]` |
| 58 | 0x7c5670 | 0x5ae260 | RemoveACBonus | verified | `005ae333 MOV CX,[EAX+0x13a]` |
| 59 | 0x7c5940 | 0x5ae530 | ApplyImprovedForceResistance | verified | `005ae5bd PUSH 0x8c`, `005ae667 CALL 0x544210` |
| 60 | 0x7c60c0 | 0x5aecb0 | RemoveChangedSavingThrow | verified | `005aeda1 MOV CL,[EAX+0x122]` |
| 61 | 0x7c6430 | 0x5af020 | RemoveSpecialWalk | verified | `005af043 MOV CX,[EAX+0x15c]`, `005af037 MOV EDX,[ECX+0x1198]` |
| 62 | 0x7c9840 | 0x5b2430 | ApplyBlasterBoltDeflectionIncrease | likely | identical to the Decrease twin except global `[0x996088]` (shared with RemoveIncrease 0x5b2560); positional |
| 63 | 0x7c9f20 | 0x5b2b10 | ApplySniperBonus | verified | `005b2b20 MOV [EAX+0x2e8],1` |
| 64 | 0x7c9f50 | 0x5b2b40 | ApplyRapidShotBonus | verified | `005b2b50 MOV [EAX+0x2e4],1` |
| 65 | 0x7e94f0 | 0x623030 | CSWSEncounter::Destructor | verified | `0062303a CALL 0x623060` (fp = GOG ~CSWSEncounter 0x7e9520, 590 B) |
| 66 | 0x7f07d0 | 0x5e6f40 | DestroyString | verified | `005e6f47 PUSH 0x3` |
| 67 | 0x7f9140 | 0x6f5b80 | InitializeSWMGCommands | verified | 103 slot stores, `006f5b9d MOV [EAX+0x91c],0x6f6320`; Mac x2 stride |
| 68 | 0x84b0b0 | 0x458ae0 | Gob::SetScene | label_suspect | = FindGobIdx (stricmp binary search on name +0x64; GOG FindGob 0x84b050 calls it) |
| 69 | 0x84c550 | 0x459f70 | DisableFog | verified | `00459f77 PUSH 0x0`, `00459f83 CALL 0x459f00` |
| 70 | 0x857f90 | 0x465bb0 | Scene::ChildAdd | verified | `00465be0 CALL 0x4659f0`; ChildRemove twin calls Remove 0x449170 |
| 71 | 0x8621f0 | 0x46ffb0 | Scene::SetCurrentRoom_2 | verified | `0046ffc3 CMP ECX,[EAX+0xd8]`, `0046ffdd MOV [ECX+0xe0],EAX`; Mac 50 B overload |
| 72 | 0x8677e0 | 0x475870 | ConsoleFunc::Constructor_2 | likely | 3 identical ctors; `004758c9 CALL 0x4765c0` (2nd sub-ctor) |
| 73 | 0x867920 | 0x4759b0 | ConsoleFunc::Destructor | verified | `004759c8 MOV EDX,[ECX+0x50]`, `004759bd CALL 0x474fa0` |
| 74 | 0x8fd240 | 0x4e3110 | Gob::AddAnimationFlag | verified | `004e3144 OR ECX,[EBP+0x8]`, `004e314a MOV [EDX+0x8],ECX` |

## Reviewer quality (this part)
- All quoted anchor lines exist in the Steam asm.
- Every decoy card was answered mismatch/label_suspect with the correct real address.
- The reviewer `label_suspect` calls were all confirmed.
- The `match` call on row 2 (GetWeaponFinesse, mac_contra) was wrong on meaning: the body is GetUseMonkAttackTables. This row is in the 100% class, not the sample. The reviewer's GetNumberKnownSpells/STR/twin reasoning was right.

sample_errors: 0  (part b contains no rows from "Sample of the other reviewer matches")

## AUTO meaning checks
Not in part b (the 8 AUTO rows belong to another part): no pass/fail table.
