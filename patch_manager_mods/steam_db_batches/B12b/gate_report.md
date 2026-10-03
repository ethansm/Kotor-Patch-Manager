# Gate report B12b

Rows: 201. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CSWSCreature | 149 | 32 | 17 | 3 | 201 |
| **all** | 149 | 32 | 17 | 3 | 201 |

AUTO by route: c2=97 sig=52 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

- CSWSCreature::CanEquipWeapon gog 0x006c7f90 -> 0x00405100: G4 steam retn 8 vs GOG retn 16 (no thunk explanation)
- CSWSCreature::CanEquipWeapon2 gog 0x006c81a0 -> 0x00670c90: G4 steam retn 8 vs GOG retn 16 (no thunk explanation)
- CSWSCreature::CheckProficiencies gog 0x006c8800 -> 0x00405970: G4 steam retn 4 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::AIActionPartyFollowLeader gog 0x006ca5b0 -> 0x00971dc0: G1 SEH unwind funclet, not a function
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_FindPath gog 0x006d2130 -> 0x0067ac20: G4 steam retn 8 vs GOG retn 16 (no thunk explanation)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Path gog 0x006d35a0 -> 0x00669020: G4 steam retn 4 vs GOG retn 16 (no thunk explanation)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Offset gog 0x006d4dc0 -> 0x005bbf40: G4 steam retn 4 vs GOG retn 16 (no thunk explanation)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath gog 0x006d5460 -> 0x0067df50: G4 steam retn 8 vs GOG retn 16 (no thunk explanation)
- CSWSCreature::AIActionWaitForEndOfRound gog 0x006db820 -> 0x00418990: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CSWSCreature::UpdateVisibleList gog 0x006dbb50 -> 0x005c3270: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CSWSCreature::AIActionRest gog 0x006dc3f0 -> 0x00983c00: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CSWSCreature::AIActionTaunt gog 0x006dc410 -> 0x00983c20: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CSWSCreature::AIActionAppear gog 0x006dc430 -> 0x00983c40: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CSWSCreature::AIActionDisappear gog 0x006dc500 -> 0x00983d10: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CSWSCreature::AIActionSurrenderToEnemies gog 0x006dd5d0 -> 0x00984de0: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CSWSCreature::AIActionStanceMeditative gog 0x006ddac0 -> 0x009852d0: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CSWSCreature::AIActionAnimalEmpathy gog 0x006dee60 -> 0x0041bfd0: G4 steam retn 0 vs GOG retn 8 (no thunk explanation)

## Duplicates and conflicts (G6)

- CSWSCreature::UpdateAppearanceForEquippedItems_2 gog 0x00698df0 -> 0x005802c0: review:held_by=CSWSCreature::GetDetectModeState
- CSWSCreature::BroadcastAttackOfOpportunity gog 0x0069df00 -> 0x00585420: review:held_by=CSWSCreature::GetIsBlindOrInDarkness
- CSWSCreature::CanEquipWeapon2 gog 0x006c81a0 -> 0x00670c90: fail:trusted_holder=Global::ExecuteCommandDistanceConversions
- CSWSCreature::CheckProficiencies gog 0x006c8800 -> 0x00405970: review:held_by=CSWCObject::ClientServerSyncHandler_Position
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_FindPath gog 0x006d2130 -> 0x0067ac20: fail:trusted_holder=Global::ExecuteCommandGetCommandable
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Path gog 0x006d35a0 -> 0x00669020: fail:trusted_holder=CSWVirtualMachineCommands::RunScriptCallback
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Offset gog 0x006d4dc0 -> 0x005bbf40: review:held_by=CSWSCreature::AIActionCheckMoveToObject
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath gog 0x006d5460 -> 0x0067df50: fail:trusted_holder=Global::ExecuteCommandGetGoldPieceValue
- CSWSCreature::AIActionWaitForEndOfRound gog 0x006db820 -> 0x00418990: fail:trusted_holder=CSWGuiControl::Constructor
- CSWSCreature::UpdateVisibleList gog 0x006dbb50 -> 0x005c3270: review:held_by=CSWSCreature::ComputeTotalWalkDistance
- CSWSCreature::AIActionRest gog 0x006dc3f0 -> 0x00983c00: review:held_by=CGuiInGame::HandleDialogEntry

## G4 metadata flags

