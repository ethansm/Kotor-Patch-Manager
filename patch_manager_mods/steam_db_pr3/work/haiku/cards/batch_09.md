## R243 FUNCTION CClientExoApp::ShutDownToMainMenu  steam=0x0073fb40
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 0073fb40 FUN_0073fb40 size=22 ret=0 callers= callees=0078e0f0

void __fastcall FUN_0073fb40(undefined4 param_1)

{
  FUN_0078e0f0(param_1);
  return;
}

```

## R244 FUNCTION CSWGuiControl::AddEvent  steam=0x00418af0
GOG build: param_bytes=12 ghidra_sig='CSWGuiControl * this, undefined4 param_1, CSWGuiObject * param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int eventFlag, void* guiObject, void* menuFunc) param_bytes=12
Steam function at this address: size=177 ret_N=12
```c
// FN 00418af0 FUN_00418af0 size=177 ret=12 callers=0075b370,008c7aa0,00880740,008d8c50,00893950,008a1170,00892090,00918b10,008c17f0,007d3750,0088aac0,009074e0,007d6170,00747210,008b4270,0084c3a0,008d1020,00882c00,0089c5f0,007d8540,0075ae40,008bba80,007d23e0,00895cc0,008b9fd0,00760210,008c7510,007d9800,008c80f0,008b7da0,008c9e10,00850770,007d4620,007dc410,007fc880,008fb7a0,00916ef0,007dacb0,008b7ed0,00886fc0,008a92d0,009153d0,00757c40,008e1810,008df7f0,008586d0,008db630,008f92b0,008fd950,008b01b0,00910490,008b91f0,008fc8e0,008bfe30,008b1ea0,007450e0,008973d0,00916640,008a75f0,008557c0,008b2f70,008b5db0,00902f60,008519f0,008d3900,008fff20,008c9030,0089cf30,008df930,007fae60,008c9c40,00754ed0,008c6650,008bc620,00901be0,00914000,008fbef0,008a25c0,008d6b90,007d31d0,0090c6a0,0075cec0,008ce930,00741940,008a6170,00887da0,00909e00 callees=007a1940,008a5fb0

void __thiscall FUN_00418af0(int param_1,int param_2,undefined4 param_3,int param_4)

{
  undefined4 *puVar1;
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x40) <= local_c) {
      if (param_4 != 0) {
        FUN_008a5fb0(param_3,param_4,param_2);
      }
      return;
    }
    puVar1 = (undefined4 *)(local_c * 0xc + *(int *)(param_1 + 0x3c));
    if (puVar1[2] == param_2) break;
    local_c = local_c + 1;
  }
  if (param_4 != 0) {
    *puVar1 = param_3;
    puVar1[1] = param_4;
    return;
  }
  FUN_007a1940(local_c);
  return;
}

```

## R245 FUNCTION CSWCObject::ClearAllActions  steam=0x007f2500
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=54 ret_N=0
```c
// FN 007f2500 FUN_007f2500 size=54 ret=0 callers=00751750,007445d0 callees=0051cba0,0051c8b0

void __fastcall FUN_007f2500(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0/*CServerExoApp::ClientToServerObjectId*/(*(undefined4 *)(param_1 + 4));
  FUN_0051cba0(uVar1);
  return;
}

```

## R246 FUNCTION CGuiInGame::HideUpgradeScreen  steam=0x007cdaa0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=311 ret_N=4
```c
// FN 007cdaa0 FUN_007cdaa0 size=311 ret=4 callers=00699df0 callees=00410530,0073f810,0087a730,0073fee0,00919723,0070bc60,008d6b90

void __thiscall FUN_007cdaa0(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_2c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952aeb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x8c) == 0)) {
    FUN_0070bc60(DAT_0099c2b8,0);
    if (*(int *)(param_1 + 0xf18) == 0) {
      FUN_0073f810(uVar1);
      FUN_0087a730();
    }
    iVar2 = FUN_00919723/*operator new*/(0x3410);
    local_8 = 0;
    if (iVar2 == 0) {
      local_2c = 0;
    }
    else {
      local_2c = FUN_008d6b90(*(undefined4 *)(param_1 + 0x38),param_2);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x8c) = local_2c;
    FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0x8c),3,1);
    FUN_0073fee0/*CClientExoApp::SetInputClass*/(2,1);
  }
  ExceptionList = local_10;
  return;
}

