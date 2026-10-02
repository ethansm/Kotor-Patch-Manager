# Digest F_Scene_gui

### CSWGuiBorder::Constructor  gog=0x912e20 (param_bytes=0, notes='CSWGuiBorder * this')
steam: cand=0x00414a50(mapper) delta=-0x4fe3d0 align=6/6 size=254 retn=0 gapG/S=288/256 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=18 callees=00414920=gog:CSWGuiBorderParams::SetBorder, 004143d0=gog:CSWGuiBorderParams::Constructor, 007107f0
```
undefined4 * __fastcall FUN_00414a50(undefined4 *param_1)
{
  void *local_10;
  puStack_c = &LAB_009485d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiDrawable::vftable;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0;
  local_8 = 0;
  *param_1 = CSWGuiBorder::vftable;
  local_1c = 0x3f800000;
  local_18 = 0x3f800000;
  local_14 = 0x3f800000;
  uVar10 = 0;
  uVar9 = 0;
  puVar8 = &local_1c;
  uVar7 = 0x3f800000;
  uVar6 = 0;
  uVar5 = 0;
  uVar2 = FUN_007107f0(0,0,0x3f800000,puVar8,0,0,uVar1);
  uVar3 = FUN_007107f0(uVar2);
  uVar4 = FUN_007107f0(uVar3);
  FUN_004143d0(uVar4,uVar3,uVar2,uVar5,uVar6,uVar7,puVar8,uVar9,uVar10);
  param_1[0x1b] = 0;
  param_1[0x1c] = 0;
  param_1[0x1d] = 0;
  FUN_00414920(param_1);
  ExceptionList = local_10;
  return param_1;
}
```

### CSWGuiBorder::Destructor  gog=0x912fc0 (param_bytes=4, notes='CSWGuiBorder * this, uint param_1')
steam: cand=0x00414bf0(mapper) delta=-0x4fe3d0 align=6/6 size=44 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=00414c60=CSWGuiBorder::Destructor_2, 009196fd=operator delete
```
undefined4 __thiscall FUN_00414bf0(undefined4 param_1,uint param_2)
{
  FUN_00414c60();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### CSWGuiBorder::Initialize  gog=0x912ff0 (param_bytes=8, notes='CSWGuiBorder * this, CSWGuiExtent * param_1, CSWGuiBorderParams * para')
steam: cand=0x00414c20(mapper) delta=-0x4fe3d0 align=6/6 size=59 retn=8 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* extent, void* borderParams) [param_bytes=8]
callers=9 callees=004144f0=gog:CSWGuiBorderParams::operator=
```
void __thiscall FUN_00414c20(int param_1,undefined4 *param_2,undefined4 param_3)
{
  *(undefined4 *)(param_1 + 4) = *param_2;
  *(undefined4 *)(param_1 + 8) = param_2[1];
  *(undefined4 *)(param_1 + 0xc) = param_2[2];
  *(undefined4 *)(param_1 + 0x10) = param_2[3];
  FUN_004144f0(param_3);
  return;
}
```

### CSWGuiBorder::GetInnerExtent  gog=0x913150 (param_bytes=4, notes='CSWGuiBorder * this, CSWGuiExtent * param_1')
steam: cand=0x00414d80(mapper) delta=-0x4fe3d0 align=6/6 size=758 retn=4 gapG/S=768/768 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* outExtent) [param_bytes=4]
callers=21 callees=00414980=gog:CSWGuiBorderParams::GetBorderDim
```
int * __thiscall FUN_00414d80(int param_1,int *param_2)
{
  iVar1 = FUN_00414980();
  if (iVar1 == 0) {
    if ((((*(int *)(param_1 + 0x6c) == 0) || (*(int *)(param_1 + 0x70) == 0)) ||
        (*(int *)(param_1 + 0x10) == 0)) || (*(int *)(param_1 + 0xc) == 0)) {
      local_20 = 0;
      local_24 = 0;
      local_18 = 0;
    }
    else {
      local_24 = (**(code **)(**(int **)(param_1 + 0x6c) + 0xc))();
      local_18 = (**(code **)(**(int **)(param_1 + 0x6c) + 0x10))();
      (**(code **)(**(int **)(param_1 + 0x70) + 0xc))();
      local_20 = (**(code **)(**(int **)(param_1 + 0x70) + 0x10))();
    }
  }
  else {
    local_24 = FUN_00414980();
    local_20 = local_24;
    local_18 = local_24;
  }
  iVar1 = *(int *)(param_1 + 0xc) + local_18 * -2;
  if (local_20 == 0) {
    local_40 = 0;
  }
  else {
    local_40 = iVar1 / local_20;
  }
  if ((iVar1 < 1) || ((local_40 == 0 && (iVar1 < local_20 / 2)))) {
    local_34 = *(int *)(param_1 + 0xc) / 2 + *(int *)(param_1 + 4);
    local_2c = 0;
  }
  else {
    local_34 = *(int *)(param_1 + 4) + local_18;
    if (*(int *)(param_1 + 0xc) + local_18 * -2 < 0) {
      local_44 = 0;
    }
    else {
      local_44 = *(int *)(param_1 + 0xc) + local_18 * -2;
    }
    local_2c = local_44;
  }
  iVar1 = *(int *)(param_1 + 0x10) + local_24 * -2;
  if (local_20 == 0) {
... (41 more lines)
```

### CSWGuiBorder::Load  gog=0x913450 (param_bytes=12, notes='CSWGuiBorder * this, CResGFF * param_1, CResStruct * param_2, CExoStri')
steam: cand=0x00415080(mapper) delta=-0x4fe3d0 align=6/6 size=639 retn=12 gapG/S=640/640 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* gff, void* item, void* label) [param_bytes=12]
callers=0 callees=00414760=gog:CSWGuiBorderParams::SetEdgeImage, 00734010=CExoString::CStr, 00718c80=gog:CResGFF::ReadFieldINT, 00719520=gog:CResGFF::ReadFieldVector, 00718960=gog:CResGFF::GetStructFromStruct, 00414680=gog:CSWGuiBorderParams::SetCornerImage, 00710840, 00718a00, 004149f0=gog:CSWGuiBorderParams::SetPulsingAlpha, 00718e40=gog:CResGFF::ReadFieldCResRef, 00414840=CSWGuiControl image setter thiscall(CResRef* name,
strings: COLOR | PULSING | INNEROFFSETY | INNEROFFSET | DIMENSION | FILLSTYLE
```
void __thiscall FUN_00415080(int param_1,undefined4 param_2,undefined4 param_3)
{
  undefined1 local_88 [12];
  undefined1 local_6d [16];
  undefined1 local_5d [16];
  undefined1 local_4d [16];
  undefined1 local_3d [16];
  undefined1 local_2d [16];
  undefined1 local_1d [16];
  undefined1 local_c [4];
  undefined1 local_8 [4];
  uVar1 = FUN_00734010();
  iVar2 = FUN_00718960(local_8,param_3,uVar1);
  if (iVar2 != 0) {
    FUN_00710840(&DAT_00986925);
    uVar1 = FUN_00718e40(local_2d,local_8,"CORNER",local_c,local_1d);
    FUN_00414680(uVar1,0);
    FUN_00710840(&DAT_00986925);
    uVar1 = FUN_00718e40(local_4d,local_8,&DAT_0098762c,local_c,local_3d);
    FUN_00414760(uVar1,0);
    FUN_00710840(&DAT_00986925);
    uVar1 = FUN_00718e40(local_6d,local_8,&DAT_00987624,local_c,local_5d);
    FUN_00414840(uVar1,0);
    uVar3 = FUN_00718c80(local_8,"FILLSTYLE",local_c,0);
    *(uint *)(param_1 + 0x34) = *(uint *)(param_1 + 0x34) & 0xfffffffc | uVar3 & 3;
    uVar1 = FUN_00718c80(local_8,"DIMENSION",local_c,0);
    *(undefined4 *)(param_1 + 0x14) = uVar1;
    uVar1 = FUN_00718c80(local_8,"INNEROFFSET",local_c,0);
    *(undefined4 *)(param_1 + 0x18) = uVar1;
    uVar1 = FUN_00718c80(local_8,"INNEROFFSETY",local_c,0);
    *(undefined4 *)(param_1 + 0x1c) = uVar1;
    local_7c = 0x3f800000;
    local_78 = 0x3f800000;
    local_74 = 0x3f800000;
    puVar4 = (undefined4 *)FUN_00719520(local_88,local_8,"COLOR",local_c,&local_7c);
    *(undefined4 *)(param_1 + 0x28) = *puVar4;
    *(undefined4 *)(param_1 + 0x2c) = puVar4[1];
    *(undefined4 *)(param_1 + 0x30) = puVar4[2];
    local_d = FUN_00718a00(local_8,"PULSING",local_c,0);
    FUN_004149f0(local_d != 0,(local_d & 1) != 0,(local_d & 2) != 0);
  }
  return;
}
```

### CSWGuiBorder::Draw  gog=0x9136d0 (param_bytes=4, notes='CSWGuiBorder * this, float param_1')
steam: cand=0x00415300(mapper) delta=-0x4fe3d0 align=6/6 size=1886 retn=4 gapG/S=1888/1888 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, float alpha) [param_bytes=4]
callers=0 callees=00414980=gog:CSWGuiBorderParams::GetBorderDim, 00415a60=gog:CSWGuiBorder::FillTile, 00415d20=gog:CSWGuiBorder::FillCenter, 0047ea60
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
void __fastcall FUN_00415300(int param_1)
{
  if ((*(int *)(param_1 + 0xc) != 0) && (*(int *)(param_1 + 0x10) != 0)) {
    local_24 = *(undefined4 *)(param_1 + 0x24);
    local_14 = *(undefined4 *)(param_1 + 0x24);
    if ((*(uint *)(param_1 + 0x34) >> 2 & 1) != 0) {
      if ((*(uint *)(param_1 + 0x34) >> 3 & 1) != 0) {
        local_24 = _DAT_009f4548;
      }
      if ((*(uint *)(param_1 + 0x34) >> 4 & 1) != 0) {
        local_14 = _DAT_009f4548;
      }
    }
    local_18 = *(int *)(param_1 + 4);
    local_20 = *(int *)(param_1 + 8);
    local_28 = 0;
    local_8 = 0;
    local_1c = 0;
    local_10 = 0;
    if (*(int *)(param_1 + 0x6c) != 0) {
      iVar1 = FUN_00414980();
      if (iVar1 == 0) {
        local_40 = (**(code **)(**(int **)(param_1 + 0x6c) + 0x10))();
        local_54 = (**(code **)(**(int **)(param_1 + 0x6c) + 0xc))();
        local_50 = local_40;
        local_3c = local_54;
        if (*(int *)(param_1 + 0x70) != 0) {
          local_50 = (**(code **)(**(int **)(param_1 + 0x70) + 0x10))();
          local_3c = (**(code **)(**(int **)(param_1 + 0x70) + 0xc))();
        }
      }
      else {
        local_54 = FUN_00414980();
        local_50 = local_54;
        local_40 = local_54;
        local_3c = local_54;
      }
      local_38 = *(int *)(param_1 + 0xc) + local_40 * -2;
      local_30 = 0;
      if (0 < local_38) {
        if (local_50 == 0) {
          local_7c = 0;
        }
        else {
... (146 more lines)
```

### CSWGuiBorder::FillTile  gog=0x913e30 (param_bytes=24, notes='CSWGuiBorder * this, int height, int width, int x, int y, float alpha,')
steam: cand=0x00415a60(mapper) delta=-0x4fe3d0 align=6/6 size=702 retn=24 gapG/S=704/704 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int height, int width, int x, int y, float alpha, Vector* color) [param_bytes=24]
callers=1 callees=
```
void __thiscall
FUN_00415a60(int param_1,int param_2,int param_3,int param_4,int param_5,undefined4 param_6,
            undefined4 param_7)
{
  iVar3 = (**(code **)(**(int **)(param_1 + 0x74) + 0x10))();
  iVar4 = (**(code **)(**(int **)(param_1 + 0x74) + 0xc))();
  if ((0 < iVar3) && (0 < iVar4)) {
    iVar5 = param_3 / iVar3;
    iVar6 = param_2 / iVar4;
    uVar1 = *(undefined4 *)(param_1 + 0x20);
    param_2 = param_2 - iVar6 * iVar4;
    iVar7 = param_3 - iVar5 * iVar3;
    if (iVar6 != 0) {
      local_14 = param_2 / iVar6;
      param_2 = param_2 - local_14 * iVar6;
    }
    iVar2 = param_2;
    if (iVar5 == 0) {
      if (iVar6 == 0) {
        (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
                  (param_4,param_5,iVar7,param_2,0,uVar1,param_7,param_6);
      }
      else {
        local_40 = param_5;
        for (local_10 = 0; local_10 < iVar6; local_10 = local_10 + 1) {
          local_44 = iVar4 + local_14;
          if (param_2 != 0) {
            param_2 = param_2 + -1;
            local_44 = local_44 + 1;
          }
          (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
                    (param_4,local_40,iVar7,local_44,0,uVar1,param_7,param_6);
          local_40 = local_40 + local_44;
        }
      }
    }
    else {
      param_3 = iVar7 - (iVar7 / iVar5) * iVar5;
      local_28 = param_4;
      for (local_10 = 0; local_10 < iVar5; local_10 = local_10 + 1) {
        local_34 = iVar3 + iVar7 / iVar5;
        if (param_3 != 0) {
          param_3 = param_3 + -1;
          local_34 = local_34 + 1;
        }
... (24 more lines)
```

### CSWGuiBorder::FillCenter  gog=0x9140f0 (param_bytes=24, notes='CSWGuiBorder * this, int height, int width, int x, int y, float alpha,')
steam: cand=0x00415d20(mapper) delta=-0x4fe3d0 align=6/6 size=384 retn=24 gapG/S=384/384 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int height, int width, int x, int y, float alpha, Vector* color) [param_bytes=24]
callers=1 callees=
```
void __thiscall
FUN_00415d20(int param_1,int param_2,int param_3,int param_4,int param_5,undefined4 param_6,
            undefined4 param_7)
{
  iVar2 = (**(code **)(**(int **)(param_1 + 0x74) + 0x10))();
  iVar3 = (**(code **)(**(int **)(param_1 + 0x74) + 0xc))();
  uVar1 = *(undefined4 *)(param_1 + 0x20);
  if ((iVar2 < param_3) && (iVar3 < param_2)) {
    (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
              ((param_3 - iVar2) / 2 + param_4,(param_2 - iVar3) / 2 + param_5,iVar2,iVar3,0,uVar1,
               param_7,param_6);
  }
  else if (iVar2 < param_3) {
    (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
              ((param_3 - iVar2) / 2 + param_4,param_5,iVar2,param_2,0,uVar1,param_7,param_6);
  }
  else if (iVar3 < param_2) {
    (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
              (param_4,(param_2 - iVar3) / 2 + param_5,param_3,iVar3,0,uVar1,param_7,param_6);
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
              (param_4,param_5,param_3,param_2,0,uVar1,param_7,param_6);
  }
  return;
}
```