- CSWSCreature::DecrementSpellReadyCount gog 0x0069a3d0: ['gog_db_param_bad'] (gog_param=5 gog_retn=20 steam_retn=20)
- CSWSCreature::AutoCloseGUIPanels gog 0x0069d9d0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSCreature::Disguise gog 0x0069dc00: ['gog_db_param_bad'] (gog_param=2 gog_retn=16 steam_retn=16)
- CSWSCreature::UnDisguise gog 0x0069ddf0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSCreature::OnEndDialog gog 0x006a2fe0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSCreature::CanEquipWeapon gog 0x006c7f90: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=8)
- CSWSCreature::CanEquipWeapon2 gog 0x006c81a0: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=8)
- CSWSCreature::CheckProficiencies gog 0x006c8800: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=4)
- CSWSCreature::AIActionPartyFollowLeader gog 0x006ca5b0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_FindPath gog 0x006d2130: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=8)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Path gog 0x006d35a0: ['gog_db_param_bad'] (gog_param=4 gog_retn=16 steam_retn=4)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Offset gog 0x006d4dc0: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=4)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath gog 0x006d5460: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=8)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest gog 0x006d5a90: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=16)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest gog 0x006d5bc0: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=16)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToOffsetWaitDest gog 0x006d5cf0: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=16)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToPathWaitDest gog 0x006d5e20: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=16)
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_TeleportForward gog 0x006d5f50: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSCreature::AIActionWaitForEndOfRound gog 0x006db820: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CSWSCreature::AIActionRest gog 0x006dc3f0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CSWSCreature::AIActionTaunt gog 0x006dc410: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CSWSCreature::AIActionAppear gog 0x006dc430: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CSWSCreature::AIActionDisappear gog 0x006dc500: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CSWSCreature::AIActionSurrenderToEnemies gog 0x006dd5d0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CSWSCreature::AIActionStanceMeditative gog 0x006ddac0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CSWSCreature::AIActionAnimalEmpathy gog 0x006dee60: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=0)

## Trusted pair body conflicts

Trusted rows that are NOT seeds (exact name + equal retn + equal size or A-SIG >= 0.5 with intersection >= 3 required, Opus gate R1). They stay trusted as G6 holders until Opus rules on them.