```

## R247 FUNCTION CSWGuiPanel::GetControl  steam=0x0040fea0
GOG build: param_bytes=4 ghidra_sig='CSWGuiPanel * this, int param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, int index) param_bytes=4
Steam function at this address: size=48 ret_N=4
```c
// FN 0040fea0 FUN_0040fea0 size=48 ret=4 callers=00418f20,00412400 callees=

undefined4 __thiscall FUN_0040fea0(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if (param_2 < *(int *)(param_1 + 0x28)) {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x24) + param_2 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

```

## R248 FUNCTION CSWCObject::GetCurrentAnimation  steam=0x007ed7d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): WORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 007ed7d0 FUN_007ed7d0 size=18 ret=0 callers=007f5b20,00772020,007b40f0,00770310,007ac470,0077fb10,007c93e0,007637e0,00769620,007c9560,007631d0,0087bec0,007f61b0,007696e0,00769780,007c84e0,0076d200,007f2380,008edb50,008e59c0,00798d70,008079b0 callees=

undefined2 __fastcall FUN_007ed7d0(int param_1)

{
  return *(undefined2 *)(param_1 + 0x48);
}

```

## R249 OFFSET CResGFF.structs = 0x44
Claim: GetStruct 0x717720 returns id*0xC + [this+0x44] (12-byte GFF struct entries)
Decompile 0x00717720:
```c
// FN 00717720 FUN_00717720 size=71 ret=4 callers=00718690,00718030,00717770,007186e0,007178e0 callees=

int __thiscall FUN_00717720(int param_1,uint *param_2)

{
  int iVar1;
  
  if ((((param_2 == (uint *)0x0) || (*(int *)(param_1 + 0x40) == 0)) ||
      (*(int *)(param_1 + 0x44) == 0)) || (*(uint *)(*(int *)(param_1 + 0x40) + 0xc) <= *param_2)) {
    iVar1 = 0;
  }
  else {
    iVar1 = *param_2 * 0xc + *(int *)(param_1 + 0x44);
  }
  return iVar1;
}

```

## R250 FUNCTION Gob::SetPosition  steam=0x00461490
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector* outPosition, Vector position) param_bytes=8
Steam function at this address: size=69 ret_N=16
```c
// FN 00461490 FUN_00461490 size=69 ret=16 callers= callees=

undefined4 * __thiscall
FUN_00461490(int param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  *(undefined4 *)(param_1 + 0xa4) = param_3;
  *(undefined4 *)(param_1 + 0xa8) = param_4;
  *(undefined4 *)(param_1 + 0xac) = param_5;
  *param_2 = *(undefined4 *)(param_1 + 0xa4);
  param_2[1] = *(undefined4 *)(param_1 + 0xa8);
  param_2[2] = *(undefined4 *)(param_1 + 0xac);
  return param_2;
}

```

## R251 FUNCTION CResGFF::WriteFieldWORD  steam=0x00719ab0
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined2 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, WORD value, char* label) param_bytes=12
Steam function at this address: size=97 ret_N=12
```c
// FN 00719ab0 FUN_00719ab0 size=97 ret=12 callers=0058c960,00602dd0,0053c680,005e5b40,005e1e60,0061bfe0,00540410,0055bd10,00664d80,005f18c0,00592310,00574500,00621ba0,00603390,0076aed0,006b4ea0,0058d2f0,006b3d10,005e1990 callees=00717aa0,00718030

undefined4 __thiscall FUN_00719ab0(int param_1,int param_2,ushort param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,2);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(uint *)(iVar2 + 8) = (uint)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}

```

## R252 FUNCTION Gob::EnableFog  steam=0x00459f90
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 00459f90 FUN_00459f90 size=31 ret=0 callers= callees=00459f00

void __fastcall FUN_00459f90(int param_1)

{
  FUN_00459f00(*(undefined4 *)(param_1 + 0x98),1);
  return;
}

```

## R253 OFFSET CSWGuiBorderParams.bit_flags = 0x20
Claim: Load stores FILLSTYLE&3 at border+0x34 (params+0x20); SetPulsingAlpha 0x4149f0 sets bits 2..4 of the same dword; Draw tests bits 2..4
Decompile 0x004149f0:
```c
// FN 004149f0 FUN_004149f0 size=91 ret=12 callers=008db630,008f92b0,008b38e0,007450e0,00893950,00916640,008a1170,00902af0,00902f60,00756930,0089cf30,008b0fb0,008d0470,007fae60,007d23e0,008fde00,008c9e10,00850770,008fb7a0,00914000,0074fa50,00885fa0,00415080,008a25c0,00745b30,008b1d00,008a92d0,009153d0,00757c40,0089ce20,008a5ba0 callees=

