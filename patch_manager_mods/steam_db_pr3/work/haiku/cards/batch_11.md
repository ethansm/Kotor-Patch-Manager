## R297 FUNCTION CWorldTimer::SubtractWorldTimes  steam=0x0051af60
GOG build: param_bytes=24 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5, undefined4 param_6'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD lhsDays, DWORD lhsMS, DWORD rhsDays, DWORD rhsMS, DWORD* outDays, DWORD* outMS) param_bytes=24
Steam function at this address: size=115 ret_N=24
```c
// FN 0051af60 FUN_0051af60 size=115 ret=24 callers=00801ab0,007a6760,005d4830,007664b0,0056b8a0,0056c310,005b96f0,005d4460,008e59c0,005d29f0,00557260,005d4700,0086da30,00623330,0062c360,00544ad0,005d18c0,00617980,007f12d0,00569320,0087b240,00600f20,005f4c00,005b8000,00521e60,008412a0,005d9aa0,005d9da0,0061e360,007fd370,008ee8c0,005d17f0,007d0b90,00891d50,005c3f70,006c9c00,00576dc0,0051acd0,0087d990 callees=0051afe0

undefined4 __thiscall
FUN_0051af60(int param_1,int param_2,int param_3,int param_4,int param_5,int *param_6,uint *param_7)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_0051afe0/*CWorldTimer::CompareWorldTimes*/(param_2,param_3,param_4,param_5);
  if (iVar1 == -1) {
    uVar2 = 0xfffffffe;
  }
  else {
    *param_6 = param_2 - param_4;
    *param_7 = param_3 - param_5;
    if (*(uint *)(param_1 + 0x3c) <= *param_7) {
      *param_6 = *param_6 + -1;
      *param_7 = *param_7 + *(int *)(param_1 + 0x3c);
    }
    uVar2 = 0;
  }
  return uVar2;
}

```

## R298 FUNCTION CResGFF::GetTopLevelStruct  steam=0x007189d0
GOG build: param_bytes=4 ghidra_sig='CResGFF * this, undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* outStruct) param_bytes=4
Steam function at this address: size=37 ret_N=4
```c
// FN 007189d0 FUN_007189d0 size=37 ret=4 callers=00525c60,005236b0,00620120,0055a650,00629d80,0061beb0,006564c0,00638bd0,00629220,00631260,00601610,008529e0,008bf280,0052ccd0,006d0db0,0083db30,00630810,005f9220,0062add0,006c53f0,0040ee40,005582f0,0055d080,00637ce0,005fbf60,006815e0,00572a70,00630f20,00625190,005ef3d0,00855f30 callees=

bool FUN_007189d0(undefined4 *param_1)

{
  if (param_1 != (undefined4 *)0x0) {
    *param_1 = 0;
  }
  return param_1 != (undefined4 *)0x0;
}

```

## R299 FUNCTION CSWGuiPanel::GetFullScreenBG  steam=0x0040fc50
GOG build: param_bytes=4 ghidra_sig='CSWGuiPanel * this, CExoString * param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* outBGString) param_bytes=4
Steam function at this address: size=290 ret_N=4
```c
// FN 0040fc50 FUN_0040fc50 size=290 ret=4 callers= callees=00733df0,007335f0,00733570,007338d0,007337c0,00733780

undefined4 __thiscall FUN_0040fc50(int param_1,undefined4 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00948369;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570/*CExoString::CExoString_char*/("800x600");
  local_8 = 1;
  if (DAT_009f42a8 < 0x4b0) {
    if (DAT_009f42a8 < 0x400) {
      if (DAT_009f42a8 < 0x3c0) {
        if (0x2ff < DAT_009f42a8) {
          FUN_007338d0("1024x768");
        }
      }
      else {
        FUN_007338d0("1280x960");
      }
    }
    else {
      FUN_007338d0("1280x1024");
    }
  }
  else {
    FUN_007338d0("1600x1200");
  }
  uVar2 = FUN_00733df0/*CExoString::operator+*/(local_20,&DAT_00a1b9f8 + (uint)*(byte *)(param_1 + 0x4c) * 8);
  local_8._0_1_ = 2;
  FUN_007337c0/*CExoString::operator=*/(uVar2);
  local_8._0_1_ = 1;
  FUN_00733780/*CExoString::~CExoString*/(uVar1);
  FUN_007335f0/*CExoString::CExoString_copy*/(local_18);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780/*CExoString::~CExoString*/();
  ExceptionList = local_10;
  return param_2;
}

```

