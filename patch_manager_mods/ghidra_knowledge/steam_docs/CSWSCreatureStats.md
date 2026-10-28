# CSWSCreatureStats

124 functions in the Steam DB for CSWSCreatureStats.

## 0x006a4260 CSWSCreatureStats::Constructor

- Size 1894 B, RET 4
- Callers (1 total):
  - 0x00561f30 CSWSCreature::Constructor (site 0x0056218c)
- Callees (28):
  - 0x006a428d CALL 0x005933b0
  - 0x006a42a1 CALL 0x0084a640
  - 0x006a42b2 CALL 0x005933b0
  - 0x006a42cd CALL 0x0084a640
  - 0x006a42dc CALL 0x00735240
  - 0x006a42eb CALL 0x00735240
  - 0x006a42fa CALL 0x007107f0
  - 0x006a4305 CALL 0x00735240
  - 0x006a4326 CALL 0x0091d67e
  - 0x006a4338 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a434a CALL 0x007107f0
  - 0x006a4358 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a4363 CALL 0x00919723 operator new
  - 0x006a437b CALL 0x006f58b0 PendingLevelUp::Constructor
  - 0x006a43a6 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006a43ce CALL 0x00710eb0
  - 0x006a442a CALL 0x007338d0
  - 0x006a4440 CALL 0x006b6a20 CSWSCreatureStats::SetClassLevel
  - 0x006a444c CALL 0x006b69e0 CSWSCreatureStats::SetClass
  - 0x006a447d CALL 0x006b6a20 CSWSCreatureStats::SetClassLevel
  - 0x006a448f CALL 0x006b69e0 CSWSCreatureStats::SetClass
  - 0x006a46f8 CALL 0x00919723 operator new
  - 0x006a4710 CALL 0x0069cab0
  - 0x006a477a CALL 0x00919723 operator new
  - 0x006a47b0 CALL 0x0091c630
  - 0x006a4892 CALL 0x00710eb0
  - 0x006a4930 CALL 0x00919723 operator new
  - 0x006a494a CALL 0x0064dd10
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Constructs creature stats: initializes embedded member objects, zeroes fields, allocates PendingLevelUp, sets default class and level.
- Named pseudo-C (first 30 of 208 lines):
```c
// NAMED 0x006a4260 CSWSCreatureStats::Constructor evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=postpass exe=6a522e71631d

int CSWSCreatureStats__Constructor(undefined4 param_1)

{
  uint uVar1;
  int iVar2;
  int in_ECX;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined1 local_4c [16];
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  undefined1 local_30 [16];
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095b92f;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_005933b0(0);
```

## 0x006a49d0 CSWSCreatureStats::Destructor

- Size 606 B, RET 0
- Callers (1 total):
  - 0x005634c0 CSWSCreature::Destructor_2 (site 0x005636ab)
- Callees (19):
  - 0x006a4a1b CALL 0x009196fd operator delete
  - 0x006a4a33 CALL 0x006b8370 CSWSCreatureStats::ClearFeats
  - 0x006a4a74 CALL 0x0049b840
  - 0x006a4a8e CALL 0x006f59b0
  - 0x006a4aa1 CALL 0x009196fd operator delete
  - 0x006a4ae1 CALL 0x0064ddb0
  - 0x006a4af4 CALL 0x009196fd operator delete
  - 0x006a4b3f CALL 0x0069cbe0
  - 0x006a4b52 CALL 0x009196fd operator delete
  - 0x006a4b83 CALL 0x00733780 CExoString::Destructor
  - 0x006a4b95 CALL 0x00733780 CExoString::Destructor
  - 0x006a4bb1 CALL 0x00919bcc
  - 0x006a4bc0 CALL 0x00735360 CExoLocString::Destructor
  - 0x006a4bcf CALL 0x00735360 CExoLocString::Destructor
  - 0x006a4bde CALL 0x00735360 CExoLocString::Destructor
  - 0x006a4bed CALL 0x0064ddb0
  - 0x006a4bfc CALL 0x0064ddb0
  - 0x006a4c0b CALL 0x0064ddb0
  - 0x006a4c1a CALL 0x0064ddb0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Tears down creature stats: frees owned buffer, clears feats, deletes per-level stat records, destroys strings and loc-strings.
- Named pseudo-C (first 30 of 72 lines):
```c
// NAMED 0x006a49d0 CSWSCreatureStats::Destructor evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=postpass exe=6a522e71631d

void CSWSCreatureStats__Destructor(void)

{
  int iVar1;
  uint uVar2;
  int in_ECX;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0095b9ce;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8 = 9;
  if (*(int *)(in_ECX + 0x174) != 0) {
    operator delete(*(undefined4 *)(in_ECX + 0x174),uVar2);
    *(undefined4 *)(in_ECX + 0x174) = 0;
  }
  CSWSCreatureStats__ClearFeats();
  local_14 = *(int *)(in_ECX + 0x2c);
  while (local_14 = local_14 + -1, -1 < local_14) {
    iVar1 = *(int *)(*(int *)(in_ECX + 0x28) + local_14 * 4);
    FUN_0049b840(local_14);
    if (iVar1 != 0) {
      FUN_006f59b0();
      operator delete(iVar1);
```

## 0x006a4c30 CSWSCreatureStats::GetFullName

- Size 145 B, RET 4
- Callers (7 total, first 5 shown):
  - 0x0058a090 CSWSCreature::DumpToLog (site 0x0058a0cb)
  - 0x00597d90 CSWSEffectListHandler::OnApplyDeath (site 0x005988dd)
  - 0x00631560 (site 0x00631dce)
  - 0x00660710 (site 0x006612bf)
  - 0x0088aac0 (site 0x0088bf66)
- Callees (4):
  - 0x006a4c62 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a4c84 CALL 0x00633620
  - 0x006a4c93 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006a4ca8 CALL 0x00733780 CExoString::Destructor
- Imports: none
- Indirect slots: none
- Ledger callsites in range (1):
  - 0x006a4c84 FUN_006a4c30 -> FUN_00633620, gender PUSH 0 (constant 0)
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Fills output string with full name, built from two name fields via a helper (likely first and last).
- Named pseudo-C (first 25 of 25 lines):
```c
// NAMED 0x006a4c30 CSWSCreatureStats::GetFullName evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=3 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetFullName(undefined4 param_1)

{
  int in_ECX;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972a61;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  CExoString__DefaultConstructor(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 1;
  FUN_00633620(0xffffffff,in_ECX + 0x34,in_ECX + 0x3c,local_18,0);
  CExoString__CExoString_copy(local_18);
  local_8 = local_8 & 0xffffff00;
  CExoString__Destructor();
  ExceptionList = local_10;
  return param_1;
}

```

## 0x006a4cd0 CSWSCreatureStats::GetSpellGainWithBonus

- Size 288 B, RET 8
- Callers (1 total):
  - 0x006bce00 CSWSCreatureStats::AdjustSpellUsesPerDay (site 0x006bcec2)
- Callees (3):
  - 0x006a4d42 CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006a4dbc CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006a4dc8 CALL 0x006f09d0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns spells gained for a class at a spell level, plus an ability-modifier bonus for force-user classes.
- Named pseudo-C (first 30 of 54 lines):
```c
// NAMED 0x006a4cd0 CSWSCreatureStats::GetSpellGainWithBonus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=postpass exe=6a522e71631d

uint CSWSCreatureStats__GetSpellGainWithBonus(byte param_1,byte param_2)

{
  undefined1 uVar1;
  uint uVar2;
  int iVar3;
  int in_ECX;
  int local_18;
  byte local_e;
  char local_d;
  byte local_5;
  
  local_5 = 0;
  local_d = '\0';
  uVar2 = (uint)param_1;
  if (uVar2 < *(byte *)(in_ECX + 0x8d)) {
    local_e = *(byte *)(in_ECX + 0xab + (uint)param_1 * 0x28);
    uVar2 = CONCAT31((int3)((uint)param_1 * 0x28 >> 8),local_e);
  }
  else {
    local_e = 0xff;
  }
  if ((uint)local_e * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) == 0) {
    uVar2 = uVar2 & 0xffffff00;
  }
  else {
    iVar3 = IsForceUserClass(classId) {3,4,5,11..16}(local_e);
    if (iVar3 != 0) {
```

## 0x006a4e00 CSWSCreatureStats::GetLevel

- Size 540 B, RET 4
- Callers (49 total, first 5 shown):
  - 0x0057e8c0 CSWSCreature::GetMaxHitPoints (site 0x0057e942)
  - 0x0057eca0 CSWSCreature::GetMaxForcePoints (site 0x0057ecf6)
  - 0x0058a1c0 CSWSCreature::CalculateSpellSaveDC (site 0x0058a20a)
  - 0x005a2ee0 CSWSEffectListHandler::OnApplyDispelAllMagic (site 0x005a2f99)
  - 0x005a3390 CSWSEffectListHandler::OnApplyDispelBestMagic (site 0x005a3449)