void __thiscall FUN_004149f0(int param_1,uint param_2,uint param_3,uint param_4)

{
  *(uint *)(param_1 + 0x20) = *(uint *)(param_1 + 0x20) & 0xfffffffb | (param_2 & 1) << 2;
  *(uint *)(param_1 + 0x20) = *(uint *)(param_1 + 0x20) & 0xfffffff7 | (param_3 & 1) << 3;
  *(uint *)(param_1 + 0x20) = *(uint *)(param_1 + 0x20) & 0xffffffef | (param_4 & 1) << 4;
  return;
}

```

## R254 FUNCTION Scene::IsAnimating  steam=0x004651f0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): bool (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=20 ret_N=0
```c
// FN 004651f0 FUN_004651f0 size=20 ret=0 callers= callees=

undefined1 __fastcall FUN_004651f0(int param_1)

{
  return *(undefined1 *)(param_1 + 0xe6);
}

```

## R255 FUNCTION CClientExoApp::DismissInGameGUI  steam=0x00740310
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 00740310 FUN_00740310 size=22 ret=0 callers=008953b0 callees=007b33b0

void __fastcall FUN_00740310(undefined4 param_1)

{
  FUN_007b33b0(param_1);
  return;
}

```

## R256 FUNCTION CClientExoApp::EnableVideoEffect  steam=0x007409a0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int effect) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 007409a0 FUN_007409a0 size=28 ret=4 callers=007e26c0,007e6ce0,007c6200,007e24d0,007c41f0,007ce3e0,006986e0 callees=0079e550

void FUN_007409a0(undefined4 param_1)

{
  FUN_0079e550(param_1);
  return;
}

```

## R257 FUNCTION CGuiInGame::HideItemCreateMenu  steam=0x007caa60
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=149 ret_N=0
```c
// FN 007caa60 FUN_007caa60 size=149 ret=0 callers=008c1fc0,008c21a0,008c2140 callees=00410670,0073fee0,00410800,007400a0,004109c0

void __fastcall FUN_007caa60(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x58));
    if (iVar1 != 0) {
      FUN_004109c0/*CSWGuiManager::PopModalPanel*/();
      FUN_00410670(*(undefined4 *)(param_1 + 0x58));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
      }
      else {
        FUN_0073fee0/*CClientExoApp::SetInputClass*/(1,1);
      }
    }
  }
  return;
}

```

## R258 FUNCTION CGuiInGame::NextSWInGameGui  steam=0x007ca230
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=389 ret_N=0
```c
// FN 007ca230 FUN_007ca230 size=389 ret=0 callers=007568f0 callees=00733570,0073fea0,00410670,00410530,007d0760,00410780,006fd8d0,00733780,00756860

void __fastcall FUN_007ca230(int param_1)

{
  undefined4 local_2c;
  undefined1 local_20 [8];
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096fe28;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x128) != 0) {
    local_18 = FUN_0073fea0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (((DAT_00a10760 != 0) && (*(int *)(param_1 + 0x30) != 0)) && (local_18 != 0)) {
      FUN_00410670(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
    }
    local_14 = *(undefined4 *)(param_1 + 0x2c);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + 1;
    if (7 < *(int *)(param_1 + 0x2c)) {
      *(undefined4 *)(param_1 + 0x2c) = 0;
    }
    if ((*(int *)(param_1 + 0x30) == 0) || (local_18 == 0)) {
      local_2c = 0xffffffff;
    }
    else {
      local_2c = *(undefined4 *)(param_1 + 0x2c);
    }
    FUN_007d0760(local_14,local_2c);
    if ((*(int *)(param_1 + 0x30) != 0) && (local_18 != 0)) {
      FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4),2,1);
      FUN_00410780(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
      FUN_00756860(*(undefined4 *)(param_1 + 0x2c),1);
      if (DAT_00a1b4a8 != 0) {
        FUN_00733570/*CExoString::CExoString_char*/("k_pend_screenchg");
        local_8 = 0;
        FUN_006fd8d0(local_20,0x7f000000,0);
        local_8 = 0xffffffff;
        FUN_00733780/*CExoString::~CExoString*/();
      }
    }
  }
  ExceptionList = local_10;
  return;
}

