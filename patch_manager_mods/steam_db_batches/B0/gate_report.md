# Gate report B0

Rows: 1704. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CClientExoApp | 14 | 38 | 0 | 0 | 52 |
| CClientOptions | 0 | 1 | 0 | 0 | 1 |
| CExoResMan | 34 | 26 | 0 | 0 | 60 |
| CExoString | 0 | 4 | 0 | 0 | 4 |
| CGameEffect | 7 | 14 | 0 | 0 | 21 |
| CGameObjectArray | 1 | 0 | 0 | 0 | 1 |
| CGuiInGame | 32 | 21 | 0 | 0 | 53 |
| CItemRepository | 4 | 11 | 0 | 0 | 15 |
| CRes | 1 | 6 | 0 | 0 | 7 |
| CResGFF | 39 | 26 | 0 | 0 | 65 |
| CSWCArea | 37 | 15 | 0 | 0 | 52 |
| CSWCAreaOfEffectObject | 14 | 2 | 0 | 0 | 16 |
| CSWCDoor | 16 | 4 | 0 | 0 | 20 |
| CSWCLevelUpStats | 0 | 1 | 0 | 0 | 1 |
| CSWCObject | 10 | 23 | 0 | 0 | 33 |
| CSWCPlaceable | 19 | 6 | 0 | 0 | 25 |
| CSWGuiBorder | 7 | 1 | 0 | 0 | 8 |
| CSWGuiBorderParams | 4 | 0 | 1 | 0 | 5 |
| CSWGuiControl | 7 | 3 | 0 | 0 | 10 |
| CSWGuiInGameInventory | 16 | 2 | 0 | 0 | 18 |
| CSWGuiMainCharGen | 0 | 1 | 0 | 0 | 1 |
| CSWGuiMainMenu | 0 | 2 | 0 | 0 | 2 |
| CSWGuiManager | 3 | 0 | 0 | 0 | 3 |
| CSWGuiPanel | 19 | 1 | 0 | 0 | 20 |
| CSWGuiSaveLoad | 0 | 5 | 0 | 0 | 5 |
| CSWGuiUpgrade | 20 | 3 | 0 | 0 | 23 |
| CSWInventory | 1 | 0 | 0 | 0 | 1 |
| CSWItem | 0 | 1 | 0 | 0 | 1 |
| CSWPartyTable | 49 | 31 | 0 | 0 | 80 |
| CSWSArea | 87 | 31 | 0 | 0 | 118 |
| CSWSAreaOfEffectObject | 19 | 3 | 0 | 0 | 22 |
| CSWSCombatRound | 40 | 13 | 0 | 0 | 53 |
| CSWSCreature | 0 | 1 | 0 | 0 | 1 |
| CSWSCreatureStats | 98 | 30 | 0 | 0 | 128 |
| CSWSDoor | 25 | 3 | 0 | 0 | 28 |
| CSWSEffectListHandler | 71 | 86 | 0 | 0 | 157 |
| CSWSEncounter | 22 | 7 | 0 | 0 | 29 |
| CSWSItem | 57 | 17 | 0 | 0 | 74 |
| CSWSItemPropertyHandler | 0 | 74 | 0 | 0 | 74 |
| CSWSObject | 16 | 8 | 0 | 0 | 24 |
| CSWSPlaceable | 30 | 8 | 0 | 0 | 38 |
| CSWSScriptVarTable | 12 | 6 | 0 | 0 | 18 |
| CSWSTrigger | 23 | 2 | 0 | 0 | 25 |
| CSWVirtualMachineCommands | 8 | 21 | 0 | 0 | 29 |
| CServerExoApp | 0 | 12 | 0 | 0 | 12 |
| CServerExoAppInternal | 3 | 1 | 0 | 0 | 4 |
| CTwoDimArrays | 87 | 7 | 0 | 0 | 94 |
| CVirtualMachine | 3 | 20 | 0 | 0 | 23 |
| CVirtualMachineInternal | 10 | 21 | 0 | 0 | 31 |
| CVirtualMachineScript | 2 | 0 | 0 | 0 | 2 |
| CVirtualMachineStack | 5 | 5 | 1 | 0 | 11 |
| CWorldTimer | 6 | 7 | 0 | 0 | 13 |
| ConsoleFunc | 0 | 4 | 0 | 0 | 4 |
| Global | 0 | 1 | 0 | 0 | 1 |
| Gob | 22 | 41 | 0 | 0 | 63 |
| Scene | 5 | 20 | 0 | 0 | 25 |
| **all** | 1005 | 697 | 2 | 0 | 1704 |