### CSWGuiBorderParams::SetCornerImage  gog=0x912a50 (param_bytes=8, notes='CSWGuiBorderParams * this, CResRef * image, int forceUpdate')
steam: cand=0x00414680(mapper) delta=-0x4fe3d0 align=5/6 size=219 retn=8 gapG/S=224/224 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* image, int forceUpdate) [param_bytes=8]
callers=3 callees=00710ca0, 00710a70=ResRef16 != (memcmp of 16 bytes), 00710a10, 0047eb60, 00710980, 009196ee=__security_check_cookie
```
void __thiscall FUN_00414680(int param_1,undefined4 param_2,int param_3)
{
  undefined1 local_2c [16];
  undefined1 local_1c [20];
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  iVar2 = FUN_00710a70(param_1 + 0x24);
  if (((iVar2 != 0) || (param_3 != 0)) &&
     (FUN_00710ca0(local_2c,param_2), *(int *)(param_1 + 0x54) != 0)) {
    if (*(int *)(*(int *)(param_1 + 0x54) + 0x6c) != 0) {
      piVar1 = *(int **)(*(int *)(param_1 + 0x54) + 0x6c);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 4))(1);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x6c) = 0;
    }
    iVar2 = FUN_00710a10();
    if (iVar2 != 0) {
      FUN_00710980(local_1c);
      uVar3 = FUN_0047eb60(local_1c);
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x6c) = uVar3;
    }
  }
  __security_check_cookie(local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### CSWGuiBorderParams::SetEdgeImage  gog=0x912b30 (param_bytes=8, notes='CSWGuiBorderParams * this, CResRef * param_1, int forceUpdate')
steam: cand=0x00414760(mapper) delta=-0x4fe3d0 align=6/6 size=219 retn=8 gapG/S=224/224 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* image, int forceUpdate) [param_bytes=8]
callers=3 callees=00710ca0, 00710a70=ResRef16 != (memcmp of 16 bytes), 00710a10, 0047eb60, 00710980, 009196ee=__security_check_cookie
```
void __thiscall FUN_00414760(int param_1,undefined4 param_2,int param_3)
{
  undefined1 local_2c [16];
  undefined1 local_1c [20];
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  iVar2 = FUN_00710a70(param_1 + 0x34);
  if (((iVar2 != 0) || (param_3 != 0)) &&
     (FUN_00710ca0(local_2c,param_2), *(int *)(param_1 + 0x54) != 0)) {
    if (*(int *)(*(int *)(param_1 + 0x54) + 0x70) != 0) {
      piVar1 = *(int **)(*(int *)(param_1 + 0x54) + 0x70);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 4))(1);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x70) = 0;
    }
    iVar2 = FUN_00710a10();
    if (iVar2 != 0) {
      FUN_00710980(local_1c);
      uVar3 = FUN_0047eb60(local_1c);
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x70) = uVar3;
    }
  }
  __security_check_cookie(local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### CSWGuiBorderParams::SetFillImage  gog=0x912c10 (param_bytes=8, notes='CSWGuiBorderParams * this, CResRef * param_1, int forceUpdate')
steam: cand=0x00414840(mapper) delta=-0x4fe3d0 align=6/6 size=219 retn=8 gapG/S=224/224 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* image, int forceUpdate) [param_bytes=8]
name_map: CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force [high] asm 0x414840..0x414918
callers=50 callees=00710ca0, 00710a70=ResRef16 != (memcmp of 16 bytes), 00710a10, 0047eb60, 00710980, 009196ee=__security_check_cookie
```
void __thiscall FUN_00414840(int param_1,undefined4 param_2,int param_3)
{
  undefined1 local_2c [16];
  undefined1 local_1c [20];
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  iVar2 = FUN_00710a70(param_1 + 0x44);
  if (((iVar2 != 0) || (param_3 != 0)) &&
     (FUN_00710ca0(local_2c,param_2), *(int *)(param_1 + 0x54) != 0)) {
    if (*(int *)(*(int *)(param_1 + 0x54) + 0x74) != 0) {
      piVar1 = *(int **)(*(int *)(param_1 + 0x54) + 0x74);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 4))(1);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x74) = 0;
    }
    iVar2 = FUN_00710a10();
    if (iVar2 != 0) {
      FUN_00710980(local_1c);
      uVar3 = FUN_0047eb60(local_1c);
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x74) = uVar3;
    }
  }
  __security_check_cookie(local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### CSWGuiBorderParams::SetBorder  gog=0x912cf0 (param_bytes=4, notes='CSWGuiBorderParams * this, CSWGuiBorder * param_1')
steam: cand=0x00414920(mapper) delta=-0x4fe3d0 align=6/6 size=82 retn=4 gapG/S=96/96 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* border) [param_bytes=4]
callers=2 callees=00414760=gog:CSWGuiBorderParams::SetEdgeImage, 00414680=gog:CSWGuiBorderParams::SetCornerImage, 00414840=CSWGuiControl image setter thiscall(CResRef* name,
```
void __thiscall FUN_00414920(int param_1,undefined4 param_2)
{
  *(undefined4 *)(param_1 + 0x54) = param_2;
  if (*(int *)(param_1 + 0x54) != 0) {
    FUN_00414680(param_1 + 0x24,1);
    FUN_00414760(param_1 + 0x34,1);
    FUN_00414840(param_1 + 0x44,1);
  }
  return;
}
```

### CSWGuiControl::Destructor  gog=0x4ce500 (param_bytes=0, notes='CSWGuiControl * this')
steam: cand=0x00418640(name_map) delta=-0xb5ec0 align=0/6 size=126 retn=0 gapG/S=128/128 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWGuiControl::Destructor [high] GOG 0x4ce500 CSWGuiControl::Destructor p0 sz 128 (Steam 126); class dtors live in a separate GOG region
callers=432 callees=0064ddb0=Free(ptr*) delete-and-zero helper, 00733780=CExoString::~CExoString
```
void __fastcall FUN_00418640(undefined4 *param_1)
{
  void *local_10;
  puStack_c = &LAB_0094892e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiControl::vftable;
  local_8 = 2;
  Free(uVar1);
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  Free();
  *param_1 = CSWGuiObject::vftable;
  ExceptionList = local_10;
  return;
}
```

### CSWGuiControl::SetActive  gog=0x4ce5d0 (param_bytes=4, notes='CSWGuiControl * this, int param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### CSWGuiControl::Constructor  gog=0x916c30 (param_bytes=0, notes='CSWGuiControl * this')
steam: cand=0x00418990(mapper) delta=-0x4fe2a0 align=6/6 size=347 retn=0 gapG/S=352/352 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSCreature::AIActionWaitForEndOfRound [low] gog 0x6db820; vote; retn=0 | DEMOTED 2026-10-01 by ai-review-2026-10-01: GUI code, not AIActionWaitForEndOfRound
callers=16 callees=0084a640, 008a5f70, 00733540=CExoString::CExoString(empty)
```
undefined4 * __fastcall FUN_00418990(undefined4 *param_1)
{
  void *local_10;
  puStack_c = &LAB_0094896e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiObject::vftable;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0;
  local_8 = 0;
  *param_1 = CSWGuiControl::vftable;
  FUN_0084a640(0);
  local_8._0_1_ = 1;
  FUN_00733540(uVar1);
  local_8 = CONCAT31(local_8._1_3_,2);
  FUN_008a5f70(0);
  param_1[0xe] = 0;
  param_1[0x12] = param_1[0x12] & 0xfffffffe;
  param_1[0x12] = param_1[0x12] | 2;
  param_1[0x17] = 0;
  param_1[0x15] = 0xffffffff;
  *(undefined1 *)(param_1 + 0x16) = 0;
  *(undefined1 *)((int)param_1 + 0x59) = 1;
  param_1[6] = 0;
  param_1[0x12] = param_1[0x12] & 0xffffffef;
  param_1[0x12] = param_1[0x12] | 8;
  param_1[0x12] = param_1[0x12] & 0xffffffdf;
  param_1[10] = 0;
  param_1[0xd] = 0;
  param_1[0x12] = param_1[0x12] & 0xffffffbf;
  ExceptionList = local_10;
  return param_1;
}
```

### CSWGuiControl::AddEvent  gog=0x916d90 (param_bytes=12, notes='CSWGuiControl * this, undefined4 param_1, CSWGuiObject * param_2, unde')
steam: cand=0x00418af0(mapper) delta=-0x4fe2a0 align=6/6 size=177 retn=12 gapG/S=192/192 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int eventFlag, void* guiObject, void* menuFunc) [param_bytes=12]
name_map: CSWGuiControl::AddEvent [medium] mapper vote GOG 0x916d90 (retn=12), Steam ret=12, sz=177
callers=87 callees=007a1940, 008a5fb0
```
void __thiscall FUN_00418af0(int param_1,int param_2,undefined4 param_3,int param_4)
{
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

### CSWGuiControl::Load_2  gog=0x917090 (param_bytes=16, notes='CSWGuiControl * this, CSWGuiObject * param_1, CResGFF * param_2, CResL')
steam: cand=0x00418df0(mapper) delta=-0x4fe2a0 align=6/6 size=300 retn=16 gapG/S=304/304 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* owner, void* gff, void* controls, void* tag) [param_bytes=16]
callers=1 callees=00718810=gog:CResGFF::GetListCount, 00733570=CExoString::CExoString_char, 007337c0=CExoString::operator=, 007339d0=CExoString::operator==, 00718f90=CResGFF::ReadFieldCExoString, 00718890=gog:CResGFF::GetListElement, 00733780=CExoString::~CExoString, 00733540=CExoString::CExoString(empty)
```
void __thiscall
FUN_00418df0(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)
{
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  undefined1 local_1c [4];
  undefined1 local_18 [4];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_009489a8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  param_1[0xe] = param_2;
  local_28 = FUN_00718810(param_4);
  FUN_00733540(uVar1);
  local_8 = 0;
  local_14 = 0;
  do {
    if (local_28 <= local_14) {
LAB_00418efc:
      local_8 = 0xffffffff;
      FUN_00733780();
      ExceptionList = local_10;
      return;
    }
    FUN_00718890(local_1c,param_4,local_14);
    FUN_00733570(&DAT_00986925);
    local_8._0_1_ = 1;
    uVar2 = FUN_00718f90(local_38,local_1c,&DAT_00987888,local_18,local_30);
    local_8._0_1_ = 2;
    FUN_007337c0(uVar2);
    local_8._0_1_ = 1;
    FUN_00733780();
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    iVar3 = FUN_007339d0(param_5);
    if (iVar3 != 0) {
      (**(code **)(*param_1 + 0x48))(param_3,local_1c);
      goto LAB_00418efc;
    }
    local_14 = local_14 + 1;
  } while( true );
}
```