```

## R259 FUNCTION CSWCObject::AnimationParry  steam=0x007ef7f0
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=129 ret_N=4
```c
// FN 007ef7f0 FUN_007ef7f0 size=129 ret=4 callers=007631d0,00772020 callees=0071b8b0

bool __thiscall FUN_007ef7f0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe3c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}

```

## R260 FUNCTION Gob::IsAlwaysRendering  steam=0x00459ee0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): unsigned char (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=17 ret_N=0
```c
// FN 00459ee0 FUN_00459ee0 size=17 ret=0 callers= callees=

undefined1 __fastcall FUN_00459ee0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x12);
}

```

## R261 OFFSET CResGFF.fields = 0x4c
Claim: GetField 0x717770 returns idx*0xC + [this+0x4c] (12-byte GFF field entries)
Decompile 0x00717770:
```c
// FN 00717770 FUN_00717770 size=171 ret=8 callers=00719800,00718b80,007190e0,00718810,00718a00,00718c00,00718a80,00718710,00718890,00718d80,00718e40,00718b00,00718d00,007193d0,00718960,00718f90,00719640,00718760,00719520,00718c80 callees=00717720

int __thiscall FUN_00717770(int param_1,undefined4 param_2,uint param_3)

{
  int iVar1;
  
  iVar1 = FUN_00717720(param_2);
  if (((iVar1 == 0) || (*(int *)(param_1 + 0x4c) == 0)) || (*(uint *)(iVar1 + 8) <= param_3)) {
    iVar1 = 0;
  }
  else if (*(int *)(iVar1 + 8) == 1) {
    if (*(uint *)(iVar1 + 4) < *(uint *)(*(int *)(param_1 + 0x40) + 0x14)) {
      iVar1 = *(int *)(iVar1 + 4) * 0xc + *(int *)(param_1 + 0x4c);
    }
    else {
      iVar1 = 0;
    }
  }
  else if ((*(int *)(param_1 + 100) == 0) ||
          (*(uint *)(*(int *)(param_1 + 0x40) + 0x2c) < *(int *)(iVar1 + 4) + 4 + param_3 * 4)) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(*(int *)(param_1 + 100) + *(int *)(iVar1 + 4) + param_3 * 4) * 0xc +
            *(int *)(param_1 + 0x4c);
  }
  return iVar1;
}

```

## R262 FUNCTION Gob::GetOrientation  steam=0x00459740
GOG build: param_bytes=4 ghidra_sig='Gob * this, undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Quaternion* orientation) param_bytes=4
Steam function at this address: size=49 ret_N=4
```c
// FN 00459740 CRichEditView::GetMargins size=49 ret=4 callers= callees=

/* Library Function - Single Match
    public: class CRect __thiscall CRichEditView::GetMargins(void)const 
   
   Library: Visual Studio 2010 Debug */

undefined4 * __thiscall CRichEditView::GetMargins(CRichEditView *this)

{
  undefined4 *in_stack_00000004;
  
  *in_stack_00000004 = *(undefined4 *)(this + 0xb0);
  in_stack_00000004[1] = *(undefined4 *)(this + 0xb4);
  in_stack_00000004[2] = *(undefined4 *)(this + 0xb8);
  in_stack_00000004[3] = *(undefined4 *)(this + 0xbc);
  return in_stack_00000004;
}

```

## R263 FUNCTION Gob::SetMaxBlurLength  steam=0x004599b0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, float length) param_bytes=4
Steam function at this address: size=25 ret_N=4
```c
// FN 004599b0 FUN_004599b0 size=25 ret=4 callers= callees=

void __thiscall FUN_004599b0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x1ac) = param_2;
  return;
}

```

## R264 OFFSET CSWGuiBorderParams.alpha = 0x10
Claim: params ctor 0x4143d0 param_7 -> params[4] (+0x10); Border::Draw reads border+0x24 as alpha; operator= copies [4]
Decompile 0x004143d0:
```c
// FN 004143d0 FUN_004143d0 size=286 ret=36 callers=008bb5d0,008b0fb0,008d0470,00895cc0,00411b20,00895620,008b38e0,00414a50,008a5ba0 callees=00710ca0,007107f0