- GOG 0x00815520 CAurTexture::SetIsEnvironmentMapped -> Steam 0x00422e20: name is not exact (suffix on the trusted row)
- GOG 0x0084b0b0 Gob::SetScene -> Steam 0x00461aa0: retn GOG 0 vs Steam 4
- GOG 0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam 0x0051e170: name is not exact (suffix on the trusted row)
- GOG 0x0064e730 CServerAIMaster::EventPending -> Steam 0x0051ed10: name is not exact (suffix on the trusted row)
- GOG 0x00720d10 CSWSArea::ClearLineOfSight -> Steam 0x0052bb60: size GOG 176 vs Steam 767, SIG 0.35 (intersection 4)
- GOG 0x007184d0 CSWSArea::SetMaxStealthXP -> Steam 0x0052de00: retn GOG 0 vs Steam 4
- GOG 0x00709980 CSWSArea::SmoothPointsOnPath -> Steam 0x0054f3d0: size GOG 938 vs Steam 518, SIG 0.44 (intersection 7)
- GOG 0x00709d30 CSWSArea::SmoothPointsOnPathRestartable -> Steam 0x0054f5e0: retn GOG 16 vs Steam 0
- GOG 0x0070a060 CSWSArea::SmoothSelectionRestartable -> Steam 0x0054fb10: retn GOG 24 vs Steam 16
- GOG 0x00709fd0 CSWSArea::SmoothSelectNodes -> Steam 0x0054fe40: retn GOG 16 vs Steam 24
- GOG 0x00709770 CSWSArea::GridDFSTransTableInitialize -> Steam 0x00552a90: size GOG 518 vs Steam 173, SIG 0.08 (intersection 1)
- GOG 0x0070cd60 CSWSArea::GridDFSTransTableGet -> Steam 0x00552b70: retn GOG 8 vs Steam 16
- GOG 0x0070ccb0 CSWSArea::GridDFSTransTablePut -> Steam 0x00552c90: retn GOG 0 vs Steam 16
- GOG 0x00707770 CSWSArea::SetHitInfoForCreature -> Steam 0x00556040: retn GOG 32 vs Steam 20
- GOG 0x00733ef0 CSWSModule::non_virtual_thunk_to_dtor_CSWSModule -> Steam 0x00561c80: size GOG 8 vs Steam 44, SIG 0.00 (intersection 0)
- GOG 0x00667710 CSWSItem::RestoreUsedActiveProperties -> Steam 0x00603dd0: size GOG 63 vs Steam 245, SIG 0.17 (intersection 3)
- GOG 0x00667600 CSWSItem::GetPropertyByTypeExists -> Steam 0x00603f60: retn GOG 4 vs Steam 8
- GOG 0x006fcf40 CSWSItem::SetNumActiveProperties -> Steam 0x00604140: size GOG 85 vs Steam 133, SIG 0.33 (intersection 2)
- GOG 0x0067a210 CSWSItem::non_virtual_thunk_to_dtor_CSWSItem -> Steam 0x00616a60: size GOG 8 vs Steam 44, SIG 0.00 (intersection 0)
- GOG 0x007ef060 CFactionManager::SetNPCFactionReputation -> Steam 0x006644d0: name is not exact (suffix on the trusted row)
- GOG 0x007ef110 CFactionManager::GetNPCFactionReputation -> Steam 0x00664580: name is not exact (suffix on the trusted row)
- GOG 0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam 0x00664650: name is not exact (suffix on the trusted row)
- GOG 0x006e5460 CSWSCreatureStats::GetUseMonkAttackTables -> Steam 0x006a8ea0: retn GOG 0 vs Steam 4
- GOG 0x006e5320 CSWSCreatureStats::GetWeaponFinesse -> Steam 0x006b8da0: size GOG 110 vs Steam 217, SIG 0.06 (intersection 1)
- GOG 0x006e5390 CSWSCreatureStats::GetWeaponSpecialization -> Steam 0x006b8ee0: retn GOG 0 vs Steam 4
- GOG 0x006f5360 CSWSCreatureStats::UnReadySpell -> Steam 0x006b91b0: size GOG 83 vs Steam 1090, SIG 0.05 (intersection 1)
- GOG 0x006fa4c0 CSWSCreatureStats::GetIsInSpellLikeAbilityList -> Steam 0x006ba8f0: retn GOG 0 vs Steam 4
- GOG 0x006e41c0 CSWSCreatureStats::GetForceResistance -> Steam 0x006bad10: size GOG 101 vs Steam 75, SIG 0.00 (intersection 0)
- GOG 0x006f5a80 CSWSCreatureStats::GetCreatureTalentRandomPower -> Steam 0x006bae00: retn GOG 8 vs Steam 28
- GOG 0x006f5630 CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility -> Steam 0x006baec0: retn GOG 4 vs Steam 32
- GOG 0x006f8bb0 CSWSCreatureStats::CheckInclusionMatch -> Steam 0x006bdd60: size GOG 447 vs Steam 205, SIG 0.05 (intersection 3)
- GOG 0x007e4850 CSWSFaction::AddMember -> Steam 0x006d3330: name is not exact (suffix on the trusted row)
- GOG 0x00642c30 CVirtualMachineInternal::ExecuteCode -> Steam 0x006ff9f0: size GOG 1214 vs Steam 9336, SIG 0.46 (intersection 35)
- GOG 0x00645270 CVirtualMachineInternal::ReadScriptFile -> Steam 0x00701fd0: size GOG 64 vs Steam 716, SIG 0.11 (intersection 4)
- GOG 0x00645540 CVirtualMachineInternal::RunScript -> Steam 0x007022a0: size GOG 9 vs Steam 740, SIG 0.10 (intersection 1)
- GOG 0x00606e70 CExoString::CompareNoCase -> Steam 0x00734f60: name is not exact (suffix on the trusted row)
- GOG 0x00457310 CSWCArea::ClearAllMapPins -> Steam 0x007a96f0: size GOG 43 vs Steam 137, SIG 0.00 (intersection 0)
- GOG 0x00456350 CSWCArea::ClearTilePathNodes -> Steam 0x007aa910: size GOG 64 vs Steam 43, SIG 0.00 (intersection 0)
- GOG 0x004dbe50 CGuiInGame::IsFadingInDialog -> Steam 0x007cb720: size GOG 43 vs Steam 44, SIG 0.00 (intersection 0)
- GOG 0x004df1f0 CGuiInGame::GetCanOrientInDialog -> Steam 0x007cf710: size GOG 90 vs Steam 112, SIG 0.10 (intersection 1)
- GOG 0x004e0c60 CGuiInGame::GetMiniMapVisible -> Steam 0x007d05a0: size GOG 49 vs Steam 120, SIG 0.00 (intersection 0)
- GOG 0x00478360 CSWParty::AddCharacter -> Steam 0x007e5730: name is not exact (suffix on the trusted row)
- GOG 0x00478870 CSWParty::IsPartyMember -> Steam 0x007e5c40: name is not exact (suffix on the trusted row)
- GOG 0x00474890 CSWCObject::AnimationPause -> Steam 0x007ef350: size GOG 68 vs Steam 129, SIG 0.08 (intersection 1)
- GOG 0x004744b0 CSWCObject::AnimationPlayOutOfPlace -> Steam 0x007ef730: size GOG 129 vs Steam 68, SIG 0.08 (intersection 1)
- GOG 0x00597ff0 CSWGuiInGameCharacter::SetStats -> Steam 0x0084e6f0: name is not exact (suffix on the trusted row)
- GOG 0x005995d0 CSWGuiInGameCharacter::ShowLevelUpGUI -> Steam 0x0084fcd0: name is not exact (suffix on the trusted row)