## R300 FUNCTION CResGFF::ReadFieldDWORD  steam=0x00718c00
GOG build: param_bytes=16 ghidra_sig='CResGFF * this, int param_1, char * param_2, undefined4 * param_3, int param_4'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, DWORD defaultValue) param_bytes=16
Steam function at this address: size=122 ret_N=16
```c
// FN 00718c00 FUN_00718c00 size=122 ret=16 callers=00625360,005e2900,0062db90,00525f90,006fef80,0053fd50,005753f0,00664cb0,005269e0,0069d1e0,00528080,006c67e0,005266f0,00630810,00619e60,005e9370,006c53f0,00561d80,005e2090,005f7cc0,008384a0,006815e0,0053e580,00527cb0,0058d5e0,00620250,00526f60,00855f30,006c75b0,005285b0,006295d0,00853550,005205f0,00703530,00664800,00527330,005263c0,006c7400,006029f0,00585b00,00833ff0,00836730,00528380,0052ccd0,00573950,006d0db0,005e5510,0062af70,0083db30,00527580,00527890,0061a210,00574180,005582f0,0055d080,006afed0,00601740,0055def0,00592840,0058ca50,005fbf60,005e6f80,00417480,00630f20,00520fc0,00523870,005ef570 callees=00717770,007178e0

int FUN_00718c00(int param_1,int param_2,undefined4 *param_3,int param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 4)) {
      *param_3 = 0;
    }
    else {
      *param_3 = 1;
      param_4 = piVar2[2];
    }
  }
  return param_4;
}

```

## R301 FUNCTION CWorldTimer::GetWorldTime  steam=0x0051ad20
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD* outDays, DWORD* outMS) param_bytes=8
Steam function at this address: size=268 ret_N=8
```c
// FN 0051ad20 FUN_0051ad20 size=268 ret=8 callers=00801ab0,007a6760,0059fd20,0051b050,005d4830,0066c660,00556670,0056c310,00781be0,007efc40,005582f0,00891d10,005d4700,00561f30,00623330,00545520,005d18c0,00624b70,006070a0,0087afe0,0087d6f0,00569320,0051e170,0053af20,006c9900,0051d790,008412a0,005d56b0,00538d00,005d9da0,005a06f0,0051b3e0,005d17f0,005a4d70,00692a00,006d9500,008e54e0,007d0b40,00533830,00891d50,005c3f70,00576dc0,0051acd0,00662060,0087d990,0058be80,0059fcb0,00589ee0,005bfc00,007664b0,0056b8a0,00545470,005d4370,005b96f0,005d4460,0051b0e0,008e59c0,00840f50,007a2590,005d29f0,00557260,00521360,0086da30,005d4610,0062c360,00784680,00617980,0066c160,007f12d0,00544210,0086d770,005a4bb0,0087b240,00600f20,005f4c00,005b8000,00521e60,005d9aa0,0061e360,007fd370,007efac0,0066c430,008ee8c0,0055bd10,0053e830,00801a70,007845b0,007645f0,007f1650,008ee580,007d0b90,0062bfc0,00622c90,006c9c00 callees=0051a920,0091d220,0091fc70

void __thiscall FUN_0051ad20(int param_1,int *param_2,uint *param_3)

{
  int iVar1;
  int iVar2;
  undefined8 uVar3;
  
  uVar3 = FUN_0051a920();
  if (*(int *)(param_1 + 0x24) == 1) {
    *param_2 = *(int *)(param_1 + 0x28);
    *param_3 = *(uint *)(param_1 + 0x2c);
  }
  else {
    uVar3 = __aulldiv(uVar3,1000,0);
    iVar1 = __aullrem(uVar3,*(undefined4 *)(param_1 + 0x3c),0);
    iVar2 = __aulldiv(uVar3,*(undefined4 *)(param_1 + 0x3c),0);
    *param_2 = iVar2 + *(int *)(param_1 + 0x30);
    *param_3 = iVar1 + *(int *)(param_1 + 0x34);
  }
  while (-*(int *)(param_1 + 0x3c) - 1U < *param_3) {
    *param_2 = *param_2 + -1;
    *param_3 = *param_3 + *(int *)(param_1 + 0x3c);
  }
  while (*(uint *)(param_1 + 0x3c) <= *param_3) {
    *param_2 = *param_2 + 1;
    *param_3 = *param_3 - *(int *)(param_1 + 0x3c);
  }
  return;
}

```