AUTO by route: c2=375 sig=630 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

- CVirtualMachineStack::ClearStack gog 0x00640860 -> 0x006fd6b0: G4 steam retn 0 vs GOG retn 12 (no thunk explanation)
- CSWGuiBorderParams::SetFillImage gog 0x00912c10 -> 0x00414840: G6 address held by trusted CSWGuiControl

## Duplicates and conflicts (G6)

- CClientExoApp::GetPlayerCreature gog 0x0040c910 -> 0x0073f450: review:held_by=CClientExoApp::GetControlledObject
- CClientExoApp::GetGameObject gog 0x0040c990 -> 0x0073f4d0: review:held_by=ClientApp::GetClientObject
- CClientExoApp::GetInGameGui gog 0x0040cc10 -> 0x0073f750: review:held_by=ClientApp::GetGuiInGame
- CClientExoApp::GetClientOptions gog 0x0040ccf0 -> 0x0072fb00: review:held_by=OptionsHolder::GetGameOptions
- CClientExoApp::GetCharacterChangeInProgress gog 0x0040e480 -> 0x00740f00: review:held_by=CClientExoApp::BusyWaitMuteSound+name_held_elsewhere=0x00740fc0
- CSWCObject::AnimationPause gog 0x00474890 -> 0x007ef730: review:held_by=CSWCObject::AnimationPlayOutOfPlace+name_held_elsewhere=0x007ef350
- CSWCLevelUpStats::GetClass gog 0x0048f9b0 -> 0x00844e40: review:held_by=CSWSCreatureStats::GetClass,CSWSCreatureStats::GetClassByPosition
- CGuiInGame::AppendToMsgBuffer gog 0x004ce720 -> 0x007be090: review:held_by=CSWGuiInGame::AddFeedbackLine
- CGuiInGame::GetCanOrientInDialog gog 0x004df1f0 -> 0x007ceaf0: review:name_held_elsewhere=0x007cf710
- CGuiInGame::GetMiniMapVisible gog 0x004e0c60 -> 0x007d0560: review:held_by=CGuiInGame::RemoveGUIPopUps
- CGuiInGame::RemoveGUIPopUps gog 0x004e0ca0 -> 0x007d05a0: review:held_by=CGuiInGame::GetMiniMapVisible
- CExoString::Constructor gog 0x004eaed0 -> 0x00733540: review:held_by=CExoString::CExoString,CExoString::DefaultConstructor
- CSWGuiMainMenu::OnNewGamePicked gog 0x0053ccd0 -> 0x00882230: review:held_by=CSWGuiMainMenu
- CSWGuiMainMenu::OnLoadGamePicked gog 0x0053cf90 -> 0x008824f0: review:name_held_elsewhere=0x008827a0
- CSWGuiSaveLoad::LoadPCAutoSave gog 0x005d1d20 -> 0x008529e0: review:held_by=LoadAutosaveSlot
- CExoString::Constructor_2 gog 0x00605680 -> 0x007338d0: review:held_by=CExoString::operator=_2
- CExoString::Constructor_4 gog 0x00605790 -> 0x00733680: review:held_by=CExoString::CStrLenConstructor
- CExoString::Destructor gog 0x00605890 -> 0x00733540: review:held_by=CExoString::CExoString,CExoString::DefaultConstructor
- CRes::Constructor gog 0x0061aa20 -> 0x007111a0: review:name_held_elsewhere=0x007110d0
- CSWSObject::GetPortraitId gog 0x006638a0 -> 0x0053f000: review:held_by=CSWSObject::GetLastName
- CSWSItem::GetPropertyByTypeExists gog 0x00667600 -> 0x00603dd0: review:held_by=CSWSItem::RestoreUsedActiveProperties
- CSWSItem::GetUpgradableType gog 0x0066af00 -> 0x00607790: review:held_by=CSWSItem::GetPropertyDescription
- CSWSCreature::GetClientCreature gog 0x006a3970 -> 0x0058ae90: review:held_by=Creature::GetServerObjectVt2c
- CSWSArea::LoadRooms gog 0x0071ab60 -> 0x005259b0: review:held_by=CSWSArea::LoadRoomInfo
- CSWSArea::Constructor_2 gog 0x00720dc0 -> 0x0052bb60: review:held_by=CSWSArea::ClearLineOfSight
- CSWSArea::Destructor_3 gog 0x00720f20 -> 0x0052bb60: review:held_by=CSWSArea::ClearLineOfSight
- CSWSEffectListHandler::OnRemoveSummonParty gog 0x0073c680 -> 0x00599d90: review:held_by=CSWSEffectListHandler::OnApplySummonParty
- CSWSEffectListHandler::OnRemoveSlowInternal gog 0x007444e0 -> 0x005a1c10: review:held_by=CSWSEffectListHandler::OnApplyInvisibility
- CSWSEffectListHandler::OnRemoveSilence gog 0x00745760 -> 0x005a2e90: review:held_by=CSWSEffectListHandler::OnApplyDispelAllMagic
- CSWSEffectListHandler::OnRemoveTimestop gog 0x00747810 -> 0x005a4f20: review:held_by=CSWSEffectListHandler::OnApplyTimestop
- CSWSEffectListHandler::OnRemoveBodyFuel gog 0x0074b0d0 -> 0x005a8820: review:held_by=CSWSEffectListHandler::OnApplyEffectImmunity
- CSWSEffectListHandler::OnApplyEffectImmunity gog 0x0074b110 -> 0x005a87e0: review:held_by=CSWSEffectListHandler::OnApplyBodyFuel
- CServerExoAppInternal::SaveGame gog 0x007b2990 -> 0x0052f610: review:held_by=StartSave
- Gob::EnableRenderBumpedOut gog 0x0084e530 -> 0x0045bf50: review:held_by=CAurGob::ApplyTXIToNamedTexture
- Global::AurPostString gog 0x00866b40 -> 0x00474c00: review:held_by=Other::AurPostString
- ConsoleFunc::Constructor gog 0x00867740 -> 0x004757d0: review:held_by=ConsoleFunc::NoParamConstructor
- ConsoleFunc::Constructor_2 gog 0x008677e0 -> 0x00475870: review:held_by=ConsoleFunc::StringConstructor
- ConsoleFunc::Constructor_3 gog 0x00867880 -> 0x00475910: review:held_by=ConsoleFunc::IntConstructor
- CSWGuiBorderParams::SetFillImage gog 0x00912c10 -> 0x00414840: fail:trusted_holder=CSWGuiControl

