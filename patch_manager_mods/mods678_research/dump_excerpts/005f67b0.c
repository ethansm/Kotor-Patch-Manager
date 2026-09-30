// FN 005f67b0 FUN_005f67b0 size=157 ret=4 callers=00742a00,00549780 callees=00606100

int __thiscall FUN_005f67b0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint local_14;
  int local_10;
  
  if (*(int *)(param_1 + 0x37c) == 0) {
    local_10 = 0;
  }
  else {
    local_10 = 0;
    iVar1 = *(int *)(param_1 + 0x3c4);
    if (iVar1 != 0) {
      for (local_14 = 0; local_14 < *(uint *)(iVar1 + 0x10); local_14 = local_14 + 1) {
        iVar2 = FUN_00606100(local_14);
        if ((iVar2 != 0) && ((param_2 == 0 || ((*(uint *)(iVar2 + 0x2c8) >> 3 & 1) != 0)))) {
          local_10 = local_10 + 1;
        }
      }
    }
  }
  return local_10;
}

