// FN 007511b0 FUN_007511b0 size=119 ret=0 callers=00752360,008995a0 callees=0073fb90,007e5d80,00541080,0077d800,005904c0

void __fastcall FUN_007511b0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  FUN_0073fb90(param_1);
  FUN_007e5d80();
  iVar1 = FUN_0077d800();
  if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10dc) != 0)) {
    iVar1 = FUN_005904c0(param_1,*(undefined4 *)(iVar1 + 0x10dc),*(int *)(iVar1 + 0x10dc),0,iVar1,
                         uVar2);
    if (iVar1 == 0) {
      FUN_00541080(1);
    }
  }
  return;
}

