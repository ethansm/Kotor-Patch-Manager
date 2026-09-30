// FN 00696580 FUN_00696580 size=197 ret=8 callers= callees=006fdaf0,0051c080,0053dfb0,006fd9a0

undefined4 FUN_00696580(void)

{
  int iVar1;
  undefined4 uVar2;
  int **ppiVar3;
  undefined4 extraout_var;
  undefined2 local_24;
  int *local_14;
  undefined4 local_10;
  undefined4 local_c;
  char local_5;
  
  local_10 = 0;
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fd9a0(&local_10);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      ppiVar3 = &local_14;
      FUN_0051c080(local_c,ppiVar3,uVar2);
      local_5 = FUN_0053dfb0(local_c,ppiVar3);
      if (local_5 == DAT_0099447f) {
        iVar1 = (**(code **)(*local_14 + 0x30))();
        if (iVar1 != 0) {
          iVar1 = (**(code **)(*local_14 + 0x30))();
          local_24 = (undefined2)local_10;
          *(undefined2 *)(*(int *)(iVar1 + 0x1198) + 0x146) = local_24;
        }
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}

