// FN 00605f40 FUN_00605f40 size=162 ret=4 callers=008a7520 callees=0049b840,0051c0e0

void __thiscall FUN_00605f40(int param_1,int *param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = param_2[1];
  while (local_8 = local_8 + -1, -1 < local_8) {
    iVar1 = FUN_0051c0e0(*(undefined4 *)(*param_2 + local_8 * 4));
    if (iVar1 != 0) {
      *(uint *)(iVar1 + 0x2c8) = *(uint *)(iVar1 + 0x2c8) & 0xffffff7f;
      *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
      FUN_0049b840(local_8);
    }
  }
  return;
}

