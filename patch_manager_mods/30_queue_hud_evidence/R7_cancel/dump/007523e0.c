// FN 007523e0 FUN_007523e0 size=112 ret=0 callers=00751710,00751750,007523b0 callees=0077fd20,004122a0,0073fb90,007e5d80,007f24c0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_007523e0(int param_1)

{
  int iVar1;
  
  FUN_0073fb90();
  iVar1 = FUN_007e5d80();
  if (iVar1 != 0) {
    FUN_0077fd20(0);
    FUN_007f24c0();
    FUN_004122a0(0);
    *(undefined4 *)(param_1 + 0x595c) = _DAT_00987474;
    *(undefined4 *)(param_1 + 0x70) = _DAT_00987474;
  }
  return;
}