### CSWGuiControl::GetIsSelectable  gog=0x917340 (param_bytes=0, notes='CSWGuiControl * this')
steam: cand=0x004190a0(mapper) delta=-0x4fe2a0 align=6/6 size=135 retn=0 gapG/S=144/144 cc_guess=thiscall
wrapper: bool (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
bool __fastcall FUN_004190a0(int *param_1)
{
  piVar1 = (int *)param_1[0xe];
  if ((piVar1 == (int *)0x0) || (iVar2 = (**(code **)(*piVar1 + 0x2c))(), iVar2 == 0)) {
    if (((uint)param_1[0x12] >> 3 & 1) != 0) {
      iVar2 = (**(code **)(*param_1 + 0x4c))();
      return iVar2 != 0;
    }
  }
  else {
    iVar2 = (**(code **)(*piVar1 + 0x2c))();
    if ((*(uint *)(iVar2 + 0x48) >> 3 & 1) != 0) {
      iVar2 = (**(code **)(*param_1 + 0x4c))();
      return iVar2 != 0;
    }
  }
  return false;
}
```

### CSWGuiControl::AddChildControl  gog=0x9175d0 (param_bytes=4, notes='CSWGuiControl * this, CSWGuiControl * child')
steam: cand=0x00419330(mapper) delta=-0x4fe2a0 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* child) [param_bytes=4]
callers=3 callees=0083ea60=CExoArrayList<uint32>::Add
```
void FUN_00419330(undefined4 param_1)
{
  FUN_0083ea60(param_1);
  return;
}
```

### CSWGuiControl::GetIsChild  gog=0x9175f0 (param_bytes=4, notes='CSWGuiControl * this, CSWGuiControl * child')
steam: cand=0x00419350(mapper) delta=-0x4fe2a0 align=6/6 size=72 retn=4 gapG/S=80/80 cc_guess=thiscall
wrapper: bool (__thiscall) (void* thisPtr, void* child) [param_bytes=4]
callers=1 callees=
```
undefined4 __thiscall FUN_00419350(int param_1,int param_2)
{
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x20) <= local_8) {
      return 0;
    }
    if (*(int *)(*(int *)(param_1 + 0x1c) + local_8 * 4) == param_2) break;
    local_8 = local_8 + 1;
  }
  return 1;
}
```

### CSWGuiControl::GetSelectableParent  gog=0x917640 (param_bytes=0, notes='CSWGuiControl * this')
steam: cand=0x004193a0(mapper) delta=-0x4fe2a0 align=6/6 size=72 retn=0 gapG/S=80/80 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=9 callees=
```
int * __fastcall FUN_004193a0(int *param_1)
{
  local_8 = param_1;
  do {
    if (local_8[6] == 0) {
      return (int *)0x0;
    }
    local_8 = (int *)local_8[6];
    iVar1 = (**(code **)(*local_8 + 0x80))();
  } while (iVar1 == 0);
  return local_8;
}
```

### CSWGuiControl::SetEnabled  gog=0x917690 (param_bytes=4, notes='CSWGuiControl * this, int param_1')
steam: cand=0x004193f0(mapper) delta=-0x4fe2a0 align=6/6 size=39 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, UINT enabled) [param_bytes=4]
callers=3 callees=
```
void __thiscall FUN_004193f0(int param_1,uint param_2)
{
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffff7 | (param_2 & 1) << 3;
  return;
}
```

### CSWGuiManager::AddPanel  gog=0x90e9b0 (param_bytes=12, notes='CSWGuiManager * this, CSWGuiPanel * panel, int flags, int playSound')
steam: cand=0x00410530(mapper) delta=-0x4fe480 align=6/6 size=317 retn=12 gapG/S=320/320 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* panel, int flags, int playSound) [param_bytes=12]
callers=151 callees=004122a0, 00410890=gog:CSWGuiManager::FindPanelIndex, 00411170=gog:CSWGuiManager::DetermineNeedToDraw, 00410900=gog:CSWGuiManager::PushModalPanel, 0083ea60=CExoArrayList<uint32>::Add
```
void __thiscall FUN_00410530(int param_1,int *param_2,uint param_3,int param_4)
{
  if (param_2 != (int *)0x0) {
    if (param_4 != 0) {
      FUN_004122a0((char)param_2[0x19]);
    }
    iVar1 = FUN_00410890(param_2);
    if (iVar1 == -1) {
      param_2[0x12] = param_2[0x12] & 0xfffffff7U | (uint)((param_3 & 2) != 0) << 3;
      if (((uint)param_2[0x12] >> 3 & 1) != 0) {
        *(char *)(param_1 + 0x70) = *(char *)(param_1 + 0x70) + '\x01';
      }
      FUN_0083ea60(param_2);
      if ((param_3 & 1) != 0) {
        FUN_00410900(param_2);
      }
      (**(code **)(*param_2 + 0x48))();
      FUN_00411170();
    }
    else if ((((uint)param_2[0x12] >> 8 & 3) == 2) || (((uint)param_2[0x12] >> 8 & 3) != 0)) {
      param_2[0x12] = param_2[0x12] & 0xffffff7f;
      param_2[0x12] = param_2[0x12] & 0xfffffcff;
      if ((param_3 & 1) != 0) {
        FUN_00410900(param_2);
      }
      (**(code **)(*param_2 + 0x48))();
      FUN_00411170();
    }
  }
  return;
}
```

### CSWGuiManager::PopModalPanel  gog=0x90ee40 (param_bytes=0, notes='void')
steam: cand=0x004109c0(mapper) delta=-0x4fe480 align=5/6 size=217 retn=0 gapG/S=224/224 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=56 callees=0049b840, 00411170=gog:CSWGuiManager::DetermineNeedToDraw
```
undefined4 __fastcall FUN_004109c0(int param_1)
{
  iVar2 = *(int *)(param_1 + 0x98);
  if (iVar2 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x94) + -4 + iVar2 * 4);
    FUN_0049b840(iVar2 + -1);
    FUN_00411170();
    if ((0 < *(int *)(param_1 + 0x98)) &&
       (iVar2 = (**(code **)(**(int **)(*(int *)(param_1 + 0x94) + -4 + *(int *)(param_1 + 0x98) * 4
                                       ) + 0x44))(), iVar2 != 0)) {
      piVar3 = (int *)(**(code **)(**(int **)(*(int *)(param_1 + 0x94) + -4 +
                                             *(int *)(param_1 + 0x98) * 4) + 0x44))();
      (**(code **)(*piVar3 + 0x7c))(1);
    }
  }
  return uVar1;
}
```

### CSWGuiManager::PlayGuiSound  gog=0x910720 (param_bytes=1, notes='undefined1 param_1')
steam: cand=0x004122a0(bracket) delta=-0x4fe480 align=/ size=94 retn=4 gapG/S=96/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, byte soundId) [param_bytes=4]
callers=139 callees=0070ad50
```
void __thiscall FUN_004122a0(int param_1,char param_2)
{
  if ((((param_2 != -1) && (param_2 < *(char *)(param_1 + 0xa4))) && (*(int *)(param_1 + 0xa0) != 0)
      ) && (*(int *)(*(int *)(param_1 + 0xa0) + param_2 * 4) != 0)) {
    FUN_0070ad50();
  }
  return;
}
```

### CSWGuiPanel::Constructor  gog=0x90cb30 (param_bytes=4, notes='CSWGuiPanel * this, CSWGuiManager * param_1')
steam: cand=0x0040e770(mapper) delta=-0x4fe3c0 align=0/6 size=13 retn=4 gapG/S=432/16 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* manager) [param_bytes=4]
callers=19 callees=
```
void FUN_0040e770(void)
{
  return;
}
```

### CSWGuiPanel::Destructor  gog=0x90cce0 (param_bytes=0, notes='CSWGuiPanel * this')
steam: cand=0x0040e860(mapper) delta=-0x4fe480 align=3/6 size=276 retn=0 gapG/S=288/288 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=211 callees=0040f5a0=gog:CSWGuiPanel::StopLoadFromLayout, 00410800=GuiManager::Contains, 0064ddb0=Free(ptr*) delete-and-zero helper, 00410670=gog:CSWGuiManager::RemovePanel
```
void __fastcall FUN_0040e860(CSize *param_1)
{
  void *local_10;
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
  FUN_0040f5a0();
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

### CSWGuiPanel::CenterPanel  gog=0x90ce80 (param_bytes=0, notes='CSWGuiPanel * this')
steam: cand=0x0040ea00(mapper) delta=-0x4fe480 align=5/6 size=149 retn=0 gapG/S=160/160 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=12 callees=
```
void __fastcall FUN_0040ea00(int param_1)
{
  *(int *)(param_1 + 4) =
       ((int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6c) - *(int *)(param_1 + 0xc)) / 2;
  *(int *)(param_1 + 8) =
       ((int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6e) - *(int *)(param_1 + 0x10)) / 2;
  local_8 = *(undefined4 *)(param_1 + 0x10);
  local_c = *(undefined4 *)(param_1 + 0xc);
  local_14 = 0;
  local_10 = 0;
  (**(code **)(**(int **)(param_1 + 0x60) + 4))(&local_14);
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) | 1;
  return;
}
```

### CSWGuiPanel::HandleInputEvent  gog=0x90cf20 (param_bytes=8, notes='CSWGuiPanel * this, int event, int param_2')
steam: cand=0x0040eaa0(mapper) delta=-0x4fe480 align=6/6 size=51 retn=8 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int event, int inputPhase) [param_bytes=8]
callers=58 callees=
```
void __thiscall FUN_0040eaa0(int param_1,undefined4 param_2,undefined4 param_3)
{
  if (*(int *)(param_1 + 0x20) != 0) {
    (**(code **)(**(int **)(param_1 + 0x20) + 0x3c))(param_2,param_3);
  }
  return;
}
```