- Callees (9):
  - 0x006a4e78 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006a4f29 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006a4f44 CALL 0x0071b670
  - 0x006a4f53 CALL 0x00733780 CExoString::Destructor
  - 0x006a4f7a CALL 0x00733570 CExoString::CStrConstructor
  - 0x006a4f9d CALL 0x0051c890 CServerExoApp::GetGlobalVariableTable
  - 0x006a4fa4 CALL 0x00654860 CSWGlobalVariableTable::GetValueNumber
  - 0x006a4fb3 CALL 0x00733780 CExoString::Destructor
  - 0x006a4fe4 CALL 0x0091c860
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4a4 READ (MOV ECX,dword ptr [0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns total level by summing class levels; a later branch likely consults a global-variable override.
- Named pseudo-C (first 30 of 76 lines):
```c
// NAMED 0x006a4e00 CSWSCreatureStats::GetLevel evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=4 exe=6a522e71631d

undefined1 CSWSCreatureStats__GetLevel(undefined4 param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int in_ECX;
  undefined1 *puVar4;
  char *pcVar5;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined4 local_30;
  char local_2a;
  char local_29;
  undefined1 local_28 [4];
  uint local_24;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ba00;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
```

## 0x006a5020 CSWSCreatureStats::GetIsJedi

- Size 229 B, RET 0
- Callers (1 total):
  - 0x0057eca0 CSWSCreature::GetMaxForcePoints (site 0x0057f2db)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns true if any class slot holds one of nine force-user class IDs (3,4,5,11 through 16).
- Named pseudo-C (first 29 of 29 lines):
```c
// NAMED 0x006a5020 CSWSCreatureStats::GetIsJedi evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetIsJedi(void)

{
  int in_ECX;
  char local_e;
  uint local_8;
  
  local_8 = 0;
  while( true ) {
    if ((int)(uint)*(byte *)(in_ECX + 0x8d) <= (int)local_8) {
      return 0;
    }
    if ((local_8 & 0xff) < (uint)*(byte *)(in_ECX + 0x8d)) {
      local_e = *(char *)(in_ECX + 0xab + (local_8 & 0xff) * 0x28);
    }
    else {
      local_e = -1;
    }
    if ((((((local_e == '\x03') || (local_e == '\x04')) || (local_e == '\x05')) ||
         ((local_e == '\r' || (local_e == '\f')))) ||
        ((local_e == '\v' || ((local_e == '\x0f' || (local_e == '\x10')))))) || (local_e == '\x0e'))
    break;
    local_8 = local_8 + 1;
  }
  return 1;
}

```

## 0x006a5110 CSWSCreatureStats::GetArmorClassVersus

- Size 7751 B, RET 8
- Callers (4 total):
  - 0x006710d0 Global::ExecuteCommandDoTouchAttack (site 0x0067127c)
  - 0x006d4560 (site 0x006d45e7)
  - 0x006d46d0 (site 0x006d4754)
  - 0x006e0b10 CSWSCreature::ResolveAttackRoll (site 0x006e0c5e)
- Callees (141):
  - 0x006a5182 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5247 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006a5253 CALL 0x006f2ed0
  - 0x006a52ec CALL 0x00733570 CExoString::CStrConstructor
  - 0x006a5310 CALL 0x0071b670
  - 0x006a5322 CALL 0x00733780 CExoString::Destructor
  - 0x006a53b3 CALL 0x0091c860
  - 0x006a53f6 CALL 0x006bad60
  - 0x006a54d1 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a54e0 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006a54f2 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a5515 CALL 0x00734270 CExoString::Format
  - 0x006a5531 CALL 0x00733df0 CExoString::operator+
  - 0x006a556c CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5576 CALL 0x007337c0 CExoString::operator=
  - 0x006a5585 CALL 0x00733780 CExoString::Destructor
  - 0x006a55a3 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a55b2 CALL 0x007337c0 CExoString::operator=
  - 0x006a55d1 CALL 0x00734270 CExoString::Format
  - 0x006a55ed CALL 0x00733df0 CExoString::operator+
  - 0x006a5628 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5632 CALL 0x007337c0 CExoString::operator=
  - 0x006a5641 CALL 0x00733780 CExoString::Destructor
  - 0x006a565f CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a566e CALL 0x007337c0 CExoString::operator=
  - 0x006a568d CALL 0x00734270 CExoString::Format
  - 0x006a56a9 CALL 0x00733df0 CExoString::operator+
  - 0x006a56e4 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a56ee CALL 0x007337c0 CExoString::operator=
  - 0x006a56fd CALL 0x00733780 CExoString::Destructor
  - 0x006a571b CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a572a CALL 0x007337c0 CExoString::operator=
  - 0x006a5739 CALL 0x00733780 CExoString::Destructor
  - 0x006a574b CALL 0x00733780 CExoString::Destructor
  - 0x006a5798 indirect
  - 0x006a579e CALL 0x00585480 CSWSCreature::GetInvisible
  - 0x006a57d3 CALL 0x005d1750 CSWSCreature::GetVisibleListElement
  - 0x006a5863 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006a58aa CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a58b9 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006a58cb CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a58f4 CALL 0x00734270 CExoString::Format
  - 0x006a5910 CALL 0x00733df0 CExoString::operator+
  - 0x006a594b CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5955 CALL 0x007337c0 CExoString::operator=
  - 0x006a5964 CALL 0x00733780 CExoString::Destructor
  - 0x006a5982 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5991 CALL 0x007337c0 CExoString::operator=
  - 0x006a59a0 CALL 0x00733780 CExoString::Destructor
  - 0x006a59b2 CALL 0x00733780 CExoString::Destructor
  - 0x006a59c1 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006a59d5 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006a59e8 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006a5a49 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5a58 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006a5a6a CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a5a8d CALL 0x00734270 CExoString::Format
  - 0x006a5aa9 CALL 0x00733df0 CExoString::operator+
  - 0x006a5ae4 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5aee CALL 0x007337c0 CExoString::operator=
  - 0x006a5afd CALL 0x00733780 CExoString::Destructor
  - 0x006a5b1b CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5b2a CALL 0x007337c0 CExoString::operator=
  - 0x006a5b49 CALL 0x00734270 CExoString::Format
  - 0x006a5b65 CALL 0x00733df0 CExoString::operator+
  - 0x006a5ba0 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5baa CALL 0x007337c0 CExoString::operator=
  - 0x006a5bb9 CALL 0x00733780 CExoString::Destructor
  - 0x006a5bd7 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a5be6 CALL 0x007337c0 CExoString::operator=
  - 0x006a5bf5 CALL 0x00733780 CExoString::Destructor
  - 0x006a5c07 CALL 0x00733780 CExoString::Destructor
  - 0x006a5d03 CALL 0x0057f870 CSWSCreature::GetDamageFlags
  - 0x006a5e0c CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a5e1f CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a5e32 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a5e45 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a5e58 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a5e6b CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a6270 CALL 0x006daac0 CSWSCreature::IsInTotalDefense
  - 0x006a641f CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006a6472 CALL 0x005763e0 CSWSCreature::IsWieldingLightsaber
  - 0x006a67b6 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a67c5 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006a67d7 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a67fa CALL 0x00734270 CExoString::Format
  - 0x006a6816 CALL 0x00733df0 CExoString::operator+
  - 0x006a6851 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a685b CALL 0x007337c0 CExoString::operator=
  - 0x006a686a CALL 0x00733780 CExoString::Destructor
  - 0x006a6888 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6897 CALL 0x007337c0 CExoString::operator=
  - 0x006a68b6 CALL 0x00734270 CExoString::Format
  - 0x006a68d2 CALL 0x00733df0 CExoString::operator+
  - 0x006a690d CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6917 CALL 0x007337c0 CExoString::operator=
  - 0x006a6926 CALL 0x00733780 CExoString::Destructor
  - 0x006a6944 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6953 CALL 0x007337c0 CExoString::operator=
  - 0x006a6975 CALL 0x00734270 CExoString::Format
  - 0x006a6991 CALL 0x00733df0 CExoString::operator+
  - 0x006a69cc CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a69d6 CALL 0x007337c0 CExoString::operator=
  - 0x006a69e5 CALL 0x00733780 CExoString::Destructor
  - 0x006a6a03 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6a12 CALL 0x007337c0 CExoString::operator=
  - 0x006a6a34 CALL 0x00734270 CExoString::Format
  - 0x006a6a50 CALL 0x00733df0 CExoString::operator+
  - 0x006a6a8b CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6a95 CALL 0x007337c0 CExoString::operator=
  - 0x006a6aa4 CALL 0x00733780 CExoString::Destructor
  - 0x006a6ac2 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6ad1 CALL 0x007337c0 CExoString::operator=
  - 0x006a6af3 CALL 0x00734270 CExoString::Format
  - 0x006a6b0f CALL 0x00733df0 CExoString::operator+
  - 0x006a6b4a CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6b54 CALL 0x007337c0 CExoString::operator=
  - 0x006a6b63 CALL 0x00733780 CExoString::Destructor
  - 0x006a6b81 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6b90 CALL 0x007337c0 CExoString::operator=
  - 0x006a6baf CALL 0x00734270 CExoString::Format
  - 0x006a6bcb CALL 0x00733df0 CExoString::operator+
  - 0x006a6c06 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6c10 CALL 0x007337c0 CExoString::operator=
  - 0x006a6c1f CALL 0x00733780 CExoString::Destructor
  - 0x006a6c3d CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6c4c CALL 0x007337c0 CExoString::operator=
  - 0x006a6c6b CALL 0x00734270 CExoString::Format
  - 0x006a6c87 CALL 0x00733df0 CExoString::operator+
  - 0x006a6cc2 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6ccc CALL 0x007337c0 CExoString::operator=
  - 0x006a6cdb CALL 0x00733780 CExoString::Destructor
  - 0x006a6cf9 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a6d08 CALL 0x007337c0 CExoString::operator=
  - 0x006a6d17 CALL 0x00733780 CExoString::Destructor
  - 0x006a6d29 CALL 0x00733780 CExoString::Destructor
  - 0x006a6d76 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006a6dac CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a6dca CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006a6ef3 CALL 0x006b6600 CSWSCreatureStats::GetTotalACFeatMod
  - 0x006a6f2a CALL 0x006daa00 CSWSCreature::IsDebilitated
- Imports: none
- Indirect slots (1):
  - 0x006a5798 slot +0x10 (006a5795 MOV EAX,dword ptr [EDX + 0x10])
- Ledger callsites in range: none
- Strings (12):
  - 0x009992b0 " + %d (Total Defense Stance Bonus)"
  - 0x009992d4 " + %d (Effect AC Deflection Bonus)"
  - 0x009992f8 " + %d (Effect AC Shield Bonus)"
  - 0x00999318 " + %d (Effect AC Armor Bonus)"
  - 0x00999338 " + %d (Effect AC Natural Bonus)"
  - 0x00999358 " + %d (Effect AC Dodge Bonus)"
  - 0x00999378 " + %d (Dodge Mod)"
  - 0x0099938c " + %d (Dex Mod)"
  - 0x0099939c " + %d (Feat Dodge)"
  - 0x009993b0 " + %d (Shield AC)"
  - 0x009993c4 " + %d (Armor AC)"
  - 0x009993d8 " + %d (Natural AC)"
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes armor class versus an attacker; builds text breakdown of each AC bonus source.
- Named pseudo-C (first 30 of 606 lines):
```c
// NAMED 0x006a5110 CSWSCreatureStats::GetArmorClassVersus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=18 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetArmorClassVersus(int param_1,int param_2)

{
  undefined1 uVar1;
  char cVar2;
  byte bVar3;
  uint uVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  int in_ECX;
  int local_388;
  int local_384;
  int local_380;
  char local_364;
  char local_264;
  char local_262;
  char local_260;
  char local_25e;
  byte local_205;
  byte local_193;
  undefined1 local_184 [8];
  undefined1 local_17c [8];
  undefined1 local_174 [8];
  undefined1 local_16c [8];
  undefined1 local_164 [8];
  undefined1 local_15c [8];
  undefined1 local_154 [8];
```

## 0x006a6f80 CSWSCreatureStats::GetBaseAttackBonus

- Size 493 B, RET 0
- Callers (8 total, first 5 shown):
  - 0x0062f070 (site 0x0062f4a9)
  - 0x00646270 (site 0x006465bf)
  - 0x006a7db0 CSWSCreatureStats::GetMeleeAttackBonus (site 0x006a7eb2)
  - 0x006a8670 (site 0x006a8737)
  - 0x006a93a0 CSWSCreatureStats::GetAttackModifierVersus (site 0x006a9503)
- Callees (6):
  - 0x006a6fe0 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006a703d CALL 0x006f08f0
  - 0x006a70b1 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006a70cc CALL 0x0071b670
  - 0x006a70db CALL 0x00733780 CExoString::Destructor
  - 0x006a7136 CALL 0x0091c860
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings (1):
  - 0x009993ec "ToHitMult"
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns total base attack bonus: sums each class's to-hit table value at its level, likely the ToHitMult column.
- Named pseudo-C (first 30 of 67 lines):
```c
// NAMED 0x006a6f80 CSWSCreatureStats::GetBaseAttackBonus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=2 exe=6a522e71631d

uint CSWSCreatureStats__GetBaseAttackBonus(void)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int in_ECX;
  byte local_3b;
  undefined1 local_38 [8];
  undefined4 local_30;
  char local_29;
  undefined1 local_28 [4];
  uint local_24;
  undefined1 local_1d;
  int local_1c;
  uint local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095bb38;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = 0;
  for (local_14 = 0; (int)local_14 < (int)(uint)*(byte *)(in_ECX + 0x8d); local_14 = local_14 + 1) {
```

## 0x006a7170 CSWSCreatureStats::GetAttacksPerRound

- Size 23 B, RET 0
- Callers (2 total):
  - 0x005905f0 CSWSCombatRound::CalculateOnHandAttacks (site 0x00590614)
  - 0x006a7190 CSWSCreatureStats::UpdateCombatInformation (site 0x006a777f)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns constant 1, i.e. one attack per round; no per-class or feat logic.
- Named pseudo-C (first 8 of 8 lines):
```c
// NAMED 0x006a7170 CSWSCreatureStats::GetAttacksPerRound evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=1 exe=6a522e71631d

undefined1 CSWSCreatureStats__GetAttacksPerRound(void)

{
  return 1;
}

```

## 0x006a7190 CSWSCreatureStats::UpdateCombatInformation

- Size 2990 B, RET 0
- Callers (7 total, first 5 shown):
  - 0x00569320 CSWSCreature::AIUpdate (site 0x0056a049)
  - 0x0056d6f0 CSWSCreature::EquipItem (site 0x0056d7bd)
  - 0x00632220 (site 0x006324d2)
  - 0x006ad400 CSWSCreatureStats::ModifyAlignment (site 0x006ad8ee)
  - 0x006b9640 CSWSCreatureStats::ValidateLevelUp (site 0x006b9773)
- Callees (68):
  - 0x006a71c3 CALL 0x0084a640
  - 0x006a7200 CALL 0x0069cbe0
  - 0x006a7213 CALL 0x009196fd operator delete
  - 0x006a7232 CALL 0x00919723 operator new
  - 0x006a724a CALL 0x0069cab0
  - 0x006a7292 CALL 0x00581920 CSWSCreature::GetFilteredEffectList
  - 0x006a72b7 CALL 0x0040e120
  - 0x006a72c3 CALL 0x0064ddb0
  - 0x006a730d CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a731f CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7331 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7343 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7356 CALL 0x00919723 operator new
  - 0x006a736e CALL 0x0069c990
  - 0x006a73b4 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a73c5 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a73d6 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a73f1 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a740c CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7427 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a744f CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7461 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7471 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a747f CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a74a3 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006a74c2 CALL 0x00581920 CSWSCreature::GetFilteredEffectList
  - 0x006a74e7 CALL 0x0040e120
  - 0x006a74f3 CALL 0x0064ddb0
  - 0x006a7549 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a755b CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a756d CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a757f CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7592 CALL 0x00919723 operator new
  - 0x006a75aa CALL 0x0069c990
  - 0x006a75f6 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7607 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7618 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7684 CALL 0x00581fc0
  - 0x006a7697 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a76b2 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a76cd CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a76e8 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7712 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7727 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7737 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a7745 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006a776f CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006a777f CALL 0x006a7170 CSWSCreatureStats::GetAttacksPerRound
  - 0x006a77d3 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a7804 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a783a CALL 0x006d5200
  - 0x006a78ae CALL 0x006d5200
  - 0x006a7978 CALL 0x006d5200
  - 0x006a79a5 CALL 0x006a8670
  - 0x006a79d7 CALL 0x006a8c20 CSWSCreatureStats::GetRangedDamageBonus
  - 0x006a7a13 CALL 0x006a7db0 CSWSCreatureStats::GetMeleeAttackBonus
  - 0x006a7a47 CALL 0x006a8370 CSWSCreatureStats::GetMeleeDamageBonus
  - 0x006a7af3 CALL 0x006a7db0 CSWSCreatureStats::GetMeleeAttackBonus
  - 0x006a7b27 CALL 0x006a8370 CSWSCreatureStats::GetMeleeDamageBonus
  - 0x006a7b59 CALL 0x006a7d40 CSWSCreatureStats::GetForceResistance
  - 0x006a7b6a CALL 0x006bad10 CSWSCreatureStats::GetForceResistance
  - 0x006a7be4 CALL 0x006a8f10 CSWSCreatureStats::GetWeaponSpecialization
  - 0x006a7c16 CALL 0x006a8fe0
  - 0x006a7c4a CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006a7c7e CALL 0x006afa60 CSWSCreatureStats::GetCriticalHitRoll
  - 0x006a7cb9 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006a7ced CALL 0x006afa60 CSWSCreatureStats::GetCriticalHitRoll
  - 0x006a7d2a CALL 0x0064ddb0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (3):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Rebuilds cached combat info: reallocates a cache object, then collects effect-derived values into arrays.
- Named pseudo-C (first 30 of 284 lines):
```c
// NAMED 0x006a7190 CSWSCreatureStats::UpdateCombatInformation evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=postpass exe=6a522e71631d

void CSWSCreatureStats__UpdateCombatInformation(void)

{
  byte bVar1;
  char cVar2;
  undefined1 uVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  int in_ECX;
  char *local_198;
  char *local_18c;
  undefined4 local_180;
  undefined1 local_64 [12];
  int local_58;
  char *local_54;
  undefined1 local_50 [12];
  int local_44;
  undefined4 local_40;
  int local_3c;
  int local_38;
  ushort local_34;
  int local_30;
  int local_2c;
  char *local_24;
  int local_20;
  int local_1c;
  int local_18;
```

## 0x006a7db0 CSWSCreatureStats::GetMeleeAttackBonus

- Size 1472 B, RET 16
- Callers (3 total):
  - 0x006710d0 Global::ExecuteCommandDoTouchAttack (site 0x00671216)
  - 0x006a7190 CSWSCreatureStats::UpdateCombatInformation (site 0x006a7a13)
  - 0x008ad930 Equipment panel slot refresh (per frame from 0x8ABA50 via 0x8ABA8C): top placeholder resets ctl0/ctl9, slot loop k=[EBP-0x28] 0..10, slot image ctl = this+0x14D0+k*0x148 (site 0x008ae0e7)
- Callees (34):
  - 0x006a7df6 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a7e1d CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a7e47 CALL 0x006d5200
  - 0x006a7ea4 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
  - 0x006a7eb2 CALL 0x006a6f80 CSWSCreatureStats::GetBaseAttackBonus
  - 0x006a7ed5 CALL 0x006b8da0 CSWSCreatureStats::GetWeaponFinesse
  - 0x006a7ee6 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006a7f2c CALL 0x006b8e80
  - 0x006a7f55 CALL 0x00615c80 CSWSItem::IsLightsaber
  - 0x006a7f69 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a7f86 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a7fa3 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a7ff7 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a801e CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a8055 CALL 0x006d5200
  - 0x006a808c CALL 0x0057e350
  - 0x006a80c2 CALL 0x006d5200
  - 0x006a80f9 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8115 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8132 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8151 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8170 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a81a0 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a81bc CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a81d8 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a81f5 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8214 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8230 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8278 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a82a9 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a82c7 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006a831a CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8334 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a834e CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range (5):
  - 0x006a7df6 GetMeleeAttackBonus left/off hand (mask 0x20): PUSH 0x20; GetItemInSlot(0x20) when param_1 != 0 and param_4 == 0
  - 0x006a7df6 mask 0x20 = left/off-hand weapon slot (nwscript INVENTORY_SLOT_LEFTWEAPON = 5, mask 1<<5)
  - 0x006a7e1d GetMeleeAttackBonus right hand (mask 0x10): ECX=[[this+0x24]+0x1150]; PUSH 0x10; GetItemInSlot(0x10)
  - 0x006a7e1d mask 0x10 = right-hand weapon slot (nwscript INVENTORY_SLOT_RIGHTWEAPON = 4, mask 1<<4)
  - 0x006a7e29 right-hand item == 0 skips the item path (unarmed case)
- Strings: none
- Globals (3):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes melee to-hit bonus: base attack, effect bonuses, finesse or Dex mod, and weapon feat bonuses.
- Named pseudo-C (first 30 of 191 lines):
```c
// NAMED 0x006a7db0 CSWSCreatureStats::GetMeleeAttackBonus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=8 exe=6a522e71631d

int CSWSCreatureStats__GetMeleeAttackBonus(int param_1,int param_2,int param_3,int param_4)

{
  char cVar1;
  int iVar2;
  int in_ECX;
  int local_34;
  undefined4 local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_20 = 0;
  local_18 = 0;
  if ((param_1 == 0) || (param_4 != 0)) {
    local_18 = CSWInventory__GetItemInSlot(0x10);
  }
  else {
    local_18 = CSWInventory__GetItemInSlot(0x20);
  }
  if ((local_18 == 0) ||
     (iVar2 = FUN_006d5200(*(undefined4 *)(local_18 + 0xc)), *(char *)(iVar2 + 0x1a) == '\0')) {
```

## 0x006a8370 CSWSCreatureStats::GetMeleeDamageBonus

- Size 753 B, RET 4
- Callers (1 total):
  - 0x006a7190 CSWSCreatureStats::UpdateCombatInformation (site 0x006a7a47)
- Callees (14):
  - 0x006a83a0 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a83cf CALL 0x006d5200
  - 0x006a840c CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a8435 CALL 0x006d5200
  - 0x006a8493 CALL 0x0091c860
  - 0x006a84e6 CALL 0x006d5200
  - 0x006a856e CALL 0x0091c860
  - 0x006a85ba CALL 0x006b8ee0 CSWSCreatureStats::GetWeaponSpecialization
  - 0x006a85db CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a85ec CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8608 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8619 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8635 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8646 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes melee damage bonus from strength, weapon specialization and feats, for main-hand or off-hand.
- Named pseudo-C (first 30 of 88 lines):
```c
// NAMED 0x006a8370 CSWSCreatureStats::GetMeleeDamageBonus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=4 exe=6a522e71631d

int CSWSCreatureStats__GetMeleeDamageBonus(int param_1)

{
  int iVar1;
  int in_ECX;
  int local_24;
  int local_20;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  if (param_1 == 0) {
    local_10 = CSWInventory__GetItemInSlot(0x10);
  }
  else {
    local_10 = CSWInventory__GetItemInSlot(0x20);
    if ((local_10 == 0) ||
       ((local_10 != 0 &&
        (iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc)), *(char *)(iVar1 + 8) != '\x03')))) {
      local_10 = 0;
    }
  }
  if ((local_10 == 0) ||
     (iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc)), *(char *)(iVar1 + 0x1a) == '\0')) {
    if (param_1 == 0) {
      local_20 = 0;
```

## 0x006a8c20 CSWSCreatureStats::GetRangedDamageBonus

- Size 625 B, RET 0
- Callers (1 total):
  - 0x006a7190 CSWSCreatureStats::UpdateCombatInformation (site 0x006a79d7)
- Callees (13):
  - 0x006a8c4a CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a8c73 CALL 0x006d5200
  - 0x006a8cc1 CALL 0x006d5200
  - 0x006a8d25 CALL 0x006042e0 CSWSItem::GetPropertyByType
  - 0x006a8d95 CALL 0x006b8ee0 CSWSCreatureStats::GetWeaponSpecialization
  - 0x006a8db6 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8dd2 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8dee CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8e08 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8e24 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8e40 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8e5c CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a8e78 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns ranged damage bonus for the main-hand ranged weapon: ability term, damage property (capped), and feat bonuses.
- Named pseudo-C (first 30 of 98 lines):
```c
// NAMED 0x006a8c20 CSWSCreatureStats::GetRangedDamageBonus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=5 exe=6a522e71631d

int CSWSCreatureStats__GetRangedDamageBonus(void)

{
  int iVar1;
  int in_ECX;
  uint local_4c;
  int local_18;
  uint local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = 0;
  local_10 = CSWInventory__GetItemInSlot(0x10);
  if ((local_10 == 0) ||
     (iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc)), *(char *)(iVar1 + 0x1a) != '\0')) {
    if (local_10 != 0) {
      local_14 = 0;
      iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc));
      if (*(char *)(iVar1 + 8) == '\a') {
        local_14 = (uint)*(char *)(in_ECX + 0xee);
      }
      else {
        if (*(char *)(in_ECX + 0xee) < '\0') {
          local_14 = (uint)*(char *)(in_ECX + 0xee);
        }
        iVar1 = CSWSItem__GetPropertyByType(&local_18,0x1e,0);
        if (iVar1 != 0) {
```

## 0x006a8ea0 CSWSCreatureStats::GetUseMonkAttackTables

- Size 110 B, RET 4
- Callers (3 total):
  - 0x0062f070 (site 0x0062f4c4)
  - 0x00646270 (site 0x006465d5)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006adc5f)
- Callees (2):
  - 0x006a8eb5 CALL 0x005766f0 CSWSCreature::GetUseMonkAbilities
  - 0x006a8edc CALL 0x006d0620 CSWInventory::GetItemInSlot
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns true when monk abilities apply and the main-hand slot is empty; the flag argument does not change this.
- Named pseudo-C (first 22 of 22 lines):
```c
// NAMED 0x006a8ea0 CSWSCreatureStats::GetUseMonkAttackTables evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=3 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetUseMonkAttackTables(int param_1)

{
  int iVar1;
  
  iVar1 = CSWSCreature__GetUseMonkAbilities();
  if (iVar1 != 0) {
    iVar1 = CSWInventory__GetItemInSlot(0x10);
    if (param_1 == 0) {
      if (iVar1 == 0) {
        return 1;
      }
    }
    else if (iVar1 == 0) {
      return 1;
    }
  }
  return 0;
}

```

## 0x006a9100 CSWSCreatureStats::GetUnarmedDamageRoll

- Size 667 B, RET 4
- Callers (1 total):
  - 0x006adec0 CSWSCreatureStats::GetDamageRoll (site 0x006aeee3)
- Callees (13):
  - 0x006a913b CALL 0x006a8fe0
  - 0x006a914c CALL 0x006a8f10 CSWSCreatureStats::GetWeaponSpecialization
  - 0x006a9171 CALL 0x006a2000
  - 0x006a919f indirect
  - 0x006a91c6 indirect
  - 0x006a91f4 CALL 0x0051c830 CServerExoApp::GetDifficultyOption
  - 0x006a921b indirect
  - 0x006a9249 CALL 0x0051c830 CServerExoApp::GetDifficultyOption
  - 0x006a92b9 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006a92d4 CALL 0x0071b670
  - 0x006a92e3 CALL 0x00733780 CExoString::Destructor
  - 0x006a9356 CALL 0x0091c860
  - 0x006a937f CALL 0x006a22c0
- Imports: none
- Indirect slots (3):
  - 0x006a919f slot +0x30 (006a919c MOV EAX,dword ptr [EDX + 0x30])
  - 0x006a91c6 slot +0x30 (006a91c3 MOV EAX,dword ptr [EDX + 0x30])
  - 0x006a921b slot +0x30 (006a9218 MOV EAX,dword ptr [EDX + 0x30])
- Ledger callsites in range: none
- Strings (1):
  - 0x009993f8 "DamageMult"
- Globals (2):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes unarmed damage roll from the monk die-size ladder and weapon specialization, scaled by difficulty and a DamageMult setting.
- Named pseudo-C (first 30 of 85 lines):
```c
// NAMED 0x006a9100 CSWSCreatureStats::GetUnarmedDamageRoll evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=4 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetUnarmedDamageRoll(int *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int in_ECX;
  undefined1 local_44 [8];
  undefined4 local_3c;
  char local_35;
  undefined1 local_34 [4];
  uint local_30;
  uint local_2c;
  uint local_28;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095bbc8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
```

## 0x006a93a0 CSWSCreatureStats::GetAttackModifierVersus

- Size 11176 B, RET 4
- Callers (2 total):
  - 0x00580900 CSWSCreature::EquipMostDamagingRangedWeapon (site 0x00580a35)
  - 0x006e0b10 CSWSCreature::ResolveAttackRoll (site 0x006e0c12)
- Callees (246):
  - 0x006a9447 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a94a1 CALL 0x00591e10 CSWSCombatRound::GetCurrentAttackWeapon
  - 0x006a94e0 CALL 0x006d5200
  - 0x006a9503 CALL 0x006a6f80 CSWSCreatureStats::GetBaseAttackBonus
  - 0x006a9556 CALL 0x006a6f80 CSWSCreatureStats::GetBaseAttackBonus
  - 0x006a9618 CALL 0x006a6f80 CSWSCreatureStats::GetBaseAttackBonus
  - 0x006a96ca CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a96d9 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006a96eb CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a9707 CALL 0x00734270 CExoString::Format
  - 0x006a9723 CALL 0x00733df0 CExoString::operator+
  - 0x006a9770 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a977a CALL 0x007337c0 CExoString::operator=
  - 0x006a9789 CALL 0x00733780 CExoString::Destructor
  - 0x006a9798 CALL 0x00733780 CExoString::Destructor
  - 0x006a97aa CALL 0x00733780 CExoString::Destructor
  - 0x006a980f CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a984a CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a9880 CALL 0x006d5200
  - 0x006a98ff CALL 0x006d5200
  - 0x006a9951 CALL 0x0057e350
  - 0x006a9987 CALL 0x006d5200
  - 0x006a99c5 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a99ec CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9a14 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9a43 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9a72 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9ae7 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9b0e CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9b35 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9b5d CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9b8c CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9bb8 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9c64 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a9c73 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006a9c85 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a9c9e CALL 0x00734270 CExoString::Format
  - 0x006a9cba CALL 0x00733df0 CExoString::operator+
  - 0x006a9d07 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a9d11 CALL 0x007337c0 CExoString::operator=
  - 0x006a9d20 CALL 0x00733780 CExoString::Destructor
  - 0x006a9d2f CALL 0x00733780 CExoString::Destructor
  - 0x006a9d41 CALL 0x00733780 CExoString::Destructor
  - 0x006a9d50 CALL 0x006acec0 CSWSCreatureStats::ResolveSpecialAttackAttackBonus
  - 0x006a9daf CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a9dbe CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006a9dd0 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006a9de9 CALL 0x00734270 CExoString::Format
  - 0x006a9e05 CALL 0x00733df0 CExoString::operator+
  - 0x006a9e52 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006a9e5c CALL 0x007337c0 CExoString::operator=
  - 0x006a9e6b CALL 0x00733780 CExoString::Destructor
  - 0x006a9e7a CALL 0x00733780 CExoString::Destructor
  - 0x006a9e8c CALL 0x00733780 CExoString::Destructor
  - 0x006a9ecb CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a9efc CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006a9f20 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006a9f88 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9fac CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006a9fd0 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa053 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa062 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006aa074 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aa090 CALL 0x00734270 CExoString::Format
  - 0x006aa0ac CALL 0x00733df0 CExoString::operator+
  - 0x006aa0f9 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa103 CALL 0x007337c0 CExoString::operator=
  - 0x006aa112 CALL 0x00733780 CExoString::Destructor
  - 0x006aa121 CALL 0x00733780 CExoString::Destructor
  - 0x006aa133 CALL 0x00733780 CExoString::Destructor
  - 0x006aa183 CALL 0x004452b0
  - 0x006aa20c CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa24a CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa2b3 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa2c2 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006aa2d4 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aa2ed CALL 0x00734270 CExoString::Format
  - 0x006aa309 CALL 0x00733df0 CExoString::operator+
  - 0x006aa356 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa360 CALL 0x007337c0 CExoString::operator=
  - 0x006aa36f CALL 0x00733780 CExoString::Destructor
  - 0x006aa37e CALL 0x00733780 CExoString::Destructor
  - 0x006aa390 CALL 0x00733780 CExoString::Destructor
  - 0x006aa3ca CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006aa3e8 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006aa429 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa455 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa4be CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa4cd CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006aa4df CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aa4f8 CALL 0x00734270 CExoString::Format
  - 0x006aa514 CALL 0x00733df0 CExoString::operator+
  - 0x006aa561 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa56b CALL 0x007337c0 CExoString::operator=
  - 0x006aa57a CALL 0x00733780 CExoString::Destructor
  - 0x006aa589 CALL 0x00733780 CExoString::Destructor
  - 0x006aa59b CALL 0x00733780 CExoString::Destructor
  - 0x006aa5df CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa5f3 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa607 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa629 CALL 0x0073fb90 CClientExoApp::GetSWParty
  - 0x006aa684 CALL 0x0073fb90 CClientExoApp::GetSWParty
  - 0x006aa6e0 CALL 0x0051c8b0 CServerExoApp::ClientToServerObjectId
  - 0x006aa738 CALL 0x0051c100 CServerExoApp::GetCreatureByGameObjectID
  - 0x006aa791 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa7b4 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa7d7 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aa830 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006aa850 CALL 0x006b8da0 CSWSCreatureStats::GetWeaponFinesse
  - 0x006aa862 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006aa8d5 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa8fd CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa943 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa952 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006aa964 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aa987 CALL 0x00734270 CExoString::Format
  - 0x006aa9a3 CALL 0x00733df0 CExoString::operator+
  - 0x006aa9f0 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aa9fa CALL 0x007337c0 CExoString::operator=
  - 0x006aaa09 CALL 0x00733780 CExoString::Destructor
  - 0x006aaa39 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aaa48 CALL 0x007337c0 CExoString::operator=
  - 0x006aaa67 CALL 0x00734270 CExoString::Format
  - 0x006aaa83 CALL 0x00733df0 CExoString::operator+
  - 0x006aaad0 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aaada CALL 0x007337c0 CExoString::operator=
  - 0x006aaae9 CALL 0x00733780 CExoString::Destructor
  - 0x006aab19 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aab28 CALL 0x007337c0 CExoString::operator=
  - 0x006aab37 CALL 0x00733780 CExoString::Destructor
  - 0x006aab49 CALL 0x00733780 CExoString::Destructor
  - 0x006aab68 CALL 0x006b8e80
  - 0x006aab9d CALL 0x00615c80 CSWSItem::IsLightsaber
  - 0x006aabb1 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aabce CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aabeb CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aac40 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aac67 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aac8e CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aacb5 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aacdc CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aad03 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aad2a CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aad4e CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aad6f CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aad90 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aae1a CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aae29 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006aae3b CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aae57 CALL 0x00734270 CExoString::Format
  - 0x006aae73 CALL 0x00733df0 CExoString::operator+
  - 0x006aaec0 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aaeca CALL 0x007337c0 CExoString::operator=
  - 0x006aaed9 CALL 0x00733780 CExoString::Destructor
  - 0x006aaee8 CALL 0x00733780 CExoString::Destructor
  - 0x006aaefa CALL 0x00733780 CExoString::Destructor
  - 0x006aaf41 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aaf50 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006aaf62 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aaf7b CALL 0x00734270 CExoString::Format
  - 0x006aaf97 CALL 0x00733df0 CExoString::operator+
  - 0x006aafe4 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aafee CALL 0x007337c0 CExoString::operator=
  - 0x006aaffd CALL 0x00733780 CExoString::Destructor
  - 0x006ab00c CALL 0x00733780 CExoString::Destructor
  - 0x006ab01e CALL 0x00733780 CExoString::Destructor
  - 0x006ab0a3 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006ab10c CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006ab1a3 CALL 0x0051c8d0
  - 0x006ab1aa CALL 0x005fa960 CSWPartyTable::GetIsNPCAvailable
  - 0x006ab1d3 CALL 0x0051c8d0
  - 0x006ab1da CALL 0x005fafd0 CSWPartyTable::GetIsMember
  - 0x006ab207 CALL 0x0051c8d0
  - 0x006ab20e CALL 0x005faaf0 CSWPartyTable::GetNPCObject
  - 0x006ab246 CALL 0x0051c100 CServerExoApp::GetCreatureByGameObjectID
  - 0x006ab27f CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006ab2f0 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006ab3bb CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ab3ca CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006ab3dc CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ab3f5 CALL 0x00734270 CExoString::Format
  - 0x006ab411 CALL 0x00733df0 CExoString::operator+
  - 0x006ab45e CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ab468 CALL 0x007337c0 CExoString::operator=
  - 0x006ab477 CALL 0x00733780 CExoString::Destructor
  - 0x006ab486 CALL 0x00733780 CExoString::Destructor
  - 0x006ab498 CALL 0x00733780 CExoString::Destructor
  - 0x006ab504 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006ab54c CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006ab5d0 CALL 0x0051c8d0
  - 0x006ab5d7 CALL 0x005fa960 CSWPartyTable::GetIsNPCAvailable
  - 0x006ab5ff CALL 0x0051c8d0
  - 0x006ab606 CALL 0x005fafd0 CSWPartyTable::GetIsMember
  - 0x006ab632 CALL 0x0051c8d0
  - 0x006ab639 CALL 0x005faaf0 CSWPartyTable::GetNPCObject
  - 0x006ab670 CALL 0x0051c100 CServerExoApp::GetCreatureByGameObjectID
  - 0x006ab6a9 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006ab717 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006ab7d2 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ab7e1 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006ab7f3 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ab80c CALL 0x00734270 CExoString::Format
  - 0x006ab828 CALL 0x00733df0 CExoString::operator+
  - 0x006ab875 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ab87f CALL 0x007337c0 CExoString::operator=
  - 0x006ab88e CALL 0x00733780 CExoString::Destructor
  - 0x006ab89d CALL 0x00733780 CExoString::Destructor
  - 0x006ab8af CALL 0x00733780 CExoString::Destructor
  - 0x006ab8d7 CALL 0x005763e0 CSWSCreature::IsWieldingLightsaber
  - 0x006ab952 CALL 0x005763e0 CSWSCreature::IsWieldingLightsaber
  - 0x006ab9f6 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aba05 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006aba17 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aba30 CALL 0x00734270 CExoString::Format
  - 0x006aba4c CALL 0x00733df0 CExoString::operator+
  - 0x006aba99 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006abaa3 CALL 0x007337c0 CExoString::operator=
  - 0x006abab2 CALL 0x00733780 CExoString::Destructor
  - 0x006abac1 CALL 0x00733780 CExoString::Destructor
  - 0x006abad3 CALL 0x00733780 CExoString::Destructor
  - 0x006abb29 indirect
  - 0x006abb34 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
  - 0x006abb77 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
  - 0x006abbe2 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006abbf1 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006abc03 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006abc1f CALL 0x00734270 CExoString::Format
  - 0x006abc3b CALL 0x00733df0 CExoString::operator+
  - 0x006abc88 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006abc92 CALL 0x007337c0 CExoString::operator=
  - 0x006abca1 CALL 0x00733780 CExoString::Destructor
  - 0x006abcb0 CALL 0x00733780 CExoString::Destructor
  - 0x006abcc2 CALL 0x00733780 CExoString::Destructor
  - 0x006abd2d CALL 0x0073fb90 CClientExoApp::GetSWParty
  - 0x006abd86 CALL 0x007e5da0 Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  - 0x006abd8d CALL 0x0077d800
  - 0x006abe56 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006abe65 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006abe77 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006abe90 CALL 0x00734270 CExoString::Format
  - 0x006abeac CALL 0x00733df0 CExoString::operator+
  - 0x006abef9 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006abf03 CALL 0x007337c0 CExoString::operator=
  - 0x006abf12 CALL 0x00733780 CExoString::Destructor
  - 0x006abf21 CALL 0x00733780 CExoString::Destructor
  - 0x006abf33 CALL 0x00733780 CExoString::Destructor
- Imports: none
- Indirect slots (1):
  - 0x006abb29 slot +0x10 (006abb26 MOV EAX,dword ptr [EDX + 0x10])
- Ledger callsites in range: none
- Strings (15):
  - 0x009992a8 " + %d"
  - 0x00999404 " + %d (PC Charisma Attack Bonus)"
  - 0x00999428 " + %d (Effect Attack Bonus)"
  - 0x00999444 " + %d (Inspire Followers Bonus)"
  - 0x00999464 " + %d (Crush Opposition Penalty)"
  - 0x00999488 " + %d (Targeting Bonus)"
  - 0x009994a0 " + %d (Weapon Focus Bonus)"
  - 0x009994bc " + %d (Str Modifier)"
  - 0x009994d4 " + %d (Dex Modifier)"
  - 0x009994ec " + %d (Offensive Training Bonus (vs Race))"
  - 0x00999518 " + %d (Good Aim)"
  - 0x0099952c " + %d (Dueling To Hit Modifier)"
  - 0x0099954c " + %d (Special Attack To Hit Modifier)"
  - 0x00999574 " + %d (Offhand Attack Penalty)"
  - 0x00999594 " + %d (Base Attack Bonus)"
- Globals (4):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4a4 READ (MOV ECX,dword ptr [0x00a1b4a4])
  - 0x00a1b4a4 READ (MOV EDX,dword ptr [0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched (1):
  - 0xae8 CSWSCombatRound::OffHandTaken (int) R
- KDB purpose: Computes attack modifier versus a target (base attack, ability, feats, effects) with a breakdown string.
- Named pseudo-C (first 30 of 918 lines):
```c
// NAMED 0x006a93a0 CSWSCreatureStats::GetAttackModifierVersus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=postpass exe=6a522e71631d

int CSWSCreatureStats__GetAttackModifierVersus(int *param_1)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  int *piVar5;
  int in_ECX;
  undefined4 uVar6;
  undefined1 local_26c [8];
  undefined1 local_264 [8];
  undefined1 local_25c [8];
  undefined1 local_254 [8];
  undefined1 local_24c [8];
  undefined1 local_244 [8];
  undefined1 local_23c [8];
  undefined1 local_234 [8];
  undefined1 local_22c [8];
  undefined1 local_224 [8];
  undefined1 local_21c [8];
  undefined1 local_214 [8];
  undefined1 local_20c [8];
  undefined1 local_204 [8];
  undefined1 local_1fc [8];
  undefined1 local_1f4 [16];
  int local_1e4;
  int local_1e0;
```

## 0x006abf70 CSWSCreatureStats::GetDamageBonus

- Size 3908 B, RET 12
- Callers (1 total):
  - 0x006adec0 CSWSCreatureStats::GetDamageRoll (site 0x006af379)
- Callees (103):
  - 0x006ac00d CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac065 CALL 0x00591e10 CSWSCombatRound::GetCurrentAttackWeapon
  - 0x006ac0c6 CALL 0x0091c860
  - 0x006ac130 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac13e CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006ac14d CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ac166 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac176 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac189 CALL 0x00734270 CExoString::Format
  - 0x006ac1a0 CALL 0x00734270 CExoString::Format
  - 0x006ac1b6 CALL 0x00733df0 CExoString::operator+
  - 0x006ac203 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac20f CALL 0x007337c0 CExoString::operator=
  - 0x006ac21e CALL 0x00733780 CExoString::Destructor
  - 0x006ac22a CALL 0x00733780 CExoString::Destructor
  - 0x006ac239 CALL 0x00733780 CExoString::Destructor
  - 0x006ac27a CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006ac28b CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006ac2c3 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006ac377 CALL 0x0091c860
  - 0x006ac3f1 CALL 0x006bada0
  - 0x006ac444 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac452 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006ac461 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ac47a CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac48a CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac49d CALL 0x00734270 CExoString::Format
  - 0x006ac4b4 CALL 0x00734270 CExoString::Format
  - 0x006ac4ca CALL 0x00733df0 CExoString::operator+
  - 0x006ac517 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac523 CALL 0x007337c0 CExoString::operator=
  - 0x006ac532 CALL 0x00733780 CExoString::Destructor
  - 0x006ac53e CALL 0x00733780 CExoString::Destructor
  - 0x006ac54d CALL 0x00733780 CExoString::Destructor
  - 0x006ac60c CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac61a CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006ac62c CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ac645 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac655 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac66b CALL 0x00734270 CExoString::Format
  - 0x006ac685 CALL 0x00734270 CExoString::Format
  - 0x006ac69e CALL 0x00733df0 CExoString::operator+
  - 0x006ac6eb CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac6f7 CALL 0x007337c0 CExoString::operator=
  - 0x006ac706 CALL 0x00733780 CExoString::Destructor
  - 0x006ac715 CALL 0x00733780 CExoString::Destructor
  - 0x006ac724 CALL 0x00733780 CExoString::Destructor
  - 0x006ac73a CALL 0x006b8ee0 CSWSCreatureStats::GetWeaponSpecialization
  - 0x006ac798 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac7a9 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006ac7bb CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ac7d4 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac7e4 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac7fa CALL 0x00734270 CExoString::Format
  - 0x006ac814 CALL 0x00734270 CExoString::Format
  - 0x006ac830 CALL 0x00733df0 CExoString::operator+
  - 0x006ac87d CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac889 CALL 0x007337c0 CExoString::operator=
  - 0x006ac898 CALL 0x00733780 CExoString::Destructor
  - 0x006ac8a7 CALL 0x00733780 CExoString::Destructor
  - 0x006ac8b9 CALL 0x00733780 CExoString::Destructor
  - 0x006ac8c8 CALL 0x006ad1a0 CSWSCreatureStats::GetFavoredEnemyBonus
  - 0x006ac91b CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006ac92c CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006ac93e CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ac957 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac967 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ac97d CALL 0x00734270 CExoString::Format
  - 0x006ac997 CALL 0x00734270 CExoString::Format
  - 0x006ac9b3 CALL 0x00733df0 CExoString::operator+
  - 0x006aca00 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aca0c CALL 0x007337c0 CExoString::operator=
  - 0x006aca1b CALL 0x00733780 CExoString::Destructor
  - 0x006aca2a CALL 0x00733780 CExoString::Destructor
  - 0x006aca3c CALL 0x00733780 CExoString::Destructor
  - 0x006aca59 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aca78 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006aca97 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acab4 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acad3 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acaf2 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acb11 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acb30 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acb52 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acb66 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acb85 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acb99 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acbb8 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acbcc CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acc55 CALL 0x005763e0 CSWSCreature::IsWieldingLightsaber
  - 0x006accd8 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006accf8 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acd18 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acd4a CALL 0x006a2000
  - 0x006acd77 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acd9a CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acdbd CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006acde0 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006ace00 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006ace20 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006ace40 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006ace60 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006ace92 CALL 0x006a2000
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings (10):
  - 0x009995b0 " + %d (Special Attack Damage Bonus)"
  - 0x009995d4 " + %d (Special Attack Damage Bonus) (Critical x%d)"
  - 0x00999608 " + %d (Weapon Specialization Bonus)"
  - 0x0099962c " + %d (Weapon Specialization Bonus) (Critical x%d)"
  - 0x00999660 " + %d (Power/Improved Power Attack Damage Bonus)"
  - 0x00999698 " + %d (Power/Improved Power Attack Damage Bonus) (Critical x%d)"
  - 0x009996d8 " + %d (Favored Enemy Bonus Damage)"
  - 0x009996fc " + %d (Favored Enemy Bonus Damage) (Critical x%d)"
  - 0x00999730 " + %d (Offhand Strength Bonus)"
  - 0x00999750 " + %d (Offhand Strength Bonus) (Critical x%d)"
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes total damage bonus: specialization, power attack, favored enemy, offhand, special-attack terms, with breakdown text.
- Named pseudo-C (first 30 of 401 lines):
```c
// NAMED 0x006abf70 CSWSCreatureStats::GetDamageBonus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=14 exe=6a522e71631d

uint CSWSCreatureStats__GetDamageBonus(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int in_ECX;
  undefined1 local_dc [8];
  undefined1 local_d4 [8];
  undefined1 local_cc [8];
  undefined1 local_c4 [8];
  undefined1 local_bc [8];
  uint local_b4;
  uint local_b0;
  undefined1 local_ac [16];
  undefined1 local_9c [16];
  undefined1 local_8c [16];
  undefined1 local_7c [16];
  int local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_5c;
  undefined1 local_58 [16];
  int local_48;
  uint local_44;
  int local_40;
  int local_3c;
```

## 0x006acec0 CSWSCreatureStats::ResolveSpecialAttackAttackBonus

- Size 562 B, RET 4
- Callers (1 total):
  - 0x006a93a0 CSWSCreatureStats::GetAttackModifierVersus (site 0x006a9d50)
- Callees (5):
  - 0x006acef8 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006acfbb CALL 0x00591e10 CSWSCombatRound::GetCurrentAttackWeapon
  - 0x006ad00a CALL 0x00591e10 CSWSCombatRound::GetCurrentAttackWeapon
  - 0x006ad059 CALL 0x00591e10 CSWSCombatRound::GetCurrentAttackWeapon
  - 0x006ad0a5 CALL 0x00591e10 CSWSCombatRound::GetCurrentAttackWeapon
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns the to-hit modifier for the special attack id, via a switch with weapon-dependent cases.
- Named pseudo-C (first 30 of 81 lines):
```c
// NAMED 0x006acec0 CSWSCreatureStats::ResolveSpecialAttackAttackBonus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=3 exe=6a522e71631d

undefined4 CSWSCreatureStats__ResolveSpecialAttackAttackBonus(void)

{
  int iVar1;
  int in_ECX;
  undefined4 local_8;
  
  iVar1 = CSWSCombatRound__GetAttack
                    (*(undefined1 *)(*(int *)(*(int *)(in_ECX + 0x24) + 0x10dc) + 0xabc));
  local_8 = 0;
  switch(*(undefined2 *)(iVar1 + 100)) {
  case 0xb:
    local_8 = 0xfffffffc;
    break;
  case 0x11:
    local_8 = 0xfffffffd;
    break;
  case 0x12:
    local_8 = 0xfffffffd;
    break;
  case 0x14:
  case 0x1f:
  case 0x4d:
    iVar1 = CSWSCombatRound__GetCurrentAttackWeapon(0);
    if ((iVar1 != 0) && (*(int *)(iVar1 + 0x2e8) != 0)) {
      local_8 = 4;
    }
    break;
```

## 0x006ad1a0 CSWSCreatureStats::GetFavoredEnemyBonus

- Size 201 B, RET 4
- Callers (1 total):
  - 0x006abf70 CSWSCreatureStats::GetDamageBonus (site 0x006ac8c8)
- Callees (1):
  - 0x006ad1d8 CALL 0x005908c0 CSWSCombatRound::GetAttack
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns favored-enemy bonus for the current attack id via a switch; values range 0 to 12.
- Named pseudo-C (first 30 of 43 lines):
```c
// NAMED 0x006ad1a0 CSWSCreatureStats::GetFavoredEnemyBonus evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetFavoredEnemyBonus(void)

{
  int iVar1;
  int in_ECX;
  undefined4 local_8;
  
  iVar1 = CSWSCombatRound__GetAttack
                    (*(undefined1 *)(*(int *)(*(int *)(in_ECX + 0x24) + 0x10dc) + 0xabc));
  local_8 = 0;
  switch(*(undefined2 *)(iVar1 + 100)) {
  case 0x11:
    local_8 = 7;
    break;
  case 0x12:
    local_8 = 7;
    break;
  case 0x1c:
    local_8 = 3;
    break;
  case 0x1d:
    local_8 = 3;
    break;
  case 0x52:
    local_8 = 0xc;
    break;
  case 0x53:
    local_8 = 0xc;
```

## 0x006ad2f0 CSWSCreatureStats::GetClassLevel

- Size 122 B, RET 8
- Callers (30 total, first 5 shown):
  - 0x0056ae40 CSWSCreature::HandleAlignmentInfluence (site 0x0056b2b9)
  - 0x0057e530 CSWSCreature::GetArmorClass (site 0x0057e60a)
  - 0x005a65d0 CSWSEffectListHandler::OnApplyNegativeLevel (site 0x005a69fd)
  - 0x005b8000 CSWSCreature::AIActionItemCastSpell (site 0x005b9132)
  - 0x0062ee30 CSWSAreaOfEffectObject::SetCreator (site 0x0062ef0f)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns class level for a class index; when flag set, subtracts negative levels, floored at zero.
- Named pseudo-C (first 29 of 29 lines):
```c
// NAMED 0x006ad2f0 CSWSCreatureStats::GetClassLevel evidence=ev_39d7ea60e5bfdea3d141f6337b9eebf1348213f1e662e35d38dea454b02b3316 names=1 exe=6a522e71631d

int CSWSCreatureStats__GetClassLevel(byte param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  int in_ECX;
  undefined4 local_8;
  
  if (param_1 < *(byte *)(in_ECX + 0x8d)) {
    uVar1 = (uint)*(byte *)(in_ECX + 0xac + (uint)param_1 * 0x28);
    local_8 = 0;
    if (param_2 != 0) {
      local_8 = (uint)*(byte *)(in_ECX + 0xad + (uint)param_1 * 0x28);
    }
    if ((int)(uVar1 - local_8) < 0) {
      iVar2 = 0;
    }
    else {
      iVar2 = uVar1 - local_8;
    }
  }
  else {
    iVar2 = 0;
  }
  return iVar2;
}

```

## 0x006ad370 CSWSCreatureStats::GetClassNegativeLevels

- Size 60 B, RET 4
- Callers (2 total):
  - 0x005a65d0 CSWSEffectListHandler::OnApplyNegativeLevel (site 0x005a6a60)
  - 0x005a6c60 CSWSEffectListHandler::OnRemoveNegativeLevel (site 0x005a6cab)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns the negative-level count for a class index (0 if index out of range).
- Named pseudo-C (first 18 of 18 lines):
```c
// NAMED 0x006ad370 CSWSCreatureStats::GetClassNegativeLevels evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetClassNegativeLevels(byte param_1)

{
  undefined4 uVar1;
  int in_ECX;
  
  if (param_1 < *(byte *)(in_ECX + 0x8d)) {
    uVar1 = CONCAT31((int3)((uint)param_1 * 0x28 >> 8),
                     *(undefined1 *)(in_ECX + 0xad + (uint)param_1 * 0x28));
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

```

## 0x006ad3b0 CSWSCreatureStats::GetLevelStats

- Size 69 B, RET 4
- Callers (5 total):
  - 0x0057e8c0 CSWSCreature::GetMaxHitPoints (site 0x0057e980)
  - 0x0057eca0 CSWSCreature::GetMaxForcePoints (site 0x0057ee23)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b2458)
  - 0x006b4ea0 CSWSCreatureStats::SaveClassInfo (site 0x006b5235)
  - 0x006b6bd0 CSWSCreatureStats::SetExperience (site 0x006b6e12)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns the per-level stats entry at an index; 0 for non-player creatures or out-of-range index.
- Named pseudo-C (first 20 of 20 lines):
```c
// NAMED 0x006ad3b0 CSWSCreatureStats::GetLevelStats evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetLevelStats(byte param_1)

{
  undefined4 uVar1;
  int in_ECX;
  
  if (*(int *)(in_ECX + 0x6c) == 0) {
    uVar1 = 0;
  }
  else if ((int)(uint)param_1 < *(int *)(in_ECX + 0x2c)) {
    uVar1 = *(undefined4 *)(*(int *)(in_ECX + 0x28) + (uint)param_1 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

```

## 0x006ad400 CSWSCreatureStats::ModifyAlignment

- Size 1284 B, RET 8
- Callers (3 total):
  - 0x0066cb50 Global::ExecuteCommandAdjustAlignment (site 0x0066d07d)
  - 0x007ab8e0 (site 0x007ab98e)
  - 0x007ab9a0 (site 0x007aba5a)
- Callees (15):
  - 0x006ad559 CALL 0x004010a0
  - 0x006ad56d CALL 0x004010a0
  - 0x006ad57f CALL 0x004010a0
  - 0x006ad593 CALL 0x004010a0
  - 0x006ad5a9 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006ad5cc CALL 0x0051c890 CServerExoApp::GetGlobalVariableTable
  - 0x006ad5d3 CALL 0x00654cc0 CSWGlobalVariableTable::SetValueNumber
  - 0x006ad5e2 CALL 0x00733780 CExoString::Destructor
  - 0x006ad61c CALL 0x0051c8d0
  - 0x006ad623 CALL 0x005faaf0 CSWPartyTable::GetNPCObject
  - 0x006ad649 CALL 0x0051c100 CServerExoApp::GetCreatureByGameObjectID
  - 0x006ad65a CALL 0x0056ae40 CSWSCreature::HandleAlignmentInfluence
  - 0x006ad7aa CALL 0x0056ae40 CSWSCreature::HandleAlignmentInfluence
  - 0x006ad8e6 CALL 0x0058b7f0
  - 0x006ad8ee CALL 0x006a7190 CSWSCreatureStats::UpdateCombatInformation
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4a4 READ (MOV EDX,dword ptr [0x00a1b4a4])
- Struct fields touched: none
- KDB purpose: Adjusts alignment by a delta, clamped to 0-100, updates party NPC influence and global variables, and refreshes combat info.
- Named pseudo-C (first 30 of 168 lines):
```c
// NAMED 0x006ad400 CSWSCreatureStats::ModifyAlignment evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=7 exe=6a522e71631d

void CSWSCreatureStats__ModifyAlignment(short param_1,int param_2)

{
  short sVar1;
  uint uVar2;
  int in_ECX;
  int iVar3;
  undefined1 *puVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_a4;
  int local_90;
  short local_7c;
  int local_78;
  short local_68;
  short local_58;
  undefined1 local_28 [8];
  int local_20;
  int local_1c;
  int local_18;
  short local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095beb8;
  local_10 = ExceptionList;
```

## 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject

- Size 1453 B, RET 12
- Callers (1 total):
  - 0x00646270 (site 0x00646d21)
- Callees (26):
  - 0x006ad950 CALL 0x006b5990 CSWSCreatureStats::GetSTRStat
  - 0x006ad967 CALL 0x006b5d00
  - 0x006ad97e CALL 0x006b5a40 CSWSCreatureStats::GetCONStat
  - 0x006ad995 CALL 0x006b5af0 CSWSCreatureStats::GetINTStat
  - 0x006ad9ac CALL 0x006b5ba0 CSWSCreatureStats::GetWISStat
  - 0x006ad9c3 CALL 0x006b5c50 CSWSCreatureStats::GetCHAStat
  - 0x006ada6a CALL 0x006ba9f0 CSWSCreatureStats::GetTotalSTRBonus
  - 0x006ada81 CALL 0x006baa40 CSWSCreatureStats::GetTotalDEXBonus
  - 0x006ada98 CALL 0x006baa90 CSWSCreatureStats::GetTotalCONBonus
  - 0x006adaaf CALL 0x006baae0 CSWSCreatureStats::GetTotalINTBonus
  - 0x006adac6 CALL 0x006bab30 CSWSCreatureStats::GetTotalWISBonus
  - 0x006adadd CALL 0x006bab80 CSWSCreatureStats::GetTotalCHABonus
  - 0x006adaf6 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006adb06 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006adb4a CALL 0x006ba620 CSWSCreatureStats::GetFortSavingThrow
  - 0x006adb89 CALL 0x006ba6b0 CSWSCreatureStats::GetWillSavingThrow
  - 0x006adbc8 CALL 0x006ba740 CSWSCreatureStats::GetReflexSavingThrow
  - 0x006adc40 CALL 0x006a6f80 CSWSCreatureStats::GetBaseAttackBonus
  - 0x006adc5f CALL 0x006a8ea0 CSWSCreatureStats::GetUseMonkAttackTables
  - 0x006adcb8 CALL 0x006f5810
  - 0x006adcfb CALL 0x0057e530 CSWSCreature::GetArmorClass
  - 0x006add7b CALL 0x006b7bb0 CSWSCreatureStats::GetSkillRank
  - 0x006ade3c CALL 0x006bc430 CSWSCreatureStats::GetFeat_2
  - 0x006ade4d CALL 0x00593450 CExoArrayList<uint16>::Add
  - 0x006ade9b CALL 0x006bc530 CSWSCreatureStats::GetBonusFeat
  - 0x006adeaf CALL 0x00593450 CExoArrayList<uint16>::Add
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Writes stats snapshot (abilities, bonuses, saves, BAB, AC, skills, feats) into an output object.
- Named pseudo-C (first 30 of 103 lines):
```c
// NAMED 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=23 exe=6a522e71631d

void CSWSCreatureStats__UpdateLastStatsObject(uint param_1,undefined4 param_2,int param_3)

{
  undefined1 uVar1;
  char cVar2;
  char cVar3;
  undefined2 uVar4;
  undefined4 uVar5;
  int in_ECX;
  undefined2 local_a8;
  undefined2 local_a4;
  undefined2 local_10;
  undefined1 local_5;
  
  if ((param_1 & 1) != 0) {
    uVar1 = CSWSCreatureStats__GetSTRStat();
    *(undefined1 *)(param_3 + 0x2c) = uVar1;
    uVar1 = FUN_006b5d00();
    *(undefined1 *)(param_3 + 0x2d) = uVar1;
    uVar1 = CSWSCreatureStats__GetCONStat();
    *(undefined1 *)(param_3 + 0x2e) = uVar1;
    uVar1 = CSWSCreatureStats__GetINTStat();
    *(undefined1 *)(param_3 + 0x2f) = uVar1;
    uVar1 = CSWSCreatureStats__GetWISStat();
    *(undefined1 *)(param_3 + 0x30) = uVar1;
    uVar1 = CSWSCreatureStats__GetCHAStat();
    *(undefined1 *)(param_3 + 0x31) = uVar1;
    *(undefined1 *)(param_3 + 0x32) = *(undefined1 *)(in_ECX + 0xed);
```

## 0x006adec0 CSWSCreatureStats::GetDamageRoll

- Size 7065 B, RET 24
- Callers (3 total):
  - 0x00580330 CSWSCreature::EquipMostDamagingMeleeWeapon (site 0x00580603)
  - 0x00580c50 CSWSCreature::GetIsWeaponEffective (site 0x00580cea)
  - 0x006e1540 CSWSCreature::ResolveDamage (site 0x006e1666)
- Callees (139):
  - 0x006adf70 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006adfa7 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006adfde CALL 0x00591e10 CSWSCombatRound::GetCurrentAttackWeapon
  - 0x006ae011 CALL 0x00591e10 CSWSCombatRound::GetCurrentAttackWeapon
  - 0x006ae03c indirect
  - 0x006ae069 indirect
  - 0x006ae0aa CALL 0x0051c830 CServerExoApp::GetDifficultyOption
  - 0x006ae0d7 indirect
  - 0x006ae117 CALL 0x0051c830 CServerExoApp::GetDifficultyOption
  - 0x006ae12c CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006ae15c CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006ae1ba CALL 0x00591fd0 CSWSCombatRound::GetWeaponAttackType
  - 0x006ae1eb CALL 0x00591fd0 CSWSCombatRound::GetWeaponAttackType
  - 0x006ae21c CALL 0x00591fd0 CSWSCombatRound::GetWeaponAttackType
  - 0x006ae231 CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006ae249 CALL 0x006042e0 CSWSItem::GetPropertyByType
  - 0x006ae298 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006ae2dc CALL 0x0071bcf0
  - 0x006ae2ee CALL 0x00733780 CExoString::Destructor
  - 0x006ae2fe CALL 0x00733570 CExoString::CStrConstructor
  - 0x006ae343 CALL 0x0071bcf0
  - 0x006ae355 CALL 0x00733780 CExoString::Destructor
  - 0x006ae3ba CALL 0x00733570 CExoString::CStrConstructor
  - 0x006ae3d8 CALL 0x0071b670
  - 0x006ae3ea CALL 0x00733780 CExoString::Destructor
  - 0x006ae4ad CALL 0x0091c860
  - 0x006ae4e6 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ae504 CALL 0x006a2000
  - 0x006ae5cb CALL 0x0091c860
  - 0x006ae5fa CALL 0x006a22c0
  - 0x006ae623 CALL 0x006a2000
  - 0x006ae6ea CALL 0x0091c860
  - 0x006ae719 CALL 0x006a22c0
  - 0x006ae755 CALL 0x006d5200
  - 0x006ae7a1 CALL 0x006d5200
  - 0x006ae821 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006ae83f CALL 0x0071b670
  - 0x006ae851 CALL 0x00733780 CExoString::Destructor
  - 0x006ae914 CALL 0x0091c860
  - 0x006ae94d CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006ae96b CALL 0x006a2000
  - 0x006aea32 CALL 0x0091c860
  - 0x006aea61 CALL 0x006a22c0
  - 0x006aea8a CALL 0x006a2000
  - 0x006aeb51 CALL 0x0091c860
  - 0x006aeb80 CALL 0x006a22c0
  - 0x006aebbb CALL 0x00591fd0 CSWSCombatRound::GetWeaponAttackType
  - 0x006aebec CALL 0x00591fd0 CSWSCombatRound::GetWeaponAttackType
  - 0x006aec1d CALL 0x00591fd0 CSWSCombatRound::GetWeaponAttackType
  - 0x006aec32 CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006aec4a CALL 0x006042e0 CSWSItem::GetPropertyByType
  - 0x006aec9c CALL 0x00733570 CExoString::CStrConstructor
  - 0x006aece3 CALL 0x0071bcf0
  - 0x006aecf5 CALL 0x00733780 CExoString::Destructor
  - 0x006aed05 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006aed4d CALL 0x0071bcf0
  - 0x006aed5f CALL 0x00733780 CExoString::Destructor
  - 0x006aed90 CALL 0x00591fd0 CSWSCombatRound::GetWeaponAttackType
  - 0x006aedc1 CALL 0x00591fd0 CSWSCombatRound::GetWeaponAttackType
  - 0x006aedd1 CALL 0x006a8f10 CSWSCreatureStats::GetWeaponSpecialization
  - 0x006aede2 CALL 0x006a8fe0
  - 0x006aee1a CALL 0x006d5200
  - 0x006aee66 CALL 0x006d5200
  - 0x006aee9c CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006aeecf CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006aeee3 CALL 0x006a9100 CSWSCreatureStats::GetUnarmedDamageRoll
  - 0x006aef00 CALL 0x006a9100 CSWSCreatureStats::GetUnarmedDamageRoll
  - 0x006aef3f CALL 0x0057f870 CSWSCreature::GetDamageFlags
  - 0x006aef48 CALL 0x0058d200
  - 0x006aef85 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006aef96 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006aefa8 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aefc1 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006aefd8 CALL 0x00734270 CExoString::Format
  - 0x006aeff3 CALL 0x00734270 CExoString::Format
  - 0x006af00f CALL 0x00733df0 CExoString::operator+
  - 0x006af05c CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006af068 CALL 0x007337c0 CExoString::operator=
  - 0x006af077 CALL 0x00733780 CExoString::Destructor
  - 0x006af086 CALL 0x00733780 CExoString::Destructor
  - 0x006af098 CALL 0x00733780 CExoString::Destructor
  - 0x006af0c3 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af0e3 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af103 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af123 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af143 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af163 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af180 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af19d CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af1ba CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af1d7 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006af1fa CALL 0x006a2000
  - 0x006af240 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006af251 CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006af263 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006af27f CALL 0x00734270 CExoString::Format
  - 0x006af29b CALL 0x00733df0 CExoString::operator+
  - 0x006af2e8 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006af2f4 CALL 0x007337c0 CExoString::operator=
  - 0x006af303 CALL 0x00733780 CExoString::Destructor
  - 0x006af312 CALL 0x00733780 CExoString::Destructor
  - 0x006af324 CALL 0x00733780 CExoString::Destructor
  - 0x006af357 indirect
  - 0x006af370 indirect
  - 0x006af379 CALL 0x006abf70 CSWSCreatureStats::GetDamageBonus
  - 0x006af396 CALL 0x006abf70 CSWSCreatureStats::GetDamageBonus
  - 0x006af3fb CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006af4b2 CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006af50f CALL 0x006042a0 CSWSItem::GetPassiveProperty
  - 0x006af570 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006af5b8 CALL 0x0071bcf0
  - 0x006af5ca CALL 0x00733780 CExoString::Destructor
  - 0x006af5fe CALL 0x00733570 CExoString::CStrConstructor
  - 0x006af645 CALL 0x0071bcf0
  - 0x006af657 CALL 0x00733780 CExoString::Destructor
  - 0x006af672 CALL 0x006a2000
  - 0x006af728 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
  - 0x006af789 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006af79a CALL 0x007335f0 CExoString::CExoString_copy
  - 0x006af7ac CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006af7c5 CALL 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier
  - 0x006af7db CALL 0x00734270 CExoString::Format
  - 0x006af7f5 CALL 0x00734270 CExoString::Format
  - 0x006af811 CALL 0x00733df0 CExoString::operator+
  - 0x006af85e CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006af86a CALL 0x007337c0 CExoString::operator=
  - 0x006af879 CALL 0x00733780 CExoString::Destructor
  - 0x006af888 CALL 0x00733780 CExoString::Destructor
  - 0x006af89a CALL 0x00733780 CExoString::Destructor
  - 0x006af8d9 CALL 0x0057f870 CSWSCreature::GetDamageFlags
  - 0x006af8e2 CALL 0x0058d200
  - 0x006af8fc CALL 0x0057f870 CSWSCreature::GetDamageFlags
  - 0x006af928 CALL 0x0057f820 CSWSCreature::CalculateDamagePower
  - 0x006af964 indirect
  - 0x006af9a0 indirect
  - 0x006af9da indirect
  - 0x006af9eb indirect
  - 0x006af9fc indirect
  - 0x006afa30 indirect
- Imports: none
- Indirect slots (11):
  - 0x006ae03c slot +0x30 (006ae039 MOV EDX,dword ptr [EAX + 0x30])
  - 0x006ae069 slot +0x30 (006ae066 MOV EDX,dword ptr [EAX + 0x30])
  - 0x006ae0d7 slot +0x30 (006ae0d4 MOV EAX,dword ptr [EDX + 0x30])
  - 0x006af357 slot +0x30 (006af354 MOV EAX,dword ptr [EDX + 0x30])
  - 0x006af370 slot +0x30 (006af36d MOV EAX,dword ptr [EDX + 0x30])
  - 0x006af964 slot +0xac (006af95e MOV EAX,dword ptr [EDX + 0xac])
  - 0x006af9a0 slot +0xa8 (006af99a MOV EAX,dword ptr [EDX + 0xa8])
  - 0x006af9da slot +0xa4 (006af9d4 MOV EAX,dword ptr [EDX + 0xa4])
  - 0x006af9eb slot +0x18 (006af9e8 MOV EAX,dword ptr [EDX + 0x18])
  - 0x006af9fc slot +0x48 (006af9f9 MOV EAX,dword ptr [EDX + 0x48])
  - 0x006afa30 slot +0x98 (006afa2a MOV EAX,dword ptr [EDX + 0x98])
- Ledger callsites in range: none
- Strings (6):
  - 0x00995614 "NumDice"
  - 0x00999780 " + %d (Effect Damage Bonus)"
  - 0x0099979c " + %d (Effect Damage Bonus) (Critical x%d)"
  - 0x009997c8 " + %d (Sneak Attack Damage)"
  - 0x009997e4 "Damage Roll: %d"
  - 0x009997f4 "Damage Roll: %d (Critical x%d)"
- Globals (5):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4a4 READ (MOV EDX,dword ptr [0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes attack damage roll from weapon dice, effect damage, sneak attack, and critical multiplier, with breakdown.
- Named pseudo-C (first 30 of 540 lines):
```c
// NAMED 0x006adec0 CSWSCreatureStats::GetDamageRoll evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=21 exe=6a522e71631d

int CSWSCreatureStats__GetDamageRoll
              (int *param_1,int param_2,int param_3,int param_4,int param_5,int param_6)

{
  char cVar1;
  short sVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  int in_ECX;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined1 local_13c [8];
  undefined1 local_134 [8];
  undefined1 local_12c [8];
  undefined1 local_124 [8];
  undefined1 local_11c [8];
  undefined1 local_114 [8];
  undefined1 local_10c [8];
  undefined1 local_104 [8];
  undefined1 local_fc [8];
  undefined1 local_f4 [8];
  undefined1 local_ec [8];
  undefined1 local_e4 [16];
  uint local_d4;
  uint local_d0;
  int local_cc;
  undefined1 local_c8 [4];
```

## 0x006afa60 CSWSCreatureStats::GetCriticalHitRoll

- Size 514 B, RET 4
- Callers (2 total):
  - 0x006a7190 CSWSCreatureStats::UpdateCombatInformation (site 0x006a7c7e)
  - 0x006e0b10 CSWSCreature::ResolveAttackRoll (site 0x006e0da0)
- Callees (7):
  - 0x006afa9d CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006afacd CALL 0x006d5200
  - 0x006afb01 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006afb30 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006afb66 CALL 0x006d5200
  - 0x006afb90 CALL 0x006042e0 CSWSItem::GetPropertyByType
  - 0x006afbda CALL 0x005908c0 CSWSCombatRound::GetAttack
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Likely computes crit range from weapon base-item byte +0x1e, doubled by property 0x1c; unarmed defaults to 1.
- Named pseudo-C (first 30 of 64 lines):
```c
// NAMED 0x006afa60 CSWSCreatureStats::GetCriticalHitRoll evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=4 exe=6a522e71631d

int CSWSCreatureStats__GetCriticalHitRoll(int param_1)

{
  int iVar1;
  int in_ECX;
  undefined4 local_24;
  uint local_20;
  uint local_1c;
  int local_18;
  uint local_14;
  int local_10;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(in_ECX + 0x24) != 0) {
    local_10 = CSWInventory__GetItemInSlot(0x10);
    if (((param_1 != 0) && (local_10 != 0)) &&
       (iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc)), *(char *)(iVar1 + 8) != '\x03')) {
      local_10 = CSWInventory__GetItemInSlot(0x20);
    }
    local_20 = 0;
    if ((local_10 == 0) && (local_10 = CSWInventory__GetItemInSlot(8), local_10 == 0)) {
      local_20 = 1;
    }
    if (local_10 != 0) {
      iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc));
      local_20 = (uint)*(byte *)(iVar1 + 0x1e);
      local_24 = 0;
```

## 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier

- Size 422 B, RET 4
- Callers (5 total):
  - 0x00582010 CSWSCreature::GetTotalEffectBonus (site 0x00582c97)
  - 0x006a7190 CSWSCreatureStats::UpdateCombatInformation (site 0x006a7c4a)
  - 0x006abf70 CSWSCreatureStats::GetDamageBonus (site 0x006ac166)
  - 0x006adec0 CSWSCreatureStats::GetDamageRoll (site 0x006ae4e6)
  - 0x006e0b10 CSWSCreature::ResolveAttackRoll (site 0x006e0ea9)
- Callees (7):
  - 0x006afd04 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006afd2d CALL 0x006d5200
  - 0x006afd61 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006afd85 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006afdaf CALL 0x006d5200
  - 0x006afdf6 CALL 0x005908c0 CSWSCombatRound::GetAttack
  - 0x006afe3d CALL 0x005763e0 CSWSCreature::IsWieldingLightsaber
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns crit multiplier: weapon base-item byte +0x1f (default 2 unarmed), plus 1 for some attack types and lightsabers.
- Named pseudo-C (first 30 of 43 lines):
```c
// NAMED 0x006afcd0 CSWSCreatureStats::GetCriticalHitMultiplier evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=4 exe=6a522e71631d

uint CSWSCreatureStats__GetCriticalHitMultiplier(int param_1)

{
  int iVar1;
  int in_ECX;
  int local_10;
  uint local_c;
  
  local_c = 2;
  if (param_1 == 0) {
    local_10 = CSWInventory__GetItemInSlot(0x10);
  }
  else {
    local_10 = CSWInventory__GetItemInSlot(0x10);
    if ((local_10 != 0) &&
       (iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc)), *(char *)(iVar1 + 8) != '\x03')) {
      local_10 = CSWInventory__GetItemInSlot(0x20);
    }
  }
  if (local_10 != 0) {
    iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc));
    local_c = (uint)*(byte *)(iVar1 + 0x1f);
  }
  iVar1 = CSWSCombatRound__GetAttack
                    (*(undefined1 *)(*(int *)(*(int *)(in_ECX + 0x24) + 0x10dc) + 0xabc));
  switch(*(undefined2 *)(iVar1 + 100)) {
  case 0x11:
  case 0x12:
```

## 0x006afed0 CSWSCreatureStats::ReadStatsFromGff

- Size 15520 B, RET 12
- Callers (5 total):
  - 0x00572a70 CSWSCreature::LoadFromTemplate (site 0x00572c89)
  - 0x00574180 CSWSCreature::LoadCreature (site 0x005741ca)
  - 0x00630810 (site 0x00630b3e)
  - 0x00631260 (site 0x0063136f)
  - 0x00638bd0 (site 0x00638e29)
- Callees (291):
  - 0x006aff13 CALL 0x00735240
  - 0x006aff22 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006aff3f CALL 0x0073f350 CClientExoApp::GetClientLanguage
  - 0x006aff4d CALL 0x00735240
  - 0x006aff86 CALL 0x007190e0 CResGFF::ReadFieldCExoLocString
  - 0x006affae CALL 0x00735400
  - 0x006affbd CALL 0x00735360 CExoLocString::Destructor
  - 0x006affcc CALL 0x00735360 CExoLocString::Destructor
  - 0x006affe1 CALL 0x00735400
  - 0x006afff6 CALL 0x007356b0
  - 0x006b0001 CALL 0x00735240
  - 0x006b003a CALL 0x007190e0 CResGFF::ReadFieldCExoLocString
  - 0x006b0062 CALL 0x00735400
  - 0x006b0071 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b0080 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b0095 CALL 0x00735400
  - 0x006b00aa CALL 0x007356b0
  - 0x006b00d0 CALL 0x007190e0 CResGFF::ReadFieldCExoLocString
  - 0x006b00fb CALL 0x00735400
  - 0x006b010a CALL 0x00735360 CExoLocString::Destructor
  - 0x006b0136 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0159 CALL 0x006b6b30 CSWSCreatureStats::GetTag
  - 0x006b0176 CALL 0x00718f90 CResGFF::ReadFieldCExoString
  - 0x006b019e CALL 0x006b6b50 CSWSCreatureStats::SetTag
  - 0x006b01ad CALL 0x00733780 CExoString::Destructor
  - 0x006b01fc CALL 0x00718e40 CResGFF::ReadFieldCResRef
  - 0x006b021e CALL 0x00710ca0
  - 0x006b024a CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b028d CALL 0x00718c80 CResGFF::ReadFieldINT
  - 0x006b02ce CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0337 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0380 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b03be CALL 0x00733780 CExoString::Destructor
  - 0x006b03d0 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b0414 CALL 0x00718f90 CResGFF::ReadFieldCExoString
  - 0x006b0442 CALL 0x007337c0 CExoString::operator=
  - 0x006b0451 CALL 0x00733780 CExoString::Destructor
  - 0x006b0480 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b04c1 CALL 0x00718f90 CResGFF::ReadFieldCExoString
  - 0x006b04ef CALL 0x007337c0 CExoString::operator=
  - 0x006b04fe CALL 0x00733780 CExoString::Destructor
  - 0x006b052d CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b053c CALL 0x006b67d0 CSWSCreatureStats::SetSTRBase
  - 0x006b056b CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b057a CALL 0x006b6810 CSWSCreatureStats::SetDEXBase
  - 0x006b05a9 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b05b8 CALL 0x006b6920 CSWSCreatureStats::SetINTBase
  - 0x006b05e7 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b05f6 CALL 0x006b6960 CSWSCreatureStats::SetWISBase
  - 0x006b0627 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0636 CALL 0x006b6850 CSWSCreatureStats::SetCONBase
  - 0x006b0665 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0674 CALL 0x006b69a0 CSWSCreatureStats::SetCHABase
  - 0x006b0697 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b06fe CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b0770 CALL 0x005812e0 CSWSCreature::GetGold
  - 0x006b0786 CALL 0x00718c00 CResGFF::ReadFieldDWORD
  - 0x006b0792 CALL 0x00581330 CSWSCreature::SetGold
  - 0x006b07bb CALL 0x00581580 CSWSCreature::GetComponent
  - 0x006b07d1 CALL 0x00718c00 CResGFF::ReadFieldDWORD
  - 0x006b07dd CALL 0x005815d0 CSWSCreature::SetComponent
  - 0x006b0806 CALL 0x005817f0 CSWSCreature::GetChemical
  - 0x006b081c CALL 0x00718c00 CResGFF::ReadFieldDWORD
  - 0x006b0828 CALL 0x00581840 CSWSCreature::SetChemical
  - 0x006b0866 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b08b3 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b091b CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b097d CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b09eb CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0a54 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0aba CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0b32 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0b97 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0bfc CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0c5e CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0cc0 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0d11 CALL 0x00718c00 CResGFF::ReadFieldDWORD
  - 0x006b0d1d CALL 0x006b6bd0 CSWSCreatureStats::SetExperience
  - 0x006b0d37 CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b0d8c indirect
  - 0x006b0dda CALL 0x00718e40 CResGFF::ReadFieldCResRef
  - 0x006b0e1c CALL 0x00710ca0
  - 0x006b0e56 CALL 0x007109b0
  - 0x006b0e94 indirect
  - 0x006b0ec2 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b0f61 CALL 0x00718a80 CResGFF::ReadFieldCHAR
  - 0x006b1038 CALL 0x00718a80 CResGFF::ReadFieldCHAR
  - 0x006b1076 CALL 0x00718c00 CResGFF::ReadFieldDWORD
  - 0x006b10bd CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b1104 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b114b CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b1192 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b11d9 CALL 0x00718c80 CResGFF::ReadFieldINT
  - 0x006b1222 CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b126c CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b130d CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b1354 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b13b5 CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b13fe CALL 0x00718d00 CResGFF::ReadFieldFLOAT
  - 0x006b1447 CALL 0x00718d00 CResGFF::ReadFieldFLOAT
  - 0x006b148e CALL 0x00718c80 CResGFF::ReadFieldINT
  - 0x006b14f5 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b1539 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b157c CALL 0x0071b670
  - 0x006b158b CALL 0x00733780 CExoString::Destructor
  - 0x006b15c8 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b160b CALL 0x0071b670
  - 0x006b161a CALL 0x00733780 CExoString::Destructor
  - 0x006b1663 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b168b CALL 0x00733780 CExoString::Destructor
  - 0x006b169d CALL 0x00735360 CExoLocString::Destructor
  - 0x006b16c5 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b171e CALL 0x0071bcf0
  - 0x006b173a CALL 0x00733780 CExoString::Destructor
  - 0x006b175b CALL 0x00733780 CExoString::Destructor
  - 0x006b176d CALL 0x00735360 CExoLocString::Destructor
  - 0x006b1788 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b17d0 CALL 0x0071b670
  - 0x006b17ec CALL 0x00733780 CExoString::Destructor
  - 0x006b180d CALL 0x00733780 CExoString::Destructor
  - 0x006b181f CALL 0x00735360 CExoLocString::Destructor
  - 0x006b1867 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b18af CALL 0x0071b670
  - 0x006b18cb CALL 0x00733780 CExoString::Destructor
  - 0x006b18ec CALL 0x00733780 CExoString::Destructor
  - 0x006b18fe CALL 0x00735360 CExoLocString::Destructor
  - 0x006b194b CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b1992 CALL 0x0071b670
  - 0x006b19ae CALL 0x00733780 CExoString::Destructor
  - 0x006b19cf CALL 0x00733780 CExoString::Destructor
  - 0x006b19e1 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b1a29 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b1a71 CALL 0x0071b670
  - 0x006b1a8d CALL 0x00733780 CExoString::Destructor
  - 0x006b1aae CALL 0x00733780 CExoString::Destructor
  - 0x006b1ac0 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b1b10 CALL 0x00718760 CResGFF::GetList
  - 0x006b1b27 CALL 0x00718810 CResGFF::GetListCount
  - 0x006b1b41 CALL 0x00733780 CExoString::Destructor
  - 0x006b1b53 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b1b6d CALL 0x00718810 CResGFF::GetListCount
  - 0x006b1bda CALL 0x00718890 CResGFF::GetListElement
  - 0x006b1c46 CALL 0x00718c80 CResGFF::ReadFieldINT
  - 0x006b1c92 CALL 0x00733780 CExoString::Destructor
  - 0x006b1ca4 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b1cc7 CALL 0x006b69e0 CSWSCreatureStats::SetClass
  - 0x006b1d30 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b1d50 CALL 0x00718b80 CResGFF::ReadFieldSHORT
  - 0x006b1d6e CALL 0x006b6a20 CSWSCreatureStats::SetClassLevel
  - 0x006b1dd4 CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006b1dfa CALL 0x00718760 CResGFF::GetList
  - 0x006b1e31 CALL 0x00718810 CResGFF::GetListCount
  - 0x006b1e57 CALL 0x00718890 CResGFF::GetListElement
  - 0x006b1e71 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b1ea0 CALL 0x006a4100
  - 0x006b1ec9 CALL 0x006a4160
  - 0x006b1fcb CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b1fe8 CALL 0x00733780 CExoString::Destructor
  - 0x006b1ffa CALL 0x00735360 CExoLocString::Destructor
  - 0x006b20d5 CALL 0x00733780 CExoString::Destructor
  - 0x006b20e7 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b2264 CALL 0x00733780 CExoString::Destructor
  - 0x006b2276 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b22b4 CALL 0x00718b80 CResGFF::ReadFieldSHORT
  - 0x006b22ff CALL 0x00718b80 CResGFF::ReadFieldSHORT
  - 0x006b234a CALL 0x00718b80 CResGFF::ReadFieldSHORT
  - 0x006b2396 CALL 0x00718760 CResGFF::GetList
  - 0x006b23c2 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b23df CALL 0x00919723 operator new
  - 0x006b2400 CALL 0x006f58b0 PendingLevelUp::Constructor
  - 0x006b2443 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006b2458 CALL 0x006ad3b0 CSWSCreatureStats::GetLevelStats
  - 0x006b247c CALL 0x00718890 CResGFF::GetListElement
  - 0x006b24b3 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b24f7 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b253b CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b257f CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b25c6 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006b25e8 CALL 0x00734270 CExoString::Format
  - 0x006b25f6 CALL 0x00734010
  - 0x006b260a CALL 0x00718760 CResGFF::GetList
  - 0x006b2642 CALL 0x00718810 CResGFF::GetListCount
  - 0x006b2661 CALL 0x00718890 CResGFF::GetListElement
  - 0x006b2682 CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b26b0 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006b26d3 CALL 0x00734270 CExoString::Format
  - 0x006b26e1 CALL 0x00734010
  - 0x006b26f5 CALL 0x00718760 CResGFF::GetList
  - 0x006b272d CALL 0x00718810 CResGFF::GetListCount
  - 0x006b274c CALL 0x00718890 CResGFF::GetListElement
  - 0x006b276d CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b279f CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006b27b3 CALL 0x00733780 CExoString::Destructor
  - 0x006b27ec CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b281f CALL 0x00718760 CResGFF::GetList
  - 0x006b283d CALL 0x0091c630
  - 0x006b2887 CALL 0x00718890 CResGFF::GetListElement
  - 0x006b289e CALL 0x006f5ac0 PendingLevelUp::GetSkillDelta
  - 0x006b28ba CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b28d1 CALL 0x006f5b00 PendingLevelUp::SetSkillDelta
  - 0x006b28f1 CALL 0x00718760 CResGFF::GetList
  - 0x006b2908 CALL 0x00718810 CResGFF::GetListCount
  - 0x006b295c CALL 0x00718890 CResGFF::GetListElement
  - 0x006b297a CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b299a CALL 0x006f5b40 PendingLevelUp::AddFeat
  - 0x006b29e4 CALL 0x00718b80 CResGFF::ReadFieldSHORT
  - 0x006b2a59 CALL 0x00718b80 CResGFF::ReadFieldSHORT
  - 0x006b2acf CALL 0x00718b80 CResGFF::ReadFieldSHORT
  - 0x006b2b25 CALL 0x00718b80 CResGFF::ReadFieldSHORT
  - 0x006b2b52 CALL 0x00718c80 CResGFF::ReadFieldINT
  - 0x006b2b90 CALL 0x00718c80 CResGFF::ReadFieldINT
  - 0x006b2bce CALL 0x00718c80 CResGFF::ReadFieldINT
  - 0x006b2c62 indirect
  - 0x006b2c8b CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b2cda CALL 0x0057eca0 CSWSCreature::GetMaxForcePoints
  - 0x006b2d39 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2d4e CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2d71 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2d80 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b2dbb CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2dcf CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b2dfc CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2e11 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2e46 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2ea5 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2f0f CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2f24 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2f47 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2f62 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b2f9d CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2fb1 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b2fe0 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b2ff5 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3018 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3027 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b3062 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3076 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b30a7 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b30bc CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b30d0 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b310e CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3183 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3198 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b31b9 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b31f9 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3260 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3275 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3289 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b32c7 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b3331 indirect
  - 0x006b334f CALL 0x0057eca0 CSWSCreature::GetMaxForcePoints
  - 0x006b336e CALL 0x00718760 CResGFF::GetList
  - 0x006b3385 CALL 0x00718810 CResGFF::GetListCount
  - 0x006b33d9 CALL 0x00718890 CResGFF::GetListElement
  - 0x006b33f7 CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b3417 CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006b3452 indirect
  - 0x006b3470 CALL 0x0057eca0 CSWSCreature::GetMaxForcePoints
  - 0x006b34db indirect
  - 0x006b34e7 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b3571 CALL 0x00718760 CResGFF::GetList
  - 0x006b3588 CALL 0x00718810 CResGFF::GetListCount
  - 0x006b35d8 CALL 0x00718890 CResGFF::GetListElement
  - 0x006b35fa CALL 0x00718c00 CResGFF::ReadFieldDWORD
  - 0x006b361a CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b3675 CALL 0x005d1400 CSWSCreature::AddToVisibleList
  - 0x006b3692 CALL 0x00718960 CResGFF::GetStructFromStruct
  - 0x006b36ce CALL 0x00592840 CSWSCombatRound::LoadCombatRound
  - 0x006b3868 CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b389b CALL 0x00718760 CResGFF::GetList
  - 0x006b38b9 CALL 0x0091c630
  - 0x006b38fe CALL 0x00718890 CResGFF::GetListElement
  - 0x006b3919 CALL 0x006b7bb0 CSWSCreatureStats::GetSkillRank
  - 0x006b3935 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b394c CALL 0x006b81c0 CSWSCreatureStats::SetSkillRank
  - 0x006b3980 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b398f CALL 0x006ba320 CSWSCreatureStats::SetMovementRate
  - 0x006b39c3 CALL 0x00718c80 CResGFF::ReadFieldINT
  - 0x006b39cf CALL 0x006ba320 CSWSCreatureStats::SetMovementRate
  - 0x006b39e4 CALL 0x00718760 CResGFF::GetList
  - 0x006b39f8 CALL 0x00718810 CResGFF::GetListCount
  - 0x006b3a0f CALL 0x00718810 CResGFF::GetListCount
  - 0x006b3a63 CALL 0x00718890 CResGFF::GetListElement
  - 0x006b3a85 CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b3aa8 CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b3acb CALL 0x00718a00 CResGFF::ReadFieldBYTE
  - 0x006b3af6 CALL 0x006ba7d0 CSWSCreatureStats::GetFeat
  - 0x006b3b20 CALL 0x0069d1e0
  - 0x006b3b36 CALL 0x00733780 CExoString::Destructor
  - 0x006b3b48 CALL 0x00735360 CExoLocString::Destructor
  - 0x006b3b65 CALL 0x009196ee __security_check_cookie
- Imports: none
- Indirect slots (6):
  - 0x006b0d8c slot +0xd4 (006b0d86 MOV EDX,dword ptr [EAX + 0xd4])
  - 0x006b0e94 slot +0xd4 (006b0e8e MOV EAX,dword ptr [EDX + 0xd4])
  - 0x006b2c62 slot +0x98 (006b2c5c MOV EDX,dword ptr [EAX + 0x98])
  - 0x006b3331 slot +0x98 (006b332b MOV EDX,dword ptr [EAX + 0x98])
  - 0x006b3452 slot +0x98 (006b344c MOV EAX,dword ptr [EDX + 0x98])
  - 0x006b34db slot +0x9c (006b34d5 MOV EDX,dword ptr [EAX + 0x9c])
- Ledger callsites in range (2):
  - 0x006afff6 FUN_006afed0 -> 0x7356b0, gender PUSH 0 (constant 0)
  - 0x006b00aa FUN_006afed0 -> 0x7356b0, gender PUSH 0 (constant 0)
- Strings (37):
  - 0x009953dc "AssignedPup"
  - 0x009953e8 "BonusForcePoints"
  - 0x0099544c "CombatRoundData"
  - 0x0099545c "PerceptionData"
  - 0x0099546c "PerceptionList"
  - 0x00996c74 "BodyBag"
  - 0x00996dcc "PartyInteract"
  - 0x00999814 "SpellCasterLevel"
  - 0x00999828 "SpellFlags"
  - 0x00999834 "SpecAbilityList"
  - 0x00999844 "WalkRate"
  - 0x00999850 "MovementRate"
  - 0x009998c0 "KnownRemoveList"
  - 0x009998f4 "LvlStatForce"
  - 0x00999914 "LvlStatAbility"
  - 0x00999930 "refbonus"
  - 0x0099993c "fortbonus"
  - 0x00999948 "willbonus"
  - 0x00999954 "NumSpellsLeft"
  - 0x00999964 "SpellsPerDayList"
  - 0x00999978 "PERCEPTIONDIST"
  - 0x00999988 "PerceptionRange"
  - 0x00999998 "AIState"
  - 0x009999a0 "ChallengeRating"
  - 0x009999b0 "BlindSpot"
  - 0x009999bc "FactionID"
  - 0x00999a4c "CurrentForm"
  - 0x00999a58 "FuryDamageBonus"
  - 0x00999a88 "Disarmable"
  - 0x00999a94 "Confused"
  - 0x00999aa0 "WillNotRender"
  - 0x00999ab0 "MultiplierSet"
  - 0x00999ac0 "IgnoreCrePath"
  - 0x00999ad0 "Hologram"
  - 0x00999adc "NotReorienting"
  - 0x00999b18 "NaturalAC"
  - 0x00999b64 "Interruptable"
- Globals (4):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched (3):
  - 0x6c CSWSCreatureStats::IsPC (int) W
  - 0x84 CSWSCreatureStats::BlindSpot (float) W
  - 0x101c CSWSCreature::PrimaryRange (spot) (float) W
- KDB purpose: Loads creature stats from GFF: names, tag, ability scores, class list, perception and assorted stat fields.
- Named pseudo-C (first 30 of 1207 lines):
```c
// NAMED 0x006afed0 CSWSCreatureStats::ReadStatsFromGff evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=postpass exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006b3519) */
/* WARNING: Removing unreachable block (ram,0x006b0edd) */
/* WARNING: Removing unreachable block (ram,0x006b0f16) */

void CSWSCreatureStats__ReadStatsFromGff(undefined4 param_1,undefined4 param_2,int param_3)

{
  byte bVar1;
  undefined1 uVar2;
  char cVar3;
  byte bVar4;
  undefined2 uVar5;
  ushort uVar6;
  short sVar7;
  undefined4 uVar8;
  undefined4 *puVar9;
  uint uVar10;
  int iVar11;
  int in_ECX;
  short sVar12;
  uint uVar13;
  int iVar14;
  float10 fVar15;
  undefined4 uVar16;
  int local_6d4;
  short local_62c;
  ushort local_624;
  short local_61c;
```

## 0x006b3b70 CSWSCreatureStats::ReadSpellsFromGff

- Size 413 B, RET 8
- Callers (4 total):
  - 0x00572a70 CSWSCreature::LoadFromTemplate (site 0x00572d25)
  - 0x00574180 CSWSCreature::LoadCreature (site 0x00574406)
  - 0x00630810 (site 0x00630c9b)
  - 0x00631260 (site 0x006314f8)
- Callees (12):
  - 0x006b3bac CALL 0x00718760 CResGFF::GetList
  - 0x006b3bee CALL 0x00718890 CResGFF::GetListElement
  - 0x006b3c1a CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006b3c39 CALL 0x00734270 CExoString::Format
  - 0x006b3c44 CALL 0x00734010
  - 0x006b3c55 CALL 0x00718760 CResGFF::GetList
  - 0x006b3c7d CALL 0x00718810 CResGFF::GetListCount
  - 0x006b3c96 CALL 0x00718890 CResGFF::GetListElement
  - 0x006b3cb4 CALL 0x00718b00 CResGFF::ReadFieldWORD
  - 0x006b3cd1 CALL 0x006b86a0 CSWSCreatureStats::AddKnownSpell
  - 0x006b3ce2 CALL 0x00733780 CExoString::Destructor
  - 0x006b3d02 CALL 0x009196ee __security_check_cookie
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Reads each class's known-spell list from GFF ClassList, calling AddKnownSpell per WORD spell id.
- Named pseudo-C (first 30 of 66 lines):
```c
// NAMED 0x006b3b70 CSWSCreatureStats::ReadSpellsFromGff evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=8 exe=6a522e71631d

void CSWSCreatureStats__ReadSpellsFromGff(undefined4 param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  int in_ECX;
  uint uVar5;
  undefined1 local_5c [4];
  ushort local_58;
  undefined1 local_54 [11];
  char local_49;
  undefined1 local_48 [4];
  undefined1 local_44 [20];
  undefined1 local_30 [4];
  undefined1 local_2c [20];
  uint local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c0c8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
```

## 0x006b3d10 CSWSCreatureStats::SaveStats

- Size 4486 B, RET 8
- Callers (2 total):
  - 0x00574500 CSWSCreature::SaveCreature (site 0x00574557)
  - 0x00638bd0 (site 0x0063982f)
- Callees (104):
  - 0x006b3d56 CALL 0x00719f30 CResGFF::WriteFieldCExoLocString
  - 0x006b3d71 CALL 0x00719f30 CResGFF::WriteFieldCExoLocString
  - 0x006b3d8c CALL 0x00719f30 CResGFF::WriteFieldCExoLocString
  - 0x006b3dae CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b3dbe CALL 0x006b6b30 CSWSCreatureStats::GetTag
  - 0x006b3dcb CALL 0x00719e50 CResGFF::WriteFieldCExoString
  - 0x006b3e00 CALL 0x00719d40 CResGFF::WriteFieldCResRef
  - 0x006b3e22 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b3e43 CALL 0x00719bf0 CResGFF::WriteFieldINT
  - 0x006b3e65 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b3e8c CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b3eaa CALL 0x00719e50 CResGFF::WriteFieldCExoString
  - 0x006b3ecf CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b3ef4 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b3f11 CALL 0x00719e50 CResGFF::WriteFieldCExoString
  - 0x006b3f39 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b3f62 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b3f8b CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b3fb4 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b3fcd CALL 0x005812e0 CSWSCreature::GetGold
  - 0x006b3fda CALL 0x00719b90 CResGFF::WriteFieldDWORD
  - 0x006b3ff3 CALL 0x00581580 CSWSCreature::GetComponent
  - 0x006b4000 CALL 0x00719b90 CResGFF::WriteFieldDWORD
  - 0x006b4019 CALL 0x005817f0 CSWSCreature::GetChemical
  - 0x006b4026 CALL 0x00719b90 CResGFF::WriteFieldDWORD
  - 0x006b4038 CALL 0x006ba740 CSWSCreatureStats::GetReflexSavingThrow
  - 0x006b4048 CALL 0x00719a40 CResGFF::WriteFieldCHAR
  - 0x006b405a CALL 0x006ba6b0 CSWSCreatureStats::GetWillSavingThrow
  - 0x006b406a CALL 0x00719a40 CResGFF::WriteFieldCHAR
  - 0x006b407c CALL 0x006ba620 CSWSCreatureStats::GetFortSavingThrow
  - 0x006b408c CALL 0x00719a40 CResGFF::WriteFieldCHAR
  - 0x006b40a5 CALL 0x0057e530 CSWSCreature::GetArmorClass
  - 0x006b40b5 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b40da CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b40ff CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4124 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4149 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b416e CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4193 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b41b2 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b41e8 CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b4222 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b425c CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4296 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b42d0 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4310 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b434a CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4384 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b43b2 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b43ec CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4426 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4460 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b449a CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b44d6 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b4551 indirect
  - 0x006b456e indirect
  - 0x006b458a CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b45c5 indirect
  - 0x006b45d2 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b460d indirect
  - 0x006b461a CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b4668 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b473e CALL 0x0057eca0 CSWSCreature::GetMaxForcePoints
  - 0x006b4766 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b479f CALL 0x00719bf0 CResGFF::WriteFieldINT
  - 0x006b47d8 CALL 0x00719bf0 CResGFF::WriteFieldINT
  - 0x006b4811 CALL 0x00719bf0 CResGFF::WriteFieldINT
  - 0x006b4830 CALL 0x0057eca0 CSWSCreature::GetMaxForcePoints
  - 0x006b4840 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b4867 CALL 0x00719b90 CResGFF::WriteFieldDWORD
  - 0x006b4892 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b48c6 indirect
  - 0x006b490b indirect
  - 0x006b4915 CALL 0x00719d40 CResGFF::WriteFieldCResRef
  - 0x006b4950 indirect
  - 0x006b495d CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b498a CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b49b7 CALL 0x00719a40 CResGFF::WriteFieldCHAR
  - 0x006b49e2 CALL 0x00719a40 CResGFF::WriteFieldCHAR
  - 0x006b4a1b CALL 0x00719b90 CResGFF::WriteFieldDWORD
  - 0x006b4a46 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4a71 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4a9c CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4ac7 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4af2 CALL 0x00719bf0 CResGFF::WriteFieldINT
  - 0x006b4b1f CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b4b4a CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4b75 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4ba0 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4bcb CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4bf6 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b4c1e CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b4c4b CALL 0x00719ce0 CResGFF::WriteFieldFLOAT
  - 0x006b4c78 CALL 0x00719ce0 CResGFF::WriteFieldFLOAT
  - 0x006b4c97 CALL 0x00719bf0 CResGFF::WriteFieldINT
  - 0x006b4caa CALL 0x006b4ea0 CSWSCreatureStats::SaveClassInfo
  - 0x006b4cb2 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006b4cfa CALL 0x0051c0e0 CServerExoApp::GetItemByGameObjectID
  - 0x006b4d37 CALL 0x007356b0
  - 0x006b4d79 CALL 0x0051c0e0 CServerExoApp::GetItemByGameObjectID
  - 0x006b4dba CALL 0x007356b0
  - 0x006b4de7 CALL 0x006d5200
  - 0x006b4e6f CALL 0x0069cdb0
  - 0x006b4e7e CALL 0x00733780 CExoString::Destructor
- Imports: none
- Indirect slots (7):
  - 0x006b4551 slot +0x98 (006b454b MOV EDX,dword ptr [EAX + 0x98])
  - 0x006b456e slot +0x9c (006b4568 MOV EAX,dword ptr [EDX + 0x9c])
  - 0x006b45c5 slot +0x98 (006b45bf MOV EAX,dword ptr [EDX + 0x98])
  - 0x006b460d slot +0x9c (006b4607 MOV EAX,dword ptr [EDX + 0x9c])
  - 0x006b48c6 slot +0xd0 (006b48c0 MOV EAX,dword ptr [EDX + 0xd0])
  - 0x006b490b slot +0xc8 (006b4905 MOV EAX,dword ptr [EDX + 0xc8])
  - 0x006b4950 slot +0xd0 (006b494a MOV EDX,dword ptr [EAX + 0xd0])
- Ledger callsites in range (2):
  - 0x006b4d37 FUN_006b3d10 -> 0x7356b0, gender PUSH 0 (constant 0)
  - 0x006b4dba FUN_006b3d10 -> 0x7356b0, gender PUSH 0 (constant 0)
- Strings: none
- Globals (3):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4a4 READ (MOV EDX,dword ptr [0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Serializes creature stats into a GFF struct: names, tag, ability bases, saves, gold, components, chemicals.
- Named pseudo-C (first 30 of 201 lines):
```c
// NAMED 0x006b3d10 CSWSCreatureStats::SaveStats evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=23 exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006b46cb) */

void CSWSCreatureStats__SaveStats(undefined4 param_1,undefined4 param_2)

{
  int *piVar1;
  undefined1 uVar2;
  undefined2 uVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  uint uVar7;
  undefined4 uVar8;
  int in_ECX;
  undefined1 *puVar9;
  undefined *puVar10;
  char *pcVar11;
  undefined1 local_160;
  undefined1 local_15e;
  undefined1 local_60;
  undefined1 local_50 [16];
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  short local_2c;
  int local_28;
```

## 0x006b4ea0 CSWSCreatureStats::SaveClassInfo

- Size 2791 B, RET 8
- Callers (1 total):
  - 0x006b3d10 CSWSCreatureStats::SaveStats (site 0x006b4caa)
- Callees (65):
  - 0x006b4ed5 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006b4ef4 CALL 0x00719740 CResGFF::AddList
  - 0x006b4f75 CALL 0x00719800 CResGFF::AddListElement
  - 0x006b4f8e CALL 0x00719bf0 CResGFF::WriteFieldINT
  - 0x006b4fa5 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b4fb9 CALL 0x00719b20 CResGFF::WriteFieldSHORT
  - 0x006b500a CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006b5029 CALL 0x00719740 CResGFF::AddList
  - 0x006b505c CALL 0x00719800 CResGFF::AddListElement
  - 0x006b5076 CALL 0x006b9600 CSWSCreatureStats::GetCreatureTalentRandomPower
  - 0x006b5089 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b50bb CALL 0x006b8800 CSWSCreatureStats::GetNumberKnownSpells
  - 0x006b50e9 CALL 0x00734270 CExoString::Format
  - 0x006b50f4 CALL 0x00734010
  - 0x006b5108 CALL 0x00719740 CResGFF::AddList
  - 0x006b513d CALL 0x00719800 CResGFF::AddListElement
  - 0x006b515c CALL 0x006b87b0 CSWSCreatureStats::GetKnownSpell
  - 0x006b516c CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b51ae CALL 0x0051c8d0
  - 0x006b51eb CALL 0x00719740 CResGFF::AddList
  - 0x006b5217 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b5235 CALL 0x006ad3b0 CSWSCreatureStats::GetLevelStats
  - 0x006b5250 CALL 0x00719800 CResGFF::AddListElement
  - 0x006b529e CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b52c6 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b52ee CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b5316 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b5380 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006b53a2 CALL 0x00734270 CExoString::Format
  - 0x006b53b0 CALL 0x00734010
  - 0x006b53c4 CALL 0x00719740 CResGFF::AddList
  - 0x006b5406 CALL 0x00719800 CResGFF::AddListElement
  - 0x006b5445 CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b5456 CALL 0x00733780 CExoString::Destructor
  - 0x006b5499 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006b54bb CALL 0x00734270 CExoString::Format
  - 0x006b54c9 CALL 0x00734010
  - 0x006b54dd CALL 0x00719740 CResGFF::AddList
  - 0x006b551f CALL 0x00719800 CResGFF::AddListElement
  - 0x006b555f CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b5570 CALL 0x00733780 CExoString::Destructor
  - 0x006b559f CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b55b4 CALL 0x00719740 CResGFF::AddList
  - 0x006b55f8 CALL 0x00719800 CResGFF::AddListElement
  - 0x006b5611 CALL 0x006f5ac0 PendingLevelUp::GetSkillDelta
  - 0x006b5624 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b563b CALL 0x00719740 CResGFF::AddList
  - 0x006b568e CALL 0x00719800 CResGFF::AddListElement
  - 0x006b56c5 CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b56fc CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b5711 CALL 0x00719740 CResGFF::AddList
  - 0x006b573e CALL 0x00719800 CResGFF::AddListElement
  - 0x006b5757 CALL 0x006b7bb0 CSWSCreatureStats::GetSkillRank
  - 0x006b576a CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b579d CALL 0x00719740 CResGFF::AddList
  - 0x006b57ed CALL 0x00719800 CResGFF::AddListElement
  - 0x006b5805 CALL 0x006bc430 CSWSCreatureStats::GetFeat_2
  - 0x006b5818 CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b5851 CALL 0x00719740 CResGFF::AddList
  - 0x006b58a5 CALL 0x00719800 CResGFF::AddListElement
  - 0x006b58de CALL 0x00719ab0 CResGFF::WriteFieldWORD
  - 0x006b5918 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b5952 CALL 0x007199d0 CResGFF::WriteFieldBYTE
  - 0x006b5966 CALL 0x00733780 CExoString::Destructor
  - 0x006b597c CALL 0x009196ee __security_check_cookie
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4a4 READ (MOV EDX,dword ptr [0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Writes the ClassList GFF: per class ID, level, known spells, and force/talent fields (likely).
- Named pseudo-C (first 30 of 218 lines):
```c
// NAMED 0x006b4ea0 CSWSCreatureStats::SaveClassInfo evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=postpass exe=6a522e71631d

void CSWSCreatureStats__SaveClassInfo(undefined4 param_1,undefined4 param_2)

{
  byte bVar1;
  undefined1 uVar2;
  byte bVar3;
  undefined2 uVar4;
  int iVar5;
  undefined4 uVar6;
  int in_ECX;
  char *pcVar7;
  undefined *puVar8;
  ushort local_140;
  byte local_134;
  byte local_12c;
  undefined1 local_10f;
  int local_10c;
  ushort local_108;
  ushort local_104;
  byte local_fd;
  undefined1 local_fc [11];
  byte local_f1;
  undefined1 local_f0 [8];
  ushort local_e8;
  byte local_e2;
  byte local_e1;
  int local_e0;
  short local_dc;
```

## 0x006b5990 CSWSCreatureStats::GetSTRStat

- Size 167 B, RET 0
- Callers (11 total, first 5 shown):
  - 0x005724d0 CSWSCreature::UpdateAttributesOnEffect (site 0x00572566)
  - 0x0058a090 CSWSCreature::DumpToLog (site 0x0058a131)
  - 0x0062f070 (site 0x0062f193)
  - 0x0063edc0 (site 0x0063f180)
  - 0x0063f550 (site 0x0064110f)
- Callees (2):
  - 0x006b59cc CALL 0x006ba9f0 CSWSCreatureStats::GetTotalSTRBonus
  - 0x006b5a13 CALL 0x006ba9f0 CSWSCreatureStats::GetTotalSTRBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns total STR: base, total STR bonus, and table entry at +0x18 (likely racial); floor of 3.
- Named pseudo-C (first 30 of 31 lines):
```c
// NAMED 0x006b5990 CSWSCreatureStats::GetSTRStat evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetSTRStat(void)

{
  char cVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  int in_ECX;
  undefined1 local_34;
  
  cVar1 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) + 0x18)
  ;
  bVar2 = *(byte *)(in_ECX + 0xed);
  cVar3 = CSWSCreatureStats__GetTotalSTRBonus();
  iVar4 = (int)cVar3;
  if ((int)((uint)bVar2 + iVar4 + (int)cVar1) < 4) {
    local_34 = '\x03';
  }
  else {
    local_34 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) +
                        0x18);
    cVar1 = *(char *)(in_ECX + 0xed);
    cVar3 = CSWSCreatureStats__GetTotalSTRBonus();
    iVar4 = (int)local_34;
    local_34 = cVar1 + cVar3 + local_34;
  }
  return CONCAT31((int3)((uint)iVar4 >> 8),local_34);
}
```

## 0x006b5a40 CSWSCreatureStats::GetCONStat

- Size 167 B, RET 0
- Callers (11 total, first 5 shown):
  - 0x005724d0 CSWSCreature::UpdateAttributesOnEffect (site 0x00572667)
  - 0x0058a090 CSWSCreature::DumpToLog (site 0x0058a181)
  - 0x0062f070 (site 0x0062f1c1)
  - 0x0063edc0 (site 0x0063f1d2)
  - 0x0063f550 (site 0x00641165)
- Callees (2):
  - 0x006b5a7c CALL 0x006baa90 CSWSCreatureStats::GetTotalCONBonus
  - 0x006b5ac3 CALL 0x006baa90 CSWSCreatureStats::GetTotalCONBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns total CON: base, total CON bonus, and table entry at +0x1d (likely racial); floor of 3.
- Named pseudo-C (first 30 of 31 lines):
```c
// NAMED 0x006b5a40 CSWSCreatureStats::GetCONStat evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetCONStat(void)

{
  char cVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  int in_ECX;
  undefined1 local_34;
  
  cVar1 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) + 0x1d)
  ;
  bVar2 = *(byte *)(in_ECX + 0xf1);
  cVar3 = CSWSCreatureStats__GetTotalCONBonus();
  iVar4 = (int)cVar3;
  if ((int)((uint)bVar2 + iVar4 + (int)cVar1) < 4) {
    local_34 = '\x03';
  }
  else {
    local_34 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) +
                        0x1d);
    cVar1 = *(char *)(in_ECX + 0xf1);
    cVar3 = CSWSCreatureStats__GetTotalCONBonus();
    iVar4 = (int)local_34;
    local_34 = cVar1 + cVar3 + local_34;
  }
  return CONCAT31((int3)((uint)iVar4 >> 8),local_34);
}
```

## 0x006b5af0 CSWSCreatureStats::GetINTStat

- Size 167 B, RET 0
- Callers (11 total, first 5 shown):
  - 0x005724d0 CSWSCreature::UpdateAttributesOnEffect (site 0x00572844)
  - 0x0058a090 CSWSCreature::DumpToLog (site 0x0058a145)
  - 0x0062f070 (site 0x0062f1d9)
  - 0x0063edc0 (site 0x0063f1f7)
  - 0x0063f550 (site 0x00641190)
- Callees (2):
  - 0x006b5b2c CALL 0x006baae0 CSWSCreatureStats::GetTotalINTBonus
  - 0x006b5b73 CALL 0x006baae0 CSWSCreatureStats::GetTotalINTBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns total INT: base, total INT bonus, and table entry at +0x1a (likely racial); floor of 3.
- Named pseudo-C (first 30 of 31 lines):
```c
// NAMED 0x006b5af0 CSWSCreatureStats::GetINTStat evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetINTStat(void)

{
  char cVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  int in_ECX;
  undefined1 local_34;
  
  cVar1 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) + 0x1a)
  ;
  bVar2 = *(byte *)(in_ECX + 0xf3);
  cVar3 = CSWSCreatureStats__GetTotalINTBonus();
  iVar4 = (int)cVar3;
  if ((int)((uint)bVar2 + iVar4 + (int)cVar1) < 4) {
    local_34 = '\x03';
  }
  else {
    local_34 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) +
                        0x1a);
    cVar1 = *(char *)(in_ECX + 0xf3);
    cVar3 = CSWSCreatureStats__GetTotalINTBonus();
    iVar4 = (int)local_34;
    local_34 = cVar1 + cVar3 + local_34;
  }
  return CONCAT31((int3)((uint)iVar4 >> 8),local_34);
}
```

## 0x006b5ba0 CSWSCreatureStats::GetWISStat

- Size 167 B, RET 0
- Callers (11 total, first 5 shown):
  - 0x005724d0 CSWSCreature::UpdateAttributesOnEffect (site 0x005728a2)
  - 0x0058a090 CSWSCreature::DumpToLog (site 0x0058a159)
  - 0x0062f070 (site 0x0062f1f1)
  - 0x0063edc0 (site 0x0063f21c)
  - 0x0063f550 (site 0x006411bb)
- Callees (2):
  - 0x006b5bdc CALL 0x006bab30 CSWSCreatureStats::GetTotalWISBonus
  - 0x006b5c23 CALL 0x006bab30 CSWSCreatureStats::GetTotalWISBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns total WIS: base, total WIS bonus, and table entry at +0x1c (likely racial); floor of 3.
- Named pseudo-C (first 30 of 31 lines):
```c
// NAMED 0x006b5ba0 CSWSCreatureStats::GetWISStat evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetWISStat(void)

{
  char cVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  int in_ECX;
  undefined1 local_34;
  
  cVar1 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) + 0x1c)
  ;
  bVar2 = *(byte *)(in_ECX + 0xf5);
  cVar3 = CSWSCreatureStats__GetTotalWISBonus();
  iVar4 = (int)cVar3;
  if ((int)((uint)bVar2 + iVar4 + (int)cVar1) < 4) {
    local_34 = '\x03';
  }
  else {
    local_34 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) +
                        0x1c);
    cVar1 = *(char *)(in_ECX + 0xf5);
    cVar3 = CSWSCreatureStats__GetTotalWISBonus();
    iVar4 = (int)local_34;
    local_34 = cVar1 + cVar3 + local_34;
  }
  return CONCAT31((int3)((uint)iVar4 >> 8),local_34);
}
```

## 0x006b5c50 CSWSCreatureStats::GetCHAStat

- Size 167 B, RET 0
- Callers (11 total, first 5 shown):
  - 0x005724d0 CSWSCreature::UpdateAttributesOnEffect (site 0x00572986)
  - 0x0058a090 CSWSCreature::DumpToLog (site 0x0058a195)
  - 0x0062f070 (site 0x0062f209)
  - 0x0063edc0 (site 0x0063f241)
  - 0x0063f550 (site 0x006411e6)
- Callees (2):
  - 0x006b5c8c CALL 0x006bab80 CSWSCreatureStats::GetTotalCHABonus
  - 0x006b5cd3 CALL 0x006bab80 CSWSCreatureStats::GetTotalCHABonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns total CHA: base, total CHA bonus, and table entry at +0x1b (likely racial); floor of 3.
- Named pseudo-C (first 30 of 31 lines):
```c
// NAMED 0x006b5c50 CSWSCreatureStats::GetCHAStat evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetCHAStat(void)

{
  char cVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  int in_ECX;
  undefined1 local_34;
  
  cVar1 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) + 0x1b)
  ;
  bVar2 = *(byte *)(in_ECX + 0xf7);
  cVar3 = CSWSCreatureStats__GetTotalCHABonus();
  iVar4 = (int)cVar3;
  if ((int)((uint)bVar2 + iVar4 + (int)cVar1) < 4) {
    local_34 = '\x03';
  }
  else {
    local_34 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) +
                        0x1b);
    cVar1 = *(char *)(in_ECX + 0xf7);
    cVar3 = CSWSCreatureStats__GetTotalCHABonus();
    iVar4 = (int)local_34;
    local_34 = cVar1 + cVar3 + local_34;
  }
  return CONCAT31((int3)((uint)iVar4 >> 8),local_34);
}
```

## 0x006b5db0 CSWSCreatureStats::GetDEXMod

- Size 299 B, RET 4
- Callers (15 total, first 5 shown):
  - 0x0057e530 CSWSCreature::GetArmorClass (site 0x0057e856)
  - 0x0062f070 (site 0x0062f317)
  - 0x00646270 (site 0x00646504)
  - 0x00679b00 Global::ExecuteCommandGetAbilityModifier (site 0x00679bbe)
  - 0x006a5110 CSWSCreatureStats::GetArmorClassVersus (site 0x006a5863)
- Callees (5):
  - 0x006b5de6 CALL 0x006daa00 CSWSCreature::IsDebilitated
  - 0x006b5e09 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006b5e21 CALL 0x006046c0 CSWSItem::ComputeArmorClass
  - 0x006b5e32 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b5e90 CALL 0x006daa00 CSWSCreature::IsDebilitated
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns DEX modifier for AC: base DEX modifier at +0xf0, likely capped by worn armor's max-DEX value.
- Named pseudo-C (first 30 of 52 lines):
```c
// NAMED 0x006b5db0 CSWSCreatureStats::GetDEXMod evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=5 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetDEXMod(int param_1)

{
  bool bVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  int in_ECX;
  undefined1 local_40;
  undefined1 local_14;
  undefined4 local_c;
  
  local_c = 0;
  bVar1 = false;
  bVar2 = false;
  if (param_1 == 1) {
    iVar3 = CSWSCreature__IsDebilitated(1);
    if (iVar3 == 0) {
      iVar3 = CSWInventory__GetItemInSlot(2);
      if (iVar3 != 0) {
        bVar1 = true;
        iVar4 = CSWSItem__ComputeArmorClass();
        if (0 < iVar4) {
          iVar4 = CSWItem__GetBaseItem();
          local_c = (uint)*(byte *)(iVar4 + 0xad) + *(int *)(iVar3 + 0x2ec);
          bVar2 = true;
        }
      }
```

## 0x006b5ee0 CSWSCreatureStats::GetRangedAttackBonus

- Size 1809 B, RET 24
- Callers (1 total):
  - 0x008ad930 Equipment panel slot refresh (per frame from 0x8ABA50 via 0x8ABA8C): top placeholder resets ctl0/ctl9, slot loop k=[EBP-0x28] 0..10, slot image ctl = this+0x14D0+k*0x148 (site 0x008ade6f)
- Callees (37):
  - 0x006b5f05 CALL 0x006a8f10 CSWSCreatureStats::GetWeaponSpecialization
  - 0x006b5f16 CALL 0x006a8fe0
  - 0x006b5f76 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b5f9e CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b5fc6 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b5fec CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6017 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6042 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b606d CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6098 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b60c0 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b60e8 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6110 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b614c CALL 0x006d5200
  - 0x006b6180 CALL 0x006d5200
  - 0x006b61ab CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b61f4 CALL 0x0091c860
  - 0x006b622b CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006b6269 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b6286 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b62e3 CALL 0x0091c860
  - 0x006b634a CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b635e CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6386 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b639a CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b63c2 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b63d6 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6401 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6429 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6451 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b6477 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b64a2 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b64ca CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b64f2 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b651a CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b653f CALL 0x006b8ee0 CSWSCreatureStats::GetWeaponSpecialization
  - 0x006b6596 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes an attack bonus from weapon specialization, a monk-ladder value and feat checks; adds STR modifier when first arg set.
- Named pseudo-C (first 30 of 247 lines):
```c
// NAMED 0x006b5ee0 CSWSCreatureStats::GetRangedAttackBonus evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=7 exe=6a522e71631d

void CSWSCreatureStats__GetRangedAttackBonus
               (int param_1,int param_2,uint *param_3,int *param_4,int param_5,int param_6)

{
  uint uVar1;
  int iVar2;
  int in_ECX;
  int local_38 [2];
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  uint local_1c;
  uint local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  uint local_8;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    local_8 = CSWSCreatureStats__GetWeaponSpecialization();
    local_8 = local_8 & 0xff;
    uVar1 = CSWSCreatureStats__GetUseMonkAttackTables();
    local_c = (uVar1 & 0xff) * local_8;
    if (param_1 != 0) {
      local_8 = (int)*(char *)(in_ECX + 0xee) + local_8;
      local_c = *(char *)(in_ECX + 0xee) + local_c;
```

## 0x006b6600 CSWSCreatureStats::GetTotalACFeatMod

- Size 243 B, RET 0
- Callers (2 total):
  - 0x0057e530 CSWSCreature::GetArmorClass (site 0x0057e877)
  - 0x006a5110 CSWSCreatureStats::GetArmorClassVersus (site 0x006a6ef3)
- Callees (6):
  - 0x006b6627 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006b6649 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006b6667 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b66b7 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b66cb CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b66df CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns 0-3 AC bonus from three feat IDs (likely defense style), gated on hand-slot items.
- Named pseudo-C (first 30 of 48 lines):
```c
// NAMED 0x006b6600 CSWSCreatureStats::GetTotalACFeatMod evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=4 exe=6a522e71631d

undefined1 CSWSCreatureStats__GetTotalACFeatMod(void)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  undefined1 local_d;
  
  local_d = 0;
  iVar2 = CSWInventory__GetItemInSlot(0x10);
  iVar3 = CSWInventory__GetItemInSlot(0x20);
  bVar1 = false;
  if ((iVar2 == 0) || (iVar3 != 0)) {
    if ((iVar2 == 0) && (iVar3 == 0)) {
      bVar1 = true;
    }
  }
  else {
    iVar2 = CSWItem__GetBaseItem();
    if (iVar2 != 0) {
      if ((*(char *)(iVar2 + 8) == '\x02') || (*(char *)(iVar2 + 8) == '\x04')) {
        bVar1 = true;
      }
    }
  }
  if (bVar1) {
    iVar2 = CSWSCreatureStats__HasFeat(0x73);
    if (iVar2 == 0) {
```

## 0x006b67d0 CSWSCreatureStats::SetSTRBase

- Size 54 B, RET 4
- Callers (5 total):
  - 0x00686c80 Global::ExecuteCommandAdjustCreatureAttributes (site 0x00686d93)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b053c)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9a77)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006b9f60)
  - 0x007ab440 (site 0x007ab518)
- Callees (2):
  - 0x006b67e6 CALL 0x006b5990 CSWSCreatureStats::GetSTRStat
  - 0x006b67f2 CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Stores STR base at +0xed, then recomputes STR modifier at +0xee via GetSTRStat and CalcStatModifier.
- Named pseudo-C (first 15 of 15 lines):
```c
// NAMED 0x006b67d0 CSWSCreatureStats::SetSTRBase evidence=ev_b55722684e9614da5eb7bb757a3852f28c437b78428c14f5bf38df62a8ad172c names=3 exe=6a522e71631d

void CSWSCreatureStats__SetSTRBase(undefined1 param_1)

{
  undefined1 uVar1;
  int in_ECX;
  
  *(undefined1 *)(in_ECX + 0xed) = param_1;
  uVar1 = CSWSCreatureStats__GetSTRStat();
  uVar1 = CSWSCreatureStats__CalcStatModifier(uVar1);
  *(undefined1 *)(in_ECX + 0xee) = uVar1;
  return;
}

```

## 0x006b6810 CSWSCreatureStats::SetDEXBase

- Size 54 B, RET 4
- Callers (5 total):
  - 0x00686c80 Global::ExecuteCommandAdjustCreatureAttributes (site 0x00686dcc)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b057a)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9a98)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006b9f81)
  - 0x007ab440 (site 0x007ab536)
- Callees (2):
  - 0x006b6826 CALL 0x006b5d00
  - 0x006b6832 CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Stores DEX base at +0xef, then recomputes DEX modifier at +0xf0 via stat getter and CalcStatModifier.
- Named pseudo-C (first 15 of 15 lines):
```c
// NAMED 0x006b6810 CSWSCreatureStats::SetDEXBase evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=2 exe=6a522e71631d

void CSWSCreatureStats__SetDEXBase(undefined1 param_1)

{
  undefined1 uVar1;
  int in_ECX;
  
  *(undefined1 *)(in_ECX + 0xef) = param_1;
  uVar1 = FUN_006b5d00();
  uVar1 = CSWSCreatureStats__CalcStatModifier(uVar1);
  *(undefined1 *)(in_ECX + 0xf0) = uVar1;
  return;
}

```

## 0x006b6850 CSWSCreatureStats::SetCONBase

- Size 207 B, RET 8
- Callers (5 total):
  - 0x00686c80 Global::ExecuteCommandAdjustCreatureAttributes (site 0x00686e07)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b0636)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9ab8)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006b9fa1)
  - 0x007ab440 (site 0x007ab553)
