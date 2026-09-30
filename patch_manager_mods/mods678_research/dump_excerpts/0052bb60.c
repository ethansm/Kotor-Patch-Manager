// FN 0052bb60 FUN_0052bb60 size=767 ret=32 callers=0054c720,005bfc00,005cb430,006d3be0,006dc5c0,006d3a80,006d3db0,0069ac60,0054cba0,006d3920,006d4560,006d46d0,0057c990,00775aa0,00585cc0,005bc1d0,00577fa0,00576dc0,00521110,006d7390 callees=0052be60,004452b0,0083ea60,0051c0a0,0084a640,0064ddb0,005ff950,0052d130

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_0052bb60(int param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,int param_9)

{
  float fVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  float local_9c;
  float local_98;
  float local_94;
  int local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined1 local_80 [4];
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int *local_24;
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094e188;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8c = 0;
  local_88 = 0;
  local_84 = 0;
  local_2c = 0x7f000000;
  local_14 = _DAT_00993900;
  local_38 = 1;
  if ((float)param_3[2] <= (float)param_2[2]) {
    local_28 = param_3[2];
    local_34 = param_2[2];
  }
  else {
    local_28 = param_2[2];
    local_34 = param_3[2];
  }
  FUN_0084a640(0);
  local_8 = 0;
  local_24 = (int *)FUN_0051c0a0(param_6);
  if ((local_24 != (int *)0x0) && (iVar4 = (**(code **)(*local_24 + 0x10))(uVar3), iVar4 != 0)) {
    FUN_0083ea60(local_24);
  }
  local_24 = (int *)FUN_0051c0a0(param_7);
  if ((local_24 != (int *)0x0) && (iVar4 = (**(code **)(*local_24 + 0x10))(), iVar4 != 0)) {
    FUN_0083ea60(local_24);
  }
  FUN_0052be60();
  local_30 = FUN_0052d130(param_2,param_3,local_28,local_34,_DAT_0098bd74,0x7f000000,param_5,1,
                          local_20,param_8,local_80,0);
  if (local_30 == 0) {
    *param_4 = local_7c;
    param_4[1] = local_78;
    param_4[2] = local_74;
    local_38 = 0;
    if (param_9 == 0) goto LAB_0052be30;
  }
  for (local_90 = 0; local_90 < *(int *)(param_1 + 0x250); local_90 = local_90 + 1) {
    iVar4 = FUN_005ff950(*param_2,param_2[1],param_2[2],*param_3,param_3[1],param_3[2],&local_8c,
                         &local_2c);
    if (iVar4 == 0) {
      FUN_004452b0(&local_9c,&local_8c,param_2);
      fVar1 = local_9c * local_9c + local_98 * local_98 + local_94 * local_94;
      if (fVar1 < local_14) {
        *param_4 = local_8c;
        param_4[1] = local_88;
        param_4[2] = local_84;
        local_38 = 0;
        local_14 = fVar1;
      }
    }
  }
LAB_0052be30:
  uVar2 = local_38;
  local_8 = 0xffffffff;
  Free();
  ExceptionList = local_10;
  return uVar2;
}