## R302 FUNCTION CClientExoApp::GetCharacterChangeInProgress  steam=0x00740fc0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 00740fc0 FUN_00740fc0 size=31 ret=0 callers=0084fcd0,007cb7c0,0084f900 callees=

undefined4 __fastcall FUN_00740fc0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x4ac);
}

```

## R303 FUNCTION Gob::BehaviorMessage  steam=0x004636d0
GOG build: param_bytes=4 ghidra_sig='Gob * this, char * param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, char* message) param_bytes=4
Steam function at this address: size=54 ret_N=4
```c
// FN 004636d0 CBaseTabbedPane::SerializeTabWindow size=54 ret=4 callers=004818d0 callees=

/* Library Function - Single Match
    public: virtual void __thiscall CBaseTabbedPane::SerializeTabWindow(class CArchive &)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CBaseTabbedPane::SerializeTabWindow(CBaseTabbedPane *this,CArchive *param_1)

{
  if (*(int *)(this + 0x1b4) != 0) {
    (**(code **)(**(int **)(this + 0x1b4) + 0xc))(param_1);
  }
  return;
}

```

## R304 FUNCTION CClientExoApp::GetPlayerCreature  steam=0x0073f470
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=28 ret_N=0
```c
// FN 0073f470 FUN_0073f470 size=28 ret=0 callers=005959e0,007abd60,00894320,00804f10,0077f6c0,006e0650,007abb20,007e5730,007eb460,005d56b0,0077fd20,008bfaa0,00765e90,007abcb0,007b3a00,007abec0,008079b0,007f1650,007abc00,007f18c0,0074c320,00813200,007abe10,007e5120,007664b0,00773560,0057f450,00774620 callees=

undefined4 __fastcall FUN_0073f470(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x20);
}

```

## R305 FUNCTION Gob::DisableFog  steam=0x00459f70
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 00459f70 FUN_00459f70 size=31 ret=0 callers= callees=00459f00

void __fastcall FUN_00459f70(int param_1)

{
  FUN_00459f00(*(undefined4 *)(param_1 + 0x98),0);
  return;
}

```

## R306 FUNCTION CRes::Destructor  steam=0x00711170
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=44 ret_N=4
```c
// FN 00711170 FUN_00711170 size=44 ret=4 callers= callees=009196fd,007111a0

undefined4 __thiscall FUN_00711170(undefined4 param_1,uint param_2)

{
  FUN_007111a0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd/*operator delete*/(param_1);
  }
  return param_1;
}

