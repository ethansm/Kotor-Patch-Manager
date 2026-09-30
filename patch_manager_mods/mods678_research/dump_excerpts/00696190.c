// FN 00696190 FUN_00696190 size=190 ret=8 callers= callees=006fdaf0,006fd9c0,0051c080,0053dfb0

undefined4 FUN_00696190(void)

{
  undefined2 uVar1;
  int iVar2;
  undefined4 uVar3;
  int **ppiVar4;
  undefined4 extraout_var;
  int *local_10;
  undefined4 local_c;
  char local_5;
  
  uVar1 = 0;
  iVar2 = FUN_006fdaf0(&local_c);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    ppiVar4 = &local_10;
    FUN_0051c080(local_c,ppiVar4,extraout_var);
    local_5 = FUN_0053dfb0(local_c,ppiVar4);
    if (local_5 == DAT_0099447f) {
      iVar2 = (**(code **)(*local_10 + 0x30))();
      if (iVar2 != 0) {
        iVar2 = (**(code **)(*local_10 + 0x30))();
        uVar1 = *(undefined2 *)(*(int *)(iVar2 + 0x1198) + 0x146);
      }
    }
    iVar2 = FUN_006fd9c0(uVar1);
    if (iVar2 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      uVar3 = 0;
    }
  }
  return uVar3;
}