- Callees (6):
  - 0x006b688c indirect
  - 0x006b6895 CALL 0x006b5a40 CSWSCreatureStats::GetCONStat
  - 0x006b68a1 CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
  - 0x006b68f0 indirect
  - 0x006b6907 indirect
  - 0x006b6912 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
- Imports: none
- Indirect slots (3):
  - 0x006b688c slot +0x98 (006b6886 MOV EDX,dword ptr [EAX + 0x98])
  - 0x006b68f0 slot +0x98 (006b68ea MOV EAX,dword ptr [EDX + 0x98])
  - 0x006b6907 slot +0x9c (006b6901 MOV EAX,dword ptr [EDX + 0x9c])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Stores CON base at +0xf1, recomputes CON modifier at +0xf2, and shifts current HP when flag is set.
- Named pseudo-C (first 28 of 28 lines):
```c
// NAMED 0x006b6850 CSWSCreatureStats::SetCONBase evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=4 exe=6a522e71631d

void CSWSCreatureStats__SetCONBase(undefined1 param_1,int param_2)

{
  int *piVar1;
  undefined1 uVar2;
  short sVar3;
  short sVar4;
  int in_ECX;
  short local_8;
  
  *(undefined1 *)(in_ECX + 0xf1) = param_1;
  if (param_2 == 1) {
    local_8 = (**(code **)(**(int **)(in_ECX + 0x24) + 0x98))(1);
  }
  uVar2 = CSWSCreatureStats__GetCONStat();
  uVar2 = CSWSCreatureStats__CalcStatModifier(uVar2);
  *(undefined1 *)(in_ECX + 0xf2) = uVar2;
  if (param_2 == 1) {
    piVar1 = *(int **)(in_ECX + 0x24);
    sVar3 = (**(code **)(**(int **)(in_ECX + 0x24) + 0x98))(1);
    sVar4 = (**(code **)(*piVar1 + 0x9c))(1);
    CSWSObject__SetCurrentHitPoints((int)sVar4 + ((int)sVar3 - (int)local_8));
  }
  return;
}

```