```

## R307 FUNCTION CResGFF::Constructor_2  steam=0x00716fd0
GOG build: param_bytes=12 ghidra_sig='CResGFF * this, undefined2 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, ResourceType resourceType, char* GFFtype, void* templateResRef) param_bytes=12
Steam function at this address: size=465 ret_N=12
```c
// FN 00716fd0 FUN_00716fd0 size=465 ret=12 callers=00525c60,00620120,00629d80,005460f0,0061beb0,006564c0,00638bd0,00629220,00631260,00601610,008529e0,0052ccd0,00545760,006d0db0,005f9220,0062add0,0040ee40,005582f0,00637ce0,005fbf60,006815e0,00572a70,00625190,005ef3d0,00855f30 callees=007111e0,00713010,0091d0f0,0091c630,007110d0,00712b60

undefined4 * __thiscall
FUN_00716fd0(undefined4 *param_1,undefined2 param_2,char *param_3,undefined4 param_4)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ff08;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_007110d0/*CRes::CRes*/(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  *param_1 = CResGFF::vftable;
  param_1[0x10] = 0;
  param_1[0x11] = 0;
  param_1[0x12] = 0;
  param_1[0x13] = 0;
  param_1[0x14] = 0;
  param_1[0x15] = 0;
  param_1[0x16] = 0;
  param_1[0x17] = 0;
  param_1[0x18] = 0;
  param_1[0x19] = 0;
  param_1[0x1a] = 0;
  param_1[0x1b] = 0;
  param_1[0x1c] = 0;
  param_1[0x1d] = 0;
  param_1[0x1e] = 0;
  param_1[10] = 0x6e;
  param_1[0xb] = 0x286;
  param_1[0xc] = 0x62;
  param_1[0xd] = 0x72c;
  param_1[0xe] = 0xfd4;
  param_1[0xf] = 0xfd4;
  *(undefined1 *)(param_1 + 0x23) = 0;
  param_1[0x25] = 0;
  param_1[0x26] = 0;
  param_1[0x27] = 0;
  _memset((void *)((int)param_1 + 0x8d),0,4);
  _strncpy((char *)((int)param_1 + 0x8d),param_3,4);
  FUN_00712b60(param_4,param_2,param_1);
  FUN_007111e0/*CRes::Demand*/();
  if (param_1[4] == 0) {
    FUN_00713010/*CExoResMan::ReleaseResObject*/(param_1);
  }
  else {
    param_1[0x27] = 1;
  }
  ExceptionList = local_10;
  return param_1;
}

```

## R308 FUNCTION Scene::GetFogRange  steam=0x00464c50
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, float* start, float* end) param_bytes=8
Steam function at this address: size=35 ret_N=8
```c
// FN 00464c50 FUN_00464c50 size=35 ret=8 callers= callees=

void __thiscall FUN_00464c50(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  *param_2 = *(undefined4 *)(param_1 + 100);
  *param_3 = *(undefined4 *)(param_1 + 0x68);
  return;
}

```

## R309 FUNCTION CResGFF::GetElementType  steam=0x00718690
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr) param_bytes=4
Steam function at this address: size=46 ret_N=4
```c
// FN 00718690 FUN_00718690 size=46 ret=4 callers=005266f0,00527580,00525f90,00527330,00527890,00621900,005263c0,0055def0,005269e0,00528080,00527cb0,00526f60,0052ccd0,00573950 callees=00717720

undefined4 FUN_00718690(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  puVar1 = (undefined4 *)FUN_00717720(param_1);
  if (puVar1 == (undefined4 *)0x0) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = *puVar1;
  }
  return uVar2;
}

```

## R310 FUNCTION CRes::CancelRequest  steam=0x007111c0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=26 ret_N=0
```c
// FN 007111c0 FUN_007111c0 size=26 ret=0 callers=0052e590,0070a8d0,00945020,00710710,0093fc20,009408e0,007038a0,0093fb70,00940990,0093f480,0093d7c0,0052e640,007208c0,00561240,0071d2b0,006fe360,00940ce0,0071d360,009450d0,00941190,00940550,00940d90,00943c20,00720970,0093f530,00941710,00943cd0,0093d710,0085f5f0,0093d2b0,00561190,0085f540,00703950,0093d200,00941660,009404a0,009410e0 callees=00711a30