undefined4 * __thiscall
FUN_004143d0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            uint param_5,undefined4 param_6,undefined4 param_7,undefined4 *param_8,
            undefined4 param_9,undefined4 param_10)

{
  undefined1 local_34 [16];
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = 0;
  FUN_007107f0();
  FUN_007107f0();
  FUN_007107f0();
  FUN_00710ca0(local_14,param_2);
  FUN_00710ca0(local_24,param_3);
  FUN_00710ca0(local_34,param_4);
  param_1[8] = param_1[8] & 0xfffffffc | param_5 & 3;
  *param_1 = param_6;
  param_1[4] = param_7;
  param_1[8] = param_1[8] & 0xfffffffb;
  param_1[5] = *param_8;
  param_1[6] = param_8[1];
  param_1[7] = param_8[2];
  param_1[8] = param_1[8] & 0xfffffff7;
  param_1[8] = param_1[8] & 0xffffffef;
  param_1[0x15] = 0;
  param_1[3] = 0;
  param_1[1] = param_9;
  param_1[2] = param_10;
  return param_1;
}

```

## R265 FUNCTION CGuiInGame::HideContainerGui  steam=0x007cad30
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=126 ret_N=0
```c
// FN 007cad30 FUN_007cad30 size=126 ret=0 callers=00545760,008b2850 callees=00410670,00740350,0073fee0,00410800,004109c0

void __fastcall FUN_007cad30(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x54));
    if (iVar1 != 0) {
      FUN_004109c0/*CSWGuiManager::PopModalPanel*/();
      FUN_00410670(*(undefined4 *)(param_1 + 0x54));
      FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
      FUN_00740350(0,0,0);
    }
  }
  return;
}

```

## R266 FUNCTION CGuiInGame::ShowItemCreateMenu  steam=0x007caa10
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=71 ret_N=0
```c
// FN 007caa10 FUN_007caa10 size=71 ret=0 callers=008bfa40 callees=00410530,0073fee0

void __fastcall FUN_007caa10(int param_1)

{
  if (*(int *)(param_1 + 0x128) != 0) {
    FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0x58),1,1);
    FUN_0073fee0/*CClientExoApp::SetInputClass*/(2,1);
  }
  return;
}

```

## R267 FUNCTION CGuiInGame::RePopulateMainInterface  steam=0x007ceb50
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=39 ret_N=0
```c
// FN 007ceb50 FUN_007ceb50 size=39 ret=0 callers=00781be0,0079d6e0 callees=0074cfe0

void __fastcall FUN_007ceb50(int param_1)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_0074cfe0(1);
  }
  return;
}

```

## R268 FUNCTION CSWSPlaceable::GetClientPlaceable  steam=0x005f6a70
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=53 ret_N=0
```c
// FN 005f6a70 FUN_005f6a70 size=53 ret=0 callers=005dfbe0 callees=0054a840

undefined4 FUN_005f6a70(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_0054a840/*CSWSObject::GetClientObject*/();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_0054a840/*CSWSObject::GetClientObject*/();
    uVar3 = (**(code **)(*piVar2 + 0x4c))(piVar2,uVar3);
  }
  return uVar3;
}

```

## R269 OFFSET CSWGuiPanel.border = 0x60
Claim: Draw 0x40ec90 calls (*(this+0x60))->vt[3](dt); ctor 0x40e5a0 allocs 0x78-byte CSWGuiBorder (ctor 0x414a50) into param_1[0x18] (+0x60); Destructor 0x40e860 deletes it
Decompile 0x0040e5a0:
```c
// FN 0040e5a0 FUN_0040e5a0/*CSWGuiPanel::Constructor*/ size=423 ret=4 callers=0075b370,008850b0,00880740,00893950,008a1170,00892090,00918b10,0088aac0,009074e0,007d6170,00747210,00880150,008c19f0,008b4270,0074a500,0084c3a0,008d1020,008b9ce0,0089c5f0,007d8540,0075ae40,007d23e0,00760210,007d9800,008c80f0,008c9e10,00850770,007d4620,007dc410,008fb7a0,0075fda0,00916ef0,007dacb0,00886fc0,008a92d0,008ba980,009153d0,007bc600,00757c40,008e1810,008586d0,008db630,008f92b0,00910490,008b91f0,008bfb60,008f4af0,008fc8e0,008b1ea0,008973d0,00916640,008bb5d0,008b82e0,00902f60,00904a80,008fff20,008bdc90,0089cf30,008df930,008c0cb0,007fae60,00754ed0,008c6650,0087fd40,00914000,008a25c0,008d6b90,00411b20,007d31d0,0090c6a0,0075cec0,008a6170,008bee10,00887da0,00909e00 callees=00919723,00414a50,0084a640