## 0x006b6920 CSWSCreatureStats::SetINTBase

- Size 62 B, RET 4
- Callers (5 total):
  - 0x00686c80 Global::ExecuteCommandAdjustCreatureAttributes (site 0x00686e40)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b05b8)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9ad6)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006b9fbf)
  - 0x007ab440 (site 0x007ab56e)
- Callees (3):
  - 0x006b6936 CALL 0x006b5af0 CSWSCreatureStats::GetINTStat
  - 0x006b6942 CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
  - 0x006b6953 CALL 0x006bce00 CSWSCreatureStats::AdjustSpellUsesPerDay
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Stores INT base at +0xf3, recomputes INT modifier at +0xf4, then calls AdjustSpellUsesPerDay.
- Named pseudo-C (first 16 of 16 lines):
```c
// NAMED 0x006b6920 CSWSCreatureStats::SetINTBase evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=4 exe=6a522e71631d

void CSWSCreatureStats__SetINTBase(undefined1 param_1)

{
  undefined1 uVar1;
  int in_ECX;
  
  *(undefined1 *)(in_ECX + 0xf3) = param_1;
  uVar1 = CSWSCreatureStats__GetINTStat();
  uVar1 = CSWSCreatureStats__CalcStatModifier(uVar1);
  *(undefined1 *)(in_ECX + 0xf4) = uVar1;
  CSWSCreatureStats__AdjustSpellUsesPerDay();
  return;
}

```

## 0x006b6960 CSWSCreatureStats::SetWISBase

- Size 62 B, RET 4
- Callers (5 total):
  - 0x00686c80 Global::ExecuteCommandAdjustCreatureAttributes (site 0x00686e76)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b05f6)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9af4)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006b9fdd)
  - 0x007ab440 (site 0x007ab589)
- Callees (3):
  - 0x006b6976 CALL 0x006b5ba0 CSWSCreatureStats::GetWISStat
  - 0x006b6982 CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
  - 0x006b6993 CALL 0x006bce00 CSWSCreatureStats::AdjustSpellUsesPerDay
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Stores WIS base at +0xf5, recomputes WIS modifier at +0xf6, then calls AdjustSpellUsesPerDay.
- Named pseudo-C (first 16 of 16 lines):
```c
// NAMED 0x006b6960 CSWSCreatureStats::SetWISBase evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=4 exe=6a522e71631d

void CSWSCreatureStats__SetWISBase(undefined1 param_1)

{
  undefined1 uVar1;
  int in_ECX;
  
  *(undefined1 *)(in_ECX + 0xf5) = param_1;
  uVar1 = CSWSCreatureStats__GetWISStat();
  uVar1 = CSWSCreatureStats__CalcStatModifier(uVar1);
  *(undefined1 *)(in_ECX + 0xf6) = uVar1;
  CSWSCreatureStats__AdjustSpellUsesPerDay();
  return;
}

```

