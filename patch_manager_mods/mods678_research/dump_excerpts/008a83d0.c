// FN 008a83d0 FUN_008a83d0 size=174 ret=4 callers= callees=008b1cc0,0051c0e0

void FUN_008a83d0(int param_1)

{
  int iVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0x50) != 0) {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    iVar1 = FUN_0051c0e0(*(undefined4 *)(param_1 + 0x1d0));
    if ((iVar1 != 0) && ((*(uint *)(iVar1 + 0x2c8) >> 7 & 1) != 0)) {
      *(uint *)(iVar1 + 0x2c8) = *(uint *)(iVar1 + 0x2c8) & 0xffffff7f;
      *(uint *)(param_1 + 0x3ac) = *(uint *)(param_1 + 0x3ac) & 0xfffffffb;
      FUN_008b1cc0(extraout_var,uVar2);
    }
  }
  return;
}