## Ambiguous rows (second candidate also reaches C2)

(none)

## Lessons

<!-- one lesson per line: `- scope | topic | note`; persist inserts each once (dedupe on scope+topic) -->
- steam-db / method | The CSWSCreature run delta drifts inside one class: -0x118B30 is wrong past GOG ~0x6A5000 | B12b: the gate used the batch delta -0x118B30 (right for GOG 0x696F00-0x6A5000; SetInParty already -0x118AE0). From GOG 0x6C2820 the true delta is -0x10ADC0, from ~0x6DB820 -0x10AC20 (-0x10AC40 for Appear/Disappear), 0x6F5300 (Stats block) -0x3C480. All 17 REJECT and 3 NOCAND rows had an exact fp match at the local delta. Fit deltas piecewise per run.
- steam-db / method | A whole-binary fp-exact scan fixes REJECT/NOCAND rows that the run delta sent into CRT/GUI/SEH code | B12b part r: all 20 REJECT/NOCAND rows got a unique Steam fp match (size, retn, n_insn, disp, imm, callee count), confirmed in asm; gate picks were SEH funclets (0x971DC0), 0x98xxxx stubs and unrelated GUI code. Run the scan before leaving a REJECT row rejected.
- steam | Steam 0x5D1990 is one ICF-folded stub for AIActionAnimalEmpathy, AIActionTaunt and AIActionPickPocket | B12b: RunActions (Steam 0x5D56B0, switch 005d5c28 JMP [EDX*4+0x5d7038]) sends cases 0x29, 0x2B and 0x36 to the same 18 B body 0x5D1990 (005d1997 MOV EAX,[0x9944bc]). A hook there fires for all three actions. RunActions case numbers follow the NWN action enum and pair same-fp stubs (Rest 0x2A vs Taunt 0x2B).
- steam-db / method | GOG CSWSCreature AIAction labels are rotated in 0x6C6230-0x6C7820; bodies follow the Mac order | B12b: Mac order ChangeFacingObject, ChangeFacingPoint, MoveAwayFromObject, MoveAwayFromLocation, InterAreaPathfinding, MoveToObject(+Radius), MoveToPoint(+Radius) matches the GOG bodies 1:1, but labels sit one or more slots off (0x6C6230/6360/66E0/6930/7820) and MoveAwayFromObject labels 0x6C9D80 (AreaWait). Decide names by callees/imm vs Mac, never by GOG label.
- steam-db / process | The sig/c2 AUTO routes and the reviewer cannot see GOG label rotations | B12b: fp 1:1 + run position made every rotated row look right. The reviewer said match on 5 rotated rows and on the AddChemical/AddComponent swap (it noted the swap but ruled match). The AUTO audit of the 2 rotated windows flagged 2 of 14 AUTO rows (0x6C6360, 0x6DDEE0). When Opus finds a rotation, audit every AUTO neighbour in that window against Mac.
- steam-db / method | GOG CanUseItem restriction checks: Class sits on RemoveFromInvitationsOffered, Gender on Class | B12b: GOG 0x6DDEE0 (HasFeat 0x5D, +0x8D/+0xAB, property 0x2C) = Mac CheckItemClassRestrictions; GOG 0x6DE780 (property 0x3C, vcall +0x88, no HasFeat) = Mac CheckItemGenderRestrictions. Use the CanUseItem callee set (Mac 7 checks incl. Gender) to name the block, not the GOG labels.
