// FN 008a8cf0 FUN_008a8cf0 size=248 ret=4 callers= callees=00417140,0073f750

void __thiscall FUN_008a8cf0(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  undefined1 local_8;
  
  iVar1 = param_2[0x17];
  iVar2 = FUN_0073f750();
  local_8 = (undefined1)iVar1;
  *(undefined1 *)(iVar2 + 0xfc1) = local_8;
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  (**(code **)(*param_2 + 0x58))();
  FUN_00417140(&DAT_00a7fdac);
  *(uint *)(param_1 + 0x24d0) = *(uint *)(param_1 + 0x24d0) | 4;
  return;
}