void __fastcall FUN_007111c0(undefined4 param_1)

{
  FUN_00711a30(param_1);
  return;
}

```

## R311 FUNCTION Scene::SetObjectScale  steam=0x004725b0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* object, float scale) param_bytes=8
Steam function at this address: size=46 ret_N=8
```c
// FN 004725b0 FUN_004725b0 size=46 ret=8 callers= callees=

void __thiscall FUN_004725b0(undefined4 param_1,int *param_2,undefined4 param_3)

{
  (**(code **)(*param_2 + 0xcc))(param_3,1,param_1);
  return;
}

```

## R312 FUNCTION CResGFF::WriteGFFFile  steam=0x0071a5a0
GOG build: param_bytes=6 ghidra_sig='undefined4 param_1, undefined2 param_2'
Wrapper typedef (what calling code expects): bool (__thiscall)(void* thisPtr, CExoString* name, ResourceType type) param_bytes=8
Steam function at this address: size=198 ret_N=8
```c
// FN 0071a5a0 FUN_0071a5a0 size=198 ret=8 callers=005fa3b0,005fb1a0,0053af20,005f8710,00534600,005f7f30,00632220,005f8130,005f8320,00655010,005fe4b0,00638bd0,0076aed0,0055cc70,005302e0,00631560,005f8520,0055d7f0,005f9ff0 callees=00733570,0071a670,0071a8b0,00733780,00739880,007398e0,007397f0

undefined4 FUN_0071a5a0(undefined4 param_1,undefined2 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  undefined1 local_20 [8];
  undefined1 local_18 [4];
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095fff0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570/*CExoString::CExoString_char*/(&DAT_0098ba1c);
  local_8 = 0;
  uVar3 = extraout_var;
  FUN_007397f0(param_1,param_2,local_20);
  local_8 = CONCAT31(local_8._1_3_,2);
  FUN_00733780/*CExoString::~CExoString*/(uVar1,uVar3);
  iVar2 = FUN_007398e0();
  if (iVar2 == 0) {
    local_8 = 0xffffffff;
    FUN_00739880();
    uVar3 = 0;
  }
  else {
    FUN_0071a8b0/*CResGFF::Pack*/(0,0);
    uVar3 = FUN_0071a670/*CResGFF::WriteGFFData*/(local_14,local_18);
    local_8 = 0xffffffff;
    FUN_00739880();
  }
  ExceptionList = local_10;
  return uVar3;
}

```

## R313 FUNCTION CSWSDoor::GetLinkedObject  steam=0x0061d620
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=179 ret_N=0
```c
// FN 0061d620 FUN_0061d620 size=179 ret=0 callers=00619d00,0068a950,00557ff0,00597d90,00617b40,0054cd50 callees=0061d000,0055eb90,0051bd90,0051c1a0,0051c0a0

int FUN_0061d620(void)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  int extraout_var;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = FUN_0061d000/*CSWSDoor::GetIsLinked*/();
  if (iVar1 != 0) {
    uVar6 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    iVar1 = extraout_var;
    local_8 = FUN_0051c1a0(*(undefined4 *)(extraout_var + 1000));
    if (local_8 == 0) {
      uVar5 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      uVar4 = 0;
      iVar2 = iVar1 + 0x3d8;
      FUN_0051bd90(iVar2,0);
      uVar4 = FUN_0055eb90(iVar2,uVar4);
      *(undefined4 *)(iVar1 + 1000) = uVar4;
      uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      piVar3 = (int *)FUN_0051c0a0/*CServerExoApp::GetGameObject*/(*(undefined4 *)(iVar1 + 1000));
      if (piVar3 != (int *)0x0) {
        local_8 = (**(code **)(*piVar3 + 0x10))(iVar1,uVar4,uVar5,uVar6);
      }
    }
  }
  return local_8;
}