## 0x006b69a0 CSWSCreatureStats::SetCHABase

- Size 62 B, RET 4
- Callers (5 total):
  - 0x00686c80 Global::ExecuteCommandAdjustCreatureAttributes (site 0x00686eac)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b0674)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9b12)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006b9ffb)
  - 0x007ab440 (site 0x007ab5a4)
- Callees (3):
  - 0x006b69b6 CALL 0x006b5c50 CSWSCreatureStats::GetCHAStat
  - 0x006b69c2 CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
  - 0x006b69d3 CALL 0x006bce00 CSWSCreatureStats::AdjustSpellUsesPerDay
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Stores CHA base at +0xf7, recomputes CHA modifier at +0xf8, then calls AdjustSpellUsesPerDay.
- Named pseudo-C (first 16 of 16 lines):
```c
// NAMED 0x006b69a0 CSWSCreatureStats::SetCHABase evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=4 exe=6a522e71631d

void CSWSCreatureStats__SetCHABase(undefined1 param_1)

{
  undefined1 uVar1;
  int in_ECX;
  
  *(undefined1 *)(in_ECX + 0xf7) = param_1;
  uVar1 = CSWSCreatureStats__GetCHAStat();
  uVar1 = CSWSCreatureStats__CalcStatModifier(uVar1);
  *(undefined1 *)(in_ECX + 0xf8) = uVar1;
  CSWSCreatureStats__AdjustSpellUsesPerDay();
  return;
}

```

## 0x006b69e0 CSWSCreatureStats::SetClass

- Size 53 B, RET 8
- Callers (5 total):
  - 0x006a4260 CSWSCreatureStats::Constructor (site 0x006a444c)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b1cc7)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b99c1)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006b9edb)
  - 0x006bdcd0 CSWSCreatureStats::AddClass (site 0x006bdd12)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Sets class ID in slot N (40-byte stride at +0xab) if N is below the class count.
- Named pseudo-C (first 13 of 13 lines):
```c
// NAMED 0x006b69e0 CSWSCreatureStats::SetClass evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=1 exe=6a522e71631d

void CSWSCreatureStats__SetClass(byte param_1,undefined1 param_2)

{
  int in_ECX;
  
  if (param_1 < *(byte *)(in_ECX + 0x8d)) {
    *(undefined1 *)(in_ECX + 0xab + (uint)param_1 * 0x28) = param_2;
  }
  return;
}

```

## 0x006b6a20 CSWSCreatureStats::SetClassLevel

- Size 129 B, RET 8
- Callers (5 total):
  - 0x006a4260 CSWSCreatureStats::Constructor (site 0x006a4440)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b1d6e)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9970)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006b9e4f)
  - 0x006bdcd0 CSWSCreatureStats::AddClass (site 0x006bdd2d)
- Callees (2):
  - 0x006b6a80 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b6a93 CALL 0x004010a0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Sets class level in slot N (+0xac, 40-byte stride); likely notifies when total level is 30 or more.
- Named pseudo-C (first 17 of 17 lines):
```c
// NAMED 0x006b6a20 CSWSCreatureStats::SetClassLevel evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=2 exe=6a522e71631d

void CSWSCreatureStats__SetClassLevel(byte param_1,undefined1 param_2)

{
  byte bVar1;
  int in_ECX;
  
  if ((((param_1 < *(byte *)(in_ECX + 0x8d)) &&
       (*(undefined1 *)(in_ECX + 0xac + (uint)param_1 * 0x28) = param_2,
       *(int *)(in_ECX + 0x24) != 0)) && (*(int *)(*(int *)(in_ECX + 0x24) + 0x1200) != 0)) &&
     (bVar1 = CSWSCreatureStats__GetLevel(0), 0x1d < bVar1)) {
    FUN_004010a0(0x2a);
  }
  return;
}

```

## 0x006b6ab0 CSWSCreatureStats::SetClassNegativeLevels

- Size 53 B, RET 8
- Callers (2 total):
  - 0x005a65d0 CSWSEffectListHandler::OnApplyNegativeLevel (site 0x005a6a93)
  - 0x005a6c60 CSWSEffectListHandler::OnRemoveNegativeLevel (site 0x005a6cda)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Sets negative-level byte in slot N (+0xad, 40-byte stride) if N is below the class count.
- Named pseudo-C (first 13 of 13 lines):
```c
// NAMED 0x006b6ab0 CSWSCreatureStats::SetClassNegativeLevels evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=1 exe=6a522e71631d

void CSWSCreatureStats__SetClassNegativeLevels(byte param_1,undefined1 param_2)

{
  int in_ECX;
  
  if (param_1 < *(byte *)(in_ECX + 0x8d)) {
    *(undefined1 *)(in_ECX + 0xad + (uint)param_1 * 0x28) = param_2;
  }
  return;
}

```

## 0x006b6af0 CSWSCreatureStats::CalcStatModifier

- Size 48 B, RET 4
- Callers (8 total, first 5 shown):
  - 0x005724d0 CSWSCreature::UpdateAttributesOnEffect (site 0x00572572)
  - 0x006b67d0 CSWSCreatureStats::SetSTRBase (site 0x006b67f2)
  - 0x006b6810 CSWSCreatureStats::SetDEXBase (site 0x006b6832)
  - 0x006b6850 CSWSCreatureStats::SetCONBase (site 0x006b68a1)
  - 0x006b6920 CSWSCreatureStats::SetINTBase (site 0x006b6942)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Converts ability score to modifier: (score minus 10) halved for 10+, (score minus 11) halved below 10.
- Named pseudo-C (first 16 of 16 lines):
```c
// NAMED 0x006b6af0 CSWSCreatureStats::CalcStatModifier evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=1 exe=6a522e71631d

int CSWSCreatureStats__CalcStatModifier(byte param_1)

{
  int iVar1;
  
  if (param_1 < 10) {
    iVar1 = param_1 - 0xb;
  }
  else {
    iVar1 = param_1 - 10;
  }
  return iVar1 / 2;
}

```

## 0x006b6b30 CSWSCreatureStats::GetTag

- Size 20 B, RET 0
- Callers (3 total):
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b0159)
  - 0x006b3d10 CSWSCreatureStats::SaveStats (site 0x006b3dbe)
  - 0x006be150 CSWSCreatureStats::CanChooseFeat (site 0x006be1e4)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns address of the tag field at +0x18 inside the owning object (pointer at +0x24).
- Named pseudo-C (first 10 of 10 lines):
```c
// NAMED 0x006b6b30 CSWSCreatureStats::GetTag evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=1 exe=6a522e71631d

int CSWSCreatureStats__GetTag(void)

{
  int in_ECX;
  
  return *(int *)(in_ECX + 0x24) + 0x18;
}

```

## 0x006b6b50 CSWSCreatureStats::SetTag

- Size 124 B, RET 4
- Callers (1 total):
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b019e)
- Callees (3):
  - 0x006b6b88 CALL 0x00734690 CExoString::ToLower
  - 0x006b6ba7 CALL 0x007337c0 CExoString::operator=
  - 0x006b6bb6 CALL 0x00733780 CExoString::Destructor
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Likely sets the owning creature's tag to a lowercased copy of the argument string.
- Named pseudo-C (first 26 of 26 lines):
```c
// NAMED 0x006b6b50 CSWSCreatureStats::SetTag evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=2 exe=6a522e71631d

void CSWSCreatureStats__SetTag(void)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00974098;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  uVar2 = CExoString__ToLower(local_18);
  local_8 = 0;
  CExoString__operator_(uVar2);
  local_8 = 0xffffffff;
  CExoString__Destructor(uVar1);
  ExceptionList = local_10;
  return;
}

```

## 0x006b6bd0 CSWSCreatureStats::SetExperience

- Size 1167 B, RET 8
- Callers (4 total):
  - 0x00656af0 (site 0x006570cc)
  - 0x00692b80 Global::ExecuteCommandSetXP (site 0x00692c33)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b0d1d)
  - 0x006b7060 CSWSCreatureStats::AddExperience (site 0x006b72d4)
- Callees (36):
  - 0x006b6c20 CALL 0x0051c600 CServerExoApp::GetClientObjectByObjectId
  - 0x006b6c36 CALL 0x0051c2e0 CServerExoApp::GetSWSMessage
  - 0x006b6c50 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006b6c71 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006b6c9f CALL 0x00734010
  - 0x006b6cbd CALL 0x00734010
  - 0x006b6cd6 CALL 0x00734270 CExoString::Format
  - 0x006b6ce5 CALL 0x00733780 CExoString::Destructor
  - 0x006b6d13 CALL 0x00919723 operator new
  - 0x006b6d2b CALL 0x005e2540
  - 0x006b6d61 CALL 0x005e1bd0
  - 0x006b6d7e CALL 0x00584da0 CSWSCreature::SendFeedbackMessage
  - 0x006b6dab CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b6dc5 CALL 0x0051bd40 CServerExoApp::GetServerInfo
  - 0x006b6e12 CALL 0x006ad3b0 CSWSCreatureStats::GetLevelStats
  - 0x006b6e1e CALL 0x006b9d80 CSWSCreatureStats::LevelDown
  - 0x006b6e33 CALL 0x006b9790 CSWSCreatureStats::CanLevelUp
  - 0x006b6e56 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b6e6a CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b6e9f CALL 0x006b9870 CSWSCreatureStats::LevelUp
  - 0x006b6eb6 CALL 0x006b9790 CSWSCreatureStats::CanLevelUp
  - 0x006b6ecb CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b6f13 CALL 0x00584da0 CSWSCreature::SendFeedbackMessage
  - 0x006b6f4f CALL 0x0073f750 CClientExoApp::GetInGameGui
  - 0x006b6f85 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b6faa CALL 0x0051c890 CServerExoApp::GetGlobalVariableTable
  - 0x006b6fb1 CALL 0x00654860 CSWGlobalVariableTable::GetValueNumber
  - 0x006b6fbd CALL 0x00733780 CExoString::Destructor
  - 0x006b6fd2 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b6ff6 CALL 0x0051c890 CServerExoApp::GetGlobalVariableTable
  - 0x006b6ffd CALL 0x00654cc0 CSWGlobalVariableTable::SetValueNumber
  - 0x006b7009 CALL 0x00733780 CExoString::Destructor
  - 0x006b7016 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b702d CALL 0x006fd8d0 CVirtualMachine::RunScript
  - 0x006b7039 CALL 0x00733780 CExoString::Destructor
  - 0x006b7048 CALL 0x00733780 CExoString::Destructor
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings (2):
  - 0x00999c28 "Tried to reduce XP of creature '%s' to '%d'. Cannot reduce XP."
  - 0x00999c68 "<NO TAG>"
- Globals (5):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4a4 READ (MOV EDX,dword ptr [0x00a1b4a4])
  - 0x00a1b4a8 READ (MOV ECX,dword ptr [0x00a1b4a8])
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Sets creature XP to a new value; logs a warning when XP would drop, then checks level changes.
- Named pseudo-C (first 30 of 135 lines):
```c
// NAMED 0x006b6bd0 CSWSCreatureStats::SetExperience evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=postpass exe=6a522e71631d

void CSWSCreatureStats__SetExperience(uint param_1,int param_2)

{
  byte bVar1;
  uint uVar2;
  undefined1 *puVar3;
  int iVar4;
  undefined4 uVar5;
  int in_ECX;
  char *pcVar6;
  undefined4 local_b4;
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  int local_50;
  undefined4 local_4c;
  undefined1 local_48 [11];
  char local_3d;
  undefined4 local_3c;
  undefined1 local_38 [8];
  char *local_30;
  int local_2c;
  undefined4 local_28;
  int local_1c;
  byte local_15;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
```

## 0x006b7060 CSWSCreatureStats::AddExperience

- Size 640 B, RET 4
- Callers (2 total):
  - 0x0058a9f0 CSWSCreature::ReceiveExperience (site 0x0058abca)
  - 0x0066c7a0 Global::ExecuteCommandAddMultiClass (site 0x0066c90c)
- Callees (8):
  - 0x006b70d2 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b70e1 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b70fb CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b710a CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b71bf CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b71dd CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b7262 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b72d4 CALL 0x006b6bd0 CSWSCreatureStats::SetExperience
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Multiclass XP award: uses the likely favored class, else the highest-level of the first three classes; then calls SetExperience.
- Named pseudo-C (first 30 of 78 lines):
```c
// NAMED 0x006b7060 CSWSCreatureStats::AddExperience evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=3 exe=6a522e71631d

void CSWSCreatureStats__AddExperience(uint param_1)

{
  byte bVar1;
  byte bVar2;
  uint uVar3;
  int in_ECX;
  uint local_48;
  byte local_2d;
  char local_23;
  char local_20;
  byte local_15;
  double local_14;
  char local_7;
  byte local_6;
  byte local_5;
  
  if (1 < *(byte *)(in_ECX + 0x8d)) {
    local_5 = 0;
    local_14 = 0.0;
    local_7 = *(char *)((uint)*(ushort *)(in_ECX + 0xe0) * 0x34 + *(int *)(DAT_00a1b4d0 + 0x128) +
                       0x1f);
    if (local_7 == -1) {
      bVar1 = CSWSCreatureStats__GetClassLevel(0,0);
      bVar2 = CSWSCreatureStats__GetClassLevel(1,0);
      local_15 = bVar1 < bVar2;
      bVar1 = CSWSCreatureStats__GetClassLevel(local_15,0);
      bVar2 = CSWSCreatureStats__GetClassLevel(2,0);
```

## 0x006b72e0 CSWSCreatureStats::GetExpNeededForLevelUp

- Size 46 B, RET 0
- Callers (5 total):
  - 0x00656af0 (site 0x006570c0)
  - 0x0066c7a0 Global::ExecuteCommandAddMultiClass (site 0x0066c83f)
  - 0x0074ee90 (site 0x0074f2b2)
  - 0x007ac1e0 Console addlevel handler (site 0x007ac33b)
  - 0x0084e6f0 CSWGuiInGameCharacter::SetStats (site 0x0084f150)
- Callees (1):
  - 0x006b72ee CALL 0x006a4e00 CSWSCreatureStats::GetLevel
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns XP needed for the next level, read from the XP table at the current level index.
- Named pseudo-C (first 11 of 11 lines):
```c
// NAMED 0x006b72e0 CSWSCreatureStats::GetExpNeededForLevelUp evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetExpNeededForLevelUp(void)

{
  byte bVar1;
  
  bVar1 = CSWSCreatureStats__GetLevel(0);
  return *(undefined4 *)(DAT_00a1b4d0 + 0x38 + (uint)bVar1 * 4);
}

```

## 0x006b7310 CSWSCreatureStats::GetPotentialLevel

- Size 132 B, RET 0
- Callers (1 total):
  - 0x0058a9f0 CSWSCreature::ReceiveExperience (site 0x0058abf0)
- Callees (2):
  - 0x006b731f CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b7364 CALL 0x0051bd40 CServerExoApp::GetServerInfo
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (3):
  - 0x00a1b4a4 READ (MOV ECX,dword ptr [0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns the level reached by current XP, stepping up thresholds, capped at server max level (when flag set).
- Named pseudo-C (first 25 of 25 lines):
```c
// NAMED 0x006b7310 CSWSCreatureStats::GetPotentialLevel evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=3 exe=6a522e71631d

byte CSWSCreatureStats__GetPotentialLevel(void)

{
  int iVar1;
  int in_ECX;
  uint local_c;
  byte local_5;
  
  local_5 = CSWSCreatureStats__GetLevel(0);
  if (*(int *)(in_ECX + 0x6c) == 1) {
    local_c = *(uint *)(DAT_00a1b4d0 + 0x38 + (uint)local_5 * 4);
    while (local_c <= *(uint *)(in_ECX + 0x68)) {
      iVar1 = CServerExoApp__GetServerInfo();
      if (*(int *)(iVar1 + 0x94) <= (int)(uint)local_5) {
        return local_5;
      }
      local_5 = local_5 + 1;
      local_c = *(uint *)(DAT_00a1b4d0 + 0x38 + (uint)local_5 * 4);
    }
  }
  return local_5;
}

```

## 0x006b73a0 CSWSCreatureStats::GetBaseFortSavingThrow

- Size 542 B, RET 0
- Callers (1 total):
  - 0x006ba620 CSWSCreatureStats::GetFortSavingThrow (site 0x006ba68f)
- Callees (9):
  - 0x006b7449 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b7455 CALL 0x006f0920
  - 0x006b74cd CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b74e8 CALL 0x0071b670
  - 0x006b74f7 CALL 0x00733780 CExoString::Destructor
  - 0x006b7552 CALL 0x0091c860
  - 0x006b7565 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b757f CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7599 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes base Fortitude save by summing per-class save values by class level, plus likely feat bonuses.
- Named pseudo-C (first 30 of 82 lines):
```c
// NAMED 0x006b73a0 CSWSCreatureStats::GetBaseFortSavingThrow evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=3 exe=6a522e71631d

char CSWSCreatureStats__GetBaseFortSavingThrow(void)

{
  undefined1 uVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  int in_ECX;
  byte local_37;
  undefined1 local_34 [8];
  undefined4 local_2c;
  char local_25;
  undefined1 local_24 [4];
  uint local_20;
  char local_19;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c1d8;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_19 = '\0';
  for (local_14 = 0; (int)local_14 < (int)(uint)*(byte *)(in_ECX + 0x8d); local_14 = local_14 + 1) {
```

## 0x006b75c0 CSWSCreatureStats::GetBaseWillSavingThrow

- Size 542 B, RET 0
- Callers (1 total):
  - 0x006ba6b0 CSWSCreatureStats::GetWillSavingThrow (site 0x006ba71f)
- Callees (9):
  - 0x006b7669 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b7675 CALL 0x006f0990
  - 0x006b76ed CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b7708 CALL 0x0071b670
  - 0x006b7717 CALL 0x00733780 CExoString::Destructor
  - 0x006b7772 CALL 0x0091c860
  - 0x006b7785 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b779f CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b77b9 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes base Will save by summing per-class save values by class level, plus likely feat bonuses.
- Named pseudo-C (first 30 of 82 lines):
```c
// NAMED 0x006b75c0 CSWSCreatureStats::GetBaseWillSavingThrow evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=3 exe=6a522e71631d

char CSWSCreatureStats__GetBaseWillSavingThrow(void)

{
  undefined1 uVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  int in_ECX;
  byte local_37;
  undefined1 local_34 [8];
  undefined4 local_2c;
  char local_25;
  undefined1 local_24 [4];
  uint local_20;
  int local_1c;
  char local_15;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c1d8;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_15 = '\0';
  for (local_14 = 0; (int)local_14 < (int)(uint)*(byte *)(in_ECX + 0x8d); local_14 = local_14 + 1) {
```

## 0x006b77e0 CSWSCreatureStats::GetBaseReflexSavingThrow

- Size 542 B, RET 0
- Callers (1 total):
  - 0x006ba740 CSWSCreatureStats::GetReflexSavingThrow (site 0x006ba7ac)
- Callees (9):
  - 0x006b7889 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b7895 CALL 0x006f0950
  - 0x006b790d CALL 0x00733570 CExoString::CStrConstructor
  - 0x006b7928 CALL 0x0071b670
  - 0x006b7937 CALL 0x00733780 CExoString::Destructor
  - 0x006b7992 CALL 0x0091c860
  - 0x006b79a5 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b79bf CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b79d9 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Computes base Reflex save by summing per-class save values by class level, plus likely feat bonuses.
- Named pseudo-C (first 30 of 82 lines):
```c
// NAMED 0x006b77e0 CSWSCreatureStats::GetBaseReflexSavingThrow evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=3 exe=6a522e71631d

char CSWSCreatureStats__GetBaseReflexSavingThrow(void)

{
  undefined1 uVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  int in_ECX;
  byte local_37;
  undefined1 local_34 [8];
  undefined4 local_2c;
  char local_25;
  undefined1 local_24 [4];
  uint local_20;
  int local_1c;
  char local_15;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c1d8;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_15 = '\0';
  for (local_14 = 0; (int)local_14 < (int)(uint)*(byte *)(in_ECX + 0x8d); local_14 = local_14 + 1) {
```

## 0x006b7a00 CSWSCreatureStats::GetCanUseSkill

- Size 423 B, RET 4
- Callers (11 total, first 5 shown):
  - 0x005479c0 (site 0x005479fb)
  - 0x00547a90 (site 0x00547acb)
  - 0x00568020 (site 0x0056803a)
  - 0x005680d0 CSWSCreature::AddMineActions (site 0x005682f0)
  - 0x0057b280 CSWSCreature::UseSkill (site 0x0057b2fa)
- Callees (6):
  - 0x006b7a67 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006b7a7c CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006b7a91 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006b7aae CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7b69 CALL 0x006f24d0
  - 0x006b7b8e CALL 0x006b7bb0 CSWSCreatureStats::GetSkillRank
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (3):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Likely checks skill usability: skill 2 passes via certain spells or a feat, else skill rank.
- Named pseudo-C (first 30 of 53 lines):
```c
// NAMED 0x006b7a00 CSWSCreatureStats::GetCanUseSkill evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=4 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetCanUseSkill(byte param_1)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int in_ECX;
  byte local_6;
  byte local_5;
  
  if (((param_1 == 100) || (param_1 == 0x65)) || (param_1 == 0x66)) {
    local_5 = 1;
  }
  else {
    local_5 = param_1;
  }
  if (local_5 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
    if (local_5 == 2) {
      iVar2 = CSWSCreatureStats__HasSpell(0,0x9c,0);
      if (((iVar2 != 0) || (iVar2 = CSWSCreatureStats__HasSpell(0,0x9d,0), iVar2 != 0)) ||
         (iVar2 = CSWSCreatureStats__HasSpell(0,0x9e,0), iVar2 != 0)) {
        return 1;
      }
      iVar2 = CSWSCreatureStats__HasFeat(0xc9);
      if (iVar2 != 0) {
        return 1;
      }
    }
```

## 0x006b7bb0 CSWSCreatureStats::GetSkillRank

- Size 1510 B, RET 12
- Callers (43 total, first 5 shown):
  - 0x0056a230 CSWSCreature::HandleHPRegeneration (site 0x0056a4da)
  - 0x0056c310 CSWSCreature::UpdateMineCheck (site 0x0056c760)
  - 0x00571570 CSWSCreature::MaxAttackRange (site 0x00571617)
  - 0x00577fa0 CSWSCreature::DoListenDetection (site 0x005784d4)
  - 0x00578850 CSWSCreature::DoSpotDetection (site 0x00578a2f)
- Callees (33):
  - 0x006b7c18 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006b7c8f CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
  - 0x006b7cf0 CALL 0x00585420
  - 0x006b7d17 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006b7d31 CALL 0x00585420
  - 0x006b7e3e CALL 0x0051c160 CServerExoApp::GetPlaceableByGameObjectID
  - 0x006b7e66 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7e7f CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7e98 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7ecf CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7ee8 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7f01 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7f1d CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7f36 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7f4f CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7f6d CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006b7f8d CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006b7fa9 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7fc2 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7fdb CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b7ff7 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b8010 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b8029 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b8064 CALL 0x004010a0
  - 0x006b8076 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b808f CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b80a8 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b80c4 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b80dd CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b80f6 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b810f CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b8128 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b8141 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (4):
  - 0x00a1b4a4 READ (MOV ECX,dword ptr [0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched (1):
  - 0x12c CSWRules::skills table (rows 0x20; +8 Category, +0xC MaxCR) (table) R
- KDB purpose: Likely returns effective skill rank: stored rank plus effect bonuses and an ability modifier, or 0 if unranked.
- Named pseudo-C (first 30 of 251 lines):
```c
// NAMED 0x006b7bb0 CSWSCreatureStats::GetSkillRank evidence=ev_5294998f51cd5b0312a35ad30e8d45c7da87961f20bdc959e5992420c7469d1c names=5 exe=6a522e71631d

undefined1 CSWSCreatureStats__GetSkillRank(byte param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  int iVar2;
  int in_ECX;
  undefined1 local_64;
  undefined1 local_60;
  int local_5c;
  undefined4 local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  if (param_1 < *(byte *)(DAT_00a1b4d0 + 0x123)) {
    local_8 = (int)*(char *)(*(int *)(in_ECX + 0x174) + (uint)param_1);
    if (param_3 == 0) {
      if (((param_1 == 2) && (local_8 == 0)) &&
         (iVar2 = CSWSCreatureStats__HasSpell(0,0x9c,0), iVar2 != 0)) {
        local_8 = 1;
      }
      if (((*(uint *)((uint)param_1 * 0x20 + *(int *)(DAT_00a1b4d0 + 300) + 0x1c) >> 1 & 1) == 0) &&
         (local_8 == 0)) {
        return 0;
      }
      local_18 = 0;
      local_14 = CSWSCreature__GetTotalEffectBonus(5,param_2,&local_18,0,0,0,0,param_1,0xff,0,0);
```

## 0x006b81c0 CSWSCreatureStats::SetSkillRank

- Size 150 B, RET 8
- Callers (6 total, first 5 shown):
  - 0x00686ee0 Global::ExecuteCommandAdjustCreatureSkills (site 0x00686fd8)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b394c)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9b8b)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006ba075)
  - 0x007ab290 (site 0x007ab335)
- Callees (5):
  - 0x006b8218 CALL 0x004010a0
  - 0x006b8224 CALL 0x004010a0
  - 0x006b8230 CALL 0x004010a0
  - 0x006b823c CALL 0x004010a0
  - 0x006b8248 CALL 0x004010a0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Sets a skill's stored rank; rank 30 or more for five skills calls a per-skill notification helper.
- Named pseudo-C (first 29 of 29 lines):
```c
// NAMED 0x006b81c0 CSWSCreatureStats::SetSkillRank evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=1 exe=6a522e71631d

void CSWSCreatureStats__SetSkillRank(byte param_1,char param_2)

{
  int in_ECX;
  
  if ((param_1 < *(byte *)(DAT_00a1b4d0 + 0x123)) &&
     (*(char *)((uint)param_1 + *(int *)(in_ECX + 0x174)) = param_2, '\x1d' < param_2)) {
    switch(param_1) {
    case 0:
      FUN_004010a0(0x2e);
      break;
    case 1:
      FUN_004010a0(0x2c);
      break;
    case 2:
      FUN_004010a0(0x2b);
      break;
    case 5:
      FUN_004010a0(0x2d);
      break;
    case 7:
      FUN_004010a0(0x2f);
    }
  }
  return;
}

```

## 0x006b8280 CSWSCreatureStats::AddFeat

- Size 229 B, RET 4
- Callers (5 total):
  - 0x006884b0 Global::ExecuteCommandGrantAbility (site 0x00688549)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b3417)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9bed)
  - 0x006bedf0 CSWSCreatureStats::AutoLevelup (site 0x006bf61b)
  - 0x008c0ac0 (site 0x008c0bb8)
- Callees (4):
  - 0x006b8294 CALL 0x006a20f0
  - 0x006b82af CALL 0x006c0a70 CExoArrayList<ushort>::AddUnique (stats feat list append)
  - 0x006b832e CALL 0x00919723 operator new
  - 0x006b835a CALL 0x0083ea60 CExoArrayList<uint32>::Add
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Adds a feat id to the feat list and, for limited-use feats (likely), a usage entry.
- Named pseudo-C (first 30 of 33 lines):
```c
// NAMED 0x006b8280 CSWSCreatureStats::AddFeat evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=postpass exe=6a522e71631d

void CSWSCreatureStats__AddFeat(short param_1)

{
  bool bVar1;
  int iVar2;
  short *psVar3;
  int in_ECX;
  ushort local_14;
  
  iVar2 = FUN_006a20f0(param_1);
  if (iVar2 != 0) {
    CExoArrayList<ushort>::AddUnique (stats feat list append)(param_1);
    bVar1 = false;
    if (*(char *)(iVar2 + 0x33) != '\0') {
      for (local_14 = 0; (!bVar1 && ((int)(uint)local_14 < *(int *)(in_ECX + 0x10)));
          local_14 = local_14 + 1) {
        if (**(short **)(*(int *)(in_ECX + 0xc) + (uint)local_14 * 4) == param_1) {
          bVar1 = true;
        }
      }
      if (!bVar1) {
        psVar3 = (short *)operator new(4);
        *psVar3 = param_1;
        *(undefined1 *)(psVar3 + 1) = 0;
        CExoArrayList<uint32>__Add(psVar3);
      }
    }
  }
```

## 0x006b8370 CSWSCreatureStats::ClearFeats

- Size 123 B, RET 0
- Callers (1 total):
  - 0x006a49d0 CSWSCreatureStats::Destructor (site 0x006a4a33)
- Callees (3):
  - 0x006b837e CALL 0x005933f0
  - 0x006b83c1 CALL 0x0049b840
  - 0x006b83d6 CALL 0x009196fd operator delete
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Likely clears the feat list, then frees each limited-use feat entry.
- Named pseudo-C (first 19 of 19 lines):
```c
// NAMED 0x006b8370 CSWSCreatureStats::ClearFeats evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=postpass exe=6a522e71631d

void CSWSCreatureStats__ClearFeats(void)

{
  int iVar1;
  int in_ECX;
  
  FUN_005933f0(0);
  while (*(int *)(in_ECX + 0x10) != 0) {
    iVar1 = *(int *)(*(int *)(in_ECX + 0xc) + -4 + *(int *)(in_ECX + 0x10) * 4);
    FUN_0049b840(*(int *)(in_ECX + 0x10) + -1);
    if (iVar1 != 0) {
      operator delete(iVar1);
    }
  }
  return;
}

```

## 0x006b83f0 CSWSCreatureStats::HasFeat

- Size 69 B, RET 4
- Callers (63 total, first 5 shown):
  - 0x00541fd0 (site 0x005429cc)
  - 0x0056a230 CSWSCreature::HandleHPRegeneration (site 0x0056a569)
  - 0x0056a800 CSWSCreature::HandleFPRegeneration (site 0x0056ab96)
  - 0x0056ae40 CSWSCreature::HandleAlignmentInfluence (site 0x0056b1e2)
  - 0x0056d350 CSWSCreature::SetAnimation (site 0x0056d582)
- Callees (2):
  - 0x006b8408 CALL 0x006c0aa0 CExoArrayList<ushort>::Contains
  - 0x006b841c CALL 0x006c0aa0 CExoArrayList<ushort>::Contains
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns whether the creature has the feat, checking its main feat list and a second list (likely granted feats).
- Named pseudo-C (first 14 of 14 lines):
```c
// NAMED 0x006b83f0 CSWSCreatureStats::HasFeat evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__HasFeat(undefined2 param_1)

{
  int iVar1;
  
  iVar1 = CExoArrayList<ushort>__Contains(param_1);
  if ((iVar1 == 0) && (iVar1 = CExoArrayList<ushort>__Contains(param_1), iVar1 == 0)) {
    return 0;
  }
  return 1;
}

```

## 0x006b8440 CSWSCreatureStats::GetFeatRemainingUses

- Size 306 B, RET 4
- Callers (7 total, first 5 shown):
  - 0x005b96f0 CSWSCreature::AIActionCastSpell (site 0x005b9c7f)
  - 0x005d3a20 CSWSCreature::CheckItemFeatRestrictions (site 0x005d3b37)
  - 0x0063f550 (site 0x0064034d)
  - 0x0067e1b0 Global::ExecuteCommandGetHasFeat (site 0x0067e293)
  - 0x006b8840 CSWSCreatureStats::GetCreatureHasTalent (site 0x006b8b27)
- Callees (2):
  - 0x006b845c CALL 0x006a20f0
  - 0x006b8479 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns remaining uses of a limited-use feat: max uses from the feat table minus uses spent.
- Named pseudo-C (first 30 of 56 lines):
```c
// NAMED 0x006b8440 CSWSCreatureStats::GetFeatRemainingUses evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=2 exe=6a522e71631d

byte CSWSCreatureStats__GetFeatRemainingUses(short param_1)

{
  bool bVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  int in_ECX;
  byte local_2c;
  ushort local_18;
  int local_10;
  
  iVar3 = FUN_006a20f0(param_1);
  if (iVar3 == 0) {
    local_2c = 0;
  }
  else {
    iVar4 = CSWSCreatureStats__HasFeat(param_1);
    if (iVar4 == 0) {
      local_2c = 0;
    }
    else {
      bVar1 = false;
      local_10 = 0;
      for (local_18 = 0; (!bVar1 && ((int)(uint)local_18 < *(int *)(in_ECX + 0x10)));
          local_18 = local_18 + 1) {
        local_10 = *(int *)(*(int *)(in_ECX + 0xc) + (uint)local_18 * 4);
        if (**(short **)(*(int *)(in_ECX + 0xc) + (uint)local_18 * 4) == param_1) {
```

## 0x006b8580 CSWSCreatureStats::DecrementFeatRemainingUses

- Size 108 B, RET 4
- Callers (1 total):
  - 0x005b96f0 CSWSCreature::AIActionCastSpell (site 0x005bb230)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Increments the used count of every matching limited-use feat entry.
- Named pseudo-C (first 18 of 18 lines):
```c
// NAMED 0x006b8580 CSWSCreatureStats::DecrementFeatRemainingUses evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=1 exe=6a522e71631d

void CSWSCreatureStats__DecrementFeatRemainingUses(short param_1)

{
  short *psVar1;
  int in_ECX;
  ushort local_c;
  
  for (local_c = 0; (int)(uint)local_c < *(int *)(in_ECX + 0x10); local_c = local_c + 1) {
    psVar1 = *(short **)(*(int *)(in_ECX + 0xc) + (uint)local_c * 4);
    if (*psVar1 == param_1) {
      *(char *)(psVar1 + 1) = (char)psVar1[1] + '\x01';
    }
  }
  return;
}

```

## 0x006b85f0 CSWSCreatureStats::RemoveFeat

- Size 172 B, RET 4
- Callers (1 total):
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006ba0ca)
- Callees (3):
  - 0x006b8601 CALL 0x005aa100
  - 0x006b866e CALL 0x0049b840
  - 0x006b8683 CALL 0x009196fd operator delete
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Removes a feat: first calls a helper, then deletes its matching limited-use entry.
- Named pseudo-C (first 24 of 24 lines):
```c
// NAMED 0x006b85f0 CSWSCreatureStats::RemoveFeat evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=postpass exe=6a522e71631d

void CSWSCreatureStats__RemoveFeat(short param_1)

{
  short *psVar1;
  int in_ECX;
  short local_c;
  
  FUN_005aa100(param_1);
  if (0 < *(int *)(in_ECX + 0x10)) {
    local_c = (short)*(undefined4 *)(in_ECX + 0x10);
    do {
      local_c = local_c + -1;
      if (local_c < 0) {
        return;
      }
      psVar1 = *(short **)(*(int *)(in_ECX + 0xc) + local_c * 4);
    } while ((*psVar1 != param_1) || (FUN_0049b840((int)local_c), psVar1 == (short *)0x0));
    operator delete(psVar1);
  }
  return;
}

```

## 0x006b86a0 CSWSCreatureStats::AddKnownSpell