### CSWGuiPanel::HitCheckMouse  gog=0x90cf60 (param_bytes=8, notes='CSWGuiPanel * this, int mouseX, int mouseY')
steam: cand=0x0040eae0(mapper) delta=-0x4fe480 align=6/6 size=287 retn=8 gapG/S=288/288 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, int mouseX, int mouseY) [param_bytes=8]
callers=3 callees=00410020=gog:CSWGuiPanel::GetExtentAccountingForPanelOffset
```
int __thiscall FUN_0040eae0(int param_1,int param_2,int param_3)
{
  local_14 = *(int *)(param_1 + 4);
  local_10 = *(int *)(param_1 + 8);
  local_c = *(int *)(param_1 + 0xc);
  local_8 = *(int *)(param_1 + 0x10);
  if ((*(uint *)(param_1 + 0x48) & 1) == 0) {
    param_2 = param_2 - (DAT_009f42a4 + -0x280) / 2;
    param_3 = param_3 - (DAT_009f42a8 + -0x1e0) / 2;
  }
  else {
    FUN_00410020(&local_14);
  }
  if ((((param_2 < local_14) || (local_14 + local_c < param_2)) || (param_3 < local_10)) ||
     (local_10 + local_8 < param_3)) {
LAB_0040ebf7:
    iVar1 = 0;
  }
  else {
    param_2 = param_2 - local_14;
    param_3 = param_3 - local_10;
    local_18 = *(int *)(param_1 + 0x28);
    do {
      local_18 = local_18 + -1;
      if (local_18 < 0) goto LAB_0040ebf7;
    } while ((*(int *)(*(int *)(param_1 + 0x24) + local_18 * 4) == 0) ||
            (iVar1 = (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_18 * 4) + 0x44))
                               (param_2,param_3), iVar1 == 0));
  }
  return iVar1;
}
```