```

## R314 FUNCTION CSWSObject::StopDialog  steam=0x00546900
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=329 ret_N=0
```c
// FN 00546900 FUN_00546900 size=329 ret=0 callers=005467b0,00546b00,006ce710,00563da0 callees=006cf740,0051c080,0057d370,0053dfb0,00546b00

undefined4 __fastcall FUN_00546900(int *param_1)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined1 *puVar5;
  undefined1 local_8 [4];
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    uVar4 = 0;
    uVar3 = DAT_00994484;
    (**(code **)(*param_1 + 0x30))(DAT_00994484,0);
    FUN_0057d370(uVar3,uVar4);
  }
  if (param_1[0x15] == 0x7f000000) {
    uVar3 = 0;
  }
  else if (param_1[0x15] == param_1[1]) {
    if (param_1[0xd] == 0) {
      uVar3 = 0;
    }
    else {
      FUN_006cf740(param_1,0);
      param_1[0xe] = 0;
      param_1[0xf] = 0;
      param_1[0x12] = 0;
      param_1[0x14] = 0;
      if ((undefined4 *)param_1[0xd] != (undefined4 *)0x0) {
        (*(code *)**(undefined4 **)param_1[0xd])(1);
      }
      param_1[0xd] = 0;
      uVar3 = 1;
    }
  }
  else {
    puVar5 = local_8;
    iVar2 = param_1[0x15];
    FUN_0051c080(iVar2,puVar5);
    cVar1 = FUN_0053dfb0/*CGameObjectArray::GetGameObject*/(iVar2,puVar5);
    if (cVar1 == DAT_0099447f) {
      FUN_00546b00(param_1[1]);
      param_1[0x15] = 0x7f000000;
    }
    uVar3 = 1;
  }
  return uVar3;
}

```

## R315 FUNCTION CSWSObject::SetDialogDelay  steam=0x00545470
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, float delay) param_bytes=4
Steam function at this address: size=162 ret_N=4
```c
// FN 00545470 FUN_00545470 size=162 ret=4 callers=006cc140,006c9770,006ce710 callees=0051c370,0051ad20,0051aef0

void __thiscall FUN_00545470(int param_1,float param_2)

{
  int iVar1;
  int iVar2;
  ushort in_FPUControlWord;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  
  iVar2 = param_1 + 0x44;
  iVar1 = param_1 + 0x40;
  FUN_0051c370/*CServerExoApp::GetWorldTimer*/(iVar1,iVar2);
  FUN_0051ad20/*CWorldTimer::GetWorldTime*/(iVar1,iVar2);
  iVar1 = param_1 + 0x3c;
  iVar2 = param_1 + 0x38;
  uVar6 = (undefined4)(longlong)ROUND((float)DAT_00992384 * param_2);
  uVar5 = 0;
  uVar4 = *(undefined4 *)(param_1 + 0x44);
  uVar3 = *(undefined4 *)(param_1 + 0x40);
  FUN_0051c370/*CServerExoApp::GetWorldTimer*/(uVar3,uVar4,0,uVar6,iVar2,iVar1,(longlong)ROUND((float)DAT_00992384 * param_2),
               in_FPUControlWord | 0xc00);
  FUN_0051aef0/*CWorldTimer::AddWorldTimes*/(uVar3,uVar4,uVar5,uVar6,iVar2,iVar1);
  return;
}

```

## R316 FUNCTION CResGFF::AddStructToStruct  steam=0x00719940
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* parentStruct, CResStruct* childStruct, char* labelText, DWORD structId) param_bytes=16
Steam function at this address: size=135 ret_N=16
```c
// FN 00719940 FUN_00719940 size=135 ret=16 callers=0052b270,005fb1a0,0053c680,0052b0d0,005e9510,00703370,0053af20,00540410,005751b0,00520190,00574500,005e72d0,006692e0,0069cdb0 callees=00717aa0,00717f30,00718030