- Size 185 B, RET 8
- Callers (5 total):
  - 0x006884b0 Global::ExecuteCommandGrantAbility (site 0x00688583)
  - 0x006b3b70 CSWSCreatureStats::ReadSpellsFromGff (site 0x006b3cd1)
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9cb5)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006ba191)
  - 0x008c0910 (site 0x008c09f2)
- Callees (3):
  - 0x006b86d5 CALL 0x006c1450
  - 0x006b872b CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006b874e CALL 0x006a4030
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Adds a spell to a class's known list, if the spell exists and (when flagged) the class is force-user.
- Named pseudo-C (first 27 of 27 lines):
```c
// NAMED 0x006b86a0 CSWSCreatureStats::AddKnownSpell evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=postpass exe=6a522e71631d

void CSWSCreatureStats__AddKnownSpell(byte param_1,undefined4 param_2)

{
  int iVar1;
  int in_ECX;
  undefined1 local_12;
  
  if ((param_1 < *(byte *)(in_ECX + 0x8d)) && (iVar1 = FUN_006c1450(param_2), iVar1 != 0)) {
    if (*(int *)(in_ECX + 0x6c) != 0) {
      if (param_1 < *(byte *)(in_ECX + 0x8d)) {
        local_12 = *(undefined1 *)(in_ECX + 0xab + (uint)param_1 * 0x28);
      }
      else {
        local_12 = 0xff;
      }
      iVar1 = IsForceUserClass(classId) {3,4,5,11..16}(local_12);
      if (iVar1 == 0) {
        return;
      }
    }
    FUN_006a4030(0,param_2);
  }
  return;
}

```

## 0x006b8760 CSWSCreatureStats::RemoveKnownSpell

- Size 80 B, RET 8
- Callers (2 total):
  - 0x006b9870 CSWSCreatureStats::LevelUp (site 0x006b9c5f)
  - 0x006b9d80 CSWSCreatureStats::LevelDown (site 0x006ba13b)
- Callees (2):
  - 0x006b877e CALL 0x006c1450
  - 0x006b87a5 CALL 0x00845a40
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Removes a spell from a class's known list if the spell exists in the spell table.
- Named pseudo-C (first 14 of 14 lines):
```c
// NAMED 0x006b8760 CSWSCreatureStats::RemoveKnownSpell evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=1 exe=6a522e71631d

void CSWSCreatureStats__RemoveKnownSpell(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = FUN_006c1450(param_2);
  if (iVar1 != 0) {
    FUN_00845a40(0,param_2);
  }
  return;
}

```

## 0x006b87b0 CSWSCreatureStats::GetKnownSpell

- Size 68 B, RET 12
- Callers (4 total):
  - 0x006362f0 (site 0x006363b9)
  - 0x006415e0 (site 0x0064199e)
  - 0x006b4ea0 CSWSCreatureStats::SaveClassInfo (site 0x006b515c)
  - 0x006bcfa0 CSWSCreatureStats::GetCreatureTalentRandomKnownPower (site 0x006bd0d5)
- Callees (1):
  - 0x006b87e9 CALL 0x006a4080
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns a class's known-spell entry (likely by level and slot), or -1 if class index out of range.
- Named pseudo-C (first 17 of 17 lines):
```c
// NAMED 0x006b87b0 CSWSCreatureStats::GetKnownSpell evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetKnownSpell(byte param_1,undefined1 param_2,undefined1 param_3)

{
  undefined4 uVar1;
  int in_ECX;
  
  if (param_1 < *(byte *)(in_ECX + 0x8d)) {
    uVar1 = FUN_006a4080(param_2,param_3);
  }
  else {
    uVar1 = 0xffffffff;
  }
  return uVar1;
}

```

## 0x006b8800 CSWSCreatureStats::GetNumberKnownSpells

- Size 62 B, RET 8
- Callers (4 total):
  - 0x006362f0 (site 0x0063643f)
  - 0x006415e0 (site 0x0064196e)
  - 0x006b4ea0 CSWSCreatureStats::SaveClassInfo (site 0x006b50bb)
  - 0x006bcfa0 CSWSCreatureStats::GetCreatureTalentRandomKnownPower (site 0x006bd089)
- Callees (1):
  - 0x006b8833 CALL 0x006a4190
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns a class's known-spell count, or 0 if the class index is out of range.
- Named pseudo-C (first 17 of 17 lines):
```c
// NAMED 0x006b8800 CSWSCreatureStats::GetNumberKnownSpells evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetNumberKnownSpells(byte param_1,undefined1 param_2)

{
  undefined4 uVar1;
  int in_ECX;
  
  if (param_1 < *(byte *)(in_ECX + 0x8d)) {
    uVar1 = FUN_006a4190(param_2);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

```

## 0x006b8840 CSWSCreatureStats::GetCreatureHasTalent

- Size 863 B, RET 8
- Callers (3 total):
  - 0x0067acc0 Global::ExecuteCommandGetCreatureHasTalent (site 0x0067ad56)
  - 0x006ba970 CSWSCreatureStats::IsSpellTalentCandidate (site 0x006ba98e)
  - 0x006bc0e0 CSWSCreatureStats::GetCreatureTalentRandomFeat (site 0x006bc224)
- Callees (14):
  - 0x006b88a9 indirect
  - 0x006b88e1 indirect
  - 0x006b891c CALL 0x006c1450
  - 0x006b8957 CALL 0x006c1450
  - 0x006b89ce CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006b89e5 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006b89fe CALL 0x006b8f40 CSWSCreatureStats::GetSpellUsesLeft
  - 0x006b8a1a CALL 0x006ba8f0 CSWSCreatureStats::GetIsInSpellLikeAbilityList
  - 0x006b8a37 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006b8a50 CALL 0x006b8f40 CSWSCreatureStats::GetSpellUsesLeft
  - 0x006b8a6c CALL 0x006c0330 CSWSCreatureStats::GetHasPowerFromItem
  - 0x006b8b16 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b8b27 CALL 0x006b8440 CSWSCreatureStats::GetFeatRemainingUses
  - 0x006b8b5c CALL 0x006b7bb0 CSWSCreatureStats::GetSkillRank
- Imports: none
- Indirect slots (2):
  - 0x006b88a9 slot +0x94 (006b88a3 MOV EAX,dword ptr [EDX + 0x94])
  - 0x006b88e1 slot +0x9c (006b88db MOV EAX,dword ptr [EDX + 0x9c])
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched (3):
  - 0x24 CSWSCreatureStats::owner creature (ptr) R
  - 0x8d CSWSCreatureStats::class count (byte) R
  - 0x1114 CSWSCreature::allowed-action mask (init 0xFFFF, death 0; bit 0x2 movement, 0x4+0x80 attack, 0x8 spells in HasTalent, 0x40 client awake) (u16) R
- KDB purpose: Tests whether the creature has a given talent (spell, feat, skill or power) it can currently use.
- Named pseudo-C (first 30 of 101 lines):
```c
// NAMED 0x006b8840 CSWSCreatureStats::GetCreatureHasTalent evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=postpass exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006b8b01) */

undefined4 CSWSCreatureStats__GetCreatureHasTalent(int param_1,uint param_2)

{
  bool bVar1;
  char cVar2;
  short sVar3;
  int iVar4;
  int in_ECX;
  undefined1 local_32;
  byte local_11;
  
  if ((param_1 != -1) && (param_2 != 0xffffffff)) {
    if (((*(ushort *)(*(int *)(in_ECX + 0x24) + 0x1114) & 6) == 6) ||
       (iVar4 = (**(code **)(**(int **)(in_ECX + 0x24) + 0x94))(), iVar4 == 0)) {
      if ((*(int **)(in_ECX + 0x24))[0x46b] == 0) {
        bVar1 = false;
      }
      else {
        sVar3 = (**(code **)(**(int **)(in_ECX + 0x24) + 0x9c))(0);
        bVar1 = sVar3 < 1;
      }
      if (!bVar1) {
        if (param_1 == 0) {
          iVar4 = FUN_006c1450(param_2);
          if (iVar4 == 0) {
            return 0;
```

## 0x006b8bb0 CSWSCreatureStats::GetCreatureTalentRandom

- Size 487 B, RET 44
- Callers (1 total):
  - 0x0067ae40 Global::ExecuteCommandGetCreatureTalent (site 0x0067b027)
- Callees (5):
  - 0x006b8c2b CALL 0x0091eb8b
  - 0x006b8ca4 CALL 0x006baec0 CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility
  - 0x006b8ccc CALL 0x006bae00 CSWSCreatureStats::GetCreatureTalentRandomPower
  - 0x006b8cfc CALL 0x006bb3a0 CSWSCreatureStats::GetCreatureTalentRandomPowerFromItem
  - 0x006b8d70 CALL 0x006bc0e0 CSWSCreatureStats::GetCreatureTalentRandomFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Randomly picks a talent (power, feat, special ability, or item power) and writes its ids to output pointers.
- Named pseudo-C (first 30 of 87 lines):
```c
// NAMED 0x006b8bb0 CSWSCreatureStats::GetCreatureTalentRandom evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=5 exe=6a522e71631d

int CSWSCreatureStats__GetCreatureTalentRandom
              (undefined4 param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4,
              undefined1 *param_5,undefined4 *param_6,undefined4 *param_7,undefined1 *param_8,
              undefined4 param_9,int param_10,int param_11)

{
  bool bVar1;
  bool bVar2;
  uint uVar3;
  int iVar4;
  int local_24;
  int local_20;
  int local_10;
  
  *param_3 = 0xffffffff;
  *param_4 = 0xffffffff;
  *param_5 = 0xff;
  *param_6 = 0x7f000000;
  *param_7 = 0xffffffff;
  *param_8 = 0xff;
  local_10 = 0;
  bVar2 = false;
  bVar1 = false;
  while( true ) {
    if (local_10 != 0) {
      return local_10;
    }
    if ((bVar1) && (bVar2)) break;
```

## 0x006b8da0 CSWSCreatureStats::GetWeaponFinesse

- Size 217 B, RET 4
- Callers (2 total):
  - 0x006a7db0 CSWSCreatureStats::GetMeleeAttackBonus (site 0x006a7ed5)
  - 0x006a93a0 CSWSCreatureStats::GetAttackModifierVersus (site 0x006aa850)
- Callees (7):
  - 0x006b8dc7 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006b8de1 CALL 0x00615c80 CSWSItem::IsLightsaber
  - 0x006b8df2 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
  - 0x006b8e07 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b8e24 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b8e41 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b8e63 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns whether a finesse feat lets DEX replace strength for the given weapon (likely).
- Named pseudo-C (first 30 of 32 lines):
```c
// NAMED 0x006b8da0 CSWSCreatureStats::GetWeaponFinesse evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=5 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetWeaponFinesse(int param_1)

{
  char cVar1;
  char cVar2;
  int iVar3;
  int in_ECX;
  
  if (param_1 == 0) {
    return 0;
  }
  cVar1 = *(char *)(in_ECX + 0xee);
  cVar2 = CSWSCreatureStats__GetDEXMod(0);
  if (cVar2 <= cVar1) {
    return 0;
  }
  iVar3 = CSWSItem__IsLightsaber();
  if ((iVar3 != 0) && (iVar3 = CSWSCreatureStats__HasFeat(0xc1), iVar3 != 0)) {
    return 1;
  }
  iVar3 = CSWItem__GetBaseItem();
  if ((((*(char *)(iVar3 + 8) == '\x01') ||
       (iVar3 = CSWItem__GetBaseItem(), *(char *)(iVar3 + 8) == '\x02')) ||
      (iVar3 = CSWItem__GetBaseItem(), *(char *)(iVar3 + 8) == '\x03')) &&
     (iVar3 = CSWSCreatureStats__HasFeat(0xc2), iVar3 != 0)) {
    return 1;
  }
  return 0;
```

## 0x006b8ee0 CSWSCreatureStats::GetWeaponSpecialization

- Size 83 B, RET 4
- Callers (4 total):
  - 0x006a8370 CSWSCreatureStats::GetMeleeDamageBonus (site 0x006a85ba)
  - 0x006a8c20 CSWSCreatureStats::GetRangedDamageBonus (site 0x006a8d95)
  - 0x006abf70 CSWSCreatureStats::GetDamageBonus (site 0x006ac73a)
  - 0x006b5ee0 CSWSCreatureStats::GetRangedAttackBonus (site 0x006b653f)
- Callees (3):
  - 0x006b8ef2 CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b8efe CALL 0x006d6e30 CSWItem::GetBaseItem
  - 0x006b8f24 CALL 0x006b83f0 CSWSCreatureStats::HasFeat
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns whether the creature has the specialization feat tied to the item's base type.
- Named pseudo-C (first 18 of 18 lines):
```c
// NAMED 0x006b8ee0 CSWSCreatureStats::GetWeaponSpecialization evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=3 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetWeaponSpecialization(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((param_1 != 0) && (iVar1 = CSWItem__GetBaseItem(), iVar1 != 0)) {
    iVar1 = CSWItem__GetBaseItem();
    if (*(short *)(iVar1 + 0xb0) != 0) {
      uVar2 = CSWSCreatureStats__HasFeat(*(short *)(iVar1 + 0xb0));
      return uVar2;
    }
  }
  return 0;
}

```

## 0x006b8f40 CSWSCreatureStats::GetSpellUsesLeft

- Size 620 B, RET 16
- Callers (6 total, first 5 shown):
  - 0x00565310 CSWSCreature::AddCastSpellActions (site 0x005654e7)
  - 0x0065be70 (site 0x0065d533)
  - 0x0066e7e0 Global::ExecuteCommandCastSpell (site 0x0066edf2)
  - 0x00670d10 Global::ExecuteCommandDoDoorAction (site 0x00671011)
  - 0x0067fa20 Global::ExecuteCommandGetIsDoorActionPossible (site 0x0067fd35)
- Callees (6):
  - 0x006b8f7b CALL 0x006c1450
  - 0x006b8f96 CALL 0x005453c0
  - 0x006b8fc1 CALL 0x0054b4f0 CSWSArea::GetRoomForceRating
  - 0x006b902d CALL 0x006c4120
  - 0x006b9098 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006b9157 indirect
- Imports: none
- Indirect slots (1):
  - 0x006b9157 slot +0x9c (006b9151 MOV EAX,dword ptr [EDX + 0x9c])
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched (1):
  - 0x130 CSWSCreatureStats::Force points (current / extra) [+0x130] (i16) R
- KDB purpose: Returns uses left for a spell at a class index, using room force rating and a list scan (likely).
- Named pseudo-C (first 30 of 75 lines):
```c
// NAMED 0x006b8f40 CSWSCreatureStats::GetSpellUsesLeft evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=3 exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006b910b) */
/* WARNING: Removing unreachable block (ram,0x006b916e) */

undefined1 CSWSCreatureStats__GetSpellUsesLeft(undefined4 param_1,byte param_2)

{
  short sVar1;
  int iVar2;
  uint uVar3;
  int in_ECX;
  short local_64;
  int local_34;
  undefined4 local_24;
  int local_18;
  undefined1 local_12;
  int local_10;
  
  if (param_2 < *(byte *)(in_ECX + 0x8d)) {
    iVar2 = FUN_006c1450(param_1);
    if (iVar2 == 0) {
      local_12 = 0;
    }
    else {
      iVar2 = FUN_005453c0();
      local_24 = 0;
      if (iVar2 != 0) {
        local_24 = CSWSArea__GetRoomForceRating(*(int *)(in_ECX + 0x24) + 0x94);
      }
```

## 0x006b91b0 CSWSCreatureStats::UnReadySpell

- Size 1090 B, RET 4
- Callers (1 total):
  - 0x005818a0 CSWSCreature::DecrementSpellReadyCount (site 0x00581909)
- Callees (10):
  - 0x006b91db CALL 0x006c1450
  - 0x006b91f6 CALL 0x005453c0
  - 0x006b9224 CALL 0x0054b4f0 CSWSArea::GetRoomForceRating
  - 0x006b92a0 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006b9330 CALL 0x006c4120
  - 0x006b9403 CALL 0x006c4120
  - 0x006b9598 indirect
  - 0x006b95aa CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b95cd indirect
  - 0x006b95e1 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
- Imports: none
- Indirect slots (2):
  - 0x006b9598 slot +0x9c (006b9592 MOV EAX,dword ptr [EDX + 0x9c])
  - 0x006b95cd slot +0x9c (006b95c7 MOV EAX,dword ptr [EDX + 0x9c])
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Validates a spell, reads room force rating and a cost effect, computes cost, then adjusts current points (likely force points).
- Named pseudo-C (first 30 of 94 lines):
```c
// NAMED 0x006b91b0 CSWSCreatureStats::UnReadySpell evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=4 exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006b9480) */
/* WARNING: Removing unreachable block (ram,0x006b94bd) */

undefined4 CSWSCreatureStats__UnReadySpell(undefined4 param_1)

{
  short sVar1;
  int iVar2;
  int in_ECX;
  undefined2 local_94;
  undefined4 local_40;
  undefined4 local_34;
  undefined4 local_2c;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_10;
  undefined4 local_8;
  
  local_8 = 0;
  iVar2 = FUN_006c1450(param_1);
  if (iVar2 != 0) {
    iVar2 = FUN_005453c0();
    local_34 = 0;
    if (iVar2 != 0) {
      local_34 = CSWSArea__GetRoomForceRating(*(int *)(in_ECX + 0x24) + 0x94);
    }
    local_20 = 0;
    local_2c = -1;
```

## 0x006b9640 CSWSCreatureStats::ValidateLevelUp

- Size 334 B, RET 4
- Callers (2 total):
  - 0x00661c30 CSWSMessage::HandlePlayerToServerLevelUpMessage (site 0x00661f21)
  - 0x006bedf0 CSWSCreatureStats::AutoLevelup (site 0x006c00aa)
- Callees (8):
  - 0x006b9672 CALL 0x006b9790 CSWSCreatureStats::CanLevelUp
  - 0x006b96b1 CALL 0x0051bd90 CServerExoApp::GetModule
  - 0x006b96c5 CALL 0x00919723 operator new
  - 0x006b96e0 CALL 0x005e1a60
  - 0x006b9756 CALL 0x0051c280 CServerExoApp::GetServerAIMaster
  - 0x006b975d CALL 0x0051e220 CServerAIMaster::AddEventAbsoluteTime
  - 0x006b976b CALL 0x006b9870 CSWSCreatureStats::LevelUp
  - 0x006b9773 CALL 0x006a7190 CSWSCreatureStats::UpdateCombatInformation
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4a4 READ (MOV EDX,dword ptr [0x00a1b4a4])
- Struct fields touched: none
- KDB purpose: Checks CanLevelUp, then either schedules a deferred level-up event on the server AI master or applies it now (likely).
- Named pseudo-C (first 30 of 56 lines):
```c
// NAMED 0x006b9640 CSWSCreatureStats::ValidateLevelUp evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=postpass exe=6a522e71631d

undefined4 CSWSCreatureStats__ValidateLevelUp(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int in_ECX;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined2 *local_44;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c20b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = CSWSCreatureStats__CanLevelUp(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    if ((*(int *)(in_ECX + 0x6c) != 0) && (iVar1 = CServerExoApp__GetModule(), iVar1 != 0)) {
      iVar3 = operator new(0x34);
      local_8 = 0;
```

## 0x006b9790 CSWSCreatureStats::CanLevelUp

- Size 210 B, RET 0
- Callers (16 total, first 5 shown):
  - 0x005fd480 CSWPartyTable::SpawnNPC (site 0x005fd7ce)
  - 0x005fe0c0 CSWPartyTable::DistributeExperience (site 0x005fe2a3)
  - 0x005fedd0 CSWPartyTable::AutoLevelUpNPCs (site 0x005fee32)
  - 0x0063edc0 (site 0x0063f003)
  - 0x0063f550 (site 0x0063f68a)
- Callees (5):
  - 0x006b97aa CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b97b5 CALL 0x0051bd40 CServerExoApp::GetServerInfo
  - 0x006b97cb CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006b980b indirect
  - 0x006b9836 indirect
- Imports: none
- Indirect slots (2):
  - 0x006b980b slot +0x94 (006b9805 MOV EAX,dword ptr [EDX + 0x94])
  - 0x006b9836 slot +0x9c (006b9830 MOV EAX,dword ptr [EDX + 0x9c])
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns 1 if level is under the server cap, XP reaches the next threshold, and no blocking check fails.
- Named pseudo-C (first 30 of 32 lines):
```c
// NAMED 0x006b9790 CSWSCreatureStats::CanLevelUp evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=3 exe=6a522e71631d

undefined4 CSWSCreatureStats__CanLevelUp(void)

{
  bool bVar1;
  byte bVar2;
  short sVar3;
  uint uVar4;
  int iVar5;
  int in_ECX;
  
  uVar4 = CSWSCreatureStats__GetLevel(0);
  iVar5 = CServerExoApp__GetServerInfo();
  if ((((int)(uVar4 & 0xff) < *(int *)(iVar5 + 0x94)) &&
      (bVar2 = CSWSCreatureStats__GetLevel(0),
      *(uint *)(DAT_00a1b4d0 + 0x38 + (uint)bVar2 * 4) <= *(uint *)(in_ECX + 0x68))) &&
     (iVar5 = (**(code **)(**(int **)(in_ECX + 0x24) + 0x94))(), iVar5 == 0)) {
    if ((*(int **)(in_ECX + 0x24))[0x46b] == 0) {
      bVar1 = false;
    }
    else {
      sVar3 = (**(code **)(**(int **)(in_ECX + 0x24) + 0x9c))(0);
      bVar1 = sVar3 < 1;
    }
    if (!bVar1) {
      return 1;
    }
  }
  return 0;
```

## 0x006b9870 CSWSCreatureStats::LevelUp

- Size 1262 B, RET 8
- Callers (2 total):
  - 0x006b6bd0 CSWSCreatureStats::SetExperience (site 0x006b6e9f)
  - 0x006b9640 CSWSCreatureStats::ValidateLevelUp (site 0x006b976b)
- Callees (23):
  - 0x006b98a7 indirect
  - 0x006b98b9 CALL 0x0057eca0 CSWSCreature::GetMaxForcePoints
  - 0x006b98d2 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006b995c CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b9970 CALL 0x006b6a20 CSWSCreatureStats::SetClassLevel
  - 0x006b99c1 CALL 0x006b69e0 CSWSCreatureStats::SetClass
  - 0x006b99d0 CALL 0x006b6a20 CSWSCreatureStats::SetClassLevel
  - 0x006b9a77 CALL 0x006b67d0 CSWSCreatureStats::SetSTRBase
  - 0x006b9a98 CALL 0x006b6810 CSWSCreatureStats::SetDEXBase
  - 0x006b9ab8 CALL 0x006b6850 CSWSCreatureStats::SetCONBase
  - 0x006b9ad6 CALL 0x006b6920 CSWSCreatureStats::SetINTBase
  - 0x006b9af4 CALL 0x006b6960 CSWSCreatureStats::SetWISBase
  - 0x006b9b12 CALL 0x006b69a0 CSWSCreatureStats::SetCHABase
  - 0x006b9b4c CALL 0x006f5ac0 PendingLevelUp::GetSkillDelta
  - 0x006b9b64 CALL 0x006b7bb0 CSWSCreatureStats::GetSkillRank
  - 0x006b9b78 CALL 0x006f5ac0 PendingLevelUp::GetSkillDelta
  - 0x006b9b8b CALL 0x006b81c0 CSWSCreatureStats::SetSkillRank
  - 0x006b9bed CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006b9c5f CALL 0x006b8760 CSWSCreatureStats::RemoveKnownSpell
  - 0x006b9cb5 CALL 0x006b86a0 CSWSCreatureStats::AddKnownSpell
  - 0x006b9ce9 indirect
  - 0x006b9cf2 CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
  - 0x006b9d03 CALL 0x0057eca0 CSWSCreature::GetMaxForcePoints
- Imports: none
- Indirect slots (2):
  - 0x006b98a7 slot +0x98 (006b98a1 MOV EAX,dword ptr [EDX + 0x98])
  - 0x006b9ce9 slot +0x98 (006b9ce3 MOV EAX,dword ptr [EDX + 0x98])
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Applies a pending level-up: walks the class table, sets class and ability bases, skill ranks, feats and spells.
- Named pseudo-C (first 30 of 101 lines):
```c
// NAMED 0x006b9870 CSWSCreatureStats::LevelUp evidence=ev_625178392331219315601d4138f1eb45c28c4e7ab9d9bbd19f22015da2a6f100 names=19 exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006b9d24) */

void CSWSCreatureStats__LevelUp(int *param_1,int param_2)

{
  bool bVar1;
  char cVar2;
  char cVar3;
  short sVar4;
  short sVar5;
  uint uVar6;
  int in_ECX;
  byte local_60;
  byte local_58;
  char local_2b;
  byte local_15;
  byte local_9;
  byte local_8;
  byte local_6;
  byte local_5;
  
  local_6 = 0;
  bVar1 = true;
  (**(code **)(**(int **)(in_ECX + 0x24) + 0x98))(1);
  sVar4 = CSWSCreature__GetMaxForcePoints();
  if (param_2 != 0) {
    CExoArrayList<uint32>__Add(param_1);
  }
```

## 0x006b9d80 CSWSCreatureStats::LevelDown

- Size 1415 B, RET 4
- Callers (1 total):
  - 0x006b6bd0 CSWSCreatureStats::SetExperience (site 0x006b6e1e)
- Callees (27):
  - 0x006b9db9 indirect
  - 0x006b9e37 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b9e4f CALL 0x006b6a20 CSWSCreatureStats::SetClassLevel
  - 0x006b9e65 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006b9edb CALL 0x006b69e0 CSWSCreatureStats::SetClass
  - 0x006b9f60 CALL 0x006b67d0 CSWSCreatureStats::SetSTRBase
  - 0x006b9f81 CALL 0x006b6810 CSWSCreatureStats::SetDEXBase
  - 0x006b9fa1 CALL 0x006b6850 CSWSCreatureStats::SetCONBase
  - 0x006b9fbf CALL 0x006b6920 CSWSCreatureStats::SetINTBase
  - 0x006b9fdd CALL 0x006b6960 CSWSCreatureStats::SetWISBase
  - 0x006b9ffb CALL 0x006b69a0 CSWSCreatureStats::SetCHABase
  - 0x006ba036 CALL 0x006f5ac0 PendingLevelUp::GetSkillDelta
  - 0x006ba04e CALL 0x006b7bb0 CSWSCreatureStats::GetSkillRank
  - 0x006ba062 CALL 0x006f5ac0 PendingLevelUp::GetSkillDelta
  - 0x006ba075 CALL 0x006b81c0 CSWSCreatureStats::SetSkillRank
  - 0x006ba0ca CALL 0x006b85f0 CSWSCreatureStats::RemoveFeat
  - 0x006ba13b CALL 0x006b8760 CSWSCreatureStats::RemoveKnownSpell
  - 0x006ba191 CALL 0x006b86a0 CSWSCreatureStats::AddKnownSpell
  - 0x006ba1a7 CALL 0x0052e4e0
  - 0x006ba1c1 CALL 0x006f59b0
  - 0x006ba1d4 CALL 0x009196fd operator delete
  - 0x006ba232 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006ba25a CALL 0x005bd6a0 CSWSCreature::CanEquipItem
  - 0x006ba276 CALL 0x0056df50 CSWSCreature::UnequipItem
  - 0x006ba2ab indirect
  - 0x006ba2ed indirect
  - 0x006ba2fa CALL 0x0054a610 CSWSObject::SetCurrentHitPoints
- Imports: none
- Indirect slots (3):
  - 0x006b9db9 slot +0x98 (006b9db3 MOV EAX,dword ptr [EDX + 0x98])
  - 0x006ba2ab slot +0x98 (006ba2a5 MOV EAX,dword ptr [EDX + 0x98])
  - 0x006ba2ed slot +0x9c (006ba2e7 MOV EAX,dword ptr [EDX + 0x9c])
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Drops one level: removes an emptied class, reverts ability bases, skills, feats and spells, and unequips now-unusable gear.
- Named pseudo-C (first 30 of 120 lines):
```c
// NAMED 0x006b9d80 CSWSCreatureStats::LevelDown evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=postpass exe=6a522e71631d

void CSWSCreatureStats__LevelDown(int *param_1)

{
  bool bVar1;
  byte bVar2;
  char cVar3;
  char cVar4;
  short sVar5;
  short sVar6;
  short sVar7;
  int iVar8;
  int in_ECX;
  byte local_64;
  byte local_60;
  byte local_3f;
  char local_3b;
  byte local_25;
  byte local_15;
  int local_14;
  byte local_e;
  int local_c;
  byte local_5;
  
  sVar5 = (**(code **)(**(int **)(in_ECX + 0x24) + 0x98))(1);
  local_5 = 0;
  do {
    if (*(byte *)(in_ECX + 0x8d) <= local_5) {
LAB_006b9e5b:
```

## 0x006ba320 CSWSCreatureStats::SetMovementRate

- Size 761 B, RET 4
- Callers (5 total):
  - 0x0057ff50 CSWSCreature::UpdateAppearanceDependantInfo (site 0x0058020f)
  - 0x005f9ff0 CSWPartyTable::AddNPC (site 0x005fa205)
  - 0x005fa3b0 CSWPartyTable::AddPUP (site 0x005fa475)
  - 0x0065a170 (site 0x0065a7ab)
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b398f)
- Callees (17):
  - 0x006ba35b CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ba36a CALL 0x00733540 CExoString::DefaultConstructor
  - 0x006ba37b CALL 0x00733570 CExoString::CStrConstructor
  - 0x006ba3bd CALL 0x0071b1d0
  - 0x006ba3cc CALL 0x00733780 CExoString::Destructor
  - 0x006ba422 CALL 0x00733570 CExoString::CStrConstructor
  - 0x006ba452 CALL 0x0071b1d0
  - 0x006ba461 CALL 0x00733780 CExoString::Destructor
  - 0x006ba473 CALL 0x00734690 CExoString::ToLower
  - 0x006ba495 CALL 0x00734690 CExoString::ToLower
  - 0x006ba4bd CALL 0x007339d0
  - 0x006ba4cc CALL 0x00733780 CExoString::Destructor
  - 0x006ba4d8 CALL 0x00733780 CExoString::Destructor
  - 0x006ba52a CALL 0x00733780 CExoString::Destructor
  - 0x006ba539 CALL 0x00733780 CExoString::Destructor
  - 0x006ba590 CALL 0x0071b3f0
  - 0x006ba5dc CALL 0x0071b3f0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings (2):
  - 0x00999c8c "2DAName"
  - 0x00999c94 "MOVERATE"
- Globals (3):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched (1):
  - 0x1114 CSWSCreature::allowed-action mask (init 0xFFFF, death 0; bit 0x2 movement, 0x4+0x80 attack, 0x8 spells in HasTalent, 0x40 client awake) (u16) W
- KDB purpose: Sets creature movement rate from the MOVERATE column of its 2DA row, only when the argument is 7 (likely).
- Named pseudo-C (first 30 of 84 lines):
```c
// NAMED 0x006ba320 CSWSCreatureStats::SetMovementRate evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=3 exe=6a522e71631d

void CSWSCreatureStats__SetMovementRate(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int in_ECX;
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  int local_40;
  undefined1 local_3c [8];
  int local_34;
  undefined1 local_30 [8];
  int local_28;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c260;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (param_1 == 7) {
    CExoString__DefaultConstructor(uVar1);
```

## 0x006ba620 CSWSCreatureStats::GetFortSavingThrow

- Size 137 B, RET 4
- Callers (8 total, first 5 shown):
  - 0x0062f070 (site 0x0062f43b)
  - 0x0063f550 (site 0x0064126d)
  - 0x00646270 (site 0x00646536)
  - 0x0067d8a0 Global::ExecuteCommandGetFortitudeSavingThrow (site 0x0067d93d)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006adb4a)
- Callees (2):
  - 0x006ba668 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
  - 0x006ba68f CALL 0x006b73a0 CSWSCreatureStats::GetBaseFortSavingThrow
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns fortitude save: base save, CON modifier, a stored bonus byte, plus effect bonuses when argument is zero.
- Named pseudo-C (first 23 of 23 lines):
```c
// NAMED 0x006ba620 CSWSCreatureStats::GetFortSavingThrow evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=2 exe=6a522e71631d

int CSWSCreatureStats__GetFortSavingThrow(int param_1)

{
  char cVar1;
  char cVar2;
  char cVar3;
  int in_ECX;
  undefined4 local_c;
  int local_8;
  
  local_8 = 0;
  if (param_1 == 0) {
    local_c = 0;
    local_8 = CSWSCreature__GetTotalEffectBonus(3,0,&local_c,0,0,1,0,0xff,0xff,0,0);
  }
  cVar1 = *(char *)(in_ECX + 0xf2);
  cVar2 = *(char *)(in_ECX + 0x1b0);
  cVar3 = CSWSCreatureStats__GetBaseFortSavingThrow();
  return (int)cVar2 + (int)cVar1 + (int)cVar3 + local_8;
}

```

## 0x006ba6b0 CSWSCreatureStats::GetWillSavingThrow

- Size 137 B, RET 4
- Callers (8 total, first 5 shown):
  - 0x0062f070 (site 0x0062f455)
  - 0x0063f550 (site 0x00641213)
  - 0x00646270 (site 0x00646559)
  - 0x0068ab60 Global::ExecuteCommandGetWillSavingThrow (site 0x0068abfd)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006adb89)
- Callees (2):
  - 0x006ba6f8 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
  - 0x006ba71f CALL 0x006b75c0 CSWSCreatureStats::GetBaseWillSavingThrow
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns will save: base save, WIS modifier, a stored bonus byte, plus effect bonuses when argument is zero.
- Named pseudo-C (first 23 of 23 lines):
```c
// NAMED 0x006ba6b0 CSWSCreatureStats::GetWillSavingThrow evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=2 exe=6a522e71631d

int CSWSCreatureStats__GetWillSavingThrow(int param_1)

{
  char cVar1;
  char cVar2;
  char cVar3;
  int in_ECX;
  undefined4 local_c;
  int local_8;
  
  local_8 = 0;
  if (param_1 == 0) {
    local_c = 0;
    local_8 = CSWSCreature__GetTotalEffectBonus(3,0,&local_c,0,0,3,0,0xff,0xff,0,0);
  }
  cVar1 = *(char *)(in_ECX + 0xf6);
  cVar2 = *(char *)(in_ECX + 0x1b1);
  cVar3 = CSWSCreatureStats__GetBaseWillSavingThrow();
  return (int)cVar2 + (int)cVar1 + (int)cVar3 + local_8;
}

```

## 0x006ba740 CSWSCreatureStats::GetReflexSavingThrow

- Size 134 B, RET 4
- Callers (8 total, first 5 shown):
  - 0x0062f070 (site 0x0062f471)
  - 0x0063f550 (site 0x00641240)
  - 0x00646270 (site 0x0064657c)
  - 0x00687cb0 Global::ExecuteCommandGetReflexSavingThrow (site 0x00687d4d)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006adbc8)
