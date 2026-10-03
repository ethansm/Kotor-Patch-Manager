# G8 mac_rotation study

## Calibration gold (positives AUTO with / without G8)

| split | variant | AUTO without G8 | AUTO with G8 | lost | % of all positives | decoys+negatives AUTO | stress AUTO |
|---|---|---|---|---|---|---|---|
| block-1 | novote | 244 | 238 | 6 (2.5% of those) | 1.4% | 0 | 0 |
| block-1 | vote | 266 | 259 | 7 (2.6% of those) | 1.6% | 0 | 0 |
| block-20 | novote | 98 | 91 | 7 (7.1% of those) | 1.6% | 0 | 0 |
| block-20 | vote | 169 | 162 | 7 (4.1% of those) | 1.6% | 0 | 0 |

Largest AUTO loss: 7.1% of the positives that were AUTO (budget 3%).

## Known GOG label errors

| GOG | name | G8 | G7 (with the override's Steam address) | sources |
|---|---|---|---|---|
| 0x00667790 | - | - | - | lead list (not a GOG function address) |
| 0x00456040 | CSWCArea::AddWeatherObject | n/a | n/a | B0 opus a |
| 0x00457310 | CSWCArea::ClearAllMapPins | pass | mac_contra(0.00) | B0 opus a |
| 0x004744b0 | CSWCObject::AnimationPlayOutOfPlace | n/a | n/a | B0 opus a; gold_annotations; lead list |
| 0x00474890 | CSWCObject::AnimationPause | rotation:0x004745d0:0.99>0.14 | mac_contra(0.00) | B0 opus a; gold_annotations; lead list |
| 0x004cdda0 | CGuiInGame::ShutDownGlobal | n/a | n/a | B0 opus a |
| 0x004db4b0 | CGuiInGame::DestroyDeathMenuGUI | n/a | - | gold_annotations |
| 0x004dbda0 | CGuiInGame::IsCameraDialog | rotation:0x004dbe50:0.91>0.12 | mac_contra(0.00) | B0 opus a |
| 0x004dbde0 | CGuiInGame::IsAnimationPlayingInDialog | pass | mac_contra(0.00) | B0 opus a; gold_annotations |
| 0x004dbe50 | CGuiInGame::IsFadingInDialog | n/a | n/a | B0 opus a |
| 0x004df1f0 | CGuiInGame::GetCanOrientInDialog | pass | n/a | B0 opus a; gold_annotations |
| 0x004dfe10 | CGuiInGame::IsOverlayDialogAnimation_2 | n/a | - | gold_annotations |
| 0x004e0bf0 | CGuiInGame::ShowDialogMessage | rotation:0x004e0c60:0.92>0.17 | - | gold_annotations; lead list |
| 0x004e0c30 | CGuiInGame::SetCombatMessage | n/a | - | lead list |
| 0x004e0c60 | CGuiInGame::GetMiniMapVisible | rotation:0x004e0ca0:0.66>0.13 | mac_contra(0.00) | B0 opus a; gold_annotations; lead list |
| 0x004e0ca0 | CGuiInGame::RemoveGUIPopUps | rotation:0x004e0d20:0.90>0.38 | pass(0.50) | B0 opus a; gold_annotations; lead list |
| 0x004e0d20 | CGuiInGame::ResetInterfaceFont | rotation:0x004e0ca0:0.56>0.22 | - | gold_annotations; lead list |
| 0x0064c2f0 | CTwoDimArrays::GetSwoopUpgradeRowProvider | n/a | n/a | B0 opus a |
| 0x00667600 | CSWSItem::GetPropertyByTypeExists | pass | pass(1.00) | B0 opus a; lead list |
| 0x00667710 | CSWSItem::RestoreUsedActiveProperties | n/a | n/a | B0 opus a; lead list |
| 0x006e5320 | CSWSCreatureStats::GetWeaponFinesse | pass | mac_contra(0.00) | B0 opus b |
| 0x006e5390 | CSWSCreatureStats::GetWeaponSpecialization | rotation:0x006e5320:0.54>0.35 | pass(0.20) | B0 opus b |
| 0x006f5a80 | CSWSCreatureStats::GetCreatureTalentRandomPower | pass | pass(1.00) | B0 opus b |
| 0x006f88f0 | CSWSCreatureStats::ComputeNumberKnownSpellsLeft | pass | mac_contra(0.00) | B0 opus b |
| 0x006f8bb0 | CSWSCreatureStats::CheckInclusionMatch | rotation:0x006f9420:0.50>0.14 | mac_contra(0.00) | B0 opus b |
| 0x006fa1e0 | CSWSCreatureStats::AddSpellLikeAbilityToList | rotation:0x006f9c90:0.95>0.40 | pass(0.25) | B0 opus b |
| 0x006fcf40 | CSWSItem::SetNumActiveProperties | n/a | n/a | B0 opus b |
| 0x00709770 | CSWSArea::GridDFSTransTableInitialize | rotation:0x00709fd0:0.59>0.06 | mac_contra(0.00) | B0 opus b |
| 0x0070ccb0 | CSWSArea::GridDFSTransTablePut | rotation:0x0070ceb0:0.70>0.41 | - | lead list |
| 0x0070cd60 | CSWSArea::GridDFSTransTableGet | pass | pass(1.00) | B0 opus c; lead list |
| 0x0070cd90 | CSWSArea::PlotSoundPath | n/a | n/a | B0 opus b; lead list |
| 0x0070ceb0 | CSWSArea::PathPointDFSTransTableInitialize | pass | pass(0.33) | B0 opus b; lead list |
| 0x00718400 | CSWSArea::GetNextObjectInArea | pass | pass(1.00) | B0 opus b |
| 0x00718460 | CSWSArea::GetFirstObjectInArea | pass | pass(1.00) | B0 opus b |
| 0x007184d0 | CSWSArea::SetMaxStealthXP | n/a | n/a | B0 opus b |
| 0x00722ee0 | CSWSArea::GetClientArea_2 | n/a | n/a | B0 opus b |
| 0x007c1880 | CSWSItemPropertyHandler::Constructor | n/a | n/a | B0 opus c |
| 0x0084b0b0 | Gob::SetScene | pass | pass(0.50) | B0 opus b |

G8 catches 11 of 37, G7 catches 9, union 15. Missed by both (22): 0x00456040 CSWCArea::AddWeatherObject, 0x004744b0 CSWCObject::AnimationPlayOutOfPlace, 0x004cdda0 CGuiInGame::ShutDownGlobal, 0x004db4b0 CGuiInGame::DestroyDeathMenuGUI, 0x004dbe50 CGuiInGame::IsFadingInDialog, 0x004df1f0 CGuiInGame::GetCanOrientInDialog, 0x004dfe10 CGuiInGame::IsOverlayDialogAnimation_2, 0x004e0c30 CGuiInGame::SetCombatMessage, 0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider, 0x00667600 CSWSItem::GetPropertyByTypeExists, 0x00667710 CSWSItem::RestoreUsedActiveProperties, 0x006f5a80 CSWSCreatureStats::GetCreatureTalentRandomPower, 0x006fcf40 CSWSItem::SetNumActiveProperties, 0x0070cd60 CSWSArea::GridDFSTransTableGet, 0x0070cd90 CSWSArea::PlotSoundPath, 0x0070ceb0 CSWSArea::PathPointDFSTransTableInitialize, 0x00718400 CSWSArea::GetNextObjectInArea, 0x00718460 CSWSArea::GetFirstObjectInArea, 0x007184d0 CSWSArea::SetMaxStealthXP, 0x00722ee0 CSWSArea::GetClientArea_2, 0x007c1880 CSWSItemPropertyHandler::Constructor, 0x0084b0b0 Gob::SetScene

## B0: AUTO rows flipped to REVIEW by G8 (24 of 1037 AUTO)

| class::name | GOG | Steam | better G' | G' name | fits (G' > G) | was |
|---|---|---|---|---|---|---|
| CSWCArea::RemovePlaceableObjectLight | 0x00456ce0 | 0x007aa2e0 | 0x00456de0 | CSWCArea::GetPlaceableObjectLight | 0.96>0.77 (rotation) | sig |
| CSWCArea::GetIsDark | 0x004571f0 | 0x007aa7f0 | 0x004571a0 | CSWCArea::UpdateShadowingLights | 0.87>0.50 (rotation) | sig |
| CTwoDimArrays::Load2DArrays_PersistentVisualEffect | 0x00481d80 | 0x006e8de0 | 0x00481920 | CTwoDimArrays::Load2DArrays_VisualEffect | 0.66>0.48 (rotation) | c2 |
| CTwoDimArrays::Load2DArrays_GenericDoors | 0x00482710 | 0x006e9770 | 0x00482800 | CTwoDimArrays::Load2DArrays_Placeables | 0.78>0.63 (rotation) | c2 |
| CTwoDimArrays::Load2DArrays_AmbientMusic | 0x004830e0 | 0x006ea140 | 0x00483240 | CTwoDimArrays::Load2DArrays_FootstepSounds | 0.67>0.50 (rotation) | c2 |
| CResGFF::GetFieldByLabel | 0x00623a40 | 0x007178e0 | 0x00624190 | CResGFF::AddField | 0.95>0.66 (rotation) | c2 |
| CResGFF::AddStructToStruct | 0x00625aa0 | 0x00719940 | 0x00625960 | CResGFF::AddListElement | 0.75>0.60 (rotation) | c2 |
| CResGFF::WriteFieldVOID | 0x00626310 | 0x0071a1b0 | 0x00625ea0 | CResGFF::WriteFieldCResRef | 0.89>0.70 (rotation) | sig |
| CVirtualMachineStack::AddToIntegerLocation | 0x00641ad0 | 0x006fe890 | 0x00640860 | CVirtualMachineStack::ClearStack | 0.60>0.11 (rotation_in) | sig |
| CVirtualMachineStack::CopyFromStack | 0x00641b20 | 0x006fe8e0 | 0x00641fc0 | CVirtualMachineStack::SaveStack | 0.83>0.65 (rotation) | sig |
| CSWSItem::GetPropertyByType | 0x00667b10 | 0x006042e0 | 0x00667d40 | CSWSItem::ApplyItemProperties | 0.88>0.72 (rotation) | c2 |
| CSWSArea::GetRoom | 0x00705570 | 0x0054b1d0 | 0x00705750 | CSWSArea::GetRoomIndex | 0.98>0.77 (rotation) | sig |
| CSWSArea::SmoothPointsOnPath | 0x00709980 | 0x0054f5e0 | 0x00709770 | CSWSArea::GridDFSTransTableInitialize | 0.80>0.63 (rotation) | sig |
| CSWSArea::SmoothSelectionRestartable | 0x0070a060 | 0x0054fe40 | 0x00709d30 | CSWSArea::SmoothPointsOnPathRestartable | 0.86>0.28 (rotation) | sig |
| CSWSArea::GridDFSTransTablePut | 0x0070ccb0 | 0x00552a90 | 0x0070ceb0 | CSWSArea::PathPointDFSTransTableInitialize | 0.70>0.41 (rotation) | sig |
| CSWSArea::LoadStores | 0x0071ce60 | 0x00527cb0 | 0x0071ca40 | CSWSArea::LoadPlaceables | 0.70>0.53 (rotation) | c2 |
| CSWSArea::SaveTriggers | 0x0071fb00 | 0x0052a950 | 0x0071ff40 | CSWSArea::SavePlaceables | 0.86>0.59 (rotation) | c2 |
| CSWSEffectListHandler::OnApplyAssuredHit | 0x0073b770 | 0x00598e80 | 0x0073b7e0 | CSWSEffectListHandler::OnRemoveAssuredHit | 0.98>0.81 (rotation) | sig |
| CSWSPlaceable::SetLightIsOn | 0x00770810 | 0x005f6470 | 0x00770910 | CSWSPlaceable::GetLightIsOn | 0.94>0.76 (rotation) | sig |
| CSWSCombatRound::AddAction | 0x00774920 | 0x00590270 | 0x00774b70 | CSWSCombatRound::RemoveLastAction | 0.90>0.51 (rotation) | sig |
| CSWSCombatRound::AddSwitchWeaponsAction | 0x00775e90 | 0x005917e0 | 0x00775f40 | CSWSCombatRound::AddSWItemSpellAction | 0.81>0.62 (rotation) | sig |
| CSWSCombatRound::AddCutsceneMoveActions | 0x00776060 | 0x005919b0 | 0x00775f40 | CSWSCombatRound::AddSWItemSpellAction | 0.82>0.66 (rotation) | sig |
| CSWSEncounter::GetInActivateArea | 0x007ea040 | 0x00623b90 | 0x007e9f00 | CSWSEncounter::LineSegmentIntersectActivateArea | 0.87>0.50 (rotation) | sig |
| Scene::LoadVisibility | 0x00861e10 | 0x0046fbd0 | 0x00861f60 | Scene::SaveVisibility | 0.81>0.42 (rotation) | sig |

B0 rows with any G8 flag (any class): 44
