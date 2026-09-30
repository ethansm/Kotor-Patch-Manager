// FN 005716a0 FUN_005716a0 size=501 ret=8 callers=005a7820,00571570,006e3a40,006d7390 callees=0071b3f0,0051c0a0,00585f10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __thiscall FUN_005716a0(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  float local_14;
  int local_10;
  float local_c;
  int *local_8;
  
  local_c = 0.0;
  local_8 = (int *)FUN_0051c0a0(param_2);
  if (local_8 != (int *)0x0) {
    iVar1 = (**(code **)(*local_8 + 0x30))();
    if (iVar1 == 0) {
      iVar1 = (**(code **)(*local_8 + 0x18))();
      if (iVar1 == 0) {
        iVar1 = (**(code **)(*local_8 + 0x48))();
        if (iVar1 == 0) {
          local_c = _DAT_0098b5c0;
        }
        else {
          iVar1 = (**(code **)(*local_8 + 0x48))();
          local_30 = *(undefined4 *)(iVar1 + 0x94);
          local_2c = *(undefined4 *)(iVar1 + 0x98);
          local_28 = *(undefined4 *)(iVar1 + 0x9c);
          FUN_00585f10(param_2,&local_30,&local_c,0,0);
        }
      }
      else {
        local_24 = (**(code **)(*local_8 + 0x18))();
        local_20 = *(undefined4 *)(local_24 + 0x94);
        local_1c = *(undefined4 *)(local_24 + 0x98);
        local_18 = *(undefined4 *)(local_24 + 0x9c);
        FUN_00585f10(param_2,&local_20,&local_c,0,0);
      }
    }
    else {
      local_10 = (**(code **)(*local_8 + 0x30))();
      if (param_3 == 0) {
        local_c = _DAT_009951dc;
        local_14 = 0.0;
        FUN_0071b3f0(*(undefined2 *)(param_1 + 0x1184),DAT_00a0fddc,&local_14);
        local_c = local_c + local_14;
        FUN_0071b3f0(*(undefined2 *)(local_10 + 0x1184),DAT_00a0fddc,&local_14);
        local_c = local_c + local_14;
      }
      else {
        local_c = _DAT_009951e0;
      }
    }
  }
  return (float10)local_c;
}