### CSWGuiPanel::SetActiveControl  gog=0x90d080 (param_bytes=8, notes='CSWGuiPanel * this, CSWGuiControl * controlToActivate, int playSound')
steam: cand=0x0040ec00(mapper) delta=-0x4fe480 align=6/6 size=143 retn=8 gapG/S=144/144 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* controlToActivate, int playSound) [param_bytes=8]
callers=32 callees=004122a0
```
void __thiscall FUN_0040ec00(int param_1,int param_2,int param_3)
{
  if (*(int *)(param_1 + 0x20) != param_2) {
    if (*(int *)(param_1 + 0x20) != 0) {
      (**(code **)(**(int **)(param_1 + 0x20) + 0x3c))(1,1);
    }
    *(int *)(param_1 + 0x20) = param_2;
    if (((*(int *)(param_1 + 0x20) != 0) &&
        ((**(code **)(**(int **)(param_1 + 0x20) + 0x3c))(0,1), param_3 != 0)) &&
       (*(int *)(param_1 + 0x1c) != 0)) {
      FUN_004122a0(*(undefined1 *)(param_2 + 0x59));
    }
  }
  return;
}
```

### CSWGuiPanel::Draw  gog=0x90d110 (param_bytes=4, notes='CSWGuiPanel * this, float param_1')
steam: cand=0x0040ec90(mapper) delta=-0x4fe480 align=6/6 size=432 retn=4 gapG/S=432/432 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, float param1) [param_bytes=4]
name_map: CSWGuiPanel::Draw [medium] mapper vote GOG 0x90d110 (retn=4), Steam ret=4, sz=432
callers=27 callees=004799c0, 00479460, 004793f0, 00410020=gog:CSWGuiPanel::GetExtentAccountingForPanelOffset, 004792c0
```
void __thiscall FUN_0040ec90(int param_1,undefined4 param_2)
{
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
        FUN_00410020(&local_14);
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

### CSWGuiPanel::StartLoadFromLayout  gog=0x90d2c0 (param_bytes=12, notes='CSWGuiPanel * this, CResRef * param_1, int param_2, int param_3')
steam: cand=0x0040ee40(mapper) delta=-0x4fe480 align=6/6 size=1347 retn=12 gapG/S=1360/1360 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* guiResref) [param_bytes=4]
callers=72 callees=00919723=operator new, 007189d0=gog:CResGFF::GetTopLevelStruct, 0040e2c0, 00733570=CExoString::CExoString_char, 00719520=gog:CResGFF::ReadFieldVector, 00718e40=gog:CResGFF::ReadFieldCResRef, 00718d00=gog:CResGFF::ReadFieldFLOAT, 0073fea0=CClientExoApp::GetSWGuiManager, 007111e0=CRes::Demand, 00718960=gog:CResGFF::GetStructFromStruct, 00718760=gog:CResGFF::GetList, 00710840, 0091c860=_ftol2_sse, 00733780=CExoString::~CExoString ...
strings: CONTROLS | ALPHA | BACKGROUND | BORDER | COLOR
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
void __thiscall FUN_0040ee40(int *param_1,undefined4 param_2,int param_3,int param_4)
{
  float10 fVar4;
  undefined1 local_90 [16];
  undefined1 local_80 [16];
  undefined1 local_60 [8];
  undefined1 local_58 [12];
  undefined1 local_38 [4];
  undefined1 local_34 [4];
  undefined1 local_30 [4];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00948313;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (((uint)param_1[0x12] >> 1 & 1) == 0) {
    local_40 = FUN_00919723(0xa0,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (local_40 == 0) {
      local_c4 = 0;
    }
    else {
      local_c4 = FUN_00716fd0(0x7ff,&DAT_00987478,param_2);
    }
    local_3c = local_c4;
    local_8 = 0xffffffff;
    param_1[0xc] = local_c4;
    local_1c = param_1[0xc];
    iVar1 = FUN_007111e0();
    if (iVar1 != 0) {
      FUN_007189d0(local_38);
      FUN_0040e2c0(local_1c,local_38);
      local_4c = _DAT_00987474;
      local_48 = _DAT_00987474;
      local_44 = _DAT_00987474;
      piVar2 = (int *)FUN_00719520(local_58,local_38,"COLOR",local_30,&local_4c);
      param_1[0x15] = *piVar2;
      param_1[0x16] = piVar2[1];
      param_1[0x17] = piVar2[2];
      iVar1 = FUN_00718960(local_34,local_38,"BORDER");
      if (iVar1 == 0) {
        FUN_00710840(&DAT_00986925);
        uVar3 = FUN_00718e40(local_90,local_38,"BACKGROUND",local_30,local_80);
        FUN_00414840(uVar3,0);
... (64 more lines)
```