undefined4 __thiscall
FUN_00719940(int param_1,int *param_2,int param_3,int param_4,undefined4 param_5)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  
  if (((param_2 == (int *)0x0) || (param_3 == 0)) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_3,param_4,0xe);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      iVar3 = FUN_00717f30/*CResGFF::AddStruct*/(param_5);
      if (iVar3 == -1) {
        uVar1 = 0;
      }
      else {
        *(int *)(iVar2 + 8) = iVar3;
        *param_2 = iVar3;
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}

```

## R317 FUNCTION CSWCObject::AnimationRunning  steam=0x007ef470
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=129 ret_N=4
```c
// FN 007ef470 FUN_007ef470 size=129 ret=4 callers=00769780 callees=0071b8b0

bool __thiscall FUN_007ef470(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe34,&local_8);
  return iVar1 != 0 && local_8 != 0;
}

```

## R318 OFFSET Scene.fog_start = 0x64
Claim: DoFog 0x46a980 glFogf(GL_FOG_START, *(this+0x64)); SetFogRange 0x464c80/GetFogRange 0x464c50 use +0x64
Decompile 0x00464c50:
```c
// FN 00464c50 FUN_00464c50/*Scene::GetFogRange*/ size=35 ret=8 callers= callees=

void __thiscall FUN_00464c50/*Scene::GetFogRange*/(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  *param_2 = *(undefined4 *)(param_1 + 100);
  *param_3 = *(undefined4 *)(param_1 + 0x68);
  return;
}

```
Decompile 0x00464c80:
```c
// FN 00464c80 FUN_00464c80/*Scene::SetFogRange*/ size=31 ret=8 callers= callees=

void __thiscall FUN_00464c80/*Scene::SetFogRange*/(int param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 100) = param_2;
  *(undefined4 *)(param_1 + 0x68) = param_3;
  return;
}

```
Decompile 0x0046a980:
```c
// FN 0046a980 FUN_0046a980 size=671 ret=0 callers= callees=00000059,0000001c,0000000e,0000005a,00000060

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0046a980(int param_1)

