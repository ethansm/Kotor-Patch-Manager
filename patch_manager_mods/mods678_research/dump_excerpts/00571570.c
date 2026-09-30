// FN 00571570 FUN_00571570 size=294 ret=12 callers=00571540,00577fa0,006d7390 callees=006b7bb0,005716a0,006d0620,006d6e30,00585f10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00571570(undefined4 param_1,int param_2,int param_3)

{
  char cVar1;
  int iVar2;
  float10 fVar3;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  float local_c;
  int local_8;
  
  local_c = 0.0;
  local_8 = FUN_006d0620(0x10);
  if ((local_8 == 0) || (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0x1a) == '\0')) {
    if (param_2 == 0) {
      fVar3 = (float10)FUN_005716a0(param_1,0);
      local_c = (float)(fVar3 + (float10)_DAT_00987f08);
    }
    else {
      local_1c = 0;
      local_18 = 0;
      local_14 = 0;
      FUN_00585f10(param_1,&local_1c,&local_c,0,0);
      local_c = local_c + (float)_DAT_00987df0;
    }
  }
  else if (param_3 == 0) {
    iVar2 = FUN_006d6e30();
    local_c = *(float *)(iVar2 + 0x34);
    if (local_c == (float)_DAT_00986b38) {
      local_c = _DAT_009919f0;
    }
    cVar1 = FUN_006b7bb0(3,0,0);
    if ('\0' < cVar1) {
      local_c = (float)(int)cVar1 + local_c;
    }
  }
  else {
    local_c = _DAT_0098bd6c;
  }
  return (float10)local_c;
}

