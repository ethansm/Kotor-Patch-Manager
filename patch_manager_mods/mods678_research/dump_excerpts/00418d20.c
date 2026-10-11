// FN 00418d20 FUN_00418d20 size=122 ret=8 callers=00902b60 callees=

int __thiscall FUN_00418d20(int param_1,int param_2,int param_3)

{
  if (((((*(uint *)(param_1 + 0x48) >> 1 & 1) == 0) || ((*(uint *)(param_1 + 0x48) >> 5 & 1) != 0))
      || (param_2 < *(int *)(param_1 + 4))) ||
     (((*(int *)(param_1 + 4) + *(int *)(param_1 + 0xc) < param_2 ||
       (param_3 < *(int *)(param_1 + 8))) ||
      (*(int *)(param_1 + 8) + *(int *)(param_1 + 0x10) < param_3)))) {
    param_1 = 0;
  }
  return param_1;
}