### CSWGuiPanel::SetBackground  gog=0x90d810 (param_bytes=4, notes='CSWGuiPanel * this, CResRef * image')
steam: cand=0x0040f390(mapper) delta=-0x4fe480 align=6/6 size=41 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* image) [param_bytes=4]
callers=5 callees=00414840=CSWGuiControl image setter thiscall(CResRef* name,
```
void FUN_0040f390(undefined4 param_1)
{
  FUN_00414840(param_1,0);
  return;
}
```

### CSWGuiPanel::OnPanelAdded  gog=0x90d840 (param_bytes=0, notes='CSWGuiPanel * this')
steam: cand=0x0040f3c0(mapper) delta=-0x4fe480 align=6/6 size=245 retn=0 gapG/S=256/256 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=40 callees=00413c50=gog:CSWGuiManager::UpdateMouseOverControl
```
void __fastcall FUN_0040f3c0(int *param_1)
{
  (**(code **)(*param_1 + 0x6c))();
  FUN_00413c50();
  piVar1 = *(int **)(param_1[7] + 8);
  if (piVar1 != (int *)0x0) {
    for (local_8 = 0; local_8 < param_1[10]; local_8 = local_8 + 1) {
      if (*(int **)(param_1[9] + local_8 * 4) == piVar1) {
        if (((uint)piVar1[0x12] >> 3 & 1) == 0) {
          return;
        }
        *(int **)(param_1[7] + 8) = piVar1;
        (**(code **)(*piVar1 + 0xc))(1);
        if (param_1[8] == 0) {
          return;
        }
        if (piVar1 == (int *)param_1[8]) {
          return;
        }
        iVar2 = (**(code **)(*piVar1 + 0x58))();
        if (iVar2 == 0) {
          return;
        }
        if (((uint)piVar1[0x12] >> 2 & 1) == 0) {
          return;
        }
        (**(code **)(*(int *)param_1[8] + 0x3c))(1,1);
        return;
      }
    }
  }
  return;
}
```

### CSWGuiPanel::OnPanelRemoved  gog=0x90d940 (param_bytes=0, notes='CSWGuiPanel * this')
steam: cand=0x0040f4c0(mapper) delta=-0x4fe480 align=6/6 size=212 retn=0 gapG/S=224/224 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=17 callees=00413f00=gog:CSWGuiManager::RemoveToolTip
```
void __fastcall FUN_0040f4c0(int *param_1)
{
  FUN_00413f00();
  if ((*(int *)(DAT_00a1b49c + 0x18) != 0) &&
     (iVar1 = *(int *)(DAT_00a1b49c + 0x18), iVar3 = (**(code **)(*param_1 + 0x44))(),
     iVar1 == iVar3)) {
    (**(code **)(**(int **)(DAT_00a1b49c + 0x18) + 0x7c))(0);
  }
  piVar2 = *(int **)(param_1[7] + 8);
  if (piVar2 != (int *)0x0) {
    for (local_8 = 0; local_8 < param_1[10]; local_8 = local_8 + 1) {
      if (*(int **)(param_1[9] + local_8 * 4) == piVar2) {
        *(undefined4 *)(param_1[7] + 8) = 0;
        (**(code **)(*piVar2 + 0xc))(0);
        return;
      }
    }
  }
  return;
}
```

### CSWGuiPanel::StopLoadFromLayout  gog=0x90da20 (param_bytes=0, notes='CSWGuiPanel * this')
steam: cand=0x0040f5a0(mapper) delta=-0x4fe480 align=5/6 size=117 retn=0 gapG/S=128/128 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=73 callees=0040fb70=gog:CSWGuiPanel::ConnectControlMoves, 007112b0=CRes::Release
```
void __fastcall FUN_0040f5a0(int param_1)
{
  if ((*(uint *)(param_1 + 0x48) >> 1 & 1) != 0) {
    FUN_007112b0();
    if (*(undefined4 **)(param_1 + 0x30) == (undefined4 *)0x0) {
      uVar1 = 0;
    }
    else {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x30))(1);
    }
    *(undefined4 *)(param_1 + 0x30) = 0;
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffd;
    FUN_0040fb70(uVar1);
  }
  return;
}
```

### CSWGuiPanel::InitControl  gog=0x90daa0 (param_bytes=16, notes='CSWGuiPanel * this, CSWGuiControl * controlToInit, CExoString * label,')
steam: cand=0x0040f620(mapper) delta=-0x4fe480 align=5/6 size=970 retn=16 gapG/S=976/976 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* controlToInit, void* label, int activate) [param_bytes=12]
name_map: CSWGuiPanel::InitControl [medium] vote GOG p16 == ret 16; db_functions verified purpose; R8 called it "float math helper 970 B" (low) - that reading is wr
callers=76 callees=00418df0=gog:CSWGuiControl::Load_2, 0040fa80, 0040f9f0, 0040fb40, 0073fea0=CClientExoApp::GetSWGuiManager, 00734010=CExoString::CStr, 0083ea60=CExoArrayList<uint32>::Add, 0091d6f0, 0091c860=_ftol2_sse
strings: LBL_BAR
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
void __thiscall FUN_0040f620(int param_1,int *param_2,undefined4 param_3,int param_4,int param_5)
{
  float10 fVar8;
  FUN_00418df0(param_1,*(undefined4 *)(param_1 + 0x30),param_1 + 0x34,param_3);
  local_1c = param_2[1];
  local_18 = param_2[2];
  local_14 = param_2[3];
  local_10 = param_2[4];
  local_8 = 1.0;
  local_c = 1.0;
  if (param_5 != 0) {
    iVar6 = FUN_0073fea0();
    if (iVar6 != 0) {
      iVar6 = FUN_0073fea0();
      local_8 = (float)(int)*(short *)(iVar6 + 0x6c) / (float)DAT_009f4390;
      iVar6 = FUN_0073fea0();
      local_c = (float)(int)*(short *)(iVar6 + 0x6e) / (float)DAT_009f4394;
    }
  }
  local_10 = FUN_0091c860();
  local_14 = FUN_0091c860();
  local_18 = FUN_0091c860();
  local_1c = FUN_0091c860();
  if ((DAT_00a1b9f0 != 0) && (param_5 != 0)) {
    iVar6 = FUN_0073fea0();
    if (iVar6 != 0) {
      fVar2 = (float)local_1c;
      fVar3 = (float)local_14;
      fVar4 = (float)local_18;
      fVar5 = (float)local_10;
      iVar6 = FUN_0073fea0();
      sVar1 = *(short *)(iVar6 + 0x6c);
      iVar6 = FUN_0073fea0();
      FUN_0040f9f0((float)(int)sVar1,(float)(int)*(short *)(iVar6 + 0x6e),
                   (float)_DAT_00987488 / ((float)(int)sVar1 / (float)(int)*(short *)(iVar6 + 0x6e))
                   ,0x3f800000);
      FUN_0040fa80(fVar2,fVar4);
      fVar8 = (float10)FUN_0040fb40(fVar4);
      FUN_0040fa80(fVar3 + fVar2,(float)fVar8);
      FUN_0040fb40(fVar5 + fVar4);
      local_1c = FUN_0091c860();
      local_14 = FUN_0091c860();
      local_18 = FUN_0091c860();
      local_10 = FUN_0091c860();
... (24 more lines)
```

### CSWGuiPanel::GetFullScreenBG  gog=0x90e0d0 (param_bytes=4, notes='CSWGuiPanel * this, CExoString * param_1')
steam: cand=0x0040fc50(mapper) delta=-0x4fe480 align=6/6 size=290 retn=4 gapG/S=304/304 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* outBGString) [param_bytes=4]
callers=0 callees=00733df0=CExoString::operator+, 007335f0=CExoString::CExoString_copy, 00733570=CExoString::CExoString_char, 007338d0=CExoString::operator=_2, 007337c0=CExoString::operator=, 00733780=CExoString::~CExoString
strings: 1024x768 | 1280x960 | 1280x1024 | 1600x1200 | 800x600
```
undefined4 __thiscall FUN_0040fc50(int param_1,undefined4 param_2)
{
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00948369;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("800x600");
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
  uVar2 = FUN_00733df0(local_20,&DAT_00a1b9f8 + (uint)*(byte *)(param_1 + 0x4c) * 8);
  local_8._0_1_ = 2;
  FUN_007337c0(uVar2);
  local_8._0_1_ = 1;
  FUN_00733780(uVar1);
  FUN_007335f0(local_18);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  ExceptionList = local_10;
  return param_2;
}
```

### CSWGuiPanel::AddControl  gog=0x90e200 (param_bytes=4, notes='CSWGuiPanel * this, CSWGuiControl * param_1')
steam: cand=0x0040fd80(mapper) delta=-0x4fe480 align=6/6 size=57 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* control) [param_bytes=4]
callers=9 callees=0083ea60=CExoArrayList<uint32>::Add, 004142e0
```
void __thiscall FUN_0040fd80(undefined4 param_1,int param_2)
{
  iVar1 = FUN_004142e0(param_2);
  if (iVar1 == -1) {
    *(undefined4 *)(param_2 + 0x38) = param_1;
    FUN_0083ea60(param_2);
  }
  return;
}
```

### CSWGuiPanel::SetVisible  gog=0x90e2e0 (param_bytes=4, notes='CSWGuiPanel * this, int isVisible')
steam: cand=0x0040fe60(mapper) delta=-0x4fe480 align=6/6 size=59 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int isVisible) [param_bytes=4]
callers=15 callees=00411170=gog:CSWGuiManager::DetermineNeedToDraw
```
void __thiscall FUN_0040fe60(int param_1,uint param_2)
{
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffb | (param_2 & 1) << 2;
  if (*(int *)(param_1 + 0x1c) != 0) {
    FUN_00411170();
  }
  return;
}
```

### CSWGuiPanel::GetControl  gog=0x90e320 (param_bytes=4, notes='CSWGuiPanel * this, int param_1')
steam: cand=0x0040fea0(mapper) delta=-0x4fe480 align=6/6 size=48 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, int index) [param_bytes=4]
callers=2 callees=
```
undefined4 __thiscall FUN_0040fea0(int param_1,int param_2)
{
  if (param_2 < *(int *)(param_1 + 0x28)) {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x24) + param_2 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### CSWGuiPanel::GetLocalMouseCoords  gog=0x90e350 (param_bytes=8, notes='CSWGuiPanel * this, int * outX, int * outY')
steam: cand=0x0040fed0(mapper) delta=-0x4fe480 align=6/6 size=215 retn=8 gapG/S=224/224 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int* outX, int* outY) [param_bytes=8]
callers=6 callees=00410020=gog:CSWGuiPanel::GetExtentAccountingForPanelOffset
```
void __thiscall FUN_0040fed0(int param_1,int *param_2,int *param_3)
{
  *param_2 = *DAT_00a1b49c;
  *param_3 = DAT_00a1b49c[1];
  local_14 = *(int *)(param_1 + 4);
  local_10 = *(int *)(param_1 + 8);
  local_c = *(undefined4 *)(param_1 + 0xc);
  local_8 = *(undefined4 *)(param_1 + 0x10);
  if ((*(uint *)(param_1 + 0x48) & 1) == 0) {
    *param_2 = *param_2 - (DAT_009f42a4 + -0x280) / 2;
    *param_3 = *param_3 - (DAT_009f42a8 + -0x1e0) / 2;
  }
  else {
    FUN_00410020(&local_14);
  }
  *param_2 = *param_2 - local_14;
  *param_3 = *param_3 - local_10;
  return;
}
```

### CSWGuiPanel::ResetFont  gog=0x90e430 (param_bytes=0, notes='CSWGuiPanel * this')
steam: cand=0x0040ffb0(mapper) delta=-0x4fe480 align=6/6 size=99 retn=0 gapG/S=112/112 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=1 callees=
```
void __fastcall FUN_0040ffb0(int param_1)
{
  if (*(int *)(param_1 + 0x18) != 0) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x28); local_8 = local_8 + 1) {
      if (*(int *)(*(int *)(param_1 + 0x24) + local_8 * 4) != 0) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_8 * 4) + 0x8c))();
      }
    }
  }
  return;
}
```

### CSWGuiPanel::GetExtentAccountingForPanelOffset  gog=0x90e4a0 (param_bytes=4, notes='CSWGuiPanel * this, CSWGuiExtent * outExtent')
steam: cand=0x00410020(mapper) delta=-0x4fe480 align=6/6 size=156 retn=4 gapG/S=160/160 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, CSWGuiExtent* outExtent) [param_bytes=4]
callers=5 callees=
```
void __thiscall FUN_00410020(int param_1,int *param_2)
{
  *param_2 = *(int *)(param_1 + 4);
  param_2[1] = *(int *)(param_1 + 8);
  param_2[2] = *(int *)(param_1 + 0xc);
  param_2[3] = *(int *)(param_1 + 0x10);
  if ((*(uint *)(param_1 + 0x48) >> 3 & 1) == 0) {
    if ((*(uint *)(param_1 + 0x48) >> 4 & 1) != 0) {
      *param_2 = (*(short *)(*(int *)(param_1 + 0x1c) + 0x6c) + -0x280) / 2 + *param_2;
    }
    if ((*(uint *)(param_1 + 0x48) >> 4 & 2) != 0) {
      param_2[1] = (*(short *)(*(int *)(param_1 + 0x1c) + 0x6e) + -0x1e0) / 2 + param_2[1];
    }
  }
  return;
}
```

### Scene::SetFogColor  gog=0x856850 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00464470(mapper) delta=-0x3f23e0 align=3/6 size=143 retn=4 gapG/S=144/144 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector* color) [param_bytes=4]
callers=0 callees=0044b8a0
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
void __thiscall FUN_00464470(int param_1,undefined4 *param_2)
{
  float10 fVar1;
  *(undefined4 *)(param_1 + 0x74) = *param_2;
  *(undefined4 *)(param_1 + 0x78) = param_2[1];
  *(undefined4 *)(param_1 + 0x7c) = param_2[2];
  local_14 = _DAT_00986b60;
  local_10 = _DAT_0098baa0;
  local_c = _DAT_0098ba9c;
  fVar1 = (float10)FUN_0044b8a0(param_1 + 0x74,&local_14);
  *(float *)(param_1 + 0x88) = (float)fVar1 * _DAT_00a04900;
  *(undefined4 *)(param_1 + 0x84) = *(undefined4 *)(param_1 + 0x88);
  *(undefined4 *)(param_1 + 0x80) = *(undefined4 *)(param_1 + 0x84);
  return;
}
```

### Scene::SetFog  gog=0x857010 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00464c30(mapper) delta=-0x3f23e0 align=6/6 size=22 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int fog) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_00464c30(int param_1,undefined4 param_2)
{
  *(undefined4 *)(param_1 + 0x60) = param_2;
  return;
}
```

### Scene::GetFogRange  gog=0x857030 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00464c50(mapper) delta=-0x3f23e0 align=6/6 size=35 retn=8 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, float* start, float* end) [param_bytes=8]
callers=0 callees=
```
void __thiscall FUN_00464c50(int param_1,undefined4 *param_2,undefined4 *param_3)
{
  *param_2 = *(undefined4 *)(param_1 + 100);
  *param_3 = *(undefined4 *)(param_1 + 0x68);
  return;
}
```

### Scene::SetFogRange  gog=0x857060 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00464c80(mapper) delta=-0x3f23e0 align=6/6 size=31 retn=8 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, float fogStart, float fogEnd) [param_bytes=8]
callers=0 callees=
```
void __thiscall FUN_00464c80(int param_1,undefined4 param_2,undefined4 param_3)
{
  *(undefined4 *)(param_1 + 100) = param_2;
  *(undefined4 *)(param_1 + 0x68) = param_3;
  return;
}
```

### Scene::SetSceneFocus  gog=0x857080 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00464ca0(mapper) delta=-0x3f23e0 align=6/6 size=38 retn=12 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector focus) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_00464ca0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)
{
  *(undefined4 *)(param_1 + 0xb4) = param_2;
  *(undefined4 *)(param_1 + 0xb8) = param_3;
  *(undefined4 *)(param_1 + 0xbc) = param_4;
  return;
}
```

### Scene::IsGeomFadeEnabled  gog=0x8570b0 (param_bytes=0, notes='void')
steam: cand=0x00464cd0(mapper) delta=-0x3f23e0 align=6/6 size=25 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: bool (__stdcall) () [param_bytes=0]
callers=0 callees=
```
undefined4 FUN_00464cd0(void)
{
  return CONCAT31((int3)((uint)DAT_009f5d28 >> 8),DAT_009f5d28 == DAT_0098ba8c);
}
```

### Scene::EnableAnimations  gog=0x857590 (param_bytes=0, notes='void')
steam: cand=0x004651b0(mapper) delta=-0x3f23e0 align=6/6 size=21 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_004651b0(int param_1)
{
  *(undefined1 *)(param_1 + 0xe6) = 1;
  return;
}
```

### Scene::DisableAnimations  gog=0x8575b0 (param_bytes=0, notes='void')
steam: cand=0x004651d0(mapper) delta=-0x3f23e0 align=5/6 size=21 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_004651d0(int param_1)
{
  *(undefined1 *)(param_1 + 0xe6) = 0;
  return;
}
```

### Scene::IsAnimating  gog=0x8575d0 (param_bytes=0, notes='void')
steam: cand=0x004651f0(mapper) delta=-0x3f23e0 align=5/6 size=20 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: bool (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
undefined1 __fastcall FUN_004651f0(int param_1)
{
  return *(undefined1 *)(param_1 + 0xe6);
}
```

### Scene::SetBlindLights  gog=0x8575f0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00465210(mapper) delta=-0x3f23e0 align=5/6 size=30 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* object) [param_bytes=4]
callers=0 callees=
```
void FUN_00465210(undefined4 param_1)
{
  DAT_00a7be44 = param_1;
  return;
}
```

### Scene::GetObjectA  gog=0x857670 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00465290(mapper) delta=-0x3f23e0 align=5/6 size=102 retn=4 gapG/S=624/112 cc_guess=thiscall
wrapper: Gob* (__thiscall) (void* thisPtr, char* objectName) [param_bytes=4]
callers=0 callees=009370c7=_stricmp
```
undefined4 __thiscall FUN_00465290(int param_1,char *param_2)
{
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x58) <= local_8) {
      return 0;
    }
    iVar1 = __stricmp(param_2,(char *)(*(int *)(*(int *)(param_1 + 0x54) + local_8 * 4) + 100));
    if (iVar1 == 0) break;
    local_8 = local_8 + 1;
  }
  return *(undefined4 *)(*(int *)(param_1 + 0x54) + local_8 * 4);
}
```

### Scene::ChildAdd  gog=0x857f90 (param_bytes=4, notes='Scene * this, Gob * param_1')
steam: cand=0x00465bb0(mapper) delta=-0x3f23e0 align=5/6 size=112 retn=4 gapG/S=112/112 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* child) [param_bytes=4]
callers=0 callees=004659f0=gog:Scene::ListSpecialPartsR, 00495f20, 00473f40=vector<T*>::push_back (`ret 4`)
```
void __thiscall FUN_00465bb0(undefined4 param_1,int *param_2)
{
  if (param_2[0x21] != 0) {
    FUN_00495f20(param_1,param_2);
    FUN_004659f0(param_2[0x26]);
  }
  FUN_00473f40(param_2);
  iVar1 = (**(code **)(*param_2 + 0x1a8))();
  if (iVar1 != 0) {
    FUN_00473f40(param_2);
  }
  return;
}
```

### Scene::ChildRemove  gog=0x858000 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00465c20(mapper) delta=-0x3f23e0 align=4/6 size=112 retn=4 gapG/S=112/112 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* child) [param_bytes=4]
callers=0 callees=00449170, 00495fb0, 00465ae0=gog:Scene::UnlistSpecialParts
```
void __thiscall FUN_00465c20(undefined4 param_1,int *param_2)
{
  if (param_2[0x21] != 0) {
    FUN_00495fb0(param_1,param_2);
    FUN_00465ae0(param_2[0x26]);
  }
  FUN_00449170(param_2);
  iVar1 = (**(code **)(*param_2 + 0x1a8))();
  if (iVar1 != 0) {
    FUN_00449170(param_2);
  }
  return;
}
```

### Scene::UpdateEmitters  gog=0x85e510 (param_bytes=0, notes='void')
steam: cand=0x0046c190(mapper) delta=-0x3f2380 align=5/6 size=281 retn=0 gapG/S=288/288 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=1 callees=004ec080=PartEmitter::AnimateParticles (per tick, `ret 4`)
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
void __fastcall FUN_0046c190(int param_1)
{
  DAT_00a20020 = param_1;
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0xd8); local_8 = local_8 + 1) {
    iVar1 = *(int *)(*(int *)(*(int *)(param_1 + 0xd4) + local_8 * 4) + 0x58);
    for (local_10 = 0; local_10 < *(int *)(iVar1 + 0x28); local_10 = local_10 + 1) {
      for (local_c = 0; local_c < 100; local_c = local_c + 1) {
        FUN_004ec080(_DAT_009876fc);
      }
    }
  }
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0xa4); local_8 = local_8 + 1) {
    for (local_c = 0; local_c < 100; local_c = local_c + 1) {
      FUN_004ec080(_DAT_009876fc);
    }
  }
  DAT_00a20020 = 0;
  return;
}
```

### Scene::EnableVisibilityGraph  gog=0x861d50 (param_bytes=0, notes='void')
steam: cand=0x0046fb10(mapper) delta=-0x3f2240 align=6/6 size=44 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: bool (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
bool __fastcall FUN_0046fb10(int param_1)
{
  *(undefined2 *)(param_1 + 0xe4) = 0;
  return *(int *)(param_1 + 0xe0) != 0;
}
```