- Callees (3):
  - 0x006ba788 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
  - 0x006ba7a1 CALL 0x006b5db0 CSWSCreatureStats::GetDEXMod
  - 0x006ba7ac CALL 0x006b77e0 CSWSCreatureStats::GetBaseReflexSavingThrow
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns total reflex save: base, DEX modifier, a stored bonus byte, and effect bonuses if argument is zero.
- Named pseudo-C (first 23 of 23 lines):
```c
// NAMED 0x006ba740 CSWSCreatureStats::GetReflexSavingThrow evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=3 exe=6a522e71631d

int CSWSCreatureStats__GetReflexSavingThrow(int param_1)

{
  char cVar1;
  char cVar2;
  char cVar3;
  int in_ECX;
  undefined4 local_c;
  int local_8;
  
  local_8 = 0;
  if (param_1 == 0) {
    local_c = 0;
    local_8 = CSWSCreature__GetTotalEffectBonus(3,0,&local_c,0,0,2,0,0xff,0xff,0,0);
  }
  cVar1 = *(char *)(in_ECX + 0x1b2);
  cVar2 = CSWSCreatureStats__GetDEXMod(0);
  cVar3 = CSWSCreatureStats__GetBaseReflexSavingThrow();
  return (int)cVar1 + (int)cVar2 + (int)cVar3 + local_8;
}

```

## 0x006ba7d0 CSWSCreatureStats::GetFeat

- Size 78 B, RET 12
- Callers (1 total):
  - 0x006afed0 CSWSCreatureStats::ReadStatsFromGff (site 0x006b3af6)
- Callees (2):
  - 0x006ba7dc CALL 0x006a4230
  - 0x006ba813 CALL 0x006c0b00
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Builds a 12-byte record from id, flag and byte and likely adds it to the +0x134 list (name misleading).
- Named pseudo-C (first 10 of 10 lines):
```c
// NAMED 0x006ba7d0 CSWSCreatureStats::GetFeat evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

void CSWSCreatureStats__GetFeat(undefined4 param_1,undefined1 param_2,undefined1 param_3)

{
  FUN_006a4230();
  FUN_006c0b00(param_1,param_2,param_3);
  return;
}

```

## 0x006ba8f0 CSWSCreatureStats::GetIsInSpellLikeAbilityList

- Size 128 B, RET 4
- Callers (3 total):
  - 0x00565310 CSWSCreature::AddCastSpellActions (site 0x005654aa)
  - 0x0066e7e0 Global::ExecuteCommandCastSpell (site 0x0066eee2)
  - 0x006b8840 CSWSCreatureStats::GetCreatureHasTalent (site 0x006b8a1a)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched (1):
  - 0x134 CSWSCreatureStats::known-spell vector {ptr data, int count}, entries 12 B {id, flag, byte} (ptr) R
- KDB purpose: Returns 1 if the known-spell list has the given id with flag 1 (a spell-like ability), else 0.
- Named pseudo-C (first 20 of 20 lines):
```c
// NAMED 0x006ba8f0 CSWSCreatureStats::GetIsInSpellLikeAbilityList evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetIsInSpellLikeAbilityList(int param_1)

{
  int in_ECX;
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (*(int *)(*(int *)(in_ECX + 0x134) + 4) <= local_8) {
      return 0;
    }
    if ((*(int *)(local_8 * 0xc + **(int **)(in_ECX + 0x134)) == param_1) &&
       (*(int *)(**(int **)(in_ECX + 0x134) + 4 + local_8 * 0xc) == 1)) break;
    local_8 = local_8 + 1;
  }
  return 1;
}

```

## 0x006ba9f0 CSWSCreatureStats::GetTotalSTRBonus

- Size 70 B, RET 0
- Callers (4 total):
  - 0x0062f070 (site 0x0062f378)
  - 0x00646270 (site 0x006463b8)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006ada6a)
  - 0x006b5990 CSWSCreatureStats::GetSTRStat (site 0x006b59cc)
- Callees (1):
  - 0x006baa27 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns total strength bonus from effects, via the effect-bonus helper with ability index 0 (STR).
- Named pseudo-C (first 13 of 13 lines):
```c
// NAMED 0x006ba9f0 CSWSCreatureStats::GetTotalSTRBonus evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetTotalSTRBonus(void)

{
  undefined4 uVar1;
  undefined4 local_c [2];
  
  local_c[0] = 0;
  uVar1 = CSWSCreature__GetTotalEffectBonus(4,0,local_c,0,0,0,0,0xff,0,0,0);
  return uVar1;
}

```

## 0x006baa40 CSWSCreatureStats::GetTotalDEXBonus

- Size 70 B, RET 0
- Callers (4 total):
  - 0x0062f070 (site 0x0062f38e)
  - 0x00646270 (site 0x006463d1)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006ada81)
  - 0x006b5d00 (site 0x006b5d3c)
- Callees (1):
  - 0x006baa77 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns total dexterity bonus from effects, via the effect-bonus helper with ability index 1 (DEX).
- Named pseudo-C (first 13 of 13 lines):
```c
// NAMED 0x006baa40 CSWSCreatureStats::GetTotalDEXBonus evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetTotalDEXBonus(void)

{
  undefined4 uVar1;
  undefined4 local_c [2];
  
  local_c[0] = 0;
  uVar1 = CSWSCreature__GetTotalEffectBonus(4,0,local_c,0,0,0,0,0xff,1,0,0);
  return uVar1;
}

```

## 0x006baa90 CSWSCreatureStats::GetTotalCONBonus

- Size 70 B, RET 0
- Callers (4 total):
  - 0x0062f070 (site 0x0062f3a6)
  - 0x00646270 (site 0x006463ea)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006ada98)
  - 0x006b5a40 CSWSCreatureStats::GetCONStat (site 0x006b5a7c)
- Callees (1):
  - 0x006baac7 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns total constitution bonus from effects, via the effect-bonus helper with ability index 2 (CON).
- Named pseudo-C (first 13 of 13 lines):
```c
// NAMED 0x006baa90 CSWSCreatureStats::GetTotalCONBonus evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetTotalCONBonus(void)

{
  undefined4 uVar1;
  undefined4 local_c [2];
  
  local_c[0] = 0;
  uVar1 = CSWSCreature__GetTotalEffectBonus(4,0,local_c,0,0,0,0,0xff,2,0,0);
  return uVar1;
}

```

## 0x006baae0 CSWSCreatureStats::GetTotalINTBonus

- Size 70 B, RET 0
- Callers (4 total):
  - 0x0062f070 (site 0x0062f3be)
  - 0x00646270 (site 0x00646403)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006adaaf)
  - 0x006b5af0 CSWSCreatureStats::GetINTStat (site 0x006b5b2c)
- Callees (1):
  - 0x006bab17 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns total intelligence bonus from effects, via the effect-bonus helper with ability index 3 (INT).
- Named pseudo-C (first 13 of 13 lines):
```c
// NAMED 0x006baae0 CSWSCreatureStats::GetTotalINTBonus evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetTotalINTBonus(void)

{
  undefined4 uVar1;
  undefined4 local_c [2];
  
  local_c[0] = 0;
  uVar1 = CSWSCreature__GetTotalEffectBonus(4,0,local_c,0,0,0,0,0xff,3,0,0);
  return uVar1;
}

```

## 0x006bab30 CSWSCreatureStats::GetTotalWISBonus

- Size 70 B, RET 0
- Callers (4 total):
  - 0x0062f070 (site 0x0062f3d6)
  - 0x00646270 (site 0x0064641c)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006adac6)
  - 0x006b5ba0 CSWSCreatureStats::GetWISStat (site 0x006b5bdc)
- Callees (1):
  - 0x006bab67 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns total wisdom bonus from effects, via the effect-bonus helper with ability index 4 (WIS).
- Named pseudo-C (first 13 of 13 lines):
```c
// NAMED 0x006bab30 CSWSCreatureStats::GetTotalWISBonus evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetTotalWISBonus(void)

{
  undefined4 uVar1;
  undefined4 local_c [2];
  
  local_c[0] = 0;
  uVar1 = CSWSCreature__GetTotalEffectBonus(4,0,local_c,0,0,0,0,0xff,4,0,0);
  return uVar1;
}

```

## 0x006bab80 CSWSCreatureStats::GetTotalCHABonus

- Size 70 B, RET 0
- Callers (4 total):
  - 0x0062f070 (site 0x0062f3ee)
  - 0x00646270 (site 0x00646435)
  - 0x006ad910 CSWSCreatureStats::UpdateLastStatsObject (site 0x006adadd)
  - 0x006b5c50 CSWSCreatureStats::GetCHAStat (site 0x006b5c8c)
- Callees (1):
  - 0x006babb7 CALL 0x00582010 CSWSCreature::GetTotalEffectBonus
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns total charisma bonus from effects, via the effect-bonus helper with ability index 5 (CHA).
- Named pseudo-C (first 13 of 13 lines):
```c
// NAMED 0x006bab80 CSWSCreatureStats::GetTotalCHABonus evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetTotalCHABonus(void)

{
  undefined4 uVar1;
  undefined4 local_c [2];
  
  local_c[0] = 0;
  uVar1 = CSWSCreature__GetTotalEffectBonus(4,0,local_c,0,0,0,0,0xff,5,0,0);
  return uVar1;
}

```

## 0x006babd0 CSWSCreatureStats::SetSpellLikeAbilityReady

- Size 307 B, RET 8
- Callers (1 total):
  - 0x006bedf0 CSWSCreatureStats::AutoLevelup (site 0x006bfbaf)
- Callees (2):
  - 0x006bacc4 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006bace5 CALL 0x006f09d0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns a table value for a force-user class at its level (plus one if flagged); 0 otherwise.
- Named pseudo-C (first 30 of 43 lines):
```c
// NAMED 0x006babd0 CSWSCreatureStats::SetSpellLikeAbilityReady evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=2 exe=6a522e71631d

uint CSWSCreatureStats__SetSpellLikeAbilityReady(byte param_1,int param_2)

{
  byte bVar1;
  uint uVar2;
  int in_ECX;
  undefined1 local_b;
  undefined1 local_9;
  
  uVar2 = (uint)param_1;
  if (uVar2 < *(byte *)(in_ECX + 0x8d)) {
    local_9 = *(byte *)(in_ECX + 0xab + (uint)param_1 * 0x28);
    uVar2 = CONCAT31((int3)((uint)param_1 * 0x28 >> 8),local_9);
  }
  else {
    local_9 = 0xff;
  }
  if ((uint)local_9 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) == 0) {
    uVar2 = uVar2 & 0xffffff00;
  }
  else if ((((((local_9 == 4) || (local_9 == 3)) || (local_9 == 5)) ||
            ((local_9 == 0xd || (local_9 == 0xb)))) ||
           ((local_9 == 0xc || ((local_9 == 0x10 || (local_9 == 0xe)))))) || (local_9 == 0xf)) {
    local_b = CSWSCreatureStats__GetClassLevel(param_1,0);
    if (param_2 != 0) {
      local_b = local_b + '\x01';
    }
    bVar1 = FUN_006f09d0(local_b,0);
```

## 0x006bad10 CSWSCreatureStats::GetForceResistance

- Size 75 B, RET 0
- Callers (3 total):
  - 0x0059f710 CSWSEffectListHandler::OnApplyForceResistanceIncrease (site 0x0059f7a7)
  - 0x006900f0 Global::ExecuteCommandResistSpell (site 0x006903c7)
  - 0x006a7190 CSWSCreatureStats::UpdateCombatInformation (site 0x006a7b6a)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns force resistance as one stored byte minus a second, floored at zero.
- Named pseudo-C (first 17 of 17 lines):
```c
// NAMED 0x006bad10 CSWSCreatureStats::GetForceResistance evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetForceResistance(void)

{
  char cVar1;
  char cVar2;
  int in_ECX;
  
  cVar1 = *(char *)(in_ECX + 299) - *(char *)(in_ECX + 300);
  cVar2 = cVar1;
  if (cVar1 < '\0') {
    cVar2 = '\0';
  }
  return CONCAT31(cVar1 >> 7,cVar2);
}

```

## 0x006bae00 CSWSCreatureStats::GetCreatureTalentRandomPower

- Size 181 B, RET 28
- Callers (1 total):
  - 0x006b8bb0 CSWSCreatureStats::GetCreatureTalentRandom (site 0x006b8ccc)
- Callees (2):
  - 0x006bae68 CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006bae97 CALL 0x006bcfa0 CSWSCreatureStats::GetCreatureTalentRandomKnownPower
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: For each force-user class, tries a random known power; returns 1 on first success, else the last result.
- Named pseudo-C (first 30 of 35 lines):
```c
// NAMED 0x006bae00 CSWSCreatureStats::GetCreatureTalentRandomPower evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=postpass exe=6a522e71631d

int CSWSCreatureStats__GetCreatureTalentRandomPower
              (undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
              undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  int iVar1;
  int in_ECX;
  undefined1 local_e;
  uint local_c;
  int local_8;
  
  local_8 = 0;
  local_c = 0;
  while( true ) {
    if ((int)(uint)*(byte *)(in_ECX + 0x8d) <= (int)local_c) {
      return local_8;
    }
    if ((local_c & 0xff) < (uint)*(byte *)(in_ECX + 0x8d)) {
      local_e = *(undefined1 *)(in_ECX + 0xab + (local_c & 0xff) * 0x28);
    }
    else {
      local_e = 0xff;
    }
    iVar1 = IsForceUserClass(classId) {3,4,5,11..16}(local_e);
    if ((iVar1 != 0) &&
       (local_8 = CSWSCreatureStats__GetCreatureTalentRandomKnownPower
                            (param_1,param_2,param_3,param_4,param_5,local_c,param_6,param_7),
       local_8 == 1)) break;
```

## 0x006baec0 CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility

- Size 1245 B, RET 32
- Callers (1 total):
  - 0x006b8bb0 CSWSCreatureStats::GetCreatureTalentRandom (site 0x006b8ca4)
- Callees (29):
  - 0x006baef7 CALL 0x00919723 operator new
  - 0x006baf14 CALL 0x0084a640
  - 0x006baf43 CALL 0x00919723 operator new
  - 0x006baf60 CALL 0x0084a640
  - 0x006bb06c CALL 0x006c1450
  - 0x006bb09e CALL 0x006ba970 CSWSCreatureStats::IsSpellTalentCandidate
  - 0x006bb0bc CALL 0x006bde30
  - 0x006bb0d7 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb0e4 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb0ef CALL 0x00734010
  - 0x006bb12c CALL 0x0064ddb0
  - 0x006bb13f CALL 0x009196fd operator delete
  - 0x006bb15e CALL 0x00919723 operator new
  - 0x006bb17b CALL 0x0084a640
  - 0x006bb1af CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb1c9 CALL 0x0064ddb0
  - 0x006bb1dc CALL 0x009196fd operator delete
  - 0x006bb1fb CALL 0x00919723 operator new
  - 0x006bb218 CALL 0x0084a640
  - 0x006bb24d CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb258 CALL 0x00734010
  - 0x006bb28c CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb299 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb2a4 CALL 0x00734010
  - 0x006bb2bd CALL 0x0091eb8b
  - 0x006bb314 CALL 0x0064ddb0
  - 0x006bb327 CALL 0x009196fd operator delete
  - 0x006bb359 CALL 0x0064ddb0
  - 0x006bb36c CALL 0x009196fd operator delete
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched (1):
  - 0x134 CSWSCreatureStats::known-spell vector {ptr data, int count}, entries 12 B {id, flag, byte} (ptr) R
- KDB purpose: Collects known spells that are valid special-ability candidates, then selects one, likely at random.
- Named pseudo-C (first 30 of 129 lines):
```c
// NAMED 0x006baec0 CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=postpass exe=6a522e71631d

bool CSWSCreatureStats__GetCreatureTalentRandomSpecialAbility
               (undefined4 param_1,int param_2,undefined4 *param_3,undefined4 *param_4,
               undefined1 *param_5,undefined1 *param_6,undefined4 param_7,undefined4 param_8)

{
  undefined1 uVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int in_ECX;
  int *local_b4;
  int *local_ac;
  int *local_a4;
  int *local_a0;
  int local_34;
  int local_30;
  int *local_2c;
  int *local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c2ac;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
```

## 0x006bb3a0 CSWSCreatureStats::GetCreatureTalentRandomPowerFromItem

- Size 3382 B, RET 36
- Callers (1 total):
  - 0x006b8bb0 CSWSCreatureStats::GetCreatureTalentRandom (site 0x006b8cfc)
- Callees (74):
  - 0x006bb3d7 CALL 0x00919723 operator new
  - 0x006bb3f4 CALL 0x0084a640
  - 0x006bb423 CALL 0x00919723 operator new
  - 0x006bb440 CALL 0x0084a640
  - 0x006bb46f CALL 0x00919723 operator new
  - 0x006bb48c CALL 0x0084a640
  - 0x006bb53c CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006bb555 CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006bb599 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006bb5cb CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006bb5fb CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006bb667 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006bb6a9 CALL 0x006c1450
  - 0x006bb6db CALL 0x006ba970 CSWSCreatureStats::IsSpellTalentCandidate
  - 0x006bb6f9 CALL 0x006bde30
  - 0x006bb714 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb72f CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb73b CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb778 CALL 0x0064ddb0
  - 0x006bb78b CALL 0x009196fd operator delete
  - 0x006bb7bd CALL 0x0064ddb0
  - 0x006bb7d0 CALL 0x009196fd operator delete
  - 0x006bb808 CALL 0x0064ddb0
  - 0x006bb81b CALL 0x009196fd operator delete
  - 0x006bb83a CALL 0x00919723 operator new
  - 0x006bb860 CALL 0x0084a640
  - 0x006bb895 CALL 0x00919723 operator new
  - 0x006bb8bb CALL 0x0084a640
  - 0x006bb8f0 CALL 0x00919723 operator new
  - 0x006bb916 CALL 0x0084a640
  - 0x006bb950 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb96b CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb977 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb9ab CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb9c6 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb9d2 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bb9f8 CALL 0x0058a2c0 CSWSCreature::GetItemRepository
  - 0x006bba3e CALL 0x00606100 CItemRepository::ItemListGetItem
  - 0x006bba78 CALL 0x006d5200
  - 0x006bba9b CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006bbadf CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006bbb11 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006bbb59 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006bbbc4 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006bbc06 CALL 0x006c1450
  - 0x006bbc38 CALL 0x006bdd60 CSWSCreatureStats::CheckInclusionMatch
  - 0x006bbc53 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbc6e CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbc7a CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbcc6 CALL 0x0064ddb0
  - 0x006bbcdc CALL 0x009196fd operator delete
  - 0x006bbd20 CALL 0x0064ddb0
  - 0x006bbd36 CALL 0x009196fd operator delete
  - 0x006bbd7a CALL 0x0064ddb0
  - 0x006bbd90 CALL 0x009196fd operator delete
  - 0x006bbdb2 CALL 0x00919723 operator new
  - 0x006bbdd8 CALL 0x0084a640
  - 0x006bbe0d CALL 0x00919723 operator new
  - 0x006bbe33 CALL 0x0084a640
  - 0x006bbe68 CALL 0x00919723 operator new
  - 0x006bbe8e CALL 0x0084a640
  - 0x006bbec8 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbee3 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbeef CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbf23 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbf3e CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbf4a CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bbf68 CALL 0x0091eb8b
  - 0x006bbfd8 CALL 0x0064ddb0
  - 0x006bbfee CALL 0x009196fd operator delete
  - 0x006bc032 CALL 0x0064ddb0
  - 0x006bc048 CALL 0x009196fd operator delete
  - 0x006bc08c CALL 0x0064ddb0
  - 0x006bc0a2 CALL 0x009196fd operator delete
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Allocates temp candidate lists, then scans item properties in an inventory slot for item-granted powers (likely).
- Named pseudo-C (first 30 of 280 lines):
```c
// NAMED 0x006bb3a0 CSWSCreatureStats::GetCreatureTalentRandomPowerFromItem evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=postpass exe=6a522e71631d

bool CSWSCreatureStats__GetCreatureTalentRandomPowerFromItem
               (undefined4 param_1,int param_2,undefined4 *param_3,undefined4 *param_4,
               undefined4 param_5,undefined4 *param_6,undefined4 *param_7,undefined4 param_8,
               undefined4 param_9)

{
  undefined2 uVar1;
  bool bVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  short *psVar6;
  int iVar7;
  int iVar8;
  int in_ECX;
  int *local_1d0;
  int *local_1cc;
  int *local_1c8;
  int *local_1b4;
  int *local_1b0;
  int *local_1ac;
  int *local_198;
  int *local_194;
  int *local_190;
  int local_50;
  int local_44;
  int local_3c;
  int local_38;
```

## 0x006bc0e0 CSWSCreatureStats::GetCreatureTalentRandomFeat

- Size 738 B, RET 28
- Callers (1 total):
  - 0x006b8bb0 CSWSCreatureStats::GetCreatureTalentRandom (site 0x006b8d70)
- Callees (16):
  - 0x006bc111 CALL 0x00919723 operator new
  - 0x006bc12e CALL 0x0084a640
  - 0x006bc191 CALL 0x006a20f0
  - 0x006bc1c1 CALL 0x006bdd60 CSWSCreatureStats::CheckInclusionMatch
  - 0x006bc1e6 CALL 0x006bde30
  - 0x006bc224 CALL 0x006b8840 CSWSCreatureStats::GetCreatureHasTalent
  - 0x006bc24c CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bc280 CALL 0x0064ddb0
  - 0x006bc293 CALL 0x009196fd operator delete
  - 0x006bc2ac CALL 0x00919723 operator new
  - 0x006bc2c9 CALL 0x0084a640
  - 0x006bc2f5 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bc31e CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bc34c CALL 0x0091eb8b
  - 0x006bc384 CALL 0x0064ddb0
  - 0x006bc397 CALL 0x009196fd operator delete
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched (1):
  - 0x108 CSWRules::feat table (rows 0x50; +0x10 Category, +0x14 MaxCR, +0x28 bit 4 valid) (table) R
- KDB purpose: Collects feats the creature can take (valid, passes inclusion check, not already held) and picks one at random (likely).
- Named pseudo-C (first 30 of 105 lines):
```c
// NAMED 0x006bc0e0 CSWSCreatureStats::GetCreatureTalentRandomFeat evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=postpass exe=6a522e71631d

bool CSWSCreatureStats__GetCreatureTalentRandomFeat
               (undefined4 param_1,int param_2,undefined4 *param_3,undefined4 *param_4,
               undefined4 param_5,undefined4 param_6,uint param_7)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  int *in_ECX;
  int *local_70;
  int *local_68;
  int local_28;
  ushort local_24;
  int *local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c386;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = operator new(0xc,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar2 == 0) {
    local_68 = (int *)0x0;
  }
  else {
```

## 0x006bc3d0 CSWSCreatureStats::SetForceResistance

- Size 33 B, RET 4
- Callers (2 total):
  - 0x0059f710 CSWSEffectListHandler::OnApplyForceResistanceIncrease (site 0x0059f7bc)
  - 0x0059f7d0 CSWSEffectListHandler::OnRemoveForceResistanceIncrease (site 0x0059f9b7)
- Callees (1):
  - 0x006bc3e6 CALL 0x006a7190 CSWSCreatureStats::UpdateCombatInformation
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Stores the new force resistance value, then calls UpdateCombatInformation to refresh combat state.
- Named pseudo-C (first 12 of 12 lines):
```c
// NAMED 0x006bc3d0 CSWSCreatureStats::SetForceResistance evidence=ev_309f550e30833ee5d8ae91b9275d2cac8262c95f280bd6146705403ccd7591d9 names=2 exe=6a522e71631d

void CSWSCreatureStats__SetForceResistance(undefined1 param_1)

{
  int in_ECX;
  
  *(undefined1 *)(in_ECX + 299) = param_1;
  CSWSCreatureStats__UpdateCombatInformation();
  return;
}

```

## 0x006bc400 CSWSCreatureStats::SetForceResistancePenalty

- Size 33 B, RET 4
- Callers (2 total):
  - 0x0059f9d0 CSWSEffectListHandler::OnApplyForceResistanceDecrease (site 0x0059fad1)
  - 0x0059fae0 CSWSEffectListHandler::OnRemoveForceResistanceDecrease (site 0x0059fc91)
- Callees (1):
  - 0x006bc416 CALL 0x006a7190 CSWSCreatureStats::UpdateCombatInformation
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Stores the force resistance penalty byte at +0x12c, then calls UpdateCombatInformation to refresh combat values.
- Named pseudo-C (first 12 of 12 lines):
```c
// NAMED 0x006bc400 CSWSCreatureStats::SetForceResistancePenalty evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=2 exe=6a522e71631d

void CSWSCreatureStats__SetForceResistancePenalty(undefined1 param_1)

{
  int in_ECX;
  
  *(undefined1 *)(in_ECX + 300) = param_1;
  CSWSCreatureStats__UpdateCombatInformation();
  return;
}

```

## 0x006bc430 CSWSCreatureStats::GetFeat_2

- Size 53 B, RET 4
- Callers (8 total, first 5 shown):
  - 0x0062f070 (site 0x0062f6ed)
  - 0x00636050 (site 0x00636134)
  - 0x0063f550 (site 0x0063ff1c)
  - 0x006415e0 (site 0x00641774)
  - 0x00646270 (site 0x00646c39)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns the feat ID at an index (list at +0x0, count +0x4), or 65535 if out of range.
- Named pseudo-C (first 17 of 17 lines):
```c
// NAMED 0x006bc430 CSWSCreatureStats::GetFeat_2 evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=1 exe=6a522e71631d

undefined2 CSWSCreatureStats__GetFeat_2(ushort param_1)

{
  undefined2 uVar1;
  int *in_ECX;
  
  if ((int)(uint)param_1 < in_ECX[1]) {
    uVar1 = *(undefined2 *)(*in_ECX + (uint)param_1 * 2);
  }
  else {
    uVar1 = 0xffff;
  }
  return uVar1;
}

```

## 0x006bc530 CSWSCreatureStats::GetBonusFeat

- Size 54 B, RET 4
- Callers (6 total, first 5 shown):
  - 0x0062f070 (site 0x0062f7be)
  - 0x00636050 (site 0x006361e3)
  - 0x0063f550 (site 0x006400cf)
  - 0x006415e0 (site 0x006417dd)
  - 0x00646270 (site 0x00646ce4)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns the bonus feat ID at an index (list at +0x18, count +0x1c), or 65535 if out of range.
- Named pseudo-C (first 17 of 17 lines):
```c
// NAMED 0x006bc530 CSWSCreatureStats::GetBonusFeat evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=1 exe=6a522e71631d

undefined2 CSWSCreatureStats__GetBonusFeat(ushort param_1)

{
  undefined2 uVar1;
  int in_ECX;
  
  if ((int)(uint)param_1 < *(int *)(in_ECX + 0x1c)) {
    uVar1 = *(undefined2 *)(*(int *)(in_ECX + 0x18) + (uint)param_1 * 2);
  }
  else {
    uVar1 = 0xffff;
  }
  return uVar1;
}

```

## 0x006bc570 CSWSCreatureStats::GetEffectImmunity

- Size 437 B, RET 8
- Callers (30 total, first 5 shown):
  - 0x00597d90 CSWSEffectListHandler::OnApplyDeath (site 0x00597f65)
  - 0x00599380 CSWSEffectListHandler::OnApplyAbilityDecrease (site 0x005993fd)
  - 0x00599e40 CSWSEffectListHandler::OnApplySavingThrowDecrease (site 0x00599ea6)
  - 0x0059a4b0 CSWSEffectListHandler::OnApplyACDecrease (site 0x0059a516)
  - 0x0059aa90 CSWSEffectListHandler::OnApplyAttackDecrease (site 0x0059ab2d)
- Callees (3):
  - 0x006bc691 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006bc69e CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006bc6ab CALL 0x005e4e60 CGameEffect::GetInteger
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Likely returns an immunity bitmask for an effect type; buckets the source's alignment (+0x18a) into three groups.
- Named pseudo-C (first 30 of 54 lines):
```c
// NAMED 0x006bc570 CSWSCreatureStats::GetEffectImmunity evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetEffectImmunity(byte param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  int in_ECX;
  int *piVar6;
  byte local_35;
  uint local_24;
  uint local_20;
  uint local_14;
  
  if (param_2 == 0) {
    local_14 = (uint)*(byte *)(DAT_00a1b4d0 + 0x122);
    local_24 = 0xffffffff;
  }
  else {
    local_14 = (uint)*(ushort *)(*(int *)(param_2 + 0x1198) + 0xe0);
    if (*(short *)(*(int *)(param_2 + 0x1198) + 0x18a) < 0x29) {
      local_35 = 3;
    }
    else if (*(short *)(*(int *)(param_2 + 0x1198) + 0x18a) < 0x3c) {
      local_35 = 1;
    }
    else {
```

## 0x006bc9a0 CSWSCreatureStats::GetEffectLinkImmunity

- Size 1112 B, RET 4
- Callers (6 total, first 5 shown):
  - 0x0059cba0 CSWSEffectListHandler::OnApplySetState (site 0x0059cc2c)
  - 0x00696010 Global::ExecuteCommandGetIsLinkImmune (site 0x006960c5)
  - 0x006bc9a0 (site 0x006bcdb2)
  - 0x006db4a0 CSWSCreature::ResolveRangedSpecialAttack (site 0x006db772)
  - 0x006dea90 CSWSCreature::ResolveMeleeAttack (site 0x006dee92)
- Callees (15):
  - 0x006bca15 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006bca23 CALL 0x006bc730 CSWSCreatureStats::CheckInclusionMatch
  - 0x006bca83 CALL 0x0071b8b0
  - 0x006bca98 CALL 0x006bc570 CSWSCreatureStats::GetEffectImmunity
  - 0x006bcca5 CALL 0x0051c100 CServerExoApp::GetCreatureByGameObjectID
  - 0x006bccaf CALL 0x00919723 operator new
  - 0x006bccca CALL 0x005e2540
  - 0x006bcd09 CALL 0x005e1c70
  - 0x006bcd16 CALL 0x00919723 operator new
  - 0x006bcd31 CALL 0x005e2540
  - 0x006bcd5c CALL 0x005e2be0
  - 0x006bcd6d CALL 0x00584da0 CSWSCreature::SendFeedbackMessage
  - 0x006bcd87 CALL 0x00584da0 CSWSCreature::SendFeedbackMessage
  - 0x006bcdb2 CALL 0x006bc9a0
  - 0x006bcdd9 CALL 0x006bc9a0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Returns the link-immunity bitmask for an effect: checks the inclusion match, then ORs per-slot immunity results.
- Named pseudo-C (first 30 of 149 lines):
```c
// NAMED 0x006bc9a0 CSWSCreatureStats::GetEffectLinkImmunity evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=postpass exe=6a522e71631d

uint CSWSCreatureStats__GetEffectLinkImmunity(int param_1)

{
  undefined2 uVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int in_ECX;
  undefined4 local_74;
  undefined4 local_70;
  uint local_24;
  uint local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c3c6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_1c = 0;
  if ((*(short *)(param_1 + 8) != 0x28) && (*(short *)(param_1 + 8) != 0x1e)) {
    uVar1 = *(undefined2 *)(param_1 + 8);
    uVar2 = CGameEffect__GetInteger(0);
    iVar3 = CSWSCreatureStats__CheckInclusionMatch(uVar1,uVar2);
```

## 0x006bce00 CSWSCreatureStats::AdjustSpellUsesPerDay

- Size 407 B, RET 0
- Callers (4 total):
  - 0x005724d0 CSWSCreature::UpdateAttributesOnEffect (site 0x00572870)
  - 0x006b6920 CSWSCreatureStats::SetINTBase (site 0x006b6953)
  - 0x006b6960 CSWSCreatureStats::SetWISBase (site 0x006b6993)
  - 0x006b69a0 CSWSCreatureStats::SetCHABase (site 0x006b69d3)
- Callees (8):
  - 0x006bce6b CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006bcead CALL 0x006a4130
  - 0x006bcec2 CALL 0x006a4cd0 CSWSCreatureStats::GetSpellGainWithBonus
  - 0x006bcef2 CALL 0x006a4160
  - 0x006bcf1d CALL 0x006a40d0
  - 0x006bcf3f CALL 0x006a40d0
  - 0x006bcf64 CALL 0x006a4100
  - 0x006bcf83 CALL 0x006a4100
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Likely recomputes spell uses per day for each force-user class, clamping remaining uses at zero.
- Named pseudo-C (first 30 of 46 lines):
```c
// NAMED 0x006bce00 CSWSCreatureStats::AdjustSpellUsesPerDay evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=postpass exe=6a522e71631d

void CSWSCreatureStats__AdjustSpellUsesPerDay(void)

{
  bool bVar1;
  char cVar2;
  char cVar3;
  int iVar4;
  uint uVar5;
  int in_ECX;
  undefined1 local_c;
  undefined1 local_8;
  
  for (local_8 = 0; local_8 < *(byte *)(in_ECX + 0x8d); local_8 = local_8 + 1) {
    if (local_8 < *(byte *)(in_ECX + 0x8d)) {
      local_c = *(undefined1 *)(in_ECX + 0xab + (uint)local_8 * 0x28);
    }
    else {
      local_c = 0xff;
    }
    iVar4 = IsForceUserClass(classId) {3,4,5,11..16}(local_c);
    if (iVar4 != 0) {
      bVar1 = false;
      while (!bVar1) {
        cVar2 = FUN_006a4130(0);
        cVar3 = CSWSCreatureStats__GetSpellGainWithBonus(local_8,0);
        cVar2 = cVar3 - cVar2;
        FUN_006a4160(0,cVar3);
        if (cVar2 < '\0') {
```

## 0x006bcfa0 CSWSCreatureStats::GetCreatureTalentRandomKnownPower

- Size 2145 B, RET 32
- Callers (1 total):
  - 0x006bae00 CSWSCreatureStats::GetCreatureTalentRandomPower (site 0x006bae97)
- Callees (38):
  - 0x006bcfd1 CALL 0x00919723 operator new
  - 0x006bcfee CALL 0x0084a640
  - 0x006bd01d CALL 0x00919723 operator new
  - 0x006bd03a CALL 0x0064de00
  - 0x006bd089 CALL 0x006b8800 CSWSCreatureStats::GetNumberKnownSpells
  - 0x006bd0b0 CALL 0x006b8800 CSWSCreatureStats::GetNumberKnownSpells
  - 0x006bd0d5 CALL 0x006b87b0 CSWSCreatureStats::GetKnownSpell
  - 0x006bd0f9 CALL 0x006c1450
  - 0x006bd112 CALL 0x005453c0
  - 0x006bd145 CALL 0x0054b4f0 CSWSArea::GetRoomForceRating
  - 0x006bd1be CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006bd26c CALL 0x006c4120
  - 0x006bd374 indirect
  - 0x006bd39d indirect
  - 0x006bd433 CALL 0x006c4120
  - 0x006bd4ea CALL 0x006ba970 CSWSCreatureStats::IsSpellTalentCandidate
  - 0x006bd504 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bd511 CALL 0x0064de40
  - 0x006bd51c CALL 0x00734010
  - 0x006bd559 CALL 0x0064ddb0
  - 0x006bd56c CALL 0x009196fd operator delete
  - 0x006bd59e CALL 0x0064ddb0
  - 0x006bd5b1 CALL 0x009196fd operator delete
  - 0x006bd5d0 CALL 0x00919723 operator new
  - 0x006bd5f6 CALL 0x0084a640
  - 0x006bd62b CALL 0x00919723 operator new
  - 0x006bd651 CALL 0x0064de00
  - 0x006bd68b CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bd698 CALL 0x0064de40
  - 0x006bd6a3 CALL 0x00734010
  - 0x006bd6cb CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bd6d8 CALL 0x0064de40
  - 0x006bd6e3 CALL 0x00734010
  - 0x006bd6fc CALL 0x0091eb8b
  - 0x006bd75c CALL 0x0064ddb0
  - 0x006bd772 CALL 0x009196fd operator delete
  - 0x006bd7b6 CALL 0x0064ddb0
  - 0x006bd7cc CALL 0x009196fd operator delete
