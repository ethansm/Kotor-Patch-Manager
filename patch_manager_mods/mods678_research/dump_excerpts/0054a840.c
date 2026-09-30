// FN 0054a840 FUN_0054a840 size=153 ret=0 callers=005f6a70,00622bd0,00698190,005d75b0,00607690,00597d90,0058a500,0051d790,005d9da0,0058ae90 callees=0073fec0,007f2610,0073f4d0

undefined4 __fastcall FUN_0054a840(int param_1)

{
  undefined4 uVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  if (*(int *)(param_1 + 0x264) == 0) {
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    uVar1 = FUN_0073fec0(*(undefined4 *)(param_1 + 4));
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    piVar2 = (int *)FUN_0073f4d0(uVar1);
    if (piVar2 != (int *)0x0) {
      uVar3 = (**(code **)(*piVar2 + 0xc))(param_1,uVar4,uVar3);
      *(undefined4 *)(param_1 + 0x264) = uVar3;
      if (*(int *)(param_1 + 0x264) != 0) {
        FUN_007f2610(param_1);
      }
    }
  }
  return *(undefined4 *)(param_1 + 0x264);
}