### Scene::DisableVisibilityGraph  gog=0x861d80 (param_bytes=0, notes='void')
steam: cand=0x0046fb40(mapper) delta=-0x3f2240 align=6/6 size=26 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_0046fb40(int param_1)
{
  *(undefined2 *)(param_1 + 0xe4) = 1;
  return;
}
```

### Scene::GetIgnoreVisibilityGraph  gog=0x861da0 (param_bytes=0, notes='void')
steam: cand=0x0046fb60(mapper) delta=-0x3f2240 align=6/6 size=21 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
int __fastcall FUN_0046fb60(int param_1)
{
  return (int)*(short *)(param_1 + 0xe4);
}
```

### Scene::LoadVisibility  gog=0x861e10 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0046fbd0(mapper) delta=-0x3f2240 align=6/6 size=330 retn=4 gapG/S=336/336 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, char* visibility) [param_bytes=4]
callers=0 callees=0046fb80=gog:Scene::SetVisibility, 0046feb0, 0091c7ad, 00475ea0=CloseModelStream, 00475f70, 009196ee=__security_check_cookie, 00475ab0
```
void __thiscall FUN_0046fbd0(int *param_1,undefined4 param_2)
{
  undefined1 local_a4 [68];
  undefined1 local_5c [64];
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  FUN_0046fb80();
  local_8 = FUN_00475ab0(param_2,&DAT_0098bda8,0,1);
  if (local_8 != 0) {
    while (local_10 = (char *)FUN_00475f70(), local_10 != (char *)0x0) {
      local_5c[0] = 0;
      local_18 = 0;
      FID_conflict__sscanf(local_10,"%s%d",local_5c,&local_18);
      local_14 = (**(code **)(*param_1 + 0x74))(local_5c);
      if ((local_14 != 0) && (local_18 != 0)) {
        for (local_c = 0; local_c < local_18; local_c = local_c + 1) {
          local_a4[0] = 0;
          local_10 = (char *)FUN_00475f70();
          FID_conflict__sscanf(local_10,"%s",local_a4);
          local_60 = (**(code **)(*param_1 + 0x74))(local_a4);
          if (local_60 != 0) {
            FUN_0046feb0(local_14,local_60,1);
          }
        }
      }
    }
    FUN_00475ea0(local_8,0);
  }
  __security_check_cookie(local_1c ^ (uint)&stack0xfffffffc);
  return;
}
```