undefined4 * __thiscall FUN_0040e5a0/*CSWGuiPanel::Constructor*/(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_009482ae;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiObject::vftable;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0;
  local_8 = 0;
  *param_1 = CSWGuiPanel::vftable;
  FUN_0084a640(0);
  local_8._0_1_ = 1;
  param_1[0x15] = 0;
  param_1[0x16] = 0;
  param_1[0x17] = 0;
  iVar2 = FUN_00919723/*operator new*/(0x78,uVar1);
  local_8 = CONCAT31(local_8._1_3_,2);
  if (iVar2 == 0) {
    local_28 = 0;
  }
  else {
    local_28 = FUN_00414a50/*CSWGuiBorder::Constructor*/();
  }
  param_1[0x18] = local_28;
  param_1[0x12] = param_1[0x12] & 0xfffffffd;
  param_1[8] = 0;
  param_1[7] = param_2;
  param_1[0xc] = 0;
  param_1[0x12] = param_1[0x12] | 4;
  param_1[0x12] = param_1[0x12] | 0x40;
  param_1[0x12] = param_1[0x12] & 0xffffffcf;
  param_1[0x12] = param_1[0x12] & 0xfffffcff;
  param_1[0x12] = param_1[0x12] & 0xffffff7f;
  param_1[0x14] = 0x3f800000;
  *(undefined1 *)(param_1 + 0x13) = 0;
  param_1[0x12] = param_1[0x12] | 1;
  *(undefined1 *)(param_1 + 0x19) = 0xff;
... (5 more lines)
```
Decompile 0x0040e860:
```c
// FN 0040e860 FUN_0040e860/*CSWGuiPanel::Destructor*/ size=276 ret=0 callers=00975670,0087fdf0,009811e0,00897e70,009766d0,0097af70,0074a730,0097f3d0,008855c0,0096a410,00969500,00978c80,00912220,009747c0,00976370,00979b20,0097ef50,009815f0,0084df70,007dcb00,0097d0a0,0097a3a0,0097c6e0,00978160,00894140,00758a70,00979d80,0097da80,00917a30,007563b0,009694c0,0096a860,00979ab0,00977cd0,00963fd0,00980ec0,00905b80,0097ab70,0097fc90,00963ed0,0096b110,00908070,00971290,00982340,00887920,00916a10,008d7af0,00969ee0,008b4f60,0097c3b0,0097fb90,009770e0,009826c0,007d8f20,00982290,0096ae50,008b2620,0097a280,009745f0,0097fda0,0097cd10,00980c20,00977640,00982080,00974570,008c84c0,008803d0,008baaa0,00974e30,00963320,00918f00,0096a090,00976db0,0097adf0,00971e80,009824b0,00969fe0,0090e440,0097ad40,00982130,008bf000,0089dc30,0097d670,0097dbc0,008b9e80,009645e0,0096afb0,00977f50,0074ad90,0096a620,0075fea0,007d2a80,0097bd10,0097a4d0,0089ca70,009798d0,008f5be0,0096be00,00760520,007db4c0,009751b0,009157d0,00980640,009801d0,0097eba0,009818d0,009828a0,009785e0,00978db0,0097a880,00903ad0,008cad50,008fd490,0097e6f0,0097f020,0074a650,00962bd0,0097b970,008b8790,00979860,0097ed70,00979d10,007da3c0,00963700,007fbad0,008c1f20,008dc620,009718e0,00972db0,009808e0,00979bf0,007d70d0,00975fc0,0088c110,009005d0,00963f50,007d52e0,00979c60,0097e2b0,00975490,008a1960,00964560,009823e0,0097a630,008fa2f0,00974d70,008925f0,0097a240,00914510,00974ff0,0075b260,009793b0,00851390,00976850,0097d8c0,0096b290,00979090,00964520,00976ca0,008a7060,008fbbf0,0096aa40,0097a310,00979d50,009640a0,008d21c0,00962910,00963c60,008c6e90,00975a40,008bde40,009774c0,008e07b0,0097aa10,0096a230,00881670,00972d30,008a3240,0097b250,00858970,007d3600,00976e60,009622d0,00977910,0096b3c0,00888ad0,0096a160,009752b0,0096ab10,00981ce0,0075d820,0097a7d0,00964250,009821e0,00982970,007bc760,008aa750,00976050,009720b0,008b94a0,0096ac90,00974540,00963a40,008e1f10,0090a9a0,00976230,00977320,0097f1e0,0097f770,0096c000,0097f640 callees=0040f5a0,00410800,0064ddb0,00410670

void __fastcall FUN_0040e860/*CSWGuiPanel::Destructor*/(CSize *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009482e3;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined ***)param_1 = CSWGuiPanel::vftable;
  local_8 = 1;
  if (*(undefined4 **)(param_1 + 0x60) == (undefined4 *)0x0) {
    uVar2 = 0;
  }
  else {
    uVar2 = (**(code **)**(undefined4 **)(param_1 + 0x60))(1,uVar1);
  }
  *(undefined4 *)(param_1 + 0x60) = 0;
  FUN_0040f5a0/*CSWGuiPanel::StopLoadFromLayout*/();
  if (*(undefined4 **)(param_1 + 0x30) == (undefined4 *)0x0) {
    uVar3 = 0;
  }
  else {
    uVar3 = (**(code **)**(undefined4 **)(param_1 + 0x30))(1);
  }
  *(undefined4 *)(param_1 + 0x30) = 0;
  if (*(int *)(param_1 + 0x1c) != 0) {
    iVar4 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x1c),param_1);
    if (iVar4 != 0) {
      FUN_00410670(param_1);
    }
  }
  local_8 = local_8 & 0xffffff00;
  Free(uVar1,uVar3,uVar2);
  *(undefined ***)param_1 = CSWGuiObject::vftable;
  ExceptionList = local_10;
  return;
}