## G4 metadata flags

- CSWCArea::LoadArea gog 0x0044f9b0: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWCArea::ApplyFireAndForgetVisualEffect gog 0x004544c0: ['gog_db_param_bad'] (gog_param=5 gog_retn=16 steam_retn=16)
- CSWCArea::SetGlobalLightAmbient gog 0x00454f20: ['gog_db_param_bad'] (gog_param=0 gog_retn=16 steam_retn=16)
- CSWCArea::Unknown_ReturnTrue_7Args gog 0x004550b0: ['gog_db_param_bad'] (gog_param=0 gog_retn=28 steam_retn=28)
- CSWCArea::UpdateDayNightCycle gog 0x004550d0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWCArea::StopWeather gog 0x00456230: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWCArea::AddPlaceableObjectStatic gog 0x00456940: ['gog_db_param_bad'] (gog_param=9 gog_retn=32 steam_retn=32)
- CGuiInGame::ShowUpgradeScreen gog 0x004ddee0: ['gog_db_param_bad'] (gog_param=20 gog_retn=24 steam_retn=24)
- CSWCDoor::LoadModel gog 0x005070e0: ['gog_db_param_bad'] (gog_param=5 gog_retn=12 steam_retn=12)
- CSWCDoor::DoorSaber gog 0x005073b0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCDoor::ToggleDoorState gog 0x00507400: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCDoor::AddFireAndForgetAnimation gog 0x005074b0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWCDoor::MenuActionSecurity gog 0x00507580: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCDoor::MenuActionBash gog 0x005075f0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCPlaceable::LoadModel gog 0x00535230: ['gog_db_param_bad'] (gog_param=5 gog_retn=12 steam_retn=12)
- CSWCPlaceable::AddFireAndForgetAnimation gog 0x005354f0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWCPlaceable::UsePlaceable gog 0x00535560: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCPlaceable::BashPlaceable gog 0x005355c0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCPlaceable::ActionMenuSecurity gog 0x00535fd0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CExoResMan::Dump gog 0x0061b7c0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CExoResMan::ServiceFromImage gog 0x0061d7e0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CExoResMan::Update gog 0x0061dc60: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CExoResMan::FreeResourceData_2 gog 0x006206e0: ['gog_db_param_bad'] (gog_param=0 gog_retn=16 steam_retn=16)
- CExoResMan::CacheResource_2 gog 0x00620740: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=16)
- CResGFF::ReadFieldBYTE gog 0x00624b60: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CResGFF::CreateGFFFile gog 0x00626530: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CVirtualMachineStack::ClearStack gog 0x00640860: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=0)
- CVirtualMachine::SetScriptParams gog 0x00640a60: ['gog_db_param_bad'] (gog_param=21 gog_retn=28 steam_retn=28)
- CSWSItem::EventHandler gog 0x006648e0: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSItem::GetUpgradableType gog 0x0066af00: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSItem::PowerItem gog 0x00679380: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSItem::non_virtual_thunk_to_dtor_CSWSItem gog 0x0067a210: ['retn_meta_mismatch'] (gog_param=0 gog_retn= steam_retn=4)
- CSWSCreatureStats::GetDamageBonus gog 0x006e83f0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSCreatureStats::ResolveSpecialAttackAttackBonus gog 0x006e9340: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSCreatureStats::GetFavoredEnemyBonus gog 0x006e9620: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSCreatureStats::SetSTRBase gog 0x006f2c50: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSCreatureStats::GetSpellUsesLeft gog 0x006f53c0: ['gog_db_param_bad'] (gog_param=5 gog_retn=16 steam_retn=16)
- CSWSCreatureStats::GetCreatureTalentRandomPowerFromItem gog 0x006f7820: ['gog_db_param_bad'] (gog_param=16 gog_retn=36 steam_retn=36)
- CSWSCreatureStats::GetCreatureTalentRandomFeat gog 0x006f8560: ['gog_db_param_bad'] (gog_param=16 gog_retn=28 steam_retn=28)
- CSWSCreatureStats::GetCreatureTalentRandomKnownPower gog 0x006f9420: ['gog_db_param_bad'] (gog_param=24 gog_retn=32 steam_retn=32)
- CSWSArea::ComputeNonVisibleLocation gog 0x00706ac0: ['gog_db_param_bad'] (gog_param=16 gog_retn=20 steam_retn=20)
- CSWSArea::TestDirectLine gog 0x0070aba0: ['gog_db_param_bad'] (gog_param=16 gog_retn=24 steam_retn=24)
- CSWSArea::NoNoneWalkPolysInStaticObject gog 0x0070cfb0: ['gog_db_param_bad'] (gog_param=21 gog_retn=36 steam_retn=36)
- CSWSArea::PathPointDFSGenerateSuccessors gog 0x0070f7e0: ['gog_db_param_bad'] (gog_param=0 gog_retn=16 steam_retn=16)
- CSWSArea::LoadRoomInfo gog 0x0071aa50: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSArea::LoadRooms gog 0x0071ab60: ['gog_db_param_bad'] (gog_param=4 gog_retn=0 steam_retn=0)
- CSWSArea::LoadWaypoints gog 0x0071c4e0: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSArea::LoadAreaEffects gog 0x0071d230: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSArea::SaveGIT gog 0x0071ef80: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSArea::HandleTransparentDoors gog 0x00720b70: ['gog_db_param_bad'] (gog_param=36 gog_retn=40 steam_retn=40)
- CSWSArea::Constructor_2 gog 0x00720dc0: ['retn_meta_mismatch'] (gog_param=0 gog_retn= steam_retn=32)
- CSWSArea::Destructor_3 gog 0x00720f20: ['gog_db_param_bad'] (gog_param=0 gog_retn=32 steam_retn=32)
- CSWSArea::LoadPathPoints gog 0x00721db0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSEffectListHandler::OnApplyForceShield gog 0x007370f0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyFury gog 0x00737830: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyDestroyShields gog 0x00737f80: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyAssassinate gog 0x007380a0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyAssuredHit gog 0x0073b770: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveAssuredHit gog 0x0073b7e0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyAssuredDeflection gog 0x0073b830: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveAssuredDeflection gog 0x0073b8d0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyResurrection gog 0x0073b940: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplySummonParty gog 0x0073c300: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveSummonParty gog 0x0073c680: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplySavingThrowIncrease gog 0x0073c6d0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplySavingThrowDecrease gog 0x0073c730: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyACIncrease gog 0x0073c7f0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyACDecrease gog 0x0073cda0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyAttackIncrease gog 0x0073d2d0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyAttackDecrease gog 0x0073d330: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyDamageIncrease gog 0x0073d410: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyDamageDecrease gog 0x0073d450: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveDamageDecrease gog 0x0073d4f0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyTemporaryHitpoints gog 0x0073d540: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyDamageImmunityIncrease gog 0x0073da20: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyDamageImmunityDecrease gog 0x0073dbd0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveEntangled gog 0x0073e160: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyDisarm gog 0x0073e870: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplySetAIState gog 0x0073eca0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyEnemyAttackBonus gog 0x0073f0e0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyArcaneSpellFailure gog 0x0073f2a0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyMovementSpeedIncrease gog 0x00741260: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyMovementSpeedDecrease gog 0x00741470: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyVisualEffect gog 0x00741560: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyAreaOfEffect gog 0x00741a40: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyForceResistanceIncrease gog 0x00741fe0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyForceResistanceDecrease gog 0x007422a0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyRegenerate gog 0x00742580: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveDisease gog 0x007434d0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnRemoveHasteInternal gog 0x00744160: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnRemoveSlowInternal gog 0x007444e0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyInvisibility gog 0x00744520: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplySanctuary gog 0x00744740: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyVision gog 0x00744e00: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyModifyNumAttacks gog 0x00744f10: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveSilence gog 0x00745760: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyDispelAllMagic gog 0x007457d0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyDispelBestMagic gog 0x00745c80: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveTaunt gog 0x007462b0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyLimitMovementSpeed gog 0x00746300: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplySkillIncrease gog 0x00746470: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplySkillDecrease gog 0x007464e0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplySpecialWalkAnimation gog 0x00746b70: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyEffectIcon gog 0x00746cb0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyTimestop gog 0x00747660: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveTimestop gog 0x00747810: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyRacialType gog 0x00747860: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplySeeInvisible gog 0x007478c0: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyTrueSeeing gog 0x00747ef0: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyMissChance gog 0x00748790: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyTurnResistance gog 0x00748940: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyDisappearAppear gog 0x007489a0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyDisappear gog 0x00748db0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyAppear gog 0x00748e60: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyBonusFeat gog 0x007495d0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyVampiricRegeneration gog 0x00749ba0: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyTemporaryForce gog 0x00749cf0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyForceDrain gog 0x00749e00: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyForceJump gog 0x00749ec0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyForceJumpDelayed gog 0x0074a0d0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyForcePush gog 0x0074a720: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveForcePush gog 0x0074acc0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnRemoveHorrified gog 0x0074ade0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyDamageForcePoints gog 0x0074ae10: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyHealForcePoints gog 0x0074af10: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyBodyFuel gog 0x0074b090: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveBodyFuel gog 0x0074b0d0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyEffectImmunity gog 0x0074b110: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyLightsaberThrow gog 0x0074b140: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnRemoveLightsaberThrow gog 0x0074bdc0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSEffectListHandler::OnApplyForceFizzle gog 0x0074bf20: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyPureGoodPowers gog 0x0074c010: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSEffectListHandler::OnApplyPureEvilPowers gog 0x0074c480: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSTrigger::EventHandler gog 0x0075e400: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSTrigger::CreateNewGeometry gog 0x00761530: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWSDoor::GetMeshBoundingBox gog 0x007627d0: ['gog_db_param_bad'] (gog_param=12 gog_retn=32 steam_retn=32)
- CSWSDoor::GetActionPoint gog 0x00762d80: ['gog_db_param_bad'] (gog_param=12 gog_retn=40 steam_retn=40)
- CSWSPlaceable::AddToArea gog 0x0076ce50: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSPlaceable::OnEndDialog gog 0x00770dd0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSCombatRound::AddAttackAction gog 0x00775100: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSCombatRound::DEBUGDumpCombatRoundToScreen gog 0x007776b0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWVirtualMachineCommands::ExecuteCommandActionDoCommand gog 0x0077efb0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWVirtualMachineCommands::ExecuteCommandActionEquipMostDamagingMelee gog 0x0077f030: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWVirtualMachineCommands::ExecuteCommandAttack gog 0x007830d0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWVirtualMachineCommands::ExecuteCommandRandom gog 0x007a4c20: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CServerExoAppInternal::SaveGame gog 0x007b2990: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyLight gog 0x007c4dc0: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveLight gog 0x007c4ee0: ['gog_db_param_bad'] (gog_param=0 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyAbilityBonus gog 0x007c4f00: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveAbilityBonus gog 0x007c5100: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyACBonus gog 0x007c53c0: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveACBonus gog 0x007c5670: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyImprovedForceResistance gog 0x007c5940: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveImprovedForceResistance gog 0x007c5a90: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyImprovedSavingThrow gog 0x007c5c30: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveImprovedSavingThrow gog 0x007c5cf0: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyReducedSavingThrow gog 0x007c5db0: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveReducedSavingThrow gog 0x007c5e70: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplySpecialWalk gog 0x007c6260: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveSpecialWalk gog 0x007c6430: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyTrueSeeing gog 0x007c65c0: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveTrueSeeing gog 0x007c66a0: ['gog_db_param_bad'] (gog_param=4 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyFreedomOfMovement gog 0x007c6770: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveFreedomOfMovement gog 0x007c6b20: ['gog_db_param_bad'] (gog_param=4 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyBonusFeat gog 0x007c6c70: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveBonusFeat gog 0x007c6d90: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDamageImmunity gog 0x007c6e90: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDamageImmunity gog 0x007c7020: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDamageReduction gog 0x007c7900: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDamageReduction gog 0x007c7a70: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDamageResistance gog 0x007c7c30: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDamageResistance gog 0x007c7da0: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDamageVulnerability gog 0x007c7f60: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDamageVulnerability gog 0x007c80d0: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDecreaseAbilityScore gog 0x007c8270: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDecreaseAbilityScore gog 0x007c8470: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDecreaseAC gog 0x007c8730: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDecreaseAC gog 0x007c88d0: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDecreaseSkillModifier gog 0x007c8ac0: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDecreaseSkillModifier gog 0x007c8c50: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyImmunity gog 0x007c8e40: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveImmunity gog 0x007c9110: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplySkillBonus gog 0x007c9340: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveSkillBonus gog 0x007c9490: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyRegeneration gog 0x007c95d0: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveRegeneration gog 0x007c9750: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionIncrease gog 0x007c9840: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionIncrease gog 0x007c9970: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionDecrease gog 0x007c9a60: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionDecrease gog 0x007c9b90: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDisguise gog 0x007c9c80: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDisguise gog 0x007c9d90: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDampenSound gog 0x007c9e80: ['gog_db_param_bad'] (gog_param=0 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveDampenSound gog 0x007c9ea0: ['gog_db_param_bad'] (gog_param=0 gog_retn=16 steam_retn=16)
- CSWSItemPropertyHandler::ApplyDoorCutting gog 0x007c9ec0: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::ApplyDoorSabering gog 0x007c9ef0: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::ApplySniperBonus gog 0x007c9f20: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::ApplyRapidShotBonus gog 0x007c9f50: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::ApplyMaxDexBonus gog 0x007c9f80: ['gog_db_param_bad'] (gog_param=8 gog_retn=20 steam_retn=20)
- CSWSItemPropertyHandler::RemoveMaxDexBonus gog 0x007c9fd0: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=16)
- CSWSAreaOfEffectObject::EventHandler gog 0x007e6800: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSAreaOfEffectObject::LineSegmentIntersectAreaOfEffect gog 0x007e6f10: ['gog_db_param_bad'] (gog_param=8 gog_retn=24 steam_retn=24)
- CSWSAreaOfEffectObject::InAreaOfEffect gog 0x007e7450: ['gog_db_param_bad'] (gog_param=1 gog_retn=12 steam_retn=12)
- CSWSEncounter::EventHandler gog 0x007e9b40: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSEncounter::RemoveFromActiveCreatureCount gog 0x007eb020: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWSEncounter::SpawnIfAppropriate gog 0x007ee1a0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Gob::SetScene gog 0x0084b0b0: ['gog_db_param_bad'] (gog_param=4 gog_retn=0 steam_retn=0)
- Scene::RemoveFromSceneManager_2 gog 0x008646a0: ['gog_db_param_bad'] (gog_param=4 gog_retn=0 steam_retn=0)
- Global::AurPostString gog 0x00866b40: ['gog_db_param_bad'] (gog_param=16 gog_retn=0 steam_retn=0)

## Trusted pair body conflicts

Trusted rows that are NOT seeds (exact name + equal retn + equal size or A-SIG >= 0.5 with intersection >= 3 required, Opus gate R1). They stay trusted as G6 holders until Opus rules on them.

- GOG 0x00815520 CAurTexture::SetIsEnvironmentMapped -> Steam 0x00422e20: name is not exact (suffix on the trusted row)
- GOG 0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam 0x0051e170: name is not exact (suffix on the trusted row)
- GOG 0x0064e730 CServerAIMaster::EventPending -> Steam 0x0051ed10: name is not exact (suffix on the trusted row)
- GOG 0x007b2f00 CServerExoAppInternal::LoadGame -> Steam 0x0052fb80: name is not exact (suffix on the trusted row)
- GOG 0x007ef060 CFactionManager::SetNPCFactionReputation -> Steam 0x006644d0: name is not exact (suffix on the trusted row)
- GOG 0x007ef110 CFactionManager::GetNPCFactionReputation -> Steam 0x00664580: name is not exact (suffix on the trusted row)
- GOG 0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam 0x00664650: name is not exact (suffix on the trusted row)
- GOG 0x00780d50 Global::ExecuteCommandChangeObjectAppearance -> Steam 0x0066b770: name is not exact (suffix on the trusted row)
- GOG 0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0: name is not exact (suffix on the trusted row)
- GOG 0x007e4850 CSWSFaction::AddMember -> Steam 0x006d3330: name is not exact (suffix on the trusted row)
- GOG 0x00606e70 CExoString::CompareNoCase -> Steam 0x00734f60: name is not exact (suffix on the trusted row)
- GOG 0x004df1f0 CGuiInGame::GetCanOrientInDialog -> Steam 0x007cf710: size GOG 90 vs Steam 112, SIG 0.10 (intersection 1)
- GOG 0x00478360 CSWParty::AddCharacter -> Steam 0x007e5730: name is not exact (suffix on the trusted row)
- GOG 0x00478870 CSWParty::IsPartyMember -> Steam 0x007e5c40: name is not exact (suffix on the trusted row)
- GOG 0x00474890 CSWCObject::AnimationPause -> Steam 0x007ef350: size GOG 68 vs Steam 129, SIG 0.08 (intersection 1)
- GOG 0x00597ff0 CSWGuiInGameCharacter::SetStats -> Steam 0x0084e6f0: name is not exact (suffix on the trusted row)
- GOG 0x005995d0 CSWGuiInGameCharacter::ShowLevelUpGUI -> Steam 0x0084fcd0: name is not exact (suffix on the trusted row)
- GOG 0x0053cf90 CSWGuiMainMenu::OnLoadGamePicked -> Steam 0x008827a0: retn GOG 0 vs Steam 4

## Ambiguous rows (second candidate also reaches C2)

(none)

## Lessons

<!-- one lesson per line: `- scope | topic | note`; persist inserts each once (dedupe on scope+topic) -->
- steam | CORRECTION to lesson 358: 0x8827a0 is OnLoadSaveGame, OnLoadGamePicked is 0x8824f0 | The +0x5dc main-menu button binding to 0x8827a0 (lesson 358) stands, but its symbol is CSWGuiMainMenu::OnLoadSaveGame: 395 B RET 4, allocates CSWGuiSaveLoad (Steam PUSH 0x1e44, GOG 0x53d240 imm 0x1b74), Mac OnLoadSaveGame. OnLoadGamePicked = 0x8824f0: fingerprint-identical to GOG 0x53cf90 (679 B, ret 0, eleven imm -3 = Mac OnLoadGamePicked eleven 0xfd). Name UI callbacks by real (Mac) symbols, not by which button binds them. Evidence steam_db_batches/B0.
- steam-db / method | KPM GOG DB labels are rotated for ~62 functions: never ship a GOG name on fingerprint identity alone | B0 found every pipeline Steam BODY right but ~62 GOG DB names wrong (rotations/swaps in CGuiInGame, CSWCObject animation block, CSWSArea path/DFS/Smooth*, CSWSCreatureStats weapon/talent, CSWSItem property block, CSWCArea map pins; 3 pseudo-functions). Guards: G7 Mac contradiction, G8 Mac rotation (catch 15/37 known), plus an Opus sweep of AUTO rows within +-4 GOG positions of any label_suspect. Ledger: steam_db_batches/GOG_LABEL_SUSPECTS.md.
- steam-db / method | GOG parity rule for GameAPI-required names whose GOG label is wrong | User decision 2026-10-03: ship the name at the proven (unique fingerprint-identical) Steam counterpart of GOG's address, note 'GOG parity'; not proven (e.g. two identical twins) -> omit. B0 shipped 6 (B0/parity.tsv); Gob::SetProcessFlag omitted (twins 0x459560/0x464bd0).
- steam-db / process | Sonnet card reviewers are a filter, not a gate: budget 100% Opus for reviewer matches | B0: the 20% Opus sample found 1 label error in 102 reviewer matches -> escalated to 100% Opus (e1-e4), 0 of 140 decoys accepted. Sonnet ~250-330k tokens per 60-card chunk, Opus ~200-340k per 75-100 gated rows.
- steam-db / process | Validated ledger: final validation passes skip rows marked in steam_validated | Every shipped row a batch validated is marked in KDB table steam_validated (steam_addr, name, verdict, tier, batch, Steam fp hash) by `batch.py persist`; `batch.py check-validated` lists branch rows as validated (skip) / stale (moved or body hash changed) / unvalidated. DESIGN.md 4.7.