- Imports: none
- Indirect slots (2):
  - 0x006bd374 slot +0x9c (006bd36e MOV EAX,dword ptr [EDX + 0x9c])
  - 0x006bd39d slot +0x9c (006bd397 MOV EAX,dword ptr [EDX + 0x9c])
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Likely picks a random known power for a creature by collecting known spell candidates into lists, then choosing one.
- Named pseudo-C (first 30 of 184 lines):
```c
// NAMED 0x006bcfa0 CSWSCreatureStats::GetCreatureTalentRandomKnownPower evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=postpass exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006bd486) */
/* WARNING: Removing unreachable block (ram,0x006bd43f) */
/* WARNING: Removing unreachable block (ram,0x006bd2cf) */
/* WARNING: Removing unreachable block (ram,0x006bd316) */

bool CSWSCreatureStats__GetCreatureTalentRandomKnownPower
               (undefined4 param_1,int param_2,undefined4 *param_3,undefined4 *param_4,
               undefined1 *param_5,uint param_6,undefined4 param_7,undefined4 param_8)

{
  ushort uVar1;
  short sVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  int in_ECX;
  int *local_114;
  int *local_110;
  int *local_f0;
  int *local_ec;
  int local_5c;
  undefined4 local_48;
  int local_40;
  int local_3c;
  int *local_28;
  int *local_18;
  byte local_11;
```

## 0x006bd810 CSWSCreatureStats::GetSpellLikeAbilityCasterLevel

- Size 120 B, RET 4
- Callers (2 total):
  - 0x0066e7e0 Global::ExecuteCommandCastSpell (site 0x0066ef05)
  - 0x0067a620 Global::ExecuteCommandGetCasterLevel (site 0x0067a81b)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns the caster level stored for a spell-like ability ID in its list (+0x134), or 255 if absent.
- Named pseudo-C (first 22 of 22 lines):
```c
// NAMED 0x006bd810 CSWSCreatureStats::GetSpellLikeAbilityCasterLevel evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetSpellLikeAbilityCasterLevel(int param_1)

{
  int iVar1;
  int in_ECX;
  int local_8;
  
  local_8 = 0;
  while( true ) {
    iVar1 = *(int *)(*(int *)(in_ECX + 0x134) + 4);
    if (iVar1 <= local_8) {
      return CONCAT31((int3)((uint)iVar1 >> 8),0xff);
    }
    if (*(int *)(local_8 * 0xc + **(int **)(in_ECX + 0x134)) == param_1) break;
    local_8 = local_8 + 1;
  }
  return CONCAT31((int3)((uint)**(int **)(in_ECX + 0x134) >> 8),
                  *(undefined1 *)(**(int **)(in_ECX + 0x134) + 8 + local_8 * 0xc));
}

```

## 0x006bd890 CSWSCreatureStats::AdjustHPAccumulatorBy

- Size 112 B, RET 4
- Callers (1 total):
  - 0x0056a230 CSWSCreature::HandleHPRegeneration (site 0x0056a72a)
- Callees (2):
  - 0x006bd8c3 CALL 0x0091ed90
  - 0x006bd8d1 CALL 0x0091c860
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Adds a float delta to the HP accumulator at +0x1bc, then returns and removes its whole-number part.
- Named pseudo-C (first 15 of 15 lines):
```c
// NAMED 0x006bd890 CSWSCreatureStats::AdjustHPAccumulatorBy evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=1 exe=6a522e71631d

int CSWSCreatureStats__AdjustHPAccumulatorBy(float param_1)

{
  short sVar1;
  int in_ECX;
  
  *(float *)(in_ECX + 0x1bc) = *(float *)(in_ECX + 0x1bc) + param_1;
  FUN_0091ed90((double)*(float *)(in_ECX + 0x1bc));
  sVar1 = FUN_0091c860();
  *(float *)(in_ECX + 0x1bc) = *(float *)(in_ECX + 0x1bc) - (float)(int)sVar1;
  return (int)sVar1;
}

```

## 0x006bd9c0 CSWSCreatureStats::HasSpell

- Size 781 B, RET 12
- Callers (17 total, first 5 shown):
  - 0x005a8860 CSWSEffectListHandler::OnApplyLightsaberThrow (site 0x005a88c6)
  - 0x0067e590 Global::ExecuteCommandGetHasSpell (site 0x0067e623)
  - 0x00691020 Global::ExecuteCommandSetCurrentForm (site 0x006910b9)
  - 0x006a5110 CSWSCreatureStats::GetArmorClassVersus (site 0x006a6d76)
  - 0x006a93a0 CSWSCreatureStats::GetAttackModifierVersus (site 0x006ab0a3)
- Callees (8):
  - 0x006bda17 CALL 0x006a41c0
  - 0x006bda52 CALL 0x006c1450
  - 0x006bda60 CALL 0x005453c0
  - 0x006bda8b CALL 0x0054b4f0 CSWSArea::GetRoomForceRating
  - 0x006bdaf6 CALL 0x005e4e60 CGameEffect::GetInteger
  - 0x006bdb7a CALL 0x006c4120
  - 0x006bdc12 indirect
  - 0x006bdca8 CALL 0x006c4120
- Imports: none
- Indirect slots (1):
  - 0x006bdc12 slot +0x9c (006bdc0c MOV EAX,dword ptr [EDX + 0x9c])
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched (1):
  - 0x130 CSWSCreatureStats::Force points (current / extra) [+0x130] (i16) R
- KDB purpose: Returns whether any class knows the spell or power; with the flag set, also checks room force rating (likely).
- Named pseudo-C (first 30 of 84 lines):
```c
// NAMED 0x006bd9c0 CSWSCreatureStats::HasSpell evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=3 exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006bdbcb) */
/* WARNING: Removing unreachable block (ram,0x006bdc67) */

int CSWSCreatureStats__HasSpell(undefined1 param_1,undefined4 param_2,int param_3)

{
  short sVar1;
  int iVar2;
  int in_ECX;
  short local_7c;
  short local_78;
  int local_38;
  undefined4 local_28;
  int local_20;
  int local_1c;
  byte local_d;
  int local_c;
  int local_8;
  
  local_c = 0;
  local_8 = 0;
  for (local_d = 0; local_d < *(byte *)(in_ECX + 0x8d); local_d = local_d + 1) {
    iVar2 = FUN_006a41c0(param_1,param_2);
    if (iVar2 != 0) {
      local_c = 1;
    }
  }
  if (param_3 == 0) {
```

## 0x006bdcd0 CSWSCreatureStats::AddClass

- Size 136 B, RET 8
- Callers (1 total):
  - 0x0066c7a0 Global::ExecuteCommandAddMultiClass (site 0x0066c866)
- Callees (3):
  - 0x006bdd12 CALL 0x006b69e0 CSWSCreatureStats::SetClass
  - 0x006bdd2d CALL 0x006b6a20 CSWSCreatureStats::SetClassLevel
  - 0x006bdd4d CALL 0x0058b7f0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Adds a class slot (max two), sets its class and level, then notifies the owner if present.
- Named pseudo-C (first 18 of 18 lines):
```c
// NAMED 0x006bdcd0 CSWSCreatureStats::AddClass evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=3 exe=6a522e71631d

void CSWSCreatureStats__AddClass(undefined1 param_1,undefined1 param_2)

{
  int in_ECX;
  
  if (*(byte *)(in_ECX + 0x8d) < 2) {
    *(char *)(in_ECX + 0x8d) = *(char *)(in_ECX + 0x8d) + '\x01';
  }
  CSWSCreatureStats__SetClass(*(byte *)(in_ECX + 0x8d) - 1,param_1);
  CSWSCreatureStats__SetClassLevel(*(byte *)(in_ECX + 0x8d) - 1,param_2);
  if (*(int *)(in_ECX + 0x24) != 0) {
    FUN_0058b7f0();
  }
  return;
}

```

## 0x006bdd60 CSWSCreatureStats::CheckInclusionMatch

- Size 205 B, RET 8
- Callers (3 total):
  - 0x006ba970 CSWSCreatureStats::IsSpellTalentCandidate (site 0x006ba9cc)
  - 0x006bb3a0 CSWSCreatureStats::GetCreatureTalentRandomPowerFromItem (site 0x006bbc38)
  - 0x006bc0e0 CSWSCreatureStats::GetCreatureTalentRandomFeat (site 0x006bc1c1)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns 1 if every hex nibble of the second value is a wildcard or matches the first value.
- Named pseudo-C (first 23 of 23 lines):
```c
// NAMED 0x006bdd60 CSWSCreatureStats::CheckInclusionMatch evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__CheckInclusionMatch(uint param_1,uint param_2)

{
  undefined4 local_10;
  
  local_10 = 1;
  if (((param_2 & 0xf000) != 0xf000) && ((param_2 & 0xf000) != (param_1 & 0xf000))) {
    local_10 = 0;
  }
  if (((param_2 & 0xf00) != 0xf00) && ((param_2 & 0xf00) != (param_1 & 0xf00))) {
    local_10 = 0;
  }
  if (((param_2 & 0xf0) != 0xf0) && ((param_2 & 0xf0) != (param_1 & 0xf0))) {
    local_10 = 0;
  }
  if (((param_2 & 0xf) != 0xf) && ((param_2 & 0xf) != (param_1 & 0xf))) {
    local_10 = 0;
  }
  return local_10;
}

```

## 0x006be0a0 CSWSCreatureStats::SetNormalBonusFlags

- Size 171 B, RET 12
- Callers (1 total):
  - 0x006be150 CSWSCreatureStats::CanChooseFeat (site 0x006be304)
- Callees (4):
  - 0x006be0cd CALL 0x006a20f0
  - 0x006be0e0 CALL 0x006be040 CSWSCreatureStats::GetIsInSpellLikeAbilityList
  - 0x006be10b CALL 0x006f2690
  - 0x006be12c CALL 0x006f2610
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Clears two out-flags, then sets each if a feat-specific helper check passes, gated on a class entry (likely).
- Named pseudo-C (first 27 of 27 lines):
```c
// NAMED 0x006be0a0 CSWSCreatureStats::SetNormalBonusFlags evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=2 exe=6a522e71631d

void CSWSCreatureStats__SetNormalBonusFlags
               (undefined2 param_1,undefined4 *param_2,undefined4 *param_3)

{
  byte bVar1;
  int iVar2;
  
  *param_2 = 0;
  *param_3 = 0;
  iVar2 = FUN_006a20f0(param_1);
  if ((iVar2 != 0) &&
     (bVar1 = CSWSCreatureStats__GetIsInSpellLikeAbilityList(),
     (uint)bVar1 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124) != 0)) {
    iVar2 = FUN_006f2690(param_1);
    if (iVar2 != 0) {
      *param_2 = 1;
    }
    iVar2 = FUN_006f2610(param_1);
    if (iVar2 != 0) {
      *param_3 = 1;
    }
  }
  return;
}

```

## 0x006be150 CSWSCreatureStats::CanChooseFeat

- Size 1181 B, RET 20
- Callers (1 total):
  - 0x006bedf0 CSWSCreatureStats::AutoLevelup (site 0x006bfb4c)
- Callees (19):
  - 0x006be1b0 CALL 0x006a20f0
  - 0x006be1e4 CALL 0x006b6b30 CSWSCreatureStats::GetTag
  - 0x006be1eb CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006be21a CALL 0x006b6b30 CSWSCreatureStats::GetTag
  - 0x006be221 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006be27b CALL 0x006bc430 CSWSCreatureStats::GetFeat_2
  - 0x006be2e4 CALL 0x006beb10 CSWSCreatureStats::FeatRequirementsMet
  - 0x006be304 CALL 0x006be0a0 CSWSCreatureStats::SetNormalBonusFlags
  - 0x006be334 CALL 0x0084a640
  - 0x006be350 CALL 0x00840ef0
  - 0x006be3ca CALL 0x006be0a0 CSWSCreatureStats::SetNormalBonusFlags
  - 0x006be3f4 CALL 0x0064ddb0
  - 0x006be444 CALL 0x0064ddb0
  - 0x006be47d CALL 0x006be0a0 CSWSCreatureStats::SetNormalBonusFlags
  - 0x006be4b9 CALL 0x0064ddb0
  - 0x006be4fc CALL 0x0064ddb0
  - 0x006be572 CALL 0x0064ddb0
  - 0x006be5ba CALL 0x0064ddb0
  - 0x006be5d5 CALL 0x0064ddb0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Likely decides if a creature can choose a feat: rejects two reserved IDs, then checks tags and prerequisites.
- Named pseudo-C (first 30 of 176 lines):
```c
// NAMED 0x006be150 CSWSCreatureStats::CanChooseFeat evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=6 exe=6a522e71631d

undefined4
CSWSCreatureStats__CanChooseFeat
          (short param_1,char param_2,char param_3,int *param_4,undefined4 param_5)

{
  short sVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  int in_ECX;
  char *pcVar5;
  ushort local_60;
  char local_39;
  int local_38;
  ushort local_34;
  char local_2d;
  int local_2c;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c448;
  local_10 = ExceptionList;
```

## 0x006be5f0 CSWSCreatureStats::CanChooseSpell

- Size 373 B, RET 16
- Callers (1 total):
  - 0x006bedf0 CSWSCreatureStats::AutoLevelup (site 0x006c0061)
- Callees (4):
  - 0x006be62b CALL 0x006c1450
  - 0x006be6ab CALL 0x006c10a0
  - 0x006be6dc CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006be735 CALL 0x006be820 CSWSCreatureStats::HasSpellPrereq
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Likely validates a spell choice: entry type 1 or 5, valid class, and spell not already known.
- Named pseudo-C (first 30 of 64 lines):
```c
// NAMED 0x006be5f0 CSWSCreatureStats::CanChooseSpell evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=3 exe=6a522e71631d

undefined4
CSWSCreatureStats__CanChooseSpell(int param_1,byte param_2,int *param_3,undefined4 param_4)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int in_ECX;
  byte bVar5;
  undefined1 local_20;
  ushort local_10;
  
  bVar5 = *(char *)(in_ECX + 0x8d) - 1;
  iVar2 = FUN_006c1450(param_1);
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else if ((*(char *)(iVar2 + 0x14c) == '\x01') || (*(char *)(iVar2 + 0x14c) == '\x05')) {
    if (bVar5 < *(byte *)(in_ECX + 0x8d)) {
      local_20 = *(undefined1 *)(in_ECX + 0xab + (uint)bVar5 * 0x28);
    }
    else {
      local_20 = 0xff;
    }
    cVar1 = FUN_006c10a0(local_20);
    if (cVar1 == -1) {
      uVar3 = 0;
```

## 0x006be770 CSWSCreatureStats::FeatAcquired

- Size 161 B, RET 8
- Callers (1 total):
  - 0x006bea00 CSWSCreatureStats::HasFeatPrereq (site 0x006bea2c)
- Callees (2):
  - 0x006be793 CALL 0x0064dc30
  - 0x006be7a7 CALL 0x0064dc30
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns 1 if a feat is held, checking two feat sets then the supplied list (likely).
- Named pseudo-C (first 29 of 29 lines):
```c
// NAMED 0x006be770 CSWSCreatureStats::FeatAcquired evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__FeatAcquired(short param_1,int *param_2)

{
  undefined4 uVar1;
  int iVar2;
  ushort local_c;
  
  if (param_1 == -1) {
    uVar1 = 0;
  }
  else {
    iVar2 = FUN_0064dc30(param_1);
    if ((iVar2 == 0) && (iVar2 = FUN_0064dc30(param_1), iVar2 == 0)) {
      if (param_2 != (int *)0x0) {
        for (local_c = 0; (int)(uint)local_c < param_2[1]; local_c = local_c + 1) {
          if (param_1 == *(short *)(*param_2 + (uint)local_c * 2)) {
            return 1;
          }
        }
      }
      return 0;
    }
    uVar1 = 1;
  }
  return uVar1;
}

```

## 0x006be820 CSWSCreatureStats::HasSpellPrereq

- Size 469 B, RET 12
- Callers (1 total):
  - 0x006be5f0 CSWSCreatureStats::CanChooseSpell (site 0x006be735)
- Callees (8):
  - 0x006be858 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006be868 CALL 0x004142e0
  - 0x006be8d8 CALL 0x006bd9c0 CSWSCreatureStats::HasSpell
  - 0x006be8e8 CALL 0x004142e0
  - 0x006be957 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006be96a CALL 0x006c10a0
  - 0x006be9c9 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006be9d9 CALL 0x006c10a0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Likely checks that all spell prerequisites in two lists are known, returning 0 if any is missing.
- Named pseudo-C (first 30 of 65 lines):
```c
// NAMED 0x006be820 CSWSCreatureStats::HasSpellPrereq evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=3 exe=6a522e71631d

undefined4 CSWSCreatureStats__HasSpellPrereq(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  byte bVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  int in_ECX;
  byte bVar6;
  undefined1 local_28;
  undefined1 local_24;
  int local_c;
  int local_8;
  
  local_8 = 0;
  local_c = *(int *)(param_1 + 0x180);
  while (local_c != -1) {
    iVar3 = CSWSCreatureStats__HasSpell(0,local_c,1);
    if ((iVar3 == 0) && (iVar3 = FUN_004142e0(local_c), iVar3 == -1)) {
      return 0;
    }
    local_8 = local_8 + 1;
    local_c = *(int *)(param_1 + 0x180 + local_8 * 4);
  }
  for (local_8 = 0; *(int *)(param_1 + 0x16c + local_8 * 4) != -1; local_8 = local_8 + 1) {
    uVar1 = *(undefined4 *)(param_1 + 0x16c + local_8 * 4);
    iVar3 = CSWSCreatureStats__HasSpell(0,uVar1,1);
```

## 0x006bea00 CSWSCreatureStats::HasFeatPrereq

- Size 267 B, RET 8
- Callers (1 total):
  - 0x006beb10 CSWSCreatureStats::FeatRequirementsMet (site 0x006bed9f)
- Callees (3):
  - 0x006bea2c CALL 0x006be770 CSWSCreatureStats::FeatAcquired
  - 0x006bea5f CALL 0x006be770 CSWSCreatureStats::FeatAcquired
  - 0x006beade CALL 0x006be770 CSWSCreatureStats::FeatAcquired
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Checks feat prereqs: two required feats must be held, plus one of five optional feats if any are listed.
- Named pseudo-C (first 30 of 46 lines):
```c
// NAMED 0x006bea00 CSWSCreatureStats::HasFeatPrereq evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=2 exe=6a522e71631d

undefined4 CSWSCreatureStats__HasFeatPrereq(int param_1,undefined4 param_2)

{
  bool bVar1;
  bool bVar2;
  int iVar3;
  undefined4 uVar4;
  uint local_14;
  short local_c;
  
  if ((*(short *)(param_1 + 0x38) != -1) &&
     (iVar3 = CSWSCreatureStats__FeatAcquired(*(short *)(param_1 + 0x38),param_2), iVar3 == 0)) {
    return 0;
  }
  if ((*(short *)(param_1 + 0x3a) != -1) &&
     (iVar3 = CSWSCreatureStats__FeatAcquired(*(short *)(param_1 + 0x3a),param_2), iVar3 == 0)) {
    return 0;
  }
  bVar2 = false;
  bVar1 = false;
  for (local_14 = 0; (!bVar1 && ((int)local_14 < 5)); local_14 = local_14 + 1) {
    if ((byte)local_14 < 5) {
      local_c = *(short *)(param_1 + 0x3c + (local_14 & 0xff) * 2);
    }
    else {
      local_c = -1;
    }
    if (local_c != -1) {
```

## 0x006beb10 CSWSCreatureStats::FeatRequirementsMet

- Size 724 B, RET 12
- Callers (1 total):
  - 0x006be150 CSWSCreatureStats::CanChooseFeat (site 0x006be2e4)
- Callees (10):
  - 0x006beb2c CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006beb41 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006beb57 CALL 0x006a20f0
  - 0x006beb6f CALL 0x006be040 CSWSCreatureStats::GetIsInSpellLikeAbilityList
  - 0x006beb98 CALL 0x006f2580
  - 0x006beca9 CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006becb5 CALL 0x006f0a30
  - 0x006becea CALL 0x006a6f80 CSWSCreatureStats::GetBaseAttackBonus
  - 0x006bed9f CALL 0x006bea00 CSWSCreatureStats::HasFeatPrereq
  - 0x006bedcb CALL 0x006b7a00 CSWSCreatureStats::GetCanUseSkill
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (2):
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Checks a feat's level, skill, and base-attack requirements against the creature, plus prerequisite feats (likely).
- Named pseudo-C (first 30 of 105 lines):
```c
// NAMED 0x006beb10 CSWSCreatureStats::FeatRequirementsMet evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=7 exe=6a522e71631d

undefined4 CSWSCreatureStats__FeatRequirementsMet(undefined2 param_1,undefined4 param_2,int param_3)

{
  bool bVar1;
  byte bVar2;
  undefined1 uVar3;
  char cVar4;
  uint uVar5;
  int iVar6;
  undefined4 uVar7;
  uint uVar8;
  int iVar9;
  int in_ECX;
  uint uVar10;
  uint uVar11;
  byte local_25;
  byte local_1d;
  uint local_10;
  undefined1 local_c;
  
  if (param_3 == 0) {
    local_10 = CSWSCreatureStats__GetLevel(0);
    local_10 = local_10 & 0xff;
  }
  else {
    uVar5 = CSWSCreatureStats__GetLevel(0);
    local_10 = (uVar5 & 0xff) + 1;
  }
```

## 0x006bedf0 CSWSCreatureStats::AutoLevelup

- Size 5244 B, RET 4
- Callers (6 total, first 5 shown):
  - 0x005fd480 CSWPartyTable::SpawnNPC (site 0x005fd7ee)
  - 0x005fe0c0 CSWPartyTable::DistributeExperience (site 0x005fe2bd)
  - 0x005fedd0 CSWPartyTable::AutoLevelUpNPCs (site 0x005fee4c)
  - 0x0066c7a0 Global::ExecuteCommandAddMultiClass (site 0x0066c976)
  - 0x0074ee90 (site 0x0074f47a)
- Callees (72):
  - 0x006bee24 CALL 0x005933b0
  - 0x006bee35 CALL 0x0084a640
  - 0x006beeee CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006bef2e CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006bef44 CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006bef54 CALL 0x00919723 operator new
  - 0x006bef75 CALL 0x006f58b0 PendingLevelUp::Constructor
  - 0x006bf06c CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
  - 0x006bf0ae CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
  - 0x006bf111 CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
  - 0x006bf153 CALL 0x006b6af0 CSWSCreatureStats::CalcStatModifier
  - 0x006bf1ae CALL 0x0051c8d0
  - 0x006bf226 CALL 0x006f2970
  - 0x006bf274 CALL 0x006f24d0
  - 0x006bf289 CALL 0x006f2440
  - 0x006bf29d CALL 0x006bc470
  - 0x006bf2d5 CALL 0x006b7bb0 CSWSCreatureStats::GetSkillRank
  - 0x006bf316 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bf409 CALL 0x006f5b00 PendingLevelUp::SetSkillDelta
  - 0x006bf4ae CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006bf4eb CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006bf50c CALL 0x006ad2f0 CSWSCreatureStats::GetClassLevel
  - 0x006bf560 CALL 0x006f2710
  - 0x006bf5fb CALL 0x006f2a30 IsForceUserClass(classId) {3,4,5,11..16}
  - 0x006bf61b CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006bf640 CALL 0x006a20f0
  - 0x006bf668 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bf6c1 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bf707 CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006bf72e CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bf742 CALL 0x006a20f0
  - 0x006bf799 CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006bf7c0 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bf7d4 CALL 0x006a20f0
  - 0x006bf82b CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006bf852 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bf866 CALL 0x006a20f0
  - 0x006bf8bd CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006bf8e4 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bf8f8 CALL 0x006a20f0
  - 0x006bf94f CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006bf976 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bf98a CALL 0x006a20f0
  - 0x006bf9e1 CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006bfa08 CALL 0x00734f60 CExoString::CompareNoCase(char*)
  - 0x006bfa1c CALL 0x006a20f0
  - 0x006bfa73 CALL 0x006b8280 CSWSCreatureStats::AddFeat
  - 0x006bfa90 CALL 0x006bdf80
  - 0x006bfaeb CALL 0x006f28b0
  - 0x006bfb08 CALL 0x006a20f0
  - 0x006bfb4c CALL 0x006be150 CSWSCreatureStats::CanChooseFeat
  - 0x006bfb5d CALL 0x006f5b40 PendingLevelUp::AddFeat
  - 0x006bfb6a CALL 0x00593450 CExoArrayList<uint16>::Add
  - 0x006bfbaf CALL 0x006babd0 CSWSCreatureStats::SetSpellLikeAbilityReady
  - 0x006bff84 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006bff93 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006c0036 CALL 0x006f2d50
  - 0x006c0061 CALL 0x006be5f0 CSWSCreatureStats::CanChooseSpell
  - 0x006c0077 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006c0084 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x006c00aa CALL 0x006b9640 CSWSCreatureStats::ValidateLevelUp
  - 0x006c00d7 CALL 0x006f59b0
  - 0x006c00ed CALL 0x009196fd operator delete
  - 0x006c017e CALL 0x0051c8d0
  - 0x006c0185 CALL 0x005faaf0 CSWPartyTable::GetNPCObject
  - 0x006c01b9 CALL 0x0051c100 CServerExoApp::GetCreatureByGameObjectID
  - 0x006c01d3 CALL 0x0056ae40 CSWSCreature::HandleAlignmentInfluence
  - 0x006c021a CALL 0x006a4e00 CSWSCreatureStats::GetLevel
  - 0x006c0232 CALL 0x0073fec0 CClientExoApp::ServerToClientObjectId
  - 0x006c023e CALL 0x007408c0 CClientExoApp::AddFloatyText
  - 0x006c024a CALL 0x0064ddb0
  - 0x006c0259 CALL 0x0064ddb0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (5):
  - 0x00a1b4a4 READ (MOV EAX,[0x00a1b4a4])
  - 0x00a1b4a4 READ (MOV EDX,dword ptr [0x00a1b4a4])
  - 0x00a1b4d0 READ (MOV EAX,[0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV ECX,dword ptr [0x00a1b4d0])
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Likely auto-picks level-up stats, skills and feats into a PendingLevelUp record for the creature.
- Named pseudo-C (first 30 of 505 lines):
```c
// NAMED 0x006bedf0 CSWSCreatureStats::AutoLevelup evidence=ev_98c1b11a40548ec632bce2d0d4f13d7b9e1f57eaa24c75d026a36c97666d6227 names=postpass exe=6a522e71631d

/* WARNING: Removing unreachable block (ram,0x006befba) */

void CSWSCreatureStats__AutoLevelup(int param_1)

{
  bool bVar1;
  bool bVar2;
  bool bVar3;
  byte bVar4;
  char cVar5;
  ushort uVar6;
  int iVar7;
  int iVar8;
  int in_ECX;
  uint uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  uint local_1a4;
  uint local_1a0;
  int local_198;
  int local_194;
  int local_190;
  char local_16f;
  undefined1 local_118;
  undefined1 local_111;
  int local_bc;
  undefined1 local_b5;
  int local_b4;
```

## 0x006c0330 CSWSCreatureStats::GetHasPowerFromItem

- Size 882 B, RET 4
- Callers (1 total):
  - 0x006b8840 CSWSCreatureStats::GetCreatureHasTalent (site 0x006b8a6c)
- Callees (14):
  - 0x006c03a3 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006c03bc CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006c03fa CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c0420 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c0444 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c049d CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c04de CALL 0x0058a2c0 CSWSCreature::GetItemRepository
  - 0x006c051e CALL 0x00606100 CItemRepository::ItemListGetItem
  - 0x006c054c CALL 0x006d5200
  - 0x006c056f CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006c05ad CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c05d3 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c0609 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c0661 CALL 0x00604260 CSWSItem::GetActiveProperty
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched (1):
  - 0x288 CSWSItem::property count (int) R
- KDB purpose: Returns 1 if an equipped item in any of 20 slots grants the power via a type-10 item property.
- Named pseudo-C (first 30 of 72 lines):
```c
// NAMED 0x006c0330 CSWSCreatureStats::GetHasPowerFromItem evidence=ev_73bef16e8b20df07f75c6f713c93d2d2641ed01b91095c5c60d0bdfcab09ebfb names=6 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetHasPowerFromItem(uint param_1)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  short *psVar4;
  int iVar5;
  int in_ECX;
  int local_20;
  int local_18;
  uint local_8;
  
  if (*(int *)(*(int *)(in_ECX + 0x24) + 0x1150) != 0) {
    for (local_8 = 0; local_8 < 0x14; local_8 = local_8 + 1) {
      iVar2 = CSWInventory__GetItemInSlot(1 << ((byte)local_8 & 0x1f));
      if ((iVar2 != 0) && (iVar3 = CSWSItem__GetPropertyByTypeExists(10,0), iVar3 != 0)) {
        for (local_18 = 0; local_18 < *(int *)(iVar2 + 0x288); local_18 = local_18 + 1) {
          psVar4 = (short *)CSWSItem__GetActiveProperty(local_18);
          if ((*psVar4 == 10) &&
             (iVar3 = CSWSItem__GetActiveProperty(local_18), *(char *)(iVar3 + 0xc) != '\0')) {
            iVar3 = CSWSItem__GetActiveProperty(local_18);
            if ((*(byte *)(iVar3 + 0x11) == 0xff) ||
               ((1 << (*(byte *)(iVar3 + 0x11) & 0x1f) & *(uint *)(iVar2 + 0x2d4)) != 0)) {
              bVar1 = true;
            }
            else {
              bVar1 = false;
```

## 0x006c06b0 CSWSCreatureStats::GetItemDetailsForPower

- Size 947 B, RET 12
- Callers (2 total):
  - 0x0057d8b0 CSWSCreature::AddUseTalentOnObjectActions (site 0x0057d9b4)
  - 0x0057da90 CSWSCreature::AddUseTalentAtLocationActions (site 0x0057db6c)
- Callees (14):
  - 0x006c0723 CALL 0x006d0620 CSWInventory::GetItemInSlot
  - 0x006c073c CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006c077a CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c07a0 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c07c4 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c081d CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c0877 CALL 0x0058a2c0 CSWSCreature::GetItemRepository
  - 0x006c08b7 CALL 0x00606100 CItemRepository::ItemListGetItem
  - 0x006c08e5 CALL 0x006d5200
  - 0x006c0908 CALL 0x00603f60 CSWSItem::GetPropertyByTypeExists
  - 0x006c0946 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c096c CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c09a2 CALL 0x00604260 CSWSItem::GetActiveProperty
  - 0x006c0a03 CALL 0x00604260 CSWSItem::GetActiveProperty
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals (1):
  - 0x00a1b4d0 READ (MOV EDX,dword ptr [0x00a1b4d0])
- Struct fields touched: none
- KDB purpose: Finds an equipped item granting the power (type-10 property) and returns its object ID and property index via out-params.
- Named pseudo-C (first 30 of 76 lines):
```c
// NAMED 0x006c06b0 CSWSCreatureStats::GetItemDetailsForPower evidence=ev_73bef16e8b20df07f75c6f713c93d2d2641ed01b91095c5c60d0bdfcab09ebfb names=6 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetItemDetailsForPower(uint param_1,undefined4 *param_2,int *param_3)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  short *psVar4;
  int iVar5;
  int in_ECX;
  int local_20;
  int local_18;
  uint local_8;
  
  if (*(int *)(*(int *)(in_ECX + 0x24) + 0x1150) != 0) {
    for (local_8 = 0; local_8 < 0x14; local_8 = local_8 + 1) {
      iVar2 = CSWInventory__GetItemInSlot(1 << ((byte)local_8 & 0x1f));
      if ((iVar2 != 0) && (iVar3 = CSWSItem__GetPropertyByTypeExists(10,0), iVar3 != 0)) {
        for (local_18 = 0; local_18 < *(int *)(iVar2 + 0x288); local_18 = local_18 + 1) {
          psVar4 = (short *)CSWSItem__GetActiveProperty(local_18);
          if ((*psVar4 == 10) &&
             (iVar3 = CSWSItem__GetActiveProperty(local_18), *(char *)(iVar3 + 0xc) != '\0')) {
            iVar3 = CSWSItem__GetActiveProperty(local_18);
            if ((*(byte *)(iVar3 + 0x11) == 0xff) ||
               ((1 << (*(byte *)(iVar3 + 0x11) & 0x1f) & *(uint *)(iVar2 + 0x2d4)) != 0)) {
              bVar1 = true;
            }
            else {
              bVar1 = false;
```

## 0x00844e40 CSWSCreatureStats::GetClass

- Size 51 B, RET 4
- Callers (43 total, first 5 shown):
  - 0x0076aed0 (site 0x0076b6bf)
  - 0x007791a0 (site 0x00779248)
  - 0x0077a9e0 (site 0x0077abbb)
  - 0x0077d890 (site 0x0077d988)
  - 0x007fefb0 (site 0x007ff550)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns the byte at +0xf8 plus slot*0x20 for slot 0 or 1, else 255; likely a class ID.
- Named pseudo-C (first 17 of 17 lines):
```c
// NAMED 0x00844e40 CSWSCreatureStats::GetClass evidence=ev_73bef16e8b20df07f75c6f713c93d2d2641ed01b91095c5c60d0bdfcab09ebfb names=1 exe=6a522e71631d

undefined1 CSWSCreatureStats__GetClass(byte param_1)

{
  undefined1 uVar1;
  int in_ECX;
  
  if (param_1 < 2) {
    uVar1 = *(undefined1 *)(in_ECX + 0xf8 + (uint)param_1 * 0x20);
  }
  else {
    uVar1 = 0xff;
  }
  return uVar1;
}

```

## 0x0089aef0 CSWSCreatureStats::GetClassByPosition

- Size 60 B, RET 4
- Callers (1 total):
  - 0x0089aaa0 CSWGuiLevelUpIconGrid::BuildPowerRows (site 0x0089abac)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: Returns the class ID at a list position, or 255 if the position is past the class count.
- Named pseudo-C (first 18 of 18 lines):
```c
// NAMED 0x0089aef0 CSWSCreatureStats::GetClassByPosition evidence=ev_73bef16e8b20df07f75c6f713c93d2d2641ed01b91095c5c60d0bdfcab09ebfb names=1 exe=6a522e71631d

undefined4 CSWSCreatureStats__GetClassByPosition(byte param_1)

{
  undefined4 uVar1;
  int in_ECX;
  
  if (param_1 < *(byte *)(in_ECX + 0x8d)) {
    uVar1 = CONCAT31((int3)((uint)param_1 * 0x28 >> 8),
                     *(undefined1 *)(in_ECX + 0xab + (uint)param_1 * 0x28));
  }
  else {
    uVar1 = 0xff;
  }
  return uVar1;
}

```