```
Decompile 0x0040ec90:
```c
// FN 0040ec90 FUN_0040ec90/*CSWGuiPanel::Draw*/ size=432 ret=4 callers=008fb130,007bcab0,008859a0,008a7350,008fe2b0,00760720,0090bbb0,0084fbb0,008b8ee0,008bb260,00892870,008b98d0,008be740,00756b70,008b82a0,0088e510,00918730,0074b3f0,00894cd0,00882080,008b8ae0,00898c20,009091e0,008aba50,008cce70,007fbce0,00907360 callees=004799c0,00479460,004793f0,00410020,004792c0

void __thiscall FUN_0040ec90/*CSWGuiPanel::Draw*/(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (((((*(uint *)(param_1 + 0x48) >> 6 & 1) != 0) && (-1 < *(int *)(param_1 + 4))) &&
      (-1 < *(int *)(param_1 + 8))) &&
     ((0 < *(int *)(param_1 + 0x10) && (0 < *(int *)(param_1 + 0xc))))) {
    if (*(int *)(param_1 + 4) + *(int *)(param_1 + 0xc) <=
        (int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6c)) {
      if (*(int *)(param_1 + 8) + *(int *)(param_1 + 0x10) <=
          (int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6e)) {
        iVar1 = *(int *)(param_1 + 0x28);
        local_14 = *(undefined4 *)(param_1 + 4);
        local_10 = *(undefined4 *)(param_1 + 8);
        local_c = *(undefined4 *)(param_1 + 0xc);
        local_8 = *(undefined4 *)(param_1 + 0x10);
        FUN_00410020/*CSWGuiPanel::GetExtentAccountingForPanelOffset*/(&local_14);
        FUN_004792c0();
        iVar2 = FUN_00479460(local_14,local_10,local_c,local_8,param_1 + 0x54,0,
                             *(undefined4 *)(param_1 + 0x50));
        if (iVar2 != 0) {
          (**(code **)(**(int **)(param_1 + 0x60) + 0xc))(param_2);
          for (local_18 = 0; local_18 < iVar1; local_18 = local_18 + 1) {
            if ((*(int *)(*(int *)(param_1 + 0x24) + local_18 * 4) != 0) &&
               ((*(uint *)(*(int *)(*(int *)(param_1 + 0x24) + local_18 * 4) + 0x48) >> 1 & 1) != 0)
               ) {
              (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_18 * 4) + 0x38))(param_2);
            }
          }
          FUN_004799c0();
        }
        FUN_004793f0();
      }
    }
  }
  return;
}

```
