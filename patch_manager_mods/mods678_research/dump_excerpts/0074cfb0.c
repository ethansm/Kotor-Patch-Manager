// FN 0074cfb0 FUN_0074cfb0 size=48 ret=8 callers=007ce710 callees=

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0074cfb0(int param_1,int param_2)

{
  if (*(int *)(param_1 + 0x68) != param_2) {
    *(int *)(param_1 + 0x68) = param_2;
    *(undefined4 *)(param_1 + 0x595c) = _DAT_00987474;
  }
  return;
}