{
  int local_3c;
  int local_2c;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (DAT_00a20064 == 0) {
    if (*(int *)(param_1 + 0x60) == 0) {
      glDisable(DAT_009f4930);
    }
    else {
      glEnable(DAT_009f4930);
      if (*(int *)(param_1 + 0x6c) == 0) {
        local_2c = 0;
      }
      else {
        local_2c = (*(int *)(param_1 + 0x6c) != 1) + 1;
      }
      glFogi(0xb65,(&DAT_0098851c)[local_2c]);
      if (DAT_00a7be44 == 0) {
        if (DAT_00a7bac4 == '\0') {
          glFogfv(DAT_009f4970,param_1 + 0x74);
        }
        else {
          glFogfv(DAT_009f4970,param_1 + 0x80);
        }
      }
      else {
        local_10 = 0;
        local_c = 0;
        local_8 = 0;
        glFogfv(DAT_009f4970,&local_10);
      }
      glFogf(DAT_009f4964,*(undefined4 *)(param_1 + 100));
      glFogf(DAT_009f4968,*(undefined4 *)(param_1 + 0x68));
      glFogf(DAT_009f496c,*(undefined4 *)(param_1 + 0x70));
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x60) = 1;
... (25 more lines)
```

## R319 FUNCTION CSWCObject::ClearAllActions  steam=0x007f24c0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=54 ret_N=0
```c
// FN 007f24c0 FUN_007f24c0 size=54 ret=0 callers=007523e0,0066ad10,0087bd10,007f23f0,006cf5d0,0087bad0,007ac470,005d75b0,007b12c0,0087bb20,0087bca0 callees=0051c8b0,0051cb10

void __fastcall FUN_007f24c0(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0/*CServerExoApp::ClientToServerObjectId*/(*(undefined4 *)(param_1 + 4));
  FUN_0051cb10(uVar1);
  return;
}

```

## R320 FUNCTION CSWSObject::GetHasFeatEffectApplied  steam=0x00549ed0
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD feat) param_bytes=4
Steam function at this address: size=151 ret_N=4
```c
// FN 00549ed0 FUN_00549ed0 size=151 ret=4 callers=0067e2d0 callees=006a20f0

undefined4 __thiscall FUN_00549ed0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_10;
  
  iVar1 = FUN_006a20f0(param_2);
  if ((iVar1 != 0) && ((*(uint *)(iVar1 + 4) & 0xffff) != 0xffffffff)) {
    for (local_10 = 0; local_10 < *(int *)(param_1 + 0x14c); local_10 = local_10 + 1) {
      if (*(uint *)(*(int *)(*(int *)(param_1 + 0x148) + local_10 * 4) + 0x1c) ==
          (*(uint *)(iVar1 + 4) & 0xffff)) {
        return 1;
      }
    }
  }
  return 0;
}

```

## R321 FUNCTION CResGFF::GetTotalSize  steam=0x0071acb0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=135 ret_N=0
```c
// FN 0071acb0 FUN_0071acb0 size=135 ret=0 callers=0071a8b0,00721bb0 callees=

int __fastcall FUN_0071acb0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x40) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(*(int *)(param_1 + 0x40) + 0x1c) * 0x10 +
            *(int *)(*(int *)(param_1 + 0x40) + 0x14) * 0xc +
            *(int *)(*(int *)(param_1 + 0x40) + 0xc) * 0xc + 0x38 +
            *(int *)(*(int *)(param_1 + 0x40) + 0x24) + *(int *)(*(int *)(param_1 + 0x40) + 0x2c) +
            *(int *)(*(int *)(param_1 + 0x40) + 0x34);
  }
  return iVar1;
}

```

## R322 FUNCTION CClientExoApp::SetPlayerCreature  steam=0x0073f910
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD creatureId) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 0073f910 FUN_0073f910 size=28 ret=4 callers=008074e0,00804f10,00670760 callees=00798660

void FUN_0073f910(undefined4 param_1)

{
  FUN_00798660(param_1);
  return;
}

```

## R323 FUNCTION CResGFF::ReleaseResource  steam=0x00717630
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=234 ret_N=0
```c
// FN 00717630 FUN_00717630 size=234 ret=0 callers=00717aa0,007171b0,0071a3d0 callees=007112b0,00713010

void __fastcall FUN_00717630(int param_1)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    if (*(int *)(param_1 + 0x9c) != 0) {
      FUN_007112b0/*CRes::Release*/();
      FUN_00713010/*CExoResMan::ReleaseResObject*/(param_1);
      *(undefined4 *)(param_1 + 0x9c) = 0;
    }
    *(undefined4 *)(param_1 + 0x98) = 0;
  }
  *(undefined4 *)(param_1 + 0x40) = 0;
  *(undefined4 *)(param_1 + 0x44) = 0;
  *(undefined4 *)(param_1 + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 0;
  *(undefined4 *)(param_1 + 0x50) = 0;
  *(undefined4 *)(param_1 + 0x54) = 0;
  *(undefined4 *)(param_1 + 0x58) = 0;
  *(undefined4 *)(param_1 + 0x5c) = 0;
  *(undefined4 *)(param_1 + 0x60) = 0;
  *(undefined4 *)(param_1 + 100) = 0;
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined4 *)(param_1 + 0x6c) = 0;
  *(undefined4 *)(param_1 + 0x70) = 0;
  *(undefined4 *)(param_1 + 0x74) = 0;
  *(undefined4 *)(param_1 + 0x78) = 0;
  return;
}

```
