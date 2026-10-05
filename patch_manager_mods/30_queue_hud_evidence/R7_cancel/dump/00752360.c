// FN 00752360 FUN_00752360 size=75 ret=0 callers=00752330,007b12c0 callees=00740a20,007511b0

void __fastcall FUN_00752360(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(uint *)(param_1 + 0x68e4) >> 1 & 1) != 0) {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    iVar1 = FUN_00740a20(0x20,0x7f000000,0x7f000000,0);
    if (iVar1 == 0) {
      FUN_007511b0(param_1,uVar2);
    }
  }
  return;
}