### Scene::GetNumRooms  gog=0x8621b0 (param_bytes=0, notes='void')
steam: cand=0x0046ff70(mapper) delta=-0x3f2240 align=6/6 size=20 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
undefined4 __fastcall FUN_0046ff70(int param_1)
{
  return *(undefined4 *)(param_1 + 0xd8);
}
```

### Scene::SetCurrentRoom_2  gog=0x8621f0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0046ffb0(mapper) delta=-0x3f2240 align=5/6 size=72 retn=4 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int index) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_0046ffb0(int param_1,int param_2)
{
  if ((param_2 < 0) || (*(int *)(param_1 + 0xd8) <= param_2)) {
    *(undefined4 *)(param_1 + 0xe0) = 0;
  }
  else {
    *(undefined4 *)(param_1 + 0xe0) = *(undefined4 *)(*(int *)(param_1 + 0xd4) + param_2 * 4);
  }
  return;
}
```

### Scene::SpawnRoom  gog=0x8624d0 (param_bytes=12, notes='Scene * this, char * modelName, Vector * position, Quaternion * orient')
steam: cand=0x00470290(mapper) delta=-0x3f2240 align=4/6 size=2598 retn=12 gapG/S=2608/2608 cc_guess=thiscall
wrapper: Gob* (__thiscall) (void* thisPtr, char* modelName, Vector* position, Quaternion* orientation) [param_bytes=12]
callers=0 callees=00919723=operator new, 0045fde0, 004545a0, 0046ea30, 0046e9b0, 00513e20, 009196ee=__security_check_cookie, 00428f60=Global::AurTextureSetModeDynamic, 0046f8e0, 0048e160=Global::FindModel, 00449230, 00463ed0, 00461aa0=gog:Gob::SetScene_2, 0050e6e0 ...
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
void __thiscall
FUN_00470290(int *param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4)
{
  undefined1 local_144 [12];
  undefined1 local_138 [12];
  undefined1 local_12c [12];
  undefined1 local_120 [12];
  char local_cc [32];
  undefined1 local_88 [68];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a86a;
  local_10 = ExceptionList;
  local_44 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00428f50(local_44);
  local_40 = FUN_0048e160(param_2);
  if (local_40 != 0) {
    *(ushort *)(local_40 + 0x50) = *(ushort *)(local_40 + 0x50) | 0x100;
    local_3c = 0;
    local_9c = 0;
    local_f4 = FUN_00919723(0xbc);
    local_8 = 0;
    if (local_f4 == 0) {
      local_17c = 0;
    }
    else {
      local_17c = FUN_0046f8e0(param_2);
    }
    local_f0 = local_17c;
    local_8 = 0xffffffff;
    local_28 = local_17c;
    local_fc = FUN_00919723(0x98);
    local_8 = 1;
    if (local_fc == 0) {
      local_180 = 0;
    }
    else {
      local_180 = FUN_00495860();
    }
    local_f8 = local_180;
    local_8 = 0xffffffff;
    local_98 = local_180;
    local_21 = '\0';
... (164 more lines)
```

### Scene::RemoveFromSceneManager_2  gog=0x8646a0 (param_bytes=4, notes='undefined4 param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### Scene::SetObjectScale  gog=0x8647f0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x004725b0(mapper) delta=-0x3f2240 align=2/6 size=46 retn=8 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* object, float scale) [param_bytes=8]
callers=0 callees=
```
void __thiscall FUN_004725b0(undefined4 param_1,int *param_2,undefined4 param_3)
{
  (**(code **)(*param_2 + 0xcc))(param_3,1,param_1);
  return;
}
```

### Scene::SetGeomFadeProperties  gog=0x864820 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x004725e0(mapper) delta=-0x3f2240 align=2/6 size=45 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: unsigned char (__stdcall) (int property) [param_bytes=4]
callers=0 callees=
```
undefined4 FUN_004725e0(int param_1)
{
  if (((param_1 == 1) || (param_1 == 2)) || (param_1 == 4)) {
    DAT_009f5d28 = param_1;
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### Scene::GetObjectScale  gog=0x864850 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00472610(mapper) delta=-0x3f2240 align=2/6 size=22 retn=4 gapG/S=4981/32 cc_guess=thiscall
wrapper: double (__stdcall) (void* object) [param_bytes=4]
callers=0 callees=
```
float10 FUN_00472610(int param_1)
{
  return (float10)*(float *)(param_1 + 0x1e0);
}
```

